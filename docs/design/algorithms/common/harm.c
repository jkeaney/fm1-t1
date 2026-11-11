/* SPDX-License-Identifier: GPL-3.0-only */
/* harm.c: HARMONY (HARMONY.md): the chord builder, the voice leader, the drone, the chord stage and the progression dice.
 * P2-008 reference copy of tests/design/harmony_proto.py, compared with it table by table (H-GOLDEN). Integer-only, deterministic,
 * stdint only. All state is behind harm_bind() so a host test can drive it; the firmware reads harm_db[] and sq_gdb[]. */
#include "harm.h"
#include "sq_prims.h"

static const SeqGlobal   *hg;
static const HarmonySave *hs;
static uint32_t           hnow;                     /* HarmRT: ticks since PLAY */
static uint8_t            seen_slot[SQ_NCH];        /* last chord slot each channel's anchor was served for; 0xFF = anchor due */

static int16_t fdiv12(int16_t s) { return (int16_t)(s >= 0 ? s / 12 : -((-s + 11) / 12)); }   /* floor: ACID plays down to -12 */

void harm_bind(const SeqGlobal *g, const HarmonySave *h) { hg = g; hs = h; }
void harm_reset(void) { for (uint8_t i = 0; i < SQ_NCH; i++) seen_slot[i] = 0xFF; hnow = 0; }
void harm_set_time(uint32_t t) { hnow = t; }

static uint8_t h_bars(void)    { return (uint8_t)((hs->len & 7u) + 1u); }
static uint8_t h_nchords(void) { return (uint8_t)(((hs->len >> 3) & 7u) + 1u); }
static uint8_t h_voice(void)   { return (uint8_t)(hs->vm & 15u); }
static uint8_t h_move(void)    { return (uint8_t)(hs->vm >> 4); }
static uint8_t h_deg(uint8_t i)    /* degree 1..7 of slot i */
{   uint32_t v = (uint32_t)hs->deg[0] | ((uint32_t)hs->deg[1] << 8) | ((uint32_t)hs->deg[2] << 16);
    return (uint8_t)(((v >> (3u * i)) & 7u) + 1u); }
static uint8_t h_role(uint8_t ch) { return (uint8_t)((hs->roles >> (2u * ch)) & 3u); }

uint8_t harm_active(void)
{   return (uint8_t)!(h_nchords() == 1 && h_deg(0) == 1 && h_voice() == 2 && hs->drone == 0); }

/* ---- the scale as a sorted tone list (relative to the key) ---- */
static uint8_t scale_tones(uint8_t *sc)
{   uint16_t m = SQ_SCALE_MASKS[hg->s.scale & 15] & 0x0FFFu;
    uint8_t n = 0;
    if (!m) m = 0x0FFFu;
    for (uint8_t t = 0; t < 12; t++) if ((m >> t) & 1u) sc[n++] = t;
    return n; }

/* nearest scale tone to x (any octave), ties to the lower (SP6) */
static int16_t scale_snap(const uint8_t *sc, uint8_t n, int16_t x)
{   int16_t bd = 0x7FFF, bc = 0;
    for (int q = -2; q <= 8; q++)
        for (uint8_t i = 0; i < n; i++) {
            int16_t c = (int16_t)(sc[i] + 12 * q), d = (int16_t)(c > x ? c - x : x - c);
            if (d < bd || (d == bd && c < bc)) { bd = d; bc = c; } }
    return bc; }

static int16_t degree_root(const uint8_t *sc, uint8_t n, uint8_t d)   /* d = 1.. */
{   int i = d - 1;
    return (int16_t)(sc[i % n] + 12 * (i / n)); }

/* VOICE 0..6: (template index, octave shift) pairs, 3.1 */
static const uint8_t VOICE_N[7] = { 1, 2, 3, 4, 5, 5, 5 };
static const uint8_t VOICE_K[7][5] = { {0}, {0,2}, {0,1,2}, {0,1,2,3}, {0,1,2,3,4}, {0,2,3,4,1}, {0,3,1,2,4} };
static const uint8_t VOICE_O[7][5] = { {0}, {0,0}, {0,0,0}, {0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,1}, {0,0,1,1,1} };
static const uint8_t TPL[5] = { 0, 4, 7, 11, 14 };

static void sort_up(int8_t *a, uint8_t n)
{   for (uint8_t i = 1; i < n; i++) {
        int8_t v = a[i]; int j = i - 1;
        while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; j--; }
        a[j + 1] = v; } }

static uint8_t root_position(uint8_t d, int8_t *out)
{   uint8_t sc[12], ns = scale_tones(sc), n = 0;
    uint16_t pcs = 0;
    int16_t r = degree_root(sc, ns, d);
    uint8_t voice = h_voice();
    if (voice > 6) voice = 6;
    for (uint8_t i = 0; i < VOICE_N[voice]; i++) {
        int16_t v = (int16_t)(scale_snap(sc, ns, (int16_t)(r + TPL[VOICE_K[voice][i]])) + 12 * VOICE_O[voice][i]);
        if ((pcs >> (v % 12)) & 1u) continue;          /* a duplicate pitch class is dropped (keeps the lowest) */
        pcs = (uint16_t)(pcs | (1u << (v % 12)));
        out[n++] = (int8_t)v; }
    sort_up(out, n);
    return n; }

/* 3.2: place chord w (n notes) after prev (np notes) */
static void lead(const int8_t *prev, uint8_t np, int8_t *w, uint8_t n, uint8_t move)
{   int8_t base[HARM_MAXV], c[HARM_MAXV], best[HARM_MAXV];
    int32_t bk[4] = {0, 0, 0, 0}; uint8_t have = 0;
    if (!np || move >= 8) return;
    for (uint8_t i = 0; i < n; i++) base[i] = w[i];
    for (uint8_t inv = 0; inv < n; inv++) {
        if (inv) { base[0] = (int8_t)(base[0] + 12); sort_up(base, n); }   /* raise the lowest note by an octave, re-sort */
        for (int8_t q = 0; q >= -12; q = (int8_t)(q - 12)) {
            uint8_t ok = 1; int32_t dist = 0;
            for (uint8_t i = 0; i < n; i++) {
                c[i] = (int8_t)(base[i] + q);
                if (c[i] < 0 || c[i] > 48) ok = 0; }
            if (!ok) continue;
            for (uint8_t i = 0; i < n && i < np; i++) dist += c[i] > prev[i] ? c[i] - prev[i] : prev[i] - c[i];
            {   int32_t cost = dist * (8 - move) + (int32_t)move * 4 * inv, aq = q < 0 ? -q : q;
                if (!have || cost < bk[0] || (cost == bk[0] && (inv < bk[1] ||
                    (inv == (uint32_t)bk[1] && (aq < bk[2] || (aq == bk[2] && q < bk[3])))))) {
                    bk[0] = cost; bk[1] = inv; bk[2] = aq; bk[3] = q; have = 1;
                    for (uint8_t i = 0; i < n; i++) best[i] = c[i]; } } } }
    if (have) for (uint8_t i = 0; i < n; i++) w[i] = best[i]; }

uint8_t harm_voicing(uint8_t slot, int8_t vt[HARM_MAXV])
{   int8_t prev[HARM_MAXV], w[HARM_MAXV];
    uint8_t np = 0, n = 0;
    for (uint8_t k = 0; k <= slot; k++) {
        n = root_position(h_deg(k), w);
        lead(prev, np, w, n, h_move());
        for (uint8_t i = 0; i < n; i++) prev[i] = w[i];
        np = n; }
    for (uint8_t i = 0; i < n; i++) vt[i] = w[i];
    return n; }

/* 3.3: the drone candidate list; n' = min(7, scale size) degrees, then the same again an octave up */
int16_t harm_drone_pitch(uint8_t p)
{   uint8_t sc[12], ns = scale_tones(sc), np = ns < 7 ? ns : 7;
    if (p == 0) return -1;
    if (p > 2 * np) p = (uint8_t)(2 * np);
    return (int16_t)(degree_root(sc, ns, (uint8_t)(((p - 1) % np) + 1)) + (p > np ? 12 : 0)); }

char harm_drone_mark(const int8_t *vt, uint8_t n, int16_t drone)
{   int clash = 0;
    for (uint8_t i = 0; i < n; i++) {
        int d = ((drone - vt[i]) % 12 + 12) % 12;
        if (d == 0) return '+';
        if (d == 1 || d == 11) clash = 1; }
    return clash ? 'X' : '-'; }

static uint8_t slot_start(uint8_t i) { return (uint8_t)(((uint16_t)i * h_bars()) / h_nchords()); }

static uint8_t slot_of_bar(uint16_t bar)
{   uint8_t b = (uint8_t)(bar % h_bars()), s = 0;
    for (uint8_t i = 0; i < h_nchords(); i++) if (slot_start(i) <= b) s = i;
    return s; }

void harm_chord_at_bar(uint16_t bar, HarmChord *out)
{   uint8_t sc[12], ns = scale_tones(sc), slot = slot_of_bar(bar), n;
    int8_t vt[HARM_MAXV];
    uint16_t pcs = 0;
    n = harm_voicing(slot, vt);
    out->active = harm_active(); out->slot = slot; out->deg = h_deg(slot);
    out->root = (int8_t)degree_root(sc, ns, out->deg);
    out->drone = (int8_t)harm_drone_pitch(hs->drone);
    for (uint8_t i = 0; i < n; i++) pcs = (uint16_t)(pcs | (1u << (vt[i] % 12)));
    if (out->drone >= 0) {
        pcs = (uint16_t)(pcs | (1u << (out->drone % 12)));
        {   uint8_t has = 0;                               /* FULL gets the drone as one more voice (3.3), unless its pitch class is there */
            for (uint8_t i = 0; i < n; i++) if (vt[i] % 12 == out->drone % 12) has = 1;
            if (!has && n < HARM_MAXV) { vt[n++] = out->drone; sort_up(vt, n); } } }
    out->n = n; out->pcs = pcs;
    for (uint8_t i = 0; i < HARM_MAXV; i++) out->vt[i] = i < n ? vt[i] : 0; }

void harm_chord_now(HarmChord *out) { harm_chord_at_bar((uint16_t)(hnow / SQ_BAR_TICKS), out); }

uint16_t harm_ticks_to_change(uint32_t ticks)
{   uint8_t bars = h_bars(), b = (uint8_t)((ticks / SQ_BAR_TICKS) % bars), s = slot_of_bar(b), nb;
    nb = (uint8_t)(s + 1 < h_nchords() ? slot_start((uint8_t)(s + 1)) : bars);
    return (uint16_t)((uint32_t)(nb - b) * SQ_BAR_TICKS - ticks % SQ_BAR_TICKS); }

static int16_t nearest_in(const int16_t *cand, uint8_t n, int16_t s)
{   int16_t bd = 0x7FFF, bc = 0;
    for (uint8_t i = 0; i < n; i++) {
        int16_t d = (int16_t)(cand[i] > s ? cand[i] - s : s - cand[i]);
        if (d < bd || (d == bd && cand[i] < bc)) { bd = d; bc = cand[i]; } }
    return bc; }

/* nearest pitch of a pitch-class set to x, any octave, ties lower (harmony_proto.nearest_pc) */
static int16_t nearest_pc(uint16_t pcs, int16_t x)
{   int16_t cand[12 * 11]; uint8_t n = 0;
    for (int q = -2; q <= 8; q++)
        for (uint8_t p = 0; p < 12; p++) if ((pcs >> p) & 1u) cand[n++] = (int16_t)(p + 12 * q);
    return nearest_in(cand, n, x); }

int16_t harm_snap(const HarmChord *c, int16_t s, uint8_t mode)
{   int16_t x;
    if (mode == HQ_ROOT) x = (int16_t)(c->root % 12 + 12 * fdiv12(s));
    else if (mode == HQ_VOICED) {                          /* exact voiced notes, not their pitch classes */
        int16_t b = 0x7FFF; x = c->vt[0];
        for (uint8_t i = 0; i < c->n; i++) {
            int16_t d = (int16_t)(c->vt[i] > s ? c->vt[i] - s : s - c->vt[i]);
            if (d < b || (d == b && c->vt[i] < x)) { b = d; x = c->vt[i]; } } }
    else x = nearest_pc(c->pcs, s);
    return x < 0 ? 0 : x > 96 ? 96 : x; }

int16_t harm_tone(const HarmChord *c, uint8_t idx)
{   return (int16_t)(c->vt[idx % c->n] + 12 * (idx / c->n)); }

/* 4.2 */
static uint8_t hsh(uint8_t t) { return (uint8_t)(((uint32_t)t * 0x9E3779B1u) >> 29); }

int16_t harm_pull_note(const HarmChord *c, int16_t s, uint8_t t, uint8_t strong, uint8_t pull, uint8_t first, int16_t lo, int16_t hi)
{   int16_t x;
    if (first) x = (int16_t)(c->root % 12 + 12 * fdiv12(s));
    else {
        uint8_t peff = (uint8_t)(pull + 4 * strong); if (peff > 8) peff = 8;
        x = hsh(t) < peff ? nearest_pc(c->pcs, s) : s; }
    while (x > hi) x = (int16_t)(x - 12);
    while (x < lo) x = (int16_t)(x + 12);
    return x; }

int16_t harm_apply(uint8_t ch, uint8_t t, uint8_t strong, int16_t s, int16_t lo, int16_t hi, uint8_t pull, uint32_t tick)
{   HarmChord c; uint8_t role = h_role(ch), first = 0; int16_t x;
    if (role == HR_OFF || (role == HR_AUTO && !harm_active())) return s;
    harm_chord_at_bar((uint16_t)(tick / SQ_BAR_TICKS), &c);
    if (role == HR_ROOT) x = (int16_t)((c.drone >= 0 ? c.drone : c.root) % 12 + 12 * fdiv12(s));
    else if (role == HR_FULL) {
        int32_t i = ((int32_t)(s - lo) * c.n) / (hi - lo + 1);          /* lo = 0 gives the spec's (s * n) / (hi + 1) */
        x = c.vt[i < 0 ? 0 : i > c.n - 1 ? c.n - 1 : i]; }
    else {
        if (seen_slot[ch] != c.slot) { first = 1; seen_slot[ch] = c.slot; }
        return harm_pull_note(&c, s, t, strong, pull, first, lo, hi); }
    while (x > hi) x = (int16_t)(x - 12);
    while (x < lo) x = (int16_t)(x + 12);
    return x; }

/* 4.3: pull = base + morph[2] * mul / div; every v1 algorithm keeps its pitch-width morph in morph id 2 */
static const uint8_t PULL_TAB[4][3] = { {3, 3, 20}, {2, 6, 24}, {2, 6, 48}, {2, 6, 36} };   /* ACID JUMP, EUCLID SPAN, SNAKE RANGE, STAGE RANGE */
uint8_t harm_pull(uint8_t algo, const uint8_t m[SQ_NMORPH])
{   uint16_t p;
    if (algo >= 4) return 0;                                /* ARP / PATTERNS: AF_CHORD, no pull (AM-3) */
    p = (uint16_t)(PULL_TAB[algo][0] + (m[2] * PULL_TAB[algo][1]) / PULL_TAB[algo][2]);
    return (uint8_t)(p > 8 ? 8 : p); }

/* The lowest semitone each algorithm can emit (the fold floor of 4.2 rule 5). ADR-0003 section 20 only gives hi(m); ACID's octave -1 needs this
 * (docs/design/algorithms/acid/SPEC.md 7, flagged): ACID -12, the others 0. */
int16_t harm_lo(uint8_t algo) { return (int16_t)(algo == SQ_ALGO_ACID ? -12 : 0); }

/* 6.1 */
void harm_dice(uint16_t seed16, uint8_t nscale, uint8_t *bars, uint8_t degs[8], uint8_t *nout)
{   static const uint8_t LT[8] = { 2, 4, 4, 4, 4, 4, 8, 8 }, WT[7] = { 2, 2, 2, 4, 4, 3, 1 };
    SqRng r; uint8_t dmax = nscale < 7 ? nscale : 7, nb, n, tot = 0, a;
    r.x = (((uint32_t)seed16 << 16) | seed16) ^ 0x4841524Du; if (!r.x) r.x = 1;
    nb = LT[sq_rnd(&r, 8)];
    n = (uint8_t)(2 + sq_rnd(&r, (uint32_t)((nb < 4 ? nb : 4) - 1)));
    for (uint8_t i = 0; i < dmax; i++) tot = (uint8_t)(tot + WT[i]);
    for (uint8_t i = 0; i < n; i++) {
        uint8_t d = 0;
        for (uint8_t tr = 0; tr < 5; tr++) {
            int32_t x = (int32_t)sq_rnd(&r, tot);
            d = 0;
            while (x >= WT[d]) { x -= WT[d]; d++; }
            d++;
            if (!i || d != degs[i - 1]) break;
            if (tr == 4) d = (uint8_t)(degs[i - 1] % dmax + 1); }
        degs[i] = d; }
    a = (uint8_t)sq_rnd(&r, 4);
    if (a < 3) { degs[0] = 1; if (n > 1 && degs[1] == 1) degs[1] = dmax < 5 ? dmax : 5; }
    else { degs[n - 1] = 1; if (degs[n - 2] == 1) degs[n - 2] = dmax < 5 ? dmax : 5; }
    *bars = nb; *nout = n; }
