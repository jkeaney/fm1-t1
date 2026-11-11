/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Host test of the stored record (firmware/src/project.c, -DPROJ_HOST part): the FUN10 record ("FUNA", 3840 bytes) that
 * the autosave writes. The formats FUN1..FUN9 and the project slots were removed in P3-011. Round trip (a matrix slot,
 * an engine number, a name), the hash, a wrong size or magic refused.
 * Run by tests/run_tests.sh (needs build/gen from one firmware build). */
#define main hostsim_main
#include "hostsim.c"
#undef main
#define PROJ_HOST 1                             /* (trk_def_engine: engines.c, through hostsim.c) */
#include "../firmware/src/project.c"

static int check(const char *what, int ok)
{
    printf("project: %-74s %s\n", what, ok ? "ok" : "FAIL");
    return ok ? 0 : 1;
}

int main(void)
{
    static project_t q, q2;
    static project_store_t st;
    int bad = 0, ok;

    bad += check("FUN10 fits one flash object", sizeof(project_store_t) <= 4096u - 256u && sizeof(project_store_t) == 3840u);
    memset(&q, 0, sizeof q);
    q.magic = PROJ_MAGIC; q.size = sizeof q; q.parts = NPART; q.phys = PROJ_PHYS;
    q.t[1].engine = 0;
    q.t[2].p[P_M2SRC] = 6;
    q.t[2].p[P_M2DST] = 11;
    q.t[2].p[P_M2AMT] = -17;
    q.t[0].step[0] = (step_t){{36, 42, 0, 0}, 2, ST_NOTE, 0, 96, 0, 0};
    q.sum = proj_sum(&q);
    ok = proj_pack(&st, &q) && proj_import(&q2, &st, sizeof st);
    bad += check("FUN10 round trip: a matrix slot, a two-note step", ok && !memcmp(&q, &q2, sizeof q) &&
                 q2.t[2].p[P_M2AMT] == -17 && q2.t[0].step[0].n == 2u);
    st.raw[100] ^= 1u;
    bad += check("FUN10 with a changed byte: the hash refuses it", !proj_import(&q2, &st, sizeof st));
    st.raw[100] ^= 1u;
    bad += check("FUN10 with a wrong length: refused", !proj_import(&q2, &st, (int)sizeof st - 2));
    ((uint32_t *)st.raw)[0] = 0x46554E39u;     /* "FUN9" */
    bad += check("FUN10 size with a FUN9 magic: refused", !proj_import(&q2, &st, (int)sizeof st));
    bad += check("the layout: name, patches, MIDI IN TRACK bytes, the reserved song and motion bytes in place",
                 PROJ_NAME_OFF == 3824u && PROJ_FM6_OFF == 3312u && 68u + NTRK * (P_COUNT + 2u + NSTEP * 9u) +
                 PROJ_CHAIN_BYTES + PROJ_MOTION_BYTES + 16u == PROJ_FM6_OFF);

    {   /* the FUN7 name: the reserved tail's last 12 bytes, covered by the hash; zero = no name */
        project_t a, c;
        project_store_t st, st2;
        uint32_t i, zero = 1;
        bad += check("FUN10 name at the end of the reserved tail, the FM6 patches before it, after the data (16 spare)",
                     PROJ_NAME_OFF == 3824u && PROJ_FM6_OFF == 3312u && 68u + NTRK * (P_COUNT + 2u + NSTEP * 9u) +
                     PROJ_CHAIN_BYTES + PROJ_MOTION_BYTES + 16u == PROJ_FM6_OFF);
        memset(&a, 0, sizeof a);
        a.magic = PROJ_MAGIC; a.size = sizeof a; a.parts = NPART; a.phys = PROJ_PHYS;
        a.t[1].p[P_LEVEL] = 99;
        a.sum = proj_sum(&a);
        ok = proj_pack(&st, &a);
        for (i = 0; i < PROJ_NAME_LEN; i++) zero &= st.raw[PROJ_NAME_OFF + i] == 0u;
        bad += check("FUN7 no name: the tail all zero (as firmware before wrote it)", ok && zero);
        bad += check("FUN7 of before names (zero tail): loads, no name", proj_import(&c, &st, sizeof st) && !c.name[0] &&
                     c.t[1].p[P_LEVEL] == 99);
        memcpy(a.name, "lofi jam", 8);
        a.sum = proj_sum(&a);
        ok = proj_pack(&st, &a) && proj_import(&c, &st, sizeof st);
        bad += check("FUN7 name round trip (upper case, 0-padded)", ok && !memcmp(c.name, "LOFI JAM\0\0\0\0", 12) &&
                     !memcmp(st.raw + PROJ_NAME_OFF, "LOFI JAM\0\0\0\0", 12));
        memcpy(a.name, "ABCDEFGHIJKL", 12);         /* 12: no 0 */
        a.sum = proj_sum(&a);
        ok = proj_pack(&st, &a) && proj_import(&c, &st, sizeof st);
        bad += check("FUN7 name of 12 characters (no terminator)", ok && !memcmp(c.name, "ABCDEFGHIJKL", 12));
        st2 = st;
        st2.raw[PROJ_NAME_OFF + 3] ^= 1u;
        bad += check("FUN7 name byte changed: the hash refuses the project", !proj_import(&c, &st2, sizeof st2));
        st2 = st;
        st2.raw[PROJ_NAME_OFF + 3] = 7u;            /* not printable, the hash right */
        {
            uint32_t sum = proj_hash(st2.raw, PROJ_STORE_SIZE - 4u);
            memcpy(st2.raw + PROJ_STORE_SIZE - 4u, &sum, 4);
        }
        bad += check("FUN7 name with a byte outside 32..126: the project loads, no name",
                     proj_import(&c, &st2, sizeof st2) && !c.name[0] && c.t[1].p[P_LEVEL] == 99);
    }

    printf("%s\n", bad ? "PROJECT FORMAT TEST FAILED" : "project format test passed");
    return bad != 0;
}
