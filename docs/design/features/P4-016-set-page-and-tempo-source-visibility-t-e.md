<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-016: SET page and tempo-source visibility (T-E): use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-016-set-page-and-tempo-source-visibility-t-e.md   Spec: SCREEN_SPEC_V2 (header 4.1, knob tiles 4.2, message slot 5), SEQUENCER_UX 2.4 and 2.8, SCREEN_SPEC 6.2 and 6.5 (strings only; zones superseded)   Scenario: S07 steps 1-6, S06 steps 3-4, 11-12, S12

**STATUS: BUILT. Gate A approved by the owner 2026-11-13; Gate B walk-through below (host; the real unit is HW-1).**

## The approved renders
Source: the SET branches of `docs/design/ui_review/proposals/v12/v12.py` (the reference implementation, scenes `set_*`). Renders (`.png`, `_4x.png`, `_grid.png`) in `docs/design/ui_review/proposals/v12/out/`; the firmware draws the same scenes and `tests/v12_compare.py` proves them equal pixel for pixel:

| PNG (prefix `v12_set_`) | What it shows |
| --- | --- |
| `int` | SET, own clock, playing: BPM 120 SWING 50% GATE 50% DIV 1/16, status `OUT MASTER`, `OCT 0`, footer `KEYS = STEPS` / `GLO = BACK` |
| `int_stopped` | the same stopped: header stop square, status `OUT ON PLAY` |
| `turn_swing` | K2 turned: the tile frames itself in the channel colour (as PERF), old level ticks |
| `octave` | OCT+ touched: slot `OCT+ OCTAVE +1`, status `OCT +1` |
| `ext_live` | host clock accepted: header `EXT` over `124`; K1 tile becomes `HOST 124` (dashed frame, no fill, MID digits); status `EXT USB  OUT FOLLOW`; footer `HOST SETS TEMPO` |
| `ext_k1_turned` | K1 turned while EXT: nothing changes, tile frames in white, slot `K1 HOST SETS TEMPO` |
| `ext_wait` | PLAY pressed, host not started: header `EXT` / `WAIT`, tile `HOST --`, footer `WAIT FOR HOST START` / `PLAY=CANCEL` |
| `ext_lost` | clock vanished: header `EXT` / `--` in red, tile `HOST --`, footer `CLOCK LOST: STOPPED` in red / `PLAY=START` |

Design points (all from existing locks unless marked NEW):
- Bands, fonts (AF_S / AF_M / AF_L), tokens, 12 icon buttons, keybed and the music row are exactly PERF's: steps and moves stay live on SET (page_states SET row).
- Message slot while open: `GLO  ALL CHANNELS` (the CHAN pattern `CHN  CHn SETUP`; it yields to hold hint, touched name and toast).
- Knob tiles per knob_map SET rows: K1 BPM 40..240 (AF_L, meter), K2 SWING 50..75 %, K3 GATE 10..100 %, K4 DIV 1/32 1/16T 1/16 1/8T 1/8 1/4 (a 6-name list: dots, no meter). One font per knob for its whole range (`value_font`); the percent knobs resolve to AF_M. Every tile is a global value: the meter uses the neutral amber of the selected channel as on PERF (NEW question 3).
- Status line (NEW, replaces PERF's ROW boxes and PAGE which have no meaning here): preset `Pnn` left; centre = clock out (`OUT MASTER` while playing as master, `OUT ON PLAY` stopped, `OUT FOLLOW` on any external source; SCREEN_SPEC 6.4) preceded, when external, by `EXT USB` or `EXT TRS` (the port the clock came from); right = `OCT -2..+2` (OCTAVE moved here by HARMONY, P2-012; OCT- / OCT+ are named in the slot while touched, as on PERF).
- Tempo source in the header on every page (SCREEN_SPEC_V2 4.1, already drawn by P4-028 in `ui_sq.c`): INT = `120`; EXT = `EXT` over the BPM; waiting = `EXT` / `WAIT`; lost = `EXT` / `--` in `T_REC`. This task adds the host test of the four strings and their widths, not new header code.
- EXT: K1 is a read-only mirror, not a control. The shape changes (label `HOST`, no fill, dashed frame), not only the colour. Turning it names why (toast). Keeping the user's own tempo while a host clock is followed, and returning it when the host stops (SEQUENCER_UX 2.8; TIMING_SPEC), is P4-031's: as built, `midi_clock.c:63` still overwrites `song.g[G_BPM]` under a host clock, and SET shows whatever `sq_bpm` holds.

## 1. Who and when
A producer wants to set the tempo, swing, gate length and step division for all channels, and to see at a glance whether the tempo is their own or a host's, without a menu.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Tap GLO (B_GLO, PERF / NOTE_EDIT / CHAN) | SET: slot `GLO  ALL CHANNELS`, tiles BPM SWING GATE DIV, status `P01  OUT MASTER  OCT 0`, footer `KEYS = STEPS` / `GLO = BACK` | none |
| 2 | Turn K1 (EN_K1), INT | BPM tile and the header tempo show the new number in the same frame | tempo changes at once (`beat_samples`) |
| 3 | Turn K2 / K3 / K4 | SWING %, GATE %, DIV name; the tile frames itself while turning | from the next step |
| 4 | Tap OCT+ / OCT- (B_OCTUP / B_OCTDN) | slot `OCT+ OCTAVE +1` while touched; status `OCT +1` (clamps at -2 / +2) | from the next step the notes sit an octave away |
| 5 | Host clock starts (S06): header becomes `EXT` / `124` | K1 tile becomes `HOST 124`, footer `HOST SETS TEMPO`, status `EXT USB  OUT FOLLOW` | the FM-1 follows the host |
| 6 | Turn K1 while EXT | nothing changes; slot `K1  HOST SETS TEMPO` | none |
| 7 | PLAY while EXT and the host has not started | header `EXT` / `WAIT`; tile `HOST --`; footer `WAIT FOR HOST START` / `PLAY=CANCEL` | none until the host's Start (P4-031 owns the transport) |
| 8 | The host clock stops | header `EXT` / `--` in red for the lost period; footer `CLOCK LOST: STOPPED`; then back to INT: the BPM tile shows `sq_bpm` (the own tempo once P4-031 stops the host overwriting it) | transport stopped, note-offs |
| 9 | Tap GLO or HOME | back to PERF | none |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| BPM, SWING, GATE, DIV, OCTAVE (page open + turn / tap) | Setup (2 gestures) |
| Read the tempo source (header, every page) | Performance (0 gestures: always on screen) |
| Read the host tempo on SET (EXT) | Setup (0 beyond opening the page) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| BPM / SWING / GATE / DIV | 3 | 3 | 3 | 3 | 12 | tile labels are the printed names; value and header BPM move in the same frame; the page names itself in the slot; footer says `GLO = BACK` | yes |
| OCTAVE | 2 | 3 | 3 | 3 | 11 | OCT- / OCT+ are printed on the panel and named in the slot while touched; the value is on the status line (small AF_M at the far right) | yes |
| Tempo source (header) | 3 | 3 | 3 | 3 | 12 | two-line `EXT` / number differs in shape from the single INT number; red `--` is a shape plus colour | yes |
| EXT on SET | 3 | 3 | 3 | 3 | 12 | the tile changes label and shape (no fill, dashed), the footer says who sets the tempo, a K1 turn is answered by a toast | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs | knob_map SET K1 and control_map EN_K1 / B_OCTDN / B_OCTUP SET rows (+ button_modes OCT rows) edited to the as-built words (HOST tile; status line `OCT n`, slot `OCTAVE n`); CONTROL_MAP.md rows edited to match; the SET SELECT row unchanged (it works as on CHAN: clamped, no add); `tools/check_control_map.py` is at its 78 earlier failures (the RIG rows and the ALGO hold clause, not this task) | yes |
| UI / screen | page_states.tsv SET row (screen, cue_strip `KEYS = STEPS   GLO = BACK`); SCREEN_SPEC_V2 4.7 (new), section 5 page title, section 7 state names, section 8; SCREEN_SPEC.md 6.5 note + the K1 quote; SEQUENCER_UX 2.4 (as-built picture, old picture kept for HARMONY), 2.8 pixel pointer; v12.py scenes `set_*`, goldens `out/v12_set_*.png`, MESSAGES rows; set_text.tsv (new table, generated into `ui_st_text`) | yes |
| UX / help | help_text EN_K1..K4 SET rows (K1 `inert while a host clock runs` still true) and B_OCT* SET rows (`OCT-: output octave down`) | yes, no change needed |
| Workflow | S07 steps 1-6, S06 note + steps 3, 4, 8, 10, 11, 12 edited; S12 / S09 / S10 regenerated by `tests/design/gen_sweeps.py` from the edited tables; `check_scenarios.py`, `check_help.py`, `check_sequencer_ux.py` pass | yes |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: GLO tap
- [x] Undoable in one press or self-reverting: values are turned back; EDIT = undo is a no-op for globals (not an undo-stack action; says nothing); OCT by tapping the other
- [x] Lands on a musical boundary, no hung or cut notes: BPM is read by the scheduler (beat length); SWING, GATE, DIV, OCTAVE are read at the next step (sq_core reads `g->s` per step); no note is cut
- [x] Screen shows the value within one frame; sound changes by the next step (BPM: at once)
- [x] One-handed
- [x] Does not change another channel (or says so on screen): the page is labelled `ALL CHANNELS`
- [x] Survives stop/start, clock-source change, preset load: the tile and header re-read the source each frame; the tile reads the live tempo and returns to the own tempo only if P4-031 stops the host clock overwriting it (not true of the device today: midi_clock.c:63); a preset load changes the global values and the tile shows them next frame
- [x] Cost recorded (RAM headroom >= 8 KB): `RAM .data+.bss 29352 B of 73728` (headroom 44376 B; was 29224 before the task: +128 B = the SET tile rows and view fields)

Hazards checked in the design (scan for bounds, stale state, surface sync):
1. Bounds: BPM 40..240 (clamped; fast = 5), SWING 50..75, GATE 10..100, DIV 0..5, OCTAVE -2..+2 (stored as a signed int8 in `GlobalSave`; every turn and tap clamps, no wrap), the `EXT` host tempo shown is the clamped 40..240 integer (never 3 digits + sign). No stack grows: SET keeps no list or history.
2. Stale state: the BPM tile mirror must read the source each frame (`clk_ext_bpm`, never a cached copy) and `bpm_own` when INT; `ui.force` / the view signature must include the clock state and the octave, or the tile and status line stay old; a pending K1 touch frame must clear on leaving SET; EXT -> INT must show the live `sq_bpm` in the tile in the same frame (that it is the own tempo is P4-031's); LOST has no last-host-tempo state (SCREEN_SPEC 6.2).
3. Surface sync: the table rows and scenarios named in section 5 are the ones that still describe the older screen (the `TEMPO INT n` status line and the OCT tag column); the build commit edits them and re-runs `check_scenarios.py` / `gen_sweeps.py`.
4. Width: every new string is measured with the mock's `text_w`: `GLO ALL CHANNELS`, `K1 HOST SETS TEMPO`, `OCT+ OCTAVE +1`, `OCT- OCTAVE -2` fit the 136 px slot (`check_messages()` returns no failure); the status line asserts the clock-out blocks end before the OCT readout (`EXT USB  OUT FOLLOW` is the widest); the footers are asserted to fit 240 px (`WAIT FOR HOST START` + `PLAY=CANCEL` in AF_S).

Design locked: James Keaney 2026-11-13 (owner; answered "Good enough, build it" to the SET renders, all five defaults below; relayed to the executor by the coordinator, recorded here).

### Owner answers (2026-11-13: all five defaults)
1. No clock dot: the tempo source is text in the header (SCREEN_SPEC_V2 4.1). 2. `OUT` and `OCT` replace ROW and PAGE on the SET status line. 3. Tile meters in the selected channel's colour.
4. EXT SYNC (first pulses) is shown like WAIT (`HOST --` tile; the header shows `EXT` over the tempo). 5. WAIT and lost footers in AF_S.

## 7. Walk-through (after the build; Gate B)
Method: host tests (`tests/ui_sq_set_test.c` drives the handlers as ui_sq_glue.c does; `tests/ui_test.c` drives the real main loop incl. `sq_frame`; `tests/ui_sq_v12_test.c` + `tests/v12_compare.py` draw the eight `set_*` scenes with the firmware and compare them with the mock pixel for pixel). Frames in build/ui_sq/set_*.png and build/ui_sq/v12/set_*.png. Real unit: pending (HW-1). Executor-prepared; the reviewer confirms.
| Step | Screen showed | MIDI / sound |
| --- | --- | --- |
| 1 GLO | SET: slot `GLO ALL CHANNELS`, tiles BPM 120 / SWING 50% / GATE 50% / DIV 1/16, status `P01 OUT MASTER OCT 0` (stopped: `OUT ON PLAY`), footer `KEYS = STEPS` / `GLO = BACK` | none |
| 2 K1 +8 | tile and header 128; `song.g[G_BPM]` 128 (one tempo value, via `uisq_bpm_req` and `sq_frame`); fast turn steps 5; clamps 40 / 240 | tempo at once |
| 3 K2 / K3 / K4 | SWING 56%, GATE 40%, DIV 1/8T (dots); clamps 50..75, 10..100, 1/32..1/4, no wrap | from the next step (sq_global_set one flip) |
| 4 OCT+ / OCT- | slot `OCT+ OCTAVE +1`, status `OCT +1`; clamps at -2 / +2 and still names the value | next step |
| 5 EXT clock | header `EXT` over `124`; K1 tile `HOST 124`; status `EXT USB OUT FOLLOW`; footer `HOST SETS TEMPO` | follows the host (P4-031) |
| 6 K1 under EXT / WAIT / lost | nothing changes (tempo and request unchanged); slot `K1 HOST SETS TEMPO`; white frame | none |
| 7 WAIT / lost | header `EXT` over `WAIT` / `--` (red); tile `HOST --`; footers `WAIT FOR HOST START  PLAY=CANCEL` / `CLOCK LOST: STOPPED  PLAY=START` (AF_S) | none |
| 8 back to INT | the tile reads the live tempo the same frame (host test: a value held in `sq_bpm` shows at once; keeping the own tempo across a host clock is P4-031's) | - |
| 9 keys, CHN, held step, GLO / HOME | steps toggle, M1 mutes, CHN opens CHAN, a held key opens NOTE_EDIT, SELECT selects without adding, a dice works and the page stays; GLO and HOME return to PERF; GLO inert in REC | as PERF |
Differences from the prediction: none. Debt: the ALGO encoder still works only on CHAN (P4-032 widens it, as the SET knob_map row says); hold GLO = HARMONY and GLO acting on release are P5-006; the external clock source itself is P4-031 (`ui_sq_clock_set(src, port)` is its hook; until then the source is always INT on the device); `midi_clock.c` still writes the host tempo into `song.g[G_BPM]`, so under EXT the HOST tile shows the host value but the own tempo is not preserved (P4-031 / P2-010's "never overwritten" rule).

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| BPM / SWING / GATE / DIV | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host; real unit HW-1) |
| OCTAVE | 2 | 3 | 3 | 3 | 11 | 11 | 0 | yes (host) |
| Tempo source (header) | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host) |
| EXT on SET | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host) |

## 9. Sync re-check and outcome
- `sh tools/check.sh`: CHECK PASS; `tests/ui_sq_set_test.c`, `tests/ui_test.c` (4 new rows), `tests/v12_compare.py` (52 scenes, 0 failed), `tests/design/check_scenarios.py`, `check_help.py`, `tools/check_sequencer_ux.py`: pass.
- Live checklist re-answered from the host runs: yes for all eight lines; "undoable" is by turning back (globals are not on the EDIT undo stack).
- Outcome: Pass with debt (the items under section 7). Gate B (reviewer) to confirm; the real unit is HW-1 (P4-021).
- Approved: James Keaney 2026-11-13 (answered "Good enough, build it" to the SET renders)
