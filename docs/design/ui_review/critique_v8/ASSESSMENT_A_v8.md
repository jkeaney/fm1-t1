# Assessment A: v8 main screen (PERF)

Independent review. Inputs: PRODUCT.md, HANDOFF/01_BRIEF.md and 02_CONSTRAINTS.md, panel_photo.jpg, button_modes.tsv,
knob_map.tsv, key_map.tsv, page_states.tsv, AlgoDesc in firmware/src/sq_types.h, and all 15 v8 renders (1x and 4x).
I read v8.py, algos_v8.py and icons_v8.py only to settle what a mark means. I did not read any designer notes or earlier
critiques.

Angles use 35 cm: 1 px = 0.125 mm = **1.23 arcmin**. Reference sizes: AF_S caps 11.0', AF_M 13.5', AF_L 25.8'.
Anything under 10' is treated as invisible at a glance. Contrast figures come from the HI-CON RGB565 values the mock
uses. Deuteranope figures use the Machado (severity 1) simulation and CIELAB ΔE.

---

## 1. Generic-look verdict

**Distinctive structure, generic iconography.** The knob tiles (AF_L value, meter fill inside the tile) and the
keybed footer drawn as the real 3-2-3-2-1 keybed belong to this instrument, and nothing in a stock template does
either. The button block does not. It is a 2x6 toolbar of stock app glyphs (house, floppy, gear, undo arrow, play,
ring) on rounded-feeling tiles, which is the phone-app anti-reference in pixel form. The four "die-framed" icons look
like framed app icons, not dice. The amber dice tint also makes the block read as a CH1-coloured toolbar (see P1-6).
It is not neon or glowy and it is not a spreadsheet. The music row is the part most likely to read as "generic bar
chart", because four interleaved 3 px voices look like a spectrum display, not a sequence.

## 2. Nielsen heuristics (0-4)

| # | Heuristic | Score | Evidence |
|---|---|---|---|
| 1 | Visibility of system status | 2 | Good: play/stop glyph, BPM, channel chips, playhead column plus the bar under the key, mutes on the black keys, preset. Failures on core states: MIDI OFF is two 2 px red dashes (`v8_midi_off`, header x 160-195) while that channel's bars and key bands keep animating as if it sounds. In REC the knob tiles still show PERF roles (`v8_rec_armed`). A selected muted channel loses its selection fill (`v8_sel_muted`). The row marker is 5 px squares (6'). The viewed page is AF_S grey text. Tempo source (INT/EXT), pending preset, PREVIEW and ALGO_PICK are not mocked. |
| 2 | Match with the real world | 2 | Good: height = pitch, keys drawn as keys, the accent ">" mark, undo arrow, floppy = save. Weak: a rounded square frame is not a die. The ARTIC icon (bar plus block) and HITS icon (dots plus bar) have no real-world referent. The CHN key's 7x5 DIN glyph is unknown to most producers. Fill meters on enumerations (DIR FWD, CURVE AUTO, PICK) imply a magnitude that does not exist. |
| 3 | User control and freedom | 3 | EDIT = undo is visible, and the dice toast says EDIT=UNDO. HOME is clear. Preset-load undo (SAVE hold) and RESET (EDIT hold) are hidden behind a 4 px notch. |
| 4 | Consistency and standards | 2 | Amber means both "dice" and "CH1": DICE_FG (255,200,80) vs CH1 (230,159,0) is ΔE 18. Chip grammar flips: a coloured frame means "unselected, live" in `v8_playing` but "selected, muted" in `v8_sel_muted`. Value font jumps between AF_L and AF_M within a row (ARP: 12 and 1X large, LINE and 2 OCT small), and within one knob as you turn it (ARP DIR UP is AF_L, DOWN is AF_M). In REC the black keys are drawn dim although they play notes. The PLAY button shows ▶ while already playing. |
| 5 | Error prevention | 1 | The highest-cost error, a silent synth from MIDI OFF or a reassigned MIDI channel, is barely signalled and never explained. The "+" chip invites a SELECT turn to "add" a channel, a role the tables do not have. FX = dice ALL fires on press with no feedforward. The pending-preset frame (x 197) collides with "MIDI 16" (ends x 203) for EUCLID, SNAKE and STAGE. |
| 6 | Recognition rather than recall | 2 | Knob roles are fully recognised, which is the strongest part. Everything else needs recall: all four hold actions (the notch says only that one exists), ROW 2 via OCT+ (nothing on screen ties the dots to the OCT buttons), S/F/R being hold-only SOLO/FILL/ROLL, mute keys shown as plain colour squares, and CHN = MIDI setup. |
| 7 | Flexibility and efficiency | 3 | Every performance action is one gesture from one screen, with peek rows and per-layer dice. An expert loses little. The deduction is for the toast hiding the header during rapid dicing. |
| 8 | Aesthetic and minimalist design | 2 | The tiles are clean. 45% of the height (y 132-240) documents controls whose roles never change on PERF, while the live music row gets 33 px with 3 px voices. The selected channel's pattern is drawn twice (music row and keys), plus "KEYS = STEPS". |
| 9 | Recognise, diagnose, recover from errors | 1 | MIDI OFF says neither "OFF" nor why (a conflict with another channel). Steps past LENGTH, the MIDI-channel conflict and the last row have no mocked message. |
| 10 | Help and documentation | 1 | The HOME-hold help/map exists in the spec, but its only on-screen sign is the same anonymous 4 px notch that the other three hold buttons carry. |
| | **Total** | **19 / 40** | |

## 3. Cognitive load

| # | Check | Result | Note |
|---|---|---|---|
| 1 | One clear primary focus | PASS | The knob tiles dominate, which is correct for "what do the knobs do". |
| 2 | Chunked into at most about 7 groups | PASS | Six zones: header, tiles, row and music, buttons, keybed, footer. |
| 3 | Every control role recognisable, not recalled | FAIL | Holds, ROW 2, S/F/R, CHN and mute meaning all need recall. |
| 4 | One encoding, one meaning | FAIL | Amber = dice and CH1. Grey = muted, unused, the playhead column and inactive black keys. Selection = fill or frame depending on mute. |
| 5 | Live state legible at true size | FAIL | Row marker 6', 4-channel bars 3.7' wide, accent tick 2.5', diced hatch 1 px pitch, MIDI OFF dashes, turning ticks 2.5'. |
| 6 | Progressive disclosure | PASS | Toasts and the HOME-hold map hold detail back from the main screen. |
| 7 | No duplicated information | FAIL | The selected lane appears in both the music row and the keybed. "KEYS = STEPS" repeats what the key bands already show. |
| 8 | Feedback at the point of action, same frame | PASS | The turned tile is framed, the pressed button lights white, a toast appears. |

**4 / 8. Density verdict:** the number of zones is right, but the pixel budget is spent in the wrong places. Static
legends take 108 px (buttons 50, keybed 34, footer 12, plus gaps). The music row, the only element that shows the
music itself, gets 33 px, and with four channels its marks fall below glance size. The screen is not too busy as a
whole; it is too busy in the music row and too generous to the button picture.

## 4. Workflow test from the screen alone

| Step | Result | Why |
|---|---|---|
| Select CH1 | PASS | SELECT is printed on the panel, and the header chip "1" fills orange. |
| Set CH1's MIDI channel | **FAIL** | The state is visible ("MIDI 2"), but the route (tap D#4 CHN, then K2) is signposted only by a 7x5 px DIN glyph (6' tall) on one black key. Nothing links the header's MIDI field to that key. This is the product's one setup step. |
| Pick an algorithm | PASS (weak) | The panel prints ALGORITHM and the header name changes in the channel colour. The ALGO_PICK list of all six (page_states) is not drawn, so the first-timer cannot see the choices. |
| Enter steps | PASS | The key bands show on-steps, and the footer says KEYS = STEPS / HOLD = EDIT. |
| Dial in with knobs | PASS | Labelled tiles with large values and in-tile meters. |
| Dice | PARTIAL | Nothing says "dice" before the first press. The die frame is an in-house convention and the six printed words FX..SEQ say nothing about dice. After a press, the toast "NOTES DICED / EDIT=UNDO" teaches well. |
| PLAY | PASS | ▶ on the PLAY tile and in the header. The tile does not switch to ■ while playing. |
| Morph | PARTIAL | ROW 1 passes. ROW 2 is unreachable from the screen: three 5 px squares with no link to OCT- and OCT+, which are drawn nowhere. |
| Dice a layer | PARTIAL | As for Dice above. The diced-step hatch disappears at true size. The toast does not say which channel was diced. |
| Mute | PARTIAL | The colour square on a black key reads as "channel n", not "mute". The X after muting confirms it. Hold = PREVIEW is undocumented. |
| Switch channel | PASS | Turn SELECT. The chips, algorithm name, tiles and key bands all recolour. The "+" chip is confusing (P2-13). |
| Undo | PASS | The undo arrow on EDIT plus the toast hint. |
| Change preset | PARTIAL | PRESETS is printed and P01 is shown, but the pending state (P05? frame) is not mocked and collides with long headers. SAVE-hold "undo load" is hidden behind the notch. |

## 5. Element verdicts

- **Header.** It works. 8 items fit in AF_M, and the channel chips are a good live-state device. Problems: a selected
  muted chip uses the unselected grammar (P1-7). "+" (P2-13). The muted strike is a 1 px diagonal, invisible at 35 cm.
  The tempo source is missing. Pending preset P0x? collides with "MIDI 16" for three of six algorithms (P2-12). The
  MIDI value has no OFF word (P0-1).
- **Knob tiles.** The best element. Label AF_M (MID on SURF 6.8:1), value AF_L in white on the fill (5.6:1 on CH3's
  olive, 8:1 or more elsewhere), and the meter is the tile. Faults: meters on enumerations are meaningless, and the font
  changes per value (P1-10). PICK renders "CAT 1" where knob_map specifies "HOUSE 3", and STAGE TOTAL's "(n)" is lost
  (P1-11).
- **Row marker.** Fails. 5x5 px squares (6'), always three although ROW 3 exists only with a RIG, and no link to OCT-
  and OCT+. ROW 2 is never shown on a full screen.
- **Music row.** Conceptually attractive, fails at size with more than 2 channels. 4-channel slots are 3 px wide
  (3.7'). The accent tick is 2 px, the diced hatch has a 1 px pitch, and muted dashes are 1 px on 1 px off; all of
  these collapse to tints at 35 cm. Muted CH3's dashed bars stand as tall as live bars and read as live clutter. Pitch
  is normalised across all channels jointly, so adding a channel rescales every other channel's heights. Slide
  (`sl`, ACID's signature) is in the data but never drawn. The fixed slot per step assumes every channel runs a common
  16-step grid, which per-channel LENGTH, ARP RATE 2X/3T and STAGE RDIV break (section 8).
- **Dice icons.** FX/ALL: a pipped die, the only one that reads as a die. SCL/NOTES: a note, clear. ENV/ACCNT: the ">"
  accent mark, clear to musicians. LFO/SHAPE: a sine, which reads as "LFO" (matching the print) but not as "register
  jumps, lengths, drift". ARP/ARTIC: a bar over a block, no meaning. SEQ/HITS: dots over a bar, reads as a keyboard or
  radiator. A dot-row-of-steps glyph would read as hits. Absent layers are supposed to draw dimmed (button_modes "dim
  if absent", algos_v8 docstring), but `buttons()` never dims (P2-15).
- **Other button icons.** EDIT undo arrow: good. GLO gear: acceptable for the SET page, but HARM on hold is invisible.
  HOME house: good. SAVE floppy: good. PLAY ▶: static (P2-14). REC ring, becoming a disc when armed: good. **Corner
  notch:** 4 px (4.9'), invisible at 35 cm, and it does not say what the hold does (P1-5).
- **Keybed footer.** True to the hardware's 3-2-3-2-1 layout, and the step bands plus playhead bar are excellent.
  Faults: the CHN glyph is unreadable (P0-3). Mute keys show a channel colour, not a mute meaning. S/F/R letters are
  ambiguous and hide that they are hold-only. The permanent footer text is AF_S (against the "AF_S only on held
  views" rule, decision 116), and "PAGE 1/2" is hard-coded grey AF_S although the viewed page is live state (P1-9).
- **Toast.** A strong pattern: inverted white bar, icon, verb, undo hint. It covers the whole header (play state,
  channel, preset) for its duration and does not name the channel (P2-11).
- **Turning state.** The double channel-colour frame is clear. The "where it was" ticks are 6x2 px (2.5'), invisible,
  and drop out of the mock scene anyway because the value shown is the default. Nothing indicates "commits at the next
  step".
- **REC.** The keybed becoming a piano with "REC: ALL KEYS PLAY NOTES" is good. The knob tiles are wrong (P0-2). Black
  keys are drawn dim although all 27 keys play. The header disc replaces the transport glyph, so you cannot tell
  whether the transport is running while armed.
- **MIDI OFF.** Fails (P0-1).

## 6. True-size readability at 35 cm (1 px = 1.23')

| Element | px | arcmin | Verdict |
|---|---|---|---|
| Knob values AF_L | 21 | 25.8 | Excellent |
| Knob labels, header, chips AF_M | 11 | 13.5 | Fine |
| Enum values in AF_M (FWD, LINE, 2 OCT) | 11 | 13.5 | Readable, but a visible drop next to AF_L neighbours |
| Footer AF_S, "MIDI" AF_S, S/F/R | 9 | 11.0 | Borderline. Permanent AF_S breaks decision 116. |
| Button icons, overall | 13-15 | 16-18 | Fine as silhouettes |
| Icon interior strokes | 2 | 2.5 | Below glance size; the inner symbol carries the meaning |
| Black-key glyphs (mute square, X, arrows) | 6-7 | 7.4-8.6 | Marginal |
| CHN DIN glyph | 5 | 6.1 | Invisible as a symbol |
| Row marker | 5 | 6.1 | Fails |
| Hold notch | 4 | 4.9 | Fails |
| Music bar width, 4 / 3 / 2 / 1 channels | 3 / 4 / 6 / 13 | 3.7 / 4.9 / 7.4 / 16 | Fails with 3-4 channels, marginal with 2 |
| Accent tick, turning ticks, MIDI-OFF dashes | 2 | 2.5 | Invisible |
| Mute strike, hatch, dash pattern | 1 | 1.2 | Invisible; the shape cue becomes a colour cue, against principle 4 |

## 7. Personas: one red flag each

- **First-timer.** The button block is icon-only, and four of its six dice icons do not say "dice". The first press
  of FX re-rolls every layer with no feedforward. They learn by damage, then undo.
- **Expert live performer.** During rapid dicing the toast hides the header for its whole duration, and "NOTES DICED"
  does not say which channel. The confirmation, a hatch on 3 px bars, is invisible at arm's length. They cannot verify
  what they just changed.
- **Deuteranope.** CH4's reddish purple simulates to grey (148,152,165), ΔE 19.7 from the muted-bar grey. In 3 px bars
  the dash pattern is gone, so a live CH4 and a muted channel look the same. CH1 orange vs CH3 yellow drops to ΔE 16.9
  and CH3 vs dice amber to ΔE 9.
- **Dana (one TB-3 on MIDI ch 2).** If any other channel is ever given MIDI 2, CH1 silently becomes OFF. Dana's screen
  then shows two tiny red dashes while the lane, keys and playhead carry on as if the TB-3 were playing. Her one setup
  step (CHN key, then K2) is also the least signposted control on the screen.

## 8. Extensibility verdict

**Partly met.** The knob, header and dice-label code is genuinely algorithm-agnostic: the four sheet renders draw all
seven algorithms' 14 rows, including the never-seen DEMO, through unchanged `knobs()`, and `check()` validates fits.
That is real progress. But the screen needs data that **AlgoDesc does not carry**, and the mock gets it by regex over
the prose column of knob_map.tsv:

| Screen need | In AlgoDesc? | Consequence |
|---|---|---|
| Label, min, max, default | yes (MorphDesc) | OK |
| Value names (FWD BWD PEND, LINE SKIP...) | **no** | The firmware has no source. Needs `const char *const *names` per morph, or a names table index. |
| Value formats ("%d OCT", "1/4", "HOUSE 3", "48 (32)", KEYS at 0) | **no** | The parser already gets PICK wrong ("CAT 1") and drops STAGE TOTAL's "(n)". Needs a format enum plus an optional callback `fmt(m, v, buf)`. |
| Meter or no meter (enums and lists) | no | Enum knobs draw a misleading fill. |
| Lane mode (pitched / single) | no (UI_EXTRA) | Fine as a flag, but it must enter AlgoDesc. |
| Short name | name[8] ≤ 6 chars | OK. The UI_EXTRA "PATT" override needs a field. |
| Absent dice layers | yes (layer[].n = 0) | **Not used**: the dice never dim. |
| Dice sub-labels | yes (sub[]) | Used only in the toast. Fine. |
| Fixed pitch scale per channel | `hi()` exists | Not used. The row normalises jointly, so heights jump when channels change. |
| Turn toasts (ARP "FIGURE n STEPS") | no | Lost. |

**What a new algorithm would break:** anything that emits more than one note per step (ARP DIR CHORD, AF_CHORD);
ratchets and repeats (PO_REP: invisible); long or tied notes (PO_HOLD, PO_TIE: drawn as one-step bars); glides
(PO_SLIDE_IN: never drawn); and any loop length or rate that is not one step per 16th (loop_len 2..64, ARP RATE
2X/3T, STAGE RDIV). The "fixed slot per step across channels" premise assumes all channels share one 16-step grid. In
polymeter, the slot at x = step 7 shows different moments in time for different channels. A value whose widest form
fits AF_M but varies across AF_L/AF_M will change font while being turned; `check()` validates fit, not stability.
Channel colours are hard-coded RGB on HI-CON only. The constraints require a token on all nine palettes plus a MONO
fallback, and on MONO the channel identity of the music row disappears entirely.

## 9. Priority issues

**P0**
1. **MIDI OFF / MIDI conflict is silent.** (`v8_midi_off`, header x 160-195; music row and keybed.) Fix: render
   "OFF" as text in an inverted T.REC chip ("MIDI OFF" in AF_M). Strike the channel chip. Draw that channel's bars and
   key bands as outlines (hollow = not sounding). When a MIDI assignment turns another channel OFF, toast "CH2 TOOK
   MIDI 2: CH1 OFF".
2. **REC shows the wrong knob roles.** (`v8_rec_armed`, tiles y 24-77.) page_states says REC's band is CURSOR ACC
   SLIDE OCT, but the screen shows DENS SLIDE JUMP ACCNT. Fix: drive the tiles from the page's knob table, not from the
   algorithm row, whenever the page is not PERF. Draw the black keys as live piano keys in REC. Keep a transport glyph
   next to the REC disc.
3. **The only setup step is undiscoverable.** (`v8_playing`, CHN key x 85-95, y 188-208.) Fix: put the selected
   channel's MIDI number on the CHN key itself (AF_S digits, "16" fits in 12 px if the key glyph area is widened by 2
   px). Give the header's MIDI field the same key-shaped background, so "this key = this field" is visible. With a
   fresh device (first-run), make the footer read "CHN KEY = MIDI CH".

**P1**
4. **Music row illegible at 3-4 channels.** (`v8_playing`, y 87-128.) Fix, which challenges the owner's "fixed slot per
   channel": give the selected channel the full step width (13 px pitched bars, an accent cap 3 px or more, slide as a
   joining ramp). Show the other used channels as one 3 px hit strip each beneath it, position = time, muted =
   outline. Scale pitch per channel from `hi()`. Draw each channel's steps at its own length or rate, or at least mark
   a channel that is not on the 16-grid.
5. **Hold actions invisible.** (All four notches, `v8_playing` buttons.) Fix: notch at least 6 px, and add
   feedforward: from 0.25 s of any hold, the toast bar names the hold ("HOLD: RESET", "HOLD: HARMONY", "HOLD: MENU",
   "HOLD: UNDO LOAD") before it fires.
6. **Amber means both dice and CH1.** (Button block vs CH1 meters.) Fix: dice tiles neutral (T.SURF, white glyph); let
   the die frame alone mean "dice". Keep hue exclusively for channels.
7. **Selected plus muted chip ambiguity.** (`v8_sel_muted`, header x 46-102.) Fix: mark selection with a position cue
   independent of state, for example a 3 px bar under the chip in white. Mute keeps grey plus strike (strike at least
   2 px).
8. **ROW 2 undiscoverable; marker too small.** (`v8_playing`, x 109-130, y 81-86.) Fix: replace the dots with AF_M "1
   2" (plus "3" only with a RIG) flanked by the OCT- and OCT+ glyphs, or prefix the tile label band with the row number.
   Render a full ROW 2 scene.
9. **Footer.** (y 228-238.) Drop "KEYS = STEPS" (the bands show it). Make the viewed page live and in AF_M white
   ("PG 1/2", taken from length and page state, not hard-coded).
10. **Enum knobs.** (Sheets: DIR, CURVE, SHAPE, PICK; `v8_arp` SHAPE and SPAN.) Fix: choose each knob's font once from
    its widest value, never per value. For enums, replace the fill with n position ticks.
11. **Value text not in the firmware contract.** (`sheet_row2_b` PICK "CAT 1".) Fix: extend MorphDesc or AlgoDesc
    with names and a format (section 8), and stop parsing prose.

**P2**
12. Pending preset frame (x 197) overlaps "MIDI 16" (ends x 203) for EUCLID, SNAKE and STAGE. Fix: right-anchor the
    MIDI block to the preset and drop "MIDI" to a key glyph when it would collide; add the case to `check()`.
13. "+" chip implies a SELECT "add channel" role that is not in button_modes or page_states (SELECT is clamped
    CH1-CH4), and "1 2 + 4" reads oddly. Fix: show unused channels as dim numerals.
14. The PLAY tile shows ▶ while playing. Fix: show ■ while running (what the press will do).
15. Absent layers are never dimmed. Fix: read `layer[].n == 0` and draw that die in T.DIM.
16. The toast hides the whole header and does not name the channel. Fix: include the channel chip at its left, or
    overlay the music row instead of the header.
17. Channel colours are not tokens. Fix: add T_CH1..T_CH4 to all nine palettes, with a MONO fallback (per-channel
    pattern: solid / outline / dotted / striped).

**P3**
18. ARTIC and HITS icons: redraw as a gate/slur glyph and a row of step dots.
19. S/F/R letters: mark as hold-only (a small notch, as on the buttons).
20. Mute keys: use a speaker glyph in the channel colour, so the key says "mute" rather than "channel".
21. Turning ticks: at least 4 px tall, or drop them.

## 10. What works

- The knob tiles: the right element at the right size, with contrast at or above 5.6:1 everywhere, label and value
  where the eye already is, and the meter costing no extra space.
- Feedback grammar: the pressed button lights white, the turned tile gets a frame, and the toast names the verb and
  the undo.
- The keybed footer: geometrically honest to the hardware, with the step bands and the playhead bar under the key
  being exactly "what the white keys do now".
- Channel chips in the header as the single place for selection and mutes (fix the muted-selected case).
- The Okabe-Ito choice and the shape-for-mute intent; the execution just needs larger marks.
- Real extensibility progress: one knob renderer for every algorithm, proven by the sheets, and a validator that
  checks fits.

## 11. Questions for the owner

1. What does "channel in use" mean, and what adds or removes one? The "+" chip implies SELECT adds a channel, which
   the tables do not define.
2. Will you accept the selected channel taking the music row's full width, with the other channels reduced to thin
   hit strips? That relaxes "a fixed slot per channel" in exchange for legible pitch, accent and slide.
3. Is the 2x6 button picture worth 50 px (21% of the screen) when its content never changes on PERF? Or may it shrink
   to two 18 px rows, with hold roles moved to a feedforward strip, to give the music row about 12 px more?
