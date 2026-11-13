<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-017: REC page: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-017-rec-page.md   Spec: SCREEN_SPEC_V2 (4.2 REC band, 4.4 cursor frame, 4.6 keybed, 4.8 REC page), SEQUENCER_UX 2.5, 5 (U54, U67), 6.1 (FS4 fallback)   Scenario: S07 steps 9-10, 19-33, S14

**STATUS: BUILT on the host (the real unit is HW-1). The keys write steps; whether they also SOUND needs the ISR end of the note queue: FB-019 (this task's scope excludes ISR code).**

## The renders
The screen is the v12 REC scenes (`docs/design/ui_review/proposals/v12/out/v12_rec_armed*.png`, `v12_rec_playing*.png`, locked with the v12 screen design). The firmware draws them and `tests/v12_compare.py` proves them equal pixel for pixel (52 scenes, 0 failed). The page adds no new drawn element: the band, the cursor frame, the red disc and the plain keybed were already in the v12 drawing code; this task replaced their literal strings with table reads (labels from `knob_map` REC rows, OFF / ON and the octave signs from `note_edit_text`, the footer from the `page_states` REC cue) and wired the behaviour. The host test also renders `build/ui_sq/rec_open|rec_written|rec_stopped|rec_playing|rec_glo.png`.

Differences between the host render and the v12 scene: none (0 of 52 scenes differ; the T_CHF rounding and glyph anti-aliasing notes of `v12_compare.py` are unchanged). The only new on-screen text is the slot line `GLO REC = DONE` (a new `MESSAGES` row in v12.py; proven to fit by `check_messages()` and `ui_sq_rec_test`).

## 1. Who and when
A producer has a channel selected and wants to play a line on the keys and have it become the steps of the pattern, stopped (step by step) or against the running loop, without a menu.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Tap REC (B_REC, PERF / NOTE_EDIT / CHAN / SET) | REC: red disc in the header, band `CURSOR 1` `ACC OFF` `SLIDE OFF` `OCT 0` in white, white frame round step 1 (round the edited step from NOTE_EDIT), keys plain, footer `PLAY KEYS TO WRITE` / `REC = DONE` | none |
| 2 | Press a key (any of the 27), stopped | the step under the cursor fills and the cursor moves on (`CURSOR 2`) | the key's note on the selected channel's MIDI channel (the ISR end is FB-019); the step plays it from now on |
| 3 | Turn K2 / K3 (ACC, SLIDE), then press a key | `ACC ON` / `SLIDE ON`; the written step carries an accent cap and a slide bridge | velocity 120 (ACC) instead of 90 |
| 4 | Turn K4 or tap OCT- / OCT+ | `OCT -2..+2` in the tile (dots), clamped | the keys sound and write an octave away |
| 5 | Turn K1 (CURSOR), stopped | the frame moves one step per detent, wraps over LENGTH, the view follows across pages | the step under the cursor sounds briefly when it hits |
| 6 | Tap PLAY, press keys | the cursor follows the nearest step | each key writes the nearest step (the one sounding, or the next in its second half); heard from the next pass |
| 7 | Tap EDIT | toast `UNDO`; the last written note is gone, the cursor is back on it; again = `REDO` | the step plays as before |
| 8 | Press a dice button | toast as on PERF; REC stays open | the generated layer changes from the next step |
| 9 | Turn SELECT / ALGO | the other channel, cursor on step 1 / the new algorithm name; written steps kept | as PERF |
| 10 | Tap REC (DONE) or HOME | back to PERF | held notes end |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Write steps from the keys (REC tap + a key per step) | Setup (2 gestures: open, then a key; each further step is 1) |
| ACC, SLIDE, OCT, CURSOR | Setup (a knob turn while in REC) |
| Dice, EDIT undo while recording | Performance (1) |
| Leave REC | Setup (1) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Write steps from the keys | 3 | 3 | 3 | 3 | 12 | the red disc and the footer say what the keys do; the white frame shows where the next note lands; the key is its own label | yes |
| ACC / SLIDE / OCT / CURSOR | 3 | 3 | 3 | 2 | 11 | printed tile labels with values and dots; the learnability gap is that ACC means velocity 120 (no velocity read-out until force sensing) | yes |
| Dice / EDIT in REC | 3 | 3 | 3 | 3 | 12 | same buttons and toasts as PERF; EDIT also puts the cursor back on the undone note | yes |
| Leave REC | 3 | 3 | 3 | 3 | 12 | the footer says `REC = DONE`; HOME is the one escape; the button that opened it closes it | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs | knob_map REC K1 (stopped / playing), button_modes + control_map REC B_EDIT hold, B_GLO tap, B_OCTDN / B_OCTUP (press, clamp), EN_PRESET rows edited to the as-built words; CONTROL_MAP.md rows edited to match; key_map REC rows unchanged (the velocity fallback was already stated); `tools/check_control_map.py` stays at its 78 earlier failures | yes |
| UI / screen | page_states.tsv REC keys cell; SCREEN_SPEC_V2 4.7 exit line and new 4.8; SEQUENCER_UX 2.5 as-built note (the mock-up kept as history); CONTROL_MAP.md REC state row; PERFORMANCE_DESIGN.md (the strip text it quoted); widths.tsv (two toasts that no longer exist removed); v12.py MESSAGES (`GLO REC = DONE`) | yes |
| UX / help | help_text REC rows (`B_GLO` / `EN_PRESET`: `tap REC to finish`; `EN_K1`: `stopped: the step sounds`; `B_EDIT`: `undo the last note`) still true; `check_help.py` OK | yes, no change needed |
| Workflow | S07 steps 9, 10, 19-28, 30, 32 rewritten to the built screen; S14 regenerated by `tests/design/gen_sweeps.py`; `check_scenarios.py` passes | yes |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: REC is printed on the panel
- [x] Undoable in one press or self-reverting: EDIT takes the last written note back (one record per key press); again = redo
- [ ] Lands on a musical boundary, no hung or cut notes: writes are step edits, read at the step's next play, no note is cut; every queued note-on has its off (owed until queued, also after REC is left: host-tested incl. a full ring), BUT the ISR does not play the queue yet (FB-019), so no key sounds on a device until that lands
- [x] Screen shows the value within one frame; sound changes by the next step: the tiles, cursor frame and step bar redraw in the frame of the key; the written step plays at its next pass
- [x] One-handed: keys and the knob turns are single-handed; two-handed play is not needed
- [x] Does not change another channel (or says so on screen): writes go to the selected channel only; SELECT while a key is held ends that note on the channel it began on
- [x] Survives stop/start, clock-source change, preset load: PLAY while REC stays open and the cursor follows the nearest step; a clock-source change does not touch REC; a preset load (P7) clears undo records and the cursor re-clamps to LENGTH every frame
- [x] Cost recorded (RAM headroom >= 8 KB): `RAM .data+.bss 29576 B of 73728` (headroom 44152 B; was 29352 before the task: +224 B = the REC tile rows, the 16-slot note queue and the key bookkeeping)

Hazards checked in the design and the tests (bounds, stale state, surface sync):
1. Bounds: key index 0..26 (an index past it is ignored); the written pitch is folded into 0..96 by octaves and the played note clamps to 0..127 (REC OCT -2..+2 with OCTAVE -2..+2 reaches both ends: tested at F3 / G5); velocity 90 / 120; the cursor 1..16 on a page, the page 0..3, always inside LENGTH (LENGTH 1, 12, 40 and a stale cursor of page 4 / step 99 tested); `sq_rt.p` beyond LENGTH is taken modulo LENGTH; a step LENGTH hides is refused by `sq_note_edit_begin`; K1 turns clamp to +-127 detents and wrap with a modulo that cannot go negative; the note queue is a fixed 16-slot ring (15 usable): a full ring drops a note-on and owes no off, never loses an off.
2. Stale state: every key remembers the note it started, so an octave change, a SELECT or leaving REC while it is held ends the right note; owed note-offs are retried every frame on every page (not only on REC); the REC cursor returns to step 1 when the selection changes by any road (SELECT, an EDIT that brings back a removed channel); keys still down when REC is left have their release marked fired so it does not toggle a step on PERF (tested); keys down when REC opens do nothing on release; the viewed page and the cursor follow the nearest step only while playing; the NOTE_EDIT session is ended after every key write so the next write takes its own undo record; a write resets the removed-channel undo stack so EDIT undoes the note, not a channel removal.
3. Surface sync: the tables, scenarios and specs named in section 5; no per-algorithm branch in the screen code (the page reads `sq_*` and the descriptor only: `ui_sq_v12_test` greps the screen code for algorithm names and passes).
4. Reading ISR state: the nearest step reads `sq_rt[sel].p`, `.grid` and `sq_now` as plain values from the main loop (p is read twice around the others). A torn read can only pick the neighbouring step at a step boundary.

Design locked: v12 screen design lock (SCREEN_SPEC_V2, ADR-0006, owner 2026-11-12) for the screen; SEQUENCER_UX 2.5 / U54 / U67 (P1-017, approved) for the behaviour.

## 7. Walk-through (after the build; Gate B)
Method: host tests (`tests/ui_sq_rec_test.c` drives the handlers as ui_sq_glue.c does and reads the note queue the ISR would read; `tests/ui_sq_v12_test.c` + `tests/v12_compare.py` draw the `rec_armed` / `rec_playing` scenes with the firmware and compare them with the mock pixel for pixel). Frames in build/ui_sq/rec_*.png and build/ui_sq/v12/rec_*.png. Real unit: pending (HW-1). Executor-prepared; the reviewer confirms.
| Step | Screen showed | MIDI / sound |
| --- | --- | --- |
| 1 REC tap | REC page as above; from NOTE_EDIT the cursor was on the edited step (9); from CHAN and SET the same page; CHN is a note in REC | none |
| 2 each of the 27 keys, stopped | step (k mod 16 + 1) ON with PITCH = key note - 36, VEL 90; the cursor moved on and wrapped after step 16 (LENGTH 16) | queue: note-on 53 + k, then its note-off (27 pairs) |
| 3 K2 / K3 | `ON` / `OFF` (dots), VEL 120 and SLIDE ON written; turning back writes 90 and SLIDE AUTO | note-on at 120 |
| 4 K4 / OCT- / OCT+ | `-2..+2`, clamped; the note and the stored pitch an octave away per step; global KEY and OCTAVE cancel (C4 with KEY 5, OCTAVE +1 plays 72, stores 19) | as written |
| 5 K1 | cursor +4 -> 5, -5 -> 16 (wrap), LENGTH 12 wraps at 12, LENGTH 40 crosses to page 2 (the view moves) | the step under the cursor sounded 120 ms when it hits |
| 6 playing | cursor = nearest step (10 ticks before step 6: step 6; 40 ticks before: the sounding step; exactly half: the next; at the loop point the previous step is 16); K1 only frames the tile | key notes as stopped |
| 7 EDIT | `UNDO`: the last note gone, cursor back on it; again `REDO`, cursor past it; after a dice, EDIT undoes the dice and leaves the cursor | none |
| 8 GLO | inert; slot `GLO REC = DONE` | none |
| 9 SELECT / ALGO | other channel, cursor on step 1, no channel added; algorithm changes, written steps kept | as PERF |
| 10 REC / HOME | PERF; two keys held across HOME: both notes end, their release toggles no step | note-offs |
Differences from the prediction: none. Debt: **FB-019** (the ISR does not play the queue: keys are silent on a device); SAVE in REC and PRESETS are the preset layer's (P7), RESET (hold EDIT) is P4-018; the K1 meter is drawn over 1..16 (a channel with LENGTH under 16 wraps earlier); force sensing (P6-006) replaces the fixed 90 / 120.

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Write steps from the keys | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host; real unit HW-1, and only with FB-019) |
| ACC / SLIDE / OCT / CURSOR | 3 | 3 | 3 | 2 | 11 | 11 | 0 | yes (host) |
| Dice / EDIT in REC | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host) |
| Leave REC | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host) |

## 9. Sync re-check and outcome
- `sh tools/check.sh`: CHECK PASS; `tests/ui_sq_rec_test.c` (0 failures), `tests/ui_sq_v12_test.c`, `tests/v12_compare.py` (52 scenes, 0 failed), `tests/design/check_scenarios.py`, `check_help.py`, `tools/check_sequencer_ux.py`: pass.
- Live checklist re-answered from the host runs: yes for seven of eight lines; the sound line is open until FB-019.
- Outcome: Pass with debt (FB-019). Gate B (reviewer) to confirm; the real unit is HW-1 (P4-021).
- Approved: pending (reviewer; owner at HW-1)
