/* SPDX-License-Identifier: GPL-3.0-only */
/* ACID (sting): a 16-step monophonic acid line. P2-008 prototype = the firmware file: P4-001 copies it, adds `static`
 * and wires the ALGOS[] row. Spec: ./SPEC.md (sections cited as S<n>). Pure: build() and step() use only their arguments
 * and const tables (ADR-0003 section 4). stdint only. */
#include "../common/sq_types.h"
#include "../common/sq_prims.h"

typedef struct {
    uint8_t  note[16];     /* final MIDI notes, sq_note applied at build (late quantise) */
    uint8_t  vel[16];
    uint8_t  gate[16];     /* % of one step: {30,70,100,127} mapped to {24,55,79,100} */
    uint16_t hit;          /* bit i = step i sounds */
    uint16_t slide;        /* bit i = step i is reached by a glide from the step played before it (the stored flag; the core decides if effective) */
    uint8_t  off, dir;     /* ROW 2 OFFSET and DIR are applied by step() (they read t) */
} AcidCache;               /* 54 B */
_Static_assert(sizeof(AcidCache) == 54 && sizeof(AcidCache) <= SQ_CACHE_MAX, "AcidCache");

#define M_DENS 0
#define M_PEDAL 1
#define M_JUMP 2
#define M_ACCNT 3
#define M_OFFSET 4
#define M_SLIDE 5
#define M_PUNCH 6
#define M_DIR 7

#define SALT_NOTES 0x5A17C3E1u
#define SALT_HITS  0x2C9E6B07u
#define SALT_OCT   0x71D4A5F3u
#define SALT_GATE  0x3B8F1D29u
#define SALT_ACCNT 0x68E2B94Du
#define SALT_SLIDE 0x4F6A9C15u

static const uint8_t BASE_P[16] = { 10,11,12,13,15,16,17,18,20,21,22,23,25,26,27,28 };      /* S4.3 JUMP: 10 + floor(20 i / 16) */
static const uint8_t PA[16]     = { 0,8,11,14,17,20,23,26,29,32,35,38,41,44,47,50 };         /* S4.4 GATE; index 0 unused (gate[0] has its own coin) */
static const uint8_t PB[16]     = { 0,11,13,14,16,18,20,22,24,26,28,31,33,35,38,40 };
static const uint8_t PS[16]     = { 0,20,10,25,8,20,10,25,8,20,10,25,8,20,10,30 };         /* S4.5 SLIDE table, percent */
static const uint8_t GATE_PCT[3] = { 24, 55, 100 };                                         /* 30 -> 24, 70 -> 55, 127 -> 100 ; 100 -> 79 on step 0 only */

static void acid_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache)
{
    AcidCache *c = (AcidCache *)cache;
    SqRng r;
    uint8_t tile[16], bag[12], ord_type[16], pitch[16], rank[16], ord_acc[16], i, n;
    uint16_t acc_mask;

    /* NOTES: motif of 3..5 pitch classes tiled over 16 steps and shuffled; then the TYPE replacement order */
    sq_rng_init(&r, seed[0], SALT_NOTES);
    n = (uint8_t)(3 + sq_rnd(&r, 3));
    for (i = 0; i < 12; i++) bag[i] = i;
    sq_shuffle(&r, bag, 12);
    for (i = 0; i < 16; i++) tile[i] = bag[i % n];
    sq_shuffle(&r, tile, 16);
    for (i = 0; i < 16; i++) ord_type[i] = i;
    sq_shuffle(&r, ord_type, 16);
    for (i = 0; i < 16; i++) pitch[i] = tile[i];
    for (i = 0; i < m[M_PEDAL] && i < 16; i++) pitch[ord_type[i]] = 0;        /* PEDAL: the first `typ` steps of the order become the root */

    /* HITS: rank order with step 0 first; DENS keeps the first `den` ranks */
    sq_rng_init(&r, seed[1], SALT_HITS);
    sq_rank_build(&r, rank, 16, 1);
    c->hit = 0;
    for (i = 0; i < 16; i++) if (rank[i] < m[M_DENS]) c->hit = (uint16_t)(c->hit | (1u << i));

    /* OCT: three draws per step, always 48 draws; JUMP scales the odds. Octave -1..+2 (clamped below at -1) */
    sq_rng_init(&r, seed[2], SALT_OCT);
    for (i = 0; i < 16; i++) {
        uint8_t u1 = (uint8_t)sq_rnd(&r, 100), u2 = (uint8_t)sq_rnd(&r, 100);
        int8_t dir = (int8_t)((int8_t)sq_rnd(&r, 3) - 1);
        uint16_t pe = (uint16_t)((BASE_P[i] * m[M_JUMP]) / 5);
        int8_t oct, k;
        if (pe > 100) pe = 100;
        k = (int8_t)((u1 < pe) + (u2 < pe));
        oct = (int8_t)(dir * k);
        if (oct < -1) oct = -1;
        c->note[i] = sq_note(g, (int16_t)(pitch[i] + 12 * oct)); }

    /* GATE: step 0 has its own coin (100 or 70), steps 1..15 two weighted coins */
    sq_rng_init(&r, seed[3], SALT_GATE);
    c->gate[0] = sq_rnd(&r, 2) == 0 ? 79 : GATE_PCT[1];
    for (i = 1; i < 16; i++) {
        uint8_t s = (uint8_t)(sq_bern(&r, PA[i]) + sq_bern(&r, PB[i]));
        c->gate[i] = GATE_PCT[s]; }

    /* ACCNT: a seeded order over all 16 steps; the first `acc` HIT steps in that order are accented */
    sq_rng_init(&r, seed[4], SALT_ACCNT);
    for (i = 0; i < 16; i++) ord_acc[i] = i;
    sq_shuffle(&r, ord_acc, 16);
    acc_mask = sq_accent_pick(ord_acc, 16, c->hit, m[M_ACCNT]);
    {   uint16_t hv = (uint16_t)(80u + m[M_PUNCH]);
        if (hv > 127) hv = 127;
        for (i = 0; i < 16; i++) c->vel[i] = ((acc_mask >> i) & 1u) ? (uint8_t)hv : 80; }

    /* SLIDE: 16 draws, flag = draw < table * SLIDE / 4 (capped at 100 %); a flag on a step that does not follow a hit is stored and silent */
    sq_rng_init(&r, seed[5], SALT_SLIDE);
    c->slide = 0;
    for (i = 0; i < 16; i++) {
        uint8_t u = (uint8_t)sq_rnd(&r, 100);
        uint16_t pe = (uint16_t)((PS[i] * m[M_SLIDE]) / 4);
        if (pe > 100) pe = 100;
        if (u < pe) c->slide = (uint16_t)(c->slide | (1u << i)); }

    c->off = m[M_OFFSET];
    c->dir = m[M_DIR];
}

static uint8_t acid_loop_len(const void *cache) { (void)cache; return 16; }

/* g = generator position (free-running). OFFSET rotates and DIR reads the loop forward, backward or ping-pong (SP16). */
static void acid_step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)
{
    const AcidCache *c = (const AcidCache *)cache;
    uint8_t i = sq_dir_map(g + c->off, 16, c->dir);
    (void)hc;                                       /* ACID does not read the chord: AF_CHORD is clear */
    out->note = c->note[i];
    out->vel = c->vel[i];
    out->gate = c->gate[i];
    out->flags = (uint8_t)((((c->hit >> i) & 1u) ? PO_HIT : 0) | (((c->slide >> i) & 1u) ? PO_SLIDE_IN : 0));
    out->hold = 0;
    out->pad = 0;
}

/* The mod lanes (MODULATION 4): lane A is the cell of the gate position i (the index the accent and slide flags use), leaning on this step's velocity;
 * lane B sweeps once per 16 steps, read at the same i so OFFSET and DIR move the filter with the notes. */
static void acid_mod(const void *cache, uint32_t g, const HarmChord *hc, const PulseOut *o, ModIdx *out)
{
    const AcidCache *c = (const AcidCache *)cache;
    uint8_t i = sq_dir_map(g + c->off, 16, c->dir);
    (void)hc;
    out->a = i; out->vel = o->vel; out->b_cell = 0; out->b_lfo = 1; out->t = i; out->period = 16;
}

/* Highest semitone above the base: pitch class 11 (or 12, the next key note, when the scale snaps up) plus octave +2 once JUMP > 0 */
static uint8_t acid_hi(const uint8_t m[SQ_NMORPH]) { return (uint8_t)(m[M_JUMP] ? 36 : 12); }

/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_ACID = {
    .name = "ACID", .id = SQ_ALGO_ACID, .n_seed = 6, .flags = AF_PAIRING, .cache_bytes = 54,
    .knob_morph = { 0, 5, 2, 3, 4, 1, 6, 7 },
    .seed_k = { 1, 2, 3, 4, 5, 6 },
    .morph = {
        { "DENS",   0, 16, 12, 1, 1 },
        { "PEDAL",  0, 12,  6, 1, 1 },
        { "JUMP",   0, 20,  5, 1, 1 },
        { "ACCNT",  0,  8,  3, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "SLIDE",  0,  8,  4, 1, 1 },
        { "PUNCH",  0, 47, 40, 1, 4 },
        { "DIR",    0,  2,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {4}}, {2, {3, 5}}, {2, {2, SQ_SEED_MOD}} },
    .sub = { "NOTES", "HITS", "ACCNT", "GT+SLD", "OCTAVE" },
    .build = acid_build, .loop_len = acid_loop_len, .step = acid_step, .hi = acid_hi, .sub_note = 0, .mod = acid_mod,
};
/* ALGODESC-END */
