/* SPDX-License-Identifier: GPL-3.0-only */
/* The deterministic dump of the shared primitives (P4-002, FB-009 option a). Included by BOTH tests/design/prims_test.c (--dump,
 * built against the prototype docs/design/algorithms/common/sq_prims.c) and tests/sq_prims_test.c (built against the firmware
 * copy firmware/src/sq_prims.c), so the two print the same bytes iff the firmware copy kept the prototype's behaviour.
 * Line format, one line per case, on stdout, fixed seeds, no timestamps:   <function> <inputs> -> <outputs>
 * Every function declared in sq_prims.h appears. Numbers are decimal unless named hex (0x, lower case). */
#ifndef SQ_PRIMS_DUMP_H
#define SQ_PRIMS_DUMP_H
#include <stdio.h>
#include <string.h>

static void dump_u8s(const uint8_t *a, int n) { for (int i = 0; i < n; i++) printf("%s%u", i ? "," : "", (unsigned)a[i]); }

static void sq_prims_dump(void)
{   SqRng r; uint8_t a[24], rank[16], ord[16]; SeqGlobal g;
    /* SP1 */
    sq_rng_init(&r, (uint16_t)(0x3A5Du * 7u), 0x5A17C3E1u);
    printf("sq_rng_init seed16=0x%04x salt=0x%08x -> x=0x%08x\n", (unsigned)(uint16_t)(0x3A5Du * 7u), 0x5A17C3E1u, (unsigned)r.x);
    for (int i = 0; i < 6; i++) printf("sq_rnd n=100 draw=%d -> %u\n", i, (unsigned)sq_rnd(&r, 100));
    sq_rng_init(&r, 0, 0); printf("sq_rng_init seed16=0x0000 salt=0x00000000 -> x=0x%08x\n", (unsigned)r.x);
    sq_rng_init(&r, 0xBEEF, 0x2C9E6B07u);
    for (int i = 0; i < 3; i++) printf("sq_rng_next seed16=0xbeef salt=0x2c9e6b07 draw=%d -> 0x%08x\n", i, (unsigned)sq_rng_next(&r));
    sq_rng_init(&r, 0x1234, 0x77u);
    printf("sq_bern seed16=0x1234 salt=0x77 p=50 draws=16 ->");
    for (int i = 0; i < 16; i++) printf(" %u", (unsigned)sq_bern(&r, 50));
    printf("\n");
    sq_rng_init(&r, 0xBEEF, 0x2C9E6B07u);
    for (int i = 0; i < 16; i++) a[i] = (uint8_t)i;
    sq_shuffle(&r, a, 16);
    printf("sq_shuffle seed16=0xbeef salt=0x2c9e6b07 len=16 -> "); dump_u8s(a, 16); printf("\n");
    printf("sq_mix x=0x00000001 -> 0x%08x\n", (unsigned)sq_mix(1));
    printf("sq_mix x=0xdeadbeef -> 0x%08x\n", (unsigned)sq_mix(0xDEADBEEFu));
    printf("sq_mix x=0x00000000 -> 0x%08x\n", (unsigned)sq_mix(0));
    sq_seeded_perm(a, 24, 0x1234u); printf("sq_seeded_perm n=24 x=0x1234 -> "); dump_u8s(a, 24); printf("\n");
    sq_seeded_perm(a, 24, 0u);      printf("sq_seeded_perm n=24 x=0x0 -> "); dump_u8s(a, 24); printf("\n");
    sq_seeded_perm(a, 7, 0xC0FFEEu); printf("sq_seeded_perm n=7 x=0xc0ffee -> "); dump_u8s(a, 7); printf("\n");
    /* SP2 / SP3 / SP4 */
    sq_rng_init(&r, 0x1111, 0x2C9E6B07u); sq_rank_build(&r, rank, 16, 1);
    printf("sq_rank_build seed16=0x1111 salt=0x2c9e6b07 n=16 first_fixed=1 -> "); dump_u8s(rank, 16); printf("\n");
    sq_rng_init(&r, 0x1111, 0x2C9E6B07u); sq_rank_build(&r, rank, 16, 0);
    printf("sq_rank_build seed16=0x1111 salt=0x2c9e6b07 n=16 first_fixed=0 -> "); dump_u8s(rank, 16); printf("\n");
    sq_rng_init(&r, 0x1111, 0x2C9E6B07u); sq_rank_build(&r, rank, 5, 1);
    printf("sq_rank_build seed16=0x1111 salt=0x2c9e6b07 n=5 first_fixed=1 -> "); dump_u8s(rank, 5); printf("\n");
    for (int i = 0; i < 16; i++) ord[i] = (uint8_t)(15 - i);
    { static const struct { uint16_t cells; uint8_t count; } c[] = { { 0x8421, 2 }, { 0x8421, 9 }, { 0xFFFF, 0 }, { 0xFFFF, 3 } };
      for (int i = 0; i < 4; i++) printf("sq_accent_pick ord=15..0 n=16 cells=0x%04x count=%u -> 0x%04x\n", (unsigned)c[i].cells, (unsigned)c[i].count, (unsigned)sq_accent_pick(ord, 16, c[i].cells, c[i].count)); }
    { uint8_t v[16]; sq_rng_init(&r, 5, 5); sq_vel_lane(&r, v, 16); printf("sq_vel_lane seed16=0x0005 salt=0x5 len=16 -> "); dump_u8s(v, 16); printf("\n"); }
    /* SP5: a table of (width, n, pace) cases, the ones the invariants sweep over plus the two spot checks */
    { static const uint8_t ws[] = { 3, 8, 12, 24, 48, 96, 192 };
      for (unsigned wi = 0; wi < sizeof ws; wi++) for (int n = 1; n <= 4; n++) for (int pace = -1; pace <= 1; pace++) {
        SqRep rep[3]; uint8_t k = sq_ratchet(ws[wi], (uint8_t)n, (int8_t)pace, 100, -10, 50, rep);
        printf("sq_ratchet width=%u n=%d pace=%d vel=100 slope=-10 gate=50 -> k=%u", (unsigned)ws[wi], n, pace, (unsigned)k);
        for (int j = 0; j < k; j++) printf(" [off=%u len=%u vel=%u]", (unsigned)rep[j].off, (unsigned)rep[j].len, (unsigned)rep[j].vel);
        printf("\n"); } }
    { SqRep rep[3]; uint8_t k = sq_ratchet(24, 3, 0, 100, 0, 50, rep);
      printf("sq_ratchet width=24 n=3 pace=0 vel=100 slope=0 gate=50 -> k=%u [off=%u len=%u vel=%u] [off=%u len=%u vel=%u]\n", (unsigned)k,
             (unsigned)rep[0].off, (unsigned)rep[0].len, (unsigned)rep[0].vel, (unsigned)rep[1].off, (unsigned)rep[1].len, (unsigned)rep[1].vel); }
    /* SP6 / SP9 / SP7: every scale's snap table, then notes and live keys */
    memset(&g, 0, sizeof g);
    for (int s = 0; s < 16; s++) { g.s.scale = (uint8_t)s; sq_gbuild(&g); printf("sq_gbuild scale=%d -> snap=", s); dump_u8s(g.snap, 12); printf("\n"); }
    { static const struct { uint8_t scale; int8_t key, oct; } c[] = { { 9, 0, 0 }, { 0, 5, -1 }, { 10, 0, 0 }, { 1, 7, 1 } };
      static const int16_t semis[] = { 0, 11, 13, -1, -12, 24, -50, 100 };
      for (int i = 0; i < 4; i++) {
        memset(&g, 0, sizeof g); g.s.scale = c[i].scale; g.s.key = (uint8_t)c[i].key; g.s.octave = c[i].oct; sq_gbuild(&g);
        printf("sq_note scale=%u key=%d octave=%d ->", (unsigned)c[i].scale, c[i].key, c[i].oct);
        for (unsigned j = 0; j < sizeof semis / sizeof semis[0]; j++) printf(" %d:%u", (int)semis[j], (unsigned)sq_note(&g, semis[j]));
        printf("\n"); } }
    memset(&g, 0, sizeof g); g.s.scale = 10; sq_gbuild(&g);
    { static const uint8_t ns[] = { 0, 1, 48, 49, 60, 61, 62, 127 };
      printf("sq_snap_abs scale=10 key=0 ->"); for (unsigned j = 0; j < sizeof ns; j++) printf(" %u:%u", (unsigned)ns[j], (unsigned)sq_snap_abs(&g, ns[j])); printf("\n"); }
    g.s.key = 5; sq_gbuild(&g);
    { static const uint8_t ns[] = { 0, 1, 48, 49, 60, 61, 62, 127 };
      printf("sq_snap_abs scale=10 key=5 ->"); for (unsigned j = 0; j < sizeof ns; j++) printf(" %u:%u", (unsigned)ns[j], (unsigned)sq_snap_abs(&g, ns[j])); printf("\n"); }
    { static const struct { uint8_t raw, rng; } c[] = { { 255, 48 }, { 0, 48 }, { 128, 36 }, { 100, 12 }, { 255, 0 } };
      for (int i = 0; i < 5; i++) printf("sq_span raw=%u rng=%u -> %u\n", (unsigned)c[i].raw, (unsigned)c[i].rng, (unsigned)sq_span(c[i].raw, c[i].rng)); }
    /* SP16 / SP17 */
    for (int d = 0; d < 3; d++) { printf("sq_dir_map n=4 d=%d u=0..11 ->", d); for (uint32_t u = 0; u < 12; u++) printf(" %u", (unsigned)sq_dir_map(u, 4, (uint8_t)d)); printf("\n"); }
    printf("sq_dir_map n=1 d=2 u=7 -> %u\n", (unsigned)sq_dir_map(7, 1, 2));
    printf("sq_dir_map n=16 d=2 u=0..35 ->"); for (uint32_t u = 0; u < 36; u++) printf(" %u", (unsigned)sq_dir_map(u, 16, 2)); printf("\n");
    { static const struct { uint8_t v, dyn; } c[] = { { 120, 10 }, { 40, 0 }, { 120, 5 }, { 60, 5 }, { 1, 20 }, { 127, 20 }, { 90, 7 } };
      for (int i = 0; i < 7; i++) printf("sq_dyn_vel v=%u dyn=%u -> %u\n", (unsigned)c[i].v, (unsigned)c[i].dyn, (unsigned)sq_dyn_vel(c[i].v, c[i].dyn)); }
    /* SP18..SP20 and the mod lane (fixed seeds) */
    { static const uint16_t sm[] = { 0x0000, 0x1235, 0xBEEF };
      for (int si = 0; si < 3; si++) for (int lane = 0; lane < 2; lane++) {
        printf("sp18_cell s_mod=0x%04x lane=%d slot=1..4 i=0,1,15,137,255 ->", (unsigned)sm[si], lane);
        for (int slot = 1; slot <= 4; slot++) { printf(" |"); for (int k = 0; k < 5; k++) { static const uint8_t is[5] = { 0, 1, 15, 137, 255 }; printf(" %d", (int)sp18_cell(sm[si], (uint8_t)lane, (uint8_t)slot, is[k])); } }
        printf("\n"); }
      for (int si = 0; si < 3; si++) for (int slot = 1; slot <= 4; slot += 3) {
        printf("sp19_lfo s_mod=0x%04x slot=%d period=16 t=0..15 ->", (unsigned)sm[si], slot);
        for (uint16_t t = 0; t < 16; t++) printf(" %d", (int)sp19_lfo(sm[si], (uint8_t)slot, t, 16)); printf("\n"); }
      printf("sp19_lfo s_mod=0x0003 slot=2 period=0 t=0..4 ->"); for (uint16_t t = 0; t < 5; t++) printf(" %d", (int)sp19_lfo(0x0003, 2, t, 0)); printf("\n");
      for (int sh = 0; sh < 4; sh++) { printf("sp19_lfo s_mod=0x%04x slot=1 period=64 t=0,8,16,24,32,40,48,56 ->", (unsigned)sh);
        for (uint16_t t = 0; t < 64; t += 8) printf(" %d", (int)sp19_lfo((uint16_t)sh, 1, t, 64)); printf("\n"); }
      printf("sp20_lean vel=0,40,89,90,91,100,120,127,255 ->"); { static const uint8_t vs[] = { 0, 40, 89, 90, 91, 100, 120, 127, 255 };
        for (unsigned j = 0; j < sizeof vs; j++) printf(" %d", (int)sp20_lean(vs[j])); } printf("\n");
      { ModIdx m; memset(&m, 0, sizeof m); m.a = 9; m.vel = 120; m.b_cell = 3; m.b_lfo = 0; m.t = 5; m.period = 0;
        for (int slot = 1; slot <= 4; slot++) printf("sq_mod_lane s_mod=0x1235 slot=%d a=9 vel=120 b_cell=3 b_lfo=0 t=5 period=0 len=16 -> %d\n", slot, (int)sq_mod_lane(0x1235, (uint8_t)slot, &m, 16));
        m.b_lfo = 1;
        for (int slot = 2; slot <= 4; slot += 2) printf("sq_mod_lane s_mod=0x1235 slot=%d a=9 vel=120 b_cell=3 b_lfo=1 t=5 period=0 len=16 -> %d\n", slot, (int)sq_mod_lane(0x1235, (uint8_t)slot, &m, 16));
        m.period = 8; m.vel = 60;
        for (int slot = 1; slot <= 4; slot++) printf("sq_mod_lane s_mod=0x1235 slot=%d a=9 vel=60 b_cell=3 b_lfo=1 t=5 period=8 len=16 -> %d\n", slot, (int)sq_mod_lane(0x1235, (uint8_t)slot, &m, 16)); } }
}
#endif /* SQ_PRIMS_DUMP_H */
