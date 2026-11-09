# UX review: hardware controls x algorithms x workflow, with an ease score

Task P1-012 (owner directive 2026-11-10). Status: **for owner review** (gate `human-review`; approval also re-approves
P1-010). Design only: no firmware is changed by this document.

Inputs: VISION.md, OWNER_QUESTIONS.md (decisions 2026-11-09 and 2026-11-10, S1-S7), ADR-0004 and its flexibility
addendum, CONTROL_INVENTORY.md (current behaviour, with file:line), ALGORITHM_SET.md (P1-010 draft), the section 6 of
sting.md, t1.md, snake.md, ml185.md, `firmware/src/panel.c` and the panel photo `docs/controls.jpg`.

The machine-readable form of section 1 is [control_map.tsv](control_map.tsv) (343 rows: 49 controls x 7 contexts). The
table in 1.3 and the TSV come from one generator, so they agree row for row.

## 0. The idea in one paragraph

The FM-1 boots into the sequencer. One main screen (PERF) shows the four channels, the selected channel's algorithm,
its 16 steps, the four knob roles and a 2 x 6 picture of the button block with each button's current role. The
three printed encoders do what their printing says: **SELECT selects the channel, ALGORITHM picks the algorithm,
PRESETS picks the preset.** KNOB 1-4 always morph the selected channel (density, form, pitch width, spice). Five
buttons dice the five shared layers, FX dices all of them, EDIT undoes. REC enters steps on the 16 white keys. The
black keys mute and select channels. Three modifier buttons whose printing names a setting open it while held: SCL
(global scale and key), EDIT (channel MIDI channels), GLO (gate, division, swing, tempo). HOME held is the menu, and
the menu holds only set-and-forget settings.

## 1. Hardware map

### 1.1 Physical controls and their printing

From `panel.c` (enums `B_*`, `EN_*`, `B_NAME`, `E_NAME`) and `docs/controls.jpg`. Physical layout, left to right:
encoder block (top row MASTER, SELECT; bottom row PRESETS, ALGORITHM) with OCT- OCT+ under it; the screen; the knob
row KNOB 1..KNOB 4; under the knobs the 2 x 6 button block; the 27-key keyboard along the bottom.

| Control | Printed | Kind | Proposed role (PERF) | Printing fit | Page or owns the sequence |
| --- | --- | --- | --- | --- | --- |
| `MASTER` | MASTER | analogue pot (not an `EN_*`) | preview-synth and USB-audio volume | exact | device |
| `EN_SELECT` | SELECT | relative encoder | **select channel** CH1..CH4 | exact ("select") | page (what the knobs act on) |
| `EN_ALGO` | ALGORITHM | relative encoder | **algorithm of the selected channel** | exact (it was the track selector before) | owns |
| `EN_PRESET` | PRESETS | relative encoder | **preset slot** P01..P16 (whole-device state) | exact | owns |
| `EN_K1`..`EN_K4` | KNOB 1..4 | relative encoders | the four morphs of the selected channel, K1 left to K4 right, drawn in the same order on screen | exact (position = column) | owns |
| `B_FX` | FX | button, row 1 col 1 | DICE ALL | new meaning; screen label `ALL` | owns |
| `B_SCL` | SCL | row 1 col 2 | tap dice NOTES; hold = SCL layer (global KEY, SCALE, OCTAVE) | exact for the hold (scale); NOTES is the pitch layer that the scale shapes | owns (tap) / page (hold) |
| `B_ENV` | ENV | row 1 col 3 | dice ACCNT | envelope = dynamics; screen label `ACCNT` | owns |
| `B_LFO` | LFO | row 1 col 4 | dice SHAPE | LFO = shape over time; screen label `SHAPE` | owns |
| `B_EDIT` | EDIT | row 1 col 5 | tap UNDO/REDO dice; hold = CHAN layer (MIDI channel of CH1..CH4) | exact for the hold (edit the channels) | owns (tap) / page (hold) |
| `B_GLO` | GLO | row 1 col 6 | GLOB page: GATE, DIV, SWING, BPM | exact | page |
| `B_HOME` | HOME | row 2 col 1 | tap = back to PERF (on PERF: KEYS legend); hold 0.7 s = MENU | exact (as today) | page |
| `B_SAVE` | SAVE | row 2 col 2 | tap save to the shown slot; hold 0.7 s undo the last load | exact (hold-undo as today) | owns |
| `B_ARP` | ARP | row 2 col 3 | dice ARTIC | arpeggio = articulation (gate, glide, repeats); screen label `ARTIC` | owns |
| `B_SEQ` | SEQ | row 2 col 4 | dice HITS | sequencer steps = which steps sound; screen label `HITS` | owns |
| `B_PLAY` | PLAY/STOP | row 2 col 5 | start / stop all channels | exact | owns |
| `B_REC` | REC | row 2 col 6 | tap = STEP entry on/off; hold 0.7 s = clear the channel's pins | exact (record steps, as Felucca's step record) | owns |
| `B_OCTDN`, `B_OCTUP` | OCT-, OCT+ | buttons under the encoders | keyboard octave; with a step held, the pin's octave | exact | owns |
| 16 white keys F3..G5 | none | keys, place 0..15 | live notes; in STEP: steps 1..16 | n/a | owns |
| 11 black keys F#3..F#5 | small stock-firmware legends, not used | keys, place 0..10 | F#3 G#3 A#3 C#4 mute CH1-4; D#4 F#4 G#4 A#4 select CH1-4; C#5 unmute all; D#5 restart; F#5 tap tempo | n/a | owns |
| Hidden maintenance | - | OCT- + OCT+ | both = octave 0; both 5 s = UPDATE MODE; both at power-on = CALIBRATION (unchanged) | n/a | device |

Why this layout:
- **The dice cluster is spatial.** Row 1 holds the pitch and dynamics layers (SCL NOTES, ENV ACCNT, LFO SHAPE), row 2
  the time layers (ARP ARTIC, SEQ HITS) next to PLAY. FX (ALL) and EDIT (UNDO) flank the row-1 dice: "new everything"
  on the left, "take it back" on the right.
- **Buttons with a setting in their name open that setting when held** (SCL, EDIT, GLO), so every setting is under
  the button that names it. The tap of SCL and EDIT is a performance action (dice, undo) because those are used
  hundreds of times per session and the settings a few times.
- **Synth-only printing (FX ENV LFO ARP SEQ) becomes dice.** Rule 2: the synth is only a preview, so those labels
  have no job; the screen's 2 x 6 button picture shows the real role under each printed position.
- Felucca's groovebox pages (ENV, LFO, FX, ARP, SONG, sound editing) are not part of this map. They stay reachable
  only as a set-and-forget legacy mode, MENU > SYSTEM > MODE `SEQ`/`GROOVE` (default `SEQ`); in `GROOVE` the panel
  behaves exactly as CONTROL_INVENTORY.md describes. Owner question 32.

### 1.2 Contexts

The task's five contexts map onto seven panel states. "Algorithm edit" is deliberately not a separate state: picking
and morphing an algorithm happens on the main screen (a separate edit page would be a menu dive, rule 1).

| Context | Task's name | Entered by | Left by | Screen |
| --- | --- | --- | --- | --- |
| `PERF` | perform, algorithm edit, channel select | power-on default | n/a | main screen (1.4) |
| `STEP` | step entry | REC tap | REC tap or HOME tap | main screen, `STEP` banner, step lane shows key names 1..16 |
| `STEP_HELD` | step entry (a step key held) | holding a white key in STEP | letting go of all white keys | K3 cell reads `NOTE`, OCT cells read `PIN OCT` |
| `SCL` | global (scale and key) | holding SCL past HOLD (MENU > HOLD, default 0.4 s) | letting go of SCL | SCL layer (K1 KEY, K2 SCALE, K3 OCTAVE, K4 the morph; "ANY KEY = KEY") |
| `CHAN` | channel setup | holding EDIT past HOLD | letting go of EDIT | CHAN layer: four cells `CH1 MID` .. `CH4 MID` with the MIDI channel and algorithm of each |
| `GLOB` | global (feel and tempo) | GLO tap (latched) or GLO hold (while held) | GLO tap, HOME tap, or releasing a hold | GLOB page (K1 GATE, K2 DIV, K3 SWING, K4 BPM) |
| `MENU` | set-and-forget only | holding HOME 0.7 s | HOME tap or hold, or any non-menu button | Felucca menu (rows listed in 1.5) |

Hold thresholds: SCL, EDIT, GLO use the layer time (MENU > HOLD 0.3-0.6 s); HOME, SAVE, REC use the fixed 0.7 s
(`btn_hold`). A button with a hold role acts on **release** when tapped; every other button acts on **press**.

Precedence when two contexts could apply: MENU > SCL/CHAN (a held layer) > GLOB > STEP_HELD > STEP > PERF. Only one
held layer at a time (the second layer button pressed while one is held keeps its PERF role), as Felucca's layer
engine does today (`ui_layer.c`).

### 1.3 The control map (every control x every context)

`= PERF` means the control keeps its PERF role in that context. Each cell gives the on-screen label and the function
id scored in section 3. Full action text, page-or-owns and score per row: [control_map.tsv](control_map.tsv).
`+sub` means the label has a second line with the algorithm's own name for that layer (section 5.2).

| Control | PERF | STEP | STEP_HELD | SCL | CHAN | GLOB | MENU |
| --- | --- | --- | --- | --- | --- | --- | --- |
| B_FX | `ALL` F14 | = PERF | = PERF | = PERF | = PERF | = PERF | `close` F29 |
| B_SCL | `NOTES +sub` F09 | = PERF | = PERF | `SCALE` F18 | = PERF | = PERF | `close` F29 |
| B_ENV | `ACCNT +sub` F11 | = PERF | = PERF | = PERF | = PERF | = PERF | `close` F29 |
| B_LFO | `SHAPE +sub` F13 | = PERF | = PERF | = PERF | = PERF | = PERF | `close` F29 |
| B_EDIT | `UNDO` F15 | = PERF | = PERF | = PERF | `CHAN` F02 | = PERF | `close` F29 |
| B_GLO | `GLOBAL` F16 | = PERF | = PERF | = PERF | = PERF | `BACK` F16 | `close` F29 |
| B_HOME | `KEYS / hold MENU` F29 | `HOME` F29 | `HOME` F29 | `HOME` F29 | `HOME` F29 | `HOME` F29 | `BACK` F29 |
| B_SAVE | `SAVE Pnn` F22 | = PERF | = PERF | = PERF | = PERF | = PERF | `close` F29 |
| B_ARP | `ARTIC +sub` F12 | = PERF | = PERF | = PERF | = PERF | = PERF | `close` F29 |
| B_SEQ | `HITS +sub` F10 | = PERF | = PERF | = PERF | = PERF | = PERF | `close` F29 |
| B_PLAY | `PLAY / STOP` F25 | = PERF | = PERF | = PERF | = PERF | = PERF | = PERF |
| B_REC | `STEP` F04 | `STEP ON` F04 | `STEP ON` F06 | = PERF | = PERF | = PERF | `close` F29 |
| B_OCTDN | `OCT n` F26 | = PERF | `PIN OCT` F05 | = PERF | = PERF | = PERF | `-` F29 |
| B_OCTUP | `OCT n` F26 | = PERF | `PIN OCT` F05 | = PERF | = PERF | = PERF | `-` F29 |
| EN_SELECT | `CH n` F01 | = PERF | = PERF | = PERF | = PERF | = PERF | = PERF |
| EN_ALGO | `ALGO name` F03 | = PERF | = PERF | = PERF | = PERF | = PERF | `tab` F29 |
| EN_PRESET | `Pnn` F23 | = PERF | = PERF | = PERF | = PERF | = PERF | `row` F29 |
| MASTER | `-` - | = PERF | = PERF | = PERF | = PERF | = PERF | = PERF |
| EN_K1 | `algo label` F08 | = PERF | = PERF | `KEY` F18 | `CH1 MID` F02 | `GATE` F16 | `value` F29 |
| EN_K2 | `algo label` F08 | = PERF | = PERF | `SCALE` F19 | `CH2 MID` F02 | `DIV` F16 | `value` F29 |
| EN_K3 | `algo label` F08 | = PERF | `NOTE` F05 | `OCTAVE` F27 | `CH3 MID` F02 | `SWING` F16 | `value` F29 |
| EN_K4 | `algo label` F08 | = PERF | = PERF | = PERF | `CH4 MID` F02 | `BPM` F17 | `value` F29 |
| KEY_W01_F3 | `-` F26 | `step 1` F04 | `step 1` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W02_G3 | `-` F26 | `step 2` F04 | `step 2` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W03_A3 | `-` F26 | `step 3` F04 | `step 3` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W04_B3 | `-` F26 | `step 4` F04 | `step 4` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W05_C4 | `-` F26 | `step 5` F04 | `step 5` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W06_D4 | `-` F26 | `step 6` F04 | `step 6` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W07_E4 | `-` F26 | `step 7` F04 | `step 7` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W08_F4 | `-` F26 | `step 8` F04 | `step 8` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W09_G4 | `-` F26 | `step 9` F04 | `step 9` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W10_A4 | `-` F26 | `step 10` F04 | `step 10` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W11_B4 | `-` F26 | `step 11` F04 | `step 11` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W12_C5 | `-` F26 | `step 12` F04 | `step 12` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W13_D5 | `-` F26 | `step 13` F04 | `step 13` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W14_E5 | `-` F26 | `step 14` F04 | `step 14` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W15_F5 | `-` F26 | `step 15` F04 | `step 15` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_W16_G5 | `-` F26 | `step 16` F04 | `step 16` F05 | `KEY` F18 | = PERF | = PERF | `-` F26 |
| KEY_B01_F#3 | `MUTE1` F20 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B02_G#3 | `MUTE2` F20 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B03_A#3 | `MUTE3` F20 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B04_C#4 | `MUTE4` F20 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B05_D#4 | `CH1` F21 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B06_F#4 | `CH2` F21 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B07_G#4 | `CH3` F21 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B08_A#4 | `CH4` F21 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B09_C#5 | `ALL ON` F20 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B10_D#5 | `RESTRT` F28 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |
| KEY_B11_F#5 | `TAP` F17 | = PERF | = PERF | `KEY` F18 | = PERF | = PERF | = PERF |

Conflict check (done on the TSV): in every context each control has exactly one action, no two controls share an
action except deliberate duplicates (SELECT and the D#4..A#4 keys both select a channel; F#5 and GLOB K4 both set
tempo), and no key or knob has two roles at once. In `SCL` every key is KEY, so mute/select keys are unavailable
only while SCL is held.

### 1.4 What the main screen shows (so every role is labelled where the eye is)

Exact pixel layout belongs to the screen spec (P2-003); these are the fixed contents and their order, top to bottom,
mirroring the panel (knob row above the button block):

| Band (240 px wide) | Content |
| --- | --- |
| Header, 24 px | transport glyph and BPM (`EXT` when slaved), global `KEY SCALE` (for example `C MINPEN`), preset `P01` (`P03>` while a load is queued, `P05 EMPTY` on an empty slot), `STEP` banner in STEP |
| Channels, 32 px | four cells `1 ACID 2` (channel, algorithm, MIDI channel); selected cell inverted; muted cell dimmed with `M` |
| Sequence, 80 px | the selected channel's algorithm name in large type and its 16-step lane: hits, accents, pins (outlined), playhead; in STEP the white-key number under each step; STAGE draws stage bars over its TOTAL pulses |
| Knobs, 40 px | four cells K1..K4: label (max 6 characters) and value; a cell turned in the last 1 s is highlighted |
| Buttons, 64 px | 2 x 6 grid in the panel's order (FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC): current label, second line `+sub` for dice buttons; absent layer drawn dimmed with `-` |

The KEYS legend (HOME tap on PERF) replaces the sequence band with the 27-key map: black keys `MUTE1..4`, `CH1..4`,
`ALL ON`, `RESTRT`, `TAP`; white keys `NOTES` (PERF) or `1..16` (STEP).

### 1.5 The menu (hold HOME): set-and-forget only

Kept: DISPLAY tab (COLOR, STYLE, LARGE, ANIM, LEDS, SCREEN OFF, SCOPE, HOME view), CONTROL tab (HOLD, ACCEL, SCALE
LEDS, STEP PREVIEW, CHORD ENTRY), AUDIO tab (SPEAKER EQ, USB LEVEL, CLICK, CLICK LEVEL, COUNT-IN, TUNE), MIDI tab (MIDI
IN routing, AUTO SELECT, SCREEN WAKE, LEARN CLEAR), SYSTEM tab (MODE `SEQ`/`GROOVE` replacing T1 SEQ, USB SERIAL,
RESTORE LAST, HELP, CALIBRATION, INFO, ABOUT).

Removed from the menu because they are musical functions now on the panel: `MI_TCH1..4` (CHAN layer), `MI_SELECT`
(SELECT is fixed to channel select), `MI_BPMLOCK` (tempo is no longer on SELECT), `MI_T1SEQ` (replaced by MODE, and
`SEQ` is the default), `MI_LATCH` (FX layer does not exist in SEQ mode; kept for GROOVE only). Tab row counts change,
so the dev task that edits `menu_items.c` updates `tests/ui_test.c`.

## 2. Workflows (exact controls, from power-on in SEQ mode)

Defaults at power-on (first boot or after INIT): CH1 ACID MIDI 1 unmuted; CH2 EUCLID MIDI 2, CH3 SNAKE MIDI 3, CH4
STAGE MIDI 4, all three muted; global KEY C, SCALE MINPEN, OCTAVE 0, GATE 50 %, DIV 1/16, SWING 50 %, BPM 120;
preset P01; keyboard OCT 0. The preview synth plays the selected channel; MASTER is its volume.

| W | Workflow | Steps (controls pressed) | Function ids |
| --- | --- | --- | --- |
| W1 | Select channel 1 | 1. Turn SELECT until the header cell `1` is inverted, **or** press black key D#4 (`CH1`). | F01, F21 |
| W2 | Set its MIDI channel (TB-3 = 2) | 1. Hold EDIT (the CHAN layer opens after HOLD). 2. Turn K1 until `CH1 MID` reads `2`. 3. Let go of EDIT. The next note goes out on MIDI channel 2. | F02 |
| W3 | Pick an algorithm | 1. Turn ALGORITHM: `ACID > EUCLID > SNAKE > STAGE`. The new algorithm plays from the next step with its own stored morphs and seeds (S7). | F03 |
| W4 | Enter steps on the 16 white keys | 1. Tap REC (LED lit, `STEP` banner). 2. Turn K1 fully down (`KEYS ONLY`) if only typed steps should play. 3. Tap white key n to pin step n; it sounds at once and is outlined in the lane. 4. To change a pin's pitch: hold its key, turn K3 (`NOTE`, one scale degree per detent) and/or press OCT-/OCT+ (`PIN OCT`); each change is auditioned. 5. Tap a pinned key again to remove its pin. 6. Tap REC to leave STEP. Hold REC 0.7 s clears all pins (EDIT tap brings them back). | F04, F05, F06 |
| W5 | Dial a sequence in with the knobs | 1. Press PLAY. 2. Turn K1 (density), K2 (form), K3 (pitch width), K4 (spice); each detent is heard from the next step and its cell shows the value. | F07, F25 |
| W6 | Morph live | Same controls as W5 while playing. Every morph is monotonic and reversible: turning back restores the exact pattern. | F08 |
| W7 | Dice each layer | Press SCL (tap) = NOTES, SEQ = HITS, ENV = ACCNT, ARP = ARTIC, LFO = SHAPE. FX = all layers. EDIT tap = undo the last dice (tap again = redo, an A/B compare). The new layer plays from the next step; the toast names the algorithm's own layer (`ACID: OCTAVE`). | F09-F15 |
| W8 | Tune the algorithm | The algorithm's own parameters are the four morphs (W5). The global feel it reads: 1. Tap GLO. 2. Turn K1 GATE, K2 DIV, K3 SWING. 3. Tap GLO (or HOME) to go back. | F16 |
| W9 | Set the global scale and key (S5) | 1. Hold SCL. 2. Press any key (white or black) for KEY, **or** turn K1 (KEY); turn K2 (SCALE); turn K3 (OCTAVE). 3. Let go of SCL. Every channel re-snaps from the next step; the header shows `KEY SCALE` on every screen. | F18, F19, F27 |
| W10 | Mute / switch channels | Mute: press F#3 / G#3 / A#3 / C#4 for CH1-4 (toggle); C#5 unmutes all. Switch: SELECT or D#4 / F#4 / G#4 / A#4. Restart all from step 1: D#5. | F20, F21, F28 |
| W11 | Save / load | Save: tap SAVE (`SAVED P01`). Save to another slot: turn PRESETS to an empty slot (`P05 EMPTY`), tap SAVE. Load: turn PRESETS to a filled slot; it loads at the next loop start (immediately when stopped), the header shows `P03>` until then. Undo a load: hold SAVE 0.7 s (again: redo). | F22, F23, F24 |
| W12 | Tempo | Tap GLO, turn K4 (BPM); or tap F#5 in time (TAP). | F17 |
| W13 | Transport | PLAY starts / stops every channel. | F25 |
| W14 | Audition notes | Play the white keys in PERF (snapped to the global scale, OCT-/OCT+ shift the octave). | F26 |
| W15 | Set-and-forget settings | Hold HOME 0.7 s; ALGORITHM = tab, PRESETS = row, K1-K4 = value; HOME tap closes. | F29 |

The reference workflow (VISION) is therefore: SELECT to `1` -> hold EDIT, K1 to `2` -> ALGORITHM to `ACID` -> PLAY ->
K1-K4 and the dice buttons. That is 4 gestures to sound on the TB-3, all on the main screen except the EDIT hold.

## 3. Ease score

### 3.1 Rubric

Each function is scored 0-3 on three axes, total /9.

| Score | (a) VISUAL CUE | (b) INPUT EFFORT | (c) OUTPUT FEEDBACK |
| --- | --- | --- | --- |
| 0 | the role is not shown | menu dive or hidden combo | none |
| 1 | shown on another page | modifier + control, or more than 2 presses | delayed more than 1 step, or not shown |
| 2 | shown but small or ambiguous | two actions | audible at the next step (screen does not show the value) |
| 3 | labelled on the main screen where the eye already is | one direct control | audible within one step AND the screen shows the new value |

Rules used while scoring:
- A held layer's own screen counts as "where the eye is" for the controls it labels (the layer appears on the press).
- A hold of one control (0.7 s) is a two-phase gesture: input 2.
- For a function with nothing to hear (save, select), output 3 = the screen confirms within one frame.
- CURRENT scores assume T1 mode is already ON (MENU > SYSTEM > T1 SEQ); reaching it is scored separately as F00.

**Pass rule.** Performance functions (P: morph, dice, mute, select algorithm, and the other live gestures marked P)
need >= 8 with no sub-score below 2. Setup functions (S) need >= 6 with no 0.

### 3.2 Scores (current firmware vs proposed)

Current = CONTROL_INVENTORY.md, T1 mode. V/I/O = visual / input / output.

| Id | Function | Class | Current control (CONTROL_INVENTORY) | Current V/I/O = total | Proposed control | Proposed V/I/O = total | Delta | Proposed pass |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| F00 | Reach the sequencer | S | hold HOME, SYSTEM tab, T1 SEQ row, K to ON | 1/0/1 = 2 | power-on default | 3/3/3 = 9 | +7 | yes |
| F01 | Select channel | P | white keys 1-6 in T1 mode, unlabelled (ui_t1.c:85-99) | 1/3/2 = 6 | SELECT encoder | 3/3/3 = 9 | +3 | yes |
| F02 | Set the channel's MIDI channel | S | SEQ x3 to MOD/CC, hold OCT+, turn K1 (torso_t1_ui.c MOD/CC shift) | 1/1/2 = 4 | hold EDIT + K1..K4 | 3/1/3 = 7 | +3 | yes |
| F03 | Pick an algorithm | P | no algorithm concept; only T1 SEQ on/off in the menu | 0/0/1 = 1 | ALGORITHM encoder | 3/3/3 = 9 | +8 | yes |
| F04 | Enter a step (pin) | S | unreachable: `edit_step` never set, Step Mod has no UI | 0/0/0 = 0 | REC tap, then white key | 3/2/3 = 8 | +8 | yes |
| F05 | Set a pin's pitch | S | unreachable (P-LOCK) | 0/0/0 = 0 | hold step key + K3 / OCT | 3/1/3 = 7 | +7 | yes |
| F06 | Clear pins | S | unreachable | 0/0/0 = 0 | hold REC 0.7 s (EDIT tap undoes) | 3/2/3 = 8 | +8 | yes |
| F07 | Dial a sequence in with knobs | S | four T-1 pages via SEQ / OCT-, OCT+ shift for the second row | 3/1/2 = 6 | K1-K4 on PERF | 3/3/3 = 9 | +3 | yes |
| F08 | Morph live | P | as F07; density on RHYTHM, pitch on HARMONY, page changes needed | 3/2/2 = 7 | K1-K4 on PERF | 3/3/3 = 9 | +2 | yes |
| F09 | Dice NOTES | P | no dice | 0/0/0 = 0 | SCL tap | 3/3/3 = 9 | +9 | yes |
| F10 | Dice HITS | P | no dice | 0/0/0 = 0 | SEQ press | 3/3/3 = 9 | +9 | yes |
| F11 | Dice ACCNT | P | no dice | 0/0/0 = 0 | ENV press | 3/3/3 = 9 | +9 | yes |
| F12 | Dice ARTIC | P | no dice | 0/0/0 = 0 | ARP press | 3/3/3 = 9 | +9 | yes |
| F13 | Dice SHAPE | P | no dice | 0/0/0 = 0 | LFO press | 3/3/3 = 9 | +9 | yes |
| F14 | Dice all | P | no dice | 0/0/0 = 0 | FX press | 3/3/3 = 9 | +9 | yes |
| F15 | Undo / redo dice | P | TEMP (hold SAVE or LFO, OCT+ commits) undoes knob edits only | 1/1/2 = 4 | EDIT tap | 3/3/3 = 9 | +5 | yes |
| F16 | Tune: GATE, DIV, SWING | S | per track: RHYTHM K3 DIV, shift K3 GATE; swing not reachable | 2/1/2 = 5 | GLO tap + K1..K3 | 3/2/3 = 8 | +3 | yes |
| F17 | Tempo | S | SELECT turn (BPM) | 2/3/3 = 8 | GLO tap + K4, or F#5 TAP | 3/2/3 = 8 | 0 | yes |
| F18 | Set global key | S | per track HARMONY K2 ROOT, not global | 1/1/2 = 4 | hold SCL + any key or K1 | 3/1/3 = 7 | +3 | yes |
| F19 | Set global scale | S | per track HARMONY shift K2 SCALE | 1/1/2 = 4 | hold SCL + K2 | 3/1/3 = 7 | +3 | yes |
| F20 | Mute channel | P | black keys 1-6, unlabelled | 1/3/2 = 6 | black F#3..C#4; C#5 all on | 2/3/3 = 8 | +2 | yes |
| F21 | Switch channel | P | white keys 1-6, unlabelled | 1/3/2 = 6 | SELECT or black D#4..A#4 | 3/3/3 = 9 | +3 | yes |
| F22 | Save | S | T-1 flash save not wired | 0/0/0 = 0 | SAVE tap | 3/3/3 = 9 | +9 | yes |
| F23 | Load | S | not wired | 0/0/0 = 0 | PRESETS turn (queued to loop start) | 3/3/1 = 7 | +7 | yes |
| F24 | Undo a load | S | not wired | 0/0/0 = 0 | hold SAVE 0.7 s | 3/2/3 = 8 | +8 | yes |
| F25 | Start / stop | P | PLAY | 3/3/3 = 9 | PLAY | 3/3/3 = 9 | 0 | yes |
| F26 | Audition notes on keys | S | keys silent in T1 mode (song.grid 2) | 0/0/0 = 0 | white keys in PERF, OCT-/OCT+ | 2/3/3 = 8 | +8 | yes |
| F27 | Global output octave | S | per track HARMONY K1 PITCH | 1/1/2 = 4 | hold SCL + K3 | 3/1/3 = 7 | +3 | yes |
| F28 | Restart all from step 1 | P | GLO + PLAY in Felucca only; none in T1 mode | 0/0/0 = 0 | black D#5 | 2/3/3 = 8 | +8 | yes |
| F29 | Set-and-forget settings | S | hold HOME 0.7 s | 3/2/3 = 8 | hold HOME 0.7 s | 3/2/3 = 8 | 0 | yes |

Totals: current 84 / 270, proposed 249 / 270. Current: 26 of 30 functions fail. Proposed: 30 of 30 pass.
Performance functions proposed: F01 F03 F08-F15 F21 F25 = 9 each; F20 and F28 = 8 (V2: the black-key labels are on the
KEYS legend, not on the main screen; reviewer correction, they had been scored V3); none has a sub-score below 2.

## 4. Redesign iterations

Every CURRENT failure in 3.2 is redesigned by the proposed column. These are the alternatives that were scored on the
way and rejected; each line gives the score that settled it.

| # | Function | Tried | Score | Kept | Score | Why |
| --- | --- | --- | --- | --- | --- | --- |
| R1 | F02 MIDI channel | OCT+ held as SHIFT + SELECT on PERF | 1/1/3 = 5 (fail, shift role unlabelled) | hold EDIT, K1..K4 per channel | 7 | The layer screen labels all four channels; the knob under the label is the channel's column. |
| R2 | F02 MIDI channel | EDIT tap opens a latched CHAN page | 3/2/3 = 8 | hold EDIT | 7 | EDIT's tap is UNDO (F15 needs 9, a performance function); setup at 7 passes. |
| R3 | F14 / F15 | FX tap = UNDO, FX hold = DICE ALL | ALL 3/2/3 = 8 | FX press = ALL, EDIT tap = UNDO | 9 / 9 | Dice-all is the fastest way to a new sequence (rule 5): it gets a direct press. |
| R4 | F12 + slide | ARP press = GATE+SLIDE; ARP hold = SLIDE only | slide-only 2/2/3 = 7 (fail as P) | one ARP press dices GATE and SLIDE (ACID) | 9 | See S1 (5.3). |
| R5 | F19 scale | PRESETS encoder = scale | 3/3/3 = 9 | hold SCL + K2 | 7 | PRESETS printing would lie and save/load would lose its encoder (F23 drops to 0). Scale is setup; 7 passes. |
| R6 | F23 load | load on PRESETS detent immediately | 3/3/3 = 9 | queue to the loop start | 3/3/1 = 7 | An immediate load cuts the bar mid-phrase and one stray detent replaces the live state. Setup rule allows the 1. |
| R7 | F04 entry | piano-roll entry (keys = notes, cursor advances) | 2/2/3 = 7 | white key = step (TR style) | 8 | VISION names the 16 white keys as steps; the lane under the keys is the visual cue. |
| R8 | F17 tempo | SELECT = tempo (today) | 8 | GLO K4 or TAP | 8 | Same score; SELECT's printing is "select", which F01 needs at 9. |
| R9 | F01 / F21 | white keys 1-4 select, black keys mute (T1 today) | 1/3/2 = 6 | SELECT + black D#4..A#4 | 9 | White keys are reserved for steps (rule 3); the channel strip shows each channel's key. |
| R10 | F13 SHAPE label | label `SHAPE` alone | 2/3/3 = 8 | `SHAPE` + algorithm sub-label | 9 | See S2 (5.3). |
| R11 | F08 ACID K2 | label `TYPE` | 2/3/3 = 8 | label `PEDAL` | 9 | See S3 (5.3). |

**Unavoidable failures: none.** Accepted low sub-scores (setup only, allowed by the pass rule): F23 output 1 (queued
load, R6); input 1 on F02, F05, F18, F19, F27 (modifier + control: a held layer is the price of keeping settings off
the main screen without a menu).

## 5. Algorithm alignment

### 5.1 Knobs (K1 DENSITY, K2 FORM, K3 PITCH, K4 SPICE)

Each cell: on-screen label, range, default. Score is the F08 score of that cell (V/I/O). K1 now starts at 0 on every
algorithm: 0 = `KEYS ONLY`, no generated hits, only pins play (rank-order density with `n = 0` plays nothing;
`t1_euclidean_bjorklund(steps, 0)` already returns 0). This is a range edit only.

| Algorithm | K1 | K2 | K3 | K4 | Score per knob |
| --- | --- | --- | --- | --- | --- |
| `ACID` | `DENS` 0..16, 12 | `PEDAL` 0..12, 6 (was `TYPE`) | `JUMP` 0..20, 5 | `ACCNT` 0..8, 3 | 3/3/3 = 9 each |
| `EUCLID` | `HITS` 0..16, 5 | `LEN` 2..16, 16 | `SPAN` 0..24, 12 | `ACCNT` 0..8, 2 | 9 each |
| `SNAKE` | `DENS` 0..16, 10 | `LEN` 1..16, 7 | `RANGE` 0..48, **12** | `PATH` 1..14, 7 | 9 each |
| `STAGE` | `DENS` 0..8, 6 | `TOTAL` 8..64, 16 (step 4; 1 when turned slowly) | `RANGE` 0..36, 12 | `RATCH` 0..8, 2 | 9 each |

### 5.2 Dice buttons

Main label (always the shared name) / sub-label (the algorithm's own layer, max 6 characters) / seeds diced.
`-` = absent: the button is drawn dimmed as `SHAPE -`, a press changes nothing and shows `NO SHAPE IN EUCLID` for
0.5 s (ALGORITHM_SET 4.4).

| Button (printed) | Layer | ACID | EUCLID | SNAKE | STAGE | Score |
| --- | --- | --- | --- | --- | --- | --- |
| SCL tap | `NOTES` | `NOTES` s_notes | `NOTES` s_notes | `NOTES` s_notes | `NOTES` s_notes | 9 |
| SEQ | `HITS` | `HITS` s_hits | `ROTATE` s_rot | `GATES` s_gates | `RESTS` s_rests | 9 |
| ENV | `ACCNT` | `ACCNT` s_acc | `ACCNT` s_acc | `VELO` s_vel | `VELO` s_vel | 9 |
| ARP | `ARTIC` | `GT+SLD` s_gate + s_slide | `RATCH` s_rat | `-` | `RT+HLD` s_mode | 9 (present) |
| LFO | `SHAPE` | `OCTAVE` s_oct | `-` | `DRIFT` s_poly | `LONG` s_lens | 9 (present) |
| FX | `ALL` | 6 seeds | 4 | 4 | 5 | 9 |
| EDIT tap | `UNDO` | swaps every seed of the last dice action with its previous seed | same | same | same | 9 |

### 5.3 Owner items S1-S4, S6 resolved by the score

| Item | Resolution | Score evidence |
| --- | --- | --- |
| **S1** slide on ARTIC | ACID's ARTIC press (ARP) dices GATE and SLIDE together, each with its own seed and previous seed (undo restores both). No separate slide gesture in v1. Sub-label `GT+SLD` says so. | One press: 3/3/3 = 9. A slide-only hold on ARP scores 2/2/3 = 7 and fails the performance rule (R4); it would also force ARP to act on release for all four algorithms. A slide-only binding stays a table row (flex point 7.2). |
| **S2** SHAPE naming and scope | Keep the layer and the name `SHAPE`; scope = "structure that is none of the other four". Its ambiguity is resolved on screen by the sub-label (`OCTAVE`, `DRIFT`, `LONG`, `-`) and the dice toast. The sub-label rule applies to all five buttons. | `SHAPE` alone: visual 2 (ambiguous) = 8; with sub-label 3 = 9 (R10). A rename alone does not help: the three algorithms' SHAPE contents differ, only the sub-label names each honestly. |
| **S3** ACID TYPE on K2 | Keep on K2 (FORM: how much of the loop repeats versus varies) and relabel `PEDAL` (0 = raw motif, 12 = the root hammered on every step: a pedal note). | `TYPE`: visual 2 = 8; `PEDAL`: 9 (R11). Moving it to K4 would push ACCNT off the SPICE slot, which ACID and EUCLID share. |
| **S3** SNAKE PATH on K4 | Keep on K4. LEN stays on K2 (FORM). | Both slots score 9 (labels are explicit). The slot rule is "K4 = the twist on top"; PATH re-orders the same notes and rhythm, which is that twist, while LEN is the loop length that every K2 is. Swapping would break "K2 = length or loop character" on 3 of 4 algorithms. |
| **S4** scale snap rule | Every note of every algorithm, **pins and live keys included**, is snapped at output by SP6: nearest scale tone by circular pitch-class distance, ties to the lower one, applied after span mapping (SP7). The unsnapped value (`pitch_orig`, a pin's `pin_note`) is kept, so changing the scale back restores the exact notes. `CHROM` passes notes through. | Scale or key change: heard on every channel from the next step, header shows it: output 3 (F18, F19). Snap-down (today's `t1_harmony_quantize`) moves a note up to 2 semitones on MINPEN versus 1 for nearest, and flattens low SPAN values (t1 risk 7). Pins entered with K3 move by scale degree, so they are in scale at entry anyway. |
| **S6** defaults | Confirmed: SCALE `MINPEN`, KEY C, OCTAVE 0; SNAKE `RANGE` 12. | Rule 5 / W5: PLAY on the defaults gives an in-scale, one-octave bass line on the first step (F25 9, F07 9). RANGE 24 spans two octaves, which on a TB-3 reads as a lead, not a bass (owner question 5). |

### 5.4 Algorithm doc edits that follow (applied by the doc-edit or P2-008 task, not here)

| File | Edit |
| --- | --- |
| sting.md 6, 6.1 | Knob slots K1 `DENS`, K2 `PEDAL` (rename of `TYPE`), K3 `JUMP`, K4 `ACCNT`; `den` range 0..16. Shared-layer column; ARTIC = GATE + SLIDE (one press); sub-labels `GT+SLD`, `OCTAVE`. Pins as `pin_note[16]` (ALGORITHM_SET 5.4). |
| t1.md 6, 6.1 | `hit` range 0..16 (0 = keys only). Sub-labels `ROTATE` (HITS), `RATCH` (ARTIC); SHAPE absent. |
| snake.md 6, 6.1 | Slots K1 `DENS` 0..16, K2 `LEN`, K3 `RANGE` default 12, K4 `PATH`. Sub-labels `GATES`, `VELO`, `DRIFT`; ARTIC absent. |
| ml185.md 6, 6.1 | Slots K1 `DENS` 0..8, K2 `TOTAL`, K3 `RANGE`, K4 `RATCH`. Sub-labels `RESTS`, `VELO`, `RT+HLD`, `LONG`. |
| all four 6.5 | Step entry gesture is now specified: REC tap, white key = step, hold + K3 = pitch by scale degree, hold + OCT = octave (this document W4). Pins are snapped at output (S4). |

## 6. Global scale and key (S5)

| Aspect | Specification |
| --- | --- |
| How many | One KEY, one SCALE and one OCTAVE for the whole device (owner decisions 3 and S5). No channel or algorithm owns a scale. |
| Where it is set | Hold SCL: any key (white or black) sets KEY to that key's pitch class; K1 KEY (C..B), K2 SCALE, K3 OCTAVE (-2..+2), K4 stays the morph. No menu. |
| Scale list (K2 order = `T1_SCALE_MASKS` index) | `CHROM MAJOR MINOR DORIAN PHRYG LYDIAN MIXO AEOL LOCRI MAJPEN MINPEN BLUES WHOLE HARMIN MELMIN DIM` (the stale `T1_SCALE_NAMES` order is deleted, ALGORITHM_SET section 9). |
| Defaults | KEY C, SCALE `MINPEN` (index 10), OCTAVE 0. |
| How each algorithm adheres | The pitch layer of every algorithm produces a semitone offset; SP9 maps it `midi = clamp(36 + KEY + 12*OCTAVE + offset, 0, 127)` and SP6 snaps it to SCALE (rule S4). ACID: motif and JUMP octaves; EUCLID: contour times SPAN; SNAKE and STAGE: RANGE span (SP7) then snap; pins: `pin_note`. Live keys in PERF use the same snap. |
| When it lands | The next step on every channel (all caches rebuild on a scale change, ALGORITHM_SET SP12). |
| Always on screen | Header of every screen except MENU: `C MINPEN` (plus `+1` when OCTAVE is not 0). The SCL layer shows the 12 pitch classes with the scale's notes lit; with MENU > SCALE LEDS on, the keys glow the scale. |
| Saved | In every preset (W11), with the channels. |

## 7. Flex points (ADR-0004 addendum)

### 7.1 Remappable by editing a table row

| Table | Row / field | What a row edit changes |
| --- | --- | --- |
| `control_map.tsv` (control x context) | action id, label | Any button, encoder, knob or key role in any context, for example swap FX and EDIT, move UNDO to OCT-, move mutes to other black keys. |
| Context table (1.2) | hold thresholds, enter/leave gesture | Make GLOB hold-only, change REC hold time. |
| Knob table (5.1) | slot, label, range, default, step | Remap K1..K4 per algorithm, rename `PEDAL`, change SNAKE RANGE default, STAGE TOTAL coarse step. |
| Dice table (5.2) | button -> layer, sub-label, seeds | Bind SLIDE alone to its own gesture (S1 fallback), rename a sub-label, mark a layer absent. |
| Global table (6) | scale list order, defaults | Default scale, key, octave; scale order on K2. |
| Black-key table | key -> MUTEn / CHn / ALL ON / RESTRT / TAP | Re-place channel keys if the channel count changes (owner question 31). |
| Defaults (section 2) | channel algorithms, MIDI channels, mutes | Power-on state. |

### 7.2 Not changeable without code

| Item | Why |
| --- | --- |
| The number of contexts and their precedence (1.2) | Input dispatch order in `ui_input()`. |
| Tap-on-release for buttons with a hold role | Gesture engine behaviour (`btn_hold`, `ui_layer.c`). |
| The five shared layers and their meanings | ALGORITHM_SET 10 (dice contract). |
| Snap rule SP6 and the output order (span, then snap) | Defines golden files. |
| Queued preset load at the loop start | Needs a scheduler hook in the ISR. |
| The main-screen bands (1.4) | Screen code; the contents of each band are table driven. |
