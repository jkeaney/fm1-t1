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
    if (seen != sq_seen_epoch) { uisq_bpm_req = 0; song.g[G_BPM] = (int16_t)clamp((int32_t)sq_bpm, GP[G_BPM].min, GP[G_BPM].max); }   /* a load landed: its tempo wins (over a SET K1 turn too) */
    else {
        if (uisq_bpm_req) { song.g[G_BPM] = (int16_t)clamp((int32_t)uisq_bpm_req, GP[G_BPM].min, GP[G_BPM].max); uisq_bpm_req = 0; }   /* SET K1 (ui_sq.c): the own tempo, the one value song.g[G_BPM] */
        sq_bpm = (uint16_t)song.g[G_BPM]; } }

static void ui_sq_poll(void)
{   static uint8_t was_sel = 0xFFu;
    if (uisq_sel != was_sel) {                          /* the selected channel moved (a page, or the first frame): the ISR's copy follows */
        was_sel = uisq_sel;
        sq_sel = uisq_sel;
        ui.force = 1; } }

static void ui_sq_controls(uint32_t pressed, uint32_t notes)
{   static const uint8_t pb[9] = { B_FX, B_SCL, B_ENV, B_LFO, B_EDIT, B_ARP, B_SEQ, B_OCTDN, B_OCTUP };            /* the panel buttons the PERF page reads ... */
    static const uint8_t pc[9] = { UI_C_B_FX, UI_C_B_SCL, UI_C_B_ENV, UI_C_B_LFO, UI_C_B_EDIT, UI_C_B_ARP, UI_C_B_SEQ, UI_C_B_OCTDN, UI_C_B_OCTUP };   /* ... and their control ids */
    static const uint8_t po[4]  = { B_PLAY, B_GLO, B_REC, B_SAVE };
    static const uint8_t poc[4] = { UI_C_B_PLAY, UI_C_B_GLO, UI_C_B_REC, UI_C_B_SAVE };
    uint32_t k;
    (void)notes;                                        /* the key EDGES are not used: ui_sq_keys() diffs the held state itself */
    song.grid = 2;                                      /* the keys never sound by themselves: a key is a step, an M key, a page key (PERF page) */
    if ((pressed >> panel.btn[B_PLAY]) & 1u)
        transport_req = song.playing || seq_counting() ? 2u : 1u;
    for (k = 0; k < 9u; k++)
        if ((pressed >> panel.btn[pb[k]]) & 1u) ui_sq_button(pc[k]);
    for (k = 0; k < 4u; k++) ui_sq_knob((uint8_t)k, panel_enc(EN_K1 + k));
    ui_sq_select_turn(panel_enc(EN_SELECT));
    for (k = 0; k < 4u; k++)                            /* the buttons with no page behaviour yet: on CHAN they are an exit for the pending MIDI channel (P4-015) */
        if ((pressed >> panel.btn[po[k]]) & 1u) ui_sq_other_button(poc[k]);
    ui_sq_algo_turn(panel_enc(EN_ALGO));                /* CHAN: the algorithm picker (P4-015); PERF's is P4-032's */
    (void)panel_enc(EN_PRESET);
    ui_sq_keys(fm1_in.notes);
    ui_sq_home((uint8_t)((fm1_in.buttons >> panel.btn[B_HOME]) & 1u));   /* HOME tap leaves NOTE_EDIT (a hold is the MENU's: ui_input.c never gets here) */
    ui_sq_input(); }

static void ui_sq_screen(void)
{   (void)ui_sq_draw();
    ui.force = 0; }
