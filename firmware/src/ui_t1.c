/* SPDX-License-Identifier: GPL-3.0-only
 * Torso T-1 on the FM-1: the UI side (main loop). The T-1 mode is always on (P3-005: the MENU row is gone; P3-006: on the host too);
 * this screen and these controls replace Felucca's pages (HOME held still opens the MENU).
 *
 *   KNOB 1..4      the page's four parameters (torso_t1_ui.c t1_ui_on_encoder_turn); OCT+ held: the second row
 *   SEQ / OCT-     next / previous page: RHYTHM, RATCHET, HARMONY, MOD/CC
 *   GLO held       knob turns move all six tracks (relative)
 *   SAVE/LFO held  TEMP: edits are undone when it is let go; OCT+ while held keeps them
 *   SELECT         tempo (BPM)
 *   PLAY           start / stop
 *   white keys 1-6 select track 1..6; white keys 7-12 its cycle 1..6
 *   black keys 1-6 mute / unmute track 1..6
 * The keys play nothing in this mode (song.grid 2, as NAME). */

static uint8_t t1u_was_on;
static uint8_t t1u_temp_btn, t1u_glo_btn;

static void t1u_temp(int on)                       /* the [TEMP] copy (4.6 KB): no tick between its halves */
{
    fm1_irq_off();
    t1_ui_set_temp(on != 0);
    t1_pipeline_invalidate_masks();
    fm1_irq_on();
}

/* every frame, from ui_input (before Felucca's own handling): the mode follows the setting */
static void t1_poll(void)
{
    uint32_t on = 1u;                               /* P3-006: always on, on the host too */
    if (on != t1u_was_on) {
        t1u_was_on = (uint8_t)on;
        panic_req = (uint8_t)((1u << NTRK) - 1u);   /* the pattern tracks' notes end (and T-1's track 1 starts clean) */
        if (!on && t1_ui_state.temp_active)
            t1u_temp(0);
        t1_ui_set_shift(false);
        t1_ui_set_global(false);
        t1u_temp_btn = t1u_glo_btn = 0;
        ui.force = 1;
    }
    t1_set_mode(on);
}

static int t1u_held(uint32_t b) { return (int)((fm1_in.buttons >> panel.btn[b]) & 1u); }
static int t1u_hit(uint32_t pressed, uint32_t b) { return (int)((pressed >> panel.btn[b]) & 1u); }

static void t1_input(uint32_t pressed, uint32_t notes)
{
    uint32_t k;
    int32_t s;
    int temp, glo;
    song.grid = 2;                                  /* the keys are T-1's: silent */
    if (t1u_hit(pressed, B_PLAY))
        transport_req = song.playing || seq_counting() ? 2u : 1u;
    /* TEMP: SAVE or LFO held */
    temp = t1u_held(B_SAVE) || t1u_held(B_LFO);
    if (temp && !t1u_temp_btn)
        t1u_temp(1);
    else if (!temp && t1u_temp_btn && t1_ui_state.temp_active)
        t1u_temp(0);
    t1u_temp_btn = (uint8_t)temp;
    if (temp && t1_ui_state.temp_active && t1u_hit(pressed, B_OCTUP)) {   /* OCT+ in TEMP: keep the edits */
        fm1_irq_off();
        t1_temp_commit();
        t1_ui_state.temp_active = false;
        fm1_irq_on();
        ui_message("KEPT");
    }
    glo = t1u_held(B_GLO);
    if (glo != t1u_glo_btn)
        t1_ui_set_global(glo != 0);
    t1u_glo_btn = (uint8_t)glo;
    t1_ui_set_shift(!temp && t1u_held(B_OCTUP));
    if (t1u_hit(pressed, B_SEQ))
        t1_ui_next_page();
    if (t1u_hit(pressed, B_OCTDN))
        t1_ui_prev_page();
    for (k = 0; k < 4u; k++)
        if ((s = panel_enc(EN_K1 + k)) != 0)
            t1_ui_on_encoder_turn((uint8_t)k, (int8_t)clamp(s, -64, 64));
    if ((s = panel_enc(EN_SELECT)) != 0)
        song.g[G_BPM] = (int16_t)clamp(song.g[G_BPM] + accel(EN_SELECT, s, 200), GP[G_BPM].min, GP[G_BPM].max);
    panel_enc(EN_ALGO);                             /* (not used here: dropped, so nothing changes behind the screen) */
    panel_enc(EN_PRESET);
    for (k = 0; k < 27u; k++) {
        uint32_t p;
        if (!((notes >> k) & 1u))
            continue;
        p = key_place(k);
        if (key_black(k)) {
            if (p < NUM_TRACKS) {
                Track *t = &t1_sequencer_ram.active_pattern.tracks[p];
                t->muted = !t->muted;
            }
        } else if (p < NUM_TRACKS)
            t1_ui_select_track((uint8_t)p);
        else if (p < 2u * NUM_TRACKS)
            t1_ui_select_cycle((uint8_t)(p - NUM_TRACKS));
    }
    if (t1_ui_state.hot_timeout)
        t1_ui_state.hot_timeout--;
}

/* ------------------------------------------------------------------ screen --- */
#define T1U_CARD_Y 0                                /* (canvas 1: y 24..144) */
#define T1U_CARD_H 52
#define T1U_GRID_Y 62
static uint32_t t1u_hash(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }

static const CycleParams *t1u_cycle(const Track **tp)
{
    const Track *t = &t1_sequencer_ram.active_pattern.tracks[t1_ui_state.selected_track];
    *tp = t;
    return &t->cycles[t->active_cycle % NUM_CYCLES];
}

static void t1_draw(void)
{
    static uint32_t shown;
    const Track *t;
    const CycleParams *cp = t1u_cycle(&t);
    const T1TrackRuntime *rt = &t1_track_runtimes[t1_ui_state.selected_track];
    uint64_t mask = t1_euclidean_eval_cycle(cp);
    uint32_t steps = cp->steps ? cp->steps : 16u, play = song.playing && rt->step_index != 0xFF ? rt->step_index : 99u;
    uint32_t i, h = 2166136261u;
    char head[40], lab[4][16], val[4][16], buf[24];
    const char *unit;
    int32_t ratio[4];
    t1_ui_get_header_string(head, sizeof head);
    for (i = 0; i < 4u; i++) {
        t1_ui_get_card_info((uint8_t)i, lab[i], val[i], &unit, &ratio[i]);
        h = t1u_hash(h, (uint32_t)ratio[i]);
        h = t1u_hash(h, (uint32_t)val[i][0] | (uint32_t)val[i][1] << 8 | (uint32_t)val[i][2] << 16 | (uint32_t)lab[i][0] << 24);
        h = t1u_hash(h, (uint32_t)val[i][3] | (uint32_t)lab[i][1] << 8);
    }
    h = t1u_hash(h, (uint32_t)mask);
    h = t1u_hash(h, (uint32_t)(mask >> 32));
    h = t1u_hash(h, steps | play << 8 | (uint32_t)t1_ui_state.current_page << 16 | (uint32_t)t1_ui_state.selected_track << 20);
    h = t1u_hash(h, (uint32_t)t1_ui_state.shift_active | (uint32_t)t1_ui_state.global_active << 1 |
                    (uint32_t)t1_ui_state.temp_active << 2 | (uint32_t)(t1_ui_state.hot_timeout ? t1_ui_state.hot_encoder : 9u) << 3 |
                    (uint32_t)song.playing << 8 | (uint32_t)song.g[G_BPM] << 9);
    for (i = 0; i < NUM_TRACKS; i++)
        h = t1u_hash(h, (uint32_t)t1_sequencer_ram.active_pattern.tracks[i].muted |
                        (uint32_t)t1_sequencer_ram.active_pattern.tracks[i].active_cycle << 1 |
                        (uint32_t)(t1_track_runtimes[i].step_index == 0u && song.playing) << 4);
    if (!ui.force && h == shown) {
        draw_head();
        return;
    }
    shown = h;
    if (ui.force)
        ui.head_sig = ~0u;
    ui.force = 0;
    draw_head();

    /* canvas 1 (y 24..144): the title, the four knob cards, the steps */
    cv_begin(240, 120, T_BG);
    cv_text(8, 4, &AF_S, head, t1_ui_state.temp_active || t1_ui_state.global_active ? T_ACCENT : T_TEXT);
    for (i = 0; i < 4u; i++) {
        int32_t x = 4 + (int32_t)i * 59, y = 20;
        int hot = t1_ui_state.hot_timeout && t1_ui_state.hot_encoder == i;
        uint16_t fill = hot ? T_THEME : T_SURF;
        cv_rrect(x, y, 55, 40, 6, fill, T_BG);
        cv_text_in(x, y + 4, 55, &AF_S, lab[i], hot ? T_INK : T_MID, fill);
        cv_text_in(x, y + 19, 55, &AF_M, val[i], hot ? T_INK : T_TEXT, fill);
        if (ratio[i] >= 0)
            cv_rect(x + 4, y + 36, (int32_t)clamp(ratio[i], 0, 1000) * 47 / 1000, 2, hot ? T_INK : T_THEME);
    }
    for (i = 0; i < steps && i < 64u; i++) {        /* 16 a row, up to 4 rows: pulse, rest, the playhead */
        int32_t x = 8 + (int32_t)(i % 16u) * 14, y = T1U_GRID_Y + 4 + (int32_t)(i / 16u) * 13;
        int on = (int)((mask >> i) & 1u);
        uint16_t c = i == play ? T_ACCENT : on ? T_THEME : T_RAISE;
        cv_rect(x, y, 11, 10, c);
    }
    cv_blit(0, H_HEAD);

    /* canvas 2 (y 144..240): the six tracks, the tempo, the controls */
    cv_begin(240, 96, T_BG);
    for (i = 0; i < NUM_TRACKS; i++) {
        const Track *k = &t1_sequencer_ram.active_pattern.tracks[i];
        int32_t x = 6 + (int32_t)i * 39;
        int sel = i == t1_ui_state.selected_track, hit = song.playing && t1_track_runtimes[i].step_index == 0u;
        uint16_t fill = sel ? T_THEME : T_SURF, ink = k->muted ? T_DIM : sel ? T_INK : T_TEXT;
        char n[8] = {'T', (char)('1' + i), 0};
        cv_rrect(x, 4, 35, 36, 6, fill, T_BG);
        cv_text_in(x, 7, 35, &AF_M, n, ink, fill);
        if (k->muted)
            cv_text_in(x, 25, 35, &AF_S, "MUTE", ink, fill);
        else {
            n[0] = i ? 'C' : 'F';                   /* track 1: the FM-1's synth; 2..6: MIDI channel */
            if (i) {
                snprintf(n, sizeof n, "CH%u", (uint32_t)(k->midi_channel & 15u) + 1u);
            } else
                str_cpy(n, "FM-1", sizeof n);
            cv_text_in(x, 25, 35, &AF_S, n, ink, fill);
        }
        if (hit)
            cv_rect(x + 4, 37, 27, 2, T_ACCENT);
    }
    snprintf(buf, sizeof buf, "%u BPM  %s", (uint32_t)song.g[G_BPM], song.playing ? "PLAY" : "STOP");
    cv_text(8, 50, &AF_M, buf, song.playing ? T_ACCENT : T_TEXT);
    cv_text(8, 72, &AF_S, "SEQ PAGE  OCT+ SHIFT  GLO ALL  SAVE TEMP", T_MID);
    cv_blit(0, 144);
}
