/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * Test Suite: the T-1 algorithm functions the new core keeps (P4-012, ADR-0003 13): Bjorklund, rotate, pace curve, scale masks, phrase contour.
 * The rest of the Phase 2 suite (eval_cycle, ratchet pattern, quantize, voicing, style, LFSR, p-locks) went with those functions. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "torso_t1_algo.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tests_run++; \
    if (!(cond)) { \
        printf("FAILED: %s (line %d): %s\n", __func__, __LINE__, msg); \
        exit(1); \
    } else { \
        g_tests_passed++; \
    } \
} while(0)

/* Helper: count set bits in a 64-bit integer */
static uint8_t popcount64(uint64_t v)
{
    uint8_t c = 0;
    while (v) {
        c += (uint8_t)(v & 1ULL);
        v >>= 1;
    }
    return c;
}

/* ---------------- Test 1: Bjorklund Euclidean Generation & Rotation --- */
static void test_euclidean_engine(void)
{
    printf("== [Test 1] Bjorklund Euclidean Rhythm Generation & Rotation\n");

    /* 1. Edge cases */
    TEST_ASSERT(t1_euclidean_bjorklund(16, 0) == 0ULL, "0 pulses gives 0 mask");
    TEST_ASSERT(t1_euclidean_bjorklund(16, 16) == 0xFFFFULL, "16 pulses in 16 steps gives all 1s");
    TEST_ASSERT(t1_euclidean_bjorklund(64, 64) == ~0ULL, "64 pulses in 64 steps gives full 64-bit mask");

    /* 2. Classical Ethnomusicological Rhythms */

    /* Tresillo E(3, 8): [1, 0, 0, 1, 0, 0, 1, 0] -> bits 0, 3, 6 set */
    uint64_t tresillo = t1_euclidean_bjorklund(8, 3);
    TEST_ASSERT(popcount64(tresillo) == 3, "Tresillo has exactly 3 pulses");
    TEST_ASSERT((tresillo & (1ULL << 0)) != 0, "Step 0 is a pulse");
    TEST_ASSERT((tresillo & (1ULL << 1)) == 0, "Step 1 is a rest");
    TEST_ASSERT((tresillo & (1ULL << 2)) == 0, "Step 2 is a rest");
    TEST_ASSERT((tresillo & (1ULL << 3)) != 0, "Step 3 is a pulse");
    TEST_ASSERT((tresillo & (1ULL << 4)) == 0, "Step 4 is a rest");
    TEST_ASSERT((tresillo & (1ULL << 5)) == 0, "Step 5 is a rest");
    TEST_ASSERT((tresillo & (1ULL << 6)) != 0, "Step 6 is a pulse");
    TEST_ASSERT((tresillo & (1ULL << 7)) == 0, "Step 7 is a rest");
    /* Mask: 0b01001001 = 0x49 */
    TEST_ASSERT(tresillo == 0x49ULL, "Tresillo bitmask matches 0x49");

    /* Cinquillo E(5, 8): 5 pulses in 8 steps */
    uint64_t cinquillo = t1_euclidean_bjorklund(8, 5);
    TEST_ASSERT(popcount64(cinquillo) == 5, "Cinquillo has 5 pulses");

    /* Four-on-the-floor E(4, 16): hits on 0, 4, 8, 12 */
    uint64_t house = t1_euclidean_bjorklund(16, 4);
    TEST_ASSERT(popcount64(house) == 4, "Four-on-the-floor has 4 pulses");
    TEST_ASSERT(house == ((1ULL << 0) | (1ULL << 4) | (1ULL << 8) | (1ULL << 12)), "House beat hits every 4 steps");

    /* 3. Circular Shift Rotation (PLAN.md: (mask >> r) | (mask << (steps - r))) */
    /* Rotate Tresillo (0, 3, 6) with r = 1 -> hits at 2, 5, 7 */
    uint64_t rot_1 = t1_euclidean_rotate(tresillo, 8, 1);
    TEST_ASSERT(popcount64(rot_1) == 3, "Rotated mask preserves pulse count");
    TEST_ASSERT((rot_1 & (1ULL << 2)) != 0, "Rotated step 2 is hit");
    TEST_ASSERT((rot_1 & (1ULL << 5)) != 0, "Rotated step 5 is hit");
    TEST_ASSERT((rot_1 & (1ULL << 7)) != 0, "Rotated step 7 is hit");

    /* Rotate by negative offset (-1) -> hits at 1, 4, 7 */
    uint64_t rot_neg1 = t1_euclidean_rotate(tresillo, 8, -1);
    TEST_ASSERT(popcount64(rot_neg1) == 3, "Negative rotated mask preserves pulse count");
    TEST_ASSERT((rot_neg1 & (1ULL << 1)) != 0, "Rotated step 1 is hit");
    TEST_ASSERT((rot_neg1 & (1ULL << 4)) != 0, "Rotated step 4 is hit");
    TEST_ASSERT((rot_neg1 & (1ULL << 7)) != 0, "Rotated step 7 is hit");

    printf("   -> Bjorklund Euclidean rhythms and circular rotation verified!\n");
}

/* ---------------- Test 2: the pace curve --- */
static void test_pace_curve(void)
{
    printf("== [Test 2] Pace curve\n");
    TEST_ASSERT(t1_pace_curve_q15(16384, 0) == 16384, "pace 0 is linear");
    TEST_ASSERT(t1_pace_curve_q15(0, 40) == 0 && t1_pace_curve_q15(32768, -40) == 32768, "the ends stay put");
    TEST_ASSERT(t1_pace_curve_q15(16384, -32) > 16384, "pace < 0 accelerates (y > x)");
    TEST_ASSERT(t1_pace_curve_q15(16384, 32) < 16384, "pace > 0 decelerates (y < x)");
    printf("   -> pace curve verified!\n");
}

/* ---------------- Test 3: Scales and phrase contours --- */
static void test_scales_and_phrase(void)
{
    printf("== [Test 3] Scales and phrase contours (16 scale masks, phrase contours)\n");

    /* 1. All 16 Scales Defined */
    TEST_ASSERT(T1_NUM_SCALES == 16, "16 scales defined");
    TEST_ASSERT(T1_SCALE_MASKS[0] == 0x0FFF, "Scale 0 is Chromatic");
    TEST_ASSERT(T1_SCALE_MASKS[1] == 0x0AB5, "Scale 1 is Major {0,2,4,5,7,9,11}");
    {   /* every scale is its interval list, in the UI's order (SCALE_NAMES) */
        static const uint8_t IV[16][13] = {
            {0,1,2,3,4,5,6,7,8,9,10,11,255},   /* Chromatic */
            {0,2,4,5,7,9,11,255},   /* Major (Ionian) */
            {0,2,3,5,7,8,10,255},   /* Natural Minor */
            {0,2,3,5,7,9,10,255},   /* Dorian */
            {0,1,3,5,7,8,10,255},   /* Phrygian */
            {0,2,4,6,7,9,11,255},   /* Lydian */
            {0,2,4,5,7,9,10,255},   /* Mixolydian */
            {0,2,3,5,7,8,10,255},   /* Aeolian */
            {0,1,3,5,6,8,10,255},   /* Locrian */
            {0,2,4,7,9,255},   /* Pentatonic Major */
            {0,3,5,7,10,255},   /* Pentatonic Minor */
            {0,3,5,6,7,10,255},   /* Blues */
            {0,2,4,6,8,10,255},   /* Whole Tone */
            {0,2,3,5,7,8,11,255},   /* Harmonic Minor */
            {0,2,3,5,7,9,11,255},   /* Melodic Minor */
            {0,1,3,4,6,7,9,10,255},   /* Diminished (Half-Whole) */
        };
        int all = 1;
        for (int i = 0; i < 16; i++) {
            uint16_t m = 0;
            for (int k = 0; IV[i][k] != 255; k++) m |= (uint16_t)(1u << IV[i][k]);
            all &= T1_SCALE_MASKS[i] == m;
        }
        TEST_ASSERT(all, "All 16 scale masks match their interval lists (UI order)");
    }
    TEST_ASSERT(T1_SCALE_MASKS[2] == 0x05AD, "Scale 2 is Natural Minor");

    /* 4. Melodic Phrase Contours */
    /* Flat phrase (0) -> always 0 */
    TEST_ASSERT(t1_harmony_eval_phrase(0, 2, 5, 16) == 0, "Flat phrase offset is 0");

    /* Triangle phrase (2): peaks at midpoint */
    int16_t p_start = t1_harmony_eval_phrase(2, 2, 0, 16);
    int16_t p_mid   = t1_harmony_eval_phrase(2, 2, 8, 16);
    int16_t p_end   = t1_harmony_eval_phrase(2, 2, 15, 16);
    TEST_ASSERT(p_start == 0, "Triangle starts at 0");
    TEST_ASSERT(p_mid > p_start, "Triangle peaks in middle");
    TEST_ASSERT(p_mid > p_end, "Triangle falls back down");

    /* BUG-001 / F3: ARCH (5) and INV A (6) must have a shape: ARCH peaks mid, INV A dips mid */
    {
        int arch_min = 99, arch_max = -99, inv_min = 99, inv_max = -99;
        for (int i = 0; i < 16; i++) {
            int a = t1_harmony_eval_phrase(5, 4, (uint8_t)i, 16);
            int v = t1_harmony_eval_phrase(6, 4, (uint8_t)i, 16);
            if (a < arch_min) arch_min = a;
            if (a > arch_max) arch_max = a;
            if (v < inv_min) inv_min = v;
            if (v > inv_max) inv_max = v;
        }
        TEST_ASSERT(arch_max > arch_min, "ARCH has more than one value");
        TEST_ASSERT(inv_max > inv_min, "INV A has more than one value");
        TEST_ASSERT(t1_harmony_eval_phrase(5, 4, 8, 16) == arch_max, "ARCH peaks at the middle");
        TEST_ASSERT(t1_harmony_eval_phrase(5, 4, 8, 16) > t1_harmony_eval_phrase(5, 4, 0, 16), "ARCH middle above start");
        TEST_ASSERT(t1_harmony_eval_phrase(6, 4, 8, 16) == inv_min, "INV A dips at the middle");
        TEST_ASSERT(t1_harmony_eval_phrase(6, 4, 8, 16) < t1_harmony_eval_phrase(6, 4, 0, 16), "INV A middle below start");
    }

    printf("   -> scale masks and phrase contours verified!\n");
}

/* ------------------------------------------------ Main Test Runner --- */
int main(void)
{
    printf("=================================================================\n");
    printf("  Running the kept T-1 algorithm functions\n");
    printf("=================================================================\n");

    test_euclidean_engine();
    test_pace_curve();
    test_scales_and_phrase();

    printf("=================================================================\n");
    printf("  ALL %d KEPT T-1 ALGORITHM TESTS PASSED (%d assertions)\n", g_tests_passed, g_tests_run);
    printf("=================================================================\n");
    return 0;
}
