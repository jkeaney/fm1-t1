/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * The T-1 algorithm functions the new core keeps (header) */

#include "torso_t1_algo.h"

#if !defined(FELUCCA_ID) && !defined(FM1_SYS_H)
#ifndef T1_DEVICE
#include <string.h>
#endif
#else
void *memset(void *d, int c, unsigned n);
void *memcpy(void *d, const void *s, unsigned n);
#endif

/* -------------------------------------------------------------------------
 * 1. EUCLIDEAN ENGINE (Bjorklund's Distribution Algorithm)
 * ------------------------------------------------------------------------- */

typedef struct {
    uint64_t pattern;
    uint8_t  len;
} BjorkGroup;

uint64_t t1_euclidean_bjorklund(uint8_t steps, uint8_t pulses)
{
    if (steps == 0) return 0;
    if (steps > 64) steps = 64;
    if (pulses == 0) return 0;
    if (pulses >= steps) {
        return (steps == 64) ? ~0ULL : ((1ULL << steps) - 1ULL);
    }

    BjorkGroup groups[64];
    uint8_t count_ones  = pulses;
    uint8_t count_zeros = (uint8_t)(steps - pulses);

    /* Initialize pulses as '1' (len 1) and rests as '0' (len 1) */
    for (uint8_t i = 0; i < count_ones; i++) {
        groups[i].pattern = 1ULL;
        groups[i].len     = 1;
    }
    for (uint8_t i = 0; i < count_zeros; i++) {
        groups[count_ones + i].pattern = 0ULL;
        groups[count_ones + i].len     = 1;
    }

    /* Iterative Bjorklund pairing */
    while (count_zeros > 1) {
        uint8_t min_c = (count_ones < count_zeros) ? count_ones : count_zeros;

        /* Append zero-groups to one-groups */
        for (uint8_t i = 0; i < min_c; i++) {
            groups[i].pattern = (groups[i].pattern << groups[count_ones + i].len) |
                                 groups[count_ones + i].pattern;
            groups[i].len    += groups[count_ones + i].len;
        }

        if (count_ones >= count_zeros) {
            /* Remaining one-groups become the new remainder (zeros group) */
            uint8_t remainder = count_ones - count_zeros;
            for (uint8_t i = 0; i < remainder; i++) {
                groups[min_c + i] = groups[count_zeros + i];
            }
            count_ones  = min_c;
            count_zeros = remainder;
        } else {
            /* Remaining zero-groups shift down */
            uint8_t remainder = count_zeros - count_ones;
            for (uint8_t i = 0; i < remainder; i++) {
                groups[min_c + i] = groups[count_ones + min_c + i];
            }
            count_zeros = remainder;
        }
    }

    /* Concatenate all remaining groups */
    uint8_t total_groups = count_ones + count_zeros;
    uint64_t result_mask = 0ULL;
    uint8_t current_bit = 0;

    for (uint8_t i = 0; i < total_groups; i++) {
        uint64_t pat = groups[i].pattern;
        uint8_t  len = groups[i].len;

        for (int b = len - 1; b >= 0; b--) {
            if ((pat >> b) & 1ULL) {
                result_mask |= (1ULL << current_bit);
            }
            current_bit++;
            if (current_bit >= steps) break;
        }
        if (current_bit >= steps) break;
    }

    return result_mask;
}

uint64_t t1_euclidean_rotate(uint64_t mask, uint8_t steps, int8_t rotate)
{
    if (steps == 0) return 0;
    if (steps > 64) steps = 64;

    int8_t r = (int8_t)(((rotate % (int8_t)steps) + (int8_t)steps) % (int8_t)steps);
    if (r == 0) return mask;

    uint64_t step_mask = (steps == 64) ? ~0ULL : ((1ULL << steps) - 1ULL);
    mask &= step_mask;

    if (steps == 64) {
        return (mask >> r) | (mask << (64 - r));
    }
    return ((mask >> r) | (mask << (steps - r))) & step_mask;
}

/* (t1_euclidean_eval_cycle went with CycleParams at the cut-over, P4-012.) */

/* -------------------------------------------------------------------------
 * 2. PACE CURVE (the maths of the ratchet spacing; sq_ratchet wraps it, ADR-0003 13)
 * ------------------------------------------------------------------------- */

/*
 * Compute non-linear pace curve in Q15 (0 to 32768).
 * pace < 0: accelerating (intervals start large and shrink -> bouncing ball)
 * pace == 0: linear
 * pace > 0: decelerating (intervals start small and expand)
 */
uint16_t t1_pace_curve_q15(uint16_t x_q15, int8_t pace)
{
    if (pace == 0 || x_q15 == 0 || x_q15 >= 32768) {
        return x_q15;
    }

    /*
     * Quadratic bell factor: b(x) = x * (1 - x), in Q15.
     * Peak is 8192 (0.25) at x = 16384 (0.5).
     */
    uint32_t inv_x = 32768u - (uint32_t)x_q15;
    uint32_t bell  = ((uint32_t)x_q15 * inv_x) >> 15; /* 0..8192 */

    /*
     * Warp delta = -pace * bell / 64
     * If pace < 0: y > x (accelerating, early intervals large, later intervals short -> bouncing ball)
     * If pace > 0: y < x (decelerating, early intervals short, later intervals large)
     */
    int32_t delta = -(((int32_t)pace * (int32_t)bell) / 64);
    int32_t y = (int32_t)x_q15 + delta;

    if (y < 0)     y = 0;
    if (y > 32768) y = 32768;
    return (uint16_t)y;
}

/* -------------------------------------------------------------------------
 * 3. SCALES AND THE PHRASE CONTOUR
 * ------------------------------------------------------------------------- */

/* Bit n = n semitones above the root. Order = TORSO_T1_GUIDE.md;
 * each mask is its interval list */
const uint16_t T1_SCALE_MASKS[T1_NUM_SCALES] = {
    0x0FFFu, /*  0: Chromatic {0,1,2,3,4,5,6,7,8,9,10,11} */
    0x0AB5u, /*  1: Major (Ionian) {0,2,4,5,7,9,11} */
    0x05ADu, /*  2: Natural Minor {0,2,3,5,7,8,10} */
    0x06ADu, /*  3: Dorian {0,2,3,5,7,9,10} */
    0x05ABu, /*  4: Phrygian {0,1,3,5,7,8,10} */
    0x0AD5u, /*  5: Lydian {0,2,4,6,7,9,11} */
    0x06B5u, /*  6: Mixolydian {0,2,4,5,7,9,10} */
    0x05ADu, /*  7: Aeolian {0,2,3,5,7,8,10} */
    0x056Bu, /*  8: Locrian {0,1,3,5,6,8,10} */
    0x0295u, /*  9: Pentatonic Major {0,2,4,7,9} */
    0x04A9u, /* 10: Pentatonic Minor {0,3,5,7,10} */
    0x04E9u, /* 11: Blues {0,3,5,6,7,10} */
    0x0555u, /* 12: Whole Tone {0,2,4,6,8,10} */
    0x09ADu, /* 13: Harmonic Minor {0,2,3,5,7,8,11} */
    0x0AADu, /* 14: Melodic Minor {0,2,3,5,7,9,11} */
    0x06DBu, /* 15: Diminished (Half-Whole) {0,1,3,4,6,7,9,10} */
};

int16_t t1_harmony_eval_phrase(uint8_t phrase_shape, uint8_t range, uint8_t step_idx, uint8_t total_steps)
{
    if (range == 0 || total_steps <= 1) return 0;
    step_idx %= total_steps;

    int32_t max_semitones = (int32_t)range;     /* RANGE: the contour's excursion in semitones (the UI's unit) */
    int32_t delta = 0;

    switch (phrase_shape) {
        case 1: { /* Sine wave contour approximation */
            /* Quarter-cycle lookup */
            int32_t phase = ((int32_t)step_idx * 360) / total_steps;
            /* Simple parabolic sine approximation */
            if (phase < 180) {
                delta = (phase * (180 - phase) * max_semitones) / 8100;
            } else {
                int32_t p = phase - 180;
                delta = -(p * (180 - p) * max_semitones) / 8100;
            }
            break;
        }
        case 2: { /* Triangle wave contour */
            int32_t half = total_steps / 2;
            if (half == 0) half = 1;
            if (step_idx <= half) {
                delta = ((int32_t)step_idx * max_semitones) / half;
            } else {
                delta = (((int32_t)total_steps - step_idx) * max_semitones) / half;
            }
            break;
        }
        case 3: /* Ramp Up (Saw) */
            delta = ((int32_t)step_idx * max_semitones) / (total_steps - 1);
            break;
        case 4: /* Ramp Down (Saw) */
            delta = (((int32_t)total_steps - 1 - step_idx) * max_semitones) / (total_steps - 1);
            break;
        case 5: { /* Exponential Arch (peaking at midpoint) */
            int32_t norm = (2 * step_idx - total_steps);
            /* multiply before dividing: the old quotient was always 0 (F3) */
            int32_t quad = (norm * norm * max_semitones) / ((int32_t)total_steps * total_steps);
            delta = max_semitones - quad;
            break;
        }
        case 6: { /* Inverted Arch (dip in middle) */
            int32_t norm = (2 * step_idx - total_steps);
            int32_t quad = (norm * norm * max_semitones) / ((int32_t)total_steps * total_steps);
            delta = quad;
            break;
        }
        case 7: { /* Deterministic pseudo-random S&H */
            uint32_t hash = ((uint32_t)step_idx * 1103515245u + 12345u) & 0x7FFFFFFFu;
            delta = (int32_t)(hash % (uint32_t)(2 * max_semitones + 1)) - max_semitones;
            break;
        }
        default:
            delta = 0;
            break;
    }

    return (int16_t)delta;
}
