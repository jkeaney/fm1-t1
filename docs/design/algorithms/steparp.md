# Assessment: Step Arp (step-sequenced arpeggiator)

Source spec: `docs/steparp-algorithms/` (read all of it). Vision: docs/project/VISION.md.
Use the source's confidence tags ([V]/[I]/⚠) when you rely on a claim, and don't promote ⚠ to fact.

Design stance: a **redesign, not a port**, in the same style as `sting.md`, `snake.md` and `ml185.md`. The source is
not a generator: its pitches come from **keys the player holds** (the "note engine" sorts the held notes and walks
them), and its rhythm and lanes are hand-edited sliders; its only randomness is a per-lane uniform "Random" button
[V, 04 §4.1]. This sequencer has no held-note input on a channel (the white keys are step entry, rule 3), so the
design has to **invent the note pool** as well as the layers. Wherever the source is ⚠ or silent, this document
defines the rule here, marked **[D]** (our design decision, no fidelity claim). Layers, morphs and dice are declared
as data tables (ADR-0004 addendum); the tables are in section 6. The shared random primitives are those of `sting.md`
6.2 (xorshift32, `init(seed16, salt)`, `rnd(n)`, `shuffle`), with new salts.

Verdict preview (detail in section 8): **LATER**. The one idea ARP adds, *a pitch order over a small chord-tone pool*,
is genuinely new, but its rhythm half duplicates SNAKE and STAGE, and its defining input (held keys) does not exist
here. The full design is below so it can be promoted without rework.

## 1. What it does (5 lines max)

Step Arp joins two machines. A **note engine** takes the held notes, sorts them (played, up, down, chord, random,
follow), spreads them over 1..6 octaves (up, down, up/down, random, plus a "mountain" climb-and-fall) and hands out
one note per trigger [V, 03]. A **16-step clock** fires those triggers; each step has on/off plus velocity, length,
chance, ratchet, transpose and CC lanes [V, 04]. Note choice and rhythm are decoupled: an off step does not advance
the note engine [V, 04 §4.2]. In this sequencer it becomes the **ARP** algorithm: a seeded **chord-tone pool** from
the global scale, walked in one of five orders, over a seeded 16-step rhythm with ratchets and a chance gate.

## 2. Generators inside it
| Generator | Inputs | Output | Deterministic? (seeded / free-running) |
| --- | --- | --- | --- |
| Held-note list (`GetPitchList`, Hold) [V, 03 §3.1] | key presses | pitch list in played order | Free-running (player input). **Replaced [D]** by the CHORD layer: a seeded chord type and root from the global scale |
| Note Order sort (`NotesSort`) [V] | pitch list, mode (Played/Up/Down/Chord/Random/Follow) | sorted list | Up/Down pure. `Random` is a `zl scramble`, reshuffled when notes change; whether per trigger or per change is ⚠ (06 §6.6 #1). **Replaced [D]** by the ORDER layer: 5 orders, `Perm` is a seeded permutation |
| Octave expansion + mountain (`AddOctavesToNotes`) [V, 03 §3.3-3.4] | sorted list, octave count, octave mode | note list of length `tones x octaves` (mountain: `2x - 2`) | Pure for Up/Down/UpDown; `Random` octave mode free-running. **Kept as a pure function [D]** with the OCT morph (octave count) |
| Step Arp counter (`StepArp`) [V, 03 §3.5] | generate bang | `list[counter]`, counter wraps | Stateful in the source. **Redefined [D]** as a pure function of the step counter `t` (6.3), so a morph never desyncs the loop |
| Chord Arp (the whole chord at once) [V, 03 §3.5] | chord | chord per trigger | **Dropped**: a channel is monophonic (TB-3 workflow) |
| Steps on/off row [V, 04 §4.2] | 16 toggles | which steps fire | Hand-edited. **Replaced [D]** by the HITS layer: a seeded rank order + the DENS morph (same technique as SNAKE GATES and STAGE RESTS) |
| Velocity lane + Random [V, 04 §4.3] | 16 values, range 27..100 | step velocity | Free-running Random. **Seeded [D]** as the VEL layer, fixed range 50..127 |
| Ratchet lane [V structure, ⚠ n vs n+1 hits, ⚠ off path, 04 §4.6] | 0..3 per step | extra hits inside the step | Hand-edited / Random. **Redefined [D]** as the RATCH layer (which steps, how many) + the RATCH morph (how many steps) |
| Chance lane [V, 04 §4.5] | 0..100 % per step, roll per hit | pass/fail per hit | Free-running `random 100`. **Redefined [D]** as one global CHANCE morph with a hashed, repeatable roll (6.3) |
| Length lane [V, 04 §4.4] | 0..127 | gate = fraction of step | **Dropped**: the global GATE setting (owner decision 4) |
| Transpose lane + TrnspAll/Reset [V, 04 §4.7] | -7..+7 per step | pitch shift before scale snap | **Dropped** (a 6th lane no knob can reach; see 6.6 for the table-edit path back) |
| Scale snap (`Scales`) [V structure, ⚠ direction and root, 03 §3.7] | 12-entry pitch-class table | scale-conformant pitch | **Replaced** by the single global scale control (owner decision 3); the snap rule used for chord tones is **[D]** (6.3), not relied on from the source |
| Map/CC lane, Live-parameter mapping [V structure, ⚠ off-step, 04 §4.8] | 16 values, CC number | CC stream / Live parameter writes | **Dropped** (not a note algorithm; same decision as SNAKE's Custom lanes, owner question 6) |
| Clock `Triggers`/`TimeDiv`/swing, Dotted/Triplet, Sync/Freq [V structure, ⚠ swing map, 05 §5.1-5.3] | transport, division | step triggers | **Dropped**; global DIV / SWING (owner decision 4) |
| StepDirection (Fwd/Back/Alternate/Random) [V Fwd/Back, ⚠ Alt/Random, 05 §5.4] | counter mode | step number | **Dropped** (Random breaks rule 4; the other three are a menu) |
| LoopLength 1..16 [V, 05 §5.4] | length | steps in the loop | **Dropped**; fixed 16. The pool length (3..30) against 16 steps already gives a drifting cycle |
| Key Retrig / Lock [V structure, 05 §5.5] | key press, transport | reset step counter | **Dropped**; `t` restarts at 0 on transport start |

## 3. Layers (what can be diced independently)

Five layers, each with its own 16-bit seed, so one can be re-rolled alone. The chord/order split is the one new idea:
CHORD changes **which notes** exist, ORDER changes **how they are walked**; neither touches rhythm. The on-screen
name is the dice label (max 6 chars).

| Layer (screen label) | What dicing it changes | What stays fixed |
| --- | --- | --- |
| CHORD (`CHORD`) | The chord type (8: maj, min, sus2, sus4, dom7, min7, maj7, fifth) and its root scale degree, so the pool of 2..4 tones. | Walk order, rhythm, velocities, ratchets, all four knob values, scale, root |
| ORDER (`ORDER`) | The walk order of the pool: Up, Down, Mountain, Perm (a seeded permutation), Weave (tone-major, `C C+12 E E+12 ...`). | Pool tones, rhythm, velocities, ratchets, knob values |
| HITS (`HITS`) | The priority order in which the 16 steps switch on as DENS rises: where the rests fall. | Pool, order, velocities, ratchets, knob values |
| VEL (`VEL`) | The 16 step velocities (50..127). | Pool, order, rhythm, ratchets, knob values |
| RATCH (`RATCH`) | Which steps are ratcheted first as the RATCH morph rises, how many extra hits each gets (1..3), and the chance-roll stream. | Pool, order, rhythm, velocities, knob values |

A sixth action, **dice all**, re-rolls all five seeds in one gesture (the physical button is the control map's job).
**Undo dice**: each layer keeps its previous seed (2 bytes), so the last dice of each layer can be recalled once
(VISION glossary).

## 4. Morph candidates (continuous knob transforms)

Every morph is a **pure function of (seeds, knob values)**: turning a knob back restores the previous pattern exactly
(VISION glossary: "undoable by turning back"). Cost figures are for recomputing the caches after a knob detent (main
loop, not the ISR).

| Morph | Range | Musical effect | Cost per step (rough) |
| --- | --- | --- | --- |
| OCT (octave count `oct`) | 1..4, default 2 | How many octaves the chord pool is spread over: 1 = a tight arpeggio of 2..4 notes, 4 = a wide four-octave run. Changes the pool length and so how the cycle drifts against the 16 steps. | Rebuild the pool list, at most 30 entries |
| DENS (hit count `den`) | 1..16, default 10 | Step `s` fires iff its rank in the HITS-seed order is `< den`. Fewer = sparser. | 1 table read + compare |
| RATCH (ratcheted steps `rat`) | 0..16, default 2 | The first `rat` **firing** steps in the RATCH-seed order become ratcheted (2..4 hits each, hits evenly spaced in the step). Counting only firing steps means every detent changes what you hear. | 16 compares at rebuild; 1 read per step |
| CHANCE (`chn`) | 10..100 % in steps of 5, default 100 | Each hit passes iff a repeatable hashed roll `< chn`; lower = a thinner, shimmering pattern that still repeats the same way every time (the roll depends on the step counter, not on a free-running generator). 100 = no thinning. | 1 hash + compare per hit in the ISR |

Rejected as morphs (kept out of the 4 knobs): chord type and order (dice layers: a discrete choice between
genuinely different things, not a continuum), gate length, swing, division (global settings), loop length, direction,
per-step transpose.

## 5. Fit against the vision (score 0–3 each, with one line of why)
| Criterion | Score | Why |
| --- | --- | --- |
| Quick to a usable sequence (rule 5) | 3 | Selecting ARP plays a full defaulted arpeggio at once: a seeded chord from the global scale, OCT 2, DENS 10, RATCH 2, CHANCE 100, fixed per-channel starting seeds; no setting besides the channel's MIDI channel (the worked example in 6.3 lists the six default channels). |
| Performable with 4 knobs + dice buttons, no menus (rules 1, 4) | 3 | Four knobs, each an audible reversible morph (range, rests, repeats, thinning); five orthogonal dice layers where two (CHORD, ORDER) change the melody's character in a way no other algorithm offers; all deterministic and stateless so there is no hidden mode. |
| Monophonic bassline/lead use (the TB-3 workflow) | 2 | One note per step, strictly monophonic; chord-tone arpeggios are a classic acid/bass-synth idiom and every note is a scale tone. But there is no accent/slide/gate-length lane (velocity stands in for accent) and a 2..4-note pool is repetitive as a bass line on its own. |
| Distinct from the other families (adds something new) | 1 | The *pitch structure* (a chord pool walked in an order) is new: ACID, SNAKE and STAGE all draw independent random scale tones with no harmonic relation. The *rhythm* is not: HITS is the same rank-order density as SNAKE GATES / STAGE RESTS, RATCH is STAGE's ratchet mode, VEL is the same lane as both. Roughly half of ARP duplicates existing designs. |
| Fits the FM-1 budget (RAM < 2 KB per channel, ISR cost small) | 3 | About 170 B per channel (6.4); the ISR does one modulo, a few table reads and a hash per hit; the rebuild is about 110 draws in the main loop. |

## 6. Redesign for this sequencer

- **Proposed name on screen (max 6 characters):** `ARP`
- **K1–K4 morphs (with on-screen labels, max 6 characters each):**
  K1 `OCT`, K2 `DENS`, K3 `RATCH`, K4 `CHANCE`. (The control map owns which context these knob roles apply in.)
- **Dice buttons (which layers):** five layer targets `CHORD`, `ORDER`, `HITS`, `VEL`, `RATCH`, plus dice-all and
  undo-dice. Physical button binding is a control-map table row; this spec only declares the targets.
- **What we drop from the original, and why:**
  - The held-note input, Hold, Key Retrig and Chord Arp: no held-note source exists on a channel (rule 3: white keys
    are step entry) and a channel is monophonic. The note source becomes the CHORD layer.
  - Note Order `Played`/`Chord`/`Follow` and Octave Mode `Random`: `Played` and `Follow` need a player; `Chord` is
    polyphonic; random octaves break rule 4. Five seeded orders replace the order x octave-mode product (12 combos
    behind two menus).
  - Transpose, Length, Chance (per-step) and Map/CC lanes: four more 16-slider lanes is a menu (rule 1). Length is the
    global GATE, chance is one morph, CC is out (owner question 6 for SNAKE applies equally).
  - Division, Dotted/Triplet, Swing, Sync/Freq, BPM, StepDirection, LoopLength: global settings or dropped (owner
    decision 4); the source's per-instance timing is exactly the settings creep rule 1 forbids.
  - Scale on/off: the global scale control owns the scale (owner decision 3).
- **What we improve over the original:**
  - A one-gesture start: seeded content for pool, order, rhythm, velocity and ratchets, where the source needs a held
    chord and hand-edited lanes.
  - Every random decision is seeded per layer, so a layer can be re-rolled alone, undone and saved (rule 4); the
    source's Random buttons rewrite a whole lane and cannot be undone.
  - The pool is **always in the scale** by construction, even on a pentatonic scale, where the source's snap only
    pulls player notes toward the scale (and its direction and root handling are ⚠).
  - The arp index is a pure function of the step counter (6.3), so DENS, OCT or a dice never loses the clock phase;
    the source's stateful counter restarts or drifts when notes change.
  - Chance is repeatable: the same settings play the same pattern, where the source's `random 100` per hit does not
    (rule 4).
  - Ratchet count is defined (hits = 1 + extra), where the source's n vs n+1 is ⚠.
- **Parameters that must not exist (settings creep that breaks rule 1):** a Note Order or Octave Mode selector, Hold,
  Key Retrig, Chord/Step Arp switch, Loop Length, Step Direction, any per-step lane edit page (velocity, length,
  chance, ratchet, transpose, CC), per-lane min/max ranges, Dotted/Triplet, Time Mode/BPM, a scale or root of its own,
  a chord-type menu, a seed entry field.

### 6.1 Parameters (data table; ranges and defaults)

This table is the single source for the on-screen labels, ranges and defaults. A remap is an edit to this table.

| Id | Kind | Label | Range | Default | Stored as |
| --- | --- | --- | --- | --- | --- |
| `oct` | morph, knob 1 | `OCT` | 1..4 | 2 | u8 |
| `den` | morph, knob 2 | `DENS` | 1..16 | 10 | u8 |
| `rat` | morph, knob 3 | `RATCH` | 0..16 | 2 | u8 |
| `chn` | morph, knob 4 | `CHANCE` | 10..100 (step 5) | 100 | u8 |
| `s_chord` | dice seed | `CHORD` | 0..65535 | `(0x9E37 * (ch*5 + 1)) & 0xFFFF` | u16 |
| `s_order` | dice seed | `ORDER` | 0..65535 | `(0x9E37 * (ch*5 + 2)) & 0xFFFF` | u16 |
| `s_hits` | dice seed | `HITS` | 0..65535 | `(0x9E37 * (ch*5 + 3)) & 0xFFFF` | u16 |
| `s_vel` | dice seed | `VEL` | 0..65535 | `(0x9E37 * (ch*5 + 4)) & 0xFFFF` | u16 |
| `s_ratch` | dice seed | `RATCH` | 0..65535 | `(0x9E37 * (ch*5 + 5)) & 0xFFFF` | u16 |
| `p_*` (x5) | previous seed for undo-dice | n/a | 0..65535 | equal to the seed (nothing to undo) | u16 each |
| `pin_mask`, `pin_note[16]` | step-entry overrides (6.5) | n/a | mask 0..65535; offset 0..96 | 0, 0 | u16 + 16 x u8 |

`ch` is the 0-based channel index. The multiplier `0x9E37` (not SNAKE's `0x3A5D`) is **[D]**: with `0x3A5D` four of the
six default channels drew the 2-tone "fifth" chord (the first draw of a small multiplicative seed is poorly mixed),
which is a thin default; `0x9E37` was checked to give varied chord types and orders on channels 0..5 (6.3). A seed of 0
is legal. When the user dices a layer the new seed is the low 16 bits of the audio sample counter at the press (the
only non-deterministic moment; it is stored, so playback afterwards is fully deterministic).

### 6.2 Random primitives (normative, needed for golden files)

Identical to `sting.md` 6.2, repeated so this file stands alone:

- State: unsigned 32-bit `x` (all shifts logical, arithmetic wraps mod 2^32; `/` and `%` are unsigned integer,
  truncating). `init(seed16, salt)`: `x = ((seed16 << 16) | seed16) ^ salt; if x == 0: x = 1; draw 4 values and discard.`
- Salts: CHORD `0x5C1E7A93`, ORDER `0x2D8B46F1`, HITS `0x6A39D0B5`, VEL `0x13F7C82E`, RATCH `0x48E5A1D7` (different
  from every other algorithm's, so no stream is shared).
- `next()`: xorshift32: `x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x`.
- `rnd(n)` (n >= 1) = `(next() >> 8) % n`.
- `shuffle(a, len)` (Fisher-Yates): for `i = len-1 down to 1`: `j = rnd(i+1); swap(a[i], a[j])`.
- `mix32(x)` (the chance hash, **[D]**, no stream state): `x ^= x >> 16; x *= 0x7FEB352D; x ^= x >> 15; x *= 0x846CA68B; x ^= x >> 16`
  (mod 2^32).
- Each layer builds its own state from its own seed and uses no other layer's stream.

### 6.3 Procedure (step by step; every layer is computed from seed only, then morphs are applied)

Steps are 0-based: `s = t % 16`, where `t` is the channel's step counter (a u32 that starts at 0 on transport start
or restart and increments once per step at the global DIV; nothing here is swung).

**Scale table `tbl`/`N`** (the single global scale, owner decisions 3 and 4): `tbl` = the global scale's semitone
degrees ascending, `tbl[0] = 0`, `N` their count (chromatic = `[0..11]`). ARP has **no scale of its own**. Golden
files pass `tbl` explicitly; the reference fixture is minor pentatonic `[0,3,5,7,10]`, `N = 5`. Root and transpose
enter only at output. `snap(v)` = `12*(v/12) + max{ d in tbl : d <= v % 12 }` (nearest scale tone **at or below**,
in the octave of `v`; **[D]**, chosen to match the source's inferred rule [I, 03 §3.7] but not relied on from it).

**CHORD layer** (stream: `init(s_chord, SALT_CHORD)`; always 2 draws)
1. `type = rnd(8)`; `ri = rnd(N)`; `rpc = tbl[ri]` (a root that is a scale tone).
2. Interval sets `IV[type]`: 0 maj `{0,4,7}`, 1 min `{0,3,7}`, 2 sus2 `{0,2,7}`, 3 sus4 `{0,5,7}`, 4 dom7 `{0,4,7,10}`,
   5 min7 `{0,3,7,10}`, 6 maj7 `{0,4,7,11}`, 7 fifth `{0,7}`.
3. `tones` = sort ascending and drop duplicates of `snap(rpc + iv)` for `iv` in `IV[type]`. If fewer than 2 remain,
   append `tones[0] + 12`. So `2 <= nt <= 4`, and every tone is a scale tone (on a sparse scale a major triad turns
   into a minor one: `snap(4) = 3` in minor pentatonic; this is accepted, the pool follows the scale).

**ORDER layer** (stream: `init(s_order, SALT_ORDER)`; always 1 + 15 draws)
1. `ord = rnd(5)`: 0 Up, 1 Down, 2 Mountain, 3 Perm, 4 Weave.
2. `perm = [0..15]; shuffle(perm, 16)` (used only by Perm).

**Pool list** (rebuilt on a CHORD or ORDER seed, OCT, scale, root or transpose change; pure):
1. `U` = for `o in 0..oct-1`, for `tone` in `tones`: `tone + 12*o` (octave-major, ascending). `Lb = nt * oct` (2..16).
2. By `ord`: Up: `U`. Down: `U` reversed. Mountain: `U + reverse(U)[1 : Lb-1]` (the peak and the start are not
   repeated; for `Lb = 2` just `U`). Perm: `U[perm[i]]` for the `i` in `0..15` with `perm[i] < Lb`, in that order (a
   fixed shuffle filtered, so raising OCT keeps the relative order of notes that already existed). Weave: for `tone`
   in `tones`, for `o in 0..oct-1`: `tone + 12*o`.
3. `L` = the list length: `Lb` except Mountain `2*Lb - 2` (`L` is 2..30). `note[i] = clamp(36 + root + list[i] + transpose, 0, 127)`.
   Base 36 (C2) **[D]**, as in ACID, SNAKE and STAGE; the highest possible note is below 36 + 11 + 11 + 11 + 36 = 105
   before transpose (that top is reached only at OCT 4 with a high root, above the TB-3's useful bass range).

**HITS layer** (stream: `init(s_hits, SALT_HITS)`): `ord_h = [0..15]; shuffle(ord_h, 16)`; `rank_h[ord_h[r]] = r`.
**DENS morph:** `fire[s] = (rank_h[s] < den) or pinned(s)`. Because `den >= 1`, at least one step fires.

**VEL layer** (stream: `init(s_vel, SALT_VEL)`): `for s in 0..15: vel[s] = 50 + rnd(78)` (50..127). The range is a
constant **[D]** (the source's lane range 27..100 for Random is only a default [V, 02]; 50..127 matches SNAKE and
STAGE so accents are audible on a TB-3).

**RATCH layer** (stream: `init(s_ratch, SALT_RATCH)`; always 15 + 16 draws, then the chance salt `s_ratch` itself)
1. `ord_r = [0..15]; shuffle(ord_r, 16)`.
2. `for s in 0..15: ex[s] = 1 + rnd(3)` (extra hits, 1..3).
**RATCH morph:** walk `ord_r`; the first `rat` steps with `fire[s]` get `ratch[s] = 1`, all others `0`. So the knob
only counts steps that audibly fire (the same trick as SNAKE's DENS and STAGE's RATCH), and a ratcheted step has
`hits = 1 + ex[s]` (2..4) **[D]**; the source's n vs n+1 is ⚠.

**Firing cache** (rebuilt on a HITS seed, DENS or pin change; pure): `pre[s]` = number of firing steps before `s`
(`pre[0] = 0`), `nfire` = total firing steps (1..16).

**Playback** (the channel engine calls this once per step):
1. `s = t % 16`. If `fire[s] == 0`: no note this step, nothing else happens (like the source, an off step does not
   advance the note engine [V, 04 §4.2]; here the index below simply skips it).
2. `cyc = t / 16`; `i = ((cyc % L) * (nfire % L) + pre[s]) % L`. `i` is the arp index: the `n`-th firing step since
   transport start reads `list[n % L]`, computed without any stored counter (the products are below 900, so no
   overflow). A pinned step (6.5) counts as firing but plays its pinned note instead of `note[i]`, so pinning never
   shifts the notes around it.
3. `pitch = pinned ? clamp(36 + root + pin_note[s] + transpose, 0, 127) : note[i]`; `velocity = vel[s]`.
4. `hits = ratch[s] ? 1 + ex[s] : 1`. Hit `j` (0..hits-1) starts at `j * stepLen / hits` into the step and sounds for
   `(stepLen / hits) * gate_pct / 100` with `gate_pct = 50` **[D]** times the global GATE. Hit `j` plays iff
   `((mix32((t << 2 | j) ^ (s_ratch << 16 | s_ratch)) >> 8) % 100) < chn`. The roll is **per hit** and **after
   ratchet**, as in the source [V, 04 §4.5, 06 §6.2]. At `chn = 100` every hit plays.

All hits stay inside their step, so a note never overlaps the next step's note (monophonic-safe, no hung-note
handling beyond the usual panic).

**Worked example** (reference fixture: minor pentatonic `[0,3,5,7,10]`, root 0, transpose 0, defaults OCT 2, DENS 10,
RATCH 2, CHANCE 100; computed with a throwaway Python model of exactly the procedure above, to be replaced by the
golden files):

| ch | chord (type, tones) | order | note list (MIDI) | firing steps `s=0..15` | ratcheted |
| --- | --- | --- | --- | --- | --- |
| 0 | min `{7,10,12}` (G, Bb, C) | Perm | 60 48 58 46 55 43 | `X...X..XXXXXXXX.` | steps 4, 11 (hits 4, 4) |
| 4 | dom7 snapped to `{0,3,7,10}` (C, Eb, G, Bb) | Up | 36 39 43 46 48 51 55 58 | `XX.XXXX.X.XXX...` | steps 3, 4 (3, 3) |

Channel 4 plays C2 Eb2 G2 Bb2 C3 Eb3 G3 Bb3 over its 10 firing steps, so the first loop reads 36 39 . 43 46 48 51 .
55 . 58 36 39 . . . , and because `L = 8` and `nfire = 10` the note a step reads shifts by 2 each loop, so the pattern repeats only every 4 loops (64 steps): a drifting arpeggio.
(That all six channels give varied, scale-conformant pools was checked while writing; see the task log.)

### 6.3a Dice and knob latency (VISION rule 1: changes feel immediate)

ARP has **no cross-step state**: no stored counter, no note pairing (unlike ACID's slide, which needed the next-step
commit of OWNER_QUESTIONS #9). The arp index (6.3 step 2) and the chance roll are pure functions of `t`. So:

- The new cache is built by the main loop in the frame of the press or detent (about 110 xorshift draws plus a 30-entry
  list build and a 16-entry prefix pass, far less than one 16 ms frame), then swapped under `fm1_irq_off/on`.
- The ISR reads the cache at the **next step**: worst-case latency from press to sound is one frame plus one step
  (about 16 ms + 125 ms at 120 BPM and 1/16, typically half that). No "commit one step ahead".
- A note already sounding keeps the note-off scheduled at its note-on; up to 3 more ratchet hits of the current step
  may already be queued and are allowed to finish (they are inside the current step, so the dice is heard at the next
  step boundary at worst). A stop, panic, channel, algorithm or MIDI-channel change sends the note-offs and CC123 at
  once, so a dice never leaves a hung note.
- OCT and DENS shift the pool/firing pattern phase-locked to `t`, not restart it, so a knob turn never makes the
  pattern jump back to the start of the cycle (a change in `L` or `nfire` changes which note a given step reads, which
  is the intended audible effect).

### 6.4 Channel RAM and CPU

| Item | Bytes |
| --- | --- |
| 4 knob values | 4 |
| 5 seeds + 5 previous seeds | 20 |
| Pin mask + 16 pin notes | 18 |
| Layer scratch (rank_h 16, vel 16, ord_r 16, ex 16, tones 4, ord 1, nt 1) | 70 |
| Playback cache (list 30, pre 16, ratch mask 2, fire mask 2, L 1, nfire 1) | 52 |
| ISR playback state (sounding note 1, pending-off time 4, spare 1) | 6 |
| **Total** | **170 B per channel** (well under 2 KB) |

CPU: a full recompute is 2 + 16 + 15 + 16 + 31 + 16 = about 100 xorshift draws plus the list build; the ISR does two
modulos, one multiply, a few table reads and a `mix32` per hit (about 10 integer ops). Scratch can be regenerated from
the seeds instead of stored (about 60 B per channel), an implementation choice.

### 6.5 Step entry interplay (rule 3: steps on the 16 white keys OR the algorithm)

- A step typed on the white keys while ARP is selected writes a **pin** on step position `i = 0..15`:
  `pin_mask |= 1<<i`, `pin_note[i] = clamp(key note - 36 - root, 0, 96)`. The step is forced to fire with that note
  (6.3 steps 1-3), ignoring DENS, and keeps the generated velocity, ratchet and chance.
- Pins are keyed by **step position**, so they survive every dice and every morph. Loop length is fixed at 16, so every
  pin is always reachable (unlike STAGE's TOTAL; owner question 10 does not arise here).
- Clearing a pin (a control-map action) returns the step to the generated line.
- The key-entry gesture itself belongs to the channel/step-entry spec and must cite this section.

### 6.6 Flex points (ADR-0004 addendum): what is a table edit, what is not

Remappable by editing a table row, no algorithm code change:
- Which knob drives which morph (K1..K4 <-> OCT/DENS/RATCH/CHANCE), each morph's label, range, default.
- Which physical button dices which layer, and the gesture for dice-all and undo-dice.
- Fixed constants: base note 36, velocity range 50..127, chord interval table `IV` and its size, ratchet extra-hit range
  (1..3), `gate_pct` 50, the CHANCE floor (10) and step (5), the default seed multiplier `0x9E37`, DENS and RATCH defaults,
  the OCT cap (4).
- Adding a dice layer that reuses an existing stream, for example splitting ORDER into walk order vs permutation, or a
  PATH-style knob that picks the order instead of ORDER being a dice.

Not changeable without algorithm code: the xorshift generator, salts and `mix32` (they define the golden files), the
shuffle method, the order-based morph principle (monotonic/reversible), the pure `t`-derived arp index and chance roll,
the tone-pool-then-walk structure, one trigger per step (monophonic), adding a genuinely new lane (a per-step
transpose, accent, length or CC lane), and the 16-step loop.

## 7. Risks and open questions

Design points that rely on ⚠ or [I] claims, and how each is neutralised:

| # | Claim in source | Where our design touches it | Treatment |
| --- | --- | --- | --- |
| 1 | Scale snap direction (at or below vs nearest) and root handling ⚠ [I] (03 §3.7, 06 §6.6 #4) | `snap()` for chord tones | Not relied on: the rule is defined here **[D]** (at or below, pitch class of the global scale). The source only motivates the choice. If nearest is preferred it is a one-line change to a shared helper. |
| 2 | `Random` order reshuffles per trigger or per note change ⚠ (03 §3.2, 06 §6.6 #1) | ORDER = Perm | Not relied on: Perm is a seeded permutation fixed by the ORDER seed **[D]**. |
| 3 | Octave Steps "+1" convention (setting 1 = 1 octave?) ⚠ (03 §3.3, 06 §6.6 #2) | OCT knob | Not relied on: OCT is the octave count, 1..4 **[D]**. |
| 4 | Mountain list at the peak and the start: repeated or not ⚠ (03 §3.8, 06 §6.6 #3) | ORDER = Mountain | Not relied on: no repeat of peak or start **[D]**, stated in 6.3. The construction rule itself is [V, 03 §3.4]. |
| 5 | Ratchet hit count (n vs n+1) and the off path ⚠ (04 §4.6, 06 §6.6 #5) | RATCH | Not relied on: `hits = 1 + extra` **[D]**; no "ratchet off" state exists (a morph at 0 means no step is ratcheted). |
| 6 | Alternate/Random StepDirection details ⚠ (05 §5.4) | none | Direction dropped. |
| 7 | Swing mapping ⚠ (05 §5.2, 06 §6.6 #7) | none | Swing is a global setting. |
| 8 | Map lane keeps sending for off steps ⚠ (04 §4.8, 06 §6.6 #8) | none | CC lane dropped. |
| 9 | Octave Mode default ⚠ (02, 06 §6.6 #9); Transpose widget range ⚠ (02) | none | Neither exists in the design. |

Relied on and **[V]**: the note engine decoupled from the step row, with off steps not advancing it (04 §4.2); the
octave expansion formulas and the mountain construction (03 §3.3-3.4); chance applied per hit after ratchet and
`chance 100 = always` (04 §4.5, 06 §6.5); the pipeline order scale then ratchet then chance (06 §6.2); Length as a
fraction of the step, which becomes the global GATE (04 §4.4). **No design point depends on an unresolved ⚠ claim.**

Other open questions for the owner or other specs (not source ⚠):
- **Overlap (for P1-010):** ARP's HITS, VEL and RATCH are the same mechanisms as SNAKE GATES/VEL and STAGE RESTS/VEL/RATCH
  (seeded rank order + count morph; seeded 50..127 velocity lane; ratchet on firing steps only). If ARP is included,
  those should be **one shared primitive** in the firmware (one rank-order density function, one ratchet scheduler),
  so the cost of ARP is only the CHORD/ORDER/pool code and the chance hash. If ARP is not included, the CHORD/ORDER
  idea could still reach other algorithms as a pitch option (owner question 3 below).
- **Pool size and a bass line:** a 2..4-note chord pool is a figure, not a bass line (see score 2). Is it enough on a TB-3,
  or does it want a root-note bias (root on the downbeat)? Not designed (it would be a new constant, not a setting).
- **Chord snap on sparse scales:** a major triad becomes a minor triad in minor pentatonic (6.3 CHORD). Acceptable (the
  pool follows the scale), or should the global scale control expose a chord-friendly mapping? Depends on owner question
  2 (how the scale snap works) and 4 (global scale default).
- Global settings (owner decisions 3 and 4): scale/root/transpose and DIV/GATE/SWING are global; ARP reads them and owns
  none. Until the global spec locks, the reference implementation uses: step = one 1/16 note, `gate_pct` 50 unscaled,
  no swing, root 0, transpose 0, the fixture scale above.
- Owner re-lock needed (ADR-0004) after the reference implementation and golden files exist; none exist yet.

## 8. Recommendation

**LATER.** Step Arp's one new idea, a pitch order over a small chord-tone pool, adds a harmonic, arpeggio-shaped
melody that ACID, SNAKE and STAGE (all independent random scale tones with no relation between notes) cannot make,
and the redesign is clean: it is stateless and so latency-free, deterministic, fits in about 170 B per channel, and
avoids every unverified source behaviour. But for v1 it is the weakest of the catalogue to ship: its defining input
(held keys) does not exist here, so the algorithm has to be rebuilt as a seeded chord generator; its rhythm half
(HITS, VEL, RATCH) duplicates SNAKE and STAGE, so the real new content is only CHORD, ORDER and the OCT morph; and a
2..4-note pool is repetitive as a bass line on its own. Ship ACID, SNAKE and STAGE first (with the shared rank-order
and ratchet primitives), then promote ARP in a later phase as a pitch-structure option with this design as written
(about two days of work on top of those primitives). If P1-010 finds the v1 set needs a harmonic algorithm, the case
for INCLUDE is strongest as a cheap fourth algorithm on those shared primitives.
