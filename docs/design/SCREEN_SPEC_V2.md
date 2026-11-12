<!-- SPDX-License-Identifier: GPL-3.0-only -->
# SCREEN_SPEC_V2: the main (PERF) screen and its states

**Design locked: James Keaney 2026-11-12** (owner, on the full state set; ADR-0004). Replaces `SCREEN_SPEC.md` for the
PERF screen and the pages drawn here (CHAN, REC band, overlays). Architecture: ADR-0006. Algorithm interface: ADR-0007.

Reference renders (pixel-exact, real fonts and palettes): `docs/design/ui_review/proposals/v12/` (`v12.py` is the
reference implementation in the mock tool; `out/v12_<scene>.png` 1x, `_4x`, `_grid`; sheets `out/S_v12_states_1.png`,
`S_v12_states_2.png`, and `../v11/out/S_v11_main.png`, `../v9/out/S_v9_algos.png`). History and owner decisions:
`docs/design/ui_review/HANDOFF/08_OPEN_QUESTIONS.md` section 0 and 0b. Independent critique of the accepted direction:
`docs/design/ui_review/critique_v11/` (26/40, no P0; the owner accepted the remaining P1s, listed in section 9).

## 1. Principles that bind every element
1. **Instant to understand** (the owner's bar): a producer knows what is happening and what each control does without a manual.
2. **Scales with use:** only channels in use are drawn; SELECT one step past the last channel adds one (section 6).
3. **Named while touched:** a control that is not drawn as a picture (SELECT, PRESETS, ALGO, OCT-/OCT+, a held button or
   key) is named in the message slot while it is touched. The one standing cue: `CHN KEY, K2 = MIDI` (section 5.6).
4. **Colour never alone:** every state also changes shape (muted = struck/hollow/dashed, MIDI OFF = red cap + hollow red
   + text, selected = filled chip + white bar, diced = hatch, accent = flat white cap, playhead = lit column + pointer).
5. **No per-algorithm screen code:** every algorithm-specific text comes from the algorithm's descriptor (ADR-0007).
6. **Fixed medium:** fonts AF_S / AF_M / AF_L only; viewing distance 30-40 cm (owner); AF_S (~10.8 arcmin) is allowed for
   short reference text, AF_M or AF_L or a shape for anything read while playing.

## 2. Bands (y in px; the grid: 16 columns of 15 px = the 16 white keys; 4 x 60 for knobs; 6 x 40 for buttons)
| Band | y | Content |
| --- | --- | --- |
| Header | 1-19 (+ 20-21 selection bar) | transport icon x3, tempo x16, channel chips x46 + 14 n, algorithm short name x105 (selected channel colour), `MIDI n` (AF_S + AF_M) |
| Message slot | x104-239, y0-21 | toasts, hold hints, touched-control names (section 5); never covers transport, tempo or chips |
| Knob tiles | 24-76 | 4 tiles 58 x 53 at x = 1 + 60 k: label band 16 px (AF_M, MID on SURF), value area 36 px |
| Status line | 77-91 (caps 80) | preset `P01` (or inverted `P02?` pending) left; `ROW` + numbered row boxes centre; `PAGE 1/2` right |
| Music row | 92-141 | pointer 92-94; selected channel's bars (full 13 px step width, height = pitch); then one 3 px strip row per channel number |
| Buttons | 146-180 | 2 x 6 tiles 38 x 17 (pitch 40 x 18), icons 13 x 13 (dice) / 13 x 11 |
| Keybed | 186-220 | 16 white keys 14 x 33 at 15 px pitch; 11 black keys 10 x 19 at x centres 15 30 45 75 90 120 135 150 180 195 225 |
| Footer | caps 225-236 | the keys' mode line (AF_M), or the setup cue (inverted) |
Firmware strips: 0-119 and 120-239 (CV_MAX 240 x 124); no text crosses y 120 (verified for every scene and sweep).

## 3. Colour
Base palette **HI-CON** (existing; black, `mono=0`, so coloured text keeps its colour). New tokens (values on HI-CON;
each needs a value on every palette and a monochrome fallback: P4-026):
| Token | HI-CON value | Use |
| --- | --- | --- |
| `T_CH1..T_CH4` | #E69F00 #56B4E9 #009E73 #CC79A7 (Okabe-Ito) | chips, bars, strips, key fronts, meters |
| `T_CHF1..4` | the channel colour x 120/255 | knob meter fill (level edge in full colour, 7 px at both sides) |
| `T_PH` | #505050 | playhead column |
| `T_DICE` | #484848 | dice tiles (neutral: must not read as a channel) |
| `T_OFF` | #DC2828 | MIDI OFF (cap, chip, hollow bars); white text on it 4.73:1 |
| white / black | #FFFFFF / #000000 | pointer, selection bar, accent cap, message slot, ink on chips |
Contrast (evidence B v11): every text pair >= 4.73:1 on its background. Accepted non-text gaps: knob fill vs empty tile
1.4-1.8 (the level edge carries the value), dice tile vs plain tile 1.62 (the die frame carries the grouping).

## 4. Elements
### 4.1 Header
- Transport: play = white triangle; stop = grey square; REC = red disc (with a white play mark while playing).
- Tempo: INT = AF_M BPM. External (TIMING_SPEC 5): two AF_S lines, `EXT` over the BPM; waiting = `EXT` / `WAIT`;
  lost = `EXT` / `--` in `T_REC`.
- Chips (SELECT): one 13 x 19 chip per channel in use: selected = filled + 2 px white bar under it; others = outline in
  channel colour; muted = SURF chip, MID digit, MID diagonal; MIDI OFF = 3 px `T_OFF` cap. The first unused slot shows
  `+` (SELECT past the last channel adds one).
- Algorithm: the descriptor's short name (<= 6 chars) in the selected channel's colour; then `MIDI` (AF_S) + channel
  number (AF_M); MIDI OFF = inverted `T_OFF` chip `MIDI OFF`.
### 4.2 Knob tiles (K1-K4; the tile is the meter)
- Label band: AF_M label (<= 6 chars, fits 58 px). Value: ONE font per knob for its whole range (AF_L when every value
  fits 54 px and uses digits/capitals, else AF_M; a zero word such as `KEYS` may drop to AF_M; AF_S as the last resort).
- Numeric: fill height = (v - min) / (max - min) of 36 px, `T_CHFn`, level edge 2 px in `T_CHn` at both sides.
- Value list (<= 10 names, ADR-0007 `names`): no fill; position dots under the value (current in channel colour).
- Turning: the tile frames itself in channel colour (2 px); two white ticks mark the old level.
- REC band: CURSOR (1..LENGTH) ACC (OFF/ON) SLIDE (OFF/ON) OCT (-2..+2) in white (knob_map REC rows).
- CHAN band: LENGTH MIDI SOURCE FOLLOW (knob_map CHAN rows); MIDI is a 17-name list (1..16, OFF) drawn with a meter.
- ROW 3 (only when the channel has a RIG device): the device profile's wire-set names (e.g. TB-3 BITE: CUT RESO
  ENV-X ENV-Y; `HwSet`, HW_SUPPORT_LOWLEVEL 9), never `WIRE1..4`.
### 4.3 Status line
Preset `Pnn` (AF_M); pending = inverted pill `Pnn?`. `ROW` (AF_S) + numbered boxes 11 x 13 (2, or 3 with a RIG), the
live row filled in channel colour. `PAGE v/n` (AF_S + AF_M); FOLLOW off = the viewed page boxed + `PLAY p` (AF_S).
### 4.4 Music row
- Selected channel: per step a 13 px bar, height 6..max by pitch within the lane's range (descriptor `lane = single`:
  a fixed 8 px hit block); rest = a 3 px floor dash; accent = 2 px white cap 2 px above; slide = 2 px bridge into the
  next step; just diced = white with a channel-colour hatch; muted = hollow MID; MIDI OFF = hollow `T_OFF`.
- Other channels: a 3 px strip per hit (13 px; muted/OFF = a 5 px grey dash) in a FIXED row per channel number (CH1..CH4
  top to bottom; the selected and unused rows stay empty); shown when 2+ channels are in use.
- Playhead: `T_PH` column + a 7 px white pointer at y 92; hidden when FOLLOW is off and the playing page is not viewed.
- Past LENGTH: the step and its key are hatched `T_LINE`. REC: a 2 px white frame around the CURSOR step.
### 4.5 Buttons (2 x 6, panel order FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC)
Icons (`icons_v12.py`, 1 bpp): a DIE FRAME = re-roll, its inside = the layer: FX pips (ALL), SCL note (NOTES), ENV `>`
(ACCNT), LFO wave (SHAPE), ARP note lengths (ARTIC), SEQ drum x heads (HITS); EDIT undo arrow; GLO cog; HOME house;
SAVE floppy; PLAY triangle; REC ring (red disc when armed). Dice tiles `T_DICE` + white icons; a layer the selected
algorithm lacks (`layer[].n == 0`) is drawn DIM. Pressed = inverted (white tile, black icon) for the press.
### 4.6 Keybed footer (the keys in their current mode)
White keys = the 16 steps of the viewed page of the selected channel: front lit in channel colour when the step is on;
the playing step gets a 2 px white bar under its key. Black keys carry their role: M1-M4 = a 6 x 7 block in the channel
colour (muted = grey X; held = inverted key), CHN = the MIDI glyph (lit white under the setup cue; on CHAN = BACK
arrow), SOL/FIL/ROL = AF_S letters (held = inverted), PG-/PG+ = arrows, RST = |<. REC: all keys plain (they play notes).
Footer line: `KEYS = STEPS` / `HOLD = EDIT`; REC `PLAY KEYS TO WRITE` / `REC = DONE`; CHAN `K2 = MIDI` / `CHN = BACK`.

## 5. The message slot (x104-239, y0-21)
Every string is in `MESSAGES` (`v12.py`); `check_messages()` proves each fits (AF_M, else AF_S, never cut). Firmware: a
string table with a width check in `tools/gen_ui_tables.py` (P4-029).
1. **Toast** (~1 s, on a press with a result): [channel chip] + what happened (AF_M) + how to reverse it (AF_S), e.g.
   `[1] NOTES  EDIT=UNDO`, `[1] UNDO  EDIT=REDO`, `[4] GONE  EDIT=UNDO`, `[1] MIDI OFF  2 TO CH2`, `PAGE 2?  PG+=ADD`.
2. **Touched control** (while touched + ~1 s): the printed name boxed (AF_S) + what it does now: `SELECT CH2 EUCLID`,
   `ALGO SNAKE AT STEP` (the screen keeps the old algorithm until it lands), `PRESETS P02 AT BAR` (stopped: `P02 IN 1.5S`),
   `OCT+ ROW 2 OF 2`, `OCT+ ROW 3 TB-3`, held moves `SOLO CH1 SOLO`, `FILL 60%  LET GO`, `ROLL 3X  LET GO`.
3. **Hold hint** (from 0.25 s): the held button's icon + its hold action (`RESET`, `HARMONY`, `UNDO LOAD`, `HELP / MENU`,
   `REMOVE CH4`); a 2 px bar fills to the fire time (EDIT/GLO 0.4 s, SAVE 0.7 s, M-key remove 1.5 s).
4. **Page title**: the CHAN page shows `CHN CHn SETUP` while open.
5. Priority: hold hint > touched control > toast > page title.
6. **Setup cue** (footer, inverted; the one standing cue): when the selected channel is new (incl. CH1 at first run) or has
   MIDI OFF: `CHN KEY, K2 = MIDI   CHn`, and the CHN key lights. It clears when the channel gets a MIDI channel.

## 6. Channel lifecycle (owner decisions 2026-11-12)
- **Add:** SELECT one detent past the last channel in use takes the next free channel (toast `[n] NEW  ALGO=PICK`, setup cue).
- **Remove:** long press the channel's mute key (M1-M4): from 0.4 s the hint reads `PREVIEW CHn`; releasing before 1.5 s
  toggles PREVIEW (as today, but on release); holding to 1.5 s removes the channel (hint `REMOVE CHn` with its bar),
  toast `[n] GONE  EDIT=UNDO`. Table change: key_map M1-M4 hold rows (P4-030).
- MIDI channel: CHN key opens CHAN; K2 sets MIDI; giving a used MIDI channel turns the other channel OFF with a toast.

## 7. State catalogue (all rendered in `proposals/v12/out/`)
playing, two_channels, one_channel, sel_muted, dice, turning_k2, turn_select, turn_algo, turn_presets, oct_row2, row3_rig,
hold_edit, hold_glo, hold_save, hold_home, hold_mute_remove, midi_off, midi_off_after, channel_added, added_after,
first_run, chan_page, chan_midi_clash, removed, undo, rec_armed, rec_playing, pending_p02, ext_sync, ext_wait, ext_lost,
follow_off, add_page, past_length, held_solo, held_fill, held_roll, arp_row2, demo_algo (a hypothetical algorithm), stage_heard;
knob sheets for every algorithm and row.

## 8. Not covered here (other specs / tasks)
NOTE_EDIT, SET, SCALE/HARMONY, RESET, HELP and MENU pages keep their current specs until re-drawn on this system (P4-014,
P4-016, P5-006, P4-018 re-pointed to this spec; SET is proposed in `features/P4-016-*.md` with renders in `proposals/v12/out_set/`, awaiting owner sign-off, then it moves into sections 4 and 5); the RIG page (HW_SUPPORT_LOWLEVEL 9) likewise.

## 9. Known gaps accepted at lock (critique v11, owner "good enough, move on")
Other-channel strips are 3 px (~3.7 arcmin at 35 cm); the playhead column is 2.6:1 on black; the knob fill can read as
"selected"; the six dice are a weak group at a glance; the MIDI cue does not show for an existing channel once its MIDI
is set; step on/off is drawn twice (music row and keys). Re-check all of them on the real unit at HW-1 (P4-021).
