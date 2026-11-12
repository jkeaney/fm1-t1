/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * The T-1 algorithm functions the new core keeps (ADR-0003 13 "Reuse"; the rest of the T-1 went at the cut-over, P4-012). */

#ifndef TORSO_T1_ALGO_H
#define TORSO_T1_ALGO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Bjorklund's Euclidean distribution: bit k of the result is step k (steps 1..64, pulses 0..steps). */
uint64_t t1_euclidean_bjorklund(uint8_t steps, uint8_t pulses);

/* Circular shift of a step mask within [0, steps-1]; rotate -64..+64. */
uint64_t t1_euclidean_rotate(uint64_t mask, uint8_t steps, int8_t rotate);

/* Non-linear pace curve warp in Q15 (0..32768). pace: -64 accelerating, 0 linear, +63 decelerating. */
uint16_t t1_pace_curve_q15(uint16_t normalized_x_q15, int8_t pace);

#define T1_NUM_SCALES 16

/* Scale interval masks: bit n = n semitones above the root. */
extern const uint16_t T1_SCALE_MASKS[T1_NUM_SCALES];

/* Melodic contour offset (phrase_shape 0 flat, 1 sine, 2 tri, 3 ramp up, 4 ramp dn, 5 arch, 6 u-arch, 7 S&H) in semitones. */
int16_t t1_harmony_eval_phrase(uint8_t phrase_shape, uint8_t range, uint8_t step_idx, uint8_t total_steps);

#endif
