/* SPDX-License-Identifier: GPL-3.0-only */
/* ui_sq_glue.c: what joins the sequencer screen (ui_sq.c) to the Felucca main loop (P4-012, the cut-over R8). It replaces the T-1 screen.
 *
 * Included right after ui_sq.c, which stays free of panel, transport and song symbols so tests/ui_sq*_test.c can build it alone.
 * What is here:
 *   sq_frame()         once per main-loop frame, BEFORE ui_input(): sq_chan_sync() (ADR-0003 6: a preset the ISR landed clears the undo
 *                      records and rebuilds the caches), then the tempo is made one value: song.g[G_BPM] (what the audio side and the
 *                      external-clock follower write) is copied into sq_bpm (what the screen shows), unless a preset has just landed,
 *                      in which case its tempo goes the other way.
 *   ui_sq_poll()       every frame from ui_input(): keeps the screen and the selected channel in step (sq_sel is the ISR's copy).
 *   ui_sq_controls()   the frame's input while the sequencer screen has the panel. P4-012 wires the transport (PLAY) and the page
 *                      dispatch (ui_sq_input). P4-015 wires the CHAN page: the CHN key (inside ui_sq_keys), K1-K4 / OCT / SELECT by page, the ALGO encoder (ui_sq_algo_turn) and
 *                      PLAY / REC / SAVE as exits (ui_sq_other_button); P4-016 adds GLO (a tap opens SET and closes it: ui_sq_other_button), K1 BPM through uisq_bpm_req (sq_frame hands it to song.g[G_BPM]). P4-017 adds REC (opens / closes the REC page: ui_sq_other_button; the keys then write steps and queue their notes for the ISR, FB-019). P4-013 wires the PERF page (ui_sq.c): SELECT (channel, one past the last adds), the M keys (tap mute,
 *                      hold preview, held 1.5 s remove), the white keys (tap step ON/OFF, held 0.4 s NOTE_EDIT), PG-/PG+, K1-K4 (sq_morph_turn),
 *                      the dice buttons, EDIT (undo / redo) and OCT-/OCT+ (the knob row). P4-014 wires the NOTE_EDIT page through the same calls (the buttons and
 *                      knobs route by page inside ui_sq.c) plus ui_sq_home for HOME. The keys stay silent (song.grid 2). ALGO and PRESETS
 *                      are drained; CHN, SOL, FIL, ROL, RST and the other buttons are the later page tasks'.
 *   ui_sq_screen()     the draw: ui_sq_draw() blits the strips whose signature changed; the force flag is spent here.
 * Included by the unity build and by the host tests (tests/ui_test.c) after ui_input.c. */

static void sq_frame(void)
{   const uint8_t seen = sq_seen_epoch;
    SQ_FRAME_SYNC();
    if (seen != sq_seen_epoch) { uisq_bpm_req = 0; song.g[G_BPM] = (int16_t)clamp((int32_t)sq_bpm, GP[G_BPM].min, GP[G_BPM].max);   /* a load landed: its tempo wins (over a SET K1 turn too) */
        uisq_gone_n = 0; uisq_undone = 0;                                  /* P4-044: the load replaced the in-use mask: a stale remove-undo stack would revive the wrong channel */
        if (!(uisq_used >> (uisq_sel % SQ_NCH) & 1u)) { uint8_t c = 0; while (c < SQ_NCH - 1u && !(uisq_used >> c & 1u)) c++; uisq_sel = c; uisq_vpage = 0; }   /* the selection must be a channel in use */
    }
    else {
        if (uisq_bpm_req) { song.g[G_BPM] = (int16_t)clamp((int32_t)uisq_bpm_req, GP[G_BPM].min, GP[G_BPM].max); uisq_bpm_req = 0; }   /* SET K1 (ui_sq.c): the own tempo, the one value song.g[G_BPM] */
        sq_bpm = (uint16_t)song.g[G_BPM]; } }

static void ui_sq_poll(void)
{   static uint8_t was_sel = 0xFFu, fr_init;
    if (!fr_init) {                                     /* P4-018: the first-run strip runs until its byte (ui.c ui_first_run) says it was done (once per device) */
        fr_init = 1;
        if (ui_first_run != 1u) { uisq_fr_on = 1; uisq_fr_step = 1; uisq_fr_other = 0; ui_sq_new_set(0, 1); } }
    if (uisq_fr_save) {                                 /* ... and ended (a step 7 done, or HOME skipped it): keep that, once */
        uisq_fr_save = 0;
        ui_first_run = 1;
        settings_save(); }
    if (uisq_sel != was_sel) {                          /* the selected channel moved (a page, or the first frame): the ISR's copy follows */
        was_sel = uisq_sel;
        sq_sel = uisq_sel;
        ui.force = 1; } }

static void ui_sq_controls(uint32_t pressed, uint32_t notes)
{   static const uint8_t pb[8] = { B_FX, B_SCL, B_ENV, B_LFO, B_ARP, B_SEQ, B_OCTDN, B_OCTUP };            /* the panel buttons the PERF page reads (EDIT has its own level handler) ... */
    static const uint8_t pc[8] = { UI_C_B_FX, UI_C_B_SCL, UI_C_B_ENV, UI_C_B_LFO, UI_C_B_ARP, UI_C_B_SEQ, UI_C_B_OCTDN, UI_C_B_OCTUP };   /* ... and their control ids */
    static const uint8_t po[4]  = { B_PLAY, B_GLO, B_REC, B_SAVE };
    static const uint8_t poc[4] = { UI_C_B_PLAY, UI_C_B_GLO, UI_C_B_REC, UI_C_B_SAVE };
    uint32_t k;
    int32_t kn[4], sel, algo, pre;
    const uint8_t hd = (uint8_t)((fm1_in.buttons >> panel.btn[B_HOME]) & 1u);       /* HOME is down */
    uint32_t mask = fm1_in.notes & ((1u << UISQ_NKEY) - 1u);
    static uint8_t edit_masked;                         /* EDIT pressed under HELP: not a press of its own once HOME is up, until it is released */
    const uint8_t ed = (uint8_t)((fm1_in.buttons >> panel.btn[B_EDIT]) & 1u);
    (void)notes;                                        /* the key EDGES are not used: ui_sq_keys() diffs the held state itself */
    song.grid = 2;                                      /* the keys never sound by themselves: a key is a step, an M key, a page key (PERF page) */
    ui_sq_home(hd);                                     /* first: a HOME press opens the HELP window before any other control of the frame is read */
    for (k = 0; k < 4u; k++) kn[k] = panel_enc(EN_K1 + k);
    sel = panel_enc(EN_SELECT); algo = panel_enc(EN_ALGO); pre = panel_enc(EN_PRESET);
    uisq_help_keys &= mask;                             /* a key pressed under HELP stays out of the page until it is physically up */
    if ((pressed >> panel.btn[B_PLAY]) & 1u) {
        transport_req = song.playing || seq_counting() ? 2u : 1u;                   /* PLAY always plays, under HELP too */
        if (hd) ui_sq_help_touch(UI_C_B_PLAY); else ui_sq_play_pressed(); }
    if (hd) {                                           /* HELP: every control is explained and none acts */
        static const uint8_t ob[4] = { UI_C_B_GLO, UI_C_B_REC, UI_C_B_SAVE, UI_C_B_EDIT };
        static const uint8_t obp[4] = { B_GLO, B_REC, B_SAVE, B_EDIT };
        uint32_t fresh = mask & ~uisq_keys & ~uisq_help_keys;
        for (k = 0; k < 8u; k++) if ((pressed >> panel.btn[pb[k]]) & 1u) ui_sq_help_touch(pc[k]);
        for (k = 0; k < 4u; k++) if ((pressed >> panel.btn[obp[k]]) & 1u) ui_sq_help_touch(ob[k]);
        for (k = 0; k < 4u; k++) if (kn[k]) ui_sq_help_touch((uint8_t)(UI_C_EN_K1 + k));
        if (sel) ui_sq_help_touch(UI_C_EN_SELECT);
        if (algo) ui_sq_help_touch(UI_C_EN_ALGO);
        if (pre) ui_sq_help_touch(UI_C_EN_PRESET);
        for (k = 0; k < (uint32_t)UISQ_NKEY; k++) if (fresh >> k & 1u) ui_sq_help_touch(uisq_key_ctl((uint8_t)k));
        uisq_help_keys |= fresh;
        if (ed) edit_masked = 1;
        if (uisq_edit_dn) uisq_edit_fired = 1;          /* an EDIT held when HOME went down is no tap on its release */
        ui_sq_edit(0);
        ui_sq_keys(mask & ~uisq_help_keys);
        ui_sq_input();
        return; }
    if (!ed) edit_masked = 0;
    ui_sq_edit((uint8_t)(ed && !edit_masked));
    for (k = 0; k < 8u; k++)
        if ((pressed >> panel.btn[pb[k]]) & 1u) ui_sq_button(pc[k]);
    for (k = 0; k < 4u; k++) ui_sq_knob((uint8_t)k, kn[k]);
    ui_sq_select_turn(sel);
    for (k = 0; k < 4u; k++)                            /* the buttons with no page behaviour yet: on CHAN they are an exit for the pending MIDI channel (P4-015) */
        if ((pressed >> panel.btn[po[k]]) & 1u) ui_sq_other_button(poc[k]);
    ui_sq_algo_turn(algo);                              /* CHAN: the algorithm picker (P4-015); PERF's is P4-032's */
    ui_sq_preset_turn(pre);                             /* (the preset layer is P7: under RESET PRESETS only says it has nothing to reset) */
    ui_sq_keys(mask & ~uisq_help_keys);
    ui_sq_input(); }

static void ui_sq_screen(void)
{   (void)ui_sq_draw();
    ui.force = 0; }
