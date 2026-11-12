/* SPDX-License-Identifier: GPL-3.0-only */
/* clk_ext.c: the clock source model, the PLL and Start / Continue / Song Position when following (P4-009; TIMING_SPEC M11, M12
 * and section 4, rules R1..R16, as amended by the owner's AUTO rulings of 2026-11-10).
 *
 * What it is: a pure state machine. Its inputs are MIDI realtime / SPP events with their stamps in TIMER4 ticks (usb.c
 * midi_in_t[], stamped at the receiving poll), the panel's PLAY / STOP presses and a poll with the current time. Its outputs are
 * the CLKX_A_* action bits each call returns (the ISR owner acts on them) and the state in `clkx` (the fields SCREEN_SPEC 6.1
 * reads). It touches no register, no sequencer state and no ring, so it is the same code on the device and in tests/clock_test.c.
 *
 * Wired at the cut-over (P4-012) as far as the SOURCE goes: seq.c events_block feeds it every realtime byte and SPP (clkx_event), polls it
 * once per block (clkx_poll), sets internal_only from the CLOCK setting, and SQ_IS_MASTER() is clkx_master() (felucca.c), so clock out is silent
 * whenever a host clock is followed. The transport and the tempo of a followed clock still come from midi_clock.c (its pulse counter and
 * beat_samples); the PLL below (clkx_beat_q4, clkx_tick_time) and the CLKX_A_* actions are NOT used by the core yet: see the P4-012 Log. The plan was:
 *   - feed it every midi_in_q packet >= 0xF8 and 0xF2 with midi_in_t[] and the source (1 USB, 2 TRS),
 *   - call clkx_poll(fm1_ticks()) once per block and clkx_play() / clkx_stop() for the panel,
 *   - act on the bits: STOP = sq_on_stop(), START = sq_on_start() with tick 0 at this pulse, CONT = continue at clkx.pulse_idx * 8,
 *   - take the beat from clkx_beat_q4() (0 = use the user's tempo) and the stamp of tick 8k+j from clkx_tick_time(j),
 *   - define SQ_IS_MASTER() as clkx_master() so clock out is silent in every EXT state (R12).
 *
 * Units: times are TIMER4 ticks (24 per microsecond, uint32 wraps every 179 s, so every difference is a signed 32-bit one); the
 * period P is in 1/256 tick; one MIDI pulse is 8 ticks of the 192-PPQN grid.
 *
 * Findings worth the reviewer's eye (tests/clock_test.c prints the numbers):
 *   - With the M11 gains (phase 1/4, frequency 1/64) the loop's P lags a steady tempo ramp by 16 pulses of ramp (0.67 BPM for
 *     1 BPM per beat), above T13's 0.3 BPM. The period the loop actually advances its phase by, P + e/4 (`clkx.peff`), has no such
 *     lag, and it is what clkx_tick_time() and clkx_beat_q4() use; T13 is asserted on that, and the lag of P is printed.
 *   - A gap over 1.5 P is credited to the loop's phase (the lost pulses), never to the position, which only counts accepted 0xF8
 *     (TIMING_SPEC 4.3); a host that really drops a pulse therefore shifts the music by that pulse until its next Start. */

#define CLKX_TIMEOUT_MS     500u                         /* CLK_TIMEOUT_MS (flex row, R5) */
#define CLKX_TICKS_PER_MS   24000u
#define CLKX_TIMEOUT        (CLKX_TIMEOUT_MS * CLKX_TICKS_PER_MS)
#define CLKX_P_MIN          250000u                      /* ticks per pulse at 240 BPM (60e6 / bpm) */
#define CLKX_P_MAX          1500000u                     /* ... at 40 BPM */
#define CLKX_ACQ            7u                           /* pulses for the first tempo (6 intervals) */
#ifndef CLKX_LEAD_TICKS
#define CLKX_LEAD_TICKS     69632u                       /* M2 LEAD: 128 samples * 544 ticks (sq_core uses MO_LEAD_SAMPLES) */
#endif
#define CLKX_G_PH           2u                           /* phase gain 1/4 = e >> 2 (flex row) */
#define CLKX_G_FR           6u                           /* frequency gain 1/64 = e >> 6 */
#define CLKX_G_DISP         4u                           /* display smoothing 1/16 */

#define CLKX_INT 0u
#define CLKX_EXT 1u

#define CLKX_A_STOP       0x01u   /* the transport stops and every sounding note is released (sq_on_stop) */
#define CLKX_A_START      0x02u   /* step 0 begins at THIS pulse (clkx.pt is its filtered time) */
#define CLKX_A_CONT       0x04u   /* continue from clkx.pulse_idx * 8 at this pulse */
#define CLKX_A_PULSE      0x08u   /* an accepted pulse while the transport follows (clkx.pulse_idx advanced) */
#define CLKX_A_LOST_PLAY  0x10u   /* strip: CLOCK LOST: STOPPED */
#define CLKX_A_LOST_ARM   0x20u   /* strip: CLOCK LOST: PLAY TO START */
#define CLKX_A_LOCAL      0x40u   /* clkx_play(): start the internal clock now */

static struct {
    /* the screen reads these (SCREEN_SPEC 6.1) */
    uint8_t  src;                 /* clk_src: CLKX_INT / CLKX_EXT */
    uint8_t  port;                /* clk_port: 1 USB, 2 TRS (0 while INT) */
    uint8_t  known;               /* clk_known: at least 7 pulses measured */
    uint8_t  armed;               /* clk_armed: PLAY pressed while EXT, waiting for the host start (WAIT) */
    uint8_t  pulse_n;             /* clk_pulse_n: free-running count of accepted pulses */
    uint8_t  playing;             /* the transport as the header shows it (EXT: from the Start message, not from its first pulse) */
    uint16_t bpm;                 /* clk_ext_bpm: smoothed, integer, +-1 hysteresis, 0 until known */
    /* the loop and the position */
    uint8_t  live;                /* pulses advance the position (the first pulse after a Start / Continue has arrived) */
    uint8_t  pend;                /* 1 Start pending, 2 Continue pending: begins at the next accepted pulse */
    uint8_t  n;                   /* pulses of the first estimate so far (saturates at CLKX_ACQ) */
    uint8_t  off_n;               /* consecutive pulses whose interval was off the expected one (R14) */
    uint8_t  internal_only;       /* CLOCK = INTERNAL ONLY: no clock input is ever accepted (question 1) */
    uint32_t last;                /* clk_last: the stamp of the last accepted pulse (the silence timer) */
    uint32_t seen;                /* the stamp of the last pulse heard on the port, accepted or not (the intervals) */
    uint32_t first;               /* stamp of the first pulse of the first estimate */
    uint32_t off_dt;              /* the first off interval, ticks */
    uint32_t pend_t;              /* stamp of a Start latched while INT (R16) */
    uint32_t P;                   /* clk_ext_p: the loop's period, 1/256 tick; 0 when no clock is held */
    uint32_t peff;                /* P + e/4: the period the phase advances by, 1/256 tick */
    uint32_t ph_t;                /* expected time of the NEXT pulse, ticks ... */
    uint8_t  ph_f;                /* ... and its 1/256 fraction */
    uint32_t pt;                  /* the filtered time of the last accepted pulse, ticks */
    uint32_t disp_p;              /* smoothed period for the display, 1/256 tick */
    uint32_t disp_t;              /* stamp of the last display update */
    uint32_t pulse_idx;           /* index of the last pulse of the run: tick = 8 * pulse_idx */
    uint32_t next_idx;            /* the index the next pulse takes when the run begins or goes on (Start 0, SPP spp * 6, held on Stop) */
} clkx;

static void clkx_ph_add(int32_t q8)                      /* phase += q8 / 256 ticks, exact */
{   int32_t t = q8 >> 8, f = (int32_t)clkx.ph_f + (q8 & 255);
    if (f >= 256) { f -= 256; t++; }
    clkx.ph_t += (uint32_t)t; clkx.ph_f = (uint8_t)f; }

static void clkx_to_int(void)                            /* R5: back to the user's tempo; the position is kept */
{   clkx.src = CLKX_INT; clkx.port = 0; clkx.known = 0; clkx.armed = 0; clkx.playing = 0; clkx.live = 0; clkx.pend = 0;
    clkx.n = 0; clkx.off_n = 0; clkx.P = 0; clkx.peff = 0; clkx.bpm = 0; }

static void clkx_adopt(uint8_t port, uint32_t a)         /* R2 / R4: a host clock is accepted; a new estimate begins */
{   clkx.src = CLKX_EXT; clkx.port = port; clkx.known = 0; clkx.n = 0; clkx.off_n = 0;
    clkx.P = 0; clkx.peff = 0; clkx.bpm = 0; clkx.last = clkx.seen = a; }

static uint8_t clkx_in_range(uint32_t pq8)
{   return pq8 >= (CLKX_P_MIN << 8) && pq8 <= (CLKX_P_MAX << 8); }

/* tempo for the screen: smoothed 1/16 per pulse, integer, +-1 hysteresis, at most 4 updates a second */
static void clkx_display(uint32_t a)
{   int32_t d = (int32_t)clkx.peff - (int32_t)clkx.disp_p;
    uint32_t pt, b10;
    clkx.disp_p = (uint32_t)((int32_t)clkx.disp_p + (d >= 0 ? d >> CLKX_G_DISP : -((-d) >> CLKX_G_DISP)));
    pt = clkx.disp_p >> 8;
    if (!pt) return;
    b10 = (600000000u + pt / 2u) / pt;                   /* tenths of a BPM (60e6 ticks per minute per pulse-per-beat * 10) */
    if (!clkx.bpm) { clkx.bpm = (uint16_t)((b10 + 5u) / 10u); clkx.disp_t = a; return; }
    if ((uint32_t)(a - clkx.disp_t) < 6000000u) return;     /* 250 ms */
    {   int32_t dv = (int32_t)b10 - (int32_t)clkx.bpm * 10;
        if (dv >= 10 || dv <= -10) { clkx.bpm = (uint16_t)((b10 + 5u) / 10u); clkx.disp_t = a; } } }

/* One accepted-port 0xF8. Returns the action bits. */
static uint8_t clkx_pulse(uint32_t a)
{   uint8_t act = 0;
    uint32_t dt = a - clkx.seen;
    clkx.seen = a;
    if (!clkx.known) {
        /* the first estimate: elapsed time over the pulses counted (a burst shares a stamp but still counts), 7 pulses */
        if (!clkx.n) { clkx.first = a; clkx.n = 1; }
        else {
            uint32_t span;
            if (dt > CLKX_P_MAX + CLKX_P_MAX / 2u) { clkx_to_int(); return 0; }       /* R13: slower than 40 BPM is not a clock */
            clkx.n++;
            span = a - clkx.first;
            clkx.P = clkx.peff = (span << 8) / (clkx.n - 1u);                   /* provisional from the 2nd pulse (span <= 6 * 2.25 M, << 8 fits) */
            if (clkx.n >= CLKX_ACQ) {
                if (!clkx_in_range(clkx.P)) {                                   /* R13: not 40..240 BPM: not a clock, never stuck in EXT -- */
                    clkx_to_int();
                    return 0;
                }
                clkx.known = 1; clkx.ph_t = a; clkx.ph_f = 0; clkx_ph_add((int32_t)clkx.P);
                clkx.pt = a; clkx.disp_p = clkx.P; clkx.bpm = 0; clkx_display(a);
            }
        }
        if (!clkx.known) clkx.pt = a;
    } else {
        uint32_t Pt = clkx.P >> 8;
        int32_t e;
        if (dt < Pt / 4u) goto counted;                                         /* a burst: counted for position, out of the loop */
        if (dt > Pt + Pt / 2u || dt < Pt * 3u / 4u || dt > Pt * 5u / 4u) {      /* off the expected interval: a lost pulse, a jump or a glitch */
            uint8_t gap = (uint8_t)(dt > Pt + Pt / 2u);
            if (clkx.off_n && (dt > clkx.off_dt ? dt - clkx.off_dt : clkx.off_dt - dt) <= clkx.off_dt / 4u) {
                uint32_t np = (((dt + clkx.off_dt) >> 1) << 8);               /* R14: two agreeing intervals re-seed P; the position is never touched */
                if (!clkx_in_range(np)) { return 0; }                          /* R13: not accepted (clk_last stays, R5 / R6 follow) */
                clkx.P = clkx.peff = np; clkx.ph_t = a; clkx.ph_f = 0; clkx_ph_add((int32_t)np);
                clkx.pt = a; clkx.off_n = 0;
                goto counted;
            }
            clkx.off_n = 1; clkx.off_dt = dt;
            if (gap) {                                                         /* the lost pulses move the phase, not the count (4.3) */
                uint32_t k = (dt + Pt / 2u) / Pt;
                {   uint32_t i; for (i = 1u; i < k; i++) clkx_ph_add((int32_t)clkx.P); }
                goto update;
            }
            clkx_ph_add((int32_t)clkx.P);                                      /* an outlier: expect the next one a period on, learn nothing */
            clkx.pt = a;
            goto counted;
        }
        clkx.off_n = 0;
update:
        e = (int32_t)(a - clkx.ph_t);
        if (e > (int32_t)(Pt / 2u)) e = (int32_t)(Pt / 2u);
        if (e < -(int32_t)(Pt / 2u)) e = -(int32_t)(Pt / 2u);
        clkx.pt = clkx.ph_t + (uint32_t)(e >> CLKX_G_PH);                       /* where the loop puts THIS pulse */
        clkx.peff = clkx.P + (uint32_t)(e * 64);                                /* e/4 in 1/256 tick */
        clkx_ph_add((int32_t)clkx.peff);
        {   int32_t p = (int32_t)clkx.P + e * 4;                                /* e/64 in 1/256 tick */
            if (p < (int32_t)(CLKX_P_MIN << 8)) p = (int32_t)(CLKX_P_MIN << 8);
            if (p > (int32_t)(CLKX_P_MAX << 8)) p = (int32_t)(CLKX_P_MAX << 8);
            clkx.P = (uint32_t)p; }
        clkx_display(a);
    }
counted:
    clkx.last = a;
    clkx.pulse_n++;
    if (clkx.pend) {                                                           /* M12: the run begins at the first pulse after the message */
        act |= clkx.pend == 1u ? CLKX_A_START : CLKX_A_CONT;
        clkx.pend = 0; clkx.live = 1; clkx.playing = 1;
    }
    if (clkx.live) { clkx.pulse_idx = clkx.next_idx; clkx.next_idx++; act |= CLKX_A_PULSE; }
    return act; }

/* Silence and latched starts (R5, R6, R9, R16). Cheap: call it once per block with fm1_ticks(). */
static uint8_t clkx_poll(uint32_t now)
{   uint8_t act = 0;
    if (clkx.src == CLKX_EXT) {
        if ((uint32_t)(now - clkx.last) > CLKX_TIMEOUT) {
            if (clkx.playing) act |= CLKX_A_STOP | CLKX_A_LOST_PLAY;           /* R6: stop, never continue on the internal clock */
            if (clkx.armed) act |= CLKX_A_LOST_ARM;                            /* R9 */
            clkx_to_int();
        }
    } else if (clkx.pend && (uint32_t)(now - clkx.pend_t) > CLKX_TIMEOUT)
        clkx.pend = 0;                                                         /* R16: no pulse followed the Start */
    return act; }

/* A realtime byte or SPP from the port (1 USB, 2 TRS) at stamp a. status 0xF8 pulse, 0xFA Start, 0xFB Continue, 0xFC Stop, 0xF2 SPP. */
static uint8_t clkx_event(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2, uint32_t a)
{   uint8_t act;
    if (clkx.internal_only) return 0;
    act = clkx_poll(a);
    if (status == 0xF8u) {
        if (clkx.src == CLKX_INT) {
            if (clkx.playing) return act;                                       /* R3: may be our own clock looped back */
            clkx_adopt(port, a);                                                /* R2 (a latched Start, R16, stays pending) */
        } else if (port != clkx.port) return act;                               /* R15 */
        act |= clkx_pulse(a);
    } else if (status == 0xFAu || status == 0xFBu) {
        if (clkx.src == CLKX_INT) {
            if (clkx.playing) {                                                 /* R4: the host takes over */
                act |= CLKX_A_STOP;
                clkx_adopt(port, a);
                clkx.playing = 1;
            } else { clkx.pend = status == 0xFAu ? 1u : 2u; clkx.pend_t = a; if (status == 0xFAu) clkx.next_idx = 0; return act; }   /* R16 */
        } else if (port != clkx.port) return act;
        else if (status == 0xFBu && clkx.live) return act;                      /* already running */
        if (status == 0xFAu) clkx.next_idx = 0;
        clkx.pend = status == 0xFAu ? 1u : 2u; clkx.live = 0; clkx.playing = 1; clkx.armed = 0;
    } else if (status == 0xFCu) {
        if (clkx.src == CLKX_EXT && port == clkx.port) {                        /* R7: stop; the source stays EXT */
            if (clkx.playing) act |= CLKX_A_STOP;
            clkx.playing = 0; clkx.live = 0; clkx.pend = 0;
        } else if (clkx.src == CLKX_INT && !clkx.playing) clkx.pend = 0;
    } else if (status == 0xF2u) {                                               /* M12: only while stopped */
        if (!clkx.playing && (clkx.src == CLKX_INT || port == clkx.port))
            clkx.next_idx = ((uint32_t)(d1 & 0x7Fu) | (uint32_t)(d2 & 0x7Fu) << 7) * 6u;
    }
    return act; }

/* The panel. PLAY: INT starts at once as master; EXT arms (WAIT) or, armed, cancels (R8); never a silent fall back to internal. */
static uint8_t clkx_play(void)
{   if (clkx.src == CLKX_INT) { clkx.playing = 1; clkx.pend = 0; return CLKX_A_LOCAL; }
    if (clkx.playing) return 0;
    clkx.armed ^= 1u;
    return 0; }

static uint8_t clkx_stop(void)                                                  /* R10: a local stop; the source stays */
{   uint8_t act = clkx.playing || clkx.armed ? CLKX_A_STOP : 0;
    clkx.playing = 0; clkx.live = 0; clkx.pend = 0; clkx.armed = 0;
    return act; }

static uint8_t clkx_set_internal_only(uint8_t on)                               /* the CLOCK row: AUTO (0) / INTERNAL ONLY (1) */
{   uint8_t act = 0;
    clkx.internal_only = on;
    if (on && clkx.src == CLKX_EXT) { if (clkx.playing) act = CLKX_A_STOP; clkx_to_int(); }
    return act; }

/* Clock out is sent only while this unit is the tempo master: the source is INT (the core also checks that it plays). R12. */
static uint8_t clkx_master(void) { return clkx.src == CLKX_INT; }

/* The time of tick 8k + j (j = 0..7) after the last accepted pulse k, stamped for the wire: filtered pulse time + j * the effective
 * period / 8 + LEAD (M11 step 4). Valid from the first accepted pulse of a known clock. */
static uint32_t clkx_tick_time(uint32_t j)
{   return clkx.pt + ((clkx.peff * j) >> 11) + CLKX_LEAD_TICKS; }

/* Samples per beat in 1/16 samples (sq_beat_q4's unit): 24 pulses of peff ticks over 544 ticks per sample, 0 when not following. */
static uint32_t clkx_beat_q4(void)
{   if (clkx.src != CLKX_EXT || !clkx.known) return 0u;
    return (clkx.peff * 3u + 544u) / 1088u; }
