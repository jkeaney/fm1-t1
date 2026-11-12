/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_algodesc_test.c (P4-027, ADR-0007): the screen reads an algorithm only through its descriptor. The real ui_sq.c (unchanged) draws the
 * knob tiles of (a) ACID, the real firmware row, and (b) DEMO, a hypothetical algorithm registered only as an ALGOS[] row (short name,
 * lane, value names, unit, zero word, a value_text hook). Nothing in ui_sq.c knows DEMO. Asserts the value text, the one-font-per-knob
 * rule, the position dots, the header short name, and that every tile's ink stays inside its 58 px tile.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/sq_algodesc_test tests/sq_algodesc_test.c -lm && /tmp/sq_algodesc_test
 * Run from the repo root after ./build.sh (needs build/gen). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#define __attribute__(x)

static uint16_t fb[240 * 240];
static void lcd_sync(void) {}
static void lcd_power(uint32_t s) { (void)s; }
static void lcd_wake_now(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{   for (uint32_t r = 0; r < h; r++) memcpy(&fb[(y + r) * 240 + x], &p[r * w], w * 2); }
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
static struct { uint8_t force; } ui;

#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"
#include "sq_algo_acid.c"

/* ---- DEMO: a hypothetical algorithm that exists only as this row ---- */
static void d_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k) { (void)m; (void)seed; (void)g; (void)k; }
static uint8_t d_len(const void *k) { (void)k; return 16; }
static void d_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o) { (void)k; (void)g; (void)hc; memset(o, 0, sizeof *o); o->note = 48; o->vel = 80; o->gate = 50; }
static const char *const demo_scale[4] = { "MAJ", "MIN", "DOR", "PENT" };
static const char *const demo_dir[3] = { "FWD", "BWD", "PEND" };
static uint8_t demo_text(uint8_t morph, uint8_t v, const void *cache, char *b)   /* LOCK: "L9", "L16"; others default */
{   (void)cache;
    if (morph != 0) return 0;
    b[0] = 'L'; if (v >= 10) { b[1] = (char)('0' + v / 10); b[2] = (char)('0' + v % 10); b[3] = 0; return 3; }
    b[1] = (char)('0' + v); b[2] = 0; return 2; }
#define DEMO_ROW { .name = "DEMO", .short_name = "DEMO", .lane = SQ_LANE_SINGLE, .id = 5, .n_seed = 2, .cache_bytes = 4, \
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 }, .seed_k = { 1, 2 }, \
    .morph = { { "LOCK", 0, 16, 9, 1, 1 }, { "BITS", 1, 16, 8, 1, 1 }, { "SCALE", 0, 3, 1, 1, 1, demo_scale }, \
               { "SPRD", 1, 4, 2, 1, 1, 0, "OCT" }, { "OFFSET", 0, 15, 0, 1, 1 }, { "GATE", 1, 8, 4, 1, 1 }, \
               { "DYN", 0, 10, 10, 1, 1 }, { "DIR", 0, 2, 0, 1, 1, demo_dir } }, \
    .layer = { {1, {0}}, {1, {1}}, {0, {0}}, {1, {0}}, {1, {1}} }, .sub = { "NOTES", "HITS", "-", "GATE", "SHAPE" }, \
    .build = d_build, .loop_len = d_len, .step = d_step, .value_text = demo_text }
#define STUB(i, nm) { .name = nm, .id = i, .n_seed = 2, .cache_bytes = 4, .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 }, .seed_k = { 1, 2 }, \
    .morph = { { "A", 0, 9, 1, 1, 1 }, { "B", 0, 9, 1, 1, 1 }, { "C", 0, 9, 1, 1, 1 }, { "D", 0, 9, 1, 1, 1 }, \
               { "E", 0, 9, 1, 1, 1 }, { "F", 0, 9, 1, 1, 1 }, { "G", 0, 9, 1, 1, 1 }, { "H", 0, 9, 1, 1, 1 } }, \
    .layer = { {1, {0}}, {1, {1}}, {1, {0}}, {1, {1}}, {1, {0}} }, .sub = { "N", "H", "A", "T", "S" }, \
    .build = d_build, .loop_len = d_len, .step = d_step }
static const AlgoDesc ALGOS[SQ_NALGO] = {
    [0] = ALGO_ACID_ROW, [1] = STUB(1, "EUCLID"), [2] = STUB(2, "SNAKE"), [3] = STUB(3, "STAGE"), [4] = STUB(4, "ARP"),
    [5] = DEMO_ROW,
};

#include "sq_core.c"
#include "sq_chan.c"
#include "../firmware/src/ui_sq.c"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

static void expect(const AlgoDesc *a, uint8_t id, uint8_t v, const char *want)
{   char t[16]; uisq_morph_text(t, sizeof t, a, id, v, 0);
    CHECK(!strcmp(t, want), "%s morph %u value %u reads `%s`, wanted `%s`", a->name, id, v, t, want); }

/* draw the four tiles of one row of `a` through the screen code, then check the ink stays in the tiles */
static void tiles(const AlgoDesc *a, uint8_t row, const uint8_t morph[SQ_NMORPH])
{   const uint16_t bg = 0;
    cv_begin(240, 120, T_BG);
    UisqView v;
    memset(&v, 0, sizeof v);
    v.band = UISQ_BAND_ROW; v.row = row; v.nrow = 2; v.algo = a; v.turn = 0;
    for (uint32_t k = 0; k < 4; k++) { v.tile_id[k] = a->knob_morph[(row * 4 + k) & 7]; v.tile[k] = &a->morph[v.tile_id[k]]; v.val[k] = morph[v.tile_id[k]]; }
    uisq_view_texts(&v);
    uisq_draw_knobs(&v, UISQ_KNOB_Y);
    cv_blit(0, 0);
    for (int y = UISQ_KNOB_Y; y < UISQ_KNOB_Y + UISQ_KNOB_LAB + 1 + UISQ_KNOB_VAL; y++) {
        for (int g = 0; g < 5; g++) {                       /* the gaps between and around the tiles stay background */
            const int x = g == 0 ? 0 : 59 + 60 * (g - 1);
            CHECK(fb[y * 240 + x] == fb[0], "%s row %u: ink in the gap at x %d y %d", a->name, row, x, y); } }
    for (uint32_t k = 0; k < 4; k++) {                      /* the label band and the value area both carry ink beyond the tile fill */
        const uint16_t fill = fb[(UISQ_KNOB_Y + 1) * 240 + 1 + 60 * k];
        int lab = 0, val = 0;
        for (int y = UISQ_KNOB_Y; y < UISQ_KNOB_Y + UISQ_KNOB_LAB; y++) for (int x = 1 + 60 * (int)k; x < 59 + 60 * (int)k; x++) lab += fb[y * 240 + x] != fill;
        for (int y = UISQ_KNOB_Y + UISQ_KNOB_LAB + 1; y < UISQ_KNOB_Y + UISQ_KNOB_LAB + 1 + UISQ_KNOB_VAL; y++) for (int x = 1 + 60 * (int)k; x < 59 + 60 * (int)k; x++) val += fb[y * 240 + x] != fill;
        CHECK(lab > 0 && val > 0, "%s row %u tile %u: label ink %d, value ink %d", a->name, row, k, lab, val); }
}

int main(void)
{   const AlgoDesc *acid = &ALGOS[0], *demo = &ALGOS[5];
    uint8_t m[SQ_NMORPH];
    palette_set(UI_HICON_INDEX);

    /* ACID, the real row */
    CHECK(!strcmp(uisq_short_of(0), "ACID") && acid->lane == SQ_LANE_PITCHED, "ACID short name / lane");
    expect(acid, 0, 0, "KEYS"); expect(acid, 0, 12, "12");
    expect(acid, 7, 0, "FWD"); expect(acid, 7, 1, "BWD"); expect(acid, 7, 2, "PEND");
    CHECK(uisq_morph_dots(acid, 7) == 3 && uisq_morph_dots(acid, 6) == 0 && uisq_morph_dots(acid, 0) == 0, "ACID dots: DIR 3, PUNCH / DENS meters");
    CHECK(uisq_morph_font(acid, 0) == &AF_L && uisq_morph_font(acid, 6) == &AF_L, "ACID DENS / PUNCH use AF_L (KEYS may drop to AF_M by itself)");
    CHECK(uisq_morph_font(acid, 7) == &AF_M || uisq_morph_font(acid, 7) == &AF_L, "ACID DIR has a font");
    for (int i = 0; i < SQ_NMORPH; i++) m[i] = acid->morph[i].def;
    tiles(acid, 0, m); tiles(acid, 1, m);
    m[0] = 0; tiles(acid, 0, m);                              /* DENS 0 -> KEYS */

    /* DEMO: no per-algorithm code in ui_sq.c */
    CHECK(!strcmp(uisq_short_of(5), "DEMO") && demo->lane == SQ_LANE_SINGLE, "DEMO short name / lane come from its row");
    expect(demo, 0, 9, "L9"); expect(demo, 0, 16, "L16");   /* the value_text hook */
    expect(demo, 2, 1, "MIN"); expect(demo, 2, 3, "PENT"); /* names, indexed by value - min */
    expect(demo, 3, 2, "2 OCT");                            /* unit */
    expect(demo, 1, 8, "8"); expect(demo, 6, 10, "10");     /* plain numbers */
    CHECK(uisq_morph_dots(demo, 2) == 4 && uisq_morph_dots(demo, 7) == 3 && uisq_morph_dots(demo, 5) == 0, "DEMO dots");
    for (int i = 0; i < SQ_NMORPH; i++) m[i] = demo->morph[i].def;
    tiles(demo, 0, m); tiles(demo, 1, m);

    printf(fails ? "SQ_ALGODESC FAIL (%d)\n" : "SQ_ALGODESC PASS\n", fails);
    return fails ? 1 : 0;
}
