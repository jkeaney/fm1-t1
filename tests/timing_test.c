/* SPDX-License-Identifier: GPL-3.0-only */
/* timing_test.c (P4-007): TIMING_SPEC 6.3 host rows for the stamped release ring and the exact tick position.
 *   cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -fno-strict-aliasing -fsanitize=address,undefined -Ifirmware/src \
 *      -o /tmp/timing_test tests/timing_test.c && /tmp/timing_test            (run from the repo root; also with -DFELUCCA_TX_NESTED=0)
 *
 * The REAL usb.c (the out ring, midi_out_event_at, midi_tx_release) and the REAL sq_core.c are in one translation unit, as in the
 * firmware. What is faked: time (fm1_ticks is a variable the test advances) and the two SIE steps of ep1_tx (ep1_tx_ready /
 * ep1_tx_arm: usb.c's FM1_HOST_SIE hook), so no register is touched. sq_core's calls to midi_out_event_at go through a tap
 * that checks and logs the raw (packet, due) before handing it to the real ring.
 *
 * Rows: tick position (M1), stamp (M2), ordering (M4), clock out count (M7), release (M3, M5), tempo constant (M10), and the
 * M3 fallback build (-DFELUCCA_TX_NESTED=0 passes the same rows; the nested call then does nothing). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"

typedef struct { int unused; } track_t;
static track_t trk[SQ_NCH];
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel) { (void)t; (void)note; (void)vel; }
static void trk_note_off(track_t *t, uint32_t note) { (void)t; (void)note; }

static uint32_t mock_ext_beat;                       /* 0 = internal tempo */
static uint32_t mock_bpm = 120;
#define SQ_EXT_BEAT() mock_ext_beat
#define SQ_BPM()      mock_bpm

static uint32_t sq_cur;                              /* (sq_core.c's own: the tick of the call in progress; declared early for the tap) */
/* ---- the tap: every packet sq_core sends is checked, then handed to the real ring ---- */
static struct {
    int      check;                                  /* the invariants below are live (runs that only play, no stop / panic stamps) */
    uint32_t last_due; long n, back;                 /* stamps going backwards */
    long     clk, fa, fb, fc, spp, ons, offs;
    uint32_t tid, tdue; int tvalid, trank; long eq_bad, rank_bad;
    int      cnt[16][128]; long wire_bad;            /* a note on twice, or an off without its on */
    uint32_t first_f8_due, last_f8_due;
} cap;
static int rank_of(uint32_t pkt)
{   unsigned st = (pkt >> 8) & 0xFFu, d2 = pkt >> 24;
    if (st == 0xF8u || st == 0xFAu) return 0;                    /* tick 0: Start, then the first pulse at the same stamp (M7); M4's "clock first" is for every other tick */
    if (st >= 0xF0u) return 1;
    if ((st & 0xF0u) == 0x80u || ((st & 0xF0u) == 0x90u && !d2)) return 2;
    if ((st & 0xF0u) == 0x90u) return 3;
    return 4; }
static void t_cap_at(uint32_t pkt, uint32_t due)
{   unsigned st = (pkt >> 8) & 0xFFu, ch = st & 15u, d1 = (pkt >> 16) & 0x7Fu, d2 = pkt >> 24;
    int r = rank_of(pkt);
    if (cap.check) {
        if (cap.n && (int32_t)(due - cap.last_due) < 0) cap.back++;
        if (cap.tvalid && cap.tid == sq_cur) {
            if (due != cap.tdue) cap.eq_bad++;
            if (r < cap.trank) cap.rank_bad++;
        }
        cap.tid = sq_cur; cap.tdue = due; cap.trank = r; cap.tvalid = 1;
    }
    cap.last_due = due; cap.n++;
    if (st == 0xF8u) { if (!cap.clk) cap.first_f8_due = due; cap.last_f8_due = due; cap.clk++; }
    else if (st == 0xFAu) cap.fa++;
    else if (st == 0xFBu) cap.fb++;
    else if (st == 0xFCu) cap.fc++;
    else if (st == 0xF2u) cap.spp++;
    else if ((st & 0xF0u) == 0x90u && d2) { if (cap.cnt[ch][d1]) cap.wire_bad++; cap.cnt[ch][d1] = 1; cap.ons++; }
    else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u) { if (!cap.cnt[ch][d1]) cap.wire_bad++; cap.cnt[ch][d1] = 0; cap.offs++; }
    midi_out_event_at(pkt, due); }
#define midi_out_event_at t_cap_at                   /* from here on, sq_core.c talks to the tap */

/* ---- a mock algorithm: dense, ratchets everywhere ---- */
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
static const AlgoDesc ALGOS[SQ_NALGO] = { MK(0, "DENSE", AF_PAIRING), MK(1, "DENSE", 0) };

#include "sq_core.c"

static int fails;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

static double fabs_(double x) { return x < 0 ? -x : x; }
static uint32_t rs = 0x2545F491u;
static uint32_t rnd(uint32_t n) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs % n; }

#define T0 1000000u                                  /* TIMER4 time of the first half (any value; the counter wraps, differences matter) */

static void reset_core(uint32_t bpm, int dense)
{   memset(sq_ch, 0, sizeof sq_ch); memset(sq_gdb, 0, sizeof sq_gdb); memset(sq_cache, 0, sizeof sq_cache);
    memset(sq_calgo, 0, sizeof sq_calgo); memset(sq_rt, 0, sizeof sq_rt); memset(sq_evq, 0, sizeof sq_evq);
    memset((void *)sq_cidx, 0, sizeof sq_cidx);
    sq_gidx = 0; sq_load_pending = 0; sq_req = 0; sq_drops = 0; sq_now = 0; sq_cur = 0; sq_acc = 0;
    sq_playing = 0; sq_off_stage = 0; sq_cc_pend = 0; sq_tref_ok = 0; sq_boff = 0; sq_tick_s = 0; sq_master = 1;
    memset(&cap, 0, sizeof cap);
    mock_ext_beat = 0; mock_bpm = bpm;
    memset(&usb, 0, sizeof usb); usb.config = 1; mo_r = mo_w = 0; midi_out_drops = 0;
    sq_gdb[0].s.gate = 50; sq_gdb[0].s.div = (uint8_t)(dense ? 2 : 2); sq_gdb[0].s.swing = (uint8_t)(dense ? 58 : 50);
    sq_gbuild(&sq_gdb[0]); sq_gdb[1] = sq_gdb[0];
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        Tbl *t = (Tbl *)sq_cache[c][0];
        sq_ch[c].midi_ch = dense ? c : SQ_MIDI_OFF; sq_ch[c].len = dense ? 8 : 1; sq_ch[c].algo = (uint8_t)(c & 1u);
        sq_calgo[c][0] = sq_calgo[c][1] = (uint8_t)(c & 1u);
        t->n = dense ? 8 : 1;
        for (uint8_t i = 0; i < t->n; i++) {
            t->s[i].note = (uint8_t)(36u + 5u * c + (dense ? i % 4u : 0u)); t->s[i].vel = 90; t->s[i].gate = (uint8_t)(30 + 10 * i);
            t->s[i].flags = (uint8_t)(dense ? (PO_HIT | (3u << PO_REP_SHIFT) | ((i & 1u) ? PO_PACE_DEC : 0u)) : 0u);
        }
    } }

/* one half = 4 blocks of 32; t0 of the half is ideal + jitter. The ring is emptied after every block (the host side is the release test). */
static uint32_t half_no;
static void run_blocks(uint32_t nblocks, int jitter)
{   for (uint32_t b = 0; b < nblocks; b++) {
        if ((b & 3u) == 0) {
            int32_t j = jitter ? (int32_t)rnd(601) - 300 : 0;
            if (jitter && rnd(5000) == 0) j = (int32_t)rnd(60001) - 30000;       /* a stall / an outlier over 1 ms */
            sq_half_begin(T0 + half_no * (HALF_FRAMES * 544u) + (uint32_t)j);
            half_no++;
        }
        sq_block(32);
        mo_r = mo_w;
    } }

static double exact_x(uint32_t bsq, uint32_t acc0) { return ((double)bsq - (double)acc0) / (double)SQ_QSTEP; }

/* ---- M1: the sample a tick lands on, for every BPM and a sweep of accumulator phases ---- */
static void t_tick_position(void)
{   long runs = 0, bad = 0, over = 0;
    for (uint32_t bpm = 40; bpm <= 240; bpm++) {
        uint32_t bsq, step;
        reset_core(bpm, 0);
        bsq = sq_beat_q4();
        step = bsq / 997u + 1u;
        for (uint32_t acc0 = 0; acc0 < bsq; acc0 += step) {
            double x = exact_x(bsq, acc0);
            uint32_t ticks_in_block = (acc0 + 32u * SQ_QSTEP) / bsq;
            if (ticks_in_block > 1u) over++;
            if (!ticks_in_block) continue;
            reset_core(bpm, 0);
            sq_tref_ok = 0; sq_half_begin(T0); sq_boff = 0;
            sq_playing = 1; sq_now = 8; sq_acc = acc0;       /* tick 8 is a clock-pulse tick: it stamps a 0xF8 */
            sq_rt_run(1u << 30);                             /* no channel steps in this test: only the clock marks the tick */
            sq_block(32);
            runs++;
            {   uint32_t s = (cap.last_f8_due - T0) / 544u - MO_LEAD_SAMPLES;
                if (cap.clk != 1 || s > 31u || (double)s > x + 1e-9 || (double)s < x - 1.0 - 1e-9) {
                    if (bad++ < 3) printf("  bpm %u acc0 %u: x=%.3f s=%u clk=%ld\n", (unsigned)bpm, (unsigned)acc0, x, (unsigned)s, cap.clk);
                }
            }
        }
    }
    CHECK(runs > 50000, "tick position swept %ld phases", runs);
    CHECK(bad == 0, "%ld ticks outside one sample of their exact time", bad);
    CHECK(over == 0, "%ld blocks held more than one tick (40..240 BPM must never)", over);
    printf("  tick position: %ld phases, 40..240 BPM, every tick within 1 sample, at most 1 tick per block\n", runs); }

/* ---- M2: the stamp formula, monotonic stamps with a jittery half reference, one stamp per tick ---- */
static void t_stamp(void)
{   uint32_t bpms[] = { 40, 60, 90, 120, 150, 180, 210, 240 };
    long total = 0;
    /* exact formula: PLAY, tick 0, sample 0 */
    reset_core(120, 1);
    sq_half_begin(T0); sq_on_start(); mo_r = mo_w; cap.n = 0;
    sq_block(32);
    CHECK(cap.n > 0 && cap.fa == 1 && cap.clk == 1, "tick 0 sends Start then the first pulse (fa=%ld clk=%ld)", cap.fa, cap.clk);
    CHECK(cap.first_f8_due == T0 + MO_LEAD_SAMPLES * 544u, "tick 0 is due at tref + LEAD (%u vs %u)", (unsigned)(cap.first_f8_due - T0), MO_LEAD_SAMPLES * 544u);
    for (unsigned k = 0; k < sizeof bpms / sizeof bpms[0]; k++) {
        uint32_t secs = (bpms[k] == 40 || bpms[k] == 120 || bpms[k] == 240) ? 600u : 60u;     /* 10 minutes at three tempi */
        reset_core(bpms[k], 1); half_no = 0;
        sq_half_begin(T0); half_no = 1; sq_on_start(); mo_r = mo_w;
        cap.check = 1;
        run_blocks(secs * 44118u / 32u, 1);
        CHECK(cap.back == 0, "bpm %u: %ld stamps went backwards (jittery reference, outliers)", (unsigned)bpms[k], cap.back);
        CHECK(cap.eq_bad == 0, "bpm %u: %ld packets of one tick carry different stamps", (unsigned)bpms[k], cap.eq_bad);
        CHECK(cap.wire_bad == 0, "bpm %u: %ld wire violations (double ON / OFF without ON)", (unsigned)bpms[k], cap.wire_bad);
        total += cap.n;
    }
    printf("  stamp: %ld packets over 8 tempi, non-decreasing, one stamp per tick, jittery reference\n", total);
    /* the filter: a steady 1 ms pull is followed within a second, an outlier resyncs */
    reset_core(120, 0); sq_half_begin(T0);
    for (uint32_t h = 1; h < 600; h++) sq_half_begin(T0 + h * (HALF_FRAMES * 544u) + 1000u);
    CHECK((int32_t)(sq_tref - (T0 + 599u * (HALF_FRAMES * 544u) + 1000u)) < 200 && (int32_t)(sq_tref - (T0 + 599u * (HALF_FRAMES * 544u) + 1000u)) > -200,
          "the reference follows a constant offset (err %d)", (int)(sq_tref - (T0 + 599u * (HALF_FRAMES * 544u) + 1000u)));
    sq_half_begin(T0 + 700u * (HALF_FRAMES * 544u) + 90000u);
    CHECK(sq_tref == T0 + 700u * (HALF_FRAMES * 544u) + 90000u, "an error over 1 ms resynchronises outright"); }

/* ---- M4: clock, Start, offs, ons at every tick of 4 dense bars; no same-note ON before its OFF ---- */
static void t_ordering(void)
{   uint32_t bpms[] = { 90, 120, 180, 240 };
    for (unsigned k = 0; k < 4; k++) {
        reset_core(bpms[k], 1); half_no = 0;
        sq_half_begin(T0); half_no = 1; sq_on_start(); mo_r = mo_w; cap.check = 1;
        while (sq_now < 4u * SQ_BAR_TICKS) run_blocks(4, 0);
        CHECK(cap.rank_bad == 0, "bpm %u: %ld packets out of clock / Start / OFF / ON order within a tick", (unsigned)bpms[k], cap.rank_bad);
        CHECK(cap.wire_bad == 0, "bpm %u: %ld wire violations", (unsigned)bpms[k], cap.wire_bad);
        CHECK(cap.ons > 500, "bpm %u: the dense run sent %ld ONs", (unsigned)bpms[k], cap.ons);
    }
    printf("  ordering: 4 bars of dense ratchets at 4 tempi, clock then Start then OFFs then ONs per tick\n"); }

/* ---- M7: clock out ---- */
static void t_clock_out(void)
{   uint32_t bpms[] = { 90, 120, 180 };
    for (unsigned k = 0; k < 3; k++) {
        uint32_t quarters = 16u, ticks = quarters * 192u;
        reset_core(bpms[k], 1); half_no = 0;
        sq_half_begin(T0); half_no = 1; sq_on_start(); mo_r = mo_w;
        while (sq_now < ticks) run_blocks(4, 0);
        CHECK(sq_now == ticks || sq_now == ticks + 1u, "ran %u ticks", (unsigned)sq_now);
        {   long want = (long)((sq_now + 7u) / 8u);
            CHECK(cap.clk == want && cap.clk == 24 * 16 + (sq_now > ticks ? 1 : 0), "bpm %u: %ld pulses in %u ticks (want %ld = 24 per quarter)", (unsigned)bpms[k], cap.clk, (unsigned)sq_now, want); }
        CHECK(cap.fa == 1 && cap.fb == 0 && cap.fc == 0 && cap.spp == 0, "bpm %u: Start once, no Continue / Stop / SPP while running", (unsigned)bpms[k]);
        {   uint32_t span = cap.last_f8_due - cap.first_f8_due;                    /* mean spacing of the pulses against the ideal */
            double ideal = (double)(cap.clk - 1) * (SEQ_FS_Q4 / 16.0 * 60.0 / bpms[k] / 24.0) * 544.0;
            CHECK(span > ideal * 0.9999 && span < ideal * 1.0001, "bpm %u: pulse span %u vs ideal %.0f ticks", (unsigned)bpms[k], (unsigned)span, ideal); }
        sq_req = SQ_REQ_STOP; sq_block(32);
        CHECK(cap.fc == 1, "bpm %u: Stop sends exactly one 0xFC", (unsigned)bpms[k]);
        sq_req = SQ_REQ_STOP; sq_block(32);
        CHECK(cap.fc == 1, "bpm %u: Stop while stopped sends nothing", (unsigned)bpms[k]);
        mo_r = mo_w;
    }
    /* every EXT state: nothing */
    reset_core(120, 1); half_no = 0; sq_master = 0;
    sq_half_begin(T0); half_no = 1; sq_on_start(); mo_r = mo_w;
    while (sq_now < 3u * 192u) run_blocks(4, 0);
    sq_clock_continue(480); sq_req = SQ_REQ_STOP; sq_block(32);
    CHECK(cap.clk == 0 && cap.fa == 0 && cap.fb == 0 && cap.fc == 0 && cap.spp == 0, "not the master: no clock, Start, Stop, SPP (clk=%ld fa=%ld fc=%ld)", cap.clk, cap.fa, cap.fc);
    CHECK(cap.ons > 0, "the notes still play when not the master");
    /* Continue: SPP (tick / 48, 14 bits) then 0xFB */
    reset_core(120, 0); sq_half_begin(T0); sq_boff = 0; mo_r = mo_w; sq_stamp_now();
    sq_clock_continue(48u * 300u);
    CHECK(mo_w == 2u, "continue sends two packets");
    CHECK(midi_out_q[0] == (0x03u | 0xF2u << 8 | (300u & 0x7Fu) << 16 | (300u >> 7) << 24), "SPP 300 = LSB 44, MSB 2 (%08x)", (unsigned)midi_out_q[0]);
    CHECK(midi_out_q[1] == (0x0Fu | 0xFBu << 8), "then 0xFB");
    printf("  clock out: exactly 24 per quarter at 3 tempi (spacing within 100 ppm), Start / Stop / SPP only on their transitions, silent as a follower\n"); }

/* ---- M3, M5: the release ring ---- */
static int ep_free = 1;
static uint32_t sent[4096], nsent, sent_at[4096], n_transfers, last_transfer_n;
static int  ep1_tx_ready(void) { return ep_free; }
static void ep1_tx_arm(uint32_t n)
{   for (uint32_t i = 0; i < n; i += 4u) {
        sent[nsent] = (uint32_t)ep1tx[i] | (uint32_t)ep1tx[i + 1] << 8 | (uint32_t)ep1tx[i + 2] << 16 | (uint32_t)ep1tx[i + 3] << 24;
        sent_at[nsent++] = fake_ticks;
    }
    last_transfer_n = n; n_transfers++; ep_free = 0; }
static uint32_t pk_on(uint32_t n) { return 0x09u | 0x90u << 8 | (n & 127u) << 16 | 100u << 24; }
static uint32_t pk_off(uint32_t n) { return 0x08u | 0x80u << 8 | (n & 127u) << 16; }
static uint32_t pk_clk(void) { return 0x0Fu | 0xF8u << 8; }
static void ring_reset(void) { mo_r = mo_w = 0; midi_out_drops = 0; nsent = 0; n_transfers = 0; ep_free = 1; memset(&usb, 0, sizeof usb); usb.config = 1; }

static void t_release(void)
{   uint32_t i;
    /* 1. a packet leaves within one 100 us tick of its due, and never before it */
    ring_reset(); fake_ticks = 5000000u;
    midi_out_event_at(pk_on(60), 5000000u + 70000u);
    for (i = 0; i < 100; i++) { fake_ticks += 2400u; midi_tx_release(); if (nsent) break; }
    CHECK(nsent == 1 && sent[0] == pk_on(60), "the packet was released");
    CHECK((int32_t)(sent_at[0] - (5000000u + 70000u)) >= 0, "never before its due");
    CHECK((int32_t)(sent_at[0] - (5000000u + 70000u)) < 2400, "within one 100 us tick of its due (%d)", (int)(sent_at[0] - (5000000u + 70000u)));
    /* 2. 2000 random dues, a release call every 100 us (with a USB endpoint that frees 1 ms after each transfer): lateness bound,
     *    the order of insertion, 16 packets per transfer at most */
    ring_reset(); fake_ticks = 0xFFFF0000u;                  /* across the 32-bit wrap of the counter */
    {   uint32_t due = fake_ticks + 5000u, want[2000], nw = 0, dues[2000], endpoint_t = 0, worst = 0, maxn = 0, pushed = 0;
        uint32_t clock_t100 = 0;
        while (nsent < 2000u) {
            while (pushed < 2000u && midi_out_free() > 20u && rnd(3) != 0) {
                due += rnd(4) == 0 ? 0u : rnd(30000u);       /* non-decreasing, many equal */
                want[pushed] = pk_on(pushed); dues[pushed] = due; midi_out_event_at(want[pushed], due); pushed++;
            }
            fake_ticks += 2400u; clock_t100++;
            if (!ep_free && (int32_t)(fake_ticks - endpoint_t) >= 0) ep_free = 1;
            { uint32_t before = nsent; midi_tx_release(); if (nsent != before) { endpoint_t = fake_ticks + 24000u; if (last_transfer_n / 4u > maxn) maxn = last_transfer_n / 4u; } }
            if (clock_t100 > 1000000u) break;
        }
        CHECK(nsent == 2000u, "all 2000 packets left (%u)", (unsigned)nsent);
        for (i = 0; i < nsent; i++) {
            if (sent[i] != want[nw]) { CHECK(0, "order broken at %u", (unsigned)i); break; }
            if ((int32_t)(sent_at[i] - dues[nw]) < 0) { CHECK(0, "packet %u left early", (unsigned)i); break; }
            nw++;
        }
        CHECK(maxn <= 16u, "a transfer holds at most 16 packets (%u)", (unsigned)maxn);
        (void)worst;
    }
    /* lateness of the head when the endpoint is free: below one tick (the packets that waited for the endpoint are the USB's) */
    ring_reset(); fake_ticks = 100000u;
    {   uint32_t worst = 0;
        for (i = 0; i < 500; i++) {
            uint32_t due = fake_ticks + 3000u + rnd(60000u), t_end;
            ring_reset(); ep_free = 1;
            midi_out_event_at(pk_on(i), due);
            t_end = fake_ticks + 200000u;
            while (!nsent && (int32_t)(fake_ticks - t_end) < 0) { fake_ticks += 2400u; midi_tx_release(); }
            if (nsent && sent_at[0] - due > worst) worst = sent_at[0] - due;
        }
        CHECK(worst < 2400u, "release lateness below one 100 us tick (%u ticks)", (unsigned)worst);
        printf("  release: lateness at most %u ticks (%.1f us) with a free endpoint, in order across the counter wrap, 16 per transfer\n", (unsigned)worst, worst / 24.0);
    }
    /* 3. 40 packets due together: 16, 16, 8 */
    ring_reset(); fake_ticks = 7000000u;
    for (i = 0; i < 40; i++) midi_out_event_at(pk_off(i), fake_ticks);
    for (i = 0; i < 3; i++) { ep_free = 1; midi_tx_release(); }
    CHECK(nsent == 40u && n_transfers == 3u, "40 due packets leave in 3 transfers (%u, %u)", (unsigned)nsent, (unsigned)n_transfers);
    /* 4. a busy endpoint keeps them; an undue head blocks, no reordering */
    ring_reset(); fake_ticks = 8000000u;
    midi_out_event_at(pk_on(1), fake_ticks + 48000u);
    midi_out_event_at(pk_off(2), fake_ticks);                /* raised to the tail's due: it cannot overtake */
    midi_tx_release();
    CHECK(nsent == 0, "an undue head waits, and nothing overtakes it");
    fake_ticks += 48000u; ep_free = 0; midi_tx_release();
    CHECK(nsent == 0, "a busy endpoint keeps the packets");
    ep_free = 1; midi_tx_release();
    CHECK(nsent == 2u && sent[0] == pk_on(1) && sent[1] == pk_off(2), "then both leave in insertion order");
    /* 5. the reserve rule with a full-ish ring of future packets: ONs go first, OFFs and clock stay */
    ring_reset(); fake_ticks = 9000000u;
    while (midi_out_free() > MO_RESERVE) midi_out_event_at(pk_off(3), fake_ticks + 100000u);
    midi_out_event_at(pk_on(70), fake_ticks + 100000u);
    CHECK(midi_out_drops == 0u, "with exactly the reserve free an ON still queues");
    midi_out_event_at(pk_on(71), fake_ticks + 100000u);
    CHECK(midi_out_drops == 1u, "below the reserve an ON is dropped and counted");
    midi_out_event_at(pk_off(72), fake_ticks + 100000u); midi_out_event_at(pk_clk(), fake_ticks + 100000u);
    CHECK(midi_out_drops == 1u && midi_out_free() == MO_RESERVE - 3u, "an OFF and a clock still queue");
    while (midi_out_free()) midi_out_event_at(pk_clk(), fake_ticks + 100000u);
    midi_out_event_at(pk_off(1), fake_ticks); CHECK(midi_out_drops == 2u, "a full ring refuses everything and counts");
    /* 6. the nested hook: with FELUCCA_TX_NESTED=0 (the M3 fallback) it does nothing, midi_tx_release does the work either way */
    ring_reset(); fake_ticks = 1000u;
    midi_out_event_at(pk_on(5), 1000u);
    midi_tx_release_nested();
#if FELUCCA_TX_NESTED
    CHECK(nsent == 1u, "FELUCCA_TX_NESTED=1: the nested call releases a due packet");
#else
    CHECK(nsent == 0u, "FELUCCA_TX_NESTED=0: the nested call releases nothing (fallback)");
    midi_tx_release();
    CHECK(nsent == 1u, "FELUCCA_TX_NESTED=0: the between-interrupts call releases it");
#endif
    /* 7. no host: nothing queued */
    ring_reset(); usb.config = 0; midi_out_event_at(pk_on(1), 0); CHECK(mo_w == mo_r, "no host: nothing queued"); }

/* ---- M10: the tempo constant ---- */
static void t_tempo(void)
{   double worst = 0, worst_old = 0;
    for (uint32_t bpm = 40; bpm <= 240; bpm++) {
        uint32_t secs = (bpm == 40 || bpm == 60 || bpm == 90 || bpm == 120 || bpm == 180 || bpm == 240) ? 300u : 20u;     /* 5 minutes at six tempi */
        uint64_t samples = (uint64_t)secs * 44118u / 32u * 32u;
        double ideal, eff, ppm;
        reset_core(bpm, 0); half_no = 0;
        sq_half_begin(T0); half_no = 1; sq_on_start();
        run_blocks((uint32_t)(samples / 32u), 0);
        ideal = (double)samples / 44117.647 * bpm * 192.0 / 60.0;            /* ticks the quartz says have elapsed */
        eff = (double)sq_now + (double)sq_acc / (double)sq_beat_q4() - 1.0;  /* ticks the accumulator counted, with its fraction */
        ppm = fabs_(eff - ideal) / ideal * 1e6;
        if (ppm > worst) worst = ppm;
        {   double old = (double)samples * bpm * 192.0 / (44100.0 * 60.0); double po = fabs_(old - ideal) / ideal * 1e6; if (po > worst_old) worst_old = po; }
        if (ppm > 5.0) printf("  bpm %u: %.2f ppm\n", (unsigned)bpm, ppm);
    }
    CHECK(worst <= 5.0, "worst tempo error %.2f ppm (limit 5)", worst);
    CHECK(worst_old > 390.0, "the old 44100 constant would have been %.0f ppm off (sanity)", worst_old);
    printf("  tempo constant: 40..240 BPM within %.2f ppm of 44117.65 Hz (the 44100 Hz count was %.0f ppm off)\n", worst, worst_old); }

int main(void)
{   printf("timing_test (FELUCCA_TX_NESTED=%d)\n", FELUCCA_TX_NESTED);
    t_tick_position();
    t_stamp();
    t_ordering();
    t_clock_out();
    t_release();
    t_tempo();
    if (fails) { printf("TIMING FAIL (%d)\n", fails); return 1; }
    printf("TIMING PASS\n");
    return 0; }
