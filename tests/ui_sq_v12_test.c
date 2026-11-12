/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_v12_test.c (P4-028, P4-029): the screen v2 elements of firmware/src/ui_sq.c (header, knob tiles, status line, music row, buttons, keybed, footer and
 * message slot; SCREEN_SPEC_V2 4.1-4.6 and 5)
 * drawn on the host for every scene of the v12 mock (docs/design/ui_review/proposals/v12/v12.py, exported to build/gen/v12_scenes.h by
 * tests/v12_export.py). The real ui_sq.c draws each scene from a UisqView built from the exported scene data; the 240x240 frames go to
 * build/ui_sq/v12/<scene>.png and tests/v12_compare.py compares them with the mock's renders (the whole 240 x 240 frame). The algorithm rows are the mock's (knob_map.tsv) except ACID, which is the firmware's own row.
 * This file also asserts uisq_view_fill() (the state -> view mapping) and that no algorithm is named by the screen code.
 *   cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_v12_test tests/ui_sq_v12_test.c -lm && /tmp/ui_sq_v12_test [outdir]
 * Run from the repo root after tests/v12_export.py and ./build.sh (needs build/gen). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#define __attribute__(x)

static uint16_t fb[240 * 240];
static int blits;
static void lcd_sync(void) {}
static void lcd_power(uint32_t s) { (void)s; }
static void lcd_wake_now(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{   blits++;
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
static struct { uint8_t force; } ui;

#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"
#include "sq_algo_acid.c"
#include "v12_scenes.h"

/* ---- the algorithm table: slot 0 the firmware's ACID, slots 1-5 the mock's rows, slot 5 swapped for DEMO (mock index 6) by the scene ---- */
static void a_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k) { (void)m; (void)seed; (void)g; (void)k; }
static uint8_t a_len(const void *k) { (void)k; return 16; }
static void a_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o)
{   (void)k; (void)hc; memset(o, 0, sizeof *o); o->note = (uint8_t)(36 + g % 12); o->vel = 80; o->gate = 50; o->flags = (uint8_t)((g % 3u) ? PO_HIT : 0); }
static uint8_t t_patt(uint8_t morph, uint8_t v, const void *cache, char *b)   /* PATTERNS PICK: tag + number within the tag */
{   (void)cache;
    if (morph != V12_PICK_SLOT) return 0;
    snprintf(b, SQ_VTEXT_MAX, "%s %d", V12_PATT_TAGS[(v - 1) / 20], (v - 1) % 20 + 1);
    return (uint8_t)strlen(b); }
static uint8_t t_stage(uint8_t morph, uint8_t v, const void *cache, char *b)  /* STAGE TOTAL: `48(32)` while the pattern has played only 32 steps; no cache = the widest form */
{   const uint8_t heard = cache ? *(const uint8_t *)cache : (uint8_t)(v - 1u);
    if (morph != V12_TOTAL_SLOT) return 0;
    if (heard >= v) snprintf(b, SQ_VTEXT_MAX, "%d", v); else snprintf(b, SQ_VTEXT_MAX, "%d(%d)", v, heard);
    return (uint8_t)strlen(b); }

static AlgoDesc ALGOS[SQ_NALGO];
static AlgoDesc ALGO_DEMO;
static void algos_init(void)
{   static const AlgoDesc acid = ALGO_ACID_ROW;
    static const char *const nm[7] = { "ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS", "DEMO" };
    AlgoDesc *t[7] = { &ALGOS[0], &ALGOS[1], &ALGOS[2], &ALGOS[3], &ALGOS[4], &ALGOS[5], &ALGO_DEMO };
    for (int i = 0; i < 7; i++) {
        AlgoDesc *a = t[i];
        if (i == 0) { *a = acid; continue; }
        memset(a, 0, sizeof *a);
        strncpy(a->name, nm[i], sizeof a->name - 1);
        a->id = (uint8_t)(i % SQ_NALGO); a->n_seed = 2; a->cache_bytes = 4;
        for (int s = 0; s < 8; s++) { a->knob_morph[s] = (uint8_t)s; a->morph[s] = V12_MORPH[i][s]; }
        for (int l = 0; l < SQ_NLAYER; l++) { a->layer[l].n = 1; strcpy(a->sub[l], "X"); }
        if (i == 6) a->layer[SQ_NLAYER - 3].n = 0;                       /* DEMO has no accent layer (mock: absent ACCNT) */
        strcpy(a->short_name, V12_SHORT[i]);
        a->lane = V12_SINGLE[i] ? SQ_LANE_SINGLE : SQ_LANE_PITCHED;
        a->build = a_build; a->loop_len = a_len; a->step = a_step;
        if (i == 5) a->value_text = t_patt;
        if (i == 3) a->value_text = t_stage; } }

#include "sq_core.c"
#include "sq_chan.c"
#include "../firmware/src/ui_sq.c"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

/* ---- a minimal PNG writer (stored deflate blocks, no dependency), as tests/ui_sq_test.c ---- */
static uint32_t crc_t[256];
static void crc_init(void) { for (uint32_t n = 0; n < 256; n++) { uint32_t c = n; for (int k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1; crc_t[n] = c; } }
static uint32_t crc_up(uint32_t c, const uint8_t *b, size_t n) { for (size_t i = 0; i < n; i++) c = crc_t[(c ^ b[i]) & 255u] ^ (c >> 8); return c; }
static void put32(FILE *f, uint32_t v) { fputc(v >> 24, f); fputc(v >> 16, f); fputc(v >> 8, f); fputc(v, f); }
static void chunk(FILE *f, const char *t, const uint8_t *d, uint32_t n)
{   uint32_t c = crc_up(0xFFFFFFFFu, (const uint8_t *)t, 4);
    put32(f, n); fwrite(t, 1, 4, f); if (n) fwrite(d, 1, n, f);
    c = crc_up(c, d, n); put32(f, ~c); }
static void png_write(const char *path)
{   const int w = 240, h = 240;
    uint32_t rawn = (uint32_t)h * (1u + (uint32_t)w * 3u), a = 1, b = 0;
    uint8_t *raw = malloc(rawn), *z, *p;
    uint32_t zn, i = 0;
    FILE *f = fopen(path, "wb");
    if (!f) { printf("FAIL: cannot write %s\n", path); fails++; free(raw); return; }
    for (int y = 0; y < h; y++) {
        raw[i++] = 0;
        for (int x = 0; x < w; x++) {
            uint16_t c = swap16(fb[y * 240 + x]);
            raw[i++] = (uint8_t)(((c >> 11) & 31u) * 255u / 31u); raw[i++] = (uint8_t)(((c >> 5) & 63u) * 255u / 63u); raw[i++] = (uint8_t)((c & 31u) * 255u / 31u); } }
    zn = 2 + (rawn / 65535u + 1u) * 5u + rawn + 4u; z = malloc(zn); p = z;
    *p++ = 0x78; *p++ = 0x01;
    for (uint32_t o = 0; o < rawn;) {
        uint32_t n = rawn - o > 65535u ? 65535u : rawn - o;
        *p++ = (uint8_t)(o + n >= rawn); *p++ = (uint8_t)n; *p++ = (uint8_t)(n >> 8); *p++ = (uint8_t)~n; *p++ = (uint8_t)(~n >> 8);
        memcpy(p, raw + o, n); p += n; o += n; }
    for (uint32_t k = 0; k < rawn; k++) { a = (a + raw[k]) % 65521u; b = (b + a) % 65521u; }
    *p++ = (uint8_t)(b >> 8); *p++ = (uint8_t)b; *p++ = (uint8_t)(a >> 8); *p++ = (uint8_t)a;
    { static const uint8_t sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 }; uint8_t ih[13] = { 0, 0, 0, 240, 0, 0, 0, 240, 8, 2, 0, 0, 0 };
      fwrite(sig, 1, 8, f); chunk(f, "IHDR", ih, 13); chunk(f, "IDAT", z, (uint32_t)(p - z)); chunk(f, "IEND", NULL, 0); }
    fclose(f); free(raw); free(z); }

static const char *outdir = "build/ui_sq/v12";
static void shot(const char *name)
{   char path[256]; snprintf(path, sizeof path, "%s/%s.png", outdir, name); png_write(path); }

/* a scene of the mock -> the view the elements draw from (the same fields uisq_view_fill() derives from the sequencer state) */
static uint8_t heard_byte;
static void scene_view(UisqView *v, const V12Scene *s)
{   const int dm = s->algo[s->sel] == 6;                                  /* the DEMO algorithm takes slot 5 for this scene */
    const AlgoDesc *a;
    memset(v, 0, sizeof *v);
    if (dm) ALGOS[5] = ALGO_DEMO;
    v->playing = s->playing; v->rec = s->rec; v->clock = s->clock; v->used = s->used; v->sel = s->sel; v->bpm = s->bpm;
    for (int i = 0; i < 4; i++) { v->ch_algo[i] = s->algo[i] == 6 ? 5 : s->algo[i]; v->ch_midi[i] = s->midi[i]; v->ch_muted[i] = s->muted[i]; }
    a = &ALGOS[v->ch_algo[s->sel]];
    v->algo = a;
    v->band = s->band; v->row = s->row; v->nrow = s->nrow;
    for (int k = 0; k < 4; k++) {
        v->val[k] = s->val[k];
        if (v->band == UISQ_BAND_REC) v->tile[k] = &uisq_rec_row[k];
        else if (v->band == UISQ_BAND_CHAN) v->tile[k] = &uisq_chan_row[k];
        else if (v->row == 2) v->tile[k] = &V12_RIG[k];
        else { v->tile_id[k] = a->knob_morph[(v->row * 4 + k) & 7]; v->tile[k] = &a->morph[v->tile_id[k]]; } }
    v->turn = s->turn; v->turn_old = s->turn_old;
    heard_byte = s->heard; v->cache = &heard_byte;
    uisq_view_texts(v);
    v->preset = s->preset; v->pend = s->pend; v->pages = s->pages; v->vpage = s->vpage; v->ppage = s->ppage; v->follow_off = s->follow_off;
    v->btn_lit = s->lit >= 0 ? (uint16_t)(1u << s->lit) : 0u; v->btn_dim = uisq_btn_dimmask(a);
    v->held_m = s->held_m; v->held_mv = s->held_mv; v->cue = s->cue;
    v->msg.kind = s->mk; v->msg.chip = s->mchip; v->msg.icon = s->micon; v->msg.frac = s->mfrac;
    strcpy(v->msg.a, s->ma); strcpy(v->msg.b, s->mb);
    v->ph = s->ph; v->len16 = s->len16; v->cursor = s->cursor; v->diced = s->diced;
    for (int i = 0; i < 16; i++) {
        const V12Step *p = &s->lane[s->sel][i];
        v->lane[i].on = p->note >= 0; v->lane[i].note = (uint8_t)(p->note < 0 ? 0 : p->note); v->lane[i].acc = p->acc; v->lane[i].slide = p->sl; }
    for (int c = 0; c < 4; c++) for (int i = 0; i < 16; i++) if (s->lane[c][i].note >= 0 && c != s->sel) v->hits[c] |= (uint16_t)(1u << i); }

static void draw_view(const UisqView *v, int sheet_y)
{   for (int st = 0; st < 2; st++) {
        cv_begin(240, 120, T_BG);
        cv_oy = -120 * st;
        if (sheet_y < 0) uisq_draw_elements(v); else uisq_draw_knobs(v, sheet_y);
        cv_oy = 0;
        cv_blit(0, (uint32_t)(120 * st)); } }

int main(int argc, char **argv)
{   char cmd[300];
    if (argc > 1) outdir = argv[1];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", outdir); if (system(cmd)) return 1;
    crc_init(); palette_set(UI_HICON_INDEX);
    algos_init();

    /* ---- every scene of the mock ---- */
    for (int i = 0; i < V12_NSCENES; i++) {
        UisqView v;
        char nm[96];
        algos_init();                                                      /* slot 5 = PATTERNS again */
        scene_view(&v, &V12_SCENES[i]);
        draw_view(&v, -1);
        snprintf(nm, sizeof nm, "%s", V12_SCENES[i].name); shot(nm); }

    /* ---- the knob sheets: every algorithm and row through the tile code (the sheets of the mock) ---- */
    for (int i = 0; i < V12_NSHEETS; i++) {
        const V12Sheet *sh = &V12_SHEETS[i];
        char nm[96];
        cv_begin(240, 120, T_BG);
        for (int st = 0; st < 2; st++) {
            cv_begin(240, 120, T_BG);
            cv_oy = -120 * st;
            for (int n = 0; n < sh->n; n++) {
                UisqView v;
                algos_init();
                memset(&v, 0, sizeof v);
                v.sel = 0; v.band = UISQ_BAND_ROW; v.row = sh->row; v.nrow = 2;
                if (sh->algo[n] == 6) ALGOS[5] = ALGO_DEMO;
                v.algo = &ALGOS[sh->algo[n] == 6 ? 5 : sh->algo[n]];
                heard_byte = 255; v.cache = &heard_byte;
                for (int k = 0; k < 4; k++) { v.tile_id[k] = v.algo->knob_morph[(v.row * 4 + k) & 7]; v.tile[k] = &v.algo->morph[v.tile_id[k]]; v.val[k] = v.tile[k]->def; }
                uisq_view_texts(&v);
                uisq_draw_knobs(&v, 58 * n); }
            cv_oy = 0;
            cv_blit(0, (uint32_t)(120 * st)); }
        snprintf(nm, sizeof nm, "%s", sh->name); shot(nm); }
    printf("ui_sq_v12: %d scenes and %d knob sheets drawn to %s\n", V12_NSCENES, V12_NSHEETS, outdir);

    /* ---- the real ACID row equals the mock's (the 'playing' scene is drawn with the firmware row) ---- */
    { const AlgoDesc *a = &ALGOS[0];
      for (int s = 0; s < 8; s++) {
          const MorphDesc *f = &a->morph[a->knob_morph[s]], *m = &V12_MORPH[0][s];
          CHECK(!strcmp(f->label, m->label) && f->min == m->min && f->max == m->max && f->def == m->def &&
                (!!f->names == !!m->names) && ((f->zero == 0) == (m->zero == 0)), "ACID slot %d: firmware `%s` %d..%d def %d vs the mock `%s` %d..%d def %d",
                s, f->label, f->min, f->max, f->def, m->label, m->min, m->max, m->def); } }

    /* ---- uisq_view_fill: the state -> view mapping (the lanes are the engine's; the rest is checked here) ---- */
    algos_init();
    sq_boot();
    fm1_ms = 1000; uisq_now = fm1_ms;
    { UisqView v;
      sq_ch[0].midi_ch = 1; sq_ch[1].midi_ch = SQ_MIDI_OFF; sq_ch[2].muted = 1; sq_ch[0].len = 40; sq_bpm = 133;
      sq_follow = 1; sq_playing = 1; sq_rt[0].p = 21;                         /* step 20 is sounding, on page 2 (0-based 1) */
      uisq_sel = 1; uisq_used = 0x0B; uisq_clock = UISQ_CLK_WAIT; uisq_row = 0; uisq_page = UI_PG_PERF; uisq_rec = 0;
      uisq_view_fill(&v);
      CHECK(v.sel == 1 && v.used == 0x0B && v.clock == UISQ_CLK_WAIT && v.bpm == 133 && v.playing == 1 && v.rec == 0, "fill: transport / channels");
      CHECK(v.ch_midi[0] == 1 && v.ch_midi[1] == SQ_MIDI_OFF && v.ch_muted[2] == 1, "fill: chips read midi_ch and muted");
      CHECK(v.band == UISQ_BAND_ROW && v.nrow == 2 && v.row == 0 && v.algo == &ALGOS[sq_ch[1].algo], "fill: knob band and the selected channel's descriptor");
      uisq_sel = 0; uisq_view_fill(&v);
      CHECK(v.pages == 3 && v.vpage == 2 && v.ppage == 2 && v.ph == 4 && v.follow_off == 0, "fill: 40 steps = 3 pages, FOLLOW puts the view on the playing page 2, playhead column 4 (%d/%d/%d ph %d)", v.vpage, v.ppage, v.pages, v.ph);
      sq_follow = 0; uisq_vpage = 2; uisq_view_fill(&v);
      CHECK(v.len16 == 8 && v.vpage == 3 && v.ph == UISQ_NO_PH, "fill: LENGTH 40 leaves 8 steps on page 3, got %d", v.len16);
      uisq_vpage = 0; uisq_view_fill(&v);
      CHECK(v.follow_off == 1 && v.vpage == 1 && v.ppage == 2 && v.ph == UISQ_NO_PH && v.len16 == 16, "fill: FOLLOW off: page 1 viewed, page 2 playing, no playhead");
      uisq_page = UI_PG_CHAN; uisq_view_fill(&v);
      CHECK(v.band == UISQ_BAND_CHAN && v.val[0] == 40 && v.val[1] == 2 && v.val[3] == 0, "fill: CHAN band LENGTH / MIDI (1-based) / FOLLOW");
      uisq_page = UI_PG_PERF; uisq_rec = 1; uisq_rec_cursor = 7; uisq_view_fill(&v);
      CHECK(v.band == UISQ_BAND_REC && v.val[0] == 7 && v.cursor == 7, "fill: REC band CURSOR");
      uisq_rec = 0;
      uisq_row = 2; uisq_view_fill(&v); CHECK(v.nrow == 2 && v.row == 0, "fill: no RIG device = two rows, ROW 3 folds back to 1");
      uisq_row = 1; uisq_view_fill(&v);
      CHECK(v.row == 1 && v.tile[0] == &ALGOS[sq_ch[0].algo].morph[ALGOS[sq_ch[0].algo].knob_morph[4]], "fill: ROW 2 takes knob_morph slots 4-7");
      { const uint32_t s0 = uisq_view_sig(&v); uisq_view_fill(&v); CHECK(s0 == uisq_view_sig(&v), "the view signature is stable for an unchanged state");
        sq_ch[0].st[sq_ch[0].algo].morph[ALGOS[sq_ch[0].algo].knob_morph[4]] ^= 1; uisq_view_fill(&v); CHECK(s0 != uisq_view_sig(&v), "a changed knob value changes the signature");
        sq_ch[0].st[sq_ch[0].algo].morph[ALGOS[sq_ch[0].algo].knob_morph[4]] ^= 1; }
      /* a turn: the tile frames itself while hot and remembers its old level */
      uisq_row = 0; uisq_view_fill(&v);
      { const uint8_t id = ALGOS[sq_ch[0].algo].knob_morph[1]; const uint8_t old = sq_ch[0].st[sq_ch[0].algo].morph[id];
        ui_sq_knob_touch(1); sq_ch[0].st[sq_ch[0].algo].morph[id] = (uint8_t)(old < 4 ? old + 4 : old - 4); uisq_view_fill(&v);
        CHECK(v.turn == 2, "a touched knob is the turning one");
        CHECK(v.turn_old == (uint8_t)(100u * (uint32_t)(old - v.tile[1]->min) / (uint32_t)(v.tile[1]->max - v.tile[1]->min)), "the ticks sit at the level before the turn (%d)", v.turn_old);
        uisq_now += UISQ_HOT_MS + 1; uisq_view_fill(&v); CHECK(v.turn == 0, "the turning frame ends %d ms after the last detent", UISQ_HOT_MS); } }

    /* ---- P4-029: buttons, keybed, footer, message slot ---- */
    { UisqView v;
      /* the generated message table: every row fits the slot by the rule of SCREEN_SPEC_V2 5 (tools/gen_ui_tables.py proves it at build time; the same rule here, on the real fonts) */
      CHECK(UI_N_MSG == UI_MSG_COUNT && UI_N_MSG >= 30, "the message table has %d rows", UI_N_MSG);
      for (int i = 0; i < UI_N_MSG; i++) {
          const char *a = ui_msgs[i].a, *b = ui_msgs[i].b;
          const int ma = text_w(&AF_M, a) < text_w(&AF_S, a) ? text_w(&AF_M, a) : text_w(&AF_S, a), mb = text_w(&AF_M, b) < text_w(&AF_S, b) ? text_w(&AF_M, b) : text_w(&AF_S, b);
          int need = ui_msgs[i].kind == UI_MK_TOUCH ? text_w(&AF_S, a) + 6 + 8 + mb : ui_msgs[i].kind == UI_MK_HOLD ? 22 + ma : 3 + 17 + ma + 6 + text_w(&AF_S, b) + 3;
          CHECK(need <= 136, "message %d `%s` `%s` needs %d of 136 px", i, a, b, need); }
      CHECK(text_w(&AF_M, uisq_foot_cue) + text_w(&AF_M, "CH4") + 8 <= 240 && text_w(&AF_M, uisq_foot_rec) + text_w(&AF_S, uisq_foot_done) + 4 <= 240 &&
            text_w(&AF_M, uisq_foot_midi) + text_w(&AF_M, uisq_foot_back) + 4 <= 240 && text_w(&AF_M, uisq_foot_steps) + text_w(&AF_M, uisq_foot_edit) + 4 <= 240, "the footer lines fit 240 px");
      /* hold times come from the TSVs: EDIT / GLO 0.4 s, SAVE 0.7 s, the keys 0.4 s; HOME acts on release */
      CHECK(uisq_hold_ms(UI_C_B_EDIT) == 400 && uisq_hold_ms(UI_C_B_GLO) == 400 && uisq_hold_ms(UI_C_B_SAVE) == 700 && uisq_hold_ms(UI_C_KEY_B04_CS4) == 400 && uisq_hold_ms(UI_C_B_HOME) == 0,
            "hold times: EDIT %d GLO %d SAVE %d HOME %d", uisq_hold_ms(UI_C_B_EDIT), uisq_hold_ms(UI_C_B_GLO), uisq_hold_ms(UI_C_B_SAVE), uisq_hold_ms(UI_C_B_HOME));
      /* timing and priority of the slot */
      fm1_ms = 10000; uisq_now = fm1_ms; uisq_home_held = 0; uisq_toast_until = uisq_touch_until = 0; uisq_hold_on = uisq_touch_held = 0; uisq_touch_a[0] = 0;
      uisq_page = UI_PG_PERF;
      uisq_msg_pick(&v.msg, fm1_ms, uisq_page, 0); CHECK(v.msg.kind == UI_MK_NONE, "an idle slot is empty");
      uisq_page = UI_PG_CHAN; uisq_msg_pick(&v.msg, fm1_ms, uisq_page, 1); CHECK(v.msg.kind == UI_MK_TOUCH && !strcmp(v.msg.a, "CHN") && !strcmp(v.msg.b, "CH2 SETUP"), "the CHAN page names itself: `%s %s`", v.msg.a, v.msg.b);
      uisq_page = UI_PG_PERF;
      ui_sq_toast_id(UI_MSG_TOAST_NOTES_EDIT_UNDO, 0);
      uisq_msg_pick(&v.msg, fm1_ms, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOAST && v.msg.chip == 1 && !strcmp(v.msg.a, "NOTES") && !strcmp(v.msg.b, "EDIT=UNDO"), "a toast: [1] NOTES EDIT=UNDO");
      uisq_page = UI_PG_CHAN; uisq_msg_pick(&v.msg, fm1_ms, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOAST, "a toast outranks the page title"); uisq_page = UI_PG_PERF;
      uisq_msg_pick(&v.msg, fm1_ms + 999, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOAST, "the toast is up at 999 ms");
      uisq_msg_pick(&v.msg, fm1_ms + 1000, uisq_page, 0); CHECK(v.msg.kind == UI_MK_NONE, "the toast is gone at 1000 ms");
      ui_sq_touch_id(UI_MSG_ENC_SELECT_CH2_EUCLID, 0);
      uisq_msg_pick(&v.msg, fm1_ms, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOUCH && !strcmp(v.msg.a, "SELECT"), "a touched control outranks a toast (the toast is still pending)");
      uisq_msg_pick(&v.msg, fm1_ms + 1000, uisq_page, 0); CHECK(v.msg.kind == UI_MK_NONE, "a touched control is named ~1 s after the last touch (the toast ended too)");
      ui_sq_touch("SOLO", "CH1 SOLO", 1);
      uisq_msg_pick(&v.msg, fm1_ms + 5000, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOUCH, "a held key stays named until let go");
      fm1_ms += 5000; ui_sq_touch_end(); uisq_msg_pick(&v.msg, fm1_ms + 999, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOUCH, "... and 1 s after");
      uisq_msg_pick(&v.msg, fm1_ms + 1000, uisq_page, 0); CHECK(v.msg.kind == UI_MK_NONE, "... then it goes");
      ui_sq_toast_id(UI_MSG_TOAST_UNDO_EDIT_REDO, 2); ui_sq_touch("PRESETS", "P02 AT BAR", 0);
      ui_sq_hold_ctl(UI_MSG_HOLD_RESET, SQI_UNDO, UI_C_B_EDIT);
      uisq_msg_pick(&v.msg, fm1_ms + 249, uisq_page, 0); CHECK(v.msg.kind == UI_MK_TOUCH, "a hold shows nothing before 0.25 s");
      uisq_msg_pick(&v.msg, fm1_ms + 250, uisq_page, 0); CHECK(v.msg.kind == UI_MK_HOLD && !strcmp(v.msg.a, "RESET") && v.msg.icon == SQI_UNDO && v.msg.frac == 62, "from 0.25 s the hold hint outranks all (bar %d%% of the 0.4 s EDIT hold)", v.msg.frac);
      uisq_msg_pick(&v.msg, fm1_ms + 400, uisq_page, 0); CHECK(v.msg.frac == 100, "the bar is full at the fire time");
      ui_sq_hold_end(); ui_sq_hold_ctl(UI_MSG_HOLD_HELP_MENU, SQI_HOME, UI_C_B_HOME);
      uisq_msg_pick(&v.msg, fm1_ms + 300, uisq_page, 0); CHECK(v.msg.kind == UI_MK_HOLD && v.msg.frac == 100, "HOME acts on release: a full bar");
      ui_sq_hold_end(); ui_sq_hold_begin("REMOVE CH4", SQI_SPKX, uisq_remove_ms(UI_C_KEY_B04_CS4));
      uisq_msg_pick(&v.msg, fm1_ms + 750, uisq_page, 0); CHECK(v.msg.frac == 50, "the remove hold's bar is half full at 0.75 s of 1.5 s, got %d", v.msg.frac);
      ui_sq_hold_end(); uisq_touch_until = uisq_toast_until = 0;
      /* the message slot belongs to strip 0 only */
      { UisqView a, b; uisq_view_fill(&a); ui_sq_toast_msg(0xFF, "PAGE 2?", "PG+=ADD"); uisq_view_fill(&b);
        CHECK(uisq_view_sig(&a) == uisq_view_sig(&b) && uisq_msg_sig(&a) != uisq_msg_sig(&b), "a toast changes the slot's signature only (strip 1 is not redrawn)"); uisq_toast_until = 0; }
      /* the view fill: buttons, keybed, cue */
      uisq_sel = 0; sq_ch[0].midi_ch = 1; uisq_new = 0; uisq_btn_down = 0; ui_sq_btn_down(4, 1); ui_sq_held_m_set(2); ui_sq_held_mv_set(3);
      uisq_view_fill(&v);
      CHECK(v.btn_lit == 0x10 && v.held_m == 2 && v.held_mv == 3 && v.cue == 0, "fill: EDIT pressed, M2 and ROL held, no setup cue for a channel with a MIDI channel");
      ui_sq_btn_down(4, 0); ui_sq_held_m_set(0); ui_sq_held_mv_set(0);
      ui_sq_new_set(0, 1); uisq_view_fill(&v); CHECK(v.cue == 1, "fill: a new channel shows the setup cue");
      ui_sq_new_set(0, 0); sq_ch[0].midi_ch = SQ_MIDI_OFF; uisq_view_fill(&v); CHECK(v.cue == 1, "fill: MIDI OFF shows the setup cue");
      sq_ch[0].midi_ch = 1; uisq_view_fill(&v); CHECK(v.cue == 0, "fill: the cue clears when the channel gets a MIDI channel");
      CHECK(uisq_btn_dimmask(&ALGO_DEMO) == (1u << 2), "DEMO lacks ACCNT: only the ENV die is dim (mask %x)", uisq_btn_dimmask(&ALGO_DEMO));
      CHECK(uisq_btn_dimmask(&ALGOS[1]) == 0, "an algorithm with all layers dims nothing"); }

    /* ---- no algorithm is named by the screen code ---- */
    { FILE *f = fopen("firmware/src/ui_sq.c", "r"); char ln[600]; int hits = 0;
      CHECK(f != NULL, "ui_sq.c opens (run from the repo root)");
      while (f && fgets(ln, sizeof ln, f)) {
          static const char *bad[] = { "SQ_ALGO_", "\"ACID\"", "\"EUCL", "\"SNAKE\"", "\"STAGE\"", "\"ARP\"", "\"PATT", "ALGO_ACID" };
          char *c = strstr(ln, "/*");
          if (c) *c = 0;                                                   /* comments may name them */
          for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; i++) if (strstr(ln, bad[i])) {
              if (strstr(ln, "uisq_short[SQ_NALGO]")) continue;            /* the placeholder header names until a row fills short_name (read by tools/gen_ui_tables.py) */
              hits++; printf("algorithm named in ui_sq.c: %s", ln); } }
      if (f) fclose(f);
      CHECK(hits == 0, "ui_sq.c names an algorithm %d times outside comments", hits); }

    printf(fails ? "ui_sq_v12_test: %d FAILED\n" : "ui_sq_v12_test: OK (%d failures)\n", fails);
    return fails != 0;
}
