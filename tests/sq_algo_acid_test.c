/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_algo_acid_test.c (P5-001): the firmware ACID (firmware/src/sq_algo_acid.c via the ALGOS[] row of sq_algos.c) against the locked prototype.
 * Derived from tests/design/proto_main.c (the matrix, the purity tests (a)-(g) and the table tests of ADR-0003 section 14), minus the HARMONY runs.
 * stdout = the plain part of the golden matrix: it must equal docs/design/algorithms/acid/golden.txt up to (not including) the `## H-INERT` line;
 * the H-INERT and HARM 1-4-5-1 sections need harm.c and are checked by P5-005's test (orchestrator decision 2026-11-12). stderr = test results.
 *   cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -fno-strict-aliasing -Ifirmware/src -o /tmp/t tests/sq_algo_acid_test.c && /tmp/t
 * Hosted libc is fine here: this is the test, not the firmware. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"
#include "sq_algo_acid.c"
#include "sq_algos.c"

#define A ALGOS[SQ_ALGO_ACID]
#define NT 128

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; fprintf(stderr, "FAIL %s: ", A.name); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

static const char *SCN[16] = { "CHROM","MAJOR","MINOR","DORIAN","PHRYG","LYDIAN","MIXO","AEOL","LOCRI","MAJPEN","MINPEN","BLUES","WHOLE","HARMIN","MELMIN","DIM" };
static const char *LAYN[SQ_NLAYER] = { "NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE" };
#define SC_CHROM 0
#define SC_MAJOR 1
#define SC_MINPEN 10

typedef struct { uint8_t ch, scale, key; int8_t oct; uint8_t m[SQ_NMORPH]; uint16_t seed[SQ_NSEED]; uint16_t smod; } Case;

static union { uint32_t w[16]; uint8_t b[SQ_CACHE_MAX]; } cache;

static void defaults(Case *c, uint8_t ch)
{   memset(c, 0, sizeof *c); c->ch = ch; c->scale = SC_MINPEN; c->smod = SQ_SMOD_DEFAULT(ch);
    for (int i = 0; i < SQ_NMORPH; i++) c->m[i] = A.morph[i].def;
    for (int k = 0; k < A.n_seed; k++)
        c->seed[k] = A.seed_k[k] == 0xFF ? 0 : (uint16_t)((0x3A5Du * (uint32_t)(ch * A.n_seed + A.seed_k[k])) & 0xFFFFu); }

static const HarmChord *hcfor(const HarmChord *given, uint32_t t) { (void)t; return given; }   /* ACID is not AF_CHORD */

static void mkglobal(SeqGlobal *g, const Case *c)
{   memset(g, 0, sizeof *g); g->s.key = c->key; g->s.octave = c->oct; g->s.scale = c->scale; g->s.gate = 50; g->s.div = 2; g->s.swing = 50; sq_gbuild(g); }

/* the dice of ADR-0003 4.1 with a fixed "entropy" E */
static void dice(Case *c, uint8_t layer, uint16_t e)
{   for (int j = 0; j < A.layer[layer].n; j++) {
        uint8_t id = A.layer[layer].seed[j];
        uint16_t v = j ? (uint16_t)(e ^ 0x5A5A) : e;
        if (id == SQ_SEED_MOD) { if (v == c->smod) v = (uint16_t)(v + 0x9E37); if (!v) v = 1; c->smod = v; continue; }   /* s_mod: ModSave, never 0 */
        if (v == c->seed[id]) v = (uint16_t)(v + 0x9E37);
        if (A.seed_k[id] == 0xFF && v == 0) v = 1;
        c->seed[id] = v; } }

static void hdr(const char *name, const Case *c)
{   printf("## %s ch=%u scale=%s key=%u oct=%d m=", name, c->ch, SCN[c->scale], c->key, c->oct);
    for (int i = 0; i < SQ_NMORPH; i++) printf("%s%u", i ? "," : "", c->m[i]);
    printf(" seeds=");
    for (int k = 0; k < A.n_seed; k++) printf("%s%04x", k ? "," : "", c->seed[k]);
    printf(" smod=%04x\n", c->smod); }

/* theory: every note of the scale (relative to the key), inside [base - 12, base + hi(m)] */
static void theory(const Case *c, const PulseOut *o)
{   int base = 36 + c->key + 12 * c->oct, s = o->note - base, pc = ((o->note - c->key) % 12 + 12) % 12;
    CHECK((SQ_SCALE_MASKS[c->scale] >> pc) & 1u, "note %d (pc %d) not in scale %s key %u", o->note, pc, SCN[c->scale], c->key);
    CHECK(s >= -12 && s <= A.hi(c->m), "note %d: semitone %d outside -12..%u", o->note, s, A.hi(c->m));
    CHECK(o->vel >= 1 && o->vel <= 127, "vel %u", o->vel);
    CHECK(o->gate >= 1 && o->gate <= 100, "gate %u", o->gate); }

/* the four slot values of the mod gen at step g (slot 1 and 3 lane A, 2 and 4 lane B), channel LENGTH 16 */
static void lanes(uint16_t smod, const void *ch, uint32_t g, const HarmChord *hc, const PulseOut *o, int8_t out[4])
{   ModIdx mi;
    A.mod(ch, g, hc, o, &mi);
    for (uint8_t sl = 1; sl <= 4; sl++) out[sl - 1] = sq_mod_lane(smod, sl, &mi, 16); }

static void run_case(const char *name, const Case *c)
{   SeqGlobal g; PulseOut o;
    mkglobal(&g, c);
    memset(&cache, 0xAA, sizeof cache);
    A.build(c->m, c->seed, &g, &cache);
    hdr(name, c);
    printf("t note vel gate flags hold lanes(slots 1..4)%s\n", A.sub_note ? " subnotes" : "");
    for (uint32_t t = 0; t < NT; t++) {
        const HarmChord *hc = hcfor(0, t);
        int8_t ln[4];
        A.step(&cache, t, hc, &o);
        theory(c, &o);
        lanes(c->smod, &cache, t, hc, &o, ln);
        printf("%3u %3u %3u %3u %02x %u %4d %4d %4d %4d", t, o.note, o.vel, o.gate, o.flags, o.hold, ln[0], ln[1], ln[2], ln[3]);
        if (A.sub_note && (o.flags & PO_HIT) && (o.flags & PO_REP_MASK)) {
            uint8_t reps = (uint8_t)((o.flags & PO_REP_MASK) >> PO_REP_SHIFT);
            printf("  +");
            for (uint8_t k = 1; k <= reps; k++) {
                PulseOut so = o; so.note = A.sub_note(&cache, t, k, hc);
                theory(c, &so);
                printf("%s%u", k > 1 ? "/" : "", so.note); } }
        printf("\n"); } }

static void matrix(void)
{   Case c; char nm[64];
    /* A. channel 0..1 x MINPEN, CHROM x key 0, 5 x octave 0, -1 at the default morphs and seeds */
    for (uint8_t ch = 0; ch < 2; ch++) for (int sci = 0; sci < 2; sci++) for (int ki = 0; ki < 2; ki++) for (int oi = 0; oi < 2; oi++) {
        defaults(&c, ch); c.scale = sci ? SC_CHROM : SC_MINPEN; c.key = ki ? 5 : 0; c.oct = oi ? -1 : 0;
        run_case("base", &c); }
    /* B. every morph at min / mid / max (ROW 1 and ROW 2), the rest at default */
    for (int id = 0; id < SQ_NMORPH; id++) for (int w = 0; w < 3; w++) {
        const MorphDesc *d = &A.morph[id];
        defaults(&c, 0);
        c.m[id] = w == 0 ? d->min : w == 1 ? (uint8_t)((d->min + d->max) / 2) : d->max;
        snprintf(nm, sizeof nm, "morph %s %s", d->label, w == 0 ? "min" : w == 1 ? "mid" : "max");
        run_case(nm, &c); }
    /* C. each of the five dice layers, two diced seeds each */
    for (int l = 0; l < SQ_NLAYER; l++) for (int e = 0; e < 2; e++) {
        uint16_t E = e ? 0x1234 : 0xBEEF;
        defaults(&c, 0); dice(&c, (uint8_t)l, E);
        snprintf(nm, sizeof nm, "dice %s %s E=%04x", LAYN[l], A.sub[l], E);
        if (!A.layer[l].n) {                                  /* an absent layer is a no-op: no seed moves (ALGORITHM_SET 4.4) */
            Case d0; defaults(&d0, 0);
            CHECK(memcmp(c.seed, d0.seed, sizeof c.seed) == 0, "absent layer %s changed a seed", LAYN[l]);
            printf("## dice %s absent (no change)\n", LAYN[l]); continue; }
        run_case(nm, &c); }
    /* D. edited-morph combinations on a CHROM / key 5 / octave -1 channel 1 with diced seeds */
    defaults(&c, 1); c.scale = SC_CHROM; c.key = 5; c.oct = -1;
    for (int i = 0; i < SQ_NMORPH; i++) c.m[i] = A.morph[i].min;
    run_case("combo all-min", &c);
    for (int i = 0; i < SQ_NMORPH; i++) c.m[i] = A.morph[i].max;
    run_case("combo all-max", &c);
    defaults(&c, 1); c.scale = SC_CHROM; c.key = 5; c.oct = -1;
    for (int l = 0; l < SQ_NLAYER; l++) dice(&c, (uint8_t)l, (uint16_t)(0xC0DE + 0x1111 * l));
    run_case("combo all-dice", &c); }

/* ---- purity and table tests (ADR-0003 section 14) ---- */
static uint32_t lcg(uint32_t *s) { *s = *s * 1664525u + 1013904223u; return *s >> 8; }

static void steps(const void *ch, PulseOut *out, const HarmChord *hc, int order)
{   for (uint32_t i = 0; i < NT; i++) {
        uint32_t t = order == 0 ? i : order == 1 ? NT - 1 - i : (i * 37u + 5u) % NT;
        A.step(ch, t, hcfor(hc, t), &out[t]); } }

static void purity(void)
{   uint32_t rs = 12345;
    for (int n = 0; n < 40; n++) {
        Case c; SeqGlobal g, g2; PulseOut ref[NT], o2[NT]; HarmChord hc;
        union { uint32_t w[16]; uint8_t b[SQ_CACHE_MAX]; } c1, c2;
        defaults(&c, (uint8_t)(lcg(&rs) % 4)); c.scale = (uint8_t)(lcg(&rs) % 16); c.key = (uint8_t)(lcg(&rs) % 12); c.oct = (int8_t)((int)(lcg(&rs) % 5) - 2);
        if (n) { for (int i = 0; i < SQ_NMORPH; i++) c.m[i] = (uint8_t)(A.morph[i].min + lcg(&rs) % (A.morph[i].max - A.morph[i].min + 1u));
                 for (int k = 0; k < A.n_seed; k++) c.seed[k] = (uint16_t)lcg(&rs); }
        mkglobal(&g, &c);
        memset(&c1, 0xAA, sizeof c1); memset(&c2, 0x55, sizeof c2);
        A.build(c.m, c.seed, &g, &c1); A.build(c.m, c.seed, &g, &c2);
        CHECK(memcmp(&c1, &c2, A.cache_bytes) == 0, "(a) build twice differs, case %d", n);       /* poison differs, result must not */
        g2 = g; g2.s.gate = (uint8_t)(10 + lcg(&rs) % 91); g2.s.div = (uint8_t)(lcg(&rs) % 6); g2.s.swing = (uint8_t)(50 + lcg(&rs) % 26);
        g2.s.scale = (uint8_t)((g.s.scale + 1) & 15);                                              /* snap[] is the input, not scale */
        memset(&c2, 0x33, sizeof c2); A.build(c.m, c.seed, &g2, &c2);
        CHECK(memcmp(&c1, &c2, A.cache_bytes) == 0, "(b) cache depends on gate / div / swing / scale, case %d", n);
        memset(&hc, 0, sizeof hc);
        steps(&c1, ref, 0, 0);
        memset(c1.b + A.cache_bytes, 0xEE, SQ_CACHE_MAX - A.cache_bytes);                         /* (c) poison the unused tail */
        steps(&c1, o2, 0, 0);
        CHECK(memcmp(ref, o2, sizeof ref) == 0, "(c) step reads beyond cache_bytes, case %d", n);
        steps(&c1, o2, 0, 1); CHECK(memcmp(ref, o2, sizeof ref) == 0, "(d) step depends on call order (reverse), case %d", n);
        steps(&c1, o2, 0, 2); CHECK(memcmp(ref, o2, sizeof ref) == 0, "(d) step depends on call order (permuted), case %d", n);
        for (int j = 0; j < HARM_MAXV; j++) hc.vt[j] = (int8_t)(lcg(&rs) % 40);
        hc.n = 3; hc.pcs = 0x0891; hc.root = 5; hc.active = 1;
        steps(&c1, o2, &hc, 0);
        CHECK((A.flags & AF_CHORD) || memcmp(ref, o2, sizeof ref) == 0, "(f) step depends on hc without AF_CHORD, case %d", n);
        for (int t = 0; t < NT; t++) {
            CHECK(ref[t].vel >= 1 && ref[t].vel <= 127 && ref[t].gate >= 1 && ref[t].gate <= 100, "(e) vel %u gate %u at t=%d, case %d", ref[t].vel, ref[t].gate, t, n);
            CHECK(ref[t].pad == 0 && (!(ref[t].flags & PO_HOLD) || (ref[t].hold >= 1 && ref[t].hold <= 8)), "PulseOut pad / hold at t=%d", t);
            theory(&c, &ref[t]); }
        { uint8_t ll = A.loop_len(&c1); CHECK(ll >= 2 && ll <= 64, "loop_len %u", ll); }
        /* the mod hook is pure like step(): same lanes in any call order; the lanes stay in -127..127. s_mod is no input of build() or step(), so dicing it can never
         * move a note (T-MOD2) */
        {   int8_t l1[NT][4], l2[NT][4];
            for (uint32_t i = 0; i < NT; i++) lanes((uint16_t)(0x1111u * (n + 1)), &c1, i, hcfor(0, i), &ref[i], l1[i]);
            for (uint32_t i = 0; i < NT; i++) { uint32_t t = (i * 37u + 5u) % NT; lanes((uint16_t)(0x1111u * (n + 1)), &c1, t, hcfor(0, t), &ref[t], l2[t]); }
            CHECK(memcmp(l1, l2, sizeof l1) == 0, "(g) mod lanes depend on call order, case %d", n);
            for (uint32_t i = 0; i < NT; i++) for (int k = 0; k < 4; k++) CHECK(l1[i][k] >= -127, "(g) lane %d at %u = %d", k + 1, i, l1[i][k]);
            for (int sm = 0; sm < 8; sm++) {
                int8_t l3[4];
                lanes((uint16_t)lcg(&rs), &c1, (uint32_t)sm * 9u, hcfor(0, (uint32_t)sm * 9u), &ref[sm * 9], l3);
                for (int k = 0; k < 4; k++) CHECK(l3[k] >= -127, "(g) lane %d = %d", k + 1, l3[k]); } } } }

static int cl(const char *s) { return (int)strlen(s); }

static void tables(void)
{   int bound[SQ_NSEED + 1] = {0}, perm[SQ_NMORPH] = {0};
    CHECK(cl(A.name) >= 1 && cl(A.name) <= 6, "name %s", A.name);
    CHECK(A.id == SQ_ALGO_ACID, "id %u", A.id);
    CHECK(A.n_seed >= 1 && A.n_seed <= SQ_NSEED, "n_seed %u", A.n_seed);
    CHECK(A.cache_bytes <= SQ_CACHE_MAX && A.cache_bytes > 0, "cache_bytes %u", A.cache_bytes);
    for (int i = 0; i < SQ_NMORPH; i++) {
        const MorphDesc *d = &A.morph[i];
        CHECK(d->min <= d->def && d->def <= d->max, "morph %d %s min <= def <= max", i, d->label);
        CHECK(cl(d->label) >= 1 && cl(d->label) <= 6, "morph %d label", i);
        CHECK(d->step == 1 && (d->fast == 1 || (d->fast == 4 && d->max - d->min > 32 - 1)), "morph %d step %u fast %u", i, d->step, d->fast);
        CHECK(A.knob_morph[i] < SQ_NMORPH && !perm[A.knob_morph[i]]++, "knob_morph is not a permutation at %d", i); }
    for (int l = 0; l < SQ_NLAYER; l++) {
        const LayerDesc *d = &A.layer[l];
        CHECK(d->n <= 2, "layer %d n", l);
        CHECK(cl(A.sub[l]) >= 1 && cl(A.sub[l]) <= 6, "sub-label %d", l);
        CHECK(d->n || strcmp(A.sub[l], "-") == 0, "absent layer %d must read -", l);
        for (int j = 0; j < d->n; j++) { CHECK(d->seed[j] < A.n_seed || d->seed[j] == SQ_SEED_MOD, "layer %d seed id %u >= n_seed", l, d->seed[j]); if (d->seed[j] <= SQ_SEED_MOD) bound[d->seed[j]]++; } }
    for (int k = 0; k < A.n_seed; k++) CHECK(bound[k] == 1, "seed %d is bound by %d layers", k, bound[k]);
    CHECK(bound[SQ_SEED_MOD] == 1, "s_mod must be bound by exactly one layer, the SHAPE layer (MODULATION 3.4): %d", bound[SQ_SEED_MOD]);
    { int sh = 0; for (int j = 0; j < A.layer[4].n; j++) sh += A.layer[4].seed[j] == SQ_SEED_MOD; CHECK(sh == 1, "SHAPE layer does not hold s_mod"); }
    CHECK(A.mod != 0, "no mod hook");
    CHECK(((A.flags & AF_CHORD) != 0) == (A.id == SQ_ALGO_ARP || A.id == SQ_ALGO_PATTERNS), "AF_CHORD belongs to ARP and PATTERNS only");
    CHECK((A.sub_note != 0) == (A.id == SQ_ALGO_ARP), "sub_note belongs to ARP only");
    for (int ch = 0; ch < SQ_NCH; ch++) {
        Case c; defaults(&c, (uint8_t)ch);
        for (int k = 0; k < A.n_seed; k++) {
            uint16_t want = A.seed_k[k] == 0xFF ? 0 : (uint16_t)((0x3A5Du * (uint32_t)(ch * A.n_seed + A.seed_k[k])) & 0xFFFFu);
            CHECK(c.seed[k] == want, "default seed ch %d k %d", ch, k); } } }


/* the table: ALGOS[] has ACID as entry 0 and every row that exists carries its own id; rows of algorithms not landed yet are zero */
static void table_rows(void)
{   CHECK(ALGOS[0].build == acid_build && ALGOS[0].id == SQ_ALGO_ACID, "ALGOS[0] is not ACID");
    for (int i = 0; i < SQ_NALGO; i++) if (ALGOS[i].build) CHECK(ALGOS[i].id == i, "ALGOS[%d].id = %u", i, ALGOS[i].id); }

int main(void)
{   matrix();
    purity();
    tables();
    table_rows();
    if (fails) { fprintf(stderr, "ACID: %d check(s) failed\n", fails); return 1; }
    fprintf(stderr, "ACID: plain matrix, purity and table checks OK\n");
    return 0; }
