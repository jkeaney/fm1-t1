/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* FELUCCA user interface.
 * Flat: SURF cards and panels on the palette's background, no rules, one type family (Inter Tight, three sizes), tracks
 * named by their numbers 1..4 on a cushion (icons.c trk_icon). Four columns <-> KNOB 1..4. Rendering is lazy:
 * every element remembers what it last drew and is redrawn only on change. */
static void panel_setup(void);
#include "favorites.c"
/* MENU's two-valued settings (ui_menu.c MENU_FLAGS), a bit each in a byte no engine uses (as ui_layer.c layer_seen):
 * saved with the settings; 0 in older ones = every setting's default (append-only: a new setting takes a new bit,
 * its default is 0) */
#define ui_prefs (favorites.factory[15][30])
#define PREF_ANIM_OFF 2u                       /* MENU > ANIM OFF (#46): values snap (no rolling digits, no glide) */
#define PREF_ACCEL 4u                          /* MENU > KNOB ACCEL ON (#52): fast turns of wide values x2..x8 */
#define PREF_USB_FIXED 8u                      /* MENU > USB LEVEL FIXED: USB audio at the full level, MASTER after */
#define PREF_LARGE 32u                         /* MENU > LARGE ON (#15, Discussion #80): big knob labels and values
                                                * (ui_draw.c large_kind); clear in every older setting = OFF */
#define PREF_SERIAL_OFF 64u                    /* MENU > USB SERIAL OFF (#67): no serial console, the device enumerates as
                                                * audio + MIDI only (usb.c usb_cdc_switch); clear in every older setting = ON */
#define PREF_RESTORE_OFF 128u                  /* MENU > RESTORE LAST OFF (1.2, Discussion #130): no autosave written, none
                                                * restored at power-on (project.c); clear in every older setting = ON */
/* MENU > AUDIO CLICK, CLICK LEVEL, COUNT-IN (1.1, Discussion #131; click.c, seq.c): two bits each in another byte no
 * engine uses, saved with the settings: bits 0-1 CLICK (OFF REC ON), 2-3 CLICK LEVEL (stored 0 MID, 1 LOW, 2 HIGH),
 * 4-5 COUNT-IN (OFF, 1 BAR, 2 BARS); 0 in older settings = OFF / MID / OFF, so nothing clicks or counts in that did
 * not; 3 (unknown) reads as 0 (settings_persist.c puts it back). Bit 6 (1.2): MENU > SCALE LEDS ON (Discussion #127,
 * ui_input.c scale_leds; 0 in older settings = OFF). Bit 7 (1.5.5, #208): MENU > MIDI > AUTO SELECT ON */
#define ui_rec_prefs (favorites.factory[15][28])
#define PREF_REC 0xFF00u                       /* (menu_items.c MENU_FLAGS: a bit of ui_rec_prefs, << 8) */
#define PREF_SCALE_LEDS (0x40u << 8)
#define PREF_MIDI_FOLLOW (0x80u << 8)          /* 1.5.5 (#208): MENU > MIDI > AUTO SELECT ON, bit 7 of ui_rec_prefs (0 in
                                                * every older setting = OFF; ui_input.c midi_follow) */
/* 1.2: three more two-valued MENU settings, bits of another byte no engine uses, saved with the settings; 0 in older
 * settings = today's behaviour (append-only, as ui_prefs): bit 0 DISPLAY > SCOPE MIX (Discussion #165, fx.c scope_mix),
 * bit 1 CONTROL > STEP PREVIEW ON (Discussion #169, ui_input.c step_preview), bit 2 CONTROL > CHORD ENTRY ADD (#155,
 * ui_input.c seq_entry). 1.5: bit 3 SYSTEM > HELP ON (Discussion #156, ui_help.c). 1.5.5: bit 4 CONTROL > SELECT PAGE
 * (Discussion #188, ui_input.c: SELECT steps the pages; 0 = BPM, as before); bit 5 MIDI > SCREEN WAKE PANEL (scr_input:
 * only the panel wakes SCREEN OFF, as up to 1.5.1; 0, every older setting, = +MIDI, the default: a note-on too).
 * 1.5.6: bit 6 SELECT's second bit (#84; retired in P3-005). P4-018: bit 7 FIRST RUN DONE (the sequencer's first-run strip ended or was skipped; 0 = not yet, so every older
 * setting runs it once: ui_sq_glue.c) */
#define ui_prefs2 (favorites.factory[15][24])
#define PREF_X 0xFF0000u                       /* (menu_items.c MENU_FLAGS: a bit of ui_prefs2, << 16) */
#define PREF_SCOPE_MIX (0x01u << 16)
#define PREF_PREVIEW (0x02u << 16)
#define PREF_HELP (0x08u << 16)
#define PREF_WAKE_PANEL (0x20u << 16)
#define PREF_FIRST_RUN_DONE (0x80u << 16)      /* P4-018: not a MENU row (menu_items.c MENU_FLAGS never names it) */
/* 1.3 (Discussions #112, #134): MENU > DISPLAY > HOME, what the HOME page shows under its four cards: 0 SCOPE (the
 * oscilloscope, as before), 1 TRACKS (the four tracks as rows: ui_graph.c draw_home_tracks), 2 LEVELS (1.5, #134: the
 * rows of TRACKS, and KNOB 1..4 the LEVEL of T1..T4 in place of the engine's four: home_param). A byte of its own no
 * engine uses, saved with the settings; 0 in older settings = SCOPE; an unknown value (a later firmware's view) reads as
 * SCOPE and is kept as saved (1.3 / 1.4 read LEVELS so) */
#define ui_home_view (favorites.factory[15][26])
/* 1.5.6 (#212): MENU > DISPLAY > LEDS INV LO / INV HI: INV with the idle LEDs at the steady mid level (ui_input.c
 * ui_leds) instead of lit. The mode stays INV (settings_leds, panel.c: so firmware before 1.5.6 reads plain INV) and
 * this byte no engine uses holds the level: 1 LO, 2 HI; 0 (every older setting) or another value = INV as before.
 * Saved with the settings; read only while LEDS is INV (menu_items.c leds_row) */
#define ui_leds_inv (favorites.factory[15][25])
static uint32_t leds_inv(void) { return settings_leds == LEDS_INV && ui_leds_inv - 1u < 2u ? ui_leds_inv : 0u; }
enum { HV_SCOPE, HV_TRACKS, HV_LEVELS, HV_COUNT };
enum { RP_CLICK, RP_LEVEL, RP_COUNTIN };
static uint32_t rp_get(uint32_t f)                     /* a field as the MENU steps it: 0..2 */
{
    uint32_t v = (ui_rec_prefs >> (2u * f)) & 3u;
    v = v == 3u ? 0u : v;
    return f == RP_LEVEL ? (v == 0u ? 1u : v == 1u ? 0u : 2u) : v;   /* (LEVEL: LOW MID HIGH, MID stored as 0) */
}
static void rp_apply(void)                             /* the settings into click.c / seq.c (every frame: ui_input) */
{
    click_mode = (uint8_t)rp_get(RP_CLICK);
    click_lvl = (uint8_t)rp_get(RP_LEVEL);
    cin_bars = (uint8_t)rp_get(RP_COUNTIN);
}
static void rp_put(uint32_t f, uint32_t v)
{
    uint32_t st = f == RP_LEVEL ? (v == 1u ? 0u : v == 0u ? 1u : 2u) : v % 3u;
    ui_rec_prefs = (uint8_t)((ui_rec_prefs & ~(3u << (2u * f))) | st << (2u * f));
    rp_apply();
}
/* MENU > STYLE (ui_menu.c): ST_FLAT ST_LINE (gfx.c) in another byte no engine uses, saved with the settings;
 * 0 in older ones = FLAT; 2, the retired PIXEL (1.0.1), reads as LINE; anything else unknown as FLAT (settings_persist.c). gfx.c draws from its copy, ux.style
 * (ui_draw.c style_apply) */
#define ui_style (favorites.factory[15][29])

static uint8_t sync_reload;                  /* engine / preset / project / user preset loaded: editor RELOAD push */

#define ACC T_THEME                /* values, curves */
#define VAL(c) ((c) == ui.hot_col && ui.hot_t ? T_ACCENT : T_THEME)   /* the knob just turned: accent */
#define RATIO(d, v) ((d)->max > (d)->min ? ((int32_t)(v) - (d)->min) * 1000 / ((d)->max - (d)->min) : -1)
/* layout: header 0..24, four cards 28..72 (57 px at x 3 + 59 c),
 * the panel 76..198 (a SURF area; the graphs live in it), footer 202..240; BG between them */
#define Y_HEAD 0
#define H_HEAD 24
/* the sequencer's screen and controls (ui_sq.c, ui_sq_glue.c; included after ui_input.c) */
static void ui_sq_poll(void);
static void ui_sq_controls(uint32_t pressed, uint32_t notes);
static void ui_sq_screen(void);
static uint8_t ui_sq_help_used(void);        /* HOME is down and a control was touched (HELP): its hold is no MENU (P4-018) */
#define Y_LABEL 28                    /* the cards */
#define Y_SEP_END 72
#define CARD_W 57
#define CARD_H 44
#define CARD_X(c) (3 + 59 * (int32_t)(c))
#define Y_GRAPH 76                    /* the panel */
#define H_GRAPH 122
#define Y_FOOT 202
#define H_FOOT 38
/* MENU > LARGE (large_kind below): on the value pages the cards are tall (28..132) and the panel a strip
 * (136..198); elsewhere (lists, rolls, layers) the layout above, its card labels in M */
#define LG_CARD_H 104
#define LG_Y_GRAPH 136
#define LG_H_GRAPH 62

static struct {
    uint8_t home;
    uint8_t page;                /* (main.c debug record only) */
    uint8_t ppick;               /* the pattern picked */
        uint8_t hot_col, hot_t;      /* column whose knob was just turned (drawn white) */
    uint8_t menu;                /* 0 off, 1 list, 2 about + credits, 3 info (1.2) (HOME held) */
    uint8_t menu_sel;            /* MENU: the row (menu_items.c MI_*; its tab MI_TAB), kept while the device runs */
    uint8_t menu_row[5];         /* MENU: the row last picked in each tab, from its first (ALGORITHM comes back to it) */
    uint16_t menu_scroll;        /* continuous ABOUT + CREDITS position, pixels */
    uint32_t menu_sig, home_t0;  /* HOME press time (btn_hold) */
    uint8_t force;               /* full redraw pending */
    uint8_t msg_t;               /* transient message frames */
    uint8_t help_t;              /* 1.5, MENU > HELP ON: frames the page's hint stays in the footer (ui_help.c) */
    uint8_t bpm_t;               /* frames the BPM stays highlighted after a SELECT turn */
    uint8_t confirm;             /* the OCT- / OCT+ dialog: CF_*, 0 = none */
    uint8_t confirm_trk;         /* the track it clears, the slot it overwrites */
    uint8_t uboot;               /* main.c: seconds left before UPDATE MODE (OCT- + OCT+ held), 0 = none */
    char msg[24];
    char msg2[24];               /* a second message, shown when the first is over */
    uint32_t enc_t[NE];
    uint32_t head_sig, foot_sig, frame;   /* drawn-state caches, the frame counter */
    /* rolling digits (ui_draw.c roll_*): the four card values and the header BPM */
    struct {
        char from[7];            /* the value rolling out; "" = idle */
        uint8_t t0;              /* (ui.frame) of the value's last change */
        int8_t dir;              /* +1: it went up (the old digit leaves upward), -1: down */
        uint8_t sig;             /* a card: what its value is of (label, unit, track, engine, palette) */
    } roll[5];
    int16_t roll_bpm;            /* the BPM last drawn */
    struct {                     /* MIDI LEARN (midi_learn.c; here, not loose globals: the audio ISR's data keeps its
                                  * layout and its cost, tests/target_budget.py) */
        uint8_t on;              /* MIDI LEARN is on */
        uint8_t pick, trk, id;   /* a parameter picked (1): its track and P_* id */
        volatile uint8_t io[2];  /* midi_control.c ml_arm, ml_heard */
    } ml;
    volatile uint8_t mpc[NTRK];  /* MIDI Program Change per track, 0x80 | program, 0 none (midi_control.c mpc_io: here
                                  * for the same reason as ml.io; midi_pc_poll) */
} ui;

enum { CF_NONE, CF_CLEAR_SEQ, CF_CLEAR_TRK, CF_OVR_PROJ, CF_OVR_USER, CF_LOAD_PAT,
       CF_DEL_ROW, CF_CLEAR_SONG, CF_INIT_SOUND, CF_CLEAR_MOTION, CF_ERASE_USER, CF_ERASE_PROJ, CF_CLEAN_SOUND };
                                  /* ui.confirm: TOOLS' clears; SAVE over a used slot; a pattern over the user's steps;
                                   * USER ERASE; PROJECT ERASE (1.5.5, #199); CLEAN INIT (1.5.5, #190) */

/* MENU > DISPLAY > SCREEN OFF (1.1.5): after a while with no input on the panel the screen goes dark (the backlight
 * stays on: lcd.c lcd_power)
 * (a picture held for hours can stay faintly on the panel); the sound, the sequencer, MIDI and USB go on, the LEDs
 * stay as they are. NEVER (the default since 1.1.5.1), 5, 15, 30, 60 minutes, kept in a byte no engine uses, saved
 * with the settings (SCR_CODE). 1.1.5 stored value ^ 3, 0 (its default and every older setting) being 30 MIN; 1.1.5.1
 * reads that 0 as NEVER (the new default: the panel went dark for good on 1.1.5, see lcd.c lcd_power) and keeps the
 * other codes, 30 MIN now 4. Unknown codes read as NEVER.
 * Input is the panel's: a button or key held or pressed, a knob turned (not the editor or MASTER, an analogue pot);
 * 1.5.5: with MENU > MIDI > SCREEN WAKE +MIDI (the default; PANEL: the panel only, as before) a MIDI IN note-on too,
 * USB or TRS, on any channel that plays a track (seq.c midi_seen: the ISR only sets its bit), not clock, CC or SysEx.
 * The press, key or turn that wakes it is swallowed: no note (seq.c kb_asleep), no button, no value, until everything
 * is let go and the knobs rest for SCR_EAT_MS; a MIDI note that wakes it is not: it plays, as every MIDI note does.
 * A dialog, the count-in and the UPDATE MODE countdown keep it on (and wake it); the crash screen, UPDATE and UBOOT wake it at once (lcd_wake_now).
 * Off: lcd_power(0) (DISPOFF, SLPIN) and nothing is drawn. Waking, a frame at a time (never a wait in
 * the loop): 120 ms after the SLPIN, SLPOUT (lcd_power(1)); 120 ms after that one frame redraws everything
 * (ui.force), then DISPON (lcd_power(2)) */
#define ui_scr (favorites.factory[15][27])
/* MENU > ANIM IDLE (1.2, Discussion #135; menu_items.c, ui_input.c idle_leds): bit 0, the LEDs' idle animation, in a
 * byte no engine uses, kept with the settings; 0 in every older setting = off. The other bits: 0 (room for a delay) */
#define ui_idle (favorites.factory[15][10])
#define SCR_DEF 0u                                     /* NEVER (1.1.5.1; 1.1.5: 30 MIN) */
static const uint8_t SCR_CODE[5] = {0, 2, 1, 4, 7};    /* the stored byte of NEVER 5 15 30 60 MIN (1.1.5: 3 2 1 0 7) */
static const uint8_t SCR_DEC[8] = {0, 2, 1, 0, 3, 0, 0, 4};   /* .. and back (3: 1.1.5's NEVER; 5, 6 unknown) */
#define SCR_EAT_MS 300u
static const uint8_t SCR_MIN[5] = {0, 5, 15, 30, 60};  /* NEVER, minutes */
enum { SCR_ON, SCR_OFF, SCR_WAKE, SCR_SLPOUT, SCR_SHOW };
static struct {
    uint8_t st;                  /* SCR_*: SCR_WAKE asked, SCR_SLPOUT sent, SCR_SHOW the frame that redraws */
    uint8_t eat;                 /* the wake gesture: panel input swallowed (ui_input) */
    uint32_t idle;               /* fm1_ms of the last panel input (or blocker) */
    uint32_t t;                  /* fm1_ms of the last panel command (SLPIN, SLPOUT) */
    uint32_t eat_t;              /* fm1_ms of the last swallowed knob detent */
} scrn;
static uint32_t scr_get(void)                          /* the MENU's value: 0 NEVER .. 4 60 MIN */
{
    return ui_scr < NELEM(SCR_DEC) ? SCR_DEC[ui_scr] : SCR_DEF;
}
static void scr_put(uint32_t v)                        /* (the time counts from the change: the editor's too) */
{
    ui_scr = SCR_CODE[v < NELEM(SCR_MIN) ? v : SCR_DEF];
    scrn.idle = fm1_ms;
}
static void scr_wake(void)
{
    if (scrn.st == SCR_OFF)
        scrn.st = SCR_WAKE;
    scrn.eat = 1;
}
static void scr_wake_now(void)                         /* main.c: the crash screen, UPDATE, UBOOT */
{
    if (scrn.st != SCR_ON)
        lcd_wake_now();
    scrn.st = SCR_ON;
    scrn.idle = fm1_ms;
}
static uint8_t mf_seen;          /* 1.5.5 (#208): the MIDI IN notes of this pass (seq.c midi_seen, taken at every pass's
                                  * start: ui_input.c midi_take), bit per track; AUTO SELECT's (midi_follow) */
/* ui_input, every pass: 1 = the pass is swallowed (the screen dark or waking, or the wake gesture going on) */
static int scr_input(uint32_t pressed, uint32_t notes)
{
    uint32_t held = fm1_in.buttons | fm1_in.notes, k;
    int32_t moved = 0;
    if (pressed | notes | held | panel_moved)
        scrn.idle = fm1_ms;
    panel_moved = 0;
    if (mf_seen && !(ui_prefs2 & (PREF_WAKE_PANEL >> 16))) {     /* +MIDI: a note-on since the last pass keeps it on,
                                                                 * wakes it; nothing swallowed (the note has played) */
        scrn.idle = fm1_ms;
        if (scrn.st == SCR_OFF)
            scrn.st = SCR_WAKE;
    }
    if (scrn.st == SCR_ON && !scrn.eat)
        return 0;
    for (k = 0; k < NE; k++)                           /* (every knob's turns go) */
        moved |= panel_enc(k);
    panel_moved = 0;
    if (moved)
        scrn.eat_t = scrn.idle = fm1_ms;
    if (pressed | notes | held | (uint32_t)moved)
        scr_wake();
    if (scrn.st != SCR_ON || held || (int32_t)(fm1_ms - scrn.eat_t) < (int32_t)SCR_EAT_MS)
        return 1;
    scrn.eat = 0;                                       /* all let go, the knobs at rest: input again */
    kb_asleep = 0;
    return 0;
}
/* ui_draw, every frame: 1 = draw it */
static int scr_frame(void)
{
    uint32_t lim = SCR_MIN[scr_get()] * 60000u;
    if (ui.confirm || ui.uboot || seq_counting()) {    /* (never dark under these) */
        scrn.idle = fm1_ms;
        if (scrn.st == SCR_OFF)
            scr_wake();
    }
    switch (scrn.st) {
    case SCR_ON:
        if (!lim || (int32_t)(fm1_ms - scrn.idle) < (int32_t)lim)   /* (signed: robust to a wrap) */
            return 1;
        lcd_power(0);
        scrn.st = SCR_OFF;
        scrn.t = fm1_ms;
        scrn.eat = 1;
        kb_asleep = 1;
        break;
    case SCR_WAKE:
        if (fm1_ms - scrn.t >= 120u) {
            lcd_power(1);
            scrn.st = SCR_SLPOUT;
            scrn.t = fm1_ms;
        }
        break;
    case SCR_SLPOUT:
        if (fm1_ms - scrn.t >= 120u) {
            scrn.st = SCR_SHOW;
            ui.force = 1;
            return 1;
        }
        break;
    case SCR_SHOW:
        return 1;
    }
    ui.msg_t = 0;                                      /* (dark: what would have shown is over; a clean page back) */
    ui.msg2[0] = 0;
    ui.hot_t = ui.bpm_t = 0;
    return 0;
}
static void scr_shown(void)                            /* after the frame: the redrawn page on */
{
    if (scrn.st != SCR_SHOW)
        return;
    lcd_power(2);
    scrn.st = SCR_ON;
    scrn.idle = fm1_ms;
}

/* MENU > LARGE (#15, Discussion #80: what KNOB 1..4 do, in bigger type). Per page type:
 *   LK_TALL  HOME and the value pages (EDIT ENV LFO MOD FX SLICER SCL CHORD ARP VOICE, MIXER, CLOCK,
 *            TOOLS): tall cards (a K1..K4 keycap, the icon, the label in M, the value in L,
 *            in M or S when L is too wide or lacks a glyph, the unit under it, the gauge), the panel a strip: HOME's
 *            scope, ENV's ADSR, LFO's wave, MIXER's four tracks (name, state, meter) small; the
 *            page's title and number in L on the others (their charts cannot be read that small);
 *   LK_LABEL the list and graph pages (PRESETS, USER, PROJECT, PHRASES, SONG, the piano roll and the drum grid,
 *            AUTOMATION, SLICES), HOME with MENU > HOME TRACKS (1.3) and the quick layers' maps: the layout as it
 *            is, the card labels in M;
 *   LK_OFF   LARGE off; the menu (its own LARGE: ui_menu.c), the dialogs and NAME keep their own layout in every case. */
enum { LK_OFF, LK_LABEL, LK_TALL };

/* transient message in the top bar, right of the transport and the BPM: a + b */
static void ui_say(const char *a, const char *b)
{
    uint32_t n;
    str_cpy(ui.msg, a, sizeof ui.msg);
    n = str_len(ui.msg);
    str_cpy(ui.msg + n, b, sizeof ui.msg - n);
    ui.msg_t = 40;
    ui.msg2[0] = 0;
}

static void ui_message(const char *s) { ui_say(s, ""); }
/* a message led by an icon: its first byte (ui_draw.c draw_head draws the icon, then the words) */
#define MSG_NOFILE "\x01"                      /* the no-file icon (ICON_X_NOFILE) */
/* a missing sample (ui_input.c sample_notice): NO SAMPLE (SAMPLE NOT FOUND, the SLICES page's, is 4 px too wide
 * for the header's 130 px after the icon) */
#define MSG_NO_SAMPLE MSG_NOFILE "NO SAMPLE"

/* Main loop only, with interrupts enabled. PLAY may be consumed between reads;
 * take one coherent snapshot before a flash operation or song preparation. */
static int transport_busy(void)
{
    int busy;
    fm1_irq_off();
    busy = song.playing || transport_req == 1u || seq_counting();   /* (a count-in: as playing) */
    fm1_irq_on();
    return busy;
}

/* 1.5 (Discussion #156): MENU > HELP ON: a page entered (here, go_home) shows its hint in the footer for ~2 s
 * (ui_help.c, ui_draw.c draw_foot) */
#define HELP_FRAMES 120u                       /* (~2 s at the UI's ~60 frames a second) */
static int help_on(void) { return (ui_prefs2 & (PREF_HELP >> 16)) != 0u; }
/* 1.5.5 (#208): a MIDI IN note into track k lights its number cushion for MLIT_MS (the header's for the selected track,
 * HOME TRACKS / LEVELS, the MIXER); midi_lit_ms[k]: fm1_ms | 1 of the last (ui_input.c midi_take), 0 none */
#define MLIT_MS 90u
static uint32_t midi_lit_ms[NTRK];
static int midi_lit(uint32_t k) { return midi_lit_ms[k % NTRK] && (fm1_ms | 1u) - midi_lit_ms[k % NTRK] < MLIT_MS; }
static void help_start(void) { ui.help_t = (uint8_t)(help_on() ? HELP_FRAMES : 0u); }

static void step_clear(step_t *st)
{
    st->n = 0;
    st->time = ST_REST;
    st->flags = 0;
    st->vel = 0;
    st->hit = st->acc = 0;
    st->probability = 0;
}

/* ------------------------------------------------------- the DRUM grid --- */

/* COLOR's value v (the menu, the editor): a built-in palette, or CUSTOM when one is stored */
static void palette_choose(uint32_t v)
{
    if (v >= palette_n())                              /* (CUSTOM with none stored: nothing changes) */
        return;
    if (v >= NPALETTES) {
        ui_cpal_flags |= CPAL_ACTIVE;
    } else {
        ui_cpal_flags &= (uint8_t)~CPAL_ACTIVE;
        settings.palette = v;
    }
    palette_apply();
    ui.force = 1;
}

/* SEQ > STEP / DETAIL follows the play position (1.6, #242, Discussion #238): a melodic pattern longer than the 16
 * steps on the screen shows the bank the playhead is in while the transport plays (ui.bank only: the cursor, the
 * pattern and the saved project are untouched; DRUM's grid keeps its bank, its keys edit what the grid shows). The
 * roll is left alone ("held") from the moment a knob moves the cursor or edits the cursor's step, or step entry has the
 * keys: the bank goes back to the cursor's, so what is being edited stays on the screen. Following starts again after
 * FOLLOW_MS without that, or once the playhead has gone one pass round the pattern, whichever is first. The transport
 * stopped, another track, or the roll left: nothing held any more */
#define FOLLOW_MS 3000u
static struct {
    uint32_t t;                                        /* fm1_ms of the last touch of the cursor's step */
    uint16_t prev, steps;                              /* the playhead at the last poll, the steps it has gone since */
    uint8_t hold;
} follow;

static void go_home(void)
{
    ui.home = 1;
    ui.hot_t = 0;
    song.seq_mode = 0;
    ui.force = 1;
    help_start();
}

/* One-step UNDO of a load. A sound load (a factory or user preset, an engine jump, TOOLS INIT, the
 * editor's PRESET / G_ENGSEL / UP_LOAD) changes the sound only; a pattern load (SAVE > PHRASES) changes
 * the steps and the pattern parameters only. Each first copies the track as it was; SAVE held 0.7 s
 * swaps back what the loads changed (held again: the loads again), so steps recorded after a sound
 * load, or a sound edited after a pattern load, stay as they are. One copy for all tracks: the last
 * load wins. Loads in a row on one track with nothing changed in between (the PRESETS knob through the
 * list, one pattern after the other, an editor audition) keep the copy from before the first, so the
 * undo goes back past the whole browse. Not snapshotted: power-on, projects.
 */
enum { UNDO_SOUND = 1, UNDO_PAT = 2 };
static struct {
    uint8_t trk;                 /* track + 1, 0 = nothing to undo */
    uint8_t keep;                /* the track is as the last load left it (undo.after): a next load keeps the copy */
    uint8_t what;                /* UNDO_SOUND | UNDO_PAT: what the loads since the copy changed (undo_swap) */
    uint8_t eng, preset, user, patn;
    int16_t p[P_COUNT];
    step_t step[NSTEP];
    uint32_t after;              /* track_sig right after the last load */
    uint32_t pat;                /* pat_sig[] of the copy */
    uint32_t t_ms;               /* time of the last load (the editor's SETs after it belong to it) */
} undo __attribute__((section(".pool")));   /* (main loop only: off the audio code's .bss) */
static uint8_t undo_depth;       /* loads nest (an engine jump loads its first preset): the outer one counts;
                                  * felucca_init and the autosave restore raise it to take no copy at all */
static uint32_t pat_sig[NTRK];   /* steps_sig of the pattern the last pattern load put into each track: such
                                  * steps, untouched, are replaced by the next pattern without asking */
static uint8_t pat_last[NTRK];   /* that pattern's list index + 1, 0 = none */
static uint32_t fnv(uint32_t h, const void *p, uint32_t n)
{
    const uint8_t *b = (const uint8_t *)p;
    while (n--)
        h = (h ^ *b++) * 16777619u;
    return h;
}
static uint32_t steps_sig(const track_t *t) { return fnv(2166136261u, t->step, sizeof t->step); }
static uint32_t track_sig(const track_t *t)      /* the sound, the steps */
{
    uint8_t id[3] = {t->eng_req, t->preset, t->user};
    uint32_t h = fnv(fnv(steps_sig(t), t->p, sizeof t->p), id, 3), k = trk_index(t);
    return h;
}
/* MOMENTARY (ui_input.c mom_turn: LFO held, the knobs' changes go back when it is let go): the values turned, where
 * they are (a track's p[], song.g[]) and what they were. In the main loop's pool (no .bss) */
#define MOM_N 8u
static struct {
    uint8_t n;                                    /* values held, 0 = none */
    int16_t *vp[MOM_N];
    int16_t v[MOM_N];
} mom __attribute__((section(".pool")));
static void mom_back(const track_t *t)            /* a sound load into t (0: a project): its values back first */
{
    uint32_t i, k = 0;
    for (i = 0; i < mom.n; i++)
        if (!t || (mom.vp[i] >= t->p && mom.vp[i] < t->p + P_COUNT) || (mom.vp[i] >= song.g && mom.vp[i] < song.g + G_COUNT)) {
            *mom.vp[i] = mom.v[i];                /* (a global: back too, before the load's undo copy) */
        } else {
            mom.vp[k] = mom.vp[i];
            mom.v[k++] = mom.v[i];
        }
    mom.n = (uint8_t)k;
}
static void load_begin(track_t *t, uint32_t what)
{
    uint32_t i = trk_index(t);
    if (what & UNDO_SOUND) {
        mom_back(t);                              /* (MOMENTARY: the sound as it was, then the load) */
    }
    if (undo_depth++)
        return;
    if (undo.keep && undo.trk == i + 1u && track_sig(t) == undo.after) {
        undo.what |= (uint8_t)what;               /* browsing on: the copy from before the first load stays */
        return;
    }
    undo.trk = (uint8_t)(i + 1u);
    undo.what = (uint8_t)what;
    undo.eng = t->eng_req;
    undo.preset = t->preset;
    undo.user = t->user;
    memcpy(undo.p, t->p, sizeof undo.p);
    memcpy(undo.step, t->step, sizeof undo.step);
    undo.pat = pat_sig[i];
    undo.patn = pat_last[i];
}

static void load_end(track_t *t)
{
    if (--undo_depth)
        return;
    undo.after = track_sig(t);
    undo.keep = 1;
    undo.t_ms = fm1_ms;
}

static int param_kept(uint32_t i);

/* a 16-step pattern (PATTERNS[] format, user presets too) into steps 1..16, the rest empty, LEN 16 */
static void load_pat16(track_t *t, const uint8_t *note, const uint8_t *flags)
{
    uint32_t i;
    for (i = 0; i < NSTEP; i++) {
        step_t *s = &t->step[i];
        uint8_t n = i < 16u ? note[i] : 0, fl = i < 16u ? flags[i] : 0;
        s->note[0] = n;
        s->n = n ? 1 : 0;
        s->time = (fl & 4u) ? ST_TIE : n ? ST_NOTE : ST_REST;
        s->flags = n ? (fl & (SF_ACCENT | SF_SLIDE | SF_RATCH)) : 0;
        s->vel = n ? 96 : 0;
        s->hit = s->acc = 0;
        s->probability = 0;
    }
    t->p[P_SLEN] = 16;
}

/* a 16-step drum grid (user presets of version 3: lane hits, their accents) into steps 1..16, the rest empty,
 * LEN 16 */
static void load_grid16(track_t *t, const uint8_t *hit, const uint8_t *acc)
{
    uint32_t i;
    for (i = 0; i < NSTEP; i++) {
        step_t *s = &t->step[i];
        step_clear(s);
        if (i < 16u && hit[i]) {
            s->time = ST_NOTE;
            s->hit = hit[i];
            s->acc = acc[i] & hit[i];
        }
    }
    t->p[P_SLEN] = 16;
}

static void track_defaults_steps(track_t *t)
{
    uint32_t i;
    for (i = 0; i < NSTEP; i++)
        step_clear(&t->step[i]);
}

/* the factory patterns (PATTERNS[], "01".."13") */
static uint32_t pat_count(void) { return NPATTERNS; }

static void pat_label(uint32_t n, char *tag, char *name)   /* tag: 4 bytes ("01"), name: 13 */
{
    tag[0] = (char)('0' + (n + 1u) / 10u);
    tag[1] = (char)('0' + (n + 1u) % 10u);
    tag[2] = 0;
    str_cpy(name, PATTERNS[n % NPATTERNS].name, 13);
}

static uint32_t pat_pick(void)                   /* ui.ppick inside the list */
{
    uint32_t n = pat_count();
    return ui.ppick < n ? ui.ppick : n - 1u;
}

/* the track's settings, not the sound's: what a sound load (factory or user preset, an engine jump,
 * TOOLS INIT) leaves alone. The mix (LEVEL, PAN, MUTE: the TRACKS faders), the arpeggiator (ARP, ARP 2),
 * the scale and key map (SCL), the pattern parameters (LEN, DIV, SWING, GATE; 1.2: QUANTIZE) and the SLICER
 * insert, which chops whatever the track plays in time with its sequencer */
static int param_kept(uint32_t i)
{
    return i == P_LEVEL || i == P_PAN || i == P_MUTE || (i >= P_AMODE && i <= P_SGATE) ||
           (i >= P_SLCR && i <= P_SLDEPTH) || i == P_CHRD || i == P_VOIC || i == P_SQNT;
}

#define preset_shown(e) (ENGINES[e]->npresets)

static void set_engine_of(track_t *t, uint32_t ei);
/* preset pi of the engine the track asked for: the sound only (not the steps, not param_kept) */
static void apply_preset_to(track_t *t, uint32_t pi)
{
    const engine_t *e = ENGINES[t->eng_req % NENGINES];
    uint32_t i;
    load_begin(t, UNDO_SOUND);
    panic_req |= (uint8_t)(1u << trk_index(t));       /* MONO/POLY may change: release what sounds */
    t->user = 0;
    if (t == TSEL)
        sync_reload = 1;
    if (!e->npresets) {
        load_end(t);
        return;
    }
    pi = pi % e->npresets;
    t->preset = (uint8_t)pi;
    for (i = 0; i < P_E0; i++)                        /* the rest of the sound to its defaults: a preset */
        if (!param_kept(i))                           /* sounds the same after any edit */
            t->p[i] = TP[i].def;
    for (i = 0; i < 8u; i++)
        t->p[P_E0 + i] = (int16_t)e->presets[pi].e[i];
    t->p[P_ATK] = e->presets[pi].env[0];
    t->p[P_DEC] = e->presets[pi].env[1];
    t->p[P_SUS] = e->presets[pi].env[2];
    t->p[P_REL] = e->presets[pi].env[3];
    t->p[P_ED_FLT] = e->presets[pi].fenv;
    t->p[P_VOICE] = e->presets[pi].mono ? V_LEGATO : V_POLY;   /* mono presets keep the legato feel */
    {   /* the sends */
        static const uint8_t FX_DEF[4] = {0, 24, 28, 36};
        const preset_t *pr = &e->presets[pi];
        for (i = 0; i < 4u; i++)
            t->p[P_DIST + i] = (int16_t)(pr->fx[i] ? pr->fx[i] - 1 : FX_DEF[i]);
    }
    load_end(t);
}

/* the engine's defaults and its first preset. With the audio IRQ off: the ISR sees the old engine with
 * its values or the new one with its own (voice.c engine_block), never one with the other's */
static void set_engine_of(track_t *t, uint32_t ei)
{
    const engine_t *e = ENGINES[ei % NENGINES];
    uint32_t i;
    load_begin(t, UNDO_SOUND);
    fm1_irq_off();
    t->eng_req = (uint8_t)(ei % NENGINES);
    for (i = 0; i < 8u; i++)
        t->p[P_E0 + i] = e->edit[i].def;
    apply_preset_to(t, 0);
    fm1_irq_on();
    load_end(t);
}

/* MIDI Program Change (1.5.1, #179 by renebohne): program n on track t, as the PRESETS list numbers the engine's sounds
 * from 0: its n-th preset, loaded as from there (the track's own settings kept, one undo for a run of them). A program
 * past the last is ignored, not wrapped: a number is always the same sound */
static volatile uint8_t *const mpc_io = ui.mpc;
static void midi_pc_load(track_t *t, uint32_t n)
{
    uint32_t e = t->eng_req % NENGINES;
    if (n >= preset_shown(e))
        return;
    apply_preset_to(t, n);
}
static void midi_pc_poll(void)                       /* main loop (ui_input) */
{
    uint32_t k, v;
    for (k = 0; k < NTRK; k++) {
        if (!ui.mpc[k])
            continue;
        fm1_irq_off();                               /* (the ISR may note another meanwhile) */
        v = ui.mpc[k];
        ui.mpc[k] = 0;
        fm1_irq_on();
        midi_pc_load(&trk[k], v & 127u);
        ui.force = 1;
    }
}

static void track_defaults(track_t *t)
{
    uint32_t i;
    for (i = 0; i < P_E0; i++)
        t->p[i] = TP[i].def;
    track_defaults_steps(t);
}

/* #197: a settings change made by a knob (PRESETS > LIST, the FAV mark on PRESETS and in the EDIT layer) is written
 * later, as MENU's are when it closes: a write erases a flash sector with the interrupts off (~40 ms), which stops the
 * LED scan (every LED but one column's goes dark: a flash of the whole panel) and the audio. Written once the view it
 * was made in is left (another page, HOME, a layer opened or let go, the MENU) or after SET_LATER_MS without panel
 * input (ui.c scrn.idle; the autosave's wait), never on every detent; settings_poll still waits for the transport */
#define SET_LATER_MS 10000u

#include "midi_learn.c"                           /* MIDI LEARN (1.5): GLO + D4, a knob, a CC */

/* ---------------------------------------------------- action pages --- */
