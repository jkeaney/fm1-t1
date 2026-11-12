/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_chan_test.c (P4-005): the channel state API of firmware/src/sq_chan.c against mock hooks and a test ALGOS table that carries the
 * real v1 morph / seed / layer tables of ADR-0003 4.1 (layers as the locked prototypes bind them, SHAPE -> SQ_SEED_MOD).
 *   cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -fno-strict-aliasing -fsanitize=address,undefined -Ifirmware/src \
 *      -o /tmp/sq_chan_test tests/sq_chan_test.c && /tmp/sq_chan_test [fuzz-seed]
 * Run from the repo root. The mock build() copies its inputs (morphs, the six seeds and s_mod) into the cache, so a test can read back
 * exactly what the core handed to build() and which buffer is live.
 *
 * Tables: SEQUENCER_UX 1.2 rows (dice / morph / algorithm change never touch the edit layer; a preset load replaces it), the unique-MIDI
 * rule of 4.4 (decisions 77), undo / redo swap (decision 46), the rebuild epoch guard, the defaults of ADR-0003 12 field by field,
 * preset round trip, queued load, corrupt-preset rejection and clamping; then a 100000-tick fuzz of random API calls, the scheduler
 * running, with the sq_core invariants (no double ON, no OFF without ON, nothing left sounding) plus the state invariants. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sq_types.h"
#include "sq_prims.h"
#include "sq_prims.c"

/* ---- host side of the device hooks ---- */
typedef struct { int unused; } track_t;
static track_t trk[SQ_NCH];
#define MO_RESERVE 8u
static uint32_t mock_free = 128;
static uint32_t midi_out_free(void) { return mock_free; }
static uint32_t beat_samples(void) { return 22050; }
#define SQ_EXT_BEAT() 22050u                         /* P4-007: sq_core counts its own beat (120 BPM at 44100 here) */
#define SQ_BPM()      120u
static uint32_t sq_cur_dbg;
static int midi_cnt[16][128], prv_cnt[SQ_NCH][128];
static long viol, midi_on_total, midi_off_total;
static const char *last_what = "";
static void vio(const char *w, unsigned ch, unsigned n) { if (viol++ < 3) printf("  violation: %s ch=%u n=%u tick %u after %s\n", w, ch, n, (unsigned)sq_cur_dbg, last_what); }
static void midi_out_event_at(uint32_t pkt, uint32_t due)
{   unsigned st = (pkt >> 8) & 0xF0u, ch = (pkt >> 8) & 0xFu, d1 = (pkt >> 16) & 0xFFu, d2 = pkt >> 24;
    (void)due;
    if (st == 0xF0u) return;                         /* clock out (P4-007) */
    if (mock_free) mock_free--;
    if (st == 0x90u && d2) { if (midi_cnt[ch][d1]) vio("double ON", ch, d1); midi_cnt[ch][d1] = 1; midi_on_total++; }
    else if (st == 0x80u || st == 0x90u) { if (!midi_cnt[ch][d1]) vio("OFF without ON", ch, d1); midi_cnt[ch][d1] = 0; midi_off_total++; }
    else if (!(st == 0xB0u && d1 == 123u)) vio("stray", ch, st); }
static void trk_note_on(track_t *t, uint32_t note, uint32_t vel) { (void)vel; unsigned c = (unsigned)(t - trk); if (prv_cnt[c][note]) vio("prv double ON", c, (unsigned)note); prv_cnt[c][note] = 1; }
static void trk_note_off(track_t *t, uint32_t note) { unsigned c = (unsigned)(t - trk); if (!prv_cnt[c][note]) vio("prv OFF without ON", c, (unsigned)note); prv_cnt[c][note] = 0; }

static int irq_depth, irq_bad;                       /* SQ_IRQ_OFF / ON must nest to zero */
#define SQ_IRQ_OFF() (irq_depth++)
#define SQ_IRQ_ON()  (irq_depth-- > 0 ? 0 : irq_bad++)

/* ---- the test ALGOS table ---- */
typedef struct { uint8_t m[SQ_NMORPH]; uint16_t seed[SQ_NSEED + 1]; uint8_t id, key; } Cache;   /* what build() was handed */
_Static_assert(sizeof(Cache) <= SQ_CACHE_MAX, "Cache fits");
static int  builds;                                  /* build() calls since the last reset */
static void (*build_hook)(void);                     /* run once inside the next build() (the epoch race) */
static uint8_t cur_id_for_build;

static void t_build(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *k)
{   Cache *c = (Cache *)k;
    if (build_hook) { void (*h)(void) = build_hook; build_hook = NULL; h(); }
    memcpy(c->m, m, SQ_NMORPH);
    memcpy(c->seed, seed, sizeof c->seed);           /* seed[6] is s_mod: the core hands build() a seventh slot */
    c->id = cur_id_for_build; c->key = g->s.key;
    builds++; }
static uint8_t t_len(const void *k) { (void)k; return 16; }
static void t_step(const void *k, uint32_t g, const HarmChord *hc, PulseOut *o)
{   const Cache *c = (const Cache *)k; uint32_t h = sq_mix(c->seed[0] ^ (uint32_t)c->m[0] << 16 ^ g * 0x9E3779B1u); (void)hc;
    memset(o, 0, sizeof *o);
    o->note = (uint8_t)(36 + h % 12); o->vel = (uint8_t)(1 + (h >> 8) % 127); o->gate = (uint8_t)(1 + (h >> 16) % 100);
    o->flags = (uint8_t)(((h >> 24) % 3u) ? PO_HIT : 0); }

#define M(l, mn, mx, df, st, fs) { l, mn, mx, df, st, fs }
#define LY(...) { __VA_ARGS__ }
#define SM SQ_SEED_MOD
#define ID8 { 0, 1, 2, 3, 4, 5, 6, 7 }
static const AlgoDesc ALGOS[SQ_NALGO] = {
  [0] = { .name = "ACID", .id = 0, .n_seed = 6, .flags = AF_PAIRING, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4, 5, 6 },
          .morph = { M("DENS",0,16,12,1,1), M("PEDAL",0,12,6,1,1), M("JUMP",0,20,5,1,1), M("ACCNT",0,8,3,1,1),
                     M("OFFSET",0,15,0,1,1), M("SLIDE",0,8,4,1,1), M("PUNCH",0,47,40,1,4), M("DIR",0,2,0,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{4}), LY(2,{3,5}), LY(2,{2,SM}) },
          .sub = { "NOTES", "HITS", "ACCNT", "GT+SLD", "OCTAVE" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [1] = { .name = "EUCLID", .id = 1, .n_seed = 4, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 0xFF, 1, 2, 3 },
          .morph = { M("HITS",0,16,5,1,1), M("LEN",2,16,16,1,1), M("SPAN",0,24,12,1,1), M("ACCNT",0,8,2,1,1),
                     M("OFFSET",0,15,0,1,1), M("RATCH",0,8,4,1,1), M("PUNCH",0,47,40,1,4), M("CURVE",0,6,0,1,1) },
          .layer = { LY(1,{1}), LY(1,{0}), LY(1,{2}), LY(1,{3}), LY(1,{SM}) },
          .sub = { "NOTES", "ROTATE", "ACCNT", "RATCH", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [2] = { .name = "SNAKE", .id = 2, .n_seed = 4, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4 },
          .morph = { M("DENS",0,16,10,1,1), M("LEN",1,16,7,1,1), M("RANGE",0,48,12,1,4), M("PATH",1,14,7,1,1),
                     M("OFFSET",0,15,0,1,1), M("GLEN",1,16,16,1,1), M("DYN",0,10,10,1,1), M("DIR",0,2,0,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{2}), LY(0), LY(2,{3,SM}) },       /* ARTIC absent: inert */
          .sub = { "NOTES", "GATES", "VELO", "-", "DRIFT" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [3] = { .name = "STAGE", .id = 3, .n_seed = 5, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4, 5 },
          .morph = { M("DENS",0,8,6,1,1), M("TOTAL",8,64,16,1,4), M("RANGE",0,36,12,1,4), M("RATCH",0,8,2,1,1),
                     M("OFFSET",0,63,0,1,4), M("RDIV",1,4,4,1,1), M("DYN",0,10,10,1,1), M("DIR",0,2,0,1,1) },
          .layer = { LY(1,{0}), LY(1,{2}), LY(1,{4}), LY(1,{3}), LY(2,{1,SM}) },
          .sub = { "NOTES", "RESTS", "VELO", "RT+HLD", "LONG" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [4] = { .name = "ARPT", .id = 4, .n_seed = 5, .cache_bytes = sizeof(Cache), .knob_morph = { 3, 2, 1, 0, 4, 5, 6, 7 },   /* a remap, to test knob_morph */
          .seed_k = { 1, 2, 3, 4, 5, 0xFF },
          .morph = { M("A",0,10,2,2,4), M("B",0,10,3,1,1), M("C",1,10,4,1,1), M("D",0,100,50,5,20),
                     M("E",0,3,0,1,1), M("F",0,3,1,1,1), M("G",0,3,2,1,1), M("H",0,3,3,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{2}), LY(1,{3}), LY(2,{4,SM}) },
          .sub = { "NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step },
  [5] = { .name = "PATT", .id = 5, .n_seed = 5, .cache_bytes = sizeof(Cache), .knob_morph = ID8,
          .seed_k = { 1, 2, 3, 4, 5, 0xFF },
          .morph = { M("A",0,15,0,1,1), M("B",0,15,1,1,1), M("C",0,15,2,1,1), M("D",0,15,3,1,1),
                     M("E",0,15,4,1,1), M("F",0,15,5,1,1), M("G",0,15,6,1,1), M("H",0,15,7,1,1) },
          .layer = { LY(1,{0}), LY(1,{1}), LY(1,{2}), LY(1,{3}), LY(2,{4,SM}) },
          .sub = { "NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE" }, .build = t_build, .loop_len = t_len, .step = t_step },
};

#include "sq_core.c"
#include "sq_chan.c"

/* ---- helpers ---- */
static int fails;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

static const Cache *live(uint8_t c) { return (const Cache *)sq_cache[c][sq_cidx[c]]; }

/* the cache a channel SHOULD have for its current state */
static int cache_matches(uint8_t c)
{   const Cache *k = live(c); const ChanSave *cs = &sq_ch[c]; uint16_t sd[SQ_NSEED + 1];
    for (int i = 0; i < SQ_NSEED; i++) sd[i] = cs->st[cs->algo].seed[i];
    sd[SQ_SEED_MOD] = sq_smod[c][cs->algo];
    return sq_calgo[c][sq_cidx[c]] == cs->algo && !memcmp(k->m, cs->st[cs->algo].morph, SQ_NMORPH) && !memcmp(k->seed, sd, sizeof sd); }

/* build() records the algorithm id the core asked for: the mock cannot see it, so the table sets it from the call order. */
static void boot_mock(void)
{   memset(sq_cache, 0, sizeof sq_cache); memset(sq_calgo, 0, sizeof sq_calgo); memset((void *)sq_cidx, 0, sizeof sq_cidx);
    memset(sq_stage_ch, 0, sizeof sq_stage_ch); sq_load_pending = 0; sq_epoch = 0; sq_playing = 0; sq_now = 0; sq_cur = 0;
    sq_entropy_ctr = 0x12345; sq_dice_n = 0; builds = 0; build_hook = NULL; mock_free = 128;
    memset(midi_cnt, 0, sizeof midi_cnt); memset(prv_cnt, 0, sizeof prv_cnt); viol = midi_on_total = midi_off_total = 0;
    sq_boot(); }

static uint32_t edhash(uint8_t c) { uint32_t h = 2166136261u; const uint8_t *p = (const uint8_t *)sq_ch[c].ed; for (size_t i = 0; i < sizeof sq_ch[c].ed; i++) h = (h ^ p[i]) * 16777619u; return h; }
static void put_edits(uint8_t c)                      /* a hand-made bassline: every kind of set field */
{   for (int i = 0; i < SQ_NSTEP; i++) {
        sq_ch[c].ed[i].note = (uint8_t)(0x80u | (i % 25)); sq_ch[c].ed[i].vel = (uint8_t)(i % 128);
        sq_ch[c].ed[i].gr = (uint8_t)((i % 12) | (((i / 3) % 5) << 4)); sq_ch[c].ed[i].fl = (uint8_t)((i % 3) | ((i % 3) << 2) | ((i & 1) << 4) | ((1 + i % 5) << 5)); } }
static int same_seeds(uint8_t c, const uint16_t *snap, uint8_t mask)   /* seeds NOT in mask unchanged */
{   for (int i = 0; i < SQ_NSEED; i++) if (!(mask & (1u << i)) && sq_ch[c].st[sq_ch[c].algo].seed[i] != snap[i]) return 0;
    return 1; }

/* ---- SEQUENCER_UX 1.2: dice, morph and algorithm change leave the edit layer alone; the layer rewrites only its own seeds ---- */
static void t_rows_1_2(void)
{   static const struct { const char *what; uint8_t algo, layer; uint8_t mask; uint8_t smod; } rows[] = {
        { "ACID  dice NOTES",  0, 0, 0x01, 0 }, { "ACID  dice HITS",  0, 1, 0x02, 0 }, { "ACID  dice ACCNT", 0, 2, 0x10, 0 },
        { "ACID  dice ARTIC",  0, 3, 0x28, 0 }, { "ACID  dice SHAPE",  0, 4, 0x04, 1 },
        { "EUCLID dice HITS (ROT)", 1, 1, 0x01, 0 }, { "EUCLID dice SHAPE", 1, 4, 0x00, 1 },
        { "SNAKE dice POLY/SHAPE", 2, 4, 0x08, 1 }, { "STAGE dice SHAPE", 3, 4, 0x02, 1 },
        { "ARPT  dice SHAPE", 4, 4, 0x10, 1 }, { "PATT  dice ARTIC", 5, 3, 0x08, 0 } };
    for (unsigned r = 0; r < sizeof rows / sizeof rows[0]; r++) {
        uint8_t c = 1; uint16_t snap[SQ_NSEED], sm; uint32_t eh;
        boot_mock(); sq_set_algo(c, rows[r].algo); put_edits(c); eh = edhash(c);
        memcpy(snap, sq_ch[c].st[rows[r].algo].seed, sizeof snap); sm = sq_smod[c][rows[r].algo];
        sq_dice_layer(c, rows[r].layer);
        CHECK(edhash(c) == eh, "%s: edit layer untouched", rows[r].what);
        CHECK(same_seeds(c, snap, rows[r].mask), "%s: only the layer's seeds move", rows[r].what);
        for (int i = 0; i < SQ_NSEED; i++) if (rows[r].mask & (1u << i)) CHECK(sq_ch[c].st[rows[r].algo].seed[i] != snap[i], "%s: seed %d changed", rows[r].what, i);
        CHECK(rows[r].smod ? sq_smod[c][rows[r].algo] != sm : sq_smod[c][rows[r].algo] == sm, "%s: s_mod %s", rows[r].what, rows[r].smod ? "re-rolled" : "kept");
        CHECK(cache_matches(c), "%s: cache rebuilt from the new state", rows[r].what);
    }
    {   uint8_t c = 0; uint16_t snap[SQ_NSEED]; uint32_t eh;          /* dice ALL, morph, algorithm change on a channel with edits */
        boot_mock(); put_edits(c); eh = edhash(c); memcpy(snap, sq_ch[c].st[0].seed, sizeof snap);
        sq_dice_all(c);
        CHECK(edhash(c) == eh, "dice ALL: edit layer untouched");
        for (int i = 0; i < SQ_NSEED; i++) CHECK(sq_ch[c].st[0].seed[i] != snap[i], "dice ALL: ACID seed %d changed", i);
        sq_morph_turn(c, 0, -5); sq_morph_turn(c, 3, 2); sq_morph_turn(c, 7, 1);
        CHECK(edhash(c) == eh, "morph: edit layer untouched");
        sq_set_algo(c, 3); sq_set_algo(c, 1); sq_set_algo(c, 0);
        CHECK(edhash(c) == eh, "algorithm change: edit layer untouched");
        CHECK(cache_matches(c), "after the algorithm round trip the cache is ACID's again"); }
    {   uint8_t c = 2; SeqPreset p; boot_mock();                       /* preset load replaces the edits (the preset carries them) */
        put_edits(c); sq_preset_save(&p); memset(p.ch[c].ed, 0, sizeof p.ch[c].ed); p.ch[c].ed[5].vel = 99;
        CHECK(sq_preset_load(&p, 0) == 0, "preset load ok");
        CHECK(sq_ch[c].ed[5].vel == 99 && sq_ch[c].ed[6].vel == 0, "preset load: the edits are the preset's"); }
}

/* ---- morph turn ---- */
static void t_morph(void)
{   boot_mock();
    sq_morph_turn(0, 0, 3);   CHECK(sq_ch[0].st[0].morph[0] == 15, "ACID K1 12 + 3 = 15 (got %u)", sq_ch[0].st[0].morph[0]);
    sq_morph_turn(0, 0, 127); CHECK(sq_ch[0].st[0].morph[0] == 16, "clamped to max 16");
    sq_morph_turn(0, 0, -128);CHECK(sq_ch[0].st[0].morph[0] == 0, "clamped to min 0 (KEYS ONLY)");
    CHECK(live(0)->m[0] == 0 && cache_matches(0), "each turn rebuilds the cache");
    sq_morph_turn(1, 1, -1);  CHECK(sq_ch[1].st[1].morph[1] == 15, "EUCLID LEN 16 - 1");
    sq_morph_turn(1, 1, -100);CHECK(sq_ch[1].st[1].morph[1] == 2, "EUCLID LEN min is 2");
    sq_set_algo(2, 4); sq_morph_turn(2, 0, 1);                          /* knob_morph remap: knob 0 -> morph 3 (step 5) */
    CHECK(sq_ch[2].st[4].morph[3] == 55 && sq_ch[2].st[4].morph[0] == 2, "knob remap: K1 turns morph 3 by its step (got m3=%u m0=%u)", sq_ch[2].st[4].morph[3], sq_ch[2].st[4].morph[0]);
    { int b = builds; sq_morph_turn(0, 0, 1); sq_morph_turn(0, 0, 1);   /* already at... turn then no-op at the ends */
      sq_morph_turn(0, 0, 100); b = builds; sq_morph_turn(0, 0, 1); CHECK(builds == b, "a turn that changes nothing builds nothing"); }
    sq_morph_turn(9, 0, 1); sq_morph_turn(0, 8, 1);                     /* out of range: ignored */
    sq_ch[0].st[0].morph[1] = 6; sq_morph_turn(0, 1, 1); sq_undo(0);
    CHECK(sq_ch[0].st[0].morph[1] == 7, "a morph is not in the undo record (turn back instead)"); }

/* ---- undo / redo ---- */
static void t_undo(void)
{   uint16_t before[SQ_NSEED], after[SQ_NSEED], sm0, sm1; uint8_t c = 0;
    boot_mock();
    memcpy(before, sq_ch[c].st[0].seed, sizeof before); sm0 = sq_smod[c][0];
    CHECK(sq_undo(c) == 0, "undo with nothing recorded does nothing");
    sq_dice_layer(c, 0); memcpy(after, sq_ch[c].st[0].seed, sizeof after);
    CHECK(sq_undo(c) == 1 && !memcmp(sq_ch[c].st[0].seed, before, sizeof before) && cache_matches(c), "undo restores the seeds and rebuilds");
    CHECK(sq_undo(c) == 1 && !memcmp(sq_ch[c].st[0].seed, after, sizeof after) && cache_matches(c), "undo again = redo");
    CHECK(sq_undo(c) == 1 && !memcmp(sq_ch[c].st[0].seed, before, sizeof before), "and again = undo");
    /* a dice of one layer then another: only the last is undone (one record per channel, decision 46) */
    sq_dice_layer(c, 0); memcpy(after, sq_ch[c].st[0].seed, sizeof after); sq_dice_layer(c, 1);
    sq_undo(c);
    CHECK(!memcmp(sq_ch[c].st[0].seed, after, sizeof after), "dice NOTES then HITS: undo reverts HITS only");
    /* dice ALL is one record for the union, s_mod included */
    boot_mock(); memcpy(before, sq_ch[c].st[0].seed, sizeof before); sm0 = sq_smod[c][0];
    sq_dice_all(c); sm1 = sq_smod[c][0];
    CHECK(sm1 != sm0, "dice ALL re-rolled s_mod");
    CHECK(sq_undo(c) && !memcmp(sq_ch[c].st[0].seed, before, sizeof before) && sq_smod[c][0] == sm0 && cache_matches(c), "one undo restores all five layers and s_mod");
    CHECK(sq_undo(c) && sq_smod[c][0] == sm1, "redo brings s_mod back");
    /* an absent layer is inert: no change, and the record of the earlier dice survives */
    boot_mock(); sq_set_algo(2, 2); sq_dice_layer(2, 1);
    { uint16_t s[SQ_NSEED]; memcpy(s, sq_ch[2].st[2].seed, sizeof s); int b = builds; sq_dice_layer(2, 3);
      CHECK(!memcmp(s, sq_ch[2].st[2].seed, sizeof s) && builds == b, "SNAKE ARTIC is absent: inert, no rebuild");
      CHECK(sq_undo(2) == 1, "the record of the dice before it is still there"); }
    /* an algorithm switch clears the record */
    boot_mock(); sq_dice_layer(1, 2); sq_set_algo(1, 3); CHECK(sq_undo(1) == 0, "algorithm switch clears the undo record");
    /* hand-made records for the edit-layer bits: swap, swap back */
    boot_mock(); put_edits(3);
    { UndoRec *u = &sq_undo_rec[3]; StepEdit keep[SQ_PAGE]; memcpy(keep, &sq_ch[3].ed[16], sizeof keep);
      memset(u, 0, sizeof *u); u->what = 2 | 16; u->algo = 3; u->idx = 1; u->len = 33; u->pg_used = 3; u->ed[0].vel = 77;
      CHECK(sq_undo(3) && sq_ch[3].ed[16].vel == 77 && sq_ch[3].len == 33 && sq_ch[3].pg_used == 3 && u->len == 16, "page + len swap in");
      CHECK(sq_undo(3) && !memcmp(&sq_ch[3].ed[16], keep, sizeof keep) && sq_ch[3].len == 16, "and swap back"); }
    /* morph swap */
    boot_mock();
    { UndoRec *u = &sq_undo_rec[0]; memset(u, 0, sizeof *u); u->what = 4; u->algo = 0; u->morph[0] = 1;
      CHECK(sq_undo(0) && sq_ch[0].st[0].morph[0] == 1 && u->morph[0] == 12 && cache_matches(0), "morph swap rebuilds"); } }

/* ---- unique MIDI channels (SEQUENCER_UX 4.4, decision 77) ---- */
static void t_midi(void)
{   static const struct { const char *what; uint8_t in[4]; uint8_t ch, set; uint8_t out[4]; int8_t ret; } T[] = {
        { "free channel",           { 0, 1, 2, 3 }, 0, 7,  { 7, 1, 2, 3 }, -1 },
        { "taking a used one",      { 0, 1, 2, 3 }, 0, 1,  { 1, SQ_MIDI_OFF, 2, 3 }, 1 },
        { "TB-3 channel 2 (value 1)", { 0, 1, 2, 3 }, 0, 1, { 1, SQ_MIDI_OFF, 2, 3 }, 1 },
        { "CH4 takes CH1's",        { 0, 1, 2, 3 }, 3, 0,  { SQ_MIDI_OFF, 1, 2, 0 }, 0 },
        { "to OFF, nobody moves",   { 0, 1, 2, 3 }, 2, SQ_MIDI_OFF, { 0, 1, SQ_MIDI_OFF, 3 }, -1 },
        { "OFF may be shared",      { 0, SQ_MIDI_OFF, 2, SQ_MIDI_OFF }, 3, SQ_MIDI_OFF, { 0, SQ_MIDI_OFF, 2, SQ_MIDI_OFF }, -1 },
        { "from OFF onto a used one", { 0, SQ_MIDI_OFF, 2, 3 }, 1, 2, { 0, 2, SQ_MIDI_OFF, 3 }, 2 },
        { "same value: no-op",      { 0, 1, 2, 3 }, 1, 1,  { 0, 1, 2, 3 }, -1 },
        { "value > OFF rejected",   { 0, 1, 2, 3 }, 1, 17, { 0, 1, 2, 3 }, -1 },
        { "channel 15",             { 0, 1, 2, 3 }, 1, 15, { 0, 15, 2, 3 }, -1 },
    };
    for (unsigned t = 0; t < sizeof T / sizeof T[0]; t++) {
        int8_t r; uint8_t i;
        boot_mock(); for (i = 0; i < 4; i++) sq_ch[i].midi_ch = T[t].in[i];
        r = sq_set_midi(T[t].ch, T[t].set);
        CHECK(r == T[t].ret, "%s: returns %d (got %d)", T[t].what, T[t].ret, r);
        for (i = 0; i < 4; i++) CHECK(sq_ch[i].midi_ch == T[t].out[i], "%s: CH%d is %u (got %u)", T[t].what, i + 1, T[t].out[i], sq_ch[i].midi_ch);
        { uint8_t n[17] = { 0 }; for (i = 0; i < 4; i++) n[sq_ch[i].midi_ch]++;
          for (i = 0; i < 16; i++) CHECK(n[i] <= 1, "%s: MIDI channel %u used twice", T[t].what, i); }
        if (T[t].ret >= 0 || (T[t].out[T[t].ch] != T[t].in[T[t].ch])) {                  /* a change: one undo restores both, again = redo */
            uint8_t after[4]; for (i = 0; i < 4; i++) after[i] = sq_ch[i].midi_ch;
            CHECK(sq_undo(0) == 1, "%s: undo available", T[t].what);
            for (i = 0; i < 4; i++) CHECK(sq_ch[i].midi_ch == T[t].in[i], "%s: undo restores CH%d (got %u)", T[t].what, i + 1, sq_ch[i].midi_ch);
            sq_undo(0);
            for (i = 0; i < 4; i++) CHECK(sq_ch[i].midi_ch == after[i], "%s: redo re-applies CH%d", T[t].what, i + 1);
        }
    }
    /* a later undoable action takes over the undo; the algorithm, mute, pattern and edits of the displaced channel are untouched */
    boot_mock(); put_edits(1); sq_set_mute(1, 1);
    { uint32_t eh = edhash(1); sq_set_midi(0, 1);
      CHECK(sq_ch[1].midi_ch == SQ_MIDI_OFF && sq_ch[1].algo == 1 && sq_ch[1].muted && sq_ch[1].len == 16 && edhash(1) == eh, "displaced channel keeps algorithm, mute, length and edits");
      sq_dice_layer(0, 0); sq_undo(0);
      CHECK(sq_ch[0].midi_ch == 1 && sq_ch[1].midi_ch == SQ_MIDI_OFF, "after a dice, undo is the dice (the MIDI record is no longer the latest)"); }
    /* mute */
    boot_mock(); sq_set_mute(0, 1); CHECK(sq_ch[0].muted == 1, "mute on"); sq_set_mute(0, 7); CHECK(sq_ch[0].muted == 1, "mute normalised"); sq_set_mute(0, 0); CHECK(sq_ch[0].muted == 0, "mute off");
    sq_set_mute(4, 1); CHECK(irq_bad == 0, "ignored out-of-range channel"); }

/* ---- sq_set_algo ---- */
static void t_algo(void)
{   uint8_t c = 0; boot_mock();
    sq_morph_turn(c, 0, -3); sq_dice_layer(c, 0);
    { ChanSave keep = sq_ch[c]; uint16_t sm = sq_smod[c][0];
      sq_set_algo(c, 2);
      CHECK(sq_ch[c].algo == 2 && cache_matches(c) && sq_calgo[c][sq_cidx[c]] == 2, "the new algorithm is built and live");
      CHECK(!memcmp(&sq_ch[c].st[0], &keep.st[0], sizeof keep.st[0]) && sq_smod[c][0] == sm, "the old algorithm's morphs, seeds and s_mod are kept (S7)");
      sq_set_algo(c, 0);
      CHECK(cache_matches(c) && !memcmp(&sq_ch[c].st[0], &keep.st[0], sizeof keep.st[0]), "and come back when switching back"); }
    { int b; sq_set_algo(c, 99); sq_set_algo(c, 0); b = builds; sq_set_algo(c, 0); CHECK(builds == b && sq_ch[c].algo == 0, "invalid or same algorithm: nothing"); }
    /* the ISR reads the algorithm from sq_calgo of the live buffer only, so the old buffer stays valid for a tick in flight */
    { uint8_t old = sq_cidx[c]; sq_set_algo(c, 3); CHECK(sq_cidx[c] != old && sq_calgo[c][old] == 0, "flip: the old buffer still belongs to the old algorithm"); } }

/* ---- rebuild: the epoch guard and the pending rule ---- */
static void race(void) { sq_epoch++; sq_ch[0].st[0].morph[0] = 9; }          /* the ISR lands a load during build() */
static void t_rebuild(void)
{   boot_mock();
    sq_ch[0].st[0].morph[0] = 4; builds = 0; build_hook = race;
    CHECK(sq_rebuild(0) == 1 && builds == 2, "epoch moved during the build: built again (builds %d)", builds);
    CHECK(live(0)->m[0] == 9 && cache_matches(0), "the live cache is built from the state after the load");
    /* pending: the inactive buffers hold the staged caches; nothing is built, state edits stay */
    boot_mock(); sq_load_pending = 1; builds = 0;
    CHECK(sq_rebuild(0) == 0 && builds == 0, "pending load: rebuild does not touch the staged buffers");
    sq_morph_turn(0, 0, -2); CHECK(sq_ch[0].st[0].morph[0] == 10 && builds == 0, "pending: the morph is stored, nothing built");
    sq_load_pending = 0;
    CHECK(sq_rebuild(9) == 0, "bad channel");
    CHECK(irq_bad == 0 && irq_depth == 0, "ISR guards balanced"); }

/* ---- defaults (ADR-0003 12), field by field ---- */
static void t_defaults(void)
{   static const uint8_t morph_def[SQ_NALGO][SQ_NMORPH] = {
        { 12, 6, 5, 3, 0, 4, 40, 0 }, { 5, 16, 12, 2, 0, 4, 40, 0 }, { 10, 7, 12, 7, 0, 16, 10, 0 }, { 6, 16, 12, 2, 0, 4, 10, 0 },
        { 2, 3, 4, 50, 0, 1, 2, 3 }, { 0, 1, 2, 3, 4, 5, 6, 7 } };
    static const uint8_t n_seed[4] = { 6, 4, 4, 5 }, algo[4] = { SQ_ALGO_ACID, SQ_ALGO_EUCLID, SQ_ALGO_SNAKE, SQ_ALGO_STAGE };
    static const uint8_t k0[4][6] = { { 1, 2, 3, 4, 5, 6 }, { 0xFF, 1, 2, 3 }, { 1, 2, 3, 4 }, { 1, 2, 3, 4, 5 } };
    SeqPreset p;
    memset(&p, 0xA5, sizeof p); sq_defaults(&p);
    CHECK(p.version == 4 && p.flags == 0 && p.bpm == 120 && p.crc == 0, "preset header: version 4, flags 0, 120 BPM, crc 0");
    CHECK(p.g.key == 0 && p.g.octave == 0 && p.g.scale == 10 && p.g.gate == 50 && p.g.div == 2 && p.g.swing == 50, "global: C, MINPEN, oct 0, GATE 50, DIV 1/16, SWING 50");
    CHECK(p.harm.len == 0x03 && p.harm.deg[0] == 0 && p.harm.deg[1] == 0 && p.harm.deg[2] == 0 && p.harm.vm == 0x82 && p.harm.drone == 0 && p.harm.roles == 0 && p.harm.rsv == 0, "HARMONY 1---, VOICE 2, MOVE 8, no drone, AUTO");
    for (int c = 0; c < SQ_NCH; c++) {
        const ChanSave *cs = &p.ch[c];
        CHECK(cs->midi_ch == c, "CH%d MIDI %d", c + 1, c + 1);
        CHECK(cs->algo == algo[c], "CH%d algorithm %u", c + 1, algo[c]);
        CHECK(cs->muted == (c ? 1 : 0), "CH%d mute %d", c + 1, c ? 1 : 0);
        CHECK(cs->len == 16 && cs->pg_src == 0 && cs->pg_used == 1 && cs->pad[0] == 0 && cs->pad[1] == 0, "CH%d len 16, pg_src 0, pg_used 1, PREVIEW off", c + 1);
        for (int i = 0; i < SQ_NSTEP; i++) CHECK(!cs->ed[i].note && !cs->ed[i].vel && !cs->ed[i].gr && !cs->ed[i].fl, "CH%d step %d is AUTO", c + 1, i);
        for (int a = 0; a < SQ_NALGO; a++) {
            for (int i = 0; i < SQ_NMORPH; i++) CHECK(cs->st[a].morph[i] == morph_def[a][i], "CH%d algo %d morph %d = MorphDesc.def %u (got %u)", c + 1, a, i, morph_def[a][i], cs->st[a].morph[i]);
            for (int k = 0; k < SQ_NSEED; k++) {
                unsigned want = 0;
                if (a < 4 && k < n_seed[a] && k0[a][k] != 0xFF) want = (0x3A5Du * (unsigned)(c * n_seed[a] + k0[a][k])) & 0xFFFFu;
                if (a >= 4 && k < 5) want = (0x3A5Du * (unsigned)(c * 5 + (unsigned)k + 1)) & 0xFFFFu;
                CHECK(cs->st[a].seed[k] == want, "CH%d algo %d seed %d = 0x%04X (got 0x%04X)", c + 1, a, k, want, cs->st[a].seed[k]);
            }
        }
    }
    CHECK(p.ch[0].st[0].seed[0] == 0x3A5D * 1 && p.ch[1].st[0].seed[0] == 0x988B, "ACID NOTES: CH1 0x3A5D, CH2 0x988B");   /* (0*6+1), (1*6+1) */
    CHECK(p.ch[1].st[1].seed[0] == 0 && p.ch[1].st[1].seed[1] == 0x23D1, "EUCLID ROT is literally 0, NOTES CH2 0x23D1");
    CHECK(p.ch[2].st[2].seed[0] == 0x0D45 && p.ch[3].st[3].seed[2] == 0x1A8A && p.ch[3].st[0].seed[5] == 0x78B8, "SNAKE CH3, STAGE CH4, ACID CH4 seeds");
    /* sq_boot: the live state is the same, the caches are built in buffer 0, s_mod is the default */
    boot_mock();
    { SeqPreset q; sq_preset_save(&q); CHECK(!memcmp(&p, &q, sizeof p), "sq_boot state == sq_defaults"); }
    for (int c = 0; c < SQ_NCH; c++) {
        CHECK(sq_cidx[c] == 0 && cache_matches(c), "boot: CH%d cache 0 built for its algorithm", c + 1);
        for (int a = 0; a < SQ_NALGO; a++) CHECK(sq_smod[c][a] == SQ_SMOD_DEFAULT(c) && sq_smod[c][a] != 0, "boot: CH%d s_mod default", c + 1);
    }
    CHECK(sq_gidx == 0 && sq_gdb[0].snap[0] == 0, "gidx 0; snap built"); }

/* ---- presets ---- */
static void t_preset(void)
{   SeqPreset a, b, bad; boot_mock();
    sq_morph_turn(0, 0, -4); sq_dice_all(1); sq_set_algo(2, 0); sq_set_midi(3, 9); sq_set_mute(0, 1); put_edits(2);
    sq_preset_save(&a);
    boot_mock();
    CHECK(sq_preset_load(&a, 0) == 0, "load ok");
    sq_preset_save(&b);
    CHECK(!memcmp(&a, &b, sizeof a), "save -> load -> save round trip is byte-identical");
    for (int c = 0; c < SQ_NCH; c++) CHECK(cache_matches(c) || c == 1, "after the load CH%d's cache matches its state (CH2's s_mod is not in the preset)", c + 1);
    CHECK(sq_load_pending == 0 && irq_depth == 0, "stopped: applied at once, guards balanced");
    /* undo is cleared by a load */
    boot_mock(); sq_dice_layer(0, 0); sq_preset_load(&a, 0); CHECK(sq_undo(0) == 0, "a load clears the undo records");
    boot_mock(); sq_set_midi(0, 1); sq_preset_load(&a, 0); CHECK(sq_undo(0) == 0, "and the MIDI record");
    /* queued: the ISR lands it on the bar */
    boot_mock(); sq_on_start(); sq_now = 0;
    for (uint32_t t = 0; t < 10; t++) { sq_tick(sq_now); sq_now++; }
    { SeqPreset live0; sq_preset_save(&live0); CHECK(sq_preset_load(&a, 1) == 0 && sq_load_pending == 1, "playing: queued");
      sq_preset_save(&b); CHECK(!memcmp(&live0, &b, sizeof b), "state unchanged until the bar");
      while (sq_now < SQ_BAR_TICKS) { sq_tick(sq_now); sq_now++; }
      CHECK(sq_load_pending == 1 && sq_epoch == 0, "just before the bar still pending");
      sq_tick(sq_now); sq_now++;
      CHECK(sq_load_pending == 0 && sq_epoch == 1, "applied at tick 768");
      sq_chan_sync(); sq_preset_save(&b); CHECK(!memcmp(&a, &b, sizeof a), "after the bar the state is the preset's");
      for (int c = 0; c < SQ_NCH; c++) CHECK(cache_matches(c) || c == 1, "queued load: CH%d cache matches", c + 1); }
    sq_on_stop();
    /* a newer load replaces a pending older one */
    boot_mock(); sq_on_start(); sq_tick(0); sq_now = 1;
    b = a; b.ch[0].len = 5; sq_preset_load(&a, 1); sq_preset_load(&b, 1);
    while (sq_now <= SQ_BAR_TICKS) { sq_tick(sq_now); sq_now++; }
    CHECK(sq_ch[0].len == 5, "the newer queued load wins");
    sq_on_stop();
    /* corrupt presets */
    boot_mock(); sq_preset_save(&b);
    bad = a; bad.version = 3;  CHECK(sq_preset_load(&bad, 0) == -1, "version 3 rejected");
    bad = a; bad.version = 0;  CHECK(sq_preset_load(&bad, 0) == -1, "version 0 rejected");
    bad = a; bad.version = 255; CHECK(sq_preset_load(&bad, 1) == -1, "version 255 rejected");
    bad = a; bad.ch[2].algo = SQ_NALGO; CHECK(sq_preset_load(&bad, 0) == -1, "algorithm 6 rejected");
    bad = a; bad.ch[3].algo = 200;      CHECK(sq_preset_load(&bad, 0) == -1, "algorithm 200 rejected");
    { SeqPreset now; sq_preset_save(&now); CHECK(!memcmp(&b, &now, sizeof b) && sq_load_pending == 0, "a rejected load changes nothing"); }
    /* clamping */
    bad = a;
    bad.bpm = 5; bad.g.key = 40; bad.g.octave = -9; bad.g.scale = 99; bad.g.gate = 0; bad.g.div = 77; bad.g.swing = 200;
    bad.ch[0].midi_ch = 200; bad.ch[0].len = 0; bad.ch[0].muted = 9; bad.ch[0].st[0].morph[0] = 255; bad.ch[0].pg_used = 0xFF;
    bad.ch[1].midi_ch = 4; bad.ch[2].midi_ch = 4; bad.ch[3].midi_ch = 4; bad.ch[1].len = 200; bad.ch[1].st[1].morph[1] = 0;
    bad.ch[2].ed[7].note = 0xFF; bad.ch[2].ed[7].vel = 255; bad.ch[2].ed[7].gr = 0xFF; bad.ch[2].ed[7].fl = 0xFF;
    bad.harm.len = 0x2F; bad.harm.vm = 0xFF; bad.harm.drone = 99; bad.harm.deg[0] = 0xFF; bad.harm.deg[1] = 0xFF; bad.harm.deg[2] = 0xFF; bad.harm.rsv = 5;
    CHECK(sq_preset_load(&bad, 0) == 0, "out-of-range fields load, clamped");
    CHECK(sq_bpm == 40 && sq_gdb[sq_gidx].s.key == 11 && sq_gdb[sq_gidx].s.octave == -2 && sq_gdb[sq_gidx].s.scale == 15 && sq_gdb[sq_gidx].s.gate == 10 &&
          sq_gdb[sq_gidx].s.div == 5 && sq_gdb[sq_gidx].s.swing == 75, "global fields clamped, bpm 40");
    CHECK(sq_ch[0].midi_ch == SQ_MIDI_OFF && sq_ch[0].len == 1 && sq_ch[0].muted == 1 && sq_ch[0].st[0].morph[0] == 16 && sq_ch[0].pg_used == 0x0F, "CH1: midi 200 -> OFF, len 1, muted 1, morph clamped to max, pg_used & 15");
    CHECK(sq_ch[1].midi_ch == 4 && sq_ch[2].midi_ch == SQ_MIDI_OFF && sq_ch[3].midi_ch == SQ_MIDI_OFF, "duplicate MIDI channel: the lowest channel keeps it");
    CHECK(sq_ch[1].len == 64 && sq_ch[1].st[1].morph[1] == 2, "len 200 -> 64; EUCLID LEN 0 -> its min 2");
    { const StepEdit *e = &sq_ch[2].ed[7]; CHECK((e->note & 0x7F) <= 96 && e->vel <= 127 && (e->gr & 15) <= 11 && ((e->gr >> 4) & 7) <= 4 && !(e->gr & 0x80) &&
                                         (e->fl & 3) <= 2 && ((e->fl >> 2) & 3) <= 2 && (e->fl >> 5) <= 5, "step edit fields forced inside their encodings"); }
    { unsigned bars = (sq_harm.len & 7u) + 1u, nch = ((sq_harm.len >> 3) & 7u) + 1u; uint32_t d = sq_harm.deg[0] | sq_harm.deg[1] << 8 | (uint32_t)sq_harm.deg[2] << 16; int ok = 1;
      for (int i = 0; i < 8; i++) if (((d >> (3 * i)) & 7u) > 6u) ok = 0;
      CHECK(nch <= bars && ok && (sq_harm.vm & 15) <= 6 && (sq_harm.vm >> 4) <= 8 && sq_harm.drone <= 14 && sq_harm.rsv == 0, "HARMONY clamped: nchords <= bars, degrees 0..6, VOICE <= 6, MOVE <= 8, drone <= 14"); }
    { uint8_t n[17] = { 0 }; for (int c = 0; c < 4; c++) n[sq_ch[c].midi_ch]++; for (int m = 0; m < 16; m++) CHECK(n[m] <= 1, "unique MIDI after the load"); }
    for (int c = 0; c < SQ_NCH; c++) CHECK(cache_matches(c) || c == 1, "clamped load: CH%d cache matches", c + 1);
    CHECK(irq_bad == 0 && irq_depth == 0, "guards balanced"); }

/* ---- the fuzz: random API calls with the scheduler running ---- */
static uint32_t fz_s = 0x1234567u, fz_seed = 0x1234567u;
static uint32_t fz_rnd(uint32_t n) { fz_s ^= fz_s << 13; fz_s ^= fz_s >> 17; fz_s ^= fz_s << 5; return (fz_s >> 8) % n; }

static void state_ok(const char *after)
{   uint8_t used[17] = { 0 };
    CHECK(irq_depth == 0 && irq_bad == 0, "after %s: ISR guard balance", after);
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        const ChanSave *cs = &sq_ch[c];
        CHECK(cs->algo < SQ_NALGO && cs->midi_ch <= SQ_MIDI_OFF && cs->len >= 1 && cs->len <= SQ_NSTEP && cs->muted <= 1, "after %s: CH%d basic ranges", after, c + 1);
        if (cs->algo >= SQ_NALGO) return;
        for (int i = 0; i < SQ_NMORPH; i++) {
            const MorphDesc *m = &ALGOS[cs->algo].morph[i];
            CHECK(cs->st[cs->algo].morph[i] >= m->min && cs->st[cs->algo].morph[i] <= m->max, "after %s: CH%d morph %d in range", after, c + 1, i);
        }
        if (cs->midi_ch < SQ_MIDI_OFF) { CHECK(!used[cs->midi_ch], "after %s: MIDI channel %u used twice", after, cs->midi_ch); used[cs->midi_ch] = 1; }
        if (!sq_load_pending) CHECK(cache_matches(c), "after %s: CH%d live cache matches the state", after, c + 1);
        if (ALGOS[cs->algo].seed_k[0] == 0xFF && cs->algo == 1) { /* ROT may be 0 only from defaults / a preset, never from a dice: checked in the table test */ }
    } }

static void fuzz(long nticks)
{   long done = 0; static SeqPreset saved, junk;
    boot_mock(); fz_s = fz_seed ? fz_seed : 1u; mock_free = 128;
    sq_on_start();
    for (uint8_t c = 0; c < SQ_NCH; c++) { sq_ch[c].pad[0] = (c & 1u) ? 0x80u : 0u; sq_ch[c].muted = 0; }   /* ch 2 and 4 preview; all unmuted */
    sq_preset_save(&saved);
    while (done < nticks) {
        uint32_t r = fz_rnd(1000); uint8_t c = (uint8_t)fz_rnd(SQ_NCH); uint32_t eh[SQ_NCH]; const char *what = "tick";
        sq_cur_dbg = sq_now;
        mock_free = fz_rnd(8) == 0 ? fz_rnd(12) : 128;
        for (int i = 0; i < SQ_NCH; i++) eh[i] = edhash((uint8_t)i);
        sq_entropy_ctr += fz_rnd(5000);
        if (r < 120) { sq_morph_turn(c, (uint8_t)fz_rnd(9), (int8_t)((int)fz_rnd(21) - 10)); what = "morph"; }
        else if (r < 150) { sq_dice_layer(c, (uint8_t)fz_rnd(SQ_NLAYER + 1)); what = "dice"; }
        else if (r < 160) { sq_dice_all(c); what = "dice all"; }
        else if (r < 190) { sq_undo((uint8_t)fz_rnd(SQ_NCH + 1)); what = "undo"; }
        else if (r < 215) { sq_set_midi(c, (uint8_t)fz_rnd(19)); what = "midi"; }
        else if (r < 235) { sq_set_algo(c, (uint8_t)fz_rnd(SQ_NALGO + 1)); what = "algo"; }
        else if (r < 255) { sq_set_mute(c, (uint8_t)(fz_rnd(4) == 0)); what = "mute"; }
        else if (r < 262) { sq_preset_save(&saved); what = "save"; }
        else if (r < 268) { sq_preset_load(&saved, (uint8_t)fz_rnd(2)); what = "load"; for (int i = 0; i < SQ_NCH; i++) eh[i] = edhash((uint8_t)i); }
        else if (r < 272) {                                            /* random junk: must be rejected or clamped into a valid state */
            uint8_t *p = (uint8_t *)&junk; for (size_t i = 0; i < sizeof junk; i++) p[i] = (uint8_t)fz_rnd(256);
            for (int k = 0; k < SQ_NCH; k++) junk.ch[k].pad[0] = sq_ch[k].pad[0];   /* a preview toggle mid-note is the preview task's (P2-024), not this API's */
            if (fz_rnd(4)) { junk.version = 4; for (int k = 0; k < SQ_NCH; k++) junk.ch[k].algo = (uint8_t)fz_rnd(SQ_NALGO + (fz_rnd(8) == 0)); }
            sq_preset_load(&junk, (uint8_t)fz_rnd(2)); what = "junk load"; for (int i = 0; i < SQ_NCH; i++) eh[i] = edhash((uint8_t)i); }
        else if (r < 280) { sq_req |= (r & 1u) ? SQ_REQ_STOP : SQ_REQ_START; }
        else if (r < 285) { if (!sq_load_pending && sq_playing) put_edits(c); for (int i = 0; i < SQ_NCH; i++) eh[i] = edhash((uint8_t)i); }
        if (what[0] != 't' && what[0] != 'l' && what[0] != 'j' && what[0] != 's' && what[0] != 'u')   /* morph / dice / midi / algo / mute never touch edits */
            for (int i = 0; i < SQ_NCH; i++) CHECK(eh[i] == edhash((uint8_t)i), "after %s: CH%d edit layer changed", what, i + 1);
        last_what = what;
        sq_chan_sync();
        sq_consume_requests();
        if (sq_off_stage) { if (sq_all_off() && sq_playing) sq_rt_run(sq_now); }
        if (sq_playing && !sq_off_stage) { sq_tick(sq_now); sq_now++; done++; sq_chan_sync(); }
        else sq_block(32);
        if (what[0] != 't') state_ok(what);
        if (fails > 20) { printf("  fuzz aborted after %ld ticks\n", done); return; }
    }
    mock_free = 128; sq_req = 0; sq_load_pending = 0;
    sq_on_stop();
    for (int i = 0; i < 8 && sq_off_stage; i++) { mock_free = 128; sq_block(32); }
    { int open = 0, q = 0;
      for (int m = 0; m < 16; m++) for (int n = 0; n < 128; n++) open += midi_cnt[m][n];
      for (int k = 0; k < SQ_NCH; k++) for (int n = 0; n < 128; n++) open += prv_cnt[k][n];
      for (int i = 0; i < SQ_EVQ_N; i++) q += sq_evq[i].kind != 0;
      CHECK(viol == 0, "fuzz: %ld packet violations (double ON, OFF without ON, stray)", viol);
      CHECK(open == 0 && q == 0 && !sq_off_stage, "fuzz: %d notes sounding, %d queued after the final STOP", open, q);
      CHECK(midi_on_total == midi_off_total, "fuzz: #ON %ld != #OFF %ld", midi_on_total, midi_off_total);
      CHECK(midi_on_total > 400, "fuzz played something (%ld notes)", midi_on_total);
      printf("  fuzz: %ld ticks, %ld MIDI notes\n", done, midi_on_total); } }

/* EUCLID ROT never lands on 0 from a dice (ADR-0003 3.1) */
static void t_rot(void)
{   int zero = 0; boot_mock(); sq_set_algo(0, 1);
    for (int i = 0; i < 4000; i++) { sq_entropy_ctr = (uint32_t)i * 7919u; sq_dice_layer(0, 1); if (sq_ch[0].st[1].seed[0] == 0) zero++; }
    CHECK(zero == 0, "EUCLID ROT diced to 0 %d times", zero); }

int main(int argc, char **argv)
{   if (argc > 1) fz_seed = (uint32_t)strtoul(argv[1], NULL, 0);
    printf("sq_chan channel state API\n");
    t_rows_1_2(); t_morph(); t_undo(); t_midi(); t_algo(); t_rebuild(); t_defaults(); t_preset(); t_rot();
    fuzz(100000);
    printf("%s\n", fails ? "SQ_CHAN FAIL" : "SQ_CHAN PASS");
    return fails ? 1 : 0; }
