/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Felucca UI input: LEDs, knobs and buttons, panel setup. P3-006 (R3b): the groovebox pages' input is gone; what is left is
 * the button hold timers, the MENU, the T-1 screen's early return (ui_t1.c) and the LEDs. */
/* ----------------------------------------------------------- LEDs --- */
/* The LED picture is built off-line and copied one byte per column: clearing
 * and relighting would let the 10 kHz scan catch the dark gap and flicker. */
static uint8_t led_pos[41];                        /* (col << 3) | row bit, 0xFF = none */

static void led_pos_init(void)
{
    uint32_t id, p, r;
    for (id = 0; id < 41u; id++) {
        led_pos[id] = 0xFF;
        for (p = 0; p < FM1_NCOL; p++)
            for (r = 1; r < 5u; r++)
                if (FM1_KEYMAP[r][p] == (int8_t)id)
                    led_pos[id] = (uint8_t)((p << 3) | r);
    }
}

static void led_put(uint8_t *nl, uint32_t id, int on)
{
    uint8_t q = led_pos[id];
    if (q != 0xFF && on)
        nl[q >> 3] |= (uint8_t)(1u << (q & 7u));
}

/* PLAY's second, green LED: not in the key matrix (no key there), found on the hardware at column 8, row PA9 (bit 1) */
#define LED_PLAY_GREEN ((8u << 3) | 1u)
static void led_clear(uint8_t *nl, uint32_t id)
{
    uint8_t q = led_pos[id];
    if (q != 0xFF)
        nl[q >> 3] &= (uint8_t)~(1u << (q & 7u));
}

/* the OCT LEDs, bit 0 OCT- lit, bit 1 OCT+ lit, OCT_BREATH OCT+ breathing (can be pressed: dark .. ~60 %,
 * hal/fm1_input.h fm1_led_breath, #119). The menu: on a value row OCT- and OCT+ both breathe while that way has a value
 * to go to, on CALIBRATION / ABOUT OCT+ breathing alone (opens), in ABOUT OCT- lit (back); a dialog: OCT- lit, OCT+
 * breathing; elsewhere the octave shift */
#define OCT_BREATH 4u
#define OCT_BREATH_DN 8u                                /* OCT- breathing (the menu: the previous value) */
static uint32_t oct_leds(void)
{
    if (ui.menu && !ui.confirm)
        return ui.menu >= 2u ? 1u :                    /* (INFO, ABOUT: OCT- back, lit) */
               ui.menu_sel == MI_LCLEAR ? (ml_count() ? OCT_BREATH : 0u) :   /* LEARN CLEAR: while CCs are learned */
               ui.menu_sel >= MI_VALUES ? OCT_BREATH :  /* CALIBRATION / INFO / ABOUT: OCT+ opens */
               (menu_step(ui.menu_sel, -1) != menu_get(ui.menu_sel) ? OCT_BREATH_DN : 0u) |   /* a value: each */
               (menu_step(ui.menu_sel, 1) != menu_get(ui.menu_sel) ? OCT_BREATH : 0u);         /* way it can go */
    if (ui.confirm)
        return 1u | OCT_BREATH;
    return (song.octave < 0 ? 1u : 0u) | (song.octave > 0 ? 2u : 0u);
}

/* Discussion #81: the keys of the notes MIDI IN (USB and TRS, routed by ROUT) holds on track t, bit k = key k: as
 * play_leds, where the keys play that note at the octave now, the lowest key that gives it; a note no key plays is
 * not shown. Read from midi_control.c's own state (midi_note_held: the notes each channel holds for the track, the
 * pedal's too): no state here; nothing to do while MIDI holds none of the track's */
static uint32_t midi_leds(const track_t *t)
{
    uint32_t k, note, m = 0, seen[4] = {0, 0, 0, 0};
    if (!midi_owners[trk_index(t)])
        return 0;
    for (k = 0; k < 27u; k++) {
        note = kb_map(t, k);
        if (note > 127u || ((seen[note >> 5] >> (note & 31u)) & 1u))
            continue;                                   /* (silent, or a lower key gives it) */
        seen[note >> 5] |= 1u << (note & 31u);
        m |= (uint32_t)midi_note_held(t, note) << k;
    }
    return m;
}

/* the keys of the notes the selected track's sequencer and ARP sound now (#38), bit k = key k: where the keys
 * play that note (kb_map: the octave, TRN, QNT, an engine's own map), the lowest key that gives it (QNT SNAP
 * rounds the keys above down onto it); a note no key plays is not shown. A snapshot of the ISR's seq_notes /
 * arp_note (arp_ch): no state of its own, nothing to do while nothing sounds */
static uint32_t play_leds(void)
{
    const track_t *t = TSEL;
    uint8_t s[4 + NLANE + 4];
    uint32_t n = t->seq_n < 4u + NLANE ? t->seq_n : 4u + NLANE, i, k, note, used = 0, m = 0, hit;
    for (i = 0; i < n; i++)
        s[i] = t->seq_notes[i];
    if (t->arp_note)
        s[n++] = t->arp_note;
    for (i = 0; i < 3u; i++)                            /* (the ARP's CHORD) */
        if (t->arp_ch[i])
            s[n++] = t->arp_ch[i];
    for (k = 0; n && k < 27u; k++) {
        note = kb_map(t, k);
        for (i = 0, hit = 0; i < n; i++)
            if (s[i] == note && !((used >> i) & 1u)) {
                used |= 1u << i;
                hit = 1;
            }
        m |= hit << k;
    }
    return m | midi_leds(t);
}

/* Discussion #127 (1.2): MENU > SCALE LEDS ON, the keys of the selected track's scale (ROOT, SCALE), bit k = key k, as
 * the keys play now: the note a key gives with TRN (QNT SNAP / SEQ: the key's own note; the others round down onto
 * the scale); QNT WHITE every white key (they walk the scale; the black ones are silent). *root: the keys of the
 * scale's root. None where the engine maps the keys itself (DRUM's GM map, the slices): 0 there, and OFF */
static uint32_t scale_leds(uint32_t *root)
{
    const track_t *t = TSEL;
    const engine_t *e = ENGINES[eng_idx(t->eng_req)];
    uint32_t k, m = 0, r = 0, mask = scale_mask(t), note;
    int32_t pc;
    *root = 0;
    if (!(ui_rec_prefs & (PREF_SCALE_LEDS >> 8)))
        return 0;
    for (k = 0; k < 27u; k++) {
        if (e->keys && e->keys(t, k) >= 0)
            continue;
        if (t->p[P_QUANT] == QN_WHITE) {
            if ((note = kb_map(t, k)) > 127u)
                continue;                               /* (a black key: silent) */
            pc = (int32_t)note - t->p[P_TRANS];         /* (WHITE walks the scale before TRN) */
        } else {
            pc = 53 + (int32_t)k + t->p[P_TRANS];
        }
        pc = ((pc - t->p[P_ROOT]) % 12 + 12) % 12;
        m |= ((mask >> pc) & 1u) << k;
        r |= (uint32_t)(pc == 0) << k;
    }
    *root = r & m;
    return m;
}

#define IDLE_LEDS_MS 60000u
static int idle_leds(void)
{
    int due = (ui_idle & 1u) && !(ui_prefs & PREF_ANIM_OFF) && !song.playing && !seq_counting() &&
              !song.rec && !transport_req && !ui.menu && !ui.confirm && !ui.uboot &&
              !(fm1_in.notes | fm1_in.buttons) && !panel_moved && !play_leds() && fm1_ms - scrn.idle >= IDLE_LEDS_MS;
    if (!due) {
        if (fm1_led_idle_on())
            fm1_led_anim_stop();
        return 0;
    }
    if (!fm1_led_anim_on())                            /* (not over the power-on sweep) */
        fm1_led_idle_start(settings_leds == LEDS_DIM || settings_leds == LEDS_DIM_LO);
    return fm1_led_idle_on();
}

#define ACC_GAP 40u
#define ACC_RATE 50u
static int32_t accel(uint32_t role, int32_t s, int32_t range)
{
    uint32_t now = fm1_ms & 0xFFFFFFu, st = ui.enc_t[role], up = s > 0, pi = st >> 25, a, i, m = 1;
    if (!(ui_prefs & PREF_ACCEL) || range <= 32 || !s)
        return s;
    a = (uint32_t)(s < 0 ? -s : s);
    i = ((now - st) & 0xFFFFFFu) / a;                   /* ms per detent of this read */
    if (!st || ((st >> 24) & 1u) != up || i >= ACC_GAP)
        i = 127u;                                       /* a new turn, or reversed */
    else if (pi < ACC_GAP) {
        m = ACC_RATE / (i > pi ? i : pi ? pi : 1u);
        a = range > 64 ? 8u : 4u;                       /* (the cap) */
        m = m < 1u ? 1u : m > a ? a : m;
    }
    ui.enc_t[role] = now | up << 24 | (i ? i : 1u) << 25;
    return s * (int32_t)m;
}

/* the keys that light: those held and the notes the selected track (the preview synth) and MIDI IN sound; the menu and
 * the T-1 screen give the keys no map of their own */
static uint32_t key_leds(void) { return fm1_in.notes | play_leds(); }

/* The LEDs: lit = active (HOME: the T-1 screen is the page; PLAY / REC running, the keys held or playing), the ones that
 * can be pressed breathe (#119: OCT+ when it would act; dark .. ~60 % of lit and back, hal/fm1_input.h fm1_led_breath,
 * in every mode: never a hard blink in a dark room; DIM LO up to ~30 %, DIM HI OFF INV ~60 %; with LEDS OFF it is the
 * only glow; INV leaves it as it is, between the dark active ones and the lit idle ones), every other button and key
 * glows dim (#35: the buttons of the black FM-1 can be found in the dark; hal/fm1_input.h fm1_led_dim, a short pulse
 * each frame). MENU > LEDS: DIM HI (default) that glow, DIM LO a darker one (fm1_led_dim_level), OFF no glow (as 1.0);
 * INV turns it around, as the stock firmware: the idle ones fully lit, the active ones dark, no glow. 1.2, MENU > SCALE
 * LEDS ON (scale_leds): the keys of the selected track's scale glow and the others are dark, the root's at the mid level,
 * in DIM HI, DIM LO and OFF alike (asked for: OFF only takes the other glow away); INV turns it around as it does the
 * rest: the root's keys lit, the scale's at the mid level, the others glowing; the keys held and playing as ever (lit,
 * INV dark). Each picture is built off-line and copied one byte per column, the glow first: an LED going from lit to
 * dim never has a dark frame */
static void ui_leds(void)
{
    uint8_t nl[FM1_NCOL] = {0}, nd[FM1_NCOL] = {0}, nb[FM1_NCOL] = {0};
    uint8_t nm[FM1_NCOL] = {0}, sg[FM1_NCOL] = {0}, sm[FM1_NCOL] = {0}, iv[FM1_NCOL] = {0};
    uint32_t k, c, sc, sr = 0;
    uint32_t mode = settings_leds, inv = leds_inv();   /* (1.5.6: INV LO 1, INV HI 2) */
    static uint8_t ready;
    if (idle_leds() || fm1_led_anim_on())              /* the idle animation, the power-on sweep have them (main.c */
        return;                                         /* boot_leds) */
    if (!ready) {
        led_pos_init();
        ready = 1;
    }
    led_put(nl, panel.btn[B_HOME], 1);
    led_put(nl, panel.btn[B_REC], song.rec != 0u);
    k = oct_leds();
    led_put(nl, panel.btn[B_OCTDN], (int)(k & 1u));
    led_put(nl, panel.btn[B_OCTUP], (int)((k >> 1) & 1u));
    led_put(nb, panel.btn[B_OCTUP], (int)(k & OCT_BREATH));
    led_put(nb, panel.btn[B_OCTDN], (k & OCT_BREATH_DN) != 0u);
    c = key_leds();
    sc = scale_leds(&sr);                               /* SCALE LEDS: the scale's keys glow, the root's brighter */
    for (k = 0; k < 27u; k++) {
        uint32_t in = (sc >> k) & 1u, rt = (sr >> k) & 1u, idle = !((c >> k) & 1u);
        led_put(nl, 14u + k, (int)((c >> k) & 1u));
        led_put(nd, 14u + k, !sc || (mode == LEDS_INV && rt));
        if (!sc)
            continue;
        led_put(sg, 14u + k, mode == LEDS_INV ? !in && idle : (int)in);   /* (INV: the active ones dark) */
        led_put(sm, 14u + k, mode == LEDS_INV ? in && !rt && idle : (int)rt);
    }
    for (k = 0; k < NB; k++)
        led_put(nd, panel.btn[k], 1);
    if (song.playing || seq_counting())                 /* playing: PLAY's green, its own LED dark in every mode */
        led_clear(nd, panel.btn[B_PLAY]);
    for (c = 0; c < FM1_NCOL; c++) {
        if (mode == LEDS_INV) {                         /* INV: the active ones dark, the rest lit (1.5.6, #212: */
            iv[c] = (uint8_t)(nd[c] & ~nl[c]);          /* INV LO / HI: at the steady mid level, below) */
            nl[c] = inv ? 0u : iv[c];
        }
        if (mode != LEDS_DIM && mode != LEDS_DIM_LO) {  /* OFF, INV: no glow; INV LO / HI: INV's lit ones at the mid level */
            nd[c] = 0u;
            nm[c] = inv ? iv[c] : 0u;
        }
        nd[c] |= sg[c];                                 /* SCALE LEDS: in every mode (INV: inverted above) */
        nm[c] |= sm[c];
        nm[c] &= (uint8_t)~nb[c];
        nl[c] &= (uint8_t)~nb[c];                       /* breathing: neither lit (INV's idle) nor the glow */
        nd[c] &= (uint8_t)~nb[c];
    }
    if (song.playing || (seq_counting() && cin_pos < click_beat_len() / 4u))   /* counting in: green on each beat */
        nl[LED_PLAY_GREEN >> 3] |= (uint8_t)(1u << (LED_PLAY_GREEN & 7u));
    fm1_led_dim_level(mode == LEDS_DIM_LO || inv == 1u);   /* (INV LO: the darker mid level, ~8 %; HI ~12.5 %) */
    for (c = 0; c < FM1_NCOL; c++) {
        fm1_led_dim[c] = nd[c];
        fm1_led_breath[c] = nb[c];
        fm1_led_mid[c] = nm[c];
    }
    for (c = 0; c < FM1_NCOL; c++)
        fm1_led[c] = nl[c];
}

/* ---------------------------------------------------------- input --- */
/* OCT- / OCT+ where they answer (the dialogs, the menu, action pages): on release, and only a press
 * that began there; both down together (UPDATE MODE, main.c) is no tap. Bit 0 OCT-, bit 1 OCT+ */
static uint32_t oct_taps(uint32_t pressed, int here)
{
    static uint8_t down, chord;
    uint32_t dn = panel.btn[B_OCTDN], up = panel.btn[B_OCTUP];
    uint32_t now = ((fm1_in.buttons >> dn) & 1u) | ((fm1_in.buttons >> up) & 1u) << 1, tap;
    if (here)
        down |= (uint8_t)(((pressed >> dn) & 1u) | ((pressed >> up) & 1u) << 1);
    if (now == 3u)
        chord = 1;
    tap = down & ~now;
    down &= (uint8_t)now;
    if (chord) {
        tap = 0;
        chord = now != 0u;
    }
    return tap;
}

enum { BT_NONE, BT_TAP, BT_HOLD };
static uint32_t btn_hold(uint32_t *t0, uint32_t label, uint32_t now, int hold_ok)
{
    uint32_t tap;
    if ((fm1_in.buttons >> panel.btn[label]) & 1u) {
        if (!*t0)
            *t0 = (now | 1u) & ~2u;
        else if (hold_ok && !(*t0 & 2u) && now - (*t0 & ~3u) > 700u * 1000u * FM1_TICKS_PER_US) {
            *t0 |= 2u;
            return BT_HOLD;
        }
        return BT_NONE;
    }
    tap = *t0 && !(*t0 & 2u);
    *t0 = 0;
    return tap ? BT_TAP : BT_NONE;
}

/* every pass's start (the menu, NAME, SCREEN OFF too): the notes since the last pass light their tracks and are this
 * pass's (mf_seen: SCREEN WAKE +MIDI, AUTO SELECT); a pass that returns before midi_follow drops them (no AUTO SELECT
 * later from a note played in the menu, NAME or the dark) */
static void midi_take(void)
{
    uint32_t k, now = fm1_ms | 1u;
    fm1_irq_off();
    mf_seen = midi_seen;
    midi_seen = 0;
    fm1_irq_on();
    for (k = 0; k < NTRK; k++)
        if ((mf_seen >> k) & 1u)
            midi_lit_ms[k] = now;
}

static void ui_input(void)
{
    uint32_t pressed = fm1_input_edges(0), notes = fm1_input_note_edges(), now = fm1_ticks();
    uint32_t home, oct, k;
    midi_take();                                        /* (MIDI IN notes: lights, SCREEN WAKE) */
    midi_pc_poll();                                     /* MIDI Program Change */
    fx_usb_fixed = (ui_prefs & PREF_USB_FIXED) != 0u;   /* (MENU > USB LEVEL: fx.c, audio.c) */
    scope_mix = (ui_prefs2 & (PREF_SCOPE_MIX >> 16)) != 0u;   /* (MENU > SCOPE: fx.c, audio.c) */
    rp_apply();                                         /* (MENU > CLICK, CLICK LEVEL, COUNT-IN: click.c, seq.c) */
    t1_poll();                                          /* (the T-1 mode, always on: ui_t1.c) */
    if (!ui.menu)
        usb_serial_apply();                             /* (MENU > USB SERIAL: when the menu has closed) */
    if (scr_input(pressed, notes))                      /* MENU > SCREEN OFF: dark, or the wake gesture: swallowed */
        return;                                         /* (before the buttons' timers: no tap or hold after it) */
    home = btn_hold(&ui.home_t0, B_HOME, now, 1);
    if (!ui.menu && !ui.confirm) {                      /* the T-1's controls (ui_t1.c); HOME held: the MENU */
        if (home == BT_HOLD) {
            ui.menu = 1;
            ui.force = 1;
            song.seq_mode = 0;
        } else
            t1_input(pressed, notes);
        return;
    }
    oct = oct_taps(pressed, 1);
    song.grid = 0;                                      /* (the menu, a dialog: the keys play) */
    if (home == BT_TAP && ui.menu) {                    /* HOME (pressed, or held) closes the menu, from ABOUT too; */
        menu_close();                                   /* the release of the hold that opened it is no tap */
    } else if (home == BT_HOLD && ui.menu) {            /* HOME held: leave it */
        menu_close();
    }
    if (ui.menu) {
        if (!ui.home_t0)
            menu_input(oct);
        return;
    }
    if (ui.confirm) {                                   /* OCT- cancels, OCT+ does it; no dialog has an action yet */
        if (oct) {
            ui.confirm = 0;
            ui.force = 1;
        }
        enc_drop();
    }
}

/* ---------------------------------------------------- panel setup --- */
/* 30 s without input: give up and keep the old table (a stuck key cannot hang the boot). A piano key cancels at
 * any time: the keys are not part of what is taught, so one works whatever the buttons' table says */
#define SETUP_IDLE_MS 30000u
#define SETUP_GIVE_UP(t0) (fm1_ms - (t0) > SETUP_IDLE_MS || fm1_in.notes != 0u)
static void setup_title(void)
{
    lcd_fill(0, 0, 240, 240, T_BG);
    {   /* the title with its icon (the menu row's), centred together; M from y 8 as before */
        const char *t = "HARDWARE CALIBRATION";
        int32_t x = (240 - (16 + 6 + text_w(&AF_M, t))) / 2;
        cv_begin(240, 24, T_BG);
        GFX_HOOK_ALIGN(0, 0, 240, 0, AL_H | AL_N(2), "calibration title centred");
        GFX_HOOK_ALIGN(0, HEAD_MY + AF_M_CAP_Y, 0, HEAD_MY + AF_M_CAP_Y + AF_M_CAP_H, AL_V | AL_PASS,
                       "header icon on its title's line");
        cv_icon_mid(x, 12, 16, ICON_X_DOCTOR, T_THEME, T_BG);
        cv_text(x + 22, HEAD_MY, &AF_M, t, T_TEXT);
        cv_blit(0, 5);
    }
    draw_text_box(0, 32, 240, &AF_S, "TEACH EACH BUTTON AND KNOB", T_MID, 1);
    lcd_fill(16, 56, 208, 1, T_LINE);
    draw_text_box(0, 206, 240, &AF_S, "ANY KEY: CANCEL", T_DIM, 1);
}
static void setup_show(const char *what, const char *name)     /* "PRESS" / "TURN RIGHT", the control */
{
    draw_text_box(0, 80, 240, &AF_S, what, T_MID, 1);
    draw_text_box(0, 100, 240, &AF_L, name, T_THEME, 1);
}
static void panel_setup(void)
{
    uint32_t i, used = 0, t0 = fm1_ms;
    const panel_t old = panel;
    setup_title();
    while (fm1_in.buttons) {                             /* wait for OCT-/OCT+ release */
        fm1_wdt_feed();
        if (SETUP_GIVE_UP(t0))
            goto timeout;
    }
    fm1_input_edges(0);
    for (i = 0; i < NB; i++) {
        uint32_t p = 0, id;
        setup_show("PRESS", B_NAME[i]);
        t0 = fm1_ms;
        while (!(p & ~used)) {
            fm1_wdt_feed();
            p |= fm1_input_edges(0);
            if (SETUP_GIVE_UP(t0))
                goto timeout;
        }
        for (id = 0; id < 14u; id++)
            if (((p & ~used) >> id) & 1u)
                break;
        panel.btn[i] = (uint8_t)id;
        used |= 1u << id;
    }
    used = 0;
    for (i = 0; i < NE; i++) {
        uint32_t e;
        int32_t st = 0;
        setup_show("TURN RIGHT", E_NAME[i]);
        for (e = 0; e < 7u; e++)
            fm1_enc_take(e);
        t0 = fm1_ms;
        for (;;) {
            fm1_wdt_feed();
            if (SETUP_GIVE_UP(t0))
                goto timeout;
            for (e = 0; e < 7u; e++)
                if (!((used >> e) & 1u) && (st = fm1_enc_take(e)) != 0)
                    break;
            if (e < 7u)
                break;
        }
        panel.enc[i] = (uint8_t)e;
        panel.dir[i] = (int8_t)(st > 0 ? 1 : -1);
        used |= 1u << e;
        fm1_delay_ms(300);
        fm1_enc_take(e);
    }
    panel.magic = PANEL_MAGIC;
    lcd_fill(0, 0, 240, 240, T_BG);
    ui.force = 1;
    return;
timeout:
    panel = old;
    lcd_fill(0, 0, 240, 240, T_BG);
    ui.force = 1;
    ui_message("SETUP CANCELLED");
}
