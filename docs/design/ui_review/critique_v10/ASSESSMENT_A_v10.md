# Assessment A: main screen proposal v10

Reviewer: independent design director (no designer notes or earlier reviews read). I scored what is drawn in
`proposals/v10/out/` and read the code only to work out what a mark means. All images are at 1x (240x240, 30 mm,
8 px/mm) unless I say 4x. Viewing assumption: about 35 cm, hands busy.

## 1. Generic-look verdict

**Not generic.** This looks like an instrument screen, not a phone app or a stock widget kit. Square tiles, no shadows or
glow, and a bitmap type hierarchy (AF_L values, AF_M labels) in the Elektron/T-1 spirit. The two parts that are clearly
its own are the knob tiles that act as their own meter, and a footer that draws the keybed. The one generic note is the
2x6 icon grid: a toolbar of framed pictograms in grey cells. It is legible, but the four "layer-in-a-die" glyphs read as
generic stock icons rather than as dice.

## 2. Nielsen heuristics (0-4)

| # | Heuristic | Score | Evidence |
|---|---|---|---|
| 1 | Visibility of system status | 3 | `playing`: transport, BPM, channel chips (mute = slash, MIDI OFF = red cap), algorithm, MIDI, preset, row and page are all shown at once. `pending_p02`: an inverted `P02?` chip. `turning_k2`: the touched tile gets an outline. Loses a point because every toast or hold hint (`dice`, `turn_*`, `hold_*`, `oct_row2`) **replaces the whole top bar**: transport, BPM, chips and the MIDI field disappear exactly while the performer is acting. Also, a 2-page pattern shows the viewed page but not which page is *playing*. |
| 2 | Match with the real world | 3 | The buttons are laid out like the panel's 2x6 block, and the footer is drawn as a keyboard with the 3-2-3-2-1 black-key grouping. Play, rec, home, save, undo and gear are conventional icons. Minus one: the screen names `OCT+` and `ALGO` (toasts) and `CHN` (`midi_off` toast) by role words that are not the silkscreen on the unit (brief: the black keys print `OP1..`/`PIT`/`GLD`...). |
| 3 | User control and freedom | 2 | Undo is one tap away (EDIT, undo icon), and the `dice` toast teaches `EDIT=UNDO`. But EDIT toggles undo and redo, and **nothing shows which one the next tap will do**. Hold actions show a fill bar (`hold_edit`), which is good, but cancelling (let go early) is never stated. Pending-preset cancel (SAVE hold) is not cued on `pending_p02`. |
| 4 | Consistency and standards | 3 | One tile anatomy for every knob. Channel colour is used consistently in the chips, lane, tiles and keys. Inconsistencies: values switch between AF_L and AF_M from tile to tile (`sheet_row1_a` STAGE TOTAL `16` is AF_M next to AF_L `6` and `2`; `demo_algo` `2 OCT`/`MIN`). Two look-alike square widgets sit on the same status line with different meanings (row and page, see P1-2). The `+` add-slot appears in the *middle* of the chip row when CH3 is unused (`turning_k2`, `pending_p02`), which contradicts "SELECT past the last adds one". |
| 5 | Error prevention | 3 | A hold names its action before it fires (`hold_edit` "HOLD EDIT: RESET", `hold_save` "UNDO LOAD"), with a progress fill. A dice layer an algorithm lacks is greyed (`demo_algo` ACCNT). Algorithm and preset changes are quantised and say so ("AT NEXT STEP", "AT NEXT BAR"). |
| 6 | Recognition rather than recall | 2 | The weakest area. Nothing at rest ties SELECT, PRESETS, ALGORITHM or OCT to anything on screen (owner decision, accepted, but it still costs here). The ARTIC (ARP) and HITS (SEQ) dice glyphs, a framed dot-bar and a framed dash-row, cannot be read without pressing them. Black keys `S F R` are single letters, and the CHN key is a 7 px DIN-like glyph that looks like a chain or an infinity sign. The hit strips have no channel number: you have to remember which order they are in. |
| 7 | Flexibility and efficiency | 3 | All performance actions take one gesture from the main screen. Hold-HOME-and-touch gives in-place help for experts and novices alike. The RIG row is only offered when a rig exists. |
| 8 | Aesthetic and minimalist design | 3 | The hierarchy is good: values are the biggest thing, the lane is the biggest area, chrome is grey. The permanent footer line `KEYS = STEPS / HOLD = EDIT` uses 16 px for a fact the first press teaches. The keybed's step colours repeat the lane one-to-one (same on/off for the selected channel). |
| 9 | Help users recover from errors | 3 | `midi_off`: names the problem in words, with a fix (`CHN=SET`). `midi_off_after`: a persistent red `MIDI OFF` chip plus red hollow steps. A muted selected channel draws hollow bars (`sel_muted`). |
| 10 | Help and documentation | 2 | `hold_home` promises `TOUCH=HELP`, but no help scene is drawn, so I cannot judge the help box. There is no first-run cue, although page_states says FIRST_RUN step 1 is the MIDI channel. |
| | **Total** | **27 / 40** | |

## 3. Cognitive load checklist

| Check | Verdict |
|---|---|
| One dominant focal point | Pass: the four AF_L values, then the lane. |
| Chunks <= 7 per zone | Pass: header 5 chunks, 4 tiles, 12 buttons (in 2 groups of dice and non-dice by cell tint), 11 black keys (grouped as on the keybed). |
| Meaning never by colour alone | Partial: mute = slash or X (good) and MIDI OFF = text (good), but the lane's MIDI-OFF state is red *outline vs grey outline*, and hit-strip identity is colour plus position only. |
| Labels where the eye is | Pass for the knobs. Fail for the left encoders and OCT (only while touched). |
| No unexplained marks | Fail: the row squares (centre) and page squares (right) on the `P01` line, the CHN glyph, `S F R`, and the ARTIC and HITS glyphs. |
| State persists through actions | Partial: toasts hide the top bar while you act. |
| Learnable by first press | Pass: every touch names itself in the top bar. This is the design's main teaching device, and it works. |
| No mode confusion | Partial: on ROW 2 the only persistent cue is a 7x7 px filled square. The labels do change, so the knobs are never wrong, but the user cannot tell which "row" they are on. |

**Density verdict:** right for the glance zones (header, tiles, lane). Slightly over-dense in the bottom 60 px, where 12
icon cells, 27 key cells and a text line compete. Most of them are things the hands already know by position.
Acceptable, but the footer text line is the cheapest place to win back space (see P2-5).

## 4. Workflow test (screen alone)

| Step | Result | Why |
|---|---|---|
| Select CH1 and set its MIDI channel | **Fail / partial** | SELECT is unnamed until turned, then the toast (`turn_select`) confirms it. The header shows `MIDI 2`, but nothing says *how* to change it. The only route is the CHN black key, drawn as a 7 px glyph, and the only text cue (`CHN=SET`) appears only *after* a conflict (`midi_off`). The CHAN page itself is not drawn. |
| Pick an algorithm | Partial | The header shows the current algorithm in channel colour, but no on-screen element points to the ALGORITHM encoder. Once turned, the toast shows only the new name ("SNAKE AT NEXT STEP"), not the list of six, so the user cannot see what else exists or how far away it is. |
| Dial in with knobs and dice | Pass / partial | The knobs pass: labelled, metered, and outlined when turned. For the dice, ALL, NOTES and ACCNT are readable. SHAPE is guessable. ARTIC and HITS are unreadable until pressed, but the toast names them and `EDIT=UNDO` makes trying them safe. |
| PLAY | Pass | Play icon on the PLAY cell, and the transport mark in the header. |
| Morph | Pass | |
| Dice | Pass | The diced steps are hatched in the lane (`dice`), which is excellent feedback. |
| Mute | Pass | Mute keys drawn in channel colour; a muted channel gets X on the key and a slash on the chip. |
| Switch channel | Partial | The chips show the selection, but SELECT is not named at rest. |
| Undo | Pass / partial | The icon is clear. Whether the next tap is undo or redo is not shown. |
| Change preset | Partial | `P01` is visible and `P02?` pending is clear, but PRESETS is not named at rest, and cancelling a pending load is not cued. |

## 5. Element verdicts

- **Header (status bar):** strong. The chips carry four independent states (in use, selected, muted, MIDI OFF) without
  colour-only coding. Weak points: the mid-row `+`, and the whole bar being lost under toasts.
- **Top-bar toasts and hold hints:** the best idea on the screen. Every touch explains itself in AF_M with the
  printed-name chip. Holds show their fill. It should not wipe the transport and chips, though.
- **Knob tiles:** very good. The values are glanceable at 1x, the meter is a filled tint plus bright edge ticks, value
  lists get position dots, and the turned tile gets an outline. Issues: a 0 value shows no meter at all (EUCLID OFFSET,
  ARP OFFSET), so the tile looks like a list tile; AF_L/AF_M size switching; and RIG tiles labelled `WIRE1..4` rather
  than the wired parameter.
- **Status line (`P01` + squares):** the weakest element. Two unlabelled sets of tiny squares mean different things.
- **Lane:** good. Pitch is drawn as bar height, accent as a white cap, slide as a bridge, the playhead as a column,
  muted or MIDI OFF as hollow, and dice as hatched. The hit strips are a clever way to keep the other channels in view,
  but their identity depends on order, and that order shifts when the selected channel changes.
- **Button grid:** the layout and dice/non-dice tinting are right. The icons are mixed: 8 are good and 2 are opaque.
- **Keybed footer:** the right concept, and REC mode (`rec_armed`, a plain keyboard plus `PLAY KEYS TO WRITE`) is
  excellent. In PERF mode the black-key labels are too cryptic (`S F R`, the CHN glyph), and the text line restates the
  obvious.

## 6. True-size readability (1x, about 35 cm)

- AF_L values (21 px, about 25 arcmin): read instantly.
- AF_M header and tile labels (11 px, about 13.7 arcmin): fine for a glance.
- AF_S `MIDI` and `S F R` (9 px, about 10.8 arcmin): at the glance floor. `S F R` measures 4.75:1 on the key grey, so it
  passes contrast but is only just legible.
- **Below the floor:** the row and page squares (7 px, about 8 arcmin, and they encode meaning by fill alone); the CHN
  glyph (about 7x5 px); the MIDI-OFF red cap on a chip (3 px tall); the hit strips (2-3 px), which read as "something
  there" but not reliably as which channel; and the slide bridge (2x3 px).
- Contrast is fine everywhere I sampled: labels about 6.8:1 and footer text about 9.7:1. White on the meter fill is about
  8.7:1.

## 7. Personas: one red flag each

- **First-timer:** "What changes ACID?" Nothing on screen points to the ALGORITHM encoder, and nothing points from
  `MIDI 2` to a control.
- **Live performer:** each dice or encoder touch replaces the top bar, so during a burst of performance gestures the
  transport, BPM, mute chips and MIDI-OFF warning are hidden for the whole burst.
- **Deuteranope:** CH1 orange (230,157,0) and CH3 yellow (`sel_muted`) collapse to similar yellows, and the MIDI-OFF red
  outlines sit next to orange strips (`midi_off`). The hit strips have no non-colour identity apart from their order,
  which shifts.
- **Dana (one TB-3 on MIDI ch 2):** her only setup step, setting CH1's MIDI channel, has no on-screen route until she
  has already caused a MIDI OFF conflict.

## 8. Extensibility verdict

**Mostly pass.** `algos_v10.py` keeps every algorithm-specific item in records that mirror AlgoDesc: tile labels,
ranges and value names, value lists turning into dots, dice greying from `sub = "-"`, and toast sub-labels. `demo_algo`
proves that a new algorithm reaches the header, tiles, dice and lane with no screen code. Two items escape the
descriptor and need a firmware change to stay generic:
1. `UI_EXTRA.value_fn`: PATTERNS `PICK` (`ACD 1`) and STAGE `TOTAL` (`16(32)`, which reads algorithm state) are
   per-algorithm *functions*. Add to AlgoDesc either a per-morph format enum (number / names table / tag+index) or a
   `fmt(const void *cache, morph, v, char out[8])` callback.
2. `lane: single|pitched` and `short`: these need an `AF_*` flag bit and either a `short[5]` or a rule that `name` is
   always <= 6 characters. The RIG row (`WIRE1..4`) is not algorithm-driven at all and should take its labels from the
   device descriptor.

## 9. Priority issues

| P | Image / region | Issue | Fix |
|---|---|---|---|
| **P0** | `playing`, `one_channel`: header `MIDI n` and footer black key D#4 | The acceptance test's only setup step (set CH1's MIDI channel) cannot be discovered from the screen: the CHN key is a 7 px glyph, and `CHN=SET` appears only after a conflict. | Give the D#4 key cell the word `MIDI` (AF_S, 4 characters fit 2 key widths) instead of the glyph. When a channel's MIDI is still at its factory default, or on FIRST_RUN, show the footer text line as `CHN KEY = MIDI CH` with an arrow under D#4. Draw the CHAN page as a scene. |
| P1 | `playing`, `sel_muted`: lane y≈130-140 | The hit strips are identified only by colour and order, and the order changes with the selection. | Pin each strip to a fixed row per channel number (CH1 always the top strip, with the selected channel's row left empty), or start each strip with a 1-digit AF_S tag in the left gutter. |
| P1 | all scenes: status line y 80-90, centre and right | The row squares and the page squares are look-alike, unlabelled and below the glance floor. | Write the row as `R1`/`R2` (AF_M) next to the tiles. Write pages as `1/2` digits, or draw page cells as 16-step bars, and mark the playing page as well as the viewed one. |
| P1 | all scenes: buttons ARP and SEQ (row 2, cols 3-4) | The ARTIC and HITS dice glyphs cannot be read. | Redraw them as musical marks inside the die frame: a staccato dot plus a tenuto line for ARTIC, a 4-onset rhythm `x.x.` for HITS. Or, under the owner's icon-only rule, put a 3-character AF_S sub-label (`ART`, `HIT`) in the cell's lower third. |
| P1 | `dice`, `turn_*`, `hold_*`: top bar | A toast hides transport, BPM, chips and the MIDI-OFF warning while the user is acting. | Keep the left 100 px (transport, BPM, chips) and draw the toast only over the algorithm/MIDI area (x ≥ 105). The MIDI-OFF chip should beat a toast, or pulse its chip red. |
| P1 | `sel_muted`, `midi_off`: palette | Deuteranope confusion between orange and yellow, and between red and orange. | Shift CH3 to a light lime/green-cyan, or keep yellow and move CH1 to a deeper red-orange. Make MIDI-OFF steps hollow plus a diagonal (a shape, not just red). |
| P2 | `turning_k2`, `pending_p02`: chip row | A `+` between CH2 and CH4 contradicts "past the last adds one". | Draw `+` only after the last used chip. An unused middle slot gets a dim dotted frame. |
| P2 | `turn_algo`: top bar | No list of the alternatives. | Toast `ACID EUCL [SNAK] STAG ARP PATT` (the page_states ALGO_PICK cue), with the current one inverted. |
| P2 | `sheet_row1_a` STAGE TOTAL, `demo_algo` SPRD/SCALE | Values switch between AF_L and AF_M per value. | Choose the size per *morph* from its widest possible value text (from the descriptor), so a tile never changes size while turning. |
| P2 | `sheet_row*` EUCLID/ARP OFFSET = 0 | At 0 the meter is invisible, so the tile looks like a list tile. | Always draw the two edge ticks at the floor, plus a 1 px floor line. |
| P2 | all: EDIT cell | Undo vs redo state is not shown. | Flip the icon to a redo arrow while the last action was an undo. |
| P2 | `row3_rig` | Tiles say `WIRE1..4`. | Use the device parameter names from the RIG wire descriptor (`CUTOFF`, `RESO`...). |
| P2 | all PERF scenes: footer line y 225-238 | `KEYS = STEPS / HOLD = EDIT` is permanent text that teaches only once. | Use the line as the context cue strip (first run, CHN, pending cancel `SAVE HOLD = CANCEL`), and fall back to the current text only when idle. |
| P3 | footer black keys F#4/G#4/A#4 | `S F R` letters are cryptic. | `SOLO`/`FILL`/`ROLL` will not fit, so use glyphs: a solo spotlight, a fill bar and a ratchet `|||`. |
| P3 | `turn_algo` | The tiles already show SNAKE morphs while the lane still plays ACID. | Outline the tiles as "pending" (dashed) until the switch lands at the next step. |

## 10. What works

- **Self-naming touch**: every encoder, OCT and hold names itself, with the printed word inverted, the moment it is
  used. Combined with the no-legend decision, this is the right way to make "learn by first press" safe.
- **Knob tiles as meters**: big values, a level tint, edge ticks, dots for lists, and an outline while turning. These
  are the most glanceable controls I have seen on a 30 mm screen.
- **Hatched dice feedback** in the lane (`dice`): you see exactly which steps the re-roll changed.
- **Status encoding without colour alone**: mute = slash or X, MIDI OFF = words plus a red chip, selection = a white
  underbar, pending = `P02?` inverted, muted or OFF steps = hollow.
- **The REC footer** turns into a plain keyboard with `PLAY KEYS TO WRITE / REC = DONE`. This is the clearest mode change
  on the device.
- **Extensibility**: `demo_algo` and the four sheets show a real descriptor-driven pipeline, including dice greying.

## 11. Owner questions (max 3)

1. Will you allow **one** persistent setup cue (the D#4 key cell reading `MIDI`, or a first-run footer line) as an
   exception to "no permanent legend"? Without it, the workflow test fails at step 1.
2. May a toast cover **only the right half** of the top bar, so that transport, BPM and channel chips never disappear
   while performing?
3. Should hit strips be **fixed per channel number** (stable rows, a gap for the selected channel) rather than packed in
   order? This trades 2-3 px of lane height for identity that never moves.
