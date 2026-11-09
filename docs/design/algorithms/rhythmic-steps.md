# Assessment: Rhythmic Steps (layered drum-step sequencer)

Source spec: `docs/rhythmic-steps-algorithms/` (read all of it, including the 19 decoded presets in 07). Vision:
docs/project/VISION.md. Confidence tags ([V]/[I]/⚠) are the source's; **[D]** marks a decision made in this document
(no fidelity claim). ⚠ is never promoted to fact; every design point that touches one is listed in section 7.

Design stance: a **redesign, not a port**, in the style of `sting.md`, `snake.md`, `ml185.md` and `steparp.md`. Three
facts about the source shape the redesign:

1. It is **not a generator**. Its only generator is a uniform per-layer "Random" (levels 0..6) and a "Not-so-random"
   mutate [V, 04 §4.4]. The grooves in its 19 presets are hand-made (chance levels: 800 zeros, 295 sixes, 50 ones,
   50 threes; roll used on 37 of 1,216 cells [V, 04 §4.6]). A random fill does not make a drum groove, so the
   redesign has to invent a **groove-first hit placement** [D].
2. It is **four lanes, one drum pad each, on MIDI channels 2..5** [V, 05 §5.7]. In this sequencer a channel is one
   lane (rule 3), so four RHYTHM channels on four MIDI channels give the same four-voice kit. No state is shared
   between lanes in the source [V], so nothing is lost.
3. It has **no pitch material**: a lane plays a main and an alternate pad note [V, 05 §5.2]. That is the central
   mismatch with the TB-3 workflow (section 5, row 3) and the reason for the verdict.

Layers, morphs and dice are declared as data tables (ADR-0004 addendum); the tables are in 6.1. The random primitives
are those of `sting.md` 6.2 (xorshift32, `init(seed16, salt)`, `rnd(n)`, `shuffle`, plus `mix32` from `steparp.md` 6.2),
with new salts.

Verdict preview (detail in section 8): **LATER**. It is the only family aimed at a drum voice, but most of its
mechanisms already exist in SNAKE, STAGE and ARP, it has no melody, and it does not bring a rhythm generator that
those lack (it has no Euclidean-style placement). Two small ideas in it are worth lifting into the shared primitives
regardless (section 8). The full design is below so it can be promoted without rework.

## 1. What it does (5 lines max)

Four lanes, each with five 16-step layer sequences: Chance, Velocity, Alt, Roll and Slide, every cell a level 0..6
[V, 01, 04]. Each layer has its own length 2..16 and its own counter, so a short Velocity layer drifts across a
16-step Chance layer and the combined loop is the LCM of the lengths (polymeter) [V, 04 §4.3]. Per step a lane rolls
Chance (`level > random(6)`), picks the main or alternate pad by the Alt level, scales velocity 10..127, adds up to 4
quick repeats at ×0.7 velocity, and sends CC74 slide [V, 05]. In this sequencer it becomes **RHYTHM**: one lane per
channel, groove-ordered seeded hits with per-step ghost-note chances, a velocity cycle that drifts against the hit
cycle, an alternate pitch, and rolls.

## 2. Generators inside it
| Generator | Inputs | Output | Deterministic? (seeded / free-running) |
| --- | --- | --- | --- |
| Layer sequencer (counter `1..length`, 16 stored cells) [V, 04 §4.1] | step bang, length | cell value at the counter | Pure given the counter. **Kept [D]** as `t % length`, a pure function of the step counter `t` (no stored counter, so a morph never desyncs the loop) |
| Chance roll `level > random(6)` [V, 05 §5.1] | chance level | play / skip | Free-running `random`. **Redefined [D]** as a repeatable hash of `t` and the CHANCE seed |
| Alt roll `level > random(6)` [V, 05 §5.2] | alt level | main or alt pad | Free-running. **Redefined [D]**: its own hash, independent of the chance roll. The source's ⚠ question of whether one draw serves both (06 §6.4 #8) does not arise |
| Velocity map `10 + level*117/6` [V, 05 §5.3] | velo level | 10..127 | Pure. **Kept** (10, 29, 49, 68, 88, 107, 127) |
| Roll (repeats at Roll Rate, ×0.7 each) [V structure, ⚠ n vs n+1, 05 §5.5] | roll level 0..4 | extra hits | Pure given the level. **Redefined [D]**: `hits = 1 + extra`, hits evenly spaced inside the step, velocity ×0.7 per repeat (the ×0.7 is [V]) |
| Random (uniform 0..6 per cell) [V, 04 §4.4] | layer | 16 cells | Free-running. **Replaced** by per-layer seeded draws from skewed tables (6.3), because uniform levels make a bad groove (preset statistics, above) |
| Not-so-random (each cell -1/0/+1, clip 0..6) [V, 04 §4.4] | layer | 16 cells | Free-running, destructive. **Redefined [D]** as the reversible NUDGE counter (6.3): `nd` presses = `nd` fixed seeded ±1 walks, so step back = `nd-1` |
| Reset (velo 4, others 0) [V, 04 §4.4] | layer | clear | **Dropped**: dice replaces clearing |
| Time shuffle: 8 per-step offsets ±0.1 beat, Shuffle Length, Global Shuffle [V, ⚠ length rule and crossing, 03 §3.2] | 8 sliders | step times | **Dropped**: the global SWING (owner decision 4) |
| Clock Speed [V structure, ⚠ label to factor, 03 §3.3] | enum | step rate | **Dropped**: the global DIV |
| Auto Retrigger [V structure, 03 §3.4] | interval | counter reset | **Dropped**: `t` restarts at transport start, and the longest cycle here is `lcm(len, vln) <= 240` steps |
| Delay ms (0..50), Mute, Lock Length / Global Length [V; Lock semantics ⚠, 04 §4.2] | per-lane | hit offset / silence / one shared length | **Dropped** (section 6) |
| Slide layer to CC74 (MPE) [V, 05 §5.4] | slide level | CC74 before note-on | **Dropped**: not a note parameter on a TB-3 (same as SNAKE's CC lanes, owner question 6) |
| Pad Note / Pad Alt Note (36..51) [V] | per-lane | drum notes | **Replaced [D]** by a fixed base note and base + 12 (owner question 17) |

## 3. Layers (what can be diced independently)

Five layers, each with its own 16-bit seed. In the source all five layers of a lane are already independent stores
[V, 01 §"central design"]; this design gives each its own seed so one can be re-rolled alone. The on-screen name is the
dice label (max 6 characters).

| Layer (screen label) | What dicing it changes | What stays fixed |
| --- | --- | --- |
| HITS (`HITS`) | The groove template (KICK, BACK, OFF or FREE) and the priority order in which steps switch on as DENS rises: where the hits and rests fall. | Ghost chances, alt pads, velocities, rolls, all four knob values, root |
| CHANCE (`CHANCE`) | The 16 per-step chance levels 0..6 (mostly 6, some ghost steps at 2, 3 or 5) and the repeatable hash stream of the play/skip rolls. So which hits are ghost notes and which ghosts actually sound. | Hit placement, alt pads, velocities, rolls, knob values |
| ALT (`ALT`) | The 16 per-step alt levels (mostly 0, a few 50 % or always): which hits play the alternate pitch. | Hit placement, chances, velocities, rolls, knob values |
| VEL (`VEL`) | The 16 velocity levels (mostly level 4 = 88, a few accents and softer steps). | Everything else |
| ROLL (`ROLL`) | Which firing steps get rolled first as the ROLL knob rises, and how many extra repeats each gets (1..3). | Hit placement, chances, alt pads, velocities, knob values |

A sixth action, **dice all**, re-rolls all five seeds in one gesture. **Undo dice**: each layer keeps its previous
seed and nudge count (3 bytes of nudge counters in all), so the last dice of each layer can be recalled once (VISION
glossary). A seventh action family is **nudge** (6.3, NUDGE): `+1` / `-1` on CHANCE, ALT or VEL, a small reversible
mutation instead of a full re-roll. The physical buttons for all of these are the control map's job; this spec only
declares the targets.

## 4. Morph candidates (continuous knob transforms)

Every morph is a **pure function of (seeds, nudge counts, knob values)**: turning a knob back restores the previous
pattern exactly (VISION glossary). Cost figures are for recomputing the caches after a knob detent (main loop, not
the ISR).

| Morph | Range | Musical effect | Cost per step (rough) |
| --- | --- | --- | --- |
| DENS (hit density `den`) | 1..16, default 6 | Number of firing steps in the loop is `nh = ceil(den*len/16)`; step `c` fires iff its rank among the first `len` cells of the HITS order is `< nh`. With a KICK template: 1 = the downbeat only, 2 adds step 8, 4 = the four quarter notes, then the 16th off-beats; 16 = every step. | 1 table read + compare |
| LEN (hit-cycle length `len`) | 2..16, default 16 | The rhythm layers (hits, chance, alt, roll) wrap at `len`: 16 = one bar, 12 or 10 = a polymetric loop against the bar, 5 = a tight cell. Cells beyond `len` are kept and return when it is raised [V, 04 §4.1]. The source has five independent lengths; here the rhythm layers share one (6, drop list). | Rebuild rank table, 16 compares |
| DRIFT (velocity-cycle length `vln`) | 2..16, default 3 | The VEL layer wraps at `vln` independently of `len`, so the accents slide across the hits: with `len` 16 and `vln` 3 the accent pattern realigns only every 48 steps [V, 04 §4.3, the source's "main creative mechanism"]. 16 = accents locked to the bar. | 1 modulo |
| ROLL (rolled steps `rol`) | 0..8, default 2 | The first `rol` **firing** steps in the ROLL-seed order are rolled (2..4 hits each). Counting only firing steps means every detent changes what you hear. | 16 compares at rebuild; 1 read per step |

Rejected as morphs (kept out of the 4 knobs): ghost-note probability (a dice layer, CHANCE, plus NUDGE), alt amount
(dice layer ALT), gate, swing, division (global settings), per-layer lengths beyond LEN and DRIFT, delay, roll rate,
mute (a channel function), velocity amount.

## 5. Fit against the vision (score 0–3 each, with one line of why)
| Criterion | Score | Why |
| --- | --- | --- |
| Quick to a usable sequence (rule 5) | 3 | Selecting RHYTHM plays a groove at once: groove-ordered hits (4 channels draw four different templates), ghost steps, a drifting accent cycle, one fixed note per channel (6.3 worked example). The only setup is the MIDI channel. |
| Performable with 4 knobs + dice buttons, no menus (rules 1, 4) | 2 | Four reversible morphs and five dice layers plus nudge, all deterministic with no hidden mode. But DRIFT and LEN are subtle until a user understands polymeter, and a pitchless pattern has less to dial than a melodic one. |
| Monophonic bassline/lead use (the TB-3 workflow) | 1 | Strictly one note per step, but only two pitches exist (base and base + 12) and nothing is melodic. It is a one-note pulse on a TB-3 and a drum voice on a drum machine, which is not the reference workflow (VISION, "Reference workflow"). |
| Distinct from the other families (adds something new) | 1 | Rank-order density = SNAKE GATES / STAGE RESTS / ACID HITS; ROLL = STAGE and ARP ratchet; VEL = the same lane in all three; an independent velocity cycle length = SNAKE POLY; alt pitch ~ ACID OCT; chance = ARP CHANCE. New: groove-template hit order, per-step ghost chances, NUDGE. See the overlap table in 6.7. |
| Fits the FM-1 budget (RAM < 2 KB per channel, ISR cost small) | 3 | About 160 B per channel (6.4); the ISR does two modulos, a few reads and two `mix32` per step. |

## 6. Redesign for this sequencer

- **Proposed name on screen (max 6 characters):** `RHYTHM`
- **K1–K4 morphs (with on-screen labels, max 6 characters each):**
  K1 `DENS`, K2 `LEN`, K3 `DRIFT`, K4 `ROLL`. (The control map owns which context these knob roles apply in.)
- **Dice buttons (which layers):** five layer targets `HITS`, `CHANCE`, `ALT`, `VEL`, `ROLL`, plus dice-all, undo-dice
  and nudge +/- on `CHANCE`, `ALT`, `VEL`. Physical button binding is a control-map table row.
- **What we drop from the original, and why:**
  - Four lanes in one instance: they become four channels of the same algorithm, each with its own MIDI channel
    (rule 3). No lane-to-lane link exists in the source [V].
  - Five independent layer lengths plus Lock/Global Length (up to 5 + 2 controls per lane): collapsed to two morphs,
    LEN for the rhythm layers and DRIFT for velocity. That keeps the polymeter idea (the source's central mechanism)
    without a length menu (rule 1), and sidesteps the ⚠ Lock semantics (04 §4.2, 06 §6.4 #1).
  - Slide / CC74 and the MPE channel scheme (lanes on channels 2..5): the channel's one setting is its MIDI channel;
    CC output is not a note algorithm (SNAKE precedent).
  - Delay, Mute, Roll Rate, Clock Speed, per-step Time Shuffle, Auto Retrigger: global settings (DIV, SWING, GATE per
    owner decision 4), a channel function (mute), or settings creep (delay, roll rate).
  - Pad Note / Pad Alt Note selectors: a note chooser is a per-channel setting. Replaced by a fixed base note and
    base + 12, with step entry able to override (6.5). Owner question 17.
  - Random / Reset / hand editing of 320 cells: replaced by seeded dice, NUDGE, and white-key pins.
- **What we improve over the original:**
  - A one-gesture start: seeded content for all layers, groove-ordered, where the source starts empty (Reset
    clears) and relies on factory presets.
  - **Groove-first hit placement (new):** the HITS order follows a metric priority template (KICK, BACK, OFF, FREE)
    instead of a uniform shuffle, so DENS 1..4 on a KICK channel gives a downbeat, then a half-bar, then the four
    on-beats, as a drummer would add them. This is the one thing in this family that the other four lack.
  - Every random decision is seeded per layer, undoable and repeatable (rule 4); the source's `random` rolls are not.
  - Ghost-note chance is per step (a seeded level), but the roll is a repeatable hash of `t`: the same settings play
    the same pattern every time.
  - NUDGE: a reversible version of the source's "Not-so-random" (a destructive random walk).
  - Roll hit count is defined (`1 + extra`), where the source's n vs n+1 is ⚠.
- **Parameters that must not exist (settings creep that breaks rule 1):** per-layer lengths, Lock Length, Global
  Length, a Pad Note or Alt Note selector, per-lane Delay, Mute (on the algorithm; mute is a channel function), Roll
  Rate, Clock Speed, Shuffle sliders and Shuffle Length, Auto Retrigger and its interval, a CC74/MPE output, a seed
  entry field, a template picker (the template is chosen by the HITS dice).

### 6.1 Parameters (data table; ranges and defaults)

This table is the single source for the on-screen labels, ranges and defaults. A remap is an edit to this table.

| Id | Kind | Label | Range | Default | Stored as |
| --- | --- | --- | --- | --- | --- |
| `den` | morph, knob 1 | `DENS` | 1..16 | 6 | u8 |
| `len` | morph, knob 2 | `LEN` | 2..16 | 16 | u8 |
| `vln` | morph, knob 3 | `DRIFT` | 2..16 | 3 | u8 |
| `rol` | morph, knob 4 | `ROLL` | 0..8 | 2 | u8 |
| `s_hits` | dice seed | `HITS` | 0..65535 | `(0x9E37 * (ch*5 + 1)) & 0xFFFF` | u16 |
| `s_chance` | dice seed | `CHANCE` | 0..65535 | `(0x9E37 * (ch*5 + 2)) & 0xFFFF` | u16 |
| `s_alt` | dice seed | `ALT` | 0..65535 | `(0x9E37 * (ch*5 + 3)) & 0xFFFF` | u16 |
| `s_vel` | dice seed | `VEL` | 0..65535 | `(0x9E37 * (ch*5 + 4)) & 0xFFFF` | u16 |
| `s_roll` | dice seed | `ROLL` | 0..65535 | `(0x9E37 * (ch*5 + 5)) & 0xFFFF` | u16 |
| `nd_chance`, `nd_alt`, `nd_vel` | nudge counters | n/a | 0..8 | 0 | u8 each |
| `p_*` (x5), `pn_*` (x3) | previous seed / nudge for undo-dice | n/a | as above | equal to the current value | u16 each / u8 each |
| `pin_mask`, `pin_note[16]` | step-entry overrides (6.5) | n/a | mask 0..65535; offset 0..96 | 0, 0 | u16 + 16 x u8 |

`ch` is the 0-based channel index. The multiplier `0x9E37` is **[D]** (ARP's value): it was checked that channels
0..3 draw four different groove templates (OFF, BACK, FREE, KICK), where `0x3A5D` drew KICK on five of six. When the
user dices a layer, the new seed is the low 16 bits of the audio sample counter at the press (the only
non-deterministic moment; it is stored). A dice also sets that layer's nudge counter to 0. A seed of 0 is legal.

### 6.2 Random primitives (normative, needed for golden files)

`xorshift32`, `init(seed16, salt)`, `next()`, `rnd(n)` and `shuffle` are exactly `sting.md` 6.2; `mix32` is exactly
`steparp.md` 6.2 (no state). Salts (different from every other algorithm's):

| Use | Salt |
| --- | --- |
| HITS stream | `0x7B21C4E9` |
| CHANCE stream | `0x3E96A05D` |
| ALT stream | `0x51D8F2B7` |
| VEL stream | `0x0AC4E316` |
| ROLL stream | `0x64F9B18C` |
| NUDGE hash, layer CHANCE / ALT / VEL | `0x2B7E1593` / `0x9D41F6A2` / `0x35C8D0E7` |
| Alt-roll hash extra xor | `0x5A5A5A5A` |

Each layer builds its own state from its own seed and uses no other layer's stream.

### 6.3 Procedure (step by step; every layer is computed from seed only, then morphs are applied)

Steps are 0-based. `t` is the channel's step counter (u32, 0 at transport start, +1 per step at the global DIV; no
swing here). RHYTHM uses **no scale** (it is pitchless); root and transpose enter at output (below).

**HITS layer** (stream `init(s_hits, SALT_HITS)`; draw order is normative)
1. `tmpl = rnd(4)`: 0 KICK, 1 BACK, 2 OFF, 3 FREE.
2. Tier lists (cell sets, in priority order): KICK `{0} {8} {4,12} {2,6,10,14}`; BACK `{4,12} {7,15} {3,10} {0,8}`;
   OFF `{2,6,10,14} {0,4,8,12}`; FREE: none. For each tier in order: copy it, `shuffle` it, append to `order`.
3. Last tier: every cell not yet in `order`, ascending, `shuffle`d, appended. `order` is a permutation of 0..15.

**CHANCE layer** (`init(s_chance, SALT_CHANCE)`): `for c in 0..15: lc[c] = CT[rnd(8)]`, `CT = {6,6,6,6,6,5,3,2}`
(62 % always, the rest ghosts at 2/3 to 1/3 odds, echoing the preset statistics).
**ALT layer** (`init(s_alt, SALT_ALT)`): `la[c] = AT[rnd(8)]`, `AT = {0,0,0,0,0,0,3,6}` (75 % never, 12 % half, 12 % always).
**VEL layer** (`init(s_vel, SALT_VEL)`): `lv[c] = VT[rnd(8)]`, `VT = {4,4,4,4,4,3,5,6}` (level 4 = 88 is the preset norm).
**ROLL layer** (`init(s_roll, SALT_ROLL)`): `ord_r = [0..15]; shuffle(ord_r)`; then `for c in 0..15: ex[c] = 1 + rnd(3)`
(15 + 16 draws).

**NUDGE** (only on `lc`, `la`, `lv`; the counter `nd` is 0..8): for layer cell `c`, `v = base[c]`; `for j in 0..nd-1:
d = (mix32(((seed16 << 16) | (j << 8) | c) ^ SALT_NUDGE_layer) >> 8) % 3 - 1; v = clamp(v + d, 0, 6)`. So `nd` is a
fixed seeded bounded walk; `nd - 1` restores the previous state exactly. At 8, further `+` does nothing; dice
returns it to 0.

**Morph application** (pure; rebuilt in the main loop):
- `ln = len`. `f` = the cells of `order` that are `< ln`, in `order` sequence; `rank[f[i]] = i`. `nh = (den*ln + 15) / 16`
  (>= 1). `fire[c] = (c < ln) and (rank[c] < nh)` (or pinned, 6.5).
- Roll flags: walk `ord_r`; the first `rol` cells `c` with `c < ln` and `fire[c]` get `roll[c] = 1`; the rest 0.
  A rolled step has `hits = 1 + ex[c]` (2..4), an unrolled step 1.
- Velocity levels `lv` and chance/alt levels `lc`, `la` come from the layers (after NUDGE).

**Playback** (the channel engine calls this once per step):
1. `c = t % ln`. If `fire[c] == 0`: no note, nothing else happens.
2. **Chance gate (per step, before any roll hits):** the step plays iff `((mix32(t ^ ((s_chance << 16) | s_chance)) >> 8)
   % 6) < lc[c]`. Level 6 always plays, level 0 never, level `n` plays `n/6` of the time [V, 05 §5.1; our hash is **[D]**].
   A pinned step skips this gate.
3. **Alt pad (per step):** `alt = ((mix32(t ^ ((s_alt << 16) | s_alt) ^ 0x5A5A5A5A) >> 8) % 6) < la[c]`.
   All hits of the step use the same pad (no mid-roll pad change).
4. `pitch = clamp(36 + root + transpose + (alt ? 12 : 0), 0, 127)`; a pinned step plays its pinned note and alt is
   ignored. Base 36 (C2) **[D]**, as in the other algorithms.
5. `v0 = 10 + lv[t % vln] * 117 / 6` (integer division; levels 0..6 give 10, 29, 49, 68, 88, 107, 127 [V]).
   Hit `j` (0..hits-1) velocity `v_j = max(1, v_(j-1) * 7 / 10)` [×0.7 is V; the integer rounding is **[D]**].
6. Hit `j` starts at `j * stepLen / hits` into the step and sounds for `(stepLen / hits) * gate_pct / 100` with
   `gate_pct = 50` **[D]** times the global GATE. All hits stay inside their step (monophonic-safe).

**Worked example** (computed with a throwaway Python model of exactly this procedure, to be replaced by the golden
files; defaults `den 6, len 16, vln 3, rol 2`, root 0, transpose 0; `X` = firing step, events are the first 16 steps;
`n`/`A` = normal/alt pitch, numbers are hit velocities):

| ch | seeds (hits, chance, alt, vel, roll) | template | firing steps | rolled (step: extra hits) | first 16 steps |
| --- | --- | --- | --- | --- | --- |
| 0 | 9E37 3C6E DAA5 78DC 1713 | OFF | `X.X.X.X...X...X.` | 6: 1, 10: 1 | A68 . . . . . n68/47 . . . . . . . A88 . |
| 1 | B54A 5381 F1B8 8FEF 2E26 | BACK | `...XX..X..X.X..X` | 4: 2, 7: 3 | . . . n88 n68/47/32 . . n68/47/32/22 . . n68 . n88 . . n88 |
| 3 | E370 81A7 1FDE BE15 5C4C | KICK | `X.X.X...X.X.X...` | 0: 2, 2: 3 | n88/61/42 . n68/47/32/22 . n127 . . . n68 . . . n88 . . . |

Channel 3 (KICK): `lv = 4 6 3 4 5 6 4 4 4 6 5 6 3 4 6 4`; with `vln 3` step 0 reads `lv[0]=4` (88), step 4 reads `lv[1]=6`
(127), step 8 reads `lv[2]=3` (68): the accent cycle drifts. Raising `len` is exercised by `len 12` giving
`X.X.X...X.X.....` (5 firing) and `len 5` giving `X...X...........` (2 firing) for the same seeds. Checks run while
writing: `den` 1..16 gives hit counts 1..16 and each set contains the previous (monotonic), and `den 6 -> 9 -> 6`
restores the pattern bit for bit. NUDGE on channel 3 VEL: `nd 0` = `4 6 3 4 5 6 4 4 4 6 5 6 3 4 6 4`, `nd 1` =
`3 6 4 4 4 5 5 5 3 6 6 5 4 3 6 3`, `nd 2` = `4 6 5 4 5 6 6 4 3 6 6 6 5 4 5 3`, and stepping back to `nd 1` is identical
to the first nudge.

### 6.3a Dice and knob latency (VISION rule 1: changes feel immediate)

RHYTHM has **no cross-step state**: no stored counter, no note pairing (unlike ACID's slide). Both the layer indices
(`t % ln`, `t % vln`) and the chance and alt rolls are pure functions of `t`. So:

- The new cache is built by the main loop in the frame of the press or detent (about 100 xorshift draws plus a rank
  table, a 16-cell NUDGE pass of at most 8 hashes per cell for three layers, and a 16-entry flag pass: far below one
  16 ms frame), then swapped under `fm1_irq_off/on`.
- The ISR reads the cache at the **next step**: worst-case latency from press to sound is one frame plus one step
  (about 16 ms + 125 ms at 120 BPM and 1/16, typically half that). No "commit one step ahead" is needed.
- A note already sounding keeps the note-off scheduled at its note-on; up to 3 more roll hits of the current step
  may already be queued and finish (they are inside the current step). A stop, panic, channel, algorithm or
  MIDI-channel change sends note-offs and CC123 at once, so a dice never leaves a hung note.
- LEN and DRIFT shift the phase-locked `t % len` read, they do not restart the loop, so a knob turn never jumps the
  pattern back to its start.

### 6.4 Channel RAM and CPU

| Item | Bytes |
| --- | --- |
| 4 knob values | 4 |
| 5 seeds + 5 previous seeds | 20 |
| 3 nudge counters + 3 previous | 6 |
| Pin mask + 16 pin notes | 18 |
| Layer scratch (order 16, lc 16, la 16, lv 16, ord_r 16, ex 16, template 1) | 97 |
| Playback cache (rank 16 packed to bits and fire mask 2, roll mask 2, nh 1) | 5 |
| ISR playback state (sounding note 1, pending-off time 4, spare 1) | 6 |
| **Total** | **156 B per channel** (well under 2 KB; the scratch can be regenerated from the seeds instead of stored) |

CPU: a full recompute is about 100 xorshift draws plus the NUDGE pass; the ISR per step does two modulos, a few table
reads and two `mix32` (about 20 integer ops); extra roll hits add only a multiply and a schedule.

### 6.5 Step entry interplay (rule 3: steps on the 16 white keys OR the algorithm)

- A step typed on the white keys while RHYTHM is selected writes a **pin** at step position `i = 0..15`:
  `pin_mask |= 1<<i`, `pin_note[i] = clamp(key note - 36 - root, 0, 96)`. The step fires with that note, ignoring
  DENS, the chance gate and the alt roll, and keeps the generated velocity and roll.
- Pins are keyed by step position, so they survive every dice and morph. A pin at `i >= len` is stored but silent
  until LEN is raised (the source behaves the same way for cells past the length [V, 04 §4.1]); owner question 18.
- This is the natural drum-pad entry: each key picks the pad note of that step. The key-entry gesture itself belongs
  to the channel/step-entry spec and must cite this section.

### 6.6 Flex points (ADR-0004 addendum): what is a table edit, what is not

Remappable by editing a table row, no algorithm code change:
- Which knob drives which morph (K1..K4 <-> DENS/LEN/DRIFT/ROLL), labels, ranges, defaults.
- Which physical button dices which layer, and the gestures for dice-all, undo-dice and nudge +/-.
- Constants: base note 36 and the alt offset (+12), the level tables `CT`, `AT`, `VT`, the tier lists of the four
  templates (and adding a fifth template, which only widens `rnd(4)`; this changes the golden files), the 0.7 roll
  decay, the extra-hit range 1..3, `gate_pct` 50, the DRIFT default, the NUDGE cap of 8, and the default seed
  multiplier.
- Promoting `len` and `vln` to dice layers (a seeded length instead of a knob) or giving ROLL a second morph.

Not changeable without algorithm code: the xorshift generator, salts and `mix32` (they define the golden files), the
shuffle method, the order-based morph principle (monotonic and reversible), the pure `t`-derived indices and rolls,
one trigger per step (monophonic), and any genuinely new lane (a pitch lane, a CC lane).

### 6.7 Overlap with the four existing assessments (for P1-010 to dedupe)

| RHYTHM part | Already in | Verdict |
| --- | --- | --- |
| HITS rank order + DENS count | ACID HITS/DENS, SNAKE GATES/DENS, STAGE RESTS/DENS, ARP HITS/DENS | **Duplicate**: same seeded rank-order primitive. Only the order *source* differs (groove template vs uniform shuffle), see below |
| VEL layer (seeded velocities) | SNAKE VEL, STAGE VEL, ARP VEL (ACID uses ACCNT, a rank-order accent) | **Duplicate** |
| ROLL layer + ROLL knob | STAGE RATCH (mode II), ARP RATCH | **Duplicate**: same "first n firing steps in a seeded order get repeats" primitive. Only the ×0.7 velocity decay is extra |
| DRIFT (velocity cycle length against the hit cycle) | SNAKE POLY (velocity-lane length 3..12 as a dice) | **Near-duplicate**: SNAKE has it as a dice, RHYTHM as a knob over 2..16 |
| LEN (hit-cycle length) | SNAKE LEN (pitch lane), STAGE TOTAL | **Partial**: loop-length morphs exist, but not on the rhythm itself with a fixed pitch |
| CHANCE (per-step ghost levels) | ARP CHANCE (one global morph, hashed roll) | **Partial**: ARP has one global thinning, RHYTHM has per-step seeded levels. The hashed-roll technique is the same |
| ALT (second pitch per hit) | ACID OCT (octave jumps, per-step probability) | **Near-duplicate**: a 2-pitch version of ACID OCT |
| **Groove-template hit order** | none | **New** |
| **NUDGE (reversible mutate)** | none (all four use full re-roll only) | **New** |
| Pitchless / drum-voice use | none | **New as a use case**, not as a mechanism |

**Does this family bring a rhythm generator the others lack, such as Euclidean-style hit placement?** Not as such. The
source has no regular-spacing or Euclidean generator; its hit placement is hand-edited cells and a uniform random fill
[V, 04 §4.4]. Euclidean placement (Björklund) exists only in the T-1 code that is already in the firmware
(`torso_t1_algo.c`) and belongs to the T-1 starting set, not to this family. The nearest RHYTHM equivalent is the
groove-template order in HITS, which places hits by metric weight rather than by even spacing. If P1-010 wants a
dedicated rhythm-generating algorithm for drum voices, the better candidate is the T-1's Euclidean one, reworked
through the same layer/morph/dice interface; RHYTHM then adds only polymeter and ghost-note chances on top.

## 7. Risks and open questions

Design points that rely on ⚠ or [I] claims, and how each is neutralised:

| # | Claim in source | Where our design touches it | Treatment |
| --- | --- | --- | --- |
| 1 | Lock Length / Global Length semantics ⚠ (04 §4.2, 06 §6.4 #1) | LEN, DRIFT | Not relied on: no Lock or Global Length exists; `len` and `vln` are the only lengths **[D]**. |
| 2 | Roll count `n` vs `n+1` total hits ⚠ (05 §5.5, 06 §6.4 #2) | ROLL | Not relied on: `hits = 1 + ex`, `ex` in 1..3 **[D]**. The source's cap of 4 repeats and the ×0.7 decay are [V structure] and only loosely followed. |
| 3 | Rolls interrupted by a new hit on the same lane ⚠ (05 §5.5, 06 §6.4 #7) | ROLL | Not relied on: all hits stay inside their own step, so no cut-off case can occur. |
| 4 | Alt note drawn from the same random as Chance ⚠ (06 §6.4 #8) | ALT | Not relied on: independent hashes **[D]**. |
| 5 | Clock Speed label-to-factor mapping ⚠ (03 §3.3) | none | Clock Speed dropped; global DIV. |
| 6 | Shuffle Length rule and behaviour when thresholds cross ⚠ (03 §3.2, 06 §6.4 #4-5) | none | Per-step shuffle dropped; global SWING. |
| 7 | Mute stops the layer counters ⚠ (02, 06 §6.4 #6) | none | Mute dropped from the algorithm; channel mute is a control-map function and `t` is a pure counter anyway. |
| 8 | Auto Retrigger also resets shuffle position ⚠ (06 §6.4 #9) | none | Retrigger and shuffle dropped. |
| 9 | Drum note names ("39 = Clap" etc.) ⚠ labels only (02, 07) | base note | Not relied on: base note 36 is a design constant **[D]**; no instrument naming. |
| 10 | "The presets are hand-made, so a random fill gives bad grooves" **[I]** from preset statistics (04 §4.6) | HITS templates | The groove-template design is **[D]**. If hardware shows uniform shuffle is fine, FREE becomes the only template (a table edit). |

Relied on and **[V]**: the layer sequencer with independent length and the LCM period (04 §4.1-4.3); chance
`level > random(6)` with levels 0..6 giving 0..100 % (05 §5.1); the velocity table 10..127 (05 §5.3); the ×0.7 roll
decay (05 §5.5); Not-so-random as a ±1 clipped walk (04 §4.4); one note per hit and a fixed 16th note length with no
gate control (05 §5.7). **No design point depends on an unresolved ⚠ claim.**

Other risks and questions for the owner (not source ⚠): see "Owner questions" below and the task log.
- **Target mismatch:** the reference workflow is one TB-3 bassline. RHYTHM is only musical there as a one-note
  pulse. It earns its place only if a drum voice or a rhythmic (non-melodic) channel is a v1 goal.
- **Fixed pitch on a drum machine:** the channel has no note setting, so a drum machine must map its voice to note
  36 (and 48 for ALT) on that MIDI channel, or the user types a pin with the right pad note.
- **Pitchless ignores the global scale:** acceptable, because the scale only concerns pitched algorithms.
- **Owner re-lock needed** (ADR-0004) after a reference implementation and golden files exist; none exist yet.

### Owner questions raised by this task
17. Is a fixed base note 36 with the alt at +12 acceptable for the pitchless algorithm, or should a drum channel carry
    a note setting (which breaks rule 3 "a channel's only setting is its MIDI channel")? Pins already let the user
    type any note per step.
18. A pin on a step at or past LEN is silent until LEN is raised. Acceptable (same as STAGE long loops, #10)?
19. Is NUDGE (reversible mutate) worth adding to the global dice vocabulary for all algorithms, independent of
    whether RHYTHM ships?
20. Is a drum-voice channel a v1 goal at all? If not, RHYTHM stays LATER and only the two lifted ideas matter.

## 8. Recommendation
**LATER.** Rhythmic Steps is the only family aimed at a drum voice and its polymeter idea (independent layer
lengths) is real, but on this device it is mostly a re-skin of mechanisms the other four already specify: seeded
rank-order density, ratchet on firing steps, a seeded velocity lane, a velocity-cycle length, an octave-jump
probability and a chance thinning (6.7). It has no pitch material, which scores 1 on the one workflow the product is
judged by (a TB-3 bassline), and it brings no Euclidean-style generator. Promote it only if P1-010 decides a drum
voice is a v1 goal; then it costs about 160 B per channel and, because its primitives are shared, only the template
order, the per-step chance level and the pitchless output are new code. **Regardless of that verdict, two ideas are
worth lifting into the shared set now:** (1) the **groove-template HITS order** as an optional source for the shared
rank-order density (it fixes a weakness of a uniform shuffle, which can drop the downbeat, and would improve ACID and
STAGE rests too); (2) **NUDGE** as a reversible, small-change companion to dice (owner question 19). The dedicated
rhythm-generating answer that this family lacks should come from the T-1's Euclidean algorithm, not from here.
