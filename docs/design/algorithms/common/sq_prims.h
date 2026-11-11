/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_prims.h: the shared primitives SP1..SP7, SP9, SP16, SP17 (ALGORITHM_SET 6). P2-008 reference copy; stdint only.
 * Every function is pure (no globals written). The firmware copy adds `static`. */
#ifndef SQ_PRIMS_H
#define SQ_PRIMS_H

#include "sq_types.h"

/* SP1 seeded RNG: xorshift32. init(seed16, salt): x = ((seed16 << 16) | seed16) ^ salt, 1 if 0, four draws discarded. */
typedef struct { uint32_t x; } SqRng;
void     sq_rng_init(SqRng *r, uint16_t seed16, uint32_t salt);
uint32_t sq_rng_next(SqRng *r);                     /* x ^= x << 13; x ^= x >> 17; x ^= x << 5 */
uint32_t sq_rnd(SqRng *r, uint32_t n);              /* (next() >> 8) % n, n >= 1 */
uint8_t  sq_bern(SqRng *r, uint8_t p);              /* rnd(100) < p */
void     sq_shuffle(SqRng *r, uint8_t *a, uint8_t len);   /* Fisher-Yates: i = len-1 down to 1, j = rnd(i+1), swap */
uint32_t sq_mix(uint32_t x);                        /* the 32-bit integer hash of ARP SPEC 3 (x ^= x >> 16; x *= 0x7FEB352D; ...) */
void     sq_seeded_perm(uint8_t *perm, uint8_t n, uint32_t x);   /* ARP RAND: perm = [0..n-1] shuffled by a stream whose state is x (1 if 0), two draws discarded; n <= 24 */

/* SP2 rank-order density: rank[c] = position of cell c in a seeded order of n cells (n <= 16); a cell plays iff rank < count.
 * first_fixed: cell 0 always ranks first and the other n-1 cells are shuffled (ACID, STAGE). */
void     sq_rank_build(SqRng *r, uint8_t *rank, uint8_t n, uint8_t first_fixed);

/* SP3 seeded accent count: ord[] is a seeded order of n cells; the first `count` cells of that order whose bit is set in
 * `cells` are accented. Returns the accent mask (bit c = cell c). */
uint16_t sq_accent_pick(const uint8_t *ord, uint8_t n, uint16_t cells, uint8_t count);

/* SP4 velocity lane: vel[i] = 50 + rnd(78), i = 0..len-1 (50..127). Read as vel[t % lane_len]. */
void     sq_vel_lane(SqRng *r, uint8_t *vel, uint8_t len);

/* SP5 retrigger inside a step. A step of `width` ticks (swing already applied by the caller) holds `n` triggers (1..4): the
 * first at offset 0 is the step's own note; this fills out[0..] with the extra ones and returns how many (0..3). Spacing
 * is even (pace 0), shrinks (pace < 0, accelerating) or grows (pace > 0, decelerating; the PO_PACE_DEC reading). EUCLID's
 * pace magnitude is P2-016's: here only the sign matters. n is reduced until every spacing is at least 3 ticks. Every repeat starts after the one before, ends before the next
 * (len < spacing) and inside [0, width - 1]. vel falls by `slope` per repeat (clamped to 1..127). */
typedef struct { uint8_t off, len, vel; } SqRep;
uint8_t  sq_ratchet(uint8_t width, uint8_t n, int8_t pace, uint8_t vel, int8_t slope, uint8_t gate_pct, SqRep out[3]);

/* SP6 scale quantise. The scale masks are T1_SCALE_MASKS (firmware/src/torso_t1_algo.c), index = GlobalSave.scale:
 * CHROM MAJOR MINOR DORIAN PHRYG LYDIAN MIXO AEOL LOCRI MAJPEN MINPEN BLUES WHOLE HARMIN MELMIN DIM. */
extern const uint16_t SQ_SCALE_MASKS[16];
/* snap[r] for r = 0..11: the nearest scale tone to interval r by distance between the pitches themselves, tones taken in
 * the octave below, this one and the next, ties to the lower pitch (so r = 11 in MAJOR is 11, r = 6 in MAJOR is 5). */
void     sq_gbuild(SeqGlobal *g);                   /* fills snap[] from g->s.scale */
/* Live keys: the same table without the octave offset: key + 12 q + snap[r], with q, r from (n - key). Clamped 0..127. */
uint8_t  sq_snap_abs(const SeqGlobal *g, uint8_t n);

/* SP7 span mapping: a raw 0..255 value over a range of `rng` semitones, before the late quantise. */
static inline uint8_t sq_span(uint8_t raw, uint8_t rng) { return (uint8_t)(((uint16_t)raw * rng) / 255u); }

/* SP9 + SP6 (ADR-0003 section 4). semi may be negative; floor division by 12. */
static inline uint8_t sq_note(const SeqGlobal *g, int16_t semi)
{   int16_t q = semi >= 0 ? semi / 12 : -((-semi + 11) / 12), r = semi - 12 * q;     /* r in 0..11 */
    int16_t n = 36 + g->s.key + 12 * g->s.octave + 12 * q + g->snap[r];
    return (uint8_t)(n < 0 ? 0 : n > 127 ? 127 : n); }

/* SP16 direction map: d 0 FWD, 1 BWD, 2 PEND (ping-pong without repeating the ends). n >= 1. */
uint8_t  sq_dir_map(uint32_t u, uint8_t n, uint8_t d);

/* SP18..SP20 the mod gen (MODULATION 3.1; P2-017 fixes their bodies). None of them touches the SP1 generator, so no note output depends on s_mod.
 * SP18: a cell of lane `lane` (0 A, 1 B) for slot 1..4 at index i (0..255; MODULATION wrote `i & 15`, the callers pass the algorithm's own index and
 * ARP's figure positions reach 137): a hash of (s_mod, lane, slot, i) in -127..127.
 * SP19: the LFO of a slot: shape = s_mod & 3 (0 TRI, 1 SAW up, 2 SAW down, 3 SINE through a 17 point table), phase = (s_mod >> 4) % period + (slot - 1) * period / 4,
 * value at position t over `period` steps (period >= 1), -127..127.
 * SP20: the lean on loudness, (vel - 90) * 2 clamped to -64..64: 90 (a plain step) = 0, 120 (an accent) = +60. */
int8_t   sp18_cell(uint16_t s_mod, uint8_t lane, uint8_t slot, uint8_t i);
int8_t   sp19_lfo(uint16_t s_mod, uint8_t slot, uint16_t t, uint16_t period);
int8_t   sp20_lean(uint8_t vel);
/* The value of one slot at one step from the algorithm's ModIdx: odd slot = lane A = clamp(sp18_cell(s_mod, 0, slot, a) / 2 + sp20_lean(vel));
 * even slot = lane B = the LFO or a cell (b_lfo). `len` replaces a period of 0 (the channel LENGTH). */
int8_t   sq_mod_lane(uint16_t s_mod, uint8_t slot, const ModIdx *m, uint16_t len);

/* SP17 velocity depth: clamp(90 - ((90 - v) * dyn) / 10, 1, 127), truncating toward zero; dyn 10 returns v, 0 returns 90. */
uint8_t  sq_dyn_vel(uint8_t v, uint8_t dyn);

#endif /* SQ_PRIMS_H */
