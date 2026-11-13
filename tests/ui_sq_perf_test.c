/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_perf_test.c (P4-013): the PERF page's controls of firmware/src/ui_sq.c on the host: input sequence -> sq_* API effects and screen state
 * (message slot, view). It drives ui_sq_keys / ui_sq_button / ui_sq_knob / ui_sq_select_turn / ui_sq_input exactly as ui_sq_glue.c does and
 * checks S01 steps 5-9 (PLAY running, morph, dice, EDIT) plus FB-017 item 2 (SELECT adds a channel, M-key tap / preview / remove, white keys as
 * steps, hold = NOTE_EDIT, PG- / PG+, OCT rows). Frames go to build/ui_sq/perf_*.png.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_perf_test tests/ui_sq_perf_test.c -lm && /tmp/ui_sq_perf_test [outdir] */
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

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    sq_boot(); uisq_used = 0x0F;      /* P4-044: the core boots with CH1 only; these pages are exercised with all four in use */
    fm1_ms = 1000;
    fresh();
    shot("perf_boot", 0, 0, 240, 240);

    /* ---- key slots: the 27 keys are 16 white + 11 black in panel order (key_map.tsv) ---- */
    { static const uint8_t wk[16] = { 0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26 };
      static const uint8_t bk[11] = { 1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25 };
      for (int i = 0; i < 16; i++) CHECK(uisq_key_slot(wk[i]) == i, "white key %d is slot %d, got %d", wk[i], i, uisq_key_slot(wk[i]));
      for (int i = 0; i < 11; i++) CHECK(uisq_key_slot(bk[i]) == (0x80 | i), "black key %d is slot b%d", bk[i], i);
      CHECK(uisq_key_ctl(0) == UI_C_KEY_W01_F3 && uisq_key_ctl(26) == UI_C_KEY_W16_G5 && uisq_key_ctl(1) == UI_C_KEY_B01_FS3 && uisq_key_ctl(25) == UI_C_KEY_B11_FS5, "key ctl ids"); }

    /* ---- S01 step 5-6: the lane is up while PLAY runs; the screen cue names the key roles ---- */
    sq_playing = 1;
    fresh();
    { char cue[48]; uisq_cue_text(cue, sizeof cue); CHECK(!strcmp(cue, "TAP STEP=ON/OFF  HOLD=EDIT NOTE"), "playing cue `%s`", cue); }
    shot("perf_playing", 0, 0, 240, 240);

    /* ---- U14: a tap on a step key toggles the step; the lane redraws within one frame; a second tap returns it to AUTO ---- */
    { const int was = on_of(0, 2); StepEdit *e = &sq_ch[0].ed[2];
      press(1u << 4); frame();                                          /* F3 G3 A3: chromatic key 4 is white key 3 = step 3 (index 2) */
      CHECK(!(e->fl & 3), "nothing is written while the key is down (tap = on release)");
      wait_ms(100);
      { int b = blits; press(0); frame(); CHECK(blits > b, "the tap redraws the screen in the next frame"); }
      CHECK(on_of(0, 2) == !was, "tap toggles step 3 (%d -> %d)", was, on_of(0, 2));
      CHECK((e->fl & 3) != 0, "the tap wrote the ON field");
      tap_key(4);
      CHECK(on_of(0, 2) == was && (e->fl & 3) == 0, "a second tap that matches the generator returns it to AUTO"); }
    { const uint8_t ff = sq_ch[0].ed[0].fl; tap_key(0); CHECK(sq_ch[0].ed[0].fl != ff, "key 0 (F3) is step 1"); tap_key(0); CHECK(sq_ch[0].ed[0].fl == ff, "and back"); }

    /* ---- hold 0.4 s: NOTE_EDIT on that step, one undo record for the session (sq_note_edit_begin) ---- */
    { const uint8_t before = sq_ch[0].ed[6].fl;
      press(1u << 12); wait_ms(300);
      CHECK(uisq_page == UI_PG_PERF && sq_ne_state == 0, "0.3 s: still PERF");
      wait_ms(150);
      CHECK(uisq_page == UI_PG_NOTE_EDIT && uisq_edit_step == 7 && sq_ne_state == 1 && sq_ne_p == 7 && sq_ne_ch == 0, "0.45 s: NOTE_EDIT on step 8 (page %d step %d ne %d)", uisq_page, uisq_edit_step, sq_ne_state);
      press(0); frame();
      CHECK(sq_ch[0].ed[6].fl == before, "the hold is no tap");
      CHECK(uisq_page == UI_PG_NOTE_EDIT, "NOTE_EDIT stays open after the key is let go (P4-014)"); uisq_ne_leave(); CHECK(uisq_page == UI_PG_PERF && sq_ne_state == 0, "leaving closes the session"); }
    { const uint8_t f = sq_ch[0].ed[7].fl; CHECK((f & 3) == 0, "release after the hold wrote nothing"); }

    /* ---- a step past LENGTH: refused ---- */
    sq_playing = 0;
    CHECK(sq_len_set(0, 8) == 1 || sq_ch[0].len == 8, "length 8");
    { const uint8_t f = sq_ch[0].ed[12].fl; tap_key(14); CHECK(sq_ch[0].ed[12].fl == f, "a tap past LENGTH writes nothing"); }
    (void)sq_len_set(0, 16);

    /* ---- K1-K4: sq_morph_turn on the live row; ROW 2 via OCT+ ---- */
    { const uint8_t m0 = sq_ch[0].st[0].morph[0];
      ui_sq_knob(0, 1); frame();
      CHECK(sq_ch[0].st[0].morph[0] == m0 + 1, "K1 +1 detent: DENS %d -> %d", m0, sq_ch[0].st[0].morph[0]);
      { UisqView v; uisq_view_fill(&v); CHECK(v.val[0] == m0 + 1 && v.turn == 1, "the tile shows the value and is hot in the same frame"); }
      ui_sq_knob(0, -1);
      CHECK(sq_ch[0].st[0].morph[0] == m0, "turning back restores it");
      ui_sq_button(UI_C_B_OCTUP);
      CHECK(uisq_row == 1, "OCT+ latches ROW 2");
      { const uint8_t o = sq_ch[0].st[0].morph[4]; ui_sq_knob(0, 2); CHECK(sq_ch[0].st[0].morph[4] == o + 2, "ROW 2 K1 turns slot 4 (OFFSET)"); }
      ui_sq_button(UI_C_B_OCTUP);
      CHECK(uisq_row == 1 && uisq_toast_until && !strcmp(uisq_toast, ui_msgs[UI_MSG_TOAST_LAST_ROW_OCT_1].a), "OCT+ on the last row: toast LAST ROW");
      ui_sq_button(UI_C_B_OCTDN);
      CHECK(uisq_row == 0, "OCT- returns to ROW 1"); }

    /* ---- the dice buttons and EDIT (S01 steps 8-9) ---- */
    { const uint16_t s0 = sq_ch[0].st[0].seed[0];
      ui_sq_button(UI_C_B_SCL);
      CHECK(sq_ch[0].st[0].seed[0] != s0, "SCL dices the NOTES layer seed");
      CHECK(uisq_toast_until && !strcmp(uisq_toast, "NOTES") && !strcmp(uisq_toast_hint, "EDIT=UNDO") && uisq_toast_chip == 1, "toast NOTES  EDIT=UNDO on chip 1");
      ui_sq_button(UI_C_B_EDIT);
      CHECK(sq_ch[0].st[0].seed[0] == s0, "EDIT takes the dice back");
      CHECK(!strcmp(uisq_toast, "UNDO"), "toast UNDO");
      ui_sq_button(UI_C_B_EDIT);
      CHECK(sq_ch[0].st[0].seed[0] != s0 && !strcmp(uisq_toast, "REDO"), "EDIT again redoes it");
      { uint16_t a0 = sq_ch[0].st[0].seed[1]; ui_sq_button(UI_C_B_SEQ); CHECK(sq_ch[0].st[0].seed[1] != a0 && !strcmp(uisq_toast, "HITS"), "SEQ dices HITS"); }
      { uint16_t a0 = sq_ch[0].st[0].seed[4]; ui_sq_button(UI_C_B_ENV); CHECK(sq_ch[0].st[0].seed[4] != a0 && !strcmp(uisq_toast, "ACCNT"), "ENV dices ACCNT"); }
      { uint16_t a0 = sq_ch[0].st[0].seed[3]; ui_sq_button(UI_C_B_ARP); CHECK(sq_ch[0].st[0].seed[3] != a0 && !strcmp(uisq_toast, "ARTIC"), "ARP dices ARTIC"); }
      { uint16_t a0 = sq_ch[0].st[0].seed[2]; ui_sq_button(UI_C_B_LFO); CHECK(sq_ch[0].st[0].seed[2] != a0 && !strcmp(uisq_toast, "SHAPE"), "LFO dices SHAPE"); }
      { uint16_t a0 = sq_ch[0].st[0].seed[0]; ui_sq_button(UI_C_B_FX); CHECK(sq_ch[0].st[0].seed[0] != a0 && !strcmp(uisq_toast, "ALL"), "FX dices all layers"); }
      /* an absent layer is inert: SNAKE has no ARTIC */
      sq_set_algo(1, 2); ui_sq_select(1); uisq_toast_until = 0;
      { uint16_t sd[6]; memcpy(sd, sq_ch[1].st[2].seed, sizeof sd); ui_sq_button(UI_C_B_ARP);
        CHECK(!memcmp(sd, sq_ch[1].st[2].seed, sizeof sd) && !uisq_toast_until, "an absent layer (SNAKE ARTIC) changes nothing and says nothing"); }
      ui_sq_select(0); }
    shot("perf_dice", 0, 0, 240, 240);

    /* ---- M keys: tap mutes, hold < 1.5 s toggles the preview, hold 1.5 s removes (key B02 = chromatic key 3 = CH2) ---- */
    { const uint8_t m = sq_ch[1].muted, pv = sq_ch[1].pad[0];
      tap_key(3);
      CHECK(sq_ch[1].muted == !m && sq_ch[1].pad[0] == pv, "M2 tap toggles the mute, the preview is untouched");
      tap_key(3);
      CHECK(sq_ch[1].muted == m, "M2 tap again unmutes");
      press(1u << 3); wait_ms(600);
      CHECK(uisq_held_m == 2 && uisq_hold_on, "M2 held: the key is named and the hold hint is up");
      { UisqMsg mm; uisq_msg_pick(&mm, UISQ_MS(), UI_PG_PERF, 0); CHECK(mm.kind == UI_MK_HOLD && !strcmp(mm.a, "PREVIEW CH2") && mm.frac > 0 && mm.frac < 100, "hint PREVIEW CH2 with a bar (%s %d)", mm.a, mm.frac); }
      press(0); frame();
      CHECK(sq_ch[1].muted == m && (sq_ch[1].pad[0] ^ pv) == 0x80, "released before 1.5 s: PREVIEW toggled, mute untouched");
      CHECK(!uisq_hold_on && !uisq_held_m, "hint cleared");
      sq_ch[1].pad[0] = pv; }
    { press(1u << 3); wait_ms(1400); CHECK(uisq_used == 0x0F, "1.4 s: not yet removed");
      wait_ms(200);
      CHECK(uisq_used == 0x0D && sq_ch[1].muted, "1.5 s: CH2 removed (out of use, muted)");
      CHECK(uisq_toast_until && !strcmp(uisq_toast, "GONE") && !strcmp(uisq_toast_hint, "EDIT=UNDO") && uisq_toast_chip == 2, "toast [2] GONE  EDIT=UNDO");
      press(0); frame();
      CHECK(uisq_used == 0x0D, "the release after the remove does nothing more");
      ui_sq_button(UI_C_B_EDIT);
      CHECK(uisq_used == 0x0F && uisq_sel == 1, "EDIT takes the removal back"); }
    /* F1: a removed channel's preview is off (and comes back with the undo); F2: two removals, two undos; F3: the hint ends with the key off PERF */
    { sq_ch[1].pad[0] |= 0x80u; sq_ch[2].pad[0] |= 0x80u; uisq_gone_n = 0;
      uisq_do_remove(1); uisq_do_remove(2);
      CHECK(!(sq_ch[1].pad[0] & 0x80u) && !(sq_ch[2].pad[0] & 0x80u) && uisq_used == 0x09, "removed channels have the preview off");
      ui_sq_button(UI_C_B_EDIT); CHECK(uisq_used == 0x0D && (sq_ch[2].pad[0] & 0x80u) && !(sq_ch[1].pad[0] & 0x80u), "first undo: the newest (CH3) with its preview");
      ui_sq_button(UI_C_B_EDIT); CHECK(uisq_used == 0x0F && (sq_ch[1].pad[0] & 0x80u), "second undo: CH2 as well");
      sq_ch[1].pad[0] &= 0x7Fu; sq_ch[2].pad[0] &= 0x7Fu;
      press(1u << 3); wait_ms(300); CHECK(uisq_hold_on && uisq_held_m == 2, "M2 hint up");
      uisq_page = UI_PG_CHAN; press(0); CHECK(!uisq_hold_on && !uisq_held_m, "released on another page: the hint is cleared"); uisq_page = UI_PG_PERF; uisq_toast_until = 0; }
    ui_sq_select(0);

    /* ---- SELECT: moves among the channels in use; one detent past the last adds the next free channel ---- */
    { uisq_used = 0x03; uisq_sel = 0;
      ui_sq_select_turn(1); CHECK(uisq_sel == 1, "SELECT +1: CH2");
      CHECK(uisq_touch_a[0] && !strcmp(uisq_touch_a, "SELECT") && !strncmp(uisq_touch_b, "CH2 ", 4), "named while touched: SELECT CH2 <algo> (%s %s)", uisq_touch_a, uisq_touch_b);
      ui_sq_select_turn(1);
      CHECK(uisq_used == 0x07 && uisq_sel == 2 && (uisq_new >> 2 & 1u), "one past the last in use adds CH3, selected, new (setup cue)");
      CHECK(!strcmp(uisq_toast, "NEW") && !strcmp(uisq_toast_hint, "ALGO=PICK") && uisq_toast_chip == 3, "toast [3] NEW  ALGO=PICK");
      ui_sq_select_turn(1); ui_sq_select_turn(1);
      CHECK(uisq_used == 0x0F && uisq_sel == 3, "CH4 added; never past CH4");
      ui_sq_select_turn(1); CHECK(uisq_sel == 3 && uisq_used == 0x0F, "clamped at CH4");
      ui_sq_select_turn(-1); ui_sq_select_turn(-1); ui_sq_select_turn(-1); ui_sq_select_turn(-1);
      CHECK(uisq_sel == 0, "clamped at CH1, no wrap"); }
    uisq_used = 0x0F; uisq_new = 0;

    /* F5: remove then add, six times, then EDIT: the removal stack never overruns */
    { uisq_used = 0x0F; uisq_gone_n = 0; uisq_sel = 0;
      for (int i = 0; i < 6; i++) {
          uisq_do_remove(3); CHECK(uisq_gone_n <= SQ_NCH, "stack bounded");
          uisq_sel = 2; ui_sq_select_turn(1);
          CHECK(uisq_used == 0x0F && uisq_gone_n == 0, "add empties the removal stack (round %d)", i); }
      for (int i = 0; i < 9; i++) { uisq_do_remove((uint8_t)(i % 3 + 1)); uisq_used = 0x0F; CHECK(uisq_gone_n <= SQ_NCH, "full stack drops the oldest"); }
      ui_sq_button(UI_C_B_EDIT); CHECK(uisq_gone_n == SQ_NCH - 1 && uisq_used == 0x0F, "EDIT pops one");
      uisq_gone_n = 0; uisq_new = 0; uisq_sel = 0; uisq_vpage = 0; for (int c = 0; c < 4; c++) sq_ch[c].muted = 0; }

    /* F6: removing a channel that is not in use does nothing; F7: a removal restore / an add clears the undo-redo toggle */
    { uisq_used = 0x0D; uisq_gone_n = 0; sq_ch[1].muted = 0;
      uisq_do_remove(1); CHECK(uisq_gone_n == 0 && !sq_ch[1].muted && uisq_used == 0x0D, "F6: an unused channel is not removed");
      uisq_used = 0x0F;
      ui_sq_button(UI_C_B_SCL); ui_sq_button(UI_C_B_EDIT); CHECK(uisq_undone == 1, "an undo sets the toggle");
      uisq_do_remove(2); ui_sq_button(UI_C_B_EDIT);
      CHECK(uisq_undone == 0 && uisq_used == 0x0F, "F7: restoring a removal clears the toggle");
      ui_sq_button(UI_C_B_SCL); ui_sq_button(UI_C_B_EDIT); CHECK(!strcmp(uisq_toast, "UNDO"), "next undo says UNDO");
      uisq_used = 0x03; uisq_sel = 1; ui_sq_select_turn(1);
      CHECK(uisq_undone == 0, "F7: an add clears the toggle");
      uisq_used = 0x0F; uisq_new = 0; uisq_sel = 0; uisq_undone = 0; uisq_gone_n = 0; }

    /* ---- the last channel in use cannot be removed ---- */
    { uisq_used = 0x01; uisq_do_remove(0); CHECK(uisq_used == 0x01, "the last channel stays"); uisq_used = 0x0F; }

    /* ---- PG-/PG+: view pages, FOLLOW off; on the last page a prompt, then the add ---- */
    { sq_follow = 1;
      press(1u << 20 | 0); /* black key 9 (C#5) is slot b8 = PG-; D#5 (key 22) is slot b9 = PG+ */
      press(0);
      CHECK(sq_ch[0].len == 16 && !sq_follow && uisq_vpage == 0, "PG- on page 1: stays, FOLLOW off");
      sq_follow = 1;
      press(1u << 22); press(0);
      CHECK(sq_ch[0].len == 16 && uisq_pg_pend && !sq_follow && !strcmp(uisq_toast, "PAGE 2?") && !strcmp(uisq_toast_hint, "PG+=ADD"), "PG+ on the last page: prompt `PAGE 2?  PG+=ADD`, nothing changed (%s)", uisq_toast);
      press(1u << 22); press(0);
      CHECK(sq_ch[0].len == 32 && uisq_vpage == 1 && !uisq_pg_pend, "PG+ again adds page 2 and shows it");
      press(1u << 20); press(0);
      CHECK(uisq_vpage == 0, "PG- goes back to page 1");
      press(1u << 22); press(0);
      CHECK(uisq_vpage == 1, "PG+ shows page 2");
      CHECK(sq_undo(0) == 1 && sq_ch[0].len == 16, "the add is one undo record"); uisq_vpage = 0; }

    /* ---- P4-032: CHN opens CHAN, ALGO turns the algorithm on PERF, PAST THE END, 64 STEPS = MAX, a stopped step sounds once, preview toast, ONE PAGE ---- */
    sq_playing = 0; uisq_page = UI_PG_PERF; uisq_vpage = 0; uisq_sel = 0; uisq_used = 0x0F; uisq_gone_n = 0;
    { uint8_t o, n, v; while (ui_sq_aud_next(&o, &n, &v)) {} }                 /* an empty queue */
    tap_key(10); CHECK(uisq_page == UI_PG_CHAN, "D#4 tap on PERF opens CHAN");
    tap_key(10); CHECK(uisq_page == UI_PG_PERF, "D#4 tap on CHAN is BACK");
    { const uint8_t a0 = sq_ch[0].algo; char b[24];
      ui_sq_algo_turn(1); frame();
      CHECK(sq_ch[0].algo == a0 + 1, "PERF ALGO +1 changes sq_ch[sel].algo (%d -> %d)", a0, sq_ch[0].algo);
      CHECK(!strcmp(uisq_touch_a, "ALGO"), "the touched control is named in the same frame (%s)", uisq_touch_a);
      snprintf(b, sizeof b, "%s", uisq_touch_b); CHECK(strstr(b, "AT STEP") != NULL, "... with `AT STEP` (%s)", b);
      ui_sq_algo_turn(100); CHECK(sq_ch[0].algo == SQ_NALGO - 1, "clamped at the last algorithm (%d)", sq_ch[0].algo);
      ui_sq_algo_turn(-100); CHECK(sq_ch[0].algo == 0, "clamped at the first (%d)", sq_ch[0].algo);
      uisq_held_m = 1; ui_sq_algo_turn(1); CHECK(sq_ch[0].algo == 0, "a held M key: no algorithm change"); uisq_held_m = 0; }
    /* a step past LENGTH */
    { (void)sq_len_set(0, 8); uisq_toast_until = 0;
      { const StepEdit e = sq_ch[0].ed[10]; tap_key(18);                          /* white slot 10 = step 11, past LENGTH 8 */
        CHECK(!memcmp(&e, &sq_ch[0].ed[10], sizeof e) && !strcmp(uisq_toast, "PAST THE END") && uisq_toast_chip == 1, "a tap past LENGTH toasts PAST THE END and writes nothing (%s)", uisq_toast); }
      CHECK(!uisq_rec_aud_note, "... and does not sound");
      (void)sq_len_set(0, 16); }
    /* 64 STEPS = MAX on PG+ and on CHAN OCT+ */
    { (void)sq_len_set(0, 64); uisq_vpage = 3; uisq_toast_until = 0; uisq_pg_pend = 0;
      press(1u << 22); press(0);
      CHECK(sq_ch[0].len == 64 && !strcmp(uisq_toast, "64 STEPS = MAX") && !uisq_pg_pend, "PG+ on page 4 at LENGTH 64: toast 64 STEPS = MAX (%s)", uisq_toast);
      uisq_toast_until = 0; uisq_page = UI_PG_CHAN; ui_sq_button(UI_C_B_OCTUP);
      CHECK(sq_ch[0].len == 64 && !strcmp(uisq_toast, "64 STEPS = MAX"), "CHAN OCT+ at 64: the same toast");
      (void)sq_len_set(0, 16); uisq_vpage = 0; uisq_toast_until = 0; ui_sq_button(UI_C_B_OCTDN);
      CHECK(sq_ch[0].len == 16 && !strcmp(uisq_toast, "ONE PAGE") && !strcmp(uisq_toast_hint, "K1=LESS"), "CHAN OCT- at one page: toast ONE PAGE (%s)", uisq_toast);
      uisq_page = UI_PG_PERF; uisq_toast_until = 0; }
    /* stopped: a step turned ON sounds once (queue: one on, one off); turned OFF it is silent */
    { int ons = 0, offs = 0, p = -1, held = 0; uint8_t o, n, v, note0 = 0;
      for (int i = 0; i < 16; i++) if (!on_of(0, (uint8_t)i)) { p = i; break; }
      CHECK(p >= 0, "a step that is off exists");
      if (p >= 0) {
          static const uint8_t wk[16] = { 0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26 };
          tap_key(wk[p]); CHECK(on_of(0, (uint8_t)p) == 1, "the tap turned step %d on", p + 1);
          while (ui_sq_aud_next(&o, &n, &v)) { if (o) { ons++; note0 = n; held++; } else { offs++; held--; } }
          CHECK(ons == 1 && offs == 0 && held == 1, "stopped, step turned ON: one NOTE_ON queued (%d on %d off)", ons, offs);
          wait_ms(300);
          while (ui_sq_aud_next(&o, &n, &v)) { if (o) { ons++; held++; } else { offs++; held--; CHECK(n == note0, "the off is the on's note"); } }
          CHECK(ons == 1 && offs == 1 && held == 0 && !uisq_rec_aud_note, "... and exactly one NOTE_OFF follows (%d on %d off)", ons, offs);
          tap_key(wk[p]); CHECK(on_of(0, (uint8_t)p) == 0, "tapped off again");
          wait_ms(300);
          CHECK(!ui_sq_aud_next(&o, &n, &v), "turning a step OFF is silent");
          /* playing: the step is the scheduler's, no audition */
          sq_playing = 1; tap_key(wk[p]); wait_ms(300); sq_playing = 0;
          CHECK(!ui_sq_aud_next(&o, &n, &v), "while playing a tap queues no audition"); } }
    /* the preview toggle toasts */
    { uisq_toast_until = 0; press(1u << 3); wait_ms(600); press(0); frame();
      CHECK(!strncmp(uisq_toast, "PREVIEW ", 8) && uisq_toast_chip == 2, "M2 hold released: a preview toast on [2] (%s)", uisq_toast);
      CHECK(!strcmp(uisq_toast, (sq_ch[1].pad[0] & 0x80u) ? "PREVIEW ON" : "PREVIEW OFF"), "... naming the state it set");
      sq_ch[1].pad[0] &= 0x7Fu; uisq_toast_until = 0; }
    /* hazards: a pseudo-random key / knob run through PERF and CHAN; the invariants hold at every step */
    { uint32_t r = 12345u; uint8_t o, n, v; int ons = 0, offs = 0;
      sq_playing = 0; uisq_page = UI_PG_PERF; uisq_used = 0x0F; uisq_sel = 0; uisq_gone_n = 0;
      for (int i = 0; i < 3000; i++) {
          r = r * 1664525u + 1013904223u;
          switch ((r >> 24) % 9u) {
          case 0: tap_key((uint8_t)((r >> 8) % 27u)); break;                    /* any key (white: step / past end; black: M, CHN, PG) */
          case 1: ui_sq_algo_turn((int32_t)((r >> 8) % 5u) - 2); break;
          case 2: ui_sq_select_turn((int32_t)((r >> 8) % 3u) - 1); break;
          case 3: ui_sq_button((uint8_t)(((r >> 8) & 1u) ? UI_C_B_OCTUP : UI_C_B_OCTDN)); break;
          case 4: ui_sq_knob((uint8_t)((r >> 8) & 3u), (int32_t)((r >> 12) % 7u) - 3); break;
          case 5: (void)sq_len_set(uisq_sel % SQ_NCH, (uint8_t)(1u + (r >> 8) % 64u)); break;
          case 6: wait_ms(50); break;
          case 7: ui_sq_button(UI_C_B_EDIT); break;
          default: ui_sq_button(UI_C_B_FX); break; }
          if (uisq_page != UI_PG_PERF && uisq_page != UI_PG_CHAN && uisq_page != UI_PG_NOTE_EDIT) uisq_page = UI_PG_PERF;
          CHECK(uisq_sel < SQ_NCH && uisq_vpage < SQ_NSTEP / SQ_PAGE && uisq_gone_n <= SQ_NCH, "bounds at step %d", i);
          CHECK(sq_ch[uisq_sel % SQ_NCH].algo < SQ_NALGO && sq_ch[uisq_sel % SQ_NCH].len >= 1 && sq_ch[uisq_sel % SQ_NCH].len <= SQ_NSTEP, "channel state in range at step %d", i);
          while (ui_sq_aud_next(&o, &n, &v)) { if (o) ons++; else offs++; }
          CHECK(ons - offs >= 0 && ons - offs <= 1, "never more than one audition note held (%d on %d off)", ons, offs);
          if (fails > 20) break; }
      press(0); wait_ms(400);
      while (ui_sq_aud_next(&o, &n, &v)) { if (o) ons++; else offs++; }
      CHECK(!uisq_rec_aud_note, "no audition note left owed after the random run");
      CHECK(ons == offs && ons > 0, "every audition note-on got its note-off (%d on %d off)", ons, offs);
      uisq_page = UI_PG_PERF; uisq_used = 0x0F; uisq_sel = 0; uisq_vpage = 0; }

    /* ---- P4-044: power-on shows CH1 only; add / remove / undo in a pseudo-random order keep the in-use invariants ---- */
    { uint32_t rng = 0x4404u; int bad = 0;
      sq_boot(); uisq_page = UI_PG_PERF; uisq_sel = 0; uisq_vpage = 0; uisq_gone_n = 0; uisq_undone = 0; uisq_new = 0; uisq_reset = 0; fresh();
      CHECK(uisq_used == 0x01 && uisq_sel == 0, "power-on: one channel in use (CH1), selected");
      CHECK(sq_ch[0].muted == 0 && sq_ch[1].muted && sq_ch[2].muted && sq_ch[3].muted, "power-on: CH1 audible, the others muted");
      ui_sq_select_turn(-1); CHECK(uisq_sel == 0, "SELECT left of CH1 does nothing");
      ui_sq_select_turn(1); CHECK(uisq_used == 0x03 && uisq_sel == 1 && !sq_ch[1].muted, "SELECT right adds CH2 (its algorithm row exists), unmuted");
      uisq_do_remove(1); CHECK(uisq_used == 0x01 && sq_ch[1].muted && sq_ch[1].midi_ch == SQ_MIDI_OFF && uisq_sel == 0, "remove CH2: out of use, muted, MIDI OFF, selection back on CH1");
      uisq_do_remove(0); CHECK(uisq_used == 0x01, "the last channel in use is never removed");
      ui_sq_button(UI_C_B_EDIT); CHECK(uisq_used == 0x03 && uisq_sel == 1, "EDIT takes the removal back");
      for (int i = 0; i < 4000 && bad < 10; i++) {
          rng = rng * 1664525u + 1013904223u;
          switch ((rng >> 24) % 5u) {
          case 0: ui_sq_select_turn(1); break;
          case 1: ui_sq_select_turn(-1); break;
          case 2: uisq_do_remove((uint8_t)((rng >> 16) & 3u)); break;
          case 3: ui_sq_button(UI_C_B_EDIT); break;
          default: tap_key((int)((rng >> 8) % 27u)); break; }
          uisq_toast_until = 0;
          if (!uisq_used || (uisq_used & 0xF0u) || !(uisq_used >> uisq_sel & 1u) || uisq_gone_n > SQ_NCH) { CHECK(0, "fuzz %d: used %02X sel %u gone %u", i, uisq_used, uisq_sel, uisq_gone_n); bad++; }
          for (uint8_t c = 0; c < SQ_NCH; c++) if (!(uisq_used >> c & 1u) && (!sq_ch[c].muted || (sq_ch[c].pad[0] & 0x80u))) { CHECK(0, "fuzz %d: CH%d out of use but audible", i, c + 1); bad++; }
      }
      CHECK(!bad, "4000 random SELECT / remove / EDIT / key steps keep: a channel in use, the selection in use, the stack bounded, removed channels silent");
      sq_boot(); uisq_used = 0x0F; uisq_sel = 0; uisq_page = UI_PG_PERF; uisq_gone_n = 0; uisq_new = 0; fresh(); }

    /* ---- strings: every toast / hint word the page raised is a ui_msgs row word ---- */
    shot("perf_end", 0, 0, 240, 240);
    printf("ui_sq_perf_test: %d failure(s)\n", fails);
    return fails ? 1 : 0; }
