/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_set_test.c (P4-016): the SET page of firmware/src/ui_sq.c on the host: input sequence -> sq_* effects and the strings on the screen.
 * It drives ui_sq_keys / ui_sq_button / ui_sq_other_button / ui_sq_knob / ui_sq_select_turn / ui_sq_home / ui_sq_input as ui_sq_glue.c does: GLO opens and closes SET,
 * K1 BPM (clamp 40..240, fast, the request the glue hands to song.g[G_BPM]), K2 SWING, K3 GATE, K4 DIV (clamps, no wrap), OCT-/OCT+ = OCTAVE -2..+2 (clamp, slot text),
 * K1 inert under an external clock (HOST n, the own tempo kept), the four tempo-source states of the header with the real text_w widths and the T_REC mark of a lost
 * clock, the keys that stay live on SET (steps, mute, CHN, NOTE_EDIT hold), the exits (HOME, GLO, CHN) and the bounds of the state it keeps. Every string is looked up in
 * ui_tables.h. Frames go to build/ui_sq/set_*.png; tests/ui_sq_v12_test.c compares the set_* scenes with the mock.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_set_test tests/ui_sq_set_test.c -lm && /tmp/ui_sq_set_test [outdir] */
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
static void glo(void) { ui_sq_other_button(UI_C_B_GLO); frame(); }
static void reset_world(void)
{   sq_boot(); uisq_page = UI_PG_PERF; uisq_sel = 0; uisq_vpage = 0; uisq_used = 0x0F; uisq_cm_val = UISQ_CM_NONE; uisq_new = 0; uisq_toast_until = 0; uisq_touch_until = 0;
    uisq_gone_n = 0; uisq_undone = 0; uisq_rec = 0; sq_playing = 0; sq_follow = 1; sq_bpm = 120; uisq_bpm_req = 0; ui_sq_clock_set(UISQ_CLK_INT, 0); uisq_preset_pend = 0; wait_ms(1200); }
static int toast_on(void) { return uisq_toast_until && (int32_t)(uisq_now - uisq_toast_until) < 0; }
static int touch_on(void) { return uisq_touch_a[0] && uisq_touch_until && (int32_t)(uisq_now - uisq_touch_until) < 0; }
static uint16_t px(int x, int y) { return swap16(fb[y * 240 + x]); }
static const SeqGlobal *G(void) { return &sq_gdb[sq_gidx]; }

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    reset_world();
    fresh();

    /* ---- GLO opens SET (S07 step 1): the band, the labels from the knob_map rows, the page title, the footer halves ---- */
    glo();
    CHECK(uisq_page == UI_PG_SET, "a GLO tap opens SET");
    fill();
    CHECK(V.band == UISQ_BAND_SET && V.val[0] == 120 && V.val[1] == 50 && V.val[2] == 50 && V.val[3] == 2, "view: BPM 120 SWING 50 GATE 50 DIV 1/16 (%d %d %d %d)", V.val[0], V.val[1], V.val[2], V.val[3]);
    { const char *want[4] = { "BPM", "SWING", "GATE", "DIV" };
      for (int k = 0; k < 4; k++) CHECK(streq(uisq_knob_label(UI_PG_SET, UI_CTX_ANY, (uint8_t)k), want[k]) && streq(V.tile[k]->label, want[k]), "K%d label is the knob_map row `%s` (tile `%s`)", k + 1, uisq_knob_label(UI_PG_SET, UI_CTX_ANY, (uint8_t)k), V.tile[k]->label); }
    CHECK(streq(V.txt[0], "120") && streq(V.txt[1], "50%") && streq(V.txt[2], "50%") && streq(V.txt[3], "1/16"), "tile texts `%s` `%s` `%s` `%s`", V.txt[0], V.txt[1], V.txt[2], V.txt[3]);
    CHECK(V.msg.kind == UI_MK_TOUCH && streq(V.msg.a, "GLO") && streq(V.msg.b, "ALL CHANNELS"), "the page title `%s %s`", V.msg.a, V.msg.b);
    { char l[24], r[24]; uisq_cue_split(uisq_page_cue(UI_PG_SET), l, sizeof l, r, sizeof r);
      CHECK(streq(l, "KEYS = STEPS") && streq(r, "GLO = BACK"), "footer halves from the page_states cue: `%s` / `%s`", l, r); }
    shot("set_open", 0, 0, 240, 240);
    glo();
    CHECK(uisq_page == UI_PG_PERF, "GLO again closes SET (BACK)");
    glo(); home_tap();
    CHECK(uisq_page == UI_PG_PERF, "HOME tap leaves SET");

    /* ---- K1 BPM (S07 step 2): +8 -> 128, the request for song.g[G_BPM], clamps, fast ---- */
    reset_world(); glo();
    for (int i = 0; i < 8; i++) knob(0, 1);
    CHECK(sq_bpm == 128 && uisq_bpm_req == 128, "K1 +8: BPM 128 and the request for the one tempo value (%u, %u)", sq_bpm, uisq_bpm_req);
    fill(); CHECK(V.val[0] == 128 && streq(V.txt[0], "128") && V.bpm == 128, "the tile and the header read 128");
    uisq_bpm_req = 0; knob(0, 3);
    CHECK(sq_bpm == 143, "a fast turn (3 detents in a frame) steps 5 each: 128 -> 143 (%u)", sq_bpm);
    uisq_bpm_req = 0; knob(0, 127);
    CHECK(sq_bpm == 240 && uisq_bpm_req == 240, "K1 clockwise stops at 240 (no wrap, no overflow of the byte value)");
    fill(); CHECK(V.val[0] == 240 && streq(V.txt[0], "240"), "the tile reads 240");
    knob(0, -127); knob(0, -127);
    CHECK(sq_bpm == 40, "K1 anticlockwise stops at 40");
    uisq_bpm_req = 0; knob(0, 0);
    CHECK(uisq_bpm_req == 0, "a zero turn asks for nothing");

    /* ---- K2 SWING, K3 GATE, K4 DIV (S07 steps 3-5) ---- */
    reset_world(); glo();
    for (int i = 0; i < 6; i++) knob(1, 1);
    CHECK(G()->s.swing == 56, "K2 +6: SWING 56 (%d)", G()->s.swing);
    knob(1, 100); CHECK(G()->s.swing == 75, "SWING stops at 75");
    knob(1, -127); CHECK(G()->s.swing == 50, "SWING stops at 50");
    for (int i = 0; i < 10; i++) knob(2, -1);
    CHECK(G()->s.gate == 40, "K3 -10: GATE 40 (%d)", G()->s.gate);
    knob(2, 127); CHECK(G()->s.gate == 100, "GATE stops at 100");
    knob(2, -127); CHECK(G()->s.gate == 10, "GATE stops at 10");
    fill(); CHECK(streq(V.txt[2], "10%"), "GATE tile `%s`", V.txt[2]);
    knob(3, 1);
    CHECK(G()->s.div == 3, "K4 +1: DIV 1/8T (%d)", G()->s.div);
    fill(); CHECK(streq(V.txt[3], "1/8T") && uisq_tile_dots(V.tile[3]) == 6, "DIV tile `%s`, six position dots", V.txt[3]);
    knob(3, 50); CHECK(G()->s.div == 5, "DIV stops at 1/4 (no wrap)");
    knob(3, -50); CHECK(G()->s.div == 0, "DIV stops at 1/32 (no wrap)");
    fill(); CHECK(streq(V.txt[3], "1/32"), "DIV tile `%s`", V.txt[3]);
    { uint32_t sw = G()->s.swing, ga = G()->s.gate; knob(1, 0); knob(2, 0); CHECK(G()->s.swing == sw && G()->s.gate == ga, "a zero turn changes nothing"); }

    /* ---- OCT- / OCT+ = OCTAVE (S07 step 6): -2..+2, the slot names it, the status line shows it ---- */
    reset_world(); glo();
    btn(UI_C_B_OCTUP);
    CHECK(G()->s.octave == 1, "OCT+ : OCTAVE +1");
    CHECK(touch_on() && streq(uisq_touch_a, "OCT+") && streq(uisq_touch_b, "OCTAVE +1"), "the slot names `%s %s`", uisq_touch_a, uisq_touch_b);
    fill(); CHECK(V.octave == 1, "view octave +1");
    { UisqMsg m; uisq_msg_pick(&m, uisq_now, UI_PG_SET, 0);
      CHECK(m.kind == UI_MK_TOUCH && 30 + 6 + 8 + (int)text_w(&AF_M, m.b) <= 136, "`OCT+ OCTAVE +1` fits the 136 px slot"); }
    shot("set_octave", 0, 0, 240, 240);
    btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP);
    CHECK(G()->s.octave == 2 && streq(uisq_touch_b, "OCTAVE +2"), "OCT+ stops at +2 and still names it (%d `%s`)", G()->s.octave, uisq_touch_b);
    for (int i = 0; i < 6; i++) btn(UI_C_B_OCTDN);
    CHECK(G()->s.octave == -2 && streq(uisq_touch_a, "OCT-") && streq(uisq_touch_b, "OCTAVE -2"), "OCT- stops at -2 (%d `%s %s`)", G()->s.octave, uisq_touch_a, uisq_touch_b);
    { UisqMsg m; uisq_msg_pick(&m, uisq_now, UI_PG_SET, 0);
      CHECK(30 + 6 + 8 + (int)text_w(&AF_M, m.b) <= 136 || 30 + 6 + 8 + (int)text_w(&AF_S, m.b) <= 136, "`OCT- OCTAVE -2` fits the slot"); }
    btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP);
    CHECK(G()->s.octave == 0 && streq(uisq_touch_b, "OCTAVE 0"), "back to 0 reads `OCTAVE 0` (`%s`)", uisq_touch_b);

    /* ---- the tempo source (SCREEN_SPEC_V2 4.1, SEQUENCER_UX 2.8): INT / EXT n / WAIT / lost on the header, K1 inert under any external source ---- */
    { static const struct { uint8_t src; const char *low; int rec; } st[4] = { { UISQ_CLK_INT, "120", 0 }, { UISQ_CLK_EXT, "124", 0 }, { UISQ_CLK_WAIT, "WAIT", 0 }, { UISQ_CLK_LOST, "--", 1 } };
      for (int i = 0; i < 4; i++) {
          int rec_px = 0;
          reset_world(); sq_bpm = (uint16_t)(i == 1 ? 124 : 120); ui_sq_clock_set(st[i].src, 0);
          fresh();
          for (int y = 0; y < 21; y++) for (int x = 14; x < 46; x++) {          /* the small `--` is anti-aliased: count the clearly red pixels */
              const uint16_t c = px(x, y); const int r = ((c >> 11) & 31) * 255 / 31, g = ((c >> 5) & 63) * 255 / 63, b = (c & 31) * 255 / 31;
              if (r - (g > b ? g : b) > 60) rec_px++; }
          CHECK((rec_px > 0) == st[i].rec, "state %d: the T_REC mark of a lost clock is %s (%d px)", i, st[i].rec ? "drawn" : "absent", rec_px);
          if (st[i].src == UISQ_CLK_INT) CHECK(16 + (int)text_w(&AF_M, "240") <= 45, "INT `240` (AF_M) fits the tempo slot before the chips at x 46");
          else CHECK(16 + (int)text_w(&AF_S, "EXT") <= 45 && 16 + (int)text_w(&AF_S, st[i].low) <= 45 && 16 + (int)text_w(&AF_S, "--") <= 45, "external states `EXT` over `%s` fit the tempo slot", st[i].low);
          glo(); fill();
          CHECK(V.clock == st[i].src, "state %d: the view carries the source", i);
          if (i) CHECK(streq(V.txt[0], i == 1 ? "124" : "--"), "state %d: K1 mirrors the host as `%s`", i, V.txt[0]);
          if (i) { char nm[40]; snprintf(nm, sizeof nm, "set_ext_state%d", i); shot(nm, 0, 0, 240, 240); }
          uisq_bpm_req = 0;
          knob(0, 3);
          if (i) CHECK(sq_bpm == (i == 1 ? 124 : 120) && uisq_bpm_req == 0, "state %d: K1 does nothing under an external source (BPM %u, req %u)", i, sq_bpm, uisq_bpm_req);
          else CHECK(sq_bpm == 135, "INT: K1 turns the tempo");
          if (i) CHECK(touch_on() && streq(uisq_touch_a, "K1") && streq(uisq_touch_b, "HOST SETS TEMPO"), "state %d: the slot says why: `%s %s`", i, uisq_touch_a, uisq_touch_b); } }
    /* the own tempo is kept, and returns the moment the source is INT again */
    reset_world(); glo(); knob(0, 1);
    uisq_bpm_req = 0; ui_sq_clock_set(UISQ_CLK_EXT, 1); sq_bpm = 133; knob(0, 5);
    ui_sq_clock_set(UISQ_CLK_INT, 0);
    fill(); CHECK(V.clock == UISQ_CLK_INT && streq(V.txt[0], "133") && V.val[0] == 133, "EXT -> INT: the tile reads the tempo of the source again, no stale HOST text (`%s`)", V.txt[0]);
    ui_sq_clock_set(9, 7); CHECK(uisq_clock == 0 && uisq_clock_port == 1, "an out-of-range source is INT; the port is 0 or 1");

    /* ---- the keys that stay live on SET (page_states SET row): steps, mute, CHN -> CHAN, a held step -> NOTE_EDIT, SELECT without add, a dice ---- */
    reset_world(); glo();
    { int was = on_of(0, 0); tap_key(WK(0)); CHECK(on_of(0, 0) != was, "a white key tap toggles step 1 on SET"); }
    reset_world(); glo();
    { uint8_t m = sq_ch[0].muted; tap_key(1); CHECK(sq_ch[0].muted != m, "the M1 key (F#3) mutes CH1 on SET"); tap_key(1); }
    reset_world(); glo(); chn_tap();
    CHECK(uisq_page == UI_PG_CHAN, "CHN (D#4) opens CHAN from SET");
    glo();
    CHECK(uisq_page == UI_PG_SET, "GLO opens SET from CHAN");
    reset_world(); glo(); hold_step(3);
    CHECK(uisq_page == UI_PG_NOTE_EDIT, "a held white key opens NOTE_EDIT from SET");
    glo();
    CHECK(uisq_page == UI_PG_SET, "GLO opens SET from NOTE_EDIT (the editor closes)");
    reset_world(); glo(); uisq_sel = 3; uisq_used = 0x0F;
    ui_sq_select_turn(1); frame();
    CHECK(uisq_page == UI_PG_SET && uisq_sel == 3 && uisq_used == 0x0F, "SELECT past the last channel on SET does not add one");
    ui_sq_select_turn(-1); frame();
    CHECK(uisq_sel == 2, "SELECT selects on SET");
    { UisqStep before[SQ_PAGE], after[SQ_PAGE]; int diff = 0;
      uisq_lane_read(before, 2, 0); btn(UI_C_B_FX); uisq_lane_read(after, 2, 0);
      for (int i = 0; i < SQ_PAGE; i++) if (memcmp(&before[i], &after[i], sizeof before[i])) diff++;
      CHECK(uisq_page == UI_PG_SET && toast_on(), "a dice button works on SET and the page stays (toast `%s`)", uisq_toast); (void)diff; }

    /* ---- REC: GLO is inert in REC; a pending preset stays visible on SET (the pill) ---- */
    reset_world(); uisq_rec = 1; glo();
    CHECK(uisq_page == UI_PG_PERF, "GLO is inert while REC is armed");
    uisq_rec = 0;
    reset_world(); glo(); uisq_preset = 2; uisq_preset_pend = 1; fresh();
    CHECK(px(2, 78) == UISQ_WHITE, "a pending preset keeps its white `Pnn?` pill on the SET status line");
    uisq_preset = 1; uisq_preset_pend = 0;

    /* ---- the status line never overlaps: the widest case (EXT USB, OUT FOLLOW, OCT -2) ends before the OCT readout (asserted by the mock; checked here on the real text_w) ---- */
    { int x = 42; x += (int)text_w(&AF_S, "EXT") + 3 + (int)text_w(&AF_M, "USB") + 7 + (int)text_w(&AF_S, "OUT") + 3 + (int)text_w(&AF_M, "FOLLOW");
      CHECK(x < 236 - (int)text_w(&AF_M, "-2") - (int)text_w(&AF_S, "OCT") - 4, "the widest clock-out blocks (ending at %d) clear `OCT -2`", x);
      CHECK((int)text_w(&AF_S, "WAIT FOR HOST START") + (int)text_w(&AF_S, "PLAY=CANCEL") + 12 <= 240 && (int)text_w(&AF_M, "HOST SETS TEMPO") + (int)text_w(&AF_M, "GLO = BACK") + 12 <= 240, "the footers fit 240 px"); }

    /* ---- bounds of the state SET keeps: nothing grows; a value the sequencer holds out of range is clamped on screen ---- */
    reset_world(); glo();
    sq_bpm = 9; fill(); CHECK(V.val[0] == 40, "a tempo below 40 is shown clamped (no wrap of the byte)");
    sq_bpm = 400; fill(); CHECK(V.val[0] == 240, "a tempo above 240 is shown clamped");
    sq_bpm = 120;
    CHECK(sizeof uisq_set_row / sizeof uisq_set_row[0] == 4 && sizeof uisq_set_names / sizeof uisq_set_names[0] == 6, "SET keeps 4 tile rows and 6 DIV names: no growing list");

    /* ---- no literal label strings on the page (the strings come from ui_tables.h): the SET code reads the knob_map, set_text and message tables ---- */
    CHECK(uisq_st_w(UI_ST_HOST) && streq(uisq_st_w(UI_ST_DIV1), "1/16T") && streq(uisq_st_w(UI_ST_OCTAVE), "OCTAVE") && streq(uisq_st_w(UI_ST_COUNT), ""), "set_text words resolve; an id out of range is empty");

    printf("ui_sq_set_test: %s (%d failures)\n", fails ? "FAIL" : "OK", fails);
    return fails ? 1 : 0; }
