/* SPDX-License-Identifier: GPL-3.0-only
 * Torso T-1 on the FM-1: the audio-ISR side. Clocks the T-1 pipeline (torso_t1_pipeline.c) from the sequencer's own
 * sample count (events_block's seq_n: the internal BPM, or an external MIDI clock as seq.c follows it), sends tracks
 * 2..6 to USB-MIDI OUT on their channels and plays track 1 on Felucca's track 1 (its engine, its preset).
 *
 * T1 mode (t1_mode, set by the UI in ui_t1.c): while on, the T-1 tracks play in place of Felucca's four pattern
 * tracks (seq_tick is skipped). PLAY / STOP / external Start / Stop drive both. Nothing here touches the flash.
 *
 * Out of the box (t1_default_pattern): T-1 track 2 plays a generative bassline on MIDI channel 2, the Roland TB-3's
 * default receive channel. The FM-1 has no MIDI OUT jack: a TB-3 is reached over USB through a USB-MIDI host
 * (FM-1 USB -> host -> 5-pin DIN -> TB-3 MIDI IN). */

/* t1_mode (1: the T-1 plays; the UI writes, the ISR reads at a block) and t1_off_req: declared in seq.c */
static uint8_t t1_ready;                           /* t1_glue_init done (the ISR does nothing before) */
static uint32_t t1_tick;                           /* 192 PPQN ticks since PLAY */
static uint32_t t1_acc;                            /* PPQN_RATE * samples, against beat_samples() */
static uint8_t t1_fm_note[T1_FM_POLYPHONY];        /* note + 1 sounding on Felucca track 1 per T-1 voice, 0: none */

/* T-1 track 1 -> Felucca track 1. The T-1 voice allocator (torso_t1_pipeline.c) hands out voices 0..5; Felucca
 * does its own voicing, so a voice here is only which note to release */
void synth_note_on(uint8_t v, uint8_t note, uint8_t vel)
{
    if (v >= T1_FM_POLYPHONY)
        return;
    if (t1_fm_note[v])
        trk_note_off(&trk[0], t1_fm_note[v] - 1u);
    t1_fm_note[v] = (uint8_t)(note + 1u);
    trk_note_on(&trk[0], note, vel ? vel : 1u);
}

void synth_note_off(uint8_t v)
{
    if (v >= T1_FM_POLYPHONY || !t1_fm_note[v])
        return;
    trk_note_off(&trk[0], t1_fm_note[v] - 1u);
    t1_fm_note[v] = 0;
}

static void t1_usb_out(uint32_t pkt) { midi_out_event(pkt); }

/* The default pattern: something that plays at PLAY. Track 1 (the FM-1's own synth) rests (0 pulses: the T-1
 * default); track 2 is the TB-3 bassline: E(7,16), A minor pentatonic (scale 10 P-MIN) around A1, random pitch and accents,
 * long gates for slides (an overlapping note is a TB-3 slide). */
static void t1_default_pattern(void)
{
    CycleParams *b = &t1_sequencer_ram.active_pattern.tracks[1].cycles[0];
    b->steps = 16;
    b->pulses = 7;
    b->division = 4;                               /* 1/16 */
    b->pitch_offset = -27;                         /* 60 - 27 = 33: A1 */
    b->root_note = 9;                              /* A */
    b->scale_id = 10;                              /* minor pentatonic */
    b->phrase = 2;                                 /* TRI contour */
    b->range = 12;
    b->time = 127;
    b->sustain = 90;
    b->velocity = 80;
    b->accent = 40;                                /* 80 -> 120 on an accent */
    b->probability = 95;
    b->random_targets = T1_RND_TARGET_PITCH | T1_RND_TARGET_ACCENT;
    b->random_depth = 14;                          /* random pitch: +-7 semitones, then into the scale */
    b->random_rate = 3;                            /* the random line repeats every 4 cycles (0..4: 1 2 3 4 6) */
    t1_sequencer_ram.active_pattern.tracks[1].midi_channel = 1;   /* MIDI channel 2 */
    memcpy(&t1_sequencer_ram.snapshot_pattern, &t1_sequencer_ram.active_pattern, sizeof(Pattern));
}

/* boot, before the audio ISR runs (main.c) */
static void t1_glue_init(void)
{
    t1_system_init();
    t1_pipeline_init();
    t1_state_init();
    t1_ui_init();
    t1_default_pattern();
    memset(t1_fm_note, 0, sizeof t1_fm_note);
    t1_ready = 1;
}

/* every note the T-1 sounds ends now: track 1's voices, and on MIDI OUT each note-off still queued (sent at once,
 * so the TB-3 is never left holding a note), then All Notes Off on each MIDI track's channel */
static void t1_all_off(void)
{
    uint16_t i;
    t1_pipeline_kill_all_voices();
    for (i = t1_micro_queue.tail; i != t1_micro_queue.head; i = (uint16_t)((i + 1u) % T1_MICRO_QUEUE_CAPACITY)) {
        const T1MicroEvent *e = &t1_micro_queue.events[i];
        if (e->type == T1_EVENT_NOTE_OFF && e->track)
            midi_out_event(0x08u | (0x80u | (e->channel & 0x0Fu)) << 8 | (uint32_t)(e->note & 0x7Fu) << 16);
    }
    t1_micro_queue_init();
    midi_ring_init(&t1_midi_uart_queue);
    midi_ring_init(&t1_midi_usb_queue);
    for (i = 1; i < NUM_TRACKS; i++)
        midi_out_event(0x0Bu | (0xB0u | (t1_sequencer_ram.active_pattern.tracks[i].midi_channel & 0x0Fu)) << 8 |
                       123u << 16);
}

/* seq.c seq_start / seq_stop */
/* each track's random line from its seed, with its cycle's loop length (RND RATE): PLAY plays the same line */
static void t1_lfsr_reseed(void)
{
    for (uint8_t t = 0; t < NUM_TRACKS; t++) {
        const Track *k = &t1_sequencer_ram.active_pattern.tracks[t];
        t1_lfsr_init(&t1_track_runtimes[t].lfsr, (uint16_t)(0x1234u + t * 0x1111u),
                     k->cycles[k->active_cycle % NUM_CYCLES].random_rate);
    }
}

static void t1_on_start(void)
{
    if (!t1_ready || !t1_mode)                     /* (T1 mode off: Felucca as it was, nothing sent) */
        return;
    t1_all_off();
    t1_pipeline_reset_transport();
    t1_pipeline_invalidate_masks();
    t1_lfsr_reseed();
    t1_tick = 0;
    t1_acc = beat_samples();                       /* tick 0 on the first block */
}

static void t1_on_stop(void)
{
    if (t1_ready && t1_mode)
        t1_all_off();
}

/* events_block, in place of the pattern tracks' seq_tick while T1 mode is on: n samples of sequencer time */
static void t1_block(uint32_t n)
{
    uint32_t bs = beat_samples();
    if (!t1_ready || !song.playing || !bs)
        return;
    t1_acc += n * PPQN_RATE;
    while (t1_acc >= bs) {
        t1_acc -= bs;
        t1_pipeline_tick(t1_tick++);
    }
    t1_pipeline_flush_midi(0, t1_usb_out);
}

/* T1 mode on / off (the UI). Turned off while playing: the T-1's notes end with it */
static void t1_set_mode(uint32_t on)
{
    if (!on && t1_mode)
        t1_off_req = 1;
    t1_mode = (uint8_t)(on != 0u);
}
