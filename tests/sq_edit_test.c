/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_edit_test.c (P4-006): the edit layer, pages and step resolution of firmware/src/sq_core.c (sq_eval, sq_slide_out, sq_tie_end) and
 * firmware/src/sq_chan.c (sq_step_set / tap / clear, sq_page_*, sq_len_set, sq_page_add, FOLLOW) against mock hooks and a test ALGOS
 * table that carries the v1 layer shapes (NOTES, HITS thinned by DENS, ACCNT, ARTIC, SHAPE -> s_mod).
 *   cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -fno-strict-aliasing -fsanitize=address,undefined -Ifirmware/src \
 *      -o /tmp/sq_edit_test tests/sq_edit_test.c && /tmp/sq_edit_test
 * Run from the repo root. GOLDEN_UPDATE=1 rewrites tests/golden/sq_edit.txt (the golden text IS the specification: review the diff).
 *
 * Covered: every row of SEQUENCER_UX 1.2 (dice, morph, algorithm, SCALE, LENGTH, preset load; the clear gestures), the worked examples
 * of 1.3 (pages: copy, partial page, hidden page, maximum), 1.4 (page sources and the generator position), 1.6 (extend duplicates,
 * shrink hides, extend restores, discard by commit, undo of a page add and a page clear, shrink past the playhead), the forced OFF / ON
 * rows of the acceptance criteria, and the scheduler's slide and TIE timing (the event log is in the golden). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"

/* ---- host side of the device hooks (sq_core.c and sq_chan.c are the only callers) ---- */
typedef struct { int unused; } track_t;
static track_t trk[SQ_NCH];
#define MO_RESERVE 8u
static uint32_t midi_out_free(void) { return 1000u; }
static uint32_t beat_samples(void) { return 22050u; }
#define SQ_EXT_BEAT() 22050u                 /* 120 BPM at 44100: 192 PPQN ticks of 114.8 samples */
#define SQ_BPM()      120u
static int irq_depth, irq_bad;               /* SQ_IRQ_OFF / ON must nest */
#define SQ_IRQ_OFF() (irq_depth++)
#define SQ_IRQ_ON()  (irq_depth-- > 0 ? 0 : irq_bad++)
static void midi_out_event_at(uint32_t pkt, uint32_t due);      /* defined after sq_core.c: it reads the core's tick */
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel);
static void trk_note_off(track_t *t, uint32_t note);

/* ---- the test ALGOS table: every layer is one seed, so a dice re-rolls exactly one layer ---- */
typedef struct { uint8_t m[SQ_NMORPH]; uint16_t seed[SQ_NSEED + 1]; } Cache;     /* what build() was handed; seed[6] = s_mod */
_Static_assert(sizeof(Cache) <= SQ_CACHE_MAX, "Cache fits");
static void t_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k)
{   Cache *c = (Cache *)k;
    (void)g;
    memcpy(c->m, m, SQ_NMORPH);
    memcpy(c->seed, seed, sizeof c->seed); }
static uint8_t t_len(const void *k) { (void)k; return 64; }
static void t_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o)
{   const Cache *c = (const Cache *)k;
    const uint32_t x = g * 0x9E3779B1u + 0x51u;
    (void)hc;
    memset(o, 0, sizeof *o);
    o->note = (uint8_t)(36u + sq_mix((uint32_t)c->seed[0] ^ x) % 12u);                              /* NOTES */
    if (sq_mix((uint32_t)c->seed[1] ^ x * 3u) % 16u < (uint32_t)c->m[0]) o->flags |= PO_HIT;        /* HITS, thinned by DENS (m[0]) */
    o->vel = (uint8_t)(50u + sq_mix((uint32_t)c->seed[2] ^ x * 5u) % 78u);                          /* ACCNT */
    o->gate = (uint8_t)(10u + sq_mix((uint32_t)c->seed[3] ^ x * 7u) % 91u);                         /* ARTIC */
    if (sq_mix((uint32_t)c->seed[SQ_SEED_MOD] ^ x * 11u) % 4u == 0u) o->flags |= PO_SLIDE_IN;       /* SHAPE: a generated glide */
    if (sq_mix((uint32_t)c->seed[SQ_SEED_MOD] ^ x * 13u) % 3u == 0u) o->flags |= (uint8_t)(1u << PO_REP_SHIFT);   /* and a ratchet */
}

#define TMORPH { { "DENS", 0, 16, 12, 1, 1 }, { "PEDAL", 0, 12, 6, 1, 1 }, { "JUMP", 0, 20, 5, 1, 1 }, { "ACCNT", 0, 8, 3, 1, 1 }, \
                 { "OFFSET", 0, 15, 0, 1, 1 }, { "SLIDE", 0, 8, 4, 1, 1 }, { "PUNCH", 0, 47, 40, 1, 4 }, { "DIR", 0, 2, 0, 1, 1 } }
#define TLAYER { { 1, { 0, 0 } }, { 1, { 1, 0 } }, { 1, { 2, 0 } }, { 1, { 3, 0 } }, { 1, { SQ_SEED_MOD, 0 } } }
#define TROW(i, nm, fl) [i] = { .name = nm, .id = i, .n_seed = SQ_NSEED, .flags = fl, .cache_bytes = sizeof(Cache), \
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 }, .seed_k = { 1, 2, 3, 4, 5, 6 }, .morph = TMORPH, .layer = TLAYER, \
    .sub = { "NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step }
static const AlgoDesc ALGOS[SQ_NALGO] = {
    TROW(0, "PAIR", AF_PAIRING), TROW(1, "PLAIN", 0), TROW(2, "PAIR", AF_PAIRING),
    TROW(3, "PLAIN", 0), TROW(4, "PAIR", AF_PAIRING), TROW(5, "PLAIN", 0),
};

#include "sq_core.c"
#include "sq_chan.c"

/* ---- the event recorder: every MIDI note the core sends, stamped with the tick it left on ---- */
typedef struct { uint32_t tick; uint8_t on, ch, note; } Ev;
static Ev   evs[4096];
static int  nev;
static int  rec_on;
static int  midi_cnt[16][128];
static long viol;                            /* MIDI or preview invariant violations (double ON, OFF without ON, stray packets) */
static void ev_add(uint8_t on, uint8_t ch, uint8_t note)
{   if (rec_on && nev < (int)(sizeof evs / sizeof evs[0])) { evs[nev].tick = sq_cur; evs[nev].on = on; evs[nev].ch = ch; evs[nev].note = note; nev++; } }
static void midi_out_event_at(uint32_t pkt, uint32_t due)
{   unsigned st = (pkt >> 8) & 0xF0u, ch = (pkt >> 8) & 0xFu, d1 = (pkt >> 16) & 0xFFu, d2 = pkt >> 24;
    (void)due;
    if (st == 0xF0u) return;                                     /* clock out: not about notes */
    if (st == 0x90u && d2) {
        if (midi_cnt[ch][d1]) viol++;
        midi_cnt[ch][d1] = 1; ev_add(1, (uint8_t)ch, (uint8_t)d1);
    } else if (st == 0x80u || st == 0x90u) {
        if (!midi_cnt[ch][d1]) viol++;
        midi_cnt[ch][d1] = 0; ev_add(0, (uint8_t)ch, (uint8_t)d1);
    } else if (!(st == 0xB0u && d1 == 123u)) viol++; }
static int prv_cnt[SQ_NCH][128];
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel) { (void)vel; unsigned c = (unsigned)(t - trk); if (prv_cnt[c][note]) viol++; prv_cnt[c][note] = 1; }
static void trk_note_off(track_t *t, uint32_t note) { unsigned c = (unsigned)(t - trk); if (!prv_cnt[c][note]) viol++; prv_cnt[c][note] = 0; }

/* ---- helpers ---- */
static int fails;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

static char   gb[1 << 17];                   /* the golden text: resolved tables and event logs of this run */
static size_t gn;
#define GP(...) do { gn += (size_t)snprintf(gb + gn, sizeof gb - gn, __VA_ARGS__); } while (0)

static void eval_at(uint8_t c, uint8_t p, uint16_t pass, PulseOut *o)      /* the resolved step with the live (active) generator */
{   const uint8_t ci = sq_cidx[c];
    const AlgoDesc *a = &ALGOS[sq_calgo[c][ci]];
    sq_eval(c, p, pass, a, sq_cache[c][ci], &sq_gdb[sq_gidx], o); }

static void gen_at(uint8_t c, uint32_t g, PulseOut *o)                    /* the raw generated step at generator position g */
{   const uint8_t ci = sq_cidx[c];
    const AlgoDesc *a = &ALGOS[sq_calgo[c][ci]];
    memset(o, 0, sizeof *o);
    a->step(sq_cache[c][ci], g, NULL, o); }

static int same_out(const PulseOut *a, const PulseOut *b)
{   return a->note == b->note && a->vel == b->vel && a->gate == b->gate &&
           (a->flags & (PO_HIT | PO_TIE | PO_REP_MASK)) == (b->flags & (PO_HIT | PO_TIE | PO_REP_MASK)); }

static void dump(const char *title, uint8_t c, uint8_t from, uint8_t to)  /* the resolved steps, one line each, into the golden */
{   GP("%s\n", title);
    for (uint8_t p = from; p < to; p++) {
        PulseOut o;
        eval_at(c, p, 0, &o);
        GP(" p%02u g%02u hit%u n%03u v%03u gt%03u r%u t%u\n", (unsigned)p, (unsigned)sq_gen_pos(&sq_ch[c], p),
           (unsigned)((o.flags & PO_HIT) ? 1 : 0), (unsigned)o.note, (unsigned)o.vel, (unsigned)o.gate,
           (unsigned)((o.flags & PO_REP_MASK) >> PO_REP_SHIFT), (unsigned)((o.flags & PO_TIE) ? 1 : 0));
    } }

static void grab_ed(uint8_t c, StepEdit *out) { memcpy(out, sq_ch[c].ed, sizeof sq_ch[c].ed); }

static void reset_ch(uint8_t c, uint8_t len)                              /* a clean channel: AUTO everywhere, page 1 only */
{   memset(sq_ch[c].ed, 0, sizeof sq_ch[c].ed);
    sq_ch[c].len = len; sq_ch[c].pg_src = 0; sq_ch[c].pg_used = 1;
    memset(&sq_undo_rec[c], 0, sizeof sq_undo_rec[c]); }

static void fresh(void)                                                   /* stop (every sounding note off), sq_boot; channel 1 audible, page 1 of 16 */
{   sq_on_stop();                                                         /* sq_boot would drop sounding notes without their OFFs */
    rec_on = 0; nev = 0;
    sq_boot();
    for (uint8_t c = 1; c < SQ_NCH; c++) { sq_ch[c].midi_ch = SQ_MIDI_OFF; sq_ch[c].muted = 1; }
    sq_ch[0].midi_ch = 0; sq_ch[0].muted = 0;
    reset_ch(0, 16); }

static int hits_of(uint8_t c, uint8_t from, uint8_t to, int skip)         /* sounding steps in [from, to), not counting `skip` */
{   int n = 0;
    for (uint8_t p = from; p < to; p++) {
        PulseOut o;
        if (p == skip) continue;
        eval_at(c, p, 0, &o);
        n += (o.flags & PO_HIT) ? 1 : 0; }
    return n; }

/* ---- 1.2: what a dice, a morph, an algorithm change and a global change do to the edits ---- */
static void section_12(void)
{   StepEdit before[SQ_NSTEP], after[SQ_NSTEP];
    SeqGlobal *gg;
    PulseOut o;
    int changed = 0, bad_off = 0, bad_on = 0, gen_on_3 = 0;
    GP("== 1.2 rows\n");

    /* Dice NOTES: a set PITCH stays; AUTO notes follow. */
    fresh();
    CHECK(sq_step_set(0, 5, SQ_F_PITCH, 7), "set step 6 PITCH");
    gg = &sq_gdb[sq_gidx];
    {   PulseOut x[16];
        for (uint8_t p = 0; p < 16; p++) eval_at(0, p, 0, &x[p]);
        sq_dice_layer(0, 0);
        eval_at(0, 5, 0, &o);
        CHECK(o.note == sq_note(gg, 7), "dice NOTES: the set PITCH of step 6 is still G (note %u)", o.note);
        for (uint8_t p = 6; p < 16; p++) { eval_at(0, p, 0, &o); if (o.note != x[p].note) changed++; }
        CHECK(changed > 0, "dice NOTES: AUTO steps follow the new notes (%d changed)", changed); }
    dump("dice NOTES: step 6 PITCH 7 set, the rest AUTO (steps 1-8)", 0, 0, 8);

    /* Dice HITS: ON / OFF set stays. Step 4 forced OFF, step 5 forced ON; forty dice. */
    fresh();
    CHECK(sq_step_set(0, 3, SQ_F_ON, 0) && sq_step_set(0, 4, SQ_F_ON, 1), "force step 4 OFF, step 5 ON");
    for (int i = 0; i < 40; i++) {
        sq_dice_layer(0, 1);
        eval_at(0, 3, 0, &o); if (o.flags & PO_HIT) bad_off++;
        eval_at(0, 4, 0, &o); if (!(o.flags & PO_HIT)) bad_on++;
        gen_on_3 += sq_gen_hit(0, 3); }
    CHECK(bad_off == 0, "dice HITS: a forced OFF step never sounds (%d sounded)", bad_off);
    CHECK(bad_on == 0, "dice HITS: a forced ON step always sounds (%d silent)", bad_on);
    CHECK(gen_on_3 > 0, "dice HITS: the generator would sound step 4 on some dice, so the OFF edit is what silences it");
    GP("dice HITS x40: forced OFF silent %d, forced ON silent %d, generator would sound step 4 on %d dice\n", bad_off, bad_on, gen_on_3);

    /* Dice ACCNT / ARTIC / SHAPE / ALL: set SLIDE and GATE stay; generated velocities move. */
    fresh();
    CHECK(sq_step_set(0, 7, SQ_F_SLIDE, 1) && sq_step_set(0, 7, SQ_F_GATE, 30), "SLIDE on and GATE 30 %% on step 8");
    grab_ed(0, before);
    sq_dice_layer(0, 2); sq_dice_layer(0, 3); sq_dice_layer(0, 4); sq_dice_all(0);
    grab_ed(0, after);
    CHECK(memcmp(before, after, sizeof before) == 0, "dice ACCNT / ARTIC / SHAPE / ALL leave every set field alone");
    CHECK(sq_slide_out(0, 7, 1, 0) == 1, "the set SLIDE of step 8 still glides, whatever the generator says");
    eval_at(0, 7, 0, &o);
    CHECK(o.gate == 30, "the set GATE of step 8 is still 30 %% (got %u)", o.gate);

    /* Morph DENS: a forced ON keeps playing; the AUTO steps thin out. */
    fresh();
    CHECK(sq_step_set(0, 6, SQ_F_ON, 1), "force step 7 ON");
    {   int before_hits = hits_of(0, 0, 16, 6);
        sq_morph_turn(0, 0, -16);                                    /* K1 = DENS, to its minimum 0 */
        CHECK(sq_ch[0].st[0].morph[0] == 0, "DENS to its minimum 0 (got %u)", sq_ch[0].st[0].morph[0]);
        eval_at(0, 6, 0, &o);
        CHECK(o.flags & PO_HIT, "DENS down: the forced-ON step 7 keeps playing");
        CHECK(hits_of(0, 0, 16, 6) == 0, "DENS down: the AUTO steps thin out to none");
        CHECK(before_hits > 1, "the AUTO steps sounded before the morph (%d)", before_hits);
        GP("morph DENS 12 -> 0: AUTO hits %d -> 0, forced ON step 7 kept\n", before_hits); }

    /* ALGORITHM change: the edits belong to the channel; AUTO steps follow the new algorithm. */
    fresh();
    CHECK(sq_step_set(0, 2, SQ_F_PITCH, 5) && sq_step_set(0, 9, SQ_F_ON, 0), "edits on steps 3 and 10");
    grab_ed(0, before);
    sq_set_algo(0, 1);
    grab_ed(0, after);
    CHECK(memcmp(before, after, sizeof before) == 0, "ALGORITHM change keeps every edit");
    {   PulseOut g4;
        gen_at(0, sq_gen_pos(&sq_ch[0], 4), &g4);
        eval_at(0, 4, 0, &o);
        CHECK(o.note == g4.note && o.vel == g4.vel, "an AUTO step follows the new algorithm (step 5)"); }
    sq_set_algo(0, 0);

    /* Global SCALE: a set PITCH is re-snapped; turning the scale back restores every note exactly. */
    fresh();
    CHECK(sq_step_set(0, 10, SQ_F_PITCH, 3), "PITCH 3 on step 11");
    {   uint8_t n_before, old_scale;
        eval_at(0, 10, 0, &o);
        n_before = o.note;
        gg = &sq_gdb[sq_gidx];
        old_scale = gg->s.scale;
        gg->s.scale = 0; sq_gbuild(gg);                              /* CHROM: every semitone is a scale tone */
        eval_at(0, 10, 0, &o);
        CHECK(o.note == sq_note(gg, 3), "a SCALE change re-snaps the set PITCH (note %u)", o.note);
        gg->s.scale = old_scale; sq_gbuild(gg);
        eval_at(0, 10, 0, &o);
        CHECK(o.note == n_before, "turning SCALE back restores the note (%u)", o.note); }

    /* LENGTH: the edits on a hidden page are kept and come back with it. */
    fresh();
    CHECK(sq_len_set(0, 32), "extend to 32");
    CHECK(sq_step_set(0, 20, SQ_F_PITCH, 9) && sq_step_set(0, 20, SQ_F_VEL, 33), "edits on step 21 (page 2)");
    grab_ed(0, before);
    CHECK(sq_len_set(0, 16), "shrink to 16: page 2 hidden");
    CHECK(sq_len_set(0, 32), "extend to 32 again");
    grab_ed(0, after);
    CHECK(memcmp(before, after, sizeof before) == 0, "LENGTH shrink and extend keep the hidden page's edits");

    /* Preset load: the preset carries the edits and replaces them. */
    fresh();
    CHECK(sq_step_set(0, 1, SQ_F_VEL, 99), "vel 99 on step 2");
    {   SeqPreset p;
        CHECK(sq_preset_save(&p, NULL) == 0, "preset save");
        CHECK(sq_step_set(0, 1, SQ_F_VEL, 12), "the live step 2 is edited to vel 12");
        CHECK(sq_preset_load(&p, NULL, 0) == 0, "preset load");
        CHECK(sq_ch[0].ed[1].vel == 99, "a preset load replaces the edits (vel %u)", sq_ch[0].ed[1].vel); }

    /* The clear gestures. */
    fresh();
    CHECK(sq_step_set(0, 9, SQ_F_PITCH, 4) && sq_step_set(0, 9, SQ_F_VEL, 77), "PITCH and VEL on step 10");
    CHECK(sq_step_set(0, 9, SQ_F_PITCH, SQ_AUTO), "RESET + one field: PITCH");
    CHECK(sq_ch[0].ed[9].note == 0 && sq_ch[0].ed[9].vel == 77, "RESET + one field keeps the other fields");
    CHECK(sq_step_clear(0, 9) && sq_ch[0].ed[9].vel == 0 && sq_ch[0].ed[9].fl == 0, "RESET + key: every field of one step");
    CHECK(sq_step_set(0, 2, SQ_F_OCT, 1) && sq_step_set(0, 5, SQ_F_RATCH, 3) && sq_step_set(0, 6, SQ_F_PROB, 50), "edits on page 1");
    CHECK(sq_page_clear(0, 0), "RESET + REC: every edit on the viewed page");
    {   int any = 0;
        for (uint8_t p = 0; p < 16; p++) any |= sq_ch[0].ed[p].note | sq_ch[0].ed[p].vel | sq_ch[0].ed[p].gr | sq_ch[0].ed[p].fl;
        CHECK(!any && (sq_ch[0].pg_used & 1u), "page 1 is all AUTO and still holds content"); }
    fresh();
    CHECK(sq_len_set(0, 32), "page 2 in view");
    CHECK(sq_page_src_set(0, 1, 1), "page 2 OWN");
    CHECK(sq_step_set(0, 3, SQ_F_VEL, 60), "edit page 1 step 4");
    CHECK(sq_page_recopy(0, 1), "RESET + D#4: re-copy page 1 onto page 2");
    CHECK(sq_ch[0].ed[19].vel == 60 && ((sq_ch[0].pg_src >> 2) & 3u) == 0, "re-copy: the edits and the source of the page before");
    CHECK(!sq_page_recopy(0, 0), "page 1 has no page before");
    dump("page 2 after the re-copy (steps 17..24)", 0, 16, 24);
}

/* ---- 1.3 pages, 1.4 sources, 1.6 length and undo: the worked examples ---- */
static void section_13_16(void)
{   PulseOut a, b;
    int same = 1;
    GP("== 1.3 pages, 1.4 sources, 1.6 length\n");

    /* Adding page 2 copies it; the music does not change. */
    fresh();
    CHECK(sq_step_set(0, 3, SQ_F_PITCH, 5) && sq_step_set(0, 4, SQ_F_ON, 0) && sq_step_set(0, 6, SQ_F_VEL, 110) && sq_step_set(0, 9, SQ_F_RATCH, 3),
          "edits on page 1");
    CHECK(sq_page_add(0) && sq_ch[0].len == 32, "PG+ adds page 2 (LENGTH 32)");
    for (uint8_t p = 0; p < 16; p++) { eval_at(0, p, 0, &a); eval_at(0, (uint8_t)(p + 16), 0, &b); if (!same_out(&a, &b)) same = 0; }
    CHECK(same, "adding page 2 does not change the music (steps 17..32 repeat 1..16)");
    CHECK(memcmp(&sq_ch[0].ed[0], &sq_ch[0].ed[16], 16 * sizeof(StepEdit)) == 0, "page 2 is an exact copy of page 1's edits");
    CHECK(((sq_ch[0].pg_src >> 2) & 3u) == 0 && ((sq_ch[0].pg_used >> 1) & 1u), "page 2 reads page 1 (src 0) and holds content");
    dump("page 1 and its copy (the loop is the same 32 steps)", 0, 0, 16);

    /* Page 3 copies page 2 as it is edited. */
    CHECK(sq_step_set(0, 20, SQ_F_VEL, 99), "edit page 2 step 21");
    CHECK(sq_page_add(0) && sq_ch[0].len == 48, "PG+ adds page 3 (LENGTH 48)");
    CHECK(sq_ch[0].ed[36].vel == 99 && sq_ch[0].ed[0].vel != 99, "page 3 copies page 2 as edited; page 1 is unaffected");
    CHECK(sq_page_add(0) && sq_ch[0].len == 64, "PG+ adds page 4 (LENGTH 64)");
    CHECK(!sq_page_add(0) && sq_ch[0].len == 64, "64 STEPS = MAX: PG+ changes nothing");

    /* Partial page: LENGTH 20 shows steps 17-20; the extend filled page 2 whole, so PG+ shows its stored steps. */
    fresh();
    CHECK(sq_step_set(0, 2, SQ_F_VEL, 44), "edit step 3");
    CHECK(sq_len_set(0, 20), "LENGTH 20 (page 2 partial)");
    CHECK(memcmp(&sq_ch[0].ed[0], &sq_ch[0].ed[16], 16 * sizeof(StepEdit)) == 0, "page 2 was filled whole, although only 17-20 show");
    CHECK(sq_page_add(0) && sq_ch[0].len == 32, "PG+ on a partial page: SHOW ALL OF PAGE 2 (LENGTH 32), no new page");
    CHECK(sq_ch[0].pg_used == 3, "still two pages held");

    /* Hidden page: page 3 comes back as stored, with the edit made while it was in view. */
    fresh();
    CHECK(sq_len_set(0, 48), "LENGTH 48");
    CHECK(sq_step_set(0, 33, SQ_F_VEL, 77), "edit page 3 step 34");
    CHECK(sq_len_set(0, 32), "shrink to 32: page 3 hidden");
    CHECK(((sq_ch[0].pg_used >> 2) & 1u) && sq_ch[0].ed[33].vel == 77, "page 3 is hidden and keeps its edit");
    CHECK(sq_page_add(0) && sq_ch[0].len == 48 && sq_ch[0].ed[33].vel == 77, "PG+ shows the hidden page 3 as stored (LENGTH 48)");

    /* 1.6: extend onto a page never held (a copy); a hidden page comes back exactly as it was. */
    fresh();
    CHECK(sq_step_set(0, 1, SQ_F_PITCH, 4), "edit page 1 step 2");
    CHECK(sq_len_set(0, 32) && sq_ch[0].ed[17].note == sq_ch[0].ed[1].note && sq_ch[0].ed[17].note != 0, "extend onto a new page copies it");
    CHECK(sq_step_set(0, 17, SQ_F_VEL, 44), "edit page 2 step 18");
    CHECK(sq_len_set(0, 16), "shrink: page 2 hidden");
    CHECK(sq_step_set(0, 0, SQ_F_VEL, 12), "page 1 step 1 edited while page 2 is hidden");
    CHECK(sq_len_set(0, 32), "extend back");
    CHECK(sq_ch[0].ed[17].vel == 44 && sq_ch[0].ed[16].vel == 0, "a hidden page comes back exactly, not copied from page 1");
    CHECK(sq_page_recopy(0, 1) && sq_ch[0].ed[16].vel == 12 && sq_ch[0].ed[17].vel == 0, "RESET + D#4 is the commit that discards hidden content");

    /* Undo of a page add (EDIT tap): one page, the LENGTH and the page bits. */
    fresh();
    CHECK(sq_step_set(0, 2, SQ_F_VEL, 66), "edit step 3");
    CHECK(sq_page_add(0) && sq_ch[0].len == 32, "add page 2");
    CHECK(sq_undo(0) == 1 && sq_ch[0].len == 16 && sq_ch[0].pg_used == 1 && sq_ch[0].ed[16].vel == 0 && sq_ch[0].ed[2].vel == 66,
          "undo of the page add: LENGTH 16, page 2 empty again");
    CHECK(sq_undo(0) == 1 && sq_ch[0].len == 32 && sq_ch[0].ed[18].vel == 66, "undo again is redo: page 2 back");
    CHECK(sq_page_clear(0, 1) && sq_ch[0].ed[18].vel == 0 && !((sq_ch[0].pg_used >> 1) & 1u), "RESET + REC clears page 2 (no content)");
    CHECK(sq_undo(0) == 1 && sq_ch[0].ed[18].vel == 66, "EDIT tap undoes the page clear");

    /* FOLLOW (1.5): one flag, on by default. */
    sq_follow_set(0);
    CHECK(sq_follow == 0, "FOLLOW can be turned off");
    sq_follow_set(1);
    CHECK(sq_follow == 1, "FOLLOW is back on");

    /* 1.4: the generator position and the OWN continuation. */
    fresh();
    CHECK(sq_len_set(0, 48), "LENGTH 48");
    CHECK(sq_gen_pos(&sq_ch[0], 33) == 1 && sq_gen_pos(&sq_ch[0], 40) == 8, "page 3 copies page 2, which reads block 0: g(33) = 1, g(40) = 8");
    CHECK(sq_page_src_set(0, 1, 1) && sq_gen_pos(&sq_ch[0], 17) == 17, "page 2 OWN: its generator positions are 17..32");
    CHECK(sq_gen_pos(&sq_ch[0], 33) == 1, "page 3 keeps block 0");
    CHECK(!sq_page_src_set(0, 0, 1) && sq_gen_pos(&sq_ch[0], 3) == 3, "page 1 is always its own: a source above 0 is refused");
    CHECK(sq_page_src_set(0, 2, 2) && sq_page_add(0) && sq_ch[0].len == 64, "page 3 OWN, PG+ adds page 4");
    CHECK(((sq_ch[0].pg_src >> 6) & 3u) == 2 && sq_gen_pos(&sq_ch[0], 49) == 33, "page 4 reads block 2 (P3): g(49) = 33");
    GP("sources: pg_src 0x%02X, g(33)=%u g(49)=%u\n", (unsigned)sq_ch[0].pg_src, (unsigned)sq_gen_pos(&sq_ch[0], 33), (unsigned)sq_gen_pos(&sq_ch[0], 49));
}

/* ---- 1.1 ON: a white-key tap writes the opposite of what the step plays; two taps never leave a stale edit; ranges are refused ---- */
static void section_tap(void)
{   int bad = 0;
    fresh();
    for (uint8_t p = 0; p < 16; p++) {
        const int gen = sq_gen_hit(0, p);
        PulseOut o;
        CHECK(sq_step_tap(0, p), "tap step %u", p);
        eval_at(0, p, 0, &o);
        if (((o.flags & PO_HIT) ? 1 : 0) != !gen) bad++;
        if ((sq_ch[0].ed[p].fl & 3u) != (gen ? 2u : 1u)) bad++;          /* the field is set: OFF over a sounding step, ON over a rest */
        CHECK(sq_step_tap(0, p), "second tap on step %u", p);
        if ((sq_ch[0].ed[p].fl & 3u) != 0u) bad++;                       /* back to the generator: AUTO */
    }
    CHECK(bad == 0, "tap: the opposite of the generator, then AUTO on the second tap (%d wrong)", bad);
    CHECK(!sq_step_tap(0, 16) && !sq_step_set(0, 16, SQ_F_VEL, 5), "a step past LENGTH is refused by tap and by set");
    CHECK(!sq_step_set(0, 0, SQ_F_GATE, 15) && !sq_step_set(0, 0, SQ_F_ON, 2) && !sq_step_set(0, 0, SQ_F_RATCH, 5) &&
          !sq_step_set(0, 0, SQ_F_OCT, 3) && !sq_step_set(0, 0, SQ_F_PROB, 75), "out-of-range values are refused and write nothing");
    CHECK(sq_ch[0].ed[0].gr == 0 && sq_ch[0].ed[0].fl == 0, "a refused write leaves the step AUTO");
    CHECK(sq_step_set(0, 0, SQ_F_OCT, -2) && sq_ch[0].ed[0].fl == (uint8_t)(1u << 5), "OCT -2 is stored as 1");
    CHECK(sq_step_set(0, 0, SQ_F_OCT, SQ_AUTO) && sq_ch[0].ed[0].fl == 0, "OCT AUTO clears it");
}

/* ---- 1.6: the shrink past the playhead ---- */
static void section_wrap(void)
{   GP("== 1.6 shrink past the playhead\n");
    fresh();
    CHECK(sq_ch[0].len == 16, "LENGTH 16");
    sq_rt[0].p = 12; sq_rt[0].grid = 0; sq_rt[0].pass = 0;
    CHECK(sq_len_set(0, 5), "shrink to 5 with the playhead on step 13");
    sq_step_start(0, 0, 48);
    CHECK(sq_rt[0].p == 1 && sq_rt[0].pass == 0, "the next step is step 1 (p wraps), got p=%u", sq_rt[0].p);
    GP("shrink past the playhead: next p = %u\n", (unsigned)sq_rt[0].p);
}

/* ---- the scheduler: slides and TIE. Each scenario is a stop, a silent channel, edits, and a play to a fixed tick. ---- */
static void scen(uint8_t len, uint8_t algo)
{   sq_on_stop();
    rec_on = 0; nev = 0;
    reset_ch(0, len);
    if (sq_ch[0].algo != algo) sq_set_algo(0, algo);
    sq_ch[0].muted = 0;
    for (uint8_t p = 0; p < len; p++) sq_ch[0].ed[p].fl = 2;      /* every step forced OFF until the scenario turns one on */ }

static void play_to(uint32_t end_tick)
{   rec_on = 1;
    sq_on_start();
    while (sq_now < end_tick) sq_block(64);
    rec_on = 0; }

static void dump_events(const char *title)
{   GP("%s\n", title);
    for (int i = 0; i < nev; i++)
        GP("  t%05u %s ch%u n%03u\n", (unsigned)evs[i].tick, evs[i].on ? "ON " : "OFF", (unsigned)evs[i].ch + 1u, (unsigned)evs[i].note); }

static int at_tick(uint8_t on, uint8_t note, uint32_t tick)                /* index of the event, or -1 */
{   for (int i = 0; i < nev; i++) if (evs[i].on == on && evs[i].note == note && evs[i].tick == tick) return i;
    return -1; }

static int first_ev(uint8_t on, uint8_t note)                              /* index of the first such event, or -1 */
{   for (int i = 0; i < nev; i++) if (evs[i].on == on && evs[i].note == note) return i;
    return -1; }

static void set_step(uint8_t p, uint8_t on, int pitch, int gate, int slide)  /* a scenario's step: on / off, PITCH, GATE, SLIDE (-1 = AUTO) */
{   sq_step_set(0, p, SQ_F_ON, on);
    if (pitch >= 0) sq_step_set(0, p, SQ_F_PITCH, pitch);
    if (gate >= 0) sq_step_set(0, p, SQ_F_GATE, gate);
    if (slide >= 0) sq_step_set(0, p, SQ_F_SLIDE, slide); }

static void section_sched(void)
{   uint8_t A, B;
    int i, j;
    long v0;
    GP("== scheduler: slides and TIE\n");

    /* S1: TIE on step 1 holds over a rest into step 3; the tie's OFF comes before the ON of step 3 (both at tick 96). */
    v0 = viol;
    fresh();
    scen(4, 0);
    A = sq_note(&sq_gdb[sq_gidx], 0); B = sq_note(&sq_gdb[sq_gidx], 2);
    set_step(0, 1, 0, SQ_GATE_TIE, -1);
    set_step(2, 1, 2, 50, -1);
    play_to(400);
    dump_events("S1 TIE over a rest (len 4: steps 1 and 3 sound)");
    i = first_ev(0, A); j = first_ev(1, B);
    CHECK(i >= 0 && evs[i].tick == 96, "S1: the tied note ends at the start of step 3 (tick 96)");
    CHECK(j > i && evs[j].tick == 96, "S1: note B starts at tick 96, after the OFF of the tied note");
    CHECK(viol == v0, "S1: no double ON or stray OFF");

    /* S2: TIE into the same pitch: the OFF and the ON of that note meet at the boundary, OFF first. */
    v0 = viol;
    fresh();
    scen(2, 0);
    A = sq_note(&sq_gdb[sq_gidx], 0);
    set_step(0, 1, 0, SQ_GATE_TIE, -1);
    set_step(1, 1, 0, 50, -1);
    play_to(200);
    dump_events("S2 TIE into the same pitch (len 2)");
    i = first_ev(0, A);
    CHECK(i >= 0 && evs[i].tick == 48, "S2: the tied note ends at tick 48");
    CHECK(i + 1 < nev && evs[i + 1].on == 1 && evs[i + 1].note == A && evs[i + 1].tick == 48, "S2: the same note starts again at tick 48, after its OFF");
    CHECK(viol == v0, "S2: no double ON");

    /* S3: TIE on the last step holds across the loop end into step 1 of the next pass (tick 144). */
    v0 = viol;
    fresh();
    scen(3, 0);
    A = sq_note(&sq_gdb[sq_gidx], 4); B = sq_note(&sq_gdb[sq_gidx], 0);
    set_step(0, 1, 4, 50, -1);
    set_step(2, 1, 0, SQ_GATE_TIE, -1);
    play_to(400);
    dump_events("S3 TIE across the loop end (len 3)");
    i = first_ev(0, B);
    CHECK(i >= 0 && evs[i].tick == 144, "S3: the tie from step 3 ends at step 1 of the next pass (tick 144)");
    CHECK(at_tick(1, A, 144) >= 0, "S3: step 1 of the next pass sounds at the same tick");
    CHECK(viol == v0, "S3: no violation across the wrap");

    /* S4: a TIE with no other sounding step holds until its own next start, where the note sounds again. */
    v0 = viol;
    fresh();
    scen(4, 0);
    A = sq_note(&sq_gdb[sq_gidx], 0);
    set_step(0, 1, 0, SQ_GATE_TIE, -1);
    play_to(400);
    dump_events("S4 TIE with no other sounding step (len 4)");
    i = first_ev(0, A);
    CHECK(i >= 0 && evs[i].tick == 192 && i + 1 < nev && evs[i + 1].on == 1 && evs[i + 1].tick == 192,
          "S4: the note ends at tick 192 and sounds again then");
    CHECK(viol == v0, "S4: no violation");

    /* S5: a SLIDE on a non-pairing algorithm (the edit makes it): the next note starts before this one ends. */
    v0 = viol;
    fresh();
    scen(2, 1);                                                      /* PLAIN: no AF_PAIRING, so only an edited SLIDE glides */
    A = sq_note(&sq_gdb[sq_gidx], 0); B = sq_note(&sq_gdb[sq_gidx], 2);
    set_step(0, 1, 0, 50, 1);
    set_step(1, 1, 2, 50, -1);
    play_to(200);
    dump_events("S5 SLIDE into the next step (PLAIN, len 2)");
    i = first_ev(1, B); j = first_ev(0, A);
    CHECK(i >= 0 && evs[i].tick == 48, "S5: note B starts at tick 48");
    CHECK(j >= 0 && evs[j].tick == 54 && i < j, "S5: note A runs on to tick 54 (the glide), after B has started");
    CHECK(viol == v0, "S5: no violation");

    /* S6: a slide across the loop end: step 2 glides into step 1 of the next pass (tick 96). */
    v0 = viol;
    fresh();
    scen(2, 1);
    A = sq_note(&sq_gdb[sq_gidx], 2); B = sq_note(&sq_gdb[sq_gidx], 0);
    set_step(0, 1, 2, 50, -1);
    set_step(1, 1, 0, 50, 1);
    play_to(200);
    dump_events("S6 SLIDE across the loop end (PLAIN, len 2)");
    i = at_tick(1, A, 96);
    j = at_tick(0, B, 102);
    CHECK(i >= 0 && j > i, "S6: note A starts at 96 (the next pass) before B is released");
    CHECK(j >= 0, "S6: note B runs on to tick 102 (the glide)");
    CHECK(viol == v0, "S6: no violation");

    /* S7: PROB 50 %: sounding on some passes and silent on others, the same on every run from PLAY. */
    v0 = viol;
    fresh();
    scen(1, 1);
    set_step(0, 1, 0, 50, -1);
    sq_step_set(0, 0, SQ_F_PROB, 50);
    {   int played = 0, sounds[16], n1;
        static Ev first[4096];
        for (int q = 0; q < 16; q++) sounds[q] = 0;
        play_to(48 * 16);
        for (i = 0; i < nev; i++) if (evs[i].on && evs[i].tick / 48 < 16) sounds[evs[i].tick / 48] = 1;
        for (int q = 0; q < 16; q++) played += sounds[q];
        CHECK(played > 0 && played < 16, "S7: PROB 50 %% sounds on some passes and not others (%d of 16)", played);
        GP("PROB 50 %%: %d of 16 passes sound\n", played);
        dump_events("S7 PROB 50 % (len 1)");
        n1 = nev;
        memcpy(first, evs, (size_t)n1 * sizeof evs[0]);
        scen(1, 1);
        set_step(0, 1, 0, 50, -1);
        sq_step_set(0, 0, SQ_F_PROB, 50);
        play_to(48 * 16);
        CHECK(nev == n1 && memcmp(first, evs, (size_t)n1 * sizeof evs[0]) == 0, "S7: the same settings play the same passes again");
    }
    CHECK(viol == v0, "S7: no violation");
}

/* ---- the golden: the resolved tables and the event logs ---- */
static void golden_check(void)
{   const char *path = "tests/golden/sq_edit.txt";
    FILE *f;
    if (getenv("GOLDEN_UPDATE")) {
        f = fopen(path, "wb");
        if (!f) { printf("FAIL cannot write %s\n", path); fails++; return; }
        fwrite(gb, 1, gn, f);
        fclose(f);
        printf("  wrote %s (%zu B)\n", path, gn);
        return; }
    f = fopen(path, "rb");
    if (!f) { printf("FAIL missing golden %s (GOLDEN_UPDATE=1 writes it)\n", path); fails++; return; }
    {   char *old = (char *)malloc(sizeof gb);
        size_t n = old ? fread(old, 1, sizeof gb, f) : 0;
        fclose(f);
        if (!old || n != gn || memcmp(old, gb, gn) != 0) { printf("FAIL %s differs from this run (GOLDEN_UPDATE=1 rewrites it)\n", path); fails++; }
        else printf("  ok  %s (%zu B)\n", path, gn);
        free(old); }
}

int main(void)
{   sq_boot();
    section_12();
    section_13_16();
    section_tap();
    section_wrap();
    section_sched();
    golden_check();
    if (irq_bad) { printf("FAIL the IRQ guard did not nest (%d)\n", irq_bad); fails++; }
    if (viol) { printf("FAIL %ld MIDI violations\n", viol); fails++; }
    if (fails) { printf("SQ_EDIT FAIL (%d)\n", fails); return 1; }
    printf("SQ_EDIT PASS\n");
    return 0;
}
