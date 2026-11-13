# ADR-0003: Sequencer core architecture and the algorithm interface

Status: **proposed, amended by P1-013** (task P2-004, 2026-11-10). Awaiting the owner's re-approval at the human-review
gate of P1-013 (which also covers this ADR). Nothing here is locked until the owner writes "Approved: <name> <date>"
in that log and, per ADR-0004, "Design locked" later.

> Change note (P1-013, owner decision "Shift", 2026-11-10): an algorithm now has **8 morphs** (two knob rows, see
> [FULL_CONTROL.md](../../design/FULL_CONTROL.md)). Changed: `SQ_NMORPH` 4 to 8; `AlgoState` 16 to 20 B; `ChanSave`
> 86 to 102 B; `SeqPreset` 356 to 420 B; `AlgoDesc.knob_morph[8]`, `morph[8]`, `build(m[8], ...)`; the morph tables of
> 4.1; the cache notes; the RAM table of section 11 (re-added with new totals); section 15 flex points. Also set as
> **settled** by the owner's seventh round (2026-11-10): swing doubled (43), GATE 50 % neutral (44), queued load on a
> fixed 768-tick bar (45), one undo record per channel (46), no note cut on algorithm or MIDI-channel change (47); the
> "owner question" wording for these is removed.

> Change note (P1-014 revision, scheme B2; for owner review): the MUTATE snapshot `sq_mut` (24 B) and `sq_mutate()` are **removed** (MUTATE is dropped as a gesture, PERFORMANCE_DESIGN 4.2); a **REC state** is added (armed bit, write cursor, ACC / SLIDE flags, REC octave: 4 B, plus a 27-entry sounding-note table so a key-up sends the matching note-off: 27 B) and 2 B for the held-step pitch gesture; the section 11.1 total is re-added below (6962 B, headroom 39534 B after the removals and 11022 B before, 4 KB floor respected). `EDIT` tap is UNDO (hold = RESET layer); the first P1-014 note's `UNDO = ARP tap` is superseded. The undo record also covers a written REC note (`what` bit 3 one step record). A preset load clears `UndoRec.what` (toast `NOTHING TO UNDO`). New API: `sq_rec_arm(on)`, `sq_rec_write(ch, key, vel)` (writes a pin at the cursor or the step nearest the playhead, `step_rec` flags from ACC / SLIDE) and `sq_pin_pitch(ch, pos, degree)` (hold + press pitch; scale degree to semitones through the section 5 table). Reasoning: PERFORMANCE_DESIGN 3.1, 3.1b, 9.1.
> Change note (P1-014, performance-first design, owner eighth round; for owner review): [PERFORMANCE_DESIGN.md](../../design/PERFORMANCE_DESIGN.md)
> adds control-level state. Changed: `UndoRec` 34 to 48 B (it now carries the 8 morphs and one touched step record so that a reset and a
> pin edit are undoable); (MUTATE snapshot withdrawn by the revision note above) `sq_perf` flags (SOLO, ROLL), saved FILL morph, scene A / B (two preset-sized
> pool copies, volatile, recalled through the section 10 bar-queued load); new API `sq_reset_morph / sq_reset_row / sq_reset_algo / sq_reset_layer`
> (factory values from `MorphDesc.def` and `seed_k[]`, so no new table); section 11 table re-added (section 11.1); section 15 flex rows. **Reserved,
> not specified here:** the ninth-round requirements R1-R4 (per-channel pattern length, pages, per-step properties, keybed expression). The
> single-pin `pin_mask` / `pin_note[16]` layout of section 3 is an upper-bound placeholder for P1-016's per-step record (reserve 64 steps x 3 B x 4 channels
> = 768 B live + 768 B undo, already inside the 11.1 totals); P1-016 integrates it. The RAM floor holds (11.1). The pin-edit gestures of section 9 (K3 NOTE, OCT- / OCT+ = +-12) are superseded by the held-step band of PERFORMANCE_DESIGN 3.1 (two rows of four properties); P1-016 integrates them. Key labels in this ADR that say `EDIT-tap` or `D#5` follow the P1-014 control map (UNDO = EDIT tap after the P1-014 revision, restart = F#5).

> Change note (P1-017, sequencer UX lock; for owner review): [SEQUENCER_UX.md](../../design/SEQUENCER_UX.md) replaces the pin model and the R1-R4 reserve with a **per-step edit layer** and **pages**. Section 9.1 of SEQUENCER_UX is the normative struct block and supersedes the matching structs of section 3 below: `StepEdit` 4 B (PITCH unsnapped + set bit, VELO, GATE / TIE, RATCH, ON, SLIDE, PROB, OCT; every field AUTO or set), `ChanSave` 102 -> **344 B** (`pin_mask` / `pin_note[16]` removed; `len` 1..64, `pg_src` (2 bits per page: the generator block it reads), `pg_used`, `StepEdit ed[64]` added), `UndoRec` 48 -> **92 B** (one page of edits, morphs, len / page bits; the pin fields removed), `SeqPreset` 420 -> **1388 B** (version 2), `ChanRT` stays 24 B: `uint32_t t` is removed and the new fields go first, in the order `uint32_t grid; uint16_t pass; uint8_t p, tie;` followed by the existing fields unchanged (`nxt` .. `open`); compiled in revision 2 (offsets grid 0, pass 4, p 6, tie 7, nxt 8, open 21; size 24). Replacing `t` in place (p, pass, tie before grid) pads to 28 B and fails the assert. `PulseOut.note` becomes ALWAYS valid like `vel` / `gate` (EUCLID evaluates its contour at non-hit positions). `step()` is called with the **generator position** `g(p) = 16 * src(k) + p % 16` (page `k = p / 16`; `src(0) = 0`; a new page inherits `src` of the page before, so a copy is exact; `src(k) = k` is OWN) instead of the free-running `t`; the loop resets at `len`. Section 9's pin overlay is replaced by the resolution of SEQUENCER_UX 9.2; swing parity is by `p`; a SLIDE edit means "glide into the next step" (step `p+1`, after the last step step 0; no glide if that step does not sound) and feeds the existing slide pairing, **across the loop end too** (revision 1: 7.3 step 5's `(t + 1) % loop_len != 0` term is replaced by the next position `n = (p + 1 >= len) ? 0 : p + 1`; the pairing is cancelled only by STOP and by a LENGTH change inside the step); TIE holds until the next sounding step, searched with the same wrap as a slide, so it crosses the loop end too (revision 2; at most one loop, and since P4-042 at most 17 steps: the ISR looks `SQ_TIE_SCAN` = 16 steps ahead; a `tie` flag in `ChanRT`, cleared by `sq_all_off` on STOP and closed at the next step start by a LENGTH change). New main-loop API (names as built, P4-006 / P4-024): `sq_step_tap(ch,p)` (was `sq_step_toggle`), `sq_step_set(ch,p,field,value)`, `sq_step_clear(ch,p)`, `sq_step_dice(ch,p,what)` (not yet built: P6-002 owns it), `sq_note_edit_begin(ch,p)` (a NOTE_EDIT session: the first changing write takes the UndoRec), `sq_page_add(ch)`, `sq_page_recopy(ch,page)` (was `sq_page_copy`), `sq_page_clear(ch,page)`, `sq_len_set(ch,len)` (was `sq_set_length`), `sq_page_src_set(ch,page,src)` (was `sq_set_source`), `sq_global_set(field,value)` (KEY SCALE OCTAVE GATE DIV SWING, P4-024); removed: `sq_pin_set / sq_pin_clear / sq_pin_clear_all / sq_pin_pitch` and the scene API `sq_scene` (scenes are dropped, owner decision 58). The undo of a preset load applies at the next step and restores the previous header slot; a PRESETS turn makes the slot **pending** (stopped: loads after `SQ_LOAD_SETTLE` 1.5 s; playing: on the first bar that starts at least `SQ_LOAD_SETTLE` after the last detent, revision 2) and a SAVE tap while pending saves into that slot instead (section 10). The `K3 SOURCE` list on the CHAN page offers only the blocks earlier pages play, labelled by the first page that plays each, plus OWN (SEQUENCER_UX 1.4, revision 2); the stored `pg_src` is unchanged. Power-on MIDI channels stay CH1..CH4 = 1..4 (section 12; the 2..5 proposal was rejected by the owner, who asked for a first-run MIDI CHANNEL prompt instead). The 4.1 `knob_morph[]` tables change (3B re-lock): ACID `{0,5,2,3,4,1,6,7}`, EUCLID `{0,4,5,3,1,2,6,7}`, SNAKE unchanged, STAGE `{0,1,7,3,4,5,6,2}`. RAM re-added in section 11.2.

> Change note (FB-001 / P2-003, tempo source and clock out; additive, owner decision 2026-11-10): the FM-1 is the MIDI clock **master** in v1 when its tempo source is INT. New: section 5 row "Clock out" and section 7.4. The screen states are in SEQUENCER_UX 2.8 and SCREEN_SPEC 6. No struct, size or budget changes beyond about 8 B of `.bss` (a pulse counter and two flags); the E4 dev task is `FB-001` triage output.

Inputs: [VISION](../VISION.md), [ADR-0004](ADR-0004-design-lock.md) (and its flexibility addendum),
[OWNER_QUESTIONS](../OWNER_QUESTIONS.md) (all owner decisions), [ALGORITHM_SET](../../design/ALGORITHM_SET.md)
(SP1-SP15, layer vocabulary), [UX_REVIEW](../../design/UX_REVIEW.md), [CONTROL_MAP](../../design/CONTROL_MAP.md),
[SYSTEM_MAP](../../design/SYSTEM_MAP.md), the four assessments in `docs/design/algorithms/` (sting = ACID, t1 = EUCLID,
snake = SNAKE, ml185 = STAGE), `firmware/src/t1_glue.c`, `torso_t1_pipeline.c`, `torso_t1.h`, `seq.c` (`events_block`).

This ADR is a **contract**. A Haiku developer implements it literally. Where it says "the table", the table is data,
not code. Section numbers are cited by dev tasks (ADR-0004 rule: a dev task cites its locked section).

## 1. Context

The firmware has a working Torso T-1 pipeline (6 tracks x 6 cycles, 128-byte `CycleParams`, a 192-PPQN evaluator that
re-derives a step from parameters every tick, a 256-entry micro-event queue, LFSR randomness, p-locks). The product
(VISION) needs 4 channels, each with a MIDI channel and an algorithm; steps typed on 16 keys; knobs that morph; buttons
that re-roll single layers; deterministic seeds; undo; and global scale/key/GATE/DIV/SWING. The T-1 evaluator cannot
express this (it has no layers, no per-layer seeds, no pins, no slide), and its data model is ~10x too big. We keep its
**clock idea and its proven maths**, and replace the evaluator.

## 2. Decision in one page

1. **Pure algorithms, shared scheduler.** Each algorithm is two pure functions, `build()` (params + seeds + global
   context -> a small cache) and `step()` (cache + step counter `t` -> one `PulseOut`). It never sees the clock, MIDI
   or the UI. A shared **core** turns `PulseOut` into timed MIDI (swing, gate, ratchet, slide, hold), pins, mute and
   panic. (Sections 4, 5, 7.)
2. **Everything an algorithm exposes is a table** (`AlgoDesc`): name, eight morphs (ROW 1 K1..K4, ROW 2 S1..S4) with
   labels/ranges/defaults, the knob-slot-to-morph map, five shared layers -> seed lists, sub-labels, seed defaults. Remapping a knob, a dice button or a
   range is a table edit (ADR-0004 addendum). (Section 4.)
3. **Time is 192 PPQN**, accumulated from audio samples exactly as `t1_block` does today. All six DIV values are whole
   tick counts. (Section 7.)
4. **Seeds are 16-bit, per channel x per algorithm x per layer-seed**, stored in full for all four algorithms so
   switching back restores (S7). Undo is one record per channel (last dice / dice-all / pin-clear), swap = redo.
   (Sections 3, 6.)
5. **Events leave through one function** (`sq_emit`) to `midi_out_event` and, for every channel with PREVIEW on (decision 110, addendum 21), to that channel's preview
   part (ANALOG on `trk[ch-1]`). Note-offs are never dropped; stop/panic/disconnect silence everything. (Section 8.)
6. **Always on, boots working.** No mode switch, no T-1 flag. (Section 12.)
7. **RAM: +2.3 KB `.bss` for the core (2257 B)**, ~7.6 KB pool (7560 B); fits today and after the E3 removals. (Section 11.)

## 3. Data model (exact C; all structs live in a new `sq_types.h`, included before every `sq_*.c`)

Constants:

```c
#define SQ_NCH        4          /* channels (owner decision 31) */
#define SQ_NALGO      4          /* 0 ACID, 1 EUCLID, 2 SNAKE, 3 STAGE */
#define SQ_NMORPH     8          /* slots 0..3 = K1..K4 (ROW 1), 4..7 = S1..S4 (ROW 2); P1-013 */
#define SQ_NSEED      6          /* seeds stored per algorithm (max of the four: ACID has 6) */
#define SQ_NLAYER     5          /* 0 NOTES, 1 HITS, 2 ACCNT, 3 ARTIC, 4 SHAPE (ALGORITHM_SET 4.1; fixed) */
#define SQ_NPIN       16         /* pin positions = the 16 white keys */
#define SQ_CACHE_MAX  64         /* bytes of an algorithm's private cache */
#define SQ_PPQN       192
#define SQ_MIDI_OFF   16         /* ChanSave.midi_ch value for OFF */
```

```c
/* One step as an algorithm reports it (and as the core consumes it). 6 bytes. */
typedef struct {
    uint8_t note;    /* final MIDI note 0..127: scale, key, octave already applied (sq_note) */
    uint8_t vel;     /* 1..127. ALWAYS valid, also when not a hit: it is the velocity a pin on this step would use */
    uint8_t gate;    /* 1..100: the algorithm's own gate as % of one step (before global GATE). ALWAYS valid */
    uint8_t flags;   /* PO_* below */
    uint8_t hold;    /* 1..8 pulses; valid only with PO_HOLD */
    uint8_t pad;     /* 0 */
} PulseOut;
#define PO_HIT        0x01   /* this step sounds */
#define PO_SLIDE_IN   0x02   /* stored glide flag "step reached by a glide from step t-1" (ACID); core decides if effective */
#define PO_HOLD       0x04   /* long note: ignores global GATE, lasts `hold` pulses (STAGE) */
#define PO_PACE_DEC   0x08   /* ratchet spacing decelerates (else accelerates; two-way, owner decision 115, section 22) */
#define PO_REP_SHIFT  4      /* bits 4..5: repeats-1 (0..3 -> 1..4 triggers in this step; EUCLID uses 1..3) */
#define PO_REP_MASK   0x30
#define PO_PIN        0x40   /* set by the core when a pin overlaid this step (UI: outline) */
_Static_assert(sizeof(PulseOut) == 6, "PulseOut");

typedef struct { char label[7]; uint8_t min, max, def, step, fast; } MorphDesc;  /* label <= 6 chars + NUL; step per slow detent, fast per detent under 40 ms (FULL_CONTROL 5.1); 12 B */
typedef struct { uint8_t n; uint8_t seed[2]; } LayerDesc;                        /* n = 0: layer absent (dimmed); 3 B */
_Static_assert(sizeof(MorphDesc) == 12, "MorphDesc");
_Static_assert(sizeof(LayerDesc) == 3, "LayerDesc");

/* Global performance settings (owner decisions 3, 4, S5). 6 B saved + 12 B derived = 18 B */
typedef struct { uint8_t key; int8_t octave; uint8_t scale; uint8_t gate; uint8_t div; uint8_t swing; } GlobalSave;
typedef struct { GlobalSave s; uint8_t snap[12]; } SeqGlobal;
/* key 0..11 (C=0); octave -2..+2; scale 0..15 (T1_SCALE_MASKS order = UX_REVIEW 6); gate 10..100 (%);
 * div 0..5 = 1/32, 1/16T, 1/16, 1/8T, 1/8, 1/4; swing 50..75 (%). BPM is NOT here: it stays song.g[G_BPM] (fx.c
 * beat_samples()). snap[r] = scale tone for interval r above the key (SP6), derived, never saved. */
_Static_assert(sizeof(GlobalSave) == 6 && sizeof(SeqGlobal) == 18, "SeqGlobal");

/* What one algorithm remembers for one channel (S7: all four are kept). 8 morphs + 6 seeds = 20 B */
typedef struct { uint8_t morph[SQ_NMORPH]; uint16_t seed[SQ_NSEED]; } AlgoState;
_Static_assert(sizeof(AlgoState) == 20, "AlgoState");

/* One channel as saved in a preset and read by the UI. The channel's only settings are midi_ch and algo (VISION 3). 102 B */
typedef struct {
    uint8_t  midi_ch;              /* 0..15 = MIDI channel 1..16; 16 = OFF (no MIDI out and so no preview: the preview plays the MIDI stream, addendum 21) */
    uint8_t  algo;                 /* 0..3 */
    uint8_t  muted;                /* 0/1 latched */
    uint8_t  pad;
    uint16_t pin_mask;             /* bit i = position i pinned (SP8). Kept across algorithm switches (N2) */
    uint8_t  pin_note[SQ_NPIN];    /* semitones above 36+key, 0..96, UNSNAPPED (so a scale change re-snaps reversibly) */
    AlgoState st[SQ_NALGO];        /* per-algorithm morphs and seeds, indexed by algorithm id */
} ChanSave;
_Static_assert(sizeof(ChanSave) == 102, "ChanSave");

/* Owner ruling 2026-11-10 (MIDI channels unique): assigning a taken MIDI channel sets the other channel to OFF. The OFF channel keeps
 * running (its lane, playhead, pattern and mute are untouched) but sq_emit sends nothing for it; the preview is silent too (it plays exactly what the channel sends as MIDI, addendum 21;
 * midi_ch == SQ_MIDI_OFF means "no MIDI out", ChanSave comment above). The reassignment is ONE undo step
 * for both channels, so it has its own 4 B record beside the per-channel UndoRec; it stays the latest undoable action until the next
 * one. +4 B .bss (SEQUENCER_UX 4.4, 9.4 headroom unchanged). */
typedef struct { uint8_t ch, old_midi, other, other_old; } UndoMidi;      /* other = 0xFF when no channel was displaced */

/* One undo/redo record per channel: the last dice / dice-all / pin edit / pin-clear / reset. Swapping it in again = redo. 48 B (P1-014) */
typedef struct {
    uint8_t  what;                 /* 0 none, bit0 seeds, bit1 pins, bit2 morphs, bit3 one step record (P1-014) */
    uint8_t  algo;                 /* algorithm the seeds / morphs belong to */
    uint8_t  seed_mask;            /* bit k = seed k is in the record */
    uint8_t  step;                 /* position of the touched step record (bit3); P1-016 sizes the record */
    uint16_t seed[SQ_NSEED];       /* the OTHER value of each masked seed (swapped on undo/redo) */
    uint16_t pin_mask;             /* the OTHER pin set */
    uint8_t  pin_note[SQ_NPIN];
    uint8_t  morph[SQ_NMORPH];     /* the OTHER 8 morph values (bit2: a reset) */
    uint8_t  step_rec[3];          /* the OTHER step record of `step` (3 B placeholder until P1-016) */
    uint8_t  pad[3];
} UndoRec;
_Static_assert(sizeof(UndoRec) == 48, "UndoRec");

/* The saved device state = a preset (P01..P16) = the autosave. 420 B */
typedef struct {
    uint8_t  version;              /* 1 */
    uint8_t  flags;                /* 0 */
    uint16_t bpm;                  /* 40..240 */
    GlobalSave g;
    ChanSave ch[SQ_NCH];
    uint16_t crc;                  /* CRC-16 over the bytes before it (the storage layer, E7, fills/checks it) */
} SeqPreset;
_Static_assert(sizeof(SeqPreset) == 420, "SeqPreset");

/* ISR-only runtime per channel. 24 B */
typedef struct {
    uint32_t t;                    /* step counter: index of the NEXT step to start; 0 at PLAY / restart */
    uint32_t grid;                 /* tick of the nominal (un-swung) start of step t */
    PulseOut nxt;                  /* pairing snapshot of step t (ALGO flag AF_PAIRING only) */
    uint8_t  nxt_ok;
    uint8_t  snd[2];               /* note+1 currently sounding (2 only during a slide overlap), 0 = free */
    uint8_t  snd_chan[2];          /* the SeqEvent.chan byte each was sent with (its note-off goes to the same place) */
    uint8_t  slide_in;             /* the step about to start was announced as a glide target */
    uint8_t  act;                  /* +1 per note-on, read by the UI as an activity pulse */
    uint8_t  open;                 /* 1 = the sounding note snd[0] has NO queued note-off (a tie was announced; section 7.3 step 7) */
} ChanRT;
_Static_assert(sizeof(ChanRT) == 24, "ChanRT");

/* A scheduled MIDI event. 8 B. The MIDI channel is stored IN the event, so a later MIDI-channel change can never
 * misdirect a pending note-off. */
typedef struct {
    uint32_t tick;                 /* 192-PPQN tick at which it is due */
    uint8_t  chan;                 /* bits 0..3 MIDI channel 0..15; bit 4 = MIDI OFF; bits 5..6 sequencer channel 0..3 */
    uint8_t  kind;                 /* 0 empty slot, 1 NOTE_OFF, 2 NOTE_ON */
    uint8_t  note;
    uint8_t  vel;
} SeqEvent;
_Static_assert(sizeof(SeqEvent) == 8, "SeqEvent");
#define SQ_EVQ_N 96                /* 768 B */
```

Static-assert plan: every `_Static_assert` above lives in `sq_types.h`; each `sq_algo_*.c` also asserts
`sizeof(its cache struct) <= SQ_CACHE_MAX` and `AlgoDesc.cache_bytes == sizeof(its cache struct)`; `sq_core.c`
asserts `SQ_EVQ_N * sizeof(SeqEvent) == 768`, `sizeof(((SeqPreset*)0)->ch) == 408`, and
`(sizeof(DIV_TICKS) / sizeof(DIV_TICKS[0])) == 6`. A struct size change is a contract change: it needs an ADR-0003
amendment, because the preset layout is stored.

### 3.1 Seed model

- A **seed** is a `uint16_t`. There is one seed per **internal layer** of an algorithm (ACID has 6, EUCLID 4, SNAKE 4,
  STAGE 5; `SQ_NSEED` = 6 slots, unused slots stay 0 and are never read).
- Stored per **channel x algorithm** (`ChanSave.st[algo].seed[k]`). Switching algorithm never touches another
  algorithm's seeds or morphs (S7). Pins are per channel and also survive a switch (N2).
- **Default** seed `k` (0-based index into the algorithm's seed list) of channel `ch` (0-based): if `seed_k[k] == 0xFF`
  the seed is literally 0 (EUCLID ROT = "no rotation"), else `(0x3A5D * (ch * n_seed + seed_k[k])) & 0xFFFF`, with
  `n_seed` and `seed_k[]` from the algorithm table (ALGORITHM_SET SP13 and each assessment's 6.1).
- **Dice** gives a seed the value `sq_entropy()` = the low 16 bits of a free-running sample counter folded with its high 16 bits (`ctr ^ ctr>>16`) (incremented by
  `n` in `sq_block`; it runs while stopped too) mixed with a dice counter (`sq_dice_n`, bumped per dice, so two dice at the same
  sample count differ); there is no `fm1_ms` hook; if the result equals the old seed, add `0x9E37`. For a layer bound to two seeds
  (ACID `ARTIC`) the second seed uses `entropy ^ 0x5A5A`. EUCLID ROT: if the new value is 0 it becomes 1. This is the
  only non-deterministic moment in the system; the value is then stored, so playback after it is deterministic.
- The **random generator** is SP1 (xorshift32, `rnd(n) = (next() >> 8) % n`, salts per layer). The salts are
  constants inside each algorithm's `build()`; they define the golden files and are **not** table data.

## 4. The algorithm interface (the vtable and its data tables)

```c
typedef struct { uint8_t key; int8_t octave; uint8_t snap[12]; } AlgoGlobalView; /* = the first/derived fields of SeqGlobal */

typedef struct AlgoDesc {
    char       name[8];                    /* "ACID" "EUCLID" "SNAKE" "STAGE" (<= 6 chars) */
    uint8_t    id;                         /* index in ALGOS[] */
    uint8_t    n_seed;                     /* seeds this algorithm uses (<= SQ_NSEED) */
    uint8_t    flags;                      /* AF_PAIRING 0x01: step t needs the next-step snapshot (slide) */
    uint8_t    cache_bytes;                /* sizeof its cache, <= SQ_CACHE_MAX */
    uint8_t    knob_morph[SQ_NMORPH];      /* knob slot (K1..K4, S1..S4) -> morph id. v1 = {0,1,2,3,4,5,6,7}. THE knob remap table */
    uint8_t    seed_k[SQ_NSEED];           /* default-seed k per seed (0xFF = literal 0) */
    MorphDesc  morph[SQ_NMORPH];           /* label/min/max/def/detent step, indexed by MORPH ID (build()'s m[]) */
    LayerDesc  layer[SQ_NLAYER];           /* shared layer -> seed ids (n = 0 absent). THE dice remap table */
    char       sub[SQ_NLAYER][8];          /* dice sub-labels ("GT+SLD"); "-" for an absent layer */
    void     (*build)(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache);
    uint8_t  (*loop_len)(const void *cache);                    /* positions of the visible loop, 2..64: pins and playhead */
    void     (*step)(const void *cache, uint32_t t, PulseOut *out);
} AlgoDesc;
extern const AlgoDesc ALGOS[SQ_NALGO];
```

**Purity contract (normative, enforced by the host test of section 14).**

1. `build` and `step` use **only** their arguments and `const` tables. No globals, no `static` mutable data, no clock, no
   MIDI, no `fm1_*`, no libc, integer arithmetic only. Stack at most 256 B for `build` (main loop), 64 B for `step` (ISR).
2. `build` reads only `g->s.key`, `g->s.octave`, `g->snap[]` of the global context. It must **not** read gate, div or
   swing (the core applies those; test: vary them, output unchanged).
3. `build` produces the cache from scratch: identical inputs give a byte-identical cache. `step` is a pure function of
   `(cache, t)`; `t` is the free-running channel step counter, so polymeter lanes are `t % lane_len` inside `step`
   (SP12). `step` must fill `vel` and `gate` for every `t`, hit or not.
4. `step` returns the algorithm's own output only. Pins, mute, the global GATE, swing and the next-step snapshot are the
   **core's** job (section 7).
5. Scale: `build` obtains notes through `sq_note(g, semi)` (below), the only place SP6/SP7/SP9 are applied.

```c
/* SP9 + SP6. semi may be negative (ACID octave -1); floor division by 12. */
static inline uint8_t sq_note(const SeqGlobal *g, int16_t semi)
{   int16_t q = semi >= 0 ? semi / 12 : -(( -semi + 11) / 12), r = semi - 12 * q;     /* r in 0..11 */
    int16_t n = 36 + g->s.key + 12 * g->s.octave + 12 * q + g->snap[r];
    return (uint8_t)(n < 0 ? 0 : n > 127 ? 127 : n); }
```
`g->snap[]` is built once per global change by `sq_gbuild()` (main loop): for each interval `r` the nearest scale tone by
circular pitch-class distance, ties to the lower (SP6; scale masks from the existing `T1_SCALE_MASKS`). Live keys use
`sq_snap_abs(g, n)` = the same table without the octave offset.

### 4.1 The data tables (v1 values; these ARE the spec, copied from ALGORITHM_SET 4.2 / 5.2 / 4.4)

Shared layer indices: 0 NOTES, 1 HITS, 2 ACCNT, 3 ARTIC, 4 SHAPE. `{n, s0, s1}` = `LayerDesc`.

| Algo | Seeds (index: name, `seed_k`) | `layer[0..4]` (NOTES HITS ACCNT ARTIC SHAPE) | `sub[0..4]` | `knob_morph` |
| --- | --- | --- | --- | --- |
| ACID (id 0, n_seed 6, `AF_PAIRING`) | 0 NOTES 1; 1 HITS 2; 2 OCT 3; 3 GATE 4; 4 ACCNT 5; 5 SLIDE 6 | `{1,0}` `{1,1}` `{1,4}` `{2,3,5}` `{1,2}` | NOTES HITS ACCNT GT+SLD OCTAVE | 0..7 |
| EUCLID (id 1, n_seed 4) | 0 ROT 0xFF; 1 NOTES 1; 2 ACCNT 2; 3 RATCH 3 | `{1,1}` `{1,0}` `{1,2}` `{1,3}` `{0}` | NOTES ROTATE ACCNT RATCH - | 0..7 |
| SNAKE (id 2, n_seed 4) | 0 NOTES 1; 1 GATES 2; 2 VEL 3; 3 POLY 4 | `{1,0}` `{1,1}` `{1,2}` `{0}` `{1,3}` | NOTES GATES VELO - DRIFT | 0..7 |
| STAGE (id 3, n_seed 5) | 0 NOTES 1; 1 LENS 2; 2 RESTS 3; 3 MODE 4; 4 VEL 5 | `{1,0}` `{1,2}` `{1,4}` `{1,3}` `{1,1}` | NOTES RESTS VELO RT+HLD LONG | 0..7 |

Morph tables (`MorphDesc{label,min,max,def,step,fast}`, values `label min,max,def,step,fast`; `step` = change per slow detent
(always 1), `fast` = change per detent when detents arrive under 40 ms apart; 4 only for ranges over 32 values). Slots
0..3 are ROW 1 (K1 DENSITY, K2 FORM, K3 PITCH, K4 SPICE), slots 4..7 are ROW 2 (S1 START, S2 ARTIC, S3 DYNAMICS,
S4 READ). Every ROW 2 default is neutral (the output equals the 4-morph design). Full definitions and formulas:
FULL_CONTROL.md section 3 and each algorithm doc 6.1.

| Algo | m0 K1 | m1 K2 | m2 K3 | m3 K4 |
| --- | --- | --- | --- | --- |
| ACID | DENS 0,16,12,1,1 | PEDAL 0,12,6,1,1 | JUMP 0,20,5,1,1 | ACCNT 0,8,3,1,1 |
| EUCLID | HITS 0,16,5,1,1 | LEN 2,16,16,1,1 | SPAN 0,24,12,1,1 | ACCNT 0,8,2,1,1 |
| SNAKE | DENS 0,16,10,1,1 | LEN 1,16,7,1,1 | RANGE 0,48,12,1,4 | PATH 1,14,7,1,1 |
| STAGE | DENS 0,8,6,1,1 | TOTAL 8,64,16,1,4 | RANGE 0,36,12,1,4 | RATCH 0,8,2,1,1 |

| Algo | m4 S1 | m5 S2 | m6 S3 | m7 S4 |
| --- | --- | --- | --- | --- |
| ACID | OFFSET 0,15,0,1,1 | SLIDE 0,8,4,1,1 | PUNCH 0,47,40,1,4 | DIR 0,2,0,1,1 |
| EUCLID | OFFSET 0,15,0,1,1 | RATCH 0,8,4,1,1 | PUNCH 0,47,40,1,4 | CURVE 0,6,0,1,1 |
| SNAKE | OFFSET 0,15,0,1,1 | GLEN 1,16,16,1,1 | DYN 0,10,10,1,1 | DIR 0,2,0,1,1 |
| STAGE | OFFSET 0,63,0,1,4 | RDIV 1,4,4,1,1 | DYN 0,10,10,1,1 | DIR 0,2,0,1,1 |

Rules: morph value 0 on knob slot 0 (K1) is shown as `KEYS ONLY` (UI rule, P2-003; no algorithm code: `build` with
`m[0] == 0` simply yields no hits). The build's parameter order `m[]` is the **morph id** order above (ACID's
assessment names `typ,den,jmp,acc`: the prototype maps names to ids `1,0,2,3`; its ROW 2 ids 4..7 are `off,sl,pun,dir`). Morphs are monotonic and reversible
(order-based, from the assessments), which is `build()`'s property, not the core's.

**Dice** (core, `sq_dice_layer(ch, L)`): for each seed id `s` in `ALGOS[a].layer[L]`: store old value in `UndoRec`, set
`new = entropy` (second seed `^ 0x5A5A`, EUCLID ROT never 0), then `sq_rebuild(ch)`. An absent layer (`n == 0`) is
inert (no seed change, no undo record) and the UI shows its toast (CONTROL_MAP). `sq_dice_all(ch)`: layers 0..4 in order,
absent skipped, ONE undo record covering the union. `sq_undo(ch)` swaps every seed in `seed_mask` (and the pin set if
`what & 2`) with the record, then rebuilds; tapping again swaps back (redo). A new dice / pin-clear overwrites the record.
The record is cleared (`what = 0`) on an algorithm switch and on a preset load.
(This **refines** ALGORITHM_SET 4.3, which keeps one previous-seed per seed: with that scheme "undo the last dice" after
dicing NOTES then HITS would revert both. One record per channel undoes exactly the last action. Settled: owner decision 46.)

### 4.2 Every v1 algorithm fits the vtable

| Feature in the assessments | How it maps | Used by |
| --- | --- | --- |
| 16-step lane of hit/note/vel/gate | `step(t)` reads `cache[t % 16]`; `loop_len = 16` | ACID, SNAKE (gate lane) |
| Loop length K2 (`LEN`, `TOTAL`) | `loop_len(cache)` returns it; `step` wraps by it | EUCLID (2..16), STAGE (8..64) |
| Polymeter (pitch lane `len`, vel lane `vlen` against 16 steps) | `step(t)` indexes each lane by `t % lane_len`; the pattern period is the lcm, which only `t` being free-running allows | SNAKE |
| Stage durations / pulse-to-stage lookup | `build` compiles `C[8]` cumulative pulse bounds; `step(t)` finds the stage (<= 8 compares) | STAGE |
| Re-fire inside a stage / long hold (`I-`) | `step` returns `PO_HIT` on re-fire pulses; hold = `PO_HOLD`+`hold`, following pulses return no hit | STAGE |
| Ratchet with pace (accel/decel) | `PO_REP` bits + `PO_PACE_DEC`; the core schedules the repeats with SP5 | EUCLID |
| Slide (overlapping notes) | `PO_SLIDE_IN`; `AF_PAIRING` makes the core snapshot step t+1 (section 7.3) | ACID |
| Accent (two-level count, or velocity lane) | `vel` | all |
| Gate lane x global GATE | `gate` (base %); core multiplies (section 7.1) | ACID (lane), others (constant) |
| Pins | core overlay (section 9); `step` supplies `vel`/`gate` for non-hits | all |
| Absent layer | `LayerDesc.n == 0` | EUCLID SHAPE, SNAKE ARTIC |
| 6 / 4 / 4 / 5 seeds | `n_seed`, `seed_k[]`, `layer[]` | all |
| Cache sizes (ACID 54, SNAKE 55, STAGE 43, EUCLID 30 B; the ROW 2 morphs OFFSET and DIR read `t`, so ACID adds `off`, `dir`; SNAKE `off`, `dir`, `glen`; STAGE `off`, `dir`; every other ROW 2 morph is baked into the cache by `build()`) | `cache_bytes <= 64` | all |

Nothing in the four assessments needs a feature outside this table. If a future algorithm does (CC lane, per-note
clock, polyphony), it needs a new ADR (section 15).

## 5. How the global settings feed every algorithm

| Setting | Where it enters | Algorithm sees it? |
| --- | --- | --- |
| KEY, OCTAVE, SCALE | `sq_gbuild()` -> `SeqGlobal.snap[]`, `key`, `octave`; `build()` calls `sq_note()`. A change rebuilds all four channels' caches (main loop), heard at the next step | yes, via `g` only |
| GATE | core, at scheduling: `gate_eff = min(100, gate * GATE_MUL(g.gate) / 100)`, where `GATE_MUL(x) = x * 2` (a table `GATE_MUL[10]`, so GATE 50 % = x1.00 = the algorithm's own gate; 100 % = x2.00, capped). HOLD notes ignore it (owner question 11) | no |
| DIV | core: `L = DIV_TICKS[g.div]` = `{24, 32, 48, 64, 96, 192}` ticks per step. STAGE "pulse" = one DIV step | no |
| SWING | core: odd steps start later by `o = (L * (swing - 50) * 2 + 50) / 100` ticks (swing 66 = triplet feel, 75 = dotted); step lengths are unchanged | no |
| BPM / external clock | `beat_samples()` (fx.c), unchanged; external clock arrives through `events_block`'s `seq_n` | no |
| MIDI channel, mute | core, per channel | no |
| Clock out (FB-001) | `sq_block`: when `song.g[G_CLOCK] == 0` (INT) emit `0xF8` every 8 ticks of the 192-PPQN grid (24 per quarter), `0xFA` on a start from step 0, `0xFB` on a continue, `0xFC` on stop, through `midi_out_event`; when the source is USB / TRS emit nothing (it follows, never echoes) **(amended by section 19: AUTO source, only while playing as master, stamped, SPP)** | no |

Because `build` reads only `key/octave/snap`, a host test that varies gate/div/swing must see identical caches.
Settled by the owner (decisions 43 and 44): swing is doubled, and GATE 50 % is neutral; the screen labels GATE as a share of the algorithm's own gate (P2-003). Both are one table edit.

## 6. Per-channel state and the main-loop / ISR split

Authoritative, main-loop-owned state (UI writes): `SeqPreset` fields in RAM as `ChanSave sq_ch[4]`, `SeqGlobal sq_g`,
`UndoRec sq_undo[4]`, `uint8_t sq_sel` (selected channel), plus `GlobalSave`. The ISR owns `ChanRT sq_rt[4]` and the
event queue. Shared (written by main, read by ISR) is exactly this list; nothing else crosses:

| Shared item | Protocol |
| --- | --- |
| `uint32_t sq_cache[4][2][16]` (64 B per buffer, 4-byte aligned: algorithms cast it to their cache struct, and pi32v2 must not see a misaligned 16/32-bit access), `uint8_t sq_calgo[4][2]` (the algorithm id each buffer was built for), `volatile uint8_t sq_cidx[4]` | Main loop builds into `[ch][cidx^1]` and writes `sq_calgo[ch][cidx^1]`, then sets `sq_cidx[ch] ^= 1` (one byte store). The ISR takes the algorithm from `ALGOS[sq_calgo[ch][cidx]]`, **never** from `ChanSave.algo`, so a cache is always read by the algorithm that built it. The ISR reads `sq_cidx[ch]` **once per channel per tick** and finishes all `step()` calls for that tick from that buffer. The ISR never keeps a pointer across blocks. |
| `SeqGlobal sq_gdb[2]`, `volatile uint8_t sq_gidx` | Same double buffer. The ISR copies nothing; it reads `sq_gdb[sq_gidx]` once per block. |
| `ChanSave.pin_mask` / `pin_note[]` | ISR reads directly. Main writes `pin_note[i]` first, then `pin_mask` (a 16-bit aligned store is atomic). A one-step glitch on a pin edit is acceptable (no stuck note: notes are paired). |
| `midi_ch`, `muted`, `algo`, `sq_sel` | Single bytes, read once per tick. |
| Requests: `sq_req` (volatile uint8_t bit flags: START 1, STOP 2, PANIC 4, RESTART 8) and `sq_chan_panic` (bit per channel) | The ISR consumes and clears, like `transport_req` / `panic_req` in seq.c. |
| Preset load | Section 10. |

`sq_rebuild(ch)` (main loop): `build(ALGOS[a], st[a].morph, st[a].seed, &sq_gdb[gidx], &sq_cache[ch][cidx^1])`, then flip.
It is called on every morph detent, dice, undo, algorithm switch, scale change. Cost is a few hundred integer ops
(assessments 6.4), far under a 16 ms frame, so knobs are unthrottled. An `sq_epoch` byte (incremented by the ISR when
it applies a preset; the ISR does nothing else to main-loop memory) guards the one race: `sq_rebuild` records the epoch before building and, if it changed before the
flip, builds again. The `UndoRec`s are main-loop memory, so the ISR cannot clear them: the main loop calls `sq_chan_sync()`
once per frame (before the UI reads state); when `sq_epoch` differs from the last one it saw, it clears every `UndoRec`, takes
a queued load's HARMONY and BPM, and rebuilds the caches.

Control-surface API (all main-loop, no ISR calls; returns nonzero if state changed). Signatures are normative so UI
tasks can be written before the core exists:

```c
void    sq_morph_turn(uint8_t ch, uint8_t knob, int8_t detents);  /* via knob_morph[], clamps to min/max, step per detent */
void    sq_dice_layer(uint8_t ch, uint8_t layer);
void    sq_dice_all(uint8_t ch);
void    sq_undo(uint8_t ch);                                      /* undo; again = redo */
void    sq_set_algo(uint8_t ch, uint8_t algo);                    /* keeps st[]; clears UndoRec; lands at the next step */
int8_t  sq_set_midi(uint8_t ch, uint8_t midi_ch);                 /* 0..15, SQ_MIDI_OFF. MIDI channels are UNIQUE (owner ruling 2026-11-10): if another channel o holds midi_ch (not OFF), o is set to SQ_MIDI_OFF in the same call, one UndoMidi record is written, and o is returned (-1 = none displaced) so the UI can toast it */
void    sq_set_mute(uint8_t ch, uint8_t on);
void    sq_select(uint8_t ch);
void    sq_pin_set(uint8_t ch, uint8_t pos, uint8_t semi);        /* pos 0..15, semi 0..96 */
void    sq_pin_clear(uint8_t ch, uint8_t pos);
void    sq_pin_clear_all(uint8_t ch);                             /* undoable (UndoRec.what bit1) */
uint8_t    sq_reset_morph(uint8_t ch, uint8_t knob);                 /* P1-014: one morph to MorphDesc.def; undoable (what bit2). P4-018: the four sq_reset_* return 1 when something changed (one record), 0 when all was default (no record) */
uint8_t    sq_reset_row(uint8_t ch, uint8_t row);                    /* row 0/1: its four morphs to defaults; undoable */
uint8_t    sq_reset_algo(uint8_t ch);                                /* all 8 morphs + all seeds to factory (seed_k); undoable */
uint8_t    sq_reset_layer(uint8_t ch, uint8_t layer);                /* that layer's seeds to factory; undoable */
void    sq_rec_arm(uint8_t on);                                   /* REC (P1-014 revision): armed = keys write pins; cursor 0 */
void    sq_rec_write(uint8_t ch, uint8_t key, uint8_t vel);       /* a REC key: pin at the cursor (stopped) or nearest the playhead (playing), ACC / SLIDE from K2 / K3 */
void    sq_pin_pitch(uint8_t ch, uint8_t pos, uint8_t degree);    /* held step + another white key: that scale degree, auditioned */
void    sq_perf(uint8_t bits);                                    /* SOLO 1, ROLL 2 (ISR flags); FILL = main-loop morph[0] max with restore */
int     sq_scene(uint8_t n, uint8_t store);                       /* scene A/B: store = copy to pool; else queue the section 10 load (returns 0 if empty) */
void    sq_global_set(const GlobalSave *g);                       /* recomputes snap[], rebuilds all caches */
void    sq_restart(void);                                         /* F#5 RESTRT (P1-014: was D#5): all channels to step 0 at the next step */
void    sq_live_on(uint8_t note, uint8_t vel);                    /* a key on the selected channel; snapped; ISR-safe */
void    sq_live_off(uint8_t note);
int     sq_preset_save(SeqPreset *out, ModSave *ms);              /* ms: SQ_NCH entries (the 100 B sibling array) or NULL to skip */
int     sq_preset_load(const SeqPreset *in, const ModSave *ms, uint8_t when);   /* ms NULL keeps the live s_mod; when: 0 now (stopped), 1 at the next bar */
```

## 7. Timing and the scheduler

### 7.1 Why 192 PPQN

192 is the T-1 resolution and `t1_block()` already derives it from `beat_samples()`: `acc += n * 192; while (acc >= bs)
{ acc -= bs; tick(); }`, which follows the internal BPM and an external MIDI clock (`events_block`'s `seq_n`). It is
kept because: (a) every v1 DIV is a whole number of ticks (`DIV_TICKS` above: no rounding drift); (b) it resolves swing
to 1/24 of a 1/32 step and a ratchet of 3 inside the shortest step (24 ticks -> 8-tick spacing); (c) the slide overlap
`D = max(1, L/8)` ticks is 3 ticks at 1/32 and 6 at 1/16; (d) the control block is `CTL` = 32 samples = 0.73 ms, and a
tick is 2.6 ms at 120 BPM and 1.0 ms at 300 BPM, so never more than two ticks run per block. Timing jitter is therefore
at most one block (0.73 ms), below USB-MIDI's own 1 ms frame. **[Superseded by section 19: that holds for the scheduling
only; on the wire the half-buffer burst and the deferred transmitter add up to 3.4 ms, and the tick grid is 400 ppm fast.]** A finer grid (sample-domain scheduling) was rejected: no
musical gain, and the integer-tick grid keeps host tests exact.

### 7.2 The per-block loop (replaces the `t1_block` call in `events_block`)

```c
static void sq_block(uint32_t n)           /* events_block: instead of the T-1 / pattern-track branch; n = seq_n */
{   if (!sq_ready) return;
    sq_entropy_ctr += n;                   /* free-running, also while stopped: the dice seed source (3.1) */
    sq_consume_requests();                 /* START: sq_all_off(); t=0, grid=tick=0, sq_acc = beat_samples() (tick 0 on the first block, as `t1_on_start` does); STOP / PANIC: sq_all_off() */
    sq_usb_edge();                         /* usb.config 0->1: panic all, so a reconnect finds nothing hanging */
    if (!song.playing || !beat_samples()) { sq_dispatch_overdue(); return; }
    sq_acc += n * SQ_PPQN;
    while (sq_acc >= beat_samples()) { sq_acc -= beat_samples(); sq_tick(sq_now++); }
}
static void sq_tick(uint32_t now)
{   const uint8_t L = DIV_TICKS[g.s.div];
    sq_apply_bar_boundary(now);            /* preset queued for the next bar: section 10 */
    for (c = 0; c < SQ_NCH; c++) if (now >= sq_rt[c].grid) sq_step_start(c, now, L);   /* all four channels, muted or not */
    sq_dispatch(now);                      /* emit due events: all NOTE_OFFs first, then NOTE_ONs */
}
```
`sq_ready` is set last in `sq_boot()` (section 12); `song.playing` is the existing transport flag, so internal PLAY,
external Start/Stop, count-in and `seq_start/seq_stop` need no change except calling `sq_on_start()/sq_on_stop()` where
they call `t1_on_start()/t1_on_stop()` today.

### 7.3 Starting a step (the one function that knows swing, gate, ratchet, slide, hold)

`sq_step_start(c, now, L)` at nominal grid tick `G = rt.grid`, step `t = rt.t`:

1. `o` = swing offset for odd steps (section 5); `S = G + ((t & 1) ? o : 0)` = actual start of step `t`;
   `S1 = G + L + (((t + 1) & 1) ? o : 0)` = actual start of step `t+1`.
2. `out = eval(c, t)` = `step()` of the active algorithm from `sq_cache[c][sq_cidx[c]]`, then the **pin overlay**
   (section 9). If the algorithm has `AF_PAIRING` and `rt.nxt_ok`, `out.flags.HIT/SLIDE_IN` and `out.note` are replaced by
   the snapshot taken one step earlier (`vel`, `gate`, `rep`, `hold` stay live: a dice of ACCNT or GATE is heard on the
   next step; a hit/note/slide change on the one after: owner question 9, ALGORITHM_SET question-9 mitigation).
3. `out1 = eval(c, t+1)`; if `AF_PAIRING`: `rt.nxt = out1; rt.nxt.pad = algo id + 1; rt.nxt_ok = 1`. In step 2 the snapshot is used only if `rt.nxt_ok && rt.nxt.pad == algo id + 1` of the cache being read, so a snapshot taken by another algorithm (algorithm switch between two steps) is ignored; no main-loop write to `ChanRT` is needed.
4. If `!(out.flags & PO_HIT)`: if `rt.open` (a previous step left the sounding note without an off, expecting a tie) emit its NOTE_OFF now (`rt.snd[0]` on `rt.snd_chan[0]`) and clear `rt.open`; `rt.slide_in = 0`; skip to step 8. (Mute is not tested here (decision 111): a muted channel resolves hits, notes and ties as usual and `sq_emit` drops only its MIDI packets at the MIDI gate, so the preview keeps playing; `t` and `grid` advance either way. Owner/T1 F10.)
5. `slideout = AF_PAIRING && (out1.flags & PO_HIT) && (out1.flags & PO_SLIDE_IN) && ((t + 1) % loop_len != 0)`.
   **P1-017 revision 1:** replaced by SEQUENCER_UX 9.2: `out1` is the resolved next step `n = (p + 1 >= len) ? 0 : p + 1` (edits included; a SLIDE edit on `p` counts as the flag), and the loop-end term is dropped, so a slide crosses from the last step into step 0. A LENGTH change inside step `p` cancels a pending pairing (the off falls back to `p`'s own gate end, or is sent at once if that has passed); STOP sends every off as before.
   `tie = rt.slide_in && rt.snd[0] && out.note == (rt.snd[0] - 1)`. If `rt.open` and not `tie` (a note change or an algorithm switch happened since the tie was announced), emit the NOTE_OFF of `rt.snd[0]` now, before step 6. `rt.open = 0` after this line.
6. **Note-on.** If `tie`: schedule nothing (the sounding note continues, `vel` ignored). Else schedule NOTE_ON(`out.note`,
   `out.vel`) at `S` (also at `S` for `rep > 1`, then SP5 repeats). If `rt.slide_in` and not tie the previous note's
   NOTE_OFF is already queued at `S + D` (step 7 of the previous step), so on-before-off holds. Same-pitch safety: if
   `out.note` is already in `rt.snd[]`, its NOTE_OFF is emitted immediately before the NOTE_ON.
7. **Note-off** (scheduled together with the on, atomically, section 8.2):
   - `slideout` and `out1.note != out.note`: NOTE_OFF at `S1 + D`, `D = max(1, L / 8)`.
   - `slideout` and `out1.note == out.note` (tie into the next step): **no** NOTE_OFF now, `rt.open = 1`; the next step schedules it (or step 4 / step 5 emits it if the tie does not happen).
   - `PO_HOLD`: NOTE_OFF at `G + hold * L - GAP` (`GAP = 2` ticks), ignoring GATE.
   - else: `len = max(1, L * gate_eff / 100)`; NOTE_OFF at `min(S + len, S1 - 1)` (strictly before the next start, so
     a non-slide pair is always off-before-on).
   - `rep > 1`: SP5 returns up to 3 `(offset, length, vel)` repeats inside `[S, S1 - 1]` (cell width `S1 - S`, which swing shortens for an odd step, so a repeat can never run into the next step); each is its own on/off pair
     (spacing at least 3 ticks, otherwise `rep` is reduced).
8. `rt.slide_in = slideout; rt.t = t + 1; rt.grid = G + L; rt.act++` (if a note was scheduled).

Ordering rule at one tick: `sq_dispatch` emits **all NOTE_OFFs due, then all NOTE_ONs due**. A slide pair never ties on
the same tick (the off is at `S + D`, `D >= 1`); a normal pair never does either (off capped at `S1 - 1`). Ratchet
repeats never overlap (length < spacing). The only legal overlap is the slide's `D`, and `ChanRT.snd[2]` has exactly
two slots for it.

**Restart (D#5) and PLAY:** `rt.t = 0`, `rt.grid = now`, `rt.nxt_ok = 0`, `rt.slide_in = 0` for all channels at the same
tick (immediately at PLAY, at the next grid tick for a restart request). The loop wrap needs no code: `t` is
free-running and `step()` reduces it.

### 7.4 Clock out (added by FB-001; master when INT, silent when following)

> **Amended by section 19 (P2-010, owner rulings 2026-11-10):** the source is AUTO, not the `G_CLOCK` setting; clock out is sent only while the FM-1 is the master and playing; every pulse is stamped to its exact sample; SPP is sent before a continue; the host tempo no longer overwrites `song.g[G_BPM]`. The bullets below are the first draft.

- 192 PPQN / 24 = one `0xF8` every 8 ticks of `sq_block`'s loop. The pulse is emitted at the **tick**, before any note scheduled on the same tick, so a receiver sees the clock first.
- `0xFA` on a start from step 0, `0xFB` on a continue (a stop followed by PLAY with the playhead kept), `0xFC` on stop. They go out only when `song.g[G_CLOCK] == 0`.
- Following (`G_CLOCK != 0`, including when the host has gone quiet) sends nothing, so a host that echoes clock cannot loop.
- The packets are 1-byte realtime messages `CIN 0xF`: `0x0F | status << 8` in `midi_out_event` (usb.c, the same ring, so a full ring drops a pulse, never a note: `sq_drops` counts it). Risk: 24 PPQN x 4 per second at 240 BPM = 96 packets per second, inside the 64-entry ring's drain rate; the hardware-test gate checks it with Ableton and the TB-3.
- Test: a host test counts 24 `0xF8` per quarter over 4 bars at 90 / 120 / 180 BPM and zero when the source is USB.
- Following overwrites the tempo: `midi_clock_pulse` writes `song.g[G_BPM]` (clamped 40..240) from the host clock (`midi_clock.c:65`), so the user's internal BPM is not preserved across an EXT session. Whether to keep a separate field is SCREEN_SPEC question 5; clock architecture and timing accuracy are P2-010.

## 8. The event path

```
main loop (UI)                         audio ISR (events_block -> sq_block -> sq_tick)             USB / synth
sq_* API, sq_rebuild --cache flip--->  sq_step_start: step() + pins -> sq_sched() into EvQ[96]
                                       sq_dispatch(now): due events -> sq_emit() ---------------> midi_out_event(pkt)  [usb.c ring]
                                                                       \-------(prv_on[chan] only)------> trk_note_on/off(&trk[chan-1], ...)  [ANALOG]
keyboard_block (white keys) --> sq_live_on/off: sq_emit(now), no queue
```

### 8.1 `sq_emit(chan_byte, kind, note, vel)` (the only exit)

- MIDI: unless `chan_byte & 0x10` (MIDI OFF), build the packet exactly as T-1 did:
  NOTE_ON `0x09 | (0x90 | m) << 8 | note << 16 | vel << 24`; NOTE_OFF `0x08 | (0x80 | m) << 8 | note << 16`;
  CC123 `0x0B | (0xB0 | m) << 8 | 123 << 16`; call `midi_out_event(pkt)`.
- Preview (amended by P2-024 decision 110 and P2-025 decision 111): if the channel's `prv_on` is set, **independently of the MIDI gate**
  (MIDI OFF and mute close only the MIDI packet above; the preview is a CUE), `trk_note_on(&trk[chan-1], note, vel)` / `trk_note_off(&trk[chan-1], note)`. Each channel has its own
  part with one of six ANALOG patches (PREVIEW.md); all four are OFF at boot. Changing `prv_snd` or `prv_on` first releases that
  channel's `snd[]` notes on its part (tracked by `ChanRT.snd`); selecting another channel releases nothing.
- A NOTE_OFF is sent only if its note is in `rt.snd[]` (otherwise its NOTE_ON was dropped by the reserve rule of 8.3 and the off is discarded), so the stream never carries an off without an on.
- Bookkeeping: NOTE_ON stores `note + 1` in a free `rt.snd[]` slot (if both are full, the older note gets a NOTE_OFF first);
  NOTE_OFF clears the matching slot. `rt.act` is bumped on every NOTE_ON.
- Velocity 0 is never sent as a NOTE_ON (clamped to 1).

### 8.2 The event queue (`SeqEvent evq[96]`, replaces `T1MicroQueue` 256 x 16 B and both `MidiRingBuffer`s)

- An unsorted slot array: `sq_dispatch(now)` makes two passes over 96 slots (offs, then ons), emits those with
  `tick <= now`, clears them. 192 compares per tick; no sort, no head/tail. Overdue events (after a stall) fire at once.
- `sq_sched_pair(chan, on_tick, off_tick, note, vel)` inserts the ON and the OFF **together** (needs two free slots);
  if the queue is full the whole pair is dropped and `sq_drops++` (a note is lost, never a hung note). A note-off whose
  on has been emitted is **never** dropped. A NOTE_OFF-only insert (slide continuation) uses one slot and, if none is free,
  evicts the earliest NOTE_ON that is still in the queue.
- 96 slots cover the worst case: 4 channels x (one ratchet of 3 = 6 events) = 24 new events per tick, plus pending offs.

### 8.3 The USB-MIDI ring (`usb.c`, unchanged semantics, two prerequisite edits)

`midi_out_event` drops when no host is configured or the ring is full. v1 needs (prerequisite task 5): a separate
`MQO = 128` for the **out** ring (today `MQ = 64` is shared with the in ring: +256 B), and `midi_out_free()` returning
free slots. `sq_emit` keeps 8 slots in reserve: if fewer than 8 are free it **drops NOTE_ONs** (counted) but still sends
NOTE_OFFs; a NOTE_OFF that cannot be sent stays in the queue and is retried next block.

### 8.4 Note-off guarantees

| Situation | Rule |
| --- | --- |
| Every sounding note | Its NOTE_OFF was queued with its NOTE_ON (8.2), carries its own MIDI channel, and is retried until sent. |
| STOP (button, external Stop), PANIC, preset undo to stopped | `sq_all_off()`: for each channel NOTE_OFF for each `rt.snd[]` entry on the channel byte it was sent with, release the preview notes, then CC123 on each MIDI channel in use (all 16 if unsure is not done; the 4 configured plus the channels of any `snd_chan`), clear `evq`, reset `rt`. Done in one block; if the ring is short it continues next block (`sq_req` PANIC stays set until clean). Same order as `t1_all_off()` in `t1_glue.c` (note-offs first, then CC123). |
| PLAY | `sq_all_off()` first (the T-1 did the same in `t1_on_start`). |
| USB host (re)connects or disconnects | PANIC on the 0->1 edge of `usb.config`. |
| MIDI-channel change, algorithm switch, preset load, mute | **No cut.** Queued events keep their own MIDI channel, so the old note ends on the old channel. (This replaces the "cut on algorithm / channel change" line in sting 6.3a item 3; owner decision 47.) |
| Mute | The MIDI gate in `sq_emit` closes at once (decision 111): the channel keeps computing its steps and feeds its preview; sounding MIDI notes end by their queued offs (no click, no hang). |

## 9. Step entry: pins on the 16 white keys, and `K1 = 0` (KEYS ONLY)

> P1-017: superseded by the edit layer of SEQUENCER_UX 1 and 9.2 (pins become `StepEdit` with `ON` set; `K1 = 0` still means KEYS ONLY). Kept below as history.

Pins are core data, not algorithm data (SP8): `ChanSave.pin_mask`, `pin_note[16]`, per channel, kept across algorithm
switches. A pin at position `pos` (0..15) overlays whatever the algorithm produced at the steps whose
`t % loop_len == pos`:

```c
static void sq_eval(uint8_t c, uint32_t t, PulseOut *o)
{   uint8_t i = sq_cidx[c];  const AlgoDesc *a = &ALGOS[sq_calgo[c][i]];  const void *k = sq_cache[c][i];     /* one read of cidx */
    a->step(k, t, o);
    uint8_t pos = t % a->loop_len(k);                       /* pins sit on the visible loop (STAGE: first 16 pulses) */
    if (pos < SQ_NPIN && (sq_ch[c].pin_mask >> pos & 1u)) {
        o->note = sq_note(g, sq_ch[c].pin_note[pos]);       /* SP6 snap, key, octave: a scale change re-snaps it */
        o->flags = (o->flags | PO_HIT | PO_PIN) & ~(PO_HOLD | PO_REP_MASK | PO_PACE_DEC);
    } }
```
(The pin keeps `vel`/`gate` from `step()`; EUCLID returns 80 for non-hits, so its pins are 80, no ratchet.) A pin at or
past `loop_len` is silent until the loop grows (owner questions 10/18). Pins never move a generator's state (they are
applied after `step()`), so morphs and dice never disturb them.

- **Enter** (a white-key press; P1-014 removed STEP mode, CONTROL_MAP W4): `sq_pin_set(ch, pos, semi)`. The UI chooses `semi`: the note generated at that
  position (`step(pos)` converted back with `semi = note - 36 - key - 12 * octave`, clamped 0..96) or 0 (the root) when
  that step is silent. `pin_note` writes before `pin_mask`.
- **Edit (P1-014):** hold a step key; K1 `NOTE` = one scale degree per detent (the UI walks the scale table), OCT+ then K4 `OCT` = +-12 per detent; both clamp 0..96.
- **Clear:** `sq_pin_clear` removes one; `sq_pin_clear_all` is recorded in `UndoRec.what bit1`, so an UNDO (EDIT) tap undoes it (P1-014 revision: UNDO stays on EDIT).
- **K1 = 0:** `m[0] == 0` means the algorithm's `build()` produces no `PO_HIT`; pins still play because the overlay
  forces `PO_HIT`. No fifth algorithm (owner decision 35). Because `step()` still fills `vel`/`gate`, pins sound at the
  right dynamics.
- **Live keys** (PERF): `keyboard_block` calls `sq_live_on(note, vel)`; the note is snapped by `sq_snap_abs`, sent on the
  **selected** channel's MIDI channel (unless OFF) and, when that channel's PREVIEW is on, its preview part. It bypasses the queue; the key-up sends the
  paired NOTE_OFF (the key table remembers the exact `chan_byte` and note sent). MIDI IN notes use the same two calls.

## 10. Presets, queued loads, undo of a load

- A preset is a `SeqPreset` (420 B). `sq_preset_save()` copies `sq_ch`, `sq_g.s`, BPM. Slot storage (P01..P16 = 6720 B,
  the autosave, the "undo last load" copy) is E7 (project.c becomes the store); this ADR fixes only the byte layout.
  `sq_preset_load()` **validates and clamps every field** (algo < 4, midi_ch <= 16 and unique: if two channels hold the same non-OFF value the lower-numbered keeps it and the others load as OFF, morph within `MorphDesc` min/max,
  pin_note <= 96, scale < 16, ...); a wrong `version` or an out-of-range algorithm id **rejects the load**: `sq_preset_load` returns -1
  and the live state is untouched (loading defaults over the live pattern because a slot is corrupt would destroy the player's
  work; a caller that wants defaults loads `sq_defaults()` through the same path). What an empty or corrupt slot shows is
  decided by the storage layer (P7-002), which also checks the CRC before calling `sq_preset_load` (the core does not).
- **Queued to the next loop start (owner decision 34):** "loop start" = the next tick where the global bar counter
  `sq_bar` (192-PPQN ticks since PLAY / restart) reaches a multiple of **768 ticks** (always one 4/4 bar, whatever DIV
  and the loop lengths are): data constant `SQ_BAR_TICKS = 768` (owner decision 45, replacing the earlier
  `SQ_BAR_STEPS = 16`). Procedure: the main loop validates the preset into a pool staging copy,
  builds the four caches into the **inactive** buffers `sq_cache[c][cidx^1]` and `sq_gdb[gidx^1]`, then sets
  `sq_load_pending = 1` (last store). At the bar tick `sq_apply_bar_boundary()` (ISR) copies the staged `ChanSave`s and
  `GlobalSave` over the live ones (<= 400 B), flips every `cidx` and `gidx`, increments
  `sq_epoch` (the main loop's `sq_chan_sync()` then clears every `UndoRec`, section 6), clears `sq_load_pending`. `rt.t/grid` are untouched (the music does not jump). Stopped: applied at once
  with the ISR guard `fm1_irq_off()/fm1_irq_on()` (the pattern of `t1u_temp` in ui_t1.c).
  **P1-017 revision 2 (pending slot, SEQUENCER_UX 4.1):** the main loop does none of the above until the pending slot's
  deadline (last PRESETS detent + `SQ_LOAD_SETTLE` 1.5 s, moved on by every detent) has passed; only then does it
  stage and set `sq_load_pending`, so the ISR applies the load on the first 768-tick bar that starts at or after the
  deadline (stopped: applied at the deadline). A SAVE tap before the load lands saves into the pending slot and
  clears `sq_load_pending` under the ISR guard. No ISR change.
- **SAVE hold 0.7 s** restores the copy of the previous state saved at load time (kept in the pool); it goes through the
  same queued path; again = redo.

## 11. Memory budget

Facts (SYSTEM_MAP section 9): RAM `.data+.bss` after the E3 removals 27232 B of 73728 = **46496 B headroom** (today
17984 B); floor 4096 B; E9 trigger 8192 B. Pool after removals 150665 B of 368640 (**217975 B** free; today 25764 B).

| Item | Per channel | x4 / total | Where |
| --- | ---: | ---: | --- |
| `ChanSave` (live) | 102 | 408 | .bss |
| `UndoRec` | 48 | 192 | .bss (P1-014: was 34 / 136) |
| `ChanRT` | 24 | 96 | .bss |
| Algorithm caches, double buffered (2 x 64) | 128 | 512 | .bss |
| `SeqGlobal` x2 (double buffer) + `cidx[4]` + `gidx` | | 41 | .bss |
| Event queue `evq[96]` (96 x 8) | | 768 | .bss |
| Transport / clock / requests / counters (`sq_acc`, `sq_now`, `sq_bar`, `sq_entropy_ctr`, flags, `sq_drops`, `sq_epoch`, `sq_sel`) | | 32 | .bss |
| `sq_calgo[4][2]` | | 8 | .bss |
| USB-MIDI out ring 64 -> 128 packets (separate `MQO`) | | +256 | .bss |
| **Core total (.bss)** | **288** | **2257** | |
| UI state allowance for P2-003 (toasts, knob highlight timers, layer state, redraw signatures) | | 3072 | .bss (cap; P2-003 must fit or amend) |
| **Total new .bss** | | **5329** | |
| Preset slots P01..P16 (16 x 420) | | 6720 | .pool / `.noinit` (replaces `proj_slot` 15360 B; E7) |
| Staging preset + "undo last load" copy | | 840 | .pool |
| **Total new pool** | | **7560** | |

| Check | Allowed | Used | Result |
| --- | ---: | ---: | --- |
| `.bss`, after E3 removals: 46496 - 4096 | 42400 | 5329 | fits, headroom left 41167 (> 8 KB trigger) |
| `.bss`, if built before the removals: 17984 - 4096 | 13888 | 5329 | fits, headroom left 12655 (> 8 KB trigger) |
| Pool, today | 25764 | 7560 | fits |
| Stack | n/a | `build` <= 256 B (main loop), `step` <= 64 B (ISR) | each dev task measures |

### 11.1 Re-added for P1-014 (performance-first; R1-R4 reserved)

| Item | B (.bss) | Where |
| --- | ---: | --- |
| Section 11 total new .bss (P1-013 version) | 5329 | |
| `UndoRec` 34 to 48 B, x4 | 56 | .bss |
| MUTATE snapshot `sq_mut` | 0 | removed in the revision (was 24) |
| REC state (armed, cursor, ACC / SLIDE, REC octave) 4 + sounding-note table 27 | 31 | .bss |
| held-step pitch gesture (step index, pressed bit) | 2 | .bss |
| `sq_perf`, saved FILL morph, scene valid / pending bits, `view_page` | 8 | .bss |
| Reserved for P1-016: step records 64 x 3 B x 4 (live) | 768 | .bss (replaces `pin_note` / `pin_mask`, 72 B) |
| Reserved for P1-016: step-record undo copy 64 x 3 B x 4 | 768 | .bss |
| **Total new .bss** | **6962** | 5329 + 56 + 0 + 8 + 31 + 2 + 768 + 768 |
| Pool: 16 presets + staging + undo-load copy + scenes A, B (about 1120 B each at the upper bound = 20 copies) | about 22400 | .pool |

Checks (SYSTEM_MAP section 9: headroom 46496 B after the removals, 17984 B before; floor 4096; E9 trigger 8192): after the removals 46496 - 6962 = **39534 B** left; before them 17984 - 6962 = **11022 B** left (above the 8 KB trigger, 6926 B above the 4096 B floor). Pool today 25764 - 22400 = **3364 B** left, so the removals (R6) must precede preset storage (E7), or P1-016 must shrink the step record; after the removals 217975 - 22400 = 195575 B. Scenes are volatile (owner question 58).

### 11.2 Re-added for P1-017 (edit layer, pages, force sensing; supersedes the 11.1 reserve)

| Item | B (.bss) |
| --- | ---: |
| Section 11 total new .bss (P1-013 version: `ChanSave` 4 x 102, `UndoRec` 4 x 34) | 5329 |
| `ChanSave` 102 -> 344 B, x4 | +968 |
| `UndoRec` 34 -> 92 B, x4 | +232 |
| REC state + sounding-note table (P1-014) | +31 |
| NOTE_EDIT / page UI state (edited step, open flag, `view_page[4]`, FOLLOW, ADD prompt, open page, HELP control) | +10 |
| `sq_perf`, saved FILL morph, FILL / ROLL force levels | +4 |
| Pending preset slot + settle deadline (revision 1) | +3 |
| Force sensing: 20 keys x 8 B, 20 floors x 2 B, 20 H3 offsets x 2 B (revision 1), MASTER cache 2 B | +242 |
| 11.1 reserve (768 + 768), scenes, MUTATE, held-step pitch | 0 (dropped) |
| **Total new .bss** | **6819** |

Checks: after the E3 removals 46496 - 6819 = **39677 B** headroom; built before them 17984 - 6819 = **11165 B**
(2973 B above the 8192 B E9 trigger, 7069 B above the 4096 B floor). Pool: 16 slots x 1388 + staging + undo-load copy
(2 x 1388) = **24984 B**; today's 25764 B free would leave 780 B, so **the R6 fx-bus removal (SYSTEM_MAP section 6)
is a prerequisite of preset storage (E7)**; until then `SQ_NSLOT = 8` (13880 B). After the removals 192991 B stay free.
Static-assert plan: `StepEdit` 4, `ChanSave` 344, `UndoRec` 92, `SeqPreset` 1388, `ChanRT` 24, and
`sizeof(((SeqPreset*)0)->ch) == 1376` in `sq_core.c` (replaces the 408 assert). CPU: `sq_eval` adds about 30 integer
operations per step per channel; force sensing runs in the scan ISR (estimate 1-2 %, gate limit 3 %), none in the
audio ISR.

Removed with R8 (T-1 retirement): `t1_micro_queue` 4096 B and the two `MidiRingBuffer` 2 x 264 B and `t1_sequencer_ram`
9348 B + the T-1 `.pool` buffers (13980 B total per SYSTEM_MAP; all in the pool, `.bss` impact 36 B). CPU: per tick
`sq_tick` does 4 x (2 `step()` + <= 8 queue inserts) plus 192 slot compares, a few thousand integer ops, at most twice per
block; the first core task records `felucca_dbg.max_us` before and after (budget: no more than +5 % of the 725 us block).

## 12. Boot sequence and power-on defaults (owner decision 32b)

The device boots straight into the sequencer: no mode, no `PREF_T1`, no `MODE` row, no groovebox route. In `main.c`
`fm1_main()`, in this order (changes marked):

1. `persist_boot`, `settings_init`, `usb_serial_apply`, `lcd_init`, `draw_splash`, input/ADC/`panel_init` (unchanged).
2. `felucca_init()` **reduced**: only the four preview parts, `trk[0..3]` = the six-patch table of PREVIEW.md (all PREVIEW OFF; CH1 ACID, CH2 BASS, CH3 LEAD, CH4 PAD). Groovebox track defaults
   and `load_pat16` are gone (E3).
3. `master_boot()` (unchanged).
4. **`sq_boot()` replaces `t1_glue_init()`**: fill `sq_ch[]`, `sq_g` from `SQ_DEFAULTS` (below); `sq_gbuild()`;
   `sq_rebuild(c)` for all four channels into buffer 0; `cidx = gidx = 0`; zero `ChanRT`/`evq`; **last**: `sq_ready = 1`.
5. `audio_init()` (the ISR may now call `sq_block`; before step 4 completes it is inert because `sq_ready == 0`), `usb_start`,
   `uart_midi_init`, `boot_leds`, `timer5_start`, `fm1_irq_enable_all` (unchanged).
6. `autosave_boot(boot_clean)`: if the last shutdown was clean and an autosave `SeqPreset` validates, `sq_preset_load(...,0)`
   (stopped: applied at once; a rejected load returns -1 and the step-4 defaults stand, section 10). Otherwise the defaults from step 4 stand. After a crash the defaults stand.
7. The 400 ms splash, then the main (PERF) screen: channel 1 selected, stopped, no menu. Press PLAY: CH1 plays.

`SQ_DEFAULTS` (a const table; CONTROL_MAP section 0, UX_REVIEW section 2):

| Item | Value |
| --- | --- |
| CH1 | ACID, MIDI channel **1**, unmuted (P1-017 owner ruling 73: channel numbers match MIDI channels; the first-run step 1 `D#4 K2: MIDI CHANNEL` prompts for the TB-3's channel 2) |
| CH2 / CH3 / CH4 | EUCLID / SNAKE / STAGE, all muted, **MIDI OFF** (P4-044, BUG-011: a channel out of use holds no MIDI channel; ADD starts at OFF with the setup cue; change: D#4, K2) |
| every `AlgoState` | morphs = `MorphDesc.def`; seeds per section 3.1 for each channel and algorithm |
| pins | none (P1-017: every `StepEdit` AUTO, every `pg_src` 0, `len` 16) |
| Global | KEY C (0), SCALE MINPEN (10), OCTAVE 0, GATE 50, DIV 1/16 (2), SWING 50, BPM 120 |
| In use | **CH1 only** (P4-044): ChanSave.pad[1] bit 0 = channel in use, core state `sq_used`; CH2..CH4 hold their defaults but are out of use until added, and can be added only when their algorithm row exists (`ALGOS[].build`). No layout change (pad[1] was reserved) |
| UI state | selected channel CH1, preset P01 label, keyboard OCT 0 |
| Preview | all four channels OFF; sounds CH1 ACID, CH2 BASS, CH3 LEAD, CH4 PAD (decision 110) |
| Transport | stopped; `sq_ready` set before the audio ISR matters |

Autosave writing, the throttle and the preset store are E7; this ADR requires only that the load path above is the same
`sq_preset_load` used by the PRESETS encoder.

## 13. What happens to the T-1 code (cite functions)

| Code | Verdict | Reason |
| --- | --- | --- |
| `t1_euclidean_bjorklund`, `t1_euclidean_rotate` (torso_t1_algo.c) | **Reuse unchanged** (SP11), called from `sq_algo_euclid.c` `build` | verified; cap `groups[]` at 16 for stack |
| `t1_pace_curve_q15`, `t1_pace_eval_ratchet` | **Reuse the maths**, new scalar wrapper `sq_ratchet(rep, pace, L)` (SP5; today it takes `CycleParams`) | F13: use the code's behaviour |
| `T1_SCALE_MASKS` | **Reuse** as the one scale table; delete stale `T1_SCALE_NAMES` (F2) | F1 fixed |
| `t1_harmony_quantize` | **Replace** by `sq_gbuild()` + `sq_note()` (SP6: nearest, ties to lower; the old one snapped down) | changes T-1 output; question 16 |
| `t1_harmony_eval_phrase` | **Reuse after F3 is fixed** (EUCLID contour, SP10) | F3 OPEN: dev prerequisite |
| `t1_harmony_eval_style/_chord/_voicing`, `t1_lfsr_*`, `t1_plock_*`, `t1_euclidean_eval_cycle` | **Drop** (delete at R8) | arp/chords/LFSR/p-locks are out of v1; SP1 replaces the LFSR (F16, F19) |
| `t1_pipeline_tick`, `t1_pipeline_alloc_voice`, `_free_voice`, `_kill_all_voices`, `_invalidate_masks`, `_reset_transport`, `t1_division_ticks`, `t1_calc_swung_len` | **Rewrite** as `sq_tick/sq_step_start/sq_all_off`; the `DIV_TICKS` table replaces `t1_division_ticks`; swing is redefined (section 5) | the 6-track evaluator is the thing being replaced (F9, F10, F14) |
| `t1_micro_queue_*`, `T1MicroEvent`, `t1_midi_worker_*`, `MidiRingBuffer`, `midi_ring_*`, `t1_timer_*`, `t1_task_table`, FreeRTOS priority comments in torso_t1.h | **Drop** | replaced by `evq[96]` and `sq_emit`; the TRS jack is MIDI IN only, so the UART ring is dead; there is no timer ISR (the clock is `events_block`) |
| `Pattern`, `Track`, `CycleParams`, `SequencerRAM` | **Drop** | replaced by `SeqPreset` (420 B vs 9348 B) |
| `t1_block` (t1_glue.c) | **Rewrite** as `sq_block`; keep its accumulator idea | |
| `t1_all_off`, `t1_on_start`, `t1_on_stop` | **Rewrite** as `sq_all_off`, `sq_on_start`, `sq_on_stop`; keep the order note-offs then CC123 | model for 8.4 |
| `t1_default_pattern`, `t1_glue_init`, `t1_set_mode` | **Drop / replace** by `SQ_DEFAULTS` and `sq_boot` | decision 32b: always on |
| `synth_note_on/off` (voice allocator callbacks) | **Drop**; preview calls `trk_note_on/off(&trk[chan-1],...)` directly | |
| `torso_t1_state.c` (TEMP, GLOBAL, flash banks), `torso_t1_ui.c`, `ui_t1.c` | **Drop** at R8; screens are P2-003 | |
| `t1_mode`/`t1_off_req` stubs in seq.c/ui.c | **Drop** (the sequencer is permanent); until prerequisite task 1 lands, the same `#ifndef T1_DEVICE` stub pattern guards `sq_block` etc. | |

Both stay in the tree and build until R8, so the new core can be developed and host-tested next to the old one; the
cut-over (one line in `events_block`, one in `main.c`) is the R8 task.

## 14. Host-test strategy

- **Layout of the sources** (unity build, include order after `torso_t1_algo.c`, before `audio.c`):
  `sq_types.h` -> `sq_prims.c` (SP1 xorshift, SP2 rank-order, SP3 accent count, SP4 velocity lane, SP5 ratchet, SP6/SP7/SP9
  `sq_gbuild`/`sq_note`, SP11 wrappers) -> `sq_algo_acid.c`, `sq_algo_euclid.c`, `sq_algo_snake.c`, `sq_algo_stage.c`
  -> `sq_algos.c` (the `ALGOS[]` table) -> `sq_core.c` (scheduler, API; the only file that touches `midi_out_event`,
  `trk_*`, `beat_samples`). `seq.c` is included before these, so it forward-declares `sq_block`, `sq_on_start`, `sq_on_stop`,
  `sq_live_on`, `sq_live_off` exactly as it does for `t1_block` today (seq.c lines 61-64), with `#ifndef T1_DEVICE` no-op stubs for the host tests. Everything in `sq_prims.c` and `sq_algo_*.c` is hosted-includes-free (`<stdint.h>` only) so the
  same files compile with plain `cc` in `tests/`.
- **Algorithm golden vectors (P2-008 produces, dev tasks reproduce).** P2-008 writes each prototype as the *same two
  functions* (`build`, `step`) against `sq_types.h` and `sq_prims.h`, in `docs/design/algorithms/<name>/`. Golden file
  per algorithm: for a fixed matrix of (channel 0..1, default morphs and a few edited morphs, default seeds and two diced
  seeds, scale MINPEN and CHROM, key 0 and 5, octave 0 and -1) dump `t note vel gate flags hold` for `t = 0..127`, as text.
  The dev task's `tests/sq_algo_<name>_test.c` includes the firmware file and must reproduce the golden text byte for
  byte. The P2-008 prototype is therefore the firmware file (a dev task only renames it, adds `static`, and
  wires the table row): no re-implementation, no drift. Note the SP6/SP7 "late quantise" rule changes the informative
  check lines printed in the assessments (they were produced before SP6); P2-008's output supersedes them.
- **Purity tests (one generic test, all four algorithms):** (a) call `build` twice -> identical caches; (b) vary `gate`, `div`,
  `swing` -> identical caches; (c) poison the cache's unused bytes -> identical `step()` output; (d) `step()` called for
  `t` in any order -> same results; (e) every `PulseOut` has `vel` 1..127, `gate` 1..100.
- **Table tests:** every `MorphDesc` has `min <= def <= max`; `knob_morph` is a permutation; every non-absent
  `LayerDesc` seed id `< n_seed`, every seed is bound by exactly one layer; `cache_bytes <= 64`; sub-labels <= 6 chars;
  default seeds equal the assessments' formula for ch 0..3.
- **Event-level golden (scheduler):** `sq_core.c` exposes `sq_tick(now)` with `sq_emit` replaced by a host hook that
  appends packets to a log. Golden: (1) the 8-step ACID slide worked example of sting 6.3a (exact ticks and the
  12 events) - this is the slide/overlap/tie acceptance test; (2) swing 66 % at 1/16 (odd note-ons 15 ticks late);
  (3) ratchet inside 1/32; (4) STAGE hold of 8 pulses with GATE changed; (5) mute keeps phase; (6) STOP mid-note ->
  note-off + CC123 for every sounding note; (7) queue full -> no unmatched NOTE_ON; (8) MIDI-channel change mid-note ->
  the NOTE_OFF goes to the old channel; (9) preset queued -> applied only at a 16-step boundary.
- **Invariant fuzz:** random morph/dice/undo/mute/algorithm/ MIDI-channel/ DIV/swing/restart/stop sequences for 100 000
  ticks under ASan/UBSan: `#NOTE_ON == #NOTE_OFF` per (MIDI channel, note) after a final STOP, never two NOTE_ONs of
  one note without an OFF between them, no OFF without an ON.
- Hardware (owner TB-3 checklist, P2-005): the overlap-glide behaviour (question 8) and tick-to-ear latency.

## 15. Flex points (ADR-0004 addendum)

Changeable by editing a table row, no algorithm or core code change:

| Table | Change |
| --- | --- |
| `AlgoDesc.knob_morph[8]` | any knob slot (K1..K4, S1..S4) to any morph of that algorithm (swap two numbers) |
| `AlgoDesc.morph[8]` | label (<= 6 chars), min, max, default, slow step, fast step |
| `AlgoDesc.layer[5]` | which seed(s) a dice button re-rolls; move a seed to another layer; bind two seeds to one layer; mark a layer absent |
| `AlgoDesc.sub[5]` | dice sub-labels |
| `AlgoDesc.seed_k[]`, `SQ_DEFAULTS` | default seeds' k (also the RESET factory seeds, P1-014), boot algorithm/MIDI/mute per channel, global defaults |
| `DIV_TICKS[6]`, `GATE_MUL[10]`, `SQ_BAR_TICKS` (768), swing formula constants, `D` (L/8), `GAP` | feel constants |
| `CLK_TIMEOUT_MS` (500), the clock-out pulse spacing (8 ticks), clock-out on / off by source | tempo-source and clock-out constants (FB-001) |
| Name order of `ALGOS[]` (`EN_ALGO` cycle order) | reorder |
| The 40 ms fast-turn threshold | UI table (P2-003), not here (the fast step itself is `MorphDesc.fast`) |

Cannot change without code (and an ADR-0003 amendment, an owner re-lock, and golden regeneration):

| Item | Why |
| --- | --- |
| `SQ_NCH`, `SQ_NALGO`, `SQ_NMORPH` (8), `SQ_NSEED`, `SQ_NLAYER`, `SQ_CACHE_MAX`, struct layouts | the stored preset format and static asserts |
| The xorshift generator, salts, shuffle method | define the golden files (SP1) |
| 16 pin positions; pins keyed by `t % loop_len` | step-entry geometry |
| Purity contract and `PulseOut` fields | adding a CC lane, polyphony or per-note clock needs a new field |
| 192 PPQN grid and the on/off pairing rule | scheduler correctness |
| Slide as overlap (not CC) | question 8 decides after the TB-3 test; CC65 would need a CC event kind |

## 16. Prerequisites for the first dev tasks (P2-005 turns these into tasks, in this order)

Each task cites the section named. None may start before the owner approves this ADR and ADR-0004's "Design locked".

1. **Host tests link the real sequencer sources** (owner decision 42): make `tests/` build `seq.c`, `ui*.c` without the
   `T1_DEVICE` stubs by supplying host hooks for `sq_block`, `sq_on_start/stop`. Section 13 last row, 14.
2. **`sq_types.h`**: constants, all structs, every `_Static_assert` (section 3). No behaviour. Host compile test.
3. **`sq_prims.c` with golden tests**: SP1 xorshift (+ salts as macros), SP2, SP3, SP4, SP5 `sq_ratchet`, `sq_gbuild`/`sq_note`/
   `sq_snap_abs` (SP6/SP7/SP9), delete `T1_SCALE_NAMES` (F2). Section 4, 5, 14.
4. **Fix F3** in `t1_harmony_eval_phrase` (ARCH, INV A) with the suggested test (EUCLID dependency). T1_ISSUES F3.
5. **`usb.c`**: `MQO = 128` out ring and `midi_out_free()`; host test of the reserve rule. Section 8.3.
6. **`sq_core.c` scheduler** with the event-level golden tests of section 14 (queue, `sq_emit`, `sq_sched_pair`,
   `sq_dispatch` two-pass ordering, `sq_step_start`, `sq_all_off`) against a **mock algorithm table**. Sections 7, 8.
7. **Channel state API** (`sq_morph_turn`, `sq_dice_*`, `sq_undo`, `sq_pin_*`, `sq_set_*`, `sq_rebuild` with the cache flip and
   epoch guard), `SQ_DEFAULTS`, `sq_preset_save/load` with validation, host tests including the invariant fuzz. Sections 3, 6, 9, 10.
8. **Algorithm tasks, in ALGORITHM_SET section 7 order**, each = pure file + `ALGOS[]` row + golden reproduction + purity/table
   tests: 8a ACID (first: validates layers, undo, slide), 8b EUCLID, 8c SNAKE, 8d STAGE. Section 4, 14. Needs P2-008 goldens.
9. **Cut-over R8**: `events_block` calls `sq_block`; `keyboard_block` and MIDI IN call `sq_live_*`; `main.c` calls `sq_boot`;
   delete the T-1 pipeline/UI/state files and the `t1_mode` plumbing; build line shows RAM headroom. Sections 8, 12, 13.
10. **UI tasks** (depend on P2-003 screen spec, CONTROL_MAP, this API): they only call the section 6 API and read `sq_ch`,
    `sq_rt[].t`, `sq_rt[].act`, `ALGOS[]` tables. No UI task touches the ISR side.
11. **Preset store (E7)**: slots, autosave, load-undo, using the `SeqPreset` layout. Section 10.
12. **Hardware-test tasks** (owner TB-3 checklist: steps, notes, slides, accents, morph, dice). Section 14 last bullet.

## 17. Deviations from earlier documents (all flagged to the owner)

| # | Earlier text | This ADR | Owner decision |
| --- | --- | --- | --- |
| a | ALGORITHM_SET 4.3 per-seed previous seed, undo swaps all previous seeds | One `UndoRec` per channel (last dice / dice-all / pin-clear) | 46 |
| b | sting 6.3a rule 3: algorithm or MIDI-channel change cuts notes | No cut; events carry their MIDI channel; only stop/panic/disconnect cut | 47 |
| c | ACID "GATE multiplies"; control map default GATE 50 % | `GATE_MUL(x) = x * 2` (percent; 50 % = x1.00): 50 % = the algorithm's own gate | 44 |
| d | T-1 swing: odd steps shifted by `(swing - 50)` % of a step | pair-ratio swing, `2 * (swing - 50)` % of a step | 43 |
| e | "preset load at the next loop start" (loop undefined with 4 different loop lengths) | a fixed bar of 768 ticks, independent of DIV | 45 |
| f | SNAKE/EUCLID informative output lines | superseded by P2-008 goldens (SP6/SP7 late quantise) | none |
| g | Assessment: MQ 64 shared ring | separate 128 out ring | none |

## 18. Consequences

- The algorithm work (P2-008, dev tasks 8a-8d) is independent of the scheduler work (6-7): both meet at `PulseOut`.
- A workflow change after hardware testing (a different knob on a different morph, a different dice grouping) is a
  table edit plus re-lock, never a code change (ADR-0004 addendum).
- 5329 B of `.bss` and ~7.6 KB of pool buy the whole product; the E9 trigger (8 KB headroom) is not approached.
- Risks: the slide-as-overlap behaviour on the TB-3 is unverified (question 8); next-step latency of up to one step
  for hit/note/slide changes (question 9); the 0.73 ms block quantisation is inaudible but measurable on a scope.

## 19. Addendum (P2-010): timing accuracy and clock architecture

Status: proposed with [TIMING_SPEC.md](../../design/TIMING_SPEC.md) (the full design, numbers and citations); additive to
sections 5, 7 and 8. It changes **no struct of section 3** and no algorithm interface.

### 19.1 Corrections

- Section 7.1's "at most one block" is replaced by the measured budget: control block 32 samples = 0.7256 ms; one audio
  interrupt renders 4 blocks (2.90 ms) back to back; the USB transmitter runs only between audio interrupts at 2 kHz.
  Today's jitter on the FM-1 side is **3.4 ms peak-to-peak** (TIMING_SPEC 1.3) and the internal tempo is **+400 ppm**
  (counted in samples at 44100 Hz while the I2S clock is 44117.65 Hz).
- Section 7.4 first draft is amended as below.

### 19.2 Decisions (all in TIMING_SPEC section 3 and 4)

1. **Exact tick position (M1).** `sq_block` computes the sample `s` inside the block where each tick lands.
2. **Wall-clock stamping (M2) and timed release (M3).** `sq_emit` hands `midi_out_event_at(pkt, due)` a TIMER4 time
   `due = tref_block + s * 544 + LEAD` (`LEAD` = 128 samples = 2.90 ms). A 128-entry `{pkt, due}` ring (1024 B, replacing
   the 512 B out ring of section 8.3) is drained by the 10 kHz TIMER5 interrupt **including when nested in the audio
   interrupt**. The per-block `t1_pipeline_flush_midi` call disappears. Target: release lateness 0.25 ms peak-to-peak.
3. **Ordering (M4).** At one tick: clock, then transport bytes, then NOTE_OFFs, then NOTE_ONs, then CCs; order kept
   through the ring and the 16-event packet. Section 7.3's off-before-on rule is unchanged.
4. **Clock out (M7, replaces 7.4's bullets).** Pulse `0xF8` at ticks 0, 8, 16, ... through the same stamped path;
   `0xFA` at tick 0 of a start from step 0; `0xF2` (SPP = tick / 48) then `0xFB` on a continue; `0xFC` on stop. Sent only
   when the FM-1 is the master **and playing**; nothing in any EXT state. No menu row.
5. **Source model (TIMING_SPEC 4, owner rulings).** AUTO: INT by default; a `0xF8` accepted while stopped from USB or TRS
   makes the source EXT (first port heard wins); 500 ms of silence returns it to INT; a clock arriving while the FM-1 plays
   as master is ignored; a host `0xFA` takes over; PLAY on EXT is a labelled wait for the host start (no fall back).
   `song.g[G_BPM]` is the user's tempo and is **never written by the clock** (removes `midi_clock.c:65`); the host tempo
   lives in a new field.
6. **Following (M11, M12).** Input stamps in TIMER4 ticks instead of `fm1_ms`; a phase-locked loop on the pulse train;
   the position stays a pulse count (no drift); Start begins step 0 at the first `0xF8` after it; SPP (`0xF2`) is parsed.
7. **Tempo constant (M10).** `beat_samples()` counts in the real sample rate (44117.65 Hz, to be confirmed by the
   timing-test image) and returns 1/16 samples so 240 BPM stays within 5 ppm.
8. **Visibility** is a screen/LED spec (SCREEN_SPEC 6, TIMING_SPEC 5): the blinking clock dot on every screen.

### 19.3 Targets (TIMING_SPEC section 2)

FM-1-side jitter at most 0.25 ms peak-to-peak; at the host at most 1.5 ms peak-to-peak and 0.4 ms RMS (USB frame floor);
tempo error at most 100 ppm; clock out exactly 24 per quarter and the same spacing accuracy; following: 0 steps of drift
in 10 minutes, round-trip jitter at most 3 ms. Measured by a timing-test firmware image (separate build, zero cost in
the normal build), `tools/fm1_timing.py` and the hardware gate G1-G7.

### 19.4 Cost

About +0.6 KB `.bss` over section 11 (the larger timed ring, PLL and source state); no pool; no change to the E9
trigger. CPU: one compare per 100 us tick when idle; the gate is `cpu_q8` not rising by more than one percentage point.

### 19.5 Dev tasks (P2-005 places them in E4 / E5)

T-A timing-test image; T-B `tools/fm1_timing.py`; T-C release ring, stamps, tempo constant (extends prerequisite task 5
and 6); T-D source model, PLL, Start/SPP; T-E visibility; T-F hardware gate. TIMING_SPEC section 9 lists dependencies.

## 20. Addendum (P2-012): the HARMONY stage

Additive; for owner review. Full spec: [HARMONY.md](../../design/HARMONY.md). The sequencer core changes in four places and nowhere else.

| # | Change | Cost |
| --- | --- | --- |
| 1 | **A chord stage after the algorithm, at emit time** (`harm_apply()` between `step()` and `sq_note()`, section 5 and SEQUENCER_UX 9.2). `build()` and `step()` stay pure and unchanged; caches are not invalidated by a chord change, because the chord is not an input of `build()`. | ISR: a few integer ops per sounding step |
| 2 | **`HarmonySave` (8 B) joins `SeqPreset`** (1388 -> 1396 B, version 3), saved per slot. `GlobalSave` is unchanged; key, scale and octave stay there. | pool +144 B (18 preset-sized copies) |
| 3 | **Runtime**: `HarmonySave harm_db[2]` (double buffer, read once per block like `sq_gdb`), `HarmRT` (14 B, ISR-owned; the query result `HarmChord` is also **14 B as compiled**, not the 16 B first written), `HarmUndo` (9 B), one `undo_last` byte. `ChanRT` stays 24 B, `ChanSave` 344 B, `UndoRec` 92 B. | .bss +42 B |
| 4 | **`AlgoDesc.hi(m)`**: a const function per algorithm returning the highest semitone it can emit for morphs `m` (P2-029 amendment, prototype values: ACID **36** when JUMP > 0, and its floor is **-12** (octave -1..+2 as sting.md 6.3 allows, not 0..24); EUCLID `SPAN + 3`; SNAKE and STAGE `RANGE + 1`; ARP and PATTERNS **60**; the old `max(..., 12)` forms are too low once jitter and snap-up are counted), used to fold a pulled note back into the algorithm's own range. `harm_apply` therefore takes `lo` as well as `hi` (`lo` = -12 for ACID, 0 for the others) and folds into `lo..hi` with floor division (HARMONY.md 4.2). ROM only. | 0 B RAM |

The chord position is a pure function of `sq_bar` (ticks since PLAY / restart, section 10) and `HarmonySave`, so PLAY resets it with no extra state, a preset load
that lands on a bar boundary swaps the progression and the position together, and RESTRT (F#5) does not touch it. With the default progression, `VOICE 2`,
no pedal, `harm_apply()` returns its input unchanged for every role except an explicit ROOT / FULL (HARMONY.md 1). Hand-off interface for ARP and PATTERNS: HARMONY.md 9.

## 21. Addendum (P2-024): preview per channel (owner decision 110)

Additive and superseding where it says so. Full design: [PREVIEW.md](../../design/PREVIEW.md). Replaces "the preview synth plays the selected channel on `trk[0]`" (sections 1, 8.1, 9, 12, 13).

| # | Change | Cost |
| --- | --- | --- |
| 1 | **Each channel has `prv_on` and `prv_snd` (ACID LEAD BASS PAD STRINGS PIANO)** in `ChanSave` (1 B, `SeqPreset` +4 B). All channels boot OFF. The part is `trk[ch-1]`. | flash tables only |
| 2 | **Preview and MIDI are the same events behind two independent gates (decision 111, P2-025).** `sq_emit` plays the preview part iff `prv_on` is set; it sends the MIDI packet iff the channel has a MIDI channel and is not muted and SOLO allows it. MIDI OFF and mute do **not** silence the preview, so a channel can be a CUE. The mute and SOLO test therefore lives at the MIDI gate in `sq_emit`, **not** in the step's hit resolution (which would also starve the preview). | two tests, no shared condition |
| 3 | **Slot CCs also drive the part** (`prv_cc`, a (device, CC) map from the profile column `prv`); the override parks with the slot. | about 40 B flash, a few bytes of override state |
| 4 | Gestures: hold a mute key 0.4 s = PREVIEW on/off; hold it and turn ALGO = sound (PREVIEW.md 1.4). | UI only |
| 5 | `sq_all_off` releases all four parts; selecting a channel releases nothing. | none |

## 22. Addendum (P2-029): contract changes after the six locked prototypes (owner lock, P2-017, 2026-11-12)

Additive; supersedes the figures it names. The authoritative header is `docs/design/algorithms/common/sq_types.h` (final for six algorithms, LOCK.md section 5). The golden files fix the behaviour.

| # | Change | Where it was | Cost |
| --- | --- | --- | --- |
| 1 | **`SeqPreset` is 1556 B, preset version 4** (six `AlgoState` slots, `st[SQ_NALGO]`, `SQ_NALGO` 4 to 6; version 3 had four, the bump is the storage layer's, P7-005). `ChanSave` 344 to **384 B**, `UndoRec` 92 to **94 B** (the other `s_mod`), plus `ModSave` 100 B per preset (MODULATION.md 7). Supersedes 1388 / 1396 B (sections 11.2 and 20). | sections 3, 11, 20 | pool: 18 x 1656 B = 29808 B (+4680 B over the 25128 B plan), pool free about 188 KB; `.bss` +160 B (`ChanSave`) +8 B (`UndoRec`) |
| 2 | **`PO_TIE = 0x80`** in `PulseOut.flags`: PATTERNS' stored T flag, the gate holds into the next sounding step (the core's TIE path of section 3 / SEQUENCER_UX 9.2). `PO_PIN` 0x40 stays reserved. `PulseOut` stays 6 B. | section 3 | none |
| 3 | **Mod lanes come from `AlgoDesc.mod(cache, g, hc, o, ModIdx *out)`, not from `PulseOut.mod_a / mod_b`.** The algorithm says which index each lane is read at (`ModIdx`, 8 B); the shared primitives SP18..SP20 turn the indices into values. A `PulseOut` field would have moved `ChanRT` offsets (MODULATION.md wrote `mod_a / mod_b`; **not adopted**). New: `SQ_SEED_MOD` (the seventh seed id, held in `ModSave`, not in `AlgoState`) and `SQ_SMOD_DEFAULT(ch)`; every SHAPE layer re-rolls `s_mod` with the seed it already had. | MODULATION.md 3.1, 7 | ROM only |
| 4 | **ARP core additions (arp/SPEC.md 6.10, AM-1 to AM-4).** `AF_CHORD = 0x02` in `AlgoDesc.flags`; `step(cache, g, const HarmChord *hc, PulseOut *out)` gains the chord argument (`NULL` unless `AF_CHORD`; the other algorithms ignore it); `AlgoDesc.sub_note(cache, g, k, hc)` (final MIDI note of repeat `k`; used whenever a step has `PO_REP`; `NULL` for the others); for an `AF_CHORD` algorithm the core skips `harm_apply` and fills `hc` from `HarmRT` (current bar) or `harm_chord_at_bar(bar + 1)` (lookahead) after applying the channel ROLE; `SQ_NALGO` is 6. | arp/SPEC.md 6.10 | ROM only |
| 5 | **Two shared primitives:** `sq_mix(x)` (the 32-bit integer hash of ARP SPEC 3) and `sq_seeded_perm(perm, n, x)` (ARP RAND: a shuffle of `0..n-1` from a stream whose state is `x`, two draws discarded, `n <= 24`); `sp18_cell` takes the whole index (ARP figures reach 137), not `i & 15`. | sq_prims.h | ROM only |
| 6 | **`hi(m)` and the floor** as amended in section 20 row 4 (EUCLID `SPAN + 3`, SNAKE and STAGE `RANGE + 1`, ACID 36 with floor -12, ARP and PATTERNS 60). | section 20 | ROM only |
| 7 | **Owner decision 115: EUCLID's ratchet pace is two-way.** `PulseOut` keeps **one** pace bit (`PO_PACE_DEC`, 0x08): a ratchet accelerates or decelerates; t1.md's third outcome ("even") is not stored (the draw count is kept, `d == 2` decelerates, the other two accelerate). No data-model change, so P4-001 locks the `PulseOut` flags as they stand. The earlier phrase "accelerates/even" (section 3 and the SP5 comment) reads "accelerates". | euclid/SPEC.md 6, OWNER_QUESTIONS 115 | none |
| 8 | EUCLID's cache is **38 B** (hit ordinals for mod lane A); ARP's `step()` has no `SeqGlobal`, so its cache keeps `base`; PATTERNS cache 22 B; ACID 54, SNAKE 55, STAGE 43, ARP 48: every cache is at most 64 B, so the 2 x 64 B x 4 channels of section 11 stand. | LOCK.md 2, 4 | none |
| 9 | **P4-024 (FB-015, FB-011, FB-012).** (a) `ChanRT.tie` holds the LENGTH a TIE hold was scheduled under (1..64, 0 none): at each step start the ISR compares it with `len` and, if they differ, purges the queued TIE OFF and sends the OFF at once (struct unchanged). (b) SEQUENCER_UX 9.2: a set SLIDE ON whose next step sounds wins over TIE (slide overlap S1 + D); otherwise TIE; TIE always cancels RATCH. (c) Step edits are undoable: `sq_note_edit_begin` + a `what = 8` UndoRec on the first changing write of a NOTE_EDIT session, on a tap, and on `sq_step_clear`. (d) `sq_global_set` builds all four caches into the inactive buffers from the other `SeqGlobal`, then flips `sq_gidx` and the four `sq_cidx` in one interrupt-off window; it is refused while a queued load is pending. (e) `SQ_PROB_LOW_256` derives from the flex constant `PROB_LOW`: `(PROB_LOW * 256 + 50) / 100` = 128. (f) The main loop calls `SQ_FRAME_SYNC()` (= `sq_chan_sync()`) once per frame before any `sq_*` write; `sq_undo` and every dice entry call it first. | sections 6, 10, 12.2; SEQUENCER_UX 9.2 | none (code only) |

Test evidence: `sh tests/design/run_protos.sh` prints `PROTOS PASS` (six algorithms) and `python3 tests/design/harmony_proto.py --check` prints `CHECK OK`.
