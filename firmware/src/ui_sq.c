/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq.c: the sequencer UI framework (P4-011): the screen zones Z0..Z7 of SCREEN_SPEC 2, the toast (2.7), the always-on control
 * labels (SCREEN_SPEC 3, drawn from the generated tables build/gen/ui_tables.h), redraw by state signature, the page enum and the
 * page dispatch with stub pages. It replaces ui_t1.c's screen at the cut-over (P4-012); until then NOTHING in ui_draw.c, ui_input.c
 * or main.c calls it, and felucca.c does not include it (the host test tests/ui_sq_test.c does, after gfx.c, sq_core.c and sq_chan.c).
 *
 * Include order (unity build): after gfx.c (cv_*, text_w, AF_*, T_*), ui.c (`ui.force`), sq_core.c and sq_chan.c (the sq_* state and
 * API), and ui_tables.h (build/gen; included here, it is a pragma-once header of flash-resident const tables). Freestanding: no libc.
 *
 * State rules. The main loop calls SQ_FRAME_SYNC() (sq_chan.c, ADR-0003 6) ONCE per frame BEFORE ui_sq_input(); everything this file
 * reads is read after that, in ui_sq_input() / ui_sq_draw(). It reads only the sq_* API and the exported state the sq files
 * document for the main loop: sq_ch (the saved model), sq_gdb / sq_cache / sq_cidx (through sq_eval, as sq_gen_hit does), sq_playing,
 * sq_bpm, sq_follow, and ChanRT's `p` and `snd[0]` as READ-ONLY playhead / activity values (single bytes). It writes nothing of the
 * sequencer's; the pages (P4-013..P4-018) write through the sq_* API.
 *
 * Data, not logic (ADR-0004 addendum): a label is a table row. The knob labels come from ui_knob_rows (page x algorithm x knob), the
 * button labels from ui_button_rows (page x button x tap/hold), the black-key labels from ui_key_rows; the dice sub-labels of PERF
 * from ALGOS[].sub. The Z4 cell widths are computed from the measured text (SCREEN_SPEC 2.5). What the tables do not carry is
 * kept here as a small static table with its source named: the cue strip text of each page (page_states.tsv, column cue_strip;
 * gen_ui_tables.py does not emit it: FB filed in the task log) and the 4-letter algorithm short names (SCREEN_SPEC 2.2).
 *
 * Pixel contract: the zones tile 240x240 (SCREEN_SPEC 2). Three blits, each at most 240x124: A = Z0..Z2 (y 0..108), B = Z3..Z4
 * (y 108..212), C = Z5..Z7 (y 212..240). A blit is redrawn only when the signature of its inputs changed or `ui.force` is set. */

#include "ui_tables.h"

/* ---- hooks (the host test and the cut-over may define them first) ---- */
#ifndef UISQ_MS
#define UISQ_MS() (fm1_ms)               /* milliseconds, the clock ui_input.c's timers use */
#endif

/* ---- the zone table (SCREEN_SPEC 2; the rows tile 240x240, tests/ui_sq_test.c checks the area sum) ---- */
#define UISQ_W        240
#define UISQ_Z0_Y     0
#define UISQ_Z0_H     20
#define UISQ_Z1_Y     20
#define UISQ_Z1_H     28
#define UISQ_Z2_Y     48
#define UISQ_Z2_H     60
#define UISQ_Z3_Y     108
#define UISQ_Z3_H     52
#define UISQ_Z4_Y     160
#define UISQ_Z4_H     52
#define UISQ_Z5_Y     212
#define UISQ_Z5_H     12
#define UISQ_Z6_Y     224
#define UISQ_Z6_H     12
#define UISQ_Z7_Y     236
#define UISQ_Z7_H     4
#define UISQ_BLIT_A_H (UISQ_Z0_H + UISQ_Z1_H + UISQ_Z2_H)   /* 108 */
#define UISQ_BLIT_B_H (UISQ_Z3_H + UISQ_Z4_H)               /* 104 */
#define UISQ_BLIT_C_H (UISQ_Z5_H + UISQ_Z6_H + UISQ_Z7_H)   /* 28 */

#define UISQ_TOAST_MS 2000u              /* SCREEN_SPEC 2.7: one constant */
#define UISQ_PICK_MS  2000u              /* ALGO_PICK: the strip lists the algorithms for 2 s after the last detent */
#define UISQ_HOT_MS   600u               /* a knob stays "hot" (T_ACCENT value) this long after a turn */
#define UISQ_TOAST_N  40

/* H-slot x positions (SCREEN_SPEC 2.1) */
#define UISQ_H1_X   2
#define UISQ_H2_X   12
#define UISQ_H2T_X  23                   /* the text after the 8x8 clock dot at x=12 */
#define UISQ_H3_X   68
#define UISQ_H4_X   96
#define UISQ_H4_W   76
#define UISQ_H5_X   174
#define UISQ_H6_X   214

static const uint8_t uisq_leg_x[11] = { 0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218 };   /* LEG_X, SCREEN_SPEC 2.6 */
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
static uint32_t uisq_sig_a, uisq_sig_b, uisq_sig_c;
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

static uint8_t uisq_algo_ok(uint8_t a) { return a < SQ_NALGO && ALGOS[a].name[0] != 0; }   /* a row not yet written by its algorithm task is zero */

/* ---- per-page data: the stub pages (P4-013..P4-018 replace the entries) ---- */
typedef struct {
    void       (*input)(void);        /* the page's frame input; stubs do nothing */
    const char *cue;                  /* page_states.tsv cue_strip (priority rows 11.2 are the page tasks') */
    const char *st1, *st2;            /* single-row pages: the band status lines (SCREEN_SPEC 7.1); static text only */
    uint8_t     two_rows;             /* PERF: ROW 1 + ROW 2 in the band */
} UisqPage;
static void uisq_pg_stub(void) {}
static const UisqPage uisq_pages[UI_PG_COUNT] = {
    [UI_PG_PERF]      = { uisq_pg_stub, "PLAY: start   K1-K4: morph", "", "", 1 },
    [UI_PG_NOTE_EDIT] = { uisq_pg_stub, "STEP n: KNOBS EDIT  HOME=done", "", "* = set by you (EDIT: undo)", 0 },
    [UI_PG_CHAN]      = { uisq_pg_stub, "CH1: K1 LENGTH  OCT=+-PAGE  D#4=back", "", "D#4 = BACK", 0 },
    [UI_PG_SET]       = { uisq_pg_stub, "SET: GLO hold=HARM  HOME=back", "", "", 0 },
    [UI_PG_SCALE]     = { uisq_pg_stub, "HARM: KEYS F3-E4 = CHORD 1-7", "", "", 0 },
    [UI_PG_REC]       = { uisq_pg_stub, "REC: PLAY KEYS TO WRITE  REC=done", "", "KEYS = NOTES (all 27)", 0 },
    [UI_PG_RESET]     = { uisq_pg_stub, "RESET: TOUCH WHAT TO RESET  EDIT=no", "", "", 0 },
    [UI_PG_HELP]      = { uisq_pg_stub, "HELP: let go = back", "", "", 0 },
    [UI_PG_MENU]      = { uisq_pg_stub, "", "", "", 0 },                       /* the Felucca menu draws itself (ui_menu.c) */
    [UI_PG_FIRST_RUN] = { uisq_pg_stub, "D#4 K2: MIDI CHANNEL", "", "", 1 },
    [UI_PG_ADD_PAGE]  = { uisq_pg_stub, "ADD PAGE 2 = COPY OF 1? PG+ = yes", "", "", 1 },
    [UI_PG_MOVE_HELD] = { uisq_pg_stub, "ROLL 3x: let go to stop", "", "", 1 },
    [UI_PG_ALGO_PICK] = { uisq_pg_stub, "", "", "", 1 },                       /* the strip is built from the algorithm list */
    [UI_PG_RIG]       = { uisq_pg_stub, "RIG: TAP 1-4 = WHO DRIVES  HOLD = PICK", "", "", 0 },
};

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
    uisq_catu(d, n, (uint32_t)ch + 1u); uisq_cat(d, n, " "); uisq_cat(d, n, uisq_short[sq_ch[ch].algo % SQ_NALGO]); uisq_cat(d, n, " ");
    if (sq_ch[ch].midi_ch >= SQ_MIDI_OFF) uisq_cat(d, n, "MOFF"); else { uisq_cat(d, n, "M"); uisq_catu(d, n, (uint32_t)sq_ch[ch].midi_ch + 1u); } }
static void uisq_h2_text(char *d, uint32_t n)                      /* H2 placeholder: the INT state only (the clock states are P4-012/P4-019) */
{   d[0] = 0; uisq_cat(d, n, "INT "); uisq_catu(d, n, sq_bpm); }
static void uisq_h3_text(char *d, uint32_t n)
{   d[0] = 0; uisq_cat(d, n, "P"); if (uisq_preset < 10u) uisq_cat(d, n, "0"); uisq_catu(d, n, uisq_preset); if (uisq_preset_pend) uisq_cat(d, n, "?"); }
static void uisq_cell_l1(char *d, uint32_t n, uint8_t ch)          /* Z1 line 1: `1 ACID` */
{   d[0] = 0; uisq_catu(d, n, (uint32_t)ch + 1u); uisq_cat(d, n, " "); uisq_cat(d, n, uisq_short[sq_ch[ch].algo % SQ_NALGO]); }
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

/* ---- drawing ---- */
static void uisq_hold_mark(int32_t x, int32_t y, uint16_t c)       /* 5x7 filled down-triangle */
{   static const uint8_t wd[7] = { 5, 5, 3, 3, 3, 1, 1 };
    for (int32_t r = 0; r < 7; r++) cv_rect(x + (5 - wd[r]) / 2, y + r, wd[r], 1, c); }

static void uisq_dot(int32_t x, int32_t y, uint16_t c)             /* the 8x8 clock dot (amber, solid: INT; SCREEN_SPEC 6.3) */
{   cv_rect(x + 1, y, 6, 8, c); cv_rect(x, y + 1, 8, 6, c); }

static void uisq_draw_z0(uint8_t ch)
{   char b[24];
    const uint8_t ph = uisq_playhead(ch);
    const uint8_t len = sq_ch[ch].len;
    /* H1 transport */
    if (uisq_page == UI_PG_REC) cv_text(UISQ_H1_X, 4, &AF_S, "R", T_REC);
    else cv_text(UISQ_H1_X, 4, &AF_S, sq_playing ? ">" : ".", T_TEXT);
    /* H2 clock dot + tempo source (placeholder: INT; amber fixed colour, SCREEN_SPEC 6.3) */
    uisq_dot(UISQ_H2_X, 6, ux.light ? 0xCB80u : 0xFD80u);
    uisq_h2_text(b, sizeof b); cv_text(UISQ_H2T_X, 4, &AF_S, b, T_TEXT);
    /* H3 preset */
    uisq_h3_text(b, sizeof b); cv_text(UISQ_H3_X, 4, &AF_S, b, T_TEXT);
    /* H4 channel, algorithm, MIDI */
    uisq_h4_text(b, sizeof b, ch); cv_text_fit(UISQ_H4_X, 4, &AF_S, b, T_TEXT, T_BG, UISQ_H4_W);
    /* H5 page map as graphics: 4 cells of 9x11 at a 10 px pitch from x=174 */
    for (uint8_t k = 0; k < 4u; k++) {
        const int32_t x = UISQ_H5_X + 10 * k, y = 4;
        const uint8_t shown = (uint8_t)((uint16_t)k * SQ_PAGE < len);
        const uint8_t used = (uint8_t)((sq_ch[ch].pg_used >> k) & 1u);
        const uint8_t playing = (uint8_t)(ph != 255u && ph / SQ_PAGE == k);
        char d[2] = { (char)('1' + k), 0 };
        if (shown) {
            const uint8_t partial = (uint8_t)(k == (len - 1u) / SQ_PAGE && (len % SQ_PAGE) != 0u);
            if (playing) { cv_rect(x, y, 9, 11, T_SEL); cv_text_in(x, y - 1, 9, &AF_S, d, T_INK, T_SEL); }
            else cv_text_in(x, y - 1, 9, &AF_S, d, T_DIM, T_BG);
            if (k == uisq_vpage || playing) cv_frame(x, y, 9, 11, T_TEXT);
            else cv_frame(x, y, 9, 11, T_DIM);
            if (partial) for (int32_t i = 1; i < 8; i += 2) cv_pset(x + i, y + 10, T_BG);   /* the bottom edge dashed */
        } else if (used) {                                                                    /* a hidden page that still holds content: dotted outline */
            for (int32_t i = 0; i < 9; i += 2) { cv_pset(x + i, y, T_DIM); cv_pset(x + i, y + 10, T_DIM); }
            for (int32_t i = 0; i < 11; i += 2) { cv_pset(x, y + i, T_DIM); cv_pset(x + 8, y + i, T_DIM); }
        } else cv_rect(x + 3, y + 4, 2, 2, T_DIM); }                                          /* never existed: a 2x2 dot */
    /* H6 length */
    b[0] = 0; uisq_cat(b, sizeof b, "L"); uisq_catu(b, sizeof b, len); cv_text(UISQ_H6_X, 4, &AF_S, b, T_TEXT); }

static void uisq_draw_z1(void)
{   const uint8_t pg = uisq_page;
    if (pg == UI_PG_NOTE_EDIT || pg == UI_PG_REC) {                   /* a banner replaces the four cells; `CH n` is the SELECT label */
        char b[40]; b[0] = 0;
        uisq_cat(b, sizeof b, "CH"); uisq_catu(b, sizeof b, (uint32_t)uisq_sel + 1u);
        if (pg == UI_PG_NOTE_EDIT) { uisq_cat(b, sizeof b, " NOTE EDIT STEP "); uisq_catu(b, sizeof b, (uint32_t)uisq_edit_step + 1u); }
        else { uisq_cat(b, sizeof b, " REC writing to page "); uisq_catu(b, sizeof b, (uint32_t)uisq_vpage + 1u); }
        cv_rect(0, UISQ_Z1_Y, UISQ_W, UISQ_Z1_H, pg == UI_PG_REC ? T_RAISE : T_TINT);
        cv_text(4, UISQ_Z1_Y + 8, &AF_S, b, pg == UI_PG_REC ? T_REC : T_ACCENT);
        return; }
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        char l1[16], l2[16];
        const int32_t x = 60 * c, y = UISQ_Z1_Y;
        const uint8_t sel = (uint8_t)(c == uisq_sel), mut = sq_ch[c].muted;
        const uint16_t ink = sel ? T_INK : mut ? T_DIM : T_TEXT, ink2 = sel ? T_INK : mut ? T_DIM : T_MID;
        uisq_cell_l1(l1, sizeof l1, c); uisq_cell_l2(l2, sizeof l2, c);
        if (sel) cv_rect(x + 1, y, 58, UISQ_Z1_H, T_SEL);
        cv_text_fit(x + 2, y + 2, &AF_S, l1, ink, sel ? T_SEL : T_BG, 56);
        cv_text_fit(x + 2, y + 14, &AF_S, l2, (!mut && sq_ch[c].midi_ch >= SQ_MIDI_OFF && !sel) ? T_ACCENT : ink2, sel ? T_SEL : T_BG, 56);
        if (sq_rt[c].snd[0]) cv_rect(x + 54, y + 4, 4, 4, sel ? T_INK : T_ACCENT); } }   /* the activity dot: a note is sounding */

static void uisq_draw_z2(const UisqStep *L, uint8_t ch)
{   char b[40];
    const uint8_t len = sq_ch[ch].len, pages = (uint8_t)((len + SQ_PAGE - 1u) / SQ_PAGE), ph = uisq_playhead(ch);
    const uint16_t ink = T_MID;
    if (uisq_page == UI_PG_SCALE) { cv_text(2, UISQ_Z2_Y + 1, &AF_S, "HARMONY", T_TEXT); return; }   /* the progression strip is the SCALE task's */
    b[0] = 0;
    uisq_cat(b, sizeof b, "PG "); uisq_catu(b, sizeof b, (uint32_t)uisq_vpage + 1u); uisq_cat(b, sizeof b, "/"); uisq_catu(b, sizeof b, pages);
    uisq_cat(b, sizeof b, sq_follow ? "  FOLLOW" : "  hold"); uisq_cat(b, sizeof b, "  hold=EDIT");
    cv_text(2, UISQ_Z2_Y + 1, &AF_S, b, ink);
    if (ph != 255u) { b[0] = 0; uisq_cat(b, sizeof b, ">P"); uisq_catu(b, sizeof b, (uint32_t)(ph / SQ_PAGE) + 1u); cv_text_r(UISQ_W - 4, UISQ_Z2_Y + 1, &AF_S, b, T_ACCENT, T_BG); }
    for (uint8_t n = 0; n < SQ_PAGE; n++) {
        const int32_t x = 15 * n, y = UISQ_Z2_Y + 14;
        const uint8_t p = (uint8_t)(uisq_vpage * SQ_PAGE + n), live = (uint8_t)(p < len);
        const uint8_t edited = (uint8_t)(uisq_page == UI_PG_NOTE_EDIT && p == uisq_edit_step);
        cv_rect(x, y, 14, 46, live ? T_LANE : T_BG);
        if (live && L[n].on) {                                         /* velocity bar: lit = a step plays, height = velocity */
            const int32_t hh = 2 + (int32_t)L[n].vel * 30 / 127;
            cv_rect(x + 1, y + 32 - hh, 12, hh, L[n].acc ? T_ACCENT : T_THEME); }
        else if (live) cv_rect(x + 1, y + 30, 12, 2, T_GRID);
        if (live) {                                                    /* the glyph row: `'` edited, `!` accent, `~` slide, `x2` ratchet */
            char g[4]; g[0] = 0;
            if (L[n].rep > 1u) { g[0] = 'x'; g[1] = (char)('0' + L[n].rep); g[2] = 0; }
            else if (L[n].slide) { g[0] = '~'; g[1] = 0; }
            else if (L[n].on && L[n].acc) { g[0] = '!'; g[1] = 0; }
            else if (L[n].ed) { g[0] = '\''; g[1] = 0; }
            if (g[0]) cv_text_in(x, y + 33, 14, &AF_S, g, T_TEXT, T_LANE); }
        if (edited) cv_frame(x, y, 14, 46, T_ACCENT);
        if (live && ph != 255u && p == ph) cv_rect(x, y + 44, 14, 2, T_ACCENT); } }   /* the playhead underline */

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

static void uisq_draw_z3(uint8_t lp, uint8_t ch)                   /* canvas y = screen y - 108 */
{   const uint8_t ctx = uisq_ctx(ch), two = uisq_pages[lp].two_rows;
    const AlgoDesc *a = &ALGOS[sq_ch[ch].algo % SQ_NALGO];
    const AlgoState *st = &sq_ch[ch].st[sq_ch[ch].algo % SQ_NALGO];
    /* tag column BT: OCT- (upper) / OCT+ (lower) */
    for (uint8_t h = 0; h < 2u; h++) {
        UisqBtn b; const uint8_t ctl = (uint8_t)(h ? UI_C_B_OCTUP : UI_C_B_OCTDN);
        const int32_t y = 26 * h + 0, hh = two ? 26 : 26;
        const uint8_t live = (uint8_t)(two && uisq_row == h);
        char l1[8], l2[8];
        uisq_btn_get(&b, lp, ctl, ch);
        if (live) cv_rect(0, y, 36, hh, T_SEL);
        if (uisq_wrap(b.tap, l1, l2, sizeof l1, 33) && !b.sub[0]) {   /* `ROW 1`, `RST R1` do not fit one line of the 36 px column: two lines (SCREEN_SPEC 2.5 wrap) */
            uisq_fit(2, y + 1, l1, live ? T_INK : T_TEXT, live ? T_SEL : T_BG, 33);
            uisq_fit(2, y + 13, l2, live ? T_INK : T_TEXT, live ? T_SEL : T_BG, 33); continue; }
        uisq_fit(0, y + 1, b.tap, live ? T_INK : T_TEXT, live ? T_SEL : T_BG, 36);   /* `+PAGE` is 36 px: the whole column, no wrap point */
        if (b.sub[0]) { if (b.sub_mark) uisq_hold_mark(2, y + 15, T_MID); uisq_fit(2 + (b.sub_mark ? UISQ_MARK_W : 0), y + 13, b.sub, T_MID, live ? T_SEL : T_BG, 33 - (b.sub_mark ? UISQ_MARK_W : 0)); } }
    for (uint8_t k = 0; k < 4u; k++) {
        const int32_t x = 36 + 51 * k;
        const uint8_t hot = (uint8_t)(uisq_now < uisq_hot_until[k]);
        for (uint8_t r = 0; r < (two ? 2u : 1u); r++) {
            const uint8_t kn = (uint8_t)(r * 4u + k);
            const char *lab = uisq_knob_label(lp, ctx, kn);
            const int32_t y = two ? 26 * r : 0, ly = two ? 1 : 2, vy = two ? 13 : 16;
            const uint8_t live = (uint8_t)(!two || uisq_row == r);
            char v[12]; v[0] = 0;
            if (lp == UI_PG_PERF) {                                    /* the morph value (the algorithm's knob remap) */
                const uint8_t m = uisq_algo_ok(sq_ch[ch].algo) ? a->knob_morph[kn] : kn;
                uisq_catu(v, sizeof v, st->morph[m % SQ_NMORPH]); }
            if (two && live) cv_rect(x, y, 51, 26, T_SEL);
            uisq_fit(x + 2, y + ly, lab, (two && live) ? T_INK : live ? T_MID : T_DIM, (two && live) ? T_SEL : T_BG, 49);
            uisq_fit(x + 2, y + vy, v, (two && live) ? T_INK : hot ? T_ACCENT : live ? T_THEME : T_DIM, (two && live) ? T_SEL : T_BG, 49); } }
    if (!two) {                                                        /* the two status lines, x=36..240 (204 px) at y=136 / 148 */
        uisq_fit(38, 136 - UISQ_Z3_Y, uisq_pages[lp].st1, T_MID, T_BG, 200);
        uisq_fit(38, 148 - UISQ_Z3_Y, uisq_pages[lp].st2, T_MID, T_BG, 200); } }

static void uisq_draw_z4(uint8_t lp, uint8_t ch)                   /* canvas y = screen y - 108; buttons start at 52 */
{   for (uint8_t row = 0; row < 2u; row++) {
        int16_t w[6]; int32_t x = 0;
        uisq_btn_widths(lp, row, ch, w);
        for (uint8_t i = 0; i < 6u; i++) {
            UisqBtn b; const int32_t y = UISQ_Z3_H + 26 * row;
            uisq_btn_get(&b, lp, (uint8_t)(row * 6u + i), ch);
            cv_text_in(x, y + 1, w[i], &AF_S, b.tap, b.dim ? T_DIM : b.accent ? T_ACCENT : T_TEXT, T_BG);
            if (b.sub[0]) {
                const int32_t sw = text_w(&AF_S, b.sub) + (b.sub_mark ? UISQ_MARK_W : 0), x0 = x + (w[i] - sw) / 2;
                if (b.sub_mark) uisq_hold_mark(x0, y + 15, T_MID);
                uisq_fit(x0 + (b.sub_mark ? UISQ_MARK_W : 0), y + 13, b.sub, b.dim ? T_DIM : T_MID, T_BG, w[i]); }
            x += w[i];
            if (i < 5u) cv_rect(x - 1, y + 3, 1, 20, T_LINE); } } }

static void uisq_draw_z5(uint8_t lp)                               /* canvas y = 0 */
{   for (uint8_t i = 0; i < 11u; i++) {
        const char *lab = uisq_key_label(lp, (uint8_t)(UI_C_KEY_B01_FS3 + i), UI_GEST_TAP);
        if (!lab[0]) lab = uisq_key_label(lp, (uint8_t)(UI_C_KEY_B01_FS3 + i), UI_GEST_HOLD);   /* SOLO / FILL / ROLL have a hold role only */
        const uint8_t mut = (uint8_t)(i < SQ_NCH && sq_ch[i].muted);
        cv_rect(uisq_leg_x[i], 0, 20, UISQ_Z5_H, T_RAISE);
        cv_text_fit(uisq_leg_x[i] + 1, 0, &AF_S, lab, mut ? T_DIM : T_TEXT, T_RAISE, 18); } }

/* the cue strip text: ALGO_PICK lists the six algorithms with the current one in brackets, else the page's cue (11.2 rows are the pages') */
static void uisq_cue_text(char *d, uint32_t n)
{   d[0] = 0;
    if (uisq_pick_until) {
        for (uint8_t a = 0; a < SQ_NALGO; a++) {
            const uint8_t cur = (uint8_t)(a == sq_ch[uisq_sel].algo);
            if (a) uisq_cat(d, n, " ");
            if (cur) uisq_cat(d, n, "[");
            uisq_cat(d, n, uisq_short[a]);
            if (cur) uisq_cat(d, n, "]"); }
        return; }
    if (uisq_page == UI_PG_PERF && sq_playing) { uisq_cat(d, n, "TAP STEP=ON/OFF  HOLD=EDIT NOTE"); return; }
    uisq_cat(d, n, uisq_pages[uisq_page < UI_PG_COUNT ? uisq_page : 0].cue); }

/* AF_S ink runs from line-top + 3 (capitals) to line-top + 14 (descenders), so a line drawn at the top of the 12 px slot spills its
 * descenders into Z7 (which stays blank): the Z6 line is drawn 3 px up, capitals on the slot's top row, descenders on its last */
#define UISQ_Z6_TEXT_DY (-3)
static void uisq_draw_z6(void)                                     /* canvas y = 12 */
{   char b[48];
    const int32_t y = UISQ_Z5_H, ty = UISQ_Z5_H + UISQ_Z6_TEXT_DY;
    if (uisq_toast_until) { cv_rect(0, y, UISQ_W, UISQ_Z6_H, T_TINT); uisq_fit(4, ty, uisq_toast, T_ACCENT, T_TINT, 236); return; }
    uisq_cue_text(b, sizeof b);
    uisq_fit(0, ty, b, T_TEXT, T_BG, UISQ_W); }

/* ---- the frame ---- */
static void ui_sq_input(void)
{   uisq_now = UISQ_MS();
    if (uisq_toast_until && (int32_t)(uisq_now - uisq_toast_until) >= 0) uisq_toast_until = 0;
    if (uisq_pick_until && (int32_t)(uisq_now - uisq_pick_until) >= 0) uisq_pick_until = 0;
    if (uisq_home_held) uisq_toast_until = 0;                          /* row 1 (the HELP line) outranks a toast */
    if (uisq_sel >= SQ_NCH) uisq_sel = 0;
    if (uisq_vpage >= SQ_NSTEP / SQ_PAGE) uisq_vpage = 0;
    if (uisq_page < UI_PG_COUNT && uisq_pages[uisq_page].input) uisq_pages[uisq_page].input(); }

/* Draws the zones whose signature changed (or all when `ui.force`). Returns 1 if anything was blitted. The MENU page is the Felucca menu's. */
static uint8_t ui_sq_draw(void)
{   const uint8_t lp = uisq_layout_page(), ch = uisq_sel;
    UisqStep L[SQ_PAGE];
    uint32_t sa = 2166136261u, sb = 2166136261u, sc = 2166136261u;
    uint8_t drew = 0;
    char cue[48];
    if (uisq_page == UI_PG_MENU) return 0;
    uisq_now = UISQ_MS();
    uisq_lane_read(L, ch, uisq_vpage);
    uisq_cue_text(cue, sizeof cue);
    /* signatures: every input a zone draws from */
    sa = uisq_h(sa, uisq_page); sa = uisq_h(sa, ch); sa = uisq_h(sa, uisq_vpage); sa = uisq_h(sa, sq_playing); sa = uisq_h(sa, sq_bpm);
    sa = uisq_h(sa, uisq_preset); sa = uisq_h(sa, uisq_preset_pend); sa = uisq_h(sa, uisq_edit_step); sa = uisq_h(sa, sq_follow);
    sa = uisq_h(sa, uisq_playhead(ch)); sa = uisq_h(sa, ux.gen);
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        sa = uisq_h(sa, sq_ch[c].midi_ch); sa = uisq_h(sa, sq_ch[c].algo); sa = uisq_h(sa, sq_ch[c].muted); sa = uisq_h(sa, sq_ch[c].len);
        sa = uisq_h(sa, sq_ch[c].pg_used); sa = uisq_h(sa, sq_rt[c].snd[0] != 0); }
    for (uint8_t n = 0; n < SQ_PAGE; n++)
        sa = uisq_h(sa, (uint32_t)L[n].on | (uint32_t)L[n].vel << 1 | (uint32_t)L[n].ed << 9 | (uint32_t)L[n].acc << 10 | (uint32_t)L[n].slide << 11 | (uint32_t)L[n].rep << 12);
    sb = uisq_h(sb, lp); sb = uisq_h(sb, ch); sb = uisq_h(sb, uisq_row); sb = uisq_h(sb, sq_ch[ch].algo); sb = uisq_h(sb, ux.gen);
    for (uint8_t i = 0; i < SQ_NMORPH; i++) sb = uisq_h(sb, sq_ch[ch].st[sq_ch[ch].algo % SQ_NALGO].morph[i]);
    for (uint8_t k = 0; k < 4u; k++) sb = uisq_h(sb, uisq_now < uisq_hot_until[k]);
    sc = uisq_h(sc, lp); sc = uisq_h(sc, ux.gen); sc = uisq_h(sc, uisq_toast_until ? uisq_toast_id : 0u);
    for (const char *s = cue; *s; s++) sc = uisq_h(sc, (uint8_t)*s);
    for (uint8_t c = 0; c < SQ_NCH; c++) sc = uisq_h(sc, sq_ch[c].muted);
    if (ui.force || !uisq_have_sig || sa != uisq_sig_a) {
        cv_begin(UISQ_W, UISQ_BLIT_A_H, T_BG);
        uisq_draw_z0(ch); uisq_draw_z1(); uisq_draw_z2(L, ch);
        cv_blit(0, UISQ_Z0_Y); uisq_sig_a = sa; drew = 1; }
    if (ui.force || !uisq_have_sig || sb != uisq_sig_b) {
        cv_begin(UISQ_W, UISQ_BLIT_B_H, T_BG);
        uisq_draw_z3(lp, ch); uisq_draw_z4(lp, ch);
        cv_blit(0, UISQ_Z3_Y); uisq_sig_b = sb; drew = 1; }
    if (ui.force || !uisq_have_sig || sc != uisq_sig_c) {
        cv_begin(UISQ_W, UISQ_BLIT_C_H, T_BG);
        uisq_draw_z5(lp); uisq_draw_z6();
        cv_blit(0, UISQ_Z5_Y); uisq_sig_c = sc; drew = 1; }
    uisq_have_sig = 1;
    return drew; }
