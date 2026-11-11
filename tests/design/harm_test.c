/* SPDX-License-Identifier: GPL-3.0-only */
/* H-GOLDEN (HARMONY.md 10): prints the tables of 3.1, 3.2, 3.3, 4.2 and 6.1 from harm.c in exactly the text of
 * `python3 tests/design/harmony_proto.py --dump`; run_protos.sh diffs the two. Then the query interface checks (section 9). */
#include <stdio.h>
#include <string.h>
#include "sq_types.h"
#include "sq_prims.h"
#include "harm.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; fprintf(stderr, "FAIL harm: "); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

static const char *SN[3] = { "MAJOR", "NATMIN", "MINPEN" };
static const uint8_t SID[3] = { 1, 2, 10 };

static void mk(SeqGlobal *g, HarmonySave *h, uint8_t scale, uint8_t voice, uint8_t move, uint8_t drone)
{   memset(g, 0, sizeof *g); g->s.scale = scale; g->s.gate = 50; g->s.div = 2; g->s.swing = 50; sq_gbuild(g);
    memset(h, 0, sizeof *h);
    h->len = (uint8_t)(3 | (3 << 3));                       /* 4 bars, 4 chords */
    h->deg[0] = (uint8_t)(0 | (3 << 3) | (4 << 6)); h->deg[1] = (uint8_t)(4 >> 2);   /* degree-1 = 0 3 4 0 : slot 2 spans bits 6..8 */
    h->vm = (uint8_t)(voice | (move << 4)); h->drone = drone; }

int main(void)
{   SeqGlobal g; HarmonySave h;
    for (int si = 0; si < 3; si++) for (int vo = 0; vo < 7; vo++) for (int mv = 0; mv < 9; mv++) {
        mk(&g, &h, SID[si], (uint8_t)vo, (uint8_t)mv, 0); harm_bind(&g, &h);
        printf("PROG %s v%d m%d", SN[si], vo, mv);
        for (int sl = 0; sl < 4; sl++) {
            int8_t vt[HARM_MAXV]; uint8_t n = harm_voicing((uint8_t)sl, vt);
            printf(sl ? " | " : " ");
            for (uint8_t i = 0; i < n; i++) printf("%s%d", i ? " " : "", vt[i]); }
        printf("\n"); }
    for (int si = 0; si < 3; si++) {
        mk(&g, &h, SID[si], 2, 8, 0); harm_bind(&g, &h);
        for (int p = 1; p < (si < 2 ? 15 : 12); p++) {
            printf("DRONE %s p%d pitch %d ", SN[si], p, harm_drone_pitch((uint8_t)p));
            for (int sl = 0; sl < 4; sl++) { int8_t vt[HARM_MAXV]; uint8_t n = harm_voicing((uint8_t)sl, vt); putchar(harm_drone_mark(vt, n, harm_drone_pitch((uint8_t)p))); }
            printf("\n"); } }
    for (int si = 0; si < 3; si += 2) {
        HarmChord c;
        mk(&g, &h, SID[si], 2, 8, 0); harm_bind(&g, &h); harm_chord_at_bar(1, &c);
        CHECK(c.slot == 1 && c.deg == 4 && c.active, "chord of bar 1");
        for (int pull = 3; pull <= 8; pull += 5) for (int first = 0; first < 2; first++) for (int t = 0; t < 16; t++) for (int s = 0; s < 25; s++)
            printf("PULL %s pull%d first%d t%d s%d -> %d\n", SN[si], pull, first, t, s, harm_pull_note(&c, (int16_t)s, (uint8_t)t, (uint8_t)(t % 4 == 0), (uint8_t)pull, (uint8_t)first, 0, 24)); }
    for (int ns = 7; ns >= 5; ns -= 2) {
        uint16_t seeds[210]; int n = 0;
        seeds[n++] = 0x0001; seeds[n++] = 0x1234; seeds[n++] = 0xBEEF; seeds[n++] = 0xFFFF;
        for (int sd = 2; sd < 202; sd += 3) seeds[n++] = (uint16_t)sd;
        for (int i = 0; i < n; i++) {
            uint8_t bars, degs[8], nd;
            harm_dice(seeds[i], (uint8_t)ns, &bars, degs, &nd);
            printf("DICE n%d %04X bars %d degs", ns, seeds[i], bars);
            for (int k = 0; k < nd; k++) printf(" %d", degs[k]);
            printf("\n"); } }
    /* ---- the query interface (HARMONY.md 9) ---- */
    {   HarmChord c, d;
        mk(&g, &h, 1, 2, 8, 0); harm_bind(&g, &h);
        CHECK(harm_active(), "1451 is active");
        harm_chord_at_bar(0, &c); harm_chord_at_bar(4, &d);
        CHECK(memcmp(&c, &d, sizeof c) == 0, "bar taken modulo bars");
        harm_chord_at_bar(2, &c);
        CHECK(c.slot == 2 && c.deg == 5 && c.root == 7 && c.n == 3 && c.vt[0] == 7 && c.vt[1] == 11 && c.vt[2] == 14, "V chord");
        CHECK(c.pcs == ((1u << 7) | (1u << 11) | (1u << 2)), "V pcs %x", c.pcs);
        CHECK(harm_tone(&c, 0) == 7 && harm_tone(&c, 3) == 19 && harm_tone(&c, 4) == 23, "harm_tone");
        CHECK(harm_snap(&c, 8, HQ_PC) == 7 && harm_snap(&c, 9, HQ_PC) == 7 && harm_snap(&c, 10, HQ_PC) == 11 && harm_snap(&c, 13, HQ_VOICED) == 14 && harm_snap(&c, 20, HQ_ROOT) == 19, "harm_snap");
        CHECK(harm_ticks_to_change(0) == 768 && harm_ticks_to_change(100) == 668 && harm_ticks_to_change(3 * 768) == 768, "ticks to change");
        harm_set_time(2 * 768 + 5); harm_chord_now(&d); CHECK(d.slot == 2, "chord_now");
        h.len = 3;                                          /* nchords 1, bars 4 */
        CHECK(harm_ticks_to_change(768) == 3 * 768, "single chord, wraps at the phrase end");
        h.len = (uint8_t)(7 | (2 << 3)); /* 8 bars, 3 chords: starts 0 2 5 */
        harm_set_time(0); CHECK(harm_ticks_to_change(0) == 2 * 768 && harm_ticks_to_change(5 * 768) == 3 * 768, "8 bars 3 chords");
        h.drone = 1;                                        /* a drone adds its pitch class, and a voice for FULL */
        harm_chord_at_bar(0, &c); CHECK(c.drone == 0 && (c.pcs & 1u), "drone in pcs");
        h.drone = 4; harm_chord_at_bar(0, &c); CHECK(c.drone == 5 && c.n == 4 && ((c.pcs >> 5) & 1u), "drone F is a 4th voice of the I chord (%d)", c.n); }
    /* roles (4.2 1..3) */
    {   HarmChord c; int16_t x;
        mk(&g, &h, 1, 2, 8, 0); harm_bind(&g, &h); harm_reset();
        h.roles = (uint8_t)(HR_OFF | (HR_ROOT << 2) | (HR_FULL << 4));
        x = harm_apply(0, 0, 0, 9, 0, 24, 3, 768); CHECK(x == 9, "OFF returns s");
        x = harm_apply(1, 0, 0, 9, 0, 24, 3, 768); CHECK(x == 5, "ROOT of IV in the octave of s: %d", x);
        x = harm_apply(1, 0, 0, 21, 0, 24, 3, 768); CHECK(x == 17, "ROOT octave 1: %d", x);
        harm_chord_at_bar(1, &c);
        x = harm_apply(2, 0, 0, 0, 0, 24, 3, 768); CHECK(x == c.vt[0], "FULL lowest %d", x);
        x = harm_apply(2, 0, 0, 24, 0, 24, 3, 768); CHECK(x == c.vt[2], "FULL highest %d", x);
        mk(&g, &h, 1, 2, 8, 0); harm_bind(&g, &h); harm_reset();
        /* AUTO anchor: the first call after PLAY and after each chord change is the chord root, the second is not */
        x = harm_apply(3, 16, 1, 2, 0, 24, 3, 768); CHECK(x == 5, "anchor of IV: %d", x);
        x = harm_apply(3, 17, 0, 2, 0, 24, 3, 768 + 48); CHECK(x == 2, "second hit keeps the scale tone: %d", x);
        x = harm_apply(3, 0, 1, 2, 0, 24, 3, 2 * 768); CHECK(x == 7, "anchor of V: %d", x); }
    if (fails) { fprintf(stderr, "harm: %d check(s) failed\n", fails); return 1; }
    fprintf(stderr, "harm: query interface checks OK\n");
    return 0; }
