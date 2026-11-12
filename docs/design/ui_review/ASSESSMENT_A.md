<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Assessment A: design review of the FM-1 PERF screen (written before reading the measured evidence)

Method: the P4-011 renders in `build/ui_sq/` (perf, perf_playing, perf_row2, perf_snake_mute, page stubs), enlarged 3-4x with no smoothing so every pixel is visible, read against `SCREEN_SPEC.md` (zones, fonts: AF_S 12 px line / 9 px caps, AF_M 15/11, AF_L 28/21) and `PRODUCT.md` (register: product; principles: glance first, every control labelled where the eye is, hierarchy over density, colour never works alone, respect the fixed medium). Assessment B (measurements) was produced separately and not read until this file was committed.

## Overall impression

The information architecture is right and is the hard part: four channels, the lane, two knob rows, a 2x6 button picture, a black-key legend and a cue strip that rewrites itself. Every control has a place and a name, which is the project's first rule. The screen is a faithful, dense Torso T-1 / Elektron-style instrument page.

The visual design has not started yet. Everything is the same 12 px font in white or one of three greys on black, with a single orange dot. The only things that stand out are the selected channel cell and the ROW 1 strip, both big pale rectangles, and neither is the thing a player is watching. The live information (where the playhead is, what each knob is set to, what is muted) is as small and as quiet as the reference information (labels, hints, "PG 1/2 FOLLOW hold=EDIT"). That is the "busy spreadsheet" the product brief names as an anti-reference. The single biggest opportunity: give the live state visual rank (size, weight, colour) over the static labels, inside the three fixed font sizes.

## What is working

1. **Every control is named where the eye already is.** The band names the four knobs, the button picture names the 12 panel buttons and their hold roles, the cue strip names what to do next. This is the product's core promise and it holds.
2. **The cue strip.** Full-contrast, one line, and it changes with the situation (`PLAY: start K1-K4: morph` becomes `TAP STEP=ON/OFF HOLD=EDIT NOTE` while playing). It is the one element that already behaves like a good instrument display.
3. **Consistent two-line pattern.** Channel cells, knob cells and buttons all use "name over detail", so after a minute you know where to look.
4. **Selected and latched states are unmistakable** (white channel cell, light ROW 1 strip with the dim ROW 2 behind it), and they differ by fill, not only by colour.

## Priority issues

**[P1] The values you are changing are the same size as their labels.** In the knob band `DENS` and `12` are both 12 px, `SLIDE`/`6`, `JUMP`/`5`. A producer turning K1 is reading the number. Mid-performance, the number is the information and the label is a reminder. Fix: draw the active row's value in AF_M (11 px caps) with the label above it in AF_S, and compress the inactive row to a single AF_S line (`OFFSET 0  PEDAL 4  PUNCH 40  DIR 0`). The band keeps its height (about 27 px for the active row plus 12 for the ghost row), the value gains 22 percent in glyph height and a clear role, and the inactive row stops competing. Suggested command: `/impeccable typeset`, then `/impeccable layout`.

**[P1] The playhead and the step glyphs are the smallest things in the biggest zone.** The lane is 60 px tall and the step bars are bold, but the playhead is a thin white sliver at the bottom of one column, and the per-step marks (`!` accent, `~` slide, `x2` ratchet) are 1 to 2 px strokes except `x2`. The playing position is the single most important live fact on a sequencer. Fix: tint the whole playing column (or invert its background) and draw the accent, slide and ratchet marks as 5 to 7 px shapes (a filled triangle, a tilde bar, a doubled tick) in a strip of their own under the bars, so they read as a second row of data rather than noise. Suggested command: `/impeccable layout`, `/impeccable colorize`.

**[P1] The black-key legend is unreadable and the layout around it is cramped.** Seven of eleven keys show `M1 M2 M3 M4` and then `C… S… FIL R… P… P… R…`, so SOLO, ROLL, PAGE-, PAGE+ and RESTART are effectively unlabelled, on the keys that exist for live performance. The cells also butt directly against the cue strip with no gap. Fix: stop trying to spell words in a 20 px cell. Use a small set of 8-10 px drawn glyphs for the five page/solo/fill/roll/restart keys (a "S" in a solo box, `F`, `R`, left/right chevrons, a restart bar-arrow) plus `M1`..`M4` as text, and add a 2 px gap above the cue strip. Glyph shapes also satisfy "colour never works alone". Suggested command: `/impeccable clarify`, `/impeccable layout`.

**[P2] Muted channels vanish.** `3 SNAK MUTE -` and `4 STAG MUTE 4` are dark grey on near-black: the algorithm name, the one thing you need to decide whether to unmute, is the hardest to read. The second line also mixes meanings: `MUTE -` is a muted channel with MIDI off, `MUTE 4` is muted on MIDI 4, and `MIDI 1` is the live one. Fix: keep the algorithm name at normal contrast, mark mute with a small solid chip reading `MUTE` (shape plus word, so it survives colour-blindness and a bad viewing angle), and keep the MIDI channel on the second line in every state. Suggested command: `/impeccable colorize`, `/impeccable clarify`.

**[P2] The dice buttons say the same word twice.** `NOTES` over `NOTES`, `ACCNT` over `ACCNT`, `HITS` over `HITS`; `ALL`, `PLAY`, `REC` have no second line. Two lines of a 12 px font in each 36 px cell is a lot of ink for no information, and the words touch their neighbours (`NOTES|ACCNT`). Fix: line 1 is the layer name, line 2 is only the extra meaning (`OCTAVE`, `GT+SLD`, the hold role with its marker) or empty. Cryptic strings (`UNDO L`, `HLP/MN`, `GT+SLD`) get plain 6-character rewrites. Suggested command: `/impeccable clarify`.

**[P2] Colour carries almost no meaning.** One orange dot and white; everything else is grey. The six dice buttons act on six layers whose marks appear on the lane, but nothing connects them. If each layer had one hue used in both places (the ACCNT button and the `!` marks; the SHAPE button and the register bars; the ARTIC button and the `~` marks), pressing a button would visibly light the thing it changes, and the player could learn the screen by watching it. This is the largest learnability gain available and costs no pixels. It must be paired with shape or position (already true: the marks differ in glyph) so colour is a redundant cue. Suggested command: `/impeccable colorize`.

## Element by element

| Element | Function (what it does) | Purpose (why it is there) | Verdict | Proposal |
| --- | --- | --- | --- | --- |
| H1 transport glyph (`>` `.` `R`) | Shows play, stop, REC-armed | The first thing you check after pressing PLAY | Too small, 1 glyph, easy to miss | Draw as a filled shape 9 px tall (play triangle, stop square, REC circle in the REC colour); shape plus colour |
| H2 clock dot + `INT 120` | Tempo source and BPM, with a blinking clock dot | You must know whether the unit or the DAW is in charge | Right content, flat weight | BPM in AF_M, source word in AF_S beside it; dot keeps the blink and its colour code |
| H3 preset `P01` | Which preset slot is loaded; `?` pending, `>` loading | Safe live preset changes | Fine | Keep; give the pending state the accent colour |
| H4 `1 ACID M1` | Selected channel, algorithm, MIDI channel | Tells you what the knobs are acting on | Duplicates the selected channel cell | Keep; it is the one place the selection is stated in text for the knob band |
| H5 page map `1 [2] . .` | Which of up to 4 pages of 16 steps you are viewing | Pattern length beyond 16 steps | Clever and compact; the dots are 2 px | Keep as graphics; make absent pages a clear dot, present pages a box |
| H6 `L20` | Pattern length | Setup fact | Fine | Keep, dim |
| Z1 channel cell (name, algo, MIDI, mute, selected, activity dot) | Selects the channel the knobs act on; shows its algorithm, MIDI output and mute | The 4-channel overview | Strong layout; mute handling is the weak point; the activity square on the selected cell is nearly invisible black-on-white | See priority issue on muted channels; show activity as a bright 4 px square in every cell that just sent a note |
| Z2 lane header (`PG 1/2 FOLLOW hold=EDIT`, `>P1`) | Page, follow mode and a permanent hint | Context for the lane | A permanent hint in the dimmest grey wastes a line; the player already has the cue strip | Keep `PG 1/2 FOLLOW`; drop `hold=EDIT` (the cue strip says it) and use the spare width for the length or the playing page |
| Z2 lane step bars | Per step: hit or rest and the note's height | The pattern at a glance | The most successful element: bar height reads instantly | Keep; tint the playing column |
| Z2 step marks (`!` `~` `x2`) | Accent, slide, ratchet | What each hit does beyond pitch | Marks too small except `x2` | See priority issue; a mark row with 5-7 px glyphs |
| Z2 playhead | Where the pattern is now | The most important live fact | A thin sliver; weakest relative to importance | Whole-column tint |
| Z3 tag column (`ROW 1`, `ROW 2`) | Labels the OCT-/OCT+ buttons as the knob-row switch | Two rows of morphs on four knobs | Correct and the right shortening of the spec's `OCT-` over `h:peek`, which does not fit | Keep `ROW 1`/`ROW 2`; put the hold-to-peek in the cue strip while a row is latched |
| Z3 knob cells (label, value) | Name and value of each of the four knobs, two rows | What K1..K4 do right now | Value same size as label; inactive row competes | See priority issue |
| Z4 button grid (2x6: label, sub-label, hold marker, separators) | Names each panel button's tap role and hold role | Buttons change role with modifiers | Content is right; typography is cramped and duplicated; the hold marker (a 4 px down-triangle) is a good idea at the wrong size | Layer name once, sub-label only when it adds meaning; hold marker as a 6 px chevron; 1 px more gap between cells |
| Z5 black-key legend | Names the 11 black keys (mute 1-4, channel, solo, fill, roll, page, restart) | The performance keys | Unreadable for 7 of 11 | See priority issue |
| Z6 cue strip | One line of what to do now | Teaches the interface without a manual | Best element | Keep; give it a 2 px gap from the legend; the toast can reuse its colour treatment |
| Z7 margin | 4 px of nothing for the bezel | Overscan | Correct | Keep |
| Toast (`SAVED P05`) | Transient confirmation | Feedback for actions with nothing to hear | Crisp, full width | Keep; give destructive confirmations (`UNDO`, `RESET`) a different shape so they are not the same strip as `SAVED` |
| Stub pages (NOTE_EDIT, CHAN, SET, SCALE, REC, RESET, HELP, FIRST_RUN, ADD_PAGE, MOVE_HELD, ALGO_PICK, RIG) | The pages the producer will use for setup | Setup within two gestures | Currently the PERF layout with the band relabelled (the SET page shows `BPM SWING GATE DIV` with no values) | Judge once built; carry the same value-first band rule to every page |

## Heuristic scores (0-4)

| # | Heuristic | Score | Key issue |
| --- | --- | --- | --- |
| 1 | Visibility of system status | 3 | Mute, row and selection are clear; the playhead and clock state are small |
| 2 | Match between system and real world | 3 | Instrument vocabulary, but `UNDO L`, `HLP/MN`, `GT+SLD` need plain rewrites |
| 3 | User control and freedom | 3 | UNDO/RESET and the row latch are visible; no sign of redo on screen |
| 4 | Consistency and standards | 3 | Strong "name over detail" pattern; muted cells and the legend break it |
| 5 | Error prevention | 3 | Safe-by-default; pending preset load is marked |
| 6 | Recognition rather than recall | 3 | Every control labelled; seven legend keys are not |
| 7 | Flexibility and efficiency of use | 3 | Tap and hold roles on every button; speed is the point of the design |
| 8 | Aesthetic and minimalist design | 2 | Equal weight everywhere; duplicate labels; permanent hint line |
| 9 | Error recovery | 3 | Undo toasts are good |
| 10 | Help and documentation | 3 | Cue strip plus hold-HOME help are the right shape |
| **Total** | | **29/40** | Good; the weak area is visual hierarchy, not content |

## Cognitive load

Failures against the checklist: **visual hierarchy** (nothing outranks anything else except the two pale rectangles) and **minimal choices / chunking** (the PERF screen presents about 55 text items at once: 6 header, 8 channel, 2 lane header, 16 steps with marks, 10 band, 24 buttons, 11 legend, 1 cue). Working memory is spared because every control is labelled on screen, which is the right trade, but the eye has no entry point. Two failures: **moderate**. The fix is rank, not removal: only the cue strip, the playing column, the four knob values and the selected channel should be bright.

## Persona red flags

**Alex (power user, live):** A hand is on a knob and the other on a key; the eyes have a fraction of a second. The knob value is not larger than its label, the playhead is a sliver, and SOLO/ROLL/PAGE keys are `S… R… P…`. Alex will memorise the keys and stop reading the screen, which defeats the screen.

**Sam (low vision, a poor viewing angle, possible colour-blindness):** The muted channels are dark grey on black, the hold markers are 4 px triangles, and the layer colour idea must never be the only cue. Sam cannot tell a muted channel's algorithm and cannot see which of the dim row 2 labels are live.

**Jordan (first-timer):** Gets the cue strip and the labels, but `UNDO L`, `HLP/MN`, `GT+SLD` and `MUTE -` are guesses, and `NOTES` under `NOTES` suggests the labels are placeholders.

**Studio producer, hands busy (project-specific):** Reads from arm's length between two actions. Wants the same three facts every glance: playing or not, where in the pattern, what the four knobs are set to. Today the screen answers each of those correctly and equally quietly.

## Minor observations

- `MIDI 1` / `MIDI 2` on the live channels but `MUTE 4` on a muted one changes what the second line means from cell to cell.
- `PG 1/2 FOLLOW` and `>P1` repeat page information that the header's page map already shows.
- The black key cells use a mid grey box behind each label; the box edges compete with the text.
- `UNDO L` is likely "undo load"; `HLP/MN` is "help / menu".

## Questions to consider

- If the four knob values were big enough to read from across a table, would the labels need to be bright at all?
- Does the lane need a second row of marks, or is one row of bars plus a bright playhead enough on a 60 px strip?
- Should each of the six dice layers have a colour of its own, so that pressing a button lights the exact steps it changes?
- Which of the 55 text items would you drop if you could only keep 25?
