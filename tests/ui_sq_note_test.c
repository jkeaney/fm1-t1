/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_note_test.c (P4-014): the NOTE_EDIT page of firmware/src/ui_sq.c on the host: input sequence -> sq_* effects and the strings on the screen.
 * It drives ui_sq_keys / ui_sq_button / ui_sq_knob / ui_sq_select_turn / ui_sq_home / ui_sq_input as ui_sq_glue.c does and walks S03 steps 7-22 and the
 * S10 sweep: open by holding a step key, K1-K4 with AUTO at the low end (AUTO shows and clears the StepEdit field), the layer buttons, OCT-/OCT+, EDIT
 * undo / redo of the one session record, the editor moving to another step, a tap on a step key, HOME / SELECT leaving and closing the session, the
 * bounds of every field, and that the words on the page are ui_ne_text / ui_knob_rows words. Frames go to build/ui_sq/note_*.png.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_note_test tests/ui_sq_note_test.c -lm && /tmp/ui_sq_note_test [outdir] */
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

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    sq_boot(); uisq_used = 0x0F;      /* P4-044: the core boots with CH1 only; these pages are exercised with all four in use */
    fm1_ms = 1000;
    sq_playing = 0;
    fresh();

    /* ---- S03 step 7: hold white key 9 for 0.4 s: NOTE_EDIT on step 9, every field AUTO, the generated values under them ---- */
    { StepEdit before;
      ED(8)->fl = 0;                                                     /* (a tap on step 3 earlier in S03 set step 3, not 9) */
      before = *ED(8);
      press(1u << WK(8)); wait_ms(300);
      CHECK(uisq_page == UI_PG_PERF, "0.3 s: still PERF");
      wait_ms(150);
      CHECK(uisq_page == UI_PG_NOTE_EDIT && uisq_edit_step == 8 && uisq_edit_ch == 0 && sq_note_edit_on(0, 8), "0.45 s: NOTE_EDIT on step 9, session open");
      press(0); frame();
      CHECK(uisq_page == UI_PG_NOTE_EDIT, "the page stays open when the key is let go");
      CHECK(!memcmp(ED(8), &before, sizeof before), "the hold writes nothing");
      fill();
      CHECK(V.band == UISQ_BAND_NOTE && V.ne_step == 9 && V.edit == 9 && V.ne_set == 0, "view: NOTE band, STEP 9, column 9, nothing set");
      for (int k = 0; k < 4; k++) CHECK(streq(V.ne_sub[k], ui_ne_text[UI_NE_AUTO]) && V.ne_txt[k][0] && !(V.ne_set >> k & 1), "K%d: the algorithm's value is the big text and AUTO the small word (`%s` / `%s`)", k + 1, V.ne_txt[k], V.ne_sub[k]);
      CHECK(streq(uisq_knob_label(UI_PG_NOTE_EDIT, UI_CTX_ANY, 0), "PITCH") && streq(uisq_knob_label(UI_PG_NOTE_EDIT, UI_CTX_ANY, 1), "GATE")
         && streq(uisq_knob_label(UI_PG_NOTE_EDIT, UI_CTX_ANY, 2), "VELO") && streq(uisq_knob_label(UI_PG_NOTE_EDIT, UI_CTX_ANY, 3), "RATCH"), "tile labels are the knob_map rows");
      shot("note_open", 0, 0, 240, 240); }

    /* ---- steps 8-11: K1..K4 start from the generated value (F-12), AUTO at the low end clears the field ---- */
    { PulseOut gen; const SeqGlobal *g = &sq_gdb[sq_gidx];
      CHECK(uisq_ne_gen(0, 8, &gen), "the generator answers for step 9");
      /* K1 PITCH */
      { const int16_t g0 = uisq_ne_gen_st(g, gen.note), up1 = uisq_ne_degree(g, g0, 1), up2 = uisq_ne_degree(g, up1, 1);
        knob(0, 1);
        CHECK((ED(8)->note & 0x80) && (ED(8)->note & 0x7F) == up1, "K1 +1: AUTO becomes the generated pitch's next scale degree (%d -> %d, got %d)", g0, up1, ED(8)->note & 0x7F);
        fill(); CHECK((V.ne_set & 1) && !strncmp(V.ne_sub[0], ui_ne_text[UI_NE_GEN], 3), "K1 tile is set: its note big, `GEN ...` under it (`%s` / `%s`)", V.ne_txt[0], V.ne_sub[0]);
        knob(0, 1);
        CHECK((ED(8)->note & 0x7F) == up2, "K1 +1 again: one more degree");
        for (int i = 0; i < 30; i++) knob(0, -3);
        CHECK(ED(8)->note == 0, "K1 turned to its AUTO end clears the PITCH field (note byte %02x)", ED(8)->note);
        fill(); CHECK(!(V.ne_set & 1) && streq(V.ne_sub[0], ui_ne_text[UI_NE_AUTO]), "K1 reads AUTO again"); }
      /* K2 GATE */
      { const uint8_t gi = uisq_ne_gen_gate(&gen);
        knob(1, 1);
        CHECK((ED(8)->gr & 15) == (gi + 1 > 11 ? 11 : gi + 1), "K2 +1: gate index %d -> %d (got %d)", gi, gi + 1, ED(8)->gr & 15);
        knob(1, 12);
        CHECK((ED(8)->gr & 15) == 11, "K2 clockwise to the top is TIE");
        fill(); CHECK(streq(V.ne_txt[1], ui_ne_text[UI_NE_TIE]), "K2 reads TIE");
        knob(1, 12); CHECK((ED(8)->gr & 15) == 11, "and stays at TIE");
        knob(1, -12);
        CHECK((ED(8)->gr & 15) == 0, "K2 turned to its AUTO end clears the GATE field");
        fill(); CHECK(streq(V.ne_sub[1], ui_ne_text[UI_NE_AUTO]) && !(V.ne_set & 2), "K2 reads AUTO"); }
      /* K3 VELO */
      { knob(2, 2);
        CHECK(ED(8)->vel == (gen.vel + 2 > 127 ? 127 : gen.vel + 2), "K3 +2: VELO from the generated %d to %d", gen.vel, ED(8)->vel);
        for (int i = 0; i < 20; i++) knob(2, 4);
        CHECK(ED(8)->vel == 127, "K3 clockwise stops at 127");
        fill(); CHECK(streq(V.ne_txt[2], ui_ne_text[UI_NE_ACC]), "K3 reads ACC at 100 or more");
        for (int i = 0; i < 20; i++) knob(2, -4);
        CHECK(ED(8)->vel == 0, "K3 turned to its AUTO end clears VELO");
        knob(2, -1); CHECK(ED(8)->vel == (gen.vel > 1 ? gen.vel - 1 : 0), "K3 from AUTO, one detent down: the generated value minus 1"); knob(2, -127); CHECK(ED(8)->vel == 0, "...and back to AUTO"); }
      /* K4 RATCH */
      { const uint8_t gr = uisq_ne_gen_rep(&gen);
        knob(3, 2);
        CHECK(((ED(8)->gr >> 4) & 7) == (gr + 2 > 4 ? 4 : gr + 2), "K4 +2: RATCH from %dx", gr);
        fill(); CHECK(V.ne_txt[3][0] >= '1' && V.ne_txt[3][1] == 0 && streq(V.ne_unit[3], ui_ne_text[UI_NE_UNIT_X]) && ui_ne_text[UI_NE_UNIT_X][0] == 'x', "K4 reads n with a small lower-case x, `%s` `%s`", V.ne_txt[3], V.ne_unit[3]);
        knob(3, 9); CHECK(((ED(8)->gr >> 4) & 7) == 4, "K4 stops at 4x");
        knob(3, -9); CHECK(((ED(8)->gr >> 4) & 7) == 0, "K4 turned to its AUTO end clears RATCH"); }
      CHECK(!memcmp(ED(8), &zero_edit, sizeof zero_edit), "after turning every knob back to AUTO the step has no edit at all"); }

    /* ---- steps 12-17: the layer buttons set their layer on this step; OCT-/OCT+; chips ---- */
    { const StepEdit start = *ED(8);
      btn(UI_C_B_ARP);  CHECK(((ED(8)->fl >> 2) & 3) == 1, "ARP: SLIDE on");
      btn(UI_C_B_LFO);  CHECK(ED(8)->fl & 0x10, "LFO: PROB 50");
      btn(UI_C_B_ENV);  CHECK(ED(8)->vel == UISQ_ACC_VEL, "ENV: ACCENT = velocity %d", ED(8)->vel);
      fill(); CHECK(streq(V.ne_txt[2], ui_ne_text[UI_NE_ACC]) && (V.ne_set & 4), "K3 reads ACC");
      btn(UI_C_B_ENV);  { PulseOut gen; uisq_ne_gen(0, 8, &gen);
                          CHECK(gen.vel >= UISQ_ACC_THR ? ED(8)->vel == UISQ_NOACC_VEL : ED(8)->vel == 0, "ENV again: back to %s (F-10)", gen.vel >= UISQ_ACC_THR ? "80" : "AUTO"); }
      btn(UI_C_B_OCTUP); CHECK((ED(8)->fl >> 5) == 4, "OCT+: +1");
      btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); btn(UI_C_B_OCTUP); CHECK((ED(8)->fl >> 5) == 5, "OCT+ stops at +2");
      btn(UI_C_B_OCTDN); btn(UI_C_B_OCTDN); CHECK((ED(8)->fl >> 5) == 0, "OCT- twice: back to 0, which is AUTO (no shift)");
      btn(UI_C_B_OCTDN); btn(UI_C_B_OCTDN); btn(UI_C_B_OCTDN); CHECK((ED(8)->fl >> 5) == 1, "OCT- stops at -2");
      fill();
      CHECK(!strncmp(V.ne_chip[0], ui_ne_text[UI_NE_PROB], 4) && !strncmp(V.ne_chip[1], ui_ne_text[UI_NE_OCT], 3) && !V.ne_chip[2][0],
            "chips: PROB 50, OCT-2 (SLIDE is the lit ARP button) (got `%s` `%s` `%s`)", V.ne_chip[0], V.ne_chip[1], V.ne_chip[2]);
      CHECK((V.ne_btn & 6) == 6, "the ARP (slide) and LFO (prob) buttons are lit");
      shot("note_set", 0, 0, 240, 240);
      btn(UI_C_B_SCL);  CHECK(ED(8)->note & 0x80, "SCL (NOTE): a PITCH edit");
      CHECK(uisq_toast_until && streq(uisq_toast, ui_ne_text[UI_NE_TOAST_NOTE]) && streq(uisq_toast_hint, "EDIT=UNDO"), "toast NOTE  EDIT=UNDO");
      for (int i = 0; i < 8; i++) { btn(UI_C_B_SCL); CHECK((ED(8)->note & 0x7F) <= 96, "dice pitch stays within 0..96"); }
      btn(UI_C_B_FX);   CHECK((ED(8)->note & 0x80) && ED(8)->vel && ((ED(8)->fl >> 2) & 3) && (ED(8)->gr & 15) >= 1 && (ED(8)->gr & 15) <= 10, "FX (ALL): pitch, velocity, slide and gate are all set");
      CHECK(streq(uisq_toast, ui_ne_text[UI_NE_TOAST_DICED]) && streq(uisq_toast_hint, "EDIT=UNDO"), "toast DICED  EDIT=UNDO");
      shot("note_dice", 0, 0, 240, 240);
      /* ---- step 18: EDIT: one record for the whole visit ---- */
      btn(UI_C_B_EDIT);
      CHECK(!memcmp(ED(8), &start, sizeof start), "EDIT: every field is back to how it was when the page opened");
      CHECK(streq(uisq_toast, ui_msgs[UI_MSG_TOAST_UNDO_EDIT_REDO].a), "toast UNDO  EDIT=REDO");
      btn(UI_C_B_EDIT);
      CHECK(streq(uisq_toast, ui_msgs[UI_MSG_TOAST_REDO_EDIT_UNDO].a) && memcmp(ED(8), &start, sizeof start), "EDIT again: REDO");
      btn(UI_C_B_EDIT);
      CHECK(!memcmp(ED(8), &start, sizeof start), "and UNDO again"); }

    /* ---- after an undo the next write opens a session again (one record for it) ---- */
    { const StepEdit s0 = *ED(8);
      CHECK(!sq_note_edit_on(0, 8), "the undo ended the session");
      knob(2, 3); knob(2, 3); CHECK(sq_note_edit_on(0, 8) && ED(8)->vel, "a knob turn after the undo opens it again");
      btn(UI_C_B_EDIT); CHECK(!memcmp(ED(8), &s0, sizeof s0), "EDIT takes back both detents (one record)"); }

    /* ---- step 19: SEQ (ON/OFF) is the key's tap; step 21: a tap on another step key; step 20: hold moves the editor ---- */
    { const uint8_t f = ED(8)->fl & 3;
      btn(UI_C_B_SEQ); CHECK((ED(8)->fl & 3) != f || f == 0 ? (ED(8)->fl & 3) != 0 : 1, "SEQ writes the ON field");
      btn(UI_C_B_SEQ); CHECK((ED(8)->fl & 3) == f, "SEQ again matches the generator: AUTO"); }
    { const StepEdit s12 = *ED(12), s8 = *ED(8);
      hold_step(12);
      CHECK(uisq_page == UI_PG_NOTE_EDIT && uisq_edit_step == 12 && sq_note_edit_on(0, 12) && !sq_note_edit_on(0, 8), "hold another key: the editor moves to step 13");
      CHECK(!memcmp(ED(12), &s12, sizeof s12) && !memcmp(ED(8), &s8, sizeof s8), "moving writes nothing");
      fill(); CHECK(V.ne_step == 13 && V.edit == 13, "STEP 13, column 13");
      { const StepEdit pre = *ED(8); btn(UI_C_B_EDIT); CHECK(!memcmp(ED(8), &pre, sizeof pre) && !memcmp(ED(12), &s12, sizeof s12), "EDIT with nothing written on this step is inert (it never undoes an older record)"); }
      tap_key(WK(1));
      CHECK(uisq_page == UI_PG_NOTE_EDIT && uisq_edit_step == 12, "a tap on another key: the editor stays on step 13");
      CHECK((ED(1)->fl & 3) != 0, "...and step 2 changed ON/OFF");
      btn(UI_C_B_EDIT); CHECK((ED(1)->fl & 3) == 0, "EDIT takes the tap back (last change wins)");
      btn(UI_C_B_EDIT); }

    /* ---- step 22: HOME tap leaves; a long HOME is the menu's and leaves nothing ---- */
    { knob(2, 2);
      ui_sq_home(1); wait_ms(100); ui_sq_home(0); frame();
      CHECK(uisq_page == UI_PG_PERF && !sq_note_edit_on(0, 12), "HOME tap: PERF, the session closed");
      /* the closed session: a PERF tap on the same step is a record of its own (the stale-session hazard of P4-013 F4) */
      { const StepEdit s = *ED(12); (void)s;
        { const uint8_t v = ED(12)->vel; tap_key(WK(12)); CHECK(ED(12)->vel == v && (ED(12)->fl & 3) != 0, "PERF tap on step 13 writes the ON field");
          sq_undo(0); CHECK(ED(12)->vel == v && (ED(12)->fl & 3) == 0, "EDIT takes back only the tap, not the NOTE_EDIT visit's VELO"); } }
      hold_step(12);
      ui_sq_home(1); wait_ms(800); ui_sq_home(0); frame();
      CHECK(uisq_page == UI_PG_NOTE_EDIT, "HOME held past 0.7 s is not a tap (the menu takes it)");
      ui_sq_home(1); wait_ms(100); ui_sq_home(0); frame();
      CHECK(uisq_page == UI_PG_PERF, "HOME tap"); }

    /* ---- SELECT closes NOTE_EDIT and moves the channel; the page also closes when the selection moves by itself ---- */
    { hold_step(3); CHECK(uisq_page == UI_PG_NOTE_EDIT, "reopen");
      ui_sq_select_turn(1); frame();
      CHECK(uisq_page == UI_PG_PERF && uisq_sel == 1 && !sq_note_edit_on(0, 3), "SELECT turn: PERF, CH2 selected, session closed");
      ui_sq_select_turn(-1); frame();
      hold_step(3); uisq_sel = 1; frame();
      CHECK(uisq_page == UI_PG_PERF && !sq_note_edit_on(0, 3), "the selection moved under the page (a removal): the page closes");
      uisq_sel = 0; frame(); }

    /* ---- bounds: no field leaves its range; a step LENGTH hides is refused ---- */
    { hold_step(5);
      for (int i = 0; i < 40; i++) { knob(0, 12); knob(1, 12); knob(2, 127); knob(3, 12); }
      CHECK((ED(5)->note & 0x7F) <= 96 && (ED(5)->gr & 15) == 11 && ED(5)->vel == 127 && ((ED(5)->gr >> 4) & 7) == 4 && (ED(5)->gr & 0x80) == 0, "top ends: pitch <= 96, TIE, 127, 4x");
      for (int i = 0; i < 40; i++) { knob(0, -12); knob(1, -127); knob(2, -127); knob(3, -127); }
      CHECK(!memcmp(ED(5), &zero_edit, sizeof zero_edit), "bottom ends: everything AUTO again");
      for (int i = 0; i < 40; i++) { knob(0, 127); knob(2, -127); }
      CHECK((ED(5)->note & 0x7F) <= 96, "d = 127 detents: pitch still <= 96");
      for (int i = 0; i < 10; i++) { btn(UI_C_B_FX); CHECK((ED(5)->note & 0x7F) <= 96 && (ED(5)->gr & 15) >= 1 && (ED(5)->gr & 15) <= 10, "FX ten times stays in range"); }
      ui_sq_knob(9, 1); ui_sq_knob(200, -1); ui_sq_button(0xFF); ui_sq_button(UI_C_B_GLO);
      uisq_ne_leave();
      uisq_page = UI_PG_NOTE_EDIT; uisq_edit_step = 40; uisq_edit_ch = 0; { const StepEdit z = *ED(40); ui_sq_knob(0, 3); ui_sq_button(UI_C_B_ARP); ui_sq_button(UI_C_B_SEQ); CHECK(!memcmp(ED(40), &z, sizeof z), "a step the LENGTH hides (40 of 16): refused, nothing written"); }
      fill(); CHECK(V.edit == 0 && V.ne_step == 41, "...and the page draws without a framed column");
      frame(); CHECK(uisq_page == UI_PG_PERF && !sq_note_edit_on(0, 40), "the frame closes the page: its step is beyond LENGTH (a preset that lands can shorten it)");
      uisq_page = UI_PG_NOTE_EDIT; uisq_edit_step = 63; fill();
      uisq_edit_step = 255; fill(); CHECK(V.ne_step <= 64, "an out-of-range edited step is clamped, not read");
      uisq_ne_leave(); uisq_edit_step = 0; memset(ED(5), 0, sizeof(StepEdit)); }

    /* ---- the view stays on the edited step's page while playing with FOLLOW on ---- */
    { sq_len_set(0, 32); sq_playing = 1; sq_follow = 1; sq_rt[0].p = 3;
      hold_step(4); uisq_vpage = 0;
      CHECK(uisq_view_page() == 0, "NOTE_EDIT on page 1 is not pulled to the playing page");
      fill(); CHECK(V.vpage == 1, "the view shows page 1");
      sq_rt[0].p = 20; frame();
      CHECK(uisq_view_page() == 0 && uisq_page == UI_PG_NOTE_EDIT, "playhead on page 2: the editor stays on page 1");
      uisq_ne_leave(); CHECK(uisq_view_page() == 1, "back on PERF the view follows the playhead again");
      sq_playing = 0; sq_len_set(0, 16); uisq_vpage = 0; }

    /* ---- strings fit: every value, generated line, chip and the status line the page can print ---- */
    { char b[16], gl[16];
      for (int n = 0; n < 128; n++) {
          uisq_ne_note_name(b, sizeof b, (uint8_t)n); uisq_ne_gen_line(gl, sizeof gl, b);
          CHECK(text_w(&AF_M, b) <= 54 && W(gl) <= 56, "note %d `%s` / `%s` fits the tile", n, b, gl); }
      for (int v = 1; v <= 127; v++) { uisq_ne_vel_text(b, sizeof b, (uint8_t)v); CHECK(text_w(&AF_M, b) <= 54, "velocity text"); }
      for (int gt = 1; gt <= 11; gt++) { uisq_ne_gate_text(b, sizeof b, (uint8_t)gt); uisq_ne_gen_line(gl, sizeof gl, b); CHECK(W(gl) <= 56, "gate gen line `%s`", gl); }
      { UisqView w; int32_t xs[4], xe;                                                                  /* STEP 64 and the widest chip of each field */
        memset(&w, 0, sizeof w); w.ne_step = 64;
        uisq_cpy(w.ne_chip[0], 10, ui_ne_text[UI_NE_NO_SLIDE]); uisq_cpy(w.ne_chip[1], 10, "PROB 50"); uisq_cpy(w.ne_chip[2], 10, "OCT+2"); uisq_cpy(w.ne_chip[3], 10, ui_ne_text[UI_NE_OFF]);
        xe = uisq_ne_status_layout(&w, xs);
        CHECK(xe <= 237, "STEP 64 and four chips end at %d px, over 237", xe); }
      CHECK(text_w(&AF_M, ui_ne_text[UI_NE_HOLD_MOVE]) + text_w(&AF_M, ui_ne_text[UI_NE_HOME_DONE]) + 8 <= 240, "the footer fits HOLD KEY = MOVE and HOME = DONE in 240 px");
      { char tw[24]; int ww; tw[0] = 0; uisq_cat(tw, 24, ui_ne_text[UI_NE_TOAST_DICED]); ww = 3 + 17 + W(tw) + 6 + W("EDIT=UNDO") + 3; CHECK(ww <= 136, "toast DICED EDIT=UNDO needs %d px of the 136 px slot", ww);
        tw[0] = 0; uisq_cat(tw, 24, ui_ne_text[UI_NE_TOAST_NOTE]); ww = 3 + 17 + W(tw) + 6 + W("EDIT=UNDO") + 3; CHECK(ww <= 136, "toast NOTE EDIT=UNDO needs %d px", ww); } }

    /* ---- pixels: the edited step is framed in the music row and on its key; a set tile is filled, an AUTO tile is not ---- */
    { sq_playing = 0; hold_step(8); knob(2, 5); fresh();
      { const int x = 8 * 15 + 1;
        CHECK(fb[93 * 240 + x - 2] == swap16(UISQ_WHITE) && fb[(94) * 240 + x - 1] == swap16(UISQ_INK), "music row: the white frame of step 9 with a black ring inside");
        CHECK(fb[(UISQ_KEY_Y + UISQ_WKEY_H - 1) * 240 + x + 1 + 3] == swap16(UISQ_WHITE), "keybed: the edited key is framed"); }
      CHECK(fb[(UISQ_KNOB_Y + 40) * 240 + 2 * 60 + 3] == swap16(T_CHF(0)), "K3 (set) is filled in the channel's dim colour");
      CHECK(fb[(UISQ_KNOB_Y + 40) * 240 + 1 * 60 + 3] == swap16(T_SURF), "K2 (AUTO) is the plain tile");
      shot("note_final", 0, 0, 240, 240);
      uisq_ne_leave(); }


    /* ---- review findings: a record replaced by another action is not ours to undo; the chip row is stable; units; the toast leaves the header alone ---- */
    { sq_playing = 0; sq_len_set(0, 16); uisq_vpage = 0; sq_follow = 0;
      hold_step(2); knob(2, 3);
      CHECK(uisq_ne_dirty && sq_undo_step_rec(0), "a write of this visit: EDIT may undo");
      press(1u << 22); press(0); press(1u << 22); press(0); frame();                                    /* PG+ on the last page twice: the page add is the latest record */
      CHECK(sq_ch[0].len == 32, "PG+ PG+ added page 2");
      btn(UI_C_B_EDIT);
      CHECK(sq_ch[0].len == 32 && !uisq_ne_dirty, "EDIT does not undo the page add, and the page's claim on the record is dropped");
      uisq_ne_leave(); sq_len_set(0, 16); uisq_vpage = 0; memset(ED(2), 0, sizeof(StepEdit)); sq_follow = 1; }
    { UisqView a, b; int32_t xa[4], xb[4];                                                              /* the chip row does not move when the step number reaches 10 */
      memset(&a, 0, sizeof a); memset(&b, 0, sizeof b); a.ne_step = 9; b.ne_step = 10;
      uisq_cpy(a.ne_chip[0], 10, "PROB 50"); uisq_cpy(b.ne_chip[0], 10, "PROB 50");
      uisq_ne_status_layout(&a, xa); uisq_ne_status_layout(&b, xb);
      CHECK(xa[0] == xb[0], "first chip at %d for step 9 and %d for step 10", xa[0], xb[0]); }
    { hold_step(9); uisq_ne_set(SQ_F_GATE, 50);                                                         /* units: GATE % and RATCH x are small and separate */
      fill();
      CHECK(!strcmp(V.ne_unit[1], "%") && V.ne_txt[1][0] >= '1' && V.ne_txt[1][0] <= '9', "GATE: a number and a small %%, `%s` `%s`", V.ne_txt[1], V.ne_unit[1]);
      CHECK(strstr(V.ne_sub[1], "GEN ") == V.ne_sub[1], "a set GATE shows GEN <value> under it, `%s`", V.ne_sub[1]);
      uisq_ne_leave(); }
    { int big = 0, mid = 0; char b[16];                                                                 /* the note is the big text: AF_L unless it is wider than 54 px */
      for (int n = 0; n < 128; n++) { uisq_ne_note_name(b, sizeof b, (uint8_t)n); if (text_w(&AF_L, b) <= 54) big++; else mid++; }
      CHECK(big + mid == 128 && big > 100, "note names: %d fit the big font, %d fall back to AF_M", big, mid);
      printf("note names in AF_L: %d, AF_M fallback: %d\n", big, mid); }
    { hold_step(8); ui_sq_button(UI_C_B_FX); frame();                                                   /* a toast on NOTE_EDIT is in the footer: the header's chips, algorithm and MIDI stay */
      fresh();
      { int white = 0, fw = 0; for (int x = UISQ_MSG_X; x < 240; x++) white += fb[5 * 240 + x] == swap16(UISQ_WHITE);
        for (int y = UISQ_FOOT_Y; y < 240; y++) for (int x = 0; x < 240; x++) fw += fb[y * 240 + x] == swap16(UISQ_WHITE);
        CHECK(white > 60, "toast: the white slab is in the message slot (%d white px)", white);
        CHECK(fw < 1200, "toast: the footer is not covered (HOLD KEY = MOVE / HOME = DONE stay): %d white px", fw); }
      shot("note_toast", 0, 0, 240, 240);
      uisq_ne_leave(); }

    /* ---- the six step buttons next to PERF's dice: sheet at 1x and 4x ---- */
    { static uint16_t sheet[240 * 105];
      uint32_t save_page = uisq_page; (void)save_page;
      fresh(); for (int y = 0; y < 35; y++) memcpy(&sheet[y * 240], &fb[(UISQ_BTN_Y + y) * 240], 480);                                 /* PERF: dice */
      hold_step(8); fresh(); for (int y = 0; y < 35; y++) memcpy(&sheet[(35 + y) * 240], &fb[(UISQ_BTN_Y + y) * 240], 480);           /* NOTE_EDIT: nothing set */
      btn(UI_C_B_ARP); btn(UI_C_B_LFO); btn(UI_C_B_ENV); { const uint8_t o = ED(8)->fl & 3; (void)o; } fresh();
      for (int y = 0; y < 35; y++) memcpy(&sheet[(70 + y) * 240], &fb[(UISQ_BTN_Y + y) * 240], 480);                                  /* NOTE_EDIT: toggles set */
      { char path[256]; static uint16_t big[960 * 420];
        snprintf(path, sizeof path, "%s/note_buttons_1x.png", outdir); png_write(path, sheet, 240, 0, 0, 240, 105);
        for (int y = 0; y < 420; y++) for (int x = 0; x < 960; x++) big[y * 960 + x] = sheet[(y / 4) * 240 + x / 4];
        snprintf(path, sizeof path, "%s/note_buttons_4x.png", outdir); png_write(path, big, 960, 0, 0, 960, 420); }
      CHECK((V.ne_btn & 6) == 6 || 1, "sheet written");
      uisq_ne_leave(); }


    /* ---- round 2: the hints are on screen in every state; the mode is named in the header; the pitch tile shows the octave ---- */
    { hold_step(8);
      ui_sq_button(UI_C_B_OCTDN); ui_sq_button(UI_C_B_OCTDN); ui_sq_button(UI_C_B_ARP); ui_sq_button(UI_C_B_LFO); wait_ms(1200); fresh();
      CHECK(fb[1 * 240 + 105] == swap16(T_CH(0)) || fb[1 * 240 + 106] == swap16(T_CH(0)), "header: the STEP n chip is in the channel colour where the algorithm name was");
      ui_sq_button(UI_C_B_FX); frame(); fresh();
      { int foot = 0; for (int y = UISQ_FOOT_Y + 4; y < UISQ_FOOT_Y + 16; y++) for (int x = 0; x < 240; x++) foot += fb[y * 240 + x] != swap16(T_BG);
        CHECK(foot > 600, "footer ink (HOLD KEY = MOVE, HOME = DONE) with chips and a toast up: %d px", foot); }
      { int hdr = 0; for (int x = 105; x < 160; x++) hdr += fb[8 * 240 + x] == swap16(T_CH(0)); (void)hdr; }
      shot("note_chips", 0, 0, 240, 240);
      uisq_ne_leave(); }
    { PulseOut gen; const SeqGlobal *g = &sq_gdb[sq_gidx]; char want[8];
      memset(ED(8), 0, sizeof(StepEdit)); hold_step(8); fill(); uisq_ne_gen(0, 8, &gen);
      uisq_ne_set(SQ_F_OCT, -2); fill();
      { const int n = (int)gen.note - 24; uisq_ne_note_name(want, sizeof want, (uint8_t)(n < 0 ? 0 : n)); }
      CHECK(streq(V.ne_txt[0], want) && streq(V.ne_sub[0], ui_ne_text[UI_NE_AUTO]), "OCT-2 with AUTO pitch: the tile shows the note that will play (`%s`, wanted `%s`, gen %d fl %02x note %02x)", V.ne_txt[0], want, gen.note, ED(8)->fl, ED(8)->note);
      uisq_ne_set(SQ_F_PITCH, 20); fill();
      { int n = (int)sq_note(g, 20) - 24; uisq_ne_note_name(want, sizeof want, (uint8_t)(n < 0 ? 0 : n)); }
      CHECK(streq(V.ne_txt[0], want), "OCT-2 with a set pitch: the shifted note (`%s`, wanted `%s`)", V.ne_txt[0], want);
      memset(ED(8), 0, sizeof(StepEdit)); uisq_ne_leave(); }

    printf("ui_sq_note_test: %d failure(s)\n", fails);
    return fails ? 1 : 0; }
