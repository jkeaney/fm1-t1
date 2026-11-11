/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_prims.c: shared primitives (see sq_prims.h). P2-008 reference copy; stdint only, integer arithmetic only. */
#include "sq_prims.h"

const uint16_t SQ_SCALE_MASKS[16] = {
    0x0FFFu, 0x0AB5u, 0x05ADu, 0x06ADu, 0x05ABu, 0x0AD5u, 0x06B5u, 0x05ADu,
    0x056Bu, 0x0295u, 0x04A9u, 0x04E9u, 0x0555u, 0x09ADu, 0x0AADu, 0x06DBu,
};

uint32_t sq_rng_next(SqRng *r)
{   uint32_t x = r->x;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    r->x = x;
    return x; }

void sq_rng_init(SqRng *r, uint16_t seed16, uint32_t salt)
{   uint32_t x = (((uint32_t)seed16 << 16) | seed16) ^ salt;
    r->x = x ? x : 1u;
    for (uint8_t i = 0; i < 4; i++) (void)sq_rng_next(r); }

uint32_t sq_rnd(SqRng *r, uint32_t n) { return (sq_rng_next(r) >> 8) % n; }
uint8_t  sq_bern(SqRng *r, uint8_t p) { return (uint8_t)(sq_rnd(r, 100) < p); }

void sq_shuffle(SqRng *r, uint8_t *a, uint8_t len)
{   for (int i = (int)len - 1; i >= 1; i--) {
        uint8_t j = (uint8_t)sq_rnd(r, (uint32_t)i + 1), t = a[i];
        a[i] = a[j]; a[j] = t; } }

uint32_t sq_mix(uint32_t x)
{   x ^= x >> 16; x *= 0x7FEB352Du; x ^= x >> 15; x *= 0x846CA68Bu; x ^= x >> 16;
    return x; }

void sq_seeded_perm(uint8_t *perm, uint8_t n, uint32_t x)
{   SqRng r; r.x = x ? x : 1u;
    (void)sq_rng_next(&r); (void)sq_rng_next(&r);
    for (uint8_t i = 0; i < n; i++) perm[i] = i;
    sq_shuffle(&r, perm, n); }

void sq_rank_build(SqRng *r, uint8_t *rank, uint8_t n, uint8_t first_fixed)
{   uint8_t ord[16];
    if (first_fixed) {
        for (uint8_t i = 1; i < n; i++) ord[i] = i;      /* ord[1..n-1] shuffled, ord[0] = 0 stays first */
        ord[0] = 0;
        sq_shuffle(r, ord + 1, (uint8_t)(n - 1));
    } else {
        for (uint8_t i = 0; i < n; i++) ord[i] = i;
        sq_shuffle(r, ord, n); }
    for (uint8_t i = 0; i < n; i++) rank[ord[i]] = i; }

uint16_t sq_accent_pick(const uint8_t *ord, uint8_t n, uint16_t cells, uint8_t count)
{   uint16_t m = 0; uint8_t k = 0;
    for (uint8_t i = 0; i < n && k < count; i++)
        if ((cells >> ord[i]) & 1u) { m |= (uint16_t)(1u << ord[i]); k++; }
    return m; }

void sq_vel_lane(SqRng *r, uint8_t *vel, uint8_t len)
{   for (uint8_t i = 0; i < len; i++) vel[i] = (uint8_t)(50 + sq_rnd(r, 78)); }

/* interval weights of n triggers: pace < 0 accelerating (n, n-1, .. 1: intervals shrink), 0 even, > 0 decelerating (1 .. n) */
static uint8_t rat_offsets(uint8_t width, uint8_t n, int8_t pace, uint8_t off[5])
{   uint16_t sum = 0, w[4], acc = 0;
    uint8_t ok = 1;
    for (uint8_t j = 0; j < n; j++) {
        w[j] = (uint16_t)(pace < 0 ? n - j : pace > 0 ? j + 1 : 1);
        sum = (uint16_t)(sum + w[j]); }
    for (uint8_t j = 0; j <= n; j++) {
        off[j] = (uint8_t)(((uint32_t)width * acc) / sum);
        if (j > 0 && off[j] - off[j - 1] < 3) ok = 0;
        if (j < n) acc = (uint16_t)(acc + w[j]); }
    return ok; }

uint8_t sq_ratchet(uint8_t width, uint8_t n, int8_t pace, uint8_t vel, int8_t slope, uint8_t gate_pct, SqRep out[3])
{   uint8_t off[5];
    if (n > 4) n = 4;
    while (n > 1 && !rat_offsets(width, n, pace, off)) n--;
    if (n <= 1) return 0;
    {   int16_t v = vel;
        for (uint8_t j = 1; j < n; j++) {
            uint8_t sp = (uint8_t)(off[j + 1] - off[j]), len = (uint8_t)(((uint16_t)sp * gate_pct) / 100u);
            if (len < 1) len = 1;
            if (len > sp - 1) len = (uint8_t)(sp - 1);
            v = (int16_t)(v + slope); if (v < 1) v = 1; if (v > 127) v = 127;
            out[j - 1].off = off[j]; out[j - 1].len = len; out[j - 1].vel = (uint8_t)v; } }
    return (uint8_t)(n - 1); }

void sq_gbuild(SeqGlobal *g)
{   uint16_t mask = SQ_SCALE_MASKS[g->s.scale & 15] & 0x0FFFu;
    if (!mask) mask = 0x0FFFu;
    for (int r = 0; r < 12; r++) {
        int bd = 99, bc = 0;
        for (int sh = -12; sh <= 12; sh += 12)
            for (int t = 0; t < 12; t++) {
                if (!((mask >> t) & 1u)) continue;
                int c = t + sh, d = c > r ? c - r : r - c;
                if (d < bd || (d == bd && c < bc)) { bd = d; bc = c; } }
        g->snap[r] = (uint8_t)(bc < 0 ? 0 : bc); } }

uint8_t sq_snap_abs(const SeqGlobal *g, uint8_t n)
{   int16_t rel = (int16_t)n - g->s.key;
    int16_t q = rel >= 0 ? rel / 12 : -((-rel + 11) / 12), r = rel - 12 * q;
    int16_t v = g->s.key + 12 * q + g->snap[r];
    return (uint8_t)(v < 0 ? 0 : v > 127 ? 127 : v); }

uint8_t sq_dir_map(uint32_t u, uint8_t n, uint8_t d)
{   if (n <= 1) return 0;
    if (d == 0) return (uint8_t)(u % n);
    if (d == 1) return (uint8_t)(n - 1 - (u % n));
    {   uint32_t q = u % (2u * n - 2u);
        return (uint8_t)(q < n ? q : 2u * n - 2u - q); } }

uint8_t sq_dyn_vel(uint8_t v, uint8_t dyn)
{   int32_t r = 90 - ((90 - (int32_t)v) * dyn) / 10;
    return (uint8_t)(r < 1 ? 1 : r > 127 ? 127 : r); }
