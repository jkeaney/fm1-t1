# Assessment A, v9 main screen (independent design director)

Scope: the 20 v9 renders in `proposals/v9/out/`, judged at 1x (true pixels) and checked at 4x. Code was read only to
settle what a mark means (`v9.py` status(), music(), keybed(), topbar(); `icons_v9.py` BUTTONS; `algos_v9.py` hooks).
Viewing model: 8 px/mm at 35 cm, so **1 px = 1.23 arcmin**. AF_S 11.0', AF_M 13.5', AF_L 25.8'. Anything under 10' is
"second look only".

Measured zone budget (y px of 240): header 0-22 (9%), knob tiles 24-77 (22%), status line 77-91 (6%), **music row
92-128 (15%)**, button map 132-182 (21%), keybed footer + cue 188-240 (22%). The two static reference maps take 43% of
the screen; the only zone that moves with the music takes 15%.

---

## 1. Generic-look verdict

**Mostly an instrument, partly a launcher.** The header and knob tiles read as a confident hardware screen: AF_L values,
fill-as-meter, flat tiles, colour carrying channel identity, no rounded cards, no glow. It is nearer Elektron than a
phone app. The generic part is the 6x2 grid of identical grey tiles with centred white pictograms. At 1x it looks like a
toolbar or app launcher, and it carries the least changing information on the screen. The footer keybed is distinctive
and specific to this device. Verdict: **not generic overall**, but the button block is the weakest part visually and it
sits in the middle of the screen.

## 2. Nielsen heuristics (0-4)

| # | Heuristic | Score | Evidence |
|---|---|---|---|
| 1 | Visibility of system status | **3** | Transport glyph, BPM, channel chips, algorithm, `MIDI 2`, preset (`P02?` inverted when pending), playhead column, mute (`3` struck through and greyed, `x` on the mute key). Weak spots: the knob-row indicator is two unlabelled 7 px squares at x~115 in the status line (v9_playing, v9_arp_row2). The page map is unlabelled squares at the right. Other channels' activity is 2 px strips (2.5' tall). |
| 2 | Match with the real world | **3** | The footer draws the actual 16+11 keybed in its current mode, and REC turns it into a piano (v9_rec_armed). This is a strong, literal mapping. The button grid follows panel order. Against it: the HITS and ARTIC dice glyphs (ARP/SEQ tiles, a frame with a dot and a frame with dashes) map to nothing a producer knows. |
| 3 | User control and freedom | **2** | `EDIT=UNDO` is named right after a dice roll (v9_dice), which is good. But SELECT past the last channel creates one (v9_channel_added), and no render shows how to remove it. A pending preset shows no cancel route (SAVE hold). UNDO/REDO is a toggle, but the icon never changes to show which one the next tap does. |
| 4 | Consistency and standards | **2** | The die glyph means "re-roll" on FX but "NEW channel" in the add toast (v9_channel_added top bar). Hollow bars mean muted (grey) and MIDI OFF (red), which is consistent, yet in v9_midi_off the footer still draws CH2's steps solid blue. The value font switches between AF_L and AF_M inside one row (sheet_row1_a: TOTAL `16` in AF_M next to `6`, `2` in AF_L). The row and page indicators use the same square vocabulary for different things. |
| 5 | Error prevention | **2** | The hold bar fills to the fire point and turns red (topbar()), which is good. But one detent too many on SELECT adds a channel mid-set, and a MIDI conflict is only reported after the fact (v9_midi_off toast). |
| 6 | Recognition rather than recall | **2** | All 12 buttons are icons with no word. The three encoders (SELECT, PRESETS, ALGORITHM) and OCT-/OCT+ (the row buttons) appear nowhere on the main screen, so their roles must be recalled. Hold roles are named only once a hold has started (owner decision, but it is still recall). |
| 7 | Flexibility and efficiency | **3** | Every performance action is one gesture from this one screen. The knob tiles give large targets for fast tweaking. Hold hints keep a page from having to open. |
| 8 | Aesthetic and minimalist design | **2** | The knob tiles rightly dominate. The music row, which is the live state, is squeezed to 36 px between two static maps that together take 43%. In v9_playing the playhead column, accent caps (2 px) and slide bridges (2x3 px) are all at or below 5'. |
| 9 | Error recognition and recovery | **3** | The MIDI OFF toast names the channel, the state and the fix (`2 [DIN] MIDI OFF  CHN=SET`). The red `MIDI OFF` pill then persists in the header (v9_midi_off_after). This is the best-executed flow in the set. |
| 10 | Help and documentation | **2** | `KEYS = STEPS / HOLD = EDIT` and the HOME hold hint `TOUCH=HELP LET GO=MENU` help. Nothing explains the dice frame convention, the row squares or the encoders. |
| | **Total** | **24 / 40** | |

## 3. Cognitive-load checklist

| Check | Result |
|---|---|
| One obvious focal point | **Pass**. The AF_L knob values. |
| Live state outranks reference | **Fail**. The music row (15%) is smaller than either static map (21% and 22%). |
| Information chunked into clear zones | **Pass**. Six bands, and the order is stable across states. |
| Each mark has one meaning | **Fail**. The die means both re-roll and new; the squares mean both row and page; `+` is both a gap and an add. |
| Recognition, not recall | **Partial**. The keybed passes. The buttons, encoders and row control fail. |
| Feedback names the action and the reversal | **Pass**. The dice toast and the MIDI OFF toast. |
| Nothing needed mid-performance is under 10' | **Fail**. Hit strips 2.5', accent caps 2.5', list dots 3.7', row squares 8.6', mute-key swatches 7.4'. |
| Noise kept low | **Partial**. The status line carries three unrelated things (preset, row, page) with no labels. |

**Density verdict:** the total density is acceptable, but it is misallocated. The screen is dense where nothing changes
(the button and key maps) and sparse where the music happens.

## 4. Workflow test (from the screen alone)

| Step | Result | Why |
|---|---|---|
| Select CH1 | **Partial** | The chips imply a selector, but nothing names the SELECT encoder. |
| Set its MIDI channel | **Partial** | `MIDI 2` is readable. The route (CHN black key, then K2) is a 9 px DIN glyph on one footer key. It is taught only by the MIDI OFF toast, which a first-timer will not have seen. |
| Pick an algorithm | **Partial** | The name is in the header in channel colour. The ALGORITHM encoder is never named, apart from `ALGO=PICK` in the add toast. The ALGO_PICK list overlay is not drawn in this set. |
| Dial in with knobs | **Pass** | Labels and values are large, and the fill reads as a meter. |
| Dice | **Partial** | The ALL die reads at once. NOTES, ACCNT and SHAPE are guessable, but HITS and ARTIC are not, until a press names them in the toast. |
| PLAY | **Pass** | |
| Morph | **Pass** | The turning frame and old-value ticks show it (v9_turning_k2). |
| Dice a layer mid-play | **Pass** | Hatched bars plus a toast naming the layer and the undo. |
| Mute | **Pass** | Coloured swatch on keys M1-M4. Muted shows `x` and a struck-through chip. |
| Switch channel | **Partial** | Same encoder problem as selecting CH1. |
| Undo | **Pass** (tap) / **Partial** (redo state not shown) | |
| Change preset | **Partial** | `P01` and pending `P02?` are clear. The encoder is unnamed and cancel is not shown. |

Overall: **5 pass, 7 partial, 0 fail.** Every partial has the same root cause: **the left-hand controls (three encoders
and OCT-/OCT+) have no representation on the main screen**, even though the right-hand controls get a whole map.

## 5. Element verdicts

- **Header**: good. Chips with number and colour, strike-through for mute, `+` for add, and the red `MIDI OFF` pill are
  all strong. The `MIDI` caption in AF_S grey is fine because the digit is AF_M. In REC, the red dot replaces the
  transport glyph, so play/stop state is lost (v9_rec_armed).
- **Knob tiles**: the best element. Fill-as-meter works at a glance. The turning frame is clear. Problems: position dots
  are 3 px tall (3.7') and decorative at distance. AF_L and AF_M values mix in one row (`48(32)`, `2 OCT`, `16`). The
  old-value ticks are 6x2 px.
- **Status line**: weak. `P01` is fine. The row squares sit unlabelled and far from the tiles they qualify. The page
  squares are unlabelled, and a single square (v9_one_channel) reads as a stop glyph.
- **Music row**: the concept is right (selected channel full width, playhead column, hatched dice, hollow when silent).
  The execution is cramped: 33 px of bar height minus the strips. Other channels' strips are 2 px tall and identified
  only by colour and by a stack order that changes when the selection changes (music() builds `others` excluding
  `sel`).
- **Button map**: legible at 24 px tiles. The lighter dice tiles vs darker function tiles are a subtle but real grouping
  cue. Absent layers grey correctly (v9_demo_algo ENV). The pictograms for HITS and ARTIC fail. The tiles never show
  hold roles at rest (owner decision).
- **Hold hint bar**: good pattern. `HOLD EDIT: RESET` does not say what happens next (per page_states it should lead to
  "touch what to reset").
- **Toasts**: good. They show the channel chip, the action and the reversal. They cover the header, which is acceptable
  for 1-2 s.
- **Keybed footer**: excellent idea and well drawn. It switches to a piano in REC. The mute swatches are 6x7 px (about
  8'). `S F R` in AF_S just passes at 11'.

## 6. True-size readability at 35 cm

| Mark | px | arcmin | Verdict |
|---|---|---|---|
| Knob values AF_L | 21 | 25.8 | glanceable |
| Knob labels, header, cue AF_M | 11 | 13.5 | readable |
| `MIDI`, `S F R` AF_S | 9 | 11.0 | marginal, second look |
| Button pictograms | ~9-11 | 11-13.5 | shape readable, meaning not |
| Row-indicator squares | 7 | 8.6 | below glance |
| Mute-key swatches | 6x7 | 7.4-8.6 | below glance (colour is detectable, shape is not) |
| Value-list dots | 3 tall | 3.7 | invisible |
| Other-channel hit strips | 2 tall | 2.5 | visible as a line, colour unreliable |
| Accent cap, playhead cap | 2 | 2.5 | visible only on contrast |
| Slide bridge | 2x3 | 2.5-3.7 | invisible |

Accent and slide define ACID, and both are below 4'.

## 7. Personas: one red flag each

- **First-timer:** cannot tell what turns channels, algorithms or presets, or what the two coloured squares in the
  middle of the screen mean. The screen documents the right-hand panel and ignores the left.
- **Live performer:** the music row is the smallest live zone. Other channels' activity is a 2 px line, and accents and
  slides are 2 px marks. Mid-set they will watch the knobs and lose the groove view.
- **Deuteranope:** the Okabe-Ito set helps, but CH1 orange vs CH3 yellow, and CH2 sky blue vs CH4 reddish purple,
  collapse on 2 px strips and 6 px swatches. The strip order shifts when the selection changes. The strips carry
  channel identity by colour alone, which breaks PRODUCT.md principle 4.
- **Dana (one TB-3, MIDI 2):** one SELECT detent too far creates CH2 (v9_channel_added). A toast covers the header, a
  second strip appears, and no screen shows how to get back to one channel. The new channel's tile reads `HITS 5`
  while its lane shows one step. That is either a mock-data bug or a real contradiction.

## 8. Extensibility verdict

**Partial pass in the mock, not yet a firmware contract.** The screen code names no algorithm. Tiles, lists, absent
dice and lane type come from one record per algorithm, and DEMO proves the knob tiles, header and dice greying are
reached. But the record carries fields the firmware `AlgoDesc`/`MorphDesc` lacks:

- `MorphDesc` is `label/min/max/def/step/fast`, with no value-name list (`FWD`, `UP`, `MIN`, `AUTO`), no format
  (`%d OCT`, `CAT %d`) and no zero-word.
- `AlgoDesc` has no lane type (`single` / `pitched`).
- STAGE's `48(32)` needs a per-algorithm `value_fn` (algos_v9 `stage_total`). That is per-algorithm display code, in
  exactly the place the requirement forbids it.

The absent-layer greying does map onto `LayerDesc.n == 0` / `sub[] = "-"`. Extending the structs is a sized, asserted
contract change that needs an ADR amendment. The proposal does not cost it.

## 9. Priority issues

**P0: none.** Nothing drawn makes the workflow impossible. Every gap is a partial, not a fail.

**P1**

1. **Left-hand controls invisible.** All main renders, header and status line. Add a fixed micro-legend tied to the
   left edge (the encoders sit left of the screen). For example, the channel chips get a tiny `SEL` tag, the algorithm
   name a tiny `ALGO` tag, and the preset a `PRE` tag. Or put the encoder names in the cue line on first run and after
   idle.
2. **Row indicator unlabelled and misplaced.** Status line x 111-129 (v9_playing, v9_arp_row2). Move it to the left end
   of the knob-tile band as `R1`/`R2` text in AF_M (OCT-/OCT+ are left of the screen), or put `ROW 2` in the first
   tile's label bar while row 2 is live.
3. **Other-channel strips at 2.5', colour-only, order shifts.** Music row y 120-127. Give each strip a fixed slot by
   channel number, not by "others" order. Make them 3 px tall. Prefix each with a 1-character channel digit or offset
   them by a column. Or give the strips back 6-8 px from the button map.
4. **Music row starved, maps oversized.** Rows y 92-128 vs y 132-182. Shrink the button tiles to 20 px (still larger
   than an AF_M cap) and the keybed white-key area by 6 px. Give the 14 px gained to the lane so accent caps and slide
   bridges reach 3-4 px.
5. **Channel colours are hard-coded RGB, not theme tokens.** `CH` in v9.py, rendered on HI-CON. They need values on
   all nine palettes (PAPER is light) and a MONO fallback where channels differ by pattern or digit.
6. **Extensibility needs a struct change.** Define a `MorphDesc` extension (name-table index, format id, zero-word id)
   and an `AF_LANE_SINGLE` flag. Replace STAGE's `value_fn` with a generic "value(heard)" format the core fills. Cost
   the bytes.

**P2**

7. Die glyph reused for NEW (v9_channel_added top bar). Use a `+` glyph.
8. The add-channel state contradicts itself (`HITS 5`, one step drawn): fix the data or the behaviour. Also show how a
   channel is removed.
9. `HOLD EDIT: RESET` (v9_hold_edit) should read `HOLD EDIT: RESET, THEN TOUCH`.
10. REC hides the transport (v9_rec_armed, top-left). Show the red dot plus a play or stop glyph.
11. MIDI OFF channel steps are still solid in the footer (v9_midi_off, keybed). Draw them hollow, matching the lane.
12. The HITS and ARTIC pictograms (ARP and SEQ tiles). Redraw them as a pulse train and a short/long gate pair.
13. Mixed value fonts within a row (sheet_row1_a TOTAL, stage_heard). Pick AF_L wherever it fits, and never mix in one
    tile row unless the value cannot fit.

**P3**

14. Value-list dots 3 px: make them 5 px or drop them.
15. Old-value ticks while turning are 6x2 px. Use 3 px.
16. The UNDO icon should flip to REDO after an undo.
17. A single page square on 1-page patterns (v9_one_channel) reads as stop. Hide the map when there is one page.

## 10. What works

- Fill-as-meter knob tiles with AF_L values. This is the screen's anchor and glanceable at 26'.
- The keybed footer that changes with the keys' mode. It is literal, specific, and best in class for "what does this
  key do".
- The MIDI OFF flow: toast with cause and fix, then a persistent red pill with text, and hollow red bars. Colour is
  never alone here.
- Dice feedback: hatched re-rolled steps plus `NOTES  EDIT=UNDO`.
- Hold bar with progress to the fire point.
- Absent dice layers greyed from data, not code (DEMO ENV tile).

## 11. Owner questions (max 3)

1. Will you accept a small permanent legend for the left-hand encoders and OCT buttons, given that they are half the
   workflow and currently undocumented on the screen?
2. Should the static button map shrink (or the keybed footer lose height) so the music row can show accents, slides and
   other channels at a glanceable size?
3. Do you accept extending `MorphDesc` / `AlgoDesc` (value names, format, lane flag; an ADR-0003 amendment) as the price
   of "new algorithms reach every element without screen code"?
