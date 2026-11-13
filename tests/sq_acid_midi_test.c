/* SPDX-License-Identifier: GPL-3.0-only */
/* ACID on the MIDI wire (P4-041): the real events_block -> sq_block -> sq_core -> midi_out_event_at, with the stamp of every packet
 * (the due time in TIMER4 ticks, 544 per sample) read from the usb.c ring the host test captures. A fixed 16-step pattern is written as
 * StepEdit on CH1 (ACID, MIDI channel 1, 120 BPM, DIV 1/16 = 48 ticks, GATE 50 %), so the checks do not depend on what ACID generates:
 *   step 0   plain note, GATE 50 %           on 0,   off 24
 *   step 1,2 SLIDE (1 -> 2, other pitch)     on 48 and 96; step 1's off is step 2's start + L/8 = 102: the overlap (no gap)
 *   step 3,4 TIE from 3 (pitch P, then P)    on 144; off at step 4's start (192), then step 4 sounds again (it is a new note: a tie is a held gate)
 *   step 5,6 SLIDE onto the SAME pitch       ONE note-on at 240 and no note-on at 288 (no retrigger); the off follows step 6's gate (312)
 *   step 7   RATCH 3x                        three note-ons inside [336, 384) at SP5 spacing 24/16/8 (sq_ratchet with the default pace: windows 3:2:1, NOT even; ACID sets no PO_PACE_DEC), each ended before the next
 *   step 8   GATE 80 %                       off 38 ticks after the on; step 9 GATE 100 %: off 47 ticks after (strictly before the next start)
 *   step 10..15 OFF
 * A second run with SWING 60 % shows the odd steps late by 10 ticks (the even ones on the grid). HOLD is not tested: ACID never sets
 * PO_HOLD (sq_algo_acid.c), so its scheduling stays covered by tests/sq_core_test.c event golden 4 (the mock algorithm).
 * Output: the event list of two bars (stdout, compared with tests/golden/sq_acid_midi.txt by run_tests.sh). The scenario runs twice and the
 * two lists must be identical (deterministic). Run from the repo root; needs build/gen from one firmware build. */
#define UI_TEST_NO_MAIN 1
#include <stdarg.h>
#include "ui_test.c"

static int bad;
static void ck(int ok, const char *fmt, ...)
{   va_list ap;
    if (ok) return;
    bad++; fprintf(stderr, "sq_acid_midi FAIL: "); va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap); fputc('\n', stderr); }

typedef struct { double tick; uint8_t on, note, vel; } Ev;     /* tick: ticks (192 per quarter) after the first note-on */
#define EVMAX 256
static Ev evs[2][EVMAX]; static int nev[2];
static uint32_t t0_due; static int t0_set, unaligned;

static void drain(Ev *e, int *n)
{   while (mo_r != mo_w) {
        const uint32_t pkt = midi_out_q[mo_r % MQO], due = midi_out_due[mo_r % MQO], st = (pkt >> 8) & 0xFFu, d1 = (pkt >> 16) & 0x7Fu, d2 = (pkt >> 24) & 0x7Fu;
        mo_r++;
        if ((st & 0x0Fu) != 0u) continue;                                 /* CH1 only (MIDI channel 1) */
        if ((st & 0xF0u) != 0x90u && (st & 0xF0u) != 0x80u) continue;     /* notes only (not clock, not CC123) */
        if (!t0_set) { t0_due = due; t0_set = 1; }
        if ((due - t0_due) % 544u) unaligned++;
        if (*n < EVMAX) {
            const double samples = (double)(int32_t)(due - t0_due) / 544.0, spt = 44117.647 * 60.0 / (120.0 * 192.0);   /* samples per tick */
            e[*n].tick = samples / spt; e[*n].on = (uint8_t)((st & 0xF0u) == 0x90u && d2); e[*n].note = (uint8_t)d1; e[*n].vel = (uint8_t)d2; (*n)++; } } }

static void set_step(uint8_t p, int semis, uint8_t gate, uint8_t ratch, uint8_t slide)
{   StepEdit *e = &sq_ch[0].ed[p];
    memset(e, 0, sizeof *e);
    e->note = (uint8_t)(0x80u | semis); e->vel = 100;
    e->gr = (uint8_t)(gate | (ratch << 4));
    e->fl = (uint8_t)(1u | (slide << 2)); }

static void pattern(uint8_t swing)
{   uint8_t p;
    sq_boot();
    sq_ch[0].muted = 0; sq_ch[0].midi_ch = 0; sq_ch[0].len = 16;
    for (p = 0; p < 16; p++) sq_ch[0].ed[p].fl = 2u;                     /* every step OFF, then the played ones */
    set_step(0, 12, 5, 0, 0);
    set_step(1, 16, 5, 0, 1);  set_step(2, 19, 5, 0, 2);                   /* SLIDE ON then SLIDE OFF */
    set_step(3, 21, 11, 0, 2); set_step(4, 21, 5, 0, 2);                   /* GATE 11 = TIE */
    set_step(5, 23, 5, 0, 1);  set_step(6, 23, 5, 0, 2);                   /* slide onto the same pitch */
    set_step(7, 24, 5, 3, 2);                                              /* RATCH 3x */
    set_step(8, 26, 8, 0, 2);  set_step(9, 28, 10, 0, 2);                  /* GATE 80 %, 100 % */
    sq_gdb[sq_gidx].s.swing = swing; sq_gdb[0].s.swing = sq_gdb[1].s.swing = swing;
    (void)sq_rebuild(0); }

static void run(int which, uint8_t swing)
{   uint32_t i, j, n = 2u * (uint32_t)FS * 2u / CTL + 8u;                   /* 2 bars at 120 BPM = 4 s, plus a few blocks */
    nev[which] = 0; t0_set = 0; mo_r = mo_w = 0;
    pattern(swing);
    host_ticks = 1000000u; sq_tref_ok = 0; j = 0;
    transport_req = 1;
    for (i = 0; i <= n + 1u; i++) {
        if (i == n + 1u) transport_req = 2;
        if (j++ % (128u / CTL) == 0) { sq_half_begin(host_ticks); host_ticks += 128u * 544u; }   /* the audio ISR's once-per-half hook */
        events_block(CTL); drain(evs[which], &nev[which]); }
    transport_req = 0; }

static const Ev *at(int which, int idx) { return idx < nev[which] ? &evs[which][idx] : &evs[which][0]; }
static int near(double a, double b) { double d = a - b; return d > -1.2 && d < 1.2; }   /* about 1 tick = 115 samples; the sample-accurate part is the ordering below */

int main(void)
{   int i;
    ui_power_on(); usb.config = 1; mo_w = mo_r = 0;
    song.g[G_BPM] = 120; song.g[G_CLOCK] = 0;
    run(0, 50);
    run(1, 50);
    ck(nev[0] == nev[1] && !memcmp(evs[0], evs[1], sizeof(Ev) * (size_t)nev[0]), "two runs give different event lists (%d vs %d events)", nev[0], nev[1]);

    {   const Ev *e = evs[0];
        int n = nev[0];
        const uint8_t Pn[10] = { 12, 16, 19, 21, 21, 23, 23, 24, 26, 28 };
        uint8_t nt[10]; int idx[8], k;
        for (i = 0; i < 10; i++) nt[i] = sq_note(&sq_gdb[sq_gidx], Pn[i]);
        /* bar one, in order: 0 on, 0 off, 1 on, 2 on, 1 off, 2 off, 3 on, 3 off, 4 on, 4 off, 5 on, 6 off (no 6 on), 7 on x3 with offs, 8 on/off, 9 on/off */
        ck(n >= 2 * 20, "too few events: %d", n);
        /* step 0: on at 0, off at 24 */
        ck(e[0].on && e[0].note == nt[0] && near(e[0].tick, 0), "step 0 on at 0 (got %.2f)", e[0].tick);
        ck(!e[1].on && e[1].note == nt[0] && near(e[1].tick, 24), "step 0 off at 24 = GATE 50 %% (got %.2f)", e[1].tick);
        /* slide: step 2's on precedes step 1's off, which is 6 ticks (L/8) after step 2's start */
        ck(e[2].on && e[2].note == nt[1] && near(e[2].tick, 48), "step 1 on at 48 (got %.2f)", e[2].tick);
        ck(e[3].on && e[3].note == nt[2] && near(e[3].tick, 96), "step 2 on at 96 (got %.2f)", e[3].tick);
        ck(!e[4].on && e[4].note == nt[1] && near(e[4].tick, 102) && e[4].tick > e[3].tick, "slide overlap: step 1 off at 102 after step 2 on (on %.2f, off %.2f)", e[3].tick, e[4].tick);
        ck(!e[5].on && e[5].note == nt[2] && near(e[5].tick, 120), "step 2 off at 120 (got %.2f)", e[5].tick);
        /* tie from step 3: held to the start of step 4 (192), not 50 % */
        ck(e[6].on && e[6].note == nt[3] && near(e[6].tick, 144), "step 3 on at 144 (got %.2f)", e[6].tick);
        ck(!e[7].on && e[7].note == nt[3] && near(e[7].tick, 192), "TIE: step 3 held to 192 (got %.2f), not the 24-tick gate", e[7].tick);
        ck(e[8].on && near(e[8].tick, 192) && e[8].tick >= e[7].tick, "the step after a TIE starts at 192 after the off");
        ck(!e[9].on && near(e[9].tick, 216), "step 4 off at 216 (got %.2f)", e[9].tick);
        /* slide onto the same pitch: one on, no retrigger at 288, off with step 6's gate */
        ck(e[10].on && e[10].note == nt[5] && near(e[10].tick, 240), "step 5 on at 240 (got %.2f)", e[10].tick);
        ck(!e[11].on && e[11].note == nt[6] && near(e[11].tick, 312), "slide+same pitch: no retrigger; the one off at 312 (got %.2f, on=%d)", e[11].tick, e[11].on);
        /* ratchet 3x inside step 7 */
        for (k = 0; k < 3; k++) idx[k] = 12 + 2 * k;
        for (k = 0; k < 3; k++) ck(e[idx[k]].on && !e[idx[k] + 1].on && e[idx[k]].tick >= 336 - 1.2 && e[idx[k] + 1].tick < 384 && e[idx[k]].tick < e[idx[k] + 1].tick, "ratchet repeat %d: on then off inside step 7 (%.2f, %.2f)", k, e[idx[k]].tick, e[idx[k] + 1].tick);
        ck(near(e[12].tick, 336), "ratchet: the first repeat is on the step start (got %.2f)", e[12].tick);
        ck(near(e[14].tick - e[12].tick, 24) && near(e[16].tick - e[14].tick, 16) && near(e[17].tick, 380) && near(48 - (e[16].tick - 336), 8), "ratchet: SP5 spacing 24 / 16 / 8 ticks (weights 3:2:1 of the 48-tick step; the default pace is not even) (%.2f, %.2f)", e[14].tick - e[12].tick, e[16].tick - e[14].tick);
        ck(e[14].tick >= e[13].tick && e[16].tick >= e[15].tick, "ratchet: each repeat ends before the next starts");
        ck(e[12].note == nt[7] && e[14].note == nt[7] && e[16].note == nt[7], "ratchet: same pitch on every repeat (ACID has no sub_note)");
        /* gate lengths: step 8 GATE 80 %, step 9 100 % */
        ck(e[18].on && near(e[18].tick, 384) && !e[19].on && near(e[19].tick - e[18].tick, 38), "step 8: GATE 80 %% = 38 ticks (on %.2f off %.2f)", e[18].tick, e[19].tick);
        ck(e[20].on && near(e[20].tick, 432) && !e[21].on && near(e[21].tick - e[20].tick, 47), "step 9: GATE 100 %% ends 1 tick before the next start (on %.2f off %.2f)", e[20].tick, e[21].tick);
        /* bar two repeats bar one 768 ticks later (no PROB, no random edit) */
        ck(e[22].on && near(e[22].tick, 768), "bar two starts at 768 (got %.2f)", e[22].tick);
        for (i = 0; i < 22 && 22 + i < n; i++) ck(e[22 + i].on == e[i].on && e[22 + i].note == e[i].note && near(e[22 + i].tick - e[i].tick, 768), "bar two event %d repeats bar one (+768): %.2f vs %.2f", i, e[22 + i].tick, e[i].tick);
        /* nothing hangs: ons and offs balance per pitch, in order (STOP sent the rest) */
        {   int held[128] = { 0 }, maxh = 0, h = 0;
            for (i = 0; i < n; i++) { if (e[i].on) { held[e[i].note]++; h++; } else { held[e[i].note]--; h--; ck(held[e[i].note] >= 0, "off without on at event %d", i); } if (h > maxh) maxh = h; }
            for (i = 0; i < 128; i++) ck(held[i] == 0, "note %d left hanging", i);
            ck(maxh <= 2, "never more than two notes sound together (slide overlap) (%d)", maxh); }
        /* sample accuracy: every stamp is a whole number of samples (544 TIMER4 ticks) from the first */
        ck(!unaligned, "%d stamps are not a whole number of samples (544 TIMER4 ticks) from the first", unaligned);
    }

    /* swing 60 %: odd steps start late by (48 * 10 * 2 + 50) / 100 = 10 ticks, even steps on the grid */
    run(0, 60);
    {   const Ev *e = evs[0];
        ck(near(e[0].tick, 0), "swing: step 0 on the grid (got %.2f)", e[0].tick);
        ck(e[2].on && near(e[2].tick, 48 + 10), "swing 60 %%: odd step 1 on at 58 (got %.2f)", e[2].tick);
        ck(e[3].on && near(e[3].tick, 96), "swing 60 %%: even step 2 on the grid, 96 (got %.2f)", e[3].tick);
        ck(e[6].on && near(e[6].tick, 144 + 10), "swing 60 %%: odd step 3 on at 154 (got %.2f)", e[6].tick); }

    /* the golden: the list of two bars at swing 50 (second run of the scenario) */
    for (i = 0; i < nev[1]; i++) printf("%7.2f %s %3d %3d\n", evs[1][i].tick, evs[1][i].on ? "ON " : "OFF", evs[1][i].note, evs[1][i].vel);
    if (bad) fprintf(stderr, "sq_acid_midi: %d failure(s)\n", bad);
    return bad ? 1 : 0; }
