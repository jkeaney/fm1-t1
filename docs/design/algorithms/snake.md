# Assessment: Snake (polymetric grid sequencer)

> Change note (P1-013, owner decision "Shift"): this algorithm now has **8 morphs** (ROW 1 K1..K4 and ROW 2 S1..S4, see docs/design/FULL_CONTROL.md). Section 6 below is updated: the morph labels and slots follow UX_REVIEW 5.1 (they had not been applied yet), 6.1 lists all 8, 6.3b adds the ROW 2 procedure, 6.4 the RAM, 6.6 the flex points. Every ROW 2 default is neutral, so the base-row output and golden files are unchanged.

Source spec: `docs/snake-algorithms/` (read all of it). Vision: docs/project/VISION.md.
Use the source's confidence tags ([V]/[I]/⚠) when you rely on a claim, and don't promote ⚠ to fact.

Design stance: a **redesign, not a port**, in the same style as `sting.md` (the worked example). Wherever the source
is ⚠ or has no random generator at all (Snake's source is a hand-edited performance sequencer; its only generator is
a uniform "Random" button [V, 05 §5.1]), this document defines the rule here, marked **[D]** (our design decision,
no fidelity claim). Everything is written so a host reference implementation (ADR-0004 item 2) can be built without
choices. Layers, morphs and dice are declared as data tables (ADR-0004 addendum); the tables are in section 6.

## 1. What it does (5 lines max)

Snake stores 16 values per lane in a 4x4 grid and plays them in the order of a **traversal shape** (one of 14
permutations: rows, columns, serpentines, diagonals, spirals) [V]. Three lanes (Gates = the only trigger, Notes =
pitch, Velocities) each run their own clock and length, so they drift against each other as a polymeter [V].
Pitch is stored as a continuous value and quantised late through the scale [V]. In this sequencer it becomes the
**SNAKE** algorithm: 4 dice layers, 4 knob morphs (path shape, pitch-loop length, density, pitch range), all
deterministic from four seeds.

## 2. Generators inside it
| Generator | Inputs | Output | Deterministic? (seeded / free-running) |
| --- | --- | --- | --- |
| Shape table + inverse [V] | shape 1..14, step number 1..16 | `order[s]` = which of the 16 cells speaks at step `s` | Pure lookup, no randomness (14 x 16 constant table, 224 B) |
| Random, Notes lane [V] | range lo..hi, `rand_int(127)` per cell | 16 continuous values 0..<84 | Free-running in source; **seeded and redefined [D]** as the NOTES layer (16 x u8, 0..255) |
| Random, Gates lane [V] | `random 2` per cell, p = 0.5 | 16 on/off toggles | Free-running in source; **replaced [D]** by a seeded rank order + DENS morph (monotonic, reversible) |
| Random, Velocities lane [V] | range lo..hi | 16 values 0..127 | Free-running in source; **seeded [D]** as the VEL layer, fixed range 50..127 |
| Per-lane clock: Length, Speed, Direction, Shift [V structure, ⚠ `live.step` exact semantics] | transport, Length 1..16 | step number 1..16 per lane | Pure function of the step counter. **Only Length is kept** (Notes lane via the LEN morph; Velocities lane via the POLY layer); Gates lane fixed at 16 |
| Quantiser: late dial value -> scale degree [V, formula verified against the 16 stored defaults] | value, scale table, root | MIDI note | Pure function. Redefined **[D]** in integer form (6.3) keeping the principle: fixed octave span regardless of scale size |
| Scale tables (37) [V] | index | degrees | **Dropped**; the global scale control supplies the scale |
| Swing, Note length [V structure, ⚠ units, ⚠ `Direction_convert` name] | swing 0..1, length 64n..4n | delay, note-off | **Dropped**; the global GATE and SWING settings (owner decision 4) |
| Custom lanes x3 (modulation of Live parameters) [V structure] | 16 values, target parameter | parameter writes, no notes | **Dropped** (not a note algorithm; Live API only). See 6 and 7 |
| MIDI-in control (root, User scale, Random All) [V structure, ⚠ octave names] | notes 48 / 60-71 / 72-83 | root, scale, re-roll | **Dropped**; root from the global scale control, re-roll is the dice-all gesture |
| Reset (per lane) [I, ⚠ what it restores] | button | defaults | **Dropped** (undo-dice replaces it) |

## 3. Layers (what can be diced independently)

Four layers, each with its own 16-bit seed. In the source the three note lanes are already independent stores
[V, 01 Architecture, "lane independence"]; this design gives each its own seed so one can be re-rolled alone. The
shape is deliberately **not** a dice layer (it is knob 1, a deliberate choice, not a random one; 6.6 lists how to add
a PATH dice by table edit). The on-screen name is the dice label (max 6 chars).

| Layer (screen label) | What dicing it changes | What stays fixed |
| --- | --- | --- |
| NOTES (`NOTES`) | The 16 raw pitch values of the pitch lane: a new melody on the same grid. | Which steps trigger (GATES), velocities, lane lengths, shape, all four knob values, scale, root |
| GATES (`GATES`) | The priority order in which the 16 cells are switched on as DENS rises, so which cells are rests and therefore where the rhythm falls. | Pitches, velocities, lane lengths, shape, knob values |
| VEL (`VEL`) | The 16 velocity values (50..127) of the velocity lane. | Pitches, rhythm, lane lengths, shape, knob values |
| POLY (`POLY`) | The velocity lane's loop length (3..12), changing how the dynamics drift against the 16-step rhythm and the pitch loop. | Pitches, rhythm, velocity values, pitch-loop length (LEN knob), shape |

A fifth action, **dice all**, re-rolls all four seeds in one gesture (which physical button does it is the control
map's job). **Undo dice**: each layer keeps its previous seed (2 bytes), so the last dice of each layer can be
recalled once (VISION glossary: "the previous seed can be recalled").

## 4. Morph candidates (continuous knob transforms)

Every morph is a **pure function of (seeds, knob values)**: the random content is fixed by the seeds and the knob only
chooses how much of it to apply or how to read it. Turning a knob back restores the previous pattern exactly (VISION
glossary: "undoable by turning back"). Cost figures are for recomputing the caches after a knob detent (main loop,
not the ISR).

| Morph | Range | Musical effect | Cost per step (rough) |
| --- | --- | --- | --- |
| PATH (shape `shp`) | 1..14, default 7 | Chooses the traversal table. Same 16 stored values and same rests, read in a new order, so melody and rhythm re-arrange at once; contiguous shapes (5-8, 11-14) step to neighbouring cells, raster/jumping shapes (1-4, 9-10) do not [V, 03 §3.4]. Stepped, not continuous (14 detents), still reversible. | 16 table reads to rebuild `order` |
| LEN (pitch-lane length `len`) | 1..16, default 7 | Loop length of the **pitch lane** against the fixed 16-step rhythm. 16 = pitch and rhythm lock; 7 = a 112-step cycle before the full pattern repeats; 1 = one repeated note. Under a serpentine/spiral shape a short length plays a connected region of the grid [V, 03 §3.5]. | 1 modulo per step in the ISR |
| DENS (hit count `den`) | 1..16, default 10 | Cell `k` triggers iff its rank in the GATES-seed order is `< den`. Fewer = sparser, more rests; 16 = every step. | 1 table read + compare |
| RANGE (pitch span `rng`) | 0..48 semitone units, default 24 | Raw pitch value `r` (0..255) is scaled to `p = r * rng / 255` and then quantised late through the scale. 0 = one note, 12 = one octave, 24 = two, 48 = four. Scale size does not change the span (same principle as the source quantiser [V]). | 16 integer multiplies + table reads |

Rejected as morphs (kept out of the 4 knobs): gate length, swing, speed, direction (global performance settings, the
global DIV / SWING / GATE), steps shift (a 5th control; LEN + PATH already move the play window), per-lane
shapes (one shape for all three lanes keeps the display and the mental model to a single picture).

## 5. Fit against the vision (score 0–3 each, with one line of why)
| Criterion | Score | Why |
| --- | --- | --- |
| Quick to a usable sequence (rule 5) | 3 | Selecting SNAKE plays a full defaulted line immediately: PATH 7, LEN 7, DENS 10, RANGE 24, the global scale and fixed per-channel starting seeds (DENS 10 means exactly 10 of 16 steps trigger; the informative example in 6.3 was checked for channels 0..5); no setting besides the channel's MIDI channel. |
| Performable with 4 knobs + dice buttons, no menus (rules 1, 4) | 3 | Four knobs map one-to-one to four audible, reversible morphs; four orthogonal dice layers plus dice-all map to buttons; the 4x4 grid with the live playhead cell is a ready-made screen picture of what each knob does. |
| Monophonic bassline/lead use (the TB-3 workflow) | 2 | Strictly one note per step, and the rhythm gate is a clean on/off mask, so it is monophonic-safe. But it has no accent/slide/gate-length lane (velocity stands in for accent; gate length is the global GATE), and the pitches are uniformly random across the span, so it is more of a lead / polymetric-riff generator than a groove-aware bass line. Range from MIDI 36 up to about 95. |
| Distinct from the other families (adds something new) | 3 | Only family whose novelty is structural, not statistical: traversal shapes (order is data) and per-lane lengths giving polymeter between rhythm, pitch and dynamics. Nothing else in the catalogue does either (cross-family comparison is not assessed here; score is on the source spec alone). |
| Fits the FM-1 budget (RAM < 2 KB per channel, ISR cost small) | 3 | About 122 B per channel (6.4) plus 224 B of shared constants; per step the ISR does 3 table reads and 3 modulos; generation is ~50 xorshift draws on the main loop. |

## 6. Redesign for this sequencer

- **Proposed name on screen (max 6 characters):** `SNAKE`
- **Morphs (on-screen labels, max 6 characters each):** K1 `DENS`, K2 `LEN`, K3 `RANGE`, K4 `PATH` (ROW 1); S1 `OFFSET`, S2 `GLEN`, S3 `DYN`, S4 `DIR` (ROW 2). Full list: docs/design/FULL_CONTROL.md 3.3. (The control map owns which context these knob roles apply in.)
- **Dice buttons (which layers):** four layer targets `NOTES`, `GATES`, `VEL`, `POLY`, plus dice-all and undo-dice.
  Physical button binding is a control-map table row; this spec only declares the targets.
- **What we drop from the original, and why:**
  - Per-lane Speed (32n..2n), Direction (5 modes, two of them ⚠), Steps Shift, and per-lane Shape: four settings per
    lane times three lanes is a menu (rule 1); the global DIV, plus PATH + LEN, cover the musical use. Direction and
    random direction would also break "same settings and seed play the same pattern" (rule 4) in the random case.
  - Swing and Note length: global settings (and ⚠ in the source).
  - The 37 scales and the User scale: the global scale control owns the scale.
  - The three Custom modulation lanes: they drive Live parameters, not notes, and rule 3 gives a channel only a MIDI
    channel. Mapping them to MIDI CC / p-locks is a possible post-v1 extra.
  - MIDI-in control (note 72-83 root, 60-71 user scale, 48 random-all): there is a global scale control and a dice-all gesture.
  - Reset: undo-dice replaces it, and what the source's Reset restores is ⚠.
  - A free Gate-lane length: the gate lane is the bar and stays 16.
- **What we improve over the original:**
  - There is a one-gesture start: seeded content for all lanes, where the source needs hand entry or a global Random.
  - Gates are an order-based DENS morph, where the source's Random gives an uncontrollable 50 % density.
  - Every random decision is seeded per layer, so a layer can be re-rolled alone, undone and saved (rule 4).
  - All morphs are monotonic/reversible and apply immediately without losing the clock phase (6.3 playback).
  - Integer quantiser without the `divide by 11` overshoot quirk (⚠ clamp assumption, 04 §4.2) so no index ever
    leaves the table.
  - Velocity lane length is diceable, so the "dynamics drift" polymeter is available without a setting.
  - The Gates-only-trigger question left open by the source (07 §7.6) is decided **[D]**: Gates is the sole trigger;
    the Notes and Velocities lanes are read at the same step on their own counters.

> P1-013: parameters listed in this section under "must not exist" or "drop" that are now ROW 2 morphs (offset, direction, slide amount, accent depth, ratchet amount, contour curve, gate-lane length, stage divider cap, velocity depth) are no longer forbidden; everything else listed stays cut. See FULL_CONTROL.md 3.1 to 3.4.

### 6.1 Parameters (data table; ranges and defaults)

This table is the single source for the on-screen labels, ranges and defaults. A remap is an edit to this table.

| Id | Kind | Label | Range | Default | Stored as |
| --- | --- | --- | --- | --- | --- |
| `den` | morph, K1 | `DENS` | 0..16 | 10 | u8 |
| `len` | morph, K2 | `LEN` | 1..16 | 7 | u8 |
| `rng` | morph, K3 | `RANGE` | 0..48 | 12 | u8 |
| `shp` | morph, K4 | `PATH` | 1..14 | 7 | u8 |
| `off` | morph, S1 | `OFFSET` | 0..15 | 0 | u8 |
| `gl` | morph, S2 | `GLEN` | 1..16 | 16 | u8 |
| `dyn` | morph, S3 | `DYN` | 0..10 | 10 | u8 |
| `dir` | morph, S4 | `DIR` | 0..2 (FWD BWD PEND) | 0 | u8 |
| `s_notes` | dice seed | `NOTES` | 0..65535 | `(0x3A5D * (ch*4 + 1)) & 0xFFFF` | u16 |
| `s_gates` | dice seed | `GATES` | 0..65535 | `(0x3A5D * (ch*4 + 2)) & 0xFFFF` | u16 |
| `s_vel` | dice seed | `VEL` | 0..65535 | `(0x3A5D * (ch*4 + 3)) & 0xFFFF` | u16 |
| `s_poly` | dice seed | `POLY` | 0..65535 | `(0x3A5D * (ch*4 + 4)) & 0xFFFF` | u16 |
| `p_*` (x4) | previous seed for undo-dice | n/a | 0..65535 | equal to the seed (nothing to undo) | u16 each |
| `pin_mask`, `pin_note[16]` | step-entry overrides (6.5) | n/a | mask 0..65535; offset 0..96 | 0, 0 | u16 + 16 x u8 |

`ch` is the 0-based channel index. A seed of 0 is legal; the mixed state `x` is forced non-zero (see 6.2). When the
user dices a layer the new seed is the low 16 bits of the audio sample counter at the press (the only
non-deterministic moment; it is stored, so playback afterwards is fully deterministic).

### 6.2 Random primitives (normative, needed for golden files)

Identical to `sting.md` 6.2, repeated here so this file stands alone:

- State: unsigned 32-bit `x` (all shifts logical, arithmetic wraps mod 2^32; all `/` and `%` below are unsigned
  integer, truncating). `init(seed16, salt)`: `x = ((seed16 << 16) | seed16) ^ salt; if x == 0: x = 1; draw 4 values and discard.`
- Salts: NOTES `0x4E2B9A71`, GATES `0x1F6D83C5`, VEL `0x7B30E5A9`, POLY `0x29C4F18D` (different from Sting's, so no
  stream is shared with another algorithm).
- `next()`: xorshift32: `x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x`.
- `rnd(n)` (n >= 1) = `(next() >> 8) % n`.
- `shuffle(a, len)` (Fisher-Yates): for `i = len-1 down to 1`: `j = rnd(i+1); swap(a[i], a[j])`.
- Each layer builds its own state from its own seed and uses no other layer's stream.

### 6.3 Procedure (step by step; every layer is computed from seed only, then morphs are applied)

Cells are numbered `0..15` in **reading order** (row-major in the 4x4 grid), 0-based here (the source is 1-based).
Steps are 0-based too: step `s` here = source step number `s+1`.

**Shape tables** (constants; copied from 03 §3.3 [V], every row a permutation of 1..16). `SHAPE[n][k]` = the source
step number at which cell `k` plays; stored 1-based exactly as in the source and converted when `order` is built.

| n | `SHAPE[n][0..15]` | n | `SHAPE[n][0..15]` |
| --- | --- | --- | --- |
| 1 | 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 | 8 | 4 3 2 1 5 6 7 8 12 11 10 9 13 14 15 16 |
| 2 | 13 14 15 16 9 10 11 12 5 6 7 8 1 2 3 4 | 9 | 1 2 6 7 3 5 8 13 4 9 12 14 10 11 15 16 |
| 3 | 1 5 9 13 2 6 10 14 3 7 11 15 4 8 12 16 | 10 | 10 11 15 16 4 9 12 14 3 5 8 13 1 2 6 7 |
| 4 | 4 8 12 16 3 7 11 15 2 6 10 14 1 5 9 13 | 11 | 1 2 3 4 12 13 14 5 11 16 15 6 10 9 8 7 |
| 5 | 1 8 9 16 2 7 10 15 3 6 11 14 4 5 12 13 | 12 | 4 3 2 1 5 14 13 12 6 15 16 11 7 8 9 10 |
| 6 | 4 5 12 13 3 6 11 14 2 7 10 15 1 8 9 16 | 13 | 10 9 8 7 11 16 15 6 12 13 14 5 1 2 3 4 |
| 7 | 1 2 3 4 8 7 6 5 9 10 11 12 16 15 14 13 | 14 | 7 8 9 10 6 15 16 11 5 14 13 12 4 3 2 1 |

(Checked while writing this spec: all 14 are permutations, and shapes 5-8 and 11-14 have every consecutive step at
Manhattan distance 1.)

**Order** (rebuilt whenever `shp` changes; pure): `for k in 0..15: order[SHAPE[shp][k] - 1] = k`. `order[s]` = the
cell that speaks at step `s`. Default `shp` 7 gives `order = [0,1,2,3,7,6,5,4,8,9,10,11,15,14,13,12]`.

**NOTES layer** (stream: `init(s_notes, SALT_NOTES)`): `for k in 0..15: raw[k] = rnd(256)`.

**GATES layer** (stream: `init(s_gates, SALT_GATES)`)
1. `ord_g = [0..15]; shuffle(ord_g, 16)`.
2. `rank_g[ord_g[r]] = r` for `r = 0..15`.
**DENS morph:** `gate[k] = (rank_g[k] < den)`. (Because `den >= 1`, at least one cell always triggers.)

**VEL layer** (stream: `init(s_vel, SALT_VEL)`): `for k in 0..15: vel[k] = 50 + rnd(78)` (50..127). The range 50..127
is a constant **[D]** (the source's 0..127 range includes velocities too quiet to be useful for an instrument;
only the top end 127 [V] is relied on).

**POLY layer** (stream: `init(s_poly, SALT_POLY)`): `vlen = 3 + rnd(10)` (3..12).

**Scale table `tbl`/`N`** (the single global scale control, owner decision 3): `tbl` = the global scale's semitone degrees ascending, `N` their
count (chromatic = `[0..11]`, `N = 12`). SNAKE has **no fallback scale of its own**: it uses whatever the global scale
control holds (chromatic included). Golden files pass `tbl` explicitly; the reference fixture is minor pentatonic
`[0,3,5,7,10]`, `N = 5`. Whether the global default is musical is owner question 4 (docs/project/OWNER_QUESTIONS.md).
Root and transpose enter only at output.

**Pitch cache** (rebuilt on a NOTES seed, RANGE, scale, root or transpose change; pure). For each cell `k`:
1. `p = (raw[k] * rng) / 255` (integer; `0..rng`)
2. `o = p / 12; pos = p % 12`
3. `idx = (pos * N) / 12` (always `<= N-1`, so no out-of-range index exists)
4. `note[k] = clamp(36 + root + tbl[idx] + 12*o + transpose, 0, 127)`
`36` (C2) is the base note **[D]** (the source's quantiser puts its lowest band at MIDI 24 [V]; 36 matches the ACID
base). The highest possible note is below 36+11+48+11 = 106 before transpose.

**Playback** (the channel engine calls this once per step; `t` = the channel's step counter, a u32 that starts
at 0 on transport start or restart and increments once per step at the global DIV; nothing here is swung):
1. `cg = order[t % 16]` (gate cell). `if pin_mask bit (t % 16)`: `hit = 1`, `n = pin_note[t % 16]` -> note below.
   Else `hit = gate[cg]`. If `hit == 0`: no note this step.
2. `cp = order[t % len]` (pitch cell). `note = (pinned) ? clamp(36 + root + pin_note + transpose, 0, 127) : note[cp]`.
3. `cv = order[t % vlen]` (velocity cell). `velocity = vel[cv]`.
4. `length` = 50 % of one step (constant `gate_pct = 50` **[D]**); the global GATE setting multiplies it.
Because every lane reads `t`, not its own stored phase, any morph (LEN, PATH, DENS, RANGE) or dice takes effect
immediately on the next step without disturbing the clock phase of the other lanes [V, 06 §6.1: lanes are
phase-locked to the song position]. The ISR only reads the caches below; `%` is three integer divisions per step.

**Caches** per channel, swapped in one go (`fm1_irq_off/on` around a copy of about 50 bytes, or double-buffered):
`order[16]`, `note[16]`, `vel[16]` (from the VEL layer, no morph), `gate_bits` u16, `len`, `vlen`.
The cache is recomputed on any seed, knob, scale, root, transpose or pin change.

**Informative check** (not normative; the golden file will supersede it). With channel 0, all defaults, no pins,
scale `[0,3,5,7,10]` (fixture), root 0, transpose 0, the first 16 steps (note, velocity or `-` for a rest) from a throwaway Python
transcription of this procedure are:
`- ; 48,95 ; 53,82 ; 43,89 ; 43,122 ; - ; - ; 53,82 ; - ; - ; 43,60 ; 43,95 ; 55,82 ; 43,89 ; - ; 48,60`
(seeds 14941 / 29882 / 44823 / 59764, `vlen` = 5).

### 6.3b ROW 2 morphs (P1-013, normative; all defaults are neutral, so the procedure above is unchanged at defaults)

Let `u = t + off` (OFFSET `off` 0..15). Playback in 6.3 becomes:
1. **Gate lane (GLEN `gl` 1..16, default 16).** `g[j] = gate[order[j]]` for `j = 0 .. gl-1` (built with the cache). If
   `den >= 1` and no `g[j]` is set, `g[0] = 1` (a short lane never goes silent). `hit = g[u % gl]`. With `gl = 16`
   this is the original `gate[order[t % 16]]`. (`den = 0` is KEYS ONLY: no hits, nothing is forced.) A pin at `t % 16` still forces a hit.
2. **Pitch lane (DIR `dir` 0 FWD, 1 BWD, 2 PEND).** `cp = order[dir_map(u, len, dir)]` (SP16); the gate and velocity
   lanes always read forward.
3. **Velocity lane (DYN `dyn` 0..10, default 10).** `cv = order[u % vlen]`; `velocity = dyn_vel(vel[cv], dyn)` (SP17):
   `clamp(100 - ((100 - v) * dyn) / 10, 1, 127)`, division truncating toward zero. `dyn = 10` is the original `v`; `dyn = 0`
   is a flat 100.
Pins (6.5) are keyed to `t % 16` and ignore OFFSET and DIR. The cache gains `gl`, `off`, `dir` and the `g` bits.

### 6.4 Channel RAM and CPU

| Item | Bytes |
| --- | --- |
| 8 morph values (ROW 1 and ROW 2) | 8 |
| 4 seeds + 4 previous seeds | 16 |
| Pin mask + 16 pin notes | 18 |
| Layer scratch (raw 16, rank_g 16, vel 16) | 48 |
| Playback cache (order 16, note 16, gate_bits 2, len 1, vlen 1) plus `vel` (counted above) | 36 |
| Cache additions (`gl`, `off`, `dir`) | 3 |
| **Total** | **129 B per channel** (well under 2 KB) |

The shape tables are shared constants (224 B in flash). Scratch can instead be regenerated from the seeds on demand
(about 50 draws) and not stored, giving about 74 B per channel; an implementation choice, not a design one. CPU: a
full recompute is under 300 integer ops, run from the main loop; the ISR does three modulos and three table reads per
step.

### 6.5 Step entry interplay (rule 3: steps on the 16 white keys OR the algorithm)

- A step typed on the white keys while SNAKE is selected writes a **pin** on the **gate step** (the rhythm position,
  which is what the player counts): `pin_mask |= 1<<i`, `pin_note[i] = clamp(key note - 36 - root, 0, 96)` (an
  absolute note offset, not a pitch class: the pitch lane's octave is not meaningful for a typed note).
  The step is forced to hit, ignoring DENS and PATH-driven rests, and keeps the velocity lane's value.
- Pins are keyed by **step**, not cell, so they survive all morphs (PATH moves cells, not steps) and all dice.
  Clearing the pin (a control-map action) returns the step to the generated line.
- The key-entry gesture itself (which key, which mode) belongs to the channel/step-entry spec and must cite this
  section.

### 6.6 Flex points (ADR-0004 addendum): what is a table edit, what is not

- ROW 2 (P1-013): which knob slot drives OFFSET, GLEN, DYN, DIR (`knob_morph[8]`), their ranges and defaults, the flat velocity 100 of `dyn = 0`.
Remappable by editing a table row, no algorithm code change:
- Which knob drives which morph (K1..K4 <-> PATH/LEN/DENS/RANGE), each morph's label, range, default.
- Which physical button dices which layer, and the gesture for dice-all and undo-dice.
- Fixed constants: base note 36, velocity range 50..127, `vlen` range 3..12, `gate_pct` 50, default shape 7, default seed multiplier 0x3A5D, the 14 shape rows.
- Adding a dice layer that reuses an existing stream, e.g. a PATH dice that offsets `shp` by a seed-derived amount,
  or splitting NOTES into pitch vs range; giving K2 to a different morph.

Not changeable without algorithm code: the xorshift generator and salts (they define the golden files), the shuffle
method, the order-based morph principle (monotonic/reversible), the t-based lane clock (`order[t % length]`),
the single-trigger-lane rule (Gates is the sole trigger), adding a genuinely new lane (for example per-lane shapes or
a CC lane), changing the grid/step count (16).

- **Parameters that must not exist (settings creep that breaks rule 1):** per-lane Speed, Direction, Shift, Shape or
  Length; Swing and Note length as algorithm settings; scale/root as SNAKE settings (global scale control); a User scale;
  Custom-lane modulation targets and ranges; MIDI-in control; Reset; Random Range sliders; a seed entry field;
  a per-cell edit grid (steps are entered on the white keys, 6.5).

## 7. Risks and open questions

Design points that rely on ⚠ claims, and how each is neutralised:

| # | ⚠ claim in source | Where our design touches it | Treatment |
| --- | --- | --- | --- |
| 1 | Default shape at load not stored (⚠ 02 table, 07 §7.5 #10) | PATH default | Not relied on: default 7 is our table value. |
| 2 | Out-of-range scale index clamps to the last degree (⚠ 04 §4.2 #1, 07 §7.5 #1) | Quantiser | Not relied on: `idx = (pos*N)/12 <= N-1` always. Fidelity to the original top-of-band sliver is not claimed. |
| 3 | What Reset restores (⚠ 05 §5.2, 07 §7.5 #2) | none | Reset dropped. |
| 4 | Ping-pong end repeats, random direction repeats (⚠ 06 §6.2, 07 §7.5 #3, #4) | none | Direction dropped. |
| 5 | Swing units and comparators, swing follows the Gates lane speed (⚠ 06 §6.4, §6.5) | none | Swing is a global setting. |
| 6 | Note length menu is really `Direction_convert` (⚠ 06 §6.6, 07 §7.5 #6) | note length | Replaced by `gate_pct` 50 x global GATE. |
| 7 | `live.step` exact semantics (⚠ 06 §6.2) | lane clock | Not relied on: we define the lane position as `order[t % length]`; only "lanes loop at their own lengths" [V] is used. The order of the lane clocks inside the original's step is unknown, so the original may read a latched pitch one step old; we define same-step reads **[D]**. |
| 8 | User scale sorting/length (⚠ 07 §7.5 #7), MIDI-in octave names (⚠ 02, 07 §7.5 #8), "Minor" scale table (⚠ 04 §4.4 #2, 07 §7.5 #9) | none | Dropped, or owned by the global scale spec. |
| 9 | Custom lane parameter mapping to p-locks (07 §7.6, [V structure]) | none | Dropped. |

Relied on and **[V]**: the 14 shape tables and their permutation/contiguity properties (03), independent lane
lengths giving polymeter (01, 06), trigger/pitch/velocity separation (01, 05 §5.4-5.6), and late quantisation (04).

Other open questions for the owner or other specs (not source ⚠):
- **Global settings (owner decisions 3 and 4, 2026-11-09):** scale (with root and transpose) and DIV / GATE / SWING
  are global performance settings, not per channel and not per algorithm. SNAKE reads them and owns none. The global
  spec is still unwritten; until it locks, the reference implementation uses: step = one 1/16 note, `gate_pct` 50
  unscaled, no swing, root 0, transpose 0, and the fixture scale above. The earlier per-algorithm minor-pentatonic
  fallback was removed.
- **Playhead display.** The 4x4 grid with a moving cell for each of the three lanes is the natural screen for SNAKE
  (it shows PATH, LEN and DENS at once); it belongs to the screen spec and is not defined here.
- **Hardware:** are the defaults (PATH 7, LEN 7, DENS 10, RANGE 24, minor pentatonic, base C2) good on a TB-3? All
  are table values (6.6), so retuning is a table edit.
- **Whole-algorithm question:** a uniformly random pitch lane is less bass-like than ACID. If the owner wants SNAKE
  usable as a bass, a lower default RANGE (12) or a scale-weighted raw distribution is a follow-up tuning decision.
- Owner re-lock needed (ADR-0004) after the reference implementation and golden files exist; none exist yet.

## 8. Recommendation

**INCLUDE in v1.** SNAKE is the one family that adds a *structure* rather than another probability: traversal
shapes re-order the same material with a single knob, and independent lane lengths give polymeter between rhythm,
pitch and dynamics, so one algorithm choice yields patterns the other families cannot. It also maps almost
one-to-one onto the panel (8 morphs on two rows of 4 knobs, 4 dice layers, a grid screen that shows the playhead). The cost is
about 122 bytes per channel plus 224 bytes of shared tables and a few table reads per step. Nothing in the redesign
depends on a ⚠ claim: all unverified source behaviour (direction, swing, note length, reset, scale clamp) is dropped
or replaced with a rule defined here. The residual risk is musical, not technical (random pitches are less
bass-like than ACID, a point for hardware tuning by table edit), and the dropped Custom modulation lanes could return
later as a CC lane.
