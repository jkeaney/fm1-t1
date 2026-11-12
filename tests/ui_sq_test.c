/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_test.c (P4-011): the sequencer UI framework firmware/src/ui_sq.c on the host. It includes the real gfx.c (fonts, the canvas,
 * text_w), the real sq_core.c / sq_chan.c / ALGOS (the state the UI reads) and build/gen/ui_tables.h (the generated label tables),
 * captures every blit into a 240x240 frame, writes PNGs to build/ui_sq/ and asserts:
 *   - screen v2 (P4-026): the band table equals SCREEN_SPEC_V2 section 2, the two strips (0-119, 120-239) are at most 240x124, no text band crosses
 *     y 120; the empty frame of `playing` has the knob tiles, button tiles, white and black keys at the spec's boxes; the 1-bpp icons are pixel-identical
 *     to the mock's art (build/gen/sq_icons_ref.txt, from tools/gen_sq_icons.py --ref); the new colour tokens are the HI-CON values of the spec;
 *   - every label that can appear in a zone fits it, measured with the real text_w: the header slots, the Z1 cell lines, the knob
 *     labels, the tag column, the status lines, every Z4 cell (all pages x all algorithms; the computed widths sum to 240), the
 *     black-key labels (a label over 20 px is cut by design, SCREEN_SPEC F3: the drawn form must fit), every row of
 *     tests/design/widths.tsv against its limit, and the cue / toast strings against the Z6 width;
 *   - redraw by signature: nothing is blitted twice for an unchanged state, ui.force redraws everything, a state change redraws only
 *     the blit it belongs to;
 *   - the toast: 2000 ms, same text restarts, new text replaces, dropped while HOME is held, never queued.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_test tests/ui_sq_test.c -lm && /tmp/ui_sq_test [outdir]
 * Run from the repo root (it reads tests/design/widths.tsv). */
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

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    sq_boot();
    fm1_ms = 1000;

    /* ---- the bands (SCREEN_SPEC_V2 2) and the strip plan ---- */
    CHECK(UISQ_HDR_Y == 0 && UISQ_HDR_H == 22 && UISQ_MSG_X == 104, "header band y0-21, message slot from x104");
    CHECK(UISQ_KNOB_Y == 24 && UISQ_KNOB_Y + UISQ_KNOB_H - 1 == 76 && UISQ_KNOB_W == 58 && UISQ_KNOB_H == 53, "knob tiles 24-76, 58 x 53");
    CHECK(UISQ_KNOB_LAB == 16 && UISQ_KNOB_VAL == 36, "knob label band 16, value area 36");
    CHECK(UISQ_STAT_Y == 77 && UISQ_STAT_Y + UISQ_STAT_H - 1 == 91, "status line 77-91");
    CHECK(UISQ_MUS_Y == 92 && UISQ_MUS_Y + UISQ_MUS_H - 1 == 141, "music row 92-141");
    CHECK(UISQ_BTN_Y == 146 && UISQ_BTN_Y + UISQ_BTN_H - 1 == 180 && UISQ_BTN_W == 38 && UISQ_BTN_TH == 17 && 2 * UISQ_BTN_TH + 1 == UISQ_BTN_H, "buttons 146-180, 2 x 6 tiles 38 x 17");
    CHECK(UISQ_KEY_Y == 186 && UISQ_KEY_Y + UISQ_KEY_H - 1 == 220 && UISQ_WKEY_W == 14 && UISQ_WKEY_H == 33 && UISQ_BKEY_W == 10 && UISQ_BKEY_H == 19, "keybed 186-220");
    CHECK(UISQ_FOOT_Y == 221 && UISQ_FOOT_Y + UISQ_FOOT_H == 240, "footer 221-239 (caps 225-236)");
    CHECK(UISQ_HDR_Y + UISQ_HDR_H <= UISQ_KNOB_Y && UISQ_KNOB_Y + UISQ_KNOB_H <= UISQ_STAT_Y && UISQ_STAT_Y + UISQ_STAT_H == UISQ_MUS_Y &&
          UISQ_MUS_Y + UISQ_MUS_H <= UISQ_BTN_Y && UISQ_BTN_Y + UISQ_BTN_H <= UISQ_KEY_Y && UISQ_KEY_Y + UISQ_KEY_H <= UISQ_FOOT_Y, "the bands are in order and do not overlap");
    CHECK(UISQ_NSTRIP == 2 && uisq_strip_y[0] == 0 && uisq_strip_y[1] == 120 && UISQ_STRIP_H == 120 && UISQ_W * UISQ_STRIP_H <= (int)CV_MAX, "two strips of 240 x 120 fit CV_MAX");
    { /* no text band straddles y 120; only the music row (bars, strips) does */
      const int tb[][2] = { { UISQ_HDR_Y, UISQ_HDR_H }, { UISQ_KNOB_Y, UISQ_KNOB_H }, { UISQ_STAT_Y, UISQ_STAT_H }, { UISQ_BTN_Y, UISQ_BTN_H }, { UISQ_KEY_Y, UISQ_KEY_H }, { UISQ_FOOT_Y, UISQ_FOOT_H } };
      for (unsigned i = 0; i < sizeof tb / sizeof tb[0]; i++) CHECK(!(tb[i][0] < 120 && tb[i][0] + tb[i][1] > 120), "text band at y%d h%d crosses y 120", tb[i][0], tb[i][1]);
      CHECK(UISQ_MUS_Y < 120 && UISQ_MUS_Y + UISQ_MUS_H > 120, "the music row is the one band across the strip seam"); }

    /* ---- colour tokens: HI-CON values of SCREEN_SPEC_V2 3 (RGB565 of the spec's RGB), a value on every palette ---- */
    palette_set(UI_HICON_INDEX);
    CHECK(T_CH1 == RGB(230,159,0) && T_CH2 == RGB(86,180,233) && T_CH3 == RGB(0,158,115) && T_CH4 == RGB(204,121,167), "HI-CON channel colours are Okabe-Ito");
    for (int c = 0; c < 4; c++) {
        const uint16_t f = T_CH(c);
        const uint32_t r = ((f >> 11) * 255u / 31u) * 120u / 255u, g = (((f >> 5) & 63u) * 255u / 63u) * 120u / 255u, b = ((f & 31u) * 255u / 31u) * 120u / 255u;
        const uint16_t w = RGB(r, g, b), d = T_CHF(c);
        CHECK(((d >> 11) - (w >> 11) + 1u) <= 2u && ((((d >> 5) & 63u) - ((w >> 5) & 63u)) + 1u) <= 2u && (((d & 31u) - (w & 31u)) + 1u) <= 2u, "T_CHF%d is the channel colour x 120/255 (%04x vs %04x)", c + 1, d, w); }
    CHECK(T_PH == RGB(80,80,80) && T_DICE == RGB(72,72,72) && T_OFF == RGB(220,40,40), "T_PH #505050, T_DICE #484848, T_OFF #DC2828");
    for (uint32_t pi = 0; pi <= NPALETTES; pi++) {
        palette_set(pi);
        CHECK(T_CH1 != T_BG || pi == UI_BW_INDEX, "palette %u: CH1 differs from BG", pi);
        CHECK(T_PH != 0 || pi == UI_BW_INDEX, "palette %u: T_PH set", pi);
        CHECK(T_DICE != 0 && T_OFF != 0 && T_CH2 != 0 && T_CH3 != 0 && T_CH4 != 0 && T_CHF1 != 0, "palette %u: every new token has a value", pi); }
    palette_set(UI_BW_INDEX);
    CHECK((T_CH1 == 0xFFFF && T_CH4 == 0xFFFF && T_OFF == 0xFFFF && T_PH == UI_BW_GREY && T_DICE == UI_BW_GREY && T_CHF3 == UI_BW_GREY), "MONO: channels white, the rest the one grey");
    palette_set(UI_GREY_INDEX);
    CHECK(!(T_CH1 >> 11 != (T_CH1 & 31u)) && !(T_CH3 >> 11 != (T_CH3 & 31u)) && !(T_OFF >> 11 != (T_OFF & 31u)), "GREY: the new tokens are grayscale");
    palette_set(UI_HICON_INDEX);

    /* ---- the icons: every 1-bpp icon equals the mock's art (build/gen/sq_icons_ref.txt) ---- */
    { FILE *f = fopen("build/gen/sq_icons_ref.txt", "r"); char ln[64], name[32]; int n = 0, w, h;
      CHECK(f != NULL, "build/gen/sq_icons_ref.txt opens (tests/run_tests.sh writes it with tools/gen_sq_icons.py --ref)");
      while (f && fgets(ln, sizeof ln, f)) {
          int bad = 0;
          if (sscanf(ln, "%31s %d %d", name, &w, &h) != 3) continue;
          CHECK(n < SQI_N && SQI_TAB[n].w == w && SQI_TAB[n].h == h, "icon %d `%s` size %dx%d vs table %dx%d", n, name, w, h, n < SQI_N ? SQI_TAB[n].w : -1, n < SQI_N ? SQI_TAB[n].h : -1);
          cv_begin(16, 16, 0x0000);
          cv_icon(1, 1, (uint32_t)n, 0xFFFFu);                                  /* drawn at (1, 1): a miss of the origin shows */
          for (int y = 0; y < h; y++) {
              if (!fgets(ln, sizeof ln, f)) break;
              for (int x = 0; x < w; x++) {
                  const int want = ln[x] == '#', got = cv_px[(y + 1) * 16 + x + 1] == swap16(0xFFFFu);
                  if (want != got) bad++; } }
          for (int y = 0; y < 16; y++) for (int x = 0; x < 16; x++) if (cv_px[y * 16 + x] == swap16(0xFFFFu) && (x < 1 || y < 1 || x > w || y > h)) bad++;
          CHECK(!bad, "icon `%s` differs from the mock art in %d pixels", name, bad);
          n++; }
      if (f) fclose(f);
      CHECK(n == SQI_N, "the reference has %d icons, the table %d", n, SQI_N);
      printf("icons: %d 1-bpp icons pixel-identical to icons_v12.py\n", n); }

    /* ---- widths: the header (SCREEN_SPEC_V2 4.1): the short name, then `MIDI n` or the inverted `MIDI OFF` chip, end inside the 240 px screen ---- */
    { for (uint8_t a = 0; a < SQ_NALGO; a++) {
          const char *nm = uisq_short_of(a);
          const int xe = 105 + (int)text_w(&AF_M, nm);
          CHECK(strlen(nm) <= 6, "short name `%s` is over 6 characters", nm);
          CHECK(xe + 5 + W("MIDI") + 2 + (int)text_w(&AF_M, "16") <= 240, "header `%s MIDI 16` runs off the screen", nm);
          CHECK(xe + 4 + W("MIDI") + (int)text_w(&AF_M, "OFF") + 10 <= 240, "header `%s [MIDI OFF]` runs off the screen", nm); }
      { char b[16]; b[0] = 0; uisq_catu(b, sizeof b, 240); CHECK(16 + text_w(&AF_M, b) < 46, "the tempo `240` in AF_M ends before the first channel chip at x 46");
        CHECK(16 + text_w(&AF_S, "WAIT") < 46 && 16 + text_w(&AF_S, "EXT") < 46, "EXT / WAIT in AF_S end before the first channel chip"); } }
    sq_boot();

    /* ---- widths: every knob label (51 px cell, drawn with a 2 px margin), the tag column, the status lines ---- */
    for (uint32_t i = 0; i < UI_N_KNOB_MAP; i++) if (ui_knob_rows[i].knob <= UI_K_S4) fit("knob label", ui_knob_rows[i].label, 49);
    for (uint8_t p = 0; p < UI_PG_COUNT; p++) {
        const char *st[2] = { uisq_pages[p].st1, uisq_pages[p].st2 };
        for (int k = 0; k < 2; k++) fit("status line", st[k], 200);
        for (uint8_t ctl = UI_C_B_OCTDN; ctl <= UI_C_B_OCTUP; ctl++) {
            UisqBtn b; uisq_btn_get(&b, p, ctl, 0);
            { char l1[8], l2[8]; if (uisq_wrap(b.tap, l1, l2, sizeof l1, 33) && !b.sub[0]) { fit("tag line 1", l1, 33); fit("tag line 2", l2, 33); } else fit("tag tap", b.tap, 36); }
            if (b.sub[0]) fit("tag sub", b.sub, 33 - (b.sub_mark ? UISQ_MARK_W : 0)); } }

    /* ---- widths: every Z4 cell on every page for every algorithm: the computed widths fill 240 and hold the label ---- */
    { int worst = 0; const char *wp = ""; (void)wp;
      for (uint8_t p = 0; p < UI_PG_COUNT; p++) for (uint8_t a = 0; a < SQ_NALGO; a++) for (uint8_t row = 0; row < 2; row++) {
          int16_t w[6]; int sum = 0; int need = 0;
          sq_ch[0].algo = a;
          uisq_btn_widths(p, row, 0, w);
          for (int i = 0; i < 6; i++) {
              UisqBtn b; uisq_btn_get(&b, p, (uint8_t)(row * 6 + i), 0);
              need = uisq_btn_need(&b); sum += w[i];
              CHECK(need <= w[i], "page %s row %d cell %d `%s`/`%s`: ink %d px over the cell %d (algo %d)", ui_page_names[p], row, i, b.tap, b.sub, need, w[i], a);
              if (need > worst) worst = need; }
          CHECK(sum == 240, "page %s row %d cell widths sum to %d, not 240 (algo %d)", ui_page_names[p], row, sum, a);
          { int ink = 0; for (int i = 0; i < 6; i++) { UisqBtn b; uisq_btn_get(&b, p, (uint8_t)(row * 6 + i), 0); ink += uisq_btn_need(&b); }
            CHECK(ink + 6 <= 240, "page %s row %d: label ink %d + 6 px of padding is over 240 (algo %d)", ui_page_names[p], row, ink, a); } } }
    sq_boot();

    /* ---- widths: black-key legend, every page: the drawn (cut) form fits the 20 px cell ---- */
    { int cut = 0;
      for (uint8_t p = 0; p < UI_PG_COUNT; p++) for (uint8_t i = 0; i < 11; i++) {
          const char *lab = uisq_key_label(p, (uint8_t)(UI_C_KEY_B01_FS3 + i), UI_GEST_TAP); char d[16];
          if (!lab[0]) lab = uisq_key_label(p, (uint8_t)(UI_C_KEY_B01_FS3 + i), UI_GEST_HOLD);
          if (text_fit(d, sizeof d, lab, &AF_S, 18)) { cut++;
              CHECK(!(lab[0] == 'M' && lab[1] >= '1' && lab[1] <= '4' && !lab[2]), "mute key label `%s` is cut to `%s` on %s: the four mutes must stay distinguishable (FB-016)", lab, d, ui_page_names[p]); }
          CHECK(W(d) <= 20, "key label `%s` drawn as `%s` is %d px over the 20 px cell", lab, d, W(d)); }
      printf("note: %d black-key labels are cut to the 20 px cell (SCREEN_SPEC F3: the full word is in HELP)\n", cut); }

    /* ---- widths: every row of tests/design/widths.tsv against its limit (header slots, cue strips, toasts, status lines) ---- */
    { FILE *f = fopen("tests/design/widths.tsv", "r"); char ln[400]; int n = 0, over = 0;
      CHECK(f != NULL, "tests/design/widths.tsv opens (run from the repo root)");
      while (f && fgets(ln, sizeof ln, f)) {
          char *t1, *t2, *t3; int lim; const aafont_t *ft;
          if (ln[0] == '#' || ln[0] == '\n') continue;
          t1 = strchr(ln, '\t'); if (!t1) continue; *t1++ = 0;
          t2 = strchr(t1, '\t'); if (!t2) continue; *t2++ = 0;
          t3 = strchr(t2, '\t'); if (t3) *t3++ = 0;
          lim = atoi(t1); ft = ln[0] == 'M' ? &AF_M : ln[0] == 'L' ? &AF_L : &AF_S;
          n++;
          if (text_w(ft, t2) > lim) { over++; printf("FAIL widths.tsv: `%s` is %d px, limit %d (%s)\n", t2, (int)text_w(ft, t2), lim, t3 ? t3 : ""); fails++; }
          if (lim == 240 && ft == &AF_S && t3 && strstr(t3, "toast")) CHECK(text_w(ft, t2) <= 236, "toast `%s` is %d px: over the 236 px a toast draws in (x=4)", t2, (int)text_w(ft, t2)); }
      if (f) fclose(f);
      printf("widths.tsv: %d rows measured, %d over\n", n, over); }
    for (uint8_t p = 0; p < UI_PG_COUNT; p++) fit("page cue", uisq_page_cue(p), 236);
    CHECK(!strcmp(uisq_page_cue(UI_PG_PERF), "PLAY: start   K1-K4: morph"), "the PERF cue comes from the generated page_states.tsv column: `%s`", uisq_page_cue(UI_PG_PERF));
    CHECK(!strcmp(uisq_page_cue(UI_PG_RIG), "RIG: TAP 1-4 = WHO DRIVES  HOLD = PICK"), "the RIG cue is generated");
    { const char *m1 = uisq_key_label(UI_PG_PERF, UI_C_KEY_B01_FS3, UI_GEST_TAP), *m2 = uisq_key_label(UI_PG_PERF, UI_C_KEY_B02_GS3, UI_GEST_TAP);
      CHECK(strcmp(m1, m2) != 0 && W(m1) <= 20 && W(m2) <= 20, "the mute keys M1 / M2 are distinct and fit 20 px: `%s` `%s`", m1, m2); }

    /* ---- render every page: full screens, then each zone of PERF ---- */
    sq_ch[0].len = 20; sq_ch[0].pg_used = 3; sq_ch[1].muted = 0; sq_ch[2].midi_ch = SQ_MIDI_OFF;
    sq_ch[0].ed[2].vel = 90; sq_ch[0].ed[4].fl = 1; sq_ch[0].ed[7].fl = (uint8_t)(1 | (1 << 2)); sq_ch[0].ed[9].gr = 0x20;
    sq_rebuild(0);
    uisq_sel = 0; uisq_page = UI_PG_PERF; sq_playing = 1; sq_rt[0].p = 6; fresh();
    CHECK(blits == 2, "the first frame draws the two strips, drew %d", blits);
    shot("frame_playing", 0, 0, 240, 240);
    { /* the empty frame of `playing`: the boxes of SCREEN_SPEC_V2 2 hold their colour, their surround is BG */
      const uint16_t bg = swap16(T_BG), sf = swap16(T_SURF), dc = swap16(T_DICE), rs = swap16(T_RAISE);
#define PX(x, y) fb[(y) * 240 + (x)]
      for (int k = 0; k < 4; k++) {
          const int x = 1 + 60 * k;
          CHECK(PX(x, 24) == sf && PX(x + 57, 39) == sf && PX(x, 41) == sf && PX(x + 57, 41) == sf, "knob tile %d: label band 24-39 and value area 41-76 are T_SURF (the corners; the meter fills from the bottom)", k);
          CHECK(PX(x + 20, 40) == bg && PX(x + 20, 23) == bg && PX(x + 20, 77) == bg && (x == 1 || PX(x - 1, 50) == bg) && PX(x + 58, 50) == bg, "knob tile %d: 1 px gap and BG surround", k); }
      for (int i = 0; i < 12; i++) {
          const int x = 1 + 40 * (i % 6), y = 146 + 18 * (i / 6);
          const uint16_t want = uisq_btn_dice[i] ? dc : sf;
          CHECK(PX(x, y) == want && PX(x + 37, y + 16) == want && PX(x + 20, y + 8) == want, "button tile %d is %s", i, uisq_btn_dice[i] ? "T_DICE" : "T_SURF");
          CHECK(PX(x - 1, y + 8) == bg && PX(x + 38, y + 8) == bg && PX(x + 20, y + 17) == bg, "button tile %d: BG surround (tile 38 x 17, pitch 40 x 18)", i); }
      CHECK(PX(20, 146) == dc && PX(20 + 120, 146) == dc && PX(1 + 40 * 4 + 5, 146) == sf, "FX SCL ENV LFO dice tiles, EDIT GLO plain");
      for (int i = 0; i < 16; i++) {
          const int x = 1 + 15 * i;
          CHECK(PX(x, 218) == sf && PX(x + 13, 186 + 32) == sf && PX(x, 186 + 33) == bg && PX(x + 13, 185) == bg, "white key %d: 14 x 33 at y 186", i); }
      for (int i = 0; i < 11; i++) {
          const int x = uisq_bkey_cx[i] - 5;
          CHECK(PX(x, 186) == rs && PX(x + 9, 186 + 18) == rs && PX(x + 5, 186 + 19) != rs && PX(x - 1, 186) == bg, "black key %d: 10 x 19 at x centre %d", i, uisq_bkey_cx[i]); }
      CHECK(PX(5, 22) == bg && PX(120, 100) == bg && PX(120, 230) == bg, "header, music row and footer bands are empty in the frame");
      /* the strip seam: the music row draws across it; tests/ui_sq_v12_test.c + v12_compare.py prove the pixels on both sides equal the mock (drawn in the same two strips) */
#undef PX
    }
    sq_playing = 0; sq_rt[0].p = 0;
    for (uint8_t p = 0; p < UI_PG_COUNT; p++) {
        char nm[48];
        if (p == UI_PG_MENU) { uisq_page = p; blits = 0; ui.force = 1; frame(); ui.force = 0; CHECK(blits == 0, "MENU draws nothing (ui_menu.c owns the screen)"); continue; }
        uisq_page = p; uisq_edit_step = 20; fresh();
        snprintf(nm, sizeof nm, "page_%02d_%s", p, ui_page_names[p]); shot(nm, 0, 0, 240, 240); }
    uisq_page = UI_PG_PERF;

    /* ---- redraw by signature ---- */
    fresh(); blits = blit_a = blit_b = blit_c = 0;
    frame(); frame(); CHECK(blits == 0, "an unchanged state blits nothing, drew %d", blits);
    ui.force = 1; frame(); ui.force = 0; CHECK(blits == 2, "ui.force redraws both strips, drew %d", blits);
    blits = blit_a = blit_b = blit_c = 0; palette_set(UI_GREY_INDEX); frame(); CHECK(blit_a == 1 && blit_b == 1, "a palette change redraws both strips: A%d B%d", blit_a, blit_b); palette_set(UI_HICON_INDEX); frame();
    blits = blit_a = blit_b = blit_c = 0; uisq_sel = 1; frame(); CHECK(blit_a == 1 && blit_b == 1, "SELECT redraws both strips: A%d B%d", blit_a, blit_b); uisq_sel = 0; frame();

    /* ---- the toast ---- */
    blits = blit_a = blit_b = blit_c = 0; fm1_ms = 5000;
    ui_sq_toast("SAVED P05"); frame(); CHECK(blit_a == 1 && blit_b == 0, "a toast redraws strip 0 (the message slot) only: A%d B%d", blit_a, blit_b);
    { uint16_t id = uisq_toast_id; fm1_ms += 1000; ui_sq_toast("SAVED P05"); CHECK(uisq_toast_id == id, "the same text only restarts the timer");
      ui_sq_toast("UNDO: pattern"); CHECK(uisq_toast_id != id && uisq_streq(uisq_toast, "UNDO: pattern"), "a new text replaces the one showing (no queue)"); }
    fm1_ms += 1950; frame(); CHECK(uisq_toast_until != 0, "the toast is still up 1967 ms after it was raised");
    fm1_ms += 40; frame(); CHECK(uisq_toast_until == 0, "the toast is gone 2000 ms after it was raised");
    ui_sq_home_held_set(1); ui_sq_toast("HOME"); CHECK(uisq_toast_until == 0, "a toast raised while HOME is held is dropped"); ui_sq_home_held_set(0);
    ui_sq_toast("P3 READS 17-32: NO PAGE PLAYS IT"); frame(); CHECK(W(uisq_toast) <= 236, "the longest locked toast fits");
    uisq_toast_until = 0;
    ui_sq_algo_pick_touch(); frame();
    { char b[64]; uisq_cue_text(b, sizeof b); CHECK(uisq_streq(b, "[ACID] EUCL SNAK STAG ARP PATT"), "ALGO_PICK strip lists the six with ACID bracketed: `%s`", b); fit("pick strip", b, 236); }
    fm1_ms += 2100; frame(); CHECK(uisq_pick_until == 0, "ALGO_PICK ends 2 s after the last detent");

    /* ---- page dispatch: every page has an entry, the overlays use PERF's layout ---- */
    for (uint8_t p = 0; p < UI_PG_COUNT; p++) CHECK(uisq_pages[p].input != NULL, "page %s has an input entry", ui_page_names[p]);
    uisq_page = UI_PG_ALGO_PICK; CHECK(uisq_layout_page() == UI_PG_PERF, "an overlay draws on PERF's layout");
    uisq_page = UI_PG_CHAN; CHECK(uisq_layout_page() == UI_PG_CHAN, "a page draws its own layout");

    printf(fails ? "ui_sq_test: %d FAILED\n" : "ui_sq_test: OK (%d failures)\n", fails);
    return fails != 0;
}
