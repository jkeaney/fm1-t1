<!-- SPDX-License-Identifier: GPL-3.0-only -->
# EVIDENCE B (deterministic) for UI proposal v3.2

Facts and measurements only. Source: `docs/design/ui_review/proposals/v3.2/proposal_v3_2.py` (not modified; its `NOTES.md` and `geometry_v3_2.tsv` were read for the author's claims), rendered with the
pixel-exact tool `docs/design/ui_review/mock/` (real AF_S / AF_M / AF_L fonts, palettes from `build/gen/ui_palettes.h`). Scale: 240 px = 30 mm, so 1 px = 0.125 mm. Angular size =
`cap_mm / 500 * 3438` arcmin at 50 cm. Contrast = WCAG 2 ratio from the tool's `contrast_ratio` on the nominal RGB565 values (anti-aliased glyph edges are not included). Default palette
GREY. "MONO" below is the palette named MONO in the generated header (the monochrome fallback: the proposal maps its three new colours to `T_TEXT`). Nothing here is hardware-tested.
Ink boxes are the tool's `screen.texts` boxes (they include the anti-aliasing row above and below the capitals, so a text with 11 px capitals has an ink box 12-13 px tall); an ink box
`x0,y0,x1,y1` has x1 / y1 exclusive.

Reproduction (repo root, `.venv/bin/python`): every number below comes from a script in `docs/design/ui_review/critique_v3_2/scripts/` whose stdout is saved in `data/` (same base name);
shared helper `evidence_b_lib.py` (adapted from `critique_v3/evidence_b_lib.py`: it loads `proposal_v3_2.py` and records every `tx()` call). Edge renders are in `edge/` (1x and `_4x`).
Each script is run from the repo root as `.venv/bin/python docs/design/ui_review/critique_v3_2/scripts/NAME.py > docs/design/ui_review/critique_v3_2/data/NAME.txt` (the `data/` file carries the script's base name; `s6_knob_sweep.py` -> `s6e_knob_sweep.txt`,
`s6g_cues.py` -> `s6g_cues.txt`, `s6h_toasts_states.py` -> `s6h_toasts_states.txt`, `s6f_tiles.py` -> `s6f_tiles.txt`, `s6l_map.py` -> `s6l_map.txt`, `s5b_centering_gaps.py` -> `s5b_centering.txt`, `s5c_gaps.py` -> `s5c_gaps.txt`);
`scripts/assemble_md.py` rebuilds this file from `scripts/md_template.md` and the data files. `tools/check.sh` and `./build.sh` were not run (`build/gen/` was already present).

## 1. Detector attempt

Command (raw output in `data/s1_detector.txt`):
`node /Users/jameskeaney/.claude/plugins/cache/impeccable/impeccable/3.8.0/skills/impeccable/scripts/detect.mjs --json docs/design/ui_review/proposals/v3.2`

| item | result |
| --- | --- |
| stdout, exactly | `[]` |
| exit code | 0 |
| plain (non-JSON) run, same path | printed nothing, exit 0 |
| files in `proposals/v3.2` by extension | 1 `.md`, 251 `.png`, 3 `.py`, 1 `.tsv` (+ git-ignored `__pycache__` with `.pyc`) |
| extensions the detector scans (as recorded in EVIDENCE_B_v3 section 1 from `scripts/detector/node/file-system.mjs`, `SCANNABLE_EXTENSIONS`) | `.html .htm .css .scss .sass .less .jsx .tsx .js .ts .vue .svelte .astro` |
| verdict | **Nothing was scannable.** The `[]` means no file was scanned, not that a scan found nothing; no detector rule was applied to this proposal. Same outcome as for v3. |

## 2. Re-render and diff against the committed renders

Commands:
`.venv/bin/python docs/design/ui_review/mock/mock.py render docs/design/ui_review/proposals/v3.2/proposal_v3_2.py --out /tmp/v32r > data/s2_render_log.txt` then `scripts/s2_diff.py`
(pixel diff of every scene's 1x PNG against the committed `proposal_v3_2_SCENE.png`, plus a byte comparison of every file).

| item | result |
| --- | --- |
| scenes rendered | 71 (213 PNG: 1x, `_4x`, `_grid`) |
| differing pixels, 1x, summed over the 71 scenes | **0** (every scene 0 of 57600; the per-scene table is `data/s2_diff.txt`) |
| files byte-identical to the committed ones | 213 of 213 |
| tool warnings (clipped text, ellipsised text, canvas over 240x124) | **none** (`data/s2_render_log.txt` holds only the 71 scene names; 0 lines contain "warning") |
| largest strip | glance: blits 240x72, 240x72, 240x96 (23040 px); MAP: 240x124 (29760 px, the limit) and 240x116 |
| author claim "no warning for any of the 71 scenes" (NOTES header) | reproduced |

## 3. Text inventory

Cap heights measured with `mock.measure("H", font)`: AF_S 9 px, AF_M 11 px, AF_L 21 px. **AF_S 9 px = 1.125 mm = 7.7 arcmin; AF_M 11 px = 1.375 mm = 9.5 arcmin; AF_L 21 px = 2.625 mm = 18.0 arcmin**
at 50 cm. "advance" = `text_w` (firmware advance); "ink" = ink box width x height. Scripts: `s3_text_inventory.py`, `s3b_counts_all.py`, `s3c_roles.py`.

### 3.1 Counts

Glance screens (strings drawn by font, distinct roles):

| scene | strings | AF_L | AF_M | AF_S | distinct roles |
|---|---|---|---|---|---|
| glance_idle | 28 | 4 | 24 | 0 | 11 |
| glance_playing | 28 | 4 | 24 | 0 | 11 |
| row2 | 29 | 3 | 26 | 0 | 13 |
| row3 | 29 | 4 | 25 | 0 | 11 |
| mutes | 32 | 3 | 29 | 0 | 14 |
| stress | 35 | 3 | 32 | 0 | 14 |
| stress_row2 | 35 | 3 | 32 | 0 | 14 |
| stress_pal_paper | 35 | 3 | 32 | 0 | 14 |
| stress_pal_mono | 35 | 3 | 32 | 0 | 14 |
| stress_pal_ice | 35 | 3 | 32 | 0 | 14 |
| stage_total | 29 | 3 | 26 | 0 | 13 |

All 71 scenes (`data/s3b_counts_all.txt`): AF_L 268, AF_M 1861, **AF_S 88**; the 88 AF_S strings are all on the four MAP-page-A scenes (`map_A`, `map_A_row3`, `map_menu_countdown`, `map_touch`, 22 each).
**AF_S on any glance screen, toast, cue table or state scene: 0** (67 scenes). Tool flags (text cut or ellipsised) on any of the 71 scenes: 0.

Role summary over the six glance screens (idle, playing, row2, row3, mutes, stress):

| role | font | cap px | cap mm | arcmin @50cm | instances over the 6 screens | strings (distinct) | widest advance px | widest ink px | lowest fg/bg contrast | in screens |
|---|---|---|---|---|---|---|---|---|---|---|
| algo name | M | 11 | 1.375 | 9.5 | 24 | `ARP`, `ACID`, `PATT`, `STAGE`, `SNAKE`, `EUCLID` | 51 | 50 | 6.42 | 6 |
| chan numeral | M | 11 | 1.375 | 9.5 | 24 | `3`, `2`, `4`, `1` | 9 | 9 | 5.37 | 6 |
| cue | M | 11 | 1.375 | 9.5 | 6 | `HOLD HOME = MAP`, `KEYS WRITE NOTES`, `OFFSET PEDAL PUNCH DIR`, `CH2 MUTED  TAP AGAIN = ON`, `FX SCL ENV LFO ARP SEQ = DICE` | 222 | 221 | 12.33 | 6 |
| cue chip | M | 11 | 1.375 | 9.5 | 3 | `M2`, `REC`, `OCT+` | 39 | 38 | 12.33 | 3 |
| knob label | M | 11 | 1.375 | 9.5 | 24 | `CUT`, `DIR`, `LEN`, `DENS`, `SPAN`, `RATE`, `JUMP`, `RESO` ... | 54 | 54 | 10.03 | 6 |
| knob value | L | 21 | 2.625 | 18.0 | 21 | `3`, `0`, `6`, `5`, `8`, `7`, `4`, `40` ... | 36 | 34 | 7.19 | 6 |
| knob value (M) | M | 11 | 1.375 | 9.5 | 3 | `7`, `FWD`, `OCTV` | 40 | 40 | 10.03 | 3 |
| midi slot | M | 11 | 1.375 | 9.5 | 24 | `3`, `2`, `4`, `10`, `16`, `--` | 18 | 17 | 6.42 | 6 |
| mute chip | M | 11 | 1.375 | 9.5 | 3 | `M` | 13 | 12 | 6.42 | 2 |
| page numeral | M | 11 | 1.375 | 9.5 | 10 | `3`, `2`, `4`, `1` | 9 | 9 | 5.23 | 6 |
| preset | M | 11 | 1.375 | 9.5 | 6 | `P16`, `P01` | 27 | 27 | 12.33 | 6 |
| ratchet digit | M | 11 | 1.375 | 9.5 | 14 | `3`, `2`, `4` | 9 | 9 | 3.54 | 6 |
| row label | M | 11 | 1.375 | 9.5 | 13 | `ROW 2`, `ROW 1`, `ROW 3` | 46 | 46 | 5.23 | 6 |
| tempo | M | 11 | 1.375 | 9.5 | 6 | `120`, `EXT 126` | 57 | 57 | 12.33 | 6 |

Roles per screen (from the table above): idle 11, playing 11, row2 13 (adds `knob value (M)`, `cue chip`), row3 11, mutes 14, stress 14. A toast screen has 11 to 13 roles
(`toast_dice_notes` 13, `toast_preset_pending` 11, `toast_undo` 13, `toast_algo` 11, `data/s3b_counts_all.txt`).

### 3.2 glance_playing: every string

| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | tempo | `120` | M | 27 | 26x13 | 11 | 1.375 | 9.5 | 17,5,43,18 | 12.33 |
| 2 | preset | `P01` | M | 27 | 24x13 | 11 | 1.375 | 9.5 | 121,5,145,18 | 12.33 |
| 3 | page numeral | `1` | M | 9 | 5x11 | 11 | 1.375 | 9.5 | 230,6,235,17 | 11.08 |
| 4 | chan numeral | `1` | M | 9 | 5x11 | 11 | 1.375 | 9.5 | 5,30,10,41 | 10.42 |
| 5 | midi slot | `2` | M | 9 | 9x12 | 11 | 1.375 | 9.5 | 24,29,33,41 | 10.42 |
| 6 | algo name | `ACID` | M | 33 | 33x13 | 11 | 1.375 | 9.5 | 5,46,38,59 | 10.42 |
| 7 | chan numeral | `2` | M | 9 | 9x12 | 11 | 1.375 | 9.5 | 65,29,74,41 | 7.19 |
| 8 | midi slot | `3` | M | 9 | 9x13 | 11 | 1.375 | 9.5 | 84,29,93,42 | 7.19 |
| 9 | algo name | `EUCLID` | M | 51 | 50x13 | 11 | 1.375 | 9.5 | 65,46,115,59 | 7.19 |
| 10 | chan numeral | `3` | M | 9 | 9x13 | 11 | 1.375 | 9.5 | 125,29,134,42 | 7.19 |
| 11 | midi slot | `4` | M | 9 | 9x11 | 11 | 1.375 | 9.5 | 144,30,153,41 | 7.19 |
| 12 | algo name | `SNAKE` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 125,46,172,59 | 7.19 |
| 13 | chan numeral | `4` | M | 9 | 9x11 | 11 | 1.375 | 9.5 | 185,30,194,41 | 7.19 |
| 14 | midi slot | `10` | M | 18 | 17x13 | 11 | 1.375 | 9.5 | 204,29,221,42 | 7.19 |
| 15 | algo name | `STAGE` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 185,46,230,59 | 7.19 |
| 16 | ratchet digit | `3` | M | 9 | 9x13 | 11 | 1.375 | 9.5 | 153,88,162,101 | 7.19 |
| 17 | ratchet digit | `2` | M | 9 | 9x12 | 11 | 1.375 | 9.5 | 198,88,207,100 | 5.58 |
| 18 | knob label | `DENS` | M | 38 | 38x13 | 11 | 1.375 | 9.5 | 11,147,49,160 | 10.03 |
| 19 | knob value | `12` | L | 36 | 32x21 | 21 | 2.625 | 18.0 | 14,165,46,186 | 10.03 |
| 20 | knob label | `SLIDE` | M | 39 | 38x13 | 11 | 1.375 | 9.5 | 71,147,109,160 | 10.03 |
| 21 | knob value | `6` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 82,165,98,187 | 10.03 |
| 22 | knob label | `JUMP` | M | 40 | 40x12 | 11 | 1.375 | 9.5 | 130,148,170,160 | 10.03 |
| 23 | knob value | `5` | L | 18 | 15x22 | 21 | 2.625 | 18.0 | 142,165,157,187 | 10.03 |
| 24 | knob label | `ACCNT` | M | 50 | 50x13 | 11 | 1.375 | 9.5 | 185,147,235,160 | 10.03 |
| 25 | knob value | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 202,165,218,187 | 10.03 |
| 26 | row label | `ROW 1` | M | 46 | 43x13 | 11 | 1.375 | 9.5 | 8,205,51,218 | 12.33 |
| 27 | row label | `ROW 2` | M | 46 | 46x13 | 11 | 1.375 | 9.5 | 67,205,113,218 | 5.23 |
| 28 | cue | `FX SCL ENV LFO ARP SEQ = DICE` | M | 222 | 221x13 | 11 | 1.375 | 9.5 | 9,221,230,234 | 12.33 |

The other five glance screens are tabulated string by string in `data/s3_text_inventory.txt` (idle line 19, row2 83, row3 116, mutes 149, stress 185) with the same columns. In `glance_idle` the strings are
identical to `glance_playing` except the cue (`HOLD HOME = MAP`, advance 133). The longest strings on a glance screen: `FX SCL ENV LFO ARP SEQ = DICE` 222 px advance (cue), `EXT 126` 57 px (tempo).

### 3.3 The cue line in every cue_* scene (chip = the tab, text = the role)

| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| cue_dice_accnt | cue chip | `ENV` | M | 29 | 29x11 | 11 | 1.375 | 9.5 | 2,222,31,233 | 12.33 |
| cue_dice_accnt | cue | `ACCNT  EDIT = UNDO` | M | 146 | 146x13 | 11 | 1.375 | 9.5 | 37,221,183,234 | 12.33 |
| cue_dice_all | cue chip | `FX` | M | 18 | 18x11 | 11 | 1.375 | 9.5 | 2,222,20,233 | 12.33 |
| cue_dice_all | cue | `ALL LAYERS  EDIT = UNDO` | M | 178 | 178x13 | 11 | 1.375 | 9.5 | 26,221,204,234 | 12.33 |
| cue_dice_artic | cue chip | `ARP` | M | 28 | 28x11 | 11 | 1.375 | 9.5 | 2,222,30,233 | 12.33 |
| cue_dice_artic | cue | `ARTIC  EDIT = UNDO` | M | 138 | 138x13 | 11 | 1.375 | 9.5 | 36,221,174,234 | 12.33 |
| cue_dice_hits | cue chip | `SEQ` | M | 28 | 28x13 | 11 | 1.375 | 9.5 | 2,221,30,234 | 12.33 |
| cue_dice_hits | cue | `HITS  EDIT = UNDO` | M | 128 | 128x13 | 11 | 1.375 | 9.5 | 36,221,164,234 | 12.33 |
| cue_dice_notes | cue chip | `SCL` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 2,221,29,234 | 12.33 |
| cue_dice_notes | cue | `NOTES  EDIT = UNDO` | M | 143 | 143x13 | 11 | 1.375 | 9.5 | 35,221,178,234 | 12.33 |
| cue_dice_shape | cue chip | `LFO` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 2,221,29,234 | 12.33 |
| cue_dice_shape | cue | `SHAPE  EDIT = UNDO` | M | 143 | 143x13 | 11 | 1.375 | 9.5 | 35,221,178,234 | 12.33 |
| cue_fill | cue chip | `FIL` | M | 19 | 19x11 | 11 | 1.375 | 9.5 | 2,222,21,233 | 12.33 |
| cue_fill | cue | `DENSITY UP WHILE HELD` | M | 171 | 171x13 | 11 | 1.375 | 9.5 | 27,221,198,234 | 12.33 |
| cue_idle_playing | cue | `FX SCL ENV LFO ARP SEQ = DICE` | M | 222 | 221x13 | 11 | 1.375 | 9.5 | 9,221,230,234 | 12.33 |
| cue_idle_stopped | cue | `HOLD HOME = MAP` | M | 133 | 133x13 | 11 | 1.375 | 9.5 | 53,221,186,234 | 12.33 |
| cue_mute_held | cue chip | `M2` | M | 22 | 22x12 | 11 | 1.375 | 9.5 | 2,221,24,233 | 12.33 |
| cue_mute_held | cue | `CH2 MUTED  TAP AGAIN = ON` | M | 202 | 201x13 | 11 | 1.375 | 9.5 | 30,221,231,234 | 12.33 |
| cue_oct_latch | cue chip | `OCT+` | M | 39 | 38x13 | 11 | 1.375 | 9.5 | 2,221,40,234 | 12.33 |
| cue_oct_latch | cue | `OFFSET PEDAL PUNCH DIR` | M | 182 | 182x13 | 11 | 1.375 | 9.5 | 46,221,228,234 | 12.33 |
| cue_page | cue chip | `PG+` | M | 29 | 28x13 | 11 | 1.375 | 9.5 | 2,221,30,234 | 12.33 |
| cue_page | cue | `PAGE 2 OF 4  FOLLOW OFF` | M | 180 | 179x13 | 11 | 1.375 | 9.5 | 36,221,215,234 | 12.33 |
| cue_roll | cue chip | `ROL` | M | 28 | 28x13 | 11 | 1.375 | 9.5 | 2,221,30,234 | 12.33 |
| cue_roll | cue | `STEPS REPEAT 2X-4X` | M | 146 | 147x13 | 11 | 1.375 | 9.5 | 36,221,183,234 | 12.33 |
| cue_save_hold | cue chip | `SAVE` | M | 36 | 36x13 | 11 | 1.375 | 9.5 | 2,221,38,234 | 12.33 |
| cue_save_hold | cue | `HOLD = UNDO PRESET LOAD` | M | 195 | 195x13 | 11 | 1.375 | 9.5 | 44,221,239,234 | 12.33 |
| cue_solo | cue chip | `SOL` | M | 28 | 28x13 | 11 | 1.375 | 9.5 | 2,221,30,234 | 12.33 |
| cue_solo | cue | `ONLY CH1 SOUNDS` | M | 135 | 134x13 | 11 | 1.375 | 9.5 | 36,221,170,234 | 12.33 |
| cue_undo | cue chip | `EDIT` | M | 31 | 31x11 | 11 | 1.375 | 9.5 | 2,222,33,233 | 12.33 |
| cue_undo | cue | `UNDONE  AGAIN = REDO` | M | 166 | 166x13 | 11 | 1.375 | 9.5 | 39,221,205,234 | 12.33 |

### 3.4 Toasts (title / chip / detail / hint strings of every `toast_*` scene and the two text toasts of `state_add_page`, `state_past_length`)

| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| toast_algo | toast title | `EUCLID` | L | 96 | 95x22 | 21 | 2.625 | 18.0 | 3,200,98,222 | 10.03 |
| toast_algo | toast detail | `ALGORITHM  CH1` | M | 120 | 117x13 | 11 | 1.375 | 9.5 | 3,223,120,236 | 10.03 |
| toast_dice_accnt | toast chip | `ENV` | L | 55 | 54x21 | 21 | 2.625 | 18.0 | 7,200,61,221 | 12.33 |
| toast_dice_accnt | toast title | `ACCNT` | L | 95 | 95x22 | 21 | 2.625 | 18.0 | 73,200,168,222 | 10.03 |
| toast_dice_accnt | toast detail | `CH1 NEW ACCENTS` | M | 137 | 137x13 | 11 | 1.375 | 9.5 | 3,223,140,236 | 10.03 |
| toast_dice_accnt | toast hint | `EDIT: UNDO` | M | 80 | 80x13 | 11 | 1.375 | 9.5 | 157,223,237,236 | 5.23 |
| toast_dice_all | toast chip | `FX` | L | 34 | 33x21 | 21 | 2.625 | 18.0 | 7,200,40,221 | 12.33 |
| toast_dice_all | toast title | `ALL` | L | 49 | 49x21 | 21 | 2.625 | 18.0 | 52,200,101,221 | 10.03 |
| toast_dice_all | toast detail | `CH1 EVERY LAYER NEW` | M | 162 | 163x13 | 11 | 1.375 | 9.5 | 3,223,166,236 | 10.03 |
| toast_dice_artic | toast chip | `ARP` | L | 53 | 53x21 | 21 | 2.625 | 18.0 | 7,200,60,221 | 12.33 |
| toast_dice_artic | toast title | `ARTIC` | L | 80 | 79x22 | 21 | 2.625 | 18.0 | 72,200,151,222 | 10.03 |
| toast_dice_artic | toast detail | `CH1 NEW GATES + SLIDES` | M | 179 | 179x13 | 11 | 1.375 | 9.5 | 3,223,182,236 | 10.03 |
| toast_dice_artic_snake | toast chip | `ARP` | L | 53 | 53x21 | 21 | 2.625 | 18.0 | 7,200,60,221 | 6.42 |
| toast_dice_artic_snake | toast title | `ARTIC` | L | 80 | 79x22 | 21 | 2.625 | 18.0 | 72,200,151,222 | 5.23 |
| toast_dice_artic_snake | toast detail | `NO ARTIC IN SNAKE` | M | 134 | 134x13 | 11 | 1.375 | 9.5 | 3,223,137,236 | 5.23 |
| toast_dice_hits | toast chip | `SEQ` | L | 54 | 53x23 | 21 | 2.625 | 18.0 | 7,200,60,223 | 12.33 |
| toast_dice_hits | toast title | `HITS` | L | 61 | 59x22 | 21 | 2.625 | 18.0 | 72,200,131,222 | 10.03 |
| toast_dice_hits | toast detail | `CH1 NEW HIT PATTERN` | M | 159 | 158x13 | 11 | 1.375 | 9.5 | 3,223,161,236 | 10.03 |
| toast_dice_notes | toast chip | `SCL` | L | 52 | 52x22 | 21 | 2.625 | 18.0 | 7,200,59,222 | 12.33 |
| toast_dice_notes | toast title | `NOTES` | L | 90 | 88x22 | 21 | 2.625 | 18.0 | 71,200,159,222 | 10.03 |
| toast_dice_notes | toast detail | `CH1 NEW PITCHES` | M | 129 | 129x13 | 11 | 1.375 | 9.5 | 3,223,132,236 | 10.03 |
| toast_dice_notes | toast hint | `EDIT: UNDO` | M | 80 | 80x13 | 11 | 1.375 | 9.5 | 157,223,237,236 | 5.23 |
| toast_dice_shape | toast chip | `LFO` | L | 51 | 50x22 | 21 | 2.625 | 18.0 | 7,200,57,222 | 12.33 |
| toast_dice_shape | toast title | `SHAPE` | L | 89 | 88x22 | 21 | 2.625 | 18.0 | 69,200,157,222 | 10.03 |
| toast_dice_shape | toast detail | `CH1 NEW OCTAVE JUMPS` | M | 178 | 178x13 | 11 | 1.375 | 9.5 | 3,223,181,236 | 10.03 |
| toast_midi_channel | toast title | `MIDI 2` | L | 81 | 79x21 | 21 | 2.625 | 18.0 | 3,200,82,221 | 10.03 |
| toast_midi_channel | toast detail | `CH1 SENDS ON MIDI CHANNEL 2` | M | 222 | 222x13 | 11 | 1.375 | 9.5 | 3,223,225,236 | 10.03 |
| toast_midi_off | toast title | `MIDI OFF` | L | 114 | 113x22 | 21 | 2.625 | 18.0 | 3,200,116,222 | 10.03 |
| toast_midi_off | toast detail | `CH1 SENDS NOTHING` | M | 149 | 149x13 | 11 | 1.375 | 9.5 | 3,223,152,236 | 10.03 |
| toast_preset_loading | toast title | `P04` | L | 53 | 51x22 | 21 | 2.625 | 18.0 | 3,200,54,222 | 10.03 |
| toast_preset_loading | toast detail | `LOADING` | M | 62 | 62x13 | 11 | 1.375 | 9.5 | 80,204,142,217 | 10.03 |
| toast_preset_loading | toast chip | `SAVE` | M | 36 | 36x13 | 11 | 1.375 | 9.5 | 5,223,41,236 | 12.33 |
| toast_preset_loading | toast detail | `HOLD = UNDO LAST LOAD` | M | 178 | 178x13 | 11 | 1.375 | 9.5 | 47,223,225,236 | 10.03 |
| toast_preset_pending | toast title | `P04` | L | 53 | 51x22 | 21 | 2.625 | 18.0 | 3,200,54,222 | 10.03 |
| toast_preset_pending | toast detail | `LOADS AT BAR` | M | 99 | 100x13 | 11 | 1.375 | 9.5 | 64,204,164,217 | 10.03 |
| toast_preset_pending | toast detail | `SAVE: KEEP HERE  HOME: CANCEL` | M | 233 | 233x13 | 11 | 1.375 | 9.5 | 3,223,236,236 | 10.03 |
| toast_preset_pending_stopped | toast title | `P04` | L | 53 | 51x22 | 21 | 2.625 | 18.0 | 3,200,54,222 | 10.03 |
| toast_preset_pending_stopped | toast detail | `LOADS IN 1.5 S` | M | 102 | 101x13 | 11 | 1.375 | 9.5 | 64,204,165,217 | 10.03 |
| toast_preset_pending_stopped | toast detail | `SAVE: KEEP HERE  HOME: CANCEL` | M | 233 | 233x13 | 11 | 1.375 | 9.5 | 3,223,236,236 | 10.03 |
| toast_redo | toast chip | `EDIT` | L | 59 | 58x21 | 21 | 2.625 | 18.0 | 7,200,65,221 | 12.33 |
| toast_redo | toast title | `REDONE` | L | 109 | 107x22 | 21 | 2.625 | 18.0 | 77,200,184,222 | 10.03 |
| toast_redo | toast detail | `CH1 NOTES` | M | 81 | 81x13 | 11 | 1.375 | 9.5 | 3,223,84,236 | 10.03 |
| toast_redo | toast hint | `EDIT AGAIN = UNDO` | M | 136 | 136x13 | 11 | 1.375 | 9.5 | 101,223,237,236 | 5.23 |
| toast_undo | toast chip | `EDIT` | L | 59 | 58x21 | 21 | 2.625 | 18.0 | 7,200,65,221 | 12.33 |
| toast_undo | toast title | `UNDONE` | L | 115 | 113x22 | 21 | 2.625 | 18.0 | 77,200,190,222 | 10.03 |
| toast_undo | toast detail | `CH1 NOTES` | M | 81 | 81x13 | 11 | 1.375 | 9.5 | 3,223,84,236 | 10.03 |
| toast_undo | toast hint | `EDIT AGAIN = REDO` | M | 133 | 133x13 | 11 | 1.375 | 9.5 | 104,223,237,236 | 5.23 |
| state_add_page | toast title | `ADD PAGE 3` | L | 156 | 155x22 | 21 | 2.625 | 18.0 | 3,200,158,222 | 10.03 |
| state_add_page | toast detail | `PG+ AGAIN: COPY OF 2  HOME: NO` | M | 236 | 236x13 | 11 | 1.375 | 9.5 | 3,223,239,236 | 10.03 |
| state_past_length | toast title | `PAST THE END` | L | 188 | 186x22 | 21 | 2.625 | 18.0 | 3,200,189,222 | 10.03 |
| state_past_length | toast detail | `CHN K1 = LENGTH` | M | 126 | 125x13 | 11 | 1.375 | 9.5 | 3,223,128,236 | 10.03 |
| state_past_length | toast hint | `20 STEPS` | M | 66 | 66x13 | 11 | 1.375 | 9.5 | 171,223,237,236 | 5.23 |

### 3.5 The MAP pages

`map_A` (buttons, encoders, OCT, K1..K4): 55 strings, AF_M 33, **AF_S 22**, 8 roles. `map_B` (keys): 32 strings, AF_M 32, AF_S 0, 8 roles. `map_A_row3`: same counts as `map_A`. `map_menu_countdown` /
`map_touch`: 54 strings (header word or help line replaces the title and hint). Per-role tables (font, count, strings, widest advance / ink, lowest contrast, cap mm, arcmin) are in
`data/s3_text_inventory.txt` lines 331-395; in short:

| page | role | font | count | widest ink px | lowest contrast | arcmin |
| --- | --- | --- | --- | --- | --- | --- |
| A | map printed (`MASTER SELECT PRESETS ALGO OCT- OCT+ K1..K4 FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC`) | AF_S | 22 | 49 | 5.23 | 7.7 |
| A | map role (`VOL CH 1-4 P01-16 OF CH 1 ROW 1 ROW+ ALL NOTES ACCNT SHAPE UNDO SET CANCEL SAVE ARTIC HITS PLAY REC`) | AF_M | 18 | 57 (`CANCEL`) | 10.03 | 9.5 |
| A | map knob row1 / row2, map hold (`RESET HARM MENU UNDO L`), map legend `= HOLD`, title `TOUCH = HELP`, hint `OCT+ = KEYS >` | AF_M | 4 + 4 + 4 + 1 + 1 + 1 | 54 | 5.23 (row 2 not live, MID on SURF) | 9.5 |
| B | black key labels (`M1 M2 M3 M4 CHN PG- PG+ RST`) + hold boxes (`SOL FIL ROL`) | AF_M | 8 + 3 | 31 | 12.33 | 9.5 |
| B | white key beat numerals `1 5 9 13`, captions `TAP = STEP ON/OFF`, `= EDIT THAT NOTE`, box `HOLD`, key words (`ACCENT SLIDE 3 RATCHET 2-4 EDITED M MUTED 2 MIDI OUT -- MIDI OFF SOUNDED`), title, hint `< OCT- = BUTTONS` | AF_M | 4 + 2 + 1 + 12 + 2 | 135 (caption) | 6.42 | 9.5 |

MAP ink at the screen edge (ink box x0 <= 1 or x1 >= 239): page A `TOUCH = HELP` starts x=1 and `OCT+ = KEYS >` has x1=239 (last pixel 238); page B `RST` x1=239, `TAP = STEP ON/OFF` and the white-key numeral `1` start x=1, the key
words `3` and `--` start x=1. Tile text with 0 to 2 px between ink and the 58 px tile edge: `CANCEL` (ink 121..178 in the tile 121..178: 0 px left, 1 px right), `OF CH 1` (ink 183..236, tile 181..238: 2 / 3 px), `UNDO L` (183..236: 2 / 3),
`OFFSET` (ink 3..57, tile 1..58: 2 / 2).

### 3.6 Author claims about text, checked

| claim (NOTES.md) | measured |
| --- | --- |
| 11 distinct text roles on the glance screen while playing | **11** in `glance_playing` (and `glance_idle`); 13 on `row2`, 11 `row3`, 14 `mutes`, 14 `stress` / `stress_row2` (author's own table lists the same) |
| AF_S on any glance screen or toast: 0 | reproduced (0 of 67 non-MAP scenes) |
| "no small font" | holds for the glance screens, toasts, cue table and state scenes; **AF_S is on MAP page A (22 strings, 7.7 arcmin)**, which NOTES discloses |
| map_A 55 strings: AF_M 33, AF_S 22; map_B 32 strings all AF_M | reproduced |
| strings per scene 28 / 28 / 29 / 29 / 32 / 35 / 35 (idle, playing, row2, row3, mutes, stress, stress_row2); 29 / 28 / 29 / 27 (dice_notes, preset_pending, undo, algo) | reproduced |
| tightest ink >= 56 px in a 58 px tile: `CANCEL` 57 | reproduced (57 px) |
| widest OCT cue 239 px (EUCLID, to ROW 1) | **not reproduced as a rendered width**: the author's formula uses the `OCT+` chip width for every OCT cue. The widest rendered OCT cue is EUCLID `OCT-` `HITS OFFSET RATCH ACCNT`: ink right edge x=237 (3 px spare); 72 combinations (6 algorithms x 3 rows x with/without a device x OCT+/OCT-) all <= 237 |
| widest cue 239 of 240 | reproduced: `SAVE` + `HOLD = UNDO PRESET LOAD` = 239 (1 px spare; ink right edge x=239) |
| 32 cue strings "all measured" | the cue table scenes (`cue_table_1..3`) draw the 32 keys of `CUE_ORDER`; the `CUES` dict holds 37 entries (35 non-OCT + 2 OCT built from tables); **5 entries are in no cue-table render**: `mute_hold`, `chn`, `play`, `rec_playing`, `gloset` (all measured here, section 6g) |
| text is 4.5:1 or better (NOTES 2a) | see section 4: one GREY text pair is under 4.5 (muted ratchet digit, 3.54, flagged `LOW` by the author) |

## 4. Contrast

Script `s4_contrast.py` (79 meaning-bearing non-text pairs, 35 text pairs, every pair on all nine palettes; output `data/s4_contrast.txt`). Thresholds: 4.5:1 text (3:1 for AF_L-only text); 3:1 for a non-text mark
that carries meaning. `!` = below the threshold. HEAD = T_PLAYHEAD, ACC = T_ACCSTEP, RED = T_RECRED (the proposal's three new literals; on MONO all three equal TEXT); LANE2 (second lane shade, every other beat), MUTEC (muted marks) and DIMINK (dimmed
numeral on the selected tile) are `ux_mix` values derived per palette. Token values of every palette are in section 7. "Fill versus ground" rows (a fill beside the background) are listed because the task asked for them; whether the fill itself is
the meaning-bearing mark is not judged here.

### 4.1 Text pairs (need 4.5:1)

| # | pair | fg / bg | GREY | GREEN | AMBER | ICE | VIOLET | ROSE | PAPER | NIGHT | MONO |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | bpm / preset / cue / map caption: TEXT on BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 2 | page numeral viewed: TEXT on SURF | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 3 | page numeral other: MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 4 | page numeral on the PLAYING page: BG on HEAD | BG / HEAD | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 5 | chan numeral/algo/MIDI slot unselected live: TEXT on RAISE | TEXT / RAISE | 7.19 | 9.98 | 9.99 | 10.06 | 10.15 | 10.62 | 9.65 | 9.72 | 16.29 |
| 6 | chan numeral/algo/MIDI slot selected: INK on SEL | INK / SEL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 7 | chan numeral/algo/MIDI slot muted unselected: MID on BG | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 8 | chan numeral muted SELECTED: DIMINK on SEL | DIMINK / SEL | 5.37 | 4.20! | 4.18! | 3.87! | 3.44! | 3.44! | 3.17! | 4.99 | 6.88 |
| 9 | mute chip M unselected: INK on MID | INK / MID | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 10 | mute chip M on selected tile: SEL on INK | SEL / INK | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 11 | SOL-dimmed tile text: MID on BG (hatched with DIM) | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 12 | knob label / value (M) ROW 1: TEXT on SURF | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 13 | knob label ROW 2/3 inverted band: INK on TEXT | INK / TEXT | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 14 | knob value / label ROW 3 body: TEXT on RAISE | TEXT / RAISE | 7.19 | 9.98 | 9.99 | 10.06 | 10.15 | 10.62 | 9.65 | 9.72 | 16.29 |
| 15 | STAGE heard '(n)': MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 16 | ROW cell live: INK on TEXT | INK / TEXT | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 17 | ROW cell not live: MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 18 | cue chip: INK on TEXT | INK / TEXT | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 19 | toast: title/detail TEXT on SURF | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 20 | toast: hint MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 21 | toast: dimmed (NO LAYER) title/detail MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 22 | toast: chip INK on TEXT / dimmed chip INK on MID | INK / MID | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 23 | ratchet digit unmuted: TEXT on LANE | TEXT / LANE | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.61 | 10.29 | 17.76 |
| 24 | ratchet digit unmuted: TEXT on LANE2 | TEXT / LANE2 | 5.58 | 8.00 | 7.94 | 8.01 | 8.24 | 8.96 | 8.51 | 7.51 | 12.60 |
| 25 | ratchet digit muted: MUTEC on LANE | MUTEC / LANE | 4.57 | 7.18 | 7.12 | 7.18 | 7.58 | 7.88 | 5.57 | 6.46 | 10.93 |
| 26 | ratchet digit muted: MUTEC on LANE2 | MUTEC / LANE2 | 3.54! | 5.00 | 4.94 | 4.99 | 5.28 | 5.58 | 4.47! | 4.72 | 7.76 |
| 27 | ratchet digit in playing column: BG on HEAD | BG / HEAD | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 28 | MAP printed AF_S: MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 29 | MAP printed AF_S hot: INK on SEL | INK / SEL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 30 | MAP role: TEXT on SURF | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 31 | MAP role/hold hot: INK on SEL | INK / SEL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 32 | MAP page hint / MENU word: MID on BG | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 33 | MAP white key numerals / black key labels: TEXT on BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 34 | MAP knob row2 not live: MID on SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 35 | MAP key word M chip: INK on MID | INK / MID | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |

Text pairs under 4.5:1, by palette:

| palette | text pairs under 4.5:1 |
| --- | --- |
| GREY | muted ratchet digit, MUTEC on LANE2: 3.54 |
| GREEN | muted selected numeral, DIMINK on SEL: 4.20 |
| AMBER | DIMINK on SEL 4.18 |
| ICE | DIMINK on SEL 3.87 |
| VIOLET | DIMINK on SEL 3.44 |
| ROSE | DIMINK on SEL 3.44 |
| PAPER | DIMINK on SEL 3.17; muted ratchet digit MUTEC on LANE2 4.47 |
| NIGHT | none |
| MONO | none |

The dimmed numeral (`chan numeral` of a muted AND selected tile) is AF_M 9.5 arcmin text; 6 of 9 palettes are under 4.5:1. The muted ratchet digit is AF_M; on GREY it is under 4.5 only on the second lane shade
(MUTEC on LANE: 4.57).

### 4.2 Non-text pairs (need 3:1)

| # | pair | fg / bg | GREY | GREEN | AMBER | ICE | VIOLET | ROSE | PAPER | NIGHT | MONO |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | playhead column HEAD vs LANE | HEAD / LANE | 6.46 | 7.15 | 7.30 | 7.23 | 7.58 | 8.01 | 3.87 | 8.43 | 17.76 |
| 2 | playhead column HEAD vs LANE2 | HEAD / LANE2 | 5.02 | 4.97 | 5.07 | 5.03 | 5.28 | 5.67 | 3.10 | 6.16 | 12.60 |
| 3 | playhead column HEAD vs BG (gap) | HEAD / BG | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 4 | accent cap ACC vs LANE | ACC / LANE | 6.25 | 6.92 | 7.06 | 7.00 | 7.34 | 7.76 | 4.30 | 8.16 | 17.76 |
| 5 | accent cap ACC vs LANE2 | ACC / LANE2 | 4.86 | 4.81 | 4.91 | 4.87 | 5.11 | 5.49 | 3.45 | 5.96 | 12.60 |
| 6 | accent cap ACC vs normal bar TEXT (distinguishing colour) | ACC / TEXT | 1.15! | 1.66! | 1.62! | 1.65! | 1.61! | 1.63! | 2.47! | 1.26! | 1.00! |
| 7 | normal bar TEXT vs LANE | TEXT / LANE | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.61 | 10.29 | 17.76 |
| 8 | normal bar TEXT vs LANE2 | TEXT / LANE2 | 5.58 | 8.00 | 7.94 | 8.01 | 8.24 | 8.96 | 8.51 | 7.51 | 12.60 |
| 9 | accent mark in playing column: BG cap on HEAD | BG / HEAD | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 10 | normal bar in playing column: BG on HEAD | BG / HEAD | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 11 | accent cap vs normal bar INSIDE playing column (both BG) | BG / BG | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! |
| 12 | slide tie TEXT on LANE | TEXT / LANE | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.61 | 10.29 | 17.76 |
| 13 | slide tie in playing column: TEXT on HEAD | TEXT / HEAD | 1.11! | 1.61! | 1.57! | 1.59! | 1.56! | 1.58! | 2.74! | 1.22! | 1.00! |
| 14 | slide tie in playing column: TEXT on BG outline (gap column x+14) | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 15 | slide tie outline BG on HEAD | BG / HEAD | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 16 | rest: baseline MUTEC vs LANE (rest = empty column) | MUTEC / LANE | 4.57 | 7.18 | 7.12 | 7.18 | 7.58 | 7.88 | 5.57 | 6.46 | 10.93 |
| 17 | quiet note: bar TEXT 4 px vs baseline MUTEC (the only difference) | TEXT / MUTEC | 1.57! | 1.60! | 1.61! | 1.60! | 1.56! | 1.61! | 1.90! | 1.59! | 1.62! |
| 18 | quiet note in playing column: BG bar vs HEAD | BG / HEAD | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 19 | edited foot ACC vs LANE | ACC / LANE | 6.25 | 6.92 | 7.06 | 7.00 | 7.34 | 7.76 | 4.30 | 8.16 | 17.76 |
| 20 | edited foot vs plain baseline MUTEC | ACC / MUTEC | 1.37! | 1.04! | 1.01! | 1.03! | 1.03! | 1.02! | 1.30! | 1.26! | 1.62! |
| 21 | muted lane bar outline MUTEC vs LANE | MUTEC / LANE | 4.57 | 7.18 | 7.12 | 7.18 | 7.58 | 7.88 | 5.57 | 6.46 | 10.93 |
| 22 | muted lane bar outline MUTEC vs LANE2 | MUTEC / LANE2 | 3.54 | 5.00 | 4.94 | 4.99 | 5.28 | 5.58 | 4.47 | 4.72 | 7.76 |
| 23 | muted hollow bar: outline MUTEC vs hollow interior LANE | MUTEC / LANE | 4.57 | 7.18 | 7.12 | 7.18 | 7.58 | 7.88 | 5.57 | 6.46 | 10.93 |
| 24 | ghost column frame HEAD vs LANE | HEAD / LANE | 6.46 | 7.15 | 7.30 | 7.23 | 7.58 | 8.01 | 3.87 | 8.43 | 17.76 |
| 25 | ghost column frame HEAD vs LANE2 | HEAD / LANE2 | 5.02 | 4.97 | 5.07 | 5.03 | 5.28 | 5.67 | 3.10 | 6.16 | 12.60 |
| 26 | beat shading LANE vs LANE2 | LANE2 / LANE | 1.29! | 1.44! | 1.44! | 1.44! | 1.44! | 1.41! | 1.25! | 1.37! | 1.41! |
| 27 | lane column LANE vs BG | LANE / BG | 1.72! | 1.51! | 1.48! | 1.46! | 1.41! | 1.34! | 1.39! | 1.38! | 1.18! |
| 28 | lane column LANE2 vs BG | LANE2 / BG | 2.21! | 2.17! | 2.13! | 2.11! | 2.02! | 1.89! | 1.73! | 1.89! | 1.67! |
| 29 | past-LENGTH hatch DIM vs BG | DIM / BG | 2.87! | 3.77 | 3.80 | 3.80 | 3.71 | 3.80 | 2.58! | 3.03 | 3.84 |
| 30 | unselected live tile RAISE vs BG | RAISE / BG | 1.72! | 1.74! | 1.69! | 1.68! | 1.64! | 1.60! | 1.53! | 1.46! | 1.29! |
| 31 | selected tile SEL vs BG | SEL / BG | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 32 | selected SEL vs unselected RAISE | SEL / RAISE | 6.07 | 3.98 | 3.95 | 3.58 | 3.08 | 3.23 | 3.07 | 5.23 | 10.03 |
| 33 | muted tile frame MID vs BG (the tile fill = BG) | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 34 | muted tile (BG) vs live tile (RAISE) fill | BG / RAISE | 1.72! | 1.74! | 1.69! | 1.68! | 1.64! | 1.60! | 1.53! | 1.46! | 1.29! |
| 35 | muted tile frame MID vs live tile RAISE | MID / RAISE | 3.75 | 5.07 | 5.03 | 5.09 | 5.27 | 5.33 | 3.73 | 4.80 | 7.73 |
| 36 | SOL-dimmed tile frame DIM vs BG | DIM / BG | 2.87! | 3.77 | 3.80 | 3.80 | 3.71 | 3.80 | 2.58! | 3.03 | 3.84 |
| 37 | mute chip fill MID vs BG (unselected muted) | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 38 | mute chip fill INK vs SEL (selected muted) | INK / SEL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 39 | MIDI arrow TEXT on RAISE | TEXT / RAISE | 7.19 | 9.98 | 9.99 | 10.06 | 10.15 | 10.62 | 9.65 | 9.72 | 16.29 |
| 40 | MIDI arrow INK on SEL | INK / SEL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 41 | MIDI arrow MID on BG (muted) | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 42 | first-run attention frame INK on SEL | INK / SEL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| 43 | activity bar HEAD vs BG | HEAD / BG | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 44 | meter fill TEXT vs track LANE | TEXT / LANE | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.61 | 10.29 | 17.76 |
| 45 | meter track LANE vs knob tile SURF | LANE / SURF | 1.40! | 1.21! | 1.22! | 1.22! | 1.19! | 1.15! | 1.16! | 1.22! | 1.18! |
| 46 | meter track LANE vs ROW 3 body RAISE | LANE / RAISE | 1.00! | 1.15! | 1.14! | 1.15! | 1.17! | 1.19! | 1.10! | 1.06! | 1.09! |
| 47 | meter fill TEXT vs ROW 3 RAISE | TEXT / RAISE | 7.19 | 9.98 | 9.99 | 10.06 | 10.15 | 10.62 | 9.65 | 9.72 | 16.29 |
| 48 | enum pips unchosen MID vs SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 49 | knob tile SURF vs BG | SURF / BG | 1.23! | 1.24! | 1.21! | 1.20! | 1.18! | 1.17! | 1.20! | 1.13! | 1.00! |
| 50 | ROW 2/3 inverted band TEXT vs SURF tile | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 51 | ROW 3 body RAISE vs BG | RAISE / BG | 1.72! | 1.74! | 1.69! | 1.68! | 1.64! | 1.60! | 1.53! | 1.46! | 1.29! |
| 52 | ROW 3 frame TEXT vs BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 53 | ROW cell live TEXT vs BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 54 | ROW cell not live SURF vs BG | SURF / BG | 1.23! | 1.24! | 1.21! | 1.20! | 1.18! | 1.17! | 1.20! | 1.13! | 1.00! |
| 55 | page box SURF vs BG | SURF / BG | 1.23! | 1.24! | 1.21! | 1.20! | 1.18! | 1.17! | 1.20! | 1.13! | 1.00! |
| 56 | page box playing HEAD vs BG | HEAD / BG | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 57 | page under-bar viewed TEXT vs BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 58 | transport play HEAD vs BG | HEAD / BG | 11.08 | 10.78 | 10.78 | 10.59 | 10.68 | 10.75 | 5.38 | 11.62 | 21.00 |
| 59 | transport stop MID vs BG | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 60 | REC disc RED vs BG | RED / BG | 5.99 | 5.83 | 5.82 | 5.72 | 5.77 | 5.81 | 5.50 | 6.28 | 21.00 |
| 61 | REC ring (stopped) RED vs BG | RED / BG | 5.99 | 5.83 | 5.82 | 5.72 | 5.77 | 5.81 | 5.50 | 6.28 | 21.00 |
| 62 | toast slab SURF vs BG | SURF / BG | 1.23! | 1.24! | 1.21! | 1.20! | 1.18! | 1.17! | 1.20! | 1.13! | 1.00! |
| 63 | toast slab SURF vs knob tile SURF (it overlays tiles' bottoms) | SURF / SURF | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! | 1.00! |
| 64 | toast top rule TEXT vs SURF | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 65 | toast slab SURF vs playhead HEAD brightness (HEAD vs SURF) | HEAD / SURF | 9.02 | 8.68 | 8.88 | 8.81 | 9.03 | 9.20 | 4.49 | 10.25 | 21.00 |
| 66 | toast algo unchosen pips MID vs SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 67 | toast countdown bar: fill TEXT vs frame/track TEXT (frame only) on SURF | TEXT / SURF | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| 68 | settle bar under P: TEXT vs MID track | TEXT / MID | 1.92! | 1.97! | 1.98! | 1.98! | 1.92! | 1.99! | 2.58! | 2.03! | 2.11! |
| 69 | settle bar track MID vs BG | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 70 | MAP hold box outline MID vs SURF | MID / SURF | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| 71 | MAP hot tile SEL vs SURF | SEL / SURF | 8.47 | 5.57 | 5.50 | 5.00 | 4.28 | 4.43 | 3.92 | 6.72 | 12.93 |
| 72 | MAP black key KEY vs BG | KEY / BG | 7.83 | 10.50 | 10.51 | 10.51 | 10.37 | 10.57 | 7.74 | 8.42 | 12.26 |
| 73 | MAP leader line MID vs BG | MID / BG | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| 74 | MAP white key tile LANE vs BG | LANE / BG | 1.72! | 1.51! | 1.48! | 1.46! | 1.41! | 1.34! | 1.39! | 1.38! | 1.18! |
| 75 | MAP white key tile LANE2 vs BG | LANE2 / BG | 2.21! | 2.17! | 2.13! | 2.11! | 2.02! | 1.89! | 1.73! | 1.89! | 1.67! |
| 76 | MAP header rule LINE vs BG | LINE / BG | 1.36! | 1.58! | 1.59! | 1.61! | 1.60! | 1.58! | 1.43! | 1.36! | 1.54! |
| 77 | MAP countdown fill TEXT vs frame TEXT on BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 78 | cue chip INK/TEXT tab vs BG: TEXT vs BG | TEXT / BG | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| 79 | FIL up-triangle INK on TEXT band | INK / TEXT | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |

Counts under 3:1 (of 79): GREY 24, GREEN 22, AMBER 22, ICE 22, VIOLET 22, ROSE 22, PAPER 24, NIGHT 22, MONO 22. The list per palette is at the end of `data/s4_contrast.txt`.

### 4.3 The pairs the task named, on GREY, PAPER and MONO

| pair | GREY | PAPER | MONO | note |
| --- | --- | --- | --- | --- |
| playhead column HEAD vs LANE / vs LANE2 | 6.46 / 5.02 | 3.87 / 3.10 | 17.76 / 12.60 | PAPER on the second shade is 3.10 |
| accent cap ACC vs LANE / LANE2 | 6.25 / 4.86 | 4.30 / 3.45 | 17.76 / 12.60 | |
| accent cap vs normal bar TEXT (colour only) | 1.15 | 2.47 | 1.00 | the cap is also a 13x8 triangle above the bar (shape); in MONO colour carries nothing |
| accent mark inside the playing column (BG on HEAD) | 11.08 | 5.38 | 21.00 | cap and bar are both BG there (1.00 against each other); position differs (cap rows 1-8, bar rows 30-63) |
| slide tie TEXT on LANE | 7.19 | 10.61 | 17.76 | |
| slide tie in the playing column, TEXT on HEAD | 1.11 | 2.74 | 1.00 | the tie is outlined by BG rows 9 and 15 (BG on HEAD 11.08 / 5.38 / 21.00) and continues in TEXT on BG in the 2 px gap |
| rest (baseline MUTEC vs LANE) | 4.57 | 5.57 | 10.93 | a rest = empty column |
| quiet note (4 px bar TEXT) vs the baseline MUTEC | 1.57 | 1.90 | 1.62 | a velocity 1-3 bar is 4 px tall |
| low-velocity step vs rest, pixels differing | 44 px | | | measured, section 6b |
| muted vs live tile: muted tile (BG + 2 px MID frame) vs RAISE fill | 1.72 (fill); frame MID vs BG 6.42, MID vs RAISE 3.75 | 1.53; 5.70; 3.73 | 1.29; 9.97; 7.73 | |
| selected SEL vs unselected RAISE | 6.07 | 3.07 | 10.03 | |
| unselected tile RAISE vs BG | 1.72! | 1.53! | 1.29! | |
| meter fill TEXT vs track LANE | 7.19 | 10.61 | 17.76 | |
| meter track LANE vs knob tile SURF | 1.40! | 1.16! | 1.18! | |
| mute chip: MID fill vs BG (unselected) / INK vs SEL (selected) | 6.42 / 10.42 | 5.70 / 4.70 | 9.97 / 12.93 | |
| MIDI arrow TEXT on RAISE / INK on SEL / MID on BG | 7.19 / 10.42 / 6.42 | 9.65 / 4.70 / 5.70 | 16.29 / 12.93 / 9.97 | |
| beat shading LANE2 vs LANE | 1.29! | 1.25! | 1.41! | LANE2 vs BG 2.21 / 1.73 / 1.67 |
| toast slab SURF vs BG / TEXT rule vs SURF | 1.23! / 10.03 | 1.20! / 12.31 | 1.00! / 21.00 | the slab fill barely differs from BG; the edge is the 2 px TEXT rule |
| toast text pairs (TEXT on SURF, MID hint on SURF) | 10.03 / 5.23 | 12.31 / 4.76 | 21.00 / 9.97 | |
| playhead HEAD vs toast SURF (brightness competition) | 9.02 | 4.49! | 21.00 | HEAD vs BG 11.08 / 5.38 / 21.00; the TEXT rule against BG 12.33 / 14.74 / 21.00 |
| row 2/3 inverted label band: TEXT band vs SURF tile | 10.03 | 12.31 | 21.00 | band text INK on TEXT 12.33 / 14.74 / 21.00; ROW 3 body RAISE vs BG 1.72 / 1.53 / 1.29 |
| ghost-column frame HEAD vs LANE / LANE2 | 6.46 / 5.02 | 3.87 / 3.10 | 17.76 / 12.60 | |
| REC disc / ring RED vs BG | 5.99 | 5.50 | 21.00 (RED = TEXT) | |

In MONO the three new colours are TEXT: HEAD / ACC / RED against each other 1.00:1 (the shape is the only distinction between a playhead column fill, an accent cap, a bar and a REC disc).
Across GREY: new colours against each other (luminance) HEAD/ACC 1.03, HEAD/RED 1.85, ACC/RED 1.79.

## 5. Grid

Scripts `s5_grid.py`, `s5b_centering_gaps.py`, `s5c_gaps.py`; outputs `data/s5_grid.txt`, `data/s5b_centering.txt`, `data/s5c_gaps.txt`. Measured on the rendered pixels (`Screen.px`), not read from the source numbers.

### 5.1 Do the zones tile 240x240?

- Zone heights stated in the module docstring: 24 + 48 + 72 + 60 + 16 + 16 + 4 = 240 (Z0 0, Z1 24, Z2 72, Z3 144, Z4 204, Z5 220, Z6 236).
- Marks the proposal draws in the grid PNG (the tool's `mark`): Z0 `0,0,240,24`; the four channel marks `x-1,24,60,44` (**y 24..67, not 24..71**: rows 68-71 lie in no mark); Z2 `0,72,240,72`; knob marks `x-1,144,60,60`; Z4 `0,204,240,16`; Z5 `0,220,240,16`; Z6 `0,236,240,4`.
- Pixel coverage: every pixel of the 240x240 screen is written (no 0x0000 pixel) in all 71 scenes except `cue_table_1..3` (11520 = 48 rows unwritten each; those are review sheets of two 96 px strips).
- Content bands measured in `glance_playing` (rows that hold any non-BG pixel): 5-18 header shapes and text, 20-23 page under-bar, 25-62 channel tiles, 64-67 activity bars, 73-142 lane, 145-202 knob tiles, 205-218 ROW cells, 221-233 cue text (the cue tab, when present, fills 221-234).
  BG-only spans between them: 0-4, 19, 24, 63, 68-72, 143-144, 203-204, 219-220, 234-239.
- Toast scenes: TEXT rule rows 196-197, SURF rows 198-235, BG rows 236-239 (slab y 196, h 40, flush x 0..239, as in the TSV).

### 5.2 `geometry_v3_2.tsv` against the pixels

99 rows. Edge test (the four edge mid-points are non-BG inside and BG just outside, in the row's own scene): **78 pass, 3 skipped (text cells: `tempo cell`, `preset`, `cue line`), 18 fail**.
The 18 are: 7 elements the TSV lists for a scene in which they are not drawn (`page bar 1..3` and `activity bar 2..4` in `stress`, `page bar 1` in `state_add_page`: a page bar exists only under the viewed page, an activity bar only while the channel sounds) and
11 `map black key` rows (an artefact of the test: the 1 px leader line runs below each key). Rectangles measured from the pixels equal the TSV rectangle (x, y, w, h) for every tile, activity bar, lane column, knob tile, ROW cell and page box that is drawn.
Exceptions: the **transport triangle measures 12x14** (playing, WAIT) against the TSV 13x14 (the stopped square, REC ring and disc measure 13x14); the knob tiles under a toast measure 58x51 (rows 145-195; the slab starts at row 196).
Module remainders recomputed from the measured rectangles (x-1 mod 15, w+2 mod 15, y-1 mod 4, h+2 mod 4): 0 for every channel tile, lane column, knob tile, ROW cell, page box and MAP tile; the activity and page bars are flush (y mod 4 = 0, h 4).

| family measured (glance_playing / stress / row3) | x | y | w | h |
| --- | --- | --- | --- | --- |
| channel tiles 1..4 | 1, 61, 121, 181 | 25 | 58 | 38 |
| activity bars | 1, 61 (the sounding ones) | 64 | 58 | 4 |
| lane columns 1..16 | 1 + 15 n | 73 | 13 | 70 |
| knob tiles 1..4 | 1, 61, 121, 181 | 145 | 58 | 58 |
| ROW cells 1..3 | 1, 61, 121 | 205 | 58 | 14 |
| page boxes (4 pages) | 181, 196, 211, 226 | 5 | 13 | 14 |
| page under-bar (viewed page) | 226 in `stress` | 20 | 13 | 4 |
| transport | 1 | 5 | 12 (triangle) / 13 | 14 |

### 5.3 Gaps between neighbouring cells (the NOTES claim: "two neighbours always have exactly a 2 px gap")

| direction | pair | gap measured |
| --- | --- | --- |
| horizontal | channel tile - tile (3 gaps), lane column - column (15), knob tile - tile (3), ROW cell - cell, page box - box (3), activity bar - bar | 2 px each, all equal |
| horizontal | left / right margin of tiles, lane, knob tiles | 1 px / 1 px (x 1..238) |
| vertical | header shapes (rows 5-18) to the page under-bar (row 20) | **1 px** (row 19) |
| vertical | page under-bar (rows 20-23) to the channel tile (row 25) | **1 px** (row 24) |
| vertical | channel tile (rows 25-62) to the activity bar (row 64) | **1 px** (row 63) |
| vertical | activity bar (rows 64-67) to the lane (row 73) | **5 px** (rows 68-72) |
| vertical | lane (73-142) to knob tiles (145) | 2 px |
| vertical | knob tiles (145-202) to ROW cells (205) | 2 px |
| vertical | ROW cells (205-218) to the cue (221) | 2 px |
| vertical | top of screen to header shapes / below the cue text to the bottom | 5 px / 6 px (the cue tab fills 221-234: 5 px) |
| horizontal | transport (x 1..13) to the tempo ink (x 17) | 3 px |

The vertical gap sequence in `glance_playing` is 1, 1, 1, 5, 2, 2, 2. The cue tab starts at x 0 (flush), the tiles at x 1.

### 5.4 Ink centring (margins in px, ink box includes the anti-aliasing rows; cap margins from the cap-box rule in `ctop`)

Selected rows of `data/s5b_centering.txt` (every drawn string with a stated cell is in the file):

| text | font | cell | ink margin L / R | cap margin T / B |
| --- | --- | --- | --- | --- |
| knob label `DENS` / `SLIDE` / `JUMP` / `ACCNT` (58 x 18 band) | M | 58 | 10/10, 10/10, 9/9, 4/4 | 3 / 4 |
| knob label `OFFSET` (row 2) / `LEGATO` (ARP row 2) | M | 58 | 2/2; **1/2** | 3 / 4 |
| knob value `12`, `6`, `5`, `3`, `40`, `2X` | L | 58 | 13/13, 21/21, 21/22, 21/21, 12/12, 12/12 | (cap 21 at y 165..185) |
| row label `ROW 1`, `ROW 2`, `ROW 3` (58 x 14) | M | 58 | 7/8, 6/6, 6/6 | 1 / 2 |
| page numeral `1` / `2`..`4` (13 x 14) | M | 13 | 4/4; 2/2 | 1 / 2 |
| mute chip `M` (14 x 14) | M | 14 | 1/1 | 1 / 2 |
| cue chip `M2` / `REC` / `OCT+` (tab = ink + 4) | M | 26 / 32 / 42 | 2/2 | 1 / 2 |
| ratchet digit `3`, `2`, `4` (13 px column) | M | 13 | 2/2 | n/a |
| toast chip `SCL`, `EDIT` (AF_L, tab = ink + 8) | L | 60 / 66 | 4/4 | n/a |
| channel numeral and algorithm name (left group) | M | tile x+4 | ink-left x 5 for both (numeral `1` 5..10, `ACID` 5..38) | numeral cap rows 30-40, name cap rows 47-57: 5 px of tile above the numeral cap, 5 below the name |
| MIDI slot digit (after the arrow) | M | x+23 | ink starts x+23 | 1 / 2 |
| tempo / preset (left aligned) | M | x 17 / 121 | ink starts x 17 / 121 | 1 / 2 |
| MAP encoder / button tiles: `VOL` 15/15, `CH 1-4` 4/5, `P01-16` 3/4, `OF CH 1` 2/3, `CANCEL` **0/1**, `UNDO L` 2/3 | M | 58 | as listed | |

Horizontal centring is exact to within 1 px for every string drawn with `text_in`; the largest offsets are 1 px (5 out of the ink widths with odd/even parity, e.g. `ROW 1` 7/8, `5` 21/22). Vertical: AF_M text in a 14 px cell sits 1 px above centre
(1 above / 2 below), in the 18 px label band 3 above / 4 below, by the rule `cap top = cell top + (cell h - cap h) // 2`.

## 6. Edge-case sweep

Renders in `docs/design/ui_review/critique_v3_2/edge/` (1x and `_4x`; 156 knob-cell strips, 408 tile combinations, 36 dice toasts, 22 MAP page-A variants and more). Tool warnings (cut / ellipsis / canvas over 240x124) were printed for every render:
**none in any of them**. Scripts `s6_edge_abcd.py`, `s6b_palettes.py`, `s6_knob_sweep.py`, `s6e_condensed.py`, `s6f_tiles.py`, `s6g_cues.py`, `s6h_toasts_states.py`, `s6l_map.py`.
Where the fixtures of `proposal_v3_2.py` (`scene_dice`, `scene_playing`) set a channel's algorithm without its knob values, the draw raises `IndexError` for algorithms other than ACID/SNAKE (e.g. STAGE with ACID knob values); the sweeps set matching knob values.

### (a) all four channels muted (`a_all_muted_sel1`, `a_all_muted_sel4`, `a_all_muted_all_off_sel2`)
- Drawn: four BG tiles with a 2 px MID frame (the selected one a SEL tile with dimmed numeral and an INK `M` chip), four `M` chips, no activity bar (head-colour pixels in rows 64-67: 0, against 464 in `glance_playing`), the lane hollow in MUTEC. The transport triangle (header head-colour pixels 241) still shows playing.
- Text contrast in these screens: 5.37 (dimmed numeral), 6.42, 10.42. No element states that all channels are muted; the cue is the idle cue (`FX SCL ENV LFO ARP SEQ = DICE`).
- All MIDI OFF + muted (`a_all_muted_all_off_sel2`): `--` in every MIDI slot, four chips; same pixel statistics.

### (b) 16 steps ON with accent, slide and ratchet 4 on every step; playhead on steps 1, 8, 16 (`b_full_ph1`, `b_full_ph8`, `b_full_ph16`; also on GREY / PAPER / MONO: `b_full_ph8_GREY|PAPER|MONO`)
Column profile (rows relative to the column top, `data/s6_edge_abcd.txt`, `data/s6b_palettes.txt`):

| column | rows | contents (GREY) |
| --- | --- | --- |
| normal | 0 | lane fill; 1-8 accent cap (ACC `565F`); 9 lane; 10-14 slide tie TEXT; 15 lane; 16-24 ratchet digit `4` (ink); 25-29 lane; 30-63 bar TEXT; 64-65 baseline MUTEC; 66-69 lane |
| playing | 0 HEAD; 1-9 BG (the 8-row cap in BG merges with tie outline row 9); 10-14 tie TEXT on HEAD (1.11:1); 15 BG; 16-24 digit; 25-29 HEAD; 30-65 BG (bar and baseline merge); 66-69 HEAD |

- Identical profiles for playhead on 1, 8 and 16. Slide chain: along the tie row (y 85) the TEXT run is continuous from x 1 to 238 (a single run); the longest BG run on that row is **1 px** (the screen-edge column) for all three. The 16 ties merge into one 238 px stripe: individual steps cannot be separated on that row.
- In a normal column the vertical gaps between stacked marks are 1 px (cap-to-tie row 9; tie-to-digit row 15). In the playing column the BG cap and the BG tie outline form one dark block (rows 1-9), the digit is rows 16-24 on HEAD (with the BG outline row 15 above it) and the BG bar and baseline form one block (rows 30-65).
- Column 16 (playhead 16): the tie is 13 px wide (x 226..238), not 15; the TEXT run ends at x 238.
- PAPER: playing column HEAD `82C2` (dark brown), same structure; MONO: HEAD = TEXT, so the playing column is white: tie rows 10-14 are white on white and visible only through the BG rows 9 and 15.
- Accent in the playing column: the cap is BG on HEAD (11.08:1) and the plain bar is BG too; they differ by shape and position, not by colour. Pixel check: accent ON vs OFF in the playing column differs by 56 px (the 13x8 cap).
- Ratchet digit ink boxes `3,89,12,100` (9 px wide, 11 tall) in a 13 px column.

### (c) no step ON (`c_empty_playing`, `c_empty_stopped`)
- Lane pixels playing: `39C7` 7072, `4A49` 6188, BG 2266, HEAD 884 (the playing column), MUTEC (baseline) 390; stopped: `39C7` 7072, `4A49` 7072, BG 2240, MUTEC 416. No text and no mark in the lane rows; the playhead column is the only difference between playing and stopped. No "empty pattern" indication anywhere on the screen.

### (d) the playhead on a page that is not viewed (64 steps; `d_view_p1_play_p2`, `d_view_p1_play_p4`, `d_view_p4_play_p1`, `d_view_p3_play_p4_step64`, reference `d_view_p2_play_p2_ref`)
- HEAD pixels in the lane: **343** in all four off-page cases (a 13x70 ghost: 2 px frame + 5 px cap, drawn over the viewed page's step with the same index), 531 when the playing page is viewed (the full column).
- Header: the playing page's box is filled HEAD (x 196 for page 2, 226 for page 4, 181 for page 1), the viewed page has the 13x4 TEXT under-bar (x 181 / 226 / 211 as viewed). Inside the ghost frame the viewed page's own step content stays visible (inner pixel `4A49`, a lane shade).
- Page boxes sit at x 181..238 for 4 pages; with 4 pages the numerals are `1..4` AF_M at 9.5 arcmin. No persistent FOLLOW state is drawn: `state_follow_off` shows the ghost, the boxes and the cue `PG-` `VIEW 2 OF 4  FOLLOW OFF`.

### (e) every knob cell, six algorithms, ROW 1 / ROW 2 (and ROW 3 on a channel with a device), at minimum, maximum and longest-name value (`e_knobs_ALGO_rowN`, 3 strips each)
156 cell renders: 6 algorithms x 2 rows x 3 cases x 4 knobs = 144, plus ROW 3 x 3 cases x 4 = 12. Tool flags / warnings: **0**. Clips: none (every ink box lies inside its 58 px tile). Condensed (`data/s6e_condensed.txt`; the full 156-row table is `data/s6e_knob_sweep.txt`):

| algorithm | row | K1..K4 labels | widest label (ink px) | min label margin L/R (of 58) | widest value (ink px) | min value margin | value drawn in AF_M (min/max/long cases) | picture-to-name gap min | tool flags | cells |
|---|---|---|---|---|---|---|---|---|---|---|
| ACID | 1 | DENS SLIDE JUMP ACCNT | `ACCNT` 50 | 4 | `KEYS` 36 | 11 | DENS=KEYS | - | 0 | 12 |
| ACID | 2 | OFFSET PEDAL PUNCH DIR | `OFFSET` 54 | 2 | `PEND` 38 | 10 | DIR=FWD, DIR=PEND | 4 | 0 | 12 |
| ACID | 3 | CUT RESO ENV-X PAD-Z | `ENV-X` 44 | 7 | `100` 50 | 4 | - | - | 0 | 12 |
| EUCLID | 1 | HITS OFFSET RATCH ACCNT | `OFFSET` 54 | 2 | `KEYS` 36 | 11 | HITS=KEYS | - | 0 | 12 |
| EUCLID | 2 | LEN SPAN PUNCH CURVE | `PUNCH` 50 | 4 | `AUTO` 40 | 9 | CURVE=AUTO, CURVE=INV | 3 | 0 | 12 |
| SNAKE | 1 | DENS LEN RANGE PATH | `RANGE` 48 | 5 | `KEYS` 36 | 4 | DENS=KEYS, PATH | - | 0 | 12 |
| SNAKE | 2 | OFFSET GLEN DYN DIR | `OFFSET` 54 | 2 | `PEND` 38 | 10 | DIR=FWD, DIR=PEND | 4 | 0 | 12 |
| STAGE | 1 | DENS TOTAL DIR RATCH | `RATCH` 47 | 5 | `PEND` 38 | 10 | DENS=KEYS, DIR=FWD, DIR=PEND | 4 | 0 | 12 |
| STAGE | 2 | OFFSET RDIV DYN RANGE | `OFFSET` 54 | 2 | `63` 34 | 12 | - | - | 0 | 12 |
| ARP | 1 | DENS SHAPE SPAN RATE | `SHAPE` 46 | 6 | `THUMB` 52 | 3 | DENS=KEYS, SHAPE=LINE, SHAPE=THUMB, SHAPE=WAVE | 4 | 0 | 12 |
| ARP | 2 | OFFSET LEGATO DYN DIR | `LEGATO` 55 | 1 | `CHORD` 51 | 3 | DIR=CHORD, DIR=UP | 3 | 0 | 12 |
| PATTERNS | 1 | DENS VARY SPAN ACCNT | `ACCNT` 50 | 4 | `KEYS` 36 | 11 | DENS=KEYS | - | 0 | 12 |
| PATTERNS | 2 | ACID SLIDE DYN CTONE TECH HOUSE | `HOUSE` 49 | 4 | `20` 33 | 12 | - | - | 0 | 12 |

- There is no "touched" frame in v3.2 (`hot` is read nowhere), so no frame overlaps a label.
- Tightest cells: `LEGATO` (ARP row 2, K2) ink 55 px in 58 (margins 1 / 2); `OFFSET` 54 px (2 / 2); `THUMB` 52 px and `CHORD` 51 px as the AF_M value (margins 3 / 3 and 3 / 4); `CURVE` name and `DIR` pictures sit 3 px above their name text at minimum (picture bottom to name ink top), others 4 px.
- Font drops: every `DENS`/`HITS` cell at value 0 draws the word `KEYS` in AF_M (36 px) in place of an AF_L number (13 cells across 6 algorithms use AF_M for the value in some case; list in the table, column "value drawn in AF_M"); enumerations with pictures (`DIR` FWD/BWD/PEND, `DIR` of ARP, `CURVE`, `SHAPE`) draw the value name in AF_M under a picture; `PATH` draws a route picture + the number in AF_M. All other values are AF_L.
- Value widths: widest AF_L value ink 50 px (`100`, ROW 3) in 58; no negative value, no 3-digit value on ROW 1 / ROW 2 (maximum there is 2 digits: `64`, `63`, `47`); ROW 3 (0..127) draws `127` at 49 px, margins 4 / 5.
- PATTERNS ROW 2 K1 (`PICK`): the label slot shows the category (`ACID DUB FUNK HOUSE MEL TECH`) and the value slot the number 1..20; the word `PICK` is not on the screen.
- ARP K3 shows `SPAN` (firmware label); the value draws `1`..`4`. STAGE `TOTAL` with fewer steps heard draws `(32)` in MID in the meter slot (`stage_total`).

### (f) the channel tile: 6 algorithm names x MIDI channel 1..16 and OFF x muted / live x selected / unselected (`f_tiles_sel_live`, `f_tiles_sel_muted`, `f_tiles_unsel_live`, `f_tiles_unsel_muted`: 6 x 17 tiles each, 408 combinations)
- Tool flags: 0. There are no channel names: a channel is its numeral `1`..`4`. The tile draws numeral, a drawn arrow, the MIDI digits (`--` for OFF), the `M` chip (muted), the algorithm name (ACID, EUCLID, SNAKE, STAGE, ARP, PATT).
- Algorithm name ink right edge (tile 58 px wide, x 1..58, edge at 59): ACID 38, ARP 33, PATT 39, STAGE 50, SNAKE 52, **EUCLID 55** (4 px to the tile edge). The name row (ink rows 46-59) is below the numeral / MIDI / chip row (ink rows 29-43), so a name never shares a row with the chip.
- Widest case MIDI 10..16 + muted: the MIDI digits' ink ends at x 41 for `10`..`16` (`11`: 38), the chip starts at x 43: **2 px between `16` and the chip** (the same for selected and unselected, any algorithm). `--` ends at x 27 (16 px to the chip); one digit ends at 29-33.
- The chip's right edge is x 56, 2 px inside the tile edge (x 58). MIDI 16 + muted + EUCLID: nothing clipped, nothing overlapping.
- Selected vs unselected differ only in colour (INK on SEL vs TEXT on RAISE; muted: SEL-tile numeral dimmed to `DIMINK`, INK chip) and the geometry is identical in all 408 combinations.

### (g) the cue string of every control (`data/s6g_cues.txt`)
- Width = tab (ink + 4) + 4 + text ink; no ellipsis path exists for the cue (the text is placed with `text_on`; an over-wide string would be cut at x 240 and the `draw_cue` assertion fires). **All 37 `CUES` entries fit one AF_M line**; the tightest: `SAVE` / `HOLD = UNDO PRESET LOAD` 239 (1 px spare), `CHN` / `MIDI CHANNEL AND LENGTH` 234, `REC` / `PLAY KEYS TO WRITE NOTES` 233, `PAST THE END: CHN K1 = LENGTH` 232, `M4` / `CH4 MUTED  TAP AGAIN = ON` 231 (the digit 1 is narrower: CH1 228). Variable parts checked: channel 1-4, page 1-4, preset P01-P16, tempo 40-240: widest 232 (`past_end`), none over 240.
- OCT+/OCT- cue for 6 algorithms x ROW 1/2/3 x with/without device x OCT+/OCT-: 72 strings, widest 237 (EUCLID, `OCT-`), none over 240. `OCT-` always names the ROW 1 knob names (also when ROW 1 is already live).
- Coverage of `docs/design/button_modes.tsv` PERF and `key_map.tsv` PERF rows (the table is in `data/s6g_cues.txt` section D). Control/gesture rows with **no cue string**: `B_GLO` hold (HARM), `B_HOME` hold (HELP / MENU), `B_OCTDN` and `B_OCTUP` hold-both (UPDATE: hidden maintenance), `KEY_B05` hold (RIG), `KEY_W01..W16` tap (STEP) and hold (EDIT) (the 16 white keys). `B_OCTDN` hold (ROW 1 peek) has no cue of its own: the only peek cue is `OCT+` `HELD: ROW 2 LIVE`. Cues exist for the six dice buttons, EDIT tap / hold, GLO tap, HOME tap, SAVE tap / hold, PLAY, REC (stopped / playing), OCT- tap, OCT+ tap and hold, M1..M4 (tap; the preview hold has `HOLD = PREVIEW CHn`), CHN tap, SOL, FIL, ROL, PG-, PG+, RST.
- Cue strings that differ from the spec text are listed in section 8.
- MAP header help line for touched controls: all 49 PERF `help_text.tsv` line-1 strings are <= 236 px in AF_M (none falls back to AF_S); `line2` of every PERF row (49 non-empty) is not drawn.

### (h) toasts, every layer x every algorithm and the longest preset / undo / redo / MIDI-channel toast (`h_dice_ALGO_LAYER`, 36 renders; `data/s6h_toasts_states.txt`)
- Dice toasts: 6 layers x 6 algorithms rendered; **0 flags**. Title (tab + role in AF_L) widest right edge 168 (`ENV ACCNT`); detail widest right edge 211 (`CH1 NEW RATCHETS + HOLDS`, STAGE ARTIC), limit 237. No overflow.
- The layer a given algorithm does not have ("NONE"): **only SNAKE / ARTIC** (1 of 36): tab dimmed (MID), title `ARTIC` in MID, detail `NO ARTIC IN SNAKE` in MID. Contrast 6.42 (tab INK on MID), 5.23 (text MID on SURF).
- The hint (`EDIT: UNDO`, or `NOTHING CHANGED` for the absent layer) is drawn only if `detail + 12 + hint <= 237`: it is drawn in **14 of the 36** toasts and omitted in 22: omitted for: ACID ALL, SHAPE, ARTIC, HITS; EUCLID ALL, SHAPE, ARTIC; SNAKE ALL, ACCNT, ARTIC; STAGE ALL, ACCNT, SHAPE, ARTIC; ARP ALL, NOTES, ACCNT, SHAPE, ARTIC; PATT ALL, NOTES, HITS (never drawn for `ALL`; the column "hint drawn" in `data/s6h_toasts_states.txt`). **For SNAKE / ARTIC the hint `NOTHING CHANGED` is not drawn** (detail 137 px + hint does not fit); NOTES row 4 says that toast shows `NO ARTIC IN SNAKE` + `NOTHING CHANGED`.
- Undo / redo toast: the proposal defines the detail `CH1 NOTES` only (detail ink 3..84, hint `EDIT AGAIN = REDO` starts x 104 / `EDIT AGAIN = UNDO` x 101). The branch draws the hint **unconditionally** (no width test, unlike the dice branch). The spec says undo covers "last dice, dice-all, step edit, page add, page clear or reset ... or the last HARMONY change"; for 12 plausible detail strings built from that sentence (not specified wording), 8 have their ink box overlapping the hint's ink box on the same row (undo, px between detail end and hint start: `CH1 ALL LAYERS` -14, `CH1 PAGE CLEARED` -36, `CH1 PRESET LOAD` -26, `HARMONY: VOICE` -21, `CH1 ALL BUT ARTIC` -35, `CH1 NEW PITCHES` -28, `CH1 PAGE ADDED` -21, `CH4 STEP EDIT` -2) and 4 do not (`CH1 NOTES` +20, `CH1 RESET` +24, `CH1 STEP 12` +11, `CH4 ACCNT` +18). The usable width before the hint is about 100 px (~11 characters).
- Preset toast, P01 / P04 / P09 / P10 / P16 x pending (`LOADS AT BAR`), stopped (`LOADS IN 1.5 S`), loading: 0 flags; strings `P16` (ink 3..54), `LOADS AT BAR` 64..164, `SAVE: KEEP HERE  HOME: CANCEL` 3..236 (233 px advance in a 240 px slab: 3 px each side), loading `HOLD = UNDO LAST LOAD` 47..225.
- MIDI-channel toast: CH1..CH4 x MIDI 1..16 / OFF: widest detail `CH4 SENDS ON MIDI CHANNEL 16` ink 231 px, right edge x 234 (limit 237); title `MIDI OFF` 113 px. Rendered `h_midi_ch4_16`: 0 flags. The detail text is a literal in the fixture (`what`), channel and MIDI number are typed into it. No "taken from CHn" / duplicate-channel warning is drawn (none is specified in the tsv files read).
- Algorithm toast, six names: title ink 63 (ACID), 95 (EUCLID), 89 (SNAKE), 85 (STAGE), 53 (ARP), **135 (PATTERNS)**; 0 flags; the six position pips occupy x 157..236 on the detail row.
- Spec toasts that exist in the tsv files and are **not drawn** by the proposal: `NOTHING TO RESET` (28 rows), `ONE PAGE: K1 = SHORTER` (2), `LEAVE REC FIRST` (2), `LAST ROW: OCT- = ROW 1` (2), `ROW 3 NEEDS A WIRE: HOLD CHN` (1), `REC: tap REC to finish` (4), `HOME` (toast on HOME tap, 2); widths in AF_M 135 / 178 / 117 / 175 / 229 px (AF_L 254 / 321 / 220 / 320 / 429 px).

### (i) preset states `P16?` and `P16>` (`i_hdr_*`)
- Header strings: `P16?` ink x 121..155, `P16>` 121..157; tempo `EXT 240` 17..74; 4 page boxes 181..238. Gaps: tempo ink end to preset 47 px, preset ink end to the first page box 26 px at 4 pages. No collision. 0 flags.
- The settle bar under the preset (2 px, y 20-21, x 121..154, 34 px): drawn with the toast's fraction while the toast is on screen; with no toast (after a knob turn) the fill is a constant **17 of 34 px** (rule in `draw_header`), so the bar does not count down.

### (j) clock states (`j_state_*`)
| state | header strings | header pixels differing from the internal-clock playing header |
| --- | --- | --- |
| EXT -- (`state_ext_sync`) | `EXT --` ink 17..60, `P01` | 627 |
| WAIT (`state_wait`) | `WAIT` 17..52; hollow triangle in the transport slot (12x14) | 467 |
| EXT lost (`state_ext_lost`) | `120`, `P01` | **0** (the header is identical to the internal-clock one; the only difference on the screen is the cue `CLOCK LOST: OWN TEMPO 120`) |
| EXT 240 / EXT 40 | `EXT 240` 17..74 / `EXT 40` 17..65 | 419 / 362 |
Cues: `WAITING FOR THE HOST TEMPO` 219 px, `WAITING FOR HOST START` + `PLAY` tab 226 px, `CLOCK LOST: OWN TEMPO 120` 214 px.

### (k) REC, selected + muted + MIDI OFF, first run
- REC stopped / playing: transport slot shape 13x14 in RED `FA49` (ring `3` px wall when stopped, solid disc when playing; the two are 13x14 pixel-different; on MONO RED = TEXT, so the ring / disc difference is shape only). Cues `PLAY KEYS TO WRITE NOTES` / `KEYS WRITE NOTES`.
- Selected + muted + OFF (`state_sel_muted_off` = `stress`, channel 4 ARP): SEL tile; numeral `4` dimmed (DIMINK on SEL 5.37), `--` and `ARP` in full INK (10.42), INK `M` chip with a SEL letter; hollow lane in MUTEC. Contrast of the dimmed numeral is the lowest on this tile (5.37 on GREY; 3.17-4.20 on six other palettes).
- First run (`state_first_run`): 2 px INK frame at x+12..x+38, y+3..y+18 of the selected tile around the MIDI slot; the frame's left edge (x+12, x+13) is **flush against the drawn arrow** (x+14..x+21): on the mid row the frame and arrow are one 10 px dark run. The MIDI digit `1` ink starts x+23. Cue `K2 = MIDI CHANNEL` with `CHN` tab.

### (l) both MAP pages with the longest names (`l_mapA_ALGO_rowN`: 18, `l_mapA_sel1..4`, `map_B`)
- 0 flags. Page A K1..K4 tiles (58 px): ink widths `OFFSET` 54, `LEGATO` 55 (margins 1 / 2), `CTONE` 49, `RATCH` 47, `SHAPE` 46, `RANGE` 48; row 1 and row 2 names are on separate lines (y 95 and y 108): no horizontal collision possible. The knob names do not depend on the row (the live row is the bright one).
- `OF CH 1` = 53 px in the 58 px tile (2 / 3 px); `OF CH 2..4` = 56 px (ink 182..238: 1 px each side).
- Page B: eleven black keys 8 px wide at x 11, 26, 41, 71, 86, 116, 131, 146, 176, 191, 221 (gaps 7, 7, 22, 7, 22, 7, 7, 22, 7, 22: groups 3-2-3-2-1 as the keybed); labels in two tiers with 1 px leader lines; `RST` ends at x 239 (screen edge); boxed `SOL` `FIL` `ROL`.
- Page A hold boxes: `RESET`, `HARM`, `MENU`, `UNDO L` (53 px, 2 / 3 margins). The MAP shows the selected channel's algorithm knob names and `OF CH n`; it shows neither its mute, nor its MIDI channel, nor its length.

## 7. Colour count

Script `s7_colour_count.py` (`data/s7_colour_count.txt`). Classes: existing T_* token values (BG SURF TEXT MID DIM LINE SEL INK RAISE LANE KEY THEME ACCENT REC GRID TINT of the palette in force), the proposal's new literals (HEAD
`0xFD80`, ACC `0x565F`, RED `0xFA49`; on PAPER `82C2`, `1310`, `B8C5`; on MONO all `FFFF`), derived values (LANE2, MUTEC, DIMINK: `ux_mix` results, not table rows) and the other pixel values (the 4-bit anti-aliasing ramp of text).

| scene | palette | distinct RGB565 pixel values | existing T_* token values present | new tokens present (HEAD/ACC/RED) | derived present (LANE2/MUTEC/DIMINK, not table rows) | other = 4-bit anti-alias ramp values | pixels by class: token / new / derived / ramp (of 57600) |
|---|---|---|---|---|---|---|---|
| glance_idle | GREY | 26 | 9 | 1 (ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 14 | 49068 / 354 / 5550 / 2628 |
| glance_playing | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48026 / 1943 / 4667 / 2964 |
| row2 | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48072 / 1943 / 4622 / 2963 |
| row3 | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 47874 / 1943 / 4572 / 3211 |
| mutes | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 47712 / 1343 / 5559 / 2986 |
| stress | GREY | 29 | 9 | 2 (HEAD=FD80,RED=FA49) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48017 / 1021 / 5483 / 3079 |
| stress_row2 | GREY | 29 | 9 | 2 (HEAD=FD80,RED=FA49) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48107 / 1021 / 5330 / 3142 |
| stage_total | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 47115 / 1789 / 5588 / 3108 |
| stress_pal_paper | PAPER | 121 | 10 | 1 (HEAD=82C2) | 2 (LANE2=B5D5,DIMINK=BE57) | 108 | 47507 / 875 / 4963 / 4255 |
| stress_pal_mono | MONO | 72 | 9 | 0 () | 2 (LANE2=31A6,DIMINK=39E7) | 61 | 48611 / 0 / 4954 / 4035 |
| stress_pal_ice | ICE | 120 | 12 | 1 (HEAD=FD80) | 2 (LANE2=326B,DIMINK=19C9) | 105 | 47546 / 875 / 4969 / 4210 |
| toast_dice_notes | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 47752 / 1943 / 4852 / 3053 |
| toast_preset_pending | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 47476 / 1943 / 4900 / 3281 |
| state_sol | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48430 / 1711 / 4647 / 2812 |
| state_fil | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48118 / 1943 / 4652 / 2887 |
| state_rol | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 48156 / 1943 / 4384 / 3117 |
| state_follow_off | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 47934 / 1369 / 5276 / 3021 |
| lane_full_playhead | GREY | 29 | 9 | 2 (HEAD=FD80,ACC=565F) | 2 (LANE2=4A49,DIMINK=4208) | 16 | 49608 / 1847 / 2738 / 3407 |

Notes (facts):
- Glance screens use 26 (idle) to 29 distinct RGB565 values on GREY: 9 existing token values, 1-2 new literals, 2 derived values (LANE2 `4A49`, DIMINK `4208` appear on those scenes), 14-16 ramp values. The new literals present per screen: idle ACC only; playing / row2 / row3 / mutes: HEAD + ACC; stress / stress_row2: HEAD + RED (the cyan is absent: the selected channel is muted).
- Three new literals plus three derived values are added to the existing 16 tokens (`LANE2`, `MUTEC`, `DIMINK` coincide with an existing token on some palettes: GREY `MUTEC` = `KEY` `A514`; MONO `MUTEC` = `SEL` `CE59`).
- GREY: `T_ACCENT` = `T_REC` = `0xFFDF` (near white) and `T_TEXT` = `0xCE59`: no chromatic token, so the three literals are the only chromatic values; on the other palettes RED `FA49` equals the palette's own `T_REC` (PAPER `B8C5`, MONO `FA49` but the proposal draws TEXT there).
- HEAD is used for: the playhead column, the play triangle (and the hollow WAIT triangle), the activity bars, the playing page's box, the ghost column frame. RED is used for the REC ring / disc only; ACC for the accent caps and the edited foot.

### 7.1 New and derived values on every palette, and their contrast

| palette | BG | TEXT | LANE | LANE2 (derived) | MUTEC (derived) | DIMINK (derived) | HEAD | ACC | RED |
|---|---|---|---|---|---|---|---|---|---|
| GREY | 0841 #080808 | CE59 #cdcacd | 39C7 #393839 | 4A49 #4a484a | A514 #a4a1a4 | 4208 #414041 | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| GREEN | 0081 #001008 | E7BC #e6f6e6 | 19C4 #183820 | 3287 #315039 | B636 #b4c6b4 | 1204 #104020 | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| AMBER | 1061 #100c08 | F77B #f6eede | 4162 #412c10 | 5A25 #5a4429 | C5F5 #c5beac | 4982 #4a3010 | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| ICE | 0883 #081018 | E79F #e6f2ff | 19A8 #183441 | 326B #314c5a | B619 #b4c2cd | 19C9 #18384a | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| VIOLET | 1063 #100c18 | F75F #f6eaff | 3149 #31284a | 4A0C #4a4062 | C5F9 #c5becd | 394A #392852 | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| ROSE | 1842 #180810 | FF5E #ffeaf6 | 40E7 #411c39 | 59AA #5a3452 | CDD8 #cdbac5 | 5107 #522039 | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| PAPER | F77C #f6eee6 | 20E3 #201c18 | CE77 #cdcebd | B5D5 #b4baac | 5248 #524841 | BE57 #bdcabd | 82C2 #835910 | 1310 #106183 | B8C5 #bd1829 |
| NIGHT | 0000 #000000 | 9735 #94e6ac | 0962 #082c10 | 1A24 #184420 | 7DB1 #7bb68b | 11A3 #103418 | FD80 #ffb200 | 565F #52caff | FA49 #ff484a |
| MONO | 0000 #000000 | FFFF #ffffff | 18C3 #181818 | 31A6 #313431 | CE59 #cdcacd | 39E7 #393c39 | FFFF #ffffff | FFFF #ffffff | FFFF #ffffff |

| palette | HEAD/BG | HEAD/LANE | HEAD/LANE2 | HEAD/SURF | ACC/BG | ACC/LANE | ACC/LANE2 | ACC/TEXT | RED/BG | RED/SURF | RED/LANE | MUTEC/LANE | MUTEC/LANE2 | MUTEC/BG | DIMINK/SEL | LANE2/LANE | HEAD/ACC | HEAD/RED | ACC/RED |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| GREY | 11.08 | 6.46 | 5.02 | 9.02 | 10.73 | 6.25 | 4.86 | 1.15 | 5.99 | 4.87 | 3.49 | 4.57 | 3.54 | 7.83 | 5.37 | 1.29 | 1.03 | 1.85 | 1.79 |
| GREEN | 10.78 | 7.15 | 4.97 | 8.68 | 10.44 | 6.92 | 4.81 | 1.66 | 5.83 | 4.69 | 3.86 | 7.18 | 5.00 | 10.84 | 4.20! | 1.44 | 1.03 | 1.85 | 1.79 |
| AMBER | 10.78 | 7.30 | 5.07 | 8.88 | 10.44 | 7.06 | 4.91 | 1.62 | 5.82 | 4.80 | 3.94 | 7.12 | 4.94 | 10.51 | 4.18! | 1.44 | 1.03 | 1.85 | 1.79 |
| ICE | 10.59 | 7.23 | 5.03 | 8.81 | 10.25 | 7.00 | 4.87 | 1.65 | 5.72 | 4.76 | 3.91 | 7.18 | 4.99 | 10.51 | 3.87! | 1.44 | 1.03 | 1.85 | 1.79 |
| VIOLET | 10.68 | 7.58 | 5.28 | 9.03 | 10.34 | 7.34 | 5.11 | 1.61 | 5.77 | 4.88 | 4.10 | 7.58 | 5.28 | 10.68 | 3.44! | 1.44 | 1.03 | 1.85 | 1.79 |
| ROSE | 10.75 | 8.01 | 5.67 | 9.20 | 10.41 | 7.76 | 5.49 | 1.63 | 5.81 | 4.97 | 4.33 | 7.88 | 5.58 | 10.57 | 3.44! | 1.41 | 1.03 | 1.85 | 1.79 |
| PAPER | 5.38 | 3.87 | 3.10 | 4.49 | 5.98 | 4.30 | 3.45 | 2.47 | 5.50 | 4.59 | 3.96 | 5.57 | 4.47 | 7.74 | 3.17! | 1.25 | 1.11 | 1.02 | 1.09 |
| NIGHT | 11.62 | 8.43 | 6.16 | 10.25 | 11.25 | 8.16 | 5.96 | 1.26 | 6.28 | 5.54 | 4.56 | 6.46 | 4.72 | 8.91 | 4.99 | 1.37 | 1.03 | 1.85 | 1.79 |
| MONO | 21.00 | 17.76 | 12.60 | 21.00 | 21.00 | 17.76 | 12.60 | 1.00 | 21.00 | 21.00 | 17.76 | 10.93 | 7.76 | 12.93 | 6.88 | 1.41 | 1.00 | 1.00 | 1.00 |
('!' = below 3:1 for a meaning-bearing mark, or below 4.5:1 for DIMINK text)

## 8. Accuracy against the firmware spec

Script `s8_accuracy.py` (`data/s8_accuracy.txt`); line numbers are lines of the files under `docs/design/`.

| # | what v3.2 draws | what the spec file says | match |
| --- | --- | --- | --- |
| 1 | MAP A, HOME tile: tap role `CANCEL`, hold box `MENU` | `button_modes.tsv:110` B_HOME tap label `HOME`; `button_modes.tsv:111` hold label `HLP/MN` (hold = HELP while any control is touched, MENU on release after 0.7 s); the cue says `TAP: CANCEL  HOLD: MAP` | **differs** (labels); consistent in meaning with the action text |
| 2 | MAP A, OCT+ tile role `ROW+` | `button_modes.tsv:237` B_OCTUP tap label `ROW 2`; `control_map.tsv:132` label `ROW 2` | **differs** |
| 3 | MAP B, black keys `M1`..`M4` with no hold box | `key_map.tsv:211, 224, 237, 250`: `KEY_B01..B04` hold 0.4 s label `PRV1..PRV4` (PREVIEW on/off; with ALGO turned: preview sound) | **missing** on the MAP (the cue `HOLD = PREVIEW CHn` exists, `mute_hold`, but is in no cue-table render) |
| 4 | MAP B, black key `CHN` with no hold box | `key_map.tsv:263`: `KEY_B05` hold 0.4 s label `RIG` (open the RIG page) | **missing**; the CHN cue has no hold |
| 5 | cue `CHN` `MIDI CHANNEL AND LENGTH` | `key_map.tsv:262` tap: CHAN page `LENGTH MIDI SOURCE FOLLOW` | partial (2 of 4 named) |
| 6 | MAP A GLO / HOME / REC etc. labels `FX ALL`, `SCL NOTES`, `ENV ACCNT`, `LFO SHAPE`, `EDIT UNDO / RESET`, `GLO SET / HARM`, `SAVE SAVE / UNDO L`, `ARP ARTIC`, `SEQ HITS`, `PLAY PLAY`, `REC REC`, `OCT- ROW 1` | `button_modes.tsv` PERF rows (labels `ALL NOTES ACCNT SHAPE UNDO/RESET SET/HARM SAVE/UNDO L ARTIC HITS PLAY REC ROW 1`) | match for 11 of 12 button tap roles and `OCT-`, and for the hold roles `RESET`, `HARM`, `UNDO L`; the differences are rows 1 and 2 |
| 7 | MAP B key order and positions `M1 M2 M3 M4 CHN SOL FIL ROL PG- PG+ RST` at the 3-2-3-2-1 centres | `key_map.tsv` `KEY_B01_F#3 .. KEY_B11_F#5` (labels `M1..M4 CHN SOL FIL ROL PG- PG+ RST`) | match |
| 8 | MAP A encoder tiles `MASTER`/`VOL`, `SELECT`/`CH 1-4`, `PRESETS`/`P01-16`, `ALGO`/`OF CH n` | `control_map.tsv:212` MASTER (volume, label `-`), `:142` EN_SELECT label `CH n`, `:162` EN_PRESET label `Pnn`, `:152` EN_ALGO label `ALGO`; no printed panel names are in the files | roles `CH 1-4`, `P01-16` are renderings of `CH n`, `Pnn`; printed names `SELECT`, `PRESETS`, `MASTER` are not in the spec files |
| 9 | ARP SPAN value `1`..`4` | `knob_map.tsv:36` ARP K3 SPAN "cell reads 1 OCT..4 OCT" | **differs** (`OCT` omitted) |
| 10 | STAGE TOTAL: value `48` in AF_L plus `(32)` in MID in the meter slot | `knob_map.tsv:27` STAGE K2 TOTAL "value reads 48 (32)" | differs in placement (same information) |
| 11 | PATTERNS PICK: category as the label (`HOUSE`), number as the value | `knob_map.tsv:46` PATTERNS S1 PICK "the cell reads CAT nn, for example HOUSE 3" | label slot no longer reads `PICK` |
| 12 | ACID DENS 0 reads `KEYS` (AF_M word) | `knob_map.tsv:2` ACID K1 "0 = KEYS ONLY (the cell value reads KEYS)" | match |
| 13 | knob labels and ranges, 48 PERF algorithm rows (6 algorithms x K1..K4, S1..S4) | `knob_map.tsv:2-49` | match for all 48 as drawn (SNAKE `PATH` is stored 0..13 and drawn 1..14; spec 1..14) |
| 14 | enum value names (ARP SHAPE `LINE SKIP THUMB OCTV REPT WAVE`, RATE `1/4 1/2 1X 2X 3T 4X`, DIR `UP DOWN UPDN RAND ASPL CHORD`, EUCLID CURVE `AUTO SINE TRI SAW+ SAW- ARCH INV`, ACID DIR `FWD BWD PEND`) | `knob_map.tsv:17` (CURVE), `:35` (SHAPE), `:37` (RATE), `:41` (ARP DIR), `:9` (ACID DIR) | match |
| 15 | dice toast detail without `+ MOD` (`CH1 NEW OCTAVE JUMPS`, `CH1 NEW MOD LANES`, `CH1 NEW DRIFT`, ...) | `ALGORITHM_SET.md:136` "the dice toast adds `+ MOD` when the mod seed moves too"; `:3` sub-labels `OCT+MOD`, `MOD`, `DRIFT+MOD`, `LONG+MOD`, `PHASE+MOD`, `UP/DN+MOD` | **differs** (the `+ MOD` text is not in any SHAPE toast) |
| 16 | per-algorithm layer sub-labels `GT+SLD OCTAVE ROTATE RATCH MOD GATES VELO DRIFT RESTS RT+HLD LONG VARY LENGTH PHASE ORDER SLIDES UP/DN` (the `SUBS` table) | `ALGORITHM_SET.md:136` sub-label list (ACID, EUCLID, SNAKE, STAGE, ARP) and `:405` (PATTERNS `VARY ORDER ACCNT SLIDES UP/DN`) | match for the sub-label words used in the detail wording and the absent layer (SNAKE ARTIC) |
| 17 | preset toast `LOADS AT BAR` (playing) / `LOADS IN 1.5 S` (stopped), `SAVE: KEEP HERE  HOME: CANCEL`; header `P04?` then `P04>` | `control_map.tsv:162` EN_PRESET: stopped loads 1.5 s after the last detent, playing on the first bar >= 1.5 s later, header `Pnn?` then `Pnn>`, SAVE while pending saves there, `button_modes.tsv:110` HOME cancels a pending slot; spec also: "turning back to the current slot cancels" | match; the turn-back cancel is not shown |
| 18 | SAVE hold cue / toast `HOLD = UNDO PRESET LOAD`, `HOLD = UNDO LAST LOAD`; MAP `UNDO L` | `button_modes.tsv` B_SAVE hold label `UNDO L` ("undo the last preset load ... again = redo; while a slot is pending: cancel it") | match; "again = redo" and the pending-slot meaning are not drawn |
| 19 | UNDO toast `UNDONE` / `REDONE`, hint `EDIT AGAIN = REDO / UNDO` | `button_modes.tsv` B_EDIT tap "UNDO / REDO the last dice, dice-all, step edit, page add, page clear or reset ... or the last HARMONY change" | match for the wording; only `CH1 NOTES` is defined as detail |
| 20 | cues `ROL` `STEPS REPEAT 2X-4X`, `FIL` `DENSITY UP WHILE HELD`, `SOL` `ONLY CH1 SOUNDS`, `RST` `ALL CHANNELS TO STEP 1`, `PLAY` `STOP: ALL NOTES OFF`, `EDIT` `RESET: TOUCH A CONTROL`, `GLO` `SET: BPM SWING GATE DIV` | `key_map.tsv` KEY_B08 / B07 / B06 / B11, `button_modes.tsv` B_PLAY (stop sends every note-off then CC123), B_EDIT hold (RESET latched, next touched control to default), B_GLO tap (`SET`: `BPM SWING GATE DIV`) | match |
| 21 | spec toasts not drawn (section 6h) | `NOTHING TO RESET`, `ONE PAGE: K1 = SHORTER`, `LEAVE REC FIRST`, `LAST ROW: OCT- = ROW 1`, `ROW 3 NEEDS A WIRE: HOLD CHN`, `REC: tap REC to finish`, `HOME` (in `key_map.tsv`, `button_modes.tsv`, `control_map.tsv`, `knob_map.tsv`) | not present |

Counts: 21 comparison rows; **differences or omissions**: rows 1-5, 9-11, 15, 21 (10 rows); **matches**: rows 6, 7, 12-14, 16-20 (10 rows); row 8 is a partial match (roles are renderings of the spec labels; printed encoder names are not in the spec files).

## 9. The ten findings of EVIDENCE_B_v3, one by one, on v3.2

| # | v3 finding | status on v3.2 | measurement |
| --- | --- | --- | --- |
| 1 | The detector printed `[]`, exit 0, but scanned zero files | **still present** | same `[]`, exit 0; 0 scannable files in the folder (1 md, 251 png, 3 py, 1 tsv) |
| 2 | A fresh render was byte-identical to the committed PNGs, no tool warning | **still holds** | 71 scenes, 213 of 213 files identical, 0 differing pixels, 0 warnings |
| 3 | Text counts: 20 strings playing, 8 roles, 0 AF_S on a glance screen, 82 AF_S / 16 roles on the MAP | **changed** | playing: 28 strings, 11 roles (AF_L 4, AF_M 24); idle 28; AF_S on a glance screen 0 (holds); MAP A 55 strings (AF_S 22, AF_M 33), MAP B 32 (AF_M 32); AF_M 9.5 arcmin is the smallest glance size, AF_L 18 arcmin carries knob values and toast titles only |
| 4 | Every text pair >= 4.5:1; non-text under 3:1: accent bar vs normal bar 1.15, fills (lane 1.72, cell 1.72, meter track 1.40, knob cell 1.23), toast pips and countdown track 1.62 | **changed** | text: one GREY pair under 4.5 (muted ratchet digit on LANE2 3.54), six palettes under 4.5 for the dimmed numeral (3.17-4.20), PAPER ratchet digit 4.47. Non-text: accent colour vs bar TEXT still 1.15 (now a 13x8 cap shape above the bar), lane 1.72, tile 1.72, meter track 1.40, knob cell 1.23 still present; toast pips / countdown on SEL are gone (toast on SURF; MID on SURF 5.23); new under-3 pairs: beat shading 1.29, edited foot vs baseline 1.37, slide tie TEXT on HEAD 1.11, settle bar TEXT vs MID 1.92, hatch DIM on BG 2.87 (24 of 79 on GREY) |
| 5 | Nothing on the 5 px module; lane group edges 1 px off the cell edges | **changed** | measured on the stated 15 x 4 module: remainders 0 for every tile, lane column, knob tile, ROW cell, page box; lane columns 5, 9, 13 start at x 61 / 121 / 181 = the tile x (fixed); vertical gaps are 1, 1, 1, 5, 2, 2, 2 px (not all 2) |
| 6 | Channel number 14 px off centre, 1-2 px gap number to name, the `OFF` word overlapped its frame | **fixed / changed** | numeral and name share ink-left x 5 (tile 0), 5 px between numeral ink and name ink; the `OFF` word is replaced by `--` (no frame); MIDI digits end 2 px before the `M` chip at MIDI 10-16 |
| 7 | In the playing column bars, accent triangle and slide tie are BG on amber: 15 px dark gap in the chain; accent = normal bar; vel 1-3 identical to a rest | **fixed** (in part) | longest BG run on the tie row 1 px; accent cap is a 13x8 shape (56 px differ from no accent); vel 1, 2, 3 vs rest: 44 px (the bar is 4 px tall); vel 1, 2, 3 are identical to each other (bar height 4 px) |
| 8 | Ratchet 6, 7, 8 identical to no ratchet | **changed** | the digit `min(count, 4)`: 1 vs 6/7/8: 61 px each; 4 vs 5..8: 0 px; a count above 4 is not distinguishable from 4 |
| 9 | Playhead on another page: 0 amber px, a 0.5 mm marker; `EXT --`, `WAIT`, `EXT lost`, `P16>`, MIDI toast, per-layer dice details, first-run undefined; one REC flag drew one disc | **changed** | ghost column: 343 HEAD px in the lane, playing page box filled HEAD, viewed page under-bar; all listed states are drawn; REC stopped / playing differ (ring / disc); no persistent FOLLOW indicator; EXT lost: header identical (0 px) to the internal clock one |
| 10 | Knob cells never clip; 20 of 42 enum names drop to AF_M; frames overlap 54-56 px names; no 3-digit value; selected + muted + OFF showed no `M` chip | **changed** | 156 cell renders, 0 flags; `hot` frame removed; value in AF_M for `KEYS`, picture enums and `PATH` (13 distinct cells); `LEGATO` 55 / `OFFSET` 54 px in 58 (margins 1-2); `127` only on ROW 3; selected + muted + OFF shows the `M` chip and `--` |

## Ten most important findings

1. The detector printed `[]` with exit 0 but scanned zero files (the folder holds `.md`, `.png`, `.py`, `.tsv` only; it scans web markup/CSS/JS/TS): a non-scan, not a clean result, as for v3.
2. A fresh render of `proposal_v3_2.py` is byte-identical to the 213 committed PNGs (0 differing pixels in all 71 scenes) with no tool warning, and all 156 knob cells, 408 channel-tile combinations, 36 dice toasts and 18 MAP variants rendered with no clip or ellipsis.
3. On a glance screen while playing there are 28 strings in 11 roles (AF_L 4, AF_M 24, AF_S 0), up to 14 roles on `mutes` / `stress`; AF_M (9.5 arcmin) is the smallest glance size, AF_S (7.7 arcmin) appears only on MAP page A (22 strings).
4. Measured on the pixels, every channel tile, lane column, knob tile, ROW cell and page box sits on the stated 15 x 4 px module with 2 px horizontal gaps, but the vertical gaps between stacked cells are 1, 1, 1, 5, 2, 2, 2 px, the transport triangle is 12x14 (TSV 13x14), and 7 TSV rows describe elements not drawn in their scene.
5. On GREY one text pair is under 4.5:1 (muted ratchet digit on the second lane shade, 3.54); the dimmed numeral of a muted selected channel is under 4.5:1 on 6 of 9 palettes (3.17 PAPER to 4.20 GREEN); 24 of the 79 meaning-bearing non-text pairs are under 3:1 on GREY (beat shading 1.29, tile fills 1.72, meter track 1.40, tie on playhead 1.11, edited foot vs baseline 1.37, hatch 2.87), and on MONO the playhead, accent cap and REC colours are all TEXT.
6. The dice toast's hint (`EDIT: UNDO`, `NOTHING CHANGED`) is drawn in only 14 of 36 algorithm x layer toasts (never for ALL); the SNAKE / ARTIC "layer absent" toast shows `NO ARTIC IN SNAKE` without the `NOTHING CHANGED` hint NOTES describes; the undo/redo toast draws its hint unconditionally, and 8 of 12 plausible detail strings overlap it.
7. All 37 cue strings fit one AF_M line (widest 239 of 240 px, `SAVE HOLD = UNDO PRESET LOAD`); no cue exists for B_GLO hold, B_HOME hold, CHN hold (RIG), the white-key tap (STEP) / hold (EDIT) or both-OCT hold, and 5 cue strings are in no cue-table render.
8. Against the spec files: MAP B omits the `PRV1..4` hold roles of M1..M4 (`key_map.tsv:211,224,237,250`) and the `RIG` hold of CHN (`:263`); MAP A labels OCT+ `ROW+` (spec `ROW 2`, `button_modes.tsv:237`) and HOME `CANCEL` / `MENU` (spec `HOME` / `HLP/MN`, `:110-111`); ARP SPAN reads `1` (spec `1 OCT`, `knob_map.tsv:36`); SHAPE dice toasts omit `+ MOD` (`ALGORITHM_SET.md:136`); 7 spec toasts (`NOTHING TO RESET`, `LAST ROW: OCT- = ROW 1`, `ROW 3 NEEDS A WIRE: HOLD CHN`, ...) are not drawn.
9. Full-lane playhead on steps 1, 8, 16: the slide chain is one 238 px stripe with a longest BG run of 1 px (v3: 15), the accent cap and the bar are both BG in the playing column (distinguished by shape and position), velocity 1, 2, 3 draw identical 4 px bars (44 px different from a rest), ratchet counts 4..8 are pixel-identical, and an off-page playhead is a 343 px ghost column plus a filled page box.
10. State coverage: `EXT lost` leaves the header identical to the internal-clock header (0 px differ; only the cue says so), the preset settle bar is a constant 17 of 34 px when no toast is on screen, FOLLOW off has no persistent indicator beyond the ghost column and page boxes, the first-run frame is flush against the MIDI arrow, and the tile text for MIDI 10-16 plus a mute chip has 2 px clearance.
