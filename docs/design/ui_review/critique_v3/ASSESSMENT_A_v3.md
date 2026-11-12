# Critique of proposal v3-glance: Assessment A (design director)

Scope: the glance screen (idle, playing, row 2, mutes, stress, stress row 2, mono fallbacks), the four toasts (algorithm,
preset pending, dice notes, undo) and the MAP shown while HOME is held. Judged against PRODUCT.md, the owner's glance brief
and the workflow acceptance test. Sources: the renders in `docs/design/ui_review/proposals/v3-glance/` (1x and 4x),
`proposal_v3.py`, `docs/controls.jpg`, `docs/design/button_modes.tsv`, `knob_map.tsv` and `key_map.tsv`. Contrast figures
are from `mock.py contrast --palette GREY`. NOTES.md, the other proposals and assessment B were not read.

Viewing geometry used throughout: 240 px = 30 mm (8 px/mm), read at 50 to 60 cm. AF_S capitals 9 px = 1.1 mm (about
7.6 arcmin), AF_M 11 px = 1.4 mm (9.6'), AF_L 21 px = 2.6 mm (18'). I treat about 10' as the floor for a glance read and
anything below 5' as invisible at arm's length.

---

## AI-slop / generic-hardware verdict

**Not slop, and mostly in category.** The screen is built from square tiles, a step lane, four aligned knob cells and a
thin transport header. That is the vocabulary of a Torso T-1, a Digitakt or a Push 2 display, not a phone app. Colour is
restrained (grey on near-black, plus three meaning-bearing accents: amber playhead, cyan accent, red REC). There are no
rounded cards, no glow and no gradients. A producer who knows Elektron or Torso would sit down and broadly trust it.

They would pause at five things, and each is a small piece of strangeness without purpose:

1. **The struck-through channel numeral** (mute) reads as "deleted" or "done", a to-do-list idiom. No sequencer in the
   reference set marks a mute that way. Elektron dims the track or inverts it, and Push dims the clip.
2. **The cyan full-height bar on step 16** (idle) reads as an edit cursor or selected step, not an accent. On first read I
   took it for a cursor. Elektron and Torso use highlight for selection, so accent as a whole bar in a second bright
   colour fights the convention.
3. **The light-grey toast slab with pixel-art die and undo arrow** pushes toward toy. It is the brightest object on the
   screen, brighter than the playhead.
4. **Three-letter enum codes forced into AF_L**: `P-P` (PEND), `UP` for forward playback, `RP+` (SAW+), `OCV`, `THB`. They
   read like a code dump, and `UP` changes the meaning (forward is not up).
5. **The ROW bar's `I` / `II` glyphs**, 3 px wide, look like rendering debris at 1x, not like a label.

Verdict: credible hardware UI with a handful of off-category idioms. Fix those five and it passes the category test.

---

## Overall impression

**What works:** the information the owner asked for is there and ranked roughly right. The four knob values are the
biggest live type on the screen. The selected channel's pattern fills the middle, and the amber full-height playhead
column with an inverted bar reads at any distance and survives the monochrome fallback. Mute state is carried three
ways (strike, M chip, outlined tile), so it does not rely on colour.

**What doesn't:** the glance screen documents only the knobs. Every button and key the acceptance test needs (the six
dice layers, undo, the four mute keys, SOL/FIL/ROL, the row buttons) is unlabelled while playing. The one hint
(`HOLD HOME = MAP`) disappears the moment PLAY is pressed, leaving a dead 20 px band. Dicing a layer, which is half of
"dial a sequence in", is a recall task against printed button names that mean something else (FX, SCL, ENV, LFO, ARP,
SEQ). The toasts confirm after the fact, and they don't name the button that was pressed, so they don't teach the mapping.
The high-stakes toasts (preset pending, undo) don't say what will happen next or how to get out. The MAP, the only
complete documentation, mislabels a control, is set entirely in AF_S, and is a 70-label chart rather than a glance.

**Single biggest opportunity:** use the empty bottom band while playing for a permanent dice/undo legend in panel order,
and make every toast lead with the physical button's printed name. That one change turns the glance screen from "the
knobs plus a hidden manual" into a screen that documents what every hand is about to do, which is PRODUCT.md principle 2.

---

## Design Health Score

| # | Heuristic | Score | Key issue |
|---|-----------|-------|-----------|
| 1 | Visibility of system status | 3 | Playhead, transport, mutes, activity and preset-pending are excellent. Invisible: MIDI channel of each channel, SOL/FIL/ROL while held, RESET latch, ROW 3, when a pending preset will load. |
| 2 | Match system / real world | 2 | Printed FX/SCL/ENV/LFO/ARP/SEQ mean ALL/NOTES/ACCNT/SHAPE/ARTIC/HITS with no on-screen bridge. `UP` for forward, `P-P`, `RP+`, `OCV`, `THB`, `SNK3`. The strike-through idiom means "deleted". |
| 3 | User control and freedom | 3 | Undo exists and is toasted, mutes toggle, OCT- is always the way home. The undo toast hides redo, and the preset toast hides cancel. |
| 4 | Consistency and standards | 2 | An outline frame means four things: muted tile, hot knob, MIDI OFF chip, HOLD role in the MAP. The activity square is amber on one tile and black on another. ARP K3 is renamed SPAN to OCTS, against the firmware table. The MAP says SAVE hold = BACK; the firmware says undo last load. |
| 5 | Error prevention | 2 | Unlabelled dice invite a wrong-layer re-roll mid-set. Row 2 looks identical to row 1 except a 10 px strip, so turning K1 for DENS can move OFFSET (mode error). The preset toast doesn't say the slot will LOAD on its own. |
| 6 | Recognition rather than recall | 2 | The knobs are fully recognisable (best part of the design). Six dice buttons, undo, eleven black keys and the off-row knob names are pure recall. |
| 7 | Flexibility and efficiency | 3 | Hold-HOME MAP, touch-for-help, OCT hold-to-peek and big values suit experts. Toasts cover the knob values right after a dice press, which is when an expert reaches for a knob. |
| 8 | Aesthetic and minimalist design | 3 | Clean, purposeful colour, a clear zone structure. Noise: a meter under every value duplicating the number, static AF_L numerals 1 to 4, and a dead band while playing. |
| 9 | Error recovery | 2 | `UNDO / DICE NOTES` doesn't say whether it was undone, on which channel, or that EDIT again redoes it. There's no visible recovery path for a pending preset. |
| 10 | Help and documentation | 2 | Touch-for-help while HOME is held is a genuinely good idea. The MAP that delivers it is AF_S throughout (7.6'), has one wrong label, omits ROW 3, and the only cue to it vanishes while playing. |
| **Total** | | **24/40** | **Acceptable**: a solid foundation, but significant improvements are needed before it passes the acceptance test |

---

## Cognitive load

| # | Checklist item | Result | Note |
|---|---|---|---|
| 1 | Single focus | Pass | Lane plus knobs is a clear primary task area. |
| 2 | Chunking (≤4 per group) | Pass (weak) | 4 channels, 4 knobs. The 16 steps are grouped into beats only by 6×3 px pips that vanish at distance. |
| 3 | Grouping | Pass | Header, channels, lane, knobs: four bands with clear separation. |
| 4 | Visual hierarchy | Pass (weak) | The playhead wins. Static numerals 1 to 4 in AF_L carry as much weight as live knob values, and the light toast slab outshines the playhead. |
| 5 | One thing at a time | Pass | Toasts are single-message. |
| 6 | Minimal choices (≤4 per decision) | **Fail** | Dice decision: 6 layer buttons + undo, unlabelled. Performance keys: 11 black-key roles, none shown. MAP: about 70 labels at once. |
| 7 | Working memory | **Fail** | You must remember FX=ALL, SCL=NOTES, ENV=ACCNT, LFO=SHAPE, ARP=ARTIC, SEQ=HITS, EDIT=UNDO, the M1 to M4/CHN/SOL/FIL/ROL/PG/RST keys, and the row-2 knob names while on row 1. |
| 8 | Progressive disclosure | Pass | MAP on hold, toasts on use, CHAN page for set-up. |

**2 failures = moderate load (address soon).** Both failures sit on the perform side of the acceptance test, not the set-up
side.

Decision points with more than 4 options: the dice layer (7 incl. undo), the black-key performance row (11), the knob rows
(8 parameters over 2 rows, 12 with ROW 3), the algorithm (6, handled well by the toast's position pips) and the MAP
(about 70).

---

## Emotional journey (a live run of the acceptance test)

| Moment | Feeling | Why |
|---|---|---|
| Power on, idle | Confident | Big tiles, big values, `HOLD HOME = MAP` offers a way in. |
| Select ch 1, set MIDI channel | Uncertain | The MIDI channel is set on the CHAN page, and the glance screen never shows it again. "Is the TB-3 really on 2?" has no on-screen answer. |
| Turn ALGORITHM | **Peak** | The toast is the best moment in the design: huge name, `CH 1 ALGORITHM`, six position pips. |
| Dial in with knobs | Good | The value updates in place in AF_L. |
| Dice a layer | **Valley** | "Which one is NOTES?" The panel says SCL. Press, and the toast says NOTES, after the re-roll has already happened. Reassurance arrives after the risky act. |
| PLAY | **Peak** | The amber column sweeps, activity squares blink. The hint disappears, though, and a dead band appears. |
| Mute (M1 to M4) | Good | The M chip and strike are immediate, though nothing on screen says which key mutes. |
| Switch channels | Good | The tile inverts and the lane redraws. |
| Undo | **Valley** | `UNDO / DICE NOTES`: undone, or offering to undo? Which channel? Can I get it back? This is a high-stakes moment and it gets the least clear copy. |
| Turn PRESETS mid-set | **Deepest valley** | `P04?` and an unlabelled bar shrinking. Nothing says "this will replace everything at the next bar" or "HOME cancels". |
| Stop | Flat end | The grey stop square and the returning hint. Fine, but the end of the peak-end pair is unremarkable. |

The peaks (algorithm toast, playhead) are real. The valleys cluster exactly where PRODUCT.md demands reassurance: dice,
undo, preset.

---

## What's working

1. **The playhead.** It is a full-height amber column (11.1:1 against black) with the step's bar drawn inverted inside
   it. At 14 px wide it's 1.75 mm, about 12', and it's the strongest object on the screen. In mono it becomes a light
   column with an inverted bar and still reads. That's the brief's "clear playhead", met without compromise.
2. **The knob cells.** Four 58 px columns sit under the four physical knobs, with the label in AF_M, the value in AF_L
   (18') and a range meter. The rule of one font per enum, so a cell never changes size while it turns, shows real
   craft. This is the one place the screen fully meets "what each control does right now".
3. **Redundant state encoding.** Mute uses strike + solid M chip + outlined tile. MIDI OFF uses a hollow OFF chip.
   Transport uses three different shapes (square, triangle, disc), not just colours. Accent uses cyan + triangle,
   ratchet uses split bars, and slide uses a tie bar that chains across steps. PRODUCT.md's "colour never works alone"
   was clearly designed for, and the mono renders prove it. The MAP's 2×2 encoder block and 2×6 button grid also mirror
   the physical panel (checked against `docs/controls.jpg`).

---

## Priority issues

### 1. [P1] The glance screen labels no button or key; the only hint vanishes on PLAY

- **What:** while playing, nothing on screen names the dice layers (FX ALL, SCL NOTES, ENV ACCNT, LFO SHAPE, ARP ARTIC,
  SEQ HITS), undo (EDIT), the mute keys, CHN, SOL/FIL/ROL or the row buttons. `draw_z4` returns early when playing, so
  `HOLD HOME = MAP` disappears too, and the 20 px band (Z4 + Z5, y 220 to 240) is left empty. The toasts confirm the
  layer after the press but never name the button, so `NOTES` never connects to `SCL`.
- **Why it matters:** PRODUCT.md principle 2 is "every control is labelled where the eye already is". The acceptance
  test's core loop is to dice rhythm, notes, accents, shape and articulation, and that loop is a six-item recall task
  against printed names that mean something else. A wrong guess re-rolls a layer in front of an audience. The owner's
  brief accepts toasts and MAP as documentation, but a toast that fires after the act and omits the button name is a
  receipt, not documentation.
- **Fix:** keep the Z4 band while playing and fill it with a fixed six-cell dice legend in panel order, AF_S on 40 px
  cells: `ALL NOTES ACCNT SHAPE ARTIC HITS` under printed-name micro-tags (or a 2×6 miniature of the button block with
  only the dice cells lit). Change the dice toast title to `SCL: NOTES` and the undo toast to `EDIT: UNDONE`. While
  stopped, cycle the band between the legend and `HOLD HOME = MAP`.
- **Suggested command:** /impeccable clarify

### 2. [P1] The high-stakes toasts don't state consequence or exit

- **What:** preset pending shows `P04?`, `SAVE = SAVE HERE` and an unlabelled shrinking bar. Per `knob_map.tsv`, the slot
  **loads by itself** (1.5 s after the last detent when stopped, at the next bar when playing), SAVE saves to it instead,
  and HOME tap or SAVE hold cancels. None of that is on screen. `SAVE = SAVE HERE` reads as a tautology. The undo toast
  `UNDO / DICE NOTES` doesn't say it has happened, which channel, or that EDIT again redoes it (`button_modes.tsv`).
- **Why it matters:** these are the two moments where a performer can lose a whole set (load) or needs reassurance that
  they got it back (undo). The countdown bar without a word creates dread rather than control.
- **Fix:** Preset: title `P04`, detail `LOADS AT BAR` (or `LOADS IN 1.5 S` when stopped), a third line
  `SAVE: KEEP HERE   HOME: CANCEL`, and the bar labelled by the detail line. Undo: title `UNDONE`, detail
  `CH1 DICE NOTES`, footer `EDIT AGAIN = REDO`, with a redo toast to match. Any knob turn should dismiss a toast at
  once, so values are never hidden while morphing.
- **Suggested command:** /impeccable clarify

### 3. [P1] The MAP mislabels a control, omits a row and is unreadable at arm's length

- **What:** `SAVE` hold is labelled `BACK`; `button_modes.tsv` says hold = undo the last preset load (UNDO L). `OCT+` shows
  only `ROW 2`, but OCT+ steps to ROW 3 (the RIG row) when a device is wired. Every one of about 70 labels is AF_S
  (1.1 mm, 7.6'). The black keys use `BLACK_X` gaps of 21 vs 23 px, so the 3-2-3-2-1 keybed grouping the code intends
  is imperceptible. Instead they're shaded by tap/hold, which is a second, competing encoding. K1 to K4 are drawn as a
  vertical list while the physical knobs are a horizontal row, which breaks the spatial mirroring the encoder block
  achieves. White-key numbers 10 to 16 touch edge to edge at 1x.
- **Why it matters:** the MAP is the screen's only complete documentation. One wrong label teaches the wrong control,
  and a performer holding HOME mid-set can't lean in to read 9 px type.
- **Fix:** correct `SAVE` hold to `UNDO LD` and add `ROW 3` (or `ROW 2/3`) to OCT+. Draw K1 to K4 as four horizontal
  cells top-right like the panel, with the row-1 name in AF_M and the row-2 name in AF_S. Widen the black-key group gaps
  to at least 6 px so 3-2-3-2-1 is visible, and drop the tap/hold shading in favour of the existing hold box. Promote the
  six dice labels in strip B to AF_M (the cells are 38 px: `NOTES`, `ACCNT`, `SHAPE`, `ARTIC` need checking with
  `mock.py measure`, and the 6-character rule allows rewriting). Consider splitting into two maps (buttons; keys) toggled
  by OCT while HOME is held.
- **Suggested command:** /impeccable layout

### 4. [P2] The knob row is a weak mode signal; the hot frame is unexplained

- **What:** the only difference between ROW 1 and ROW 2, apart from the labels, is which half of a 10 px strip is filled.
  Its `I`/`II` glyphs are 3×6 px (0.4 × 0.75 mm, under 3'). Two halves can't express ROW 3. A white double frame marks the
  last-turned ("hot") knob (PUNCH in row 2, TOTAL in stress). It is undocumented and looks like a selection cursor,
  which suggests the knob is selected for something.
- **Why it matters:** this is a textbook mode error. A hands-busy performer turns K1 expecting DENS and moves OFFSET.
  PRODUCT.md principle 3 says the current knob row must outrank reference information. Here it ranks below everything.
- **Fix:** put the row on the knob cells themselves. On ROW 2, draw the four label bars inverted (light label band, dark
  text) so the whole knob zone changes character, and write `ROW 2` / `ROW 3` in AF_M in the strip instead of the
  glyphs. Show the off-row's four names in AF_S in the strip (`OCT+: OFFSET PEDAL PUNCH DIR`) so a peek isn't needed.
  Replace the hot frame with a brief value flash, or drop it: the value already updates in AF_L.
- **Suggested command:** /impeccable bolder

### 5. [P2] Weight goes to static information while a needed fact is missing

- **What:** the channel numerals 1 to 4 are AF_L, the same weight as the live knob values, yet they never change. The
  algorithm name, which does change, is AF_M. The MIDI channel each lane sends on, the first fact in the acceptance test
  ("set its MIDI channel"; TB-3 on 2), appears nowhere on the glance screen. The `OFF` chip is 29 px wide and the AF_M
  word clips its right frame at 1x and 4x.
- **Why it matters:** hierarchy over density. The channel strip spends its biggest type on ordinal positions, while the
  routing a producer most needs to trust is invisible. A mis-set MIDI channel is the commonest "why is my synth silent"
  failure.
- **Fix:** drop the numerals to AF_M and add the MIDI channel as `→2` (AF_M) top-right of each tile where the activity
  square sits. Move activity to a 4 px bottom edge bar in amber, which also unifies the amber-vs-black inconsistency.
  Keep `OFF` but give the chip 34 px, or use `--` in the MIDI slot.
- **Suggested command:** /impeccable layout

---

## Persona red flags

**Alex (power user, knows Elektron/Torso).**
- Has to hold HOME to recall what row 2 holds; the off-row's names and values are nowhere on the glance screen.
- Every dice press puts an 88 px toast over the knob cells and the row strip, exactly when Alex reaches for a knob to
  follow up the re-roll. Nothing in the proposal says a knob turn dismisses it.
- Enum codes `P-P`, `RP+`, `OCV`, `THB`, `SNK3` must be learned as a private language, and `UP` contradicts the firmware's
  `FWD`.
- No quick-cancel is shown for a pending preset.

**Sam (low vision, poor viewing angle, colour-blind).**
- The whole MAP is AF_S, 7.6', below a glance threshold.
- A rest (MID dash, 12×4 px at the baseline) and a quiet note (TEXT bar, 12×4 px at the baseline) differ only by
  brightness. That's a colour-only state in the lane.
- The accent bar is cyan on the same luminance as a normal bar (#52caff vs #cdcacd, 1.15:1). For deuteranopes only the
  11×6 px triangle (1.4 × 0.75 mm) carries accent, and at an angle the cyan desaturates toward grey.
- Muted lane bars are 1 px hollow outlines at MID on LANE (3.75:1), which disappear at an angle.
- At an angle, the unmuted tile fill (RAISE #393839) and the muted tile (BG #080808 + outline) converge; only the M chip
  and the strike remain.
- The playing-page marker is a 4×4 px dot (0.5 mm, about 3.4'). The ROW glyphs are 3 px wide. Beat pips are 6×3 px.

**Jordan (first-timer).**
- Reads `FX` on the button and expects effects; it re-rolls everything. Same for SCL, ENV, LFO, ARP and SEQ.
- Presses PLAY and the only hint (`HOLD HOME = MAP`) disappears.
- Reads the cyan step 16 as "selected step" and the hot frame on PUNCH as "selected knob". Neither is true.
- Reads the struck-through `3` as "channel deleted"; `OFF` and `M` together on one tile are unexplained.
- Jargon with no inline gloss: DENS, ACCNT, ARTIC, PEDAL, PUNCH, JUMP, RDIV, GLEN, CTONE, PATT.
- `SAVE = SAVE HERE` while a number counts down: does not know whether to press anything.

**"Dana", studio producer, hands busy** (derived from PRODUCT.md: arm's length, glancing between actions, one TB-3 on
MIDI channel 2).
- Can't confirm on the main screen that channel 1 is sending on MIDI 2. Silence from the TB-3 has no on-screen
  explanation.
- Reaching for the dice with eyes on the screen: the screen doesn't say which of the twelve buttons is NOTES, so the eyes
  must leave the screen for the panel, whose print says SCL.
- Holding SOL, FIL or ROL: the screen shows no change (no solo indication on the other tiles, no fill marker on DENS, no
  roll marker in the lane), so a held key has no confirmation.
- After bumping OCT+, the knob zone looks almost unchanged; the next turn moves the wrong parameter.
- After a dice, the knob values are covered for the toast's lifetime, right when the next morph is due.

---

## Viewing-distance test (30 mm screen, 50 to 60 cm)

| Element | Size | Angle (approx.) | Result |
|---|---|---|---|
| Knob values, channel numerals, toast titles (AF_L) | 21 px, 2.6 mm | 18' | Pass |
| Playhead column | 14 px wide, full lane height | 12' wide | Pass |
| Activity square, M chip, transport shape | 14 to 18 px | 12' to 15' | Pass |
| Knob labels, algorithm names, BPM, preset, toast details, footer (AF_M) | 11 px, 1.4 mm | 9.6' | Marginal pass (a second look, not a glance) |
| Strike through numeral | 20×3 px | 3 px = 2.2' thick | Marginal: the shape reads, the stroke is thin |
| Accent triangle | 11×6 px | 4' tall | Fails as the sole cue (mono, colour-blind) |
| Slide tie bar | 15×6 px | 4' tall | Marginal pass (contrasting, wide) |
| ROW strip half-fill | 118×10 px | 7' tall | Pass for "which half"; the `I`/`II` glyphs fail |
| Page boxes / playing-page dot | 14 px / 4 px | 10' / 3' | Boxes pass, dot fails |
| Rest dash vs quiet note | 12×4 px each | 3' | Fail (brightness only) |
| Ratchet sub-bars x4 | 2 px wide | 1.4' | Fail to count (x2 and x3 pass) |
| Muted hollow bars | 1 px outline, 3.75:1 | under 1' stroke | Fail (intentionally dim, but too dim to track the pattern while muted) |
| Beat pips | 6×3 px | 2' | Fail (beats are not legible) |
| `OFF` chip text | AF_M, clipped | 9.6' | Fail (clipped) |
| Entire MAP (AF_S) | 9 px, 1.1 mm | 7.6' | Fail at arm's length |
| PATH ruler (14 pips, 1 px gaps) | 2 px pips | 1.4' | Fail |

---

## Element verdicts

| Element | Function | Verdict | One-line fix |
|---|---|---|---|
| Transport shape | Stop square / play triangle / REC disc | Keep | None: shape + colour, survives mono. |
| BPM / `EXT 126` | Tempo and clock source | Keep | None. |
| Preset `Pnn` / `Pnn?` | Current slot, pending flag | Keep | Pair `?` with the toast copy fix (issue 2). |
| Page boxes | Pages that exist, viewed page | Keep | Enlarge the playing-page dot to 6×6 or invert the whole box. |
| Channel tile, selected (inverted) | Which channel the knobs and lane belong to | Keep | None: the strongest selection cue on screen. |
| Channel numeral (AF_L) | Ordinal 1 to 4 | Demote | AF_M; give the space to the MIDI channel. |
| Algorithm name (AF_M) | Each channel's algorithm | Keep | `PATT` is fine; keep the 6-character rule. |
| Activity square | Channel just sounded | Rework | One colour (amber edge bar) on every tile, including the selected one. |
| Mute strike-through | Mute, shape cue | Replace | Use a dimmed numeral with the solid M chip; drop the "deleted" idiom. |
| M chip | Mute | Keep | None. |
| OFF chip | MIDI OFF | Fix | Widen to 34 px or show `--` in a MIDI slot; the text clips today. |
| Muted tile outline | Mute, tile cue | Keep | Don't reuse outline for other meanings. |
| Lane bars (height = velocity) | Pattern of the selected channel | Keep | None. |
| Playhead column | Current step | Keep | None: the design's best element. |
| Accent (cyan bar + triangle) | Accented step | Rework | Keep the triangle and make it 13×8; drop the cyan bar (reads as a cursor) or make it a notch, not a whole bar. |
| Slide tie bar | Slide / tie | Keep | None. |
| Ratchet split bars | Ratchet count | Keep | Cap the drawn splits at 3, and write `x4` above the bar for 4. |
| Rest dash | Rest step | Fix | Draw rests as an empty column with no dash (absence = rest), or a hollow dash. |
| Beat pips | Beat grouping | Rework | Alternate LANE shading per beat (steps 1 to 4, 9 to 12) instead of 6×3 pips. |
| Muted-lane hollow bars | Selected channel muted | Adjust | 2 px outline, or a solid DIM fill. |
| Knob label (AF_M) | Parameter of K1 to K4 | Keep | Invert the label band on ROW 2 (issue 4). |
| Knob value (AF_L) | Value | Keep | None. |
| Knob meter / pips | Position in range | Keep (quieter) | Thinner (6 px); the 14-pip PATH ruler becomes a plain bar. |
| Enum abbreviations | Enum values in AF_L | Fix | `FWD`/`BWD` as drawn arrows; no `UP` for forward, no `P-P`/`RP+`/`OCV`/`THB`. |
| Hot knob frame | Last-turned knob | Remove | A value flash or nothing; the frame reads as selection. |
| ROW strip | Live knob row | Rework | Write `ROW 2` in AF_M, show the off-row names, support ROW 3. |
| Footer `HOLD HOME = MAP` | Discoverability | Extend | Keep it while playing as part of a dice legend band (issue 1). |
| Empty band while playing | None | Use | Dice/undo legend in panel order. |
| Toast slab (replaces knobs) | Feedback after an action | Adjust | Dismiss on any knob turn; title leads with the printed button name. |
| Algorithm toast | Algorithm chosen | Keep | Best toast; optionally label the end pips. |
| Preset-pending toast | Slot pending load | Fix | `P04 / LOADS AT BAR / SAVE: KEEP HERE  HOME: CANCEL`. |
| Dice toast | Layer re-rolled | Fix | `SCL: NOTES / CH1 NEW PITCHES`; the die icon is fine. |
| Undo toast | Undo happened | Fix | `UNDONE / CH1 DICE NOTES / EDIT AGAIN = REDO`; redraw the arrow. |
| MAP title | Hold-HOME actions | Keep | None: `TOUCH = HELP  LET GO = MENU` is precise. |
| MAP encoder 2×2 | Encoder roles | Keep | Mirrors the panel; good. |
| MAP OCT cells | Row buttons | Fix | Add ROW 3 to OCT+. |
| MAP K1 to K4 list | Knob roles, both rows | Rework | Four horizontal cells like the physical knob row. |
| MAP `= HOLD` legend | Explains the boxed role | Keep | None. |
| MAP 2×6 buttons | Tap and hold roles | Fix | `SAVE` hold = `UNDO LD`, not `BACK`; dice labels larger. |
| MAP black keys + leader lines | Performance key roles | Rework | Visible 3-2-3-2-1 gaps; one encoding for hold. |
| MAP caption | White-key gesture | Keep | None. |
| MAP white keys 1 to 16 | Step keys | Adjust | One AF_S digit set that doesn't touch at 10 to 16, or label only 1, 5, 9, 13. |

---

## Minor observations

- STAGE K2 `TOTAL` drops the firmware's `(n)` count of steps heard (`knob_map.tsv`); `TOTAL 64` alone hides how much is
  audible.
- ARP K3 is renamed `OCTS` (from `SPAN`), so the screen, the MAP and the firmware label table now disagree.
- The SNAKE `PATH` names (`SNK3`, `SPI2`...) are invented, per the source comment; they are also the only knob value in
  AF_M on ROW 1, so that cell looks different from its neighbours.
- `KEYS ONLY` is drawn as `KEY`, singular, while the firmware cell reads `KEYS`.
- In stress, the hot frame on `TOTAL` merges into the ROW strip at 1x, producing a notch that looks like a tab.
- The toast slab (SEL #bdbabd) is the brightest object on screen and outranks the playhead for its lifetime. Use SURF
  with a TEXT frame, or shorten it to the knob zone only.
- The undo arrow pixel art reads as a bent pipe at 1x. The die is fine.
- There's no indication anywhere for SOL, FIL or ROL while held, RESET latched (EDIT hold), FOLLOW off / viewing a
  non-playing page, the ADD PAGE prompt, or steps past LENGTH. None are drawn in the proposal's fixtures.
- With 4 pages, `EXT 126  P16` and four boxes leave about 20 px of air in the header. It fits, with no room for a
  `REC` word if wanted.

## Questions to consider

1. What if the bottom band were never empty: a dice legend while playing, the MAP cue while stopped?
2. Do the numerals 1 to 4 need 21 px when they never change? What if each tile said `1 ACID →2`?
3. Should a dice press toast at all? The lane already redraws the new pitches. A one-line `SCL: NOTES` in the bottom band
   would confirm without covering the knobs.
4. What should the screen do while SOL, FIL or ROL is held? Those are the most performative gestures on the device, and
   currently they are invisible.
5. Is the MAP one screen or two? Buttons and encoders on one, keys on another (switched by OCT while HOME is held), each
   in AF_M, may beat one 70-label chart in AF_S.
6. Would the owner accept an accent shown as shape only (a taller notch or a filled triangle cap), freeing the second
   colour for "edited step", which the lane currently cannot show at all?
