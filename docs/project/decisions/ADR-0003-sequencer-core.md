# ADR-0003: Sequencer core architecture and the algorithm interface

Status: **proposed** (task P2-004, 2026-11-10). Awaiting the owner's review at the human-review gate. Nothing here is
locked until the owner writes "Approved: <name> <date>" in the P2-004 log and, per ADR-0004, "Design locked" later.

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
2. **Everything an algorithm exposes is a table** (`AlgoDesc`): name, four morphs with labels/ranges/defaults, the
   knob-to-morph map, five shared layers -> seed lists, sub-labels, seed defaults. Remapping a knob, a dice button or a
   range is a table edit (ADR-0004 addendum). (Section 4.)
3. **Time is 192 PPQN**, accumulated from audio samples exactly as `t1_block` does today. All six DIV values are whole
   tick counts. (Section 7.)
4. **Seeds are 16-bit, per channel x per algorithm x per layer-seed**, stored in full for all four algorithms so
   switching back restores (S7). Undo is one record per channel (last dice / dice-all / pin-clear), swap = redo.
   (Sections 3, 6.)
5. **Events leave through one function** (`sq_emit`) to `midi_out_event` and, for the selected channel, to the preview
   synth (ANALOG on `trk[0]`). Note-offs are never dropped; stop/panic/disconnect silence everything. (Section 8.)
6. **Always on, boots working.** No mode switch, no T-1 flag. (Section 12.)
7. **RAM: +2.2 KB `.bss` for the core (2193 B)**, ~6.4 KB pool; fits today and after the E3 removals. (Section 11.)

## 3. Data model (exact C; all structs live in a new `sq_types.h`, included before every `sq_*.c`)

Constants:

```c
#define SQ_NCH        4          /* channels (owner decision 31) */
#define SQ_NALGO      4          /* 0 ACID, 1 EUCLID, 2 SNAKE, 3 STAGE */
#define SQ_NMORPH     4          /* K1..K4 */
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
#define PO_PACE_DEC   0x08   /* ratchet spacing decelerates (else accelerates/even, per SP5) */
#define PO_REP_SHIFT  4      /* bits 4..5: repeats-1 (0..3 -> 1..4 triggers in this step; EUCLID uses 1..3) */
#define PO_REP_MASK   0x30
#define PO_PIN        0x40   /* set by the core when a pin overlaid this step (UI: outline) */
_Static_assert(sizeof(PulseOut) == 6, "PulseOut");

typedef struct { char label[8]; uint8_t min, max, def, step; } MorphDesc;       /* label <= 6 chars + NUL; 12 B */
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

/* What one algorithm remembers for one channel (S7: all four are kept). 16 B */
typedef struct { uint8_t morph[SQ_NMORPH]; uint16_t seed[SQ_NSEED]; } AlgoState;
_Static_assert(sizeof(AlgoState) == 16, "AlgoState");

/* One channel as saved in a preset and read by the UI. The channel's only settings are midi_ch and algo (VISION 3). 86 B */
typedef struct {
    uint8_t  midi_ch;              /* 0..15 = MIDI channel 1..16; 16 = OFF (no MIDI out, still previews) */
    uint8_t  algo;                 /* 0..3 */
    uint8_t  muted;                /* 0/1 latched */
    uint8_t  pad;
    uint16_t pin_mask;             /* bit i = position i pinned (SP8). Kept across algorithm switches (N2) */
    uint8_t  pin_note[SQ_NPIN];    /* semitones above 36+key, 0..96, UNSNAPPED (so a scale change re-snaps reversibly) */
    AlgoState st[SQ_NALGO];        /* per-algorithm morphs and seeds, indexed by algorithm id */
} ChanSave;
_Static_assert(sizeof(ChanSave) == 86, "ChanSave");

/* One undo/redo record per channel: the last dice / dice-all / pin-clear. Swapping it in again = redo. 34 B */
typedef struct {
    uint8_t  what;                 /* 0 none, bit0 seeds, bit1 pins */
    uint8_t  algo;                 /* algorithm the seeds belong to */
    uint8_t  seed_mask;            /* bit k = seed k is in the record */
    uint8_t  pad;
    uint16_t seed[SQ_NSEED];       /* the OTHER value of each masked seed (swapped on undo/redo) */
    uint16_t pin_mask;             /* the OTHER pin set */
    uint8_t  pin_note[SQ_NPIN];
} UndoRec;
_Static_assert(sizeof(UndoRec) == 34, "UndoRec");

/* The saved device state = a preset (P01..P16) = the autosave. 356 B */
typedef struct {
    uint8_t  version;              /* 1 */
    uint8_t  flags;                /* 0 */
    uint16_t bpm;                  /* 40..240 */
    GlobalSave g;
    ChanSave ch[SQ_NCH];
    uint16_t crc;                  /* CRC-16 over the bytes before it (the storage layer, E7, fills/checks it) */
} SeqPreset;
_Static_assert(sizeof(SeqPreset) == 356, "SeqPreset");

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
asserts `SQ_EVQ_N * sizeof(SeqEvent) == 768`, `sizeof(((SeqPreset*)0)->ch) == 344`, and
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
- **Dice** gives a seed the value `sq_entropy()` = low 16 bits of a free-running sample counter (incremented by
  `n` in `sq_block`) XOR `fm1_ms << 3`; if the result equals the old seed, add `0x9E37`. For a layer bound to two seeds
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
    uint8_t    knob_morph[SQ_NMORPH];      /* K1..K4 -> morph id. v1 = {0,1,2,3}. THE knob remap table */
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
| ACID (id 0, n_seed 6, `AF_PAIRING`) | 0 NOTES 1; 1 HITS 2; 2 OCT 3; 3 GATE 4; 4 ACCNT 5; 5 SLIDE 6 | `{1,0}` `{1,1}` `{1,4}` `{2,3,5}` `{1,2}` | NOTES HITS ACCNT GT+SLD OCTAVE | 0,1,2,3 |
| EUCLID (id 1, n_seed 4) | 0 ROT 0xFF; 1 NOTES 1; 2 ACCNT 2; 3 RATCH 3 | `{1,1}` `{1,0}` `{1,2}` `{1,3}` `{0}` | NOTES ROTATE ACCNT RATCH - | 0,1,2,3 |
| SNAKE (id 2, n_seed 4) | 0 NOTES 1; 1 GATES 2; 2 VEL 3; 3 POLY 4 | `{1,0}` `{1,1}` `{1,2}` `{0}` `{1,3}` | NOTES GATES VELO - DRIFT | 0,1,2,3 |
| STAGE (id 3, n_seed 5) | 0 NOTES 1; 1 LENS 2; 2 RESTS 3; 3 MODE 4; 4 VEL 5 | `{1,0}` `{1,2}` `{1,4}` `{1,3}` `{1,1}` | NOTES RESTS VELO RT+HLD LONG | 0,1,2,3 |

Morph tables (`MorphDesc{label,min,max,def,step}`; `step` = value change per detent; a slow turn is always 1):

| Algo | m0 (K1 DENSITY) | m1 (K2 FORM) | m2 (K3 PITCH) | m3 (K4 SPICE) |
| --- | --- | --- | --- | --- |
| ACID | DENS 0,16,12,1 | PEDAL 0,12,6,1 | JUMP 0,20,5,1 | ACCNT 0,8,3,1 |
| EUCLID | HITS 0,16,5,1 | LEN 2,16,16,1 | SPAN 0,24,12,1 | ACCNT 0,8,2,1 |
| SNAKE | DENS 0,16,10,1 | LEN 1,16,7,1 | RANGE 0,48,12,1 | PATH 1,14,7,1 |
| STAGE | DENS 0,8,6,1 | TOTAL 8,64,16,4 | RANGE 0,36,12,1 | RATCH 0,8,2,1 |

Rules: morph value 0 on knob slot 0 (K1) is shown as `KEYS ONLY` (UI rule, P2-003; no algorithm code: `build` with
`m[0] == 0` simply yields no hits). The build's parameter order `m[]` is the **morph id** order above (ACID's
assessment names `typ,den,jmp,acc`: the prototype maps names to ids `1,0,2,3`). Morphs are monotonic and reversible
(order-based, from the assessments), which is `build()`'s property, not the core's.

**Dice** (core, `sq_dice_layer(ch, L)`): for each seed id `s` in `ALGOS[a].layer[L]`: store old value in `UndoRec`, set
`new = entropy` (second seed `^ 0x5A5A`, EUCLID ROT never 0), then `sq_rebuild(ch)`. An absent layer (`n == 0`) is
inert (no seed change, no undo record) and the UI shows its toast (CONTROL_MAP). `sq_dice_all(ch)`: layers 0..4 in order,
absent skipped, ONE undo record covering the union. `sq_undo(ch)` swaps every seed in `seed_mask` (and the pin set if
`what & 2`) with the record, then rebuilds; tapping again swaps back (redo). A new dice / pin-clear overwrites the record.
The record is cleared (`what = 0`) on an algorithm switch and on a preset load.
(This **refines** ALGORITHM_SET 4.3, which keeps one previous-seed per seed: with that scheme "undo the last dice" after
dicing NOTES then HITS would revert both. One record per channel undoes exactly the last action. Owner question 46.)

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
| Cache sizes (ACID 52, SNAKE 52, STAGE 41, EUCLID 30 B) | `cache_bytes <= 64` | all |

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

Because `build` reads only `key/octave/snap`, a host test that varies gate/div/swing must see identical caches.
Owner questions 43 (swing formula) and 44 (GATE neutral point) are open; both are one table edit.

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
it applies a preset) guards the one race: `sq_rebuild` records the epoch before building and, if it changed before the
flip, builds again.

Control-surface API (all main-loop, no ISR calls; returns nonzero if state changed). Signatures are normative so UI
tasks can be written before the core exists:

```c
void    sq_morph_turn(uint8_t ch, uint8_t knob, int8_t detents);  /* via knob_morph[], clamps to min/max, step per detent */
void    sq_dice_layer(uint8_t ch, uint8_t layer);
void    sq_dice_all(uint8_t ch);
void    sq_undo(uint8_t ch);                                      /* undo; again = redo */
void    sq_set_algo(uint8_t ch, uint8_t algo);                    /* keeps st[]; clears UndoRec; lands at the next step */
void    sq_set_midi(uint8_t ch, uint8_t midi_ch);                 /* 0..15, SQ_MIDI_OFF */
void    sq_set_mute(uint8_t ch, uint8_t on);
void    sq_select(uint8_t ch);
void    sq_pin_set(uint8_t ch, uint8_t pos, uint8_t semi);        /* pos 0..15, semi 0..96 */
void    sq_pin_clear(uint8_t ch, uint8_t pos);
void    sq_pin_clear_all(uint8_t ch);                             /* undoable (UndoRec.what bit1) */
void    sq_global_set(const GlobalSave *g);                       /* recomputes snap[], rebuilds all caches */
void    sq_restart(void);                                         /* D#5: all channels to step 0 at the next step */
void    sq_live_on(uint8_t note, uint8_t vel);                    /* a key on the selected channel; snapped; ISR-safe */
void    sq_live_off(uint8_t note);
int     sq_preset_save(SeqPreset *out);
int     sq_preset_load(const SeqPreset *in, uint8_t when);        /* when: 0 now (stopped), 1 at the next bar */
```

## 7. Timing and the scheduler

### 7.1 Why 192 PPQN

192 is the T-1 resolution and `t1_block()` already derives it from `beat_samples()`: `acc += n * 192; while (acc >= bs)
{ acc -= bs; tick(); }`, which follows the internal BPM and an external MIDI clock (`events_block`'s `seq_n`). It is
kept because: (a) every v1 DIV is a whole number of ticks (`DIV_TICKS` above: no rounding drift); (b) it resolves swing
to 1/24 of a 1/32 step and a ratchet of 3 inside the shortest step (24 ticks -> 8-tick spacing); (c) the slide overlap
`D = max(1, L/8)` ticks is 3 ticks at 1/32 and 6 at 1/16; (d) the control block is `CTL` = 32 samples = 0.73 ms, and a
tick is 2.6 ms at 120 BPM and 1.0 ms at 300 BPM, so never more than two ticks run per block. Timing jitter is therefore
at most one block (0.73 ms), below USB-MIDI's own 1 ms frame. A finer grid (sample-domain scheduling) was rejected: no
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
3. `out1 = eval(c, t+1)`; if `AF_PAIRING`: `rt.nxt = out1; rt.nxt_ok = 1`.
4. If `muted` or `!(out.flags & PO_HIT)`: if `rt.open` (a previous step left the sounding note without an off, expecting a tie) emit its NOTE_OFF now (`rt.snd[0]` on `rt.snd_chan[0]`) and clear `rt.open`; `rt.slide_in = 0`; skip to step 8. (Muting keeps phase: `t` and `grid` still
   advance. Owner/T1 F10.)
5. `slideout = AF_PAIRING && (out1.flags & PO_HIT) && (out1.flags & PO_SLIDE_IN) && ((t + 1) % loop_len != 0)`.
   `tie = rt.slide_in && rt.snd[0] && out.note == (rt.snd[0] - 1)`. If `rt.open` and not `tie` (a mute, a note change or an algorithm switch happened since the tie was announced), emit the NOTE_OFF of `rt.snd[0]` now, before step 6. `rt.open = 0` after this line.
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
8. `rt.slide_in = slideout; sq_set_algo clears `nxt_ok` (the snapshot belongs to the old algorithm); rt.t = t + 1; rt.grid = G + L; rt.act++` (if a note was scheduled).

Ordering rule at one tick: `sq_dispatch` emits **all NOTE_OFFs due, then all NOTE_ONs due**. A slide pair never ties on
the same tick (the off is at `S + D`, `D >= 1`); a normal pair never does either (off capped at `S1 - 1`). Ratchet
repeats never overlap (length < spacing). The only legal overlap is the slide's `D`, and `ChanRT.snd[2]` has exactly
two slots for it.

**Restart (D#5) and PLAY:** `rt.t = 0`, `rt.grid = now`, `rt.nxt_ok = 0`, `rt.slide_in = 0` for all channels at the same
tick (immediately at PLAY, at the next grid tick for a restart request). The loop wrap needs no code: `t` is
free-running and `step()` reduces it.

## 8. The event path

```
main loop (UI)                         audio ISR (events_block -> sq_block -> sq_tick)             USB / synth
sq_* API, sq_rebuild --cache flip--->  sq_step_start: step() + pins -> sq_sched() into EvQ[96]
                                       sq_dispatch(now): due events -> sq_emit() ---------------> midi_out_event(pkt)  [usb.c ring]
                                                                       \-------(chan == sq_sel)--> trk_note_on/off(&trk[0], ...)  [ANALOG]
keyboard_block (white keys) --> sq_live_on/off: sq_emit(now), no queue
```

### 8.1 `sq_emit(chan_byte, kind, note, vel)` (the only exit)

- MIDI: unless `chan_byte & 0x10` (MIDI OFF), build the packet exactly as T-1 did:
  NOTE_ON `0x09 | (0x90 | m) << 8 | note << 16 | vel << 24`; NOTE_OFF `0x08 | (0x80 | m) << 8 | note << 16`;
  CC123 `0x0B | (0xB0 | m) << 8 | 123 << 16`; call `midi_out_event(pkt)`.
- Preview: if the sequencer channel in the byte equals `sq_sel`, `trk_note_on(&trk[0], note, vel)` /
  `trk_note_off(&trk[0], note)`. The preview synth is ANALOG preset 0 (owner decision 40) and plays only the selected
  channel. When `sq_sel` changes, the ISR first releases the previous channel's `snd[]` notes on `trk[0]` (tracked by
  `ChanRT.snd`).
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
| MIDI-channel change, algorithm switch, preset load, mute | **No cut.** Queued events keep their own MIDI channel, so the old note ends on the old channel. (This replaces the "cut on algorithm / channel change" line in sting 6.3a item 3; owner question 47.) |
| Mute | New hits stop at once; sounding notes end by their queued offs (no click, no hang). |

## 9. Step entry: pins on the 16 white keys, and `K1 = 0` (KEYS ONLY)

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

- **Enter** (STEP mode, CONTROL_MAP W4): `sq_pin_set(ch, pos, semi)`. The UI chooses `semi`: the note generated at that
  position (`step(pos)` converted back with `semi = note - 36 - key - 12 * octave`, clamped 0..96) or 0 (the root) when
  that step is silent. `pin_note` writes before `pin_mask`.
- **Edit:** K3 = one scale degree per detent (the UI walks the scale table), OCT-/OCT+ = +-12; both clamp 0..96.
- **Clear:** `sq_pin_clear` removes one; `sq_pin_clear_all` is recorded in `UndoRec.what bit1`, so EDIT-tap undoes it.
- **K1 = 0:** `m[0] == 0` means the algorithm's `build()` produces no `PO_HIT`; pins still play because the overlay
  forces `PO_HIT`. No fifth algorithm (owner decision 35). Because `step()` still fills `vel`/`gate`, pins sound at the
  right dynamics.
- **Live keys** (PERF): `keyboard_block` calls `sq_live_on(note, vel)`; the note is snapped by `sq_snap_abs`, sent on the
  **selected** channel's MIDI channel (unless OFF) and the preview synth. It bypasses the queue; the key-up sends the
  paired NOTE_OFF (the key table remembers the exact `chan_byte` and note sent). MIDI IN notes use the same two calls.

## 10. Presets, queued loads, undo of a load

- A preset is a `SeqPreset` (356 B). `sq_preset_save()` copies `sq_ch`, `sq_g.s`, BPM. Slot storage (P01..P16 = 5696 B,
  the autosave, the "undo last load" copy) is E7 (project.c becomes the store); this ADR fixes only the byte layout.
  `sq_preset_load()` **validates and clamps every field** (algo < 4, midi_ch <= 16, morph within `MorphDesc` min/max,
  pin_note <= 96, scale < 16, ...); an invalid `version` or CRC loads the power-on defaults.
- **Queued to the next loop start (owner decision 34):** "loop start" = the next tick where the global bar counter
  `sq_bar` (steps since PLAY / restart, `/ 16`) rolls over, i.e. every 16 steps (one bar at 1/16): data constant
  `SQ_BAR_STEPS = 16` (owner question 45). Procedure: the main loop validates the preset into a pool staging copy,
  builds the four caches into the **inactive** buffers `sq_cache[c][cidx^1]` and `sq_gdb[gidx^1]`, then sets
  `sq_load_pending = 1` (last store). At the bar tick `sq_apply_bar_boundary()` (ISR) copies the staged `ChanSave`s and
  `GlobalSave` over the live ones (<= 400 B), flips every `cidx` and `gidx`, clears every `UndoRec`, increments
  `sq_epoch`, clears `sq_load_pending`. `rt.t/grid` are untouched (the music does not jump). Stopped: applied at once
  with the ISR guard `fm1_irq_off()/fm1_irq_on()` (the pattern of `t1u_temp` in ui_t1.c).
- **SAVE hold 0.7 s** restores the copy of the previous state saved at load time (kept in the pool); it goes through the
  same queued path; again = redo.

## 11. Memory budget

Facts (SYSTEM_MAP section 9): RAM `.data+.bss` after the E3 removals 27232 B of 73728 = **46496 B headroom** (today
17984 B); floor 4096 B; E9 trigger 8192 B. Pool after removals 150665 B of 368640 (**217975 B** free; today 25764 B).

| Item | Per channel | x4 / total | Where |
| --- | ---: | ---: | --- |
| `ChanSave` (live) | 86 | 344 | .bss |
| `UndoRec` | 34 | 136 | .bss |
| `ChanRT` | 24 | 96 | .bss |
| Algorithm caches, double buffered (2 x 64) | 128 | 512 | .bss |
| `SeqGlobal` x2 (double buffer) + `cidx[4]` + `gidx` | | 41 | .bss |
| Event queue `evq[96]` (96 x 8) | | 768 | .bss |
| Transport / clock / requests / counters (`sq_acc`, `sq_now`, `sq_bar`, `sq_entropy_ctr`, flags, `sq_drops`, `sq_epoch`, `sq_sel`) | | 32 | .bss |
| `sq_calgo[4][2]` | | 8 | .bss |
| USB-MIDI out ring 64 -> 128 packets (separate `MQO`) | | +256 | .bss |
| **Core total (.bss)** | **272** | **2193** | |
| UI state allowance for P2-003 (toasts, knob highlight timers, layer state, redraw signatures) | | 3072 | .bss (cap; P2-003 must fit or amend) |
| **Total new .bss** | | **5265** | |
| Preset slots P01..P16 (16 x 356) | | 5696 | .pool / `.noinit` (replaces `proj_slot` 15360 B; E7) |
| Staging preset + "undo last load" copy | | 712 | .pool |
| **Total new pool** | | **6408** | |

| Check | Allowed | Used | Result |
| --- | ---: | ---: | --- |
| `.bss`, after E3 removals: 46496 - 4096 | 42400 | 5265 | fits, headroom left 41231 (> 8 KB trigger) |
| `.bss`, if built before the removals: 17984 - 4096 | 13888 | 5265 | fits, headroom left 12719 (> 8 KB trigger) |
| Pool, today | 25764 | 6408 | fits |
| Stack | n/a | `build` <= 256 B (main loop), `step` <= 64 B (ISR) | each dev task measures |

Removed with R8 (T-1 retirement): `t1_micro_queue` 4096 B and the two `MidiRingBuffer` 2 x 264 B and `t1_sequencer_ram`
9348 B + the T-1 `.pool` buffers (13980 B total per SYSTEM_MAP; all in the pool, `.bss` impact 36 B). CPU: per tick
`sq_tick` does 4 x (2 `step()` + <= 8 queue inserts) plus 192 slot compares, a few thousand integer ops, at most twice per
block; the first core task records `felucca_dbg.max_us` before and after (budget: no more than +5 % of the 725 us block).

## 12. Boot sequence and power-on defaults (owner decision 32b)

The device boots straight into the sequencer: no mode, no `PREF_T1`, no `MODE` row, no groovebox route. In `main.c`
`fm1_main()`, in this order (changes marked):

1. `persist_boot`, `settings_init`, `usb_serial_apply`, `lcd_init`, `draw_splash`, input/ADC/`panel_init` (unchanged).
2. `felucca_init()` **reduced**: only the preview voice, `trk[0]` = ANALOG preset 0 (decision 40). Groovebox track defaults
   and `load_pat16` are gone (E3).
3. `master_boot()` (unchanged).
4. **`sq_boot()` replaces `t1_glue_init()`**: fill `sq_ch[]`, `sq_g` from `SQ_DEFAULTS` (below); `sq_gbuild()`;
   `sq_rebuild(c)` for all four channels into buffer 0; `cidx = gidx = 0`; zero `ChanRT`/`evq`; **last**: `sq_ready = 1`.
5. `audio_init()` (the ISR may now call `sq_block`; before step 4 completes it is inert because `sq_ready == 0`), `usb_start`,
   `uart_midi_init`, `boot_leds`, `timer5_start`, `fm1_irq_enable_all` (unchanged).
6. `autosave_boot(boot_clean)`: if the last shutdown was clean and an autosave `SeqPreset` validates, `sq_preset_load(...,0)`
   (stopped: applied at once). Otherwise the defaults from step 4 stand. After a crash the defaults stand.
7. The 400 ms splash, then the main (PERF) screen: channel 1 selected, stopped, no menu. Press PLAY: CH1 plays.

`SQ_DEFAULTS` (a const table; CONTROL_MAP section 0, UX_REVIEW section 2):

| Item | Value |
| --- | --- |
| CH1 | ACID, MIDI channel 1, unmuted |
| CH2 / CH3 / CH4 | EUCLID MIDI 2 / SNAKE MIDI 3 / STAGE MIDI 4, all muted |
| every `AlgoState` | morphs = `MorphDesc.def`; seeds per section 3.1 for each channel and algorithm |
| pins | none |
| Global | KEY C (0), SCALE MINPEN (10), OCTAVE 0, GATE 50, DIV 1/16 (2), SWING 50, BPM 120 |
| UI state | selected channel CH1, preset P01 label, keyboard OCT 0 |
| Preview | ANALOG preset 0 |
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
| `Pattern`, `Track`, `CycleParams`, `SequencerRAM` | **Drop** | replaced by `SeqPreset` (356 B vs 9348 B) |
| `t1_block` (t1_glue.c) | **Rewrite** as `sq_block`; keep its accumulator idea | |
| `t1_all_off`, `t1_on_start`, `t1_on_stop` | **Rewrite** as `sq_all_off`, `sq_on_start`, `sq_on_stop`; keep the order note-offs then CC123 | model for 8.4 |
| `t1_default_pattern`, `t1_glue_init`, `t1_set_mode` | **Drop / replace** by `SQ_DEFAULTS` and `sq_boot` | decision 32b: always on |
| `synth_note_on/off` (voice allocator callbacks) | **Drop**; preview calls `trk_note_on/off(&trk[0],...)` directly | |
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
| `AlgoDesc.knob_morph[4]` | any knob to any morph of that algorithm (swap two numbers) |
| `AlgoDesc.morph[]` | label (<= 6 chars), min, max, default, detent step |
| `AlgoDesc.layer[5]` | which seed(s) a dice button re-rolls; move a seed to another layer; bind two seeds to one layer; mark a layer absent |
| `AlgoDesc.sub[5]` | dice sub-labels |
| `AlgoDesc.seed_k[]`, `SQ_DEFAULTS` | default seeds' k, boot algorithm/MIDI/mute per channel, global defaults |
| `DIV_TICKS[6]`, `GATE_MUL[10]`, `SQ_BAR_STEPS`, swing formula constants, `D` (L/8), `GAP` | feel constants |
| Name order of `ALGOS[]` (`EN_ALGO` cycle order) | reorder |
| Number of detents for fast/slow knob turns | UI table (P2-003/P2-001), not here |

Cannot change without code (and an ADR-0003 amendment, an owner re-lock, and golden regeneration):

| Item | Why |
| --- | --- |
| `SQ_NCH`, `SQ_NALGO`, `SQ_NSEED`, `SQ_NLAYER`, `SQ_CACHE_MAX`, struct layouts | the stored preset format and static asserts |
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

| # | Earlier text | This ADR | Owner question |
| --- | --- | --- | --- |
| a | ALGORITHM_SET 4.3 per-seed previous seed, undo swaps all previous seeds | One `UndoRec` per channel (last dice / dice-all / pin-clear) | 46 |
| b | sting 6.3a rule 3: algorithm or MIDI-channel change cuts notes | No cut; events carry their MIDI channel; only stop/panic/disconnect cut | 47 |
| c | ACID "GATE multiplies"; control map default GATE 50 % | `GATE_MUL(x) = x * 2` (percent; 50 % = x1.00): 50 % = the algorithm's own gate | 44 |
| d | T-1 swing: odd steps shifted by `(swing - 50)` % of a step | pair-ratio swing, `2 * (swing - 50)` % of a step | 43 |
| e | "preset load at the next loop start" (loop undefined with 4 different loop lengths) | every 16 steps (a bar at 1/16) | 45 |
| f | SNAKE/EUCLID informative output lines | superseded by P2-008 goldens (SP6/SP7 late quantise) | none |
| g | Assessment: MQ 64 shared ring | separate 128 out ring | none |

## 18. Consequences

- The algorithm work (P2-008, dev tasks 8a-8d) is independent of the scheduler work (6-7): both meet at `PulseOut`.
- A workflow change after hardware testing (a different knob on a different morph, a different dice grouping) is a
  table edit plus re-lock, never a code change (ADR-0004 addendum).
- 5265 B of `.bss` and ~6.4 KB of pool buy the whole product; the E9 trigger (8 KB headroom) is not approached.
- Risks: the slide-as-overlap behaviour on the TB-3 is unverified (question 8); next-step latency of up to one step
  for hit/note/slide changes (question 9); the 0.73 ms block quantisation is inaudible but measurable on a scope.
