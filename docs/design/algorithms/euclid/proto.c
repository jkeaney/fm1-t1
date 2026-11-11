/* SPDX-License-Identifier: GPL-3.0-only */
/* EUCLID (t1): Bjorklund hits, a contour melody that follows the hit ordinal, a seeded accent count and seeded ratchets.
 * P2-016 prototype = the firmware file: P4 copies it, adds `static` and wires the ALGOS[1] row. Spec: ./SPEC.md (sections
 * cited as S<n>). Pure: build() and step() use only their arguments and const tables (ADR-0003 section 4). stdint only.
 * eu_bjorklund / eu_rotate are t1_euclidean_bjorklund / t1_euclidean_rotate (firmware/src/torso_t1_algo.c) with the same
 * output for n <= 16 (SP11); eu_phrase is t1_harmony_eval_phrase for shapes 1..6 with the BUG-001 fix (F3). */
#include "../common/sq_types.h"
#include "../common/sq_prims.h"

typedef struct {
    uint8_t  note[16];     /* final MIDI note per step position; non-hit positions carry the contour of the nearest hit at or before them */
    uint16_t hit;          /* bit i = step i sounds (Bjorklund, rotated) */
    uint16_t acc;          /* bit i = step i is an accented hit */
    uint8_t  rp[8];        /* 4 bits per step i (low nibble = even i): bits 0..1 repeats-1, bit 2 pace decelerates; 0 on a rest */
    uint8_t  ho[8];        /* 4 bits per step i (low nibble = even i): the ordinal (0..15) of the hit that step carries; a rest carries the ordinal of the hit before it (mod lane A) */
    uint8_t  len;          /* LEN: the generator's own cycle, 2..16 */
    uint8_t  hv;           /* accented velocity min(127, 80 + PUNCH) */
} EuclidCache;             /* 38 B */
_Static_assert(sizeof(EuclidCache) == 38 && sizeof(EuclidCache) <= SQ_CACHE_MAX, "EuclidCache");

#define M_HITS 0
#define M_LEN 1
#define M_SPAN 2
#define M_ACCNT 3
#define M_OFFSET 4
#define M_RATCH 5
#define M_PUNCH 6
#define M_CURVE 7

#define SALT_ROT    0x1E3779B9u
#define SALT_NOTES  0x2545F491u
#define SALT_ACCENT 0x1B873593u
#define SALT_RATCH  0x3C6EF372u

static uint8_t eu_popcount(uint32_t x) { uint8_t n = 0; while (x) { n = (uint8_t)(n + (x & 1u)); x >>= 1; } return n; }

/* SP11: Bjorklund's distribution, as t1_euclidean_bjorklund (patterns of at most 16 bits, groups capped at 16). Bit k = step k. */
static uint32_t eu_bjorklund(uint8_t steps, uint8_t pulses)
{
    struct { uint32_t pattern; uint8_t len; } groups[16];
    uint8_t ones, zeros, i, total, bit = 0;
    uint32_t result = 0;
    if (steps == 0 || pulses == 0) return 0;
    if (steps > 16) steps = 16;
    if (pulses >= steps) return (1u << steps) - 1u;
    ones = pulses; zeros = (uint8_t)(steps - pulses);
    for (i = 0; i < ones; i++) { groups[i].pattern = 1u; groups[i].len = 1; }
    for (i = 0; i < zeros; i++) { groups[ones + i].pattern = 0u; groups[ones + i].len = 1; }
    while (zeros > 1) {
        uint8_t minc = ones < zeros ? ones : zeros;
        for (i = 0; i < minc; i++) {
            groups[i].pattern = (groups[i].pattern << groups[ones + i].len) | groups[ones + i].pattern;
            groups[i].len = (uint8_t)(groups[i].len + groups[ones + i].len); }
        if (ones >= zeros) {
            uint8_t rem = (uint8_t)(ones - zeros);
            for (i = 0; i < rem; i++) groups[minc + i] = groups[zeros + i];
            ones = minc; zeros = rem;
        } else {
            uint8_t rem = (uint8_t)(zeros - ones);
            for (i = 0; i < rem; i++) groups[minc + i] = groups[ones + minc + i];
            zeros = rem; } }
    total = (uint8_t)(ones + zeros);
    for (i = 0; i < total && bit < steps; i++) {
        for (int b = groups[i].len - 1; b >= 0 && bit < steps; b--, bit++)
            if ((groups[i].pattern >> b) & 1u) result |= 1u << bit; }
    return result;
}

/* t1_euclidean_rotate: bit k moves to bit k - r (wrapping), r in 0..steps-1 */
static uint32_t eu_rotate(uint32_t mask, uint8_t steps, uint8_t r)
{   uint32_t full = (1u << steps) - 1u;
    mask &= full;
    if (r == 0) return mask;
    return ((mask >> r) | (mask << (steps - r))) & full; }

/* t1_harmony_eval_phrase, shapes 1 SINE 2 TRI 3 SAW+ 4 SAW- 5 ARCH 6 INV (F3 fixed). Returns 0 when range is 0 or total <= 1. */
static int16_t eu_phrase(uint8_t shape, uint8_t range, uint8_t idx, uint8_t total)
{   int32_t mx = range, delta = 0, norm, quad, half;
    if (range == 0 || total <= 1) return 0;
    idx = (uint8_t)(idx % total);
    switch (shape) {
    case 1: { int32_t phase = ((int32_t)idx * 360) / total;
              if (phase < 180) delta = (phase * (180 - phase) * mx) / 8100;
              else { int32_t p = phase - 180; delta = -(p * (180 - p) * mx) / 8100; }
              break; }
    case 2: half = total / 2; if (half == 0) half = 1;
            delta = idx <= half ? ((int32_t)idx * mx) / half : (((int32_t)total - idx) * mx) / half;
            break;
    case 3: delta = ((int32_t)idx * mx) / (total - 1); break;
    case 4: delta = (((int32_t)total - 1 - idx) * mx) / (total - 1); break;
    case 5: norm = 2 * idx - total; quad = (norm * norm * mx) / ((int32_t)total * total); delta = mx - quad; break;
    case 6: norm = 2 * idx - total; quad = (norm * norm * mx) / ((int32_t)total * total); delta = quad; break;
    default: break; }
    return (int16_t)delta;
}

static void euclid_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache)
{
    EuclidCache *c = (EuclidCache *)cache;
    SqRng r;
    uint8_t *z = (uint8_t *)cache, ord[16], reps[16], dec[16], jit[16], i, n, p, H, shape;
    uint32_t m0, m1, R, r16, accm, p2, p3;

    for (i = 0; i < sizeof *c; i++) z[i] = 0;                                 /* every byte of the cache is defined */
    n = m[M_LEN]; if (n < 2) n = 2; if (n > 16) n = 16;
    p = m[M_HITS] < n ? m[M_HITS] : n;
    m0 = eu_bjorklund(n, p);
    H = eu_popcount(m0);

    /* ROT: one draw when the seed is not 0 (0 = no rotation); OFFSET adds on top */
    sq_rng_init(&r, seed[0], SALT_ROT);
    r16 = seed[0] ? sq_rnd(&r, 65536) : 0;
    R = ((r16 % n) + m[M_OFFSET]) % n;
    m1 = eu_rotate(m0, n, (uint8_t)R);
    c->hit = (uint16_t)m1;

    /* NOTES: always 1 + 16 draws, so the stream is the same for every CURVE and HITS */
    sq_rng_init(&r, seed[1], SALT_NOTES);
    shape = (uint8_t)(1 + sq_rnd(&r, 6));
    if (m[M_CURVE]) shape = m[M_CURVE];
    for (i = 0; i < 16; i++) jit[i] = (uint8_t)sq_rnd(&r, 5);                  /* jitter = jit - 2 */

    /* ACCENT: a seeded order of 16 ordinals; the first ACCNT of them that exist (< H) are accented */
    sq_rng_init(&r, seed[2], SALT_ACCENT);
    for (i = 0; i < 16; i++) ord[i] = i;
    sq_shuffle(&r, ord, 16);
    accm = sq_accent_pick(ord, 16, (uint16_t)(H >= 16 ? 0xFFFFu : ((1u << H) - 1u)), m[M_ACCNT]);

    /* RATCH: always two draws per ordinal; ROW 2 RATCH scales the odds (4 = 15 % and 5 %) */
    p2 = (15u * m[M_RATCH]) / 4u; p3 = (20u * m[M_RATCH]) / 4u;
    sq_rng_init(&r, seed[3], SALT_RATCH);
    for (i = 0; i < 16; i++) {
        uint32_t u = sq_rnd(&r, 100), d = sq_rnd(&r, 3);
        reps[i] = (uint8_t)(u < p2 ? 2 : u < p3 ? 3 : 1);
        dec[i] = (uint8_t)(d == 2); }

    for (i = 0; i < n; i++) {
        uint8_t j = (uint8_t)((i + R) % n), cnt = eu_popcount(m0 & ((2u << j) - 1u)), h = (uint8_t)(cnt ? cnt - 1 : 0);
        int16_t s = (int16_t)(eu_phrase(shape, m[M_SPAN], h, H) + (m[M_SPAN] ? (int16_t)jit[h] - 2 : 0));
        c->note[i] = sq_note(g, s < 0 ? 0 : s);                                 /* the line never goes below the root */
        c->ho[i >> 1] = (uint8_t)(c->ho[i >> 1] | ((h & 15u) << ((i & 1u) * 4)));
        if ((m1 >> i) & 1u) {
            uint8_t nib = (uint8_t)((reps[h] - 1) | (dec[h] << 2));
            if ((accm >> h) & 1u) c->acc = (uint16_t)(c->acc | (1u << i));
            c->rp[i >> 1] = (uint8_t)(c->rp[i >> 1] | (nib << ((i & 1u) * 4))); } }
    c->len = n;
    c->hv = (uint8_t)(80u + m[M_PUNCH] > 127u ? 127u : 80u + m[M_PUNCH]);
}

static uint8_t euclid_loop_len(const void *cache) { return ((const EuclidCache *)cache)->len; }

/* g = generator position; the cycle is LEN, so a page that reads the same block repeats it and LEN 12 drifts across 32 steps */
static void euclid_step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)
{
    const EuclidCache *c = (const EuclidCache *)cache;
    uint8_t i = (uint8_t)(g % c->len), hit = (uint8_t)((c->hit >> i) & 1u), nib = (uint8_t)((c->rp[i >> 1] >> ((i & 1u) * 4)) & 15u);
    (void)hc;                                       /* EUCLID does not read the chord: AF_CHORD is clear */
    out->note = c->note[i];
    out->vel = ((c->acc >> i) & 1u) ? c->hv : 80;
    out->gate = 50;
    out->flags = hit ? (uint8_t)(PO_HIT | ((nib & 3u) << PO_REP_SHIFT) | ((nib & 4u) ? PO_PACE_DEC : 0)) : 0;
    out->hold = 0;
    out->pad = 0;
}

/* The mod lanes (MODULATION 4): lane A is the cell of the hit ORDINAL (the k-th hit gets the k-th cell, so the value stays with the note when OFFSET rotates;
 * a rest reads the ordinal of the hit before it, like its contour pitch); lane B sweeps once per LEN steps. */
static void euclid_mod(const void *cache, uint32_t g, const HarmChord *hc, const PulseOut *o, ModIdx *out)
{
    const EuclidCache *c = (const EuclidCache *)cache;
    uint8_t i = (uint8_t)(g % c->len);
    (void)hc;
    out->a = (uint8_t)((c->ho[i >> 1] >> ((i & 1u) * 4)) & 15u); out->vel = o->vel; out->b_cell = 0; out->b_lfo = 1; out->t = i; out->period = c->len;
}

/* Highest semitone: SPAN + 2 (jitter) + 1 (a snap up to the next scale tone, never more than 1: no scale has a gap over 3), at least one octave for the chord stage's fold */
static uint8_t euclid_hi(const uint8_t m[SQ_NMORPH]) { uint8_t h = (uint8_t)(m[M_SPAN] + 3); return h < 12 ? 12 : h; }

/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_EUCLID = {
    .name = "EUCLID", .id = SQ_ALGO_EUCLID, .n_seed = 4, .flags = 0, .cache_bytes = 38,
    .knob_morph = { 0, 4, 5, 3, 1, 2, 6, 7 },
    .seed_k = { 0xFF, 1, 2, 3 },
    .morph = {
        { "HITS",   0, 16,  5, 1, 1 },
        { "LEN",    2, 16, 16, 1, 1 },
        { "SPAN",   0, 24, 12, 1, 1 },
        { "ACCNT",  0,  8,  2, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "RATCH",  0,  8,  4, 1, 1 },
        { "PUNCH",  0, 47, 40, 1, 4 },
        { "CURVE",  0,  6,  0, 1, 1 },
    },
    .layer = { {1, {1}}, {1, {0}}, {1, {2}}, {1, {3}}, {1, {SQ_SEED_MOD}} },
    .sub = { "NOTES", "ROTATE", "ACCNT", "RATCH", "MOD" },
    .build = euclid_build, .loop_len = euclid_loop_len, .step = euclid_step, .hi = euclid_hi, .sub_note = 0, .mod = euclid_mod,
};
/* ALGODESC-END */
