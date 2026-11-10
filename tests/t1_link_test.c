/* SPDX-License-Identifier: GPL-3.0-only */
/* The T-1 through the real seq.c: host tests link the real torso_t1*.c / t1_glue.c / ui_t1.c (P3-001, ADR-0003 16.1).
 *   T1 on, PLAY, 2 bars of events_block: note-ons reach midi_out_event on MIDI channel 2 (the default bass track),
 *   none on the other channels; STOP sends All Notes Off (CC123) on channel 2 and every sounding note ends;
 *   T1 off: events_block sends nothing.
 * Run by tests/run_tests.sh (needs build/gen from one firmware build). */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int bad;
static void ck(const char *what, int ok)
{
    printf("t1_link: %-84s %s\n", what, ok ? "ok" : "FAIL");
    bad += !ok;
}

static uint32_t n_on[16], n_off[16], n_cc123[16], n_other_cc;
static void drain(void)                                  /* the captured midi_out_event log */
{
    while (mo_r != mo_w) {
        uint32_t pkt = midi_out_q[mo_r % MQ], st = (pkt >> 8) & 0xFFu, ch = st & 0x0Fu, d1 = (pkt >> 16) & 0x7Fu,
                 d2 = (pkt >> 24) & 0x7Fu;
        mo_r++;
        if ((st & 0xF0u) == 0x90u && d2) n_on[ch]++;
        else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u) n_off[ch]++;
        else if ((st & 0xF0u) == 0xB0u && d1 == 123u) n_cc123[ch]++;
        else n_other_cc++;
    }
}
static uint32_t total(const uint32_t *a) { uint32_t i, s = 0; for (i = 0; i < 16; i++) s += a[i]; return s; }

int main(void)
{
    uint32_t i, bar;
    ui_power_on();
    usb.config = 1;
    mo_w = mo_r = 0;
    t1_glue_init();
    song.g[G_BPM] = 120;
    song.g[G_CLOCK] = 0;
    ck("t1_mode is the real variable, off at power-on", t1_mode == 0);

    /* T1 off: PLAY sends nothing on MIDI OUT */
    transport_req = 1; events_block(CTL);
    for (i = 0; i < 200; i++) { events_block(CTL); drain(); }
    ck("T1 off: no T-1 note reaches MIDI OUT", total(n_on) == 0 && total(n_cc123) == 0);
    transport_req = 2; events_block(CTL); drain();

    /* T1 on: 2 bars (4/4 at 120 BPM = 2 s each) */
    memset(n_on, 0, sizeof n_on); memset(n_off, 0, sizeof n_off); memset(n_cc123, 0, sizeof n_cc123);
    t1_set_mode(1);
    ck("t1_set_mode(1) sets t1_mode", t1_mode == 1);
    transport_req = 1; events_block(CTL); drain();
    memset(n_cc123, 0, sizeof n_cc123);                      /* (the start's own All Notes Off) */
    ck("PLAY: song.playing", song.playing != 0);
    bar = (uint32_t)FS * 2u / CTL;
    for (i = 0; i < 2u * bar; i++) { events_block(CTL); drain(); }
    printf("t1_link: 2 bars: ch2 note-ons %u, note-offs %u\n", n_on[1], n_off[1]);
    ck("2 bars: channel-2 note-ons reach the captured midi_out_event", n_on[1] >= 8u);
    ck("2 bars: nothing on the other channels", total(n_on) == n_on[1]);

    /* STOP: CC123 on channel 2 */
    transport_req = 2; events_block(CTL); drain();
    ck("STOP: CC123 (All Notes Off) on channel 2", n_cc123[1] >= 1u);
    ck("STOP: every started note was ended", n_off[1] >= n_on[1] - 1u);
    ck("STOP: transport stopped", song.playing == 0);

    /* T1 off again */
    t1_set_mode(0); events_block(CTL); drain();
    ck("t1_set_mode(0) clears t1_mode", t1_mode == 0);
    printf("t1_link: %s\n", bad ? "FAILED" : "all checks ok");
    return bad;
}
