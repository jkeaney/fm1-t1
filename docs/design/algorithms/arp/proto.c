/* SPDX-License-Identifier: GPL-3.0-only */
/* ARP: an arpeggio over the HARMONY chord progression. P2-017 prototype = the firmware file: P4 copies it, adds `static` and wires
 * the ALGOS[4] row. Spec: ./SPEC.md (sections cited as S<n>). Pure: build() and step() use only their arguments and const tables
 * (ADR-0003 section 4); the chord arrives as the third argument of step() (AF_CHORD, S6.10). stdint only.
 * The note of a step is `base + s`: chord tones are scale tones, so SP6 would leave them alone, and `base` = 36 + key + 12 * octave is
 * kept in the cache instead of the whole SeqGlobal. */
#include "../common/sq_types.h"
#include "../common/sq_prims.h"

typedef struct {
    uint16_t hit;          /* bit c = cell c sounds (rank < DENS) */
    uint8_t  vel[16];      /* step velocities 50..127 */
    uint8_t  gm[16];       /* note-length factors 2..6 (x LEGATO length / 4) */
    uint16_t hop;          /* bit n = the figure note with counter n mod 16 jumps an octave (SPAN >= 2) */
    uint16_t s_notes;      /* RAND's permutation seed */
    uint16_t s_rot;        /* the figure's start phase */
    uint8_t  shp, spn, rat, dir, off, lg, dyn;
    uint8_t  base;         /* 36 + key + 12 * octave */
} ArpCache;                /* 48 B */
_Static_assert(sizeof(ArpCache) == 48 && sizeof(ArpCache) <= SQ_CACHE_MAX, "ArpCache");

#define M_DENS 0
#define M_SHAPE 1
#define M_SPAN 2
#define M_RATE 3
#define M_OFFSET 4
#define M_LEGATO 5
#define M_DYN 6
#define M_DIR 7

#define SH_LINE 0
#define SH_SKIP 1
#define SH_THUMB 2
#define SH_OCTV 3
#define SH_REPT 4
#define SH_WAVE 5
#define D_UP 0
#define D_DOWN 1
#define D_UPDN 2
#define D_RAND 3
#define D_ASPL 4
#define D_CHORD 5

#define SALT_HITS 0x6D3B27A1u
#define SALT_VEL  0x13F5C8D9u
#define SALT_GATE 0x5E92B407u
#define SALT_HOP  0x4A1C9E35u

static const uint8_t RATE_STP[6] = { 4, 2, 1, 1, 1, 1 };   /* steps per note slot */
static const uint8_t RATE_SUB[6] = { 1, 1, 1, 2, 3, 4 };   /* notes per step */

static void arp_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache)
{
    ArpCache *c = (ArpCache *)cache;
    SqRng r;
    uint8_t *z = (uint8_t *)cache, rank[16], a[16], i;

    for (i = 0; i < sizeof *c; i++) z[i] = 0;                                 /* every byte of the cache is defined */

    /* HITS: cell 0 ranks first, the other 15 cells are shuffled (SP2); DENS keeps the first `den` ranks */
    sq_rng_init(&r, seed[1], SALT_HITS);
    sq_rank_build(&r, rank, 16, 1);
    for (i = 0; i < 16; i++) if (rank[i] < m[M_DENS]) c->hit = (uint16_t)(c->hit | (1u << i));

    /* VEL: 16 values 50..127 (SP4); DYN is applied by step() */
    sq_rng_init(&r, seed[2], SALT_VEL);
    sq_vel_lane(&r, c->vel, 16);

    /* GATE: 16 factors 2..6, mean 4 */
    sq_rng_init(&r, seed[3], SALT_GATE);
    for (i = 0; i < 16; i++) c->gm[i] = (uint8_t)(2 + sq_rnd(&r, 5));

    /* HOP: 4 of the 16 counter positions jump an octave; seed 0 = no hops, so a fresh ARP is the plain figure */
    if (seed[0]) {
        sq_rng_init(&r, seed[0], SALT_HOP);
        for (i = 0; i < 16; i++) a[i] = i;
        sq_shuffle(&r, a, 16);
        for (i = 0; i < 4; i++) c->hop = (uint16_t)(c->hop | (1u << a[i])); }

    c->s_notes = seed[0]; c->s_rot = seed[4];
    c->shp = m[M_SHAPE]; c->spn = m[M_SPAN]; c->rat = m[M_RATE]; c->dir = m[M_DIR];
    c->off = m[M_OFFSET]; c->lg = m[M_LEGATO]; c->dyn = m[M_DYN];
    c->base = (uint8_t)(36 + g->s.key + 12 * g->s.octave);
}

static uint8_t arp_loop_len(const void *cache) { (void)cache; return 64; }     /* no shorter period holds for every morph (S6.3 step 2) */

/* S6.3 step 2: the base length LB of one pass over the pool and the figure length F of the shape */
static uint16_t arp_lb(const ArpCache *c, uint8_t nt)
{   uint16_t p = (uint16_t)(nt * c->spn);
    if (c->dir == D_UPDN) return p > 1 ? (uint16_t)(2 * p - 2) : 1;
    return p; }

static uint16_t arp_f(const ArpCache *c, uint16_t lb)
{   switch (c->shp) {
    case SH_SKIP: case SH_OCTV: case SH_REPT: return (uint16_t)(2 * lb);
    case SH_THUMB: return (uint16_t)(2 * (lb > 1 ? lb - 1 : 1));
    case SH_WAVE: return (uint16_t)(3 * lb);
    default: return lb; } }

/* S6.4: the tone list T[i] of the direction (semitones above the key, before + 12 * (j div nt)) */
static int16_t arp_tone(const HarmChord *hc, uint8_t dir, uint8_t i)
{   uint8_t nt = hc->n ? hc->n : 1;
    if (dir == D_ASPL) {                           /* the voiced notes rotated so the root comes first, wrapped notes an octave up */
        uint8_t r = 0, k;
        for (k = 0; k < nt; k++) if ((((hc->vt[k] - hc->root) % 12) + 12) % 12 == 0) { r = k; break; }
        return (int16_t)(hc->vt[(r + i) % nt] + 12 * ((r + i) / nt)); }
    if (dir == D_CHORD) {                          /* stack order, root position: sort rel = (vt - root) mod 12 ascending */
        uint8_t rel[HARM_MAXV], k, j;
        for (k = 0; k < nt; k++) rel[k] = (uint8_t)((((hc->vt[k] - hc->root) % 12) + 12) % 12);
        for (k = 1; k < nt; k++) { uint8_t v = rel[k]; for (j = k; j > 0 && rel[j - 1] > v; j--) rel[j] = rel[j - 1]; rel[j] = v; }
        return (int16_t)(hc->root + rel[i % nt]); }
    return hc->vt[i % nt]; }

/* S6.3 step 4..6: the semitone of note counter n (before the fold's base), and its figure position m */
static int16_t arp_note_n(const ArpCache *c, const HarmChord *hc, uint32_t n, uint8_t *m_out)
{
    uint8_t nt = hc->n ? hc->n : 1;
    uint16_t lb = arp_lb(c, nt), f = arp_f(c, lb), p = (uint16_t)(nt * c->spn), b, j;
    uint32_t ntot = n + (c->s_rot % f), m = ntot % f, y = ntot / f;
    uint8_t home = 0, octv = 0;
    int16_t s;
    switch (c->shp) {
    case SH_SKIP: b = (uint16_t)((m & 1u) ? (m / 2 + 2) % lb : (m / 2) % lb); break;
    case SH_THUMB: if (!(m & 1u)) { home = 1; b = 0; } else b = lb > 1 ? (uint16_t)(1 + (m / 2) % (lb - 1u)) : 0; break;
    case SH_OCTV: b = (uint16_t)(m / 2); octv = (uint8_t)(m & 1u); break;
    case SH_REPT: b = (uint16_t)(m / 2); break;
    case SH_WAVE: b = (uint16_t)((m / 3 + m % 3) % lb); break;
    default: b = (uint16_t)m; break; }
    switch (c->dir) {
    case D_DOWN: j = (uint16_t)(p - 1 - b); break;
    case D_UPDN: j = b < p ? b : (uint16_t)(2 * p - 2 - b); break;
    case D_RAND: { uint8_t perm[24];
        uint32_t x = sq_mix(((uint32_t)c->s_notes * 0x9E3779B1u) ^ (y * 0x85EBCA6Bu));
        sq_seeded_perm(perm, (uint8_t)p, x ? x : 1u);
        j = perm[b % p]; break; }
    default: j = b; break; }
    s = (int16_t)(arp_tone(hc, c->dir, (uint8_t)(j % nt)) + 12 * (j / nt));
    if (octv) s = (int16_t)(s + 12);
    if (home) s = hc->drone >= 0 ? hc->drone : hc->vt[0];
    if (c->spn >= 2 && ((c->hop >> (n & 15u)) & 1u)) s = (int16_t)(s + 12);
    while (s > 60) s = (int16_t)(s - 12);                                      /* the 5-octave ceiling */
    *m_out = (uint8_t)m;
    return s;
}

static uint8_t arp_midi(const ArpCache *c, int16_t s) { int16_t n = (int16_t)(c->base + s); return (uint8_t)(n < 0 ? 0 : n > 127 ? 127 : n); }

/* g = generator position (unreduced); hc = the chord of the step's start tick (never NULL: AF_CHORD) */
static void arp_step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)
{
    const ArpCache *c = (const ArpCache *)cache;
    uint8_t stp = RATE_STP[c->rat], sub = RATE_SUB[c->rat], cell = (uint8_t)(g & 15u), m, hit;
    uint32_t u = g + c->off, n0 = (u / stp) * sub;
    uint16_t gt;
    int16_t s = arp_note_n(c, hc, n0, &m);
    uint8_t v;
    hit = (uint8_t)(((c->hit >> cell) & 1u) && (u % stp) == 0);
    v = sq_dyn_vel(c->vel[cell], c->dyn);
    if (m == 0) v = (uint8_t)(v + 12 > 127 ? 127 : v + 12);                     /* the figure start is accented */
    gt = (uint16_t)(((20u + 10u * c->lg) * c->gm[cell]) / 4u);
    out->note = arp_midi(c, s);
    out->vel = v;
    out->gate = (uint8_t)(gt < 5 ? 5 : gt > 100 ? 100 : gt);
    out->flags = hit ? (uint8_t)(PO_HIT | ((sub - 1u) << PO_REP_SHIFT)) : 0;
    out->hold = 0;
    out->pad = 0;
}

/* the pitch of repeat k (1..3) inside a step: the next figure notes (AM-2). The core calls it for any PO_REP, also from a step RATCH edit. */
static uint8_t arp_sub_note(const void *cache, uint32_t g, uint8_t k, const HarmChord *hc)
{
    const ArpCache *c = (const ArpCache *)cache;
    uint8_t stp = RATE_STP[c->rat], sub = RATE_SUB[c->rat], m;
    uint32_t u = g + c->off, n0 = (u / stp) * sub;
    return arp_midi(c, arp_note_n(c, hc, n0 + k, &m));
}

/* The mod lanes (MODULATION 4): lane A is the cell of the FIGURE position of the step's note (each arpeggio note has its own value, so the filter plays the
 * figure); lane B sweeps once per channel LENGTH (period 0: the core fills it). */
static void arp_mod(const void *cache, uint32_t g, const HarmChord *hc, const PulseOut *o, ModIdx *out)
{
    const ArpCache *c = (const ArpCache *)cache;
    uint8_t stp = RATE_STP[c->rat], sub = RATE_SUB[c->rat], m;
    uint32_t u = g + c->off;
    (void)arp_note_n(c, hc, (u / stp) * sub, &m);
    out->a = m; out->vel = o->vel; out->b_cell = 0; out->b_lfo = 1; out->t = (uint16_t)g; out->period = 0;
}

/* The highest semitone: the fold ceiling (chord tones are scale tones, so no snap is added) */
static uint8_t arp_hi(const uint8_t m[SQ_NMORPH]) { (void)m; return 60; }

/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_ARP = {
    .name = "ARP", .id = SQ_ALGO_ARP, .n_seed = 5, .flags = AF_CHORD, .cache_bytes = 48,
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 },
    .seed_k = { 0xFF, 2, 3, 4, 0xFF },
    .morph = {
        { "DENS",   0, 16, 12, 1, 1 },
        { "SHAPE",  0,  5,  0, 1, 1 },
        { "SPAN",   1,  4,  2, 1, 1 },
        { "RATE",   0,  5,  2, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "LEGATO", 0,  8,  3, 1, 1 },
        { "DYN",    0, 10, 10, 1, 1 },
        { "DIR",    0,  5,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {2}}, {1, {3}}, {2, {4, SQ_SEED_MOD}} },
    .sub = { "VARY", "RESTS", "VELO", "LENGTH", "PHASE" },
    .build = arp_build, .loop_len = arp_loop_len, .step = arp_step, .hi = arp_hi, .sub_note = arp_sub_note, .mod = arp_mod,
};
/* ALGODESC-END */
