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
 *                      dispatch (ui_sq_input: the stub pages); every other control is a page task's: the keys are silent (song.grid 2),
 *                      the knobs and encoders are drained so nothing piles up behind the screen.
 *                      NOT wired here, and P4-013 must: SELECT adds a channel and an M-key held 1.5 s removes it / held for the
 *                      preview (SCREEN_SPEC_V2 6, key_map.tsv, button_modes.tsv; the tables and timings are in build/gen/ui_tables.h),
 *                      the white keys as steps, K1-K4 as morphs (sq_morph_turn), the dice buttons.
 *   ui_sq_screen()     the draw: ui_sq_draw() blits the strips whose signature changed; the force flag is spent here.
 * Included by the unity build and by the host tests (tests/ui_test.c) after ui_input.c. */

static void sq_frame(void)
{   const uint8_t seen = sq_seen_epoch;
    SQ_FRAME_SYNC();
    if (seen != sq_seen_epoch) song.g[G_BPM] = (int16_t)clamp((int32_t)sq_bpm, GP[G_BPM].min, GP[G_BPM].max);   /* a load landed: its tempo wins */
    else sq_bpm = (uint16_t)song.g[G_BPM]; }

static void ui_sq_poll(void)
{   static uint8_t was_sel = 0xFFu;
    if (uisq_sel != was_sel) {                          /* the selected channel moved (a page, or the first frame): the ISR's copy follows */
        was_sel = uisq_sel;
        sq_sel = uisq_sel;
        ui.force = 1; } }

static void ui_sq_controls(uint32_t pressed, uint32_t notes)
{   uint32_t k;
    (void)notes;                                        /* the keys are the PERF page's (P4-013): silent until then, as the T-1 screen's were */
    song.grid = 2;
    if ((pressed >> panel.btn[B_PLAY]) & 1u)
        transport_req = song.playing || seq_counting() ? 2u : 1u;
    for (k = 0; k < 4u; k++) (void)panel_enc(EN_K1 + k);   /* (P4-013/P6-001: the morphs) */
    (void)panel_enc(EN_SELECT);                         /* (P4-013: SELECT adds a channel) */
    (void)panel_enc(EN_ALGO);
    (void)panel_enc(EN_PRESET);
    ui_sq_input(); }

static void ui_sq_screen(void)
{   (void)ui_sq_draw();
    ui.force = 0; }
