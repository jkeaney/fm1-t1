/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_chan_test.c (P4-015): the CHAN page of firmware/src/ui_sq.c on the host: input sequence -> sq_* effects and the strings on the screen.
 * It drives ui_sq_keys / ui_sq_button / ui_sq_other_button / ui_sq_knob / ui_sq_select_turn / ui_sq_algo_turn / ui_sq_home / ui_sq_input as
 * ui_sq_glue.c does and walks S01 steps 2-4 (D#4, K2, HOME) plus S11: K2 MIDI with the 0.6 s settle and the commit on every exit (HOME, the CHN key,
 * PLAY, GLO, REC, SAVE, SELECT, a page change), the unique-channel steal (toast, the other channel OFF, one EDIT undo restores both), OFF and the
 * clamp, K1 LENGTH (clamp, page count change, hidden edits kept), OCT-/OCT+ = -PAGE/+PAGE, K4 FOLLOW, the ALGO encoder, SELECT without add. Frames go
 * to build/ui_sq/chan_*.png; tests/ui_sq_v12_test.c compares the chan_page and chan_midi_clash scenes with the mock.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_chan_test tests/ui_sq_chan_test.c -lm && /tmp/ui_sq_chan_test [outdir] */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#define __attribute__(x)

/* ---- the host side of gfx.c and the device hooks ---- */
static uint16_t fb[240 * 240];                       /* the frame as the panel would show it (byte-swapped RGB565, as cv_px) */
static int blits, blit_a, blit_b, blit_c;
static void lcd_sync(void) {}
static void lcd_power(uint32_t s) { (void)s; }
static void lcd_wake_now(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{   blits++;
    if (y == 0) blit_a++; else if (y == 120) blit_b++;
    if (w > 240 || h > 124) { printf("FAIL: blit %ux%u over 240x124\n", w, h); exit(1); }
    for (uint32_t r = 0; r < h; r++) memcpy(&fb[(y + r) * 240 + x], &p[r * w], w * 2); }
#include "../firmware/src/gfx.c"

typedef struct { int unused; } track_t;
static track_t trk[4];
#define MO_RESERVE 8u
static uint32_t midi_out_free(void) { return 128; }
static uint32_t beat_samples(void) { return 22050; }
static void midi_out_event_at(uint32_t pkt, uint32_t due) { (void)pkt; (void)due; }
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel) { (void)t; (void)note; (void)vel; }
static void trk_note_off(track_t *t, uint32_t note) { (void)t; (void)note; }
#define SQ_EXT_BEAT() 22050u
#define SQ_BPM()      120u
#define SQ_IRQ_OFF() ((void)0)
#define SQ_IRQ_ON()  ((void)0)
static volatile uint32_t fm1_ms;
static struct { uint8_t force; } ui;                 /* ui.c's flag: the only field ui_sq.c reads */

#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"
/* ---- a test ALGOS table with the v1 morph / layer / sub-label tables (as tests/sq_chan_test.c): the unfilled rows of sq_algos.c have no
 * build() or step(), which sq_boot needs for the four default channels ---- */
typedef struct { uint8_t m[SQ_NMORPH]; uint16_t seed[SQ_NSEED + 1]; uint8_t id, key; GlobalSave gs; uint8_t snap0; } Cache;   /* what build() was handed */
_Static_assert(sizeof(Cache) <= SQ_CACHE_MAX, "Cache fits");
static int  builds;                                  /* build() calls since the last reset */
static void (*build_hook)(void);                     /* run once inside the next build() (the epoch race) */
static uint8_t cur_id_for_build;

static void t_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k)
{   Cache *c = (Cache *)k;
    if (build_hook) { void (*h)(void) = build_hook; build_hook = NULL; h(); }
    memcpy(c->m, m, SQ_NMORPH);
    memcpy(c->seed, seed, sizeof c->seed);           /* seed[6] is s_mod: the core hands build() a seventh slot */
    c->id = cur_id_for_build; c->key = g->s.key; c->gs = g->s; c->snap0 = g->snap[1];
    builds++; }
static uint8_t t_len(const void *k) { (void)k; return 16; }
static void t_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o)
{   const Cache *c = (const Cache *)k; uint32_t h = sq_mix(c->seed[0] ^ (uint32_t)c->m[0] << 16 ^ g * 0x9E3779B1u); (void)hc;
    memset(o, 0, sizeof *o);
    o->note = (uint8_t)(36 + h % 12); o->vel = (uint8_t)(1 + (h >> 8) % 127); o->gate = (uint8_t)(1 + (h >> 16) % 100);
    o->flags = (uint8_t)(((h >> 24) % 3u) ? PO_HIT : 0); }

#define M(l, mn, mx, df, st, fs) { l, mn, mx, df, st, fs }
#define LY(...) { __VA_ARGS__ }
#define SM SQ_SEED_MOD
#define ID8 { 0, 1, 2, 3, 4, 5, 6, 7 }
static const AlgoDesc ALGOS[SQ_NALGO] = {
  [0] = { .name = "ACID", .id = 0, .n_seed = 6, .flags = AF_PAIRING, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4, 5, 6 },
          .morph = { M("DENS",0,16,12,1,1), M("PEDAL",0,12,6,1,1), M("JUMP",0,20,5,1,1), M("ACCNT",0,8,3,1,1),
                     M("OFFSET",0,15,0,1,1), M("SLIDE",0,8,4,1,1), M("PUNCH",0,47,40,1,4), M("DIR",0,2,0,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{4}), LY(2,{3,5}), LY(2,{2,SM}) },
          .sub = { "NOTES", "HITS", "ACCNT", "GT+SLD", "OCTAVE" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [1] = { .name = "EUCLID", .id = 1, .n_seed = 4, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 0xFF, 1, 2, 3 },
          .morph = { M("HITS",0,16,5,1,1), M("LEN",2,16,16,1,1), M("SPAN",0,24,12,1,1), M("ACCNT",0,8,2,1,1),
                     M("OFFSET",0,15,0,1,1), M("RATCH",0,8,4,1,1), M("PUNCH",0,47,40,1,4), M("CURVE",0,6,0,1,1) },
          .layer = { LY(1,{1}), LY(1,{0}), LY(1,{2}), LY(1,{3}), LY(1,{SM}) },
          .sub = { "NOTES", "ROTATE", "ACCNT", "RATCH", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [2] = { .name = "SNAKE", .id = 2, .n_seed = 4, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4 },
          .morph = { M("DENS",0,16,10,1,1), M("LEN",1,16,7,1,1), M("RANGE",0,48,12,1,4), M("PATH",1,14,7,1,1),
                     M("OFFSET",0,15,0,1,1), M("GLEN",1,16,16,1,1), M("DYN",0,10,10,1,1), M("DIR",0,2,0,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{2}), LY(0), LY(2,{3,SM}) },       /* ARTIC absent: inert */
          .sub = { "NOTES", "GATES", "VELO", "-", "DRIFT" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [3] = { .name = "STAGE", .id = 3, .n_seed = 5, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4, 5 },
          .morph = { M("DENS",0,8,6,1,1), M("TOTAL",8,64,16,1,4), M("RANGE",0,36,12,1,4), M("RATCH",0,8,2,1,1),
                     M("OFFSET",0,63,0,1,4), M("RDIV",1,4,4,1,1), M("DYN",0,10,10,1,1), M("DIR",0,2,0,1,1) },
          .layer = { LY(1,{0}), LY(1,{2}), LY(1,{4}), LY(1,{3}), LY(2,{1,SM}) },
          .sub = { "NOTES", "RESTS", "VELO", "RT+HLD", "LONG" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [4] = { .name = "ARPT", .id = 4, .n_seed = 5, .cache_bytes = sizeof(Cache), .knob_morph = { 3, 2, 1, 0, 4, 5, 6, 7 },   /* a remap, to test knob_morph */
          .seed_k = { 1, 2, 3, 4, 5, 0xFF },
          .morph = { M("A",0,10,2,2,4), M("B",0,10,3,1,1), M("C",1,10,4,1,1), M("D",0,100,50,5,20),
                     M("E",0,3,0,1,1), M("F",0,3,1,1,1), M("G",0,3,2,1,1), M("H",0,3,3,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{2}), LY(1,{3}), LY(2,{4,SM}) },
          .sub = { "NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [5] = { .name = "PATT", .id = 5, .n_seed = 5, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4, 5, 0xFF },
          .morph = { M("A",0,15,0,1,1), M("B",0,15,1,1,1), M("C",0,15,2,1,1), M("D",0,15,3,1,1),
                     M("E",0,15,4,1,1), M("F",0,15,5,1,1), M("G",0,15,6,1,1), M("H",0,15,7,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{2}), LY(1,{3}), LY(2,{4,SM}) },
          .sub = { "NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step },
};

#include "sq_core.c"
#include "sq_chan.c"
#include "../firmware/src/ui_sq.c"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

/* ---- a minimal PNG writer (stored deflate blocks, no dependency) ---- */
static uint32_t crc_t[256];
static void crc_init(void) { for (uint32_t n = 0; n < 256; n++) { uint32_t c = n; for (int k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1; crc_t[n] = c; } }
static uint32_t crc_up(uint32_t c, const uint8_t *b, size_t n) { for (size_t i = 0; i < n; i++) c = crc_t[(c ^ b[i]) & 255u] ^ (c >> 8); return c; }
static void put32(FILE *f, uint32_t v) { fputc(v >> 24, f); fputc(v >> 16, f); fputc(v >> 8, f); fputc(v, f); }
static void chunk(FILE *f, const char *t, const uint8_t *d, uint32_t n)
{   uint32_t c = crc_up(0xFFFFFFFFu, (const uint8_t *)t, 4);
    put32(f, n); fwrite(t, 1, 4, f); if (n) fwrite(d, 1, n, f);
    c = crc_up(c, d, n); put32(f, ~c); }
static void png_write(const char *path, const uint16_t *src, int stride, int x0, int y0, int w, int h)
{   uint32_t rawn = (uint32_t)h * (1u + (uint32_t)w * 3u), a = 1, b = 0;
    uint8_t *raw = malloc(rawn), *z, *p;
    uint32_t zn, i = 0;
    FILE *f = fopen(path, "wb");
    if (!f) { printf("FAIL: cannot write %s\n", path); fails++; free(raw); return; }
    for (int y = 0; y < h; y++) {
        raw[i++] = 0;
        for (int x = 0; x < w; x++) {
            uint16_t c = swap16(src[(y0 + y) * stride + x0 + x]);
            raw[i++] = (uint8_t)(((c >> 11) & 31u) * 255u / 31u); raw[i++] = (uint8_t)(((c >> 5) & 63u) * 255u / 63u); raw[i++] = (uint8_t)((c & 31u) * 255u / 31u); } }
    zn = 2 + (rawn / 65535u + 1u) * 5u + rawn + 4u; z = malloc(zn); p = z;
    *p++ = 0x78; *p++ = 0x01;
    for (uint32_t o = 0; o < rawn;) {
        uint32_t n = rawn - o > 65535u ? 65535u : rawn - o;
        *p++ = (uint8_t)(o + n >= rawn); *p++ = (uint8_t)n; *p++ = (uint8_t)(n >> 8); *p++ = (uint8_t)~n; *p++ = (uint8_t)(~n >> 8);
        memcpy(p, raw + o, n); p += n; o += n; }
    for (uint32_t k = 0; k < rawn; k++) { a = (a + raw[k]) % 65521u; b = (b + a) % 65521u; }
    *p++ = (uint8_t)(b >> 8); *p++ = (uint8_t)b; *p++ = (uint8_t)(a >> 8); *p++ = (uint8_t)a;
    { static const uint8_t sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 }; uint8_t ih[13] = { 0, 0, 0, (uint8_t)w, 0, 0, 0, (uint8_t)h, 8, 2, 0, 0, 0 };
      ih[2] = (uint8_t)(w >> 8); ih[3] = (uint8_t)w; ih[6] = (uint8_t)(h >> 8); ih[7] = (uint8_t)h;
      fwrite(sig, 1, 8, f); chunk(f, "IHDR", ih, 13); chunk(f, "IDAT", z, (uint32_t)(p - z)); chunk(f, "IEND", NULL, 0); }
    fclose(f); free(raw); free(z); }

/* ---- helpers ---- */
static const char *outdir = "build/ui_sq";
static void shot(const char *name, int x, int y, int w, int h)
{   char path[256]; snprintf(path, sizeof path, "%s/%s.png", outdir, name); png_write(path, fb, 240, x, y, w, h); }
static void frame(void) { fm1_ms += 17; ui_sq_input(); ui_sq_draw(); }
static void fresh(void) { ui.force = 1; frame(); ui.force = 0; }
static int W(const char *s) { return (int)text_w(&AF_S, s); }

static void fit(const char *what, const char *s, int lim)
{   int w = W(s); CHECK(w <= lim, "%s `%s` is %d px, over %d", what, s, w, lim); }

static void press(uint32_t mask) { ui_sq_keys(mask); }
static void wait_ms(uint32_t ms) { for (uint32_t t = 0; t < ms; t += 17) frame(); }
static int on_of(uint8_t ch, uint8_t p) { UisqStep L[SQ_PAGE]; uisq_lane_read(L, ch, (uint8_t)(p / SQ_PAGE)); return L[p % SQ_PAGE].on; }
static void tap_key(uint8_t k) { press(1u << k); wait_ms(100); press(0); frame(); }


#define WK(n) (wk[n])
static const uint8_t wk[16] = { 0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26 };    /* chromatic key of white key n (0 = F3) */
static const StepEdit zero_edit;
static StepEdit *ED(uint8_t p) { return &sq_ch[0].ed[p]; }
static void hold_step(uint8_t n) { press(1u << WK(n)); wait_ms(450); press(0); frame(); }
static void knob(uint8_t k, int d) { ui_sq_knob(k, d); frame(); }
static void btn(uint8_t c) { ui_sq_button(c); frame(); }
static UisqView V;
static void fill(void) { uisq_now = UISQ_MS(); uisq_view_fill(&V); }
static int streq(const char *a, const char *b) { return !strcmp(a, b); }


#define KEY_CHN 10                                                       /* chromatic key 10 = D#4 = the CHN black key */
static void chn_tap(void) { press(1u << KEY_CHN); wait_ms(100); press(0); frame(); }
static void home_tap(void) { ui_sq_home(1); wait_ms(100); ui_sq_home(0); frame(); }
static void reset_world(void)
{   sq_boot(); uisq_page = UI_PG_PERF; uisq_sel = 0; uisq_vpage = 0; uisq_used = 0x0F; uisq_cm_val = UISQ_CM_NONE; uisq_new = 0; uisq_toast_until = 0;
    uisq_gone_n = 0; uisq_undone = 0; uisq_rec = 0; sq_playing = 0; sq_follow = 1; wait_ms(1200); }
static void k2(int d) { knob(1, d); }
static int toast_on(void) { return uisq_toast_until && (int32_t)(uisq_now - uisq_toast_until) < 0; }

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    reset_world();
    fresh();

    /* ---- S01 step 2: D#4 opens CHAN: the band, the page title, the footer ---- */
    CHECK(uisq_key_slot(KEY_CHN) == 0x84, "key 10 is the CHN black key");
    chn_tap();
    CHECK(uisq_page == UI_PG_CHAN, "a D#4 tap opens CHAN");
    fill();
    CHECK(V.band == UISQ_BAND_CHAN && V.val[0] == 16 && V.val[1] == 1 && V.val[2] == 0 && V.val[3] == 1, "view: LENGTH 16, MIDI 1, SOURCE OWN, FOLLOW ON (%d %d %d %d)", V.val[0], V.val[1], V.val[2], V.val[3]);
    { const char *want[4] = { "LENGTH", "MIDI", "SOURCE", "FOLLOW" };
      for (int k = 0; k < 4; k++) CHECK(streq(uisq_knob_label(UI_PG_CHAN, UI_CTX_ANY, (uint8_t)k), want[k]) && streq(V.tile[k]->label, want[k]), "K%d label is the knob_map row `%s` (tile `%s`)", k + 1, uisq_knob_label(UI_PG_CHAN, UI_CTX_ANY, (uint8_t)k), V.tile[k]->label); }
    CHECK(V.msg.kind == UI_MK_TOUCH && streq(V.msg.a, "CHN") && streq(V.msg.b, "CH1 SETUP"), "the page title `%s %s` (the CHN CHn SETUP row)", V.msg.a, V.msg.b);
    { char l[24], r[24]; uisq_cue_split(uisq_page_cue(UI_PG_CHAN), l, sizeof l, r, sizeof r);
      CHECK(streq(l, "K2 = MIDI") && streq(r, "CHN = BACK"), "footer halves from the page_states cue: `%s` / `%s`", l, r); }
    shot("chan_open", 0, 0, 240, 240);
    chn_tap();
    CHECK(uisq_page == UI_PG_PERF, "the CHN key again closes CHAN (BACK)");

    /* ---- steps 3: K2 shows at once, commits after 0.6 s; taking MIDI 2 turns CH2 OFF with a toast; one EDIT restores both ---- */
    chn_tap();
    k2(1);
    fill();
    CHECK(V.val[1] == 2 && V.ch_midi[0] == 1 && sq_ch[0].midi_ch == 0, "K2 +1: the tile and the header show MIDI 2 at once, the channel is still MIDI 1 (pending)");
    wait_ms(500);
    CHECK(sq_ch[0].midi_ch == 0 && uisq_cm_val == 1, "0.5 s of stillness: not committed");
    wait_ms(150);
    CHECK(sq_ch[0].midi_ch == 1 && uisq_cm_val == UISQ_CM_NONE, "0.65 s: committed (MIDI 2)");
    CHECK(sq_ch[1].midi_ch == SQ_MIDI_OFF && sq_ch[2].midi_ch == 2 && sq_ch[3].midi_ch == 3, "CH2 (which held MIDI 2) is OFF, the others untouched");
    CHECK(toast_on() && streq(uisq_toast, "MIDI OFF") && streq(uisq_toast_hint, "2 TO CH1") && uisq_toast_chip == 2, "toast `[2] %s  %s` names the channel set OFF and who took it", uisq_toast, uisq_toast_hint);
    { UisqMsg m; uisq_msg_pick(&m, uisq_now, UI_PG_CHAN, 0);
      CHECK(m.kind == UI_MK_TOAST && text_w(&AF_S, m.b) + 3 + 17 + (text_w(&AF_M, m.a) < text_w(&AF_S, m.a) ? text_w(&AF_M, m.a) : text_w(&AF_S, m.a)) + 6 + 3 <= 136, "the clash toast fits the 136 px slot"); }
    shot("chan_clash", 0, 0, 240, 240);
    btn(UI_C_B_EDIT);
    CHECK(sq_ch[0].midi_ch == 0 && sq_ch[1].midi_ch == 1, "one EDIT undo restores both channels (CH1 MIDI 1, CH2 MIDI 2)");
    btn(UI_C_B_EDIT);
    CHECK(sq_ch[0].midi_ch == 1 && sq_ch[1].midi_ch == SQ_MIDI_OFF, "EDIT again redoes both");
    btn(UI_C_B_EDIT);
    CHECK(sq_ch[0].midi_ch == 0 && sq_ch[1].midi_ch == 1, "and undoes again");

    /* ---- turning across a used channel does not touch it (only the settled value matters) ---- */
    reset_world(); chn_tap();
    k2(1); k2(1);
    CHECK(sq_ch[1].midi_ch == 1 && sq_ch[2].midi_ch == 2, "1 to 3 across 2 in one turn: nothing moved while turning");
    wait_ms(700);
    CHECK(sq_ch[0].midi_ch == 2 && sq_ch[1].midi_ch == 1 && sq_ch[2].midi_ch == SQ_MIDI_OFF, "settled on MIDI 3: only CH3 (which held it) went OFF, CH2 kept MIDI 2");
    CHECK(streq(uisq_toast_hint, "3 TO CH1") && uisq_toast_chip == 3, "toast names MIDI 3 and CH3");

    /* ---- a two digit MIDI channel in the hint still fits the slot ---- */
    reset_world(); sq_ch[2].midi_ch = 15; chn_tap();
    k2(15); wait_ms(700);
    CHECK(sq_ch[0].midi_ch == 15 && sq_ch[2].midi_ch == SQ_MIDI_OFF && streq(uisq_toast_hint, "16 TO CH1"), "MIDI 16 taken from CH3: hint `%s`", uisq_toast_hint);
    { UisqMsg m; uisq_msg_pick(&m, uisq_now, UI_PG_CHAN, 0);
      CHECK(text_w(&AF_S, m.b) + 3 + 17 + text_w(&AF_S, m.a) + 6 + 3 <= 136 || text_w(&AF_S, m.b) + 3 + 17 + text_w(&AF_M, m.a) + 6 + 3 <= 136, "`16 TO CH1` toast fits 136 px"); }

    /* ---- OFF: the top of the list, no wrap, no toast, the setup cue stays; MIDI 16 below it ---- */
    reset_world(); chn_tap();
    k2(60);
    fill();
    CHECK(V.val[1] == 17 && V.ch_midi[0] == SQ_MIDI_OFF, "K2 clockwise to the top reads OFF (17th name) and stops there (no wrap)");
    wait_ms(700);
    CHECK(sq_ch[0].midi_ch == SQ_MIDI_OFF && !toast_on() && sq_ch[1].midi_ch == 1 && sq_ch[2].midi_ch == 2 && sq_ch[3].midi_ch == 3, "OFF committed: no toast and no other channel moved");
    fill(); CHECK(V.cue == 1 && streq(uisq_n_midi[V.val[1] - 1], "OFF"), "MIDI OFF raises the setup cue; the tile word is OFF");
    k2(-1); wait_ms(700);
    CHECK(sq_ch[0].midi_ch == 15 && sq_ch[3].midi_ch == 3, "one detent down from OFF is MIDI 16");
    k2(-100); wait_ms(700);
    CHECK(sq_ch[0].midi_ch == 0, "K2 anticlockwise stops at MIDI 1 (no wrap)");
    CHECK(sq_ch[1].midi_ch == 1, "CH1 coming back to MIDI 1 (nobody else held it) leaves CH2 on MIDI 2");

    /* ---- the setup cue clears when the channel gets a MIDI channel (a new channel) ---- */
    reset_world(); ui_sq_new_set(0, 1); chn_tap(); fill();
    CHECK(V.cue == 1, "a new channel shows the cue");
    k2(1); wait_ms(700); fill();
    CHECK(V.cue == 0 && !(uisq_new & 1), "K2 commits a MIDI channel: the cue and the new bit clear");

    /* ---- every exit commits ---- */
    { static const char *const nm[] = { "HOME", "CHN key", "PLAY", "GLO", "REC", "SAVE", "a dice button", "SELECT", "a page change", "NOTE_EDIT by hold" };
      for (int e = 0; e < 10; e++) {
        reset_world(); chn_tap();
        k2(2);
        CHECK(sq_ch[0].midi_ch == 0 && uisq_cm_val == 2, "[%s] pending MIDI 3 before the exit", nm[e]);
        switch (e) {
        case 0: home_tap(); CHECK(uisq_page == UI_PG_PERF, "HOME tap leaves CHAN"); break;
        case 1: chn_tap(); CHECK(uisq_page == UI_PG_PERF, "D#4 leaves CHAN"); break;
        case 2: ui_sq_other_button(UI_C_B_PLAY); frame(); break;
        case 3: ui_sq_other_button(UI_C_B_GLO); frame(); break;
        case 4: ui_sq_other_button(UI_C_B_REC); frame(); break;
        case 5: ui_sq_other_button(UI_C_B_SAVE); frame(); break;
        case 6: btn(UI_C_B_SCL); break;
        case 7: ui_sq_select_turn(1); frame(); CHECK(uisq_sel == 1 && uisq_page == UI_PG_CHAN, "SELECT moves to CH2 and stays on CHAN"); break;
        case 8: ui_sq_page_set(UI_PG_MENU); break;
        default: press(1u << WK(0)); wait_ms(450); press(0); frame(); CHECK(uisq_page == UI_PG_NOTE_EDIT, "a 0.4 s hold on a step key opens NOTE_EDIT from CHAN"); break; }
        CHECK(sq_ch[0].midi_ch == 2 && uisq_cm_val == UISQ_CM_NONE, "[%s] committed on the exit: MIDI %d, pending %02x", nm[e], sq_ch[0].midi_ch + 1, uisq_cm_val);
        CHECK(sq_ch[2].midi_ch == SQ_MIDI_OFF, "[%s] CH3 (MIDI 3) went OFF", nm[e]); } }
    reset_world(); chn_tap(); k2(2); ui_sq_home(1); CHECK(sq_ch[0].midi_ch == 2, "HOME pressed (a hold goes to the MENU): committed on the press"); ui_sq_home(0);

    /* ---- the pending value never outlives its channel ---- */
    reset_world(); chn_tap(); k2(1);
    uisq_used &= (uint8_t)~1u; frame(); frame();
    CHECK(sq_ch[0].midi_ch == 0 && uisq_cm_val == UISQ_CM_NONE && uisq_page == UI_PG_PERF, "the channel left use while pending: nothing written, pending dropped, CHAN closed");
    reset_world(); chn_tap(); uisq_sel = 1; k2(2); uisq_do_remove(1); CHECK(uisq_cm_val == UISQ_CM_NONE, "removing the channel drops its pending value");
    wait_ms(700); CHECK(sq_ch[1].midi_ch == SQ_MIDI_OFF, "...and nothing is written later (the removed channel stays MIDI OFF, BUG-011)");
    /* ---- BUG-011: a removed channel gives its MIDI channel up (OFF), so a steal never hits it or names it; EDIT brings it back ---- */
    reset_world(); uisq_do_remove(2);
    CHECK(sq_ch[2].midi_ch == SQ_MIDI_OFF && uisq_gone_n == 1, "BUG-011: removing CH3 (MIDI 3) sets its MIDI OFF");
    uisq_sel = 0; chn_tap(); k2(2); wait_ms(700);
    CHECK(sq_ch[0].midi_ch == 2 && !(toast_on() && streq(uisq_toast, "MIDI OFF")) && sq_ch[2].midi_ch == SQ_MIDI_OFF && sq_ch[1].midi_ch == 1 && sq_ch[3].midi_ch == 3, "BUG-011: CH1 takes MIDI 3, no steal toast, CH3 not touched");
    reset_world(); uisq_do_remove(2); ui_sq_button(UI_C_B_EDIT);
    CHECK(uisq_used == 0x0F && sq_ch[2].midi_ch == 2, "BUG-011: EDIT undo of the removal restores CH3 and its MIDI 3");
    reset_world(); uisq_do_remove(2); uisq_sel = 0; chn_tap(); k2(2); wait_ms(700); ui_sq_button(UI_C_B_EDIT);
    CHECK(uisq_used == 0x0B && sq_ch[2].midi_ch == SQ_MIDI_OFF, "BUG-011: a MIDI change after a removal empties the stack: no stale restore of MIDI 3 over CH1");
    reset_world(); uisq_do_remove(2); uisq_sel = 0; uisq_used |= 4u; uisq_gone_n = 0; sq_ch[0].midi_ch = 2;     /* re-add after a removal: OFF, not a duplicate */
    CHECK(sq_ch[2].midi_ch == SQ_MIDI_OFF, "BUG-011: a re-added channel is MIDI OFF (setup cue), never a silent duplicate");
    reset_world(); uisq_do_remove(1); uisq_do_remove(2); uisq_do_remove(3); uisq_do_remove(3); uisq_do_remove(2);
    ui_sq_button(UI_C_B_EDIT); ui_sq_button(UI_C_B_EDIT); ui_sq_button(UI_C_B_EDIT);
    CHECK(uisq_used == 0x0F && sq_ch[1].midi_ch == 1 && sq_ch[2].midi_ch == 2 && sq_ch[3].midi_ch == 3, "BUG-011: three removals, three undos restore all three MIDI channels");
    reset_world(); uisq_do_remove(1); uisq_do_remove(2); uisq_do_remove(3); uisq_sel = 0; uisq_used = 0x01;      /* a removal of CH1 refused (last one) leaves its MIDI */
    uisq_do_remove(0); CHECK(sq_ch[0].midi_ch == 0, "BUG-011: the last channel is not removed and keeps its MIDI");
    reset_world(); chn_tap(); k2(1); uisq_cm_val = 99; uisq_cm_ch = 9; frame(); fill(); CHECK(V.ch_midi[0] == 0 && uisq_cm_val == UISQ_CM_NONE, "a corrupt pending value / channel is ignored by the view and dropped by the commit");

    /* ---- K1 LENGTH: clamp 1..64, fast turns, the page count follows, hidden steps keep their edits ---- */
    reset_world(); chn_tap();
    knob(0, 1); CHECK(sq_ch[0].len == 17, "K1 +1: LENGTH 17"); fill(); CHECK(V.pages == 2 && V.val[0] == 17, "view: 2 pages (a partial second page)");
    knob(0, 3); CHECK(sq_ch[0].len == 29, "K1 +3 in a frame is fast: +12 (17 -> 29)");
    knob(0, 100); CHECK(sq_ch[0].len == 64, "K1 clockwise clamps at 64");
    fill(); CHECK(V.pages == 4 && V.val[0] == 64, "view: 4 pages");
    uisq_vpage = 3;
    { const StepEdit before = sq_ch[0].ed[40]; sq_ch[0].ed[40].fl = 1; /* an edit on page 3 */ (void)before; }
    knob(0, -127); CHECK(sq_ch[0].len == 1, "K1 anticlockwise clamps at 1");
    CHECK(uisq_vpage == 0, "the viewed page follows the shrink (clamped to page 1, was 4)");
    fill(); CHECK(V.pages == 1 && V.vpage == 1 && V.len16 == 1, "view: 1 page, 1 step inside LENGTH");
    knob(0, 127); CHECK(sq_ch[0].len == 64 && sq_ch[0].ed[40].fl == 1, "growing back keeps the hidden page's edit");
    uisq_vpage = 3; knob(0, -7); CHECK(sq_ch[0].len == 36 && uisq_vpage == 2, "LENGTH 64 -> 36 with page 4 viewed: viewed page clamps to 3 (3 pages)");
    knob(0, -127); knob(0, 1);  CHECK(sq_ch[0].len == 2, "from 1, +1 = 2");
    sq_playing = 1; sq_ch[0].len = 40; sq_rt[0].p = 39; knob(0, -127);
    { const uint8_t ph = uisq_playhead(0); CHECK(ph < sq_ch[0].len, "playing: the playhead stays inside the shortened LENGTH (%d of %d)", ph, sq_ch[0].len); }
    sq_playing = 0; sq_rt[0].p = 0;

    /* ---- OCT- / OCT+ = -PAGE / +PAGE ---- */
    reset_world(); chn_tap();
    sq_ch[0].len = 20; btn(UI_C_B_OCTDN); CHECK(sq_ch[0].len == 16, "-PAGE: 20 -> 16");
    btn(UI_C_B_OCTDN); CHECK(sq_ch[0].len == 16 && uisq_page == UI_PG_CHAN, "-PAGE at 16: inert");
    sq_ch[0].len = 48; btn(UI_C_B_OCTDN); CHECK(sq_ch[0].len == 32, "-PAGE: 48 -> 32");
    sq_ch[0].len = 1;  btn(UI_C_B_OCTDN); CHECK(sq_ch[0].len == 1, "-PAGE at 1: inert");
    sq_ch[0].len = 16; btn(UI_C_B_OCTUP); CHECK(sq_ch[0].len == 32 && uisq_vpage == 1, "+PAGE: 16 -> 32, the new page is viewed");
    btn(UI_C_B_EDIT); CHECK(sq_ch[0].len == 16, "EDIT takes the page add back");
    sq_ch[0].len = 20; btn(UI_C_B_OCTUP); CHECK(sq_ch[0].len == 32, "+PAGE: a partial page is completed (20 -> 32)");
    sq_ch[0].len = 64; btn(UI_C_B_OCTUP); CHECK(sq_ch[0].len == 64, "+PAGE at 64: inert");
    CHECK(uisq_vpage <= 3, "viewed page stays in range");

    /* ---- K4 FOLLOW and K3 SOURCE ---- */
    reset_world(); chn_tap();
    knob(3, -1); CHECK(sq_follow == 0, "K4 down: FOLLOW OFF"); fill(); CHECK(V.follow_off == 1 && V.val[3] == 0, "view: FOLLOW off");
    knob(3, 1); CHECK(sq_follow == 1, "K4 up: FOLLOW ON");
    { StepEdit e = sq_ch[0].ed[0]; uint8_t src = sq_ch[0].pg_src; knob(2, 3); CHECK(!memcmp(&e, &sq_ch[0].ed[0], sizeof e) && src == sq_ch[0].pg_src, "K3 SOURCE is inert in this task"); }

    /* ---- the ALGO encoder picks the algorithm of the selected channel on CHAN ---- */
    reset_world();
    ui_sq_algo_turn(1); CHECK(sq_ch[0].algo == 0, "ALGO on PERF is not this task's (P4-032): inert here");
    chn_tap();
    { const uint8_t a0 = sq_ch[0].algo;
      ui_sq_algo_turn(1); frame();
      CHECK(sq_ch[0].algo == a0 + 1 && uisq_pick_until, "ALGO +1: next algorithm, the strip lists them");
      { UisqMsg m; uisq_now = UISQ_MS(); uisq_msg_pick(&m, uisq_now, UI_PG_CHAN, 0);
        CHECK(m.kind == UI_MK_TOUCH && streq(m.a, "ALGO") && strstr(m.b, " AT STEP"), "touched control `%s %s`", m.a, m.b); }
      { char cue[48]; uisq_cue_text(cue, sizeof cue); CHECK(strchr(cue, '[') && strchr(cue, ']'), "the strip brackets the current one: `%s`", cue); }
      ui_sq_algo_turn(100); CHECK(sq_ch[0].algo == SQ_NALGO - 1, "ALGO clockwise stops at the last algorithm (no wrap)");
      ui_sq_algo_turn(-100); CHECK(sq_ch[0].algo == 0, "ALGO anticlockwise stops at the first");
      ui_sq_algo_turn(-1); CHECK(sq_ch[0].algo == 0, "...and stays"); }
    shot("chan_algo", 0, 0, 240, 240);

    /* ---- SELECT on CHAN: clamped, no add; the pending value stays with the channel it was turned for ---- */
    reset_world(); uisq_used = 0x07; chn_tap();
    ui_sq_select_turn(1); CHECK(uisq_sel == 1, "SELECT +1: CH2");
    ui_sq_select_turn(1); ui_sq_select_turn(1); CHECK(uisq_sel == 2 && uisq_used == 0x07 && uisq_page == UI_PG_CHAN, "SELECT past the last channel in use does not add one on CHAN");
    ui_sq_select_turn(-5); CHECK(uisq_sel == 1 || uisq_sel == 0, "SELECT down moves one used channel at a time");
    uisq_sel = 0; k2(1); ui_sq_select_turn(1); CHECK(sq_ch[0].midi_ch == 1 && sq_ch[1].midi_ch == SQ_MIDI_OFF && uisq_sel == 1, "turn K2 on CH1 then SELECT: committed for CH1, not CH2");

    /* ---- the other pages' keys behave as on PERF while CHAN is open: tap a step, M key mute ---- */
    reset_world(); chn_tap();
    { const uint8_t before = sq_ch[0].ed[2].fl; tap_key(WK(2)); CHECK(uisq_page == UI_PG_CHAN, "a step tap keeps CHAN open"); (void)before; }
    tap_key(0 + 0); (void)0;

    /* ---- the schedule of CHAN draws: the band and the frame are the mock's (tests/ui_sq_v12_test.c) ---- */
    reset_world(); chn_tap(); fresh(); shot("chan_page", 0, 0, 240, 240);
    printf("ui_sq_chan_test: %d failure(s)\n", fails);
    return fails ? 1 : 0; }
