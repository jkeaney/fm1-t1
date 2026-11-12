/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_core_test.c (P4-004): the scheduler of firmware/src/sq_core.c against mock hooks and a mock ALGOS table.
 *
 * The nine event-level goldens of ADR-0003 section 14 (tests/golden/sq_core_*.txt: one line per packet the core sends, stamped
 * with the tick it left on) and the 100000-tick invariant fuzz (run it under -fsanitize=address,undefined):
 *   cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -fno-strict-aliasing -fsanitize=address,undefined -Ifirmware/src \
 *      -o /tmp/sq_core_test tests/sq_core_test.c && /tmp/sq_core_test
 * GOLDEN_UPDATE=1 rewrites the golden files (review the diff: the goldens ARE the specification). Run from the repo root.
 *
 * sq_core.c is the only caller of midi_out_event_at / trk_note_on / trk_note_off / beat_samples; the mocks below are the host side of
 * those hooks, and the host log is what midi_out_event_at receives (so sq_emit's output, reserve rule included, is what is checked). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"

/* ---- host side of the device hooks ---- */
typedef struct { int unused; } track_t;
static track_t trk[SQ_NCH];
#define MO_RESERVE 8u
static uint32_t mock_free = 128;                     /* free slots of the USB out ring; the test drains it by resetting this */
static uint32_t mock_bs = 22050;                     /* samples per beat: 120 BPM at 44100 */
static uint32_t midi_out_free(void) { return mock_free; }
static uint32_t beat_samples(void) { return mock_bs; }
#define SQ_EXT_BEAT() mock_bs                        /* P4-007: sq_core counts its own beat in 1/16 samples; here the mocked external beat, so 120 BPM = 22050 as before */
#define SQ_BPM()      120u

static char     logbuf[1 << 18];
static size_t   logn;
static int      log_on = 1;
static long     viol;                                /* invariant violations seen in the packet stream */
static int      midi_cnt[16][128];                   /* 1 while a (channel, note) sounds on the wire */
static long     midi_on_total, midi_off_total;
static int      prv_cnt[SQ_NCH][128];
static long     prv_on_total, prv_off_total;
static uint32_t log_tick_now;
static void vio(const char *w, unsigned ch, unsigned n) { if (viol++ < 3) printf("  violation: %s ch=%u n=%u at tick %u\n", w, ch, n, (unsigned)log_tick_now); }                        /* set by the test before each call: the stamp of the lines */

static void logf_(const char *fmt, unsigned a, unsigned b, unsigned c, unsigned d)
{   if (!log_on) return;
    logn += (size_t)snprintf(logbuf + logn, sizeof logbuf - logn, fmt, a, b, c, d); }

static long clk_pkts;                                /* realtime / common packets (clock out, P4-007): counted, not logged: the goldens are about notes */
static void midi_out_event_at(uint32_t pkt, uint32_t due)
{   unsigned st = (pkt >> 8) & 0xF0u, ch = (pkt >> 8) & 0xFu, d1 = (pkt >> 16) & 0xFFu, d2 = pkt >> 24;
    (void)due;
    if (st == 0xF0u) { clk_pkts++; return; }
    if (mock_free) mock_free--;
    if (st == 0x90u && d2) {
        if (midi_cnt[ch][d1]) vio("double ON", ch, d1);                /* two NOTE_ONs of one note without an OFF between */
        midi_cnt[ch][d1] = 1; midi_on_total++;
        logf_("%5u MIDI ch=%2u ON  n=%3u v=%3u\n", log_tick_now, ch + 1, d1, d2);
    } else if (st == 0x80u || st == 0x90u) {
        if (!midi_cnt[ch][d1]) vio("OFF without ON", ch, d1);               /* an OFF without its ON */
        midi_cnt[ch][d1] = 0; midi_off_total++;
        logf_("%5u MIDI ch=%2u OFF n=%3u\n", log_tick_now, ch + 1, d1, 0);
    } else if (st == 0xB0u && d1 == 123u) {
        logf_("%5u MIDI ch=%2u CC123\n", log_tick_now, ch + 1, 0, 0);
    } else vio("stray", ch, st);
}
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel)
{   unsigned c = (unsigned)(t - trk);
    if (prv_cnt[c][note]) vio("prv double ON", c, (unsigned)note);
    prv_cnt[c][note] = 1; prv_on_total++;
    logf_("%5u PRV  c=%u ON  n=%3u v=%3u\n", log_tick_now, c + 1, (unsigned)note, (unsigned)vel); }
static void trk_note_off(track_t *t, uint32_t note)
{   unsigned c = (unsigned)(t - trk);
    if (!prv_cnt[c][note]) vio("prv OFF without ON", c, (unsigned)note);
    prv_cnt[c][note] = 0; prv_off_total++;
    logf_("%5u PRV  c=%u OFF n=%3u\n", log_tick_now, c + 1, (unsigned)note, 0); }

/* ---- the mock algorithm table: a step table in the cache, read by every algorithm id; they differ in flags and build ---- */
typedef struct { uint8_t n, pad; PulseOut s[9]; } Tbl;           /* 56 B <= SQ_CACHE_MAX */
_Static_assert(sizeof(Tbl) <= SQ_CACHE_MAX, "Tbl fits a cache");
static uint8_t  tbl_len(const void *k) { return ((const Tbl *)k)->n; }
static void     tbl_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o)
{   const Tbl *t = (const Tbl *)k; (void)hc; *o = t->s[g % t->n]; }
static uint8_t  tbl_sub(const void *k, uint32_t g, uint8_t kk, const HarmChord *hc)
{   const Tbl *t = (const Tbl *)k; (void)hc; return (uint8_t)(t->s[g % t->n].note + 3u * kk); }
static void     tbl_nobuild(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k)
{   (void)m; (void)seed; (void)g; (void)k; }

/* fuzz builders: a random but seeded table (few distinct notes so ties and same-pitch cases come up) */
static void fz_fill(const uint16_t seed[SQ_NSEED], void *k, int pairing, int rich)
{   Tbl *t = (Tbl *)k; SqRng r;
    sq_rng_init(&r, seed[0], 0x5151u);
    memset(t, 0, sizeof *t);
    t->n = (uint8_t)(2 + sq_rnd(&r, 8));
    for (uint8_t i = 0; i < t->n; i++) {
        PulseOut *o = &t->s[i];
        o->note = (uint8_t)(36 + 3 * sq_rnd(&r, 3));
        o->vel = (uint8_t)(1 + sq_rnd(&r, 127));
        o->gate = (uint8_t)(1 + sq_rnd(&r, 100));
        o->flags = (uint8_t)(sq_rnd(&r, 100) < 65 ? PO_HIT : 0);
        if (pairing && sq_rnd(&r, 100) < 45) o->flags |= PO_SLIDE_IN;
        if (rich) {
            if (sq_rnd(&r, 100) < 30) o->flags |= (uint8_t)(sq_rnd(&r, 4) << PO_REP_SHIFT);
            if (sq_rnd(&r, 100) < 50) o->flags |= PO_PACE_DEC;
            if (sq_rnd(&r, 100) < 20) { o->flags |= PO_HOLD; o->hold = (uint8_t)(1 + sq_rnd(&r, 8)); }
        }
    } }
static void fz_a(const uint8_t m[SQ_NMORPH], const uint16_t s[SQ_NSEED], const SeqGlobal *g, void *k) { (void)m; (void)g; fz_fill(s, k, 1, 0); }
static void fz_b(const uint8_t m[SQ_NMORPH], const uint16_t s[SQ_NSEED], const SeqGlobal *g, void *k) { (void)m; (void)g; fz_fill(s, k, 0, 1); }
static void fz_c(const uint8_t m[SQ_NMORPH], const uint16_t s[SQ_NSEED], const SeqGlobal *g, void *k) { (void)m; (void)g; fz_fill(s, k, 1, 1); }

#define MK(i, nm, fl, bld, sub) [i] = { .name = nm, .id = i, .n_seed = 1, .flags = fl, .cache_bytes = sizeof(Tbl), .build = bld, \
                                        .loop_len = tbl_len, .step = tbl_step, .sub_note = sub }
static const AlgoDesc ALGOS[SQ_NALGO] = {
    MK(0, "TBLP", AF_PAIRING, tbl_nobuild, NULL),                /* hand-written table, slide pairing */
    MK(1, "TBL",  0,          tbl_nobuild, NULL),                /* hand-written table, no pairing */
    MK(2, "FZA",  AF_PAIRING, fz_a, NULL),
    MK(3, "FZB",  0,          fz_b, NULL),
    MK(4, "FZC",  AF_PAIRING | AF_CHORD, fz_c, tbl_sub),
    MK(5, "FZD",  0,          fz_c, tbl_sub),
};

#include "sq_core.c"

/* ---- helpers ---- */
static int fails;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

static void reset_all(void)
{   memset(sq_ch, 0, sizeof sq_ch); memset(sq_gdb, 0, sizeof sq_gdb); memset(sq_cache, 0, sizeof sq_cache);
    memset(sq_calgo, 0, sizeof sq_calgo); memset(sq_rt, 0, sizeof sq_rt); memset(sq_evq, 0, sizeof sq_evq);
    memset(sq_stage_ch, 0, sizeof sq_stage_ch); memset((void *)sq_cidx, 0, sizeof sq_cidx);
    sq_gidx = 0; sq_load_pending = 0; sq_epoch = 0; sq_req = 0; sq_drops = 0; sq_now = 0; sq_cur = 0; sq_acc = 0;
    sq_playing = 0; sq_off_stage = 0; sq_cc_pend = 0;
    memset(midi_cnt, 0, sizeof midi_cnt); memset(prv_cnt, 0, sizeof prv_cnt);
    midi_on_total = midi_off_total = prv_on_total = prv_off_total = 0; viol = 0;
    mock_free = 128; mock_bs = 22050; logn = 0; logbuf[0] = 0; log_on = 1; log_tick_now = 0;
    for (uint8_t c = 0; c < SQ_NCH; c++) { sq_ch[c].midi_ch = SQ_MIDI_OFF; sq_ch[c].len = 1; sq_ch[c].algo = 1; sq_calgo[c][0] = 1; sq_calgo[c][1] = 1; }
    sq_gdb[0].s.gate = 50; sq_gdb[0].s.div = 2; sq_gdb[0].s.swing = 50; sq_gdb[0].s.scale = 0;
    sq_gbuild(&sq_gdb[0]); sq_gdb[1] = sq_gdb[0];
    for (uint8_t c = 0; c < SQ_NCH; c++) {                         /* a silent one-step table everywhere until a test says otherwise */
        Tbl *t = (Tbl *)sq_cache[c][0]; t->n = 1; t->s[0].note = 36; t->s[0].vel = 80; t->s[0].gate = 50; t->s[0].flags = 0; }
}

static Tbl *tbl_of(uint8_t c, uint8_t buf, uint8_t algo, uint8_t n)
{   Tbl *t = (Tbl *)sq_cache[c][buf];
    memset(t, 0, sizeof *t); t->n = n; sq_calgo[c][buf] = algo; if (buf == 0) { sq_cidx[c] = 0; sq_ch[c].algo = algo; sq_ch[c].len = n; }
    return t; }
static void put(Tbl *t, uint8_t i, uint8_t hit, uint8_t note, uint8_t vel, uint8_t gate, uint8_t flags, uint8_t hold)
{   t->s[i].note = note; t->s[i].vel = vel; t->s[i].gate = gate; t->s[i].flags = (uint8_t)(flags | (hit ? PO_HIT : 0)); t->s[i].hold = hold; }

static void start(void) { log_tick_now = 0; sq_on_start(); logn = 0; logbuf[0] = 0; }   /* PLAY's own CC123 is not what the goldens are about */
static void run(uint32_t from, uint32_t to)                                              /* ticks from..to inclusive */
{   for (uint32_t t = from; t <= to; t++) { mock_free = 128; log_tick_now = t; sq_tick(t); } }

static void golden(const char *name)
{   char path[160]; FILE *f; static char want[1 << 18]; size_t n = 0;
    snprintf(path, sizeof path, "tests/golden/%s", name);
    if (getenv("GOLDEN_UPDATE")) { f = fopen(path, "wb"); if (!f) { printf("FAIL cannot write %s\n", path); fails++; return; } fwrite(logbuf, 1, logn, f); fclose(f); printf("  wrote %s (%zu B)\n", path, logn); return; }
    f = fopen(path, "rb");
    if (!f) { printf("FAIL missing golden %s (GOLDEN_UPDATE=1 writes it)\n", path); fails++; return; }
    n = fread(want, 1, sizeof want - 1, f); fclose(f); want[n] = 0;
    if (n != logn || memcmp(want, logbuf, n)) {
        size_t i = 0; while (i < n && i < logn && want[i] == logbuf[i]) i++;
        printf("FAIL %s differs at byte %zu\n--- got ---\n%s--- want ---\n%s", name, i, logbuf, want); fails++;
    } else printf("  ok  %s (%zu B)\n", name, n); }

static long sounding(void) { long k = 0; for (int c = 0; c < SQ_NCH; c++) k += (sq_rt[c].snd[0] != 0) + (sq_rt[c].snd[1] != 0); return k; }

static int count_lines(const char *what)
{   int k = 0; const char *p = logbuf;
    while ((p = strstr(p, what)) != NULL) { k++; p += strlen(what); }
    return k; }

/* ---- the nine goldens ---- */

/* 1. sting 6.3a's 8-step worked example: L = 48, D = 6, MIDI channel 2. Exact ticks, the 12 events. */
static void g1_slide(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 0, 8);                                       /* TBLP: pairing on */
    put(t, 0, 1, 36, 120, 80, 0, 0);
    put(t, 1, 1, 39, 80, 55, PO_SLIDE_IN, 0);
    put(t, 2, 1, 39, 80, 24, PO_SLIDE_IN, 0);
    put(t, 3, 0, 0, 80, 50, 0, 0);
    put(t, 4, 1, 48, 80, 100, PO_SLIDE_IN, 0);                    /* the flag is stored but silent: step 3 is a rest */
    put(t, 5, 1, 36, 80, 55, PO_SLIDE_IN, 0);
    put(t, 6, 1, 41, 80, 24, PO_SLIDE_IN, 0);
    put(t, 7, 1, 43, 120, 55, 0, 0);
    sq_ch[0].midi_ch = 1;
    start(); run(0, 383);
    CHECK(count_lines("ON ") == 6 && count_lines("OFF") == 6, "12 events in total");
    golden("sq_core_1_slide.txt"); }

/* 2. swing 66 % at 1/16: the odd steps start 15 ticks late (o = (48 * 16 * 2 + 50) / 100) and their offs follow their own starts. */
static void g2_swing(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 4);
    for (uint8_t i = 0; i < 4; i++) put(t, i, 1, (uint8_t)(48 + i), 100, 50, 0, 0);
    sq_ch[0].midi_ch = 0; sq_gdb[0].s.swing = 66;
    start(); run(0, 383);                                         /* two bars of the 4-step loop */
    golden("sq_core_2_swing.txt"); }

/* 3. ratchet inside 1/32 (L = 24): 3 triggers accelerating, 3 decelerating, 4 triggers (spacing under 3 ticks reduces the count). */
static void g3_ratchet(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 3);
    put(t, 0, 1, 40, 100, 50, (uint8_t)(2u << PO_REP_SHIFT), 0);
    put(t, 1, 1, 41, 100, 50, (uint8_t)((2u << PO_REP_SHIFT) | PO_PACE_DEC), 0);
    put(t, 2, 1, 42, 100, 50, (uint8_t)(3u << PO_REP_SHIFT), 0);
    sq_ch[0].midi_ch = 0; sq_gdb[0].s.div = 0;
    start(); run(0, 71);
    golden("sq_core_3_ratchet.txt"); }

/* 4. STAGE hold of 8 pulses: the off is at G + 8 * L - 2 whatever GATE is; GATE 100 % is heard from the next ordinary step. */
static void g4_hold(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 16);
    put(t, 0, 1, 38, 100, 50, PO_HOLD, 8);
    put(t, 8, 1, 38, 100, 50, 0, 0);
    sq_ch[0].midi_ch = 0;
    start();
    run(0, 99);
    sq_gdb[0].s.gate = 100;                                       /* GATE changed mid-hold */
    run(100, 449);
    golden("sq_core_4_hold.txt"); }

/* 5. mute keeps phase: muted at tick 100, unmuted at tick 250. The note sounding at 100 still ends; ONs are silent while muted;
 * the playhead (p, pass, grid) ends where an unmuted run's does. */
static void g5_mute(void)
{   uint8_t bp, bpass; uint32_t bgrid;
    for (int pass = 0; pass < 2; pass++) {
        Tbl *t; reset_all();
        t = tbl_of(0, 0, 1, 8);
        for (uint8_t i = 0; i < 8; i++) put(t, i, 1, (uint8_t)(50 + i), 100, 80, 0, 0);
        sq_ch[0].midi_ch = 2;
        start();
        run(0, 99);
        if (pass == 1) sq_ch[0].muted = 1;
        run(100, 249);
        if (pass == 1) sq_ch[0].muted = 0;
        run(250, 399);
        if (pass == 0) { bp = sq_rt[0].p; bpass = (uint8_t)sq_rt[0].pass; bgrid = sq_rt[0].grid; }
        else {
            CHECK(sq_rt[0].p == bp && sq_rt[0].pass == bpass && sq_rt[0].grid == bgrid, "mute keeps phase");
            CHECK(midi_on_total - midi_off_total == sounding(), "mute leaves no unmatched note");
            golden("sq_core_5_mute.txt");
        }
    } }

/* 6. STOP mid-note: ch 1 is inside a slide overlap (two notes), ch 2 holds one: an OFF for each on the channel it was sent with,
 * then CC123 on every channel in use, ascending; nothing is left queued. */
static void g6_stop(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 0, 2);
    put(t, 0, 1, 36, 120, 80, 0, 0);
    put(t, 1, 1, 39, 80, 55, PO_SLIDE_IN, 0);
    t = tbl_of(1, 0, 1, 2);
    put(t, 0, 1, 60, 90, 100, PO_HOLD, 2);                         /* a 2-pulse hold: sounding at tick 50 */
    put(t, 1, 0, 0, 90, 100, 0, 0);
    sq_ch[0].midi_ch = 1; sq_ch[1].midi_ch = 4; sq_ch[2].midi_ch = 7;      /* ch 3 is configured but silent: it still gets CC123 */
    start();
    run(0, 49);                                                    /* 36 and 39 overlap from tick 48 (OFF of 36 at 54) */
    CHECK(sq_rt[0].snd[0] && sq_rt[0].snd[1] && sq_rt[1].snd[0], "three notes sound at the stop");
    log_tick_now = 50; sq_cur = 50; sq_on_stop();
    CHECK(sq_rt[0].snd[0] == 0 && sq_rt[0].snd[1] == 0 && sq_rt[1].snd[0] == 0, "nothing left sounding");
    { int q = 0; for (int i = 0; i < SQ_EVQ_N; i++) q += sq_evq[i].kind != 0; CHECK(q == 0, "the queue is cleared"); }
    CHECK(midi_on_total == midi_off_total, "ON == OFF after STOP");
    golden("sq_core_6_stop.txt"); }

/* 7. queue full: with one free slot a pair cannot be queued, so the step is lost whole (no unmatched NOTE_ON); freeing slots lets
 * the later steps through. */
static void g7_queue_full(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 4);
    for (uint8_t i = 0; i < 4; i++) put(t, i, 1, (uint8_t)(60 + i), 100, 50, 0, 0);
    sq_ch[0].midi_ch = 0;
    start();
    for (int i = 0; i < SQ_EVQ_N - 1; i++) sq_evq_put(i, 1000000u, 0x10u | (3u << 5), 1, 5, 0);   /* a filler OFF that never falls due */
    run(0, 99);
    CHECK(sq_drops == 3, "three steps (0, 48, 96) were lost whole, drops = %u", (unsigned)sq_drops);
    CHECK(count_lines("ON ") == 0, "no note leaves while the queue is full");
    for (int i = 0; i < 4; i++) sq_evq[i].kind = 0;
    run(100, 399);
    CHECK(count_lines("ON ") - count_lines("OFF") == sounding(), "every ON has its OFF queued or sent");
    CHECK(count_lines("ON ") > 0, "steps get through once slots are free");
    golden("sq_core_7_queue_full.txt"); }

/* 8. MIDI-channel change mid-note: no cut. The OFF of the sounding note goes to the channel it was sent on; the next ON uses the new one. */
static void g8_midi_change(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 4);
    for (uint8_t i = 0; i < 4; i++) put(t, i, 1, (uint8_t)(70 + i), 100, 80, 0, 0);
    sq_ch[0].midi_ch = 1;
    start();
    run(0, 19);
    sq_ch[0].midi_ch = 5;
    run(20, 120);
    golden("sq_core_8_midi_change.txt"); }

/* 9. a preset queued at tick 100 lands only on the 768-tick bar start; the playhead does not jump (length 5, so tick 768 is step 1). */
static void g9_preset_bar(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 5);
    for (uint8_t i = 0; i < 5; i++) put(t, i, 1, (uint8_t)(36 + i), 100, 50, 0, 0);
    sq_ch[0].midi_ch = 1;
    t = tbl_of(0, 1, 1, 5);                                       /* the staged caches: buffer 1 */
    for (uint8_t i = 0; i < 5; i++) put(t, i, 1, (uint8_t)(60 + i), 100, 50, 0, 0);
    for (uint8_t c = 1; c < SQ_NCH; c++) { Tbl *q = (Tbl *)sq_cache[c][1]; q->n = 1; }
    sq_cidx[0] = 0; sq_ch[0].algo = 1; sq_ch[0].len = 5;
    sq_stage_ch[0] = sq_ch[0]; sq_stage_ch[0].midi_ch = 3;
    for (uint8_t c = 1; c < SQ_NCH; c++) sq_stage_ch[c] = sq_ch[c];
    sq_gdb[1] = sq_gdb[0]; sq_gdb[1].s.gate = 100;
    start();
    run(0, 99);
    sq_load_pending = 1;
    run(100, 766);
    CHECK(sq_epoch == 0 && sq_cidx[0] == 0, "nothing lands before the bar");
    run(767, 767);
    CHECK(sq_epoch == 0, "not at 767");
    run(768, 900);
    CHECK(sq_epoch == 1 && sq_cidx[0] == 1 && sq_gidx == 1 && !sq_load_pending, "applied once at tick 768");
    CHECK(sq_rt[0].p == (uint8_t)((900 / 48 + 1) % 5), "the playhead did not jump");
    golden("sq_core_9_preset_bar.txt"); }

/* ---- sq_block: the accumulator runs 192 ticks per beat from the first block ---- */
static void t_block(void)
{   reset_all(); log_on = 0;
    sq_on_start();
    for (uint32_t b = 0; b < (22050u * 8u) / 32u; b++) { mock_free = 128; sq_block(32); }
    {   uint32_t blocks = (22050u * 8u) / 32u, want = 1u + (blocks * 32u * 192u) / 22050u;
        CHECK(sq_now == want, "sq_block ran %u ticks, want %u", (unsigned)sq_now, (unsigned)want); }
    CHECK(sq_rt[0].grid > 0, "ticks reach the channels"); }

/* ---- the reserve rule and the retry of a short ring ---- */
static void t_reserve(void)
{   Tbl *t; reset_all();
    t = tbl_of(0, 0, 1, 2);
    put(t, 0, 1, 50, 100, 50, 0, 0); put(t, 1, 1, 51, 100, 50, 0, 0);
    sq_ch[0].midi_ch = 0;
    start();
    log_tick_now = 0; mock_free = 7; sq_tick(0);                   /* fewer than MO_RESERVE free: the ON is dropped and counted, its OFF discarded */
    CHECK(count_lines("ON ") == 0 && sq_drops == 1, "ON dropped under the reserve (drops %u)", (unsigned)sq_drops);
    mock_free = 128; log_tick_now = 12; sq_tick(12);               /* the OFF queued for tick 24 is not sent early; nothing unmatched */
    mock_free = 128; log_tick_now = 24; sq_tick(24);
    CHECK(count_lines("OFF") == 0, "no OFF without its ON");
    /* a sounding note whose OFF the ring cannot take is retried, and the next ON waits behind it */
    reset_all(); t = tbl_of(0, 0, 1, 1); put(t, 0, 1, 52, 100, 50, 0, 0); sq_ch[0].midi_ch = 0; start();
    log_tick_now = 0; sq_tick(0);
    mock_free = 0; log_tick_now = 24; sq_tick(24);
    CHECK(count_lines("OFF") == 0 && sq_rt[0].snd[0], "OFF deferred while the ring is full");
    mock_free = 128; log_tick_now = 25; sq_tick(25);
    CHECK(count_lines("OFF") == 1 && !sq_rt[0].snd[0], "OFF sent once there is room");
    /* STOP with a short ring: resumes in sq_block, the clock waits for it */
    reset_all(); t = tbl_of(0, 0, 1, 1); put(t, 0, 1, 52, 100, 100, PO_HOLD, 8); sq_ch[0].midi_ch = 0; sq_ch[1].midi_ch = 1; start();
    log_tick_now = 0; sq_tick(0);
    mock_free = 0; sq_on_stop();
    CHECK(sq_off_stage == 1 && sq_rt[0].snd[0], "short ring: the stop is unfinished");
    mock_free = 128; sq_block(32);
    CHECK(sq_off_stage == 0 && !sq_rt[0].snd[0] && count_lines("CC123") == 2, "finished on the next block"); }

/* ---- the invariant fuzz ---- */
static uint32_t fz_s = 0x1234567u, fz_seed = 0x1234567u;   /* argv[1] picks another seed */
static uint32_t fz_rnd(uint32_t n) { fz_s ^= fz_s << 13; fz_s ^= fz_s >> 17; fz_s ^= fz_s << 5; return (fz_s >> 8) % n; }

static void fz_rebuild(uint8_t c, uint8_t algo, uint8_t into_stage_buf)
{   uint16_t seed[SQ_NSEED]; uint8_t m[SQ_NMORPH] = { 0 };
    uint8_t buf = (uint8_t)(sq_cidx[c] ^ 1u);
    for (int i = 0; i < SQ_NSEED; i++) seed[i] = (uint16_t)fz_rnd(65536);
    ALGOS[algo].build(m, seed, &sq_gdb[sq_gidx], sq_cache[c][buf]);
    sq_calgo[c][buf] = algo;
    if (!into_stage_buf) { sq_cidx[c] = buf; sq_ch[c].algo = algo; } }

static void fuzz(long nticks)
{   long done = 0;
    reset_all(); log_on = 0; fz_s = fz_seed ? fz_seed : 1u;
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        sq_ch[c].midi_ch = (uint8_t)(c * 4u + fz_rnd(4)); sq_ch[c].len = (uint8_t)(1 + fz_rnd(16));
        sq_ch[c].pad[0] = (c & 1u) ? 0x80u : 0u;                  /* ch 2 and 4 have their preview on */
        fz_rebuild(c, (uint8_t)(2 + fz_rnd(SQ_NALGO - 2)), 0); }
    sq_on_start();
    while (done < nticks) {
        uint32_t r = fz_rnd(1000);
        uint8_t c = (uint8_t)fz_rnd(SQ_NCH);
        mock_free = fz_rnd(8) == 0 ? fz_rnd(12) : 128;            /* a USB ring that is sometimes nearly full */
        if (r < 6) sq_ch[c].muted ^= 1u;
        else if (r < 12) { uint32_t o = fz_rnd(5); sq_ch[c].midi_ch = (uint8_t)(o == 4 ? SQ_MIDI_OFF : c * 4u + o); }   /* private pools: no two channels share a MIDI channel */
        else if (r < 20 && !sq_load_pending) fz_rebuild(c, (uint8_t)(2 + fz_rnd(SQ_NALGO - 2)), 0);                               /* algorithm switch / morph: cache flip */
        else if (r < 24) sq_gdb[sq_gidx].s.div = (uint8_t)fz_rnd(6);
        else if (r < 28) sq_gdb[sq_gidx].s.swing = (uint8_t)(50 + fz_rnd(26));
        else if (r < 32) sq_gdb[sq_gidx].s.gate = (uint8_t)(10 + fz_rnd(91));
        else if (r < 38) sq_ch[c].len = (uint8_t)(1 + fz_rnd(20));
        else if (r < 41) sq_req |= SQ_REQ_RESTART;
        else if (r < 43) sq_req |= SQ_REQ_PANIC;
        else if (r < 45) sq_req |= SQ_REQ_STOP;
        else if (r < 48) sq_req |= SQ_REQ_START;
        else if (r < 52 && !sq_load_pending) {                    /* stage a preset: caches into the inactive buffers, then pending last */
            for (uint8_t k = 0; k < SQ_NCH; k++) { sq_stage_ch[k] = sq_ch[k]; sq_stage_ch[k].len = (uint8_t)(1 + fz_rnd(16));
                sq_stage_ch[k].midi_ch = (uint8_t)(fz_rnd(5) == 4 ? SQ_MIDI_OFF : k * 4u + fz_rnd(4)); fz_rebuild(k, (uint8_t)(2 + fz_rnd(SQ_NALGO - 2)), 1); sq_stage_ch[k].algo = sq_calgo[k][sq_cidx[k] ^ 1u]; }
            sq_gdb[sq_gidx ^ 1u] = sq_gdb[sq_gidx];
            sq_load_pending = 1; }
        sq_consume_requests();
        if (sq_off_stage) { if (sq_all_off() && sq_playing) sq_rt_run(sq_now); }
        if (sq_playing && !sq_off_stage) { log_tick_now = sq_now; sq_tick(sq_now); sq_now++; done++; }
        else { log_tick_now = sq_cur; sq_block(32); }
    }
    mock_free = 128; sq_req = 0; sq_load_pending = 0;
    sq_on_stop();
    for (int i = 0; i < 8 && sq_off_stage; i++) { mock_free = 128; sq_block(32); }
    { int open = 0, q = 0;
      for (int m = 0; m < 16; m++) for (int n = 0; n < 128; n++) open += midi_cnt[m][n];
      for (int k = 0; k < SQ_NCH; k++) for (int n = 0; n < 128; n++) open += prv_cnt[k][n];
      for (int i = 0; i < SQ_EVQ_N; i++) q += sq_evq[i].kind != 0;
      CHECK(viol == 0, "fuzz: %ld violations (double ON, OFF without ON, stray packet)", viol);
      CHECK(open == 0, "fuzz: %d notes still sounding after the final STOP", open);
      CHECK(midi_on_total == midi_off_total, "fuzz: #NOTE_ON %ld != #NOTE_OFF %ld", midi_on_total, midi_off_total);
      CHECK(prv_on_total == prv_off_total, "fuzz: preview ON %ld != OFF %ld", prv_on_total, prv_off_total);
      CHECK(q == 0 && !sq_off_stage, "fuzz: queue not empty / stop unfinished");
      CHECK(midi_on_total > 1000, "fuzz played something (%ld notes)", midi_on_total);
      printf("  fuzz: %ld ticks, %ld MIDI notes, %ld preview notes, %u drops\n", done, midi_on_total, prv_on_total, (unsigned)sq_drops); } }

int main(int argc, char **argv)
{   if (argc > 1) fz_seed = (uint32_t)strtoul(argv[1], NULL, 0);
    printf("sq_core scheduler\n");
    g1_slide(); g2_swing(); g3_ratchet(); g4_hold(); g5_mute(); g6_stop(); g7_queue_full(); g8_midi_change(); g9_preset_bar();
    t_block(); t_reserve();
    fuzz(100000);
    printf("%s\n", fails ? "SQ_CORE FAIL" : "SQ_CORE PASS");
    return fails ? 1 : 0; }
