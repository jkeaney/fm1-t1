/* SPDX-License-Identifier: GPL-3.0-only */
/* PATTERNS: the preset pattern library, quantised to the global scale and, while HARMONY is active, pulled to the chord. P2-017
 * prototype = the firmware file: P4 copies it, adds `static` and wires the ALGOS[5] row; P5-008 generates patterns_data.h from
 * patterns.tsv at build time. Spec: ./SPEC.md (sections cited as S<n>); the arithmetic is tests/design/patterns_proto.py (render()).
 * Pure: build() and step() use only their arguments and const tables (ADR-0003 section 4). Stateless (S4.7): the cache holds the
 * morphs, the five seeds, the scale mask and the base note; every step re-derives which steps sound from them (at most 32 steps,
 * about 12 000 integer operations, once per step). stdint only. */
#include "../common/sq_types.h"
#include "../common/sq_prims.h"
#include "patterns_data.h"

typedef struct {
    uint16_t seed[5];      /* s_notes s_hits s_acc s_art s_shape */
    uint16_t mask;         /* the scale as a 12-bit mask, derived from g->snap[] (snap[r] == r exactly for the scale tones) */
    uint8_t  m[SQ_NMORPH];
    uint8_t  base;         /* 36 + key + 12 * octave */
    uint8_t  pad;
} PattCache;               /* 22 B */
_Static_assert(sizeof(PattCache) == 22 && sizeof(PattCache) <= SQ_CACHE_MAX, "PattCache");

#define M_DENS 0
#define M_VARY 1
#define M_SPAN 2
#define M_ACCNT 3
#define M_PICK 4
#define M_SLIDE 5
#define M_DYN 6
#define M_CTONE 7

#define MAXLEN 32

/* one decoded step */
typedef struct { uint8_t present, opt, tie, acc, sld, ct, rank, gate, p; } PatStep;

static void pat_decode(uint16_t w, PatStep *s)
{
    s->present = (uint8_t)(w != 0);
    s->opt = (uint8_t)((w >> 15) & 1u); s->tie = (uint8_t)((w >> 6) & 1u); s->p = (uint8_t)(w & 63u);
    s->acc = (uint8_t)((w >> 8) & 1u); s->sld = (uint8_t)((w >> 9) & 1u); s->ct = (uint8_t)((w >> 10) & 1u);
    s->rank = (uint8_t)((w >> 11) & 3u); s->gate = (uint8_t)((w >> 13) & 3u);
}

/* h16: the only random source (patterns_proto.h16), 16-bit wrapping */
static uint16_t pat_h16(uint16_t seed, uint8_t i, uint8_t salt)
{   uint16_t x = (uint16_t)(seed * 0x9E37u + i * 0x85EBu + salt * 0xC2B3u + 0x1234u);
    x ^= (uint16_t)(x >> 7); x = (uint16_t)(x * 0x2C1Bu);
    x ^= (uint16_t)(x >> 9); x = (uint16_t)(x * 0x6B43u);
    x ^= (uint16_t)(x >> 8);
    return x; }

static int16_t pat_fdiv(int32_t a, int32_t b) { int32_t q = a / b; if ((a % b != 0) && ((a < 0) != (b < 0))) q--; return (int16_t)q; }   /* Python's // */
static int16_t pat_mod12(int16_t x) { return (int16_t)(((x % 12) + 12) % 12); }

/* SP6: the nearest scale tone to x, any octave, ties to the lower */
static int16_t pat_snap(uint16_t mask, int16_t x)
{   int16_t bd = 0x7FFF, bc = 0;
    for (int q = -2; q <= 8; q++)
        for (uint8_t k = 0; k < 12; k++) {
            int16_t c, d;
            if (!((mask >> k) & 1u)) continue;
            c = (int16_t)(k + 12 * q); d = (int16_t)(c > x ? c - x : x - c);
            if (d < bd || (d == bd && c < bc)) { bd = d; bc = c; } }
    return bc; }

/* the next scale tone strictly above (up) or below s */
static int16_t pat_neighbour(uint16_t mask, int16_t s, uint8_t up)
{   for (int d = 1; d <= 12; d++) {
        int16_t x = (int16_t)(up ? s + d : s - d);
        if ((mask >> pat_mod12(x)) & 1u) return x; }
    return s; }

static int16_t pat_deg(uint16_t mask, uint8_t d)
{   uint8_t t[12], n = 0;
    for (uint8_t k = 0; k < 12; k++) if ((mask >> k) & 1u) t[n++] = k;
    return (int16_t)(t[d % n] + 12 * (d / n)); }

/* HARMONY 9 harm_snap(HQ_PC) as the prototype uses it: the nearest chord pitch class, candidates kept at or above the base octave, ties lower, 0..96 */
static int16_t pat_chord_pull(uint16_t pcs, int16_t s)
{   int16_t bd = 0x7FFF, bc = 0;
    for (int q = 0; q <= 8; q++)
        for (uint8_t k = 0; k < 12; k++) {
            int16_t c, d;
            if (!((pcs >> k) & 1u)) continue;
            c = (int16_t)(k + 12 * q); d = (int16_t)(c > s ? c - s : s - c);
            if (d < bd || (d == bd && c < bc)) { bd = d; bc = c; } }
    return bc < 0 ? 0 : bc > 96 ? 96 : bc; }

/* sort idx[0..n-1] by the key array (ascending, ties by the index value, which is already unique) */
static void pat_sort(uint8_t *idx, uint8_t n, const uint32_t *key)
{   for (uint8_t i = 1; i < n; i++) {
        uint8_t v = idx[i], j = i;
        while (j > 0 && (key[idx[j - 1]] > key[v] || (key[idx[j - 1]] == key[v] && idx[j - 1] > v))) { idx[j] = idx[j - 1]; j--; }
        idx[j] = v; } }

static void pat_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache)
{
    PattCache *c = (PattCache *)cache;
    uint8_t *z = (uint8_t *)cache, i;
    for (i = 0; i < sizeof *c; i++) z[i] = 0;                                 /* every byte of the cache is defined */
    for (i = 0; i < 5; i++) c->seed[i] = seed[i];
    for (i = 0; i < 12; i++) if (g->snap[i] == i) c->mask = (uint16_t)(c->mask | (1u << i));
    for (i = 0; i < SQ_NMORPH; i++) c->m[i] = m[i];
    if (c->m[M_PICK] < 1) c->m[M_PICK] = 1;
    if (c->m[M_PICK] > PAT_N) c->m[M_PICK] = PAT_N;
    c->base = (uint8_t)(36 + g->s.key + 12 * g->s.octave);
}

static uint8_t pat_loop_len(const void *cache) { return PAT_HEAD[((const PattCache *)cache)->m[M_PICK] - 1].len; }

/* S4.1: render one step. Same procedure as patterns_proto.render(), evaluated for the whole pattern and read at step i. */
static void pat_step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)
{
    const PattCache *c = (const PattCache *)cache;
    const PatHead *hd = &PAT_HEAD[c->m[M_PICK] - 1];
    uint8_t n = hd->len, i = (uint8_t)(g % n), k, ndeco = 0, nopt = 0, nflag = 0, noth = 0, nA = 0, nN = 0, nS = 0, ncand = 0;
    uint8_t on[MAXLEN], acc[MAXLEN], sld[MAXLEN], pulled[MAXLEN];
    uint8_t deco[MAXLEN], opts[MAXLEN], flag[MAXLEN], oth[MAXLEN], lA[MAXLEN], lN[MAXLEN], lS[MAXLEN], lC[MAXLEN];
    uint32_t key[MAXLEN];
    PatStep S[MAXLEN];
    int16_t base[MAXLEN], pmin = 0x7FFF, s;
    uint8_t dens = c->m[M_DENS], vary = c->m[M_VARY], span = c->m[M_SPAN], accnt = c->m[M_ACCNT], slide = c->m[M_SLIDE], dyn = c->m[M_DYN], ctone = c->m[M_CTONE];
    uint16_t s_notes = c->seed[0], s_hits = c->seed[1], s_acc = c->seed[2], s_art = c->seed[3], s_shape = c->seed[4];
    int16_t v;
    uint8_t prev;

    for (k = 0; k < n; k++) {
        pat_decode(PAT_STEP[hd->first + k], &S[k]);
        on[k] = (uint8_t)(S[k].present && !S[k].opt); acc[k] = 0; sld[k] = 0; pulled[k] = 0; base[k] = 0;
        if (S[k].present) {
            base[k] = (hd->flags & 1u) ? pat_deg(c->mask, S[k].p) : pat_snap(c->mask, S[k].p);
            if (base[k] < pmin) pmin = base[k]; } }

    /* 1. which steps sound: decoration thins as DENS falls, optional adds join as it rises, anchors stay */
    if (dens == 0) { for (k = 0; k < n; k++) on[k] = 0; }
    else {
        for (k = 0; k < n; k++) {
            key[k] = ((uint32_t)S[k].rank << 24) | ((uint32_t)(pat_h16(s_hits, k, 1) >> 8) << 8);
            if (on[k] && S[k].rank > 0) deco[ndeco++] = k;
            if (S[k].present && S[k].opt) opts[nopt++] = k; }
        pat_sort(deco, ndeco, key); pat_sort(opts, nopt, key);
        if (dens < 8) { uint8_t keep = (uint8_t)((ndeco * (dens - 1u)) / 7u); for (k = keep; k < ndeco; k++) on[deco[k]] = 0; }
        else if (dens > 8) { uint8_t add = (uint8_t)((nopt * (dens - 8u)) / 8u); for (k = 0; k < add; k++) on[opts[k]] = 1; } }

    /* 2. pitch: SPAN fold, VARY neighbour, SP6 snap (computed for every defined step: a rest or a dropped step still reports a valid note) */
    {   int16_t pitch[MAXLEN];
        for (k = 0; k < n; k++) {
            if (!S[k].present) { pitch[k] = 0; continue; }
            s = base[k];
            while (s - pmin >= 12 * span) s = (int16_t)(s - 12);
            if (S[k].rank > 0 && (pat_h16(s_notes, k, 2) % 8u) < vary) s = pat_neighbour(c->mask, pat_snap(c->mask, s), (uint8_t)(pat_h16(s_shape, k, 3) & 1u));
            s = pat_snap(c->mask, s);
            if (s < 0) s = (int16_t)(s + 12);
            pitch[k] = s; }
        /* 3. chord-tone bias, only while HARMONY is active */
        if (hc && hc->active && hc->pcs) {
            nflag = 0; noth = 0;
            for (k = 0; k < n; k++) if (on[k]) {
                if (S[k].ct) flag[nflag++] = k;
                else { oth[noth++] = k; key[k] = ((uint32_t)S[k].rank << 24) | ((uint32_t)pat_h16(s_notes, k, 5) << 8); } }
            pat_sort(oth, noth, key);
            if (ctone == 1) { for (k = 0; k < (nflag + 1u) / 2u; k++) pulled[flag[k]] = 1; }
            else if (ctone >= 2) {
                for (k = 0; k < nflag; k++) pulled[flag[k]] = 1;
                if (ctone > 2) { uint8_t cnt = (uint8_t)((noth * (ctone - 2u) + 5u) / 6u); for (k = 0; k < cnt; k++) pulled[oth[k]] = 1; } }
            for (k = 0; k < n; k++) if (pulled[k]) pitch[k] = pat_chord_pull(hc->pcs, pitch[k]); }
        s = pitch[i]; }

    /* 4. accents, slides, velocity */
    for (k = 0; k < n; k++) if (on[k]) { acc[k] = S[k].acc; sld[k] = S[k].sld; }
    for (k = 0; k < n; k++) {
        key[k] = ((uint32_t)pat_h16(s_acc, k, 4) << 8);
        if (on[k]) { if (acc[k]) lA[nA++] = k; else lN[nN++] = k; } }
    pat_sort(lA, nA, key); pat_sort(lN, nN, key);
    if (accnt < 4) { uint8_t cnt = (uint8_t)((nA * (4u - accnt)) / 4u); for (k = 0; k < cnt; k++) acc[lA[k]] = 0; }
    else if (accnt > 4) { uint8_t cnt = (uint8_t)((nN * (accnt - 4u)) / 8u); for (k = 0; k < cnt; k++) acc[lN[k]] = 1; }
    for (k = 0; k < n; k++) {
        key[k] = ((uint32_t)pat_h16(s_art, k, 6) << 8);
        if (on[k]) { if (sld[k]) lS[nS++] = k; else if (on[(k + 1) % n]) lC[ncand++] = k; } }
    pat_sort(lS, nS, key); pat_sort(lC, ncand, key);
    if (slide < 4) { uint8_t cnt = (uint8_t)((nS * (4u - slide)) / 4u); for (k = 0; k < cnt; k++) sld[lS[k]] = 0; }
    else if (slide > 4) { uint8_t cnt = (uint8_t)((ncand * (slide - 4u)) / 4u); for (k = 0; k < cnt; k++) sld[lC[k]] = 1; }

    v = acc[i] ? 120 : 80;
    v = (int16_t)(100 + pat_fdiv((int32_t)(v - 100) * (dyn < 10 ? dyn : 10), 10));
    if (dyn > 10) v = (int16_t)(v + pat_fdiv((int32_t)(((int32_t)(pat_h16(s_acc, i, 7) % 7u)) - 3) * (dyn - 10), 2));
    prev = (uint8_t)((i + n - 1u) % n);

    {   int16_t nn = (int16_t)(c->base + s);
        out->note = (uint8_t)(nn < 0 ? 0 : nn > 127 ? 127 : nn); }
    out->vel = (uint8_t)(v < 1 ? 1 : v > 127 ? 127 : v);
    out->gate = S[i].tie ? 100 : S[i].gate == 0 ? 25 : S[i].gate == 2 ? 95 : 50;       /* short / normal / long; a tie fills the step */
    out->flags = (uint8_t)((on[i] ? PO_HIT : 0) | (on[i] && on[prev] && sld[prev] ? PO_SLIDE_IN : 0) | (on[i] && S[i].tie ? PO_TIE : 0));
    out->hold = 0;
    out->pad = 0;
}

/* The mod lanes (MODULATION 4): lane A is the cell of the library pattern's step number (the authored accents give the lean); lane B sweeps once per pattern length. */
static void pat_mod(const void *cache, uint32_t g, const HarmChord *hc, const PulseOut *o, ModIdx *out)
{
    const PattCache *c = (const PattCache *)cache;
    uint8_t n = PAT_HEAD[c->m[M_PICK] - 1].len, i = (uint8_t)(g % n);
    (void)hc;
    out->a = i; out->vel = o->vel; out->b_cell = 0; out->b_lfo = 1; out->t = i; out->period = n;
}

/* Highest semitone: a library pattern is under 4 octaves (48) plus a VARY neighbour and a snap, and a chord pull stays within 6 of that */
static uint8_t pat_hi(const uint8_t m[SQ_NMORPH]) { (void)m; return 60; }

/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_PATTERNS = {
    .name = "PATT", .id = SQ_ALGO_PATTERNS, .n_seed = 5, .flags = AF_CHORD, .cache_bytes = 22,
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 },
    .seed_k = { 1, 2, 3, 4, 5 },
    .morph = {
        { "DENS",   0,  16,  8, 1, 1 },
        { "VARY",   0,   8,  0, 1, 1 },
        { "SPAN",   1,   4,  4, 1, 1 },
        { "ACCNT",  0,   8,  4, 1, 1 },
        { "PICK",   1, 120,  1, 1, 1 },
        { "SLIDE",  0,   8,  4, 1, 1 },
        { "DYN",    0,  14, 10, 1, 1 },
        { "CTONE",  0,   8,  2, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {2}}, {1, {3}}, {2, {4, SQ_SEED_MOD}} },
    .sub = { "VARY", "ORDER", "ACCNT", "SLIDES", "UP/DN" },
    .build = pat_build, .loop_len = pat_loop_len, .step = pat_step, .hi = pat_hi, .sub_note = 0, .mod = pat_mod,
};
/* ALGODESC-END */
