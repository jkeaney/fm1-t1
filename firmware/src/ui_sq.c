/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq.c: the sequencer UI framework: the screen v2 bands, the two-strip plan and the empty frame (P4-026, SCREEN_SPEC_V2 2, ADR-0006),
 * the toast (state only: the message slot is drawn by P4-029), the label lookups of the generated tables build/gen/ui_tables.h,
 * redraw by state signature, the page enum and the page dispatch with stub pages. It replaces ui_t1.c's screen at the cut-over
 * (P4-012); until then NOTHING in ui_draw.c, ui_input.c or main.c calls it, and felucca.c does not include it (the host test
 * tests/ui_sq_test.c does, after gfx.c, sq_core.c and sq_chan.c).
 *
 * Include order (unity build): after gfx.c (cv_*, cv_icon, text_w, AF_*, T_*), ui.c (`ui.force`), sq_core.c and sq_chan.c (the sq_*
 * state and API), and ui_tables.h (build/gen; included here, it is a pragma-once header of flash-resident const tables). Freestanding.
 *
 * State rules. The main loop calls SQ_FRAME_SYNC() (sq_chan.c, ADR-0003 6) ONCE per frame BEFORE ui_sq_input(); everything this file
 * reads is read after that, in ui_sq_input() / ui_sq_draw(). It reads only the sq_* API and the exported state the sq files
 * document for the main loop: sq_ch, sq_gdb / sq_cache / sq_cidx (through sq_eval, as sq_gen_hit does), sq_playing, sq_bpm, sq_follow,
 * and ChanRT's `p` and `snd[0]` as READ-ONLY values (single bytes). It writes nothing of the sequencer's.
 *
 * Pixel contract (SCREEN_SPEC_V2 2): the screen is drawn in SCREEN coordinates in two strips, 0-119 and 120-239 (CV_MAX = 240 x 124
 * holds 120 rows); a strip sets cv_oy = -y0, so an element that crosses y 120 (the music row's bars) is drawn by both strips and
 * clipped by each. No text crosses y 120 (tests/ui_sq_test.c checks the bands). A strip is redrawn only when the signature of its
 * inputs changed or `ui.force` is set. The element drawing (header, knob tiles, status line, music row, buttons, keybed, footer, message
 * slot) is P4-028 / P4-029 and draws into these bands; this file draws the empty frame: the tiles and keys the elements sit on.
 *
 * The helpers below the band table (uisq_btn_*, uisq_cell_*, uisq_h4_text, uisq_lane_read ...) are the screen-v1 label and lane
 * code, kept until P4-028 / P4-029 reuse or delete them (they are not drawn by this file; tests/ui_sq_test.c still measures them). */

#include "ui_tables.h"

/* ---- hooks (the host test and the cut-over may define them first) ---- */
#ifndef UISQ_MS
#define UISQ_MS() (fm1_ms)               /* milliseconds, the clock ui_input.c's timers use */
#endif



/* ---- the bands (SCREEN_SPEC_V2 2; y in screen px). The grid: 16 columns of 15 px = the 16 white keys; 4 x 60 for knobs; 6 x 40 for buttons ---- */
#define UISQ_W        240
#define UISQ_H        240
#define UISQ_HDR_Y    0                  /* header 1-19 + selection bar 20-21; the message slot (x 104-239) shares these rows */
#define UISQ_HDR_H    22
#define UISQ_MSG_X    104
#define UISQ_KNOB_Y   24                 /* four tiles 58 x 53 at x = 1 + 60 k: label band 16 px, value area 36 px (1 px gap) */
#define UISQ_KNOB_H   53
#define UISQ_KNOB_W   58
#define UISQ_KNOB_LAB 16
#define UISQ_KNOB_VAL 36
#define UISQ_STAT_Y   77                 /* status line (caps 80) */
#define UISQ_STAT_H   15
#define UISQ_MUS_Y    92                 /* music row: pointer 92-94, the selected channel's bars, then a 3 px strip row per channel */
#define UISQ_MUS_H    50                 /* 92-141 */
#define UISQ_BTN_Y    146                /* 2 x 6 tiles 38 x 17 at pitch 40 x 18 */
#define UISQ_BTN_H    35                 /* 146-180 */
#define UISQ_BTN_W    38
#define UISQ_BTN_TH   17
#define UISQ_KEY_Y    186                /* 16 white keys 14 x 33 at a 15 px pitch; 11 black keys 10 x 19 */
#define UISQ_KEY_H    35                 /* 186-220 (33 + the 2 px playing bar under a key) */
#define UISQ_WKEY_W   14
#define UISQ_WKEY_H   33
#define UISQ_BKEY_W   10
#define UISQ_BKEY_H   19
#define UISQ_FOOT_Y   221                /* footer: caps 225-236, the keys' mode line (AF_M) or the setup cue */
#define UISQ_FOOT_H   19
/* the firmware strips: two, each at most 240 x 124 (CV_MAX) */
#define UISQ_NSTRIP   2
#define UISQ_STRIP_H  120
static const uint8_t uisq_strip_y[UISQ_NSTRIP] = { 0, 120 };
/* black key x centres (the 11 black keys, F#3 .. A#5 by panel order) */
static const uint8_t uisq_bkey_cx[11] = { 15, 30, 45, 75, 90, 120, 135, 150, 180, 195, 225 };
/* the panel order of the 12 buttons FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC: 1 = a dice tile (T_DICE), 0 = a plain tile (T_SURF) */
static const uint8_t uisq_btn_dice[12] = { 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0 };

#define UISQ_TOAST_MS 2000u              /* SCREEN_SPEC 2.7: one constant */
#define UISQ_PICK_MS  2000u              /* ALGO_PICK: the strip lists the algorithms for 2 s after the last detent */
#define UISQ_HOT_MS   600u               /* a knob stays "hot" (T_ACCENT value) this long after a turn */
#define UISQ_TOAST_N  40

static const char *const uisq_short[SQ_NALGO] = { "ACID", "EUCL", "SNAK", "STAG", "ARP", "PATT" };   /* SCREEN_SPEC 2.2 */

/* ---- UI state (main loop only) ---- */
static uint8_t  uisq_page = UI_PG_PERF;  /* the page shown: ui_page_t */
static uint8_t  uisq_sel;                /* the selected channel 0..3 */
static uint8_t  uisq_vpage;              /* the viewed lane page 0..3 */
static uint8_t  uisq_row;                /* PERF knob band: 0 = ROW 1 live, 1 = ROW 2 live */
static uint8_t  uisq_edit_step;          /* NOTE_EDIT: the step being edited, 0..63 */
static uint8_t  uisq_preset = 1;         /* header H3 slot (the preset layer is P7; a placeholder here) */
static uint8_t  uisq_preset_pend;        /* 1 = `Pnn?` */
static uint8_t  uisq_home_held;          /* a toast raised while HOME is held is dropped (SCREEN_SPEC 2.7) */
static uint32_t uisq_now;                /* UISQ_MS() of this frame */
static char     uisq_toast[UISQ_TOAST_N];
static uint32_t uisq_toast_until;        /* 0 = no toast */
static uint16_t uisq_toast_id;
static uint32_t uisq_pick_until;         /* ALGO_PICK overlay end, 0 = off */
static uint32_t uisq_hot_until[4];
static uint32_t uisq_sig[UISQ_NSTRIP];   /* the strips' signatures */
static uint8_t  uisq_have_sig;

/* ---- small freestanding string helpers ---- */
static void uisq_cat(char *d, uint32_t n, const char *s)
{   uint32_t k = 0;
    while (k + 1u < n && d[k]) k++;
    while (k + 1u < n && *s) d[k++] = *s++;
    d[k] = 0; }
static void uisq_catu(char *d, uint32_t n, uint32_t v)
{   char t[12]; uint32_t i = 0;
    do { t[i++] = (char)('0' + v % 10u); v /= 10u; } while (v && i < sizeof t);
    { char o[12]; uint32_t j = 0; while (i) o[j++] = t[--i]; o[j] = 0; uisq_cat(d, n, o); } }
static uint32_t uisq_h(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }
static uint32_t uisq_streq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }

/* ---- table lookups ---- */
static uint8_t uisq_ctx(uint8_t ch) { return sq_ch[ch].algo < SQ_NALGO ? sq_ch[ch].algo : UI_CTX_ANY; }   /* ALGO ids = UI_CTX ids 0..5 */

static const char *uisq_knob_label(uint8_t page, uint8_t ctx, uint8_t knob)    /* exact algorithm first, then the any-algorithm row */
{   const char *any = "";
    for (uint32_t i = 0; i < UI_N_KNOB_MAP; i++) {
        const ui_knob_row_t *r = &ui_knob_rows[i];
        if (r->page != page || r->knob != knob) continue;
        if (r->ctx == ctx) return r->label;
        if (r->ctx == UI_CTX_ANY) any = r->label; }
    return any; }

static const char *uisq_btn_label(uint8_t page, uint8_t ctl, uint8_t mode)
{   for (uint32_t i = 0; i < UI_N_BUTTON_MODES; i++) {
        const ui_button_row_t *r = &ui_button_rows[i];
        if (r->page == page && r->ctl == ctl && r->mode == mode) return r->label; }
    return ""; }

static const char *uisq_key_label(uint8_t page, uint8_t ctl, uint8_t gest)
{   for (uint32_t i = 0; i < UI_N_KEY_MAP; i++) {
        const ui_key_row_t *r = &ui_key_rows[i];
        if (r->page == page && r->ctl == ctl && r->gesture == gest) return r->label; }
    return ""; }

/* the dice layer a PERF button rolls: SCL NOTES(0), SEQ HITS(1), ENV ACCNT(2), ARP ARTIC(3), LFO SHAPE(4); -1 = not a dice button */
static int8_t uisq_layer_of(uint8_t ctl)
{   switch (ctl) { case UI_C_B_SCL: return 0; case UI_C_B_SEQ: return 1; case UI_C_B_ENV: return 2; case UI_C_B_ARP: return 3; case UI_C_B_LFO: return 4; default: return -1; } }

/* the cue strip text of a page: the generated page_states.tsv cue_strip (gen_ui_tables.py) unless the page keeps its own */
static const char *uisq_page_cue(uint8_t p);
static uint8_t uisq_algo_ok(uint8_t a) { return a < SQ_NALGO && ALGOS[a].name[0] != 0; }   /* a row not yet written by its algorithm task is zero */

/* ---- the algorithm descriptor as the screen reads it (ADR-0007, P4-027): the screen names no algorithm ---- */
/* the header short name: AlgoDesc.short_name; the placeholder table above serves a row whose P5 task has not filled it yet */
static const char *uisq_short_of(uint8_t a) { a %= SQ_NALGO; return ALGOS[a].short_name[0] ? ALGOS[a].short_name : uisq_short[a]; }

/* The text of a knob tile value: the algorithm's hook, else the zero word, else the value name, else the number (+ " " + unit). */
static void uisq_morph_text(char *d, uint32_t n, const AlgoDesc *a, uint8_t id, uint8_t v, const void *cache)
{   const MorphDesc *m = &a->morph[id];
    char b[SQ_VTEXT_MAX];
    d[0] = 0;
    b[0] = 0;
    if (a->value_text && a->value_text(id, v, cache, b) && b[0]) { uisq_cat(d, n, b); return; }
    if (v == 0 && m->zero) { uisq_cat(d, n, m->zero); return; }
    if (m->names && v >= m->min && v <= m->max) { uisq_cat(d, n, m->names[v - m->min]); return; }
    uisq_catu(d, n, v);
    if (m->unit) { uisq_cat(d, n, " "); uisq_cat(d, n, m->unit); } }

/* ONE font per knob for its whole range, so the value never jumps size while turning: AF_L when every value except the zero word is
 * digits / capitals / spaces and fits 54 px, else AF_M (the zero word is a mode change: it may drop to AF_M by itself). */
static const aafont_t *uisq_morph_font(const AlgoDesc *a, uint8_t id)
{   const MorphDesc *m = &a->morph[id];
    char t[16];
    for (uint32_t v = m->min; v <= m->max; v++) {
        if (v == 0 && m->zero) continue;
        uisq_morph_text(t, sizeof t, a, id, (uint8_t)v, 0);
        if (text_w(&AF_L, t) > 54) return &AF_M;
        for (const char *c = t; *c; c++) if (!((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'Z') || *c == ' ')) return &AF_M; }
    return &AF_L; }

/* position dots for a value list (<= SQ_NAMES_MAX entries), else 0 = a meter */
static uint8_t uisq_morph_dots(const AlgoDesc *a, uint8_t id)
{   const MorphDesc *m = &a->morph[id];
    const uint32_t n = (uint32_t)m->max - m->min + 1u;
    return (uint8_t)(m->names && n <= SQ_NAMES_MAX ? n : 0); }

/* One knob tile of tile column k (0..3) from the descriptor alone: label band (AF_M, T_MID), value (one font, T_ACCENT when hot), then the
 * position dots or the meter under the value. slot = row * 4 + k indexes knob_morph[]. morph[] = the channel's current morph values. */
static void uisq_knob_tile(uint32_t k, const AlgoDesc *a, uint8_t slot, const uint8_t morph[SQ_NMORPH], const void *cache, uint8_t hot)
{   const int32_t x = 1 + 60 * (int32_t)k, vy = UISQ_KNOB_Y + UISQ_KNOB_LAB + 1;
    const uint8_t id = a->knob_morph[slot & 7u], v = morph[id];
    const MorphDesc *m = &a->morph[id];
    const aafont_t *f = uisq_morph_font(a, id);
    const uint8_t dots = uisq_morph_dots(a, id);
    char t[16];
    cv_rect(x, UISQ_KNOB_Y, UISQ_KNOB_W, UISQ_KNOB_LAB, T_SURF);
    cv_rect(x, vy, UISQ_KNOB_W, UISQ_KNOB_VAL, T_SURF);
    cv_text_in(x, UISQ_KNOB_Y + 1, UISQ_KNOB_W, &AF_M, m->label, T_MID, T_SURF);
    uisq_morph_text(t, sizeof t, a, id, v, cache);
    cv_text_in(x, vy + 4, UISQ_KNOB_W, (v == 0 && m->zero) ? &AF_M : f, t, hot ? T_ACCENT : T_TEXT, T_SURF);
    if (dots) {                                                          /* a list: one dot per entry, the current one lit */
        for (uint32_t i = 0; i < dots; i++)
            cv_rect(x + (UISQ_KNOB_W - 6 * (int32_t)dots) / 2 + 6 * (int32_t)i, vy + UISQ_KNOB_VAL - 7, 4, 4, i == (uint32_t)(v - m->min) ? T_ACCENT : T_DIM); }
    else if (m->max > m->min)                                            /* a meter: (v - min) / (max - min) of 50 px */
        cv_rect(x + 4, vy + UISQ_KNOB_VAL - 6, 50 * (int32_t)(v - m->min) / (m->max - m->min), 3, hot ? T_ACCENT : T_MID); }

/* ---- per-page data: the stub pages (P4-013..P4-018 replace the entries) ---- */
typedef struct {
    void       (*input)(void);        /* the page's frame input; stubs do nothing */
    const char *cue;                  /* cue override: NULL = the generated page_states.tsv cue_strip; set only where that column is prose (HELP, MENU, FIRST_RUN, ALGO_PICK) */
    const char *st1, *st2;            /* single-row pages: the band status lines (SCREEN_SPEC 7.1); static text only */
    uint8_t     two_rows;             /* PERF: ROW 1 + ROW 2 in the band */
} UisqPage;
static void uisq_pg_stub(void) {}
static const UisqPage uisq_pages[UI_PG_COUNT] = {
    [UI_PG_PERF]      = { uisq_pg_stub, NULL, "", "", 1 },
    [UI_PG_NOTE_EDIT] = { uisq_pg_stub, NULL, "", "* = set by you (EDIT: undo)", 0 },
    [UI_PG_CHAN]      = { uisq_pg_stub, NULL, "", "D#4 = BACK", 0 },
    [UI_PG_SET]       = { uisq_pg_stub, NULL, "", "", 0 },
    [UI_PG_SCALE]     = { uisq_pg_stub, NULL, "", "", 0 },
    [UI_PG_REC]       = { uisq_pg_stub, NULL, "", "KEYS = NOTES (all 27)", 0 },
    [UI_PG_RESET]     = { uisq_pg_stub, NULL, "", "", 0 },
    [UI_PG_HELP]      = { uisq_pg_stub, "HELP: let go = back", "", "", 0 },
    [UI_PG_MENU]      = { uisq_pg_stub, "", "", "", 0 },                       /* the Felucca menu draws itself (ui_menu.c) */
    [UI_PG_FIRST_RUN] = { uisq_pg_stub, "D#4 K2: MIDI CHANNEL", "", "", 1 },
    [UI_PG_ADD_PAGE]  = { uisq_pg_stub, NULL, "", "", 1 },
    [UI_PG_MOVE_HELD] = { uisq_pg_stub, NULL, "", "", 1 },
    [UI_PG_ALGO_PICK] = { uisq_pg_stub, "", "", "", 1 },                       /* the strip is built from the algorithm list */
    [UI_PG_RIG]       = { uisq_pg_stub, NULL, "", "", 0 },
};

static const char *uisq_page_cue(uint8_t p)
{   if (p >= UI_PG_COUNT) p = UI_PG_PERF;
    if (uisq_pages[p].cue) return uisq_pages[p].cue;
    for (uint32_t i = 0; i < UI_N_PAGE_STATES; i++) if (ui_page_rows[i].id == p) return ui_page_rows[i].cue;
    return ""; }

/* the page whose zones are drawn: the overlays (FIRST_RUN, ADD_PAGE, MOVE_HELD, ALGO_PICK) sit on PERF's layout */
static uint8_t uisq_layout_page(void)
{   switch (uisq_page) {
    case UI_PG_FIRST_RUN: case UI_PG_ADD_PAGE: case UI_PG_MOVE_HELD: case UI_PG_ALGO_PICK: return UI_PG_PERF;
    default: return uisq_page < UI_PG_COUNT ? uisq_page : (uint8_t)UI_PG_PERF; } }

/* ---- public setters the pages call ---- */
static void ui_sq_page_set(uint8_t p) { if (p < UI_PG_COUNT) uisq_page = p; }
static void ui_sq_select(uint8_t ch)  { if (ch < SQ_NCH) uisq_sel = ch; }
static void ui_sq_home_held_set(uint8_t on) { uisq_home_held = on ? 1u : 0u; }
static void ui_sq_knob_touch(uint8_t k) { if (k < 4u) uisq_hot_until[k] = UISQ_MS() + UISQ_HOT_MS; }
static void ui_sq_algo_pick_touch(void) { uisq_pick_until = UISQ_MS() + UISQ_PICK_MS; }

/* SCREEN_SPEC 2.7: no queue; a new text replaces the one showing and restarts the 2000 ms; the same text only restarts the timer;
 * never over row 1 (HOME held: a toast raised then is dropped). The text is one line; the width test measures the longest. */
static void ui_sq_toast(const char *s)
{   uint32_t n = 0;
    if (uisq_home_held || !s || !s[0]) return;
    if (uisq_toast_until && uisq_streq(uisq_toast, s)) { uisq_toast_until = UISQ_MS() + UISQ_TOAST_MS; return; }
    while (s[n] && n + 1u < UISQ_TOAST_N) { uisq_toast[n] = s[n]; n++; }
    uisq_toast[n] = 0;
    uisq_toast_id++;
    uisq_toast_until = UISQ_MS() + UISQ_TOAST_MS;
    if (!uisq_toast_until) uisq_toast_until = 1u; }

/* ---- Z0 strings (also used by the width test) ---- */
static void uisq_h4_text(char *d, uint32_t n, uint8_t ch)         /* `1 ACID M1`, `1 ACID MOFF` (no CH prefix: SCREEN_SPEC 2.1) */
{   d[0] = 0;
    uisq_catu(d, n, (uint32_t)ch + 1u); uisq_cat(d, n, " "); uisq_cat(d, n, uisq_short_of(sq_ch[ch].algo)); uisq_cat(d, n, " ");
    if (sq_ch[ch].midi_ch >= SQ_MIDI_OFF) uisq_cat(d, n, "MOFF"); else { uisq_cat(d, n, "M"); uisq_catu(d, n, (uint32_t)sq_ch[ch].midi_ch + 1u); } }
static void uisq_h2_text(char *d, uint32_t n)                      /* H2 placeholder: the INT state only (the clock states are P4-012/P4-019) */
{   d[0] = 0; uisq_cat(d, n, "INT "); uisq_catu(d, n, sq_bpm); }
static void uisq_h3_text(char *d, uint32_t n)
{   d[0] = 0; uisq_cat(d, n, "P"); if (uisq_preset < 10u) uisq_cat(d, n, "0"); uisq_catu(d, n, uisq_preset); if (uisq_preset_pend) uisq_cat(d, n, "?"); }
static void uisq_cell_l1(char *d, uint32_t n, uint8_t ch)          /* Z1 line 1: `1 ACID` */
{   d[0] = 0; uisq_catu(d, n, (uint32_t)ch + 1u); uisq_cat(d, n, " "); uisq_cat(d, n, uisq_short_of(sq_ch[ch].algo)); }
static void uisq_cell_l2(char *d, uint32_t n, uint8_t ch)          /* Z1 line 2: `MIDI 2`, `MIDI OFF`, `MUTE 2`, `MUTE -` */
{   const uint8_t m = sq_ch[ch].midi_ch;
    d[0] = 0; uisq_cat(d, n, sq_ch[ch].muted ? "MUTE " : "MIDI ");
    if (m >= SQ_MIDI_OFF) uisq_cat(d, n, sq_ch[ch].muted ? "-" : "OFF"); else uisq_catu(d, n, (uint32_t)m + 1u); }

/* ---- lane data: the resolved view of the 16 steps of the viewed page (sq_eval, as the scheduler resolves them) ---- */
typedef struct { uint8_t on, vel, ed, acc, slide, rep; } UisqStep;
static void uisq_lane_read(UisqStep *L, uint8_t ch, uint8_t vpage)
{   const AlgoDesc *a = &ALGOS[sq_ch[ch].algo % SQ_NALGO];
    const uint8_t ci = sq_cidx[ch];
    for (uint8_t n = 0; n < SQ_PAGE; n++) {
        const uint8_t p = (uint8_t)(vpage * SQ_PAGE + n);
        PulseOut o;
        memset(&L[n], 0, sizeof L[n]);
        if (p >= sq_ch[ch].len || p >= SQ_NSTEP || !a->step || sq_calgo[ch][ci] != sq_ch[ch].algo) continue;
        memset(&o, 0, sizeof o);
        sq_eval(ch, p, 0, a, sq_cache[ch][ci], &sq_gdb[sq_gidx], &o);
        L[n].on = (uint8_t)((o.flags & PO_HIT) != 0u);
        L[n].vel = o.vel;
        L[n].acc = (uint8_t)(o.vel >= 100u);
        L[n].rep = (uint8_t)(((o.flags & PO_REP_MASK) >> PO_REP_SHIFT) + 1u);
        L[n].slide = (uint8_t)(((sq_ch[ch].ed[p].fl >> 2) & 3u) == 1u);
        L[n].ed = (uint8_t)((sq_ch[ch].ed[p].note | sq_ch[ch].ed[p].vel | sq_ch[ch].ed[p].gr | sq_ch[ch].ed[p].fl) != 0u); } }

static uint8_t uisq_playhead(uint8_t ch)                           /* the step now sounding (ChanRT.p is the NEXT one), 255 = stopped */
{   const uint8_t len = sq_ch[ch].len ? sq_ch[ch].len : 1u;
    if (!sq_playing) return 255u;
    return (uint8_t)((sq_rt[ch].p + len - 1u) % len); }

/* ---- Z4 geometry (SCREEN_SPEC 2.5): the cell widths are data per page, computed from the measured text ---- */
#define UISQ_MARK_W 6                    /* the hold mark (5 px) plus 1 px gap */
typedef struct { const char *tap; const char *sub; uint8_t sub_mark; uint8_t dim; uint8_t accent; } UisqBtn;

static void uisq_btn_get(UisqBtn *b, uint8_t page, uint8_t ctl, uint8_t ch)
{   const char *tap = uisq_btn_label(page, ctl, UI_MODE_TAP), *hold = uisq_btn_label(page, ctl, UI_MODE_HOLD);
    const int8_t ly = uisq_layer_of(ctl);
    const uint8_t algo = sq_ch[ch].algo % SQ_NALGO;
    b->tap = tap; b->sub = ""; b->sub_mark = 0; b->dim = 0;
    b->accent = (uint8_t)(tap[0] == 'R' && tap[1] == 'S' && tap[2] == 'T');   /* a tap label that differs from the printed word: RST.., BACK, CANCEL */
    if (uisq_streq(tap, "BACK") || uisq_streq(tap, "CANCEL")) b->accent = 1;
    if (page == UI_PG_PERF && ly >= 0 && uisq_algo_ok(algo)) {         /* the dice sub-label of the algorithm (ALGORITHM_SET 4.4) */
        b->sub = ALGOS[algo].sub[ly];
        if (ALGOS[algo].layer[ly].n == 0) { b->dim = 1; b->sub = "-"; } }
    else if (hold[0] && !uisq_streq(hold, tap)) { b->sub = hold; b->sub_mark = 1; } }   /* a hold role: the mark replaces `h:` (2.5, F2) */

static int32_t uisq_btn_need(const UisqBtn *b)                       /* ink width of the widest of the two lines */
{   int32_t a = text_w(&AF_S, b->tap), s = b->sub[0] ? text_w(&AF_S, b->sub) + (b->sub_mark ? UISQ_MARK_W : 0) : 0;
    return a > s ? a : s; }

/* w[0..5] for row 0 (FX..GLO) or row 1 (HOME..REC): the widest label + a padding of 3 (1 when the row is too tight), widened to 240,
 * the spare going to the last two cells. Returns the sum of the labels' ink widths (the test checks it against the spare). */
static void uisq_btn_widths(uint8_t page, uint8_t row, uint8_t ch, int16_t w[6])
{   int32_t sum = 0, pad = 3, spare;
    for (uint8_t i = 0; i < 6u; i++) { UisqBtn b; uisq_btn_get(&b, page, (uint8_t)(row * 6u + i), ch); w[i] = (int16_t)uisq_btn_need(&b); sum += w[i]; }
    if (sum + 6 * pad > UISQ_W) pad = 1;
    for (uint8_t i = 0; i < 6u; i++) w[i] = (int16_t)(w[i] + pad);
    spare = UISQ_W - (sum + 6 * pad);
    if (spare > 0) { w[4] = (int16_t)(w[4] + spare / 2); w[5] = (int16_t)(w[5] + spare - spare / 2); } }

/* splits `s` at its first space into two lines when it is wider than maxw; returns 1 if it split (l1, l2 filled), else 0 and l1 = s */
static uint8_t uisq_wrap(const char *s, char *l1, char *l2, uint32_t n, int32_t maxw)
{   uint32_t k = 0, j = 0;
    l1[0] = 0; l2[0] = 0;
    while (s[k] && k + 1u < n) { l1[k] = s[k]; k++; }
    l1[k] = 0;
    if (text_w(&AF_S, s) <= maxw) return 0;
    for (k = 0; s[k] && s[k] != ' '; k++) {}
    if (!s[k]) return 0;
    { uint32_t i; for (i = 0; i < k && i + 1u < n; i++) l1[i] = s[i]; l1[i] = 0; }
    k++;
    while (s[k] && j + 1u < n) l2[j++] = s[k++];
    l2[j] = 0;
    return 1; }

/* a text that must fit: the cut form if it does not (a label over its slot is the test's finding, not a draw overflow) */
static void uisq_fit(int32_t x, int32_t y, const char *s, uint16_t fg, uint16_t bg, int32_t maxw) { cv_text_fit(x, y, &AF_S, s, fg, bg, maxw); }

/* the cue strip text: ALGO_PICK lists the six algorithms with the current one in brackets, else the page's cue (11.2 rows are the pages') */
static void uisq_cue_text(char *d, uint32_t n)
{   d[0] = 0;
    if (uisq_pick_until) {
        for (uint8_t a = 0; a < SQ_NALGO; a++) {
            const uint8_t cur = (uint8_t)(a == sq_ch[uisq_sel].algo);
            if (a) uisq_cat(d, n, " ");
            if (cur) uisq_cat(d, n, "[");
            uisq_cat(d, n, uisq_short_of(a));
            if (cur) uisq_cat(d, n, "]"); }
        return; }
    if (uisq_page == UI_PG_PERF && sq_playing) { uisq_cat(d, n, "TAP STEP=ON/OFF  HOLD=EDIT NOTE"); return; }
    uisq_cat(d, n, uisq_page_cue(uisq_page)); }

/* ---- the empty frame (SCREEN_SPEC_V2 2): what the elements of P4-028 / P4-029 sit on ---- */
static void uisq_draw_frame(void)
{   for (uint32_t k = 0; k < 4u; k++) {                                 /* knob tiles: the label band, a 1 px gap, the value area (T_SURF) */
        const int32_t x = 1 + 60 * (int32_t)k;
        cv_rect(x, UISQ_KNOB_Y, UISQ_KNOB_W, UISQ_KNOB_LAB, T_SURF);
        cv_rect(x, UISQ_KNOB_Y + UISQ_KNOB_LAB + 1, UISQ_KNOB_W, UISQ_KNOB_VAL, T_SURF); }
    for (uint32_t i = 0; i < 12u; i++) {                                /* button tiles: dice tiles neutral, the rest T_SURF */
        const int32_t x = 1 + 40 * (int32_t)(i % 6u), y = UISQ_BTN_Y + (UISQ_BTN_TH + 1) * (int32_t)(i / 6u);
        cv_rect(x, y, UISQ_BTN_W, UISQ_BTN_TH, uisq_btn_dice[i] ? T_DICE : T_SURF); }
    for (uint32_t i = 0; i < 16u; i++) cv_rect(1 + 15 * (int32_t)i, UISQ_KEY_Y, UISQ_WKEY_W, UISQ_WKEY_H, T_SURF);   /* white keys */
    for (uint32_t i = 0; i < 11u; i++) {                                /* black keys, with a 1 px BG surround on both sides */
        const int32_t x = (int32_t)uisq_bkey_cx[i] - UISQ_BKEY_W / 2;
        cv_rect(x - 1, UISQ_KEY_Y, UISQ_BKEY_W + 2, UISQ_BKEY_H + 1, T_BG);
        cv_rect(x, UISQ_KEY_Y, UISQ_BKEY_W, UISQ_BKEY_H, T_RAISE); } }

/* ---- the frame ---- */
static void ui_sq_input(void)
{   uisq_now = UISQ_MS();
    if (uisq_toast_until && (int32_t)(uisq_now - uisq_toast_until) >= 0) uisq_toast_until = 0;
    if (uisq_pick_until && (int32_t)(uisq_now - uisq_pick_until) >= 0) uisq_pick_until = 0;
    if (uisq_home_held) uisq_toast_until = 0;                          /* row 1 (the HELP line) outranks a toast */
    if (uisq_sel >= SQ_NCH) uisq_sel = 0;
    if (uisq_vpage >= SQ_NSTEP / SQ_PAGE) uisq_vpage = 0;
    if (uisq_page < UI_PG_COUNT && uisq_pages[uisq_page].input) uisq_pages[uisq_page].input(); }

/* Draws the strips whose signature changed (or both when `ui.force`). Returns 1 if anything was blitted. The MENU page is the Felucca menu's.
 * Signature 0 (rows 0-119): the page, the palette (ux.gen), the selected channel, the toast in the message slot. Signature 1 (rows 120-239):
 * the same, and the footer line. The elements of P4-028 / P4-029 add their inputs to these hashes. */
static uint8_t ui_sq_draw(void)
{   uint32_t sg[UISQ_NSTRIP] = { 2166136261u, 2166136261u };
    uint8_t drew = 0;
    char cue[48];
    if (uisq_page == UI_PG_MENU) return 0;
    uisq_now = UISQ_MS();
    uisq_cue_text(cue, sizeof cue);
    for (uint32_t s = 0; s < UISQ_NSTRIP; s++) { sg[s] = uisq_h(sg[s], uisq_page); sg[s] = uisq_h(sg[s], ux.gen); sg[s] = uisq_h(sg[s], uisq_sel); }
    sg[0] = uisq_h(sg[0], uisq_toast_until ? uisq_toast_id : 0u);
    for (const char *s = cue; *s; s++) sg[1] = uisq_h(sg[1], (uint8_t)*s);
    for (uint32_t s = 0; s < UISQ_NSTRIP; s++) {
        if (!ui.force && uisq_have_sig && sg[s] == uisq_sig[s]) continue;
        cv_begin(UISQ_W, UISQ_STRIP_H, T_BG);
        cv_oy = -(int32_t)uisq_strip_y[s];                              /* the elements draw in screen coordinates */
        uisq_draw_frame();
        cv_oy = 0;
        cv_blit(0, uisq_strip_y[s]);
        uisq_sig[s] = sg[s]; drew = 1; }
    uisq_have_sig = 1;
    return drew; }
