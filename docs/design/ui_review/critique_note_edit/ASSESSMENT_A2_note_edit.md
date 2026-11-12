# Assessment A2: NOTE_EDIT page (independent design review)

Method: Impeccable critique, Assessment A only (design review; no detector run, no source or docs/design/ui_review/critique_note_edit read, no task logs). Evidence: build/ui_sq/note_open, note_set, note_dice, note_final, note_toast, note_buttons_1x/4x, plus perf_dice (PERF locked reference), PRODUCT.md, SCREEN_SPEC_V2 section 1, S03, SEQUENCER_UX F-12, button_modes.tsv, note_edit_text.tsv, P4-014 sections 1-3. Pixel measurements were taken from the PNGs with Pillow.

Geometry: 240 px over 30 mm = 0.125 mm/px. At 350 mm one px = 1.23 arcmin (at 400 mm, 1.07; at 300 mm, 1.43).

## Measured legibility (35 cm)

| Element | Pixels | Arcmin | Contrast | Verdict |
| --- | --- | --- | --- | --- |
| Big tile value (C2, 40, 43, 1) | ~20 px cap | ~25 | 6.75:1 grey (AUTO) / 14.7:1 white on amber fill (set) | Good, the page's real hero |
| Footer `KNOBS = STEP 9` / `HOME = DONE` | 11 px cap | ~13.5 | 20:1 left, ~9.7:1 right (dimmer grey) | OK |
| `STEP 9` status | 9 px cap | ~11 | ~20:1 | Marginal for the "which step am I on" answer |
| Chips `PROB 50`, `OCT-2`, `NO SLD` | 9 px cap on a 16 px chip | ~11 | black on amber, 9:1 | Marginal size, strong contrast |
| `AUTO` under tile | 9 px | ~11 | 6.75:1 | Marginal, it is the unset cue |
| `= SET` legend (grey on black, 106,101,106) | 9 px | ~11 | 3.68:1 | FAILS PRODUCT.md 4.5:1 body floor |
| `HOLD KEY = MOVE` | 9 px | ~11 | ~6.5:1 | Passes, but is absent in 3 of 5 renders |
| Layer icons | 13 px square, 1 px stroke | ~16 overall, stroke 1.2, bracket gaps ~3-4 | white on #484848, 2.3:1 non-text | Identity of the six depends on 3-4 arcmin gaps and 1.2 arcmin strokes |
| Set-tile marker square | ~4 px | ~5 | white on amber/grey | A small mark; carries the "set" cue only with the white/grey text change |

The project spec itself calls AF_S (~10.8 arcmin) acceptable only for short reference text. In NOTE_EDIT the answer to "which step, what did I change, how do I leave, how do I move" is all AF_S/9 px. Only the values are large.

## Anti-patterns verdict

Not AI slop. This is a pixel-font hardware screen with a strict token palette (Okabe-Ito, T_DICE neutral), no gradients, no cards, no glow. It reads as T-1/Elektron-style parametric UI, which PRODUCT.md asks for. The only "slop-adjacent" tells are generic: three-row stack of equal-weight bands (tiles, status line, graph, buttons, keys, footer) with no single focal point beyond the tile values, and 12 icon tiles of identical size competing at equal weight.

## Overall impression

The tile row is excellent: grey "AUTO" versus white set value is a real, colour-independent, instant answer to "did I change this?". The page falls down below the tiles. The six "this step" icons look like PERF's dice with the middles bitten out, the status line crams four jobs (step number, set-chips, the SET legend, the move hint) into 9 px, and the HOME/MOVE cues vanish exactly when the user has done something (the next thing they need). Biggest opportunity: stop making the state line and the legend do the teaching, and make the page's mode unmistakable (it currently differs from PERF mainly by the tile row and the 4 corner-bracket icons).

## Nielsen scores (honest)

| # | Heuristic | Score | Key issue |
| --- | --- | --- | --- |
| 1 | Visibility of system status | 3 | Tile fill + white vs grey + marker + GEN + chips make set/unset and diced results clear. Loses a point: chips drop the `= SET` legend and `HOLD KEY = MOVE` as soon as any chip exists (note_set, note_dice, note_toast); OCT-2 shows in a chip but PITCH still reads `C2 AUTO` (note_set), so the played note is not stated anywhere |
| 2 | Match system / real world | 2 | `KNOBS = STEP 9`, `HOME = DONE`, `GEN C2` are plain. `NO SLD`, `ACC`, `TIE`, `GEN`, bare `ALL`, and "this step" bracket icons need learning. The six layer names (ALL NOTE ACCENT PROB SLIDE ON/OFF) exist only in the TSV: the screen shows no button labels at all |
| 3 | User control and freedom | 3 | HOME exits, EDIT undoes/redoes, hold EDIT = RESET, hold key moves step. Good model. But `EDIT=UNDO` is visible only in a transient toast, and during the toast the `HOME = DONE` cue is replaced (note_toast footer reads `1 ALL   EDIT=UNDO`) |
| 4 | Consistency and standards | 2 | Same tile positions, grey dice tiles, same pixel font as PERF: good. But the six icons reuse PERF's die glyphs with a different frame, so the same position means "re-roll the whole layer" on PERF and "re-roll/toggle this step" here with a 3 px visual difference; some are one-shot dice (ALL, NOTE) and some are toggles (ACCENT, PROB, SLIDE, ON/OFF) with identical look |
| 5 | Error prevention | 3 | Edits are per-step, undoable, deterministic; RESET protected behind a hold. No risk of destroying the pattern. Accidental press of a one-shot dice (FX/ALL) rewrites 4 layers with only `ALL` as evidence |
| 6 | Recognition rather than recall | 2 | Knob roles are labelled (PITCH GATE VELO RATCH): excellent. Button roles are icons only; the user must recall that the 4th bracket icon = PROB, 3rd = ACCENT. `HOLD KEY = MOVE` and `= SET` are the only teaching and are conditional |
| 7 | Flexibility and efficiency | 4 | One hold to enter, knobs act at once, dice per step, undo, key move; expert path is strong |
| 8 | Aesthetic and minimalist design | 2 | Density is on-brand, but all bands are equal weight; the status line is overloaded; 12 look-alike icon tiles; the white lane frame with its glyphs, the amber button fills and the amber tile border compete |
| 9 | Error recovery | 3 | EDIT undo/redo and per-property RESET are exactly right. The toast that offers the undo is brief, bare (`1 ALL EDIT=UNDO`), and says nothing about what changed |
| 10 | Help and documentation | 2 | HOME-hold = HELP exists per TSV, but nothing on this page says an icon is "this step" or what it is named; `= SET` is the only in-page gloss and it is 3.68:1 grey |
| | **Total** | **26/40** | **Good, with real gaps** |

Evidence band: renders only; no hardware. Score is capped by that, not raised.

## Cognitive load (8-point checklist)

Failures: (a) visual hierarchy below the tiles (equal-weight bands), (b) minimal choices: 12 button tiles, 4 knobs, 27 keys, plus 4 chip slots visible; (c) working memory: the six icon roles are not named on screen. Single focus, chunking, grouping, progressive disclosure pass. 3 failures = moderate load, address soon. It is better than PERF at one thing: the tile row gives a single obvious focal area.

## The six "this step" icons versus PERF's dice (note_buttons_4x)

- Difference is the frame only: PERF = closed die frame with the layer glyph inside; NOTE_EDIT = four corner brackets with a smaller glyph inside. The glyphs (pips, note, `>`, `~`/`%`, die) are the same family and sit on the same T_DICE grey tile.
- At 30 mm the gap in each frame edge is ~3 px = ~3.7 arcmin. That is within reach of a rested eye but not of a quick glance mid-performance; in the 1x render the two sets read as "the same icons, one set lighter". A first-time user will not know the bracket set is a different mode. A second-time user who knows the bracket convention will find it fast, but only by comparing against memory of PERF (the PERF screen is not visible at the same time).
- Distinct glyph inside bracket frames are small: 7-9 px, 1 px strokes, with `>`, `~` and `%` strokes at ~1.2 arcmin. The pip die vs the pip bracket, and the `>` vs the `~`-bracket, are the pairs most likely to confuse at 35 cm.
- Set/unset without colour (4x strip): set tile = amber fill + black glyph, unset = grey tile + white glyph. Luminance and polarity both flip (amber 9:1 against black glyph vs white on 2.3:1 grey), so it survives greyscale. Strong. The bright 1-2 px white bottom edge is redundant and tiny. Good.
- Weakness: ALL and NOTE (one-shot dice) can never light; ACCENT, PROB, SLIDE, ON/OFF toggle. Nothing differentiates action from toggle until pressed; the user pressing a lit one expects "off" and may get something else (ACCENT off returns to 80/AUTO, per TSV).
- Labels from button_modes.tsv (ALL NOTE ACCENT PROB UNDO SET, HOME SAVE SLIDE ON/OFF PLAY REC) are not drawn. The screen cannot confirm the TSV, because the TSV labels never reach the screen. That is consistent with the locked PERF screen (icons only, sub-labels under the dice on PERF show the algorithm word), but PRODUCT.md principle 2 and the product rule "on-screen labels for the current button roles" are only half met on both pages.

## Legend, move cue, HOME cue

- `■ = SET`: the marker square is ~4 px and sits grey-on-black at 3.68:1; the legend is conditional (shows in open and set, gone in dice and toast). The marker also appears on the tiles themselves, so the legend is explaining a mark the user has not yet seen when the page opens (note_open: the square appears next to `= SET` while no tile has one). The legend is not wrong, but it is reference text at the weakest size/contrast on the screen.
- `HOLD KEY = MOVE`: shown only in note_open and note_final (no chips). With any chip (note_set, note_dice, note_toast) it disappears: the status line becomes `STEP 9 [chip] [chip]` and, in note_dice, even `= SET` is dropped. TSV note says "status line right: how to move the editor", so this is a spec deviation in effect (and an F-12-adjacent discoverability gap).
- `HOME = DONE`: strong in the open/set/final states (footer, 11 px, ~13.5 arcmin, 9.7:1). It is replaced by the toast in note_dice/note_toast for the toast's lifetime. The exit cue disappearing right after a dice press is the wrong moment to hide it.
- `KNOBS = STEP 9`: duplicates `STEP 9` directly above the graph (same number twice, 6 rows apart) while the actual job of the line, telling the user the knobs now edit one step, is expressed with an equals sign. It works; it costs 11 px of the 240 px budget for a duplicate number.

## The played-note text

- Tile shows `C2` big and grey with `AUTO`; after a turn `F2` white with `GEN C2` under it. That answers "what plays" and "what would play" in one tile: the strongest element on the page.
- Gap 1: when OCT-2 is set (note_set), PITCH still shows `C2 AUTO`. The audible pitch (two octaves down) is not shown anywhere; the chip says `OCT-2` but the note name does not change. A producer will read `C2` as the played note.
- Gap 2: the dice view shows `GEN C2` and `F2` but VELO `80` over `GEN 43` and `ACC`-marker; the numeric velocity vs `ACC` rule (>=100 reads `ACC`) is invisible until you hit it. Minor.
- Gap 3: AUTO sits flush with the tile bottom (no padding; with the selected-tile amber frame it touches), so it feels clipped at 4x.
- `1x` ratchet with a small `x` and `40%` with a small `%` are good: the unit is subordinate. `NO SLD` is jargon; `NO SLIDE` was shortened to fit four chips.

## Consistency with the locked main screen

Strong: identical header, same 4-tile row geometry (58 x 53), same graph band, same two button rows, same 27-key strip, same palette. Weak: (1) the page's identity (mode) rests on the tile contents and the status line; the header reads `ACID MIDI 1` like PERF, with no `NOTE EDIT` or step-flag anywhere in the header; (2) the status line height is also the lane's header zone in PERF, so the graph shifts relative to PERF; (3) PERF's selected-tile amber frame (RATCH in note_set) means "knob last touched" here, but amber also means "set" on the buttons and chips, so amber has three jobs on one screen (selection, set, channel colour).

## Persona red flags

Personas (project-specific; PRODUCT.md has none, so these are assumed): Alex = live performer who knows the box; Sam = producer who dials a TB-3 line in at the desk; Jordan = first-timer; Dana = low-vision or colour-blind user at arm's length.

- **Alex**: One hold, knobs, HOME: perfect. Red flags: the toast hides `HOME = DONE` and `EDIT=UNDO` is only there; `ALL` dice is on the same tile family as the toggles, so a stray press rewrites four layers; cannot tell at a glance which of the four bracket tiles is lit when playing in low light (amber 2 px bottom edge only), though the amber fill helps.
- **Sam**: Understands PITCH/GATE/VELO/RATCH at once. Red flags: sets OCT-2 and the tile still says C2; does not know `GEN`; cannot find which icon is SLIDE without trying (ARP printed on the panel does not say SLIDE; the screen does not either); `NO SLD` reads as an error.
- **Jordan**: Holds a key and arrives on a page that looks like PERF. Red flags: no page name, no step explanation beyond `STEP 9`; the bracket icons read as "the same buttons"; `= SET` with a tiny square is a riddle before any tile has one; no hint that the icons are named. Will find knobs, will not find ACCENT/PROB/SLIDE without help.
- **Dana**: Colour-blind: set/unset is safe (luminance flips, marker, text colour). Red flags: chips and tile frames in amber/grey differ in luminance only weakly in the 41-vs-74 grey scale (2.3:1 icon tile vs bg); `= SET` at 3.68:1 and 11 arcmin; the bracket gaps at ~3.7 arcmin are below a comfortable discrimination threshold at low acuity; legibility of 9 px text at 35 cm is borderline.

## Priority issues

**[P1] The six "this step" icons are not reliably distinguishable from PERF's dice, and are unnamed.**
Why: the only difference is the frame gap (~3.7 arcmin); roles are not labelled on screen; the user can mistake NOTE_EDIT for PERF and press ALL expecting a layer dice (which rewrites four step properties). Fix: add a one-glance mode cue that does not depend on the icon detail: draw the six tiles with a different tile treatment (for example a thick white 1 px top edge or an inverted label strip) or relabel the page-identifying cell: the header algorithm field becomes `STEP 9` in the channel colour. Also give the six an AF_S 3-letter label under or inside the tile (`ALL`, `NOTE`, `ACC`, `PRB`, `SLD`, `ON`) at least while the page is open; alternatively a 2 s label flash on entry. Command: /impeccable clarify, /impeccable typeset.

**[P1] Conditional cues vanish when the user acts (`HOLD KEY = MOVE`, `= SET`, `HOME = DONE`).**
Why: they disappear at the exact moment the next step is needed; the spec lists the move hint as a permanent right-hand status field. Fix: reserve a fixed slot per cue; move chips below or into the tile (a small `OCT-2` corner tag on PITCH, `PROB` on the footer) so the status line is `STEP 9   HOLD KEY = MOVE` always; keep `HOME = DONE` in the footer and show the toast above the graph instead of over the footer (the footer is the only page-wide cue). Command: /impeccable layout, /impeccable polish.

**[P1] The played note is wrong or missing in common states (OCT, ratchet gap).**
Why: `OCT-2` as a chip with `C2 AUTO` in the tile misleads; the sound is C0. Fix: the PITCH tile value must include the octave offset (`C0`, with `AUTO`/`GEN` under it only for the generator), or the chip is replaced by the changed value in the tile with a marker. Command: /impeccable clarify.

**[P2] `= SET` legend fails contrast and is premature.**
Why: 3.68:1 versus the 4.5 floor in PRODUCT.md; it explains a mark that is not yet visible. Fix: bring it to the T_MID tone used by `HOME = DONE` or 6.5:1, show it only once a marker exists, and use the freed space for `HOLD KEY = MOVE` or `GEN` ("GEN = ALGO"). Command: /impeccable polish.

**[P2] Toast `1 ALL  EDIT=UNDO` is terse and replaces the footer.**
Why: `1` is the channel number, `ALL` is ambiguous; no statement of what changed (`DICED STEP 9`, `NOTE`, `OCT`); hides HOME=DONE. Fix: `DICED ALL  EDIT=UNDO`, `NOTE`, with the toast in the status line, not the footer. Command: /impeccable clarify.

**[P2] Amber carries three jobs (channel colour, "set" button fill, selected-tile frame).**
Why: the set cue is partly colour; the selected tile frame and set fills look alike when two are adjacent (note_set, RATCH framed beside set chips). Fix: use white for the knob focus frame or a different shape (underline), and keep amber for "set". Command: /impeccable colorize, /impeccable polish.

**[P3] `KNOBS = STEP 9` duplicates `STEP 9`.** Replace the footer left with a value that matters (`K1-K4 = THIS STEP`) or drop the duplicate number; the equals sign grammar is fine.

**[P3] AUTO touches the tile bottom; chips are `NO SLD`.** Add 1 px padding; use `NO SL` or an icon.

**[P3] Dice vs toggle indistinguishable.** One-shot tiles (ALL, NOTE) should look like PERF's dice, toggles like switches (corner tick on the tile when set is already there). Visual language for "action vs state" is not defined.

## What works

1. The tile row: grey `AUTO` vs white value + fill + marker + `GEN` is a redundant (colour-independent) and instant "did I change this" answer, and the biggest type on screen goes to the values that matter.
2. Set/unset on the buttons survives greyscale: fill luminance and glyph polarity both flip, with a redundant bottom edge.
3. Control model: one hold in, knobs act immediately, HOME out, EDIT undoes, hold-key moves: exactly "instant to understand" at the gesture level. Same layout and palette as PERF means no re-orientation cost.

## Minor observations

- Selected-step frame in the lane is a white box that extends over the full lane height: strong and clear, consistent with the key frame.
- `OCT-2` chip in chip row duplicates what the OCT+/OCT- keys do, fine, but a signed `OCT-2` at 9 px is the same size as the `-` in `OCT+1`: the sign is easy to miss at 35 cm.
- The `x` and `%` unit subscripts are ~5 px; fine because the numbers are huge.
- The `1` in the footer toast is channel colour amber on white; `ALL` black on white: colour-coded without text for the channel (colour alone for channel identity here).

## Questions to consider

- What if the page announced itself in the header (`STEP 9` replaces the algorithm name for the visit) so no one mistakes it for PERF?
- Could the six icons be replaced with three-letter labels at AF_S, since 13 px icons cannot carry the distinction the spec relies on?
- Does the user need `GEN` shown on every set tile, or only on request (hold)?
- What if the toast lived in the status line and never touched the footer?

Verdict for the P4-014 owner round: no P0. Three P1s centre on cues that exist in the spec but are absent, conditional or below the contrast floor on the rendered screen. Score 26/40.
