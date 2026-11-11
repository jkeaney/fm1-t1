# ARP: an arpeggio over the HARMONY chord progression (P2-013)

Task P2-013, prototype P2-017. Status: **prototype and golden written (P2-017), for the owner's design lock** (gate `human-review`). No firmware is written here. [`proto.c`](proto.c) is the firmware file (P4 copies it and adds `static`);
[`golden.txt`](golden.txt) is its output; [`tests/design/arp_ref.py`](../../../../tests/design/arp_ref.py) re-implements sections 6.3 to 6.5 independently and agrees on all 1728 grid cases; [`tests/design/arp_extra.h`](../../../../tests/design/arp_extra.h)
holds the ARP golden sections of section 11. Changes found while writing the prototype are in 6.13. Source: the deferred assessment [../steparp.md](../steparp.md) (Step Arp, `docs/steparp-algorithms/`), redesigned on the chord
progression of [../../HARMONY.md](../../HARMONY.md) (P2-012). Vision: [../../../project/VISION.md](../../../project/VISION.md). Every worked example below was computed by a throwaway
Python transcription of sections 6.3 to 6.5 and the HARMONY prototype (`tests/design/harmony_proto.py` for the chords); the golden files of P2-008 supersede them.

Where this document is silent on an algorithm-independent matter (core scheduling, edit layer, pages, presets), the locked rule of ADR-0003, SEQUENCER_UX and ALGORITHM_SET applies.
Marks: **[D]** is a decision made here (no fidelity claim to the source); **[V]/[I]** are the source's tags where a claim from `docs/steparp-algorithms/` is relied on.

## 0. Summary

`ARP` is the fifth v1 algorithm (owner decision 82: six in all, ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS). It arpeggiates **the chord HARMONY is on right now**: the tones come from
the progression, VOICE, MOVE and DRONE dials of the HARMONY page, so changing a chord, the voicing or the drone changes the arpeggio at once, with no setting on the ARP side.
Eight morphs (four performed, four dialled in) set the **shape** of the figure, the **direction**, the **speed** (rate against the global clock, triplets included), the **octave span**,
how many notes sound, how long they are, how hard, and where the figure starts. The five dice layers re-roll the octave hops and random order, the rests, the velocities, the note
lengths and the starting phase, one at a time. With HARMONY at its default (one chord, VOICE 2) ARP already plays: it arpeggiates the key's own triad (C Eb G in the default MINPEN).

| Question of the brief | Answer | Where |
| --- | --- | --- |
| What voicing does the arp follow? | The chord of the bar (`harm_chord_at_bar`), as voiced by VOICE, MOVE and DRONE, through the P2-012 query interface; the drone is one of the walked tones | 6.4 |
| Direction set | UP, DOWN, UPDN, RAND, ASPL (as played), CHORD (chord order) | 6.5 |
| Shape set | LINE, SKIP, THUMB, OCTV, REPT, WAVE | 6.5 |
| Rate set (global clock) | `1/4` `1/2` `1X` `2X` `3T` `4X` of the global step, so DIV 1/16 gives quarter to 1/64 notes and the global triplet DIVs stay available | 6.5 |
| Pattern length 1..64 against the arp cycle | The figure is locked to the pattern position and restarts with the loop; the figure length is toasted; pages may continue a long figure | 6.6 |
| Chord change mid-pattern | The pool changes at the bar line, the figure does not restart (stateless) | 6.7 |
| One held note, HARMONY `1----` | A one-chord progression is a held chord; VOICE 0 makes it a held single note whose arp is an octave figure | 6.8 |
| Cost | `ChanRT` 24 B unchanged; cache 48 B; +20 B per channel in the preset | 9 |

## 1. What it does (5 lines max)

ARP turns the HARMONY chord of the current bar into a repeating note figure: the voiced chord tones are stacked over 1 to 4 octaves into a **pool**, a **direction** orders the pool,
a **shape** rewrites the order into a figure (straight, skipping, home-note bouncing, octave bouncing, stuttering, rolling), and a **rate** says how many figure notes fall in a
step. A seeded rest pattern, velocity lane and note-length lane sit on the same 16 steps. It is stateless: the note at a step is a pure function of the step position, the chord,
the eight morphs and five seeds, so a morph or a dice never loses the clock phase.

## 2. Generators inside it

The source ([../steparp.md](../steparp.md) 2) is a held-note arpeggiator; this sequencer has no held keys on a channel (rule 3), so the chord comes from HARMONY instead.

| Generator | Inputs | Output | Deterministic? (seeded / free-running) |
| --- | --- | --- | --- |
| Chord pool (replaces held-note list, Hold) | `HarmChord` of the step's tick (HARMONY.md 9), SPAN | up to `6 x 4 = 24` pool tones | Pure: HARMONY owns the chord; the pool is `tone j = T[j mod n] + 12 (j div n)` (6.3 step 5) |
| Direction (source Note Order x Octave Mode, 12 combinations) | DIR, pool, NOTES seed (RAND only) | base order of the pool | UP, DOWN, UPDN, ASPL, CHORD pure; RAND a hashed bijection per cycle, seeded (6.3) |
| Shape (new; the source's Step Arp counter only walks forward) | SHAPE, base order | the figure of 3 to 138 positions | Pure |
| Rate (source `TimeDiv`, `Triggers`) | RATE | note slots per step (slower or faster than the step) | Pure; tied to the global DIV |
| Rest pattern (source steps on/off row [V, 04 4.2]) | HITS seed, DENS | which of the 16 steps sound | Seeded rank order (SP2, cell 0 first) |
| Velocity lane (source lane [V, 04 4.3]) | VEL seed, DYN | step velocity | Seeded (SP4), depth by SP17 |
| Note-length lane (source Length lane [V, 04 4.4]) | GATE seed, LEGATO | step gate % | Seeded |
| Octave hops, starting phase (new) | NOTES seed, ROT seed | which figure notes jump an octave; where the figure starts | Seeded |
| Chord Arp (the whole chord at once) | | | **Dropped** (a channel is monophonic) |
| Hold, Key Retrig, Lock, Sync/Freq, Dotted/Triplet, StepDirection, LoopLength, Transpose, Chance and CC lanes | | | **Dropped** (section 6.9); the global clock, HARMONY, the channel LENGTH and the edit layer own those jobs |

## 3. Layers (what can be diced independently)

All five shared layers exist (ALGORITHM_SET 4.1). Every layer has its own 16-bit seed; a layer reads no other layer's stream, so dicing one cannot move another.

| Layer (button) | Internal seed | Sub-label | What dicing it changes | What stays fixed |
| --- | --- | --- | --- | --- |
| `NOTES` (SCL) | `s_notes` (seed id 0) | `VARY` | Which 4 of the 16 positions jump an octave up (when SPAN is 2 or more), and the random order of RAND. Default seed 0 means no hops: a fresh ARP is the plain figure | Chord, pool, shape, rests, velocities, lengths, phase, all morph values |
| `HITS` (SEQ) | `s_hits` (1) | `RESTS` | The priority order in which the 16 steps switch on as DENS rises: where the rests fall | Notes, velocities, lengths, phase |
| `ACCNT` (ENV) | `s_vel` (2) | `VELO` | The 16 step velocities (50..127) | Notes, rests, lengths, phase |
| `ARTIC` (ARP) | `s_gate` (3) | `LENGTH` | The 16 note-length factors (0.5x to 1.5x of the LEGATO length) | Notes, rests, velocities, phase |
| `SHAPE` (LFO) | `s_rot` (4) and `s_mod` (MODULATION 3.4) | `PHASE` | Where in its cycle the figure starts: the start position `s_rot mod F`. Default seed 0 means the figure starts on its first note. `s_mod` re-rolls the mod lanes (6.12a) with it | Pool, order, rests, velocities, lengths, every note except through the phase |

**Dice all** (FX) re-rolls the five in the order NOTES, HITS, ACCNT, ARTIC, SHAPE with one undo record; **undo** (EDIT tap) swaps them back (ADR-0003 4.1, one record per channel).
Dice on NOTES or SHAPE gives the new seed from the entropy as for every layer; the chance of a result of 0 (the plain value) is 1 in 65536 and is harmless. No layer is absent, so no
button is dimmed for ARP (ALGORITHM_SET 4.4). The progression itself is **not** a layer: the PROG dice stays on the HARMONY page (HARMONY.md 6.3).

## 4. Morph candidates (continuous knob transforms)

Eight morphs, the ADR-0003 layout: slots 0..3 = ROW 1 (K1..K4, the performed row), 4..7 = ROW 2 (S1..S4, tapped or held OCT+). The slot meanings follow ALGORITHM_SET 5.1 (K1 DENSITY,
K2 FORM, K3 PITCH, K4 SPICE; S1 START, S2 ARTIC, S3 DYNAMICS, S4 READ). Every morph is a pure function of (seeds, morph values, chord): turning back restores the previous pattern exactly.

| Id | Slot | Label | Range | Default | Value text (at most 5 characters) | Musical effect | Cost |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | K1 | `DENS` | 0..16 | 12 | number; 0 reads `KEYS` (no generated notes, entered steps still play) | Step `c` sounds iff its HITS rank is below DENS: from silence to a note on every step | 1 compare per step |
| 1 | K2 | `SHAPE` | 0..5 | 0 | `LINE` `SKIP` `THUMB` `OCTV` `REPT` `WAVE` | Rewrites the base order into a figure (6.5) | arithmetic per step |
| 2 | K3 | `SPAN` | 1..4 | 2 | `1 OCT` to `4 OCT` | Octaves the chord is stacked over: 1 is a tight 2 to 6 note figure, 4 a wide run | pool length only |
| 3 | K4 | `RATE` | 0..5 | 2 | `1/4` `1/2` `1X` `2X` `3T` `4X` | Notes per step (6.5): slower or faster than the global step | 1 table read |
| 4 | S1 | `OFFSET` | 0..15 | 0 | number | Slides the figure along the bar by that many steps (lanes stay put) | 1 add |
| 5 | S2 | `LEGATO` | 0..8 | 3 | number | Note length: gate % = `(20 + 10 x LEGATO)` before the ARTIC factor; 3 is the neutral 50 %, 8 is 100 % | 1 multiply |
| 6 | S3 | `DYN` | 0..10 | 10 | number | Velocity depth by SP17: 10 the lane as diced, 0 flat 90 | SP17 |
| 7 | S4 | `DIR` | 0..5 | 0 | `UP` `DOWN` `UPDN` `RAND` `ASPL` `CHORD` | Order of the pool (6.5) | per step |

`knob_morph = {0,1,2,3,4,5,6,7}`. Steps: all `step 1, fast 1` (no range exceeds 32 values). K1 = 0 is `KEYS ONLY` by the UI rule of ADR-0003 4.1 (no algorithm code).
Fit of each slot: ROW 1 holds the four things a player turns while it plays (how many, which figure, how wide, how fast); ROW 2 holds the dial-ins. DIR is on ROW 2 because it is READ
(S4) in every algorithm; it is still one gesture to perform (hold OCT+ and turn, FULL_CONTROL 1.2). **Rejected as morphs:** swing (global), division (global; RATE multiplies it), humanise
(replaced by DYN and the dice lanes, 6.9), chord type, voicing and drone (HARMONY dials, one source), hold and latch (HARMONY holds the chord, 6.9).

## 5. Fit against the vision (score 0 to 3 each, with one line of why)

| Criterion | Score | Why |
| --- | --- | --- |
| Quick to a usable sequence (rule 5) | 3 | Turn ALGO to ARP on a running channel: a seeded 12-of-16 arpeggio of the key's triad sounds on the next step with no other setting (the worked example in 10.1) |
| Performable with 4 knobs + dice buttons, no menus (rules 1, 4) | 3 | K1 how many, K2 which figure, K3 how wide, K4 how fast are audible reversible morphs; the chord moves under it from the HARMONY page; five dice, each audible at the defaults |
| Monophonic bassline/lead use (the TB-3 workflow) | 3 | One note per step (sub-notes are sequential), fold to a 5-octave ceiling, a home-note THUMB figure and a 1 OCT / DRONE bass pedal give acid-style arps; accents land on the figure start |
| Distinct from the other families (adds something new) | 3 | The only algorithm whose pitches come from the chord progression, and the only one that changes with a HARMONY edit mid-bar; rhythm is deliberately shared (SP2/SP4) because that part is already learnt |
| Fits the FM-1 budget (RAM < 2 KB per channel, ISR cost small) | 3 | Cache 48 B, no `ChanRT` growth, under 150 integer operations per step (9) |

Total 15 of 15. This revises the deferred assessment's 12 of 15 (steparp.md section 5): the held-key input it lacked is now the chord, and the rhythm half it duplicated is no longer the
selling point.

## 6. Redesign for this sequencer

- **Proposed name on screen (max 6 characters):** `ARP` (channel cell `1 ARP`, header `1 ARP M2`; id 4 in `ALGOS[]`).
- **K1 to K4 morphs (labels, max 6 characters):** `DENS` `SHAPE` `SPAN` `RATE`; ROW 2 `OFFSET` `LEGATO` `DYN` `DIR`.
- **Dice buttons:** NOTES (`VARY`), HITS (`RESTS`), ACCNT (`VELO`), ARTIC (`LENGTH`), SHAPE (`PHASE`), all five present.
- **What we drop from the original, and why:** section 6.9.
- **What we improve over the original:** the notes follow a progression the producer types in two gestures; the figure is stateless so no morph or dice desyncs it (the source's counter
  restarts or drifts when notes change [I]); six shapes and six directions replace a 12-combination menu pair; rate includes a triplet burst and slower-than-step notes; velocity, rests and
  lengths are seeded and undoable, where the source's Random buttons rewrite a lane for good.
- **Parameters that must not exist (settings creep, rule 1):** a Note Order or Octave Mode menu, Hold, Latch, Key Retrig, a chord-type or voicing selector on the ARP side (HARMONY owns it), a
  scale or root of its own, Loop Length, Step Direction, per-step lane edit pages, swing or division of its own, a seed entry field.

### 6.1 Parameters (data table; ranges and defaults)

The single source for labels, ranges, defaults and seeds; a remap is a table edit. `ch` is the 0-based channel index; `n_seed = 5`.

| Id | Kind | Label | Range | Default | Stored as |
| --- | --- | --- | --- | --- | --- |
| `den` | morph 0, K1 | `DENS` | 0..16 | 12 | u8 |
| `shp` | morph 1, K2 | `SHAPE` | 0..5 | 0 | u8 |
| `spn` | morph 2, K3 | `SPAN` | 1..4 | 2 | u8 |
| `rat` | morph 3, K4 | `RATE` | 0..5 | 2 | u8 |
| `off` | morph 4, S1 | `OFFSET` | 0..15 | 0 | u8 |
| `lg` | morph 5, S2 | `LEGATO` | 0..8 | 3 | u8 |
| `dyn` | morph 6, S3 | `DYN` | 0..10 | 10 | u8 |
| `dir` | morph 7, S4 | `DIR` | 0..5 | 0 | u8 |
| `s_notes` | seed 0, layer NOTES | `NOTES` | 0..65535 | literal 0 (`seed_k` = `0xFF`) | u16 |
| `s_hits` | seed 1, layer HITS | `HITS` | 0..65535 | `(0x3A5D * (ch*5 + 2)) & 0xFFFF` (`seed_k` 2) | u16 |
| `s_vel` | seed 2, layer ACCNT | `ACCNT` | 0..65535 | `(0x3A5D * (ch*5 + 3)) & 0xFFFF` (`seed_k` 3) | u16 |
| `s_gate` | seed 3, layer ARTIC | `ARTIC` | 0..65535 | `(0x3A5D * (ch*5 + 4)) & 0xFFFF` (`seed_k` 4) | u16 |
| `s_rot` | seed 4, layer SHAPE | `SHAPE` | 0..65535 | literal 0 (`seed_k` = `0xFF`) | u16 |
| `s_mod` | seed id 6 (`SQ_SEED_MOD`), layer SHAPE (second seed) | `SHAPE` | 0..65535 | `SQ_SMOD_DEFAULT(ch)` | u16 in `ModSave`, not in `AlgoState` (MODULATION 7) |

The default seeds of channel 0: `s_hits` 29882, `s_vel` 44823, `s_gate` 59764 (the same multiplier as the other algorithms, ADR-0003 3.1). The two literal-0 seeds make a fresh ARP the plain
figure from its first note (no hops, no phase); their first dice makes them live. AlgoState stays 8 morphs + 6 seed slots = 20 B (slot 5 unused). The `AlgoDesc` row (ADR-0003 4.1 form):

| Algo | Seeds (index: name, `seed_k`) | `layer[0..4]` (NOTES HITS ACCNT ARTIC SHAPE) | `sub[0..4]` | `knob_morph` | flags |
| --- | --- | --- | --- | --- | --- |
| ARP (id 4, n_seed 5, cache 48 B) | 0 NOTES `0xFF`; 1 HITS 2; 2 VEL 3; 3 GATE 4; 4 ROT `0xFF` | `{1,0}` `{1,1}` `{1,2}` `{1,3}` `{2,{4,SQ_SEED_MOD}}` | VARY RESTS VELO LENGTH PHASE | 0..7 | `AF_CHORD` (6.10) |

Morph rows for the ADR table (`label min,max,def,step,fast`): m0 `DENS 0,16,12,1,1`; m1 `SHAPE 0,5,0,1,1`; m2 `SPAN 1,4,2,1,1`; m3 `RATE 0,5,2,1,1`; m4 `OFFSET 0,15,0,1,1`; m5 `LEGATO 0,8,3,1,1`;
m6 `DYN 0,10,10,1,1`; m7 `DIR 0,5,0,1,1`.

### 6.2 Random and hash primitives (normative, needed for golden files)

- SP1 as in sting.md 6.2: `init(seed16, salt)`, xorshift32 `next()`, `rnd(n) = (next() >> 8) % n`, Fisher-Yates `shuffle`. Salts (new, one per stream): HITS `0x6D3B27A1`, VEL `0x13F5C8D9`,
  GATE `0x5E92B407`, NOTES (hop mask) `0x4A1C9E35`.
- `mix(x)` (a 32-bit integer hash, wraps mod 2^32): `x ^= x >> 16; x *= 0x7FEB352D; x ^= x >> 15; x *= 0x846CA68B; x ^= x >> 16; return x`.

### 6.3 Procedure

**Build** (main loop; pure; runs on any seed, morph, scale, key or octave change; the chord is NOT an input of the build, it is an input of the step):

1. HITS: `o = [1..15]; shuffle(o)`; `order = [0] + o` (cell 0 ranks first, SP2 `first_fixed`); `rank[order[i]] = i`; `hit[c] = (rank[c] < den)`, packed as `hit_bits` (u16).
2. VEL: stream `init(s_vel, VEL)`: `vel[c] = 50 + rnd(78)` for `c = 0..15`.
3. GATE: stream `init(s_gate, GATE)`: `gm[c] = 2 + rnd(5)` (2..6, mean 4) for `c = 0..15`.
4. HOP: if `s_notes != 0`: stream `init(s_notes, NOTES)`, `a = [0..15]; shuffle(a)`, `hop_bits` has bits `a[0..3]` set; else `hop_bits = 0`.
5. Copy `shp, spn, rat, dir, off, lg, dyn`, `s_notes`, `s_rot` into the cache.

**Step** (ISR; a pure function of `(cache, gp, hc)`; `gp` is the generator position the core supplies, ADR-0003 and SEQUENCER_UX 1.4; `hc` is the `HarmChord` of the step's start tick):

1. Rate: `RATE_STP = [4,2,1,1,1,1]` (steps per note slot), `RATE_SUB = [1,1,1,2,3,4]` (notes per step) indexed by `rat`.
2. Pool: `nt = hc.n`, `P = nt * spn`. Base length `LB = P` for UP, DOWN, RAND, ASPL, CHORD; `LB = max(1, 2P - 2)` for UPDN. Figure length `F` by shape: LINE `LB`; SKIP `2 LB`; THUMB `2 max(1, LB-1)`;
   OCTV `2 LB`; REPT `2 LB`; WAVE `3 LB`. Cycle in steps `cyc = ceil(F * STP / SUB)` (used for the figure cue, 6.6). `loop_len` returns 64: the output has no shorter period that holds for every morph, so the algorithm reduces nothing.
3. Position: `g = gp` (0..63, no reduction), `u = g + off`, `cell = g mod 16`. Note slot: the step is a slot iff `u mod STP = 0`; its note counter is `n0 = (u div STP) * SUB`; sub-note `k` (0..SUB-1) has counter `n = n0 + k`.
4. Note of counter `n` (`note_n`): `ntot = n + (s_rot mod F)`; `m = ntot mod F` (figure position); `y = ntot div F` (figure cycle). Shape gives a base position `b` (0..LB-1), a home flag and an octave bit:

   | SHAPE | `b` | extra |
   | --- | --- | --- |
   | LINE | `m` | |
   | SKIP | `m` even: `(m/2) mod LB`; `m` odd: `(m div 2 + 2) mod LB` | |
   | THUMB | `m` even: home; `m` odd: `w = (m div 2) mod (LB-1)`, `b = w` if `w < bh` else `w + 1` (`0` if `LB = 1`), where `bh` is the first figure position whose pool index is the **home note's** (owner O-5, 2026-11-12: the walking tones skip the home note, not blindly position 0; with the root as home and DIR UP `bh = 0` and this is the earlier `1 + w`) | home tone = `hc.drone` if `hc.drone >= 0`, else `hc.vt[0]` (the drone replaces the root as the home note) |
   | OCTV | `m div 2` | octave bit `m mod 2` (that note +12) |
   | REPT | `m div 2` | |
   | WAVE | `(m div 3 + m mod 3) mod LB` | |

   Direction maps `b` to a pool index `j`: UP, ASPL, CHORD `j = b`; DOWN `j = P - 1 - b`; UPDN `j = b` if `b < P` else `2P - 2 - b`; RAND: a seeded permutation of the pool, new every figure cycle: `x = mix(s_notes * 0x9E3779B1 xor y * 0x85EBCA6B)` (`1` if 0); run xorshift32 from state `x`, discard 2 draws;
   `perm = [0..P-1]; shuffle(perm)` with that stream; `j = perm[b mod P]`. It is recomputed on each call (at most 24 draws) and repeats exactly each pass of the loop.
5. Tone: the **tone list** `T[0..nt-1]` by direction (6.4): UP, DOWN, UPDN, RAND use `hc.vt` (voiced ascending); ASPL and CHORD below. Semitone `s = T[j mod nt] + 12 * (j div nt)` (+12 if the OCTV bit; the home flag replaces `s` by the home tone).
6. Hop: if `hop_bits` bit `(n mod 16)` is set and `spn >= 2`: `s += 12`. Fold: `while s > 60: s -= 12` (the 5-octave ceiling, MIDI 96 at key C octave 0). The note is `sq_note(g_global, s)` (SP9; chord tones are scale tones, so SP6 leaves them).
7. Output: `hit = hit_bits[cell] and (u mod STP = 0)`. `vel = dyn_vel(vel[cell], dyn)` (SP17), then `+12` (clamped to 127) when `m = 0` (the figure start is accented). `gate = clamp(((20 + 10 lg) * gm[cell]) / 4, 5, 100)`.
   `note` is the note of counter `n0`; valid also when `hit = 0` (the core needs it for a step turned ON by hand, ADR-0003 4 rule 3). If `hit` and `SUB > 1`: `PO_HIT` with `PO_REP` count `SUB - 1` and the core asks `sub_note(k)` for the
   pitch of repeat `k` = `note_n(n0 + k)` (6.10); the repeats share `vel` and `gate`. A rest step, or a step that is not a slot, has no repeats.

Because the lanes (`hit`, `vel`, `gm`) read `cell = g mod 16` and the figure reads `n`, a rest keeps its place in the figure (the note it would have played is skipped, not delayed): DENS and the HITS dice thin the
figure without shifting it. **[D]** The source's off steps do not advance its note engine [V, 04 4.2]; ours do, to keep every morph stateless.

### 6.4 What chord voicing the arp follows, and the ROLE of the channel

ARP follows `harm_chord_at_bar` through `HarmChord` (HARMONY.md 9): `vt[0..n-1]` is the **voiced chord**, which already contains VOICE (0..6: 1 to 5 tones), MOVE (placement: root position or the smoothest inversion
after the chord before) and DRONE (the drone tone is added to `vt` as one more voice, up to 6). So:

| HARMONY dial | What the arp does with it | Audible when |
| --- | --- | --- |
| VOICE 0..6 | sets `n` (1 to 5 chord tones, plus the drone) and so the pool: VOICE 1 is root and fifth, 3 adds the seventh, 4 the ninth, 5 and 6 spread open | always; the arp is the most direct way to hear VOICE |
| MOVE 0..8 | decides the octave placement of each chord's tones: 8 root position, 0 the smoothest inversion; the figure starts on the lowest voiced tone, so MOVE changes where each bar's figure starts | with UP, DOWN, UPDN, RAND (voiced order); ASPL starts at the root, CHORD ignores the placement (below) |
| DRONE 0..14 | the drone is a pool tone like any other (it walks with the figure); with SHAPE `THUMB` it replaces the chord root as the **home note** that alternates with the walking tones (a pedal tone under the arp) | any shape; THUMB makes it a true pedal |
| progression, BARS | the pool follows the chord of the bar (6.7) | at each bar line |

The tone list by direction (`T[i]`, `i = 0..nt-1`, semitones above the key, before `+12 (j div nt)`):
- UP, DOWN, UPDN, RAND: `T[i] = hc.vt[i]`, the voiced notes ascending.
- **ASPL (as played)**: the voiced notes, rotated so the chord **root** comes first, each wrapped note an octave higher: `r` = the lowest index with `(vt[r] - root) mod 12 = 0` (0 if none), `T[i] = vt[(r+i) mod nt] + 12 ((r+i) div nt)`.
  It equals UP in root position, and differs after a MOVE inversion or in an open voicing.
- **CHORD (chord order)**: the tones in the order of the chord stack, root position: sort `rel = (vt - root) mod 12` ascending, `T[i] = root + rel[i]` (so a ninth, `rel` 2, comes before the third). It ignores MOVE and the open spread.
  **[D]** The brief's "as played" and "chord order" name two source Note Orders [V, 03 3.1] that need held keys; these are their meaning for a generated chord.

Worked (MAJOR, chords I V I at MOVE 0, so the last I is the second inversion `4 7 12`; then VOICE 5 open ninth `0 7 11 14 16`), first 6 notes, SPAN 1, LINE:

| Voicing | UP | ASPL | CHORD |
| --- | --- | --- | --- |
| I, MOVE 0 (`4 7 12`) | `4 7 12 4 7 12` | `12 16 19 12 16 19` | `0 4 7 0 4 7` |
| I, VOICE 5 (`0 7 11 14 16`) | `0 7 11 14 16` | `0 7 11 14 16` | `0 2 4 7 11` |

**ROLE of an ARP channel (HARMONY.md 3.5).** ARP is chord-based by construction, so the chord stage `harm_apply` is **bypassed** for it (`AF_CHORD`); the ROLE (a HARMONY page setting per channel) picks how much of HARMONY the arp hears:

| ROLE | The arp pool is |
| --- | --- |
| `AUTO` (default) and `FULL` | the voiced chord of the bar, drone included (the two are the same for ARP; the status line reads the role as set) |
| `OFF` | a fixed reference chord: degree 1, VOICE 2, no drone, whatever the progression says (the arp stays put while the other channels follow the chords) |
| `ROOT` | the chord root only (the drone if set), so the figure is an octave figure over the root |

Entered steps: a step with PITCH set is scale-quantised and never moved (HARMONY.md 1.2); its note replaces the arp note at that step only, and the figure does not shift (`n` is a function of the position).

### 6.5 Direction, shape and rate sets

**Direction** (S4 `DIR`), pool of `P` tones (the examples: chord I of MINPEN `0 3 7`, SPAN 2, so the pool is `0 3 7 12 15 19`, first 14 notes):

| DIR | Rule | Notes |
| --- | --- | --- |
| 0 `UP` | `j = b` | `0 3 7 12 15 19 0 3 7 12 15 19 0 3` |
| 1 `DOWN` | `j = P-1-b` | `19 15 12 7 3 0 19 15 12 7 3 0 19 15` |
| 2 `UPDN` | up, then down without repeating the ends (cycle `2P-2` = 10) | `0 3 7 12 15 19 15 12 7 3 0 3 7 12` |
| 3 `RAND` | a seeded permutation of the pool, new every cycle (6.3 step 4); with `s_notes` 0 the hash still gives a fixed order | `7 19 0 12 3 15 19 3 7 15 12 0 7 12` |
| 4 `ASPL` | root first (above) | as UP in root position |
| 5 `CHORD` | stack order (above) | as UP for a triad in root position |

**Shape** (K2 `SHAPE`), applied to the UP base of a triad `0 3 7` at SPAN 1 (one full figure each):

| SHAPE | Figure length `F` | Notes | Feel |
| --- | --- | --- | --- |
| 0 `LINE` | 3 | `0 3 7` | the plain arpeggio |
| 1 `SKIP` | 6 | `0 7 3 0 7 3` | leaps (tone, tone+2, tone+1, ...) |
| 2 `THUMB` | 4 | `0 3 0 7` | the home note alternates with each tone: a pedal bass line |
| 3 `OCTV` | 6 | `0 12 3 15 7 19` | each tone then its octave |
| 4 `REPT` | 6 | `0 0 3 3 7 7` | each tone twice, a stutter |
| 5 `WAVE` | 9 | `0 3 7 3 7 0 7 0 3` | three-note windows rolling along the cycle |

**Rate** (K4 `RATE`), relative to the **global step**, which is the global DIV (SET page): the note slot is the step divided or multiplied, so the global triplet DIVs and swing still apply and RATE multiplies them.

| RATE | Label | Steps per note / notes per step | At DIV 1/16 | At DIV 1/8 | Behaviour |
| --- | --- | --- | --- | --- | --- |
| 0 | `1/4` | 4 / 1 | quarter notes | half notes | a note only on steps `u mod 4 = 0` |
| 1 | `1/2` | 2 / 1 | eighth notes | quarter notes | every second step |
| 2 | `1X` | 1 / 1 | sixteenth notes | eighth notes | one figure note per step (default) |
| 3 | `2X` | 1 / 2 | 32nd notes | sixteenth notes | two evenly spaced notes per step (the second via `sub_note`) |
| 4 | `3T` | 1 / 3 | three notes per 16th (a triplet burst) | three notes per 8th (16th triplets) | three evenly spaced notes per step |
| 5 | `4X` | 1 / 4 | 64th notes | 32nd notes | four notes per step |

The triplet feel between steps comes from the global DIV (`1/16T`, `1/8T`); `3T` is the triplet inside one step (a 3-note chord played as a triplet). Examples, chord I, SPAN 1, UP, DENS 16, first 6 steps:
`1X` `0 3 7 0 3 7`; `1/2` `0 - 3 - 7 -`; `2X` `0+3  7+0  3+7  0+3  7+0  3+7` (each step plays its first note and then the repeat); `3T` `0+3/7` every step; `4X` `0+3/7/0  3+7/0/3  7+0/3/7 ...`.
The step edit layer's RATCH (SEQUENCER_UX 9.2) on an ARP step plays that many figure notes (the repeats walk the figure) instead of repeating one pitch.

### 6.6 Pattern length 1..64 against the arp cycle

- The channel loop resets the generator (SEQUENCER_UX 1.4): at the loop end `gp` returns to 0, so **the figure restarts with the loop**. At the default LENGTH 16 and DIV 1/16 the loop is exactly one bar, so every bar starts the figure on its first note
  and the chord changes line up with figure starts. A figure that does not divide the loop is cut at the loop end (the same plain consequence as SNAKE's polymeter): the default figure is 6 notes (`0 3 7 12 15 19`) and the 16-step bar plays it twice and a third time as `0 3 7 12`.
- **Figure cue.** Turning K2, K3, K4 or S4 toasts the figure length in steps `cyc` (6.3 step 2): `FIGURE 6 STEPS`; when `cyc` does not divide LENGTH the toast is `FIGURE 6: LENGTH 16 CUTS IT`. The producer who wants the figure whole sets LENGTH to the figure length or a multiple of it (CHAN page K1: for example 10 for the 10 step figure of UPDN over a triad at SPAN 2; a multiple above 16 needs the later pages OWN).
- **Pages.** `loop_len` returns 64 and `step` takes `g` unreduced, so the page-source rules of SEQUENCER_UX 1.4 decide: a page that reads an earlier page (the default for a new page, `P1`) plays the same positions and so restarts the figure at its start; an OWN page plays
  `g` 16..31 and the figure **continues** (a 10 step figure on LENGTH 20 with page 2 OWN plays two whole figures). The lanes (rests, velocity, length) read `g mod 16`, so they repeat every 16 steps whatever the page. The heard-span cue of STAGE `TOTAL` does not apply to ARP (no knob sets a loop length; the figure toast replaces it).
- Extremes: the shortest figure is 1 note (VOICE 0, SPAN 1, LINE); the longest is 138 (6 tones, SPAN 4, UPDN, WAVE), cut to 64 by the loop.

### 6.7 Behaviour on a chord change mid-pattern

HARMONY changes the chord at the bar line only (HARMONY.md 2.3), and the core hands each step the chord of the step's start tick (also for the one-step lookahead). ARP is stateless, so:
- from the first step of the new bar the **pool** is the new chord's: the figure position `m` continues (it is a function of the pattern position, not of the chord), so the same figure walks the new notes. Example (MINPEN `1451`, defaults, ch 0, midi note = 36 + s), the 16 steps of each bar:

| Bar | Chord | Notes (`-` = rest) |
| --- | --- | --- |
| 1 | I (`0 3 7`) | `36 - 43 48 51 55 36 39 - 48 51 55 36 39 - -` |
| 2 | iv/G (`7 10 15`) | `43 - 51 55 58 63 43 46 - 55 58 63 43 46 - -` |
| 3 | V/Bb (`10 15 17`) | `46 - 53 58 63 65 46 51 - 58 63 65 46 51 - -` |
| 4 | I | as bar 1 |

- A change of VOICE, MOVE, DRONE or PROG made on the HARMONY page lands on the next 48-tick step grid for the dials (the dial flip of HARMONY.md 2.5) and at the next bar for the progression, so the arp changes under the player's hand.
- Notes already sounding ring out to their queued note-offs (decision 47); nothing is cut at the bar line.
- With the pool length changing (VOICE 2 to 3 makes `n` 3 to 4) the figure length `F` changes and with it `cyc` and `m`: the figure re-aligns from the pattern position at that step (no state to repair).

### 6.8 A held single note: HARMONY `1----`

The owner's `1----` is one chord held for five bars (`nchords = 1`, `bars = 5`; the default is `1---`, HARMONY.md 2.2). There is no chord change, so the pool is constant for the whole phrase: ARP is a plain looping arpeggio of that one chord.
With **VOICE 0** the chord is one note (the root, or the root plus the drone), which is the held single note:

| Case (pool `P`) | Result (notes, semitones above the key; SPAN 3 unless stated) |
| --- | --- |
| VOICE 0, SPAN 1, any direction and any shape but OCTV | `0 0 0 0 ...`: a re-triggered pedal; DENS, LEGATO, DYN and the dice still shape it (an acid 16th pedal). OCTV alternates `0 12 0 12` (found by the P2-017 golden; the earlier "any shape" was wrong for it) |
| VOICE 0, LINE, UP, SPAN 3 | `0 12 24` repeating (octave run) |
| VOICE 0, OCTV, SPAN 3 | `0 12 12 24 24 36` (octave bounce) |
| VOICE 0, THUMB, SPAN 3 | `0 12 0 24` (root against the octaves) |
| VOICE 0, UPDN, SPAN 3 | `0 12 24 12` (cycle 4) |
| VOICE 0 with a drone (pool = root and drone) | the two tones alternate in the chosen order; with THUMB the drone is the home and the root walks: `7 0 7 0 ...` (golden `same, THUMB`; owner O-5) |

With a **chord** held (VOICE 2 or more) the same shapes apply to the chord's tones. Because HARMONY is inert at `1---` with VOICE 2 and no drone, ARP then plays the key's own triad; the H-INERT rule of HARMONY.md 10 concerns the other four algorithms and is unaffected.

### 6.9 What we drop from the source, and why

| Source feature | Decision |
| --- | --- |
| Held-note input, Hold, Key Retrig, Lock | **Dropped**: no held keys exist on a channel; HARMONY holds the chord. A "latch" is the progression itself (the chord stays until it is changed) |
| Chord Arp (the whole chord per trigger) | **Dropped**: a channel is monophonic. The `3T` and `4X` bursts give a chord-like roll |
| Note Order `Played`/`Follow`; Octave Mode `Random` | Replaced by DIR (6.5): ASPL and CHORD are the generated-chord readings; random octaves break rule 4, so RAND is a seeded order and octave hops come from the NOTES dice |
| Transpose lane, Map/CC lane, per-step Chance lane | **Dropped**: more lanes than knobs (rule 1); the edit layer gives per-step pitch, PROB and RATCH |
| Division, Dotted/Triplet, Swing, Sync/Freq, BPM | **Dropped**: global DIV and SWING; RATE multiplies the global step |
| StepDirection, LoopLength | **Dropped**: DIR and the channel LENGTH (CHAN K1) |
| Humanise (timing jitter) | **Dropped as a morph**: DYN and the three seeded lanes (rests, velocity, length) give a human feel deterministically; timing jitter would need a core feature and break byte-identical replay. A later core "humanise" would be a table row |

### 6.10 Changes ARP needs in the core (an ADR-0003 amendment, scheduled in 11)

ARP is the first algorithm that reads the chord and the first whose repeats change pitch. The vtable of ADR-0003 4 needs four additions; none changes a struct size:

| # | Change | Why |
| --- | --- | --- |
| AM-1 | `step(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out)`: a third argument, `NULL` unless `AlgoDesc.flags & AF_CHORD` (`AF_CHORD = 0x02`). The other algorithms ignore it | The chord changes every bar, the cache is built only on a morph; 8 chords do not fit the 64 B cache |
| AM-2 | `AlgoDesc.sub_note(cache, g, k, hc)` (`uint8_t`, final MIDI note of repeat `k`, 1..3; `NULL` for the others) used by the core whenever a step has `PO_REP`, from the algorithm's rate or from a step RATCH edit | Repeats of EUCLID/STAGE re-fire one pitch (SP5); an arp repeat is the next figure note |
| AM-3 | The core skips `harm_apply` (HARMONY.md 4.1) for an algorithm with `AF_CHORD`; it fills `hc` from `HarmRT` (current bar) or `harm_chord_at_bar(bar + 1)` (lookahead), applies the channel ROLE of 6.4 (OFF reference chord, ROOT root only) before the call | ARP's notes are chord tones by construction |
| AM-4 | `SQ_NALGO` 4 to 6 (owner decision 82, shared with PATTERNS), `ALGOS[4] = ARP`; preset version bump with P2-014's | The algorithm list |

The purity contract (ADR-0003 4) is kept with `hc` as a third pure input: `step` still uses only `(cache, g, hc)` and `const` tables. The host test varies `hc` and checks the output depends on nothing else.

### 6.12 The AlgoDesc table (the one `proto.c` reads; P2-017)

`run_protos.sh` extracts the block between the two marker comments from this file and from `proto.c` and requires them to be identical: a remap (a knob, a dice button, a range, a default, a label) is an edit to this block and the same edit in the firmware copy, nothing else. `SQ_SEED_MOD` is the seed id of `s_mod` (MODULATION 3.4; stored in `ModSave`, not in `AlgoState`).

```c
/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_ARP = {
    .name = "ARP", .id = SQ_ALGO_ARP, .n_seed = 5, .flags = AF_CHORD, .cache_bytes = 48,
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 },
    .seed_k = { 0xFF, 2, 3, 4, 0xFF },
    .morph = {
        { "DENS",   0, 16, 12, 1, 1 },
        { "SHAPE",  0,  5,  0, 1, 1 },
        { "SPAN",   1,  4,  2, 1, 1 },
        { "RATE",   0,  5,  2, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "LEGATO", 0,  8,  3, 1, 1 },
        { "DYN",    0, 10, 10, 1, 1 },
        { "DIR",    0,  5,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {2}}, {1, {3}}, {2, {4, SQ_SEED_MOD}} },
    .sub = { "VARY", "RESTS", "VELO", "LENGTH", "PHASE" },
    .build = arp_build, .loop_len = arp_loop_len, .step = arp_step, .hi = arp_hi, .sub_note = arp_sub_note, .mod = arp_mod,
};
/* ALGODESC-END */
```

### 6.11 Flex points (ADR-0004 addendum): what is a table edit, what is not

Remappable by a table edit, no algorithm code change: which knob drives which morph (`knob_morph`), every label, range and default of 6.1, the rate table `RATE_STP/RATE_SUB`, the dice button of each layer and the sub-labels, the velocity range 50..127, the gate factor range 2..6, the accent bonus 12,
the number of hop positions (4), the fold ceiling 60, the salts' *table* (not their values), the default seeds.
Not changeable without algorithm code: the shape and direction rules of 6.3 (they define the golden files), `mix`, the stateless counter, the `cell = g mod 16` lane clock, the sub-note mechanism of AM-2, the chord interface.
A new shape or direction is a new case of the `switch` plus new goldens (one owner re-lock).

### 6.12a The mod lanes (P2-017, MODULATION 4)

`AlgoDesc.mod` is `arp_mod`: lane A (slots 1 and 3) is the cell of the **figure position** `m` of the step's first note (0..F-1, up to 137, so `sp18_cell` takes the whole `uint8_t`, not `i & 15`), leaning on the step's velocity; lane B (slots 2 and 4)
is an LFO over the channel LENGTH (`ModIdx.period = 0`: the core fills it), read at `g`. Both are pure functions of `(cache, g, hc, step)`. The golden shows lanes for LENGTH 16.

**Decided (owner 2026-11-12, option A):** the SHAPE sub-label stays the primary name (`PHASE`); the dice toast adds `+ MOD` when the mod seed moves too (an active device slot). MODULATION.md 3.4's `OCT+MOD` style labels are not used.

### 6.13 What the prototype changed in this design (found while writing it; each is a table or text edit, none moves a control)

| # | Finding | Resolution |
| --- | --- | --- |
| 1 | `step()` has no `SeqGlobal`, so `sq_note` (SP9) cannot run in it | Chord tones are scale tones, so SP6 would leave them alone: the cache keeps `base = 36 + key + 12 * octave` (the spare pad byte, still 48 B) and the note is `base + s`. The cache does not depend on scale, gate, DIV or swing (purity test (b)) |
| 2 | `hi(m)` of ADR-0003 section 20 for ARP | 60 semitones, the fold ceiling (no snap is added: the chord tones are scale tones). P2-029 writes it into section 20 |
| 3 | SPEC 6.8 said "any shape" gives a plain pedal at VOICE 0, SPAN 1 | OCTV alternates the root and its octave; fixed in 6.8 and tested |
| 4 | The brief's cost bounds (section 9) | Structural, asserted: cache 48 B; the RAND shuffle is at most 24 draws; the CHORD sort at most 6 values; no loop is longer. The golden has an `## ARP COST` line |
| 5 | ROLE (6.4) is the core's job (AM-3) | The driver's `role_chords()` is the reference of what the core hands `step()`: AUTO and FULL the voiced chord, OFF the reference chord (degree 1, VOICE 2, no drone), ROOT the root (the drone if set) only. The golden `HARM` sections are those four runs |
| 7 | THUMB with a drone played the drone on every step (the walking tones skipped pool index 0 = the root) | Owner O-5 (2026-11-12): skip the **home** note's pool index. `proto.c` finds the home's index in the tone list and the figure position that reads it; `arp_ref.py` re-implements it. Only the THUMB lines of the grids (non-UP directions) and the drone case changed in the golden |
| 6 | The chord run of the driver is a test approximation | ARP is `AF_CHORD`, so the core bypasses `harm_apply` and there is no strong-step rule for ARP at all; the approximation of the other four SPECs (velocity >= 100 and holds) does not apply |

## 7. UX: roles, labels and gestures

### 7.1 Knob and button roles (PERF page, ARP selected)

| Control | Role | On-screen label | Gestures |
| --- | --- | --- | --- |
| K1 (ROW 1) | DENS | `DENS 12` | 1 (performance) |
| K2 | SHAPE | `SHAPE LINE` | 1; toast `FIGURE n STEPS` |
| K3 | SPAN | `SPAN 2 OCT` | 1 |
| K4 | RATE | `RATE 1X` | 1 |
| S1 (OCT+ latch, or hold OCT+ and turn) | OFFSET | `OFFSET 0` | 1 (held) / 2 (latched) |
| S2 | LEGATO | `LEGATO 3` | same |
| S3 | DYN | `DYN 10` | same |
| S4 | DIR | `DIR UP` | same |
| FX / SCL / SEQ / ENV / ARP / LFO (tap) | dice ALL / NOTES / HITS / ACCNT / ARTIC / SHAPE | `ALL` `NOTES` `HITS` `ACCNT` `ARTIC` `SHAPE`, sub-labels `VARY` `RESTS` `VELO` `LENGTH` `PHASE` | 1 |
| EDIT tap | undo / redo the last dice or morph record | `UNDO` | 1 |
| ALGO turn | pick the algorithm; the picker cue (7.3) names all six | `ALGO` | 1 |
| hold GLO, then K3 / K4 / OCT- / OCT+ / SCL / LFO / ARP | VOICE, MOVE, DRONE, PROG dice, BARS, ROLE of the chord under the arp | HARMONY page labels (HARMONY.md 5.2) | 2 |

PERF screen additions (ARP selected): the chord chip (HARMONY.md 5.4) is drawn **always**, not only while HARMONY is active, because the arp follows the chord (`I`, `iv`, `V`, `I`); it is the only visual change. The lane draws the sounding steps; the accent glyph `!` marks
figure starts whose velocity reaches the accent threshold, so the figure's phase is visible.

### 7.2 Performance gestures (each one control, the screen shows the new value in the same frame)

| Gesture | What it does | Class and L rubric (V/I/O/L = total) |
| --- | --- | --- |
| Turn K4 RATE through `1/2` `1X` `2X` `3T` `4X` | slows to eighths or builds to a roll | P: 3/3/3/3 = 12 (a ROW 1 knob, F08) |
| Turn K2 SHAPE | the figure changes on the next step (LINE to THUMB gives a pedal bass) | P: 3/3/3/3 = 12 |
| Turn K3 SPAN | the run widens or tightens | P: 3/3/3/3 = 12 |
| Turn K1 DENS | thins or fills | P: 3/3/3/3 = 12 |
| Hold OCT+ and turn K4 (DIR) | flips the order: up, down, up-down, random, as played, chord order | P: 3/3/3/2 = 11 (the momentary ROW 2 of FULL_CONTROL 1.2, the OCT+ role is labelled on screen) |
| Press SCL (NOTES dice) | octave hops and random order change | P: 3/3/3/2 = 11 (F09) |
| Press SEQ / ENV / ARP / LFO | new rests / velocities / lengths / phase | P: 3/3/3/2 = 11 each (F10 to F13) |
| Press FX (dice all) then EDIT (undo) | a new arpeggio, or back | P: 3/3/3/2 = 11 each (F14, F15) |
| Hold GLO, turn K3 (VOICE) or K4 (MOVE), or tap OCT+ (DRONE) | the chord under the arp changes (seventh, ninth, inversion, a pedal tone) | P: 3/3/3/2 = 11 (HARMONY U77); the arp is the audible result |
| Hold GLO, press SCL (PROG) | a new progression; the arp follows from the next bar | P: 3/3/3/2 = 11 (HARMONY U78) |
| Turn ALGO to ARP | the arp sounds on the next step | P: 3/3/3/3 = 12 (F03) |
| Turn K2/K3/K4/S4 and read the toast | know the figure length | display: 3/3/3/3 = 12 |

All pass the performance rule (11, no sub-score below 2). The ROW 2 dial-ins (OFFSET, LEGATO, DYN as setup, and DIR as above) are 2 gestures latched or 1 held, class S, pass at 10 (F09).

### 7.3 The algorithm picker with six algorithms

Choosing the algorithm is the ALGO encoder (control map `EN_ALGO`): each detent renames the selected channel's algorithm in the channel cell (Z1 line 1) and the header slot H4, and for 2 s after the last detent **the cue strip lists all six** with the current one in brackets.
The channel names are the 4-letter short forms (`ACID` `EUCL` `SNAK` `STAG` `ARP` `PATT`; `PATT` is the placeholder short form for PATTERNS until P2-014 fixes its own, it must stay within 4 letters). Measured with `tests/design/width_probe.sh` (real `text_w`, AF_S):

```
 H4 (76 px)   | 1 ARP M2   | worst case six: 1 PATT MOFF = 71 px ; 1 PATT M16 = 64 px
 CHB cell     | [1 ARP  ]  | 1 PATT = 37 px in the 60 px cell (ink 56)
 CUE (240 px) | ACID EUCL SNAK STAG [ARP] PATT   (185 px)
 K1R          | DENS 12 | SHAPE LINE | SPAN 2 OCT | RATE 1X
 K2R          | OFFSET 0 | LEGATO 3 | DYN 10 | DIR UP
```

The six names fit the header, the cell and the strip with at least 5 px to spare in H4 and 55 px in the strip; the longest ARP value text is `2 OCT`/`THUMB`/`CHORD` at most 41 px in the 51 px knob cell, and the label `LEGATO` is 43 px. All strings are in `tests/design/widths.tsv`. The picker is the existing encoder (one gesture,
F03), so no new control or page is added; the strip is an overlay state `ALGO_PICK` (page_states.tsv).

## 8. Risks and open questions

| # | Item | Treatment |
| --- | --- | --- |
| 1 | The brief's "as played" and "chord order" mean held-key behaviours that a generated chord lacks | Defined in 6.4 as root-first and root-position stack order; the names are table text; the owner may rename or replace them (owner question below) |
| 2 | The figure restarts with the loop, not with the chord: at LENGTH 16 and DIV 1/16 they coincide, at other lengths the figure may start mid-chord | By design (stateless, ADR-0003 loop rule); the `FIGURE n` cue shows the cut; a "restart at the chord" option would need chord-age in the core |
| 3 | AM-1 to AM-3 change the core vtable | Listed for the P2-005 planning and the ADR-0003 amendment; the golden files fix the behaviour |
| 4 | `3T` is a triplet inside one step, not a triplet between steps | Between steps the global DIV `1/16T` / `1/8T` does it; documented in 6.5 |
| 5 | Hardware feel: default DENS 12, 2 octaves, 50 % gate on a TB-3 | All are table values (6.11); hardware test HW-ARP in P2-008's scope |
| 6 | PATTERNS (P2-014) fixes its own short name; the picker strip width assumes 4 letters | If it uses more, the strip (240 px) still holds up to 7 letters; `width_probe.sh` is rerun |
| 7 | Owner question: the DIR label set (`ASPL`, `CHORD`) and the SHAPE names | Recorded in the task log for the owner; renames are table edits |

## 9. Cost: ChanRT, RAM and pool

| Item | Bytes | Where |
| --- | ---: | --- |
| `ChanRT` | 0 added (stays 24 B): ARP is stateless | .bss |
| ARP cache: `hit_bits` 2, `vel[16]` 16, `gm[16]` 16, `hop_bits` 2, `s_notes` 2, `s_rot` 2, `shp spn rat dir off lg dyn` 7, pad 1 | 48 (cache max 64) | the per-channel cache already allocated |
| `AlgoState` for the fifth algorithm | +20 per channel = +80 in the live preset | .bss |
| `SeqPreset` | 1396 to 1476 B with ARP alone; **1556 B with ARP and PATTERNS** (`ChanSave` holds six `AlgoState`, 384 B; P2-017 `sq_types.h`); the 18 preset-sized pool copies (HARMONY.md 7.2) | +2880 B pool for both (ARP half 1440 B) |
| `HarmChord` argument on the ISR stack | 16 | stack |
| Flash | rate tables 12 B, shape and direction code about 1.5 KB | flash |
| CPU: build | about 120 draws and a 16-shuffle, under 400 integer operations, main loop | |
| CPU: step | 3 divisions, 1 sort of at most 6 values (CHORD only), a seeded 24-element shuffle (RAND only, at most about 400 operations), otherwise under 150 operations; `harm_chord_at_bar` is paid once per bar by the core | ISR |

RAM headroom after the removals is about 46 KB (HANDOFF budget): ARP's +80 B .bss is far inside the 4 KB floor. The pool cost of 1440 B is inside the pool free figure after P3 (about 218 KB, PROGRAM_PLAN 5).

## 10. Worked examples (informative; the P2-008 goldens supersede them)

### 10.1 The default ARP on channel 1 (MINPEN, key C, octave 0, HARMONY default `1---`, DENS 12 SHAPE LINE SPAN 2 RATE 1X DIR UP)

Chord I = `0 3 7`, pool `0 3 7 12 15 19`, `F = 6`, `cyc 6`. Hits are steps 0, 2..7, 9..13 (rank order of `s_hits` 29882). Output (step: MIDI note, velocity, gate %):
`0: 36 v109 g37`, `2: 43 v88 g62`, `3: 48 v88 g50`, `4: 51 v126 g62`, `5: 55 v90 g62`, `6: 36 v120 g37`, `7: 39 v123 g62`, `9: 48 v74 g50`, `10: 51 v83 g62`, `11: 55 v72 g37`, `12: 36 v115 g25`, `13: 39 v119 g25` (steps 1, 8, 14, 15 rest; velocities
at steps 0 and 6 include the figure-start accent `+12`).

### 10.2 Morph and dice moves from 10.1 (bar 1, MIDI notes at the sounding steps)

| Change | Result |
| --- | --- |
| K2 to `OCTV` | `36 - 39 51 43 55 48 60 - 63 55 67 36 48 - -` (each tone then its octave; `F = 12`, `cyc 12`) |
| then K4 to `2X` | the sounding steps play `36+48`, `43+55`, `48+60`, ... (each step's second note after the plus; `cyc 12/2 = 6`) |
| S4 `DIR` to `UPDN` | `36 - 43 48 51 55 51 48 - 39 36 39 43 48 - -` (`F = 10`) |
| SCL (NOTES dice) with `s_notes` 0x1234 | `36 - 55 48 51 55 36 51 - 60 51 55 36 39 - -` (hops at note counters 2, 7 and 9, the three of the four hop positions that fall on a sounding step) |
| K2 `SHAPE` back to `LINE`, then HARMONY VOICE 3 | pool `0 3 7 10` (`n = 4`, `F = 8`): `36 - 43 46 48 51 55 58 - 39 43 46 48 51 - -` |

### 10.3 Direction, shape, rate and single-note tables

See 6.4, 6.5 and 6.8 (all computed with the same transcription).

## 11. Hand-off to the dev tasks and to P2-008

- **Delivered by P2-017:** `proto.c`, `golden.txt` (the matrix of the driver with the chord of each bar, plus the sections of `arp_extra.h`: the default of 10.1 checked against its twelve values, the two DIR x SHAPE x RATE x SPAN grids, the single-note cases, the RAND property, the cost line) and `arp_ref.py`.
- **P2-008 scope addition (scheduled by this task, backlog/tasks/P2-008):** the reference prototype and golden files for ARP: `docs/design/algorithms/arp/` build/step/sub_note in the same Python style as the other four, the HARMONY chord from `harmony_proto.py`, goldens for (a) the default of 10.1, (b) every DIR x SHAPE x RATE
  combination at SPAN 1..4 for chord I and for the `1451` bars of 6.7, (c) the single-note cases of 6.8, (d) ROLE OFF/ROOT, (e) each dice layer from a fixed seed, (f) the RAND permutation property (a permutation of 0..P-1 for every `P` 1..24 and cycle `y`), (g) the cache size 48 B and the cost bounds of 9.
- **Dev tasks that cite this document:** the ARP `step/build/sub_note` (needs AM-1 to AM-4 first), the table rows (knob_map, help_text, ALGORITHM_SET), the picker overlay. Each cites 6.1, 6.3, 6.5 and S19.
- **Locked lines this changes** (each one): ALGORITHM_SET (v1 list is six, the ARP layer row, sub-label, knob rows, the question table); knob_map.tsv (8 PERF rows ARP); control_map.tsv / knob_map.tsv / CONTROL_MAP.md `EN_ALGO` text (six names); help_text.tsv `EN_ALGO` line 2
  (`ACID EUCL SNAK STAG ARP PATT`, via `gen_help.py`); page_states.tsv (`ALGO_PICK` overlay); SCREEN_SPEC (the picker strip, the always-on chord chip for ARP); tests/design/widths.tsv; scenarios S09 to S13 regenerated (EN_ALGO text), S19 added; ADR-0003 (AM-1 to AM-4, to be written into
  the amendment by P2-005); HARMONY.md 9 is **unchanged** (ARP only calls the interface).
- **Owner decisions needed** (recorded in the task log): the DIR names `ASPL` / `CHORD` and their meanings (6.4); the default DENS 12; whether the figure should restart at each chord (risk 2); the always-on chord chip for ARP (7.1).

## 12. Recommendation

**INCLUDE in v1** as the fifth algorithm (owner decision 82). ARP is the one algorithm in which the harmony the producer sets on the HARMONY page is directly audible and performable, with a morph set (how many, which figure, how wide, how fast; where it starts, how long, how hard, which way) that
maps one to one onto the panel. It adds no ChanRT, 48 B of cache, 20 B per channel in the preset and four small core additions. All source behaviour that needs held keys is replaced by the chord, and nothing relies on a ⚠ claim of the source. The residual risks are musical (the defaults on a TB-3) and
the four core additions, both table-able or scheduled.
