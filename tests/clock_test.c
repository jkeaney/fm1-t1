/* SPDX-License-Identifier: GPL-3.0-only */
/* clock_test.c (P4-009): TIMING_SPEC 6.3 host rows for following an external clock: PLL (M11), source rules R1..R16 (section 4),
 * Start / Continue / SPP (M12), and clock out silent in every EXT state (R12).
 *   cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -fno-strict-aliasing -fsanitize=address,undefined -Ifirmware/src \
 *      -o /tmp/clock_test tests/clock_test.c -lm && /tmp/clock_test            (from the repo root)
 *
 * The REAL clk_ext.c, usb.c (the out ring and the SPP / realtime forwarding of the in ring) and sq_core.c are one translation unit. Time
 * is a variable the test advances (fm1_ticks, 24 ticks per microsecond). The stamp noise of the PLL rows is a uniform +-0.5 ms (the
 * 0.5 ms poll of the receiving side, M11 step 1) from a fixed seed, so the rows are repeatable. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define RING_PUBLISH() __asm__ volatile("" ::: "memory")
#define HALF_FRAMES 128
#define FELUCCA_OTA 0
#define FELUCCA_CDC 0
#define FELUCCA_UAC 0
#define FELUCCA_USB_LAYOUT 0
#define FELUCCA_CDC_DEFAULT 0
#define FELUCCA_UAC_48K 0
#define FELUCCA_UAC_AS_FIRST 0
static void fm1_delay_ms(uint32_t ms) { (void)ms; }
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#define FM1_HOST_SIE 1
#define FM1_TICKS_PER_US 24u
static uint32_t fake_ticks;
static uint32_t fm1_ticks(void) { return fake_ticks; }
static int  ep1_tx_ready(void);
static void ep1_tx_arm(uint32_t n);
#include "../firmware/src/usb.c"
#include "../firmware/src/clk_ext.c"

#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"

typedef struct { int unused; } track_t;
static track_t trk[SQ_NCH];
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel) { (void)t; (void)note; (void)vel; }
static void trk_note_off(track_t *t, uint32_t note) { (void)t; (void)note; }
#define SQ_EXT_BEAT() 0u
#define SQ_BPM()      120u
#define SQ_IS_MASTER() clkx_master()                  /* what the cut-over defines (R12) */

static struct { long clk, fa, fb, fc, spp, ons; } cap;
static void t_cap_at(uint32_t pkt, uint32_t due)
{   unsigned st = (pkt >> 8) & 0xFFu, d2 = pkt >> 24;
    if (st == 0xF8u) cap.clk++; else if (st == 0xFAu) cap.fa++; else if (st == 0xFBu) cap.fb++; else if (st == 0xFCu) cap.fc++;
    else if (st == 0xF2u) cap.spp++; else if ((st & 0xF0u) == 0x90u && d2) cap.ons++;
    midi_out_event_at(pkt, due); }
#define midi_out_event_at t_cap_at

typedef struct { uint8_t n, pad; PulseOut s[9]; } Tbl;
static uint8_t tbl_len(const void *k) { return ((const Tbl *)k)->n; }
static void    tbl_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o)
{   const Tbl *t = (const Tbl *)k; (void)hc; *o = t->s[g % t->n]; }
static uint8_t tbl_sub(const void *k, uint32_t g, uint8_t kk, const HarmChord *hc)
{   const Tbl *t = (const Tbl *)k; (void)hc; return (uint8_t)(t->s[g % t->n].note + 3u * kk); }
static void    tbl_nobuild(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k)
{   (void)m; (void)seed; (void)g; (void)k; }
#define MK(i, nm, fl) [i] = { .name = nm, .id = i, .n_seed = 1, .flags = fl, .cache_bytes = sizeof(Tbl), .build = tbl_nobuild, \
                              .loop_len = tbl_len, .step = tbl_step, .sub_note = tbl_sub }
static const AlgoDesc ALGOS[SQ_NALGO] = { MK(0, "T", 0), MK(1, "T", 0) };

#include "sq_core.c"

static int  ep1_tx_ready(void) { return 0; }                  /* (the ring is emptied by the test: the release is timing_test's row) */
static void ep1_tx_arm(uint32_t n) { (void)n; }

static int fails;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

static uint32_t rs = 0x2545F491u;
static uint32_t rnd(uint32_t n) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs % n; }

#define USB 1u
#define TRS 2u
#define MS  24000u
#define T0  5000000u
static uint32_t now_t;                               /* the test's clock, TIMER4 ticks */

static void clk_reset(void) { memset(&clkx, 0, sizeof clkx); now_t = T0; fake_ticks = T0; }
static uint8_t ev(uint8_t port, uint8_t st) { return clkx_event(port, st, 0, 0, now_t); }
static uint8_t evat(uint8_t port, uint8_t st, uint32_t t) { return clkx_event(port, st, 0, 0, t); }
static uint32_t period_of(double bpm) { return (uint32_t)(60e6 / bpm + 0.5); }                 /* ticks per pulse */
static double bpm_of(uint32_t q8) { return 60e6 * 256.0 / (double)q8; }

/* n pulses at period P (ticks) from the port, each stamped now_t + noise (+-noise ticks); returns the OR of the actions */
static uint8_t pulses(uint8_t port, int n, uint32_t P, int noise)
{   uint8_t act = 0;
    for (int i = 0; i < n; i++) {
        int32_t j = noise ? (int32_t)rnd(2u * (uint32_t)noise + 1u) - noise : 0;
        act |= evat(port, 0xF8u, now_t + (uint32_t)j);
        now_t += P; }
    return act; }

/* ---------------------------------------------------------------- PLL */
static void t_pll_jitter(void)
{   double bpms[] = { 60, 90, 120, 180, 240 };
    for (unsigned k = 0; k < 5; k++) {
        uint32_t P = period_of(bpms[k]);
        double s = 0, s2 = 0, mn = 1e18, mx = -1e18, serr = 0; long m = 0;
        clk_reset();
        for (int i = 0; i < 3000; i++) {
            uint32_t ideal = T0 + (uint32_t)i * P;
            int32_t j = (int32_t)rnd(24001u) - 12000;                                  /* +-0.5 ms */
            now_t = ideal;
            evat(USB, 0xF8u, ideal + (uint32_t)j);
            if (i >= 200) {
                double err = (double)(int32_t)(clkx.pt - ideal) / 24.0;                  /* us: the loop's pulse time against the true one */
                s += err; s2 += err * err; m++;
                if (err < mn) mn = err;
                if (err > mx) mx = err;
                serr += fabs(bpm_of(clkx.P) - bpms[k]);
            }
        }
        {   double mean = s / (double)m, rms = sqrt(s2 / (double)m - mean * mean);
            CHECK(clkx.known && clkx.src == CLKX_EXT, "bpm %.0f: known EXT", bpms[k]);
            CHECK(rms < 150.0, "bpm %.0f: output jitter RMS %.1f us (limit 150)", bpms[k], rms);
            CHECK(fabs(mean) < 40.0, "bpm %.0f: mean offset %.1f us", bpms[k], mean);
            CHECK(serr / (double)m < 0.35, "bpm %.0f: mean tempo error %.3f BPM", bpms[k], serr / (double)m);
            printf("  pll jitter %3.0f BPM: input +-500 us -> output RMS %.0f us, p-p %.0f us, mean tempo error %.3f BPM\n", bpms[k], rms, mx - mn, serr / (double)m); } } }

static void t_pll_burst(void)
{   uint32_t P = period_of(120);
    long errs = 0; double worst = 0;
    clk_reset();
    for (int i = 0; i < 400; i++) { now_t = T0 + (uint32_t)i * P; evat(USB, 0xF8u, now_t); }
    {   uint32_t idx0 = clkx.pulse_n, n_before = idx0;
        long fed = 0;
        for (int i = 400; i < 1400; i++) {
            uint32_t ideal = T0 + (uint32_t)i * P;
            if (i % 50 == 3) {                                                   /* pulses 3,4,5,6 share the stamp of the 4th (a USB burst) */
                for (int b = 0; b < 4; b++) { evat(USB, 0xF8u, ideal + 3u * P); fed++; }
                i += 3;
            } else { evat(USB, 0xF8u, ideal); fed++; }
            if (i % 50 >= 10 || i > 410) {
                double err = fabs((double)(int32_t)(clkx.pt - ideal) / 24.0);
                if (i % 50 > 8 && err > worst) worst = err;
                if (i % 50 > 8 && err > 200.0) errs++;
            }
        }
        CHECK((uint8_t)clkx.pulse_n == (uint8_t)(n_before + (uint32_t)fed), "burst: pulse count %u vs %u", (unsigned)clkx.pulse_n, (unsigned)(n_before + (uint32_t)fed));
        CHECK(errs == 0, "burst: %ld settled pulses off by more than 0.2 ms (worst %.0f us)", errs, worst);
        printf("  pll burst: 4 pulses on one stamp every 50, count exact (%ld fed), phase within %.0f us once settled\n", fed, worst); } }

static void t_pll_ramp(void)
{   /* 100 -> 140 BPM at 1 BPM per beat (per 24 pulses): the period the loop advances by tracks within 0.3 BPM; P itself lags (printed) */
    double worst_eff = 0, worst_p = 0, worst_disp = 0; uint32_t t = T0; clk_reset();
    for (int i = 0; i < 24 * 40; i++) {
        double bpm = 100.0 + (double)i / 24.0;
        evat(USB, 0xF8u, t);
        t += (uint32_t)(60e6 / bpm + 0.5);
        if (i > 24 * 4) {
            double te = fabs(bpm_of(clkx.peff) - bpm), tp = fabs(bpm_of(clkx.P) - bpm), td = fabs((double)clkx.bpm - bpm);
            if (te > worst_eff) worst_eff = te;
            if (tp > worst_p) worst_p = tp;
            if (td > worst_disp) worst_disp = td;
        }
    }
    CHECK(worst_eff < 0.3, "ramp: effective period off by %.3f BPM (T13 0.3)", worst_eff);
    CHECK(worst_p < 0.75, "ramp: P off by %.3f BPM", worst_p);
    printf("  pll ramp 100->140 BPM at 1 BPM/beat: effective period within %.3f BPM (T13 0.3); P lags %.2f BPM; the shown integer BPM within %.1f\n", worst_eff, worst_p, worst_disp); }

static void t_no_loss(void)
{   /* 10 minutes at 120 BPM from a started host, noisy stamps and bursts: every pulse counted, none added (T9) */
    uint32_t P = period_of(120), N = 600u * 2u * 24u;
    uint32_t cnt = 0, act_start = 0; uint8_t act;
    clk_reset();
    now_t = T0;
    ev(USB, 0xFAu);
    for (uint32_t i = 0; i < N; i++) {
        uint32_t ideal = T0 + 10u * MS + i * P;
        int32_t j = (int32_t)rnd(24001u) - 12000;
        now_t = ideal;
        if (i % 977u == 5u && i + 3u < N) { for (int b = 0; b < 4; b++) { act = evat(USB, 0xF8u, ideal + 3u * P); cnt += (act & CLKX_A_PULSE) ? 1u : 0u; act_start += (act & CLKX_A_START) ? 1u : 0u; } i += 3u; continue; }
        act = evat(USB, 0xF8u, ideal + (uint32_t)j);
        cnt += (act & CLKX_A_PULSE) ? 1u : 0u; act_start += (act & CLKX_A_START) ? 1u : 0u;
    }
    CHECK(act_start == 1, "10 min: %u starts", (unsigned)act_start);
    CHECK(clkx.pulse_idx == cnt - 1u, "10 min: position %u after %u accepted pulses", (unsigned)clkx.pulse_idx, (unsigned)cnt);
    CHECK(cnt >= N, "10 min: %u accepted for %u sent", (unsigned)cnt, (unsigned)N);
    CHECK(fabs(bpm_of(clkx.P) - 120.0) < 0.1, "10 min: tempo %.3f", bpm_of(clkx.P));
    printf("  no loss: 10 min at 120 BPM, noise and bursts: %u pulses in, %u counted, position %u, tempo %.3f BPM\n", (unsigned)N, (unsigned)cnt, (unsigned)clkx.pulse_idx, bpm_of(clkx.P)); }

/* ---------------------------------------------------------------- the rules */
#define EXPECT(cond, what) CHECK(cond, "%s: %s (src %u port %u known %u armed %u playing %u live %u pend %u)", row, what, clkx.src, clkx.port, clkx.known, clkx.armed, clkx.playing, clkx.live, clkx.pend)

static void feed_known(uint8_t port, uint32_t P) { pulses(port, 12, P, 0); }

static void t_rules(void)
{   const char *row; uint32_t P = period_of(120); uint8_t act;
    row = "R1";  clk_reset();
    EXPECT(clkx.src == CLKX_INT && clkx_master() && !clkx.playing && !clkx.known, "power-on: INT, master");
    row = "R2";  clk_reset();
    act = pulses(USB, 2, P, 0);
    EXPECT(clkx.src == CLKX_EXT && clkx.port == USB && !clkx.known && !act, "a pulse while stopped: EXT --");
    { uint8_t n0 = clkx.pulse_n; uint32_t tt = now_t; pulses(TRS, 3, P, 0); now_t = tt; EXPECT(clkx.port == USB && clkx.pulse_n == n0, "R15: the other port is ignored while this one is heard"); }
    pulses(USB, 6, P, 0);
    EXPECT(clkx.known && clkx.bpm == 120, "7 pulses: known, 120 BPM shown");
    row = "R3";   clk_reset();
    act = clkx_play();
    EXPECT(act == CLKX_A_LOCAL && clkx.playing && clkx.src == CLKX_INT, "PLAY on INT starts locally");
    act = pulses(USB, 20, P, 0);
    EXPECT(clkx.src == CLKX_INT && clkx.pulse_n == 0 && !act && clkx.n == 0, "pulses while playing as master are ignored");
    row = "R4";   clk_reset();
    clkx_play(); now_t += 100 * MS;
    act = ev(USB, 0xFAu);
    EXPECT((act & CLKX_A_STOP) && clkx.src == CLKX_EXT && clkx.playing && clkx.pend == 1 && !clkx.live, "host Start while playing as INT: stop, take over, restart pending");
    now_t += 10 * MS; act = ev(USB, 0xF8u);
    EXPECT((act & CLKX_A_START) && (act & CLKX_A_PULSE) && clkx.live && clkx.pulse_idx == 0, "...step 0 at the first pulse");
    clk_reset(); clkx_play(); act = ev(USB, 0xFBu);
    EXPECT((act & CLKX_A_STOP) && clkx.src == CLKX_EXT && clkx.pend == 2, "a bare Continue is treated the same");
    row = "R5";   clk_reset();
    pulses(USB, 12, P, 0);
    act = clkx_poll(clkx.last + 499 * MS);
    EXPECT(clkx.src == CLKX_EXT, "499 ms of silence: still EXT");
    act = clkx_poll(clkx.last + 501 * MS);
    EXPECT(clkx.src == CLKX_INT && !act && clkx.P == 0 && clkx_beat_q4() == 0, "500 ms of silence stopped: back to INT, nothing else");
    row = "R6";   clk_reset();
    ev(USB, 0xFAu); pulses(USB, 12, P, 0);
    EXPECT(clkx.live, "running on the host");
    act = clkx_poll(now_t + 600 * MS);
    EXPECT((act & CLKX_A_STOP) && (act & CLKX_A_LOST_PLAY) && !(act & CLKX_A_LOST_ARM) && clkx.src == CLKX_INT && !clkx.playing, "clock lost while playing: stop, strip, INT");
    row = "R7";   clk_reset();
    ev(USB, 0xFAu); pulses(USB, 12, P, 0);
    act = ev(USB, 0xFCu);
    EXPECT((act & CLKX_A_STOP) && clkx.src == CLKX_EXT && !clkx.playing && !clkx.live, "host Stop: stop, the source stays EXT");
    { uint8_t n0 = clkx.pulse_n; pulses(USB, 3, P, 0); EXPECT(clkx.pulse_n == (uint8_t)(n0 + 3) && clkx.src == CLKX_EXT && !clkx.live, "pulses go on without moving the position"); }
    row = "R8";   clk_reset();
    feed_known(USB, P);
    act = clkx_play();
    EXPECT(!act && clkx.armed && !clkx.playing && clkx.src == CLKX_EXT, "PLAY while EXT arms (WAIT), no start");
    act = pulses(USB, 5, P, 0);
    EXPECT(!act && clkx.armed && !clkx.playing, "no silent start while waiting");
    act = clkx_play();
    EXPECT(!clkx.armed && !act, "PLAY again cancels");
    clkx_play(); act = ev(USB, 0xFAu);
    EXPECT(!clkx.armed && clkx.pend == 1 && clkx.playing, "the host Start while armed begins the run");
    act = ev(USB, 0xF8u);
    EXPECT(act & CLKX_A_START, "...at its next pulse");
    row = "R9";   clk_reset();
    feed_known(USB, P); clkx_play();
    act = clkx_poll(now_t + 700 * MS);
    EXPECT((act & CLKX_A_LOST_ARM) && !(act & CLKX_A_LOST_PLAY) && !(act & CLKX_A_STOP) && clkx.src == CLKX_INT && !clkx.armed && !clkx.playing, "armed and the clock lost: INT, strip, nothing starts");
    row = "R10";  clk_reset();
    ev(USB, 0xFAu); pulses(USB, 12, P, 0);
    act = clkx_stop();
    EXPECT((act & CLKX_A_STOP) && clkx.src == CLKX_EXT && !clkx.playing, "local STOP while the host runs: the source stays EXT");
    pulses(USB, 4, P, 0);
    act = clkx_play();
    EXPECT(!act && clkx.armed, "the next PLAY is R8");
    row = "R11";  /* K1 inert and bpm_own untouched: clk_ext.c has no access to song.g[]; the shown tempo is clkx.bpm only */
    clk_reset(); feed_known(USB, P);
    EXPECT(clkx.bpm == 120, "the host tempo is a mirror in clkx.bpm");
    row = "R12";  /* clock out: see t_clock_out_silent */
    row = "R13";  clk_reset();
    pulses(USB, 6, period_of(300), 0);
    EXPECT(clkx.src == CLKX_EXT && !clkx.known, "300 BPM: EXT -- for the first 6 pulses");
    pulses(USB, 1, period_of(300), 0);
    EXPECT(clkx.src == CLKX_INT && !clkx.known, "the 7th pulse: not 40..240, back to INT, never stuck in EXT --");
    clk_reset(); pulses(USB, 2, period_of(20), 0);
    EXPECT(clkx.src == CLKX_INT, "20 BPM (slower than 40): INT at once");
    clk_reset(); feed_known(USB, P);
    { uint32_t last; uint8_t act2; pulses(USB, 2, period_of(300), 0); last = clkx.last; act2 = pulses(USB, 40, period_of(300), 0);
      EXPECT(clkx.src == CLKX_EXT && clkx.last == last && !act2, "a known clock that turns to 300 BPM is not accepted: clk_last does not advance");
      act = clkx_poll(now_t + 600 * MS);
      EXPECT(clkx.src == CLKX_INT, "...so R5 returns to INT after 500 ms"); }
    row = "R14";  clk_reset();
    ev(USB, 0xFAu); pulses(USB, 60, P, 0);
    { uint32_t idx = clkx.pulse_idx; uint8_t pn = clkx.pulse_n; double b0;
      pulses(USB, 1, period_of(180), 0);                                         /* (the pulse itself is on time: the interval before it was 120) */
      pulses(USB, 1, period_of(180), 0);                                         /* an interval 33 % off */
      pulses(USB, 1, period_of(180), 0);                                         /* a second, agreeing: re-seed */
      pulses(USB, 20, period_of(180), 0);
      b0 = bpm_of(clkx.P);
      EXPECT(clkx.pulse_idx == idx + 23 && (uint8_t)(clkx.pulse_n - pn) == 23, "a tempo jump over 25 % never touches the position");
      CHECK(fabs(b0 - 180.0) < 1.0, "R14: re-seeded to %.2f BPM", b0); }
    clk_reset(); ev(USB, 0xFAu); pulses(USB, 60, P, 0);
    { uint32_t idx = clkx.pulse_idx, g = now_t;                                  /* one pulse 40 % early, the train otherwise on its grid */
      evat(USB, 0xF8u, g - P * 4u / 10u); evat(USB, 0xF8u, g + P); now_t = g + 2u * P; pulses(USB, 20, P, 0);
      EXPECT(clkx.pulse_idx == idx + 22 && fabs(bpm_of(clkx.P) - 120.0) < 0.3, "a single glitch pulse is counted and does not move the tempo"); }
    row = "R15";  /* shown with R2 above; and the port may take over after silence */
    clk_reset(); feed_known(USB, P); act = clkx_poll(now_t + 600 * MS); pulses(TRS, 2, P, 0);
    EXPECT(clkx.src == CLKX_EXT && clkx.port == TRS, "after 500 ms of silence the other port wins");
    row = "R16";  clk_reset();
    act = ev(USB, 0xFAu);
    EXPECT(!act && clkx.src == CLKX_INT && !clkx.playing && clkx.pend == 1, "Start while stopped on INT: latched, nothing changes");
    now_t += 20 * MS; act = ev(USB, 0xF8u);
    EXPECT((act & CLKX_A_START) && clkx.src == CLKX_EXT && clkx.playing && clkx.pulse_idx == 0, "the first pulse makes it EXT and begins step 0");
    clk_reset(); ev(USB, 0xFAu); act = clkx_poll(now_t + 600 * MS);
    EXPECT(clkx.pend == 0 && !act, "no pulse in 500 ms: the pending start is dropped");
    clk_reset(); ev(USB, 0xFAu); ev(USB, 0xFCu);
    EXPECT(clkx.pend == 0, "a Stop cancels it");
    clk_reset(); pulses(USB, 12, P, 0); clkx_set_internal_only(1);
    EXPECT(clkx.src == CLKX_INT, "INTERNAL ONLY returns to INT");
    { uint8_t pn = clkx.pulse_n; act = pulses(USB, 12, P, 0);
    EXPECT(clkx.src == CLKX_INT && !act && clkx.pulse_n == pn, "...and ignores every clock (question 1)"); }
    printf("  rules: R1..R16 (+ INTERNAL ONLY) table of events in, state and actions out\n"); }

/* ---------------------------------------------------------------- Start, Continue, SPP */
static void t_start(void)
{   const char *row = "start"; uint32_t P = period_of(120); uint8_t act;
    clk_reset();
    pulses(USB, 12, P, 0);                                                     /* a host clock is running, stopped */
    act = ev(USB, 0xFAu);
    EXPECT(!(act & (CLKX_A_START | CLKX_A_PULSE)) && clkx.playing && !clkx.live, "Start: the header plays, step 0 does not begin at the message");
    now_t += P / 3u;
    act = ev(USB, 0xF8u); now_t += P;
    EXPECT(act == (CLKX_A_START | CLKX_A_PULSE) && clkx.pulse_idx == 0 && clkx.live, "step 0 begins at the first 0xF8 after Start");
    {   uint32_t t = clkx.pt; act = pulses(USB, 1, P, 0);                      /* the next pulses advance the position by one each */
        EXPECT(act == CLKX_A_PULSE && clkx.pulse_idx == 1 && clkx.pt != t, "then one pulse, one index"); }
    pulses(USB, 98, P, 0);
    EXPECT(clkx.pulse_idx == 99, "100 pulses: index 99");
    act = ev(USB, 0xFCu);
    EXPECT(act & CLKX_A_STOP, "Stop");
    pulses(USB, 5, P, 0);
    EXPECT(clkx.pulse_idx == 99 && clkx.next_idx == 100, "stopped: the position is held");
    act = ev(USB, 0xFBu); pulses(USB, 1, P, 0);
    EXPECT(clkx.pulse_idx == 100, "Continue resumes with the next index");
    ev(USB, 0xFCu);
    clkx_event(USB, 0xF2u, 10, 0, now_t);                                      /* SPP 10 = 160 sixteenth notes... 10 * 6 pulses */
    act = ev(USB, 0xFBu); act = pulses(USB, 1, P, 0);
    EXPECT((act & CLKX_A_CONT) && clkx.pulse_idx == 60 && clkx.pulse_idx * 8 == 10u * 48u, "SPP 10 then Continue: pulse 60, tick 480 (step 10)");
    ev(USB, 0xFCu);
    clkx_event(USB, 0xF2u, 0x7Fu, 0x7Fu, now_t);
    CHECK(clkx.next_idx == 16383u * 6u, "SPP is 14 bits, LSB first");
    act = ev(USB, 0xFAu); act = pulses(USB, 1, P, 0);
    EXPECT((act & CLKX_A_START) && clkx.pulse_idx == 0, "a Start after an SPP begins at 0");
    clkx_event(USB, 0xF2u, 5, 0, now_t);
    CHECK(clkx.next_idx == 1u, "SPP while running is ignored (next_idx %u)", (unsigned)clkx.next_idx);
    /* the real input path: usb.c forwards 0xF2 and stamps in ticks */
    memset(&usb, 0, sizeof usb); usb.config = 1; mi_r = mi_w = 0; midi_in_overflow = 0; fake_ticks = 123456u;
    midi_in_event(0x03u | 0xF2u << 8 | 44u << 16 | 2u << 24);
    midi_in_event(0x0Fu | 0xF8u << 8);
    midi_in_event(0x03u | 0xF2u << 8 | 0x90u << 16);                           /* a data byte with bit 7: refused */
    CHECK(mi_w == 2u && midi_in_q[0] == (0x03u | 0xF2u << 8 | 44u << 16 | 2u << 24) && midi_in_t[0] == 123456u && midi_in_t[1] == 123456u,
          "usb.c: SPP forwarded, stamped in ticks (w=%u t=%u)", (unsigned)mi_w, (unsigned)midi_in_t[0]);
    printf("  start: step 0 at the first pulse after Start (not at the message), Stop holds, Continue resumes, SPP + Continue lands on the right step\n"); }

/* ---------------------------------------------------------------- clock out is silent in every EXT state */
static void core_reset(void)
{   memset(sq_ch, 0, sizeof sq_ch); memset(sq_gdb, 0, sizeof sq_gdb); memset(sq_cache, 0, sizeof sq_cache);
    memset(sq_calgo, 0, sizeof sq_calgo); memset(sq_rt, 0, sizeof sq_rt); memset(sq_evq, 0, sizeof sq_evq);
    memset((void *)sq_cidx, 0, sizeof sq_cidx);
    sq_gidx = 0; sq_load_pending = 0; sq_req = 0; sq_drops = 0; sq_now = 0; sq_cur = 0; sq_acc = 0;
    sq_playing = 0; sq_off_stage = 0; sq_cc_pend = 0; sq_tref_ok = 0; sq_boff = 0; sq_tick_s = 0;
    memset(&cap, 0, sizeof cap);
    memset(&usb, 0, sizeof usb); usb.config = 1; mo_r = mo_w = 0; midi_out_drops = 0;
    sq_gdb[0].s.gate = 50; sq_gdb[0].s.div = 2; sq_gdb[0].s.swing = 50;
    sq_gbuild(&sq_gdb[0]); sq_gdb[1] = sq_gdb[0];
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        Tbl *t = (Tbl *)sq_cache[c][0];
        sq_ch[c].midi_ch = c; sq_ch[c].len = 8; sq_ch[c].algo = (uint8_t)(c & 1u);
        sq_calgo[c][0] = sq_calgo[c][1] = (uint8_t)(c & 1u);
        t->n = 8;
        for (uint8_t i = 0; i < 8; i++) { t->s[i].note = (uint8_t)(36u + i); t->s[i].vel = 90; t->s[i].gate = 60; t->s[i].flags = PO_HIT; }
    } }

static void run_core(uint32_t quarters, uint8_t req_first, uint8_t req_last)
{   uint32_t half = 0, ticks = quarters * 192u;
    sq_req = req_first;
    while (sq_now < ticks) {
        for (int b = 0; b < 4; b++) {
            if (b == 0) { sq_half_begin(T0 + half * (HALF_FRAMES * 544u)); half++; }
            sq_block(32); mo_r = mo_w;
        }
        if (half > 20000) break;
    }
    sq_req = req_last; sq_block(32); mo_r = mo_w; }

static void t_clock_out_silent(void)
{   uint32_t P = period_of(120);
    long ext_clock = 0;
    /* the positive control: INT and playing sends the clock, one Start, one Stop */
    clk_reset(); core_reset();
    run_core(4, SQ_REQ_START, SQ_REQ_STOP);
    CHECK(cap.clk >= 96 && cap.fa == 1 && cap.fc == 1, "INT playing: clock %ld Start %ld Stop %ld", cap.clk, cap.fa, cap.fc);
    CHECK(cap.ons > 0, "INT: notes play");
    for (int st = 0; st < 5; st++) {
        const char *name = "?";
        clk_reset(); core_reset();
        if (st == 0) { name = "EXT --"; pulses(USB, 3, P, 0); }
        if (st == 1) { name = "EXT stopped, known"; pulses(USB, 12, P, 0); }
        if (st == 2) { name = "WAIT"; pulses(USB, 12, P, 0); clkx_play(); }
        if (st == 3) { name = "EXT playing"; ev(USB, 0xFAu); pulses(USB, 12, P, 0); }
        if (st == 4) { name = "R4 take-over"; clkx_play(); ev(USB, 0xFAu); }
        CHECK(clkx.src == CLKX_EXT && !clkx_master(), "%s: not the master", name);
        run_core(4, SQ_REQ_START, SQ_REQ_STOP);
        clkx_event(USB, 0xFCu, 0, 0, now_t);
        sq_clock_continue(480);                                                  /* even a continue sends nothing */
        mo_r = mo_w;
        ext_clock += cap.clk + cap.fa + cap.fb + cap.fc + cap.spp;
        CHECK(cap.clk == 0 && cap.fa == 0 && cap.fb == 0 && cap.fc == 0 && cap.spp == 0, "%s: clock out must be silent (clk %ld fa %ld fb %ld fc %ld spp %ld)", name, cap.clk, cap.fa, cap.fb, cap.fc, cap.spp);
        CHECK(cap.ons > 0, "%s: the notes still play", name);
    }
    /* back on INT after the host goes away: the clock returns */
    clk_reset(); core_reset(); pulses(USB, 12, P, 0); clkx_poll(now_t + 600 * MS);
    run_core(2, SQ_REQ_START, SQ_REQ_STOP);
    CHECK(cap.clk >= 48, "back on INT after R5: clock out again (%ld)", cap.clk);
    printf("  clock out silent: 0 of any F8/FA/FB/FC/F2 in EXT --, EXT stopped, WAIT, EXT playing and a take-over (%ld); INT sends it\n", ext_clock); }

int main(void)
{   t_pll_jitter();
    t_pll_burst();
    t_pll_ramp();
    t_no_loss();
    t_rules();
    t_start();
    t_clock_out_silent();
    if (fails) { printf("CLOCK FAIL (%d)\n", fails); return 1; }
    printf("CLOCK PASS\n");
    return 0; }
