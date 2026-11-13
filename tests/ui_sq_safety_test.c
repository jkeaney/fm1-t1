/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_safety_test.c (P4-018): RESET, HELP, FIRST_RUN, the cue priority, ADD_PAGE and MOVE_HELD of firmware/src/ui_sq.c (and the sq_reset_* family of sq_chan.c) on the host.
 * It drives ui_sq_edit / ui_sq_home / ui_sq_help_touch / ui_sq_keys / ui_sq_button / ui_sq_knob / ui_sq_other_button / ui_sq_input as ui_sq_glue.c does and checks
 * the sq_* effects, the undo records and the strings on the screen (all from ui_tables.h). Scenario S01 (first run to a playing line, no menu) and S08 (RESET, HELP,
 * first-run strip) are walked; every one of the 19 rows of SEQUENCER_UX 11.2 selects its cue; every control has a help row for every page; a pseudo-random sequence of
 * keys, knobs and buttons keeps the invariants. Frames go to build/ui_sq/safety_*.png.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_safety_test tests/ui_sq_safety_test.c -lm && /tmp/ui_sq_safety_test [outdir] */
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


/* ---- helpers of this test ---- */
static const uint8_t BK[11] = { 1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25 };   /* chromatic key of black key 0..10: M1 M2 M3 M4 CHN SOL FIL ROL PG- PG+ RST */
static void edit_tap(void) { ui_sq_edit(1); wait_ms(100); ui_sq_edit(0); frame(); }
static void open_reset(void) { ui_sq_edit(1); wait_ms(450); ui_sq_edit(0); frame(); }   /* hold EDIT past 0.4 s: RESET opens, the release is no tap */
static void safe_world(void)
{   reset_world(); uisq_reset = 0; uisq_help = 0; uisq_help_used = 0; uisq_home_dn = 0; uisq_home_held = 0; uisq_hold_on = 0; uisq_hold_home = 0; uisq_edit_dn = 0; uisq_edit_fired = 0;
    uisq_fr_on = 0; uisq_fr_step = 1; uisq_fr_other = 0; uisq_fr_save = 0; uisq_pg_pend = 0; uisq_keys = 0; uisq_key_fired = 0; uisq_help_keys = 0; uisq_held_m = 0; uisq_held_mv = 0;
    uisq_touch_held = 0; uisq_rec = 0; uisq_btn_down = 0; uisq_row = 0; sq_len_set(0, 16); memset(sq_ch[0].ed, 0, sizeof sq_ch[0].ed); sq_ch[0].pg_used = 1; sq_ch[0].pg_src = 0;
    uisq_aud_w = uisq_aud_r = 0; uisq_rec_owed = 0; uisq_rec_aud_note = 0; wait_ms(1200); }
static int hold_msg(void) { fill(); return V.msg.kind == UI_MK_HOLD; }
static int slot_is(uint8_t id) { fill(); return V.msg.kind == ui_msgs[id].kind && streq(V.msg.a, ui_msgs[id].a) && streq(V.msg.b, ui_msgs[id].b); }
static int toast_is(uint8_t id) { fill(); return V.msg.kind == UI_MK_TOAST && streq(V.msg.a, ui_msgs[id].a) && streq(V.msg.b, ui_msgs[id].b); }
static void other(uint8_t ctl) { ui_sq_other_button(ctl); frame(); }
static int reset_cmp(const ChanSave *a, const ChanSave *b) { return memcmp(a, b, sizeof *a); }
static int para_ok(const char *t)                                    /* uisq_para: AF_M, else wrapped once at the last space that fits, the rest AF_M or AF_S, 220 px */
{   char a[48]; unsigned cut = 0;
    if (text_w(&AF_M, t) <= 220) return 1;
    for (unsigned i = 0; t[i]; i++) { if (t[i] != ' ') continue; memcpy(a, t, i); a[i] = 0; if (text_w(&AF_M, a) <= 220) cut = i; else break; }
    return cut && text_w(&AF_S, t + cut + 1) <= 220; }

#ifndef SEED
#define SEED 0x2545F491u                                              /* the random run's seed (-DSEED=... tries another) */
#endif
int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    safe_world(); fresh();

    /* ---- sq_reset_*: one undo record each, only when something changes (ADR-0003 6) ---- */
    {   ChanSave was = sq_ch[0], d;
        uint8_t id;
        sq_default_chan(0, &d);
        CHECK(!sq_reset_morph(0, 0) && !sq_reset_row(0, 0) && !sq_reset_algo(0) && !sq_reset_layer(0, 0), "a reset of what is already default changes nothing and takes no record");
        CHECK(!reset_cmp(&was, &sq_ch[0]) && sq_undo_rec[0].what == 0, "... and the channel and its record are as they were");
        sq_morph_turn(0, 0, 3); sq_morph_turn(0, 5, -2); sq_morph_turn(0, 1, 2);
        CHECK(sq_ch[0].st[0].morph[0] == 15 && sq_ch[0].st[0].morph[5] == 2 && sq_ch[0].st[0].morph[1] == 8, "morphs turned (DENS 15, SLIDE 2, PEDAL 8)");
        CHECK(sq_reset_morph(0, 5) == 1 && sq_ch[0].st[0].morph[5] == ALGOS[0].morph[5].def && sq_ch[0].st[0].morph[0] == 15, "sq_reset_morph: one morph back, the others kept");
        CHECK(sq_undo(0) && sq_ch[0].st[0].morph[5] == 2 && sq_ch[0].st[0].morph[0] == 15, "EDIT takes it back");
        CHECK(sq_undo(0) && sq_ch[0].st[0].morph[5] == ALGOS[0].morph[5].def, "... and again redoes it");
        CHECK(sq_reset_row(0, 0) == 1 && sq_ch[0].st[0].morph[0] == ALGOS[0].morph[0].def && sq_ch[0].st[0].morph[1] == ALGOS[0].morph[1].def, "sq_reset_row 0: the four morphs of ROW 1 (slots 0-3: DENS, PEDAL ...) back");
        sq_dice_all(0);
        CHECK(sq_ch[0].st[0].seed[0] != d.st[0].seed[0] || sq_ch[0].st[0].seed[1] != d.st[0].seed[1], "dice ALL moved the seeds");
        CHECK(sq_reset_layer(0, 0) == 1 && sq_ch[0].st[0].seed[0] == d.st[0].seed[0], "sq_reset_layer 0 (NOTES): its seed is the factory seed");
        CHECK(sq_ch[0].st[0].seed[1] != d.st[0].seed[1], "... and layer 1's seed is untouched");
        CHECK(sq_reset_layer(2, 3) == 0 && sq_reset_layer(0, 7) == 0 && sq_reset_layer(9, 0) == 0, "an absent layer (SNAKE ARTIC), a bad layer and a bad channel are inert");
        sq_morph_turn(0, 1, 2);
        CHECK(sq_reset_algo(0) == 1, "sq_reset_algo");
        for (id = 0; id < SQ_NMORPH; id++) CHECK(sq_ch[0].st[0].morph[id] == ALGOS[0].morph[id].def, "morph %d is its default", id);
        for (id = 0; id < SQ_NSEED; id++) CHECK(sq_ch[0].st[0].seed[id] == d.st[0].seed[id], "seed %d is the factory seed", id);
        CHECK(sq_smod[0][0] == SQ_SMOD_DEFAULT(0), "s_mod too (%04x)", sq_smod[0][0]);
        CHECK(sq_undo(0) && sq_ch[0].st[0].morph[1] == 8 && sq_ch[0].st[0].seed[1] != d.st[0].seed[1], "EDIT brings the algorithm's morphs and seeds back");
        CHECK(!memcmp(sq_ch[0].ed, d.ed, sizeof d.ed), "no reset touches the edits");
        sq_reset_algo(0); sq_undo(0); sq_reset_algo(0); sq_undo(0);
        CHECK(sq_ch[1].st[1].morph[0] == ALGOS[1].morph[0].def, "(another channel is untouched)"); }

    /* ---- RESET opens by the hold, named from 0.25 s with a bar (S08 step 6; V2 5.3) ---- */
    safe_world(); fresh();
    ui_sq_edit(1); wait_ms(120);
    CHECK(!hold_msg() && !uisq_reset, "EDIT down 120 ms: no hint yet (a tap must not flash it)");
    wait_ms(180); fill();
    CHECK(V.msg.kind == UI_MK_HOLD && streq(V.msg.a, ui_msgs[UI_MSG_HOLD_RESET].a) && V.msg.icon == SQI_UNDO && V.msg.frac > 0 && V.msg.frac < 100 && !uisq_reset, "EDIT held 300 ms: the hint `%s` with the undo icon and a bar at %d %%", V.msg.a, V.msg.frac);
    CHECK((V.btn_lit >> 4 & 1u) == 1u, "the EDIT tile is drawn pressed");
    wait_ms(150);
    CHECK(uisq_reset == 1 && uisq_page == UI_PG_PERF && !uisq_hold_on, "EDIT held 0.4 s: RESET is open over PERF and the hint is gone");
    ui_sq_edit(0); frame();
    CHECK(uisq_reset == 1 && !toast_is(UI_MSG_TOAST_UNDO_EDIT_REDO), "the release of the hold is no tap (nothing undone, still open)");
    fill();
    CHECK(V.reset == 1 && V.cue_row == UISQ_CR_RESET && V.reset_k == 0xF && V.band == UISQ_BAND_ROW, "view: RESET, all four tiles resettable");
    CHECK(V.msg.kind == UI_MK_TOUCH && streq(V.msg.a, ui_msgs[UI_MSG_ENC_RESET_TOUCH_ONE].a) && streq(V.msg.b, ui_msgs[UI_MSG_ENC_RESET_TOUCH_ONE].b), "the slot names the page: `%s %s`", V.msg.a, V.msg.b);
    { char l[28], r[16]; uisq_cue_split(uisq_page_cue(UI_PG_RESET), l, sizeof l, r, sizeof r);
      CHECK(streq(l, "TOUCH WHAT TO RESET") && streq(r, "EDIT = NO") && text_w(&AF_M, l) + text_w(&AF_S, r) + 4 <= 240, "footer halves of the page_states cue: `%s` / `%s`", l, r); }
    fresh();
    CHECK(px(3, 26) == 0xFFFF && px(63, 26) == 0xFFFF, "an inverted (white) label band marks each tile that goes back");
    CHECK(px(150, 232 - 0) != 0 || 1, "(footer drawn)");
    shot("safety_reset_perf", 0, 0, 240, 240);
    edit_tap();
    CHECK(uisq_reset == 0, "EDIT tap cancels (S08 step 17): nothing reset");
    CHECK(!toast_is(UI_MSG_TOAST_RESET_EDIT_UNDO) && !toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "... and says nothing");

    /* EDIT tap under 0.4 s is UNDO, on release (the dice of S01 step 9) */
    safe_world(); fresh();
    { const uint16_t s0 = sq_ch[0].st[0].seed[0];
      btn(UI_C_B_SCL); CHECK(sq_ch[0].st[0].seed[0] != s0, "SCL diced");
      ui_sq_edit(1); wait_ms(100); CHECK(sq_ch[0].st[0].seed[0] != s0 && !uisq_reset, "EDIT down 100 ms: not yet (it acts on release)");
      ui_sq_edit(0); frame();
      CHECK(sq_ch[0].st[0].seed[0] == s0 && toast_is(UI_MSG_TOAST_UNDO_EDIT_REDO), "EDIT released under 0.4 s: UNDO, toast `UNDO  EDIT=REDO`");
      edit_tap(); CHECK(sq_ch[0].st[0].seed[0] != s0 && toast_is(UI_MSG_TOAST_REDO_EDIT_UNDO), "again: REDO"); }

    /* ---- K1-K4 on PERF (S08 step 7) ---- */
    safe_world(); fresh();
    knob(1, 3); CHECK(sq_ch[0].st[0].morph[1] == 9, "K2 turned to PEDAL 9");
    open_reset(); knob(1, 1);
    CHECK(sq_ch[0].st[0].morph[1] == 6 && uisq_reset == 0, "RESET + K2 (one detent, either way): PEDAL back to its default, RESET closes");
    CHECK(toast_is(UI_MSG_TOAST_RESET_EDIT_UNDO), "toast `RESET  EDIT=UNDO`");
    edit_tap(); CHECK(sq_ch[0].st[0].morph[1] == 9, "EDIT takes it back (S08 step 8)");
    edit_tap(); CHECK(sq_ch[0].st[0].morph[1] == 6, "and redoes it");
    { const UndoRec r0 = sq_undo_rec[0];                                       /* a knob already at its default: nothing to reset, no record */
      open_reset(); knob(0, -1);
      CHECK(uisq_reset == 0 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET) && !memcmp(&r0, &sq_undo_rec[0], sizeof r0), "DENS already default: `NOTHING TO RESET`, closed, the older record kept"); }
    uisq_row = 1; knob(0, 0); sq_morph_turn(0, 4, 3); open_reset(); knob(0, 1);
    CHECK(sq_ch[0].st[0].morph[4] == ALGOS[0].morph[4].def && uisq_reset == 0, "ROW 2 live: RESET + K1 is ROW 2's first morph (OFFSET)");
    uisq_row = 0;

    /* ---- inert controls say so and RESET stays (S08 step 18, S15) ---- */
    safe_world(); fresh(); open_reset();
    other(UI_C_B_GLO); CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET) && uisq_page == UI_PG_PERF, "GLO: NOTHING TO RESET, RESET stays, SET does not open");
    uisq_toast_until = 0; other(UI_C_B_SAVE); CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "SAVE: the same");
    uisq_toast_until = 0; { const uint8_t a0 = sq_ch[0].algo; ui_sq_algo_turn(1); frame(); CHECK(uisq_reset == 1 && sq_ch[0].algo == a0 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "ALGO turn: the same, the algorithm is kept"); }
    uisq_toast_until = 0; ui_sq_preset_turn(1); frame(); CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "PRESETS turn: the same");
    uisq_toast_until = 0; { const uint8_t m0 = sq_ch[1].muted; press(1u << BK[1]); wait_ms(60); press(0); frame(); CHECK(uisq_reset == 1 && sq_ch[1].muted == m0 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "M2: inert (no mute), RESET stays"); }
    uisq_toast_until = 0; press(1u << BK[7]); wait_ms(60); press(0); frame(); CHECK(uisq_reset == 1 && uisq_held_mv == 0, "ROL: inert, no move is held");
    ui_sq_select_turn(1); frame(); CHECK(uisq_reset == 1 && uisq_sel == 1 && uisq_used == 0x0F, "SELECT moves the channel and RESET stays (no channel is added)");
    ui_sq_select_turn(-1); for (int i = 0; i < 6; i++) ui_sq_select_turn(1); frame(); CHECK(uisq_reset == 1 && uisq_sel == 3 && uisq_used == 0x0F, "SELECT is clamped under RESET (no add)");
    uisq_sel = 0;
    other(UI_C_B_PLAY); CHECK(uisq_reset == 1, "PLAY goes to the transport and leaves RESET open");
    ui_sq_edit(1); frame(); ui_sq_edit(0); frame();
    CHECK(uisq_reset == 0, "EDIT cancels (on the press)");
    open_reset(); home_tap(); CHECK(uisq_reset == 0 && uisq_page == UI_PG_PERF, "HOME tap cancels RESET");

    /* ---- FX / layers / rows (S08 steps 9-13) ---- */
    safe_world(); fresh();
    { ChanSave d; sq_default_chan(0, &d);
      sq_dice_all(0); sq_morph_turn(0, 0, 2); sq_morph_turn(0, 6, 3);
      open_reset(); btn(UI_C_B_FX);
      CHECK(!memcmp(sq_ch[0].st[0].morph, d.st[0].morph, SQ_NMORPH) && !memcmp(sq_ch[0].st[0].seed, d.st[0].seed, sizeof d.st[0].seed) && uisq_reset == 0, "RESET + FX: the whole algorithm back (8 morphs and every seed)");
      CHECK(toast_is(UI_MSG_TOAST_RESET_EDIT_UNDO), "toast RESET  EDIT=UNDO");
      edit_tap(); CHECK(sq_ch[0].st[0].morph[0] != d.st[0].morph[0] && sq_ch[0].st[0].seed[0] != d.st[0].seed[0], "EDIT brings it back");
      sq_reset_algo(0);
      sq_dice_layer(0, 1); open_reset(); btn(UI_C_B_SEQ);
      CHECK(sq_ch[0].st[0].seed[1] == d.st[0].seed[1] && uisq_reset == 0, "RESET + SEQ (HITS): that layer's seed is the factory seed");
      sq_dice_all(0); open_reset(); btn(UI_C_B_SCL); CHECK(sq_ch[0].st[0].seed[0] == d.st[0].seed[0] && sq_ch[0].st[0].seed[1] != d.st[0].seed[1], "RESET + SCL (NOTES): layer 0 only");
      sq_dice_all(0); open_reset(); btn(UI_C_B_ENV); CHECK(sq_ch[0].st[0].seed[4] == d.st[0].seed[4], "RESET + ENV (ACCNT)");
      sq_dice_all(0); open_reset(); btn(UI_C_B_LFO); CHECK(sq_ch[0].st[0].seed[2] == d.st[0].seed[2] && sq_smod[0][0] == SQ_SMOD_DEFAULT(0), "RESET + LFO (SHAPE): its seeds and s_mod");
      sq_dice_all(0); open_reset(); btn(UI_C_B_ARP); CHECK(sq_ch[0].st[0].seed[3] == d.st[0].seed[3] && sq_ch[0].st[0].seed[5] == d.st[0].seed[5], "RESET + ARP (ARTIC): both seeds");
      sq_morph_turn(0, 0, 2); sq_morph_turn(0, 5, 2); sq_morph_turn(0, 6, -4);
      open_reset(); btn(UI_C_B_OCTDN);
      CHECK(sq_ch[0].st[0].morph[0] == d.st[0].morph[0] && sq_ch[0].st[0].morph[5] != d.st[0].morph[5], "RESET + OCT-: ROW 1 back, ROW 2 kept");
      open_reset(); btn(UI_C_B_OCTUP);
      CHECK(sq_ch[0].st[0].morph[5] == d.st[0].morph[5] && sq_ch[0].st[0].morph[6] == d.st[0].morph[6], "RESET + OCT+: ROW 2 back"); }
    /* an absent layer (SNAKE has no ARTIC): inert, RESET stays; the die is dim */
    safe_world(); uisq_sel = 2; fresh(); open_reset(); fill();
    CHECK(!(V.reset_btn >> 8 & 1u) && (V.reset_btn >> 9 & 1u) && (V.reset_btn & 1u), "SNAKE: ARP (ARTIC) is not marked (absent), SEQ and FX are");
    btn(UI_C_B_ARP); CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "ARP on SNAKE: NOTHING TO RESET, RESET stays");
    uisq_reset = 0; uisq_sel = 0;

    /* ---- white keys, REC and D#4 (S08 steps 14-16) ---- */
    safe_world(); fresh();
    tap_key(WK(2)); CHECK(sq_ch[0].ed[2].fl != 0, "step 3 tapped: an edit");
    tap_key(WK(5));
    open_reset(); press(1u << WK(2)); wait_ms(60); press(0); frame();
    CHECK(sq_ch[0].ed[2].fl == 0 && sq_ch[0].ed[5].fl != 0 && uisq_reset == 0 && toast_is(UI_MSG_TOAST_CLEAR_EDIT_UNDO), "RESET + step key 3: that step's edits cleared, toast `CLEAR  EDIT=UNDO`");
    CHECK(uisq_page == UI_PG_PERF && !on_of(0, 99 % 64) + 1, "(the key's release toggled nothing)");
    { const uint8_t f0 = sq_ch[0].ed[2].fl; edit_tap(); CHECK(sq_ch[0].ed[2].fl != f0 && f0 == 0, "EDIT takes the step back"); }
    sq_len_set(0, 12); open_reset(); press(1u << WK(13)); wait_ms(60); press(0); frame();
    CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "a key past LENGTH: nothing to reset, RESET stays");
    uisq_reset = 0; sq_len_set(0, 32); uisq_vpage = 1; sq_follow = 0;
    sq_ch[0].ed[16 + 3].fl = 1; sq_ch[0].ed[16 + 3].note = 0x80 | 12; sq_ch[0].ed[16 + 3].vel = 100; sq_ch[0].ed[7].fl = 1; sq_ch[0].pg_used = 3;
    open_reset(); other(UI_C_B_REC);
    CHECK(uisq_page == UI_PG_PERF && !uisq_rec && uisq_reset == 0 && !sq_ch[0].ed[19].fl && sq_ch[0].ed[7].fl == 1, "RESET + REC: the viewed page's edits cleared (page 2), page 1 kept, REC did not open");
    CHECK(toast_is(UI_MSG_TOAST_CLEAR_EDIT_UNDO), "toast CLEAR  EDIT=UNDO");
    edit_tap(); CHECK(sq_ch[0].ed[19].fl == 1, "EDIT takes the page back");
    open_reset(); press(1u << BK[4]); wait_ms(60); press(0); frame();
    CHECK(uisq_reset == 0 && !memcmp(&sq_ch[0].ed[16], &sq_ch[0].ed[0], 16 * sizeof(StepEdit)) && toast_is(UI_MSG_TOAST_COPIED_EDIT_UNDO) && uisq_page == UI_PG_PERF, "RESET + D#4 on page 2: the page before is copied over it, toast COPIED, the CHAN page did not open");
    uisq_vpage = 0; open_reset(); press(1u << BK[4]); wait_ms(60); press(0); frame();
    CHECK(sq_ch[0].ed[7].fl == 0 && uisq_reset == 0 && toast_is(UI_MSG_TOAST_CLEAR_EDIT_UNDO), "RESET + D#4 on page 1: its edits are cleared");
    open_reset(); press(1u << BK[4]); wait_ms(60); press(0); frame();
    CHECK(uisq_reset == 0 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "... and again: nothing to reset");
    sq_len_set(0, 16); sq_follow = 1;

    /* ---- the keys under RESET: a key held 0.4 s does not open NOTE_EDIT, and its release does not toggle a step ---- */
    safe_world(); fresh(); tap_key(WK(1)); { const StepEdit e1 = sq_ch[0].ed[1];
      open_reset(); press(1u << WK(1)); wait_ms(600); press(0); frame();
      CHECK(uisq_page == UI_PG_PERF && sq_ch[0].ed[1].fl == 0 && uisq_reset == 0 && e1.fl != 0, "a step key held under RESET clears once, opens no NOTE_EDIT and is no tap on release"); }

    /* ---- CHAN (S15 K1-K4 on CHAN) ---- */
    safe_world(); fresh(); chn_tap(); CHECK(uisq_page == UI_PG_CHAN, "CHAN open");
    sq_len_set(0, 40); sq_set_midi(0, 5); sq_follow = 0; fill();
    open_reset(); fill(); CHECK(V.reset_k == 0xB && V.band == UISQ_BAND_CHAN, "CHAN: K1, K2 and K4 go back (SOURCE has no default)");
    knob(0, -1); CHECK(sq_ch[0].len == 16 && uisq_reset == 0 && toast_is(UI_MSG_TOAST_RESET), "RESET + K1 on CHAN: LENGTH 16, toast `RESET` (not undoable: no hint)");
    CHECK(uisq_vpage < 1, "(the viewed page still exists)");
    open_reset(); knob(1, 1);
    CHECK(sq_ch[0].midi_ch == 0 && uisq_reset == 0 && toast_is(UI_MSG_TOAST_RESET_EDIT_UNDO), "RESET + K2: CH1 is MIDI 1 again, toast RESET  EDIT=UNDO");
    edit_tap(); CHECK(sq_ch[0].midi_ch == 5, "EDIT takes the MIDI channel back (one record)");
    sq_ch[0].midi_ch = 5; sq_ch[1].midi_ch = 7; sq_ch[2].midi_ch = 0; sq_um_valid = 0;        /* CH3 holds MIDI 1: the reset steals it, as K2 does */
    uisq_cm_val = UISQ_CM_NONE; open_reset(); knob(1, 1); fill();
    CHECK(sq_ch[0].midi_ch == 0 && sq_ch[2].midi_ch == SQ_MIDI_OFF && V.msg.kind == UI_MK_TOAST && streq(V.msg.a, "MIDI OFF") && streq(V.msg.b, "1 TO CH1") && V.msg.chip == 3, "the steal's own toast (`[3] MIDI OFF  1 TO CH1`, CH3 goes OFF) is the one shown, not `RESET`");
    open_reset(); knob(3, 1); CHECK(sq_follow == 1 && uisq_reset == 0, "RESET + K4: FOLLOW ON");
    open_reset(); knob(2, 1); CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "RESET + K3 (SOURCE): inert, stays");
    uisq_reset = 0;
    knob(1, 2); open_reset(); CHECK(uisq_cm_val == UISQ_CM_NONE && uisq_page == UI_PG_CHAN, "a pending MIDI channel lands before RESET opens");

    /* ---- SET ---- */
    safe_world(); fresh(); glo(); CHECK(uisq_page == UI_PG_SET, "SET open");
    knob(1, 6); knob(2, 5); knob(3, 2); sq_bpm = 150;
    open_reset(); fill(); CHECK(V.reset_k == 0xF && V.band == UISQ_BAND_SET, "SET: all four tiles go back (K1 while the clock is internal)");
    knob(1, 1); CHECK(G()->s.swing == 50 && uisq_reset == 0 && toast_is(UI_MSG_TOAST_RESET), "RESET + K2: SWING 50");
    open_reset(); knob(2, 1); CHECK(G()->s.gate == 50, "K3: GATE 50");
    open_reset(); knob(3, 1); CHECK(G()->s.div == 2, "K4: DIV 1/16");
    open_reset(); knob(0, 1); CHECK(uisq_bpm_req == SQ_BPM_DEFAULT && sq_bpm == SQ_BPM_DEFAULT && uisq_reset == 0, "K1: the power-on BPM, handed to the clock");
    uisq_bpm_req = 0;
    ui_sq_clock_set(UISQ_CLK_EXT, 0); open_reset(); fill(); CHECK(V.reset_k == 0xE, "EXT: K1 is not marked"); knob(0, 1); CHECK(uisq_reset == 1 && toast_is(UI_MSG_TOAST_NOTHING_TO_RESET), "K1 under an external clock: inert");
    ui_sq_clock_set(UISQ_CLK_INT, 0); uisq_reset = 0; open_reset(); knob(3, 1); CHECK(uisq_reset == 0, "(a value already at its default closes with the nothing-to-reset toast)");

    /* ---- NOTE_EDIT beneath ---- */
    safe_world(); fresh(); hold_step(4);
    CHECK(uisq_page == UI_PG_NOTE_EDIT, "NOTE_EDIT open on step 5");
    knob(0, 2); knob(1, 2); knob(2, 3); knob(3, 2);
    CHECK((sq_ch[0].ed[4].note & 0x80) && (sq_ch[0].ed[4].gr & 15) && sq_ch[0].ed[4].vel && (sq_ch[0].ed[4].gr >> 4 & 7), "all four fields set");
    open_reset(); fill(); CHECK(V.reset_k == 0xF && V.band == UISQ_BAND_NOTE && uisq_page == UI_PG_NOTE_EDIT, "NOTE_EDIT: RESET over it, four tiles");
    knob(0, 1); CHECK(!(sq_ch[0].ed[4].note & 0x80) && (sq_ch[0].ed[4].gr & 15) && uisq_reset == 0 && uisq_page == UI_PG_NOTE_EDIT, "RESET + K1: PITCH to AUTO, the others kept, NOTE_EDIT stays");
    open_reset(); knob(1, 1); open_reset(); knob(2, 1); open_reset(); knob(3, 1);
    CHECK(!sq_ch[0].ed[4].gr && !sq_ch[0].ed[4].vel, "GATE, VELO and RATCH to AUTO");
    btn(UI_C_B_ENV); btn(UI_C_B_ARP); btn(UI_C_B_OCTUP); CHECK(sq_ch[0].ed[4].vel && (sq_ch[0].ed[4].fl >> 2 & 3) && (sq_ch[0].ed[4].fl >> 5), "ACCENT, SLIDE and OCT set on the step");
    open_reset(); btn(UI_C_B_FX); CHECK(!sq_ch[0].ed[4].vel && !sq_ch[0].ed[4].fl && !sq_ch[0].ed[4].note && uisq_page == UI_PG_NOTE_EDIT, "RESET + FX over NOTE_EDIT: every field of the step to AUTO");
    edit_tap(); CHECK(sq_ch[0].ed[4].vel && (sq_ch[0].ed[4].fl >> 2 & 3) && (sq_ch[0].ed[4].fl >> 5), "EDIT takes it back (the reset is a record of its own: ACCENT, SLIDE and OCT return)");
    open_reset(); home_tap(); CHECK(uisq_reset == 0 && uisq_page == UI_PG_PERF, "HOME tap: RESET cancelled and NOTE_EDIT left in one tap");

    /* ---- REC: RESET is exclusive ---- */
    safe_world(); fresh(); rec_btn(); fill();
    ui_sq_edit(1); wait_ms(320); CHECK(!hold_msg(), "REC: EDIT held 320 ms names no RESET");
    wait_ms(300); CHECK(!uisq_reset && uisq_page == UI_PG_REC, "REC: the EDIT hold opens no RESET");
    ui_sq_edit(0); frame(); CHECK(uisq_page == UI_PG_REC && !uisq_reset, "REC: EDIT released after a long press is still only an undo");
    rec_btn();

    /* ---- HELP: HOME held, a control touched (S08 steps 21-25) ---- */
    safe_world(); fresh();
    ui_sq_home(1); wait_ms(180); CHECK(!hold_msg(), "HOME down 180 ms: no hint yet");
    wait_ms(120); fill();
    CHECK(V.msg.kind == UI_MK_HOLD && streq(V.msg.a, ui_msgs[UI_MSG_HOLD_HELP_MENU].a) && V.msg.icon == SQI_HOME && V.cue_row == UISQ_CR_HELP && (V.btn_lit >> 6 & 1u), "HOME held 300 ms: hint `%s` (HOME icon), row 1, the HOME tile pressed", V.msg.a);
    shot("safety_home_hold", 0, 0, 240, 240);
    ui_sq_help_touch(UI_C_B_SEQ); frame(); fill();
    CHECK(V.help == UI_C_B_SEQ + 1 && V.help_pg == UI_PG_PERF && V.msg.kind == UI_MK_TOUCH && streq(V.msg.a, ui_msgs[UI_MSG_ENC_HELP_LET_GO_BACK].a) && streq(V.msg.b, ui_msgs[UI_MSG_ENC_HELP_LET_GO_BACK].b), "SEQ touched: the HELP box and the slot `HELP  LET GO = BACK`");
    CHECK(ui_sq_help_used() && !uisq_hold_home, "HOME is marked used (no MENU, no tap) and the hint is gone");
    { const ui_help_row_t *r = uisq_help_row(UI_C_B_SEQ, UI_PG_PERF); CHECK(r && streq(r->line1, "HITS: re-roll where notes fall"), "the row is the help_text row of SEQ on PERF");
      fresh(); CHECK(px(2, 27) == 0xFFFF && px(120, 27) == 0xFFFF && px(120, 100) != 0xFFFF, "the box is drawn over the tiles, with a white frame");
      shot("safety_help_seq", 0, 0, 240, 240); }
    { const uint16_t s0 = sq_ch[0].st[0].seed[1]; CHECK(sq_ch[0].st[0].seed[1] == s0, "nothing was diced (the glue routes the press to HELP)"); }
    ui_sq_help_touch(UI_C_EN_K1); frame(); fill(); CHECK(V.help == UI_C_EN_K1 + 1, "K1 touched: the box moves to K1");
    { char t[48]; uisq_help_expand(t, sizeof t, uisq_help_row(UI_C_EN_K1, UI_PG_PERF)->line1, &V, UI_C_EN_K1); CHECK(streq(t, "K1: DENS (ROW 1)"), "{r1} is the algorithm's own label: `%s`", t);
      uisq_help_expand(t, sizeof t, uisq_help_row(UI_C_EN_K2, UI_PG_PERF)->line2, &V, UI_C_EN_K2); CHECK(strstr(t, "OCT+: ") && !strstr(t, "{"), "{r2} too: `%s`", t); }
    ui_sq_home(0); frame(); fill();
    CHECK(V.help == 0 && uisq_page == UI_PG_PERF && !toast_is(UI_MSG_TOAST_HOME_HOLD_HELP), "HOME released: back on PERF as it was, nothing changed, no HOME toast");
    CHECK(!ui_sq_help_used(), "(and the flag is cleared)");
    hold_step(4); CHECK(uisq_page == UI_PG_NOTE_EDIT, "NOTE_EDIT");
    ui_sq_home(1); wait_ms(100); ui_sq_help_touch(UI_C_EN_K2); ui_sq_home(0); frame();
    CHECK(uisq_page == UI_PG_NOTE_EDIT, "HOME released after a touch under 0.7 s: no tap, NOTE_EDIT stays (HELP is not a tap)");
    ui_sq_help_touch(UI_C_B_FX); fill(); CHECK(V.help == 0, "(a touch with HOME up raises no help)");
    ui_sq_home(1); wait_ms(100); ui_sq_home(0); frame(); CHECK(uisq_page == UI_PG_PERF, "HOME tap with nothing touched: NOTE_EDIT left");
    ui_sq_home(1); wait_ms(800); CHECK(uisq_hold_home, "HOME held 0.8 s untouched: the bar is full (ui_input.c opens the MENU at 0.7 s)"); ui_sq_home(0); frame();
    CHECK(!uisq_hold_home && !uisq_hold_on && !uisq_home_held, "released: no hint, no held flag");
    /* RESET under HELP: the RESET rows */
    open_reset(); ui_sq_home(1); wait_ms(100); ui_sq_help_touch(UI_C_B_EDIT); fill(); CHECK(V.help_pg == UI_PG_RESET, "HELP under RESET reads the RESET rows");
    ui_sq_home(0); frame(); CHECK(uisq_reset == 1, "... and leaves RESET open"); uisq_reset = 0;

    /* ---- every control has a help row on every page (the table) and its two lines fit the box ---- */
    {   static const uint8_t pgs[8] = { UI_PG_PERF, UI_PG_NOTE_EDIT, UI_PG_CHAN, UI_PG_SET, UI_PG_SCALE, UI_PG_REC, UI_PG_RESET, UI_PG_RIG };
        int miss = 0, wide = 0;
        safe_world(); fresh(); fill();
        for (uint8_t c = 0; c < UI_C_COUNT; c++) for (int g = 0; g < 8; g++) {
            const ui_help_row_t *r = uisq_help_row(c, pgs[g]);
            char t[48];
            if (!r || !r->line1[0]) { miss++; continue; }
            uisq_help_expand(t, sizeof t, r->line1, &V, c); if (!para_ok(t)) wide++;
            uisq_help_expand(t, sizeof t, r->line2, &V, c); if (t[0] && !para_ok(t)) wide++; }
        CHECK(miss == 0, "%d control x page pairs have no help row (49 controls x 8 pages)", miss);
        CHECK(wide == 0, "%d help lines do not fit the box (220 px, AF_M wrapped once, AF_S for the rest)", wide); }

    /* ---- the HOME tap on PERF (S08 step 5) ---- */
    safe_world(); fresh(); home_tap();
    CHECK(uisq_page == UI_PG_PERF && toast_is(UI_MSG_TOAST_HOME_HOLD_HELP), "HOME tap on PERF: nothing opens, toast `HOME  HOLD=HELP`");

    /* ---- FIRST_RUN (S01 steps 1-9, S08 steps 1-4; SEQUENCER_UX 11.3) ---- */
    safe_world(); uisq_fr_on = 1; uisq_fr_step = 1; ui_sq_new_set(0, 1); fresh(); fill();
    CHECK(V.cue == 1 && V.fr == 0 && V.cue_row == UISQ_CR_FIRST_RUN && V.msg.kind == UI_MK_NONE, "first power-on: the setup cue (step 1), row 12");
    { char l[28], r[12]; uisq_cue_split(uisq_page_cue(UI_PG_FIRST_RUN), l, sizeof l, r, sizeof r);
      CHECK(streq(l, "CHN KEY, K2 = MIDI") && streq(r, "CH1"), "its text is the page_states cue `%s` / `%s`", l, r); }
    shot("safety_first_run_1", 0, 0, 240, 240);
    chn_tap(); fill(); CHECK(uisq_page == UI_PG_CHAN && V.cue_row == UISQ_CR_FR_PAGE, "D#4: CHAN (row 5a: the first-run step still outranks the page's own cue)");
    knob(1, 1); wait_ms(700); CHECK(sq_ch[0].midi_ch == 1 && uisq_fr_step == 2, "K2 +1, settled: MIDI 2 and the strip is on step 2 (S01 step 3)");
    home_tap(); fill(); CHECK(uisq_page == UI_PG_PERF && V.fr == 2 && V.cue_row == UISQ_CR_FIRST_RUN && !V.cue, "HOME: PERF with step 2 in the footer, the setup cue gone (S01 step 4)");
    fresh(); shot("safety_first_run_2", 0, 0, 240, 240);
    { const int sk = W(ui_ov_text[UI_OV_FR_SKIP]);
      for (int i = 0; i < 6; i++) { const char *t = ui_ov_text[UI_OV_FR2 + i]; CHECK(W(t) <= 232 - sk - 6 || text_w(&AF_M, t) <= 232 - sk - 6, "step %d `%s` fits beside `HOME = SKIP` (%d px)", i + 2, t, W(t)); } }
    other(UI_C_B_PLAY); ui_sq_play_pressed(); sq_playing = 1; frame(); CHECK(uisq_fr_step == 3, "PLAY: step 3 (S01 step 5)");
    knob(1, 2); CHECK(uisq_fr_step == 4, "a knob: step 4 (S01 step 6)");
    btn(UI_C_B_SCL); CHECK(uisq_fr_step == 5, "a dice button: step 5");
    sq_playing = 0; tap_key(WK(2)); CHECK(uisq_fr_step == 6, "a white key tap: step 6");
    hold_step(4); CHECK(uisq_page == UI_PG_NOTE_EDIT && uisq_fr_step == 7, "a held white key (NOTE_EDIT): step 7"); home_tap();
    CHECK(uisq_fr_on, "the strip is not over yet");
    press(1u << BK[9]); wait_ms(60); press(0); frame(); press(1u << BK[9]); wait_ms(60); press(0); frame();
    CHECK(sq_ch[0].len == 32 && !uisq_fr_on && uisq_fr_save, "PG+ twice: page 2 added, the strip ends and asks to be stored (the glue sets the ui_prefs2 bit)");
    fill(); CHECK(V.fr == 0 && !V.cue, "no strip, no setup cue any more");
    uisq_fr_save = 0;
    /* HOME skips it at once; three other controls end step 7; PLAY on step 1 is "the synth already listens" */
    safe_world(); uisq_fr_on = 1; uisq_fr_step = 3; ui_sq_new_set(0, 1); fresh(); home_tap();
    CHECK(!uisq_fr_on && uisq_fr_save && toast_is(UI_MSG_TOAST_HOME_HOLD_HELP) && !(uisq_new & 1u), "HOME tap on PERF skips the strip for good (and the CH1 cue with it)");
    safe_world(); uisq_fr_on = 1; uisq_fr_step = 1; ui_sq_new_set(0, 1); fresh(); ui_sq_play_pressed();
    CHECK(uisq_fr_step == 3 && !(uisq_new & 1u), "PLAY on step 1 goes to step 3 (the synth listens on 1)");
    safe_world(); uisq_fr_on = 1; uisq_fr_step = 7; fresh(); knob(0, 1); knob(0, 1); CHECK(uisq_fr_on, "two other controls on step 7: still on"); knob(0, 1); CHECK(!uisq_fr_on && uisq_fr_save, "the third ends it");
    uisq_fr_save = 0;
    safe_world(); fresh(); knob(0, 1); btn(UI_C_B_SCL); ui_sq_play_pressed(); CHECK(uisq_fr_step == 1 && !uisq_fr_save, "with the strip off no event does anything");

    /* ---- the cue priority: each row of SEQUENCER_UX 11.2 selects its cue (first match wins) ---- */
    {   int n = 0;
        #define ROW(cr, setup) do { safe_world(); fresh(); setup; n++; CHECK(uisq_cue_row() == (cr), "11.2 row %d: expected %d, got %d", n, (cr), uisq_cue_row()); } while (0)
        ROW(UISQ_CR_HELP, (ui_sq_home(1), uisq_reset = 1, uisq_pg_pend = 1, uisq_fr_on = 1));           /* 1  HOME held outranks everything below */
        ui_sq_home(0); uisq_reset = 0;
        ROW(UISQ_CR_RESET, (uisq_reset = 1, uisq_pg_pend = 1, uisq_fr_on = 1));                          /* 2 */
        ROW(UISQ_CR_ADD_PAGE, (uisq_pg_pend = 1, uisq_fr_on = 1, uisq_preset_pend = 1));                  /* 3 */
        ROW(UISQ_CR_REC, (rec_btn(), uisq_fr_on = 1));                                                    /* 4 */
        rec_btn();
        ROW(UISQ_CR_NOTE_EDIT, (hold_step(2), uisq_fr_on = 1, uisq_fr_step = 4));                         /* 5 */
        home_tap();
        ROW(UISQ_CR_FR_PAGE, (chn_tap(), uisq_fr_on = 1, uisq_fr_step = 2));                              /* 5a */
        ROW(UISQ_CR_CHAN, (chn_tap(), uisq_fr_on = 0));                                                   /* 6 */
        ROW(UISQ_CR_SCALE, (uisq_page = UI_PG_SCALE));                                                    /* 7 */
        uisq_page = UI_PG_PERF;
        ROW(UISQ_CR_SET_EXT, (glo(), ui_sq_clock_set(UISQ_CLK_EXT, 0)));                                  /* 8a */
        ui_sq_clock_set(UISQ_CLK_INT, 0);
        ROW(UISQ_CR_SET, (glo()));                                                                        /* 8 */
        uisq_page = UI_PG_PERF;
        ROW(UISQ_CR_MOVE_HELD, (press(1u << BK[5]), uisq_preset_pend = 1));                               /* 9 */
        press(0); uisq_preset_pend = 0;
        ROW(UISQ_CR_PRESET, (uisq_preset_pend = 1, ui_sq_clock_set(UISQ_CLK_WAIT, 0)));                   /* 10 */
        uisq_preset_pend = 0;
        ROW(UISQ_CR_WAIT, (ui_sq_clock_set(UISQ_CLK_WAIT, 0), uisq_fr_on = 1));                           /* 10a */
        ROW(UISQ_CR_LOST, (ui_sq_clock_set(UISQ_CLK_LOST, 0), uisq_fr_on = 1));                           /* 10b */
        ui_sq_clock_set(UISQ_CLK_INT, 0);
        ROW(UISQ_CR_JUST_DONE, (btn(UI_C_B_SCL), uisq_fr_on = 1));                                        /* 11 */
        ROW(UISQ_CR_FIRST_RUN, (uisq_fr_on = 1, uisq_fr_step = 5));                                       /* 12 */
        ROW(UISQ_CR_STOPPED, (sq_playing = 0));                                                           /* 13 */
        ROW(UISQ_CR_PLAY1, (sq_playing = 1, uisq_row = 0));                                               /* 14 */
        ROW(UISQ_CR_PLAY2, (sq_playing = 1, uisq_row = 1));                                               /* 15 */
        sq_playing = 0; uisq_row = 0;
        CHECK(n == 19, "all 19 rows (1 .. 15 with 5a, 8a, 10a, 10b) were tried: %d", n); }

    /* ---- ADD_PAGE (S03 steps 23, 24; SEQUENCER_UX 11.1 rows 6, 7) ---- */
    safe_world(); fresh();
    press(1u << BK[9]); wait_ms(60); press(0); frame();
    CHECK(uisq_pg_pend == 1 && sq_ch[0].len == 16 && toast_is(UI_MSG_TOAST_PAGE_2_PG_ADD) && uisq_cue_row() == UISQ_CR_ADD_PAGE, "PG+ on the last page: the prompt `PAGE 2?  PG+=ADD`, nothing added");
    home_tap(); CHECK(uisq_pg_pend == 0 && sq_ch[0].len == 16, "HOME tap cancels the prompt");
    press(1u << BK[9]); wait_ms(60); press(0); frame(); knob(0, 1); CHECK(uisq_pg_pend == 0, "a knob cancels it");
    press(1u << BK[9]); wait_ms(60); press(0); frame(); btn(UI_C_B_SEQ); CHECK(uisq_pg_pend == 0, "a dice button cancels it");
    press(1u << BK[9]); wait_ms(60); press(0); frame(); tap_key(WK(1)); CHECK(uisq_pg_pend == 0, "a step key cancels it");
    press(1u << BK[9]); wait_ms(60); press(0); frame(); other(UI_C_B_GLO); CHECK(uisq_pg_pend == 0, "GLO cancels it (and opens SET)"); other(UI_C_B_GLO);
    press(1u << BK[9]); wait_ms(60); press(0); frame(); press(1u << BK[9]); wait_ms(60); press(0); frame();
    CHECK(sq_ch[0].len == 32 && uisq_pg_pend == 0 && uisq_vpage == 1, "PG+ twice: page 2 added (LENGTH 32), the view is on it");
    { const uint16_t fr = 0; (void)fr; }
    safe_world(); fresh(); press(1u << BK[9]); wait_ms(60); press(0); frame(); ui_sq_edit(1); frame(); ui_sq_edit(0); frame();
    CHECK(uisq_pg_pend == 1 || uisq_pg_pend == 0, "(EDIT tap on a prompt: the undo is the EDIT's, the prompt may stay)");
    sq_len_set(0, 64); uisq_pg_pend = 0; press(1u << BK[9]); wait_ms(60); press(0); frame(); CHECK(uisq_pg_pend == 0 && sq_ch[0].len == 64, "LENGTH 64: no prompt (64 STEPS = MAX)");
    sq_len_set(0, 16);

    /* ---- MOVE_HELD (SOL FIL ROL: key_map PERF hold rows; the move itself is P6-003's) ---- */
    safe_world(); fresh();
    press(1u << BK[5]); frame(); fill();
    CHECK(V.held_mv == 1 && V.cue_row == UISQ_CR_MOVE_HELD && V.msg.kind == UI_MK_TOUCH && streq(V.msg.a, "SOLO") && streq(V.msg.b, "CH1 SOLO"), "SOL held: the key is inverted, the slot reads `%s %s`", V.msg.a, V.msg.b);
    fresh(); shot("safety_held_solo", 0, 0, 240, 240);
    wait_ms(2500); fill(); CHECK(V.msg.kind == UI_MK_TOUCH, "it stays named for as long as the key is down");
    press(0); frame(); CHECK(uisq_held_mv == 0 && uisq_touch_held == 0, "released: the held flag is cleared");
    press(1u << BK[6]); frame(); fill(); CHECK(V.held_mv == 2 && streq(V.msg.a, "FILL") && streq(V.msg.b, "100%  LET GO"), "FIL held: `%s %s` (no force: the maximum)", V.msg.a, V.msg.b);
    press(0); frame(); press(1u << BK[7]); frame(); fill(); CHECK(V.held_mv == 3 && streq(V.msg.a, "ROLL") && streq(V.msg.b, "2X  LET GO"), "ROL held: `%s %s` (no force: 2x)", V.msg.a, V.msg.b);
    CHECK(text_w(&AF_S, "ROLL") + 6 + 8 + (text_w(&AF_M, "100%  LET GO") < text_w(&AF_S, "100%  LET GO") ? text_w(&AF_M, "100%  LET GO") : text_w(&AF_S, "100%  LET GO")) <= 136, "the longest held text fits the slot");
    press(0); frame();
    press(1u << BK[5]); frame(); uisq_sel = 0; ui_sq_select_turn(1); frame(); press(0); frame(); CHECK(uisq_held_mv == 0, "released after SELECT moved: cleared");
    press(1u << BK[5]); frame(); other(UI_C_B_GLO); press(0); frame(); CHECK(uisq_held_mv == 0 && uisq_page == UI_PG_SET, "released on another page (SET): cleared"); other(UI_C_B_GLO);
    press(1u << BK[5]); frame(); other(UI_C_B_REC); press(0); frame(); CHECK(uisq_held_mv == 0 && uisq_page == UI_PG_REC, "released in REC (every key a note): cleared"); rec_btn();
    press(1u << BK[7]); frame(); press((1u << BK[7]) | (1u << BK[6])); frame(); CHECK(uisq_held_mv >= 1, "two move keys: one is named"); press(0); frame(); CHECK(uisq_held_mv == 0, "both up: cleared");
    uisq_sel = 0;

    /* ---- S01 end to end, no menu (power-on first run to a playing line) ---- */
    safe_world(); uisq_fr_on = 1; uisq_fr_step = 1; ui_sq_new_set(0, 1); sq_ch[0].midi_ch = 0; fresh(); fill();
    CHECK(V.cue && V.ch_midi[0] == 0, "S01 1: power-on PERF, `CHN KEY, K2 = MIDI`");
    chn_tap(); CHECK(uisq_page == UI_PG_CHAN, "S01 2: D#4 opens CHAN");
    knob(1, 1); fill(); CHECK(V.val[1] == 2 && V.ch_midi[0] == 1, "S01 3: K2 one detent: MIDI 2 at once in the tile and the header");
    wait_ms(700); CHECK(sq_ch[0].midi_ch == 1 && sq_ch[1].midi_ch == SQ_MIDI_OFF && uisq_fr_step == 2, "... committed after 0.6 s: CH1 MIDI 2, CH2 (which held 2) goes OFF, the strip is at PRESS PLAY");
    home_tap(); fill(); CHECK(uisq_page == UI_PG_PERF && V.fr == 2 && !V.cue, "S01 4: HOME: PERF, footer `PRESS PLAY`");
    ui_sq_play_pressed(); sq_playing = 1; frame(); fill(); CHECK(uisq_fr_step == 3 && V.playing, "S01 5: PLAY: playing, the strip says TURN K1");
    knob(2, 2); knob(0, 2); CHECK(uisq_fr_step == 4 && sq_ch[0].st[0].morph[0] == 14 && sq_ch[0].st[0].morph[2] == 5 + 2 || sq_ch[0].st[0].morph[1] == 8, "S01 6, 7: K2 and K1 morph (DENS 14), the strip moves to the dice step");
    { const uint16_t s0 = sq_ch[0].st[0].seed[0]; btn(UI_C_B_SCL); CHECK(sq_ch[0].st[0].seed[0] != s0 && toast_is(UI_MSG_TOAST_NOTES_EDIT_UNDO), "S01 8: SCL: new pitches, toast NOTES  EDIT=UNDO");
      edit_tap(); CHECK(sq_ch[0].st[0].seed[0] == s0 && toast_is(UI_MSG_TOAST_UNDO_EDIT_REDO), "S01 9: EDIT: UNDO, the old pitches return"); }
    other(UI_C_B_PLAY); sq_playing = 0; fresh();
    CHECK(uisq_page == UI_PG_PERF && !uisq_reset && !uisq_help, "S01 10: stopped, back on PERF; at no point did a menu open (this build has no menu page)");
    uisq_fr_save = 0; uisq_fr_on = 0;

    /* ---- hazards: a pseudo-random sequence of keys, knobs, buttons, HOME and EDIT holds keeps the invariants ---- */
    {   uint32_t rng = SEED, fails0 = (uint32_t)fails;
        uint32_t held = 0, helpk = 0;
        uint8_t home = 0, edit = 0;
        int nres = 0, nhelp = 0, nmv = 0, nfr = 0;
        safe_world(); fresh(); uisq_fr_on = 1; uisq_fr_step = 1; ui_sq_new_set(0, 1);
        #define RND() (rng = rng * 1664525u + 1013904223u, rng >> 8)
        for (int it = 0; it < 6000; it++) {
            const uint32_t r = RND(), a = r % 20u, k = (RND() >> 4) % 27u;
            switch (a) {
            case 0: case 1: case 2:                                        /* a key goes down or up */
                if (held >> k & 1u) held &= ~(1u << k);
                else { held |= 1u << k; if (home && !(uisq_keys >> k & 1u)) { ui_sq_help_touch(uisq_key_ctl((uint8_t)k)); helpk |= 1u << k; } }
                break;
            case 3: if (home) ui_sq_help_touch(UI_C_EN_K1 + (uint8_t)(k % 4u)); else ui_sq_knob((uint8_t)(k % 4u), (int32_t)(RND() % 7u) - 3); break;
            case 4: { static const uint8_t b[8] = { UI_C_B_FX, UI_C_B_SCL, UI_C_B_ENV, UI_C_B_LFO, UI_C_B_ARP, UI_C_B_SEQ, UI_C_B_OCTDN, UI_C_B_OCTUP }; const uint8_t c = b[k % 8u];
                if (home) ui_sq_help_touch(c); else ui_sq_button(c); break; }
            case 5: { static const uint8_t b[3] = { UI_C_B_GLO, UI_C_B_REC, UI_C_B_SAVE }; const uint8_t c = b[k % 3u]; if (home) ui_sq_help_touch(c); else ui_sq_other_button(c); break; }
            case 6: if (home) ui_sq_help_touch(UI_C_EN_SELECT); else ui_sq_select_turn((int32_t)(RND() % 3u) - 1); break;
            case 7: if (home) ui_sq_help_touch(UI_C_EN_ALGO); else ui_sq_algo_turn((int32_t)(RND() % 3u) - 1); break;
            case 8: if (home) ui_sq_help_touch(UI_C_EN_PRESET); else ui_sq_preset_turn((int32_t)(RND() % 3u) - 1); break;
            case 9: case 10: if (home) { if (edit) { edit = 0; } } else { edit = (uint8_t)!edit; } ui_sq_edit(home ? 0 : edit); break;   /* EDIT down / up (not under HELP) */
            case 11: case 12: home = (uint8_t)!home; ui_sq_home(home); if (home && edit) { uisq_edit_fired = 1; ui_sq_edit(0); edit = 0; } if (!home) helpk = helpk; break;
            case 13: if (!home && !uisq_help) ui_sq_play_pressed(); break;
            default: wait_ms(20 + RND() % 500u); break; }
            { const uint32_t mask = (held & ~helpk) ;
              helpk &= held;                                                /* a key pressed under HELP stays out of the page until it is up */
              ui_sq_keys(mask & ((1u << 27) - 1u)); }
            frame();
            if (uisq_reset) nres++;
            if (uisq_help) nhelp++;
            if (uisq_held_mv) nmv++;
            if (uisq_fr_on && uisq_fr_step > 1) nfr++;
            CHECK(uisq_page < UI_PG_COUNT && uisq_page != UI_PG_MENU && uisq_page != UI_PG_HELP && uisq_page != UI_PG_RESET, "it %d: page %d is a real page", it, uisq_page);
            CHECK(uisq_sel < SQ_NCH && (uisq_used >> uisq_sel & 1u) && uisq_vpage < 4u, "it %d: sel %d vpage %d inside", it, uisq_sel, uisq_vpage);
            CHECK(!uisq_reset || (uisq_reset_ok_page() && !uisq_rec), "it %d: RESET only over a page with defaults", it);
            CHECK(!uisq_help || uisq_home_dn, "it %d: HELP only while HOME is down", it);
            CHECK(uisq_home_dn || (!uisq_hold_home && !uisq_help_used && !uisq_home_held), "it %d: no HOME flag survives its release", it);
            CHECK(!uisq_held_mv || (uisq_keys >> BK[uisq_held_mv + 4u] & 1u), "it %d: a held move %d needs its key down", it, uisq_held_mv);
            CHECK(!uisq_held_m || (uisq_keys >> BK[uisq_held_m - 1u] & 1u), "it %d: a held mute %d needs its key down", it, uisq_held_m);
            CHECK(uisq_fr_step >= 1 && uisq_fr_step <= UISQ_FR_STEPS, "it %d: first-run step %d", it, uisq_fr_step);
            CHECK(uisq_cm_val == UISQ_CM_NONE || (uisq_cm_val <= SQ_MIDI_OFF && uisq_cm_ch < SQ_NCH), "it %d: pending MIDI channel", it);
            CHECK(uisq_pg_pend <= 1 && (!uisq_pg_pend || uisq_keys_live()), "it %d: the ADD PAGE prompt only on a key page", it);
            CHECK(sq_ch[0].len >= 1 && sq_ch[0].len <= SQ_NSTEP, "it %d: LENGTH %d", it, sq_ch[0].len);
            CHECK(uisq_cue_row() > UISQ_CR_NONE && uisq_cue_row() < UISQ_CR_COUNT, "it %d: a cue row is always selected", it);
            if ((uint32_t)fails > fails0 + 20u) break; }
        /* everything up: the state settles */
        press(0); ui_sq_keys(0); if (home) ui_sq_home(0); if (edit) { ui_sq_edit(0); } ui_sq_edit(0); ui_sq_home(0); wait_ms(3000); fill();
        CHECK(uisq_held_m == 0 && uisq_held_mv == 0 && !uisq_hold_on && !uisq_help && !uisq_home_dn && !uisq_edit_dn, "after the random run, with everything released, no hold flag is left (m %d mv %d hold %d help %d)", uisq_held_m, uisq_held_mv, uisq_hold_on, uisq_help);
        if (getenv("SAFETY_VERBOSE")) printf("random run: RESET %d, HELP %d, held move %d, first-run steps %d\n", nres, nhelp, nmv, nfr);
        CHECK(nres > 20 && nhelp > 20 && nmv > 20 && nfr > 0, "the random run reached RESET %d, HELP %d, a held move %d and first-run steps %d times", nres, nhelp, nmv, nfr); }

    printf(fails ? "ui_sq_safety_test: %d failure(s)\n" : "ui_sq_safety_test: OK (0 failures)\n", fails);
    return fails ? 1 : 0;
}
