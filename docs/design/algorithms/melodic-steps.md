# Assessment: Melodic Steps (five-lane polymetric melody)

Source spec: `docs/melodic-steps-algorithms/` (read all of it, including the 7 decoded presets in 07). Vision:
docs/project/VISION.md. Confidence tags ([V]/[I]/⚠) are the source's; **[D]** marks a decision made in this document
(no fidelity claim). ⚠ is never promoted to fact; every design point that touches one is listed in section 7.

Design stance: a **redesign, not a port**, in the style of `sting.md`, `snake.md`, `ml185.md`, `steparp.md` and
`rhythmic-steps.md`. Three facts about the source shape the redesign:

1. It is **not a generator**. Its only generators are a uniform per-lane "Random" (levels 0..6) and a "Not-so-random"
   mutate of -1/0/+1 per fader [V, 03 §3.3]. There is no motif, contour or interval logic. The melodies in its presets
   are hand-made: the pitch lanes are stepwise ramps and plateaus, for example Dorian `001122334`, Locrian `3210432`,
   Aeolian `0033311` [V, 07; reading them as "stepwise on purpose" is [I]]. A uniform random fill does not make that
   shape, so the redesign has to **invent a contour generator [D]**. That is the one genuinely new thing here.
2. Its structure is **five lanes (octave, scale degree, note length, velocity, chance), each with its own loop length
   2..16**, combining into a period of `lcm` of the lengths [V, 03 §3.2]. Pitch is a **scale degree plus an octave
   lane**, so every value is in key by construction [V, 05 §5.1].
3. Every lane advances on every clock tick, rests included; a failed chance roll silences the note but the pitch lane
   still moves on [V structure, 01 flow and 03 §3.2: all five lanes advance on every tick]. That is the opposite of what a bass line wants when you thin it out. The redesign
   advances the note lanes **per sounding note [D]** so thinning keeps the melody's shape.

Layers, morphs and dice are declared as data tables (ADR-0004 addendum); the tables are in 6.1. The random primitives
are those of `sting.md` 6.2 (xorshift32, `init(seed16, salt)`, `rnd(n)`, `shuffle`), with new salts.

Verdict preview (detail in section 8): **LATER**, with three ideas lifted into the shared set now. Most of the
mechanism overlaps SNAKE, STAGE and ACID (section 6.7); what is new is a contour pitch generator and a
count-driven (per-note) lane clock.

## 1. What it does (5 lines max)

Melodic Steps builds a melody from five parallel lanes of 16 faders (octave, scale degree, note length, velocity,
chance), each with its own length 2..16, so the lanes drift against each other (default lengths 2/3/8/15/4; the six
melodic presets repeat only after 420..2,520 steps) [V]. A note is `root + scale[degree] + 12 x octave`, with
velocity and length looked up from the lanes and a chance roll deciding whether it sounds [V]. In this sequencer it
becomes **MELODY**: a stepwise contour walk through the global scale as the pitch lane (new), polymetric octave,
gate-length and accent lanes that advance per note, a rest order with a density morph, 5 dice layers and 4 knob
morphs, all deterministic from five seeds.

## 2. Generators inside it
| Generator | Inputs | Output | Deterministic? (seeded / free-running) |
| --- | --- | --- | --- |
| Five lane counters with independent lengths [V] | tick, length 2..16 per lane | one level 0..6 per lane per tick, period = lcm of lengths | Pure function of the tick count. Kept in a reduced form **[D]**: lane lengths for OCT, GATE and ACCNT are picked by their own dice seeds, the pitch lane length is the LEN morph, the rhythm is a fixed 16 |
| Random per lane [V] | none | 16 uniform levels 0..6 | Free-running in source; **replaced [D]** by five seeded layers (6.3) |
| Not-so-random per lane [V] | none | each level `+= random(3) - 1`, clipped | Free-running in source; **dropped** as a button, its idea lifted as NUDGE (section 6.7; owner question 19 already covers it) |
| Reset per lane [V] | none | neutral levels (3/0/2/5/6) | **Dropped** (undo-dice replaces it) |
| Contour walk **[D]** (no source equivalent) | NOTES seed, LEAP, SPAN | 16 scale-degree indices, `deg[0] = 0` (the root), neighbours differ by at most LEAP degrees | Seeded; new |
| Degree to semitone lookup, 8 scales [V] (⚠ key base) | level, scale table | semitones above root | **Dropped**; the global scale control supplies the scale and root. The degree-index idea is kept: the walk works in scale-degree units and maps through the global table (6.3) |
| Octave lane: level 0..6 -> -3..+3 octaves [V] | level | octave shift | Redefined **[D]**: 3 values (-12, 0, +12), seeded, own length |
| Length lane: level -> 1/64 .. 1/2 note [V] | level | note length | Redefined **[D]**: 4 gate fractions of one step, seeded, own length |
| Velocity lane: 15..120 in 7 levels [V] | level | MIDI velocity | Redefined **[D]**: 3 levels (ghost 70, normal 95, accent 127), seeded, own length |
| Chance lane: level/6 probability, `random(100)+1 <= pct` [V] | level, free-running RNG | note plays or not | Free-running in source; **replaced [D]** by the RESTS layer + DENS morph (rank order, the primitive of every other family) |
| Clock: 12 speeds, triplets, double/half, resync [V structure, ⚠ arithmetic] | transport | tick | **Dropped**; the global DIV |
| Swing [V structure, ⚠ range] | 0..127 | delay of off-beat step | **Dropped**; the global SWING |
| MIDI-in root and clock gating [V structure, ⚠ note-off] | notes | root, start/stop | **Dropped**; the global scale control and transport |
| LenRndEven button [⚠ rule not traced] | button | unknown | **Dropped** |

## 3. Layers (what can be diced independently)

Five layers, each with its own 16-bit seed. In the source the five lanes are already independent stores with their
own Random/Reset buttons [V, 03]; this design gives each its own seed so one can be re-rolled alone. The on-screen name
is the dice label (max 6 chars). OCT, GATE and ACCNT also pick their own lane length (2..7, 2..7, 2..9), so dicing
one of them changes how that lane drifts against the pitch loop, without a knob.

| Layer (screen label) | What dicing it changes | What stays fixed |
| --- | --- | --- |
| NOTES (`NOTES`) | The 16 random magnitudes and signs of the contour walk: a new melodic shape (new tune, same size and density). | Which steps sound (RESTS), octave, gate and accent lanes, all four knob values, scale, root |
| RESTS (`RESTS`) | The priority order in which the 16 steps switch on as DENS rises: where the rests fall. Step 0 always ranks first. | Contour, octave, gate, accent, knob values |
| OCT (`OCT`) | The octave lane: its length (2..7) and which notes jump down or up an octave. | Contour, rests, gate, accent, knob values |
| GATE (`GATE`) | The note-length lane: its length (2..7) and the four gate fractions per note. | Contour, rests, octave, accent, knob values |
| ACCNT (`ACCNT`) | The velocity lane: its length (2..9) and which notes are ghost, normal or accented. | Contour, rests, octave, gate, knob values |

A sixth action, **dice all**, re-rolls all five seeds in one gesture (which physical button does it is the control
map's job). **Undo dice**: each layer keeps its previous seed (2 bytes), so the last dice of each layer can be recalled
once (VISION glossary: "the previous seed can be recalled").

## 4. Morph candidates (continuous knob transforms)

Every morph is a **pure function of (seeds, knob values)**: the random content is fixed by the seeds and the knob only
chooses how much of it to apply or how to read it. Turning a knob back restores the previous pattern exactly (VISION
glossary: "undoable by turning back"). Cost figures are for recomputing the caches after a knob detent (main loop, not
the ISR).

| Morph | Range | Musical effect | Cost per step (rough) |
| --- | --- | --- | --- |
| LEN (pitch-loop length `len`) | 2..16, default 7 | Length of the pitch lane in notes. Reads the first `len` of the 16 walk values, so a longer loop only appends notes and a shorter one truncates; it never reshuffles the notes already there. 2 = a two-note pedal; 16 = a long phrase. It drifts against the 16-step rhythm and the octave/gate/accent lane lengths. Same idea as source Transpose length [V, 03 §3.2]. | 1 modulo per note in the ISR |
| LEAP (largest interval `leap`) | 0..5 degrees, default 2 | Maximum scale-degree move between neighbouring notes: note `k` moves by `(mag[k] x (leap+1)) >> 8` degrees (0..leap) up or down. 0 = one repeated note; 1 = crawling, stepwise lines; 2 = the default walking bass; 5 = angular, leaping. | 15 integer multiplies to rebuild `deg` |
| SPAN (pitch span `span`) | 0..14 scale degrees, default 7 | The walk is reflected inside `0..span` degrees above the root. 0 = the root only; 7 = about an octave in a 7-note scale; 14 = about two octaves (a bit less than three in a 5-note scale). Scale size does not change the number of degrees, so the pitch width grows with the scale. | 15 reflects to rebuild `deg` |
| DENS (hit count `den`) | 1..16, default 12 | Step `s` sounds iff its rank in the RESTS-seed order is `< den`. Fewer = sparser; 16 = every step. Step 0 always sounds. | 1 table read + compare |

Rejected as morphs (kept out of the 4 knobs): lane lengths for OCT, GATE and ACCNT (dice-picked, a fifth/sixth knob
would be a menu: rule 1), per-lane Speed and the Triplets/Double/Half switches (global DIV), swing and note length
(global SWING and GATE), the per-step chance lane (replaced by DENS, a per-step shimmer is ARP CHANCE), octave
probability (OCT is a dice layer; ACID has a JUMP knob for it), stepwise-vs-leaping *per note* (LEAP already does it
globally).

## 5. Fit against the vision (score 0–3 each, with one line of why)
| Criterion | Score | Why |
| --- | --- | --- |
| Quick to a usable sequence (rule 5) | 3 | Selecting MELODY plays a full line at once from defaults (LEN 7, LEAP 2, SPAN 7, DENS 12) and fixed per-channel seeds; the first 16 steps of channel 0 are in the informative check (6.3), and the contour starts on the root so it never opens on a wrong-sounding note. No setting beyond the channel's MIDI channel. |
| Performable with 4 knobs + dice buttons, no menus (rules 1, 4) | 3 | Four knobs, one musical idea each (loop length, interval size, range, density), all audible per detent and reversible; five dice layers plus dice-all map to buttons; a contour plot with the playhead is a ready-made screen. |
| Monophonic bassline/lead use (the TB-3 workflow) | 2 | One note per hit and gates of at most 100 % of a step, so it is monophonic-safe. Stepwise walks from the root with an octave lane are bass-like, and `deg[0] = 0` anchors the loop. But it has no slide and no pitch-class motif (ACID's root-hammering TYPE is a better bass), and the pitch walk drifts without a harmonic anchor beyond the root. |
| Distinct from the other families (adds something new) | 1 | Four of its five lanes and both its loop-length ideas are already in SNAKE, STAGE, ACID, ARP and RHYTHM (6.7). New: a contour (interval-limited) pitch generator, a per-note (not per-tick) lane clock so thinning keeps the melody, and independent octave/gate/accent lane lengths as dice. The new part is small enough to lift into other algorithms. |
| Fits the FM-1 budget (RAM < 2 KB per channel, ISR cost small) | 3 | About 130 B per channel (6.4). Per sounding note the ISR does four modulos and four table reads; generation is under 250 draws and ops on the main loop. |

## 6. Redesign for this sequencer

- **Proposed name on screen (max 6 characters):** `MELODY`
- **K1–K4 morphs (with on-screen labels, max 6 characters each):**
  K1 `LEN`, K2 `LEAP`, K3 `SPAN`, K4 `DENS`. (The control map owns which context these knob roles apply in.)
- **Dice buttons (which layers):** five layer targets `NOTES`, `RESTS`, `OCT`, `GATE`, `ACCNT`, plus dice-all and
  undo-dice. Physical button binding is a control-map table row; this spec only declares the targets.
- **What we drop from the original, and why:**
  - The Chance lane (per-step probability, free-running RNG): not deterministic (rule 4); RESTS + DENS gives
    performable, repeatable rests, and a per-step shimmer already exists as ARP CHANCE.
  - Per-lane Random/Not-so-random/Reset buttons and the 80 hand-edited faders: a lane editor is a menu (rule 1).
    Steps are entered on the white keys (6.5), the rest is dice and morph. The Not-so-random idea survives as the
    shared NUDGE proposal (owner question 19), not as five buttons.
  - The 8 scales, Root Note and the MIDI-in root: the global scale control supplies them (owner decisions 3 and 4).
  - Speed (12 note values), Triplets, Double/Half Time, Swing, Re-Sync, MIDI-clocked start/stop: global DIV / SWING and
    transport (owner decision 4); the resync is not needed because every lane index is a function of the sounding-note
    counter, which restarts with the transport.
  - Seven levels per lane: velocity, length and octave collapse to 3..4 musically useful values each; seven levels of
    octave (+-3 octaves) would leave the TB-3's range.
  - The Length lane's multipliers up to 1/2 note (32 sixty-fourths): a note longer than a step overlaps the next one.
    On a monophonic synth that is a tie or glide, owned by ACID's SLIDE layer, not a gate length.
  - LenRndEven (⚠ never traced).
- **What we improve over the original:**
  - **A contour generator.** The source can only fill a lane uniformly at random; its own presets are hand-made
    stepwise lines. LEAP and SPAN give a performable "how melodic / how wide" pair, reversible by turning back.
  - **Thinning keeps the tune.** In the source a rest still advances every lane. Here the note lanes advance only on
    sounding notes, so DENS or a pin changes the rhythm without scrambling which note comes next.
  - **The loop starts on the root** (`deg[0] = 0`), which the source's random fill does not guarantee.
  - Every random decision is seeded per layer, so a layer can be re-rolled alone, undone and saved (rule 4); the
    source's chance roll and Random buttons are free-running.
  - Rests exist as a morph (DENS) and a layer (RESTS), where the source relies on a random chance lane.

### 6.1 Parameters (data table; ranges and defaults)

This table is the single source for the on-screen labels, ranges and defaults. A remap is an edit to this table.

| Id | Kind | Label | Range | Default | Stored as |
| --- | --- | --- | --- | --- | --- |
| `len` | morph, knob 1 | `LEN` | 2..16 | 7 | u8 |
| `lep` | morph, knob 2 | `LEAP` | 0..5 | 2 | u8 |
| `spn` | morph, knob 3 | `SPAN` | 0..14 | 7 | u8 |
| `den` | morph, knob 4 | `DENS` | 1..16 | 12 | u8 |
| `s_notes` | dice seed | `NOTES` | 0..65535 | `(0x3A5D * (ch*5 + 1)) & 0xFFFF` | u16 |
| `s_rests` | dice seed | `RESTS` | 0..65535 | `(0x3A5D * (ch*5 + 2)) & 0xFFFF` | u16 |
| `s_oct` | dice seed | `OCT` | 0..65535 | `(0x3A5D * (ch*5 + 3)) & 0xFFFF` | u16 |
| `s_gate` | dice seed | `GATE` | 0..65535 | `(0x3A5D * (ch*5 + 4)) & 0xFFFF` | u16 |
| `s_acc` | dice seed | `ACCNT` | 0..65535 | `(0x3A5D * (ch*5 + 5)) & 0xFFFF` | u16 |
| `p_*` (x5) | previous seed for undo-dice | n/a | 0..65535 | equal to the seed (nothing to undo) | u16 each |
| `pin_mask`, `pin_note[16]` | step-entry overrides (6.5) | n/a | mask 0..65535; offset 0..96 | 0, 0 | u16 + 16 x u8 |

`ch` is the 0-based channel index. A seed of 0 is legal; the mixed state `x` is forced non-zero (see 6.2). When the
user dices a layer the new seed is the low 16 bits of the audio sample counter at the press (the only non-deterministic
moment; it is stored, so playback afterwards is fully deterministic).

### 6.2 Random primitives (normative, needed for golden files)

Identical to `sting.md` 6.2, repeated so this file stands alone:

- State: unsigned 32-bit `x` (all shifts logical, arithmetic wraps mod 2^32; all `/`, `%` and `>>` below are unsigned
  integer, truncating). `init(seed16, salt)`: `x = ((seed16 << 16) | seed16) ^ salt; if x == 0: x = 1; draw 4 values and discard.`
- Salts: NOTES `0x6D3A81F5`, RESTS `0x1B72C4E9`, OCT `0x57A0D63B`, GATE `0x3E91B7C2`, ACCNT `0x0C5F28A4` (different from
  every other algorithm's, so no stream is shared).
- `next()`: xorshift32: `x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x`.
- `rnd(n)` (n >= 1) = `(next() >> 8) % n`.
- `shuffle(a, len)` (Fisher-Yates): for `i = len-1 down to 1`: `j = rnd(i+1); swap(a[i], a[j])`.
- Each layer builds its own state from its own seed, draws in the order given below, and uses no other layer's stream.

### 6.3 Procedure (step by step; every layer is computed from seed only, then morphs are applied)

**NOTES layer** (stream: `init(s_notes, SALT_NOTES)`): for `k = 0..15`, in this order: `mag[k] = rnd(256)`, then
`sgn[k] = rnd(2)` (32 draws, always, whatever the morphs).

**Contour (LEAP, SPAN morphs; pure):**
1. `deg[0] = 0` (the loop opens on the root).
2. For `k = 1..15`: `m = (mag[k] * (lep + 1)) >> 8` (0..lep). `x = deg[k-1] + (sgn[k] ? m : -m)`.
   If `x < 0`: `x = -x`. If `x > spn`: `x = 2*spn - x`. Then clamp `x` to `0..spn`. `deg[k] = x`.
   (Reflection keeps the walk inside the span; the final clamp covers the case where `lep > spn`.)
3. The pitch lane reads `deg[0 .. len-1]` only; `len` does not change any `deg` value.

**RESTS layer** (stream: `init(s_rests, SALT_RESTS)`)
1. `ord = [1..15]; shuffle(ord, 15)`.
2. `rank[0] = 0` and `rank[ord[r]] = r + 1` for `r = 0..14`.
**DENS morph:** step `s` is a hit iff `rank[s] < den`. Because `den >= 1`, step 0 always sounds.

**OCT layer** (stream: `init(s_oct, SALT_OCT)`): `olen = 2 + rnd(6)` (2..7); then for `j = 0..8` (always 9 draws):
`v = rnd(8)`; `oct[j] = -12` if `v == 0`, `+12` if `v == 7`, else `0`.

**GATE layer** (stream: `init(s_gate, SALT_GATE)`): `glen = 2 + rnd(6)` (2..7); then for `j = 0..8`:
`gate[j] = GT[rnd(4)]` with `GT = [25, 50, 75, 100]` (percent of one step; the global GATE multiplies it).

**ACCNT layer** (stream: `init(s_acc, SALT_ACCNT)`): `alen = 2 + rnd(8)` (2..9); then for `j = 0..8`: `v = rnd(8)`;
`vel[j] = 70` if `v < 2`, `127` if `v == 2`, else `95`.

**Scale table `tbl`/`N`** (the single global scale control, owner decision 3): `tbl` = the global scale's semitone
degrees ascending, `N` their count (chromatic = `[0..11]`, `N = 12`). MELODY has no scale of its own. Golden files
pass `tbl` explicitly; the reference fixture is minor pentatonic `[0,3,5,7,10]`, `N = 5` (as `snake.md`).

**Degree to note** (pure; cached as `dn[0..14]`): `dn[d] = 12 * (d / N) + tbl[d % N]` (so degrees past the top of the
scale continue into the next octave). The note is
`note = clamp(36 + root + transpose + dn[deg[hc % len]] + oct[hc % olen], 0, 127)` where `hc` is the hit counter
below. `36` (C2) is the base note **[D]**, as in SNAKE. The highest possible note is below
`36 + 11 + dn[14] + 12 + transpose`, under 100 for all realistic scales.

**Playback** (the channel engine calls this once per step; `t` = the channel's step counter, a u32 that starts at 0 on
transport start or restart and increments once per step at the global DIV; nothing here is swung):
1. `r = t % 16`. If `pin_mask` bit `r` is set: pinned hit with `note = clamp(36 + root + transpose + pin_note[r], 0, 127)`,
   gate 75 %, velocity 95 (constants **[D]**), and **`hc` is not advanced**. Otherwise `hit = rank[r] < den`; if
   `hit == 0`: no note this step.
2. For a generated hit: `note` as above with `hc`; `gate_pct = gate[hc % glen]`; `velocity = vel[hc % alen]`; then
   `hc = hc + 1`. The note-off is scheduled at the note-on, `gate_pct` of one step times the global GATE, and is
   never longer than one step (no overlap, so no hung or tied note).
3. `hc` is a u32 held per channel, reset to 0 with `t` on transport start or restart. It is the **only** cross-step
   state.
Because the lanes are indexed by the sounding-note count and not by `t`, a DENS change or a pin changes the rhythm
and leaves the tune in order: the next generated note is always the next note of the contour. A knob or dice change
swaps the caches only; `hc` and `t` keep running, so no morph restarts the loop.

**Caches** per channel, swapped in one go (`fm1_irq_off/on` around a copy of about 80 bytes, or double-buffered):
`dn[15]`, `deg[16]`, `rank[16]`, `oct[9]`, `gate[9]`, `vel[9]`, `olen`, `glen`, `alen`, `len`, `den`.
The cache is recomputed on any seed, knob, scale, root, transpose or pin change.

**Informative check** (not normative; the golden file will supersede it). Channel 0, all defaults, no pins, scale
`[0,3,5,7,10]` (fixture), root 0, transpose 0, seeds 14941 / 29882 / 44823 / 59764 / 9169, from a throwaway Python
transcription of this procedure:
- `deg` = `0 1 2 0 1 0 1 1 1 0 2 4 5 7 5 3`; `rank` = `0 14 4 10 6 9 2 13 11 12 3 1 7 15 8 5`.
- `olen` 5, `oct` = `0 -12 0 0 0 0 0 12 0`; `glen` 5, `gate` = `100 50 75 100 100 50 50 50 50`;
  `alen` 2, `vel` = `95 95 127 127 95 95 95 70 95`.
- First 16 steps (note,gate%,velocity or `-` for a rest):
  `36,100,95 ; - ; 27,50,95 ; 41,75,95 ; 36,100,95 ; 39,100,95 ; 36,100,95 ; - ; 27,50,95 ; - ; 36,75,95 ; 39,100,95 ; 41,100,95 ; - ; 36,100,95 ; 27,50,95`.
- The lane lengths 7 / 5 / 5 / 2 give a pattern period of lcm = 70 sounding notes.
(Only `vel[0..1]` are read at the default `alen` 2; the other entries are kept so a later dice with a longer `alen`
is a pure function of its seed.)

### 6.3a Dice and knob latency (VISION rule 1: changes feel immediate)

MELODY has one cross-step value, `hc`, and no note pairing (it has no slide; the ACID SLIDE layer is not part of it).

- The new cache is built by the main loop in the frame of the press or detent (about 90 xorshift draws, a 15-entry
  walk, a 16-entry rank pass and a 15-entry `dn` pass: far below one 16 ms frame), then swapped under
  `fm1_irq_off/on`.
- The ISR reads the cache at the **next step**: worst-case latency from press or turn to sound is one frame plus one
  step (about 16 ms + 125 ms at 120 BPM and 1/16, typically half that). No "commit one step ahead" is needed because
  no note spans two steps.
- A note already sounding keeps the note-off scheduled at its note-on. A stop, panic, channel, algorithm or
  MIDI-channel change sends note-offs and CC123 at once, so a dice never leaves a hung note.
- LEN, LEAP, SPAN and DENS act on the next generated note without restarting `hc`, so a knob turn never jumps the
  pattern back to its start. Dicing NOTES changes which note each `hc` position means, so the next sounding note is
  the new tune's note at the current position (the tune changes at once but the rhythm and phase do not).

### 6.4 Channel RAM and CPU

| Item | Bytes |
| --- | --- |
| 4 knob values | 4 |
| 5 seeds + 5 previous seeds | 20 |
| Pin mask + 16 pin notes | 18 |
| Cache: `dn` 15, `deg` 16, `rank` 16, `oct` 9, `gate` 9, `vel` 9, 3 lane lengths | 77 |
| Hit counter `hc` (u32) | 4 |
| **Total** | **123 B per channel** (well under 2 KB) |

No constant tables beyond `GT` (4 B). Scratch (`mag`, `sgn`, 32 B) can be regenerated from the seed on demand (about 32
draws) and not stored, so 123 B includes no scratch. CPU: a full recompute is under 250 integer ops, run from the main
loop; the ISR does four modulos, four table reads and one add per sounding note.

### 6.5 Step entry interplay (rule 3: steps on the 16 white keys OR the algorithm)

- A step typed on the white keys while MELODY is selected writes a **pin** on the rhythm step `i = 0..15` (the position
  the player counts): `pin_mask |= 1<<i`, `pin_note[i] = clamp(key note - 36 - root, 0, 96)` (an absolute note offset,
  as in SNAKE). The step is forced to sound, ignoring DENS and RESTS, with constant gate 75 % and velocity 95.
- A pin does **not** consume a contour note (`hc` is not advanced), so pinning a step never shifts the tune that the
  neighbouring generated steps play. That is the property to verify with the owner on the first hardware test.
- Pins are keyed by step position, so they survive every dice and morph. Clearing the pin (a control-map action)
  returns the step to the generated line.
- The key-entry gesture itself belongs to the channel/step-entry spec and must cite this section.

### 6.6 Flex points (ADR-0004 addendum): what is a table edit, what is not

Remappable by editing a table row, no algorithm code change:
- Which knob drives which morph (K1..K4 <-> LEN/LEAP/SPAN/DENS), each morph's label, range, default.
- Which physical button dices which layer, and the gesture for dice-all and undo-dice.
- Fixed constants: base note 36, the octave values (-12, 0, +12) and their 1/8 odds, `GT` = 25/50/75/100, the three
  velocities 70/95/127 and their odds, the lane-length ranges 2..7 / 2..7 / 2..9, the pin gate and velocity, the
  default seed multiplier 0x3A5D.
- Adding a dice layer that reuses an existing stream, for example splitting NOTES into a magnitude and a sign layer,
  or giving K2 to a different morph (for example OCT density).
- Swapping the pitch generator at the NOTES layer for a different one (random uniform, motif tile) is a table row if the
  generator list is data.

Not changeable without algorithm code: the xorshift generator and salts (they define the golden files), the contour
walk itself (reflected walk with a per-step magnitude cap), the hit-counter lane clock (`hc` instead of `t`), the
fixed 16-step rhythm, the order-based DENS principle (monotonic/reversible), adding a genuinely new lane (for example
a CC lane or a slide lane).

- **Parameters that must not exist (settings creep that breaks rule 1):** per-lane Length, Speed, Random, Reset or
  Not-so-random controls; per-step fader editors for any lane; a level-count or note-value menu; Speed, Triplets,
  Double/Half, Swing or Re-Sync as algorithm settings; Scale or Root as MELODY settings (global scale control); MIDI-in
  root or clock gating; a seed entry field; a chance-percentage dial.

### 6.7 Overlap with the five existing assessments (for P1-010 to dedupe)

| MELODY part | Already in | Verdict |
| --- | --- | --- |
| RESTS rank order + DENS count | ACID HITS/DENS, SNAKE GATES/DENS, STAGE RESTS/DENS, ARP HITS/DENS, RHYTHM HITS/DENS | **Duplicate**: same seeded rank-order primitive (step 0 first, as ACID) |
| ACCNT lane (seeded velocity levels with own length) | SNAKE VEL + POLY (length 3..12), STAGE VEL, ARP VEL, RHYTHM VEL + DRIFT; ACID ACCNT | **Near-duplicate**: SNAKE POLY already makes the velocity-lane length a dice |
| GATE lane (4 gate fractions, own length) | ACID GATE (4 gate codes 30/70/100/127 %, per-step, no own length) | **Near-duplicate** of ACID's gate lane; the only addition is the independent lane length |
| OCT lane (octave jumps) | ACID OCT + JUMP knob (per-step probability), RHYTHM ALT | **Near-duplicate**: ACID's version has a knob and direction; MELODY's is 3 values with its own length |
| LEN (pitch-loop length) | SNAKE LEN (pitch lane, 1..16), STAGE TOTAL, RHYTHM LEN | **Duplicate** of SNAKE LEN (MELODY 2..16 instead of 1..16) |
| Pitch lane in scale degrees through the global scale | SNAKE (raw 0..255 scaled by RANGE then quantised), STAGE (same) | **Partial**: both already scale-aware and late-quantised; MELODY works in degrees (a 14-degree span instead of 24 or 36 semitone units) |
| Polymeter of independent lane lengths | SNAKE (pitch vs velocity vs fixed 16), RHYTHM (hit vs velocity) | **Partial**: MELODY adds octave and gate lane lengths on top, nothing structurally new |
| **Contour pitch walk with interval cap (LEAP) and reflected span (SPAN)** | none (ACID motif tile + root hammering is a motif, not a walk; SNAKE/STAGE are uniform random) | **New** |
| **Per-note (hit-count) lane clock `hc`** | none (all five index lanes by `t`) | **New** |
| Pin does not consume a note (step entry keeps the tune) | none | **New** (follows from `hc`) |
| NUDGE (reversible small mutate) | RHYTHM already proposes it (owner question 19) | **Same idea** as the source's Not-so-random; do not duplicate |
| Dropped chance lane | ARP CHANCE (global thinning) | Covered elsewhere |

**What melodic capability is genuinely new for a TB-3 bass line?** Three things, none of them present in the five
assessed families:
1. **Contour.** A pitch lane whose neighbouring notes differ by at most LEAP scale degrees inside a SPAN, starting on the
   root. It answers owner question 5 (is a uniformly random pitch lane acceptable as a bass?): this is the
   scale-weighted fallback, and it is a morph (LEAP 0..5) rather than a fixed bias. ACID's root-hammering TYPE gives
   motif-based variation; neither SNAKE nor STAGE offers stepwise movement. A "motif" in the usual sense (a short cell
   repeated and transformed) is **not** in Melodic Steps at all [V, no motif logic in 03/05]; ACID's tile-and-shuffle is
   the only motif mechanism in the catalogue.
2. **A note-count lane clock.** The octave, gate and accent lanes advance on sounding notes, so thinning the rhythm
   keeps the tune in order (rule 4: perform it live without wrecking it).
3. **Pins that do not consume the contour.** A typed step plays its key without moving the generated notes.
Scale-aware pitch walking in degrees is the mechanism; the source itself offers only the degree *index* idea (pitch from
`scale[level]`, always in key [V]).

Recommended lifts (section 8): the contour generator and `hc` clock as an optional pitch source and lane clock for SNAKE
and STAGE's NOTES layers (a table row, 6.6), and the independent lane lengths as dice on the shared layers.

## 7. Risks and open questions

Design points that rely on ⚠ or [I] claims, and how each is neutralised:

| # | Claim in source | Where our design touches it | Treatment |
| --- | --- | --- | --- |
| 1 | Transpose level 0 returns the root; `coll` key base 0 or 1 ⚠ (05 §5.1, 06 §6.4 #1) | Degree-to-note mapping | Not relied on: the global scale table is indexed by `d % N` from 0 and `deg[0] = 0` is the root **[D]**. |
| 2 | Chromatic scale spans only 0..6 semitones ⚠ (05 §5.1, 06 §6.4 #2) | Scale span | Not relied on: SPAN counts the *global* scale's degrees; a chromatic global scale gives a 14-semitone span at SPAN 14. |
| 3 | LenRndEven rule ⚠ (03 §3.3, 06 §6.4 #6) | none | Dropped. |
| 4 | Clock arithmetic: triplet branch `x 1.5`, not-playing behaviour ⚠ (04 §4.1, 06 §6.4 #3) | none | Dropped; the global DIV. |
| 5 | Swing range 0..127 and whether it delays the counters ⚠ (04 §4.3, 06 §6.4 #7-8) | none | Global SWING. |
| 6 | Resync boundary ⚠ (04 §4.4, 06 §6.4 #5) | none | Not needed: lane positions are functions of `hc` and `t`, restarted with the transport. |
| 7 | MIDI-in note-off stops the clock ⚠ (04 §4.5, 05 §5.6, 06 §6.4 #4) | none | Dropped. |
| 8 | MIDI output channel ⚠ (06 §6.4 #9) | none | The channel's MIDI channel is its only setting (rule 3). |
| 9 | "The presets use stepwise lines on purpose" **[I]** from the decoded faders (07 notes 3, 5) | The contour walk | The contour design is **[D]**. If hardware shows uniform random degrees sound as good (LEAP 5 is nearly that), LEAP's default is a table value (6.6). The claim "no motif logic" is read from the absence of any such object in 03 and 05, not from a negative proof of the patch. |
| 10 | Chance is random per tick, not deterministic [V from 05 §5.3; also 07 note 5] | Dropped chance lane | The seeded RESTS layer replaces it; presets that lean on chance (31..58 % mean) get their rests from DENS, so the sound will not match presets byte for byte. |

Relied on and **[V]**: per-lane lengths 2..16 and the lcm period (03 §3.2); pitch as degree plus octave, always in key
(05 §5.1); the octave level 3 = no shift convention (05 §5.1); the Not-so-random mutate of +-1 (03 §3.3) as the
NUDGE idea; the defaults 2/3/8/15/4 and preset lengths (02, 07). **No design point depends on an unresolved ⚠
claim.**

Other risks and questions for the owner (not source ⚠):
- **Overlap versus value.** Most of MELODY is already specified elsewhere (6.7); a full algorithm for the contour walk
  plus a count-driven clock is a small increment over SNAKE. Hence LATER (section 8).
- **The loop opens on the root.** `deg[0] = 0` anchors the line harmonically but also means each loop restart is a
  root. Whether bass lines want that every time is a hardware-listening question (a table constant if not).
- **Contour drift.** Reflected random walks spend more time near the edges when SPAN is small. The informative check
  shows the default line staying low (degrees 0..2 for most of the loop); LEAP 3..4 widens it. If the default sounds
  timid on a TB-3, raise the LEAP default (a table value).
- **Octave lane can drop below the base note.** The base note is C2 (36), an octave down gives C1 (24..) which the TB-3
  can play; a synth with a narrower range might not. Table constant (6.6).
- **Owner re-lock needed** (ADR-0004) after a reference implementation and golden files exist; none exist yet.

### Owner questions raised by this task
22. If MELODY is not promoted, should the contour walk (LEAP/SPAN, starting on the root) become a **pitch-source option**
    for SNAKE's and STAGE's NOTES layers, and is "stepwise movement" worth a morph slot on those algorithms? It
    answers question 5 (uniformly random pitch lane as a bass).
23. Should the note lanes (pitch, octave, gate, accent) advance **per sounding note** (`hc`, as here) or per step (`t`, as
    in the source, SNAKE and STAGE)? Per note keeps the tune in order when thinned or pinned; per step is simpler and
    consistent across algorithms. If the answer is "per note" for one algorithm, it should be a shared primitive.
24. Should a pinned (typed) step consume a contour note? Recommended: no (the typed note is an addition, the tune stays
    in order). A table/behaviour choice for the first hardware test.
25. If a dedicated bass-focused melodic algorithm is wanted for v1 and the owner prefers to avoid a 4th/5th algorithm,
    is the cheaper v1 option "SNAKE with a LEAP morph on its pitch lane"? (Recommendation: yes, only if question 22 is
    answered yes.)

## 8. Recommendation

**LATER** (do not ship MELODY as a separate algorithm in v1). Melodic Steps is the one family that thinks of pitch as a
scale-degree melody rather than as random numbers, and its lane-per-parameter polymeter is real, but on this device
that polymeter is already specified in SNAKE (pitch and velocity lane lengths), STAGE and ACID, its velocity, gate,
octave and rest lanes are re-skins of the shared rank-order and per-step primitives (6.7), and the source has no
generator of its own to bring (its Random is uniform; its presets are hand-edited). Promote it only if P1-010 wants a
fourth pitch-focused algorithm and is willing to pay about 123 B per channel, one new generator (the contour walk) and
one new lane clock for a modest gain. **Regardless of that verdict, three ideas are worth lifting into the shared set
now:** (1) the **contour pitch walk** (LEAP, SPAN, starting on the root) as an alternative pitch source for the NOTES
layer of SNAKE and STAGE, which is the only scale-aware, bass-friendly pitch generator in the catalogue (owner
questions 5 and 22); (2) the **per-note lane clock** `hc`, so thinning or pinning keeps the tune in order (owner
question 23); (3) **independent lane lengths as dice** (OCT, GATE, ACCNT pick their own lengths), generalising SNAKE's
POLY. NUDGE (the Not-so-random idea) stays with the existing proposal in `rhythmic-steps.md` (owner question 19).
