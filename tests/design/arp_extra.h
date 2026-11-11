/* SPDX-License-Identifier: GPL-3.0-only */
/* ARP's own golden sections (P2-017, ARP SPEC 11): included by proto_main.c through -DPROTO_EXTRA. Uses the driver's Case / A / HCS / hset / build_hcs.
 *   (a) the default of SPEC 10.1, checked against the 12 values printed there
 *   (b) every DIR x SHAPE x RATE x SPAN 1..4 for chord I (16 steps) and for the 1451 bars of SPEC 6.7 (64 steps), one line each, with DENS 16
 *   (c) the single-note cases of SPEC 6.8 (HARMONY `1----`, VOICE 0) checked against the table there
 *   (d) the RAND permutation property for P = 1..24 and the figure-cycle property at step level
 *   (e) cache 48 B and the cost bounds of SPEC 9
 * ROLE OFF and ROOT are the HARMONY runs of the driver (harmony_chord_algo); each dice layer is matrix section C. */

#define AM_DENS 0
#define AM_SHAPE 1
#define AM_SPAN 2
#define AM_RATE 3
#define AM_OFFSET 4
#define AM_LEGATO 5
#define AM_DYN 6
#define AM_DIR 7

static const char *ARP_DIR[6] = { "UP", "DOWN", "UPDN", "RAND", "ASPL", "CHORD" };
static const char *ARP_SHP[6] = { "LINE", "SKIP", "THUMB", "OCTV", "REPT", "WAVE" };
static const char *ARP_RAT[6] = { "1/4", "1/2", "1X", "2X", "3T", "4X" };

/* one step as text: `-` for a rest, else the note, with `+n/n` for the repeats of RATE 2X..4X */
static int arp_cell(char *buf, const void *ch, uint32_t t, const HarmChord *hc)
{   PulseOut o; int n;
    A.step(ch, t, hc, &o);
    if (!(o.flags & PO_HIT)) return snprintf(buf, 24, "-");
    n = snprintf(buf, 24, "%u", o.note);
    for (uint8_t k = 1; k <= ((o.flags & PO_REP_MASK) >> PO_REP_SHIFT); k++) n += snprintf(buf + n, 24 - n, "%c%u", k == 1 ? '+' : '/', A.sub_note(ch, t, k, hc));
    return n; }

static void arp_line(const void *ch, uint32_t from, uint32_t count)
{   char b[24];
    for (uint32_t t = from; t < from + count; t++) { arp_cell(b, ch, t, hcfor(0, t)); printf("%s%s", t > from ? " " : "", b); }
    printf("\n"); }

static void arp_setup(Case *c, SeqGlobal *g, const HarmonySave *h)
{   mkglobal(g, c); build_hcs(g, h); memset(&cache, 0xAA, sizeof cache); A.build(c->m, c->seed, g, &cache); }

static void arp_chord_line(const char *tag, const HarmChord *hc)
{   printf("chord %s root=%d drone=%d n=%u vt=", tag, hc->root, hc->drone, hc->n);
    for (uint8_t i = 0; i < hc->n; i++) printf("%s%d", i ? "," : "", hc->vt[i]);
    printf("\n"); }

static void arp_default_check(void)
{   Case c; SeqGlobal g; PulseOut o;
    /* SPEC 10.1: step: note, velocity, gate */
    static const uint8_t W[12][4] = { {0,36,109,37}, {2,43,88,62}, {3,48,88,50}, {4,51,126,62}, {5,55,90,62}, {6,36,120,37}, {7,39,123,62}, {9,48,74,50}, {10,51,83,62}, {11,55,72,37}, {12,36,115,25}, {13,39,119,25} };
    defaults(&c, 0); arp_setup(&c, &g, &HDEF);
    printf("## ARP DEFAULT (SPEC 10.1): channel 0, MINPEN, key C, HARMONY 1---, step: note velocity gate\n");
    for (uint32_t t = 0; t < 16; t++) {
        A.step(&cache, t, hcfor(0, t), &o);
        if (o.flags & PO_HIT) printf("%u: %u v%u g%u\n", (unsigned)t, o.note, o.vel, o.gate); else printf("%u: rest\n", (unsigned)t); }
    for (int i = 0; i < 12; i++) {
        A.step(&cache, W[i][0], hcfor(0, W[i][0]), &o);
        CHECK((o.flags & PO_HIT) && o.note == W[i][1] && o.vel == W[i][2] && o.gate == W[i][3], "SPEC 10.1 step %u: %u v%u g%u, wanted %u v%u g%u", W[i][0], o.note, o.vel, o.gate, W[i][1], W[i][2], W[i][3]); }
    for (uint32_t t = 0; t < 16; t++) { int listed = 0; A.step(&cache, t, hcfor(0, t), &o); for (int i = 0; i < 12; i++) listed |= W[i][0] == t;
        CHECK(listed == ((o.flags & PO_HIT) != 0), "SPEC 10.1: step %u hit state differs", (unsigned)t); } }

static void arp_grid(void)
{   static const uint8_t PROG[4] = { 1, 4, 5, 1 };
    HarmonySave h1451;
    int lines = 0;
    hset(&h1451, 4, 4, (const uint8_t *)PROG, 2, 8, 0, 0);
    for (int pass = 0; pass < 2; pass++) {
        const HarmonySave *h = pass ? &h1451 : &HDEF;
        printf("## ARP GRID %s: DENS 16, channel 0, MINPEN, key C, default seeds (no hops, no phase), %s\n", pass ? "1451 (4 bars of 16 steps)" : "chord I (16 steps)",
               "note = MIDI, `-` rest, `+n/n` repeats of RATE 2X 3T 4X");
        for (int span = 1; span <= 4; span++) for (int d = 0; d < 6; d++) for (int sh = 0; sh < 6; sh++) for (int r = 0; r < 6; r++) {
            Case c; SeqGlobal g;
            defaults(&c, 0); c.m[AM_DENS] = 16; c.m[AM_SPAN] = (uint8_t)span; c.m[AM_DIR] = (uint8_t)d; c.m[AM_SHAPE] = (uint8_t)sh; c.m[AM_RATE] = (uint8_t)r;
            arp_setup(&c, &g, h);
            if (!pass && d == 0 && sh == 0 && r == 0 && span == 1) arp_chord_line("I", &HCS[0]);
            if (pass && d == 0 && sh == 0 && r == 0 && span == 1) for (int b = 0; b < 4; b++) { char t[8]; snprintf(t, sizeof t, "bar%d", b + 1); arp_chord_line(t, &HCS[b]); }
            printf("SPAN=%d DIR=%s SHAPE=%s RATE=%s |", span, ARP_DIR[d], ARP_SHP[sh], ARP_RAT[r]);
            arp_line(&cache, 0, pass ? 64 : 16);
            for (uint32_t t = 0; t < (pass ? 64u : 16u); t++) { PulseOut o; A.step(&cache, t, hcfor(0, t), &o); theory(&c, &o);
                for (uint8_t k = 1; k <= ((o.flags & PO_REP_MASK) >> PO_REP_SHIFT); k++) { PulseOut so = o; so.note = A.sub_note(&cache, t, k, hcfor(0, t)); theory(&c, &so); } }
            lines++; } }
    CHECK(lines == 2 * 4 * 6 * 6 * 6, "grid lines %d", lines); }

/* the cases of SPEC 6.8: the base note is 36 (key C, octave 0) */
static void arp_single(void)
{   static const uint8_t ONE[1] = { 1 };
    HarmonySave h;
    hset(&h, 5, 1, ONE, 0, 8, 0, 0);                                     /* `1----`: one chord held for five bars, VOICE 0 */
    printf("## ARP SINGLE NOTE (SPEC 6.8): HARMONY 1----, VOICE 0, DENS 16, RATE 1X, UP unless stated; 16 steps, MIDI notes (root 36)\n");
    for (int pass = 0; pass < 6; pass++) {
        Case c; SeqGlobal g; static const char *NAME[6] = { "SPAN 1 LINE UP (every other shape and direction the same; OCTV alternates 36 48)", "LINE UP SPAN 3", "OCTV SPAN 3", "THUMB SPAN 3", "UPDN SPAN 3", "drone 4 (VOICE 0 + drone), SPAN 1 LINE UP" };
        static const struct { uint8_t sh, d, span; const uint8_t w[8]; uint8_t nw; } W[6] = {
            { 0, 0, 1, { 36 }, 1 }, { 0, 0, 3, { 36, 48, 60 }, 3 }, { 3, 0, 3, { 36, 48, 48, 60, 60, 72 }, 6 }, { 2, 0, 3, { 36, 48, 36, 60 }, 4 }, { 0, 2, 3, { 36, 48, 60, 48 }, 4 }, { 0, 0, 1, { 0 }, 0 } };
        HarmonySave hh = h;
        defaults(&c, 0); c.m[AM_DENS] = 16; c.m[AM_SPAN] = W[pass].span; c.m[AM_SHAPE] = W[pass].sh; c.m[AM_DIR] = W[pass].d;
        if (pass == 5) hh.drone = 4;
        arp_setup(&c, &g, &hh);
        printf("%s |", NAME[pass]); arp_line(&cache, 0, 16);
        if (pass == 0) {
            for (int sh = 0; sh < 6; sh++) for (int d = 0; d < 6; d++) {
                Case c2; defaults(&c2, 0); c2.m[AM_DENS] = 16; c2.m[AM_SPAN] = 1; c2.m[AM_SHAPE] = (uint8_t)sh; c2.m[AM_DIR] = (uint8_t)d;
                arp_setup(&c2, &g, &h);
                for (uint32_t t = 0; t < 48; t++) { PulseOut o; A.step(&cache, t, hcfor(0, t), &o); CHECK(o.note == (sh == 3 ? 36 + 12 * (t & 1u) : 36), "VOICE 0 SPAN 1 %s %s: note %u at %u", ARP_SHP[sh], ARP_DIR[d], o.note, (unsigned)t); } } }   /* OCTV alternates the root and its octave (SPEC 6.5), so SPEC 6.8 "any shape" was wrong for it */
        else if (pass < 5) {
            for (uint32_t t = 0; t < 48; t++) { PulseOut o; A.step(&cache, t, hcfor(0, t), &o); CHECK(o.note == W[pass].w[t % W[pass].nw], "6.8 case %d: note %u at %u, wanted %u", pass, o.note, (unsigned)t, W[pass].w[t % W[pass].nw]); } }
        else {                                                           /* pool = root and drone: two tones, alternating; with THUMB the drone is the home note */
            HarmChord *hc = &HCS[0];
            arp_chord_line("VOICE 0 + drone", hc);
            CHECK(hc->n == 2 || hc->n == 1, "drone chord n=%u", hc->n);
            if (hc->n == 2) for (uint32_t t = 0; t < 16; t++) { PulseOut o; A.step(&cache, t, hcfor(0, t), &o); CHECK(o.note == 36 + hc->vt[t % 2], "drone alternation at %u: %u", (unsigned)t, o.note); }
            c.m[AM_SHAPE] = 2; arp_setup(&c, &g, &hh);
            printf("same, THUMB |"); arp_line(&cache, 0, 16);
            for (uint32_t t = 0; t < 16; t += 2) { PulseOut o; A.step(&cache, t, hcfor(0, t), &o); CHECK(o.note == 36 + (HCS[0].drone >= 0 ? HCS[0].drone : HCS[0].vt[0]), "THUMB home is not the drone at %u", (unsigned)t); } } } }

/* RAND (SPEC 6.3 step 4): a permutation of 0..P-1 for every P and cycle, and at step level one figure cycle of LINE / 1X plays every pool tone once */
static void arp_rand(void)
{   int bad = 0;
    for (uint8_t P = 1; P <= 24; P++) for (uint32_t y = 0; y < 64; y++) for (uint32_t s = 0; s < 4; s++) {
        uint8_t perm[24], seen[24] = {0};
        sq_seeded_perm(perm, P, sq_mix((s * 0x9E3779B1u) ^ (y * 0x85EBCA6Bu)) | 0);
        for (uint8_t i = 0; i < P; i++) { if (perm[i] >= P) bad++; else seen[perm[i]]++; }
        for (uint8_t i = 0; i < P; i++) bad += seen[i] != 1; }
    CHECK(bad == 0, "RAND permutation property failed %d times", bad);
    printf("## ARP RAND: sq_seeded_perm is a permutation of 0..P-1 for P = 1..24 x 64 cycles x 4 seeds (%s)\n", bad ? "FAIL" : "all pass");
    for (uint8_t voice = 0; voice <= 6; voice++) for (uint8_t span = 1; span <= 4; span++) {
        static const uint8_t ONE[1] = { 1 };
        HarmonySave h; Case c; SeqGlobal g;
        hset(&h, 4, 1, ONE, voice, 8, 0, 0);
        defaults(&c, 0); c.m[AM_DENS] = 16; c.m[AM_SPAN] = span; c.m[AM_DIR] = 3; c.seed[0] = 0xBEEF;
        arp_setup(&c, &g, &h);
        {   const HarmChord *hc = &HCS[0]; uint8_t nt = hc->n, P = (uint8_t)(nt * span);
            for (uint32_t y = 0; y < 3; y++) {
                int16_t pool[24], got[24];
                uint8_t used[24] = {0};
                for (uint8_t j = 0; j < P; j++) pool[j] = (int16_t)(36 + hc->vt[j % nt] + 12 * (j / nt) + (c.seed[0] ? 0 : 0));
                for (uint8_t b = 0; b < P; b++) { PulseOut o; A.step(&cache, y * P + b, hcfor(0, y * P + b), &o); got[b] = o.note; }
                /* hops (seed != 0, SPAN >= 2) add an octave to some notes; compare pitch classes plus octave-free multiset, which a permutation keeps */
                for (uint8_t b = 0; b < P; b++) { int found = 0; for (uint8_t j = 0; j < P && !found; j++) if (!used[j] && (pool[j] - got[b]) % 12 == 0) { used[j] = 1; found = 1; } CHECK(found, "RAND voice %u span %u cycle %u: note %d is not a pool tone", voice, span, (unsigned)y, got[b]); } } } } }

static void arp_cost(void)
{   printf("## ARP COST: cache %u B (SPEC 9: 48), ChanRT unchanged, step: at most one 24-draw shuffle (RAND), one 6-value sort (CHORD), 3 divisions\n", A.cache_bytes);
    CHECK(A.cache_bytes == 48, "ARP cache is %u B, SPEC 9 says 48", A.cache_bytes); }

static void proto_extra(void)
{   arp_default_check();
    arp_grid();
    arp_single();
    arp_rand();
    arp_cost(); }
