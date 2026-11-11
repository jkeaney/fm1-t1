/* SPDX-License-Identifier: GPL-3.0-only */
/* STAGE (ml185): eight stages of 1..8 pulses each, a loop of TOTAL 8..64 pulses; each stage rests, fires once, holds, or
 * re-fires every `div` pulses. P2-016 prototype = the firmware file: P4 copies it, adds `static` and wires the ALGOS[3] row.
 * Spec: ./SPEC.md (sections cited as S<n>). Pure: build() and step() use only their arguments and const tables (ADR-0003
 * section 4). stdint only. */
#include "../common/sq_types.h"
#include "../common/sq_prims.h"

typedef struct {
    uint8_t note[8];       /* final MIDI note per stage, sq_note applied at build (late quantise) */
    uint8_t vel[8];        /* velocity per stage with DYN already applied */
    uint8_t mode[8];       /* 0 rest, 1 I (one note), 2 I- (one note held for the rest of the stage), 3 II (re-fires every div pulses) */
    uint8_t div[8];        /* div_eff = min(div, RDIV), 1..4 */
    uint8_t C[8];          /* cumulative pulse bounds: stage k owns pulses C[k-1]..C[k]-1; C[7] = TOTAL */
    uint8_t tot, off, dir; /* TOTAL; OFFSET and DIR are applied by step() (they read t) */
} StageCache;              /* 43 B */
_Static_assert(sizeof(StageCache) == 43 && sizeof(StageCache) <= SQ_CACHE_MAX, "StageCache");

#define M_DENS 0
#define M_TOTAL 1
#define M_RANGE 2
#define M_RATCH 3
#define M_OFFSET 4
#define M_RDIV 5
#define M_DYN 6
#define M_DIR 7

#define SALT_NOTES 0x6C1E94B3u
#define SALT_LENS  0x13A7D5F9u
#define SALT_RESTS 0x5E82C0A7u
#define SALT_MODE  0x34F9B16Du
#define SALT_VEL   0x7A05E3C1u

#define MD_REST 0
#define MD_ONE  1
#define MD_HOLD 2
#define MD_RE   3

static void stage_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache)
{
    StageCache *c = (StageCache *)cache;
    SqRng r;
    uint8_t *z = (uint8_t *)cache, slots[56], len[8], rank[16], ord_m[8], dv[8], lg[8], vel[8], play[8], k, i, tot, rat, rdiv;

    for (i = 0; i < sizeof *c; i++) z[i] = 0;                                 /* every byte of the cache is defined */
    tot = m[M_TOTAL]; if (tot < 8) tot = 8; if (tot > 64) tot = 64;

    /* NOTES: 8 raw values (SP7 span, then the late quantise) */
    sq_rng_init(&r, seed[0], SALT_NOTES);
    for (k = 0; k < 8; k++) c->note[k] = sq_note(g, sq_span((uint8_t)sq_rnd(&r, 256), m[M_RANGE]));

    /* LENS: each stage seven times in a shuffled list of 56; TOTAL takes the first TOTAL-8 entries as spare pulses (every stage is 1..8 long) */
    sq_rng_init(&r, seed[1], SALT_LENS);
    for (i = 0; i < 56; i++) slots[i] = (uint8_t)(i / 7);
    sq_shuffle(&r, slots, 56);
    for (k = 0; k < 8; k++) len[k] = 1;
    for (i = 0; i < tot - 8u; i++) len[slots[i]]++;
    for (k = 0, i = 0; k < 8; k++) { i = (uint8_t)(i + len[k]); c->C[k] = i; }

    /* RESTS: stage 0 ranks first; DENS keeps the first `den` ranks */
    sq_rng_init(&r, seed[2], SALT_RESTS);
    sq_rank_build(&r, rank, 8, 1);
    for (k = 0; k < 8; k++) play[k] = (uint8_t)(rank[k] < m[M_DENS]);

    /* MODE: always 7 + 8 + 8 draws. RDIV caps the divider; RATCH turns the first `rat` stages that really repeat (play and L > div_eff) into mode II */
    sq_rng_init(&r, seed[3], SALT_MODE);
    for (k = 0; k < 8; k++) ord_m[k] = k;
    sq_shuffle(&r, ord_m, 8);
    rdiv = m[M_RDIV]; if (rdiv < 1) rdiv = 1; if (rdiv > 4) rdiv = 4;
    for (k = 0; k < 8; k++) { dv[k] = (uint8_t)(1 + sq_rnd(&r, 4)); if (dv[k] > rdiv) dv[k] = rdiv; c->div[k] = dv[k]; }
    for (k = 0; k < 8; k++) lg[k] = (uint8_t)(sq_rnd(&r, 100) < 25);
    for (k = 0; k < 8; k++) c->mode[k] = !play[k] ? MD_REST : lg[k] ? MD_HOLD : MD_ONE;
    for (i = 0, rat = 0; i < 8 && rat < m[M_RATCH]; i++) {
        k = ord_m[i];
        if (play[k] && len[k] > dv[k]) { c->mode[k] = MD_RE; rat++; } }

    /* VEL: 8 values 50..127, DYN applied here */
    sq_rng_init(&r, seed[4], SALT_VEL);
    sq_vel_lane(&r, vel, 8);
    for (k = 0; k < 8; k++) c->vel[k] = sq_dyn_vel(vel[k], m[M_DYN]);

    c->tot = tot; c->off = m[M_OFFSET]; c->dir = m[M_DIR];
}

static uint8_t stage_loop_len(const void *cache) { return ((const StageCache *)cache)->tot; }

/* g = generator position. u = g + OFFSET, p = the pulse of the loop read at u (DIR: SP16 over TOTAL pulses), k = the stage that owns p. */
static void stage_step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)
{
    const StageCache *c = (const StageCache *)cache;
    uint32_t u = g + c->off;
    uint8_t p = sq_dir_map(u, c->tot, c->dir), k = 0, start, stop, pos, md, fire = 0, hold = 0;
    (void)hc;                                       /* STAGE does not read the chord: AF_CHORD is clear */
    while (k < 7 && p >= c->C[k]) k++;
    start = k ? c->C[k - 1] : 0; stop = c->C[k]; pos = (uint8_t)(p - start); md = c->mode[k];
    if (md == MD_ONE || md == MD_HOLD) fire = pos == 0;
    else if (md == MD_RE) fire = pos % c->div[k] == 0;
    if (fire && md == MD_HOLD) {                    /* hold = the pulses the traversal stays inside this stage, going forward (all of them with FWD, 1 with BWD) */
        hold = 1;
        for (uint8_t j = 1; pos + j < stop - start; j++) {
            if (sq_dir_map(u + j, c->tot, c->dir) != p + j) break;
            hold++; } }
    out->note = c->note[k];
    out->vel = c->vel[k];
    out->gate = 50;
    out->flags = (uint8_t)(fire ? PO_HIT : 0);
    if (hold) { out->flags |= PO_HOLD; }
    out->hold = hold;
    out->pad = 0;
}

/* Highest semitone: RANGE plus 1 (a snap up to the next scale tone), at least one octave for the chord stage's fold */
static uint8_t stage_hi(const uint8_t m[SQ_NMORPH]) { uint8_t h = (uint8_t)(m[M_RANGE] + 1); return h < 12 ? 12 : h; }

/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_STAGE = {
    .name = "STAGE", .id = SQ_ALGO_STAGE, .n_seed = 5, .flags = 0, .cache_bytes = 43,
    .knob_morph = { 0, 1, 7, 3, 4, 5, 6, 2 },
    .seed_k = { 1, 2, 3, 4, 5 },
    .morph = {
        { "DENS",   0,  8,  6, 1, 1 },
        { "TOTAL",  8, 64, 16, 1, 4 },
        { "RANGE",  0, 36, 12, 1, 4 },
        { "RATCH",  0,  8,  2, 1, 1 },
        { "OFFSET", 0, 63,  0, 1, 4 },
        { "RDIV",   1,  4,  4, 1, 1 },
        { "DYN",    0, 10, 10, 1, 1 },
        { "DIR",    0,  2,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {2}}, {1, {4}}, {1, {3}}, {1, {1}} },
    .sub = { "NOTES", "RESTS", "VELO", "RT+HLD", "LONG" },
    .build = stage_build, .loop_len = stage_loop_len, .step = stage_step, .hi = stage_hi, .sub_note = 0,
};
/* ALGODESC-END */
