<!-- SPDX-License-Identifier: GPL-3.0-only -->
# UI PROPOSAL v3.2: the glance screen, answering the critique of v3

Source: `proposal_v3_2.py` (every zone; it imports the DATA of v3.1: knob tables, enum pictures, SNAKE routes, palette tokens, fixtures), `render_all.py` (all PNGs, the before/after
sheets, the states sheets, the colour-blind test), `report_v3_2.py` (every table below; also writes `geometry_v3_2.tsv`, one row per element). Palette GREY unless a scene says otherwise.
Per scene `proposal_v3_2_SCENE.png` (1x), `_4x.png`, `_grid.png` (zones, cells and every text ink box outlined), and `SCENE_v3_1_vs_v3_2.png` (v3.1 left, v3.2 right, both 2x; the v3.1 side
is v3.1's own `draw()` on the same state, so where v3.1 has no such state the left side shows what v3.1 would draw, which is the answer to "undefined").
Inputs read: `critique_v3/ASSESSMENT_A_v3.md` (all), `critique_v3/EVIDENCE_B_v3.md` (all), `PRODUCT.md`, `docs/design/button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, `help_text.tsv`,
`ALGORITHM_SET.md` 4.2 / 4.4, `SCREEN_SPEC.md` 6 (clock states), `sq_types.h` (ratchet field). Not hardware-tested: the mock is pixel-exact for fonts and palettes only; it says nothing about
viewing angle, glare or the panel's real contrast. The tool printed **no warning** (no cut text, no ellipsis, no strip over 240x124) for any of the 71 scenes.

Viewing geometry used throughout (fixed by the brief): 240 px = 30 mm, 8 px/mm, 50 cm: AF_S capitals 9 px = 7.6', AF_M 11 px = 9.5', AF_L 21 px = 18'.

## What v3.2 is, in one paragraph

The four glance facts are unchanged (four knob values, the selected channel's pattern with a clear playhead, four mute states, the selected channel and algorithm). What is new is
**reassurance at the risky moments**: the bottom band is never empty and carries ONE AF_M line that names the control your hand is on and what it just did or will do next
(a printed-name tab, then the role: `[SCL] NOTES  EDIT = UNDO`); every toast leads with the printed button name (`[SCL] NOTES / CH1 NEW PITCHES`); the preset toast says what happens,
when and how to get out (`P04  LOADS AT BAR [bar] / SAVE: KEEP HERE  HOME: CANCEL`); UNDO says what it undid and that EDIT again redoes it. The lane keeps accent, slide and ratchet visible
under the playhead, a step with velocity 1..3 is no longer a rest, and a playhead on another page is a ghost column. The channel tile shows the MIDI channel (the first step of the TB-3 workflow).
The knob zone changes character on ROW 2 / ROW 3 (inverted label bands). The MAP is split into two AF_M pages. Every element sits on a stated, measured grid (99 of 99 elements on the module).

## Scenes (each: `proposal_v3_2_NAME.png`, `_4x.png`, `_grid.png`, and where marked `NAME_v3_1_vs_v3_2.png`)

- **Glance screens (asked: glance_idle, glance_playing, row2, row3 with a RIG device, mutes, stress, stress_row2, stress on PAPER / MONO / ICE)**: `glance_idle`, `glance_playing`, `row2`, `row3`, `mutes`, `stress`, `stress_row2`, `stress_pal_paper`, `stress_pal_mono`, `stress_pal_ice`, `stage_total`
- **The cue-follows-the-hand series (idle stopped / playing, each of the six dice presses, mute key held, solo, fill, roll, undo, OCT latch, SAVE hold, PG; `cue_table_1..3` = every cue string)**: `cue_idle_stopped`, `cue_idle_playing`, `cue_dice_all`, `cue_dice_notes`, `cue_dice_accnt`, `cue_dice_shape`, `cue_dice_artic`, `cue_dice_hits`, `cue_mute_held`, `cue_solo`, `cue_fill`, `cue_roll`, `cue_undo`, `cue_oct_latch`, `cue_save_hold`, `cue_page`, `cue_table_1`, `cue_table_2`, `cue_table_3`
- **Toasts (the six dice, the SNAKE no-ARTIC case, undo, redo, preset pending playing / stopped, loading, algorithm, MIDI channel / OFF)**: `toast_dice_all`, `toast_dice_notes`, `toast_dice_accnt`, `toast_dice_shape`, `toast_dice_artic`, `toast_dice_hits`, `toast_dice_artic_snake`, `toast_undo`, `toast_redo`, `toast_preset_pending`, `toast_preset_pending_stopped`, `toast_preset_loading`, `toast_algo`, `toast_midi_channel`, `toast_midi_off`
- **Lane fidelity (full lane with playhead on step 8 / step 1, velocity 1..3, ratchet 1..8 sweep, muted lane, playhead on another page)**: `lane_full_playhead`, `lane_full_playhead_first`, `lane_vel_low`, `lane_ratchet_sweep`, `lane_muted`, `lane_offpage`
- **States (the states sheet: `state_*`, also in `states_sheet_*.png`)**: `state_ext_sync`, `state_wait`, `state_ext_lost`, `state_rec_stopped`, `state_rec_playing`, `state_add_page`, `state_past_length`, `state_first_run`, `state_reset`, `state_sol`, `state_fil`, `state_rol`, `state_follow_off`, `state_midi_channel`, `state_sel_muted_off`
- **MAP (A = buttons, B = keys, the MENU countdown, a touched control)**: `map_A`, `map_B`, `map_A_row3`, `map_menu_countdown`, `map_touch`
- Other: 

Contact sheets (composed from the scenes above by `render_all.py`): `states_sheet_1x.png` and `states_sheet_2x.png` (15 states), `toast_dice_strip.png` (the six dice toasts + undo, redo, preset,
loading, MIDI channel), `cue_series_sheet.png` (the cue after each control), `cue_table_1..3` (every cue string in its real band), `lane_accent_test.png` (colour-blind and blur test),
`map_v3_1_vs_v3_2.png` (v3.1 one page | A | B).

## Required changes: verdict

| # | Change | Verdict |
| --- | --- | --- |
| P1-1 | Perform loop documented while playing: never-empty bottom band, hand-following AF_M cue (32 strings, all measured), dice toasts led by the printed name with a per-algorithm detail | Done. The toast slab grew from 32 to 40 px (cost). The cue has two states for dice (toast first, cue after) |
| P1-2 | High-stakes toasts: preset pending with consequence and exit, `P04>` loading, UNDONE / REDONE with channel and `EDIT AGAIN`, knob turn dismisses | Done; the preset toast needs the 40 px slab (3 facts); a 32 px AF_M-only form is described below |
| P1-3 | MAP: `UNDO L`, ROW 3, K1..K4 as a row, 3-2-3-2-1 visible, one hold encoding, numerals, split | Done and split into A (buttons) and B (keys); recommended; the panel's 2x6 button layout is lost (3x4) |
| P1-4 | Lane fidelity | Done; ratchet is now a digit (the comb could not be counted at 2 px) |
| P2-5 | Mode signal | Done (inverted label bands, ROW 1/2/3 cells, hot frame removed); the off-row names do NOT fit the strip (measured), they appear in the cue when OCT is pressed |
| P2-6 | Static vs missing facts | Done (numerals AF_M, MIDI channel on every tile, `--` for OFF, activity bar); costs 1 px tighter tile |
| P2-7 | Accent as a shape | Done (13x8 filled cap), tested in three colour-vision deficiencies, greyscale and blurred |
| P2-8 | Grid | Done: module 15 x 4 px, 99 of 99 elements (v3.1: 0 of 6 element families) |
| P3-9 | States sheet | 15 states drawn (EXT --, WAIT, EXT lost, REC armed stopped / playing, MIDI channel toast, FOLLOW off, ADD PAGE, past LENGTH, first run, RESET, SOL, FIL, ROL, selected+muted+OFF) |
| ALSO | slab, strike, pips, hollow bars, rests, enum names, OCTS | All done except two caveats listed under "What got worse" |

## 1. Finding by finding

"v3.1 fixed?" is judged from v3.1's `proposal_v3_1.py` and its renders (and the pixel checks of section 6 where a number is quoted), not from v3.1's NOTES. FIXED / PARTIAL / NOT / (n/a).

| # | Critique finding (source) | v3.1 fixed? | What v3.2 does | Evidence: render + measurement |
| --- | --- | --- | --- | --- |
| 1 | Bottom band is empty while playing; `HOLD HOME = MAP` disappears (A P1-1) | NOT (`draw_z4` returns when playing) | The cue band (y 220..236) is never empty: stopped `HOLD HOME = MAP`, playing `FX SCL ENV LFO ARP SEQ = DICE`, then the hand-following cue | `cue_idle_playing`, `cue_series_sheet.png`; non-BG pixels in y220..240 while playing: v3.1 0, v3.2 1269 |
| 2 | Six dice buttons, undo, mute keys, SOL/FIL/ROL, rows are unlabelled; printed names (FX SCL ENV LFO ARP SEQ) mean something else (A P1-1, Jordan, Dana) | NOT | The cue names the control by its printed name in a tab and says the role; every dice toast leads with `[PRINTED] ROLE`; idle-playing cue lists the six dice buttons | `cue_dice_*` (6), `cue_mute_held`, `cue_solo`, `cue_fill`, `cue_roll`, `cue_undo`, `cue_oct_latch`, `cue_table_1..3` (32 strings, widest 239 of 240 px) |
| 3 | The toast names the layer after the act but not the button, `NOTES` never connects to `SCL` (A P1-1) | NOT | Toast title = inverted AF_L tab with the printed name + the role: `[SCL] NOTES`, `[FX] ALL`, `[ENV] ACCNT`, `[LFO] SHAPE`, `[ARP] ARTIC`, `[SEQ] HITS` | `toast_dice_all/notes/accnt/shape/artic/hits`, `toast_dice_strip.png` |
| 4 | Dice detail was one stand-in string (`CH 1  NEW PITCHES`) for every layer; layer sub-labels per algorithm not drawn; absent layers (SNAKE ARTIC) not shown (B (i)) | NOT | Detail says what changed per algorithm x layer (36 strings from ALGORITHM_SET 4.2 / 4.4 / 13 / PATTERNS); the absent layer shows `NO ARTIC IN SNAKE` + `NOTHING CHANGED` with a dim tab | `toast_dice_artic_snake`; table `[dice]` below |
| 5 | Held SOL / FIL / ROL / mute show nothing (A Dana, B) | NOT | SOL: the other three tiles hatched (not muted: no chip); FIL: K1 label band inverted + up-triangle + the value it is being pushed to; ROL: a `2` on every sounding step; mute: the tile hollows, `M` chip, cue `CH2 MUTED TAP AGAIN = ON` | `cue_solo`, `cue_fill`, `cue_roll`, `cue_mute_held`, `state_sol/fil/rol` |
| 6 | Preset toast does not say the slot loads by itself, when, or how to cancel; the bar is unlabelled (A P1-2, A journey "deepest valley") | NOT (`SAVE = SAVE HERE`, bar unlabelled) | `P04  LOADS AT BAR [bar]` (playing) or `LOADS IN 1.5 S` (stopped), then `SAVE: KEEP HERE  HOME: CANCEL`; the bar sits right of the sentence that labels it; the header keeps `P04?` and a 2 px settle bar under it | `toast_preset_pending`, `toast_preset_pending_stopped` |
| 7 | No loading state `P16>` (B (h)) | NOT | Header `P04>`, toast `P04 [play triangle] LOADING / [SAVE] HOLD = UNDO LAST LOAD` | `toast_preset_loading` |
| 8 | Undo toast `UNDO / DICE NOTES`: undone or offered? which channel? redo? (A P1-2, journey valley) | NOT | `[EDIT] UNDONE`, `CH1 NOTES`, hint `EDIT AGAIN = REDO`; and a REDONE twin | `toast_undo`, `toast_redo` |
| 9 | A knob turn must dismiss any toast at once (A P1-2, Alex) | PARTIAL (spec only in v3.1 NOTES 7: dismissed ALGO / DICE / UNDO, NOT the preset toast) | Any K1..K4 turn dismisses every toast, the preset one too (its facts survive in the header: `P04?` + the 2 px settle bar); full table in section 4 | `toast_preset_pending` header; spec section 4 |
| 10 | The toast hides the knob values (A persona Dana) | FIXED in v3.1 (bottom band) | Kept: the slab is at y 196; the meters end at y 195 | `toast_dice_notes_v3_1_vs_v3_2.png` shows both |
| 11 | MAP: SAVE hold labelled `BACK`, button_modes.tsv says undo last preset load (A P1-3) | NOT (`BTN_ROWS` has `BACK`) | `UNDO L` (the tsv label; `UNDO LD` is 63 px and does not fit the 58 px box, `UNDO L` is 53) | `map_A`; `[evidence]`: v3.1 label `BACK` |
| 12 | MAP: OCT+ reaches ROW 3 (RIG row) but only ROW 2 shown (A P1-3) | NOT | OCT+ role `ROW+` (next row: 2, then 3); the strip shows ROW 3 when the channel has a device; the OCT cue lists the TB-3 ROW 3 names | `map_A`, `row3`, `cue_oct_latch` |
| 13 | MAP: every one of ~70 labels is AF_S (7.7') (A P1-3, Sam) | NOT (89 AF_S boxes) | Split in two: A = encoders, OCT, K1..K4, 12 buttons (33 AF_M strings = the roles, 22 AF_S = the printed names the panel already prints); B = keys (32 AF_M, 0 AF_S). Section 5 has the counts and the recommendation | `map_A`, `map_B`, `map_v3_1_vs_v3_2.png` |
| 14 | MAP: K1..K4 drawn as a vertical list, the panel has a row (A P1-3) | NOT | Four 58 px tiles in a row, both row names, the live one bright | `map_A` |
| 15 | MAP: black-key gaps 21/23 px make 3-2-3-2-1 invisible; tap/hold shading is a second encoding (A P1-3) | NOT (`BLACK_X` 0,21,42,65..; tap = KEY fill, hold-only = RAISE fill + boxed word) | Keys centred on the 15 px white-key pitch: 7 px inside a group, 22 px between groups; ONE hold encoding (a boxed word) for buttons and keys, all keys one fill | `map_B`; `[evidence]` |
| 16 | MAP: white-key numerals 10..16 touch (A P1-3; v3.1 made it worse, 13 px tiles) | NOT (worse) | Only the beat starts are numbered (1, 5, 9, 13) under the keys in AF_M, tiles shaded per beat | `map_B` |
| 17 | MAP: header `LET GO = MENU`; menu countdown | FIXED in v3.1 (countdown bar) | Kept (`MENU` + bar, AF_M) | `map_menu_countdown` |
| 18 | Accent, slide, ratchet marks vanish in the playing column (dark on dark); accent and plain bar identical there (B (b), A Sam) | NOT | Marks keep contrasting shapes in the playing column: dark cap, dark digit, slide tie in TEXT with a dark outline; accent is a cap, not a bar colour | `lane_full_playhead_v3_1_vs_v3_2.png`, `lane_full_playhead_first`; lane check below |
| 19 | The slide chain breaks at the playhead (15 px dark gap) (B (b)) | NOT | The tie stays TEXT through the playing column (dark outline top and bottom only) | longest BG run on the tie row: v3.1 15 px, v3.2 1 px |
| 20 | A step with velocity 1..3 is pixel-identical to a rest under the playhead (B (b)) | NOT | Min bar 4 px; rests are an empty column (baseline only) | differing pixels ON vel 1/2/3 vs REST under the playhead: v3.1 [0, 0, 0], v3.2 [44, 44, 44]; `lane_vel_low` |
| 21 | Ratchets 6, 7, 8 draw like x1; NOTES said capped at 4 (B (b)) | NOT | The firmware range is 1..4 (`sq_types.h`: `gr` bits 4..6 = 0 AUTO, 1..4; `knob_map.tsv` NOTE_EDIT K4 1x..4x); the lane draws the digit `min(count, 4)` (tool clamps); a ratchet shows its count as an AF_M digit | `lane_ratchet_sweep`; differing px 1 vs 6/7/8: v3.1 0 / 0 / 0, v3.2 61 each, 4 vs 5..8: 0 (capped) |
| 22 | The playhead on a page that is not viewed is a 4x4 px dot; 0 amber pixels in the lane (B (d), A Sam) | NOT | A ghost column in the lane (13x70, 2 px head-colour frame + 5 px cap) at the playing step, the header page box of the PLAYING page is a filled head-colour 13x14 box, the VIEWED page has a 13x4 TEXT bar under it | `lane_offpage`, `state_follow_off`; amber pixels in the lane: v3.1 0, v3.2 343 |
| 23 | Row bar `I` / `II` glyphs are debris (A "five things") | FIXED in v3.1 (`ROW 1` / `ROW 2`, AF_M) | Kept as 58x14 cells `ROW 1 / ROW 2 / ROW 3`, the live cell solid | `row2`, `row3` |
| 24 | ROW 2 looks like ROW 1 except a strip; K1 can move OFFSET (mode error) (A P2-4/5) | PARTIAL (labels, but the knob zone does not change) | Label bands inverted on ROW 2; ROW 3 = inverted bands + RAISE body + 2 px frame; the strip shows the live cell | `row2`, `row3`, `row2_v3_1_vs_v3_2.png` |
| 25 | Off-row names should be shown (A P2-5) | NOT | They do not fit the strip: 7 of 12 (algorithm, row) sets do not fit next to a ROW cell (table `[rows]`; without it all 12 fit, widest 201 px, but then ROW 2 / ROW 3 are not written). They appear in the cue when OCT is touched (`[OCT+] OFFSET PEDAL PUNCH DIR`, widest 239 px of 240) | `cue_oct_latch`; `[rows]`, `[cues]` |
| 26 | The 'hot knob' frame is undocumented and reads as selection (A P2-5, B 5.2) | NOT | Removed; the value updates in place | `[evidence]`: `hot` read 0 times |
| 27 | Channel numerals AF_L never change but outweigh live data (A P2-6) | NOT | Numerals AF_M | `glance_playing` |
| 28 | The MIDI channel (first step of the workflow) is nowhere on the glance screen (A P2-6, Dana, journey) | NOT | Every tile shows `[arrow]n` (a drawn arrow + the 1-based channel; `[arrow]16` fits); first-run draws an attention frame round the slot | `glance_playing`, `state_first_run` |
| 29 | The `OFF` word clips its 2 px frame (A P2-6, B (g)) | FIXED in v3.1 by removing the word | No word: `--` in the MIDI slot; the ring-slash is gone | `stress`, `mutes` |
| 30 | A selected AND muted AND OFF channel must show all states (B (g)) | FIXED in v3.1 (chip + ring + strike) | Dimmed numeral + `--` + solid `M` chip + name | `stress`, `state_sel_muted_off` |
| 31 | Activity square is amber on one tile and black on another (A "consistency") | NOT | ONE indicator in ONE colour: a 58x4 bar of the head colour under the tile, on BG, same on every tile | `glance_playing`, `[geometry]` |
| 32 | The strike-through reads as 'deleted' (A "five things") | NOT | Dimmed numeral + solid `M` chip (+ hollow tile). The strike is gone | `mutes`; section 7 says why I did not keep it |
| 33 | Cyan whole bar reads as a cursor, 1.15:1 against a plain bar (A P2-7, B 4) | NOT (v3.1 only moved the edited diamond) | Accent = a filled cap 13x8 (52 px); bars stay TEXT | cap vs LANE 6.25:1 on a GREY lane (4.30 on PAPER, 7.0 ICE); v3.1 bar vs TEXT 1.15:1 (GREY); `lane_accent_test.png` |
| 34 | Edited-step mark | PARTIAL (5x5 diamond at the column top, "smallest shape on the screen") | A 13x5 foot in the second colour (the baseline is 2 px): wider, in the same row as the baseline | `glance_playing` steps 3, 7; `lane_accent_test.png`; compare `glance_playing_v3_1_vs_v3_2.png` |
| 35 | Beat pips 6x3 px fail (A, B 3.1) | NOT | Alternating lane shade per beat (LANE / LANE + 12 % TEXT) | `glance_playing`; shade contrast 1.25 (PAPER) to 1.44:1 (weak: see section 8) |
| 36 | Muted hollow bars 1 px outline fail (A Sam) | NOT | 2 px outline (bars >= 6 px) in the muted-mark colour (3.5..10.9:1) | `lane_muted`, `stress` |
| 37 | Rest dash vs quiet note differ by brightness only (A Sam, B (b)) | NOT | Rest = empty column | `lane_vel_low` |
| 38 | Toast slab is the brightest object, bigger than the playhead (A "toy", B 4) | NOT | SURF slab, 2 px TEXT rule on top, no icon | `[evidence]` slab line; `toast_dice_notes_v3_1_vs_v3_2.png` |
| 39 | Pixel-art die and bent-pipe undo arrow (A) | NOT | No icons | any toast |
| 40 | Enum codes `UP` (forward), `P-P`, `RP+`, `OCV`, `THB`, `SNK3` (A "five things", Alex) | FIXED in v3.1 (pictures + firmware names) | Kept; pictures scaled to 42x16 so the name fits under them | `toast_algo`, `stress` (SHAPE OCTV), `row2` (DIR FWD) |
| 41 | ARP K3 renamed `OCTS` against the firmware table (A minor) | NOT | Back to `SPAN` (firmware); the rename is an owner question | `stress` |
| 42 | `KEYS ONLY` drawn `KEY` (A minor) | NOT | `KEYS` in AF_M (68 px in AF_L does not fit 56) | `[text_glance]` knob value (M) |
| 43 | STAGE `TOTAL` drops the `(n)` steps heard (A minor) | NOT | `(32)` in the meter slot when fewer steps are heard than TOTAL | `stage_total` |
| 44 | Nothing sits on the 5 px module (0 of 35); lane group edges 1 px off the cell edges (B 5) | PARTIAL (lane group edges fixed; 0 of 6 element families on the 15 x 4 module, remainders in section 6) | Module 15 x 4 px stated and measured: 99 of 99 elements on it | `[geometry]`, `geometry_v3_2.tsv` |
| 45 | Text not centred by ink box; 1 px offsets (A, B 5.2) | PARTIAL (number/name ink edge shared) | Rule + measured table (`[centring]`) | `[centring]` |
| 46 | Channel number 14 px off centre, 1-2 px gap number to name (B 5.2) | FIXED in v3.1 (left group) | Kept: one ink-left edge (x + 4) for numeral and name | `[centring]` |
| 47 | `EXT --`, `WAIT`, `EXT lost` undefined (B (f)) | NOT | Header `EXT --` / hollow triangle + `WAIT`; EXT lost = back to the own tempo + cue `CLOCK LOST: OWN TEMPO 120` | `state_ext_sync`, `state_wait`, `state_ext_lost` |
| 48 | REC armed stopped vs playing draw the same disc (B (f)) | NOT | Ring (armed, stopped) vs solid disc (playing); the cue says what REC does | `state_rec_stopped`, `state_rec_playing` |
| 49 | FOLLOW off / viewing a non-playing page; ADD PAGE prompt; steps past LENGTH; first run; RESET latched (A minor, B (k)) | NOT (none defined) | Drawn | `state_follow_off`, `state_add_page`, `state_past_length`, `state_first_run`, `state_reset` |
| 50 | MIDI-channel change toast undefined (B (i)) | NOT | `MIDI 2 / CH1 SENDS ON MIDI CHANNEL 2`; `MIDI OFF / CH1 SENDS NOTHING` | `toast_midi_channel`, `toast_midi_off` |
| 51 | Algorithm toast: keep (A "what's working") | (kept) | Name AF_L, `ALGORITHM CH1` + the six position pips back (v3.1 dropped them) | `toast_algo` |
| 52 | Stress: ARP selected AND muted AND OFF, EXT 126, P16, L64 page 4/4 | FIXED in v3.1 | Rebuilt for v3.2 | `stress`, `stress_row2`, `stress_pal_paper/mono/ice` |
| 53 | Palette tokens; PAPER contrast | FIXED in v3.1 | The new shades are palette-derived (`ux_mix`): lane shade, muted-mark colour, dimmed numeral; measured on all nine | `[contrast]`, `stress_pal_*` |

## 2. Text items (font, cap height, arcmin at 50 cm, contrast)

Cap mm = px / 8; arcmin = mm / 500 x 3438. Text needs 4.5:1 (AF_L 3:1). Worst = the lowest fg / bg over every instance in the scenes listed. **Distinct text roles on the glance screen while playing: 11** (see `What got worse`, item 2): algo name, chan numeral, cue, knob label, knob value, midi slot, page numeral, preset, ratchet digit, row label, tempo. **AF_S on any glance screen or toast: 0.**

### 2a. Glance screens and toasts

| role | font | strings drawn (all glance scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need |  max instances in one scene |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| tempo | M | `120, EXT 126` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| preset | M | `P01, P16, P04?` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| page numeral | M | `1, 2, 3, 4` | 11 | 1.38 | 9.5 | 5.23:1 | 4.5 ok | 4 |
| chan numeral | M | `1, 2, 3, 4` | 11 | 1.38 | 9.5 | 5.37:1 | 4.5 ok | 4 |
| midi slot | M | `2, 3, 4, 10, --, 16` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 4 |
| algo name | M | `ACID, EUCLID, SNAKE, STAGE, PATT, ARP` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 4 |
| ratchet digit | M | `3, 2, 4` | 11 | 1.38 | 9.5 | 3.54:1 | 4.5 LOW | 4 |
| knob label | M | `DENS, SLIDE, JUMP, ACCNT, OFFSET, PEDAL, PUNCH, DIR ...` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 4 |
| knob value | L | `12, 6, 5, 3, 0, 4, 40, 74 ...` | 21 | 2.62 | 18.0 | 7.19:1 | 3.0 ok | 4 |
| row label | M | `ROW 1, ROW 2, ROW 3` | 11 | 1.38 | 9.5 | 5.23:1 | 4.5 ok | 3 |
| cue | M | `HOLD HOME = MAP, FX SCL ENV LFO ARP SEQ = DICE, OFFSET PEDAL PUNCH DIR, CH2 MUTED  TAP AGAIN = ON, KEYS WRITE NOTES, OFFSET LEGATO DYN DIR` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| knob value (M) | M | `FWD, 7, OCTV, CHORD` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 1 |
| cue chip | M | `OCT+, M2, REC` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| mute chip | M | `M` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 2 |
| toast chip | L | `SCL, EDIT` | 21 | 2.62 | 18.0 | 12.33:1 | 3.0 ok | 1 |
| toast title | L | `NOTES, P04, UNDONE, EUCLID` | 21 | 2.62 | 18.0 | 10.03:1 | 3.0 ok | 1 |
| toast detail | M | `CH1 NEW PITCHES, LOADS AT BAR, SAVE: KEEP HERE  HOME: CANCEL, CH1 NOTES, ALGORITHM  CH1` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 2 |
| toast hint | M | `EDIT: UNDO, EDIT AGAIN = REDO` | 11 | 1.38 | 9.5 | 5.23:1 | 4.5 ok | 1 |

per scene: strings drawn, by font; roles in the scene
| scene | strings | AF_L | AF_M | AF_S | distinct roles |
| --- | --- | --- | --- | --- | --- |
| glance_idle | 28 | 4 | 24 | 0 | 11 |
| glance_playing | 28 | 4 | 24 | 0 | 11 |
| row2 | 29 | 3 | 26 | 0 | 13 |
| row3 | 29 | 4 | 25 | 0 | 11 |
| mutes | 32 | 3 | 29 | 0 | 14 |
| stress | 35 | 3 | 32 | 0 | 14 |
| stress_row2 | 35 | 3 | 32 | 0 | 14 |
| toast_dice_notes | 29 | 6 | 23 | 0 | 13 |
| toast_preset_pending | 28 | 5 | 23 | 0 | 11 |
| toast_undo | 29 | 6 | 23 | 0 | 13 |
| toast_algo | 27 | 5 | 22 | 0 | 11 |


### 2b. The MAP

- map_A: 55 strings: AF_M 33, AF_S 22, AF_L 0
| role | font | strings | cap mm | arcmin @50 cm | worst contrast | n |
| --- | --- | --- | --- | --- | --- | --- |
| map title | M | `TOUCH = HELP` | 1.38 | 9.5 | 12.33:1 | 1 |
| map page hint | M | `OCT+ = KEYS >` | 1.38 | 9.5 | 6.42:1 | 1 |
| map printed | S | `MASTER, SELECT, PRESETS, ALGO, OCT-, OCT+, K1, K2, K3 ...` | 1.12 | 7.7 | 5.23:1 | 22 |
| map role | M | `VOL, CH 1-4, P01-16, OF CH 1, ROW 1, ROW+, ALL, NOTES, ACCNT ...` | 1.38 | 9.5 | 10.03:1 | 18 |
| map legend | M | `= HOLD` | 1.38 | 9.5 | 12.33:1 | 1 |
| map knob row1 | M | `DENS, SLIDE, JUMP, ACCNT` | 1.38 | 9.5 | 10.03:1 | 4 |
| map knob row2 | M | `OFFSET, PEDAL, PUNCH, DIR` | 1.38 | 9.5 | 5.23:1 | 4 |
| map hold | M | `RESET, HARM, MENU, UNDO L` | 1.38 | 9.5 | 10.03:1 | 4 |

- map_B: 32 strings: AF_M 32, AF_S 0, AF_L 0
| role | font | strings | cap mm | arcmin @50 cm | worst contrast | n |
| --- | --- | --- | --- | --- | --- | --- |
| map title | M | `TOUCH = HELP` | 1.38 | 9.5 | 12.33:1 | 1 |
| map page hint | M | `< OCT- = BUTTONS` | 1.38 | 9.5 | 6.42:1 | 1 |
| map black key | M | `M1, M2, M3, M4, CHN, PG-, PG+, RST` | 1.38 | 9.5 | 12.33:1 | 8 |
| map black key hold | M | `SOL, FIL, ROL` | 1.38 | 9.5 | 12.33:1 | 3 |
| map white key | M | `1, 5, 9, 13` | 1.38 | 9.5 | 12.33:1 | 4 |
| map caption | M | `TAP = STEP ON/OFF, = EDIT THAT NOTE` | 1.38 | 9.5 | 12.33:1 | 2 |
| map hold | M | `HOLD` | 1.38 | 9.5 | 12.33:1 | 1 |
| map key word | M | `ACCENT, SLIDE, 3, RATCHET 2-4, EDITED, M, MUTED, 2, MIDI OUT ...` | 1.38 | 9.5 | 6.42:1 | 12 |

v3.1 map: 89 text boxes drawn, every one AF_S except the MENU word (see v3.1 NOTES (b)).

tightest ink widths (>= 56 px in a 58 px tile): map_A map role `CANCEL` 57


## 3. Element geometry: the module, stated and measured

**Module: 15 px horizontally by 4 px vertically.** One lane step (= one white key) is 15 px; a channel / knob column is 4 steps = 60 px; zone heights are multiples of 4. Every element is a whole number of modules
minus a 1 px inset on each side, so two neighbours always have exactly a 2 px gap (tiles: x = 60 k + 1 wide 58; lane columns: x = 15 n + 1 wide 13; tile rows: y = 4 m + 1, height 4 n - 2). Four kinds are exempt on one axis and are
checked on the other: `bar` (activity bar, page bar: flush vertically), `cue` (full-width: flush horizontally), `toast` (flush on both: 240 x 40 at y 196) and `keybed` (black keys: centred on a module line, 8 px wide, so the pair
gap inside a group is 7 px). What is NOT on the module, by design: text (its width follows the ink, its cell is on the module), glyph interiors (the arrow, the transport shapes' sides, the hatch), lane marks inside a column (rows
1..8 accent, 10..14 slide, 16..26 digit, bars from 28, baseline 64..65, foot 64..68), the cue chip (width = ink + 4) and the toast tabs.

| element family | kind | n | x | y | w | h | x rem | w rem | y rem | h rem | on module |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| transport | shape | 1 | 1 | 5 | 13 | 14 | 0 | 0 | 0 | 0 | yes |
| tempo cell | text | 1 | 16 | 5 | 58 | 14 | 0 | 0 | 0 | 0 | yes |
| preset | text | 1 | 121 | 5 | 58 | 14 | 0 | 0 | 0 | 0 | yes |
| page box | shape | 7 | 181..226 | 5 | 13 | 14 | 0 | 0 | 0 | 0 | yes |
| page bar | bar | 7 | 181..226 | 20 | 13 | 4 | 0 | 0 | 0 | 0 | yes |
| chan tile | tile | 4 | 1..181 | 25 | 58 | 38 | 0 | 0 | 0 | 0 | yes |
| activity bar | bar | 4 | 1..181 | 64 | 58 | 4 | 0 | 0 | 0 | 0 | yes |
| lane col | col | 16 | 1..226 | 73 | 13 | 70 | 0 | 0 | 0 | 0 | yes |
| knob tile | tile | 4 | 1..181 | 145 | 58 | 58 | 0 | 0 | 0 | 0 | yes |
| row cell | tile | 3 | 1..121 | 205 | 58 | 14 | 0 | 0 | 0 | 0 | yes |
| cue line | cue | 1 | 0 | 221 | 240 | 14 | 0 | 0 | 0 | 0 | yes |
| toast slab | toast | 1 | 0 | 196 | 240 | 40 | 0 | 0 | 0 | 0 | yes |
| map tile | tile | 18 | 1..181 | 17 | 58 | 30 | 0 | 0 | 0 | 0 | yes |
| map tile K | tile | 4 | 1..181 | 81 | 58 | 42 | 0 | 0 | 0 | 0 | yes |
| map black key | keybed | 11 | 11..221 | 17 | 8 | 14 | 0 | 0 | 0 | 0 | yes |
| map white key | col | 16 | 1..226 | 81 | 13 | 26 | 0 | 0 | 0 | 0 | yes |

elements measured: 99, on the module: 99 (0 off). Module = 15 px (x) by 4 px (y); remainder columns are the worst over the instances of the family.

Full per-element list (scene, name, kind, x, y, w, h, remainders): `geometry_v3_2.tsv`. Text centring (rule and measurement):

| text | font | cell w x h | ink w | margin L / R | cap h | margin T / B |
| --- | --- | --- | --- | --- | --- | --- |
| `ACCNT` | M | 58x18 | 50 | 4 / 4 | 11 | 3 / 4 |
| `OFFSET` | M | 58x18 | 54 | 2 / 2 | 11 | 3 / 4 |
| `12` | L | 58x21 | 32 | 13 / 13 | 21 | 0 / 0 |
| `40` | L | 58x21 | 34 | 12 / 12 | 21 | 0 / 0 |
| `ROW 1` | M | 58x14 | 43 | 7 / 8 | 11 | 1 / 2 |
| `ROW 3` | M | 58x14 | 46 | 6 / 6 | 11 | 1 / 2 |
| `3` | M | 13x14 | 9 | 2 / 2 | 11 | 1 / 2 |
| `M` | M | 14x14 | 12 | 1 / 1 | 11 | 1 / 2 |
| `EDIT` | M | 35x14 | 31 | 2 / 2 | 11 | 1 / 2 |
| `OCT+` | M | 42x14 | 38 | 2 / 2 | 11 | 1 / 2 |
| `FWD` | M | 58x11 | 32 | 13 / 13 | 11 | 0 / 0 |

Measured on the rendered screens (ink boxes of the real glyphs, `screen.texts`, against the cell they were centred in):
| scene | text | cell x,y,w,h | ink box | margin L / R | margin T / B |
| --- | --- | --- | --- | --- | --- |
| glance_playing | `ACCNT` | 181,145,58,18 | 185,147..235,160 | 4 / 4 | 2 / 3 |
| row2 | `OFFSET` | 1,145,58,18 | 3,147..57,160 | 2 / 2 | 2 / 3 |
| glance_playing | `ROW 1` | 1,205,58,14 | 8,205..51,218 | 7 / 8 | 0 / 1 |
| stress | `4` | 226,5,13,14 | 228,6..237,17 | 2 / 2 | 1 / 2 |
| mutes | `M` | 103,29,14,14 | 104,30..116,41 | 1 / 1 | 1 / 2 |
| glance_playing | `ACID` | 1,25,58,38 | 5,46..38,59 | 4 / 21 | 21 / 4 |
(the tool's ink box includes the glyph's anti-aliasing rows, so it is 1 row taller above and below than the 11 px cap box used by the rule; the algorithm name is left-aligned in its tile on purpose.)

The channel tile is a left-aligned group, not a centred cell: numeral and algorithm name share one ink-left edge (x + 4), the MIDI slot starts at x + 14 (arrow) / x + 23 (digits), the chip's right edge is x + 56.

## 4. Toast and cue specs

### 4a. Toasts (all in the bottom slab: 240 x 40 at y 196..236, SURF, 2 px TEXT rule on top; title AF_L cap 21 at y 200, detail AF_M cap 11 at y 224)

Why 40 px and not v3.1's 32: a toast with an AF_L title AND a one-line AF_M consequence needs 21 + 3 + 11 + the rule = 37 px; 32 px only holds one row. The knob tiles end their content at y 195 (value, picture
and meter; tile padding to y 203), so the slab covers only empty tile bottoms and the ROW strip and cue (rows that are replaced anyway). The preset toast has three facts (title + when + a labelled bar, then two actions) and does not
fit 32 px in any form with an AF_L title; its AF_M-only 32 px fallback (`P04  LOADS AT BAR [bar]` on the first row, `SAVE: KEEP HERE  HOME: CANCEL` on the second, 233 px of 238) would work if the owner prefers
no 8 px growth, at the price of a small `P04` (the header keeps `P04?` in AF_M anyway). Not rendered; the strings and widths are in `[cues]` / below.

| kind | trigger | title (AF_L) | detail (AF_M) | right hint (AF_M, only if it fits) | duration | dismissed by |
| --- | --- | --- | --- | --- | --- | --- |
| dice | FX / SCL / ENV / LFO / ARP / SEQ press | `[PRINTED] ROLE`, e.g. `[SCL] NOTES` (inverted tab with the printed name) | `CH1 NEW PITCHES` etc. (36 strings, `[dice]`); absent layer: `NO ARTIC IN SNAKE`, dim | `EDIT: UNDO` (fits for 14 of 36; otherwise the cue shows it) / `NOTHING CHANGED` | 1.2 s | K1..K4 turn, SELECT turn, ALGO / PRESET turn (replaced), HOME hold, any new toast |
| undo / redo | EDIT tap | `[EDIT] UNDONE` / `[EDIT] REDONE` | what: `CH1 NOTES`, `CH1 STEP 5`, `CH1 PAGE 2`, `RESET` ... (<= 90 px) | `EDIT AGAIN = REDO` / `= UNDO` | 1.2 s | as dice |
| algo | ALGO detent | the algorithm (AF_L, widest `PATTERNS` 135 px) | `ALGORITHM  CH1` + six position pips right | | 1.2 s after the last detent, replaced per detent | as dice |
| preset pending | PRESET detent | `P04` | `LOADS AT BAR` (playing) / `LOADS IN 1.5 S` (stopped) + a labelled bar (outlined, fills over the settle time: 1.5 s, or until the first bar that starts >= 1.5 s after the last detent); second row `SAVE: KEEP HERE  HOME: CANCEL` | | until it loads or is cancelled (<= settle, longer while the user keeps turning) | PRESET turn (updates), K1..K4 turn (toast gone, header keeps `P04?` + a 2 px settle bar), HOME tap (cancel), SAVE tap (save into P04, no load), SAVE hold 0.7 s (cancel), turning back to the current slot |
| preset loading | the bar line / the stopped settle ends | `P04 [triangle]` + `LOADING` | `[SAVE] HOLD = UNDO LAST LOAD` | | 1.5 s after the load applies (next step) | K1..K4 turn, any new toast |
| MIDI channel | CHAN K2 turn (or MIDI learn) while PERF shows | `MIDI 2` / `MIDI OFF` | `CH1 SENDS ON MIDI CHANNEL 2` / `CH1 SENDS NOTHING` | | 1.2 s | as dice |
| text (PAST THE END, ADD PAGE n) | tap of a dim step; PG+ on the last page | `PAST THE END` / `ADD PAGE 3` | `CHN K1 = LENGTH` / `PG+ AGAIN: COPY OF 2  HOME: NO` | `20 STEPS` | 1.2 s / until PG+, HOME, or 3 s | as dice (ADD PAGE: only PG+ again, HOME, or the timeout) |

One toast slot, no queue: a new toast replaces the current one and restarts the timer. A toast is never shown under the MAP or the MENU. The knob row (labels, values, pictures, meters) is never covered.

### 4b. The cue (one AF_M line, y 221..235, follows the hand)

- **Content**: `[PRINTED NAME TAB] role text`. The tab is the name printed on the panel (`FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCT- OCT+`) or the key (`M1..M4 CHN SOL FIL ROL PG- PG+ RST`) in INK on a TEXT block flush at x 0 (2 px side padding); the role follows after 4 px.
- **Priority**: a control held now > the last control pressed (3 s after release) > idle. Idle = `HOLD HOME = MAP` (stopped) / `FX SCL ENV LFO ARP SEQ = DICE` (playing). A toast on screen hides the cue (same band); when the toast ends the cue of the same press is shown for the rest of its 3 s.
- **Events**: tap of a dice button: toast 1.2 s, then `[SCL] NOTES  EDIT = UNDO` until 3 s; held mute / SOL / FIL / ROL: the cue only (no toast) while held; OCT+ / OCT-: the cue lists the four knob names of the row the press goes to (also while OCT is held as a peek); SAVE hold, HOME, PG, RST, CHN, PLAY, REC: the cue only.
- **Every string** (width = tab + 4 + text; limit 240; the assertion in `draw_cue` fails the render if one is over): table below. The OCT cue is built from the knob tables (`cue_oct`); the widest is EUCLID to ROW 1: 239 px.

| key | chip | text | total px (chip + 4 + text) | limit 240 |
| --- | --- | --- | --- | --- |
| idle_stopped | - | HOLD HOME = MAP | 133 | ok |
| idle_playing | - | FX SCL ENV LFO ARP SEQ = DICE | 221 | ok |
| dice_all | FX | ALL LAYERS  EDIT = UNDO | 204 | ok |
| dice_notes | SCL | NOTES  EDIT = UNDO | 178 | ok |
| dice_accnt | ENV | ACCNT  EDIT = UNDO | 183 | ok |
| dice_shape | LFO | SHAPE  EDIT = UNDO | 178 | ok |
| dice_artic | ARP | ARTIC  EDIT = UNDO | 174 | ok |
| dice_hits | SEQ | HITS  EDIT = UNDO | 164 | ok |
| undo | EDIT | UNDONE  AGAIN = REDO | 205 | ok |
| redo | EDIT | REDONE  AGAIN = UNDO | 205 | ok |
| mute | M2 | CH2 MUTED  TAP AGAIN = ON | 231 | ok |
| unmute | M2 | CH2 ON  TAP AGAIN = MUTE | 221 | ok |
| solo | SOL | ONLY CH1 SOUNDS | 170 | ok |
| fill | FIL | DENSITY UP WHILE HELD | 198 | ok |
| roll | ROL | STEPS REPEAT 2X-4X | 183 | ok |
| pg_next | PG+ | PAGE 2 OF 4  FOLLOW OFF | 215 | ok |
| pg_add | PG+ | AGAIN = ADD PAGE 2 | 178 | ok |
| restart | RST | ALL CHANNELS TO STEP 1 | 212 | ok |
| oct_peek | OCT+ | HELD: ROW 2 LIVE | 169 | ok |
| save_hold | SAVE | HOLD = UNDO PRESET LOAD | 239 | ok |
| save_tap | SAVE | SAVED TO P01 | 140 | ok |
| home | HOME | TAP: CANCEL  HOLD: MAP | 227 | ok |
| edit_hold | EDIT | RESET: TOUCH A CONTROL | 226 | ok |
| rec_stopped | REC | PLAY KEYS TO WRITE NOTES | 233 | ok |
| ext_sync | - | WAITING FOR THE HOST TEMPO | 219 | ok |
| wait | PLAY | WAITING FOR HOST START | 226 | ok |
| ext_lost | - | CLOCK LOST: OWN TEMPO 120 | 214 | ok |
| follow_off | PG- | VIEW 2 OF 4  FOLLOW OFF | 213 | ok |
| past_end | - | PAST THE END: CHN K1 = LENGTH | 232 | ok |
| first_run | CHN | K2 = MIDI CHANNEL | 175 | ok |

OCT+ / OCT- cue (chip + the four names of the row the press goes to), every algorithm and row:
| algorithm | to ROW 1 | px | to ROW 2 | px | ROW 3 (TB-3 profile) | px |
| --- | --- | --- | --- | --- | --- | --- |
| ACID | DENS SLIDE JUMP ACCNT | 223 | OFFSET PEDAL PUNCH DIR | 228 | CUT RESO ENV-X PAD-Z | 210 |
| EUCLID | HITS OFFSET RATCH ACCNT | 239 | LEN SPAN PUNCH CURVE | 218 | CUT RESO ENV-X PAD-Z | 210 |
| SNAKE | DENS LEN RANGE PATH | 205 | OFFSET GLEN DYN DIR | 200 | CUT RESO ENV-X PAD-Z | 210 |
| STAGE | DENS TOTAL DIR RATCH | 208 | OFFSET RDIV DYN RANGE | 220 | CUT RESO ENV-X PAD-Z | 210 |
| ARP | DENS SHAPE SPAN RATE | 213 | OFFSET LEGATO DYN DIR | 217 | CUT RESO ENV-X PAD-Z | 210 |
| PATT | DENS VARY SPAN ACCNT | 218 | PICK SLIDE DYN CTONE | 205 | CUT RESO ENV-X PAD-Z | 210 |

widest OCT cue = 239 px of 240 (chip flush at x 0, 2 px padding each side, 4 px gap).

### 4c. Dice toast strings, every algorithm and layer

Title width = tab (printed name + 8) + 8 + role in AF_L (limit 232). Detail limit 234 px.

| algorithm | layer | printed | toast title (AF_L tab + role) | detail (AF_M) | detail px | + `EDIT: UNDO` fits |
| --- | --- | --- | --- | --- | --- | --- |
| ACID | ALL | FX | FX: ALL (title 98 px) | CH1 EVERY LAYER NEW | 163 | no |
| ACID | NOTES | SCL | SCL: NOTES (title 156 px) | CH1 NEW PITCHES | 129 | yes |
| ACID | ACCNT | ENV | ENV: ACCNT (title 165 px) | CH1 NEW ACCENTS | 137 | yes |
| ACID | SHAPE | LFO | LFO: SHAPE (title 154 px) | CH1 NEW OCTAVE JUMPS | 178 | no |
| ACID | ARTIC | ARP | ARP: ARTIC (title 148 px) | CH1 NEW GATES + SLIDES | 179 | no |
| ACID | HITS | SEQ | SEQ: HITS (title 128 px) | CH1 NEW HIT PATTERN | 158 | no |
| EUCLID | ALL | FX | FX: ALL (title 98 px) | CH1 EVERY LAYER NEW | 163 | no |
| EUCLID | NOTES | SCL | SCL: NOTES (title 156 px) | CH1 NEW PITCHES | 129 | yes |
| EUCLID | ACCNT | ENV | ENV: ACCNT (title 165 px) | CH1 NEW ACCENTS | 137 | yes |
| EUCLID | SHAPE | LFO | LFO: SHAPE (title 154 px) | CH1 NEW MOD LANES | 153 | no |
| EUCLID | ARTIC | ARP | ARP: ARTIC (title 148 px) | CH1 NEW RATCHETS | 143 | no |
| EUCLID | HITS | SEQ | SEQ: HITS (title 128 px) | CH1 HITS ROTATED | 132 | yes |
| SNAKE | ALL | FX | FX: ALL (title 98 px) | CH1 ALL BUT ARTIC NEW | 173 | no |
| SNAKE | NOTES | SCL | SCL: NOTES (title 156 px) | CH1 NEW PITCHES | 129 | yes |
| SNAKE | ACCNT | ENV | ENV: ACCNT (title 165 px) | CH1 NEW VELOCITIES | 150 | no |
| SNAKE | SHAPE | LFO | LFO: SHAPE (title 154 px) | CH1 NEW DRIFT | 110 | yes |
| SNAKE | ARTIC | ARP | ARP: ARTIC (title 148 px) | NO ARTIC IN SNAKE (NONE: layer absent) | 134 | no |
| SNAKE | HITS | SEQ | SEQ: HITS (title 128 px) | CH1 NEW GATES | 115 | yes |
| STAGE | ALL | FX | FX: ALL (title 98 px) | CH1 EVERY LAYER NEW | 163 | no |
| STAGE | NOTES | SCL | SCL: NOTES (title 156 px) | CH1 NEW PITCHES | 129 | yes |
| STAGE | ACCNT | ENV | ENV: ACCNT (title 165 px) | CH1 NEW VELOCITIES | 150 | no |
| STAGE | SHAPE | LFO | LFO: SHAPE (title 154 px) | CH1 NEW LONG STAGES | 166 | no |
| STAGE | ARTIC | ARP | ARP: ARTIC (title 148 px) | CH1 NEW RATCHETS + HOLDS | 208 | no |
| STAGE | HITS | SEQ | SEQ: HITS (title 128 px) | CH1 NEW RESTS | 114 | yes |
| ARP | ALL | FX | FX: ALL (title 98 px) | CH1 EVERY LAYER NEW | 163 | no |
| ARP | NOTES | SCL | SCL: NOTES (title 156 px) | CH1 NEW OCTAVE HOPS | 168 | no |
| ARP | ACCNT | ENV | ENV: ACCNT (title 165 px) | CH1 NEW VELOCITIES | 150 | no |
| ARP | SHAPE | LFO | LFO: SHAPE (title 154 px) | CH1 NEW START PHASE | 164 | no |
| ARP | ARTIC | ARP | ARP: ARTIC (title 148 px) | CH1 NEW NOTE LENGTHS | 177 | no |
| ARP | HITS | SEQ | SEQ: HITS (title 128 px) | CH1 NEW RESTS | 114 | yes |
| PATT | ALL | FX | FX: ALL (title 98 px) | CH1 EVERY LAYER NEW | 163 | no |
| PATT | NOTES | SCL | SCL: NOTES (title 156 px) | CH1 NEW VARIATIONS | 152 | no |
| PATT | ACCNT | ENV | ENV: ACCNT (title 165 px) | CH1 NEW ACCENTS | 137 | yes |
| PATT | SHAPE | LFO | LFO: SHAPE (title 154 px) | CH1 NEW UP/DOWN | 139 | yes |
| PATT | ARTIC | ARP | ARP: ARTIC (title 148 px) | CH1 NEW SLIDES | 117 | yes |
| PATT | HITS | SEQ | SEQ: HITS (title 128 px) | CH1 NEW THINNING ORDER | 190 | no |

## 5. The MAP: split or not (counts, recommendation)

| | v3.1 (one page) | v3.2 A (buttons) | v3.2 B (keys) |
| --- | --- | --- | --- |
| text boxes | 89 | 55 | 32 |
| AF_M (9.5') | 0 (the `MENU` word only in the countdown) | 33 | 32 |
| AF_S (7.7') | 89 | 22 (printed names of the panel: `FX`, `SCL`, `K1` ...) | 0 |
| text roles | 19 | 8 | 8 |
| gap between printed name and role | 1 px | 2 px | (labels under keys: leader lines) |

**Recommendation: split.** The panel has 12 buttons and 27 keys; on one page at AF_M the buttons alone (4 columns of 58 px for roles up to 57 px: `ACCNT`, `CANCEL`) plus encoders plus knob names fill 240 x 236, and the keys need their own two tiers of labels. The roles (what the control does) are the large type; the printed names (what the panel already says) are the 9 px tags. The price: the
12 buttons are a 4 x 3 grid in the panel's reading order, not the panel's 2 x 6; and AF_S is not gone (22 printed-name tags on A).
**Switching**: OCT+ / OCT- while HOME is held (`OCT+ = KEYS >` / `< OCT- = BUTTONS` in the header). This collides with touch-for-help (touching any control while HOME is down shows its help instead of acting): so the OCT buttons are the one exception, they flip the page and the header shows
`OCT+: ...` help for a moment. It is an owner question (section 9). Alternatives: the SELECT encoder turn (also a control), or both pages on a 3 s cycle.
`UNDO L` (53 px, the firmware label) instead of `UNDO LD` (63): `UNDO LD` would overflow the 56 px box.

## 6. Verification of the critique, measured (v3.1 against v3.2, same state)

### 6a. Pixel checks (v3.1's own draw() and v3.2's draw() on the same state)

- bottom band y220..240 while playing: non-BG pixels v3.1 = 0, v3.2 = 1269 (the cue).
- slide chain, 16 slides, playhead on step 8: longest run of BG-coloured pixels along the tie row: v3.1 = 15 px (the dark gap at the playhead), v3.2 = 1 px.
- ON step with vel 1/2/3 vs a REST, both under the playhead (first lane column): differing pixels v3.1 = [0, 0, 0], v3.2 = [44, 44, 44].
- ratchet counts, differing pixels in the column: v3.1 {'1 vs 6': 0, '1 vs 7': 0, '1 vs 8': 0, '1 vs 4': 99, '4 vs 5': 132, '4 vs 8': 99}; v3.2 {'1 vs 6': 61, '1 vs 7': 61, '1 vs 8': 61, '1 vs 4': 61, '4 vs 5': 0, '4 vs 8': 0}.  v3.2 draws the digit min(count, 4): counts above 4 do not exist in the firmware (gr bits 4..6 = 0 AUTO, 1..4; knob_map NOTE_EDIT K4 1x..4x), so 5..8 repeat the 4.
- playhead on page 3 while page 2 is viewed: amber pixels in the lane: v3.1 = 0, v3.2 = 343 (ghost column 13x70, 2 px frame + 5 px cap); header marker v3.1 = 4x4 px dot, v3.2 = a filled 13x14 box + under-bar on the viewed page.
- accent: v3.1 cyan bar vs a normal bar = 1.15:1 (the whole bar, 11x34 px); v3.2 cap 13x8 vs LANE = 6.25:1 and the bars stay TEXT.
- toast fill vs BG: v3.1 SEL slab 10.42:1 over 240x88 = 21120 px (v3.1 bottom band 238x32 = 7616 px); the playhead column is 11.08:1 over 13x70 = 910 px, so the slab is the largest and, with 10.4 vs 11.1:1, an equally bright object; v3.2 slab SURF 1.23:1 over 240x40 with a 2 px TEXT rule (480 px of TEXT).
- v3.1 elements against the 15 x 4 module (inset 1): transport shape x1 y4 w15 h15 -> rem 0/2/3/1; channel cell x1 y26 w58 h44 -> rem 0/0/1/2; lane column x1 y72 w13 h70 -> rem 0/0/3/0; knob cell x1 y144 w58 h58 -> rem 0/0/3/0; ROW half x1 y204 w118 h16 -> rem 0/0/3/2; page box x177 y5 w14 h14 -> rem 11/1/0/0
- hot-knob frame: v3.1 draws it (`if hot:` frame in draw_knob_cell: yes); v3.2: `hot` is not read anywhere (grep S["hot"] / .get("hot"): 0 hits).
- map labels: v3.1 SAVE hold label `BACK` (BTN_ROWS), ROW 3 in the map: no; v3.2 SAVE hold `UNDO L` (the button_modes.tsv label), OCT+ role `ROW+` (ROW 2 then ROW 3).
- map black keys: v3.1 BLACK_X pitch 21/23 px ((0, 21, 42, 65)); v3.2 centres on the 15 px white-key pitch, group gaps 22 px between groups, 7 px inside a group.
- v3.1 knob cell `hot` frame, `OCTS` label for ARP SPAN: v3.2 label `SPAN` (firmware table); KEYS word at DENS 0 (v3.1 `KEY`).

### 6b. Contrast of every new pair, nine palettes

Non-text pairs need 3:1; text 4.5:1 (AF_L 3:1).  head = T_PLAYHEAD, acc = T_ACCSTEP (per palette, P1.PAL_TOKENS; MONO = TEXT).

| pair | need | use | GREY | GREEN | AMBER | ICE | VIOLET | ROSE | NIGHT | PAPER | MONO |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| head on BG | 3.0 | playhead column, play shape, activity bar | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 11.62 | 5.38 | 21.00 |
| head on LANE | 3.0 | playhead vs lane (beat 1/3) | 6.46 | 7.15 | 7.30 | 7.23 | 7.58 | 8.01 | 8.43 | 3.87 | 17.76 |
| head on LANE2 | 3.0 | playhead vs lane (beat 2/4) | 5.02 | 4.97 | 5.07 | 5.03 | 5.28 | 5.67 | 6.16 | 3.10 | 12.60 |
| acc on LANE | 3.0 | accent cap, edited foot | 6.25 | 6.92 | 7.06 | 7.00 | 7.34 | 7.76 | 8.16 | 4.30 | 17.76 |
| acc on LANE2 | 3.0 | accent cap on beat 2/4 | 4.86 | 4.81 | 4.91 | 4.87 | 5.11 | 5.49 | 5.96 | 3.45 | 12.60 |
| TEXT on LANE | 3.0 | bars, slide tie | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.29 | 10.61 | 17.76 |
| TEXT on LANE2 | 3.0 | bars on beat 2/4 | 5.58 | 8.00 | 7.94 | 8.01 | 8.24 | 8.96 | 7.51 | 8.51 | 12.60 |
| MID on LANE | 3.0 | inactive ROW cell text on BG/SURF is 4.5 elsewhere | 3.75 | 5.83 | 5.76 | 5.83 | 6.15 | 6.35 | 5.08 | 4.10 | 8.43 |
| muted mark on LANE | 3.0 | baseline, muted bar outline, cap, tie, digit, foot | 4.57 | 7.18 | 7.12 | 7.18 | 7.58 | 7.88 | 6.46 | 5.57 | 10.93 |
| MID on LANE2 | 3.0 | baseline on beat 2/4 | 2.91! | 4.06 | 4.00 | 4.05 | 4.28 | 4.50 | 3.71 | 3.29 | 5.98 |
| muted mark on LANE2 | 3.0 | muted marks on beat 2/4 | 3.54 | 5.00 | 4.94 | 4.99 | 5.28 | 5.58 | 4.72 | 4.47 | 7.76 |
| LANE2 vs LANE | 1.0 | beat shading (grouping only; not a text/shape pair) | 1.29 | 1.44 | 1.44 | 1.44 | 1.44 | 1.41 | 1.37 | 1.25 | 1.41 |
| acc vs TEXT (the v3.1 cyan whole bar vs a normal bar) | 3.0 | v3.1 accent bar (the 1.15:1 finding) | 1.15! | 1.66! | 1.62! | 1.65! | 1.61! | 1.63! | 1.26! | 2.47! | 1.00! |
| BG on head | 3.0 | marks inside the playing column (bar, cap, digit) | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 11.62 | 5.38 | 21.00 |
| TEXT on head | 3.0 | slide tie inside the playing column (needs its dark outline) | 1.11! | 1.61! | 1.57! | 1.59! | 1.56! | 1.58! | 1.22! | 2.74! | 1.00! |
| TEXT on SURF | 4.5 | toast text, knob values, ROW 2 cell | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.50 | 12.31 | 21.00 |
| MID on SURF | 4.5 | toast hint, page numerals, inactive ROW cell | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 6.17 | 4.76 | 9.97 |
| TEXT on BG (toast rule) | 3.0 | toast top rule vs BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.17 | 14.74 | 21.00 |
| INK on TEXT | 4.5 | cue chip, toast chip, ROW live cell, inverted label band | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.17 | 14.74 | 21.00 |
| SEL vs SURF (v3.1 toast slab vs knob tile) | 3.0 | v3.1 slab brightness | 8.47 | 5.57 | 5.50 | 5.00 | 4.28 | 4.43 | 6.72 | 3.92 | 12.93 |
| dimmed numeral on SEL | 3.0 | muted selected tile numeral (dim on purpose) | 5.37 | 4.20 | 4.18 | 3.87 | 3.44 | 3.44 | 4.99 | 3.17 | 6.88 |
| INK on SEL | 4.5 | selected tile text | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 7.62 | 4.70 | 12.93 |
| MID on RAISE | 4.5 | none drawn at 4.5 (muted tiles are BG) | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 7.00 | 5.70 | 9.97 |
| SEL on RAISE | 3.0 | selected vs normal tile | 6.07 | 3.98 | 3.95 | 3.58 | 3.08 | 3.23 | 5.23 | 3.07 | 10.03 |
| DIM on BG | 1.0 | hatch of past-LENGTH steps (deliberately faint) | 2.87 | 3.77 | 3.80 | 3.80 | 3.71 | 3.80 | 3.03 | 2.58 | 3.84 |

luminance contrast of the accent cap vs a plain bar in greyscale (the shape must carry it): see lane_accent_test.png; numbers:
- GREY: acc 6.25:1 on LANE, TEXT bar 7.19:1 on LANE, acc vs TEXT 1.15:1 (v3.1 bar)
- PAPER: acc 4.30:1 on LANE, TEXT bar 10.61:1 on LANE, acc vs TEXT 2.47:1 (v3.1 bar)
- ICE: acc 7.00:1 on LANE, TEXT bar 11.52:1 on LANE, acc vs TEXT 1.65:1 (v3.1 bar)

accent cap 13x8 filled triangle (area 52 px, 1.6 x 1.0 mm), v3.1 triangle 11x6 (33 px).  Edited foot 13x5 vs baseline 13x2.

### 6c. Can the off-row names live in the strip?

The off-row names, can they live in the strip?  Strip = 238 px.  Four names at AF_M with 6 px gaps, plus a `ROW n` cell (58 px) in front:
| algorithm | row | names | px (6 px gaps) | + ROW cell 58 + gap 4 | fits 238 |
| --- | --- | --- | --- | --- | --- |
| ACID | 1 | DENS SLIDE JUMP ACCNT | 184 | 246 | NO |
| ACID | 2 | OFFSET PEDAL PUNCH DIR | 190 | 252 | NO |
| EUCLID | 1 | HITS OFFSET RATCH ACCNT | 201 | 263 | NO |
| EUCLID | 2 | LEN SPAN PUNCH CURVE | 179 | 241 | NO |
| SNAKE | 1 | DENS LEN RANGE PATH | 165 | 227 | yes |
| SNAKE | 2 | OFFSET GLEN DYN DIR | 161 | 223 | yes |
| STAGE | 1 | DENS TOTAL DIR RATCH | 171 | 233 | yes |
| STAGE | 2 | OFFSET RDIV DYN RANGE | 182 | 244 | NO |
| ARP | 1 | DENS SHAPE SPAN RATE | 174 | 236 | yes |
| ARP | 2 | OFFSET LEGATO DYN DIR | 179 | 241 | NO |
| PATT | 1 | DENS VARY SPAN ACCNT | 180 | 242 | NO |
| PATT | 2 | PICK SLIDE DYN CTONE | 166 | 228 | yes |

Aligned under their knobs (one name per 58 px tile) every name fits (widest `LEGATO` 55, `OFFSET` 54 px) but then the ROW cell has no room: the strip would be four names and no ROW label.

## 7. Own decisions, and where I tried something and kept or reverted it

- **Ratchet as a digit.** The v3.1 comb (2 px teeth at x4) cannot be counted (1.4'); dots would need 4 x 3 px + gaps = 15 px in a 13 px column; a digit `2/3/4` in AF_M is 9.5'. Cost: one more text role, and under blur (lane_accent_test.png, last row) the digits fade more than the shapes do. Kept; the comb version is v3.1 and stays in the before/after sheets.
- **Strike-through vs dimmed numeral + chip.** The strike tested worse: it was a second mark for what the chip already says, and for a muted AND selected tile it ended on the inverted fill as a 3 px bar. Dimmed numeral on a hollow tile + a solid `M` chip passes (numeral 5.4:1 on GREY, 3.2:1 on PAPER, vs 10.4 / 4.7 for full ink); the chip is the unambiguous mark. Replaced.
- **Edited-step mark**: tried the v3.1 diamond (5 x 5 at the column top) against a foot (13 x 5 in the baseline row, second colour). The foot is 2.6x wider, sits where the eye already reads the baseline, and survives the playing column (drawn BG). Kept the foot. A third option (thicker bar) conflicts with velocity = height.
- **MIDI OFF**: `--` in the MIDI slot instead of the ring-slash. It is shorter than a word in 58 px (the word is 27 px + the chip 14 + the numeral...) and says "no channel" by its absence of a number. Weaker than a word: it is learned (MAP B key `-- MIDI OFF`).
- **Activity**: moved off the tile (on BG, 58 x 4 under the tile) because amber on the inverted selected tile is 1:1; one colour, one place for all four.
- **Hot frame**: removed, not redefined; the value updates in place and `FIL` / `RESET` have their own signals.
- **Off-row names** in the strip: tested and rejected (`[rows]`: 7 of 12 do not fit next to a ROW label cell), moved to the OCT cue.
- **The ROW strip lost its OCT hint** (`OCT+ >` in v3.2's first draft): the cue names the button when it is touched, and the hint cost one text role.

## What got worse (honest list)

1. **Toast slab 40 px instead of 32.** It covers empty tile padding and the strip; meters are never covered. Needed for AF_L title + AF_M consequence.
2. **Text roles on the glance screen: 11** (tempo, preset, page numeral, channel numeral, MIDI slot, algorithm name, knob label, knob value, ratchet digit, row label, cue) against 9 in v3.1 when stopped, **0 AF_S**. Each addition carries a fact that was missing (MIDI channel, which page, ratchet count, the cue). A toast adds up to 4 (chip, title, detail, hint) and removes the strip and cue roles while it shows. The brief says "near 8": this is over it.
3. **Channel tiles are tighter**: numeral + arrow + two-digit MIDI channel + `M` chip leave 2 px between `16` and the chip. 
4. **Ratchet digit and accent cap are small marks** (digit 9.5', cap 13 x 8 = 1.6 x 1.0 mm); the digits fade under the blur test.
5. **Beat shading is weak** (1.25..1.44:1 between the two lane shades); it groups, it does not label. The 6x3 pips were worse at 2' but were an unambiguous mark; shading could be lost in glare.
6. **Activity bar is 4 px tall** (v3.1 14 x 14): lower salience, in exchange for consistency.
7. **MAP: the 2 x 6 mirror of the panel is gone** (4 x 3 grid), and AF_S remains for 22 printed names.
8. **Bar height resolution** is 33 levels (v3.1: 39): the marks above the bar take 28 px of the column.

## Firmware cost (what adopting v3.2 means beyond v3.1)

- **`ui_sq.c`**: every zone draw rewritten to the coordinates above (blits A 240x72, B 240x72, C 240x96, all under CV_MAX 240x124; MAP strips 240x124 and 240x116). New primitives: ellipse (REC disc and ring), hollow triangle, 8x7 arrow, up-triangle 13x8 (the 11x6 one generalised), hatch (past LENGTH and SOL), dashed ghost box (ADD PAGE), page boxes with a numeral, an inverted-tab text routine (cue chip, toast tab, boxed hold word), the toast slab with two rows. Removed: ring-slash, diamond, comb, die, undo arrow, `?` glyph, 4x4 page dot, hot frame.
- **State the screen needs** that may not exist yet: pending preset slot + settle fraction + bar/stopped; clock state (INT / EXT / syncing / waiting); REC armed; FOLLOW; the playing page per channel (exists via `p`); held-control flags (SOL / FIL / ROL / mute key / OCT) and the last-pressed control with a 3 s timer; `heard` for STAGE TOTAL; a rig-present flag per channel; per-channel `midi_ch`; edit flag per step (the foot); ratchet count 1..4 per step.
- **Generated tables / strings** (all `static const`, flash): cue strings 32 (about 700 B with the chip names), dice detail 6 algorithms x 6 layers, <= 22 chars each (about 900 B; the 6-char sub-labels of ALGORITHM_SET 4.4 already exist), toast strings (preset when / actions, loading, MIDI, PAST THE END, ADD PAGE, about 200 B), the TB-3 ROW 3 names are a placeholder owned by the RIG profile. `tools/gen_ui_tables.py` emits them; `tests/design/widths.tsv` gains one row per string (about 110).
- **Palette tables**: no new rows vs v3.1 (T_PLAYHEAD / T_ACCSTEP / GREY T_REC); lane shade, muted-mark colour and dimmed numeral are `ux_mix()` of existing tokens (three calls at palette change).
- **Behaviour**: the cue state machine (held > last > idle, 3 s), toast kinds (8 in the table above) with the dismissal events, the MAP page switch (OCT while HOME), the OCT cue from the knob tables.
- **Memory**: no new buffers; the largest blit is 240 x 96 (23040 px).
- **CPU**: nothing new per frame beyond v3.1 (rect fills and a few hundred pixel sets; the hatch is the largest, a few thousand `pset`s, only while SOL is held or past LENGTH).

## 8. Least sure about

1. **The cue's two-step life on dice presses** (toast, then cue): the cue is only seen after the toast has hidden it. If the toast alone reassures, the cue's value for dice is the `EDIT = UNDO` reminder; if it doesn't, the cue is redundant. Owner call (question 4).
2. **Beat shading at 1.25..1.44:1** may not read at all on the real panel; v3.1's pips were worse on paper, but at least they were an unambiguous mark. Test first in daylight.
3. **The ghost column** for an off-page playhead is large (13 x 70, amber) but hollow; I have not compared it with a filled low-opacity column. In MONO it is a TEXT frame and relies on shape alone.
4. **Digit ratchet marks under blur** (see `lane_accent_test.png`).
5. **`[arrow]n` as the MIDI channel**: a learned symbol; `1 [arrow]2` reads "channel 1 sends to 2", but without the MAP key it is a guess. The arrow is drawn (not `>`: `1 >2` reads as an inequality).
6. **ROW 3 content** is a placeholder (TB-3 profile names CUT RESO ENV-X PAD-Z, values 0..127): MODULATION.md replaced the wire design with a MOD page; the knob_map.tsv R1..R4 rows still say WIRE1..4 LEVEL.
7. **The MAP page switch via OCT** collides with touch-for-help.
8. **The slab's 4 px side margins**: `SAVE: KEEP HERE  HOME: CANCEL` is 233 px in a 240 px slab (3 px each side); it is the widest string and will clip if the wording grows.
9. **PAPER**: head on the second lane shade is 3.1:1 and the dimmed numeral 3.2:1, both at the floor; selection by tone remains thin there (inherited).
10. **Not checked**: HI-CON palette (not in the generated header), real glare, the cue strings' wording by a native musician.

## 9. Owner questions

1. **ARP K3: `SPAN` (firmware label, restored) or `OCTS` (v3.1's rename)?** v3.2 uses `SPAN` and the firmware table; if `OCTS` is wanted the firmware label table changes.
2. **MAP: split in two pages (A buttons, B keys)?** And **how to switch**: OCT+ / OCT- while HOME is held (collides with touch-for-help of OCT), or something else?
3. **What is printed on the encoders** (`MASTER`, `SELECT`, `PRESETS`, `ALGO`?): v3.2 uses `ALGO` (`ALGORITHM` is 65 px in AF_S, does not fit a 58 px tile).
4. **Dice feedback**: toast (1.2 s) then cue (3 s), toast only, or cue only (no slab for dice at all)? A one-line cue would keep the whole screen visible.
5. **Toast height**: 40 px slab (recommended) or the 32 px AF_M-only preset form?
6. **MIDI channel on the tile as `[arrow]n`, OFF as `--`**: accept? Or the word `OFF` (no room beside the chip)?
7. **`UNDO L` (the tsv label) or `UNDO LD`?** (`UNDO LD` does not fit.)
8. **REC**: ring when armed and stopped, solid disc when recording: is the playing/stopped distinction in the REC mark wanted, or should REC be a separate chip next to the transport shape?
9. **FOLLOW off**: the ghost column + page boxes + cue are the only signals; is a persistent FOLLOW indicator wanted?
10. **ROW 3**: which names and ranges should it show (the RIG wires, or the MOD page of MODULATION.md)?
