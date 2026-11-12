<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Assessment A: design review of the NOTE_EDIT page

Reviewer: independent design director (Impeccable critique, Assessment A: no detector output seen).
Target: the NOTE_EDIT page as rendered in `build/ui_sq/note_open.png`, `note_set.png`, `note_dice.png`, `note_final.png`
(240 x 240, 8 px/mm, viewed at 30-40 cm). Compared with the locked PERF screen (`proposals/v12/out/v12_hold_edit.png`,
`v12_dice.png`, SCREEN_SPEC_V2, ADR-0006). Truth for roles: S03, SEQUENCER_UX F-12 and 2.2, `button_modes.tsv`,
`knob_map.tsv`, `key_map.tsv`, `note_edit_text.tsv`, `features/P4-014-note-edit-page.md` sections 1-6 (the product intent;
sections 7-9 are the executor's walk-through and I did not use them to judge). Renders only: nothing was seen on a real unit.

## 1. Verdict in one paragraph

This is a competent, honest page that inherits the locked PERF grid almost perfectly, and that inheritance is both its
strength and its problem. It looks like PERF with the four knob labels swapped, so **a producer glancing at it will not
instantly know they are in a different mode, nor that every button just changed meaning**. The four tiles are good.
The six layer buttons are the weak point: they are still the die ("re-roll") pictures while four of the six now do a
different thing (toggle on one step), and the one on-screen sentence that could fix this ("KNOBS EDIT") says nothing
about "this step". The most useful number on the screen (what note the step will play when AUTO) is also the smallest
text. Score 24/40: acceptable, not yet "instant to understand".

## 2. AI-slop verdict

Not AI slop. No gradients, glow, rounded cards, decorative colour, identical card grids, or hero numbers. It is a 1-bpp
hardware screen built from a fixed palette and three bitmap fonts, and it reads as an instrument, not a template. The
anti-reference list in PRODUCT.md (menu-diving LCD, spreadsheet, toy skin, phone app) is avoided: it is nearest to the
"spreadsheet" risk, because the lane, 12 buttons and a keybed are all still at equal weight under four good tiles.

The "tells" that are present are the machine-systematic kind, not the decorative kind:
- Reuse over thought: the dice icons were carried over unchanged to a page where they lie (see 7, P1-2).
- One mechanism for two meanings: the tile fill that means "level" on PERF means "set by you" here (P2-1).
- Over-literal copy: `KNOBS EDIT`, `GEN`, `= SET BY YOU` are spec words, not producer words.

## 3. Hierarchy (what the eye does, in order)

1. The four tiles (y 24-76): correct. Value AF_L is 20 px high, white on the dim channel fill; this is the loudest thing
   and it is the right thing.
2. The set-field chips (y 80-92): orange chips with black text, readable, correctly second.
3. The lane (y 92-141, the biggest region of the screen): the white frame round step 9 is clear, but the framed column
   contains **no bar** in all four renders (a bare dash, or empty), so the step being edited is the one step that
   shows nothing about itself. Setting PITCH F2 in `note_dice.png` does not change what the frame shows.
4. Buttons, keybed, footer: unchanged PERF furniture.

Inverted pyramid at fault: the tile says `AUTO` big and `GEN C2` small (AF_S, 7 px caps). The useful fact ("this step
will play C2") is the least visible element, and the large word `AUTO` carries nothing a producer can use. Four tiles of
the large word `AUTO` in `note_open.png` is the loudest message and the emptiest one.

Mode identity: nothing on the page is named "NOTE EDIT". The S03 banner (`CH1 NOTE EDIT STEP 9`) is not drawn. The
only signs of a mode change are the tile labels, the 2 px white column frame, `STEP 9` at AF_S+AF_M, and the footer.
In a live setting, with the lane and keys identical to PERF, that is a weak signal.

## 4. Cognitive-load checklist (8 items; 3 fail = moderate)

| Item | Result | Note |
| --- | --- | --- |
| Single focus | PASS | the four tiles dominate; one step is boxed |
| Chunking (<= 4 per group) | PASS | 4 tiles, up to 4 chips (`NO SLIDE`, `PROB 50`, `OCT-2`, ON/OFF) |
| Grouping | PASS | chips sit with `STEP n`; tiles in one band |
| Visual hierarchy | **FAIL (partial)** | AUTO word beats the generated value; the edited step shows no bar; mode not named |
| One thing at a time | PASS | knobs, then buttons, then HOME |
| Minimal choices (<= 4 at a decision point) | **FAIL** | live at once: 4 knobs, 6 layer buttons, OCT-, OCT+, EDIT, HOME, 16 step keys, 11 black keys. Honest, but the producer must know which 12 matter |
| Working memory | **FAIL** | must remember that die pictures now mean "this step", which key moves the editor (hold 0.4 s), and how to clear a value (turn to the low end); none is drawn |
| Progressive disclosure | PASS | chips appear only when a field is set; set fill appears on set |

Decision points over 4 options: the button block (12 tiles, of which 8 are live for this page). Memory bridges:
button meaning (icon to "this step"), step-move gesture, clear-to-AUTO gesture.

## 5. Nielsen's 10 (0-4)

| # | Heuristic | Score | Key issue |
| --- | --- | --- | --- |
| 1 | Visibility of system status | 3 | step framed in lane and on its key, set fields shown by fill and chip, toast `ALL EDIT=UNDO`. Gaps: the lane bar does not reflect the edit; toggle buttons show no state; the legend `= SET BY YOU` vanishes once the first chip appears |
| 2 | Match with the real world | 2 | `GEN`, `AUTO`, `= SET BY YOU` are system words. A wave icon for PROB, a note-length icon for SLIDE and a die for ACCENT do not match a producer's picture of those ideas. Units are missing (`100` for GATE reads like a velocity) |
| 3 | User control and freedom | 3 | EDIT = undo/redo and HOME = done are clear, and the toast names the reverse. Reset-to-AUTO by EDIT-hold is not built (debt); only knob-to-low-end clears a field |
| 4 | Consistency and standards | 2 | grid, header, toast, footer slot all match PERF (good). But the die frame (= re-roll) now sometimes toggles; fill = level on PERF, = "set" here; footer grammar `X = Y` (PERF `KEYS = STEPS`) becomes `KNOBS EDIT` |
| 5 | Error prevention | 2 | any stray knob nudge turns AUTO into a set value (first detent), and a silent step can be edited with no on-screen sign that it will not sound |
| 6 | Recognition rather than recall | 2 | no button label on screen (names appear only as chips after the fact or in the toast); OCT-/OCT+ are not drawn at all; step-move and clear gestures unstated |
| 7 | Flexibility and efficiency | 3 | one hold to enter, one press per property, dice-this-step (FX/SCL), undo whole visit in one tap, hold another key to move: fast and good |
| 8 | Aesthetic and minimalist design | 3 | disciplined; 12 live buttons and the entire PERF keybed remain under a one-step task |
| 9 | Recover from errors | 2 | UNDO covers the visit. No message when a press does nothing (for example ENV on a step that is OFF) is shown in these renders; clearing one field is by feel |
| 10 | Help and documentation | 2 | HOME-held HELP exists system-wide, but the page itself carries no hint for gestures and says nothing of what the layer buttons now do |
| | **Total** | **24 / 40** | **Acceptable: significant improvements needed to reach "instant to understand"** |

## 6. Personas

(Alex, Sam, Jordan from the Impeccable set; Dana is the project persona from PRODUCT.md: a producer sequencing one TB-3,
hands busy, glancing at arm's length, performing live.)

**Alex (power user, knows the T-1 / Elektron).** Enters in one hold, loves K1-K4 plus per-property buttons, loves
EDIT-undo. Red flags: no way to see the generated pitch at a glance when AUTO (has to read AF_S `GEN C2`); no direct
"hold EDIT then touch = reset" yet (debt), so clearing PITCH means rolling K1 all the way down; no step-to-step nudge
(the only way to move is hold a key 0.4 s, not drawn). Will say it feels like PERF with a different knob row.

**Sam (accessibility: low vision, cognitive differences).** Contrast is good (measured on the render: AUTO value text
about 7:1 on the tile; `GEN` text white-ish on dim orange, high; STEP label about 6:1 on black). Red flags: the three
AF_S strings (`GEN ..`, `STEP`, `= SET BY YOU`) are 7 px caps; the set-state is shape plus colour (fill and chip), which
passes "colour never alone"; the full-height fill at any value (see P2-1) hides magnitude; the thin 1-2 px frame round the
edited step and its key is the only "where am I" and is small.

**Jordan (first-timer, never used a step sequencer).** Held a key, got four tiles saying AUTO AUTO AUTO AUTO. Does not
know AUTO means "the algorithm decides", does not know GEN is "generated", cannot tell what the die icons do here (they
looked like "random" on the main screen), and cannot see how to get out except by guessing HOME (footer does say
`HOME = DONE`: good). Will turn a knob, hear nothing (no audition is built: debt), and not know whether it worked.

**Dana (live TB-3 performer, hands busy).** Needs: which note is this step (AUTO shows the answer small); accent and
slide on this one note, now; no accident. Red flags: the screen looks the same as PERF at arm's length, so a hand
reaching for FX expecting "re-roll everything" will instead re-roll one step, or the reverse. Mode confusion is the real
live-performance hazard here, and it is the issue the page hides best.

## 7. The 30 mm legibility test

Conversion: 30 mm over 240 px gives 0.125 mm/px. One pixel subtends 1.43 arcmin at 30 cm, 1.23 at 35 cm, 1.07 at 40 cm.
(SCREEN_SPEC_V2 quotes "AF_S ~10.8 arcmin"; my measure of AF_S caps on the render is 7 px, so 10.0 / 8.6 / 7.5 arcmin
at 30 / 35 / 40 cm. SCREEN_SPEC's number is only right at about 30 cm with 8-9 px caps; treat AF_S as 8-10 arcmin.)
Cap height measured from pixels of `note_dice.png` and `note_open.png`.

| Element (font) | Cap px | 30 cm | 35 cm | 40 cm | Verdict |
| --- | --- | --- | --- | --- | --- |
| Tile value F2 / 100 / ACC (AF_L) | 20 | 28.6' | 24.6' | 21.4' | comfortable at any distance |
| Tile labels PITCH GATE VELO RATCH, `AUTO`, footer, `NOTE`-style chips (AF_M) | 11 | 15.7' | 13.5' | 11.8' | readable; at the low edge at 40 cm; fine for words read once |
| `STEP` word, `GEN ...` under every tile, `= SET BY YOU` (AF_S) | 7 | 10.0' | 8.6' | 7.5' | **fails the usual 12-16 arcmin floor for glancing**; acceptable only for reference. But `GEN C2` is *the* piece of information for AUTO tiles |
| Chip text `NO SLIDE`, `PROB 50`, `OCT-2` (AF_M, 11 px caps) | 11 | 15.7' | 13.5' | 11.8' | readable; good |
| White frame round the edited step (2 px) | 2 | 2.9' | 2.5' | 2.1' | detectable as a rectangle (long line), not as a label |
| 13 x 13 button icon strokes (2 px) | 2 | 2.9' | 2.5' | 2.1' | strokes are at the limit; the **icon silhouette** (13 px = 18.6'/16'/13.9') reads, the inner glyph (9 px = 12.9'/11'/9.6') is marginal: is the wave a wave? |

Conclusion: the page passes for what it asks you to read mid-performance (value, labels) and fails for the one thing
Dana most needs to glance at (the generated value under AUTO) and for the six button pictures at distance.

## 8. Does every label and icon match its TSV role?

| Item | On screen | TSV role | Match |
| --- | --- | --- | --- |
| K1 / K2 / K3 / K4 | PITCH GATE VELO RATCH | knob_map NOTE_EDIT | yes, exact, all <= 6 chars |
| Value words | AUTO, ACC at >= 100, TIE, `2X`-style | note_edit_text.tsv, knob_map | yes (TIE and ACC not visible in these four renders except the VELO 100 case; unverified) |
| `GEN <v>` | under each value | status line 1 / F-05 | text yes; but `GEN` is not an English word a producer owns |
| Footer | `KNOBS EDIT` / `HOME = DONE` | TSV KNOBS_EDIT, HOME_DONE | yes, as specified. Spec is the problem (see P1-3) |
| Legend | square + `= SET BY YOU` | TSV SET_MARK | yes; disappears once chips fill the row |
| Chips | `NO SLIDE`, `PROB 50`, `OCT-2` | TSV SLIDE, NO_SLIDE, PROB, OCT | yes. ON/OFF and ACCENT chips: no render shows them (ACCENT shows as VELO ACC) |
| FX (all-dice) | five pips die | ALL = dice this step | die = re-roll: **correct** meaning and the only dice button that is still a dice on this page |
| SCL (note) | die with note | NOTE = dice this step's pitch | correct (re-roll) |
| ENV (`>`) | die with `>` | ACCENT toggle | **mismatch**: a die says "roll"; it toggles accent on/off |
| LFO (wave) | die with wave | PROB toggle 100 / 50 % | **mismatch twice**: die = roll, wave = movement, the role is probability |
| ARP (long bar over three short) | die with note lengths | SLIDE toggle | **weak**: lengths are not glide; die says roll |
| SEQ (x heads) | die with drum x | ON/OFF of this step | **weak**: x heads are drum hits, not "this step on/off"; die says roll |
| EDIT | undo arrow | UNDO (hold: RESET) | yes |
| HOME / SAVE / PLAY / REC / GLO | house, floppy, triangle, ring, cog | as TSV | yes |
| OCT- / OCT+ | not drawn | OCT- / OCT+ | **absent**; the chip `OCT-2` exists but its controller is not shown (named only while touched, per SCREEN_SPEC 1.3) |
| White keys | frame under edited step only | STEP tap / EDIT hold | the **hold-to-move** gesture is not stated anywhere on this page |
| Black keys | M1-M4, CHN, S F R, arrows, RST | key_map NOTE_EDIT rows | yes (inherits PERF) |

Net: four of the six layer buttons carry a picture that contradicts their role on this page.

## 9. Consistency with the locked main screen

Good: header and toast slot (`1 ALL EDIT=UNDO` is exactly the PERF toast grammar), tile geometry and label band, button
grid, keybed, palette and the channel colour, white column frame (reused from the REC cursor, as the spec said), font
and chip use. This is as consistent as a derived page gets.

Where it diverges:
- Fill semantics: PERF fill height = value (`DENS 12` is a quarter-high meter); here a set tile is filled to 100 % at any
  value (`VELO 83` and `PITCH F2` look maxed). Same pixels, different meaning. `AUTO` tiles are plain.
- The die frame stays but no longer always means re-roll.
- Status line: PERF's `P01  ROW 1 2  PAGE 1/2` becomes `STEP 9` plus chips; fine, but with `STEP 9` in AF_S+AF_M it is the
  smallest identifier of the mode.
- Footer grammar: PERF `KEYS = STEPS` / `HOLD = EDIT` (a mapping plus the hold gesture); here `KNOBS EDIT` / `HOME = DONE`
  drops the "=" mapping and the step gesture. PERF's footer is the very line that told the producer how to get here.
- A turning-frame on RATCH persists in `note_set.png` / `note_dice.png` (RATCH framed while its tile is AUTO and no knob is
  being turned): it can be read as "selected" or "set"; unclear. Verify on hardware whether it is a render of the 1 s
  turning state.

## 10. Would a producer understand it instantly?

Not yet. Split by task:
- "Change this note's pitch / length / velocity / ratchet": **yes, instantly.** Four labelled tiles, turn the knob under the
  label, value appears and the tile fills. This is the best 40 % of the page.
- "Make this note accented / slid / probabilistic / off": **no.** The producer must press a die picture that does not say
  accent, slide or probability, and learn the result from a chip afterwards. The first press is a guess.
- "What is this step now?": **partly.** Tiles say what you have set; GEN tells (small) what the algorithm gives; the lane
  frame shows no bar.
- "Where am I / how do I leave / how do I move to another step?": leave yes (HOME = DONE); where partly; move no.
Owner's bar not met on two of four questions.

## 11. Priority issues

### P1 (fix before sign-off)

**P1-1. AUTO tiles hide the one number that matters.** `AUTO` is AF_L-or-M, `GEN C2` is AF_S 7 px (8.6 arcmin). Fix: in
AUTO, show the *generated value* as the big tile text in MID grey (`C2`, `40`, `43`, `1X`), and the small AF_S line says
`AUTO`. Set tiles: big white value, small line `GEN C2` (what it replaces) or `SET`. The fill says set; the grey value
says following. Nothing new to learn: "grey = the algorithm's, white = yours", and there is no loud empty word on screen.

**P1-2. The six layer buttons show a die ("re-roll") but four of them toggle on one step.** (See section 8; this is the
open decision of the use case, ease score 9 below the pass bar of 11.) Fix: a dedicated "this step" picture and a
live-state tile (section 12 below). Minimum viable if icons cannot change: swap the dice frame for the step frame on
NOTE_EDIT only (it is a frame change, not new glyph artwork), and add a state tint.

**P1-3. The page is not named and the footer does not say "this step".** Fix: footer left `KNOBS = STEP 9` (same `X = Y`
grammar as PERF `KEYS = STEPS`, with the step number which is also the AF_M-size answer to "which step"). Right:
`HOME = DONE` unchanged. Show `STEP 9` in AF_M (not AF_S) and draw the status line as a channel-colour band across the
width (a distinct strip, like the inverted setup cue) so the whole page reads different from PERF at arm's length: a
shape and a position, not colour alone. Optionally draw the S03 banner `NOTE EDIT` in the message slot's resting state.

### P2

**P2-1. Fill means "level" on PERF and "set" here.** 100 % fill at any value reads as maxed. Fix: use the same pale
channel fill for set (it is already the T_CHF) but add a 2 px corner tick or the white set-mark square (the same
`= SET BY YOU` square) at the top-left of the tile, and let the fill height be the real level for GATE, VELO, RATCH and the
pitch within its range, exactly like PERF meters. Then `VELO 83` is a four-fifths tile everywhere in the product.

**P2-2. Set-mark legend disappears when needed most.** `= SET BY YOU` is shown only while no chips occupy the row. Fix: put
the tiny square on each set tile (P2-1) and drop the legend text; the square is the legend and it never competes for
space.

**P2-3. The edited step in the lane has no bar.** Fix: in the framed column draw the step's actual current bar (the edited
pitch, accent cap, slide bridge, hollow if forced OFF) so the lane previews each edit. If the step is OFF, draw the
hollow cell and an `OFF` chip so the producer knows why a pitch edit is silent.

**P2-4. Missing units and OCT.** GATE `100` reads as velocity; add `%` (AF_S superscript) or show `100%` at AF_M. `OCT-2`
chip: say what moved: `OCT-2` appears but not which control. Draw the OCT- / OCT+ keys' arrows next to the chip as
`< OCT-2 >` (the two arrows are the panel's printed labels).

**P2-5. Gesture hints for the three non-obvious gestures** (hold key = move editor, EDIT = undo, EDIT hold = reset when
built). Fix: right footer cycles or alternates; or the lane header text `HOLD KEY = MOVE` in AF_S where PERF puts
`hold=EDIT`. Cheap and removes the Jordan failure.

### P3

- P3-1. `GEN` is jargon; consider `ALGO C2` (the channel's own word `ALGO` is on the encoder) or an `auto C2` small line.
- P3-2. Chips use only channel colour; add the toggled-state pair as text (`SLIDE` vs `NO SLIDE` already does; do the same
  for `ACCENT` / `NO ACC` and `ON` / `OFF`) so every layer button has a visible result chip.
- P3-3. The two AF_S strings that remain (`STEP`, `GEN`) are 8.6 arcmin at 35 cm; if the P1 changes demote more text to AF_S
  keep it to `AUTO`/`SET` and the units only.
- P3-4. RATCH persists its turning frame in `note_set.png` / `note_dice.png`; check on hardware and clear it with the same
  timer as PERF.

## 12. Dedicated "this step" picture for the six layer buttons

Concept: the die is the PERF frame for "re-roll the whole pattern". The NOTE_EDIT frame should echo what NOTE_EDIT
already draws round the edited step: the **white step frame** (the lane's frame, the REC cursor frame, the white frame on
its key). So: **layer glyph inside a step bracket (open corner brackets) instead of a die.**
Grammar: die = whole pattern roll; bracket = this one step. The same 9 x 9 inner glyphs for the layers that keep their
meaning, three new inner glyphs for the three layers whose role changed meaning. Keeps the 1-bpp, 13 x 13, 2-px-stroke
rule of icons_v12.py (`die()`-style helper, margin 2 px).

Frame (13 x 13, brackets 2 px thick, arms 3 px, open mid-sides, so the 9 x 9 inner area at (2,2) is untouched):

```
STEP = [
"###.......###",
"##.........##",
"##.........##",
"#...........#",
".............",
".............",
".............",
".............",
".............",
"#...........#",
"##.........##",
"##.........##",
"###.......###"]
```

Python in the icons_v12 idiom (`step(inner)` places 9 rows of 9 chars at (2,2), exactly as `die(inner)` in icons_v7.py):

```
def step(inner):
    rows = [list(r) for r in STEP]
    for j, r in enumerate(inner.strip("\n").split("\n")):
        for i, c in enumerate(r):
            if c == "#": rows[2 + j][2 + i] = "#"
    return "\n".join("".join(r) for r in rows)
ICONS["step_all"]    = step(<same 9x9 as dice_all>)      # FX  : new pitch, accent, slide, gate for THIS step
ICONS["step_notes"]  = step(<same 9x9 as dice_notes>)    # SCL : new pitch for THIS step
ICONS["step_accnt"]  = step(<same 9x9 as dice_accnt>)    # ENV : accent on/off for THIS step
ICONS["step_slide"]  = step(SLIDE9)                      # ARP : glide into next step
ICONS["step_prob"]   = step(PROB9)                       # LFO : chance 100 / 50 %
ICONS["step_onoff"]  = step(ONOFF9)                      # SEQ : step on / off
```

New inner glyphs (9 x 9, 2 px strokes; needs a render pass on the mock tool before adoption, drawn by hand here):

```
SLIDE9            PROB9             ONOFF9
.........         ##.....#.         .........
.......##         ##....##.         .........
.......##         .....##..         ..#####..
......##.         ....##...         ..#####..
.....##..         ...##....         ..#####..
##..##...         ..##.....         ..#####..
##.##....         .##....##         ..#####..
.........         .#.....##         .........
.........         .........         .........
```
(SLIDE9: a low note joined by a diagonal glide to a high note. PROB9: a percent sign. ONOFF9: a solid step cell, the
lane's "on" cell; when the step is OFF the tile draws it with an X strike. The dice versions of FX, SCL, ENV keep their
inner glyphs because accent, pitch and "all of it" are the same idea at pattern level and step level; only the frame
changes.)

Behaviour, because a picture alone is not enough (colour never alone):
1. **State on the tile for toggles.** SLIDE, PROB, ENV (accent), SEQ show their current state: unset = `T_DICE` grey tile,
   white glyph; set = the step tile filled with the channel's dim fill (`T_CHFn`) and a white 2 px bottom edge. So
   "SLIDE is on for this step" is visible on the button itself, not only in the chip.
2. **Pressed = inverted** as PERF (white tile, black glyph) for the press.
3. **Message slot on press**: `[1] SLIDE ON  EDIT=UNDO`, `[1] PROB 50`, `[1] ACCENT`, `[1] STEP OFF` (these already exist as chips; the toast
   is the same grammar as `[1] NOTES  EDIT=UNDO`).
4. **A one-time hint** the first time the page opens in a session in the message slot: `BUTTONS = THIS STEP` (AF_M, fits
   the slot), then gone, like the other cues.
5. Label letters under or inside are not available on a 38 x 17 tile; do not add them. The pictures and the toast do the
   naming; the chips name the result.

Why not "just add a label": 38 x 17 tile with a 13 x 13 icon has no room; labels <= 6 chars in AF_S (7 px) at 8.6
arcmin would fail the glance test.

Preview of the frame on the three unchanged glyphs and three die originals: `this_step_icon_concept_A.png` (top row die as
today, bottom row bracket). It reads, but the brackets are thinner than the die frame (the die is a closed outline;
brackets are 40 % of the outline length). If that is too faint at 35 cm, add the top arm pointer (a 3 px wide white notch
centred on the top edge, as the playhead pointer above the lane): a pointer means "this one" and echoes `T_PH`; this
needs to be tested on the real panel.

## 13. What is genuinely good

1. **The knob tiles.** Label, value in AF_L white, `GEN` under it, and a fill when set: one glance gives "which knob,
   what value, mine or the algorithm's". The colour-plus-shape rule holds. This is the most "instant" element in the
   whole sequencer UI and should be the template for the other pages.
2. **Fidelity to the locked grid and grammar.** Header, toast slot, tile geometry, button grid, keybed and palette are
   identical to PERF; the toast `1 ALL EDIT=UNDO` is the same sentence as `[1] NOTES  EDIT=UNDO`. A producer who has used
   PERF has nothing new to learn about structure.
3. **One-hold entry, one-press edits, whole-visit undo.** EDIT is the same undo arrow everywhere and the toast always
   names the reverse. HOME = DONE is stated. These are the right rules for a live instrument.
4. **Chips as a compact change log.** `NO SLIDE` / `PROB 50` / `OCT-2` appear only when set, in channel colour with black
   ink, readable at 13.5 arcmin, and name the result in words even though the button picture does not.
5. **The white step frame in the lane and on the key.** A strong, consistent "this is the step" device: it is also what
   the new button frame above borrows.

## 14. Questions for the owner

- If the page is a different mode, why does it look 90 % identical to PERF? A distinct band (P1-3) costs a few pixels and
  prevents the live mistake.
- Is "AUTO" the right headline for an algorithm-following value? A producer thinks "the note it plays", not "AUTO".
- Should a stray first detent on a tile really overwrite AUTO with a set value and fill the tile, or should the first
  detent only reveal (and show) the generated value, and the second set it?
- Is the lane (the largest region of the screen) earning its pixels on a one-step page? A taller tile row, or a
  zoomed single step, could answer "what is this step?" better.
