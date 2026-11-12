<!-- SPDX-License-Identifier: GPL-3.0-only -->
# EVIDENCE B (deterministic) for UI proposal v3-glance

Facts and measurements only. Source: `docs/design/ui_review/proposals/v3-glance/proposal_v3.py` (not modified), rendered with the pixel-exact tool
`docs/design/ui_review/mock/` (real AF_S / AF_M / AF_L fonts, GREY palette). Scale: 240 px = 30 mm, so 1 px = 0.125 mm. Angular size = `cap_mm / 500 * 3438`
arcmin at 50 cm. Contrast = WCAG 2 ratio from the tool's `contrast_ratio` on the RGB565 values (nominal fg vs bg; anti-aliased glyph edges are not
included). Nothing here is hardware-tested. `NOTES.md` claims were read only after the measurements were planned; each numeric claim is checked below.

Reproduction (repo root, `.venv/bin/python`): scripts in `docs/design/ui_review/critique_v3/scripts/` (one per section; stdout saved beside them in `data/`),
fixture builders `evidence_b_edge.py`, `evidence_b_sweep.py` (write `edge/`), shared helpers `evidence_b_lib.py`. Each script is run as
`.venv/bin/python docs/design/ui_review/critique_v3/scripts/NAME.py` from the repo root (they import `evidence_b_lib` by path).

## 1. Detector attempt

Command: `node /Users/jameskeaney/.claude/plugins/cache/impeccable/impeccable/3.8.0/skills/impeccable/scripts/detect.mjs --json docs/design/ui_review/proposals/v3-glance`

| item | result |
| --- | --- |
| stdout, exactly | `[]` |
| exit code | 0 |
| plain (non-JSON) run, same path | printed nothing, exit 0 |
| extensions the detector scans (from `scripts/detector/node/file-system.mjs`, `SCANNABLE_EXTENSIONS`) | `.html .htm .css .scss .sass .less .jsx .tsx .js .ts .vue .svelte .astro` |
| files in `proposals/v3-glance` | 4 `.py`, 52 `.png`, 1 `.md` (+ `__pycache__`): none has a scannable extension |
| verdict | **Nothing was scannable.** The `[]` is "no files scanned", not "scanned and clean". The tool reads web markup/CSS/JS; this proposal is Python drawing RGB565 pixels, so no detector rule was applied to it. |

## 2. Re-render and diff against the committed renders

Command: `.venv/bin/python docs/design/ui_review/mock/mock.py render docs/design/ui_review/proposals/v3-glance/proposal_v3.py --out /tmp/v3r`
then `mock.py diff NEW COMMITTED` per scene and a byte comparison of every PNG.

| scene | differing pixels (of 57600) |
| --- | --- |
| glance_idle | 0 |
| glance_playing | 0 |
| glance_playing_mono | 0 |
| map | 0 |
| mutes | 0 |
| row2 | 0 |
| stress | 0 |
| stress_mono | 0 |
| stress_row2 | 0 |
| toast_algo | 0 |
| toast_dice_notes | 0 |
| toast_preset_pending | 0 |
| toast_undo | 0 |

- 13 scenes rendered; all 13 1x PNGs show 0 differing pixels, and all 39 PNGs (1x, `_4x`, `_grid`) are byte-identical to the committed files.
- Tool warnings printed during the render (clipped text, ellipsised text, canvas over 240x124): **none**. Largest strip: blit C 240x96 = 23040 px (limit 29760).
- Author's NOTES claim "Proposal scenes: none (no cut, no ellipsised text, no strip over 240x124), 13 scenes": reproduced.

## 3. Text inventory

Per-string tables for the five glance screens (idle, playing, row2, mutes, stress) and the MAP. "advance" = `text_w` (firmware advance); "ink" = ink box from the real
glyph bitmaps; cap px from `mock.measure` (AF_S 9, AF_M 11, AF_L 21). Cap mm = px/8; arcmin = mm/500*3438: **AF_S 9 px = 1.125 mm = 7.7 arcmin; AF_M 11 px = 1.375 mm = 9.5
arcmin; AF_L 21 px = 2.625 mm = 18.0 arcmin** (18.05 unrounded; the author's 7.7 / 9.5 / 18.0 reproduce). Contrast column = fg vs bg of that string.

### 3.1 Counts

| scene | strings drawn | AF_L | AF_M | AF_S | distinct roles |
| --- | --- | --- | --- | --- | --- |
| glance_idle | 21 | 8 | 13 | 0 | 9 |
| glance_playing | 20 | 8 | 12 | 0 | 8 |
| row2 | 21 | 8 | 13 | 0 | 9 |
| mutes | 20 | 7 | 13 | 0 | 8 |
| stress | 20 | 8 | 12 | 0 | 8 |
| stress_row2 / glance_playing_mono / stress_mono | 20 / 20 / 20 | 8 | 12 | 0 | 8 |
| toast_algo / preset_pending / dice_notes / undo | 14 each | 5 | 9 | 0 | 8 |
| MAP | 82 | 0 | 0 | 82 | 16 |

- AF_S on any glance screen or toast: **0** (checked all 9 glance/toast scenes).
- Distinct roles over the five glance scenes together: 10 (bpm, preset, chan number, algo name, OFF chip, MUTE chip, knob label, knob value, knob value (M), cue); adding the toast title and detail gives the author's 12.
- Author claims reproduced: 20 strings playing, 21 idle (8 L + 12-13 M), 14 under a toast, 8 roles playing and 9 stopped (the cue), 0 AF_S, MAP 82 strings in 16 roles all AF_S, "no clipped text".
- Text clipped or ellipsised by the tool on any scene: none (no flags). Ink wider than the cell it was centred in: one string, MAP `ACCNT` (button-tap role): ink 40 px in a 38 px cell.
- Author claim "ACCNT touches NOTES and SHAPE" measured: gap to `NOTES` 2 px (ink 41..78 vs 80..120), gap to `SHAPE` 1 px (121). No overlap. Claim `ALGORITHM` ink 65 in a 65 px cell: reproduced (advance 66). Claim steps 10..16 are 13 px ink in 14 px cells: reproduced.
- Other numeric claims measured: `MELODY` 59 and `TECHNO` 60 px AF_M (claimed 59-60); `KEYS` AF_L advance 70 / ink 68, `KEY` 53 / 52; `FWD` AF_L ink 60; `PATTRN` AF_M 54; `PATTERNS` AF_L 136 / ink 135; `1 OCT` AF_L advance 82; `EXT 126` AF_S 45, AF_M 57. All as claimed.
- Claim "4 knob values + 4 knob names + 4 channel numbers + 1 algorithm = 13 minimum strings": not a measurement; the screen draws 20 while playing because of 3 more algorithm names, BPM, preset and chips (8 + 12).

### 3.2 Glance screens, every string

#### glance_idle
| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | bpm | `120` | M | 27 | 26x13 | 11 | 1.375 | 9.5 | 31,6,57,19 | 12.33 |
| 2 | preset | `P01` | M | 27 | 24x13 | 11 | 1.375 | 9.5 | 110,6,134,19 | 12.33 |
| 3 | chan number | `1` | L | 18 | 10x21 | 21 | 2.625 | 18.0 | 11,31,21,52 | 10.42 |
| 4 | algo name | `ACID` | M | 33 | 33x13 | 11 | 1.375 | 9.5 | 13,54,46,67 | 10.42 |
| 5 | chan number | `2` | L | 18 | 15x21 | 21 | 2.625 | 18.0 | 68,31,83,52 | 7.19 |
| 6 | algo name | `EUCLID` | M | 51 | 50x13 | 11 | 1.375 | 9.5 | 65,54,115,67 | 7.19 |
| 7 | chan number | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 128,31,144,53 | 6.42 |
| 8 | algo name | `SNAKE` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 126,54,173,67 | 6.42 |
| 9 | OFF chip | `OFF` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 150,33,177,46 | 6.42 |
| 10 | chan number | `4` | L | 18 | 16x21 | 21 | 2.625 | 18.0 | 188,31,204,52 | 6.42 |
| 11 | algo name | `STAGE` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 187,54,232,67 | 6.42 |
| 12 | MUTE chip | `M` | M | 13 | 12x11 | 11 | 1.375 | 9.5 | 220,34,232,45 | 6.42 |
| 13 | knob label | `DENS` | M | 38 | 38x13 | 11 | 1.375 | 9.5 | 11,149,49,162 | 10.03 |
| 14 | knob value | `12` | L | 36 | 32x21 | 21 | 2.625 | 18.0 | 14,165,46,186 | 10.03 |
| 15 | knob label | `SLIDE` | M | 39 | 38x13 | 11 | 1.375 | 9.5 | 71,149,109,162 | 10.03 |
| 16 | knob value | `6` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 82,165,98,187 | 10.03 |
| 17 | knob label | `JUMP` | M | 40 | 40x12 | 11 | 1.375 | 9.5 | 130,150,170,162 | 10.03 |
| 18 | knob value | `5` | L | 18 | 15x22 | 21 | 2.625 | 18.0 | 142,165,157,187 | 10.03 |
| 19 | knob label | `ACCNT` | M | 50 | 50x13 | 11 | 1.375 | 9.5 | 185,149,235,162 | 10.03 |
| 20 | knob value | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 202,165,218,187 | 10.03 |
| 21 | cue | `HOLD HOME = MAP` | M | 133 | 133x13 | 11 | 1.375 | 9.5 | 53,222,186,235 | 12.33 |


#### glance_playing
| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | bpm | `120` | M | 27 | 26x13 | 11 | 1.375 | 9.5 | 31,6,57,19 | 12.33 |
| 2 | preset | `P01` | M | 27 | 24x13 | 11 | 1.375 | 9.5 | 110,6,134,19 | 12.33 |
| 3 | chan number | `1` | L | 18 | 10x21 | 21 | 2.625 | 18.0 | 11,31,21,52 | 10.42 |
| 4 | algo name | `ACID` | M | 33 | 33x13 | 11 | 1.375 | 9.5 | 13,54,46,67 | 10.42 |
| 5 | chan number | `2` | L | 18 | 15x21 | 21 | 2.625 | 18.0 | 68,31,83,52 | 7.19 |
| 6 | algo name | `EUCLID` | M | 51 | 50x13 | 11 | 1.375 | 9.5 | 65,54,115,67 | 7.19 |
| 7 | chan number | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 128,31,144,53 | 6.42 |
| 8 | algo name | `SNAKE` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 126,54,173,67 | 6.42 |
| 9 | OFF chip | `OFF` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 150,33,177,46 | 6.42 |
| 10 | chan number | `4` | L | 18 | 16x21 | 21 | 2.625 | 18.0 | 188,31,204,52 | 6.42 |
| 11 | algo name | `STAGE` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 187,54,232,67 | 6.42 |
| 12 | MUTE chip | `M` | M | 13 | 12x11 | 11 | 1.375 | 9.5 | 220,34,232,45 | 6.42 |
| 13 | knob label | `DENS` | M | 38 | 38x13 | 11 | 1.375 | 9.5 | 11,149,49,162 | 10.03 |
| 14 | knob value | `12` | L | 36 | 32x21 | 21 | 2.625 | 18.0 | 14,165,46,186 | 10.03 |
| 15 | knob label | `SLIDE` | M | 39 | 38x13 | 11 | 1.375 | 9.5 | 71,149,109,162 | 10.03 |
| 16 | knob value | `6` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 82,165,98,187 | 10.03 |
| 17 | knob label | `JUMP` | M | 40 | 40x12 | 11 | 1.375 | 9.5 | 130,150,170,162 | 10.03 |
| 18 | knob value | `5` | L | 18 | 15x22 | 21 | 2.625 | 18.0 | 142,165,157,187 | 10.03 |
| 19 | knob label | `ACCNT` | M | 50 | 50x13 | 11 | 1.375 | 9.5 | 185,149,235,162 | 10.03 |
| 20 | knob value | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 202,165,218,187 | 10.03 |


#### row2
| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | bpm | `120` | M | 27 | 26x13 | 11 | 1.375 | 9.5 | 31,6,57,19 | 12.33 |
| 2 | preset | `P01` | M | 27 | 24x13 | 11 | 1.375 | 9.5 | 110,6,134,19 | 12.33 |
| 3 | chan number | `1` | L | 18 | 10x21 | 21 | 2.625 | 18.0 | 11,31,21,52 | 10.42 |
| 4 | algo name | `ACID` | M | 33 | 33x13 | 11 | 1.375 | 9.5 | 13,54,46,67 | 10.42 |
| 5 | chan number | `2` | L | 18 | 15x21 | 21 | 2.625 | 18.0 | 68,31,83,52 | 7.19 |
| 6 | algo name | `EUCLID` | M | 51 | 50x13 | 11 | 1.375 | 9.5 | 65,54,115,67 | 7.19 |
| 7 | chan number | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 128,31,144,53 | 6.42 |
| 8 | algo name | `SNAKE` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 126,54,173,67 | 6.42 |
| 9 | OFF chip | `OFF` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 150,33,177,46 | 6.42 |
| 10 | chan number | `4` | L | 18 | 16x21 | 21 | 2.625 | 18.0 | 188,31,204,52 | 6.42 |
| 11 | algo name | `STAGE` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 187,54,232,67 | 6.42 |
| 12 | MUTE chip | `M` | M | 13 | 12x11 | 11 | 1.375 | 9.5 | 220,34,232,45 | 6.42 |
| 13 | knob label | `OFFSET` | M | 54 | 54x13 | 11 | 1.375 | 9.5 | 3,149,57,162 | 10.03 |
| 14 | knob value | `0` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 22,165,38,187 | 10.03 |
| 15 | knob label | `PEDAL` | M | 45 | 45x11 | 11 | 1.375 | 9.5 | 67,150,112,161 | 10.03 |
| 16 | knob value | `4` | L | 18 | 16x21 | 21 | 2.625 | 18.0 | 82,165,98,186 | 10.03 |
| 17 | knob label | `PUNCH` | M | 51 | 50x13 | 11 | 1.375 | 9.5 | 125,149,175,162 | 10.03 |
| 18 | knob value | `40` | L | 36 | 34x22 | 21 | 2.625 | 18.0 | 133,165,167,187 | 10.03 |
| 19 | knob label | `DIR` | M | 23 | 23x11 | 11 | 1.375 | 9.5 | 198,150,221,161 | 10.03 |
| 20 | knob value | `UP` | L | 36 | 35x22 | 21 | 2.625 | 18.0 | 192,165,227,187 | 10.03 |
| 21 | cue | `HOLD HOME = MAP` | M | 133 | 133x13 | 11 | 1.375 | 9.5 | 53,222,186,235 | 12.33 |


#### mutes
| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | bpm | `120` | M | 27 | 26x13 | 11 | 1.375 | 9.5 | 31,6,57,19 | 12.33 |
| 2 | preset | `P01` | M | 27 | 24x13 | 11 | 1.375 | 9.5 | 110,6,134,19 | 12.33 |
| 3 | chan number | `1` | L | 18 | 10x21 | 21 | 2.625 | 18.0 | 11,31,21,52 | 7.19 |
| 4 | algo name | `ACID` | M | 33 | 33x13 | 11 | 1.375 | 9.5 | 13,54,46,67 | 7.19 |
| 5 | chan number | `2` | L | 18 | 15x21 | 21 | 2.625 | 18.0 | 68,31,83,52 | 6.42 |
| 6 | algo name | `EUCLID` | M | 51 | 50x13 | 11 | 1.375 | 9.5 | 65,54,115,67 | 6.42 |
| 7 | MUTE chip | `M` | M | 13 | 12x11 | 11 | 1.375 | 9.5 | 100,34,112,45 | 6.42 |
| 8 | chan number | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 128,31,144,53 | 10.42 |
| 9 | algo name | `SNAKE` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 126,54,173,67 | 10.42 |
| 10 | chan number | `4` | L | 18 | 16x21 | 21 | 2.625 | 18.0 | 188,31,204,52 | 6.42 |
| 11 | algo name | `STAGE` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 187,54,232,67 | 6.42 |
| 12 | MUTE chip | `M` | M | 13 | 12x11 | 11 | 1.375 | 9.5 | 220,34,232,45 | 6.42 |
| 13 | knob label | `DENS` | M | 38 | 38x13 | 11 | 1.375 | 9.5 | 11,149,49,162 | 10.03 |
| 14 | knob value | `10` | L | 36 | 32x22 | 21 | 2.625 | 18.0 | 14,165,46,187 | 10.03 |
| 15 | knob label | `LEN` | M | 27 | 26x11 | 11 | 1.375 | 9.5 | 77,150,103,161 | 10.03 |
| 16 | knob value | `7` | L | 18 | 15x21 | 21 | 2.625 | 18.0 | 82,165,97,186 | 10.03 |
| 17 | knob label | `RANGE` | M | 49 | 48x13 | 11 | 1.375 | 9.5 | 126,149,174,162 | 10.03 |
| 18 | knob value | `12` | L | 36 | 32x21 | 21 | 2.625 | 18.0 | 134,165,166,186 | 10.03 |
| 19 | knob label | `PATH` | M | 36 | 35x11 | 11 | 1.375 | 9.5 | 192,150,227,161 | 10.03 |
| 20 | knob value (M) | `SNK3` | M | 38 | 38x13 | 11 | 1.375 | 9.5 | 191,169,229,182 | 10.03 |


#### stress
| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | bpm | `EXT 126` | M | 57 | 57x13 | 11 | 1.375 | 9.5 | 30,6,87,19 | 12.33 |
| 2 | preset | `P16` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 110,6,137,19 | 12.33 |
| 3 | chan number | `1` | L | 18 | 10x21 | 21 | 2.625 | 18.0 | 11,31,21,52 | 7.19 |
| 4 | algo name | `ARP` | M | 28 | 28x11 | 11 | 1.375 | 9.5 | 16,55,44,66 | 7.19 |
| 5 | chan number | `2` | L | 18 | 15x21 | 21 | 2.625 | 18.0 | 68,31,83,52 | 7.19 |
| 6 | algo name | `PATT` | M | 35 | 34x11 | 11 | 1.375 | 9.5 | 73,55,107,66 | 7.19 |
| 7 | chan number | `3` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 128,31,144,53 | 7.19 |
| 8 | algo name | `SNAKE` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 126,54,173,67 | 7.19 |
| 9 | OFF chip | `OFF` | M | 27 | 27x13 | 11 | 1.375 | 9.5 | 150,33,177,46 | 7.19 |
| 10 | chan number | `4` | L | 18 | 16x21 | 21 | 2.625 | 18.0 | 188,31,204,52 | 10.42 |
| 11 | algo name | `STAGE` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 187,54,232,67 | 10.42 |
| 12 | MUTE chip | `M` | M | 13 | 12x11 | 11 | 1.375 | 9.5 | 220,34,232,45 | 10.42 |
| 13 | knob label | `DENS` | M | 38 | 38x13 | 11 | 1.375 | 9.5 | 11,149,49,162 | 10.03 |
| 14 | knob value | `8` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 22,165,38,187 | 10.03 |
| 15 | knob label | `TOTAL` | M | 45 | 45x13 | 11 | 1.375 | 9.5 | 67,149,112,162 | 10.03 |
| 16 | knob value | `64` | L | 36 | 34x22 | 21 | 2.625 | 18.0 | 73,165,107,187 | 10.03 |
| 17 | knob label | `DIR` | M | 23 | 23x11 | 11 | 1.375 | 9.5 | 138,150,161,161 | 10.03 |
| 18 | knob value | `P-P` | L | 45 | 44x21 | 21 | 2.625 | 18.0 | 128,165,172,186 | 10.03 |
| 19 | knob label | `RATCH` | M | 47 | 47x13 | 11 | 1.375 | 9.5 | 186,149,233,162 | 10.03 |
| 20 | knob value | `8` | L | 18 | 16x22 | 21 | 2.625 | 18.0 | 202,165,218,187 | 10.03 |


### 3.3 The MAP (HOME held)

| role | font | count | strings | min contrast | max advance |
|---|---|---|---|---|---|
| map title | S | 1 | `TOUCH = HELP   LET GO = MENU` | 12.33 | 176 |
| map encoder name | S | 4 | `MASTER`, `SELECT`, `PRESETS`, `ALGORITHM` | 5.23 | 66 |
| map encoder role | S | 4 | `VOLUME`, `CHANNEL`, `P01-P16`, `OF CH 1` | 10.03 | 54 |
| map OCT name | S | 2 | `OCT-`, `OCT+` | 5.23 | 30 |
| map OCT role | S | 2 | `ROW 1`, `ROW 2` | 10.03 | 36 |
| map knob id | S | 4 | `K1`, `K2`, `K3`, `K4` | 6.42 | 14 |
| map knob row1 | S | 4 | `DENS`, `SLIDE`, `JUMP`, `ACCNT` | 12.33 | 39 |
| map knob row2 | S | 4 | `OFFSET`, `PEDAL`, `PUNCH`, `DIR` | 6.42 | 42 |
| map legend | S | 1 | `= HOLD` | 12.33 | 41 |
| map button printed | S | 12 | `FX`, `SCL`, `ENV`, `LFO`, `EDIT`, `GLO`, `HOME`, `SAVE` ... | 5.23 | 34 |
| map button tap | S | 12 | `ALL`, `NOTES`, `ACCNT`, `SHAPE`, `UNDO`, `SET`, `HOME`, `SAVE` ... | 10.03 | 39 |
| map button hold | S | 4 | `RESET`, `HARM`, `MENU`, `BACK` | 10.03 | 35 |
| map black key | S | 8 | `M1`, `M2`, `M3`, `M4`, `CHN`, `PG-`, `PG+`, `RST` | 12.33 | 25 |
| map black key hold | S | 3 | `SOL`, `FIL`, `ROL` | 12.33 | 22 |
| map caption | S | 1 | `STEPS 1-16: TAP ON/OFF  HOLD EDIT` | 12.33 | 199 |
| map white key | S | 16 | `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8` ... | 7.19 | 14 |
MAP total strings (all AF_S) 82 roles 16 fonts {'AF_L': 0, 'AF_M': 0, 'AF_S': 0, 'L': 0, 'M': 0, 'S': 82}

Roles with fewer than 2 px between neighbouring ink boxes on the MAP (82 strings pairwise): one pair, `ACCNT` / `SHAPE` (1 px). Ink reaching the screen edge: `OFFSET` (x 196..239) and `RST` (218..239): last ink column 238, 1 px from the edge.
Bottom of MAP ink: y 232 (blit ends y 234).

## 4. Contrast

GREY is the default palette; in it `T_ACCENT = T_REC = T_TEXT`, so the proposal's three new literal colours are the only colour on the screen. The monochrome fallback maps all three to `T_TEXT`.
Thresholds: 4.5:1 text; 3:1 large text (AF_L) and non-text elements that carry meaning. "NT" = non-text. Rows marked UNDER are below the threshold for the pair as stated; the
last column says nothing about whether the pair is the one that carries the meaning (see notes under the table).

GREY tokens: BG #080808, SURF #202020, TEXT #cdcacd, MID #949194, DIM #5a595a, LINE #292829, SEL #bdbabd, INK #080808 (= BG), RAISE #393839 (= LANE), KEY #a4a1a4. New: T_PLAYHEAD 0xFD80 #ffb000 (255,176,0), T_ACCSTEP 0x565F (80,200,255), T_RECRED 0xFA49 (255,72,72).

| # | pair | fg | bg | GREY ratio | need | verdict |
|---|---|---|---|---|---|---|
| 1 | TEXT: bpm/preset/cue AF_M | TEXT | BG | 12.33:1 | 4.5 | ok |
| 2 | TEXT: unselected channel number+algo AF_L/M on RAISE | TEXT | RAISE | 7.19:1 | 4.5 | ok |
| 3 | TEXT: selected channel number+algo on SEL (INK on SEL) | INK | SEL | 10.42:1 | 4.5 | ok |
| 4 | TEXT: muted channel number/algo/OFF text MID on BG | MID | BG | 6.42:1 | 4.5 | ok |
| 5 | TEXT: M letter on muted chip (INK on MID) | INK | MID | 6.42:1 | 4.5 | ok |
| 6 | TEXT: M letter on selected muted chip (SEL on INK) | SEL | INK | 10.42:1 | 4.5 | ok |
| 7 | TEXT: knob label/value TEXT on SURF | TEXT | SURF | 10.03:1 | 4.5 | ok |
| 8 | TEXT: toast title/detail INK on SEL | INK | SEL | 10.42:1 | 4.5 | ok |
| 9 | TEXT MAP: encoder/OCT/printed name MID on SURF | MID | SURF | 5.23:1 | 4.5 | ok |
| 10 | TEXT MAP: white-key number TEXT on RAISE | TEXT | RAISE | 7.19:1 | 4.5 | ok |
| 11 | TEXT MAP: knob ids MID on BG | MID | BG | 6.42:1 | 4.5 | ok |
| 12 | NT playhead column PH vs LANE column | PH | LANE | 6.46:1 | 3.0 | ok |
| 13 | NT playhead column PH vs BG (gap, neighbours) | PH | BG | 11.08:1 | 3.0 | ok |
| 14 | NT bar inside playhead column BG on PH | BG | PH | 11.08:1 | 3.0 | ok |
| 15 | NT normal bar TEXT on LANE | TEXT | LANE | 7.19:1 | 3.0 | ok |
| 16 | NT accent bar AC on LANE | AC | LANE | 6.25:1 | 3.0 | ok |
| 17 | NT accent bar AC vs normal bar TEXT (distinguishing colour) | AC | TEXT | 1.15:1 | 3.0 | UNDER |
| 18 | NT accent triangle AC on LANE | AC | LANE | 6.25:1 | 3.0 | ok |
| 19 | NT slide tie bar TEXT on LANE | TEXT | LANE | 7.19:1 | 3.0 | ok |
| 20 | NT slide tie bar in the 1 px gap column (x+14) TEXT on BG | TEXT | BG | 12.33:1 | 3.0 | ok |
| 21 | NT rest dash MID on LANE | MID | LANE | 3.75:1 | 3.0 | ok |
| 22 | NT beat pip MID on LANE | MID | LANE | 3.75:1 | 3.0 | ok |
| 23 | NT lane column LANE vs BG | LANE | BG | 1.72:1 | 3.0 | UNDER |
| 24 | NT muted selected hollow bar frame MID on LANE | MID | LANE | 3.75:1 | 3.0 | ok |
| 25 | NT muted unselected cell: frame MID on BG | MID | BG | 6.42:1 | 3.0 | ok |
| 26 | NT live unselected cell fill RAISE vs BG | RAISE | BG | 1.72:1 | 3.0 | UNDER |
| 27 | NT selected cell fill SEL vs BG | SEL | BG | 10.42:1 | 3.0 | ok |
| 28 | NT selected SEL vs unselected RAISE | SEL | RAISE | 6.07:1 | 3.0 | ok |
| 29 | NT muted cell (BG+MID frame) vs live cell (RAISE)  [fill vs fill] | BG | RAISE | 1.72:1 | 3.0 | UNDER |
| 30 | NT M chip fill MID vs BG | MID | BG | 6.42:1 | 3.0 | ok |
| 31 | NT M chip fill INK(sel) vs SEL cell | INK | SEL | 10.42:1 | 3.0 | ok |
| 32 | NT struck-through line MID on BG | MID | BG | 6.42:1 | 3.0 | ok |
| 33 | NT strike INK on SEL (selected muted) | INK | SEL | 10.42:1 | 3.0 | ok |
| 34 | NT OFF chip frame+text TEXT on RAISE (live, unselected, MIDI OFF) | TEXT | RAISE | 7.19:1 | 3.0 | ok |
| 35 | NT OFF chip frame+text MID on BG (muted, unselected, MIDI OFF) | MID | BG | 6.42:1 | 3.0 | ok |
| 36 | NT activity square PH on RAISE | PH | RAISE | 6.46:1 | 3.0 | ok |
| 37 | NT activity square INK on SEL | INK | SEL | 10.42:1 | 3.0 | ok |
| 38 | NT meter fill TEXT vs track LANE | TEXT | LANE | 7.19:1 | 3.0 | ok |
| 39 | NT meter track LANE vs cell SURF | LANE | SURF | 1.40:1 | 3.0 | UNDER |
| 40 | NT enum pips unselected MID on SURF | MID | SURF | 5.23:1 | 3.0 | ok |
| 41 | NT enum pip selected TEXT on SURF | TEXT | SURF | 10.03:1 | 3.0 | ok |
| 42 | NT hot-knob frame TEXT on SURF | TEXT | SURF | 10.03:1 | 3.0 | ok |
| 43 | NT knob cell SURF vs BG | SURF | BG | 1.23:1 | 3.0 | UNDER |
| 44 | NT ROW bar live TEXT on BG | TEXT | BG | 12.33:1 | 3.0 | ok |
| 45 | NT ROW bar I/II INK on TEXT | INK | TEXT | 12.33:1 | 3.0 | ok |
| 46 | NT ROW bar idle frame MID on BG | MID | BG | 6.42:1 | 3.0 | ok |
| 47 | NT ROW bar idle numerals MID on BG | MID | BG | 6.42:1 | 3.0 | ok |
| 48 | NT page box viewed TEXT on BG | TEXT | BG | 12.33:1 | 3.0 | ok |
| 49 | NT page box other MID on BG | MID | BG | 6.42:1 | 3.0 | ok |
| 50 | NT page playing marker PH on BG (hollow page) | PH | BG | 11.08:1 | 3.0 | ok |
| 51 | NT page playing marker BG on TEXT (viewed page) | BG | TEXT | 12.33:1 | 3.0 | ok |
| 52 | NT transport play PH on BG | PH | BG | 11.08:1 | 3.0 | ok |
| 53 | NT transport stop MID on BG | MID | BG | 6.42:1 | 3.0 | ok |
| 54 | NT REC disc RD on BG | RD | BG | 5.99:1 | 3.0 | ok |
| 55 | NT toast algo unchosen pips MID on SEL | MID | SEL | 1.62:1 | 3.0 | UNDER |
| 56 | NT toast countdown track MID on SEL | MID | SEL | 1.62:1 | 3.0 | UNDER |
| 57 | NT toast fill INK on SEL | INK | SEL | 10.42:1 | 3.0 | ok |
| 58 | NT toast block SEL vs BG | SEL | BG | 10.42:1 | 3.0 | ok |

Mono fallback (PH=AC=RD=TEXT): 
| playhead col TEXT vs LANE | 7.19:1 |
| bar in playhead column BG on TEXT | 12.33:1 |
| accent bar vs normal bar (both TEXT) | 1.00:1 |
| REC disc TEXT vs BG | 12.33:1 |
| play triangle TEXT vs BG | 12.33:1 |
| activity square TEXT vs RAISE | 7.19:1 |
| playing page marker TEXT on BG | 12.33:1 |
| playing page marker on hollow page | 12.33:1 |
new tokens vs each other: PH-AC 1.03, PH-RD 1.85, AC-RD 1.79
new tokens vs TEXT: PH 1.11 AC 1.15 RD 2.06
GREY RD in mono = TEXT; triangle vs disc: shape only
pixel samples glance_playing: playhead col (5*15+7,100) 0xFD80 lane col other (7,100) 0x39C7 gap (14,100) 0x0841

Notes (facts):
- Every **text** pair in the table is at or above 4.5:1 (lowest 5.23:1: MID on SURF, MAP only; lowest on a glance screen 6.42:1: MID on BG for muted cells and INK on MID for the `M` chip). AF_L text lowest 6.42:1 (muted channel number, MID on BG). No text pair is under threshold.
- Under 3:1 (non-text): pair 17 accent bar (cyan) vs normal bar (TEXT) is 1.15:1 in luminance (hue differs; in the monochrome fallback the ratio is 1.00:1, identical colour; the up-triangle in mark row A is the only accent cue there).
  Pairs 23, 26, 29, 39, 43 are fill-versus-ground relations (lane column vs gap 1.72, unselected channel fill vs BG 1.72, meter track vs cell 1.40, knob cell vs BG 1.23). Pairs 55 and 56: the unchosen toast pips and the countdown track are MID on SEL at 1.62:1; the filled/tall element against them is INK vs MID 6.42:1.
  For the meter, fill vs track is TEXT vs LANE 7.19:1 (pair 38) and the cell/track edge is 1.40:1.
- MID on RAISE/LANE is 3.75:1 and is used for the hollow bars of a muted channel, the rest dash and the beat pips (shapes). That is above 3:1 and under 4.5:1; no text uses it.
- The M chip letter is INK on MID 6.42:1 (unselected) and SEL on INK 10.42:1 (selected). Struck-through numerals: the numeral is MID on BG 6.42:1 (unselected muted) and the strike MID on BG 6.42:1; selected muted: INK on SEL 10.42:1.
- The playhead column vs lane: 6.46:1 (GREY) and TEXT vs LANE 7.19:1 (monochrome fallback). Bars inside the playing column are BG on amber 11.08:1 (mono: BG on TEXT 12.33:1); there they lose the accent colour (accent bar and normal bar both BG).
- New tokens against each other, luminance: amber vs cyan 1.03:1, amber vs red 1.85:1, cyan vs red 1.79:1; amber vs TEXT 1.11:1, cyan vs TEXT 1.15:1, red vs TEXT 2.06:1.
- Author's contrast table (NOTES "Contrast of what is drawn") reproduced: TEXT on BG 12.33, RAISE 7.19, SURF 10.03, INK on SEL 10.42, MID on BG 6.42, MID on RAISE/LANE 3.75, MID on SURF 5.23, T_PLAYHEAD on BG 11.08 / LANE 6.46, T_ACCSTEP on BG 10.73 (not re-listed) / LANE 6.25, T_RECRED on BG 5.99 / LANE 3.49 (REC disc is drawn on BG only).

## 5. Grid alignment

Zone tiling (assert in the file plus the rendered marks): Z0 y0..24, Z1 24..72, Z2 72..144, Z3 144..220, Z4 220..236, Z5 236..240: contiguous, sum 240, no gap, no overlap; blits A 240x72 at y0, B 240x72 at y72, C 240x96 at y144.
The zone *heights* are 24, 48, 72, 76, 16, 4: 24, 48, 72, 76, 16, 4 mod 5 = 4, 3, 2, 1, 1, 4. Zone boundaries y = 24, 72, 144, 220, 236 mod 5 = 4, 2, 4, 0, 1.

### 5.1 Geometry against the 5 px module (screen coordinates; scanlines in `data/s5_scanlines.txt`)

| element | x | y | w | h | x%5 | y%5 | w%5 | h%5 | right edge | bottom edge | on 5px grid? |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Z0 transport glyph | 6 | 4 | 15 | 16 | 1 | 4 | 0 | 1 | 21 | 20 | no |
| page box 1 | 172 | 5 | 14 | 14 | 2 | 0 | 4 | 4 | 186 | 19 | no |
| page box 2 | 188 | 5 | 14 | 14 | 3 | 0 | 4 | 4 | 202 | 19 | no |
| page box 3 | 204 | 5 | 14 | 14 | 4 | 0 | 4 | 4 | 218 | 19 | no |
| page box 4 | 220 | 5 | 14 | 14 | 0 | 0 | 4 | 4 | 234 | 19 | no |
| page playing marker (box1) | 177 | 10 | 4 | 4 | 2 | 0 | 4 | 4 | 181 | 14 | no |
| Z1 cell 1 | 1 | 26 | 58 | 44 | 1 | 1 | 3 | 4 | 59 | 70 | no |
| Z1 cell 2 | 61 | 26 | 58 | 44 | 1 | 1 | 3 | 4 | 119 | 70 | no |
| Z1 cell 3 | 121 | 26 | 58 | 44 | 1 | 1 | 3 | 4 | 179 | 70 | no |
| Z1 cell 4 | 181 | 26 | 58 | 44 | 1 | 1 | 3 | 4 | 239 | 70 | no |
| Z1 number box (cell1) | 4 | 31 | 24 | 21 | 4 | 1 | 4 | 1 | 28 | 52 | no |
| Z1 strike (cell1) | 7 | 40 | 20 | 3 | 2 | 0 | 0 | 3 | 27 | 43 | no |
| Z1 OFF chip (cell1) | 29 | 30 | 29 | 18 | 4 | 0 | 4 | 3 | 58 | 48 | no |
| Z1 M chip (cell1) | 35 | 30 | 22 | 18 | 0 | 0 | 2 | 3 | 57 | 48 | no |
| Z1 activity square (cell1) | 41 | 32 | 14 | 14 | 1 | 2 | 4 | 4 | 55 | 46 | no |
| Z2 lane column n=0 | 0 | 72 | 14 | 70 | 0 | 2 | 4 | 0 | 14 | 142 | no |
| Z2 lane column n=15 | 225 | 72 | 14 | 70 | 0 | 2 | 4 | 0 | 239 | 142 | no |
| Z2 accent triangle | 1 | 75 | 11 | 6 | 1 | 0 | 1 | 1 | 12 | 81 | no |
| Z2 tie bar | 0 | 83 | 15 | 6 | 0 | 3 | 0 | 1 | 15 | 89 | no |
| Z2 bar (full width) | 1 | 90 | 12 | 44 | 1 | 0 | 2 | 4 | 13 | 134 | no |
| Z2 rest dash | 1 | 130 | 12 | 4 | 1 | 0 | 2 | 4 | 13 | 134 | no |
| Z2 beat pip | 4 | 137 | 6 | 3 | 4 | 2 | 1 | 3 | 10 | 140 | no |
| Z3 knob cell 1 | 1 | 144 | 58 | 66 | 1 | 4 | 3 | 1 | 59 | 210 | no |
| Z3 knob cell 2 | 61 | 144 | 58 | 66 | 1 | 4 | 3 | 1 | 119 | 210 | no |
| Z3 knob cell 3 | 121 | 144 | 58 | 66 | 1 | 4 | 3 | 1 | 179 | 210 | no |
| Z3 knob cell 4 | 181 | 144 | 58 | 66 | 1 | 4 | 3 | 1 | 239 | 210 | no |
| Z3 meter (cell1) | 7 | 194 | 46 | 10 | 2 | 4 | 1 | 0 | 53 | 204 | no |
| Z3 ROW bar left | 1 | 210 | 118 | 10 | 1 | 0 | 3 | 0 | 119 | 220 | no |
| Z3 ROW bar right | 121 | 210 | 118 | 10 | 1 | 0 | 3 | 0 | 239 | 220 | no |
| Z4 cue strip | 0 | 220 | 240 | 16 | 0 | 0 | 0 | 1 | 240 | 236 | no |
| toast block | 0 | 146 | 240 | 88 | 0 | 1 | 0 | 3 | 240 | 234 | no |
| toast countdown | 40 | 214 | 160 | 10 | 0 | 4 | 0 | 0 | 200 | 224 | no |
| toast algo pips | 45 | 212 | 20 | 14 | 0 | 2 | 0 | 4 | 65 | 226 | no |

- Of the 35 elements listed, 0 have x, y, w and h all multiples of 5. Individual edges that land on a multiple of 5: the channel cell bottom (y70), the ROW bar top and bottom (y210, y220), the Z4 strip (y220), the lane column pitch (15) and the lane column start (x0); none of these elements is 5-aligned on all four values (the lane column is 14 wide, the channel cell 58x44 at x1/y26, the knob cell 58x66 at x1/y144).
- Channel cells: x 1, 61, 121, 181, w 58, y 26..70 (h 44); a 2 px BG band above (24..26) and below (70..72); 2 px between cells, 1 px at the outer left and right.
- Knob cells: same x and w as channel cells (1/61/121/181, w58), y 144..210 (h66), directly under a 2 px BG band (142..144).
- Lane columns: x = 15 n, w 14, 1 px gap; first column starts at x 0 (0 px left margin), last ends at x 239 (1 px right margin). Lane column edges at x 60, 120, 180 against channel/knob cell edges at 61, 121, 181: **1 px offset** between the lane's 4-step groups and the cells above and below.
  Lane is 70 px tall (y 72..142), 2 px BG below, and the channel cell above ends at y70 + 2 px band.
- Page boxes: x 172, 188, 204, 220 (w14, 2 px gap), y 5..19; right margin 240-234 = 6 px; the transport glyph starts at x6.
- Header text: BPM ink starts x31 (x30 origin), preset ink starts x110; no shared column with the cells below (channel cells at 1/61/121/181).
- Tie bar (slide): 15 px wide for steps 1..15 (covers the 1 px gap), 14 px for step 16; y 83..89. Accent triangle 11x6 at y 75..81 (x+1). Bars: x+1, w12 (single), y from 62-hh. Beat pip 6x3 at x+4, y137. Rest dash 12x4 at y130.
- Ratchet combs (bar width 5/3/2 inside a 12 px span): x2 gap 2; x3 gaps 1,2; x4 gaps 1,2,1 (uneven); x5 gaps 0,1,1,0; x6 gaps 0 (contiguous).
- Enum pips (46 px meter slot): n=3 and n=6 fill 46 px exactly (margins 0/0); n=7 is 40 px (3/3); n=14 is 41 px (2/3, uneven).
- Meter: x+6, w46 in a 58 px cell: 6 px margin each side; y 194..204 in a cell that ends at y210 (6 px below); value ink bottom to meter top: 7..8 px; label ink bottom to value ink top: 3 px; cell top to label cap top: 5..6 px.

### 5.2 Ink position in its cell (margins in px; centre offsets in px)

| scene | role | string | cell x,y,w,h | ink x0..x1 | L margin | R margin | ink centre - cell centre (x) | T margin | B margin | ink centre - cell centre (y) |
|---|---|---|---|---|---|---|---|---|---|---|
| glance_playing | chan number | `1` | 1,26,58,44 | 11..21 | 10 | 38 | -14.0 | 5 | 18 | -6.5 |
| glance_playing | algo name | `ACID` | 1,26,58,44 | 13..46 | 12 | 13 | -0.5 | 28 | 3 | +12.5 |
| glance_playing | chan number | `2` | 61,26,58,44 | 68..83 | 7 | 36 | -14.5 | 5 | 18 | -6.5 |
| glance_playing | algo name | `EUCLID` | 61,26,58,44 | 65..115 | 4 | 4 | +0.0 | 28 | 3 | +12.5 |
| glance_playing | chan number | `3` | 121,26,58,44 | 128..144 | 7 | 35 | -14.0 | 5 | 17 | -6.0 |
| glance_playing | algo name | `SNAKE` | 121,26,58,44 | 126..173 | 5 | 6 | -0.5 | 28 | 3 | +12.5 |
| glance_playing | OFF chip | `OFF` | 149,30,29,18 | 150..177 | 1 | 1 | +0.0 | 3 | 2 | +0.5 |
| glance_playing | chan number | `4` | 181,26,58,44 | 188..204 | 7 | 35 | -14.0 | 5 | 18 | -6.5 |
| glance_playing | algo name | `STAGE` | 181,26,58,44 | 187..232 | 6 | 7 | -0.5 | 28 | 3 | +12.5 |
| glance_playing | MUTE chip | `M` | 215,30,22,18 | 220..232 | 5 | 5 | +0.0 | 4 | 3 | +0.5 |
| glance_playing | knob label | `DENS` | 1,144,58,66 | 11..49 | 10 | 10 | +0.0 | 5 | 48 | -21.5 |
| glance_playing | knob value | `12` | 1,144,58,66 | 14..46 | 13 | 13 | +0.0 | 21 | 24 | -1.5 |
| glance_playing | knob label | `SLIDE` | 61,144,58,66 | 71..109 | 10 | 10 | +0.0 | 5 | 48 | -21.5 |
| glance_playing | knob value | `6` | 61,144,58,66 | 82..98 | 21 | 21 | +0.0 | 21 | 23 | -1.0 |
| glance_playing | knob label | `JUMP` | 121,144,58,66 | 130..170 | 9 | 9 | +0.0 | 6 | 48 | -21.0 |
| glance_playing | knob value | `5` | 121,144,58,66 | 142..157 | 21 | 22 | -0.5 | 21 | 23 | -1.0 |
| glance_playing | knob label | `ACCNT` | 181,144,58,66 | 185..235 | 4 | 4 | +0.0 | 5 | 48 | -21.5 |
| glance_playing | knob value | `3` | 181,144,58,66 | 202..218 | 21 | 21 | +0.0 | 21 | 23 | -1.0 |
| stress | chan number | `1` | 1,26,58,44 | 11..21 | 10 | 38 | -14.0 | 5 | 18 | -6.5 |
| stress | algo name | `ARP` | 1,26,58,44 | 16..44 | 15 | 15 | +0.0 | 29 | 4 | +12.5 |
| stress | chan number | `2` | 61,26,58,44 | 68..83 | 7 | 36 | -14.5 | 5 | 18 | -6.5 |
| stress | algo name | `PATT` | 61,26,58,44 | 73..107 | 12 | 12 | +0.0 | 29 | 4 | +12.5 |
| stress | chan number | `3` | 121,26,58,44 | 128..144 | 7 | 35 | -14.0 | 5 | 17 | -6.0 |
| stress | algo name | `SNAKE` | 121,26,58,44 | 126..173 | 5 | 6 | -0.5 | 28 | 3 | +12.5 |
| stress | OFF chip | `OFF` | 149,30,29,18 | 150..177 | 1 | 1 | +0.0 | 3 | 2 | +0.5 |
| stress | chan number | `4` | 181,26,58,44 | 188..204 | 7 | 35 | -14.0 | 5 | 18 | -6.5 |
| stress | algo name | `STAGE` | 181,26,58,44 | 187..232 | 6 | 7 | -0.5 | 28 | 3 | +12.5 |
| stress | MUTE chip | `M` | 215,30,22,18 | 220..232 | 5 | 5 | +0.0 | 4 | 3 | +0.5 |
| stress | knob label | `DENS` | 1,144,58,66 | 11..49 | 10 | 10 | +0.0 | 5 | 48 | -21.5 |
| stress | knob value | `8` | 1,144,58,66 | 22..38 | 21 | 21 | +0.0 | 21 | 23 | -1.0 |
| stress | knob label | `TOTAL` | 61,144,58,66 | 67..112 | 6 | 7 | -0.5 | 5 | 48 | -21.5 |
| stress | knob value | `64` | 61,144,58,66 | 73..107 | 12 | 12 | +0.0 | 21 | 23 | -1.0 |
| stress | knob label | `DIR` | 121,144,58,66 | 138..161 | 17 | 18 | -0.5 | 6 | 49 | -21.5 |
| stress | knob value | `P-P` | 121,144,58,66 | 128..172 | 7 | 7 | +0.0 | 21 | 24 | -1.5 |
| stress | knob label | `RATCH` | 181,144,58,66 | 186..233 | 5 | 6 | -0.5 | 5 | 48 | -21.5 |
| stress | knob value | `8` | 181,144,58,66 | 202..218 | 21 | 21 | +0.0 | 21 | 23 | -1.0 |
| mutes | chan number | `1` | 1,26,58,44 | 11..21 | 10 | 38 | -14.0 | 5 | 18 | -6.5 |
| mutes | algo name | `ACID` | 1,26,58,44 | 13..46 | 12 | 13 | -0.5 | 28 | 3 | +12.5 |
| mutes | chan number | `2` | 61,26,58,44 | 68..83 | 7 | 36 | -14.5 | 5 | 18 | -6.5 |
| mutes | algo name | `EUCLID` | 61,26,58,44 | 65..115 | 4 | 4 | +0.0 | 28 | 3 | +12.5 |
| mutes | MUTE chip | `M` | 95,30,22,18 | 100..112 | 5 | 5 | +0.0 | 4 | 3 | +0.5 |
| mutes | chan number | `3` | 121,26,58,44 | 128..144 | 7 | 35 | -14.0 | 5 | 17 | -6.0 |
| mutes | algo name | `SNAKE` | 121,26,58,44 | 126..173 | 5 | 6 | -0.5 | 28 | 3 | +12.5 |
| mutes | chan number | `4` | 181,26,58,44 | 188..204 | 7 | 35 | -14.0 | 5 | 18 | -6.5 |
| mutes | algo name | `STAGE` | 181,26,58,44 | 187..232 | 6 | 7 | -0.5 | 28 | 3 | +12.5 |
| mutes | MUTE chip | `M` | 215,30,22,18 | 220..232 | 5 | 5 | +0.0 | 4 | 3 | +0.5 |
| mutes | knob label | `DENS` | 1,144,58,66 | 11..49 | 10 | 10 | +0.0 | 5 | 48 | -21.5 |
| mutes | knob value | `10` | 1,144,58,66 | 14..46 | 13 | 13 | +0.0 | 21 | 23 | -1.0 |
| mutes | knob label | `LEN` | 61,144,58,66 | 77..103 | 16 | 16 | +0.0 | 6 | 49 | -21.5 |
| mutes | knob value | `7` | 61,144,58,66 | 82..97 | 21 | 22 | -0.5 | 21 | 24 | -1.5 |
| mutes | knob label | `RANGE` | 121,144,58,66 | 126..174 | 5 | 5 | +0.0 | 5 | 48 | -21.5 |
| mutes | knob value | `12` | 121,144,58,66 | 134..166 | 13 | 13 | +0.0 | 21 | 24 | -1.5 |
| mutes | knob label | `PATH` | 181,144,58,66 | 192..227 | 11 | 12 | -0.5 | 6 | 49 | -21.5 |
| mutes | knob value (M) | `SNK3` | 181,144,58,66 | 191..229 | 10 | 10 | +0.0 | 25 | 28 | -1.5 |

Facts from the table and the rows after it (`data/s5b_geometry_margins.txt`):
- Algorithm names, knob labels and knob values are centred on their cell within 0.5 px (x).
- The **channel number is not centred in its 58 px cell**: ink centre is 14 to 14.5 px left of the cell centre (ink x 11..21 for `1`, 68..83 for `2` in cells 1..59 / 61..119; left margin 7..10, right margin 35..38). It is centred in a 24 px box at x+3.
- Channel-cell vertical stack (selected or not): number cap top 5 px below the cell top; number ink to algorithm-name ink gap 1 to 2 px (`3` is 1 px); algorithm ink to cell bottom 3 px (`ARP`/`PATT` 4 px).
- Knob-cell vertical stack: label cap top 5..6 px from the cell top; label to value gap 3 px; value to meter 7..8 px; meter to cell bottom 6 px.
- Right-hand items in a channel cell sit at three different x and sizes: OFF chip x+28, 29x18; M chip x+34, 22x18; activity square x+40, 14x14 (y+6). Right edges: chip OFF 57, M chip 56, square 54 of a 58 px cell.
- OFF chip: frame 29x18 with 2 px stroke (inner 25x14); the word `OFF` ink is 27 px wide, so the ink overlaps the stroke by 1 px on the left and 1 px on the right (ink 150..177 in a chip at 149..178); ink bottom row 45 touches the stroke (rows 46..47); top inner row 32, ink starts 33.
- `M` chip: 22x18 solid, `M` ink 12x11 centred (5 px left/right, 4 top, 3 bottom).
- Header: `EXT 126` ink 30..87 and `P16` ink 110..137: 23 px between them; `P16?` ink 110..144; first page box at 172.

## 6. Edge-case sweep

Fixtures are built by `scripts/../evidence_b_edge.py` (and `evidence_b_sweep.py`) from `proposal_v3.draw()` with modified state, and written to `docs/design/ui_review/critique_v3/edge/` as `NAME.png` (1x) and `NAME_4x.png`.
Tool warnings (cut / ellipsised / canvas) were captured on every render: **none on any render** (65 images in `edge/`, 36 knob-sheet renders and 12 MAP renders among them; `data/s6_edge_renders.txt`, `data/s6e_s6j_sweep.txt`). The lines below say what is drawn and what the measurement shows.
`draw()` reads these keys: `playing rec ext bpm preset preset_pend sel vpage row knobs hot lane toast channels[i]{algo midi_ch muted len snd0 p}`. It does not read `follow`, `playhead` or `page`; the playhead of the selected channel is `(p + len - 1) % len` and is not drawn when stopped.

### (a) all four channels muted (`a_all_muted_sel1`, `a_all_muted_sel3`, `a_all_muted_all_off`)
- Selected cell: SEL fill, struck number, solid `M` chip (INK, letter SEL). Other three: 2 px MID frame on BG, struck MID number, MID `M` chip (or `OFF` chip where MIDI channel is OFF). 0 activity squares.
- Lane of the selected muted channel: hollow bars (frame MID) for bars 3 px or wider; the playing column stays amber with a solid BG bar. Combs of 2 px bars (ratchet) are drawn solid MID, not hollow.
- The header still shows the amber play triangle and the amber playhead column moves. No element aggregates "all muted"; the facts are per cell only.
- All four muted AND MIDI OFF: 0 `M` chips, 4 `OFF` chips (OFF takes precedence in `draw_z1`); mute is then shown by the strike and (for unselected cells) the hollow frame only.
- Contrast: unchanged from section 4 (all pairs there). Nothing clipped.

### (b) 16 steps ON with accent, slide and ratchet on every step (`b_full_ph1`, `b_full_ph8`, `b_full_ph16`, `b_full_vel1_ph8`, `b_full_muted_ph8`, `b_ratchet_sweep_1to8`, `b_slide_chain_ph8`)
- Rep 4 on every step (vel 127): every bar is a 4-bar comb of 2 px bars (gaps 1,2,1) in cyan, a cyan triangle above, a TEXT tie bar across all 16; max bar height 44 px (top y 90), tie bar rows 83..88, so 1 px between the tie bar and the tallest bar, triangle rows 75..80.
- Playhead on step 1 / 8 / 16: the playing column is amber with BG bars, a BG triangle and a **BG tie bar**: the tie row y85 reads TEXT 0..105, BG 105..120, TEXT 120..239 at step 8 (the 15 px BG run = the column plus its gap): the slide chain is interrupted by a 15 px dark gap exactly at the playhead (step 1: BG 0..15; step 16: BG 225..240). In the playing column accent bars and normal bars are the same colour (BG).
- Muted channel with the same pattern (`b_full_muted_ph8`): the 2 px combs, triangles and tie bar are all MID; 2 px bars are filled, so the lane is a grey striped block (no hollow outlines because the width rule is `>= 3`).
- Ratchet counts 5..8 (hypothetical: the NOTE_EDIT K4 range in `knob_map.tsv` is 1x..4x): x5 is 5 bars with gaps 0,1,1,0; **x6, x7 and x8 are pixel-identical to x1** (a single 12 px bar) because the 2 px bars at the computed positions tile the 12 px span. The NOTES text says "ratchet is capped at 4 combs"; `draw_z2` has no cap.
- `vel` 1..3 gives `hh = 4`: the ON bar is 12x4 at y58..61, the same rectangle as a rest dash (12x4 at y58). Normal and muted: they differ in colour only (TEXT vs MID, 7.19 vs 3.75 against LANE; for a muted channel an ON bar is a hollow/filled MID frame vs a filled MID dash). **Under the playhead the two are pixel-identical** (both BG on amber) for vel 1..3; vel 4 differs (5 px).
- Accent triangle 11x6 (1.375 x 0.75 mm, 5.2 arcmin short side), comb bar 2 px = 0.25 mm = 1.7 arcmin wide.
- Nothing clipped; all marks inside the 70 px column.

### (c) no step ON while playing (`c_empty_playing`, `c_empty_stopped`)
- 16 identical 12x4 MID rest dashes at y130..134, the 4 beat pips, the amber playhead column with a BG dash. No text, no icon: an empty pattern and a pattern with all rests are the same picture; nothing on the screen states "empty". Header, cells and knobs draw as usual (the channel `snd0` squares only if set).

### (d) the playhead on a page that is not viewed (`d_view_p2_playhead_p4`, `d_view_p4_playhead_p1`, reference `d_view_p2_playhead_p2`)
- 64-step pattern, viewing page 2, playhead on step 56 (page 4): **0 amber pixels in the lane** (checked). The lane shows page 2's bars with no playhead column.
- The only indicator: page box 4 is a hollow double-outline box (14x14, MID) with a 4x4 amber marker (16 amber pixels at x225..229, y10..14: 0.5 mm, 3.4 arcmin) in the header; box 2 is solid TEXT. No lane-edge arrow, no text, and no FOLLOW state (`draw()` ignores `follow`).
- The viewed-page = playing-page case draws a BG 4x4 square inside the solid box.

### (e) every knob cell, six algorithms, minimum, maximum and longest-name value
Cell 60 px (58 drawable); AF_L ink limit used by the proposal 56. All 6 algorithms x 8 knobs (both rows): min, max and the value with the widest ink. Sheets: `edge/e_knobs_ALGO.png` (+ `_4x`): six 240x76 strips (ROW 1 min, max, longest; ROW 2 min, max, longest).

| algo | K | label (AF_M ink) | min -> text (font, ink) | max -> text (font, ink) | longest value -> text (font, ink) | verdict |
|---|---|---|---|---|---|---|
| ACID | K1 | DENS (38) | 0 -> `KEY` (AF_L, 52) | 16 -> `16` (AF_L, 32) | 0 -> `KEY` (AF_L, 52) | fits |
| ACID | K2 | SLIDE (38) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| ACID | K3 | JUMP (40) | 0 -> `0` (AF_L, 16) | 20 -> `20` (AF_L, 33) | 20 -> `20` (AF_L, 33) | fits |
| ACID | K4 | ACCNT (50) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| ACID | S1 | OFFSET (54) | 0 -> `0` (AF_L, 16) | 15 -> `15` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| ACID | S2 | PEDAL (45) | 0 -> `0` (AF_L, 16) | 12 -> `12` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| ACID | S3 | PUNCH (50) | 0 -> `0` (AF_L, 16) | 47 -> `47` (AF_L, 33) | 30 -> `30` (AF_L, 34) | fits |
| ACID | S4 | DIR (23) | 0 -> `UP` (AF_L, 35) | 2 -> `P-P` (AF_L, 44) | 2 -> `P-P` (AF_L, 44) | fits |
| EUCLID | K1 | HITS (32) | 0 -> `KEY` (AF_L, 52) | 16 -> `16` (AF_L, 32) | 0 -> `KEY` (AF_L, 52) | fits |
| EUCLID | K2 | OFFSET (54) | 0 -> `0` (AF_L, 16) | 15 -> `15` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| EUCLID | K3 | RATCH (47) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| EUCLID | K4 | ACCNT (50) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| EUCLID | S1 | LEN (26) | 2 -> `2` (AF_L, 15) | 16 -> `16` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| EUCLID | S2 | SPAN (37) | 0 -> `0` (AF_L, 16) | 24 -> `24` (AF_L, 33) | 20 -> `20` (AF_L, 33) | fits |
| EUCLID | S3 | PUNCH (50) | 0 -> `0` (AF_L, 16) | 47 -> `47` (AF_L, 33) | 30 -> `30` (AF_L, 34) | fits |
| EUCLID | S4 | CURVE (48) | 0 -> `AUT` (AF_L, 56) | 6 -> `INV` (AF_L, 44) | 0 -> `AUT` (AF_L, 56) | fits |
| SNAKE | K1 | DENS (38) | 0 -> `KEY` (AF_L, 52) | 16 -> `16` (AF_L, 32) | 0 -> `KEY` (AF_L, 52) | fits |
| SNAKE | K2 | LEN (26) | 1 -> `1` (AF_L, 10) | 16 -> `16` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| SNAKE | K3 | RANGE (48) | 0 -> `0` (AF_L, 16) | 48 -> `48` (AF_L, 34) | 30 -> `30` (AF_L, 34) | fits |
| SNAKE | K4 | PATH (35) | 0 -> `ROW1` (AF_M, 40) | 13 -> `SPI4` (AF_M, 30) | 1 -> `ROW2` (AF_M, 43) | drops to AF_M |
| SNAKE | S1 | OFFSET (54) | 0 -> `0` (AF_L, 16) | 15 -> `15` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| SNAKE | S2 | GLEN (37) | 1 -> `1` (AF_L, 10) | 16 -> `16` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| SNAKE | S3 | DYN (29) | 0 -> `0` (AF_L, 16) | 10 -> `10` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| SNAKE | S4 | DIR (23) | 0 -> `UP` (AF_L, 35) | 2 -> `P-P` (AF_L, 44) | 2 -> `P-P` (AF_L, 44) | fits |
| STAGE | K1 | DENS (38) | 0 -> `KEY` (AF_L, 52) | 8 -> `8` (AF_L, 16) | 0 -> `KEY` (AF_L, 52) | fits |
| STAGE | K2 | TOTAL (45) | 8 -> `8` (AF_L, 16) | 64 -> `64` (AF_L, 34) | 30 -> `30` (AF_L, 34) | fits |
| STAGE | K3 | DIR (23) | 0 -> `UP` (AF_L, 35) | 2 -> `P-P` (AF_L, 44) | 2 -> `P-P` (AF_L, 44) | fits |
| STAGE | K4 | RATCH (47) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| STAGE | S1 | OFFSET (54) | 0 -> `0` (AF_L, 16) | 63 -> `63` (AF_L, 34) | 30 -> `30` (AF_L, 34) | fits |
| STAGE | S2 | RDIV (33) | 1 -> `1` (AF_L, 10) | 4 -> `4` (AF_L, 16) | 3 -> `3` (AF_L, 16) | fits |
| STAGE | S3 | DYN (29) | 0 -> `0` (AF_L, 16) | 10 -> `10` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| STAGE | S4 | RANGE (48) | 0 -> `0` (AF_L, 16) | 36 -> `36` (AF_L, 34) | 30 -> `30` (AF_L, 34) | fits |
| ARP | K1 | DENS (38) | 0 -> `KEY` (AF_L, 52) | 16 -> `16` (AF_L, 32) | 0 -> `KEY` (AF_L, 52) | fits |
| ARP | K2 | SHAPE (46) | 0 -> `LINE` (AF_M, 30) | 5 -> `WAVE` (AF_M, 40) | 2 -> `THMB` (AF_M, 42) | drops to AF_M |
| ARP | K3 | OCTS (39) | 1 -> `1` (AF_L, 10) | 4 -> `4` (AF_L, 16) | 3 -> `3` (AF_L, 16) | fits |
| ARP | K4 | RATE (35) | 0 -> `1/4` (AF_L, 42) | 5 -> `4X` (AF_L, 35) | 0 -> `1/4` (AF_L, 42) | fits |
| ARP | S1 | OFFSET (54) | 0 -> `0` (AF_L, 16) | 15 -> `15` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| ARP | S2 | LEGATO (55) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| ARP | S3 | DYN (29) | 0 -> `0` (AF_L, 16) | 10 -> `10` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| ARP | S4 | DIR (23) | 0 -> `UP` (AF_L, 35) | 5 -> `CRD` (AF_L, 55) | 4 -> `ORD` (AF_L, 55) | fits |
| PATTERNS | K1 | DENS (38) | 0 -> `KEY` (AF_L, 52) | 16 -> `16` (AF_L, 32) | 0 -> `KEY` (AF_L, 52) | fits |
| PATTERNS | K2 | VARY (37) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| PATTERNS | K3 | SPAN (37) | 1 -> `1` (AF_L, 10) | 4 -> `4` (AF_L, 16) | 3 -> `3` (AF_L, 16) | fits |
| PATTERNS | K4 | ACCNT (50) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| PATTERNS | S1 | HOUSE,FUNK,TECH (49) | 1 -> `1` (AF_L, 10) | 120 -> `20` (AF_L, 33) | 80 -> `20` (AF_L, 33) | fits |
| PATTERNS | S2 | SLIDE (38) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |
| PATTERNS | S3 | DYN (29) | 0 -> `0` (AF_L, 16) | 14 -> `14` (AF_L, 32) | 10 -> `10` (AF_L, 32) | fits |
| PATTERNS | S4 | CTONE (49) | 0 -> `0` (AF_L, 16) | 8 -> `8` (AF_L, 16) | 0 -> `0` (AF_L, 16) | fits |

### enum font choice (one font per enum)
DIR3 all in AF_L AF_L inks [35, 37, 44] AF_M inks [20, 20, 38]
ARPDIR all in AF_L AF_L inks [35, 37, 49, 54, 55, 55] AF_M inks [20, 45, 40, 40, 36, 40]
CURVE all in AF_L AF_L inks [56, 42, 40, 48, 43, 55, 44] AF_M inks [40, 31, 21, 40, 38, 39, 24]
SHAPE AF_M (fallback) AF_L inks [39, 52, 54, 59, 50, 62] AF_M inks [30, 31, 42, 40, 36, 40]
RATE all in AF_L AF_L inks [42, 42, 33, 34, 34, 35] AF_M inks [22, 22, 18, 19, 18, 19]
PATH AF_M (fallback) AF_L inks [None, None, None, None, None, None, None, None, None, None, None, None, None, None] AF_M inks [40, 43, 35, 38, 35, 38, 38, 38, 29, 32, 27, 30, 30, 30]
enum names in AF_L: 22 in AF_M: 20 total 42
widest AF_L numeric value strings:
0 16 | 8 16 | 10 32 | 16 32 | 20 33 | 24 33 | 36 34 | 47 33 | 48 34 | 63 34 | 64 34 | 100 50 | 127 49 | -2 28 | +2 33 | 
largest hi of any knob: 120 ; any lo<0: False
PICK labels ink AF_M: {'ACID': 33, 'DUB': 30, 'FUNK': 39, 'HOUSE': 49, 'MEL': 29, 'TECH': 38} numbers 1..20 max AF_L ink 33
KEY (v=0 on DENS/HITS): AF_L ink 52 ; firmware KEYS 68


Facts:
- Nothing clips: widest AF_L value ink 56 (`AUT`), widest AF_M value 43 (`ROW2`), widest label 55 (`LEGATO`), all inside 58. No value is ellipsised (the tool reports none).
- Drops to the smaller font: SNAKE `PATH` (14 names) and ARP `SHAPE` (6 names) = 20 of 42 enum names (AF_M, 9.5 arcmin instead of 18 arcmin); 22 names in AF_L. Reproduces the author's 22 / 20.
- At the limit: `AUT` 56 of 56; `CRD` and `ORD` 55; labels `OFFSET` 54, `LEGATO` 55 of 58. With the 2 px "just-touched" frame the interior is 54 px: `LEGATO` (55) overlaps the frame by 1 px, `AUT` (56) and `CRD`/`ORD`/`ARC` (55) by 1-2 px; `OFFSET` (54) exactly fills it (renders `edge/e_hot_ARP_row2_LEGATO_ORD`, `edge/e_hot_EUCLID_row2_AUT`; list in `data/s6e3_hot_frame.txt`).
- Tightest gap between adjacent labels: 5 px (ARP row 2 `OFFSET` | `LEGATO`); adjacent values: 13 px.
- 3-digit values: **none is produced** by any knob (largest displayed number 64 `TOTAL`; PATTERNS `PICK` is shown as 1..20 inside a category, raw range 1..120). AF_L `100` ink 50 px, `127` 49 px, both under 56 if they occurred. Negative values: none (every `lo` is 0, 1, 2 or 8); `-2` would be 28 px and `+2` 33 px (AF_L has `-` and `+`).
- Meter at the low end: ranged knobs whose `lo` is not 0 (`1`, `2`, `8`) show an empty meter at `lo` (same look as 0); `KEY` (DENS/HITS 0) also an empty meter.
- PATTERNS ROW 2 K1: the label slot shows the category (`ACID DUB FUNK HOUSE MEL TECH`, widest `HOUSE` 49) instead of `PICK`; no cell on this screen is labelled `PICK`.
- `knob_map.tsv` cross-check (`data/s6e_knob_sweep_table.txt`): every numeric range and label equals the TSV except SNAKE `PATH`: TSV `shp 1..14 def 7`, proposal index 0..13 (path n drawn as the name at index n-1; no number is shown).
- AF_L has no glyph for `?`, `Z`, `:` or `>` (index 0, the fallback); AF_M has `?` and `>`.

### (f) clock and record states (`f_*`)
- `draw()` knows `ext` (bool) and `bpm`: it prints `120` or `EXT 126` (`EXT %d`). Defined by the proposal: INT and EXT with a tempo. **Not defined**: `EXT --`, `WAIT`, `EXT lost`. With the same inputs `ext=1, bpm=0` it prints `EXT 0` (`f_ext_unknown_stopped`); `WAIT` (SCREEN_SPEC 6: `clk_armed`) has no input; "EXT lost" returns to INT after 500 ms (SCREEN_SPEC 6), which this screen would draw as `120`, identical to INT.
- `EXT 126` playing: ink 30..87, 23 px before the preset text at x110; stopped on EXT: grey square plus `EXT 126` plus the cue line `HOLD HOME = MAP`.
- REC: a single `rec` flag draws a 16 px red disc (r 8) in place of the play triangle / stop square. REC armed while playing (`f_rec_armed_playing`) and REC armed while stopped (`f_rec_armed_stopped`) draw the same header glyph; the transport state is not shown while REC is set. With `rec` set the cue line is hidden even when stopped (`draw_z4` tests `playing or rec`). Red disc on BG 5.99:1.
- REC with EXT and a pending preset (`h_preset_p16_ext126_rec_header`): `EXT 126` and `P16?` do not collide (gap 23 px).

### (g) a channel that is selected AND muted AND MIDI OFF (`g_sel_muted_off_ref`, `g_sel_ch1_muted_off_snd`, `g_sel_muted_off_lane_full`)
- Selected fill SEL, number struck by a 3 px INK line, hollow `OFF` chip (INK on SEL), **no `M` chip**, no activity square even with `snd0 = 1`, lane bars hollow MID. In that cell the mute is shown only by the strike line and the lane drawing; the word/letter `M` is absent.
- The `OFF` ink overlaps its 2 px chip frame by 1 px at left and right (section 5.2). The strike line (x+6..x+26) ends 2 px before the chip frame starts (x+28).

### (h) preset pending `P16?` and loading `P16>` (`h_preset_p16_pending_header`, `h_preset_p16_pending_toast`)
- Header: `P16?` AF_M ink 110..144 (34 px). The `?` is an AF_M glyph.
- `P16>` (loading): **no loading state exists** in `draw()`; only `preset_pend` (adds `?`). For reference `P16>` would be 36 px advance in AF_M (the `>` glyph exists).
- Toast: `P16` in AF_L (ink 84..135) plus a hand-drawn 15x21 `?` at x142..157 (composite 84..157, centred), detail `SAVE = SAVE HERE` ink 56..183, countdown bar x40..200, y214..224. Nothing clipped.

### (i) toasts: longest strings (`i_toast_*`; widths from `data/s6i_toasts.txt`)

| toast | title | title ink x0..x1 (w) | detail | detail ink x0..x1 (w) | box limit | free margin L/R in box |
|---|---|---|---|---|---|---|
| algo ACID | ACID | 88..151 (63) | CH 1 ALGORITHM | 60..180 (120) | 0..240 | title 88/89, detail 60/60 |
| algo EUCLID | EUCLID | 72..167 (95) | CH 1 ALGORITHM | 60..180 (120) | 0..240 | title 72/73, detail 60/60 |
| algo SNAKE | SNAKE | 75..164 (89) | CH 1 ALGORITHM | 60..180 (120) | 0..240 | title 75/76, detail 60/60 |
| algo STAGE | STAGE | 77..162 (85) | CH 1 ALGORITHM | 60..180 (120) | 0..240 | title 77/78, detail 60/60 |
| algo ARP | ARP | 93..146 (53) | CH 1 ALGORITHM | 60..180 (120) | 0..240 | title 93/94, detail 60/60 |
| algo PATTERNS | PATTERNS | 52..187 (135) | CH 1 ALGORITHM | 60..180 (120) | 0..240 | title 52/53, detail 60/60 |
| dice ALL | ALL | 121..170 (49) | CH 1  NEW PITCHES | 78..213 (135) | 66..226 | title 55/56, detail 12/13 |
| dice NOTES | NOTES | 102..190 (88) | CH 1  NEW PITCHES | 78..213 (135) | 66..226 | title 36/36, detail 12/13 |
| dice ACCNT | ACCNT | 98..193 (95) | CH 1  NEW PITCHES | 78..213 (135) | 66..226 | title 32/33, detail 12/13 |
| dice SHAPE | SHAPE | 102..190 (88) | CH 1  NEW PITCHES | 78..213 (135) | 66..226 | title 36/36, detail 12/13 |
| dice ARTIC | ARTIC | 106..185 (79) | CH 1  NEW PITCHES | 78..213 (135) | 66..226 | title 40/41, detail 12/13 |
| dice HITS | HITS | 116..175 (59) | CH 1  NEW PITCHES | 78..213 (135) | 66..226 | title 50/51, detail 12/13 |
| undo ALL | UNDO | 107..184 (77) | DICE ALL | 115..176 (61) | 66..226 | title 41/42, detail 49/50 |
| undo NOTES | UNDO | 107..184 (77) | DICE NOTES | 104..187 (83) | 66..226 | title 41/42, detail 38/39 |
| undo ACCNT | UNDO | 107..184 (77) | DICE ACCNT | 103..188 (85) | 66..226 | title 41/42, detail 37/38 |
| undo SHAPE | UNDO | 107..184 (77) | DICE SHAPE | 105..187 (82) | 66..226 | title 41/42, detail 39/39 |
| undo ARTIC | UNDO | 107..184 (77) | DICE ARTIC | 107..184 (77) | 66..226 | title 41/42, detail 41/42 |
| undo HITS | UNDO | 107..184 (77) | DICE HITS | 112..179 (67) | 66..226 | title 41/42, detail 46/47 |

- Toast block is 240x88 at y146..234 (not 92: the zone Z3+Z4 is 92, the block leaves 2 px above and 6 px below); it replaces the knob values, labels, meters and the ROW bar.
- Algorithm toast: widest title `PATTERNS` (AF_L ink 135 of 240); header cell 1 draws `PATT`. Dice toast: widest title `ACCNT` 95 px in a 160 px box; detail `CH 1  NEW PITCHES` 135 px (12 / 13 px margins in the 160 box). UNDO naming the longest layer: `DICE ACCNT` 85 px. Nothing clips.
- Defined by the proposal: toast kinds `algo`, `preset`, `dice`, `undo`. The dice detail string exists only for `NOTES` (`CH 1  NEW PITCHES`); for the other layers (ALL, ACCNT, SHAPE, ARTIC, HITS) the same string was used in the renders (stand-in). The layer sub-labels of ALGORITHM_SET 4.4 (max 6 chars, e.g. ACID ARTIC `GT+SLD`) are not drawn by the proposal; `CH 1  GT+SLD` would be 95 px AF_M.
- **A MIDI channel change toast is not defined** by the proposal and no cell shows a MIDI channel number (only `OFF`). Reference widths, not renders: `MIDI CH 16` 75 px, `MIDI CH 16 TAKEN FROM CH 2` 205 px (AF_M).

### (j) the MAP with the longest channel/algorithm context (`edge/j_map_ALGO_row1`, `j_map_ALGO_row2`: 12 renders)
- No tool warnings. The only content that varies with channel/algorithm is `OF CH n` and the eight K1..K4 names (ROW 1 at x151, ROW 2 at x196; 45 px between).
- Tightest ROW 1 / ROW 2 gap: EUCLID K2 `OFFSET` (ink 151..194) | `SPAN` (196): **2 px**; next 5 px (ACID K4 `ACCNT` | `DIR`, EUCLID K4 `ACCNT` | `CURVE`, and the PATTERNS minimum). Rightmost: `OFFSET` and `LEGATO` end at x239 (1 px from the screen edge).
- The MAP shows the selected channel's algorithm knob names and `OF CH n` but not its mute, MIDI OFF or length state.

### (k) first-run / empty state
The proposal defines none (grep of `proposal_v3.py` and `NOTES.md` for first-run, empty, welcome, power-on: no definition). `knob_map.tsv` describes a first-run MIDI prompt for CHAN K2; this proposal does not draw it. The nearest renders are (c) (empty pattern) and the idle scene.

## 7. Colour count

RGB565 values per screen (`data/s7_colour_count.txt`): "drawn flat colours" = colours passed to rect/frame/pset/text/begin; the other distinct pixel values are the 4-bit anti-aliasing ramp of text glyphs (19 on each glance screen).

| scene | distinct pixel values | drawn flat colours | existing T_* values drawn | new literals drawn |
| --- | --- | --- | --- | --- |
| glance_idle | 26 | 7 | 6 | 1 (T_ACCSTEP cyan) |
| glance_playing | 27 | 8 | 6 | 2 (T_PLAYHEAD amber, T_ACCSTEP) |
| row2 | 26 | 7 | 6 | 1 (T_ACCSTEP) |
| mutes | 27 | 8 | 6 | 2 (T_PLAYHEAD, T_ACCSTEP) |
| stress | 27 | 8 | 6 | 2 (T_PLAYHEAD, T_RECRED red) |
| stress_row2 | 27 | 8 | 6 | 2 (T_PLAYHEAD, T_RECRED) |
| glance_playing_mono | 25 | 6 | 6 | 0 |
| stress_mono | 25 | 6 | 6 | 0 |

- Existing token values drawn (GREY): `T_BG`/`T_INK` (same value 0x0841), `T_SURF`/`T_TINT` (0x2104), `T_TEXT` (0xCE59), `T_MID` (0x9492), `T_RAISE`/`T_LANE` (0x39C7), `T_SEL` (0xBDD7): 6 distinct values. GREY has 11 distinct token values; `T_DIM`, `T_LINE`, `T_KEY`, `T_THEME`, `T_ACCENT`/`T_REC` are not drawn on the glance screens (a few of them appear only as coincidental anti-alias blend values).
- New: 3 literal RGB565 values (T_PLAYHEAD 0xFD80, T_ACCSTEP 0x565F, T_RECRED 0xFA49), at most 2 on any one screen. In `stress` the cyan is not drawn (the selected channel is muted, bars are MID). No new value is a function of the palette: they are literals in `proposal_v3.py`; 0xFA49 is the REC value in AMBER (`mock.py palette AMBER`), while GREY's `T_REC` is 0xFFDF.
- Pixels in `glance_playing` by colour: BG 17155, RAISE/LANE 13894, SURF 11474, TEXT 6260, MID 2395, SEL 2035, amber 1200, cyan 504, ramp values 4 % of the screen.
- Amber is used for four elements: playhead column, play triangle, activity squares, playing-page marker.

## Ten most important findings

1. The detector printed `[]` with exit 0 but scanned zero files (the folder holds only `.py`, `.png` and `.md`; it scans web markup/CSS/JS/TS only): this is a non-scan, not a clean result.
2. A fresh render of `proposal_v3.py` is byte-identical to the 39 committed PNGs (0 differing pixels in all 13 scenes) with no tool warning (no clipped or ellipsised text, no strip over 240x124).
3. Text counts reproduce: 20 strings playing, 21 idle, 8 roles while playing, 0 AF_S on any glance screen, 82 AF_S strings / 16 roles on the MAP; AF_M (9.5 arcmin) is the smaller glance size and carries the algorithm name, knob labels, BPM, preset and chips; AF_L (18.0 arcmin) carries channel numbers and knob values.
4. Every text pair is at least 4.5:1 (lowest on a glance screen 6.42:1); non-text pairs under 3:1 are the cyan accent bar against the normal bar (1.15:1; 1.00:1 in the mono fallback), the fill-versus-ground relations (lane 1.72, cell 1.72, meter track 1.40, knob cell 1.23) and the toast pips/countdown track (MID on SEL 1.62).
5. Nothing sits on the 5 px module (0 of 35 measured elements); cells are x1/y26 58x44 and 58x66, the lane is x0 with 15 px pitch and a 0/1 px side margin, so lane group edges (60/120/180) are 1 px off the cell edges (61/121/181).
6. The channel number is not centred in its cell (ink centre 14 px left of the cell centre), the number-to-algorithm gap is 1-2 px, and the `OFF` word is 27 px in a chip whose 2 px frame leaves 25 px inside, so the ink overlaps the frame by 1 px on each side.
7. In the lane, the playing column draws bars, accent triangle and slide tie in the same BG colour: the slide chain is interrupted by a 15 px dark gap at the playhead, accent and normal bars are indistinguishable there, and a velocity 1-3 ON step is pixel-identical to a rest.
8. A ratchet of 6, 7 or 8 renders pixel-identically to no ratchet (the 2 px bars tile the 12 px bar), although the NOTES say ratchet is capped at 4; NOTE_EDIT K4 offers only 1x..4x.
9. With the playhead on another page of a 64-step pattern the lane has 0 amber pixels; the only cue is a 4x4 px (0.5 mm) marker in a header page box, and the proposal has no FOLLOW state; `EXT --`, `WAIT`, `EXT lost`, `P16>` (loading), a MIDI-channel toast, per-layer dice details and a first-run state are not defined, and one `rec` flag draws the same red disc whether playing or stopped.
10. Knob cells never clip across the six algorithms (all ink at or under 56 px in 58), but 20 of 42 enum names drop to AF_M, `AUT`/`CRD`/`ORD` are 55-56 px and `LEGATO`/`OFFSET` 55/54 px so a 2 px "touched" frame overlaps or exactly fills them, no knob produces a 3-digit or negative value, and a selected AND muted AND MIDI OFF channel shows no `M` chip (OFF replaces it).

