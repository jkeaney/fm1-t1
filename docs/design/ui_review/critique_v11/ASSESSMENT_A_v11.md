# Assessment A: v11 main screen (independent review)

Scope: the PNGs in `docs/design/ui_review/proposals/v11/out/` (1x read as true pixels, _4x for detail), checked against
PRODUCT.md, HANDOFF 01/02, the design TSVs and `AlgoDesc`. Code read only to settle what a mark means.
Viewing model: 8 px/mm, 35 cm, so **1 px = 1.23 arcmin**: AF_S 9 px = 11 arcmin, AF_M 11 px = 13.5, AF_L 21 px = 26.
(02_CONSTRAINTS still says 50-60 cm. At that distance every figure below is about 35% smaller, which makes the P1
items worse.)

Bands in `playing` (y px): header 0-22 | knob tiles 23-76 | info row 78-92 | music row 93-142 | buttons 146-182 |
keybed 186-219 | footer 222-238.

**Score: 26 / 40.** No P0. Five P1s.

---

## 1. Generic-look verdict

**It mostly looks like an instrument and not like a template.** The flat tiles, the boxed channel numerals, the
Okabe-Ito colours used only where they mean something, and the keybed drawn as a keybed all look like a T-1 / Elektron
screen. One part looks generic: the 2x6 grid of grey tiles holding a gear, a house, a floppy disk and a play triangle
reads as a phone or web toolbar. The four knob tiles, with their big numerals on a coloured fill, also come close to a
"KPI card" dashboard. Neither part is decorative noise, but these are the two places a first look says "app" and not
"sequencer".

## 2. Nielsen heuristics (0-4)

| # | Heuristic | Score | Evidence |
|---|---|---|---|
| 1 | Visibility of system status | 3 | Good: play/rec glyph, BPM, channel chips (filled = selected, slash = muted, red cap = MIDI OFF), `P02?` stays pending, the RESET/UNDO LOAD/HARMONY toasts while held, the hatched diced steps (`dice`). Weak: the playhead is a 2.6:1 grey column (`playing`, step 6). The other channels' activity is 3 px strips. The knob-row state is two unlabelled chips mid-row. |
| 2 | Match with the real world | 3 | The button block mirrors the 2x6 panel and the footer mirrors the 16+11 keybed with 3-2-3-2-1 grouping. Accent `>` and drum-notation x heads are true music idioms. Gear and house are computer idioms. `~` for SHAPE and the dashes for ARTIC do not come from any music convention. |
| 3 | User control and freedom | 3 | The dice toast says `EDIT=UNDO` (`dice`). SAVE hold = `UNDO LOAD` (`hold_save`). A preset is pending before it loads (`pending_p02`). But no scene shows that undo is available or that the next tap is a REDO (EDIT toggles). |
| 4 | Consistency and standards | 2 | Boxed numerals mean three things: channels (header), knob rows (`1 2` in the info row) and, nearby, `PAGE 1/2`. A white 2 px cap means both "accent" and "playhead top" in the same row (`playing`, steps 6-8). The coloured fill is a meter, but full tiles (`oct_row2` PUNCH 40, `arp_row2` DYN 10) read as "selected", which competes with the orange outline that really means "being turned" (`turning_k2`). Numeric values switch between AF_L and AF_M from tile to tile (`stage_heard` TOTAL `48(32)` next to `6`). |
| 5 | Error prevention | 2 | Dice act on press, with icon-only labels told apart by 2 px interior detail. An algorithm change "AT STEP" is good. A MIDI collision turns the other channel OFF, and the screen says `TAKEN` but not who took it (`midi_off`). |
| 6 | Recognition rather than recall | 2 | Hold roles on EDIT/GLO/HOME/SAVE carry no mark at rest, so they must be remembered (owner-decided, but there is no affordance at all). The left encoders show only when touched (owner-decided). The `1 2` row chips have no label. |
| 7 | Flexibility and efficiency | 3 | Every performance action is one gesture. Row peek works via OCT. ROW 3 = RIG appears only when it exists (`row3_rig`). The pending preset loads on the bar. |
| 8 | Aesthetic and minimalist design | 3 | Clean and flat, with no glow. But 87 px (36%) is reference (buttons + keybed + footer) against 50 px (21%) for the music. Step on/off is drawn twice, in the lane and in the keybed white-key row. |
| 9 | Recognise, diagnose, recover from errors | 3 | MIDI OFF is handled well: red hollow bars, a persistent red `MIDI OFF` block in the header, and the `CHN KEY = MIDI` cue (`midi_off_after`). The diagnosis lacks the culprit channel. |
| 10 | Help and documentation | 2 | HOME hold = `HELP / MENU` exists but cannot be seen until held. The footer line is the only standing help. No first-run scene was drawn. |
| | **Total** | **26** | |

## 3. Cognitive-load checklist

| Check | Verdict |
|---|---|
| 1. One clear focal point while playing | Partial. The knob numerals (AF_L, white on fill) outshout the music row. The eye lands on `12 4 5 3`, not on the playhead. |
| 2. 7 or fewer chunks | Pass (borderline). There are exactly 7 bands. |
| 3. Recognition over recall | Partial. Tap roles have icons. Hold roles, row chips and dice interior detail must be learned. |
| 4. One encoding means one thing | Fail. Boxed numerals x3. White cap = accent and playhead. Fill = meter but also looks like selection. |
| 5. Progressive disclosure | Pass. Toasts and hold names appear only while a control is touched, and the toast slot is always the same place. |
| 6. No redundant data | Partial. Lane steps and keybed steps duplicate each other. `KEYS = STEPS HOLD = EDIT` is permanent text that never changes in PERF. |
| 7. Live state outranks reference | Fail. The live music row has 50 px and a dim playhead. Static control maps take 87 px. |
| 8. Feedback inside one glance | Pass. A press inverts the button and a toast appears in the top-right slot within the same frame region. |

**Density verdict:** the total amount is about right for 240 px, but the space is given to the wrong things. The
reference bands (buttons + keybed + footer, 36%) are larger than the live band (21%). The selected channel's tallest
bar is about 29 px (3.6 mm). Take height from the keybed's duplicated step row and the static footer and give it to the
music row and the playhead.

## 4. Workflow test (from the screen alone)

| Step | Result | Why |
|---|---|---|
| Select CH1 | Pass | SELECT turn → `SELECT CH2 EUCLID` toast, chip fills, colours re-key (`turn_select`). |
| Set its MIDI channel | **Partial** | The `CHN KEY = MIDI` cue appears only for a new channel or one with MIDI OFF (`channel_added`, `midi_off`). On a channel showing `MIDI 1` (any non-new channel), nothing links the header's MIDI field to the CHN key except a 7 px DIN glyph on a black key. The CHAN page itself was not drawn. |
| Pick an algorithm | Pass | ALGORITHM turn → `ALGO SNAKE AT STEP` (`turn_algo`). Minor: the toast does not show the list, so the user cannot see what comes next or how many there are. |
| Dial in with knobs | Pass | Labelled tiles, active tile outlined (`turning_k2`), row switch named (`oct_row2`). |
| Dice | Partial | Pressing names the layer (`dice`: `NOTES`). Before pressing, only ALL (die) and NOTES (note) are guessable. ACCNT `>` is guessable for musicians. SHAPE `~`, ARTIC (dashes) and HITS (x heads) are not, at 9 px. |
| PLAY | Pass | Play glyph, header play triangle. |
| Morph | Pass | |
| Dice (live) | Pass | Hatched new steps show what changed. |
| Mute | Partial | Black keys M1-M4 show channel swatches and a muted key shows `x`. The swatch alone does not say "mute". The state is clear (slashed chip, x, grey dashes), the action is not. |
| Switch channel | Partial | Nothing on the screen at rest says SELECT switches channels. The chips are clear once you know. |
| Undo | Pass | Undo arrow on EDIT, `EDIT=UNDO` in the dice toast. Redo is not visible. |
| Change preset | Pass | `P01` → `PRESETS P02 AT BAR` → `P02?` (`turn_presets`, `pending_p02`). |

## 5. Element verdicts

- **Header (chips + algo + MIDI):** strong. Filled = selected, slash = muted, red cap + red block = MIDI OFF, `+` =
  free slot. A `+` in the middle (`1 2 + 4`, `dice`) reads as "add" more than as "CH3 is free". Acceptable because
  SELECT past it adds. `MIDI` in AF_S next to an AF_M digit is fine.
- **Top-right toast slot:** one place for every transient, always inverted, always says what happens and when (`AT STEP`,
  `AT BAR`, `EDIT=UNDO`). This is the design's best idea. `TAKEN` needs a subject.
- **Knob tiles:** labels AF_M (8:1 contrast), values AF_L, one font per knob, value lists get dots (`DIR FWD`). The
  fill-as-meter is the weak point: high values look like highlight, and the meter edge runs through the numerals.
- **Info row:** `P01` is clear. The `1 2` row chips are unlabelled, centred far from the OCT buttons, and look like page
  or channel numbers. `PAGE 1/2` is fine.
- **Music row:** the encodings are good (height = pitch, bridge = slide, dash = rest, hollow = silent, red hollow =
  MIDI OFF, hatch = diced). The marks are too small for a glance: the slide bridge is 2x3 px, the accent cap 2 px and
  the other-channel strips 3 px. The playhead is dim.
- **Button block:** the panel order is right. The dice tiles are lighter grey (72) than the others (41), which is too
  subtle to read as a group at a glance. There are no hold marks.
- **Keybed footer:** the black-key row works (swatches, `x`, S F R, page arrows, restart). In REC it becomes a piano,
  which is an excellent mode change (`rec_armed`). The white-key row duplicates the lane.
- **Footer text:** good as a mode line (`PLAY KEYS TO WRITE / REC = DONE`). In PERF it never changes, so it is dead
  weight after day one. It goes inverted for the MIDI cue (`midi_off`), which is good.

## 6. True-size readability (35 cm)

| Element | Size | Arcmin | Verdict |
|---|---|---|---|
| Knob values (AF_L) | 21 px | 26 | Glance-proof |
| Knob labels, header algo, toasts, footer (AF_M) | 11 px | 13.5 | Good for position-identified labels |
| `MIDI`, `PAGE`, `TAKEN`, black-key S F R (AF_S) | 9 px | 11 | Marginal. `TAKEN` matters and is AF_S |
| Button icons | ~9-11 px outer, 2 px strokes | 11-13 outer / 2.5 detail | The outline reads, the interior (which die) does not at a glance |
| Other-channel hit strips | 3 px tall | 3.7 | **Below the ~5 arcmin floor** |
| Muted other-channel dash | 5x3 px grey | 3.7 | Effectively invisible |
| Accent cap / slide bridge | 2 px / 2x3 px | 2.5 | Second-look detail only |
| Playhead column | 15 px wide, grey 80 on black (2.6:1) | size fine | Contrast too low for the most important live mark |
| MIDI OFF chip cap | 3 px red | 3.7 | Rescued by the red header block |

## 7. Personas (one red flag each)

- **First-timer:** the `1 2` chips in the info row. Nothing says they are the knob row, and they sit next to channel
  chips and `PAGE 1/2` that use the same visual form.
- **Live performer:** after muting CH4 or dicing CH2, the only confirmation that the other channels are still playing
  or silent is a 3 px strip, or a 5x3 grey dash for a mute. It cannot be checked mid-set at arm's length.
- **Deuteranope:** CH3 bluish green and CH4 reddish purple simulate to (137,133,117) and (147,151,163), a 1.12:1
  luminance ratio, and both sit near the grey used for "muted". Fixed rows and chip numerals save identity. A selected
  CH3 (`sel_muted` fills, `stage`-style tiles) still reads as greyed out.
- **Dana (one TB-3, MIDI ch 2):** if her CH1 is not flagged "new" at power-on and defaults to MIDI 1, the screen shows
  `MIDI 1` and plays to the wrong channel. It gives no cue saying the CHN key fixes this. Step 1 of the acceptance
  test then depends on the 7 px DIN glyph.

## 8. Extensibility verdict

**Pass, with descriptor additions that should be named.** The screen code never names an algorithm. `demo_algo` shows
a new algorithm reaching every element: header name, four tile labels, value lists (`MIN` with dots), unit text
(`2 OCT`), a greyed absent dice layer (ACCNT), and the lane. Dice sub-labels and absent layers already map to
`AlgoDesc.sub` / `LayerDesc.n == 0`. What `AlgoDesc`/`MorphDesc` do **not** carry today and the mock relies on:
1. a short name (`PATTERNS` is 8 characters, while the field comment says 6 or fewer, and the mock uses `PATT`);
2. a lane mode (pitched / single), which could be a new `AF_SINGLE` flag;
3. value-name lists, a zero word (`KEYS`) and a unit suffix (`OCT`, `X`). `MorphDesc` is 12 B with none of these;
4. a per-algorithm value formatter: `UI_EXTRA` hard-codes `patt_pick` and `stage_total`. In firmware this needs a
   `fmt` function pointer in `AlgoDesc`. That is a small amount of per-algorithm screen code, kept in the descriptor.

The lane can only express pitched or single. A chordal or poly algorithm (ARP with `AF_CHORD`) has no way to show more
than one pitch per step. The `check()` width linter is a good guard and should become a host test.

## 9. Priority issues

**P0:** none. The main screen supports the whole workflow, with partial passes.

**P1**
1. **Other-channel strips below threshold.** `playing` / `turn_select`, music row y 128-141. 3 px strips and 5x3 px
   mute dashes come to 3.7 arcmin. *Fix:* allocate rows only for used channels, still in fixed CH order. Make strips
   5 px with a 1 px gap. Draw a muted channel as a full-width hollow 5 px outline, not a shorter grey dash. Take the
   height from the keybed white-key row (see P2-1).
2. **Boxed-numeral overload / unlabelled row chips.** `playing` info row x 105-130 (`1 2`); `oct_row2`; `row3_rig`.
   *Fix:* label them (`ROW 1 2` in AF_S + AF_M) and move them to the left end of the info row, nearest the OCT buttons.
   Better still, show the row as a tab on the tile label strip (for example a 2 px bar above the four labels with
   `R2` at its left).
3. **Weak playhead, and its cap clashes with the accent mark.** `playing` step 6, y 92-142. *Fix:* draw the playhead
   as a 3 px white bar under the lane baseline plus a lighter column (T_MID about 120, at least 3:1). Move accent to a
   different form, for example the top 3 px of the bar in white inside the bar, not a cap above it.
4. **MIDI setup path for an existing channel.** `one_channel` / `playing` header `MIDI 2`, keybed D#4. *Fix:* treat the
   factory CH1 as "new" until its MIDI field has been set once, so the `CHN KEY = MIDI` cue shows on first power-on.
   Also draw the same DIN glyph in front of the header's MIDI number so the header field and the key are visibly
   linked.
5. **Meter fill reads as selection.** `oct_row2` PUNCH, `arp_row2` DYN, `sheet_row2_a` (full tiles). *Fix:* keep the
   fill but cap it below the label strip at about 70% tile brightness. Add a 2 px bright level line at the fill top so
   it reads as a gauge. Keep the outline as the only "active" signal. Check it beside `turning_k2`.

**P2**
1. **Step data drawn twice.** Keybed white-key row y 205-219 against the lane. *Fix:* shrink white-key cells to a 6 px
   on/off row (the playhead underline stays) and give 8 px to the music row.
2. **Dice not readable as a group or individually.** Button row 1 x 0-160, row 2 x 80-160. *Fix:* make the dice
   background a hue-free but clearly distinct value (about 96), or add a common 2 px die-corner pip to all six, so "these
   six re-roll" reads as one group. Keep the press toast as the identifier.
3. **Hold roles have no affordance.** EDIT, GLO, HOME, SAVE. *Fix:* a 2x2 px corner notch on the four buttons that have
   a hold. It is not a legend, so it stays within the owner's rule.
4. **`TAKEN` without a subject.** `midi_off` toast. *Fix:* `MIDI 2 → CH1` or `CH1 HAS 2` (fits in AF_M), and make that
   text AF_M, not AF_S.
5. **Deuteranope CH3/CH4.** All channel uses. *Fix:* separate CH3 and CH4 by luminance (at least 1.5:1 between them,
   for example a darker green with a lighter pink). Make sure no channel colour sits within 1.3:1 of the muted grey.
6. **ALGO toast shows no position.** `turn_algo`. *Fix:* `◀SNAKE▶ 3/6`, or show neighbours in AF_S.

**P3**
1. `added_after`: the `CHN KEY = MIDI` cue stays after MIDI 10 was set. Drop it once the channel's MIDI is set.
2. `rec_armed`: `CURSOR` runs edge to edge in its tile, and `OFF OFF` in AF_L on adjacent tiles reads as one word.
   Give AF_L words a 3 px side margin, or use AF_M for words.
3. `stage_heard`: `48(32)` is cryptic. Use `48` plus a small "heard" meter or `32/48`.
4. PERF footer text never changes. Consider showing the black-key names there only while a black key is held.

## 10. What works

- The top-right toast slot: one place, always inverted, and it names the consequence and the timing (`AT STEP`,
  `AT BAR`, `EDIT=UNDO`, `UNDO LOAD`).
- The panel mirroring: the 2x6 button block and the 16+11 keybed in true grouping make "which control is this" a
  matter of position, not reading.
- The handling of MIDI OFF: red hollow bars, a persistent red header block, and the one standing cue.
- No state relies on colour alone: slash, hollow, `x`, hatch and red text each back up a colour.
- The keybed changes mode visibly in REC (a piano) and the footer names the exit (`REC = DONE`).
- Extensibility is real: the screen is driven by data, a greyed absent layer is shown, and `check()` lints every
  algorithm against the widths.

## 11. Owner questions (max 3)

1. At factory power-on, is CH1 "new" (so the `CHN KEY = MIDI` cue shows) or does it default to MIDI 1 with no cue?
   Dana's first minute depends on this.
2. Would a 2 px corner notch on the four buttons that have a hold count as a "permanent legend"? It conveys only "this
   has a hold", not what the hold does.
3. Are you willing to trade the keybed's white-key step row (a duplicate of the lane) for a taller music row and
   legible other-channel strips?
