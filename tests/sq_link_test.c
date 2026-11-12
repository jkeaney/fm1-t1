/* SPDX-License-Identifier: GPL-3.0-only */
/* The sequencer core through the real seq.c (P4-012; replaces tests/t1_link_test.c): the host tests link the real sq_*.c, ui_sq.c and
 * ui_sq_glue.c next to the real seq.c / ui*.c (tests/host_sq_hooks.h, ADR-0003 16.1).
 *   sq_ready off: events_block plays the pattern tracks as before and sends nothing on MIDI OUT.
 *   sq_ready on, PLAY, 2 bars of events_block: note-ons of CH1 (ACID, MIDI channel 1) reach midi_out_event_at, none on the other
 *   channels (CH2..CH4 are muted); STOP sends the note-offs, then CC123 on channel 1, and every started note ends.
 *   FB-014: every ALGOS[] row without build / step (rows 1..5 until P5-002..P5-008) is an inert channel: select it, rebuild, run ticks,
 *   no crash and no note from that channel; the other channels are untouched.
 *   Live notes: sq_live_on / sq_live_off play on the selected channel's MIDI channel and the off follows after the selection moved.
 *   SQ_IS_MASTER is clkx_master (clock out only on INT); sq_half_begin is called by the audio ISR.
 * Run by tests/run_tests.sh (needs build/gen from one firmware build). */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int bad;
static void ck(const char *what, int ok)
{
    printf("sq_link: %-90s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

static uint32_t n_on[16], n_off[16], n_cc123[16], n_clock;
static void drain(void)                                  /* the captured midi_out_event_at log */
{
    while (mo_r != mo_w) {
        uint32_t pkt = midi_out_q[mo_r % MQO], st = (pkt >> 8) & 0xFFu, ch = st & 0x0Fu, d1 = (pkt >> 16) & 0x7Fu,
                 d2 = (pkt >> 24) & 0x7Fu;
        mo_r++;
        if (st >= 0xF8u) n_clock++;
        else if ((st & 0xF0u) == 0x90u && d2) n_on[ch]++;
        else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u) n_off[ch]++;
        else if ((st & 0xF0u) == 0xB0u && d1 == 123u) n_cc123[ch]++;
    }
}
static uint32_t total(const uint32_t *a) { uint32_t i, s = 0; for (i = 0; i < 16; i++) s += a[i]; return s; }
static void zero(void) { memset(n_on, 0, sizeof n_on); memset(n_off, 0, sizeof n_off); memset(n_cc123, 0, sizeof n_cc123); n_clock = 0; }
static void bars(uint32_t nbars)                         /* 4/4 at 120 BPM = 2 s a bar */
{
    uint32_t i, n = nbars * (uint32_t)FS * 2u / CTL;
    for (i = 0; i < n; i++) { events_block(CTL); drain(); }
}

int main(void)
{
    uint32_t a;
    ui_power_on();
    usb.config = 1;
    mo_w = mo_r = 0;
    song.g[G_BPM] = 120;
    song.g[G_CLOCK] = 0;
    ck("sq_ready is the real variable, off at power-on of the test (the pattern tracks play)", sq_ready == 0);

    /* the core off: PLAY sends nothing on MIDI OUT */
    transport_req = 1; events_block(CTL);
    bars(1);
    ck("core off: no sequencer note reaches MIDI OUT", total(n_on) == 0 && total(n_cc123) == 0);
    transport_req = 2; events_block(CTL); drain();

    /* the core on: 2 bars */
    sq_boot();
    zero();
    ck("sq_boot sets sq_ready last", sq_ready == 1);
    ck("SQ_IS_MASTER is clkx_master (INT: the master)", SQ_IS_MASTER() == 1);
    transport_req = 1; events_block(CTL); drain();
    ck("PLAY: song.playing and sq_playing", song.playing != 0 && sq_playing == 1);
    zero();
    bars(2);
    printf("sq_link: 2 bars: channel-1 note-ons %u, note-offs %u, clock bytes %u\n", n_on[0], n_off[0], n_clock);
    ck("2 bars: channel-1 note-ons reach the captured midi_out_event_at", n_on[0] >= 8u);
    ck("2 bars: nothing on the other channels (CH2..CH4 muted)", total(n_on) == n_on[0]);
    ck("2 bars: 24 clock bytes a quarter while the master (INT)", n_clock >= 2u * 4u * 24u - 24u);

    /* live notes follow the selection, the off follows the on */
    zero();
    sq_ch[1].muted = 0; sq_ch[1].midi_ch = 5;            /* CH2 audible on MIDI channel 6 */
    sq_sel = 1; sq_live_on(60, 100);
    sq_sel = 0; sq_live_off(60);
    events_block(CTL); drain();
    ck("live: a note on the selected channel goes out its MIDI channel (6)", n_on[5] == 1u);
    ck("live: the note-off follows the note-on after the selection moved", n_off[5] >= 1u);
    sq_ch[1].muted = 1;

    /* STOP: note-offs then CC123 */
    zero();
    transport_req = 2; events_block(CTL); drain();
    ck("STOP: CC123 (All Notes Off) on channel 1", n_cc123[0] >= 1u);
    ck("STOP: transport stopped", song.playing == 0 && sq_playing == 0);
    ck("STOP: no sounding note is left (ChanRT.snd empty on CH1)", !sq_rt[0].snd[0] && !sq_rt[0].snd[1]);

    /* FB-014: the empty ALGOS[] rows */
    for (a = 0; a < SQ_NALGO; a++) {
        char what[96];
        const uint8_t empty = ALGOS[a].build == 0 || ALGOS[a].step == 0;
        if (!empty) continue;
        sq_boot();
        sq_ch[0].algo = (uint8_t)a;
        sq_rebuild(0);
        sq_set_algo(0, (uint8_t)a);
        sq_morph_turn(0, 0, 1);                           /* a morph on an empty row: nothing to dereference */
        sq_dice_all(0);
        zero();
        transport_req = 1; events_block(CTL);
        bars(2);
        snprintf(what, sizeof what, "empty ALGOS[%u] row on CH1: 2 bars, no crash, no note from it", (unsigned)a);
        ck(what, total(n_on) == 0 && sq_playing == 1);
        (void)ui_sq_draw();                               /* the screen draws it too */
        transport_req = 2; events_block(CTL); drain();
    }
    ck("ACID (row 0) is not an empty row", ALGOS[0].build != 0 && ALGOS[0].step != 0);

    /* sq_half_begin is the audio ISR's: grep-able in audio.c (the host does not build audio.c here) */
    printf("sq_link: %s\n", bad ? "FAILED" : "all checks ok");
    return bad;
}
