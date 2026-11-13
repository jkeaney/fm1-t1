/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq.c: the sequencer UI: the screen v2 bands, the two-strip plan and the frame (P4-026, SCREEN_SPEC_V2 2, ADR-0006), and the live-state elements
 * header, knob tiles, status line and music row (P4-028, SCREEN_SPEC_V2 4.1-4.4: drawn from a UisqView, filled from sq_* and the descriptor alone),
 * the buttons, keybed, footer and message slot (P4-029, SCREEN_SPEC_V2 4.5, 4.6, 5), the label lookups of the generated tables build/gen/ui_tables.h,
 * redraw by state signature, the page enum and the page dispatch with stub pages. It replaced the T-1 screen at the cut-over
 * (P4-012): ui_draw.c, ui_input.c and main.c reach it through ui_sq_glue.c, and felucca.c includes both (the host test
 * tests/ui_sq_test.c includes this file alone, after gfx.c, sq_core.c and sq_chan.c).
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
 * inputs changed or `ui.force` is set. P4-028 draws the header, knob tiles, status line and music row, P4-029 the button picture, the keybed in its
 * mode, the footer line and the message slot (uisq_draw_elements, which lays the frame of tiles and keys under them first).
 *
 * The helpers below the band table (uisq_btn_*, uisq_playhead ...) are the screen-v1 button and lane code, kept until P4-029 reuses or deletes
 * them (tests/ui_sq_test.c still measures the button ones). */

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

#define UISQ_TOAST_MS 1000u              /* SCREEN_SPEC_V2 5.1: a toast is up ~1 s */
#define UISQ_DICED_MS 2000u              /* the just-diced hatch on the music row */
#define UISQ_TOUCH_MS 1000u              /* SCREEN_SPEC_V2 5.2: a touched control stays named ~1 s after the last touch */
#define UISQ_HINT_MS  250u               /* SCREEN_SPEC_V2 5.3: a hold names itself from 0.25 s */
#define UISQ_PICK_MS  2000u              /* ALGO_PICK: the strip lists the algorithms for 2 s after the last detent */
#define UISQ_HOT_MS   600u               /* a knob stays "hot" (T_ACCENT value) this long after a turn */
#define UISQ_TOAST_N  40

static const char *const uisq_short[SQ_NALGO] = { "ACID", "EUCL", "SNAK", "STAG", "ARP", "PATT" };   /* SCREEN_SPEC 2.2 */

/* ---- UI state (main loop only) ---- */
static uint8_t  uisq_page = UI_PG_PERF;  /* the page shown: ui_page_t */
static uint8_t  uisq_sel;                /* the selected channel 0..3 */
static uint8_t  uisq_vpage;              /* the viewed lane page 0..3 */
static uint8_t  uisq_row;                /* PERF knob band: 0 = ROW 1 live, 1 = ROW 2, 2 = ROW 3 (a channel with a RIG device only) */
static uint8_t  uisq_edit_step;          /* NOTE_EDIT: the step being edited, 0..63 */
static uint8_t  uisq_edit_ch;            /* NOTE_EDIT: its channel (the page closes when the selection leaves it) */
static uint8_t  uisq_ne_dirty;           /* NOTE_EDIT: a write was made since the editor opened on this step (EDIT undoes only then: never an older dice) */
static uint8_t  uisq_ne_undone;          /* NOTE_EDIT: the last EDIT tap was an undo (the next one is a redo) */
static uint8_t  uisq_home_dn;            /* HOME is down (ui_sq_home) */
static uint32_t uisq_home_t0;
static uint8_t  uisq_preset = 1;         /* header H3 slot (the preset layer is P7; a placeholder here) */
static uint8_t  uisq_preset_pend;        /* 1 = `Pnn?` */
static uint8_t  uisq_home_held;          /* a toast raised while HOME is held is dropped (SCREEN_SPEC 2.7) */
static uint32_t uisq_now;                /* UISQ_MS() of this frame */
static char     uisq_toast[UISQ_TOAST_N];     /* the toast's word (or the whole legacy one-liner) */
static char     uisq_toast_hint[18];          /* ... and how to reverse it */
static uint8_t  uisq_toast_chip;              /* 0 = none, else 1 + the channel the toast is about */
static char     uisq_touch_a[14], uisq_touch_b[18];   /* the touched control's printed name and what it does now */
static uint32_t uisq_touch_until;
static uint8_t  uisq_touch_held;              /* a held key / move: named until it is let go */
static char     uisq_hold_text[18];           /* the hold hint: the action the hold will fire */
static uint8_t  uisq_hold_on, uisq_hold_icon;
static uint32_t uisq_hold_t0;
static uint16_t uisq_hold_fire;               /* ms to the fire time; 0 = acts on release (full bar) */
static uint16_t uisq_btn_down;                /* bit i = panel button i (FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC) is pressed: drawn inverted */
static uint8_t  uisq_held_m;                  /* 0 = none, else 1 + the channel whose mute key is held */
static uint8_t  uisq_held_mv;                 /* 0 = none, 1 SOL, 2 FIL, 3 ROL: the held move key */
static uint8_t  uisq_new;                     /* bit n = channel n is new: the setup cue shows until the CHAN page clears it (ui_sq_new_set) */
static uint32_t uisq_toast_until;        /* 0 = no toast */
static uint16_t uisq_toast_id;
static uint32_t uisq_pick_until;         /* ALGO_PICK overlay end, 0 = off */
static uint32_t uisq_hot_until[4];
static uint8_t  uisq_knob_ref[4];        /* each live knob's value while it was last NOT being turned: the "old level" ticks of the turning frame */
static uint8_t  uisq_used = 0x0Fu;       /* channels in use, bit n = channel n (lifecycle, SCREEN_SPEC_V2 6: add / remove are P4-029 / the CHAN task) */
static uint8_t  uisq_clock;              /* tempo source: UISQ_CLK_INT / EXT / WAIT / LOST (the clock task, TIMING_SPEC 5, sets it) */
static uint8_t  uisq_clock_port;         /* the port an external clock came from: 0 USB, 1 TRS (the SET status line names it; ui_sq_clock_set) */
static uint16_t uisq_bpm_req;            /* SET K1: the tempo asked for (0 = none); sq_frame (ui_sq_glue.c) hands it to song.g[G_BPM], the one place tempo lives */
static uint8_t  uisq_rec;                /* REC armed (set with uisq_page == UI_PG_REC by uisq_rec_open): the header disc, the REC knob band, the cursor frame */
static uint8_t  uisq_rec_cursor = 1;     /* REC: the step being written, 1..16 of the viewed page (K1 CURSOR) */
static uint8_t  uisq_rec_acc, uisq_rec_slide, uisq_rec_oct = 2;   /* REC knobs ACC / SLIDE (0/1) and OCT (0..4 = -2..+2) */
static uint16_t uisq_diced;              /* steps of the viewed page just re-rolled (white with a hatch) until uisq_diced_until */
static uint32_t uisq_diced_until;
/* CHAN K2 (P4-015): the MIDI channel turned to and not yet committed (0..15, SQ_MIDI_OFF; UISQ_CM_NONE = none), the channel it was turned for, and the
 * time of the last detent. It commits 0.6 s after the last detent and on ANY exit from CHAN (uisq_chan_commit). */
#define UISQ_MIDI_SETTLE_MS 600u         /* SEQUENCER_UX 4.4 SQ_MIDI_SETTLE */
#define UISQ_CM_NONE        0xFFu
static uint8_t  uisq_cm_val = UISQ_CM_NONE, uisq_cm_ch;
static uint32_t uisq_cm_t0;
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

/* The text of a knob tile value: the algorithm's hook, else the zero word, else the value name, else the number (+ " " + unit).
 * `a` may be NULL (a page row: REC, CHAN, RIG) and then there is no hook; `id` is the morph id the hook is asked about. With cache NULL a
 * hook returns its WIDEST form (ADR-0007), which is what the font choice below measures. */
static void uisq_tile_text(char *d, uint32_t n, const AlgoDesc *a, uint8_t id, const MorphDesc *m, uint8_t v, const void *cache)
{   char b[SQ_VTEXT_MAX];
    d[0] = 0;
    b[0] = 0;
    if (a && a->value_text && a->value_text(id, v, cache, b) && b[0]) { uisq_cat(d, n, b); return; }
    if (v == 0 && m->zero) { uisq_cat(d, n, m->zero); return; }
    if (m->names && v >= m->min && v <= m->max) { uisq_cat(d, n, m->names[v - m->min]); return; }
    uisq_catu(d, n, v);
    if (m->unit) { uisq_cat(d, n, " "); uisq_cat(d, n, m->unit); } }
static void uisq_morph_text(char *d, uint32_t n, const AlgoDesc *a, uint8_t id, uint8_t v, const void *cache)
{   uisq_tile_text(d, n, a, id, &a->morph[id], v, cache); }

/* ONE font per knob for its whole range, so the value never jumps size while turning: AF_L when every value except the zero word is
 * digits / capitals / spaces and fits 54 px, else AF_M (the zero word is a mode change: it may drop to AF_M by itself). */
static const aafont_t *uisq_tile_font(const AlgoDesc *a, uint8_t id, const MorphDesc *m)
{   char t[16];
    for (uint32_t v = m->min; v <= m->max; v++) {
        if (v == 0 && m->zero) continue;
        uisq_tile_text(t, sizeof t, a, id, m, (uint8_t)v, 0);
        if (text_w(&AF_L, t) > 54) return &AF_M;
        for (const char *c = t; *c; c++) if (!((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'Z') || *c == ' ')) return &AF_M; }
    return &AF_L; }
static const aafont_t *uisq_morph_font(const AlgoDesc *a, uint8_t id) { return uisq_tile_font(a, id, &a->morph[id]); }

/* position dots for a value list (<= SQ_NAMES_MAX entries), else 0 = a meter */
static uint8_t uisq_tile_dots(const MorphDesc *m)
{   const uint32_t n = (uint32_t)m->max - m->min + 1u;
    return (uint8_t)(m->names && n <= SQ_NAMES_MAX ? n : 0); }
static uint8_t uisq_morph_dots(const AlgoDesc *a, uint8_t id) { return uisq_tile_dots(&a->morph[id]); }

/* ---- the page rows that belong to no algorithm (knob_map.tsv REC and CHAN rows; SCREEN_SPEC_V2 4.2). Values are stored from `min`, so
 * OCT -2..+2 is 0..4 with names (a list: dots). ---- */
static const char *const uisq_n_offon[2] = { "OFF", "ON" };
static const char *const uisq_n_own[1]   = { "OWN" };
static const char *const uisq_n_midi[17] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "OFF" };
/* REC (P4-017): the four tiles are RAM rows filled once by uisq_rec_init() (after uisq_ne_w): the labels are the knob_map REC rows, OFF / ON and the signs
 * of the octave names are the note_edit_text words. K1 CURSOR 1..16, K2 ACC and K3 SLIDE OFF / ON (lists: dots), K4 OCT -2..+2 stored 0..4 (a list). */
static MorphDesc    uisq_rec_row[4];
static char         uisq_rec_octn[5][3];
static const char  *uisq_rec_octp[5], *uisq_rec_offon[2];
static uint8_t      uisq_rec_ready;
static const MorphDesc uisq_note_row = { "", 0, 0, 0, 1, 1 };       /* NOTE_EDIT: the tiles' MorphDesc is unused (the labels are the knob_map rows); this keeps the pointers valid */
static const MorphDesc uisq_chan_row[4] = { { "LENGTH", 1, 64, 16, 1, 4 }, { "MIDI", 1, 17, 2, 1, 1, uisq_n_midi }, { "SOURCE", 0, 0, 0, 1, 1, uisq_n_own },
                                            { "FOLLOW", 0, 1, 1, 1, 1, uisq_n_offon } };
/* ROW 3 exists only for a channel with a RIG device (HwSet, HW_SUPPORT_LOWLEVEL 9): its profile's four wire names. The RIG layer is not in the
 * firmware yet, so no channel has one; the RIG task returns its four MorphDesc here and nothing else in the screen changes. */
static const MorphDesc *uisq_rig_row(uint8_t ch) { (void)ch; return 0; }


/* ---- SET (P4-016): the four tiles. Labels come from the knob_map SET rows, value names from set_text.tsv: both are table data read at run time, so the
 * MorphDesc rows are RAM (4 x 24 B + 6 pointers) filled once by uisq_set_init(). K1 BPM 40..240 (step 1, fast 5), K2 SWING 50..75 %, K3 GATE 10..100 %
 * (step 1, fast 5), K4 DIV 1/32 1/16T 1/16 1/8T 1/8 1/4 (a list: dots). ---- */
static const char *uisq_st_w(uint8_t id) { return id < UI_ST_COUNT ? ui_st_text[id] : ""; }
static MorphDesc    uisq_set_row[4];
static const char  *uisq_set_names[6];
static uint8_t      uisq_set_ready;
static void uisq_set_init(void)
{   static const uint8_t mn[4] = { 40, 50, 10, 0 }, mx[4] = { 240, 75, 100, 5 }, df[4] = { 120, 50, 50, 2 }, fa[4] = { 5, 1, 5, 1 };
    if (uisq_set_ready) return;
    for (uint8_t i = 0; i < 6u; i++) uisq_set_names[i] = uisq_st_w((uint8_t)(UI_ST_DIV0 + i));
    for (uint8_t k = 0; k < 4u; k++) {
        MorphDesc *m = &uisq_set_row[k];
        const char *l = uisq_knob_label(UI_PG_SET, UI_CTX_ANY, k);
        uint32_t n = 0;
        memset(m, 0, sizeof *m);
        while (l[n] && n + 1u < sizeof m->label) { m->label[n] = l[n]; n++; }
        m->min = mn[k]; m->max = mx[k]; m->def = df[k]; m->step = 1; m->fast = fa[k];
        if (k == 3u) m->names = uisq_set_names; }
    uisq_set_ready = 1; }

/* ---- per-page data: the stub pages (P4-013..P4-018 replace the entries) ---- */
typedef struct {
    void       (*input)(void);        /* the page's frame input; stubs do nothing */
    const char *cue;                  /* cue override: NULL = the generated page_states.tsv cue_strip; set only where that column is prose (HELP, MENU, FIRST_RUN, ALGO_PICK) */
    const char *st1, *st2;            /* single-row pages: the band status lines (SCREEN_SPEC 7.1); static text only */
    uint8_t     two_rows;             /* PERF: ROW 1 + ROW 2 in the band */
} UisqPage;
static void uisq_pg_stub(void) {}
static void uisq_pg_note_edit(void);                                /* the NOTE_EDIT page's frame input (P4-014) */
static void uisq_pg_chan(void);                                     /* the CHAN page's frame input (P4-015) */
static void uisq_pg_rec(void);                                      /* the REC page's frame input (P4-017) */
static const UisqPage uisq_pages[UI_PG_COUNT] = {
    [UI_PG_PERF]      = { uisq_pg_stub, NULL, "", "", 1 },
    [UI_PG_NOTE_EDIT] = { uisq_pg_note_edit, NULL, "", "", 0 },
    [UI_PG_CHAN]      = { uisq_pg_chan, NULL, "", "D#4 = BACK", 0 },
    [UI_PG_SET]       = { uisq_pg_stub, NULL, "", "", 0 },
    [UI_PG_SCALE]     = { uisq_pg_stub, NULL, "", "", 0 },
    [UI_PG_REC]       = { uisq_pg_rec, NULL, "", "", 0 },
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
static void uisq_chan_commit(void);
static void ui_sq_page_set(uint8_t p) { if (p < UI_PG_COUNT) { if (p != UI_PG_CHAN) uisq_chan_commit(); uisq_page = p; } }   /* leaving CHAN by any road commits its pending MIDI channel */
static void ui_sq_select(uint8_t ch)  { if (ch < SQ_NCH) uisq_sel = ch; }
static void ui_sq_home_held_set(uint8_t on) { uisq_home_held = on ? 1u : 0u; }
static void ui_sq_knob_touch(uint8_t k) { if (k < 4u) uisq_hot_until[k] = UISQ_MS() + UISQ_HOT_MS; }
static void ui_sq_diced_set(uint16_t steps) { uisq_diced = steps; uisq_diced_until = UISQ_MS() + UISQ_DICED_MS; if (!uisq_diced_until) uisq_diced_until = 1u; }
static void ui_sq_algo_pick_touch(void) { uisq_pick_until = UISQ_MS() + UISQ_PICK_MS; }

/* ---- the message slot (SCREEN_SPEC_V2 5): one slot, four sources, priority hold hint > touched control > toast > page title ----
 * A new toast replaces the one showing and restarts the timer (no queue); the same text only restarts the timer; a toast raised while HOME is
 * held is dropped. ui_sq_toast_msg(chip, word, hint) is the toast of 5.1 ([chip] word, hint small on the right); chip 0xFF = none. */
static void ui_sq_toast_msg(uint8_t chip, const char *word, const char *hint)
{   uint32_t n = 0, m = 0;
    const uint8_t c = chip < SQ_NCH ? (uint8_t)(chip + 1u) : 0u;
    if (uisq_home_held || !word || !word[0]) return;
    if (!hint) hint = "";
    if (uisq_toast_until && c == uisq_toast_chip && uisq_streq(uisq_toast, word) && uisq_streq(uisq_toast_hint, hint)) { uisq_toast_until = UISQ_MS() + UISQ_TOAST_MS; return; }
    while (word[n] && n + 1u < UISQ_TOAST_N) { uisq_toast[n] = word[n]; n++; }
    uisq_toast[n] = 0;
    while (hint[m] && m + 1u < sizeof uisq_toast_hint) { uisq_toast_hint[m] = hint[m]; m++; }
    uisq_toast_hint[m] = 0;
    uisq_toast_chip = c;
    uisq_toast_id++;
    uisq_toast_until = UISQ_MS() + UISQ_TOAST_MS;
    if (!uisq_toast_until) uisq_toast_until = 1u; }
static void ui_sq_toast(const char *s) { ui_sq_toast_msg(0xFFu, s, ""); }
/* the generated table (ui_msgs, tools/gen_ui_tables.py from MESSAGES, width-checked): a row's kind picks the call */
static void ui_sq_toast_id(uint8_t id, uint8_t chip) { if (id < UI_N_MSG) ui_sq_toast_msg(chip, ui_msgs[id].a, ui_msgs[id].b); }

static void uisq_cpy(char *d, uint32_t n, const char *s) { uint32_t k = 0; while (s && s[k] && k + 1u < n) { d[k] = s[k]; k++; } d[k] = 0; }
/* a touched control (5.2): the printed name boxed + what it does now; named while `held` (a held key) and ~1 s after the last touch */
static void ui_sq_touch(const char *name, const char *what, uint8_t held)
{   uisq_cpy(uisq_touch_a, sizeof uisq_touch_a, name); uisq_cpy(uisq_touch_b, sizeof uisq_touch_b, what);
    uisq_touch_held = held ? 1u : 0u;
    uisq_touch_until = UISQ_MS() + UISQ_TOUCH_MS; if (!uisq_touch_until) uisq_touch_until = 1u; }
static void ui_sq_touch_id(uint8_t id, uint8_t held) { if (id < UI_N_MSG) ui_sq_touch(ui_msgs[id].a, ui_msgs[id].b, held); }
static void ui_sq_touch_end(void) { uisq_touch_held = 0; uisq_touch_until = UISQ_MS() + UISQ_TOUCH_MS; if (!uisq_touch_until) uisq_touch_until = 1u; }

/* a hold hint (5.3): from UISQ_HINT_MS after the press it shows the icon + the action; the bar fills to `fire_ms` (0 = acts on release: full) */
static uint16_t uisq_remove_ms(uint8_t ctl)                        /* the M-key hold that removes a channel: the "held 1.5 s: REMOVE" clause of key_map.tsv (decision 125) */
{   for (uint32_t i = 0; i < UI_N_REMOVE; i++) if (ui_remove_rows[i].ctl == ctl) return ui_remove_rows[i].ms;
    return 0; }
static uint16_t uisq_hold_ms(uint8_t ctl)                          /* the "0.4 s" of the hold row of button_modes.tsv / key_map.tsv; 0 = none */
{   for (uint32_t i = 0; i < UI_N_HOLD; i++) if (ui_hold_rows[i].ctl == ctl) return ui_hold_rows[i].ms;
    return 0; }
static void ui_sq_hold_begin(const char *text, uint8_t icon, uint16_t fire_ms)
{   uisq_cpy(uisq_hold_text, sizeof uisq_hold_text, text); uisq_hold_icon = icon; uisq_hold_fire = fire_ms;
    uisq_hold_on = 1; uisq_hold_t0 = UISQ_MS(); }
static void ui_sq_hold_ctl(uint8_t id, uint8_t icon, uint8_t ctl) { if (id < UI_N_MSG) ui_sq_hold_begin(ui_msgs[id].a, icon, uisq_hold_ms(ctl)); }
static void ui_sq_hold_end(void) { uisq_hold_on = 0; }

static void ui_sq_btn_down(uint8_t i, uint8_t on) { if (i < 12u) { if (on) uisq_btn_down |= (uint16_t)(1u << i); else uisq_btn_down &= (uint16_t)~(1u << i); } }
static void ui_sq_held_m_set(uint8_t ch1) { uisq_held_m = ch1 <= SQ_NCH ? ch1 : 0u; }
static void ui_sq_held_mv_set(uint8_t mv) { uisq_held_mv = mv <= 3u ? mv : 0u; }
/* the tempo source the header and SET show: src UISQ_CLK_INT / EXT / WAIT / LOST, port 0 USB / 1 TRS. P4-031 (the external clock) calls it from the main loop; the
 * clock task owns the transitions, this file only draws them. A value out of range is INT. */
static void ui_sq_clock_set(uint8_t src, uint8_t port) { uisq_clock = src <= 3u ? src : 0u; uisq_clock_port = port ? 1u : 0u; }
static void ui_sq_new_set(uint8_t ch, uint8_t on) { if (ch < SQ_NCH) { if (on) uisq_new |= (uint8_t)(1u << ch); else uisq_new &= (uint8_t)~(1u << ch); } }

/* ---- lane data: the resolved view of the 16 steps of the viewed page (sq_eval, as the scheduler resolves them) ---- */
typedef struct { uint8_t on, vel, ed, acc, slide, rep, note; } UisqStep;
static void uisq_step_read(UisqStep *L, uint8_t ch, uint8_t p)      /* one step as the scheduler resolves it (zero when hidden by LENGTH or no cache) */
{   const AlgoDesc *a = &ALGOS[sq_ch[ch].algo % SQ_NALGO];
    const uint8_t ci = sq_cidx[ch];
    PulseOut o;
    memset(L, 0, sizeof *L);
    if (p >= sq_ch[ch].len || p >= SQ_NSTEP || !a->step || sq_calgo[ch][ci] != sq_ch[ch].algo) return;
    memset(&o, 0, sizeof o);
    sq_eval(ch, p, 0, a, sq_cache[ch][ci], &sq_gdb[sq_gidx], &o);
    L->on = (uint8_t)((o.flags & PO_HIT) != 0u);
    L->vel = o.vel;
    L->note = o.note;
    L->acc = (uint8_t)(o.vel >= 100u);
    L->rep = (uint8_t)(((o.flags & PO_REP_MASK) >> PO_REP_SHIFT) + 1u);
    L->slide = (uint8_t)(((sq_ch[ch].ed[p].fl >> 2) & 3u) == 1u);
    L->ed = (uint8_t)((sq_ch[ch].ed[p].note | sq_ch[ch].ed[p].vel | sq_ch[ch].ed[p].gr | sq_ch[ch].ed[p].fl) != 0u); }
static void uisq_lane_read(UisqStep *L, uint8_t ch, uint8_t vpage)
{   for (uint8_t n = 0; n < SQ_PAGE; n++) uisq_step_read(&L[n], ch, (uint8_t)(vpage * SQ_PAGE + n)); }

static uint8_t uisq_playhead(uint8_t ch)                           /* the step now sounding (ChanRT.p is the NEXT one), 255 = stopped */
{   const uint8_t len = sq_ch[ch].len ? sq_ch[ch].len : 1u;
    if (!sq_playing) return 255u;
    return (uint8_t)((sq_rt[ch].p + len - 1u) % len); }

/* ---- NOTE_EDIT: the edited step as values (P4-014; SEQUENCER_UX 1.1, 1.2, 2.2, 4.1, F-10, F-12) ----
 * Every field of a step is AUTO (it follows the algorithm) or set (a StepEdit field). The AUTO end of a knob is its LOW end: turning down past the
 * lowest value writes AUTO, which clears the field. The first detent from AUTO starts at the GENERATED value rounded to the knob's grid and moves one
 * step in the turning direction (F-12), so the generated value is read from the algorithm's own step() with no edit applied. All words come from
 * ui_ne_text (docs/design/note_edit_text.tsv); only digits and their signs are built here. */
#define UISQ_ACC_VEL   120u              /* the velocity ENV sets (SEQUENCER_UX ACC_VEL) */
#define UISQ_ACC_THR   100u              /* VELO at or above this reads ACC (ACC_THR; the TB-3 accent) */
#define UISQ_NOACC_VEL 80u               /* the explicit non-accent velocity ENV falls back to when AUTO would accent again (F-10) */
#define UISQ_PITCH_MAX 96                /* PITCH: semitones above 36 + KEY + 12 OCTAVE (sq_step_set) */

static const char *uisq_ne_w(uint8_t id) { return id < UI_NE_COUNT ? ui_ne_text[id] : ""; }
static void uisq_rec_init(void)
{   if (uisq_rec_ready) return;
    uisq_rec_offon[0] = uisq_ne_w(UI_NE_OFF); uisq_rec_offon[1] = uisq_ne_w(UI_NE_ON);
    for (uint8_t i = 0; i < 5u; i++) {                                  /* -2 -1 0 +1 +2: the table's sign words and one digit */
        char *d = uisq_rec_octn[i];
        const int32_t o = (int32_t)i - 2;
        uint32_t n = 0;
        if (o) { const char *sg = uisq_ne_w(o > 0 ? UI_NE_PLUS : UI_NE_MINUS); d[n++] = sg[0] ? sg[0] : ' '; }
        d[n++] = (char)('0' + (o < 0 ? -o : o)); d[n] = 0;
        uisq_rec_octp[i] = d; }
    for (uint8_t k = 0; k < 4u; k++) {
        static const uint8_t mn[4] = { 1, 0, 0, 0 }, mx[4] = { 16, 1, 1, 4 }, df[4] = { 1, 0, 0, 2 };
        MorphDesc *m = &uisq_rec_row[k];
        const char *l = uisq_knob_label(UI_PG_REC, UI_CTX_ANY, k);
        uint32_t n = 0;
        memset(m, 0, sizeof *m);
        while (l[n] && n + 1u < sizeof m->label) { m->label[n] = l[n]; n++; }
        m->min = mn[k]; m->max = mx[k]; m->def = df[k]; m->step = 1; m->fast = 1;
        if (k == 1u || k == 2u) m->names = uisq_rec_offon;
        if (k == 3u) m->names = uisq_rec_octp; }
    uisq_rec_ready = 1; }

/* the generator's step p with no edit applied (what AUTO means); 0 when the channel has no usable cache for its algorithm */
static uint8_t uisq_ne_gen(uint8_t ch, uint8_t p, PulseOut *o)
{   const AlgoDesc *a;
    uint8_t ci;
    memset(o, 0, sizeof *o);
    if (ch >= SQ_NCH || p >= SQ_NSTEP || sq_ch[ch].algo >= SQ_NALGO) return 0;
    a = &ALGOS[sq_ch[ch].algo];
    ci = sq_cidx[ch];
    if (!a->step || sq_calgo[ch][ci] != sq_ch[ch].algo) return 0;
    a->step(sq_cache[ch][ci], sq_gen_pos(&sq_ch[ch], p), (a->flags & AF_CHORD) ? &sq_hc_inert : NULL, o);
    o->note &= 0x7Fu;
    if (o->vel < 1u) o->vel = 1u;
    if (o->vel > 127u) o->vel = 127u;
    if (o->gate < 1u) o->gate = 1u;
    if (o->gate > 100u) o->gate = 100u;
    return 1; }

/* PITCH helpers. A set pitch is semitones above base = 36 + KEY + 12 OCTAVE (unsnapped), the scale snaps at play (sq_note). A scale degree is a
 * semitone r whose snap[r] is r. */
static int16_t uisq_ne_base(const SeqGlobal *g) { return (int16_t)(36 + g->s.key + 12 * g->s.octave); }
static int16_t uisq_ne_gen_st(const SeqGlobal *g, uint8_t gnote)      /* the generated note as a semitone offset, on a scale tone, 0..PITCH_MAX */
{   int16_t st = (int16_t)((int16_t)gnote - uisq_ne_base(g)), r;
    if (st < 0) st = 0;
    if (st > UISQ_PITCH_MAX) st = UISQ_PITCH_MAX;
    r = (int16_t)(st % 12);
    st = (int16_t)(st - r + g->snap[r]);
    return st > UISQ_PITCH_MAX ? (int16_t)UISQ_PITCH_MAX : st; }
static int16_t uisq_ne_degree(const SeqGlobal *g, int16_t st, int8_t dir)   /* one scale degree up / down; -1 = below the lowest (AUTO); st at the top end */
{   for (int16_t s = (int16_t)(st + dir); s >= 0 && s <= UISQ_PITCH_MAX; s = (int16_t)(s + dir)) if (g->snap[s % 12] == (uint8_t)(s % 12)) return s;
    return dir > 0 ? st : (int16_t)-1; }

/* "G2" for a MIDI note (C4 = 60: the octave is n / 12 - 1); a negative octave is built from the sign word */
static void uisq_ne_note_name(char *d, uint32_t n, uint8_t note)
{   const int32_t oct = (int32_t)(note / 12u) - 1;
    d[0] = 0;
    uisq_cat(d, n, uisq_ne_w((uint8_t)(UI_NE_NOTE_C + note % 12u)));
    if (oct < 0) uisq_cat(d, n, uisq_ne_w(UI_NE_MINUS));
    uisq_catu(d, n, (uint32_t)(oct < 0 ? -oct : oct)); }
static void uisq_ne_gate_text(char *d, uint32_t n, uint8_t gt)         /* GATE field 1..10 = 10..100, 11 = TIE */
{   d[0] = 0;
    if (gt >= 11u) uisq_cat(d, n, uisq_ne_w(UI_NE_TIE)); else uisq_catu(d, n, (uint32_t)gt * 10u); }
static void uisq_ne_vel_text(char *d, uint32_t n, uint8_t vel)
{   d[0] = 0;
    if (vel >= UISQ_ACC_THR) uisq_cat(d, n, uisq_ne_w(UI_NE_ACC)); else uisq_catu(d, n, vel); }
static void uisq_ne_rat_text(char *d, uint32_t n, uint8_t r)
{   d[0] = 0; uisq_catu(d, n, r); }
static void uisq_ne_gen_line(char *d, uint32_t n, const char *v) { d[0] = 0; uisq_cat(d, n, uisq_ne_w(UI_NE_GEN)); uisq_cat(d, n, " "); uisq_cat(d, n, v); }

/* the generated GATE field of a step: its gate rounded to the 10 % grid (at least 10), or TIE */
static uint8_t uisq_ne_gen_gate(const PulseOut *o)
{   uint32_t g10 = ((uint32_t)o->gate + 5u) / 10u;
    if (o->flags & PO_TIE) return 11u;
    return (uint8_t)(g10 < 1u ? 1u : g10 > 10u ? 10u : g10); }
static uint8_t uisq_ne_gen_rep(const PulseOut *o) { return (uint8_t)(((o->flags & PO_REP_MASK) >> PO_REP_SHIFT) + 1u); }

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

/* ---- the view: everything the elements draw from, in one struct (SCREEN_SPEC_V2 4.1-4.4) ----
 * uisq_view_fill() builds it from the sequencer state (the only reader of sq_* / ChanSave here); the draw functions read nothing else, so the
 * host test (tests/ui_sq_test.c) can hand them the state of any scene of the v12 mock and compare pixels. The signature of a strip is the hash of
 * the bytes of the view before its pointers. */
#define UISQ_CLK_INT  0
#define UISQ_CLK_EXT  1
#define UISQ_CLK_WAIT 2
#define UISQ_CLK_LOST 3
#define UISQ_BAND_ROW  0                 /* the knob tiles show the algorithm's live row (view.row) */
#define UISQ_BAND_REC  1                 /* REC: CURSOR ACC SLIDE OCT, in white */
#define UISQ_BAND_CHAN 2                 /* CHAN: LENGTH MIDI SOURCE FOLLOW */
#define UISQ_BAND_SET  4                 /* SET: BPM SWING GATE DIV, all channels (P4-016); K1 is HOST n while an external clock runs */
#define UISQ_BAND_NOTE 3                 /* NOTE_EDIT: PITCH GATE VELO RATCH of the edited step, each AUTO or set (P4-014) */
#define UISQ_NO_PH     255               /* view.ph: no playhead column */
#define UISQ_WHITE     0xFFFFu           /* SCREEN_SPEC_V2 3: pointer, selection bar, accent cap, message slot */
#define UISQ_INK       0x0000u           /* ... and the ink on chips */
/* the message slot as drawn: kind UI_MK_*, chip 0 or 1 + channel, icon (a hold: SQI_*), frac 0..100 (a hold: the bar), a = word / printed name / action, b = hint / what it does */
typedef struct { uint8_t kind, chip, icon, frac; char a[34], b[18]; } UisqMsg;
typedef struct {
    uint8_t  playing, rec, clock, used, sel;           /* transport; tempo source UISQ_CLK_*; channels in use (bit n); selected channel */
    uint16_t bpm;
    uint8_t  ch_algo[SQ_NCH], ch_midi[SQ_NCH], ch_muted[SQ_NCH];   /* ch_midi: 0..15, SQ_MIDI_OFF = OFF */
    uint8_t  band, row, nrow;                          /* knob band UISQ_BAND_*; live row 0..2; rows in the band (2, or 3 with a RIG device) */
    uint8_t  val[4], tile_id[4];                       /* the four tiles: the value and the morph id (the hook's argument) */
    uint8_t  turn, turn_old;                           /* 0 = none, else 1 + the knob being turned; its level before the turn, 0..100 % */
    char     txt[4][12];                               /* the tiles' value texts (uisq_view_texts) */
    uint8_t  preset, pend, pages, vpage, ppage, follow_off;   /* preset Pnn, pending Pnn (0 = none); pages in use; viewed / playing page (1-based) */
    uint8_t  ph, len16, cursor;                        /* playhead column 0..15 or UISQ_NO_PH; steps of the viewed page inside LENGTH (16 = all); REC cursor 1..16 */
    uint16_t diced;                                    /* just-diced steps of the selected channel (bit n = step n of the page) */
    uint16_t hits[SQ_NCH];                             /* per channel: bit n = a hit at step n of the viewed page (the other channels' strips) */
    UisqStep lane[SQ_PAGE];                            /* the selected channel's 16 steps: on, note, acc, slide */
    /* ---- NOTE_EDIT (P4-014): the edited step as the tiles, the status line and the keybed draw it; all zero on the other pages ---- */
    uint8_t  edit, ne_step, ne_set;                    /* the edited step's column 1..16 of the viewed page (0 = not on it); its number 1..64; bit k = knob k is set (else AUTO) */
    char     ne_txt[4][8], ne_unit[4][2], ne_sub[4][12];   /* per knob: the BIG value (the set one, else the one the algorithm gives), its small unit (% / x) and the line under it (AUTO, or `GEN F2` for a set value) */
    uint8_t  ne_btn;                                   /* the toggle buttons that are ON for the step: bit 0 ENV (accent), 1 LFO (prob), 2 ARP (slide), 3 SEQ (step on) */
    char     ne_chip[4][10];                           /* the step's other set fields, one chip each: ON/OFF, SLIDE, PROB 50, OCT+1 ("" = AUTO, no chip) */
    uint16_t btn_lit, btn_dim;                         /* P4-029: buttons pressed (inverted) and dice buttons whose layer the algorithm lacks (bit = panel index) */
    int8_t   octave;                                   /* SET: the global OCTAVE -2..+2 (status line) */
    uint8_t  port;                                     /* SET: the external clock's port, 0 USB, 1 TRS */
    uint8_t  held_m, held_mv, cue;                     /* held mute key (1 + channel), held move key (1 SOL 2 FIL 3 ROL), the setup cue (new channel or MIDI OFF) */
    /* ---- the message slot: hashed by strip 0 only (it lives in y 0-21), so a toast or a filling hold bar never redraws strip 1 ---- */
    UisqMsg  msg;
    /* ---- pointers: not in the signature ---- */
    const AlgoDesc  *algo;                             /* the selected channel's algorithm (value_text hook, lane type, absent layers) */
    const MorphDesc *tile[4];
    const void      *cache;                            /* its cache, for the hook */
} UisqView;

/* the algorithm whose value_text hook a tile may call: only the algorithm's own rows (ROW 1, ROW 2); REC, CHAN and RIG rows have none */
static const AlgoDesc *uisq_view_hook(const UisqView *v) { return (v->band == UISQ_BAND_ROW && v->row < 2u) ? v->algo : 0; }
static void uisq_view_texts(UisqView *v)
{   if (v->band == UISQ_BAND_SET) {                                    /* SET: BPM, SWING %, GATE %, DIV name; while an external clock runs K1 mirrors the host */
        for (uint32_t k = 0; k < 4u; k++) {
            v->txt[k][0] = 0;
            if (k == 0u && v->clock != 0u) { if (v->clock == 1u) uisq_catu(v->txt[0], sizeof v->txt[0], v->bpm); else uisq_cat(v->txt[0], sizeof v->txt[0], uisq_st_w(UI_ST_DASH)); }
            else if (k == 3u) uisq_cat(v->txt[3], sizeof v->txt[3], uisq_set_names[v->val[3] < 6u ? v->val[3] : 5u]);
            else { uisq_catu(v->txt[k], sizeof v->txt[k], v->val[k]); if (k) uisq_cat(v->txt[k], sizeof v->txt[k], uisq_st_w(UI_ST_PCT)); } }
        return; }
    for (uint32_t k = 0; k < 4u; k++) uisq_tile_text(v->txt[k], sizeof v->txt[k], uisq_view_hook(v), v->tile_id[k], v->tile[k], v->val[k], v->cache); }

static uint32_t uisq_view_sig(const UisqView *v)                   /* everything but the message slot: the signature of strip 1 */
{   const uint8_t *b = (const uint8_t *)v;
    uint32_t h = 2166136261u;
    for (uint32_t i = 0; i < offsetof(UisqView, msg); i++) h = uisq_h(h, b[i]);
    return h; }
static uint32_t uisq_msg_sig(const UisqView *v)
{   const uint8_t *b = (const uint8_t *)&v->msg;
    uint32_t h = 2166136261u;
    for (uint32_t i = 0; i < sizeof v->msg; i++) h = uisq_h(h, b[i]);
    return h; }

/* the panel order of the 12 buttons -> control ids, and their icons (SCREEN_SPEC_V2 4.5) */
static const uint8_t uisq_btn_ctl[12]  = { UI_C_B_FX, UI_C_B_SCL, UI_C_B_ENV, UI_C_B_LFO, UI_C_B_EDIT, UI_C_B_GLO,
                                           UI_C_B_HOME, UI_C_B_SAVE, UI_C_B_ARP, UI_C_B_SEQ, UI_C_B_PLAY, UI_C_B_REC };
static const uint8_t uisq_btn_icon[12] = { SQI_DICE_ALL, SQI_DICE_NOTES, SQI_DICE_ACCNT, SQI_DICE_SHAPE, SQI_UNDO, SQI_SETTINGS,
                                           SQI_HOME, SQI_SAVE, SQI_DICE_ARTIC, SQI_DICE_HITS, SQI_PLAY, SQI_REC_RING };
/* the dice buttons whose layer the algorithm does not have (layer[].n == 0): drawn DIM. FX (ALL) is never dim. A descriptor not yet written has no layers: nothing dims. */
static uint16_t uisq_btn_dimmask(const AlgoDesc *a)
{   uint16_t m = 0;
    if (!a || !a->name[0]) return 0;
    for (uint32_t i = 0; i < 12u; i++) { const int8_t ly = uisq_layer_of(uisq_btn_ctl[i]); if (ly >= 0 && a->layer[ly].n == 0) m |= (uint16_t)(1u << i); }
    return m; }

/* the message slot (5): hold hint > touched control > toast > page title. `now` is a parameter so the host test can walk the timers. */
static void uisq_msg_pick(UisqMsg *m, uint32_t now, uint8_t page, uint8_t sel)
{   memset(m, 0, sizeof *m);
    if (uisq_hold_on && (int32_t)(now - uisq_hold_t0) >= (int32_t)UISQ_HINT_MS) {
        const uint32_t el = now - uisq_hold_t0;
        m->kind = UI_MK_HOLD; m->icon = uisq_hold_icon; uisq_cpy(m->a, sizeof m->a, uisq_hold_text);
        m->frac = (uint8_t)(!uisq_hold_fire || el >= uisq_hold_fire ? 100u : 100u * el / uisq_hold_fire); }
    else if (uisq_touch_a[0] && (uisq_touch_held || (uisq_touch_until && (int32_t)(now - uisq_touch_until) < 0))) {
        m->kind = UI_MK_TOUCH; uisq_cpy(m->a, sizeof m->a, uisq_touch_a); uisq_cpy(m->b, sizeof m->b, uisq_touch_b); }
    else if (uisq_toast_until && (int32_t)(now - uisq_toast_until) < 0) {
        m->kind = UI_MK_TOAST; m->chip = uisq_toast_chip; uisq_cpy(m->a, sizeof m->a, uisq_toast); uisq_cpy(m->b, sizeof m->b, uisq_toast_hint); }
    else if (page == UI_PG_SET) {                                      /* SET names itself: GLO  ALL CHANNELS (every value here is global) */
        m->kind = UI_MK_TOUCH; uisq_cpy(m->a, sizeof m->a, ui_msgs[UI_MSG_ENC_GLO_ALL_CHANNELS].a); uisq_cpy(m->b, sizeof m->b, ui_msgs[UI_MSG_ENC_GLO_ALL_CHANNELS].b); }
    else if (page == UI_PG_CHAN) {                                     /* a page names itself while it is open */
        const char *t = ui_msgs[UI_MSG_ENC_CHN_CH2_SETUP].b;               /* `CH2 SETUP`: the table row with the channel's digit put in */
        m->kind = UI_MK_TOUCH; uisq_cpy(m->a, sizeof m->a, ui_msgs[UI_MSG_ENC_CHN_CH2_SETUP].a);
        for (uint32_t i = 0; *t && i + 1u < sizeof m->b; i++, t++) m->b[i] = (*t >= '0' && *t <= '9') ? (char)('1' + sel) : *t; } }

static const char *uisq_ne_unit(uint8_t k, const char *val)                /* the small unit after a number: GATE %, RATCH x (the big font has neither); TIE and ACC have none */
{   const uint8_t num = (uint8_t)(val[0] >= '0' && val[0] <= '9');
    if (num && k == 1u) return uisq_ne_w(UI_NE_UNIT_PCT);
    if (num && k == 3u) return uisq_ne_w(UI_NE_UNIT_X);
    return ""; }
/* one tile: `val` is the text of the set value, `gen` the one the algorithm gives; a set tile shows its own value big and `GEN <gen>` under it, an AUTO tile
 * shows the generated value big (grey) and the word AUTO under it */
static void uisq_ne_tile(UisqView *v, uint8_t k, uint8_t set, const char *val, const char *gen, uint8_t have)
{   char b[12];
    const char *shown = set ? val : gen;
    if (set) v->ne_set |= (uint8_t)(1u << k);
    if (set || have) { uisq_cpy(v->ne_txt[k], sizeof v->ne_txt[k], shown); uisq_cpy(v->ne_unit[k], sizeof v->ne_unit[k], uisq_ne_unit(k, shown)); }
    if (!set) uisq_cpy(v->ne_sub[k], sizeof v->ne_sub[k], uisq_ne_w(UI_NE_AUTO));
    else if (have) { uisq_ne_gen_line(b, sizeof b, gen); uisq_cat(b, sizeof b, uisq_ne_unit(k, gen)); uisq_cpy(v->ne_sub[k], sizeof v->ne_sub[k], b); } }

/* the four tiles, the chips and the framed column of the edited step, into the view (all zero when the page is not NOTE_EDIT) */
static void uisq_ne_fill(UisqView *v, uint8_t vp)
{   const uint8_t ch = uisq_edit_ch < SQ_NCH ? uisq_edit_ch : 0u, p = uisq_edit_step < SQ_NSTEP ? uisq_edit_step : 0u;
    const StepEdit *e = &sq_ch[ch].ed[p];
    const SeqGlobal *g = &sq_gdb[sq_gidx];
    const uint8_t gt = (uint8_t)(e->gr & 15u), rt = (uint8_t)((e->gr >> 4) & 7u), on = (uint8_t)(e->fl & 3u), sl = (uint8_t)((e->fl >> 2) & 3u), oc = (uint8_t)(e->fl >> 5);
    PulseOut o;
    char b[8], c[8];
    UisqStep cur;
    b[0] = 0;
    uint8_t have = uisq_ne_gen(ch, p, &o), nc = 0;
    v->ne_step = (uint8_t)(p + 1u);
    v->edit = (p / SQ_PAGE == vp) ? (uint8_t)(p % SQ_PAGE + 1u) : 0u;
    c[0] = 0;
    {   /* the OCT edit shifts the sounding note: the tile shows the note that WILL play (a chip alone would contradict `C2`) */
        const int32_t sh = (oc >= 1u && oc <= 5u) ? 12 * ((int32_t)oc - 3) : 0;
        int32_t n;
        if (have) { n = (int32_t)o.note + sh; uisq_ne_note_name(c, sizeof c, (uint8_t)(n < 0 ? 0 : n > 127 ? 127 : n)); }
        if (e->note & 0x80u) { n = (int32_t)sq_note(g, (int16_t)(e->note & 0x7Fu)) + sh; uisq_ne_note_name(b, sizeof b, (uint8_t)(n < 0 ? 0 : n > 127 ? 127 : n)); } }
    uisq_ne_tile(v, 0, (uint8_t)((e->note & 0x80u) != 0u), b, c, have);                                              /* K1 PITCH */
    if (gt >= 1u && gt <= 11u) uisq_ne_gate_text(b, sizeof b, gt);
    if (have) uisq_ne_gate_text(c, sizeof c, uisq_ne_gen_gate(&o));
    uisq_ne_tile(v, 1, (uint8_t)(gt >= 1u && gt <= 11u), b, c, have);                                                 /* K2 GATE */
    if (e->vel) uisq_ne_vel_text(b, sizeof b, e->vel);
    if (have) uisq_ne_vel_text(c, sizeof c, o.vel);
    uisq_ne_tile(v, 2, (uint8_t)(e->vel != 0u), b, c, have);                                                          /* K3 VELO */
    if (rt >= 1u && rt <= 4u) uisq_ne_rat_text(b, sizeof b, rt);
    if (have) uisq_ne_rat_text(c, sizeof c, uisq_ne_gen_rep(&o));
    uisq_ne_tile(v, 3, (uint8_t)(rt >= 1u && rt <= 4u), b, c, have);                                                  /* K4 RATCH */
    /* the toggle buttons' state: what the step does now (the resolved step), not only what was set */
    uisq_step_read(&cur, ch, p);
    if (cur.acc) v->ne_btn |= 1u;
    if (e->fl & 0x10u) v->ne_btn |= 2u;
    if (sl == 1u) v->ne_btn |= 4u;
    if (cur.on) v->ne_btn |= 8u;
    /* the chips: a field that is set and has no toggle on a button (OFF, NO SLD, PROB 50, OCT+1); four at most */
    if (on == 2u) uisq_cpy(v->ne_chip[nc++], sizeof v->ne_chip[0], uisq_ne_w(UI_NE_OFF));
    if (sl == 2u) uisq_cpy(v->ne_chip[nc++], sizeof v->ne_chip[0], uisq_ne_w(UI_NE_NO_SLIDE));
    if (e->fl & 0x10u) { uisq_cpy(v->ne_chip[nc], sizeof v->ne_chip[0], uisq_ne_w(UI_NE_PROB)); uisq_cat(v->ne_chip[nc], sizeof v->ne_chip[0], " 50"); nc++; }
    if (oc >= 1u && oc <= 5u && oc != 3u) {
        const int32_t sh = (int32_t)oc - 3;
        uisq_cpy(v->ne_chip[nc], sizeof v->ne_chip[0], uisq_ne_w(UI_NE_OCT));
        uisq_cat(v->ne_chip[nc], sizeof v->ne_chip[0], uisq_ne_w(sh > 0 ? UI_NE_PLUS : UI_NE_MINUS));
        uisq_catu(v->ne_chip[nc], sizeof v->ne_chip[0], (uint32_t)(sh > 0 ? sh : -sh)); nc++; } }

static void uisq_view_fill(UisqView *v)
{   const uint8_t sel = uisq_sel < SQ_NCH ? uisq_sel : 0u;
    const ChanSave *c = &sq_ch[sel];
    const uint8_t aidx = c->algo % SQ_NALGO;
    const AlgoDesc *a = &ALGOS[aidx];
    const MorphDesc *rig = uisq_rig_row(sel);
    const uint8_t len = c->len ? c->len : 1u;
    const uint8_t pp = sq_playing ? (uint8_t)((sq_rt[sel].p + len - 1u) % len) : 255u;     /* the step now sounding */
    uint8_t vp = uisq_vpage;                                                               /* the viewed page, 0-based */
    uint32_t best = 0;
    memset(v, 0, sizeof *v);
    v->playing = sq_playing; v->rec = uisq_rec; v->clock = uisq_clock; v->used = uisq_used; v->sel = sel; v->bpm = sq_bpm;
    for (uint8_t i = 0; i < SQ_NCH; i++) { v->ch_algo[i] = sq_ch[i].algo; v->ch_midi[i] = sq_ch[i].midi_ch; v->ch_muted[i] = sq_ch[i].muted; }
    if (uisq_cm_val <= SQ_MIDI_OFF && uisq_cm_ch < SQ_NCH) v->ch_midi[uisq_cm_ch] = uisq_cm_val;   /* CHAN K2: the header follows the knob at once (SEQUENCER_UX 4.4) */
    v->algo = a;
    v->cache = sq_cache[sel][sq_cidx[sel]];
    /* knob tiles */
    v->nrow = rig ? 3u : 2u;
    v->row = uisq_row < v->nrow ? uisq_row : 0u;
    v->band = uisq_page == UI_PG_NOTE_EDIT ? UISQ_BAND_NOTE : uisq_rec ? UISQ_BAND_REC : uisq_page == UI_PG_CHAN ? UISQ_BAND_CHAN : uisq_page == UI_PG_SET ? UISQ_BAND_SET : UISQ_BAND_ROW;
    v->octave = sq_gdb[sq_gidx].s.octave; v->port = uisq_clock_port;
    uisq_set_init(); uisq_rec_init();
    for (uint32_t k = 0; k < 4u; k++) {
        if (v->band == UISQ_BAND_NOTE) { v->tile[k] = &uisq_note_row; v->val[k] = 0; }    /* the NOTE_EDIT tiles are drawn from ne_* below */
        else if (v->band == UISQ_BAND_REC) {
            v->tile[k] = &uisq_rec_row[k];
            v->val[k] = k == 0 ? uisq_rec_cursor : k == 1 ? uisq_rec_acc : k == 2 ? uisq_rec_slide : uisq_rec_oct; }
        else if (v->band == UISQ_BAND_SET) {
            const SeqGlobal *g = &sq_gdb[sq_gidx];
            v->tile[k] = &uisq_set_row[k];
            v->val[k] = k == 0 ? (uint8_t)(sq_bpm < 40u ? 40u : sq_bpm > 240u ? 240u : sq_bpm) : k == 1 ? g->s.swing : k == 2 ? g->s.gate : g->s.div; }
        else if (v->band == UISQ_BAND_CHAN) {
            v->tile[k] = &uisq_chan_row[k];
            v->val[k] = k == 0 ? c->len : k == 1 ? (uint8_t)((uisq_cm_val <= SQ_MIDI_OFF && uisq_cm_ch == sel ? uisq_cm_val : c->midi_ch) + 1u) : k == 2 ? 0u : sq_follow; }
        else if (v->row == 2u) { v->tile[k] = &rig[k]; v->val[k] = rig[k].def; }
        else {
            const uint8_t id = a->knob_morph[(v->row * 4u + k) & 7u];
            v->tile_id[k] = id; v->tile[k] = &a->morph[id]; v->val[k] = c->st[aidx].morph[id]; }
        if (uisq_hot_until[k] && (int32_t)(uisq_now - uisq_hot_until[k]) < 0 && (!best || (int32_t)(uisq_hot_until[k] - best) > 0)) { best = uisq_hot_until[k]; v->turn = (uint8_t)(k + 1u); }
        if (!(uisq_hot_until[k] && (int32_t)(uisq_now - uisq_hot_until[k]) < 0)) uisq_knob_ref[k] = v->val[k]; }   /* not being turned: remember the level */
    if (v->turn) {
        const MorphDesc *m = v->tile[v->turn - 1u];
        const uint8_t r = uisq_knob_ref[v->turn - 1u];
        v->turn_old = m->max == m->min ? 0u : (uint8_t)(100u * (uint32_t)(r - m->min) / (uint32_t)(m->max - m->min)); }
    uisq_view_texts(v);
    /* status line and pages */
    v->preset = uisq_preset; v->pend = uisq_preset_pend ? uisq_preset : 0u;     /* placeholder: the preset layer is P7 */
    v->pages = (uint8_t)((len + SQ_PAGE - 1u) / SQ_PAGE);
    if (sq_follow && pp != 255u && v->band != UISQ_BAND_NOTE && v->band != UISQ_BAND_REC) vp = (uint8_t)(pp / SQ_PAGE);   /* FOLLOW: the view goes where the playing step is (NOTE_EDIT stays put; REC's cursor owns the view) */
    if (vp >= v->pages) vp = 0;
    v->vpage = (uint8_t)(vp + 1u);
    v->ppage = pp != 255u ? (uint8_t)(pp / SQ_PAGE + 1u) : v->vpage;
    v->follow_off = (uint8_t)!sq_follow;
    /* music row */
    v->ph = (pp != 255u && pp / SQ_PAGE == vp) ? (uint8_t)(pp % SQ_PAGE) : (uint8_t)UISQ_NO_PH;
    v->len16 = len - vp * SQ_PAGE >= SQ_PAGE ? SQ_PAGE : (uint8_t)(len - vp * SQ_PAGE);
    v->cursor = uisq_rec_cursor;
    v->diced = (uisq_diced_until && (int32_t)(uisq_now - uisq_diced_until) < 0) ? uisq_diced : 0u;
    uisq_lane_read(v->lane, sel, vp);
    /* buttons, keybed, footer, message slot */
    v->btn_lit = uisq_btn_down; v->btn_dim = v->band == UISQ_BAND_NOTE ? 0u : uisq_btn_dimmask(a);   /* NOTE_EDIT: every layer button works on the step */
    if (v->band == UISQ_BAND_NOTE) uisq_ne_fill(v, vp);
    v->held_m = uisq_held_m; v->held_mv = uisq_held_mv;
    v->cue = (uint8_t)((uisq_new >> sel & 1u) || c->midi_ch >= SQ_MIDI_OFF);   /* the one standing cue; it clears when the channel gets a MIDI channel */
    uisq_msg_pick(&v->msg, uisq_now, uisq_page, sel);
    for (uint8_t i = 0; i < SQ_NCH; i++) {
        UisqStep L[SQ_PAGE];
        if (i == sel || !(uisq_used >> i & 1u)) continue;
        uisq_lane_read(L, i, vp);
        for (uint8_t n = 0; n < SQ_PAGE; n++) if (L[n].on) v->hits[i] |= (uint16_t)(1u << n); } }

/* ---- text and shape helpers in the mock's terms: a text is placed by its CAP TOP (v12.py P.text), not its line top ---- */
static int32_t uisq_captop(const aafont_t *f) { return f == &AF_S ? AF_S_CAP_Y : f == &AF_M ? AF_M_CAP_Y : AF_L_CAP_Y; }
static int32_t uisq_text(int32_t x, int32_t ycap, const aafont_t *f, const char *s, uint16_t fg, uint16_t bg)
{   cv_text_on(x, ycap - uisq_captop(f), f, s, fg, bg);
    return x + text_w(f, s); }
static void uisq_text_c(int32_t x0, int32_t w, int32_t ycap, const aafont_t *f, const char *s, uint16_t fg, uint16_t bg)
{   uisq_text(x0 + HALF_UP(w - text_w(f, s)), ycap, f, s, fg, bg); }
static void uisq_text_r(int32_t xr, int32_t ycap, const aafont_t *f, const char *s, uint16_t fg, uint16_t bg) { uisq_text(xr - text_w(f, s), ycap, f, s, fg, bg); }
static void uisq_hatch(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c, int32_t step)   /* diagonal lines every `step` px */
{   for (int32_t j = 0; j < h; j++) for (int32_t i = 0; i < w; i++) if ((i + j) % step == 0) cv_pset(x + i, y + j, c); }
static uint16_t uisq_mixk(uint16_t c, uint32_t k)                  /* c scaled to k/255 over black, per RGB565 channel (the REC band's white fill) */
{   return (uint16_t)((((c >> 11) & 31u) * k / 255u) << 11 | (((c >> 5) & 63u) * k / 255u) << 5 | ((c & 31u) * k / 255u)); }
static void uisq_digit(char *d, uint32_t n, uint32_t v) { d[0] = 0; uisq_catu(d, n, v); }

/* ---- 4.1 header (y 1-19 and the selection bar 20-21) ---- */
static void uisq_draw_header(const UisqView *v)
{   const int32_t y = 1;
    const uint8_t sel = v->sel;
    char b[12];
    uint8_t plus = 0;
    int32_t xe, xx;
    if (v->rec) {                                                      /* REC: a red disc, with a play mark in it while playing */
        cv_rect(4, y + 6, 9, 9, T_REC); cv_rect(3, y + 7, 11, 7, T_REC); cv_rect(5, y + 5, 7, 11, T_REC);
        if (v->playing) { cv_rect(6, y + 8, 2, 5, UISQ_WHITE); cv_rect(8, y + 9, 2, 3, UISQ_WHITE); } }
    else if (v->playing) cv_icon(3, y + 5, SQI_PLAY_S, UISQ_WHITE);
    else cv_icon(3, y + 5, SQI_STOP_S, T_MID);
    if (v->clock == UISQ_CLK_INT) { uisq_digit(b, sizeof b, v->bpm); uisq_text(16, y + 4, &AF_M, b, T_TEXT, T_BG); }
    else {                                                             /* external: EXT over the tempo, or WAIT, or -- when it is lost */
        uisq_text(16, y, &AF_S, uisq_st_w(UI_ST_EXT), T_MID, T_BG);
        if (v->clock == UISQ_CLK_EXT) uisq_digit(b, sizeof b, v->bpm); else { b[0] = 0; uisq_cat(b, sizeof b, uisq_st_w(v->clock == UISQ_CLK_WAIT ? UI_ST_WAIT : UI_ST_DASH)); }
        uisq_text(16, y + 10, &AF_S, b, v->clock == UISQ_CLK_LOST ? T_REC : T_TEXT, T_BG); }
    for (uint32_t i = 0; i < SQ_NCH; i++) {                            /* the chips: one per channel in use, then a `+` in the first free slot */
        const int32_t cx = 46 + 14 * (int32_t)i;
        const uint8_t off = v->ch_midi[i] >= SQ_MIDI_OFF;
        b[0] = (char)('1' + i); b[1] = 0;
        if (v->used >> i & 1u) {
            if (v->ch_muted[i]) {                                      /* muted: a dark chip, a grey digit, a diagonal */
                cv_rect(cx, y, 13, 19, T_SURF);
                uisq_text_c(cx, 13, y + 4, &AF_M, b, T_MID, T_SURF);
                cv_line(cx, y + 18, cx + 12, y, T_MID); }
            else if (i == sel) { cv_rect(cx, y, 13, 19, T_CH(i)); uisq_text_c(cx, 13, y + 4, &AF_M, b, UISQ_INK, T_CH(i)); }
            else { cv_frame(cx, y, 13, 19, T_CH(i)); uisq_text_c(cx, 13, y + 4, &AF_M, b, T_CH(i), T_BG); }
            if (i == sel) cv_rect(cx, y + 20, 13, 2, UISQ_WHITE);      /* selection: a white bar under the chip, whatever its state */
            if (off) cv_rect(cx + 1, y + 1, 11, 3, T_OFF); }           /* MIDI OFF: a red cap */
        else if (!plus) { plus = 1; uisq_text_c(cx, 13, y + 4, &AF_M, "+", T_MID, T_BG); } }
    if (v->band == UISQ_BAND_NOTE) {                                   /* NOTE_EDIT: the mode is named where the algorithm is: an inverted channel-colour chip `STEP n` */
        char sn[12];
        int32_t w;
        sn[0] = 0; uisq_cat(sn, sizeof sn, uisq_ne_w(UI_NE_STEP)); uisq_cat(sn, sizeof sn, " "); uisq_catu(sn, sizeof sn, v->ne_step);
        w = text_w(&AF_M, sn) + 8;
        cv_rect(105, y, w, 19, T_CH(sel));
        xe = uisq_text(109, y + 4, &AF_M, sn, UISQ_INK, T_CH(sel)) + 4; }
    else xe = uisq_text(105, y + 4, &AF_M, uisq_short_of(v->ch_algo[sel]), T_CH(sel), T_BG);
    if (v->ch_midi[sel] >= SQ_MIDI_OFF) {                              /* loud: an inverted red chip that says what is wrong */
        cv_rect(xe + 4, y, text_w(&AF_S, "MIDI") + text_w(&AF_M, "OFF") + 10, 19, T_OFF);
        xx = uisq_text(xe + 7, y + 6, &AF_S, "MIDI", UISQ_WHITE, T_OFF);
        uisq_text(xx + 3, y + 4, &AF_M, "OFF", UISQ_WHITE, T_OFF); }
    else {
        xx = uisq_text(xe + 5, y + 6, &AF_S, "MIDI", T_MID, T_BG);
        uisq_digit(b, sizeof b, (uint32_t)v->ch_midi[sel] + 1u);
        uisq_text(xx + 2, y + 4, &AF_M, b, UISQ_WHITE, T_BG); } }

/* ---- 4.2 knob tiles: four 58 x 53 tiles at y (the tile is the meter) ---- */
static void uisq_draw_knobs(const UisqView *v, int32_t y)
{   const uint16_t col = v->band == UISQ_BAND_REC ? UISQ_WHITE : T_CH(v->sel);
    const uint16_t fc = v->band == UISQ_BAND_REC ? uisq_mixk(UISQ_WHITE, 120) : T_CHF(v->sel);
    for (uint32_t k = 0; k < 4u; k++) {
        const int32_t x = 1 + 60 * (int32_t)k;
        const MorphDesc *m = v->tile[k];
        const uint8_t dots = uisq_tile_dots(m);
        const uint8_t set = v->band == UISQ_BAND_SET, host = (uint8_t)(set && k == 0u && v->clock != 0u);   /* SET K1 under an external clock: a read-only mirror */
        const aafont_t *f = set ? (k == 0u ? &AF_L : &AF_M) : (v->val[k] == 0 && m->zero) ? &AF_M : uisq_tile_font(uisq_view_hook(v), v->tile_id[k], m);
        const int32_t span = (int32_t)m->max - m->min, d = (int32_t)v->val[k] - m->min;
        uint16_t bgv = T_SURF;
        cv_rect(x, y, 58, 16, T_SURF);
        uisq_text_c(x, 58, y + 3, &AF_M, host ? uisq_st_w(UI_ST_HOST) : m->label, T_MID, T_SURF);
        cv_rect(x, y + 17, 58, 36, T_SURF);
        if (host) {                                                    /* no fill (the shape changes, not only the colour), a dashed frame, the host tempo in MID */
            for (int32_t i = 0; i < 58; i += 6) { cv_rect(x + i, y + 17, 3, 1, T_MID); cv_rect(x + i, y + 52, 3, 1, T_MID); }
            for (int32_t j = 17; j < 53; j += 6) { cv_rect(x, y + j, 1, 3, T_MID); cv_rect(x + 57, y + j, 1, 3, T_MID); }
            uisq_text_c(x, 58, y + 24, &AF_L, v->txt[k], T_MID, T_SURF);
            if (v->turn == k + 1u) { cv_frame(x - 1, y - 1, 60, 55, UISQ_WHITE); cv_frame(x, y, 58, 53, UISQ_WHITE); }   /* touched: white (it is no channel value) */
            continue; }
        if (dots) {                                                    /* a value list: position dots, no meter */
            const int32_t dw = (50 - ((int32_t)dots - 1) * 2) / dots < 8 ? (50 - ((int32_t)dots - 1) * 2) / dots : 8;
            const int32_t x0 = x + (58 - ((int32_t)dots * dw + ((int32_t)dots - 1) * 2)) / 2;
            for (int32_t i = 0; i < dots; i++) {
                if (i == d) cv_rect(x0 + i * (dw + 2), y + 47, dw, 3, col); else cv_frame(x0 + i * (dw + 2), y + 47, dw, 3, T_DIM); } }
        else {
            const int32_t fh = 36 * (span ? 100 * d / span : 0) / 100;
            cv_rect(x, y + 53 - fh, 58, fh, fc);
            if (fh >= 2) { cv_rect(x, y + 53 - fh, 7, 2, col); cv_rect(x + 51, y + 53 - fh, 7, 2, col); }   /* the level edge, clear of the value */
            if (fh >= 30) bgv = fc; }
        { const char *t = v->txt[k];
          const aafont_t *g = text_w(f, t) > 54 ? &AF_S : f;
          uisq_text_c(x, 58, g == &AF_L ? y + 24 : y + 26, g, t, UISQ_WHITE, bgv); }
        if (v->turn == k + 1u) {                                       /* being turned: a frame in the channel colour, ticks at the old level */
            cv_frame(x - 1, y - 1, 60, 55, col);
            cv_frame(x, y, 58, 53, col);
            if (!dots) { const int32_t oh = 36 * v->turn_old / 100; cv_rect(x + 1, y + 53 - oh - 1, 6, 2, UISQ_WHITE); cv_rect(x + 51, y + 53 - oh - 1, 6, 2, UISQ_WHITE); } } } }

/* ---- NOTE_EDIT tiles (P4-014): K1-K4 of the edited step. The BIG text is the value the step will play: yours (white, the tile filled like a full meter in the
 * channel's dim colour, a white marker square top left, `GEN <value>` under it) or the algorithm's (grey on the plain tile, the word AUTO under it). AF_L when
 * the text fits 54 px, else AF_M. GATE's % and RATCH's x are small (AF_S) beside the digits: the big font has neither. The label is the knob_map NOTE_EDIT row. ---- */
static void uisq_draw_notetiles(const UisqView *v, int32_t y)
{   const uint16_t col = T_CH(v->sel), fc = T_CHF(v->sel);
    for (uint32_t k = 0; k < 4u; k++) {
        const int32_t x = 1 + 60 * (int32_t)k;
        const uint8_t set = (uint8_t)(v->ne_set >> k & 1u);
        const char *t = v->ne_txt[k], *u = v->ne_unit[k];
        const int32_t uw = u[0] ? text_w(&AF_S, u) + 1 : 0;
        const aafont_t *f = (text_w(&AF_L, t) + uw <= 54) ? &AF_L : &AF_M;
        const uint16_t bgv = set ? fc : T_SURF, fg = set ? UISQ_WHITE : T_MID;
        const int32_t tw = text_w(f, t) + uw, tx = x + HALF_UP(58 - tw), cap = f == &AF_L ? y + 19 : y + 23;
        cv_rect(x, y, 58, 16, T_SURF);
        uisq_text_c(x, 58, y + 3, &AF_M, uisq_knob_label(UI_PG_NOTE_EDIT, UI_CTX_ANY, (uint8_t)k), T_MID, T_SURF);
        cv_rect(x, y + 17, 58, 36, bgv);
        if (set) { cv_rect(x, y + 17, 7, 2, col); cv_rect(x + 51, y + 17, 7, 2, col); cv_rect(x + 3, y + 22, 5, 5, UISQ_WHITE); }   /* the level edge and the set marker */
        if (t[0]) {
            const int32_t xe = uisq_text(tx, cap, f, t, fg, bgv);
            if (u[0]) uisq_text(xe + 1, cap + (f == &AF_L ? 12 : 2), &AF_S, u, fg, bgv); }                    /* the unit sits on the digits' baseline */
        uisq_text_c(x, 58, y + 44, &AF_S, v->ne_sub[k], set ? T_TEXT : T_MID, bgv);
        if (v->turn == k + 1u) { cv_frame(x - 1, y - 1, 60, 55, col); cv_frame(x, y, 58, 53, col); } } }

/* the NOTE_EDIT status line: `STEP n` (AF_M), then one chip per set field in order; xs[i] = the left edge of chip i, the return the right edge of the last.
 * The first chip starts after the widest step number (two digits), so the row does not move when the step reaches 10 (the test proves STEP 64 with
 * every chip fits 237 px) */
static int32_t uisq_ne_status_layout(const UisqView *v, int32_t xs[4])
{   int32_t x = 3 + text_w(&AF_M, uisq_ne_w(UI_NE_STEP)) + 4 + text_w(&AF_M, "88") + 4;
    (void)v;
    for (uint32_t i = 0; i < 4u; i++) {
        xs[i] = x;
        if (v->ne_chip[i][0]) x += text_w(&AF_S, v->ne_chip[i]) + 4 + 2; }
    return x; }

/* ---- 4.3 status line (caps 80): preset | the knob row | the page ---- */
static void uisq_draw_status(const UisqView *v)
{   const int32_t y = 80;
    char b[16];
    if (v->band == UISQ_BAND_NOTE) {                                   /* NOTE_EDIT: STEP n, the step's other set fields as chips, the move hint and the set mark's legend */
        int32_t xs[4];
        (void)uisq_ne_status_layout(v, xs);
        b[0] = 0; uisq_catu(b, sizeof b, v->ne_step);
        (void)uisq_text(uisq_text(3, y, &AF_M, uisq_ne_w(UI_NE_STEP), UISQ_WHITE, T_BG) + 4, y, &AF_M, b, UISQ_WHITE, T_BG);
        for (uint32_t i = 0; i < 4u; i++) {
            if (!v->ne_chip[i][0]) continue;
            cv_rect(xs[i], y - 1, text_w(&AF_S, v->ne_chip[i]) + 4, 13, T_CH(v->sel));
            uisq_text(xs[i] + 2, y + 1, &AF_S, v->ne_chip[i], UISQ_INK, T_CH(v->sel)); }
        return; }
    if (v->pend) {
        b[0] = 0; uisq_cat(b, sizeof b, "P"); if (v->pend < 10u) uisq_cat(b, sizeof b, "0"); uisq_catu(b, sizeof b, v->pend); uisq_cat(b, sizeof b, "?");
        cv_rect(1, y - 2, text_w(&AF_M, b) + 6, 14, UISQ_WHITE);
        uisq_text(4, y, &AF_M, b, UISQ_INK, UISQ_WHITE); }
    else {
        b[0] = 0; uisq_cat(b, sizeof b, "P"); if (v->preset < 10u) uisq_cat(b, sizeof b, "0"); uisq_catu(b, sizeof b, v->preset);
        uisq_text(3, y, &AF_M, b, T_TEXT, T_BG); }
    if (v->band == UISQ_BAND_SET) {                                    /* SET: [EXT USB] OUT word, then OCT n (ROW and PAGE mean nothing here) */
        const uint8_t ext = (uint8_t)(v->clock != UISQ_CLK_INT);
        char ot[4];
        int32_t x = 42, xa;
        if (ext) { xa = uisq_text(x, y + 2, &AF_S, uisq_st_w(UI_ST_EXT), T_MID, T_BG); x = uisq_text(xa + 3, y, &AF_M, uisq_st_w(v->port ? UI_ST_TRS : UI_ST_USB), T_TEXT, T_BG) + 7; }
        xa = uisq_text(x, y + 2, &AF_S, uisq_st_w(UI_ST_OUT), T_MID, T_BG);
        (void)uisq_text(xa + 3, y, &AF_M, uisq_st_w(ext ? UI_ST_FOLLOW : v->playing ? UI_ST_MASTER : UI_ST_ON_PLAY), T_TEXT, T_BG);
        ot[0] = 0;
        if (v->octave) { uisq_cat(ot, sizeof ot, uisq_ne_w(v->octave > 0 ? UI_NE_PLUS : UI_NE_MINUS)); uisq_catu(ot, sizeof ot, (uint32_t)(v->octave > 0 ? v->octave : -v->octave)); }
        else uisq_cat(ot, sizeof ot, "0");
        uisq_text_r(238, y, &AF_M, ot, T_TEXT, T_BG);
        uisq_text_r(236 - text_w(&AF_M, ot), y + 2, &AF_S, uisq_st_w(UI_ST_OCT), T_MID, T_BG);
        return; }
    if (!v->rec) {                                                     /* ROW + numbered boxes, the live row filled (a REC has no rows) */
        const int32_t x0 = 128 - (int32_t)v->nrow * 13 / 2;
        uisq_text_r(x0 - 3, y + 2, &AF_S, "ROW", T_MID, T_BG);
        for (uint32_t r = 0; r < v->nrow; r++) {
            const int32_t bx = x0 + 13 * (int32_t)r;
            b[0] = (char)('1' + r); b[1] = 0;
            if (r == v->row) { cv_rect(bx, y - 1, 11, 13, T_CH(v->sel)); uisq_text_c(bx, 11, y + 1, &AF_S, b, UISQ_INK, T_CH(v->sel)); }
            else { cv_frame(bx, y - 1, 11, 13, T_MID); uisq_text_c(bx, 11, y + 1, &AF_S, b, T_MID, T_BG); } } }
    { char pt[8];
      pt[0] = 0; uisq_catu(pt, sizeof pt, v->vpage); uisq_cat(pt, sizeof pt, "/"); uisq_catu(pt, sizeof pt, v->pages);
      if (v->follow_off) {                                             /* FOLLOW off: the viewed page is boxed; the playing page is named */
          const int32_t w = text_w(&AF_M, pt) + 4;
          cv_frame(238 - w, y - 2, w + 1, 14, UISQ_WHITE);
          uisq_text_r(237, y, &AF_M, pt, T_TEXT, T_BG);
          b[0] = 0; uisq_cat(b, sizeof b, "PLAY "); uisq_catu(b, sizeof b, v->ppage);
          uisq_text_r(234 - w, y + 2, &AF_S, b, T_TEXT, T_BG); }
      else {
          uisq_text_r(238, y, &AF_M, pt, T_TEXT, T_BG);
          uisq_text_r(236 - text_w(&AF_M, pt), y + 2, &AF_S, "PAGE", T_MID, T_BG); } } }

/* ---- 4.4 music row (y 92-141): the selected channel full width, the others as 3 px strips in a fixed row per channel number ---- */
static void uisq_draw_music(const UisqView *v)
{   const int32_t y0 = 96, y1 = 141, strip_h = 3;
    const uint8_t sel = v->sel, others = (uint8_t)(v->used & ~(1u << sel));
    const int32_t bh_max = (y1 - y0) - (others ? 4 : 0) * (strip_h + 1) - 1;
    const uint8_t dim = v->ch_muted[sel] || v->ch_midi[sel] >= SQ_MIDI_OFF;
    const uint16_t col = T_CH(sel);
    int32_t lo = 255, hi = 0;
    for (uint32_t i = 0; i < SQ_PAGE; i++) if (v->lane[i].on) { if (v->lane[i].note < lo) lo = v->lane[i].note; if (v->lane[i].note > hi) hi = v->lane[i].note; }
    if (lo > hi) { lo = 0; hi = 1; }
    for (uint32_t i = 0; i < SQ_PAGE; i++) {
        const int32_t x = 1 + 15 * (int32_t)i;
        const UisqStep *s = &v->lane[i];
        if (i >= v->len16) { uisq_hatch(x, y0, 13, y1 - y0, T_LINE, 3); continue; }   /* past LENGTH: a hatched, empty step */
        if (i == v->ph) {                                              /* the playhead: a lit column and a white pointer */
            cv_rect(x - 1, y0 - 1, 15, y1 - y0 + 1, T_PH);
            for (int32_t d = 0; d < 3; d++) cv_rect(x + 3 + d, y0 - 4 + d, 7 - 2 * d, 1, UISQ_WHITE); }
        if (!s->on) cv_rect(x + 5, y0 + bh_max - 1, 3, 1, T_DIM);      /* a rest: a floor dash */
        else {
            const int32_t span = hi - lo > 1 ? hi - lo : 1;
            const int32_t bh = v->algo->lane == SQ_LANE_SINGLE ? 8 : 6 + (bh_max - 9) * ((int32_t)s->note - lo) / span;
            const int32_t by = y0 + bh_max - bh;
            if (dim) cv_frame(x, by, 13, bh, v->ch_muted[sel] ? T_MID : T_OFF);          /* muted or MIDI OFF: hollow, nothing is sounding */
            else if (v->diced >> i & 1u) { cv_rect(x, by, 13, bh, UISQ_WHITE); uisq_hatch(x, by, 13, bh, col, 2); }
            else cv_rect(x, by, 13, bh, col);
            if (s->acc && !dim) cv_rect(x, by - 4, 13, 2, UISQ_WHITE);                    /* accent: a full-width white cap */
            if (s->slide && i < SQ_PAGE - 1u && v->lane[i + 1].on && !dim) cv_rect(x + 13, by + 2, 2, 3, col); }   /* slide: a bridge into the next step */
        for (uint32_t c = 0; c < SQ_NCH; c++) {                        /* the other channels: a hit strip in their own fixed row */
            const int32_t yy = y0 + bh_max + 2 + (int32_t)c * (strip_h + 1);
            if (!(others >> c & 1u) || !(v->hits[c] >> i & 1u)) continue;
            if (v->ch_muted[c] || v->ch_midi[c] >= SQ_MIDI_OFF) cv_rect(x, yy, 5, strip_h, T_DIM);   /* a short grey dash: silent */
            else cv_rect(x, yy, 13, strip_h, T_CH(c)); } }
    if (v->edit) {                                                     /* NOTE_EDIT: the edited step is framed (the REC cursor's frame) */
        const int32_t x = ((int32_t)v->edit - 1) * 15 + 1;
        cv_frame(x - 2, 93, 17, 49, UISQ_WHITE);
        cv_frame(x - 1, 94, 15, 47, UISQ_INK); }                       /* a black ring inside the white one: the amber bar never touches white (2.3:1) */
    if (v->rec) {                                                      /* REC: the write cursor frames its step */
        const int32_t x = ((int32_t)v->cursor - 1) * 15 + 1;
        cv_frame(x - 2, 93, 17, 49, UISQ_WHITE);
        cv_frame(x - 1, 94, 15, 47, UISQ_WHITE); } }

/* ---- 4.5 buttons: 2 x 6 tiles (the frame lays them), the icon centred; dice tiles neutral with white icons, an absent layer DIM, REC armed a red disc, pressed inverted ---- */
static void uisq_draw_buttons(const UisqView *v)
{   for (uint32_t i = 0; i < 12u; i++) {
        const int32_t x = 1 + 40 * (int32_t)(i % 6u), y = UISQ_BTN_Y + (UISQ_BTN_TH + 1) * (int32_t)(i / 6u);
        uint32_t ic = uisq_btn_icon[i];
        uint16_t fg = uisq_btn_dice[i] ? UISQ_WHITE : T_TEXT, bg = uisq_btn_dice[i] ? T_DICE : T_SURF;
        if (uisq_btn_dice[i] && (v->btn_dim >> i & 1u)) fg = T_DIM;     /* the algorithm has no such layer: the die is greyed */
        if (uisq_btn_ctl[i] == UI_C_B_REC && v->rec) { ic = SQI_REC; fg = T_REC; }
        if (v->band == UISQ_BAND_NOTE && uisq_btn_dice[i]) {            /* NOTE_EDIT: the layer buttons act on THIS step: corner brackets, not the die; a toggle shows its state */
            static const uint8_t sti[12] = { SQI_STEP_ALL, SQI_STEP_NOTES, SQI_STEP_ACCNT, SQI_STEP_PROB, 0, 0, 0, 0, SQI_STEP_SLIDE, SQI_STEP_ONOFF, 0, 0 };
            static const int8_t tog[12] = { -1, -1, 0, 1, -1, -1, -1, -1, 2, 3, -1, -1 };      /* the bit of ne_btn each toggle reads */
            ic = sti[i]; fg = UISQ_WHITE; bg = T_DICE;
            if (tog[i] >= 0 && (v->ne_btn >> tog[i] & 1u)) { bg = T_CH(v->sel); fg = UISQ_INK; cv_rect(x, y, UISQ_BTN_W, UISQ_BTN_TH, bg); cv_rect(x, y + UISQ_BTN_TH - 2, UISQ_BTN_W, 2, UISQ_WHITE); }
            cv_frame(x, y, UISQ_BTN_W, UISQ_BTN_TH, UISQ_WHITE); }       /* a white outline round the six "this step" tiles: the page differs from PERF without reading an icon */
        if (v->btn_lit >> i & 1u) { bg = UISQ_WHITE; fg = UISQ_INK; cv_rect(x, y, UISQ_BTN_W, UISQ_BTN_TH, bg); }
        cv_icon(x + (UISQ_BTN_W - SQI_TAB[ic].w) / 2, y + (UISQ_BTN_TH - SQI_TAB[ic].h) / 2, ic, fg); } }

/* ---- 4.6 keybed: white keys = the steps of the viewed page; black keys carry their role (panel order M1 M2 M3 M4 CHN SOL FIL ROL PG- PG+ RST); REC: plain keys ---- */
static const char *const uisq_foot_steps = "KEYS = STEPS", *const uisq_foot_edit = "HOLD = EDIT";
static const char *const uisq_foot_cue = "CHN KEY, K2 = MIDI";
/* a page cue of page_states.tsv is `left   right` (three spaces): the footer's two halves (CHAN: `K2 = MIDI` / `CHN = BACK`, P4-015) */
static void uisq_cue_split(const char *cue, char *l, uint32_t ln, char *r, uint32_t rn)
{   uint32_t k = 0, j = 0;
    while (cue[k] && !(cue[k] == ' ' && cue[k + 1] == ' ' && cue[k + 2] == ' ') && k + 1u < ln) { l[k] = cue[k]; k++; }
    l[k] = 0;
    while (cue[k] == ' ') k++;
    while (cue[k] && j + 1u < rn) r[j++] = cue[k++];
    r[j] = 0; }
static void uisq_draw_keybed(const UisqView *v)
{   const int32_t y = UISQ_KEY_Y, kh = UISQ_WKEY_H, bh = UISQ_BKEY_H;
    const uint8_t rec = v->rec, chan = v->band == UISQ_BAND_CHAN;
    const uint8_t sel = v->sel;
    for (uint32_t i = 0; i < 16u; i++) {
        const int32_t x = 15 * (int32_t)i;
        if (rec) cv_rect(x + 1, y, 14, kh, T_KEY);                      /* REC: plain playable keys */
        if (i >= v->len16) { uisq_hatch(x + 1, y, 14, kh, T_LINE, 3); continue; }   /* this key is past LENGTH */
        if (!rec && v->lane[i].on) cv_rect(x + 1, y + bh + 2, 14, kh - bh - 2, T_CH(sel));   /* the step is on: the key's front in the channel colour */
        if (i == v->ph && !rec) cv_rect(x + 1, y + kh, 14, 2, UISQ_WHITE); }   /* the playing step: a white bar under its key */
    for (uint32_t k = 0; k < 11u; k++) {
        const int32_t x = (int32_t)uisq_bkey_cx[k] - UISQ_BKEY_W / 2, gy = y + 7;
        cv_rect(x - 1, y, UISQ_BKEY_W + 2, bh + 1, T_BG);               /* the black key over the white keys' fronts, hatch and REC fill */
        cv_rect(x, y, UISQ_BKEY_W, bh, T_RAISE);
        if (rec) continue;                                              /* REC: plain playable keys */
        if (k < 4u) {                                                   /* M1-M4: the channel's mute key */
            if (v->held_m == k + 1u) { cv_rect(x, y, UISQ_BKEY_W, bh, UISQ_WHITE); cv_rect(x + 2, gy, 6, 7, T_CH(k)); }
            else if (v->used >> k & 1u) { if (v->ch_muted[k]) cv_icon(x + 2, gy, SQI_X7, T_MID); else cv_rect(x + 2, gy, 6, 7, T_CH(k)); } }
        else if (k == 4u) {                                             /* CHN: channel setup; BACK on the CHAN page; lit under the setup cue */
            if (chan) { cv_rect(x, y, UISQ_BKEY_W, bh, UISQ_WHITE); cv_icon(x + 3, gy, SQI_LEFT, UISQ_INK); }
            else if (v->cue) { cv_rect(x, y, UISQ_BKEY_W, bh, UISQ_WHITE); cv_icon(x + 2, gy + 1, SQI_MIDI_S, UISQ_INK); }
            else cv_icon(x + 2, gy + 1, SQI_MIDI_S, T_TEXT); }
        else if (k <= 7u) {                                             /* SOL FIL ROL: AF_S letters, the held one inverted */
            static const char *const letter[3] = { "S", "F", "R" };
            if (v->held_mv == k - 4u) { cv_rect(x, y, UISQ_BKEY_W, bh, UISQ_WHITE); uisq_text_c(x, UISQ_BKEY_W, gy - 1, &AF_S, letter[k - 5u], UISQ_INK, UISQ_WHITE); }
            else uisq_text_c(x, UISQ_BKEY_W, gy - 1, &AF_S, letter[k - 5u], T_TEXT, T_RAISE); }
        else cv_icon(x + 3, gy, k == 8u ? SQI_LEFT : k == 9u ? SQI_RIGHT : SQI_RST, T_TEXT); }   /* PG- PG+ RST */
    if (v->edit) {                                                      /* NOTE_EDIT: the edited key's front, below the black keys, framed white with a black ring inside */
        const int32_t x = 15 * ((int32_t)v->edit - 1) + 1, fy = y + bh + 1;
        cv_frame(x, fy, 14, y + kh - fy, UISQ_WHITE);
        cv_frame(x + 1, fy + 1, 12, y + kh - fy - 2, UISQ_INK); }
    { const int32_t ym = y + kh + 6;                                    /* the footer: the keys' mode line, or the setup cue (inverted) */
      char b[8];
      if (rec) {                                                      /* REC: the page_states cue, `PLAY KEYS TO WRITE` / `REC = DONE` */
          char l[24], r[24];
          uisq_cue_split(uisq_page_cue(UI_PG_REC), l, sizeof l, r, sizeof r);
          uisq_text(2, ym, &AF_M, l, T_TEXT, T_BG); uisq_text_r(238, ym + 1, &AF_S, r, T_MID, T_BG); }
      else if (v->band == UISQ_BAND_NOTE) {                           /* NOTE_EDIT: how to move the editor and the way out: on screen in every state (chips, toast) */
          uisq_text(2, ym, &AF_M, uisq_ne_w(UI_NE_HOLD_MOVE), T_TEXT, T_BG);
          uisq_text_r(238, ym, &AF_M, uisq_ne_w(UI_NE_HOME_DONE), T_MID, T_BG); }
      else if (v->band == UISQ_BAND_SET) {                            /* SET: the clock state is the footer; INT / EXT keep the keys line's size, WAIT and lost are AF_S (too wide with their hint) */
          char cl[24], cr[24];
          const uint8_t c = v->clock, small = (uint8_t)(c == UISQ_CLK_WAIT || c == UISQ_CLK_LOST);
          const aafont_t *ff = small ? &AF_S : &AF_M;
          const char *l = c == UISQ_CLK_INT ? cl : uisq_st_w(c == UISQ_CLK_EXT ? UI_ST_HOST_SETS : c == UISQ_CLK_WAIT ? UI_ST_WAIT_HOST : UI_ST_CLOCK_LOST);
          const char *r = c == UISQ_CLK_WAIT ? uisq_st_w(UI_ST_PLAY_CANCEL) : c == UISQ_CLK_LOST ? uisq_st_w(UI_ST_PLAY_START) : cr;
          uisq_cue_split(uisq_page_cue(UI_PG_SET), cl, sizeof cl, cr, sizeof cr);
          uisq_text(2, small ? ym + 2 : ym, ff, l, c == UISQ_CLK_LOST ? T_REC : T_TEXT, T_BG);
          uisq_text_r(238, small ? ym + 2 : ym, ff, r, T_MID, T_BG); }
      else if (chan) { char l[24], r[24]; uisq_cue_split(uisq_page_cue(UI_PG_CHAN), l, sizeof l, r, sizeof r); uisq_text(2, ym, &AF_M, l, T_TEXT, T_BG); uisq_text_r(238, ym, &AF_M, r, T_MID, T_BG); }
      else if (v->cue) {
          b[0] = 'C'; b[1] = 'H'; b[2] = (char)('1' + sel); b[3] = 0;
          cv_rect(0, ym - 3, UISQ_W, 16, UISQ_WHITE);
          uisq_text(4, ym, &AF_M, uisq_foot_cue, UISQ_INK, UISQ_WHITE);
          uisq_text_r(236, ym, &AF_M, b, UISQ_INK, UISQ_WHITE); }
      else { uisq_text(2, ym, &AF_M, uisq_foot_steps, T_TEXT, T_BG); uisq_text_r(238, ym, &AF_M, uisq_foot_edit, T_MID, T_BG); } } }

/* ---- 5 the message slot (x 104-239, y 0-21): AF_M when it fits, else AF_S, never cut ---- */
static int32_t uisq_fitm(int32_t x, int32_t ycap, const char *t, uint16_t fg, uint16_t bg, int32_t maxw)
{   const aafont_t *f = text_w(&AF_M, t) <= maxw ? &AF_M : &AF_S;
    return uisq_text(x, f == &AF_M ? ycap : ycap + 2, f, t, fg, bg); }
static void uisq_draw_msg(const UisqView *v)
{   const UisqMsg *m = &v->msg;
    const int32_t x0 = UISQ_MSG_X, w = UISQ_W - UISQ_MSG_X;
    char b[4];
    if (m->kind == UI_MK_HOLD) {                                        /* the held button's icon + its hold action; a bar fills to the fire time */
        cv_rect(x0, 0, w, 22, T_TEXT);
        cv_icon(x0 + 3, 4, m->icon, UISQ_INK);
        uisq_fitm(x0 + 20, 5, m->a, UISQ_INK, T_TEXT, w - 22);
        cv_rect(x0, 20, w * m->frac / 100, 2, UISQ_INK); }
    else if (m->kind == UI_MK_TOUCH) {                                  /* the printed name boxed + what it does now */
        const int32_t nw = text_w(&AF_S, m->a) + 6;
        cv_rect(x0, 0, w, 22, T_TEXT);
        cv_rect(x0 + 2, 3, nw, 16, UISQ_INK);
        uisq_text(x0 + 5, 7, &AF_S, m->a, T_TEXT, UISQ_INK);
        uisq_fitm(x0 + nw + 6, 5, m->b, UISQ_INK, T_TEXT, w - nw - 8); }
    else if (m->kind == UI_MK_TOAST) {                                  /* [channel chip] what happened, how to reverse it */
        int32_t x = x0 + 3;
        const int32_t hw = text_w(&AF_S, m->b);
        cv_rect(x0, 0, w, 22, UISQ_WHITE);
        if (m->chip) {
            b[0] = (char)('0' + m->chip); b[1] = 0;
            cv_rect(x, 2, 13, 18, T_CH(m->chip - 1u));
            uisq_text_c(x, 13, 6, &AF_M, b, UISQ_INK, T_CH(m->chip - 1u));
            x += 17; }
        uisq_fitm(x, 5, m->a, UISQ_INK, UISQ_WHITE, UISQ_W - 4 - hw - 6 - x);
        uisq_text_r(237, 7, &AF_S, m->b, UISQ_INK, UISQ_WHITE); } }

/* the elements, in the mock's order (screen v2 4.1-4.6, 5): the frame of tiles and keys first, the elements over it */
static void uisq_draw_frame(void);
static void uisq_draw_elements(const UisqView *v)
{   uisq_draw_frame();
    uisq_draw_header(v);
    if (v->band == UISQ_BAND_NOTE) uisq_draw_notetiles(v, UISQ_KNOB_Y); else uisq_draw_knobs(v, UISQ_KNOB_Y);
    uisq_draw_status(v);
    uisq_draw_music(v);
    uisq_draw_buttons(v);
    uisq_draw_keybed(v);
    uisq_draw_msg(v); }

/* ---- the frame (SCREEN_SPEC_V2 2): the button tiles and keys the elements sit on (the knob tiles are drawn by uisq_draw_knobs) ---- */
static void uisq_draw_frame(void)
{   for (uint32_t i = 0; i < 12u; i++) {                                /* button tiles: dice tiles neutral, the rest T_SURF */
        const int32_t x = 1 + 40 * (int32_t)(i % 6u), y = UISQ_BTN_Y + (UISQ_BTN_TH + 1) * (int32_t)(i / 6u);
        cv_rect(x, y, UISQ_BTN_W, UISQ_BTN_TH, uisq_btn_dice[i] ? T_DICE : T_SURF); }
    for (uint32_t i = 0; i < 16u; i++) cv_rect(1 + 15 * (int32_t)i, UISQ_KEY_Y, UISQ_WKEY_W, UISQ_WKEY_H, T_SURF);   /* white keys */
    for (uint32_t i = 0; i < 11u; i++) {                                /* black keys, with a 1 px BG surround on both sides */
        const int32_t x = (int32_t)uisq_bkey_cx[i] - UISQ_BKEY_W / 2;
        cv_rect(x - 1, UISQ_KEY_Y, UISQ_BKEY_W + 2, UISQ_BKEY_H + 1, T_BG);
        cv_rect(x, UISQ_KEY_Y, UISQ_BKEY_W, UISQ_BKEY_H, T_RAISE); } }


/* ---- the PERF page's controls (P4-013; SEQUENCER_UX 1.5, 1.7, 2.1, U02 U14; key_map.tsv / button_modes.tsv / knob_map.tsv PERF rows) ----
 * Event-driven so the host test can drive them without a panel: ui_sq_keys(mask) takes the 27-key state (bit 0 = F3) once per frame, ui_sq_button(ctl)
 * a panel-button press (a UI_C_B_* id), ui_sq_knob(k, d) a K1-K4 turn, ui_sq_select_turn(d) the SELECT encoder; ui_sq_input() runs the hold timers.
 * Labels and message words come from ui_tables.h (ui_msgs, ui_hold_rows, ui_remove_rows); the only words built here are a digit or a channel
 * algorithm name put into a table row. The sequencer is written through the main-loop API only (sq_step_tap, sq_note_edit_begin, sq_morph_turn,
 * sq_dice_*, sq_undo, sq_set_mute, sq_page_add); the keys stay silent (song.grid 2), so a tap never makes a sound by itself. */
#define UISQ_NKEY 27
static uint32_t uisq_keys;                    /* the keys down at the last ui_sq_keys() */
static uint32_t uisq_key_fired;               /* bit k: the hold action of key k has fired (no tap on release) */
static uint32_t uisq_key_t0[UISQ_NKEY];       /* UISQ_MS() of the press */
static uint8_t  uisq_pg_pend;                 /* PG+ on the last page was pressed once: the next PG+ adds the page */
/* removed channels, newest last: EDIT = UNDO pops one (the channel, its mute and its preview bit come back). A dice, a page add or an EDIT
 * that undoes something else empties the stack (the removal is then no longer the latest action). */
static uint8_t  uisq_gone_n, uisq_gone_ch[SQ_NCH], uisq_gone_muted[SQ_NCH], uisq_gone_prv[SQ_NCH], uisq_gone_midi[SQ_NCH];   /* BUG-011: a removed channel gives its MIDI channel up (OFF); the undo gives it back */
static uint8_t  uisq_undone;                  /* the last EDIT tap was an undo: the next one says REDO */

/* chromatic key 0..26 (F3 .. G5) -> 0..15 white, or 0x80 | 0..10 black (panel order, as key_map.tsv) */
static uint8_t uisq_key_slot(uint8_t k)
{   static const uint8_t white_pc[12] = { 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1 };   /* by semitone above C */
    uint8_t w = 0, b = 0;
    for (uint8_t i = 0; i <= k && i < UISQ_NKEY; i++) {
        const uint8_t white = white_pc[(i + 5u) % 12u];
        if (i == k) return white ? w : (uint8_t)(0x80u | b);
        if (white) w++; else b++; }
    return 0; }

static uint8_t uisq_key_ctl(uint8_t k)                                  /* the control id of chromatic key k, for the hold / remove tables */
{   const uint8_t s = uisq_key_slot(k);
    return (uint8_t)((s & 0x80u) ? UI_C_KEY_B01_FS3 + (s & 0x7Fu) : UI_C_KEY_W01_F3 + s); }

/* a table message with its digit (channel / page) or a channel's algorithm put in; the table row is the source of every word */
static void uisq_msg_toast_n(uint8_t id, uint8_t chip, uint8_t digit)   /* "PAGE 2?" with its 2 replaced */
{   char w[UISQ_TOAST_N];
    uisq_cpy(w, sizeof w, ui_msgs[id].a);
    for (uint32_t i = 0; w[i]; i++) if (w[i] >= '0' && w[i] <= '9') w[i] = (char)('0' + digit);
    ui_sq_toast_msg(chip, w, ui_msgs[id].b); }
static void uisq_hold_ch(uint8_t id, uint8_t ch)                        /* "REMOVE CH4" / "PREVIEW CH4" for channel ch */
{   char w[18];
    uisq_cpy(w, sizeof w, ui_msgs[id].a);
    for (uint32_t i = 0; w[i]; i++) if (w[i] >= '0' && w[i] <= '9') w[i] = (char)('1' + ch);
    ui_sq_hold_begin(w, SQI_MIDI, uisq_remove_ms(UI_C_KEY_B01_FS3)); }

static uint8_t uisq_view_page(void)                                     /* the viewed lane page 0..3, as uisq_view_fill computes it */
{   const ChanSave *c = &sq_ch[uisq_sel % SQ_NCH];
    const uint8_t len = c->len ? c->len : 1u, pages = (uint8_t)((len + SQ_PAGE - 1u) / SQ_PAGE);
    uint8_t vp = uisq_vpage;
    if (sq_follow && sq_playing && uisq_page != UI_PG_NOTE_EDIT && uisq_page != UI_PG_REC) vp = (uint8_t)(uisq_playhead(uisq_sel) / SQ_PAGE);   /* NOTE_EDIT stays on the page it was opened on */
    return vp >= pages ? 0u : vp; }

static uint8_t uisq_keys_live(void)                                    /* the pages whose keys are the steps, the M keys and the page keys: PERF, NOTE_EDIT, CHAN, SET */
{   return (uint8_t)(uisq_page == UI_PG_PERF || uisq_page == UI_PG_NOTE_EDIT || uisq_page == UI_PG_CHAN || uisq_page == UI_PG_SET); }

static uint8_t uisq_next_used(uint8_t from, int8_t dir)                 /* the nearest channel in use after `from` in `dir`, else `from` */
{   for (int32_t c = (int32_t)from + dir; c >= 0 && c < SQ_NCH; c += dir) if (uisq_used >> c & 1u) return (uint8_t)c;
    return from; }

/* ---- the NOTE_EDIT page's controls (P4-014; knob_map / button_modes / key_map NOTE_EDIT rows, SEQUENCER_UX 2.2, 2.7, U16..U30) ----
 * The page is open on (uisq_edit_ch, uisq_edit_step) and owns one sq note-edit session: every write shares ONE undo record (sq_note_edit_begin), EDIT
 * undoes / redoes it, and leaving the page closes it (sq_note_edit_end) so a later tap on the same step is a record of its own. Writes go through
 * uisq_ne_set(), which opens the session again first when something ended it (an EDIT undo, a tap on another step, a landed preset). A step the
 * LENGTH hides is refused by the core and nothing is written. The page leaves by: HOME tap, SELECT turn, the selection moving (a channel removed),
 * REC / GLO / CHN (their pages are later tasks: those keys are inert here until then). Not here: the audition of a changed pitch (the ISR sounds notes:
 * a request flag is a later ISR task), hard-tap ACCENT (needs key force), EDIT hold = RESET (P4-018). ---- */
static void uisq_ne_leave(void)
{   sq_note_edit_end();
    uisq_page = UI_PG_PERF;
    uisq_ne_dirty = 0; uisq_ne_undone = 0;
    ui.force = 1; }
static uint8_t uisq_ne_ready(void)                                      /* the page is open and its session is on the edited step */
{   if (uisq_page != UI_PG_NOTE_EDIT || uisq_edit_ch >= SQ_NCH || uisq_edit_step >= SQ_NSTEP) return 0;
    if (sq_note_edit_on(uisq_edit_ch, uisq_edit_step)) return 1;
    return sq_note_edit_begin(uisq_edit_ch, uisq_edit_step); }
static uint8_t uisq_ne_cmp(const StepEdit *a, const StepEdit *b) { return (uint8_t)(memcmp(a, b, sizeof *a) != 0); }
static void uisq_ne_wrote(void) { uisq_ne_dirty = 1; uisq_ne_undone = 0; }
static void uisq_ne_set(uint8_t field, int16_t v)                       /* one field of the edited step; v = SQ_AUTO clears it */
{   StepEdit was;
    if (!uisq_ne_ready()) return;
    was = sq_ch[uisq_edit_ch].ed[uisq_edit_step];
    if (sq_step_set(uisq_edit_ch, uisq_edit_step, field, v) && uisq_ne_cmp(&was, &sq_ch[uisq_edit_ch].ed[uisq_edit_step])) uisq_ne_wrote(); }

static int16_t uisq_ne_clamp(int32_t v, int32_t lo, int32_t hi) { return (int16_t)(v < lo ? lo : v > hi ? hi : v); }
static int16_t uisq_ne_cur_st(const PulseOut *gen)                      /* the pitch the edit starts from: its own, else the generated one (F-12) */
{   const StepEdit *e = &sq_ch[uisq_edit_ch].ed[uisq_edit_step];
    return (e->note & 0x80u) ? (int16_t)(e->note & 0x7Fu) : uisq_ne_gen_st(&sq_gdb[sq_gidx], gen->note); }
static int16_t uisq_ne_walk(int16_t st, int32_t n)                      /* n scale degrees (sign = direction) from st; -1 = below the lowest (AUTO) */
{   const SeqGlobal *g = &sq_gdb[sq_gidx];
    const int8_t dir = (int8_t)(n < 0 ? -1 : 1);
    for (int32_t i = 0, m = n < 0 ? -n : n; i < m && i < 12 && st >= 0; i++) st = uisq_ne_degree(g, st, dir);
    return st; }
static void uisq_ne_pitch_moves(const PulseOut *gen, int32_t n)         /* the knob: below the lowest degree the field becomes AUTO */
{   const int16_t st = uisq_ne_walk(uisq_ne_cur_st(gen), n);
    uisq_ne_set(SQ_F_PITCH, st < 0 ? (int16_t)SQ_AUTO : uisq_ne_clamp(st, 0, UISQ_PITCH_MAX)); }
static void uisq_ne_pitch_dice(const PulseOut *gen, int32_t n)          /* a dice: always a SET pitch, so a way blocked at the bottom goes the other way */
{   int16_t st = uisq_ne_walk(uisq_ne_cur_st(gen), n);
    if (st < 0) st = uisq_ne_walk(uisq_ne_cur_st(gen), -n);
    uisq_ne_set(SQ_F_PITCH, uisq_ne_clamp(st < 0 ? 0 : st, 0, UISQ_PITCH_MAX)); }

/* K1..K4: PITCH GATE VELO RATCH; the low end of each is AUTO */
static void uisq_ne_knob(uint8_t k, int32_t d)
{   const StepEdit *e;
    PulseOut gen;
    int32_t v;
    if (k >= 4u || !d || !uisq_ne_ready()) return;
    if (d > 12) d = 12;
    if (d < -12) d = -12;
    e = &sq_ch[uisq_edit_ch].ed[uisq_edit_step];
    if (!uisq_ne_gen(uisq_edit_ch, uisq_edit_step, &gen)) return;
    switch (k) {
    case 0: uisq_ne_pitch_moves(&gen, d); break;
    case 1: {                                                           /* GATE: AUTO, 10 .. 100 %, TIE as indices 0, 1 .. 10, 11 */
        const uint8_t gt = (uint8_t)(e->gr & 15u);
        v = (gt >= 1u && gt <= 11u ? gt : uisq_ne_gen_gate(&gen)) + d;
        uisq_ne_set(SQ_F_GATE, v < 1 ? (int16_t)SQ_AUTO : v >= 11 ? (int16_t)SQ_GATE_TIE : (int16_t)(v * 10));
        break; }
    case 2: {                                                           /* VELO 1..127: a fast turn (3 or more detents in a frame) steps 8 */
        v = (e->vel ? e->vel : gen.vel) + ((d >= 3 || d <= -3) ? d * 8 : d);
        uisq_ne_set(SQ_F_VEL, v < 1 ? (int16_t)SQ_AUTO : uisq_ne_clamp(v, 1, 127));
        break; }
    default: {                                                          /* RATCH 1x .. 4x */
        const uint8_t rt = (uint8_t)((e->gr >> 4) & 7u);
        v = (rt >= 1u && rt <= 4u ? rt : uisq_ne_gen_rep(&gen)) + d;
        uisq_ne_set(SQ_F_RATCH, v < 1 ? (int16_t)SQ_AUTO : uisq_ne_clamp(v, 1, 4));
        break; } }
    ui_sq_knob_touch(k); }

/* the layer buttons act on the edited step (SEQUENCER_UX 3: "a layer button acts on its layer"); OCT- / OCT+ shift its octave */
static void uisq_ne_button(uint8_t ctl)
{   const StepEdit *e;
    PulseOut gen;
    uint32_t r;
    if (!uisq_ne_ready()) return;
    e = &sq_ch[uisq_edit_ch].ed[uisq_edit_step];
    switch (ctl) {
    case UI_C_B_FX: case UI_C_B_SCL: {                                  /* dice this step: SCL its pitch, FX pitch + accent + slide + gate, from fresh entropy */
        if (!uisq_ne_gen(uisq_edit_ch, uisq_edit_step, &gen)) return;
        r = sq_entropy();
        uisq_ne_pitch_dice(&gen, (int32_t)(1u + r % 5u) * ((r >> 3 & 1u) ? 1 : -1));
        if (ctl == UI_C_B_FX) {
            uisq_ne_set(SQ_F_VEL, (r >> 5 & 3u) == 0u ? (int16_t)UISQ_ACC_VEL : (int16_t)UISQ_NOACC_VEL);
            uisq_ne_set(SQ_F_SLIDE, (r >> 7 & 3u) == 0u ? 1 : 0);
            uisq_ne_set(SQ_F_GATE, (int16_t)(10u * (1u + (r >> 9) % 10u))); }
        ui_sq_toast_msg(uisq_edit_ch, uisq_ne_w(ctl == UI_C_B_FX ? UI_NE_TOAST_DICED : UI_NE_TOAST_NOTE), ui_msgs[UI_MSG_TOAST_ALL_EDIT_UNDO].b);   /* [1] DICED  EDIT=UNDO */
        break; }
    case UI_C_B_ENV: {                                                  /* ACCENT: 120, or back (F-10): AUTO, or the explicit 80 when AUTO would accent again */
        if (!uisq_ne_gen(uisq_edit_ch, uisq_edit_step, &gen)) return;
        if ((e->vel ? e->vel : gen.vel) < UISQ_ACC_THR) uisq_ne_set(SQ_F_VEL, (int16_t)UISQ_ACC_VEL);
        else uisq_ne_set(SQ_F_VEL, gen.vel >= UISQ_ACC_THR ? (int16_t)UISQ_NOACC_VEL : (int16_t)SQ_AUTO);
        break; }
    case UI_C_B_LFO: uisq_ne_set(SQ_F_PROB, (e->fl & 0x10u) ? (int16_t)SQ_AUTO : (int16_t)50); break;         /* PROB 50 <-> 100 (AUTO) */
    case UI_C_B_ARP: uisq_ne_set(SQ_F_SLIDE, ((e->fl >> 2 & 3u) == 1u) ? (int16_t)SQ_AUTO : (int16_t)1); break;   /* SLIDE on <-> AUTO */
    case UI_C_B_SEQ: {                                                  /* ON/OFF: the key's own tap */
        const StepEdit was = *e;
        if (sq_step_tap(uisq_edit_ch, uisq_edit_step) && uisq_ne_cmp(&was, &sq_ch[uisq_edit_ch].ed[uisq_edit_step])) uisq_ne_wrote();
        break; }
    case UI_C_B_OCTDN: case UI_C_B_OCTUP: {                             /* OCT -2..+2: 0 is AUTO (no shift) */
        const uint8_t oc = (uint8_t)(e->fl >> 5);
        const int32_t cur = (oc >= 1u && oc <= 5u) ? (int32_t)oc - 3 : 0, nv = cur + (ctl == UI_C_B_OCTUP ? 1 : -1);
        if (nv < -2 || nv > 2) break;
        uisq_ne_set(SQ_F_OCT, nv == 0 ? (int16_t)SQ_AUTO : (int16_t)nv);
        break; }
    case UI_C_B_EDIT:                                                   /* UNDO / REDO the step's edits; nothing to take back = inert (never an older dice or a page add) */
        if (!uisq_ne_dirty || !sq_undo_step_rec(uisq_edit_ch)) { uisq_ne_dirty = 0; break; }   /* another action replaced the record (or a preset cleared it): the page's claim is gone */
        if (sq_undo(uisq_edit_ch)) {
            ui_sq_toast_id(uisq_ne_undone ? UI_MSG_TOAST_REDO_EDIT_UNDO : UI_MSG_TOAST_UNDO_EDIT_REDO, uisq_edit_ch);
            uisq_ne_undone = (uint8_t)!uisq_ne_undone; }
        break;
    default: break; } }

/* the frame's check: the page lives while its channel is the selected one and in use and its step is inside LENGTH (a preset that lands can shorten it) */
static void uisq_pg_note_edit(void)
{   if (uisq_sel != uisq_edit_ch || !(uisq_used >> uisq_edit_ch & 1u) || uisq_edit_ch >= SQ_NCH || uisq_edit_step >= sq_ch[uisq_edit_ch].len) uisq_ne_leave();
    else if (uisq_ne_dirty && !sq_undo_step_rec(uisq_edit_ch)) uisq_ne_dirty = 0; }      /* the session was ended elsewhere: EDIT here has nothing of ours to undo */

/* ---- the CHAN page (P4-015; knob_map / button_modes / key_map CHAN rows, SEQUENCER_UX 1.6, 2.3, 4.4, SCREEN_SPEC_V2 4.2, 5.6) ----
 * Open: ui_sq_chan_open() (the CHN key, D#4 tap, from PERF or NOTE_EDIT); leave: ui_sq_chan_close() (the CHN key again, HOME tap). P4-032 owns the CHN key's
 * hold (RIG) and may call these two; SET -> CHAN switching is the SET task's. K1 LENGTH (sq_len_set), K2 MIDI (pending, committed by sq_set_midi after
 * 0.6 s of stillness and on every exit: HOME, the CHN key, PLAY / GLO / REC / SAVE and any other button, SELECT, a page change by any road), K4 FOLLOW,
 * OCT-/OCT+ = -PAGE / +PAGE, the ALGO encoder (ui_sq_algo_turn). The white keys, M keys, dice and EDIT work as on PERF. K3 SOURCE is drawn (OWN) and
 * inert: its list of offered blocks is not part of this task.
 * Bounds: the pending value is 0..SQ_MIDI_OFF or UISQ_CM_NONE and its channel < SQ_NCH (both re-checked at the commit, which also drops it when the
 * channel left use); LENGTH is clamped 1..SQ_NSTEP; the viewed page is clamped to the pages that LENGTH leaves; the ALGO walk stops at the ends. */
static void uisq_chan_clash_toast(uint8_t other, uint8_t taker, uint8_t midi)    /* `[1] MIDI OFF  2 TO CH2`: CH1 lost MIDI 2 to CH2 */
{   const char *t = ui_msgs[UI_MSG_TOAST_MIDI_OFF_2_TO_CH2].b;
    char h[18];
    uint32_t run = 0;
    h[0] = 0;
    while (*t) {
        if (*t >= '0' && *t <= '9') {                                   /* the table row's digits: the first is the MIDI number, the second the taker */
            uisq_catu(h, sizeof h, run ? (uint32_t)taker + 1u : (uint32_t)midi + 1u);
            run++;
            while (*t >= '0' && *t <= '9') t++; }
        else { char c[2]; c[0] = *t++; c[1] = 0; uisq_cat(h, sizeof h, c); } }
    ui_sq_toast_msg(other, ui_msgs[UI_MSG_TOAST_MIDI_OFF_2_TO_CH2].a, h); }

static void uisq_chan_commit(void)                                      /* no-op without a pending value */
{   const uint8_t ch = uisq_cm_ch, m = uisq_cm_val;
    uisq_cm_val = UISQ_CM_NONE;
    if (m == UISQ_CM_NONE || m > SQ_MIDI_OFF || ch >= SQ_NCH || !(uisq_used >> ch & 1u)) return;
    {   const uint8_t was = sq_ch[ch].midi_ch;
        const int8_t other = sq_set_midi(ch, m);
        if (m < SQ_MIDI_OFF) ui_sq_new_set(ch, 0);                      /* the setup cue clears when the channel gets a MIDI channel */
        if (sq_ch[ch].midi_ch != was) { uisq_gone_n = 0; uisq_undone = 0; }   /* the MIDI record is now the latest: EDIT undoes it, not an older removal */
        if (other >= 0) uisq_chan_clash_toast((uint8_t)other, ch, m); } }

static void ui_sq_chan_open(void)                                       /* the hook for the CHN key (P4-032) */
{   if (uisq_rec || (uisq_page != UI_PG_PERF && uisq_page != UI_PG_NOTE_EDIT && uisq_page != UI_PG_SET) || !(uisq_used >> (uisq_sel % SQ_NCH) & 1u)) return;
    if (uisq_page == UI_PG_NOTE_EDIT) uisq_ne_leave();                  /* closes the note-edit session */
    uisq_pg_pend = 0;
    uisq_cm_val = UISQ_CM_NONE;
    uisq_page = UI_PG_CHAN;
    ui.force = 1; }
static void ui_sq_chan_close(void)
{   if (uisq_page != UI_PG_CHAN) return;
    uisq_chan_commit();
    uisq_page = UI_PG_PERF;
    ui.force = 1; }

static void uisq_pg_chan(void)                                          /* the frame's check: the settle timer, and the page lives while its channel is in use */
{   if (uisq_cm_val != UISQ_CM_NONE && (uisq_cm_ch != uisq_sel || (int32_t)(uisq_now - uisq_cm_t0) >= (int32_t)UISQ_MIDI_SETTLE_MS)) uisq_chan_commit();
    if (!(uisq_used >> (uisq_sel % SQ_NCH) & 1u)) ui_sq_chan_close(); }

static void uisq_chan_pages_clamp(void)                                 /* after LENGTH changed: the viewed page must still exist */
{   const uint8_t len = sq_ch[uisq_sel % SQ_NCH].len ? sq_ch[uisq_sel % SQ_NCH].len : 1u, pages = (uint8_t)((len + SQ_PAGE - 1u) / SQ_PAGE);
    if (uisq_vpage >= pages) uisq_vpage = (uint8_t)(pages - 1u); }

static void uisq_chan_knob(uint8_t k, int32_t d)
{   const uint8_t sel = uisq_sel % SQ_NCH;
    if (!d) return;
    uisq_pg_pend = 0;
    switch (k) {
    case 0: {                                                           /* LENGTH 1..64: step 1, a fast turn (3 or more detents in a frame) steps 4 */
        int32_t n = (int32_t)sq_ch[sel].len + ((d >= 3 || d <= -3) ? d * 4 : d);
        if (n < 1) n = 1;
        if (n > SQ_NSTEP) n = SQ_NSTEP;
        (void)sq_len_set(sel, (uint8_t)n);
        uisq_chan_pages_clamp();
        break; }
    case 1: {                                                           /* MIDI 1..16, OFF: shown at once, committed on stillness / exit; no wrap */
        int32_t n;
        if (uisq_cm_val != UISQ_CM_NONE && uisq_cm_ch != sel) uisq_chan_commit();
        n = (int32_t)(uisq_cm_val != UISQ_CM_NONE ? uisq_cm_val : sq_ch[sel].midi_ch) + d;
        if (n < 0) n = 0;
        if (n > SQ_MIDI_OFF) n = SQ_MIDI_OFF;
        uisq_cm_val = (uint8_t)n; uisq_cm_ch = sel; uisq_cm_t0 = UISQ_MS();
        break; }
    case 3: sq_follow_set(d > 0); break;                                /* FOLLOW: up = ON */
    default: break; }                                                   /* K3 SOURCE: not part of this task */
    ui_sq_knob_touch(k); }

static void uisq_chan_pages(uint8_t ctl)                                /* OCT- / OCT+ = -PAGE / +PAGE */
{   const uint8_t sel = uisq_sel % SQ_NCH, len = sq_ch[sel].len;
    uisq_pg_pend = 0;
    if (ctl == UI_C_B_OCTDN) {                                          /* LENGTH to the page boundary below (20 -> 16, 48 -> 32); at 16 or less inert */
        const uint8_t n = (uint8_t)(len ? ((len - 1u) / SQ_PAGE) * SQ_PAGE : 0u);
        if (n >= 1u) (void)sq_len_set(sel, n);
        else ui_sq_toast_id(UI_MSG_TOAST_ONE_PAGE_K1_LESS, sel); }       /* already one page: say how to shorten it */
    else if (len >= SQ_NSTEP) ui_sq_toast_id(UI_MSG_TOAST_64_STEPS_MAX, sel);
    else if (sq_page_add(sel)) {                      /* a partial page is completed, else a new page (a copy of the page before) */
        uisq_gone_n = 0; uisq_undone = 0;
        uisq_vpage = (uint8_t)((sq_ch[sel].len - 1u) / SQ_PAGE); }
    uisq_chan_pages_clamp(); }

/* the ALGO encoder: the algorithm of the selected channel, over the ALGOS[] rows that are written; no wrap; it lands at the next step (sq_set_algo). Only on
 * CHAN, REC and PERF (P4-032; a held M key is the preview sound's, not this). The strip lists the algorithms for 2 s (ALGO_PICK) and the touched control names the new one. */
static void ui_sq_algo_turn(int32_t d)
{   const uint8_t sel = uisq_sel % SQ_NCH;
    int32_t n = sq_ch[sel].algo, left = d < 0 ? -d : d;
    const int32_t dir = d < 0 ? -1 : 1;
    if ((uisq_page != UI_PG_CHAN && uisq_page != UI_PG_REC && uisq_page != UI_PG_PERF) || !d) return;
    if (uisq_page == UI_PG_PERF && uisq_held_m) return;                 /* PERF with a mute key held: PREVIEW.md's sound turn, not wired yet: no algorithm change by accident */
    uisq_pg_pend = 0;
    if (left > SQ_NALGO) left = SQ_NALGO;
    while (left-- > 0) {
        int32_t m = n + dir;
        while (m >= 0 && m < SQ_NALGO && !uisq_algo_ok((uint8_t)m)) m += dir;
        if (m < 0 || m >= SQ_NALGO) break;
        n = m; }
    ui_sq_algo_pick_touch();
    if (n != sq_ch[sel].algo) {
        const char *t = ui_msgs[UI_MSG_ENC_ALGO_PATT_AT_STEP].b;
        char b[18];
        sq_set_algo(sel, (uint8_t)n);
        uisq_gone_n = 0; uisq_undone = 0;                               /* sq_set_algo cleared the channel's undo record */
        while (*t && *t != ' ') t++;                                    /* the table row's own ` AT STEP` */
        b[0] = 0; uisq_cat(b, sizeof b, uisq_short_of((uint8_t)n)); uisq_cat(b, sizeof b, t);
        ui_sq_touch(ui_msgs[UI_MSG_ENC_ALGO_PATT_AT_STEP].a, b, 0); } }

/* ---- the REC page (P4-017; knob_map / button_modes / key_map REC rows, SEQUENCER_UX 2.5, 5, U54, U67, SCREEN_SPEC_V2 4.2, 4.4, 4.6) ----
 * REC is latched: the REC button opens it from PERF, NOTE_EDIT, CHAN or SET (from NOTE_EDIT the cursor starts on the edited step, else on step 1 of the viewed
 * page), the REC button or a HOME tap leaves. All 27 keys are notes: each plays (see the audition queue below) and WRITES the step as ON + PITCH + VEL (+ SLIDE)
 * in ONE undo record (a note-edit session of its own per key), at the CURSOR when stopped (the cursor then advances; past the end of LENGTH it wraps to step 1)
 * and at the NEAREST step when playing (the step now sounding, or the next one in its second half: ChanRT.grid against sq_now and the DIV's tick length). The
 * pitch is the key's note + the REC octave (K4 / OCT-, OCT+), stored as semitones above 36 + KEY with the global OCTAVE cancelling (sq_note adds it back at play), folded
 * into 0..96 by octaves. Velocity is the FS4 fallback (SEQUENCER_UX 6.1): 90, or 120 with K2 ACC ON (force sensing is P6-006). K3 SLIDE ON writes SLIDE ON, OFF writes AUTO.
 * K1 CURSOR moves the cursor one step per detent when stopped (the step under it sounds if it hits); playing, the cursor IS the nearest step and K1 only frames its tile.
 * EDIT undoes the last write (the cursor steps back to it; again = redo and the cursor goes past it) or whatever the latest undo record is; the dice buttons work
 * (performing while recording); GLO is inert and names how to finish; PRESETS is drained by the glue. SELECT picks a channel (no add) and the cursor returns to step 1.
 * State rules. Every key remembers the note it started (uisq_rec_note), so an octave change or a channel change while it is held cannot hang it; the note-off is
 * owed until the queue takes it (uisq_rec_owed, retried every frame on every page by uisq_rec_flush), on release, on leaving REC and on SELECT. The queue: the UI is
 * the producer, the ISR the consumer (events_block in seq.c pops it into sq_live_on / sq_live_off; sq_aud.h). A full queue drops a
 * note-on (nothing is owed for it) and never an off. */
#include "sq_aud.h"               /* the queue itself (P4-034): uisq_aud_push / ui_sq_aud_next, shared with seq.c, which is included first */
#define UISQ_REC_VEL    90u              /* the fixed velocity (FS4 fallback) */
#define UISQ_REC_VEL_ACC 120u            /* ... with K2 ACC ON */
#define UISQ_REC_AUD_MS 120u             /* the cursor step sounds this long when K1 moves it */
static uint8_t  uisq_rec_note[UISQ_NKEY];                              /* the note a held key started, +1; 0 = none */
static uint32_t uisq_rec_owed;                                         /* bit k: the note-off of key k is still owed to the queue */
static uint8_t  uisq_rec_aud_note;                                     /* K1's step audition: the note +1 until its off is sent */
static uint32_t uisq_rec_aud_t0;
static uint8_t  uisq_rec_ch = 0xFFu;                                   /* the channel the cursor belongs to: a different selection returns it to step 1 */
static uint8_t uisq_rec_len(void) { const uint8_t n = sq_ch[uisq_sel % SQ_NCH].len; return n ? n : 1u; }
static uint8_t uisq_rec_pos(void)                                       /* the cursor as a step of the channel, 0..len-1 */
{   const uint32_t p = (uint32_t)uisq_vpage * SQ_PAGE + (uisq_rec_cursor ? uisq_rec_cursor - 1u : 0u);
    return (uint8_t)(p < uisq_rec_len() ? p : 0u); }
static void uisq_rec_goto(uint8_t p)
{   if (p >= uisq_rec_len()) p = 0;
    uisq_vpage = (uint8_t)(p / SQ_PAGE); uisq_rec_cursor = (uint8_t)(p % SQ_PAGE + 1u); }

/* the step a key press writes while playing: the nearest to the playhead (see the comment above). Reads the ISR's p, grid and sq_now as plain values; p is read
 * twice so a step that starts between the reads is not mixed with the old grid. */
static uint8_t uisq_rec_nearest(void)
{   const uint8_t sel = uisq_sel % SQ_NCH, len = uisq_rec_len();
    uint32_t grid = 0, now = 0;
    uint8_t p = 0, p2 = sq_rt[sel].p;
    int32_t rem, L;
    for (uint8_t i = 0; i < 3u; i++) { p = p2; grid = sq_rt[sel].grid; now = sq_now; p2 = sq_rt[sel].p; if (p == p2) break; }
    p = (uint8_t)(p % len);
    L = SQ_DIV_TICKS[sq_gdb[sq_gidx].s.div > 5u ? 5u : sq_gdb[sq_gidx].s.div];
    rem = (int32_t)(grid - now);                                        /* ticks until the next step starts */
    return (rem < 0 || 2 * rem <= L) ? p : (uint8_t)((p + len - 1u) % len); }

static void uisq_rec_release(uint8_t k)                                 /* the note-off of key k, now or as soon as the queue can take it */
{   if (!uisq_rec_note[k]) return;
    if (uisq_aud_push(0, (uint8_t)(uisq_rec_note[k] - 1u), 0)) uisq_rec_note[k] = 0;
    else uisq_rec_owed |= 1u << k; }
static void uisq_rec_release_all(void)
{   for (uint8_t k = 0; k < UISQ_NKEY; k++) uisq_rec_release(k);
    if (uisq_rec_aud_note && uisq_aud_push(0, (uint8_t)(uisq_rec_aud_note - 1u), 0)) uisq_rec_aud_note = 0; }

/* the written step: ON + PITCH + VEL (+ SLIDE) inside one note-edit session = one undo record. Returns 1 when the step was written. */
static uint8_t uisq_rec_write(uint8_t sel, uint8_t p, uint8_t note, uint8_t vel)
{   int32_t st = (int32_t)note - 36 - sq_gdb[sq_gidx].s.key - 12 * sq_gdb[sq_gidx].s.octave;     /* the key's note as semitones above 36 + KEY (sq_note adds KEY and OCTAVE back) */
    uint8_t ok;
    while (st < 0) st += 12;
    while (st > 96) st -= 12;
    if (!sq_note_edit_begin(sel, p)) return 0;                          /* a step LENGTH hides: nothing is written */
    ok = sq_step_set(sel, p, SQ_F_ON, 1);
    ok &= sq_step_set(sel, p, SQ_F_PITCH, (int16_t)st);
    ok &= sq_step_set(sel, p, SQ_F_VEL, vel);
    ok &= sq_step_set(sel, p, SQ_F_SLIDE, uisq_rec_slide ? (int16_t)1 : (int16_t)SQ_AUTO);
    sq_note_edit_end();
    uisq_gone_n = 0; uisq_undone = 0;                                   /* a write is a newer action than any removal or undo */
    return ok; }

static void uisq_rec_key(uint8_t k, uint8_t down)
{   const uint8_t sel = uisq_sel % SQ_NCH, len = uisq_rec_len();
    int32_t n;
    uint8_t vel, p;
    if (k >= UISQ_NKEY) return;
    if (!down) { uisq_rec_release(k); return; }
    uisq_pg_pend = 0;
    n = 53 + (int32_t)k + 12 * ((int32_t)uisq_rec_oct - 2) + 12 * (int32_t)sq_gdb[sq_gidx].s.octave;   /* F3 = 53, the REC octave and the global OCTAVE */
    if (n < 0) n = 0;
    if (n > 127) n = 127;
    vel = uisq_rec_acc ? UISQ_REC_VEL_ACC : UISQ_REC_VEL;
    uisq_rec_release(k);                                                /* (a key down twice without its up: end the first) */
    if (uisq_aud_push(1, (uint8_t)n, vel)) uisq_rec_note[k] = (uint8_t)(n + 1);
    p = sq_playing ? uisq_rec_nearest() : uisq_rec_pos();
    if (p >= len) p = 0;
    if (uisq_rec_write(sel, p, (uint8_t)n, vel) && !sq_playing) uisq_rec_goto((uint8_t)(p + 1u < len ? p + 1u : 0u)); }

static void uisq_rec_open(void)                                         /* the REC button: from PERF, NOTE_EDIT, CHAN or SET; REC itself leaves (uisq_rec_close) */
{   uint8_t p;
    if (!(uisq_used >> (uisq_sel % SQ_NCH) & 1u)) return;
    if (uisq_page == UI_PG_NOTE_EDIT) { p = uisq_edit_step; uisq_ne_leave(); }       /* the cursor starts on the edited step */
    else { if (uisq_page == UI_PG_CHAN) uisq_chan_commit(); p = (uint8_t)(uisq_view_page() * SQ_PAGE); }
    uisq_pg_pend = 0; ui_sq_held_m_set(0); ui_sq_hold_end();
    uisq_rec = 1; uisq_page = UI_PG_REC; uisq_rec_ch = uisq_sel % SQ_NCH;
    uisq_rec_goto(p);
    ui.force = 1; }
static void uisq_rec_close(void)
{   if (uisq_page != UI_PG_REC) return;
    uisq_rec_release_all();
    uisq_key_fired |= uisq_keys;                                        /* the keys still down belong to REC: their release must not toggle a step on PERF */
    uisq_rec = 0; uisq_page = UI_PG_PERF; uisq_rec_ch = 0xFFu;
    ui.force = 1; }

/* every frame, on every page: the note-offs still owed to the queue and the end of K1's step audition (a note must end whichever page the keys left it on) */
static void uisq_rec_flush(void)
{   if (uisq_rec_owed) for (uint8_t k = 0; k < UISQ_NKEY; k++) if (uisq_rec_owed >> k & 1u) {
        if (!uisq_rec_note[k]) { uisq_rec_owed &= ~(1u << k); continue; }
        if (uisq_aud_push(0, (uint8_t)(uisq_rec_note[k] - 1u), 0)) { uisq_rec_note[k] = 0; uisq_rec_owed &= ~(1u << k); } }
    if (uisq_rec_aud_note && (int32_t)(uisq_now - uisq_rec_aud_t0) >= (int32_t)UISQ_REC_AUD_MS && uisq_aud_push(0, (uint8_t)(uisq_rec_aud_note - 1u), 0)) uisq_rec_aud_note = 0; }

/* A step turned ON while the sequencer is stopped sounds once (key_map PERF white key): the note goes on through the queue (P4-034) and its off is pushed by
 * uisq_rec_flush after UISQ_REC_AUD_MS, every frame on every page, so it ends whichever page or channel the player moves to. Shares K1's audition slot. */
static void uisq_step_audition(uint8_t ch, uint8_t p)
{   UisqStep L;
    if (sq_playing || ch >= SQ_NCH) return;
    uisq_step_read(&L, ch, p);
    if (uisq_rec_aud_note && uisq_aud_push(0, (uint8_t)(uisq_rec_aud_note - 1u), 0)) uisq_rec_aud_note = 0;   /* the last one first */
    if (L.on && !uisq_rec_aud_note && uisq_aud_push(1, L.note, L.vel)) { uisq_rec_aud_note = (uint8_t)(L.note + 1u); uisq_rec_aud_t0 = UISQ_MS(); } }

static void uisq_pg_rec(void)                                           /* the page's frame: the cursor follows the playhead */
{   const uint8_t len = uisq_rec_len();
    if (uisq_rec_ch != uisq_sel % SQ_NCH) { uisq_rec_ch = uisq_sel % SQ_NCH; uisq_vpage = 0; uisq_rec_cursor = 1; }   /* another channel (SELECT, or an undo that brought one back): step 1 */
    if (sq_playing) uisq_rec_goto(uisq_rec_nearest());                  /* playing, the cursor is the nearest step (the view follows it) */
    else if ((uint32_t)uisq_vpage * SQ_PAGE + uisq_rec_cursor > len) uisq_rec_goto(0);   /* LENGTH shrank under the cursor */
    if (!uisq_rec_cursor || uisq_rec_cursor > SQ_PAGE) uisq_rec_cursor = 1; }

static void uisq_rec_knob(uint8_t k, int32_t d)
{   if (!d) return;
    if (d > 127) d = 127;
    if (d < -127) d = -127;
    uisq_pg_pend = 0;
    switch (k) {
    case 0:
        if (!sq_playing) {                                              /* stopped: the cursor moves one step per detent, wraps over LENGTH, and the step sounds */
            const int32_t len = uisq_rec_len();
            const uint8_t sel = uisq_sel % SQ_NCH;
            UisqStep L;
            uisq_rec_goto((uint8_t)((((int32_t)uisq_rec_pos() + d) % len + len) % len));
            uisq_step_read(&L, sel, uisq_rec_pos());
            if (uisq_rec_aud_note && uisq_aud_push(0, (uint8_t)(uisq_rec_aud_note - 1u), 0)) uisq_rec_aud_note = 0;
            if (L.on && !uisq_rec_aud_note && uisq_aud_push(1, L.note, L.vel)) { uisq_rec_aud_note = (uint8_t)(L.note + 1u); uisq_rec_aud_t0 = UISQ_MS(); } }
        break;
    case 1: uisq_rec_acc = (uint8_t)(d > 0); break;
    case 2: uisq_rec_slide = (uint8_t)(d > 0); break;
    default: { const int32_t n = (int32_t)uisq_rec_oct + (d > 4 ? 4 : d < -4 ? -4 : d); uisq_rec_oct = (uint8_t)(n < 0 ? 0 : n > 4 ? 4 : n); break; } }
    ui_sq_knob_touch(k); }
static void uisq_rec_octb(uint8_t ctl)                                  /* OCT- / OCT+: the REC octave, the same value as K4 */
{   const int32_t n = (int32_t)uisq_rec_oct + (ctl == UI_C_B_OCTUP ? 1 : -1);
    uisq_rec_oct = (uint8_t)(n < 0 ? 0 : n > 4 ? 4 : n);
    ui_sq_knob_touch(3); }

/* PLAY, GLO, REC, SAVE and the buttons no page owns yet: on CHAN they commit the pending MIDI channel (an exit); their own actions are the later tasks' */
/* GLO (P4-016): a tap opens SET from PERF, NOTE_EDIT or CHAN and closes it again (button_modes GLO rows). Acts on press for now: the hold = HARMONY (P5-006) will move it
 * to release. Inert in REC. The other buttons here are exits for a pending CHAN MIDI channel. */
static void ui_sq_other_button(uint8_t ctl)
{   if (ctl == UI_C_B_REC) {                                            /* REC (P4-017): latched; the same button opens and closes */
        if (uisq_page == UI_PG_REC) uisq_rec_close();
        else if (uisq_page == UI_PG_PERF || uisq_page == UI_PG_NOTE_EDIT || uisq_page == UI_PG_CHAN || uisq_page == UI_PG_SET) uisq_rec_open();
        return; }
    if (uisq_page == UI_PG_CHAN) uisq_chan_commit();
    if (ctl == UI_C_B_GLO && uisq_page == UI_PG_REC) { ui_sq_touch_id(UI_MSG_ENC_GLO_REC_DONE, 0); return; }   /* inert while recording; the slot says how to finish */
    if (ctl != UI_C_B_GLO || uisq_rec) return;
    if (uisq_page == UI_PG_SET) { uisq_page = UI_PG_PERF; ui.force = 1; }
    else if (uisq_page == UI_PG_PERF || uisq_page == UI_PG_NOTE_EDIT || uisq_page == UI_PG_CHAN) {
        if (uisq_page == UI_PG_NOTE_EDIT) uisq_ne_leave();
        uisq_pg_pend = 0; uisq_page = UI_PG_SET; ui.force = 1; } }

/* HOME (level, once per frame): a tap under 0.7 s leaves NOTE_EDIT or CHAN for PERF; a longer hold is the MENU's (ui_input.c) and does not come here */
static void ui_sq_home(uint8_t down)
{   if (down && !uisq_home_dn) { uisq_home_t0 = UISQ_MS(); uisq_chan_commit(); }     /* HOME down commits a pending MIDI channel (a hold goes to the MENU) */
    else if (!down && uisq_home_dn && UISQ_MS() - uisq_home_t0 < 700u) {
        if (uisq_page == UI_PG_NOTE_EDIT) uisq_ne_leave();
        else if (uisq_page == UI_PG_CHAN) ui_sq_chan_close();
        else if (uisq_page == UI_PG_SET) { uisq_page = UI_PG_PERF; ui.force = 1; }
        else if (uisq_page == UI_PG_REC) uisq_rec_close(); }
    uisq_home_dn = down ? 1u : 0u; }

/* SELECT (U02): channel CH1..CH4 clamped; one detent past the last channel in use adds the next free one (SCREEN_SPEC_V2 6) */
static void ui_sq_select_turn(int32_t d)
{   static const char t_space[2] = { 0x20, 0 };
    char b[18];
    if (uisq_page == UI_PG_NOTE_EDIT && d) uisq_ne_leave();             /* SELECT closes NOTE_EDIT (the edited step belongs to the old channel), then moves */
    if ((uisq_page != UI_PG_PERF && uisq_page != UI_PG_CHAN && uisq_page != UI_PG_SET && uisq_page != UI_PG_REC) || !d) return;
    uisq_chan_commit();                                                 /* CHAN: the pending MIDI channel belongs to the channel being left */
    uisq_pg_pend = 0;
    if (d > 0) {
        const uint8_t nx = uisq_next_used(uisq_sel, 1);
        if (nx != uisq_sel) uisq_sel = nx;
        else if (uisq_page == UI_PG_PERF) {                             /* (CHAN and SET select only: clamped, no add) */
            uint8_t f = SQ_NCH;
            for (uint8_t c = 0; c < SQ_NCH; c++) if (!(uisq_used >> c & 1u)) { f = c; break; }
            if (f < SQ_NCH) {                                           /* add: the next free slot, new (setup cue), unmuted */
                uisq_used |= (uint8_t)(1u << f);
                uisq_gone_n = 0; uisq_undone = 0;                       /* an add is a newer action than any removal or undo */
                sq_set_mute(f, 0);
                ui_sq_new_set(f, 1);
                uisq_sel = f;
                uisq_vpage = 0;
                ui_sq_toast_id(UI_MSG_TOAST_NEW_ALGO_PICK, f); } } }
    else uisq_sel = uisq_next_used(uisq_sel, -1);
    { const char *t = ui_msgs[UI_MSG_ENC_SELECT_CH2_EUCLID].b; uint32_t n = 0;       /* the row's own `CH` prefix, then the number and the algorithm */
      while (t[n] && (t[n] < '0' || t[n] > '9') && n + 1u < sizeof b) { b[n] = t[n]; n++; }
      b[n] = 0; }
    uisq_catu(b, sizeof b, (uint32_t)uisq_sel + 1u);
    uisq_cat(b, sizeof b, t_space); uisq_cat(b, sizeof b, uisq_short_of(sq_ch[uisq_sel].algo));
    ui_sq_touch(ui_msgs[UI_MSG_ENC_SELECT_CH2_EUCLID].a, b, 0); }

/* K1..K4 (U03): one morph of the selected channel's live row; the algorithm owns what the knob means (sq_morph_turn) */
/* SET K1..K4 (P4-016; knob_map SET rows): BPM 40..240 (a fast turn, 3 or more detents in a frame, steps 5), SWING 50..75, GATE 10..100 (fast 5), DIV 0..5; every
 * value clamps at its ends (no wrap). K1 is inert while an external clock runs (the own tempo is kept; the slot says why). Globals go through sq_global_set (an
 * off-side build, one flip: a refused call, a load pending, changes nothing and the tile keeps reading the live value); BPM goes through uisq_bpm_req. */
static void uisq_set_knob(uint8_t k, int32_t d)
{   const SeqGlobal *g = &sq_gdb[sq_gidx];
    const int32_t fast = (d >= 3 || d <= -3);
    int32_t n;
    if (!d) return;
    switch (k) {
    case 0:
        ui_sq_knob_touch(0);
        if (uisq_clock != UISQ_CLK_INT) { ui_sq_touch_id(UI_MSG_ENC_K1_HOST_SETS_TEMPO, 0); return; }
        n = (int32_t)sq_bpm + (fast ? d * 5 : d);
        if (n < 40) n = 40;
        if (n > 240) n = 240;
        sq_bpm = (uint16_t)n; uisq_bpm_req = (uint16_t)n;
        return;
    case 1: n = (int32_t)g->s.swing + d; (void)sq_global_set(SQ_G_SWING, (int16_t)(n < 50 ? 50 : n > 75 ? 75 : n)); break;
    case 2: n = (int32_t)g->s.gate + (fast ? d * 5 : d); (void)sq_global_set(SQ_G_GATE, (int16_t)(n < 10 ? 10 : n > 100 ? 100 : n)); break;
    default: n = (int32_t)g->s.div + d; (void)sq_global_set(SQ_G_DIV, (int16_t)(n < 0 ? 0 : n > 5 ? 5 : n)); break; }
    ui_sq_knob_touch(k); }

/* OCT- / OCT+ on SET: the global OCTAVE -2..+2 (clamped), named in the slot while touched: OCT+ OCTAVE +1 */
static void uisq_set_oct(uint8_t ctl)
{   const uint8_t up = (uint8_t)(ctl == UI_C_B_OCTUP);
    int32_t n = (int32_t)sq_gdb[sq_gidx].s.octave + (up ? 1 : -1);
    const uint8_t id = up ? UI_MSG_ENC_OCT_OCTAVE_1 : UI_MSG_ENC_OCT_OCTAVE_2;
    char b[18];
    if (n < -2) n = -2;
    if (n > 2) n = 2;
    (void)sq_global_set(SQ_G_OCTAVE, (int16_t)n);
    n = sq_gdb[sq_gidx].s.octave;                                       /* what is live now (a refused change keeps the old one) */
    b[0] = 0; uisq_cat(b, sizeof b, uisq_st_w(UI_ST_OCTAVE)); uisq_cat(b, sizeof b, " ");
    if (n) { uisq_cat(b, sizeof b, uisq_ne_w(n > 0 ? UI_NE_PLUS : UI_NE_MINUS)); uisq_catu(b, sizeof b, (uint32_t)(n > 0 ? n : -n)); } else uisq_cat(b, sizeof b, "0");
    ui_sq_touch(ui_msgs[id].a, b, 0); }

static void ui_sq_knob(uint8_t k, int32_t d)
{   if (uisq_page == UI_PG_NOTE_EDIT) { uisq_ne_knob(k, d); return; }
    if (uisq_page == UI_PG_SET && k < 4u) { if (d > 127) d = 127; if (d < -127) d = -127; uisq_set_knob(k, d); return; }
    if (uisq_page == UI_PG_CHAN && k < 4u) { if (d > 127) d = 127; if (d < -127) d = -127; uisq_chan_knob(k, d); return; }
    if (uisq_page == UI_PG_REC && k < 4u) { uisq_rec_knob(k, d); return; }
    if (uisq_page != UI_PG_PERF || k >= 4u || !d || uisq_rec || uisq_row >= 2u) return;
    uisq_pg_pend = 0;
    if (d > 127) d = 127;
    if (d < -127) d = -127;
    sq_morph_turn(uisq_sel, (uint8_t)(uisq_row * 4u + k), (int8_t)d);
    ui_sq_knob_touch(k); }

static void uisq_do_remove(uint8_t ch)                                  /* removal of a channel: out of use and muted (the last one stays) */
{   uint8_t left = (uint8_t)(uisq_used & ~(1u << ch));
    if (ch >= SQ_NCH || !(uisq_used >> ch & 1u) || !left) return;     /* not in use: nothing to remove (no stack entry, no mute) */
    if (uisq_gone_n >= SQ_NCH) {                                        /* full: drop the oldest */
        for (uint8_t i = 1; i < SQ_NCH; i++) { uisq_gone_ch[i - 1] = uisq_gone_ch[i]; uisq_gone_muted[i - 1] = uisq_gone_muted[i]; uisq_gone_prv[i - 1] = uisq_gone_prv[i]; uisq_gone_midi[i - 1] = uisq_gone_midi[i]; }
        uisq_gone_n = (uint8_t)(SQ_NCH - 1u); }
    uisq_gone_ch[uisq_gone_n] = ch; uisq_gone_muted[uisq_gone_n] = sq_ch[ch].muted; uisq_gone_prv[uisq_gone_n] = (uint8_t)(sq_ch[ch].pad[0] & 0x80u); uisq_gone_midi[uisq_gone_n] = sq_ch[ch].midi_ch; uisq_gone_n++;
    uisq_used = left;
    if (uisq_cm_ch == ch) uisq_cm_val = UISQ_CM_NONE;                   /* a pending MIDI channel of the removed channel is dropped */
    sq_set_mute(ch, 1);
    sq_ch[ch].midi_ch = SQ_MIDI_OFF;                                    /* BUG-011: not in use = holds no MIDI channel, so sq_set_midi never steals from it (and names it in a toast) */
    sq_um_valid = 0;                                                    /* the removal is newer than any MIDI record: a later sq_undo must not swap a stale pair */
    sq_ch[ch].pad[0] &= 0x7Fu;                                          /* a removed channel must not sound on the built-in synth either */
    if (uisq_sel == ch) { uint8_t n = uisq_next_used(ch, -1); if (n == ch) n = uisq_next_used(ch, 1); uisq_sel = n; uisq_vpage = 0; }
    ui_sq_toast_id(UI_MSG_TOAST_GONE_EDIT_UNDO, ch); }

/* the dice buttons and EDIT (U10..U13), the OCT row buttons; ctl is a UI_C_B_* id */
static const uint8_t uisq_dice_msg[SQ_NLAYER] = { UI_MSG_TOAST_NOTES_EDIT_UNDO, UI_MSG_TOAST_HITS_EDIT_UNDO, UI_MSG_TOAST_ACCNT_EDIT_UNDO,
                                                  UI_MSG_TOAST_ARTIC_EDIT_UNDO, UI_MSG_TOAST_SHAPE_EDIT_UNDO };
static void ui_sq_button(uint8_t ctl)
{   const uint8_t sel = uisq_sel % SQ_NCH;
    const int8_t ly = uisq_layer_of(ctl);
    if (uisq_page == UI_PG_NOTE_EDIT) { uisq_ne_button(ctl); return; }
    if (uisq_page != UI_PG_PERF && uisq_page != UI_PG_CHAN && uisq_page != UI_PG_SET && uisq_page != UI_PG_REC) return;
    if (uisq_page == UI_PG_REC && (ctl == UI_C_B_OCTDN || ctl == UI_C_B_OCTUP)) { uisq_pg_pend = 0; uisq_rec_octb(ctl); return; }
    if (uisq_page == UI_PG_SET && (ctl == UI_C_B_OCTDN || ctl == UI_C_B_OCTUP)) { uisq_pg_pend = 0; uisq_set_oct(ctl); return; }
    if (uisq_page == UI_PG_CHAN) {
        uisq_chan_commit();                                             /* any button is an exit for the pending MIDI channel */
        if (ctl == UI_C_B_OCTDN || ctl == UI_C_B_OCTUP) { uisq_chan_pages(ctl); return; } }
    uisq_pg_pend = 0;
    if (ly >= 0 || ctl == UI_C_B_FX) {                                  /* dice: a layer, or all of them (FX) */
        const AlgoDesc *a = &ALGOS[sq_ch[sel].algo % SQ_NALGO];
        UisqStep before[SQ_PAGE], after[SQ_PAGE];
        uint16_t diff = 0;
        const uint8_t vp = uisq_view_page();
        uint8_t any = 0;
        if (ctl == UI_C_B_FX) { for (uint8_t l = 0; l < SQ_NLAYER; l++) any |= (uint8_t)(a->layer[l].n != 0); }
        else any = (uint8_t)(a->layer[ly].n != 0);
        if (!any) return;                                               /* an absent layer is inert: dim on the button, no change, no toast */
        uisq_lane_read(before, sel, vp);
        if (ctl == UI_C_B_FX) sq_dice_all(sel); else sq_dice_layer(sel, (uint8_t)ly);
        uisq_lane_read(after, sel, vp);
        for (uint8_t n = 0; n < SQ_PAGE; n++) if (memcmp(&before[n], &after[n], sizeof before[n])) diff |= (uint16_t)(1u << n);
        ui_sq_diced_set(diff);
        uisq_gone_n = 0; uisq_undone = 0;
        ui_sq_toast_id(ctl == UI_C_B_FX ? UI_MSG_TOAST_ALL_EDIT_UNDO : uisq_dice_msg[ly], sel);
        return; }
    switch (ctl) {
    case UI_C_B_EDIT:                                                   /* UNDO / REDO; a channel just removed comes back first */
        if (uisq_gone_n) {
            const uint8_t i = (uint8_t)(uisq_gone_n - 1u), c = uisq_gone_ch[i];
            {   uint8_t m = uisq_gone_midi[i];                                   /* BUG-011: the MIDI channel comes back unless an in-use channel holds it meanwhile (then OFF, never a duplicate) */
                for (uint8_t o = 0; o < SQ_NCH; o++) if (o != c && (uisq_used >> o & 1u) && sq_ch[o].midi_ch == m) m = SQ_MIDI_OFF;
                sq_ch[c].midi_ch = m; }
            uisq_used |= (uint8_t)(1u << c); sq_set_mute(c, uisq_gone_muted[i]); sq_ch[c].pad[0] |= uisq_gone_prv[i]; uisq_gone_n = i; uisq_undone = 0; uisq_sel = c;
            ui_sq_toast_id(UI_MSG_TOAST_UNDO_EDIT_REDO, c); }
        else {
            const uint8_t stp = (uint8_t)(uisq_page == UI_PG_REC && sq_undo_step_rec(sel)), at = sq_undo_rec[sel].idx;   /* REC: a written step's record moves the cursor */
            if (sq_undo(sel)) {
                if (stp) uisq_rec_goto(uisq_undone ? (uint8_t)(at + 1u) : at);     /* undo: the cursor steps back onto the step; redo: past it */
                ui_sq_toast_id(uisq_undone ? UI_MSG_TOAST_REDO_EDIT_UNDO : UI_MSG_TOAST_UNDO_EDIT_REDO, sel);
                uisq_undone = (uint8_t)!uisq_undone; } }
        break;
    case UI_C_B_OCTDN: uisq_row = 0; break;                             /* ROW 1, from any row */
    case UI_C_B_OCTUP: {
        const uint8_t nrow = uisq_rig_row(sel) ? 3u : 2u;
        if (uisq_row + 1u < nrow) uisq_row++; else ui_sq_toast_id(UI_MSG_TOAST_LAST_ROW_OCT_1, 0xFFu);
        break; }
    default: break; } }

/* a key press (edge): the hold timers start; the actions that act on press run here */
static void uisq_key_down(uint8_t k)
{   const uint8_t slot = uisq_key_slot(k);
    uisq_key_t0[k] = UISQ_MS();
    if (uisq_page == UI_PG_REC) { uisq_rec_key(k, 1); return; }          /* REC: every key is a note */
    if (!uisq_keys_live()) return;       /* NOTE_EDIT: the white keys and the black keys work as on PERF */
    if (!(slot & 0x80u)) { uisq_pg_pend = 0; return; }                  /* a white key: a tap or a hold, decided by the time */
    switch (slot & 0x7Fu) {
    case 0: case 1: case 2: case 3: {                                   /* M1..M4 */
        const uint8_t c = (uint8_t)(slot & 0x7Fu);
        uisq_pg_pend = 0;
        if (!(uisq_used >> c & 1u)) break;
        ui_sq_held_m_set((uint8_t)(c + 1u));
        uisq_hold_ch(UI_MSG_HOLD_PREVIEW_CH4, c);
        break; }
    case 8: {                                                           /* PG-: the previous page (no wrap); FOLLOW goes off */
        const uint8_t vp = uisq_view_page();
        uisq_pg_pend = 0;
        sq_follow_set(0);
        uisq_vpage = vp ? (uint8_t)(vp - 1u) : 0u;
        break; }
    case 9: {                                                           /* PG+: the next page; on the last one a prompt, then the add */
        const uint8_t len = sq_ch[uisq_sel % SQ_NCH].len, pages = (uint8_t)((len + SQ_PAGE - 1u) / SQ_PAGE), vp = uisq_view_page();
        sq_follow_set(0);
        if (vp + 1u < pages) { uisq_vpage = (uint8_t)(vp + 1u); uisq_pg_pend = 0; }
        else if (len < SQ_NSTEP) {
            uisq_vpage = vp;
            if (uisq_pg_pend) { (void)sq_page_add(uisq_sel); uisq_pg_pend = 0; uisq_vpage = (uint8_t)(vp + 1u); uisq_gone_n = 0; uisq_undone = 0; }
            else { uisq_pg_pend = 1; uisq_msg_toast_n(UI_MSG_TOAST_PAGE_2_PG_ADD, uisq_sel, (uint8_t)(vp + 2u)); } }
        else { uisq_pg_pend = 0; ui_sq_toast_id(UI_MSG_TOAST_64_STEPS_MAX, uisq_sel); }   /* page 4 at LENGTH 64: nothing to add */
        break; }
    default: uisq_pg_pend = 0; break; } }

static void uisq_key_up(uint8_t k)
{   const uint8_t slot = uisq_key_slot(k), fired = (uint8_t)(uisq_key_fired >> k & 1u);
    const uint32_t el = UISQ_MS() - uisq_key_t0[k];
    uisq_key_fired &= ~(1u << k);
    if ((slot & 0x80u) && (slot & 0x7Fu) < 4u) { ui_sq_held_m_set(0); ui_sq_hold_end(); }   /* the M-key hint ends with the key, on any page */
    if (uisq_page == UI_PG_REC) { uisq_rec_key(k, 0); return; }
    if (!uisq_keys_live()) return;
    if (!(slot & 0x80u)) {                                              /* a white key released before the hold time: step n ON/OFF (U14) */
        if (!fired) {
            const uint8_t p = (uint8_t)(uisq_view_page() * SQ_PAGE + slot);
            const StepEdit was = sq_ch[uisq_sel % SQ_NCH].ed[p % SQ_NSTEP];
            if (p >= sq_ch[uisq_sel % SQ_NCH].len) { ui_sq_toast_id(UI_MSG_TOAST_PAST_THE_END_LEN, uisq_sel); return; }   /* past LENGTH: say so, change nothing */
            if (sq_step_tap(uisq_sel, p)) {
                if (uisq_page == UI_PG_NOTE_EDIT && uisq_ne_cmp(&was, &sq_ch[uisq_sel % SQ_NCH].ed[p % SQ_NSTEP])) uisq_ne_wrote();   /* (NOTE_EDIT: EDIT may take it back) */
                uisq_step_audition(uisq_sel, p); } }
        return; }
    if (slot == (0x80u | 4u)) {                                         /* CHN (D#4): a tap opens CHAN from PERF / NOTE_EDIT, and closes it (key_map CHN / BACK); a longer hold is RIG (P4-032) */
        const uint16_t hold = uisq_hold_ms(UI_C_KEY_B05_DS4);
        if (!fired && (uisq_page == UI_PG_CHAN || !hold || el < hold)) { if (uisq_page == UI_PG_CHAN) ui_sq_chan_close(); else ui_sq_chan_open(); }
        return; }
    if ((slot & 0x7Fu) < 4u) {                                          /* an M key released: a tap mutes, a hold under 1.5 s toggles the preview */
        const uint8_t c = (uint8_t)(slot & 0x7Fu);
        ui_sq_held_m_set(0); ui_sq_hold_end();
        if (fired || !(uisq_used >> c & 1u)) return;
        if (el < uisq_hold_ms(uisq_key_ctl(k))) sq_set_mute(c, (uint8_t)!sq_ch[c].muted);
        else { sq_ch[c].pad[0] ^= 0x80u; ui_sq_toast_id((sq_ch[c].pad[0] & 0x80u) ? UI_MSG_TOAST_PREVIEW_ON : UI_MSG_TOAST_PREVIEW_OFF, c); } } }                              /* PREVIEW on/off: bit 7 of the preview byte (PREVIEW.md) */

/* the keys: diff against the last frame, then the hold timers (the white key 0.4 s -> NOTE_EDIT, the M key 1.5 s -> remove) */
static void ui_sq_keys(uint32_t mask)
{   uint32_t ch;
    mask &= (1u << UISQ_NKEY) - 1u;
    ch = mask ^ uisq_keys;
    for (uint8_t k = 0; k < UISQ_NKEY; k++) {
        if (!(ch >> k & 1u)) continue;
        if (mask >> k & 1u) uisq_key_down(k); else uisq_key_up(k); }
    uisq_keys = mask; }
static void uisq_keys_tick(void)
{   if (!uisq_keys_live()) return;
    for (uint8_t k = 0; k < UISQ_NKEY; k++) {
        const uint8_t slot = uisq_key_slot(k);
        const uint32_t el = UISQ_MS() - uisq_key_t0[k];
        if (!(uisq_keys >> k & 1u) || (uisq_key_fired >> k & 1u)) continue;
        if (!(slot & 0x80u)) {
            if (el >= uisq_hold_ms(uisq_key_ctl(k))) {                                /* the white key held: NOTE_EDIT on this step, ONE undo record for the session */
                const uint8_t p = (uint8_t)(uisq_view_page() * SQ_PAGE + slot);
                uisq_key_fired |= 1u << k;
                if (uisq_page == UI_PG_NOTE_EDIT && p == uisq_edit_step && uisq_edit_ch == uisq_sel) continue;   /* the editor is already here: keep its session */
                if (sq_note_edit_begin(uisq_sel, p)) {                  /* open, or move the editor (a new snapshot: the last change wins) */
                    uisq_edit_step = p; uisq_edit_ch = uisq_sel; uisq_vpage = (uint8_t)(p / SQ_PAGE);
                    uisq_ne_dirty = 0; uisq_ne_undone = 0;
                    uisq_page = UI_PG_NOTE_EDIT; ui.force = 1; } } }
        else if ((slot & 0x7Fu) < 4u && el >= uisq_remove_ms(uisq_key_ctl(k))) {      /* the M key held 1.5 s */
            uisq_key_fired |= 1u << k; ui_sq_held_m_set(0); ui_sq_hold_end();
            uisq_do_remove((uint8_t)(slot & 0x7Fu)); } } }


/* ---- the frame ---- */
static void ui_sq_input(void)
{   uisq_now = UISQ_MS();
    uisq_rec_flush();
    uisq_keys_tick();
    if (uisq_toast_until && (int32_t)(uisq_now - uisq_toast_until) >= 0) uisq_toast_until = 0;
    if (uisq_pick_until && (int32_t)(uisq_now - uisq_pick_until) >= 0) uisq_pick_until = 0;
    if (uisq_home_held) uisq_toast_until = 0;                          /* row 1 (the HELP line) outranks a toast */
    if (uisq_sel >= SQ_NCH) uisq_sel = 0;
    if (uisq_vpage >= SQ_NSTEP / SQ_PAGE) uisq_vpage = 0;
    if (uisq_page != UI_PG_CHAN) uisq_chan_commit();                    /* a pending MIDI channel never outlives CHAN, whichever road left it */
    if (uisq_page < UI_PG_COUNT && uisq_pages[uisq_page].input) uisq_pages[uisq_page].input(); }

/* Draws the strips whose signature changed (or both when `ui.force`). Returns 1 if anything was blitted. The MENU page is the Felucca menu's.
 * Signature 0 (rows 0-119): the page, the palette (ux.gen), the view (header, knob tiles, status line, the music row's upper part, buttons), and the
 * message slot (uisq_msg_sig: a toast, a touched control or a filling hold bar redraws this strip only). Signature 1 (rows 120-239): the same view without
 * the message slot, and the cue strip text. */
static uint8_t ui_sq_draw(void)
{   uint32_t sg[UISQ_NSTRIP] = { 2166136261u, 2166136261u };
    uint8_t drew = 0;
    char cue[48];
    UisqView view;
    if (uisq_page == UI_PG_MENU) return 0;
    uisq_now = UISQ_MS();
    uisq_cue_text(cue, sizeof cue);
    uisq_view_fill(&view);
    for (uint32_t s = 0; s < UISQ_NSTRIP; s++) { sg[s] = uisq_h(sg[s], uisq_page); sg[s] = uisq_h(sg[s], ux.gen); sg[s] = uisq_h(sg[s], uisq_view_sig(&view)); }
    sg[0] = uisq_h(sg[0], uisq_msg_sig(&view));
    for (const char *s = cue; *s; s++) sg[1] = uisq_h(sg[1], (uint8_t)*s);
    for (uint32_t s = 0; s < UISQ_NSTRIP; s++) {
        if (!ui.force && uisq_have_sig && sg[s] == uisq_sig[s]) continue;
        cv_begin(UISQ_W, UISQ_STRIP_H, T_BG);
        cv_oy = -(int32_t)uisq_strip_y[s];                              /* the elements draw in screen coordinates */
        uisq_draw_elements(&view);
        cv_oy = 0;
        cv_blit(0, uisq_strip_y[s]);
        uisq_sig[s] = sg[s]; drew = 1; }
    uisq_have_sig = 1;
    return drew; }
