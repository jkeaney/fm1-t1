/* SPDX-License-Identifier: GPL-3.0-only */
/* SNAKE (snake): three lanes (gates, pitch, velocity) read through one of 14 traversals of a 4x4 grid, each lane with its own
 * length, so the pattern is a polymeter. P2-016 prototype = the firmware file: P4 copies it, adds `static` and wires the
 * ALGOS[2] row. Spec: ./SPEC.md (sections cited as S<n>). Pure: build() and step() use only their arguments and const
 * tables (ADR-0003 section 4). stdint only. */
#include "../common/sq_types.h"
#include "../common/sq_prims.h"

typedef struct {
    uint8_t  order[16];    /* order[s] = the grid cell that speaks at traversal position s (from PATH) */
    uint8_t  note[16];     /* final MIDI note per CELL, sq_note applied at build (late quantise) */
    uint8_t  vel[16];      /* velocity per CELL with DYN already applied */
    uint8_t  gb[2];        /* little-endian u16 (kept as bytes so the struct is 55 B, unpadded): bit j = gate lane position j sounds, for j < gl (DENS, GLEN and the never-silent rule applied) */
    uint8_t  len, vlen;    /* pitch lane length LEN; velocity lane length (POLY) */
    uint8_t  gl, off, dir; /* GLEN, OFFSET and DIR are applied by step() (they read t) */
} SnakeCache;              /* 55 B */
_Static_assert(sizeof(SnakeCache) == 55 && sizeof(SnakeCache) <= SQ_CACHE_MAX, "SnakeCache");

#define M_DENS 0
#define M_LEN 1
#define M_RANGE 2
#define M_PATH 3
#define M_OFFSET 4
#define M_GLEN 5
#define M_DYN 6
#define M_DIR 7

#define SALT_NOTES 0x4E2B9A71u
#define SALT_GATES 0x1F6D83C5u
#define SALT_VEL   0x7B30E5A9u
#define SALT_POLY  0x29C4F18Du

/* the 14 traversals (snake-algorithms 03 section 3.3): SHAPE[n-1][k] = the 1-based step at which cell k plays */
static const uint8_t SHAPE[14][16] = {
    { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16},
    {13,14,15,16, 9,10,11,12, 5, 6, 7, 8, 1, 2, 3, 4},
    { 1, 5, 9,13, 2, 6,10,14, 3, 7,11,15, 4, 8,12,16},
    { 4, 8,12,16, 3, 7,11,15, 2, 6,10,14, 1, 5, 9,13},
    { 1, 8, 9,16, 2, 7,10,15, 3, 6,11,14, 4, 5,12,13},
    { 4, 5,12,13, 3, 6,11,14, 2, 7,10,15, 1, 8, 9,16},
    { 1, 2, 3, 4, 8, 7, 6, 5, 9,10,11,12,16,15,14,13},
    { 4, 3, 2, 1, 5, 6, 7, 8,12,11,10, 9,13,14,15,16},
    { 1, 2, 6, 7, 3, 5, 8,13, 4, 9,12,14,10,11,15,16},
    {10,11,15,16, 4, 9,12,14, 3, 5, 8,13, 1, 2, 6, 7},
    { 1, 2, 3, 4,12,13,14, 5,11,16,15, 6,10, 9, 8, 7},
    { 4, 3, 2, 1, 5,14,13,12, 6,15,16,11, 7, 8, 9,10},
    {10, 9, 8, 7,11,16,15, 6,12,13,14, 5, 1, 2, 3, 4},
    { 7, 8, 9,10, 6,15,16,11, 5,14,13,12, 4, 3, 2, 1},
};

static void snake_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache)
{
    SnakeCache *c = (SnakeCache *)cache;
    SqRng r;
    uint8_t *z = (uint8_t *)cache, rank[16], k, j, shp, gl;
    uint16_t any = 0;

    for (k = 0; k < sizeof *c; k++) z[k] = 0;                                 /* every byte of the cache is defined */

    /* PATH: order[SHAPE[shp][k] - 1] = k */
    shp = m[M_PATH]; if (shp < 1) shp = 1; if (shp > 14) shp = 14;
    for (k = 0; k < 16; k++) c->order[SHAPE[shp - 1][k] - 1] = k;

    /* NOTES: 16 raw values (SP7 span, then the late quantise) */
    sq_rng_init(&r, seed[0], SALT_NOTES);
    for (k = 0; k < 16; k++) c->note[k] = sq_note(g, sq_span((uint8_t)sq_rnd(&r, 256), m[M_RANGE]));

    /* GATES: rank order of the 16 cells; DENS keeps the first `den` ranks (no fixed first cell). GLEN reads the first `gl` positions of the traversal */
    sq_rng_init(&r, seed[1], SALT_GATES);
    sq_rank_build(&r, rank, 16, 0);
    gl = m[M_GLEN]; if (gl < 1) gl = 1; if (gl > 16) gl = 16;
    for (j = 0; j < gl; j++) if (rank[c->order[j]] < m[M_DENS]) any = (uint16_t)(any | (1u << j));
    if (m[M_DENS] >= 1 && !any) any = 1;                                       /* a short lane never goes silent (DENS 0 = KEYS ONLY forces nothing) */
    c->gb[0] = (uint8_t)any; c->gb[1] = (uint8_t)(any >> 8);

    /* VEL: 16 values 50..127, DYN applied here */
    sq_rng_init(&r, seed[2], SALT_VEL);
    sq_vel_lane(&r, c->vel, 16);
    for (k = 0; k < 16; k++) c->vel[k] = sq_dyn_vel(c->vel[k], m[M_DYN]);

    /* POLY: the velocity lane length */
    sq_rng_init(&r, seed[3], SALT_POLY);
    c->vlen = (uint8_t)(3 + sq_rnd(&r, 10));

    c->len = m[M_LEN] < 1 ? 1 : m[M_LEN] > 16 ? 16 : m[M_LEN];
    c->gl = gl; c->off = m[M_OFFSET]; c->dir = m[M_DIR];
}

static uint8_t snake_loop_len(const void *cache) { (void)cache; return 16; }

/* g = generator position. u = g + OFFSET drives all three lanes; DIR reads the pitch lane forward, backward or ping-pong (SP16). */
static void snake_step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)
{
    const SnakeCache *c = (const SnakeCache *)cache;
    uint32_t u = g + c->off;
    uint8_t hit = (uint8_t)(((c->gb[0] | (c->gb[1] << 8)) >> (u % c->gl)) & 1u);
    (void)hc;                                       /* SNAKE does not read the chord: AF_CHORD is clear */
    out->note = c->note[c->order[sq_dir_map(u, c->len, c->dir)]];
    out->vel = c->vel[c->order[u % c->vlen]];
    out->gate = 50;
    out->flags = hit ? PO_HIT : 0;
    out->hold = 0;
    out->pad = 0;
}

/* Highest semitone: RANGE plus 1 (a snap up to the next scale tone), at least one octave for the chord stage's fold */
static uint8_t snake_hi(const uint8_t m[SQ_NMORPH]) { uint8_t h = (uint8_t)(m[M_RANGE] + 1); return h < 12 ? 12 : h; }

/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_SNAKE = {
    .name = "SNAKE", .id = SQ_ALGO_SNAKE, .n_seed = 4, .flags = 0, .cache_bytes = 55,
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 },
    .seed_k = { 1, 2, 3, 4 },
    .morph = {
        { "DENS",   0, 16, 10, 1, 1 },
        { "LEN",    1, 16,  7, 1, 1 },
        { "RANGE",  0, 48, 12, 1, 4 },
        { "PATH",   1, 14,  7, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "GLEN",   1, 16, 16, 1, 1 },
        { "DYN",    0, 10, 10, 1, 1 },
        { "DIR",    0,  2,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {2}}, {0}, {1, {3}} },
    .sub = { "NOTES", "GATES", "VELO", "-", "DRIFT" },
    .build = snake_build, .loop_len = snake_loop_len, .step = snake_step, .hi = snake_hi, .sub_note = 0,
};
/* ALGODESC-END */
