# Assessment: Sting (acid line generator)

Source spec: `docs/sting-algorithms/` (read all of it). Vision: docs/project/VISION.md.
Use the source's confidence tags ([V]/[I]/⚠) when you rely on a claim, and don't promote ⚠ to fact.

Design stance: this is a **redesign, not a port**. Wherever the source is ⚠, this document either replaces the
behaviour with a rule defined here (marked **[D]** = our design decision, no fidelity claim to the original) or lists
it in section 7. Everything below is written so that a host reference implementation (ADR-0004 item 2) can be
written without choices. Layer, morph and dice definitions are declared as data tables (ADR-0004 addendum); the
tables are in section 6.

## 1. What it does (5 lines max)

Sting builds a 16-step monophonic acid line from independent lanes: pitch class, octave, gate length and
velocity/accent. Pitch comes from a short random motif of 3 to 5 pitch classes tiled over 16 steps and shuffled [V].
A "Type" morph overwrites a growing number of steps with one repeated note, sliding the line from random towards
hammering acid [V structure]. Octave jumps, gate lengths and accents are small step-position-weighted coin flips [V].
In this sequencer it becomes the **ACID** algorithm: 5 dice layers, 4 knob morphs, all deterministic from five seeds.

## 2. Generators inside it
| Generator | Inputs | Output | Deterministic? (seeded / free-running) |
| --- | --- | --- | --- |
| Pitch v2 (motif tile + shuffle) [V structure] | seed, urn of 12 pitch classes, motif length N = 3..5 | `pitch_orig[16]`, 0..11 | Seeded in this design (source uses Max `urn`/`random`, free-running) |
| Pitch v1 "Classic" (12-urn then 4 more) [V] | seed | `pitch[16]` 0..11, chromatic wander | Seeded; **dropped** here (see 6) |
| Type replacement pass [V structure, ⚠ replacement note and count] | Type, 15-slot urn | N steps overwritten with a repeated note | Seeded; redefined **[D]** as the TYPE morph |
| Scale snap (nearest degree) [V] | `pitch_orig`, scale list, Root | `pitch[16]` | Pure function, no randomness |
| Octave lane [V structure] | step index, two coin flips, direction `rand(3)-1` | `octave[16]`, -2..+2 | Seeded |
| Gate lane [V structure, ⚠ index base] | step index, two curves, first-step coin | `gate[16]` in {30,70,100,127} | Seeded |
| Accent/velocity lane [V structure, ⚠ K, ⚠ final mapping] | urn of 16 positions, accent amount | `vel[16]` | Seeded; K and mapping redefined **[D]** |
| Density [I, ⚠ role] | 0..16 | unknown (hit count, accent count or regen trigger) | Replaced by the HITS layer + DENS morph **[D]** |
| Playback: direction, offset, jitter, swing [V structure, ⚠ units] | transport, Steps | read position, delay | Jitter is random in source; **dropped** here |
| CakeWalk drift [V structure, ⚠ range] | amount, random walk | auto-moving Density/Type dial | Free-running; **dropped** here |

## 3. Layers (what can be diced independently)

Five layers, each with its own 16-bit seed. Source behaviour: lanes are already independent and each listens
for the same "gen" message [V, 01 Architecture]; this design just gives each its own seed so one can be re-rolled
alone. The on-screen name is the dice label (max 6 chars).

| Layer (screen label) | What dicing it changes | What stays fixed |
| --- | --- | --- |
| NOTES (`NOTES`) | New motif length N, new motif pitch classes, new tiled/shuffled order, new TYPE replacement order. So the melody changes. | Which steps play (HITS), octaves, gates, accents, all four knob values, scale, root |
| HITS (`HITS`) | The priority order in which steps are switched on as DENS rises (which steps are rests at the current DENS). Step 0 is always first in the order. | Pitches, octaves, gates, accent order, knob values |
| OCT (`OCT`) | Per-step octave random numbers (two thresholds and a direction per step), so which steps jump and which way. | Pitches, hits, gates, accents, knob values. JUMP knob amount stays where it is. |
| GATE (`GATE`) | The 16 gate codes (staccato to full). | Pitches, hits, octaves, accents |
| ACCENT (`ACCNT`) | The priority order in which steps become accented as ACCNT rises. | Pitches, hits, octaves, gates |

A sixth action, **dice all**, re-rolls all five seeds in one gesture (which physical button does it is the control
map's job). **Undo dice**: each layer keeps its previous seed (2 bytes), so the last dice of each layer can be
recalled once (VISION glossary: "the previous seed can be recalled").

## 4. Morph candidates (continuous knob transforms)

Every morph is a **pure function of (seeds, knob values)**: the random orders are fixed by the seeds, the knob only
chooses how much of them to apply. Turning a knob back restores the previous pattern exactly (VISION glossary:
"undoable by turning back"). Cost figures are for recomputing the 16-step cache after a knob detent (main loop, not
the ISR).

| Morph | Range | Musical effect | Cost per step (rough) |
| --- | --- | --- | --- |
| TYPE (replacement count `k`) | 0..12 detents, default 6 | Overwrites the first `k` steps of the NOTES-seed replacement order with the root (pitch 0). 0 = raw motif line, 12 = mostly root-hammering acid. Capped at 12 so at least 4 steps keep their motif note. Replaces the source's 16-position Type dial [V structure]. | 1 compare + 1 store |
| DENS (hit count `n`) | 1..16, default 12 | Step `i` plays iff its rank in the HITS-seed order is < `n`. Fewer = sparser, more rests; 16 = every step. | 1 table read + compare |
| JUMP (octave probability factor `j`) | 0..20 detents, default 5 | `p_eff[i] = min(100, base_p[i] * j / 5)`; each of the two coin flips passes if its stored 0..99 number is `< p_eff[i]`. 0 = no jumps, 5 = original odds (10 to 28 %), 20 = nearly every step jumps. | 2 compares + 1 multiply (integer) |
| ACCNT (accent count `a`) | 0..8, default 3 | The first `a` **hit** steps in the ACCENT-seed order are accented. 0 = flat dynamics, 8 = half the pattern. | 1 table read + compare |
| (not morphs) Scale/Root | via SCL layer, shared | Pitches re-snapped from `pitch_orig`, reversible [V] | 12-entry loop per step, only on change |

Rejected as morphs (kept out of the 4 knobs): gate length (no knob left; it is a dice layer and the channel's shared
GATE setting scales it), swing and direction (shared channel concerns), jitter (see 6).

## 5. Fit against the vision (score 0–3 each, with one line of why)
| Criterion | Score | Why |
| --- | --- | --- |
| Quick to a usable sequence (rule 5) | 3 | Selecting ACID plays a full, defaulted, TB-3-sensible 16-step line immediately: defaults TYPE 6, DENS 12, JUMP 5, ACCNT 3 and fixed per-channel starting seeds; zero required settings besides the channel's MIDI channel. |
| Performable with 4 knobs + dice buttons, no menus (rules 1, 4) | 3 | Four knobs map one-to-one to four morphs that are each audible, monotonic and reversible; five orthogonal dice layers plus dice-all map directly to buttons; nothing needs a menu. |
| Monophonic bassline/lead use (the TB-3 workflow) | 3 | Native fit: one note per step, 16 steps, accent and gate-length lanes mirror the TB-3's own accent/gate model; the range is MIDI 24 up to about 82 (base 36, octave -1..+2, root up to 11). Slide is not generated (a possible post-v1 extra). |
| Distinct from the other families (adds something new) | 2 | Adds "urn" non-repeating motif pitch sets and the random-to-acid morph that no other family has; but a rest/hit mask and accents overlap with the Euclidean/steps families, and the algorithm is only a 16-step single loop. (Cross-family comparison is not assessed here; score is on the source spec alone.) |
| Fits the FM-1 budget (RAM < 2 KB per channel, ISR cost small) | 3 | About 190 bytes per channel (section 6), generation is a few hundred integer ops on the main loop, and the ISR only reads a precomputed 16-step cache. |

## 6. Redesign for this sequencer

- **Proposed name on screen (max 6 characters):** `ACID`
- **K1–K4 morphs (with on-screen labels, max 6 characters each):**
  K1 `TYPE`, K2 `DENS`, K3 `JUMP`, K4 `ACCNT`. (The control map owns which context these knob roles apply in.)
- **Dice buttons (which layers):** five layer targets `NOTES`, `HITS`, `OCT`, `GATE`, `ACCNT`, plus dice-all and
  undo-dice. Physical button binding is a control-map table row; this spec only declares the targets.
- **What we drop from the original, and why:**
  - Algo v1 "Classic": a chromatic 12-urn wander that sounds random and needs a scale to be usable; v2 covers the
    musical use and TYPE covers the "random" end. One generator = one less switch (rule 1).
  - Direction (Bwd, Pendulum), Offset, Poly Reset, Clock = Gate: shared channel/transport features, not algorithm
    parameters; belong to the shared channel spec if wanted at all.
  - Jitter: random step displacement at playback breaks "same settings and seed play the same pattern" (rule 4).
  - CakeWalk: free-running random drift of dials; breaks determinism and fights the player's hands on the knobs.
  - Gen Clip (Ableton clip export): not applicable.
  - Division (15 values): the channel's shared DIV applies.
  - Root as a "scale reference only" quirk and **linear** (non-circular) snap distance: replaced, see below.
  - Bipolar Accent control (-1..+1): replaced by the ACCNT count, two fixed velocities, no extra parameter.
  - Global Gate dial (⚠ role unknown): replaced by the channel's shared GATE setting scaling the gate lane.
- **What we improve over the original:**
  - Rests: Sting plays 16 notes always [V, 06 §6.3 "all active steps", ⚠ role of Density]. We add a HITS layer + DENS
    morph so the line breathes and is performable.
  - Every random decision is seeded per layer, so a layer can be re-rolled alone, undone, and saved (rule 4).
  - All morphs are order-based (fixed order from seed, knob picks how much), so they are monotonic and reversible,
    where the source's Gen re-rolled everything.
  - Root now transposes the line: pitch 0 = root note (see Output), instead of only shifting the scale list.
  - Scale snap uses **circular** pitch-class distance (11 is 1 from 0), ties go to the **lower** pitch class, so the
    result no longer changes tie direction with Root (fixes quirks 1 and 2 of 03 §3.5). `pitch_orig` is kept, so Scale
    off or a different scale restores/re-snaps exactly [V].

### 6.1 Parameters (data table; ranges and defaults)

This table is the single source for the on-screen labels, ranges and defaults. A remap is an edit to this table.

| Id | Kind | Label | Range | Default | Stored as |
| --- | --- | --- | --- | --- | --- |
| `typ` | morph, knob 1 | `TYPE` | 0..12 | 6 | u8 |
| `den` | morph, knob 2 | `DENS` | 1..16 | 12 | u8 |
| `jmp` | morph, knob 3 | `JUMP` | 0..20 | 5 | u8 |
| `acc` | morph, knob 4 | `ACCNT` | 0..8 | 3 | u8 |
| `s_notes` | dice seed | `NOTES` | 0..65535 | `(0x3A5D * (ch*5 + 1)) & 0xFFFF` | u16 |
| `s_hits` | dice seed | `HITS` | 0..65535 | `(0x3A5D * (ch*5 + 2)) & 0xFFFF` | u16 |
| `s_oct` | dice seed | `OCT` | 0..65535 | `(0x3A5D * (ch*5 + 3)) & 0xFFFF` | u16 |
| `s_gate` | dice seed | `GATE` | 0..65535 | `(0x3A5D * (ch*5 + 4)) & 0xFFFF` | u16 |
| `s_acc` | dice seed | `ACCNT` | 0..65535 | `(0x3A5D * (ch*5 + 5)) & 0xFFFF` | u16 |
| `p_*` (x5) | previous seed for undo-dice | n/a | 0..65535 | equal to the seed (nothing to undo) | u16 each |
| `pin_mask`, `pin_pitch[16]` | step-entry overrides (6.5) | n/a | mask 0..65535; pitch 0..11 | 0, 0 | u16 + 16 x u8 |

`ch` is the 0-based channel index. A seed of 0 is legal; the mixed state `x` is forced non-zero (see 6.2). When the user dices a layer the new seed
is the low 16 bits of the audio sample counter at the press (the only non-deterministic moment; it is stored, so
playback afterwards is fully deterministic).

### 6.2 Random primitives (normative, needed for golden files)

- State: unsigned 32-bit `x` (all shifts logical, arithmetic wraps mod 2^32; all `/` and `%` below are unsigned integer, truncating). `init(seed16, salt)`: `x = ((seed16 << 16) | seed16) ^ salt; if x == 0: x = 1; draw 4 values and discard.`
- Salts: NOTES `0x5A17C3E1`, HITS `0x2C9E6B07`, OCT `0x71D4A5F3`, GATE `0x3B8F1D29`, ACCNT `0x68E2B94D`.
- `next()`: xorshift32: `x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x`.
- `rnd(n)` (n >= 1) = `(next() >> 8) % n`.
- `bern(p)` = `rnd(100) < p`.
- `shuffle(a, len)` (Fisher-Yates): for `i = len-1 down to 1`: `j = rnd(i+1); swap(a[i], a[j])`.
- Each layer builds its own state from its own seed, in the order given below, and uses no other layer's stream.
  Any later change to one layer's code therefore cannot reshuffle another layer [07 §7.2].

### 6.3 Procedure (step by step; every layer is computed from seed only, then morphs are applied)

**NOTES layer** (stream: `init(s_notes, SALT_NOTES)`)
1. `N = 3 + rnd(3)` (3, 4 or 5).
2. `bag = [0..11]; shuffle(bag, 12); motif = bag[0..N-1]`.
3. `tile[i] = motif[i % N]` for `i = 0..15`.
4. `shuffle(tile, 16)`; `base_pitch = tile`.
5. `ord_type = [0..15]; shuffle(ord_type, 16)` (replacement order; uses the next draws of the same stream).

**TYPE morph** (pure): `pitch_orig = copy(base_pitch)`; for `r = 0 .. typ-1`: `pitch_orig[ord_type[r]] = 0` **[D]**. The
replacement note is the root (pitch class 0, relative to Root), not "the first note" (⚠ in source); this makes the
hammering note always the tonic of the key.

**Scale snap** (pure): reference list `ref` = the channel's/global scale intervals (SCL layer scale; chromatic
`[0..11]` when Scale is off or OFF/CHROM). For each step:
`pitch[i] = argmin over ref of circ(pitch_orig[i], r)` where `circ(a,b) = min(|a-b|, 12-|a-b|)`; on equal distance
choose the **numerically lower** `r`. When Scale is off, `pitch = pitch_orig`. Root does not enter the snap: pitch
classes are intervals above Root **[D]**.

**HITS layer** (stream: `init(s_hits, SALT_HITS)`)
1. `rest = [1..15]; shuffle(rest, 15)`; `ord_hit = [0] + rest` (step 0 always ranks first).
2. `rank_hit[ord_hit[r]] = r`.
**DENS morph:** `hit[i] = (rank_hit[i] < den)`.

**OCT layer** (stream: `init(s_oct, SALT_OCT)`), for `i = 0..15` in order, three draws each:
1. `u1[i] = rnd(100)`; `u2[i] = rnd(100)`; `dir[i] = rnd(3) - 1`.
**JUMP morph:** `base_p = [10,11,12,13,15,16,17,18,20,21,22,23,25,26,27,28]` (`10 + floor(20*i/16)`, from 04 §4.1 [V
structure]). `p_eff = min(100, (base_p[i] * jmp) / 5)` (integer division); `n = (u1[i] < p_eff) + (u2[i] < p_eff)`;
`octave[i] = max(-1, dir[i] * n)` **[D]** (clamped at -1: MIDI 24 is the lowest sensible bass note, source allows -2).

**GATE layer** (stream: `init(s_gate, SALT_GATE)`)
1. `gate[0] = rnd(2) == 0 ? 100 : 70`.
2. `pa = [5,8,11,14,17,20,23,26,29,32,35,38,41,44,47,50]` (`5 + round(45*i/15)`, 04 §4.2).
   `pb = [10,11,13,14,16,18,20,22,24,26,28,31,33,35,38,40]` (`10 + round(30*(i/15)^1.2)`). Tables, not `pow` at runtime.
3. for `i = 1..15`: `s = bern(pa[i]) + bern(pb[i])`; `gate[i] = {30, 70, 127}[s]`.
Output as a fraction of the step: `gate_pct = {30: 24, 70: 55, 100: 79, 127: 100}` (05/06: `gate/127 x one step`).
The channel's shared GATE setting multiplies this; no per-algorithm gate parameter. 100 occurs only on step 0
(⚠ as the source tags it; preserved, harmless if wrong).

**ACCENT layer** (stream: `init(s_acc, SALT_ACCNT)`)
1. `ord_acc = [0..15]; shuffle(ord_acc, 16)`.
**ACCNT morph:** walk `ord_acc` in order, counting only steps with `hit[i] == 1`; the first `acc` such steps are
accented. `vel[i] = accented ? 120 : 80` (MIDI velocity). Both velocities are fixed constants **[D]** (source
range 40..127, ⚠ neutral point), listed under flex points.

**Output per step** (for the shared channel engine; `i` = the step the engine selects):
- if `hit[i] == 0`: no note.
- else `note = clamp(36 + root + pitch[i] + 12 * octave[i] + transpose, 0, 127)`; `base 36` (C2) as in 06 §6.1 [V].
  `root` = SCL root 0..11, `transpose` = SCL TRN. Pins (6.5) replace `pitch[i]`.
- `velocity = vel[i]`; `length = gate_pct(gate[i])` of one step.
Cache stored per channel (hit mask u16 separate): `note[16]` (u8),
`vel[16]` (u8), `gate[16]` (u8): 64 bytes. Cache is recomputed on any seed, knob, scale, root or pin change, and
swapped in one go (`fm1_irq_off/on` around a 64-byte copy, or double-buffered).

### 6.4 Channel RAM and CPU

| Item | Bytes |
| --- | --- |
| 4 knob values | 4 |
| 5 seeds + 5 previous seeds | 20 |
| Pin mask + 16 pin pitches | 18 |
| Layer scratch (base_pitch 16, ord_type 16, ord_hit/rank 16, u1/u2/dir 48, gate 16, ord_acc 16) | 128 |
| Playback cache (note, vel, gate) | 48 |
| Hit mask | 2 |
| **Total** | **220 B per channel** (well under 2 KB) |

Scratch layers can instead be regenerated from seeds on demand (about 100 draws each) and not stored, giving 92 B
per channel; this is an implementation choice, not a design one. CPU: a full recompute is under 400 integer ops
(about 100 xorshift draws, 16 snap lookups), run from the main loop; the ISR only reads the cache.

### 6.5 Step entry interplay (rule 3: steps on the 16 white keys OR the algorithm)

- A step typed on the white keys while ACID is selected writes a **pin**: `pin_mask |= 1<<i`,
  `pin_pitch[i] = (key note - 36 - root) mod 12`. Pins fix the pitch class only (the octave layer still applies),
  and the step is forced to hit (`hit[i] = 1`), ignoring DENS.
- Pinned steps survive all morphs and all dice. Clearing the pin (a control-map action) returns the step to the
  generated line.
- This is the minimum interplay needed so "enter or dial" works on the same channel; the key-entry gesture itself
  (which key, which mode) belongs to the channel/step-entry spec and must cite this section.

### 6.6 Flex points (ADR-0004 addendum): what is a table edit, what is not

Remappable by editing a table row, no algorithm code change:
- Which knob drives which morph (K1..K4 <-> TYPE/DENS/JUMP/ACCNT), each morph's label, range, default.
- Which physical button dices which layer, and which gesture does dice-all and undo-dice.
- Fixed constants: base note 36, octave clamp -1, velocities 120/80, `base_p`, `pa`, `pb` tables, motif length
  range 3..5, replacement note (root), TYPE cap 12, gate code to percent table, default seeds multiplier 0x3A5D.
- Adding a sixth dice layer that reuses an existing stream, or splitting NOTES into motif vs TYPE-order (two seeds),
  or giving K3 to a different morph, are table changes.

Not changeable without algorithm code: the xorshift generator and salts (they define the golden files), the
shuffle method, the order-based morph principle (monotonic/reversible), adding a genuinely new generator (for
example a slide lane), changing the lane count or step count (16).

- **Parameters that must not exist (settings creep that breaks rule 1):** Algo v1/v2 switch, Type as a 16-label
  menu, Poly Reset, Clock mode, jitter amount, CakeWalk amount and its modulation-source assignments, per-lane
  probability parameters, bipolar Accent, Gate dial, Density as a separate "regenerate" control, Gen Clip, Root
  and Scale as ACID-specific settings (they live in the shared SCL layer), a Direction selector, a seed entry field.

## 7. Risks and open questions

Design points that rely on ⚠ claims, and how each is neutralised:

| # | ⚠ claim in source | Where our design touches it | Treatment |
| --- | --- | --- | --- |
| 1 | Type replacement note and count (⚠ 03 §3.4, 07 §7.4 #1, #2) | TYPE morph | Not relied on. We define replacement note = root and count = knob value, never-fixed slot dropped. Fidelity to the original Type is explicitly not claimed. If the owner wants parity, 07 §7.4 #1/#2 must be checked in Ableton first. |
| 2 | Density role (⚠ 04 §4.4, 07 §7.4 #3) | DENS morph, HITS layer, ACCNT | Not relied on. DENS is a new hit-count morph **[D]**. The source's accent count K is also replaced by ACCNT. |
| 3 | Accent neutral point and direction (⚠ 04 §4.3, 07 §7.4 #4) | ACCNT morph, velocities 120/80 | Not relied on; fixed velocities are our choice. The endpoints 40 and 127 [V] are the only fact used (120 and 80 sit inside them). |
| 4 | Gate dial role (⚠ 07 §7.4 #9) | Gate length scaling | Replaced by the channel's shared GATE setting. |
| 5 | Gate step index base 0..15 vs 1..15 (⚠ 04 §4.2) and `100` only on step 0 | GATE tables `pa`, `pb` | Relied on as stated: tables use 0..15 and step 0 takes the first-step coin. Effect on sound is tiny; golden file fixes it. |
| 6 | Whether v2's tile truncates to 16 (⚠ 03 §3.3) | NOTES step 3 | Truncation assumed (the expected result per the source); consequence if wrong: motif balance differs by one note. |
| 7 | Swing units, pendulum ends, jitter numbers (⚠ 05) | not used | Swing is the channel's shared SWING; direction and jitter dropped. |
| 8 | Incoming MIDI note feeds root (⚠ 03 §3.6, 07 §7.4 #7) | Root | Not used; Root comes from the SCL layer. |

Other open questions for the owner or other specs (not source ⚠):
- **Needs a decision outside this doc:** how the shared SCL "QNT" (quantise) option relates to ACID's own snap. This
  doc assumes ACID snaps whenever a non-chromatic scale is set and ignores QNT; the scale/quantise spec must confirm.
- Step-entry gesture and pin clearing (6.5) are specified here only as data model; the gesture belongs elsewhere.
- Hardware: whether TB-3 range (octave clamp -1, base C2) is musically right is a hardware test item, and the
  TYPE cap 12 and defaults (TYPE 6, DENS 12, JUMP 5, ACCNT 3) are starting values to tune on the device. Because they
  are table values (6.6) the retune is a table edit.
- Slide (TB-3 portamento) is not generated; whether acid without slide is acceptable is an owner question.
- **Dependence on shared channel settings.** Gate scaling, DIV (step length), SWING and SCL are referenced as "shared" but VISION rule 3 gives a channel only a MIDI channel, and no such spec exists yet. Until those specs lock, the reference implementation uses the defaults: gate_pct unscaled, step = one 1/16 note, no swing, Scale off (pitch = pitch_orig), root 0, transpose 0.
- Owner re-lock needed (ADR-0004) after the reference implementation and golden files exist; none exist yet.

## 8. Recommendation

**INCLUDE in v1.** ACID is the most direct match to the reference workflow (a TB-3 acid bassline dialled in with
four knobs and five dice layers): it reaches a convincing 16-step line instantly from defaults, its lanes are
already independent so the layer-dice model costs nothing, and every morph can be made monotonic and reversible by
fixing a seeded priority order and letting the knob choose how much to apply. The cost is about 220 bytes per channel
and a few hundred integer ops on the main loop, comfortably within budget. The redesign deliberately removes
everything ⚠ or free-running (Classic, jitter, CakeWalk, Poly Reset, Gate dial) and replaces the unconfirmed
Type/Density/Accent semantics with rules defined here, so no design point depends on an unverified claim about the
original; the only residual risks are musical (are the defaults right on a TB-3, is slide missing), which are tuning
by table edit during hardware testing.
