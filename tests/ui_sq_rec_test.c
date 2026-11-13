/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_rec_test.c (P4-017): the REC page of firmware/src/ui_sq.c on the host: input sequence -> sq_* effects, audition queue events and the strings on the screen.
 * It drives ui_sq_keys / ui_sq_button / ui_sq_other_button / ui_sq_knob / ui_sq_select_turn / ui_sq_algo_turn / ui_sq_home / ui_sq_input as ui_sq_glue.c does: the REC button
 * opens and closes REC (from PERF, NOTE_EDIT, CHAN, SET; HOME leaves), every one of the 27 keys writes ON + PITCH + VEL at the cursor (stopped; the cursor advances and
 * wraps) or at the nearest step (playing) in ONE undo record, K2 ACC (vel 120), K3 SLIDE, K4 / OCT-/OCT+ (the key's pitch moves by octaves, clamped), K1 CURSOR (wraps,
 * the step sounds, inert while playing), EDIT undo / redo with the cursor, the dice buttons, GLO inert with its slot text, SELECT / ALGO, the note queue (every note-on has
 * its note-off, a full queue loses no off, a key released after REC was left toggles no step) and the bounds. Strings come from ui_tables.h.
 * Frames go to build/ui_sq/rec_*.png; tests/ui_sq_v12_test.c compares the rec_* scenes with the mock.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_rec_test tests/ui_sq_rec_test.c -lm && /tmp/ui_sq_rec_test [outdir] */
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
{   sq_boot(); for (uint8_t c_ = 0; c_ < SQ_NCH; c_++) sq_ch[c_].midi_ch = c_; uisq_page = UI_PG_PERF; uisq_sel = 0; uisq_vpage = 0; uisq_used = 0x0F;   /* P4-044: boots CH1-only with CH2..4 OFF; these scenarios want four on MIDI 1..4 */ uisq_cm_val = UISQ_CM_NONE; uisq_new = 0; uisq_toast_until = 0; uisq_touch_until = 0;
    uisq_gone_n = 0; uisq_undone = 0; uisq_rec = 0; sq_playing = 0; sq_follow = 1; sq_bpm = 120; uisq_bpm_req = 0; ui_sq_clock_set(UISQ_CLK_INT, 0); uisq_preset_pend = 0; wait_ms(1200); }
static int toast_on(void) { return uisq_toast_until && (int32_t)(uisq_now - uisq_toast_until) < 0; }
static int touch_on(void) { return uisq_touch_a[0] && uisq_touch_until && (int32_t)(uisq_now - uisq_touch_until) < 0; }
static uint16_t px(int x, int y) { return swap16(fb[y * 240 + x]); }
static const SeqGlobal *G(void) { return &sq_gdb[sq_gidx]; }


/* ---- the REC helpers ---- */
static uint8_t evo[256], evn[256], evv[256];
static int nev;
static void drain(void) { uint8_t o, n, v; while (ui_sq_aud_next(&o, &n, &v)) if (nev < 256) { evo[nev] = o; evn[nev] = n; evv[nev] = v; nev++; } }
static void drain_clear(void) { drain(); nev = 0; }
static int count_on(void) { int c = 0; for (int i = 0; i < nev; i++) c += evo[i]; return c; }
static int count_off(void) { return nev - count_on(); }
static void rec_btn(void) { ui_sq_other_button(UI_C_B_REC); frame(); }
static void rec_world(void)
{   reset_world(); uisq_rec_cursor = 1; uisq_rec_acc = 0; uisq_rec_slide = 0; uisq_rec_oct = 2; uisq_rec_owed = 0; uisq_rec_aud_note = 0;
    memset(uisq_rec_note, 0, sizeof uisq_rec_note); uisq_aud_w = uisq_aud_r = 0; nev = 0; sq_gdb[sq_gidx].s.key = 0; sq_gdb[sq_gidx].s.octave = 0;
    uisq_page = UI_PG_PERF; uisq_rec = 0; uisq_keys = 0; uisq_key_fired = 0; }
static int cursor_abs(void) { return uisq_vpage * 16 + uisq_rec_cursor; }
static int is_written(uint8_t ch, uint8_t p, uint8_t note_semis, uint8_t vel)
{   const StepEdit *e = &sq_ch[ch].ed[p];
    return (e->fl & 3u) == 1u && e->note == (uint8_t)(0x80u | note_semis) && e->vel == vel; }
static void play_at(uint8_t next_p, int rem)                              /* the playhead: step next_p starts in `rem` ticks */
{   sq_playing = 1; sq_rt[0].p = next_p; sq_now = 1000; sq_rt[0].grid = (uint32_t)(1000 + rem); }

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    rec_world();
    fresh();

    /* ---- REC opens (S07 step 19): the page, the band from the knob_map REC rows, the footer from the page_states cue ---- */
    rec_btn();
    CHECK(uisq_page == UI_PG_REC && uisq_rec == 1, "the REC button opens REC");
    fill();
    CHECK(V.band == UISQ_BAND_REC && V.rec == 1 && V.val[0] == 1 && V.val[1] == 0 && V.val[2] == 0 && V.val[3] == 2 && V.cursor == 1, "view: CURSOR 1 ACC OFF SLIDE OFF OCT 0 (%d %d %d %d)", V.val[0], V.val[1], V.val[2], V.val[3]);
    { const char *want[4] = { "CURSOR", "ACC", "SLIDE", "OCT" };
      for (int k = 0; k < 4; k++) CHECK(streq(uisq_knob_label(UI_PG_REC, UI_CTX_ANY, (uint8_t)k), want[k]) && streq(V.tile[k]->label, want[k]), "K%d label is the knob_map row `%s` (tile `%s`)", k + 1, uisq_knob_label(UI_PG_REC, UI_CTX_ANY, (uint8_t)k), V.tile[k]->label); }
    CHECK(streq(V.txt[0], "1") && streq(V.txt[1], "OFF") && streq(V.txt[2], "OFF") && streq(V.txt[3], "0"), "tile texts `%s` `%s` `%s` `%s`", V.txt[0], V.txt[1], V.txt[2], V.txt[3]);
    CHECK(uisq_tile_dots(V.tile[1]) == 2 && uisq_tile_dots(V.tile[3]) == 5 && uisq_tile_dots(V.tile[0]) == 0, "ACC / SLIDE are two-name lists, OCT a five-name list, CURSOR a meter");
    { char l[24], r[24]; uisq_cue_split(uisq_page_cue(UI_PG_REC), l, sizeof l, r, sizeof r);
      CHECK(streq(l, "PLAY KEYS TO WRITE") && streq(r, "REC = DONE"), "footer halves from the page_states cue: `%s` / `%s`", l, r);
      CHECK(text_w(&AF_M, l) + text_w(&AF_S, r) + 4 <= 240, "the REC footer fits 240 px"); }
    shot("rec_open", 0, 0, 240, 240);
    rec_btn();
    CHECK(uisq_page == UI_PG_PERF && uisq_rec == 0, "the REC button again leaves REC (back to PERF)");
    rec_btn(); home_tap();
    CHECK(uisq_page == UI_PG_PERF && uisq_rec == 0, "a HOME tap leaves REC");

    /* ---- stopped: each of the 27 keys writes at the cursor and advances (S07 step 20); the pitch is the key's note ---- */
    rec_world(); rec_btn(); drain_clear();
    for (uint8_t k = 0; k < 27; k++) {
        const uint8_t step = (uint8_t)(k % 16);
        tap_key(k);
        drain();
        CHECK(is_written(0, step, (uint8_t)(17 + k), 90), "key %d wrote step %d: ON, PITCH %d, VEL 90 (%02x %02x %02x %02x)", k, step + 1, 17 + k, sq_ch[0].ed[step].fl, sq_ch[0].ed[step].note, sq_ch[0].ed[step].vel, sq_ch[0].ed[step].gr);
        CHECK(cursor_abs() == (k + 1) % 16 + 1, "after key %d the cursor is %d (wraps past LENGTH 16)", k, cursor_abs()); }
    CHECK(count_on() == 27 && count_off() == 27, "27 notes in the queue, each with its note-off (%d on, %d off)", count_on(), count_off());
    for (int i = 0; i < nev; i += 2) CHECK(evo[i] == 1 && evo[i + 1] == 0 && evn[i] == 53 + i / 2 && evn[i + 1] == evn[i] && evv[i] == 90, "event pair %d: ON %d vel %d then OFF %d", i / 2, evn[i], evv[i], evn[i + 1]);
    CHECK(uisq_page == UI_PG_REC, "REC stays open while writing");
    shot("rec_written", 0, 0, 240, 240);

    /* ---- K2 ACC (vel 120), K3 SLIDE, K4 OCT and OCT-/OCT+ (S07 steps 21, 23) ---- */
    rec_world(); rec_btn(); drain_clear();
    knob(1, 1); fill(); CHECK(uisq_rec_acc == 1 && streq(V.txt[1], "ON"), "K2 +1: ACC ON (`%s`)", V.txt[1]);
    knob(2, 1); fill(); CHECK(uisq_rec_slide == 1 && streq(V.txt[2], "ON"), "K3 +1: SLIDE ON (`%s`)", V.txt[2]);
    tap_key(WK(0) + 0); drain();
    CHECK(is_written(0, 0, 17, 120) && ((sq_ch[0].ed[0].fl >> 2) & 3u) == 1u, "ACC ON writes VEL 120 and SLIDE ON (vel %d fl %02x)", sq_ch[0].ed[0].vel, sq_ch[0].ed[0].fl);
    CHECK(count_on() == 1 && evv[0] == 120, "the note sounds at 120");
    knob(1, 5); knob(1, 1); CHECK(uisq_rec_acc == 1, "K2 stops at ON (no wrap)");
    knob(1, -1); knob(2, -9); fill(); CHECK(uisq_rec_acc == 0 && uisq_rec_slide == 0 && streq(V.txt[1], "OFF"), "K2 / K3 turned back: OFF");
    uisq_rec_cursor = 1; tap_key(0); drain();
    CHECK(is_written(0, 0, 17, 90) && ((sq_ch[0].ed[0].fl >> 2) & 3u) == 0u, "OFF again: the same step rewritten at VEL 90 with SLIDE AUTO (fl %02x)", sq_ch[0].ed[0].fl);
    rec_world(); rec_btn(); drain_clear();
    knob(3, 1); fill(); CHECK(uisq_rec_oct == 3 && streq(V.txt[3], "+1"), "K4 +1: OCT +1 (`%s`)", V.txt[3]);
    tap_key(4); drain(); CHECK(is_written(0, 0, 17 + 4 + 12, 90) && evn[0] == 53 + 4 + 12, "OCT +1: the key sounds and is stored an octave up (note %d)", evn[0]);
    btn(UI_C_B_OCTDN); btn(UI_C_B_OCTDN); fill();
    CHECK(uisq_rec_oct == 1 && streq(V.txt[3], "-1"), "OCT- twice: OCT -1 (`%s`)", V.txt[3]);
    btn(UI_C_B_OCTDN); btn(UI_C_B_OCTDN); btn(UI_C_B_OCTDN); fill();
    CHECK(uisq_rec_oct == 0 && streq(V.txt[3], "-2"), "OCT- stops at -2 (`%s`)", V.txt[3]);
    btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); fill();
    CHECK(uisq_rec_oct == 4 && streq(V.txt[3], "+2") && uisq_page == UI_PG_REC, "OCT+ stops at +2 and stays on REC (`%s`)", V.txt[3]);
    knob(3, -100); CHECK(uisq_rec_oct == 0, "K4 anticlockwise stops at -2");
    drain_clear(); uisq_rec_cursor = 1; uisq_vpage = 0; tap_key(0); drain();
    CHECK(evn[0] == 29 && is_written(0, 0, 5, 90), "OCT -2, F3: note 29, stored folded up by an octave into 0..96 (semitones %d)", sq_ch[0].ed[0].note & 0x7F);
    knob(3, 100); sq_gdb[sq_gidx].s.octave = 2; drain_clear(); uisq_rec_cursor = 1; uisq_vpage = 0; tap_key(26); drain();
    CHECK(evn[0] == 127, "OCT +2 and OCTAVE +2 on G5: the note clamps at 127 (%d)", evn[0]);
    CHECK((sq_ch[0].ed[0].note & 0x80u) && (sq_ch[0].ed[0].note & 0x7Fu) <= 96, "... and the stored pitch stays inside 0..96 (%d)", sq_ch[0].ed[0].note & 0x7F);
    sq_gdb[sq_gidx].s.octave = 1; sq_gdb[sq_gidx].s.key = 5; knob(3, -100); knob(3, 2); drain_clear(); uisq_rec_cursor = 1; uisq_vpage = 0; tap_key(7); drain();
    CHECK(evn[0] == 60 + 12 && is_written(0, 0, (uint8_t)(72 - 36 - 5 - 12), 90), "KEY 5 and OCTAVE +1: C4 plays 72; stored as 72 - 36 - 5 - 12 = %d so sq_note adds KEY and OCTAVE back (stored %d)", 72 - 36 - 5 - 12, sq_ch[0].ed[0].note & 0x7F);
    { const SeqGlobal *g = &sq_gdb[sq_gidx]; CHECK(sq_note(g, (int16_t)(sq_ch[0].ed[0].note & 0x7F)) >= 60 && sq_note(g, (int16_t)(sq_ch[0].ed[0].note & 0x7F)) <= 84, "the stored pitch plays (SP6) near the key that was played"); }

    /* ---- K1 CURSOR stopped (S07 step 22): one step per detent, wraps over LENGTH, the step under it sounds ---- */
    rec_world(); rec_btn(); drain_clear();
    knob(0, 4); CHECK(cursor_abs() == 5, "K1 +4: CURSOR 5 (%d)", cursor_abs());
    knob(0, -5); CHECK(cursor_abs() == 16, "K1 -5 from 5 wraps to 16");
    knob(0, 1); CHECK(cursor_abs() == 1, "K1 +1 from 16 wraps to 1");
    knob(0, 127); knob(0, -127); fill(); CHECK(V.val[0] >= 1 && V.val[0] <= 16, "a huge turn leaves the cursor inside 1..16 (%d)", V.val[0]);
    rec_world(); sq_ch[0].ed[2].fl = 1; sq_ch[0].ed[2].note = 0x80 | 24; sq_ch[0].ed[2].vel = 100;
    rec_btn(); drain_clear(); knob(0, 2);
    drain(); { UisqStep L; uisq_step_read(&L, 0, 2); CHECK(cursor_abs() == 3 && count_on() == 1 && evn[0] == L.note && evo[0] == 1, "K1 onto a hit step sounds its note %d (queue: %d on)", L.note, count_on()); }
    wait_ms(200); drain();
    CHECK(count_on() == 1 && count_off() == 1, "... and ends it after 120 ms (%d on, %d off)", count_on(), count_off());
    CHECK(uisq_rec_aud_note == 0, "no audition note is left owing");
    rec_world(); rec_btn(); sq_len_set(0, 12); knob(0, 11); CHECK(cursor_abs() == 12, "LENGTH 12: K1 reaches step 12");
    knob(0, 1); CHECK(cursor_abs() == 1, "... and wraps to 1 (LENGTH, not 16)");
    knob(0, -1); tap_key(0);
    CHECK(cursor_abs() == 1 && is_written(0, 11, 17, 90), "writing step 12 wraps the cursor to step 1 (LENGTH 12)");
    tap_key(1); CHECK(is_written(0, 0, 18, 90) && cursor_abs() == 2, "... and the next key writes step 1");
    CHECK(!(sq_ch[0].ed[12].fl & 3u), "nothing is written past LENGTH");

    /* ---- LENGTH 64: the cursor crosses pages and the view follows it ---- */
    rec_world(); sq_len_set(0, 40); rec_btn();
    for (int i = 0; i < 17; i++) tap_key(0);
    CHECK(cursor_abs() == 18 && uisq_vpage == 1, "17 writes into LENGTH 40: cursor on page 2, step 18 (abs %d, page %d)", cursor_abs(), uisq_vpage + 1);
    CHECK(is_written(0, 16, 17, 90), "step 17 is on page 2");
    fill(); CHECK(V.vpage == 2 && V.cursor == 2, "the view shows page 2 with the cursor on its step 2");
    knob(0, 22); CHECK(cursor_abs() == 40, "K1 reaches step 40");
    knob(0, 1); CHECK(cursor_abs() == 1 && uisq_vpage == 0, "... and wraps to step 1 on page 1");

    /* ---- playing: a key writes at the NEAREST step (S07 step 30) ---- */
    rec_world(); rec_btn(); drain_clear();
    play_at(5, 10); frame();                         /* step 5 starts in 10 of 48 ticks: it is the nearest */
    CHECK(cursor_abs() == 6, "playing: the cursor is the nearest step (abs %d)", cursor_abs());
    tap_key(2); drain();
    CHECK(is_written(0, 5, 19, 90) && !(sq_ch[0].ed[4].fl & 3u), "10 ticks before step 6 (index 5): the key writes index 5");
    CHECK(cursor_abs() == 6, "playing: the cursor does not advance");
    play_at(5, 40); frame(); tap_key(3); drain();
    CHECK(is_written(0, 4, 20, 90), "40 ticks before the next step (the sounding step is half over): the key writes the SOUNDING one, index 4");
    play_at(5, 24); frame(); tap_key(4); drain();
    CHECK(is_written(0, 5, 21, 90), "exactly half a step to go: the next step");
    play_at(0, 40); frame(); tap_key(5); drain();
    CHECK(is_written(0, 15, 22, 90) && cursor_abs() == 16, "the playhead at the loop point: the previous step is step 16 (wraps back)");
    play_at(0, -3); frame(); CHECK(cursor_abs() == 1, "a step already due (negative remainder) counts as started");
    sq_gdb[sq_gidx].s.div = 5; play_at(7, 150); frame(); tap_key(6); drain();
    CHECK(is_written(0, 6, 23, 90), "DIV 1/4 (192 ticks per step): 150 to go is the sounding step, index 6");
    sq_gdb[sq_gidx].s.div = 2;
    play_at(5, 10); knob(0, 3); CHECK(cursor_abs() == 6, "playing: K1 does not move the cursor");
    sq_playing = 0;
    fresh();

    /* ---- the screen while recording: cursor frame, red disc with the play mark (v12 rec_armed / rec_playing) ---- */
    rec_world(); rec_btn(); fresh(); shot("rec_stopped", 0, 0, 240, 240);
    play_at(3, 10); frame(); fresh(); shot("rec_playing", 0, 0, 240, 240);
    CHECK(px(5, 9) == T_REC, "the transport disc is red while recording (%04x)", px(5, 9));
    sq_playing = 0;

    /* ---- EDIT undoes the last written note, the cursor steps back; again = redo (button_modes REC B_EDIT) ---- */
    rec_world(); rec_btn(); tap_key(3); tap_key(5);
    CHECK(cursor_abs() == 3, "two writes: cursor 3");
    btn(UI_C_B_EDIT);
    CHECK(!(sq_ch[0].ed[1].fl & 3u) && is_written(0, 0, 20, 90) && cursor_abs() == 2, "EDIT: the last write (step 2) is taken back and the cursor steps back onto it (abs %d)", cursor_abs());
    CHECK(toast_on() && streq(uisq_toast, "UNDO"), "toast `%s`", uisq_toast);
    btn(UI_C_B_EDIT);
    CHECK(is_written(0, 1, 22, 90) && cursor_abs() == 3 && streq(uisq_toast, "REDO"), "EDIT again: redone, cursor past it (abs %d, toast `%s`)", cursor_abs(), uisq_toast);
    rec_world(); rec_btn(); tap_key(0); btn(UI_C_B_SCL);
    CHECK(uisq_page == UI_PG_REC && toast_on(), "a dice button works in REC (toast `%s`)", uisq_toast);
    { const int c = cursor_abs(); btn(UI_C_B_EDIT); CHECK(cursor_abs() == c, "EDIT after a dice undoes the dice and leaves the cursor"); }
    rec_world(); rec_btn(); tap_key(3); sq_ch[0].ed[0] = zero_edit; sq_ch[0].ed[0].fl = 0;
    rec_world(); rec_btn(); tap_key(3); tap_key(3);
    CHECK(is_written(0, 0, 20, 90) && is_written(0, 1, 20, 90), "(two equal keys write two steps)");
    rec_world(); rec_btn(); tap_key(3); uisq_rec_cursor = 1; tap_key(3);     /* the same key over the same step: nothing changes */
    btn(UI_C_B_EDIT);
    CHECK(!(sq_ch[0].ed[0].fl & 3u) && cursor_abs() == 1, "rewriting a step with the same note takes no second record: EDIT undoes the one write (abs %d)", cursor_abs());

    /* ---- GLO is inert in REC; the slot says how to finish ---- */
    rec_world(); rec_btn(); ui_sq_other_button(UI_C_B_GLO); frame();
    CHECK(uisq_page == UI_PG_REC, "GLO tap does not leave REC");
    CHECK(touch_on() && streq(uisq_touch_a, "GLO") && streq(uisq_touch_b, "REC = DONE"), "the slot names `%s %s`", uisq_touch_a, uisq_touch_b);
    { UisqMsg m; uisq_msg_pick(&m, uisq_now, UI_PG_REC, 0); CHECK(m.kind == UI_MK_TOUCH && 30 + 6 + 8 + (int)text_w(&AF_M, m.b) <= 136, "`GLO REC = DONE` fits the slot"); }
    fresh(); shot("rec_glo", 0, 0, 240, 240);

    /* ---- the entries: NOTE_EDIT (cursor on the edited step), CHAN, SET ---- */
    rec_world(); hold_step(8);
    CHECK(uisq_page == UI_PG_NOTE_EDIT, "a held step key opens NOTE_EDIT");
    rec_btn();
    CHECK(uisq_page == UI_PG_REC && cursor_abs() == 9, "REC from NOTE_EDIT: the cursor starts on the edited step 9 (abs %d)", cursor_abs());
    rec_world(); chn_tap(); CHECK(uisq_page == UI_PG_CHAN, "CHN opens CHAN"); rec_btn();
    CHECK(uisq_page == UI_PG_REC && cursor_abs() == 1, "REC from CHAN opens REC");
    rec_world(); glo(); rec_btn();
    CHECK(uisq_page == UI_PG_REC && cursor_abs() == 1, "REC from SET opens REC");
    rec_btn(); glo(); CHECK(uisq_page == UI_PG_SET, "after REC, GLO works again");
    rec_world(); sq_len_set(0, 32); uisq_vpage = 1; rec_btn();
    CHECK(cursor_abs() == 17, "REC from PERF on page 2: the cursor starts on that page's step 1 (abs %d)", cursor_abs());
    rec_world(); chn_tap(); rec_btn(); chn_tap(); CHECK(uisq_page == UI_PG_REC, "the CHN key is a note in REC: it does not open CHAN"); rec_btn(); drain_clear();

    /* ---- SELECT / ALGO / PRESETS (knob_map REC rows) ---- */
    rec_world(); uisq_used = 0x03; rec_btn(); tap_key(0); tap_key(1);
    ui_sq_select_turn(1); frame();
    CHECK(uisq_sel == 1 && cursor_abs() == 1 && uisq_page == UI_PG_REC, "SELECT: channel 2, the cursor back on step 1");
    tap_key(2); CHECK(is_written(1, 0, 19, 90) && is_written(0, 0, 17, 90), "the next key writes on the new channel's step 1");
    ui_sq_select_turn(1); frame();
    CHECK(uisq_sel == 1 && uisq_used == 0x03, "SELECT past the last channel does not add one in REC");
    ui_sq_select_turn(-1); frame(); CHECK(uisq_sel == 0 && cursor_abs() == 1, "SELECT back: channel 1, cursor on 1");
    { const uint8_t was = sq_ch[0].algo; ui_sq_algo_turn(1); frame();
      CHECK(sq_ch[0].algo != was && uisq_page == UI_PG_REC, "ALGO turns the algorithm in REC (%d -> %d)", was, sq_ch[0].algo);
      CHECK(is_written(0, 0, 17, 90), "... and the written steps are kept"); }

    /* ---- the note queue: every ON has its OFF; leaving REC with keys down; a full queue; a late release ---- */
    rec_world(); rec_btn(); drain_clear();
    press(1u << 3); press((1u << 3) | (1u << 8)); frame();
    home_tap();
    drain();
    CHECK(uisq_page == UI_PG_PERF && count_on() == 2 && count_off() == 2, "two keys down, HOME: both notes end (%d on, %d off)", count_on(), count_off());
    { const StepEdit a = sq_ch[0].ed[0]; const StepEdit b = sq_ch[0].ed[1]; const uint8_t on0 = sq_ch[0].ed[2].fl;
      press(0); frame(); drain();
      CHECK(!memcmp(&a, &sq_ch[0].ed[0], sizeof a) && !memcmp(&b, &sq_ch[0].ed[1], sizeof b) && sq_ch[0].ed[2].fl == on0 && count_off() == 2, "releasing them on PERF toggles no step and sends no second off"); }
    rec_world(); rec_btn(); drain_clear();
    press(1u << 5); knob(3, 1); press(0); frame(); drain();                       /* the octave changes while the key is held */
    CHECK(count_on() == 1 && count_off() == 1 && evn[0] == evn[1], "an octave change while a key is held: the off names the note that started (%d, %d)", evn[0], evn[1]);
    rec_world(); rec_btn(); drain_clear();
    press((1u << 27) - 1u); frame();
    CHECK(count_on() == 0, "(drain not yet run)");
    drain(); CHECK(count_on() == 15, "27 keys down at once: the 16-slot queue takes 15 note-ons; the rest are dropped without being owed (%d)", count_on());
    CHECK(uisq_rec_owed == 0, "a dropped note-on owes no off");
    { int steps = 0; for (int k = 0; k < 27; k++) steps += (sq_ch[0].ed[k % 16].fl & 3u) == 1u; CHECK(steps >= 16, "... but every key still wrote its step"); }
    press(0); frame();
    drain(); { int offs = count_off();
      for (int i = 0; i < 40 && offs < 15; i++) { frame(); drain(); offs = count_off(); }
      CHECK(offs == 15 && uisq_rec_owed == 0, "all 27 released: every note that started gets its off, owed ones follow when the queue has room (%d offs)", offs); }
    rec_world(); rec_btn(); drain_clear();
    for (int i = 0; i < 12; i++) uisq_aud_push(1, 60, 90);                           /* a ring nearly full of other events */
    press(1u << 1); frame();                                                         /* the key's note-on is the 13th */
    uisq_aud_push(1, 61, 90); uisq_aud_push(1, 62, 90);                              /* now it is full (15 of 16) */
    press(0); frame();                                                               /* the release finds no room */
    CHECK(uisq_rec_owed == 2u && uisq_rec_note[1] == 55, "the release into a full queue is owed (mask %x, note %d)", uisq_rec_owed, uisq_rec_note[1]);
    for (int tries = 0; tries < 20; tries++) frame();
    CHECK(uisq_rec_owed == 2u && uisq_rec_note[1] == 55, "... still owed while the consumer is away");
    drain_clear();
    { int tries = 0; while (tries++ < 5) { frame(); drain(); } }
    CHECK(uisq_rec_owed == 0 && count_off() == 1 && evn[nev - 1] == 54 && evo[nev - 1] == 0, "... and it lands once the consumer has drained (owed %u, off note %d)", uisq_rec_owed, nev ? evn[nev - 1] : -1);

    /* ---- bounds ---- */
    rec_world(); rec_btn();
    uisq_rec_cursor = 99; uisq_vpage = 3; frame();
    CHECK(cursor_abs() >= 1 && cursor_abs() <= 16 && uisq_rec_cursor <= 16, "a stale cursor (page 4 of a 16-step channel, step 99) is pulled back inside LENGTH (abs %d)", cursor_abs());
    uisq_rec_cursor = 0; uisq_vpage = 0; frame(); CHECK(uisq_rec_cursor == 1, "cursor 0 becomes 1");
    sq_len_set(0, 1); uisq_rec_cursor = 1; uisq_vpage = 0; tap_key(0); tap_key(1);
    CHECK(cursor_abs() == 1 && is_written(0, 0, 18, 90), "LENGTH 1: every key writes step 1 and the cursor stays");
    { uint8_t k_bad = 200; uisq_rec_key(k_bad, 1); uisq_rec_key(k_bad, 0); CHECK(1, "a key index outside 0..26 is ignored"); }
    play_at(200, 5); frame(); CHECK(cursor_abs() >= 1 && cursor_abs() <= 1, "a playhead step beyond LENGTH (stale p) is taken modulo LENGTH");
    sq_playing = 0;
    rec_world(); rec_btn(); uisq_sel = 3; uisq_used = 0x08; frame();
    tap_key(0); CHECK(is_written(3, 0, 17, 90), "a single channel in use (CH4): keys write on it");
    rec_world(); rec_btn(); uisq_rec_ch = 0xFF; uisq_sel = 2; frame(); CHECK(cursor_abs() == 1, "a changed selection by any road returns the cursor to step 1");
    CHECK(sizeof uisq_rec_row / sizeof uisq_rec_row[0] == 4 && UISQ_AUD_N == 16u, "REC keeps 4 tile rows and a 16-slot queue: no growing state");
    uisq_rec = 0;

    printf("ui_sq_rec_test: %s (%d failures)\n", fails ? "FAIL" : "OK", fails);
    return fails ? 1 : 0; }
