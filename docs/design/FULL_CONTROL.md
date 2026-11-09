# Full algorithm control: a second knob row (up to 8 morphs per algorithm)

Task P1-013 (owner decision 2026-11-10, sixth round, "Shift"). Status: **for owner review** (gate `human-review`;
approval also re-approves ADR-0003 / P2-004 and unblocks P2-003, P2-007, P2-008). Design only: no firmware.

> Change note (P1-014 revision, scheme B2; for owner review): the dice and UNDO return to their P1-012 / P1-013 buttons (`FX` ALL, `SCL` NOTES, `SEQ` HITS, `ENV` ACCNT, `ARP` ARTIC, `LFO` SHAPE; `EDIT` = UNDO, hold = RESET), `GLO` = the SET page (not `SEQ`), `REC` = RECORD, MUTATE is removed, a sixth context `REC` exists, and a held step key plus another white key sets that step's pitch. The knob rows (sections 2, 3, 5) are still **unchanged**. **Historical sections:** the context rows of 1.3 and 1.4 (`SCL`, `CHAN`, `GLOB`, hold SCL = keyboard octave), the rejected-options table, the F35 row and the per-section edit lists below describe the P1-013 approval and are kept for the record only; the current role of every control in every context is [control_map.tsv](control_map.tsv) / [CONTROL_MAP.md](CONTROL_MAP.md) Appendix A, and the reasoning is [PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md). Keyboard octave: SET row 2 K2 (global) and the REC K4 `OCT`.

> Change note (P1-014, performance-first; for owner review): the knob rows themselves (sections 2, 3, 5.1 step rule, 5.2) are **unchanged**. Superseded by [PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md): (1) the contexts `STEP`, `SCL`, `CHAN`, `GLOB` of section 1.3 are replaced by `SET` (tap or hold SEQ: row 1 `GATE DIV SWING BPM`, row 2 `SCALE OCTAVE MIDI LENGTH`, with OCT- / OCT+ picking the SET row) and `RESET` (hold UNDO); there is no STEP mode. (2) In a held step (`STEP_HELD`) OCT- / OCT+ now pick one of two step-band rows instead of pinning +-12 semitones (the pin octave is the `OCT` cell of row 2). (3) The keyboard octave (F35, hold SCL + OCT) is **retired** because the white keys are always steps and there are no live notes. (4) Section 5.1's last bullet ("no per-morph reset in v1", owner question 49) is replaced: hold UNDO and turn the knob resets that one morph; hold UNDO and tap OCT- / OCT+ resets a row; hold UNDO and tap FX resets the whole algorithm. Every reset is undoable. (5) The ease scores of section 4 gain the L sub-score in UX_REVIEW 3.3 / PERFORMANCE_DESIGN 7.2 (F30-F34 are 12 and 11 of 12 there).

> Change note (P1-013): this file is new. It supersedes the 4-morph cap and the "no shift page" stance of
> UX_REVIEW.md (P1-012). The edits it forces are applied consistently to [control_map.tsv](control_map.tsv),
> [CONTROL_MAP.md](CONTROL_MAP.md), [UX_REVIEW.md](UX_REVIEW.md), [ALGORITHM_SET.md](ALGORITHM_SET.md), section 6 of
> `algorithms/{sting,t1,snake,ml185}.md` and [ADR-0003](../project/decisions/ADR-0003-sequencer-core.md), each with a
> change note at its top.

Inputs: VISION.md, OWNER_QUESTIONS.md (all rounds; the sixth-round "Shift" decision), ADR-0004 and its flexibility
addendum, UX_REVIEW.md (rubric 3.1), CONTROL_MAP.md and control_map.tsv, ALGORITHM_SET.md, the four algorithm docs,
the source specs in `docs/{sting,snake,ml185,t1}-algorithms/`, `firmware/src/panel.c`, `ui_input.c` (`accel`).

## 0. The scheme in one paragraph

Every algorithm has **two rows of four knob functions**: ROW 1 (K1 DENSITY, K2 FORM, K3 PITCH, K4 SPICE, unchanged
from UX_REVIEW 5.1) and ROW 2 (S1 START, S2 ARTIC, S3 DYNAMICS, S4 READ, new). The same four physical knobs drive
whichever row is **live**. The row is switched with the two buttons printed OCT- and OCT+ (OCT- = ROW 1, OCT+ =
ROW 2): a tap latches the row, holding the button while turning is a momentary peek. The knob band of the main screen
always shows **both rows** with all eight labels and values, the live row filled and the other dimmed. The keyboard
octave, the only function of those two buttons, moves into the SCL layer.

## 1. The SHIFT control

### 1.1 Which control: OCT- and OCT+ (the row buttons)

Candidates were scored against control_map.tsv for conflicts in all seven contexts. A candidate needs a **free tap**
(so a tap can latch the row, which is what makes the input score 2 instead of 1, see 4.1) and, if possible, a free
hold (so hold-and-turn can be momentary).

| Candidate | Existing roles (tap / hold / combo) | Verdict |
| --- | --- | --- |
| `B_HOME` | tap KEYS legend (the only on-screen key labels, F20 and F28 depend on it) / hold 0.7 s MENU | Rejected: the hold is taken by the menu, the tap by the legend; moving the legend drops F20 and F28 visual to 1 (total 7, fail). |
| `B_SAVE` | tap save / hold 0.7 s undo a load | Rejected: both taken. |
| `B_REC` | tap enter or leave STEP / hold 0.7 s clear pins | Rejected: both taken. |
| `B_EDIT` | tap UNDO (F15 = 9, performance) / hold CHAN layer | Rejected: both taken. |
| `B_GLO` | tap GLOB page / hold GLOB page | Rejected: both taken. |
| `B_PLAY`, `B_FX`, `B_SCL`, `B_ENV`, `B_LFO`, `B_ARP`, `B_SEQ` | dice, layers, transport | Rejected: performance gestures (each scores 9). |
| **`B_OCTDN` + `B_OCTUP`** | press = keyboard octave -1 / +1 (F26, setup, 8); in STEP_HELD = pin octave; both held 5 s = UPDATE MODE; both at power-on = CALIBRATION | **Chosen.** The tap and the hold are free once the keyboard octave moves. Two buttons give an explicit pair (OCT- = ROW 1, OCT+ = ROW 2): no toggle state to guess. |

Why the keyboard octave can move: it only transposes live-played notes (F26 audition, setup class). It becomes
**hold SCL, then OCT- / OCT+** (see 1.4, F35 = 7, setup needs >= 6, no 0). The global OCTAVE (hold SCL, K3) is
untouched.

R1 in UX_REVIEW (section 4) rejected "OCT+ held as SHIFT" because the shift role was **unlabelled**; here the role is
labelled in the button band and the knob band (section 2), which was the failing sub-score.

### 1.2 Gestures (exact)

| Gesture | Result |
| --- | --- |
| **Tap OCT+** (released before HOLD, 0.4 s default, and no knob turned during the press) | Latches ROW 2. Acts on release (the button now has a hold role, so it follows the tap-on-release rule). Toast `ROW 2` for 0.8 s. |
| **Tap OCT-** (same) | Latches ROW 1. Toast `ROW 1`. Tapping the button of the row that is already live changes nothing (a set, not a toggle), and shows the toast. |
| **Hold OCT+ and turn a knob** | ROW 2 is live **while held** (momentary). On release the latched row returns and the release does nothing else. The first detent of the turn applies to ROW 2 at once. |
| **Hold OCT- and turn a knob** | ROW 1 is live while held (a peek at ROW 1 while ROW 2 is latched). Same release rule. |
| Hold either OCT past HOLD without turning | The knob band previews that row (see 2.3) from 0.15 s after the press; on release nothing changes (no latch, no octave). |
| Press the second OCT button while one is held | No effect (the pair stays reserved for the hidden gestures: both held 5 s = UPDATE MODE; both at power-on = CALIBRATION, unchanged). |
| Turning a knob while neither OCT is held | Turns the latched row. Power-on: ROW 1. The latch is UI state, not saved in presets, one latch for the whole device (not per channel); it is not reset by channel select, algorithm change, preset load, mute, STEP, or transport start/stop (the knob band keeps showing the latched row while stopped, so the screen is never ambiguous). Only power-on resets it to ROW 1. |

"Shift held while turning" is therefore well defined: the held button's row wins for every knob turned during the hold.
There is no timeout: a latched ROW 2 stays until OCT- is tapped, and the screen shows it permanently (section 2).

### 1.3 Relation to the other holds and every context

One held modifier at most (CONTROL_MAP section 1) still holds: hold-OCT is the only modifier in PERF and STEP; the
other holds (SCL, EDIT, GLO, a white key) are different contexts, in which OCT has its own role below.

| Context | OCT- | OCT+ | Knob rows |
| --- | --- | --- | --- |
| `PERF` | tap: latch ROW 1; hold + turn: momentary ROW 1 | tap: latch ROW 2; hold + turn: momentary ROW 2 | K1..K4 turn the live row |
| `STEP` | as PERF | as PERF | as PERF (typed steps and morphs coexist) |
| `STEP_HELD` | pin note -12 semitones (unchanged) | pin note +12 semitones (unchanged) | K1, K2, K4 turn the latched row; K3 is the pin NOTE (labelled), the row cannot be switched while a step key is held |
| `SCL` (held) | keyboard octave -1 (new home, `KB OCT`) | keyboard octave +1 | K1 KEY, K2 SCALE, K3 OCTAVE; K4 turns the live row's fourth function (K4 or S4) |
| `CHAN` (held EDIT) | inert (`-`) | inert (`-`) | K1..K4 are the four MIDI channels |
| `GLOB` | inert (`-`) | inert (`-`) | K1 GATE, K2 DIV, K3 SWING, K4 BPM |
| `MENU` | previous value / back (unchanged) | next value / do it (unchanged) | value of the selected row |

No role is lost: the keyboard octave (the only previous function of the pair) lives in SCL; the two hidden
maintenance gestures need no tap or hold role from the pair. In CHAN and GLOB the pair is inert because the knob band
is replaced there and a row switch would be invisible.

## 2. The live-screen rule (both rows always visible, the live row unmistakable)

This is the contract for P2-003 (exact pixel placement is P2-003's; the zone, text limits and states are fixed here).

### 2.1 The knob band (replaces the 40 px band of UX_REVIEW 1.4)

Band height **52 px** at the same place (between the sequence band and the button band); the sequence band gives up
12 px (80 to 68 px). Header 24 + channels 32 + sequence 68 + knobs 52 + buttons 64 = 240.

```
x:   0  12                                                    239
y0   | 1 | [ DENS  ][ PEDAL ][ JUMP  ][ ACCNT ]      ROW 1  (26 px)
y26  | 2 | [ OFFSET][ SLIDE ][ PUNCH ][ DIR   ]      ROW 2  (26 px)
```

- **Gutter** 12 px wide at the left edge holding the row number `1` / `2` (AF_S). The live row's digit is drawn
  inverted (theme `T_ACCENT` background, `T_INK` digit); the other is `T_DIM` on `T_SURF`.
- **Cells**: 4 per row, each 57 x 26 px (4 x 57 = 228 + 12 gutter = 240). Each cell has two text lines in AF_S
  (nominal 12 px; the face's line box is 15 px and its capital letters are 9 px tall, ink rows 3..11, so two lines of
  capitals fit a 26 px cell only when P2-003 places them by the capitals' band, as `cv_text` does for centred text; the
  labels and values are capitals, digits and symbols, so no descender is needed). Text widths measured with `text_w(&AF_S, ...)`:
  the longest real label `OFFSET` is 42 px and `ACCNT` 39 px, inside the 57 px cell. The two lines: the **label** (line 1, up to **6 characters**, left aligned) and the **value readout** (line 2, up to
  **5 characters**, left aligned).
- **Live row**: cells filled `T_ACCENT`, text `T_INK`. **Other row**: cells `T_SURF` fill, label and value in
  `T_DIM`, so all eight values stay readable at once. A cell turned in the last 1.0 s is inverted again (`T_TEXT`
  background, `T_INK` text) on top of its row style, as in the existing highlight.
- Label limits: every label is at most 6 characters (tables in section 3 and the check in section 6). Value text: the
  number, or a name of at most 5 characters (`FWD BWD PEND`, `AUTO SINE TRI SAW+ SAW- ARCH INV`, `KEYS` for K1 = 0).
- A function that does not exist for an algorithm would be drawn `-`; v1 has none (all 8 slots exist for all 4
  algorithms).

### 2.2 Shift indicator, three redundant cues

1. The **gutter digit** and the **filled row** (above): the live row is the filled one.
2. The **button band**: the OCT- cell reads `ROW 1` and the OCT+ cell reads `ROW 2` (sub-label `tap/hold`); the cell
   of the live row is drawn inverted. These cells replace `OCT n` / `PIN OCT` labels outside STEP_HELD (where they
   read `PIN OCT` as before) and outside the SCL layer (where they read `KB OCT n`).
3. A **toast** on every row change (tap, or the first detent of a hold): `ROW 2` or `ROW 1`, 0.8 s, centred in the
   top 12 px of the sequence band, plus `(held)` while the row is momentary.

### 2.3 Instruction text and value readout on turn

- While a row other than the latched one is live (a hold), the sequence band's top line reads
  `HOLD OCT+: ROW 2` / `HOLD OCT-: ROW 1`, so the screen says how to get back.
- A tap-latched ROW 2 shows `ROW 2: tap OCT- for ROW 1` in the same line for the first 3 s after the latch and then
  clears (the gutter and fill keep saying it).
- **Readout on turn**: the turned cell shows the new value in its line 2 within the same frame (output 3), and a
  1.0 s readout replaces the toast line: `S2 SLIDE 6   0..8` (slot, label, value, range; 20 characters at most).
  Value names for enums (`DIR BWD`) are used in both places.
- Hold preview: from 0.15 s after pressing an OCT button (without turning), the band shows the row of that button as
  live, so a producer who presses and thinks sees what the turn will do.

### 2.4 States that must be drawn (acceptance list for P2-003 and the scenarios)

| State | Gutter | Rows | OCT cells | Top line |
| --- | --- | --- | --- | --- |
| ROW 1 latched, nothing held | `1` inverted | ROW 1 filled | `ROW 1` inverted, `ROW 2` plain | none |
| ROW 2 latched | `2` inverted | ROW 2 filled | `ROW 2` inverted | `ROW 2: tap OCT- for ROW 1` (3 s) |
| ROW 1 latched, OCT+ held | `2` inverted | ROW 2 filled | `ROW 2` inverted | `HOLD OCT+: ROW 2` |
| ROW 2 latched, OCT- held | `1` inverted | ROW 1 filled | `ROW 1` inverted | `HOLD OCT-: ROW 1` |
| STEP_HELD | latched row | latched row filled; K3 cell reads `NOTE` | `PIN OCT` | none |
| SCL layer | latched row | band replaced by the SCL layer (K4 cell shows the live row's fourth function) | `KB OCT n` | none |

## 3. The morph tables (per algorithm, up to 8)

Conventions.

- **ROW 1** (unchanged): K1 DENSITY (how many), K2 FORM (loop length or character), K3 PITCH (how wide), K4 SPICE
  (the twist on top).
- **ROW 2** (new): **S1 START** (where the loop begins: `OFFSET`), **S2 ARTIC** (how much articulation: slide,
  ratchet, gate lane length, ratchet speed), **S3 DYNAMICS** (how loud and how different: `PUNCH`, `DYN`), **S4 READ**
  (the order in which the content is read: direction, or contour curve).
- **Every ROW 2 default is the neutral value**: with ROW 2 at its defaults the output is **byte-identical** to the
  4-morph design. The golden files of the base row stay valid (a regression check for P2-008).
- Every ROW 2 morph is a pure function of (its value, the seeds, the cache), monotonic where it has a count or
  probability, so turning back restores the exact pattern (VISION rule 4). No ROW 2 morph draws a random number: each
  uses draws that already exist.
- Columns: `slot` K1..K4 / S1..S4 (= morph ids 0..7 in order, ADR-0003); `step` = change per detent when turned slowly;
  `fast` = per detent when detents arrive under 40 ms apart (5.1); `id` = the parameter id in the algorithm doc 6.1;
  formulas use the primitives of the algorithm docs 6.2/6.3 plus SP16 and SP17 (ALGORITHM_SET 6).

### 3.1 ACID (docs/design/algorithms/sting.md)

| Slot | Label | Id | Range | Default | Step / fast | Formula | Restores (source) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| K1 | `DENS` | `den` | 0..16 | 12 | 1 / 1 | `hit[i] = rank_hit[i] < den` | unchanged |
| K2 | `PEDAL` | `typ` | 0..12 | 6 | 1 / 1 | first `typ` of `ord_type` set to the root | unchanged |
| K3 | `JUMP` | `jmp` | 0..20 | 5 | 1 / 1 | `p_eff = min(100, base_p[i]*jmp/5)` | unchanged |
| K4 | `ACCNT` | `acc` | 0..8 | 3 | 1 / 1 | first `acc` hit steps in `ord_acc` accented | unchanged |
| S1 | `OFFSET` | `off` | 0..15 | 0 | 1 / 1 | read index `u = t + off` (before DIR) | sting-algorithms 05 section 5.3 Offset (cut in sting.md 6) |
| S2 | `SLIDE` | `sl` | 0..8 | 4 | 1 / 1 | `ps_eff[i] = min(100, ps[i] * sl / 4)`; `slide[i] = u[i] < ps_eff[i]` | owner question 7 (slide amount); sting.md 6.3 SLIDE (fixed table) |
| S3 | `PUNCH` | `pun` | 0..47 | 40 | 1 / 4 | accented velocity `min(127, 80 + pun)`, unaccented 80 | sting-algorithms 02 "Dynamics controls" bipolar Accent (cut in sting.md 6) |
| S4 | `DIR` | `dir` | 0..2 | 0 | 1 / 1 | `i = dir_map(u, 16, dir)` (SP16): 0 `FWD`, 1 `BWD`, 2 `PEND` | sting-algorithms 05 section 5.3 Direction (Bwd, Pendulum) |

Still cut, with reason: Classic algorithm v1 (chromatic urn: a scale is mandatory, TYPE covers the random end);
Jitter and CakeWalk (free-running randomness breaks "same settings and seed play the same pattern"); Poly Reset and
Clock = Gate (transport features, not algorithm parameters); Division (global DIV); Global Gate dial (global GATE);
Gen Clip (not applicable); a gate-length morph (the GATE dice layer plus global GATE already cover it).

### 3.2 EUCLID (docs/design/algorithms/t1.md)

| Slot | Label | Id | Range | Default | Step / fast | Formula | Restores (source) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| K1 | `HITS` | `hit` | 0..16 | 5 | 1 / 1 | `p = min(hit, n)`; Bjorklund `E(p, n)` | unchanged |
| K2 | `LEN` | `len` | 2..16 | 16 | 1 / 1 | `n = len` | unchanged |
| K3 | `SPAN` | `spn` | 0..24 | 12 | 1 / 1 | contour amplitude | unchanged |
| K4 | `ACCNT` | `acc` | 0..8 | 2 | 1 / 1 | first `acc` ordinals in `ord` accented | unchanged |
| S1 | `OFFSET` | `off` | 0..15 | 0 | 1 / 1 | `R = (R_dice + off) mod n` | owner question 28 (ROTATE as a knob); t1-algorithms 03 rotation |
| S2 | `RATCH` | `rat` | 0..8 | 4 | 1 / 1 | `P2 = 15*rat/4`, `P3 = 20*rat/4`; `reps = 2 if u < P2, else 3 if u < P3, else 1` | owner question 27 (RATCH amount); t1-algorithms 04 section 4.1 repeats |
| S3 | `PUNCH` | `pun` | 0..47 | 40 | 1 / 4 | accented velocity `min(127, 80 + pun)`, unaccented and pinned 80 | t1-algorithms 04 section 4.3 velocity slope (as an accent depth) |
| S4 | `CURVE` | `crv` | 0..6 | 0 | 1 / 1 | `shape = crv if crv > 0 else 1 + rnd(6)`; 1 SINE 2 TRI 3 SAW+ 4 SAW- 5 ARCH 6 INV | t1-algorithms 05 section 5.3 melodic contour (selectable shape) |

Value names for `CURVE`: 0 `AUTO`, 1 `SINE`, 2 `TRI`, 3 `SAW+`, 4 `SAW-`, 5 `ARCH`, 6 `INV`.

Still cut, with reason: steps 17..64 (keys reach 16; owner question 26); probability and the random depth/rate/targets
(non-deterministic); chord size, voicing and arp style (monophonic bass; chords belong to the deferred ARP);
per-channel scale, root, division, swing (global); repeats/pace/offset/time/sustain sliders and groove id (global DIV,
SWING, GATE; the pace direction stays a seeded per-hit draw); P-locks (replaced by pins); cycles and 6 tracks.

### 3.3 SNAKE (docs/design/algorithms/snake.md)

| Slot | Label | Id | Range | Default | Step / fast | Formula | Restores (source) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| K1 | `DENS` | `den` | 0..16 | 10 | 1 / 1 | `gate[k] = rank_g[k] < den` | unchanged (0 = KEYS ONLY) |
| K2 | `LEN` | `len` | 1..16 | 7 | 1 / 1 | pitch lane length | unchanged |
| K3 | `RANGE` | `rng` | 0..48 | 12 | 1 / 4 | `p = raw*rng/255` | unchanged |
| K4 | `PATH` | `shp` | 1..14 | 7 | 1 / 1 | `order[]` from `SHAPE[shp]` | unchanged |
| S1 | `OFFSET` | `off` | 0..15 | 0 | 1 / 1 | `u = t + off` for all three lanes | snake-algorithms 06 section 6.3 Steps Shift |
| S2 | `GLEN` | `gl` | 1..16 | 16 | 1 / 1 | gate lane length: `hit = g[u mod gl]`, `g[j] = gate[order[j]]` for `j < gl`; if none set, `g[0] = 1` | snake-algorithms 02 per-lane length (cut as "a free Gate-lane length" in snake.md 6) |
| S3 | `DYN` | `dyn` | 0..10 | 10 | 1 / 1 | `vel = clamp(100 - ((100 - v) * dyn) / 10, 1, 127)` (SP17) | snake-algorithms 02 velocity lane range |
| S4 | `DIR` | `dir` | 0..2 | 0 | 1 / 1 | pitch lane index `cp = order[dir_map(u, len, dir)]` (SP16); the gate and velocity lanes read forward | snake-algorithms 02 per-lane Direction (5 modes; the deterministic ones are kept as FWD, BWD, PEND) |

Still cut, with reason: per-lane Speed 32n..2n (global DIV; polymeter comes from LEN, GLEN and the dice POLY);
Direction `random` and the two unconfirmed modes (non-deterministic or unconfirmed); per-lane Shape (PATH is one
shape for all lanes); Swing and Note length (global); the 37 scales and User scale (global scale); the three Custom
modulation lanes (rule 3: a channel has only a MIDI channel; owner question 6); MIDI-in control and Reset (global
scale, dice-all and undo replace them).

### 3.4 STAGE (docs/design/algorithms/ml185.md)

| Slot | Label | Id | Range | Default | Step / fast | Formula | Restores (source) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| K1 | `DENS` | `den` | 0..8 | 6 | 1 / 1 | `play[k] = rank_r[k] < den` | unchanged (0 = KEYS ONLY) |
| K2 | `TOTAL` | `tot` | 8..64 | 16 | 1 / 4 | pulses per loop | unchanged |
| K3 | `RANGE` | `rng` | 0..36 | 12 | 1 / 4 | `p = raw*rng/255` | unchanged |
| K4 | `RATCH` | `rat` | 0..8 | 2 | 1 / 1 | first `rat` eligible stages get mode II | unchanged |
| S1 | `OFFSET` | `off` | 0..63 | 0 | 1 / 4 | `u = t + off`; `p = dir_map(u, tot, dir)` | new for the convention (the source follows the host position, so it has no offset; justified in 3.5) |
| S2 | `RDIV` | `rdiv` | 1..4 | 4 | 1 / 1 | ratchet divider cap: `div_eff[k] = min(div[k], rdiv)`; eligibility uses `div_eff` | ml185-algorithms 03 per-stage Div (cut as a hand control in ml185.md 6) |
| S3 | `DYN` | `dyn` | 0..10 | 10 | 1 / 1 | `vel[k]` as SNAKE S3 (SP17) | ml185-algorithms 04 velocity lane range |
| S4 | `DIR` | `dir` | 0..2 | 0 | 1 / 1 | `p = dir_map(u, tot, dir)` over the `tot` pulses (SP16) | ml185-algorithms 05 section 5.3 Direction (`>`, `<`, `><`) |

Still cut, with reason: per-stage Mode, Length and Div as hand controls (24 settings; the MODE, LENS and RESTS dice
layers replace them); Direction `R` and the unconfirmed values 3 and 4 (non-deterministic or unconfirmed); Length Mode
and the Sequence Length dial (TOTAL replaces both); Clock Timing and Swing Amount (global); Coarse Tune (global
OCTAVE); Aux layer and Aux CC (rule 3); Global Step Division and hidden reset; MIDI Thru; a long-hold odds morph (the
LONG dice layer re-rolls it; a knob would need a ninth slot).

### 3.5 Checks and honesty notes

- Every label above is at most 6 characters (`OFFSET` is the longest); every enum value name at most 5.
- Restored versus new: ACID S1 S2 S3 S4, EUCLID S1 S2 S3 S4, SNAKE S1 S2 S3 S4 and STAGE S2 S3 S4 restore a named
  source parameter or an owner question (7, 27, 28). **STAGE S1 OFFSET is the one slot with no source parameter**; it
  is kept so that "S1 = where the loop starts" is true for all four algorithms (a producer learns one rule). A table
  edit can give STAGE S1 to a ninth idea without touching the others (flex point 6.2).
- EUCLID `RATCH`: whether a hit is ratcheted at all is monotonic in `rat`; its repeat count may drop from 3 to 2 when `rat` rises past a threshold (the 3x band `P2 <= u < P3` is overtaken by the 2x band). The result is still a pure function of `rat`, so turning back restores the pattern.
- Dice layers are unchanged: the five shared layers and their seeds (ALGORITHM_SET 4). The ROW 2 morphs only read
  seeds; they add none. `ROT` (EUCLID dice) and `OFFSET` add: the dice gives a random rotation, the knob moves it
  from there.
- Pins are keyed to the step position `t mod loop_len` by the core and ignore OFFSET and DIR (a pin on key 5 stays on
  key 5), so step entry is never disturbed by the second row.

## 4. Ease scores

Rubric: UX_REVIEW 3.1 (visual / input / output 0..3). Pass for a performance function: total >= 8, no sub-score below
2. "Current" is the firmware as scored in UX_REVIEW 3.2 (the 4-knob design had no way to reach the cut parameters: 0; the T-1 pages had an unlabelled OCT+ SHIFT: F30 = 4).

### 4.1 Scores

| Id | Function | Class | Current V/I/O = total | Proposed control | Proposed V/I/O = total | Pass |
| --- | --- | --- | --- | --- | --- | --- |
| F30 | Switch the knob row | P | T-1: SEQ pages and unlabelled OCT+ SHIFT: 1/1/2 = 4 | tap OCT+ / OCT-, or hold and turn | 3/3/3 = 9 | yes |
| F08 | Morph live, ROW 1 (K1 DENSITY, K2 FORM, K3 PITCH, K4 SPICE), each of the 4 knobs, all 4 algorithms | P | 3/3/3 = 9 | K1..K4 on a latched ROW 1 | 3/3/3 = 9 | yes |
| F31 | Turn S1 START (`OFFSET`) | P | not reachable (cut): 0/0/0 = 0 | tap OCT+, turn K1; or hold OCT+ and turn K1 | 3/2/3 = 8 | yes |
| F32 | Turn S2 ARTIC (`SLIDE`, `RATCH`, `GLEN`, `RDIV`) | P | not reachable: 0/0/0 = 0 | tap OCT+, turn K2 | 3/2/3 = 8 | yes |
| F33 | Turn S3 DYNAMICS (`PUNCH`, `DYN`) | P | not reachable: 0/0/0 = 0 | tap OCT+, turn K3 | 3/2/3 = 8 | yes |
| F34 | Turn S4 READ (`DIR`, `CURVE`) | P | not reachable: 0/0/0 = 0 | tap OCT+, turn K4 | 3/2/3 = 8 | yes |
| F35 | Keyboard octave | S | keys silent in T1 mode: 0/0/0 = 0 | hold SCL, then OCT- / OCT+ | 3/1/3 = 7 | yes (setup >= 6, no 0) |

Why each sub-score:

- **F30**: V3 (the OCT cells read `ROW 1` / `ROW 2` on the main screen and the band shows both rows). I3 (one direct
  control: one tap; the explicit pair removes toggle guessing). O3 (the gutter, fill and toast change within one
  frame, so the screen shows the new state).
- **F31..F34**: V3 (all eight labels are on the main screen at once, so the eye is already there). **I2**: two actions
  (tap OCT+, turn the knob). The rubric scores a hold plus a control as 1 ("modifier + control"); the **latch is
  what makes this 2**, and holding is offered only as a convenience on top of it (a producer who already has ROW 2
  latched needs one action). O3 (audible at the next step and the cell shows the new value in the same frame).
- **F08**: unchanged by this task (the band redraw does not alter the gesture), so the ROW 1 knob scores stay 9 while ROW 1 is latched (the power-on state). With ROW 2 latched a ROW 1 turn needs tap OCT- first (two actions, input 2, total 8) or hold OCT- and turn; still a pass.
- **F35**: V3 (`KB OCT n` in the SCL layer), I1 (modifier + control, the price of freeing the pair), O3.

### 4.2 The eight knobs for every algorithm

| Knob | ACID | EUCLID | SNAKE | STAGE | V/I/O = total |
| --- | --- | --- | --- | --- | --- |
| K1 | `DENS` | `HITS` | `DENS` | `DENS` | 3/3/3 = 9 |
| K2 | `PEDAL` | `LEN` | `LEN` | `TOTAL` | 3/3/3 = 9 |
| K3 | `JUMP` | `SPAN` | `RANGE` | `RANGE` | 3/3/3 = 9 |
| K4 | `ACCNT` | `ACCNT` | `PATH` | `RATCH` | 3/3/3 = 9 |
| S1 | `OFFSET` | `OFFSET` | `OFFSET` | `OFFSET` | 3/2/3 = 8 |
| S2 | `SLIDE` | `RATCH` | `GLEN` | `RDIV` | 3/2/3 = 8 |
| S3 | `PUNCH` | `PUNCH` | `DYN` | `DYN` | 3/2/3 = 8 |
| S4 | `DIR` | `CURVE` | `DIR` | `DIR` | 3/2/3 = 8 |

All 32 performance knob functions (8 knobs x 4 algorithms) score at least 8 with no sub-score below 2.
Totals over all 36 functions F00..F35 (as in UX_REVIEW 3.2; recomputed by script): current 88 / 324 (32 of 36 fail), proposed 297 / 324 (36 of 36 pass).

## 5. Knob value-change rule

### 5.1 Step and acceleration (relative detent knobs, `panel_enc()`)

- **Relative detents, no pickup.** K1..K4 are relative encoders, so switching rows can never make a value jump: the
  newly live knob continues from that morph's stored value.
- **One detent = one `step`** (column `Step`), always 1 in v1. **Slow is always exact**: a detent that arrives
  40 ms or more after the previous one in the same direction changes the value by `step` (1).
- **Fast turn, ranges over 32 values only**: when detents arrive under 40 ms apart in the same direction (the existing
  `ACC_GAP`), the change per detent is `fast` (column `Step / fast`: 4). Morphs with a range of 32 values or fewer
  never accelerate. The fast list is exactly: STAGE `TOTAL` (57 values), SNAKE `RANGE` (49), STAGE `RANGE` (37),
  `PUNCH` on ACID and EUCLID (48), STAGE `OFFSET` (64). The menu's KNOB ACCEL setting does not affect morphs (it keeps
  its existing effect on the menu rows); the morph behaviour is the same for everyone.
- A reversal of direction or a pause of 40 ms or more returns to `step` at once, so a value can always be landed
  exactly. Values clamp at `min` and `max`; there is no wrap.
- Worst case end to end: 64 values (STAGE `OFFSET`) = 16 fast detents, or 64 slow detents (3 turns of a 24-detent
  knob) for a deliberate sweep; every value is reachable slowly.
- A knob turn takes effect at the next step (the next-step commit of CONTROL_MAP section 0); the cell shows the value
  in the same frame.
- **Per-morph reset (P1-014, owner question 49):** hold UNDO (EDIT) and turn the knob: that one morph returns to its default; hold UNDO and tap OCT- / OCT+: a row; hold UNDO (EDIT) and tap FX: the whole algorithm (morphs and factory seeds). A tap of UNDO afterwards takes the reset back. The defaults are the neutral values, and a preset load or algorithm switch restores each algorithm's own stored values (5.2).

### 5.2 Morph values are kept per algorithm (decision S7)

Each channel stores, for each of the four algorithms, **eight morph values** and the seeds (ADR-0003 `AlgoState`
becomes `morph[8]` + `seed[6]` = 20 B). Switching a channel's algorithm only changes which `AlgoState` is read;
switching back restores all eight values (both rows) and every seed. The latched row is UI state and is not stored.
Presets save and restore the eight values for all four algorithms of all four channels.

## 6. Flex points (ADR-0004 addendum)

### 6.1 Changeable by a table row edit (no algorithm or screen code change)

| Table | Row / field | What the edit changes |
| --- | --- | --- |
| `AlgoDesc.knob_morph[8]` (ADR-0003 4) | slot -> morph id | Any of the 8 slots to any morph of that algorithm; swap S2 and S4; put DIR on K4 and PATH on S4. |
| `AlgoDesc.morph[8]` | label, min, max, default, step, fast step | Rename (<= 6 chars), re-range, re-default, make a knob fast or slow, for example STAGE `RANGE` fast 1. |
| Section 3 tables | the morph assigned to a ROW 2 slot | Replace STAGE `OFFSET` with another idea; replace `GLEN`; add `DIR` to EUCLID if a hardware test wants it. |
| `control_map.tsv` rows `B_OCTDN`, `B_OCTUP` | action, label | Move the row switch to other buttons, make it a toggle on one button, put the keyboard octave back. |
| Screen table (2.1) | band height, gutter width, fill colours, toast text, timings (0.8 s, 1.0 s, 3 s, 0.15 s) | Retune the instruction texts and timings; swap which row is drawn on top. |
| Fast threshold (5.1) | 40 ms, fast step 4 | Retune. |

### 6.2 Not changeable without code

| Item | Why |
| --- | --- |
| Two rows (8 slots) | `SQ_NMORPH`, `AlgoState` and `SeqPreset` layouts and the stored preset format (ADR-0003 3); a third row needs an ADR amendment. |
| ROW 2 morphs reading `t` (OFFSET, DIR): they live in `step()`, with `off` and `dir` in the cache | Changing which function reads `t` moves it between `build()` and `step()`. |
| The ROW 2 defaults being neutral | Defines the golden files (a non-neutral default changes every base-row golden). |
| Pins ignore OFFSET and DIR | Defined by the core's pin overlay keyed to `t mod loop_len`. |
| The tap-on-release rule for OCT | Gesture engine behaviour (`btn_hold`, `ui_layer.c`). |

## 7. Edits this design forces (all applied in this task)

| File | Edit |
| --- | --- |
| `control_map.tsv`, `CONTROL_MAP.md` | `B_OCTDN` / `B_OCTUP` rows (PERF, STEP, SCL, CHAN, GLOB); `EN_K1..K4` rows (PERF, STEP, STEP_HELD, SCL); section 0/1/2/3/4 text. |
| `UX_REVIEW.md` | 1.1 and 1.3 OCT rows; 1.4 knob band; workflows W5/W6/W14; scores F26 and F30..F35; 5.1 knob table (8 morphs); R1. |
| `ALGORITHM_SET.md` | Section 5 (convention gains ROW 2), SP16 and SP17 added to section 6. |
| Four algorithm docs, section 6 | 8-morph parameter tables (6.1), the new procedure lines (6.3), RAM (6.4), flex points (6.6). |
| ADR-0003 | `SQ_NMORPH` 8, `AlgoState` 20 B, `ChanSave` 102 B, `SeqPreset` 420 B, the `AlgoDesc` tables, the RAM table re-added, section 15. |

New owner questions: 48 to 52 in OWNER_QUESTIONS.md.
