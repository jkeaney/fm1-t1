/* SPDX-License-Identifier: GPL-3.0-only */
/* Tests of the shared primitives (sq_prims.c). The SP1 vectors were produced by an independent Python implementation of
 * docs/design/algorithms/sting.md 6.2 (the spec), not by this C code. */
#include <stdio.h>
#include <string.h>
#include "sq_types.h"
#include "sq_prims.h"
#include "../sq_prims_dump.h"     /* --dump: the golden lines of P4-002 (FB-009) */

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; fprintf(stderr, "FAIL prims: "); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

int main(int argc, char **argv)
{   SqRng r; uint8_t a[16], rank[16], ord[16]; SeqGlobal g;
    static const uint8_t want_shuf[16] = { 5, 7, 0, 13, 6, 9, 11, 3, 2, 14, 10, 1, 12, 8, 15, 4 };
    static const uint8_t want_rnd[6] = { 4, 37, 55, 6, 65, 12 };
    if (argc > 1 && strcmp(argv[1], "--dump") == 0) { sq_prims_dump(); return 0; }   /* stdout only; the normal run prints nothing new */
    /* SP1 */
    sq_rng_init(&r, (uint16_t)(0x3A5Du * 7u), 0x5A17C3E1u);
    for (int i = 0; i < 6; i++) CHECK(sq_rnd(&r, 100) == want_rnd[i], "rnd %d", i);
    sq_rng_init(&r, 0xBEEF, 0x2C9E6B07u);
    for (int i = 0; i < 16; i++) a[i] = (uint8_t)i;
    sq_shuffle(&r, a, 16);
    CHECK(memcmp(a, want_shuf, 16) == 0, "shuffle vector");
    CHECK(sq_mix(1) == 0x688990c0u && sq_mix(0xDEADBEEFu) == 0xe628c683u, "mix");
    sq_rng_init(&r, 0, 0); CHECK(r.x != 0, "zero state forced non-zero");
    { uint8_t p[24], seen[24] = {0}; sq_seeded_perm(p, 24, 0x1234u); for (int i = 0; i < 24; i++) seen[p[i]]++;
      for (int i = 0; i < 24; i++) CHECK(seen[i] == 1, "perm is a permutation");
      uint8_t q[24]; sq_seeded_perm(q, 24, 0x1234u); CHECK(memcmp(p, q, 24) == 0, "perm deterministic"); }
    /* SP2 / SP3 */
    sq_rng_init(&r, 0x1111, 0x2C9E6B07u); sq_rank_build(&r, rank, 16, 1);
    { uint8_t seen[16] = {0}; for (int i = 0; i < 16; i++) seen[rank[i]]++; for (int i = 0; i < 16; i++) CHECK(seen[i] == 1, "rank is a permutation"); }
    CHECK(rank[0] == 0, "first_fixed puts cell 0 first");
    for (int i = 0; i < 16; i++) ord[i] = (uint8_t)(15 - i);
    CHECK(sq_accent_pick(ord, 16, 0x8421, 2) == 0x8400, "accent pick: first two set cells in the order (%x)", sq_accent_pick(ord, 16, 0x8421, 2));
    CHECK(sq_accent_pick(ord, 16, 0x8421, 9) == 0x8421 && sq_accent_pick(ord, 16, 0xFFFF, 0) == 0, "accent pick limits");
    { uint8_t v[16]; sq_rng_init(&r, 5, 5); sq_vel_lane(&r, v, 16); for (int i = 0; i < 16; i++) CHECK(v[i] >= 50 && v[i] <= 127, "vel lane"); }
    /* SP5: invariants over every width, count and pace */
    for (int w = 3; w <= 192; w++) for (int n = 1; n <= 4; n++) for (int pace = -1; pace <= 1; pace++) {
        SqRep rep[3]; uint8_t k = sq_ratchet((uint8_t)w, (uint8_t)n, (int8_t)pace, 100, -10, 50, rep), prev_end = 0;
        CHECK(k <= 3 && k <= n - 1, "ratchet count w%d n%d", w, n);
        for (int j = 0; j < k; j++) {
            CHECK(rep[j].off >= 1 && rep[j].len >= 1 && rep[j].off + rep[j].len <= w - 1, "ratchet inside the step w%d n%d j%d off %u len %u", w, n, j, rep[j].off, rep[j].len);
            CHECK(rep[j].off > prev_end || j == 0, "ratchet repeats overlap w%d n%d j%d", w, n, j);
            CHECK(j == 0 || rep[j].off - rep[j - 1].off >= 3, "ratchet spacing >= 3");
            CHECK(rep[j].vel >= 1 && rep[j].vel <= 127, "ratchet vel");
            prev_end = (uint8_t)(rep[j].off + rep[j].len); } }
    { SqRep rep[3]; uint8_t k = sq_ratchet(8, 3, 0, 100, 0, 50, rep); CHECK(k == 1, "ratchet 3 in 8 ticks reduces to 2 (%u)", k);
      k = sq_ratchet(24, 3, 0, 100, 0, 50, rep); CHECK(k == 2 && rep[0].off == 8 && rep[1].off == 16, "even triplet in a 1/32"); }
    /* SP6 / SP9 / SP7 */
    memset(&g, 0, sizeof g); g.s.scale = 10; sq_gbuild(&g);                        /* MINPEN {0,3,5,7,10} */
    { static const uint8_t want[12] = { 0, 0, 3, 3, 3, 5, 5, 7, 7, 10, 10, 10 }; CHECK(memcmp(g.snap, want, 12) == 0, "MINPEN snap table"); }
    g.s.scale = 1; sq_gbuild(&g);                                                  /* MAJOR: the tie at 6 goes to the lower tone */
    { static const uint8_t want[12] = { 0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11 }; CHECK(memcmp(g.snap, want, 12) == 0, "MAJOR snap table"); }
    g.s.scale = 13; sq_gbuild(&g);                                                 /* HARMIN has no 9: 9 -> 8 (tie 8 / 10 -> 8); 10 -> 11 */
    CHECK(g.snap[9] == 8 && g.snap[10] == 11, "HARMIN snap %d %d", g.snap[9], g.snap[10]);
    g.s.scale = 9; sq_gbuild(&g);                                                  /* MAJPEN {0,2,4,7,9}: 11 -> 12 (1 away) beats 9 (2 away) */
    CHECK(g.snap[11] == 12, "wrap to the next key note (%d)", g.snap[11]);
    CHECK(sq_note(&g, 11) == 48, "sq_note wraps into the next octave");
    g.s.scale = 0; g.s.key = 5; g.s.octave = -1; sq_gbuild(&g);
    CHECK(sq_note(&g, 0) == 29 && sq_note(&g, -12) == 17 && sq_note(&g, 13) == 42 && sq_note(&g, -1) == 28, "sq_note key 5 octave -1 chromatic: %d %d %d %d", sq_note(&g, 0), sq_note(&g, -12), sq_note(&g, 13), sq_note(&g, -1));
    g.s.scale = 10; g.s.key = 0; g.s.octave = 0; sq_gbuild(&g);
    CHECK(sq_snap_abs(&g, 61) == 60, "snap_abs 61 -> 60 (%d)", sq_snap_abs(&g, 61));
    CHECK(sq_snap_abs(&g, 62) == 63 && sq_snap_abs(&g, 49) == 48, "snap_abs 62 -> 63 (%d), 49 -> 48 (%d)", sq_snap_abs(&g, 62), sq_snap_abs(&g, 49));
    CHECK(sq_span(255, 48) == 48 && sq_span(0, 48) == 0 && sq_span(128, 36) == 18, "span");
    /* SP16 / SP17 */
    { static const uint8_t pend[10] = { 0, 1, 2, 3, 2, 1, 0, 1, 2, 3 };
      for (uint32_t u = 0; u < 10; u++) { CHECK(sq_dir_map(u, 4, 2) == pend[u], "pend %u", u); CHECK(sq_dir_map(u, 4, 1) == 3 - u % 4, "bwd"); CHECK(sq_dir_map(u, 4, 0) == u % 4, "fwd"); }
      CHECK(sq_dir_map(7, 1, 2) == 0, "pend n=1"); }
    CHECK(sq_dyn_vel(120, 10) == 120 && sq_dyn_vel(40, 0) == 90 && sq_dyn_vel(120, 5) == 105 && sq_dyn_vel(60, 5) == 75, "dyn_vel");
    if (fails) { fprintf(stderr, "prims: %d check(s) failed\n", fails); return 1; }
    fprintf(stderr, "prims: primitive checks OK\n");
    return 0; }
