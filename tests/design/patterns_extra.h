/* SPDX-License-Identifier: GPL-3.0-only */
/* PATTERNS' own golden sections (P2-017), included by proto_main.c through -DPROTO_EXTRA:
 *   (a) PATTERNS-COMPACT: for the first pattern of the golden set of each of the six categories, every dial at min / mid / max, printed in the format of
 *       patterns_golden.txt (semitone, S slide, T tie, :velocity; the accent letter is left out because PulseOut carries none). patterns_ref.py compares it,
 *       line by line, with patterns_golden.txt, which patterns_proto.py (the Python prototype, P2-014) wrote: two implementations, one text.
 *   (b) PATTERNS-LIBRARY: all 120 patterns x 16 scales x 3 keys at the neutral dials and at every dial's extreme: every note is in the scale (HARMONY inert),
 *       the neutral dials play exactly the stored steps, DENS 1 plays exactly the anchors, and a chord pull stays inside the chord. */
#include "../../docs/design/algorithms/patterns/patterns_data.h"

#define PM_DENS 0
#define PM_VARY 1
#define PM_SPAN 2
#define PM_ACCNT 3
#define PM_PICK 4
#define PM_SLIDE 5
#define PM_DYN 6
#define PM_CTONE 7

static void pat_golden_cases(void)
{   static const struct { const char *name; int id; } GP[6] = { { "ACID 1", 0 }, { "HOUSE 3", 22 }, { "FUNK 4", 43 }, { "TECHNO 1", 60 }, { "DUB 3", 82 }, { "MELODY 1", 100 } };
    static const struct { const char *lab; int m; int v[3]; } GC[7] = { { "DENS", PM_DENS, { 1, 8, 16 } }, { "VARY", PM_VARY, { 0, 4, 8 } }, { "SPAN", PM_SPAN, { 1, 2, 4 } },
        { "ACCNT", PM_ACCNT, { 0, 4, 8 } }, { "SLIDE", PM_SLIDE, { 0, 4, 8 } }, { "DYN", PM_DYN, { 0, 10, 14 } }, { "CTONE", PM_CTONE, { 0, 2, 8 } } };
    printf("## PATTERNS-COMPACT: MINPEN, key C, seeds 1 2 3 4 5; semitone above the key, S slide into the next step, T tie, :velocity, . rest (accent letter omitted)\n");
    for (int p = 0; p < 6; p++) {
        printf("== %s\n", GP[p].name);
        for (int cs = 0; cs < 7; cs++) for (int vi = 0; vi < 3; vi++) {
            Case c; SeqGlobal g; PulseOut o[32]; HarmChord hc;
            uint8_t len;
            defaults(&c, 0);
            for (int k = 0; k < 5; k++) c.seed[k] = (uint16_t)(k + 1);
            c.m[PM_PICK] = (uint8_t)(GP[p].id + 1);
            c.m[GC[cs].m] = (uint8_t)GC[cs].v[vi];
            mkglobal(&g, &c);
            memset(&cache, 0xAA, sizeof cache); A.build(c.m, c.seed, &g, &cache);
            len = A.loop_len(&cache);
            memset(&hc, 0, sizeof hc);
            if (GC[cs].m == PM_CTONE) { hc.active = 1; hc.n = 3; hc.vt[0] = 5; hc.vt[1] = 7; hc.vt[2] = 10; hc.pcs = (uint16_t)((1u << 7) | (1u << 10) | (1u << 5)); }
            for (uint8_t i = 0; i < len; i++) A.step(&cache, i, &hc, &o[i]);
            printf("%-6s %-2d |", GC[cs].lab, GC[cs].v[vi]);
            for (uint8_t i = 0; i < len; i++) {
                if (!(o[i].flags & PO_HIT)) { printf(" ."); continue; }
                printf(" %d%s%s:%u", o[i].note - 36, (o[(i + 1) % len].flags & PO_SLIDE_IN) ? "S" : "", (o[i].flags & PO_TIE) ? "T" : "", o[i].vel); }
            printf("\n"); } } }

/* decode a stored step the way the generator wrote it */
static int pat_present(uint16_t w) { return w != 0; }
static int pat_opt(uint16_t w) { return (w >> 15) & 1; }
static int pat_rank(uint16_t w) { return (w >> 11) & 3; }

static void pat_library(void)
{   long checks = 0;
    static const uint8_t KEYS[3] = { 0, 5, 10 };
    int dense_bad = 0, neutral_bad = 0;
    for (int id = 0; id < PAT_N; id++) {
        const PatHead *hd = &PAT_HEAD[id];
        for (int sc = 0; sc < 16; sc++) for (int ki = 0; ki < 3; ki++) for (int ex = 0; ex < 3; ex++) {
            Case c; SeqGlobal g; PulseOut o;
            defaults(&c, 0); c.scale = (uint8_t)sc; c.key = KEYS[ki];
            c.m[PM_PICK] = (uint8_t)(id + 1);
            if (ex == 1) { c.m[PM_DENS] = 16; c.m[PM_VARY] = 8; c.m[PM_SPAN] = 1; c.m[PM_ACCNT] = 8; c.m[PM_SLIDE] = 8; c.m[PM_DYN] = 14; c.m[PM_CTONE] = 8; }
            if (ex == 2) { c.m[PM_DENS] = 1; c.m[PM_VARY] = 8; c.m[PM_SPAN] = 1; c.m[PM_ACCNT] = 0; c.m[PM_SLIDE] = 0; c.m[PM_DYN] = 0; c.m[PM_CTONE] = 0; }
            mkglobal(&g, &c);
            memset(&cache, 0xAA, sizeof cache); A.build(c.m, c.seed, &g, &cache);
            CHECK(A.loop_len(&cache) == hd->len, "pattern %d: loop_len %u, head %u", id + 1, A.loop_len(&cache), hd->len);
            for (uint8_t i = 0; i < hd->len; i++) {
                uint16_t w = PAT_STEP[hd->first + i];
                int want = pat_present(w) && !pat_opt(w) && (ex != 2 || pat_rank(w) == 0);
                A.step(&cache, i, 0, &o);
                theory(&c, &o);
                checks++;
                if (ex == 0) { if (((o.flags & PO_HIT) != 0) != want) neutral_bad++; }       /* neutral dials: exactly the stored on-steps (anchors and decoration, no optional adds) */
                if (ex == 2 && ((o.flags & PO_HIT) != 0) != want) dense_bad++;                /* DENS 1: the anchors only */
                if (ex == 1 && !(o.flags & PO_HIT)) continue; } } }
    CHECK(neutral_bad == 0, "neutral dials differ from the stored pattern in %d steps", neutral_bad);
    CHECK(dense_bad == 0, "DENS 1 is not the anchor set in %d steps", dense_bad);
    printf("## PATTERNS-LIBRARY: %d patterns x 16 scales x 3 keys x 3 dial sets: %ld step checks, all notes in the scale (HARMONY inert), neutral = stored steps, DENS 1 = anchors (%s)\n",
           PAT_N, checks, (neutral_bad || dense_bad) ? "FAIL" : "all pass");
    CHECK(PAT_N == 120 && PAT_NSTEP == 1996, "library size %d / %d", PAT_N, PAT_NSTEP); }

static void proto_extra(void)
{   pat_golden_cases();
    pat_library(); }
