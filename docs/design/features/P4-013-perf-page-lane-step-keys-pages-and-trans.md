<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-013: PERF page: lane, step keys, pages and transport: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-013-perf-page-lane-step-keys-pages-and-transport.md   Spec: docs/design/SCREEN_SPEC_V2.md (4-6), SEQUENCER_UX 1.5, 1.7, 2.1   Scenario: S01 steps 5-9, S02, S03 (step entry), S04 (channels)

Gate A is cited from the locked design: the roles of every control come from key_map.tsv, button_modes.tsv and knob_map.tsv (PERF rows); the screen is SCREEN_SPEC_V2 (ADR-0006). This task wires the controls; it changes no table row.

## 1. Who and when
A producer with a TB-3 on MIDI channel 2, the bass line playing, wants to enter steps, morph, dice, mute, add and remove channels and page the lane without leaving the main screen.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Tap white key (KEY_Wnn, PERF) | the step cell flips ON/OFF in the next frame (music row and keybed) | the step plays or not from the next pass |
| 2 | Tap the same key again | the step returns to its generated state (AUTO) | as generated |
| 3 | Hold a white key 0.4 s | NOTE_EDIT on that step (stand-in until P4-014) | none |
| 4 | Turn K1..K4 (EN_K1..K4) | the tile value, hot, same frame | the pattern morphs at the next step |
| 5 | Press SCL / SEQ / ENV / ARP / LFO / FX | toast `NOTES` / `HITS` / `ACCNT` / `ARTIC` / `SHAPE` / `ALL`  `EDIT=UNDO` on the channel chip; changed steps hatched 2 s; a dim button (layer absent) does nothing | new layer from the next step |
| 6 | Press EDIT | toast `UNDO  EDIT=REDO`, again `REDO` | previous state back |
| 7 | Tap M1..M4 | mute toggles | channel silent / sounding |
| 8 | Hold M1..M4, release before 1.5 s | hint `PREVIEW CHn` with a bar, then the preview toggles | preview synth on/off |
| 9 | Hold M1..M4 1.5 s | toast `[n] GONE  EDIT=UNDO`, channel leaves the screen | channel silent; EDIT brings it back |
| 10 | Turn SELECT past the last channel | toast `[n] NEW  ALGO=PICK`, setup cue | none |
| 11 | PG- / PG+ (black keys C#5 / D#5) | lane page changes, FOLLOW off; PG+ on the last page: `PAGE n?  PG+=ADD`, again adds it | the added page copies the one before |
| 12 | OCT+ / OCT- | knob band ROW 2 / ROW 1 (`LAST ROW  OCT-=1` at the end) | none |

## 3. Functions and class
| Function | Class |
| --- | --- |
| Step ON/OFF tap, hold = EDIT, morph, dice, undo, mute, PG-/PG+, row | Performance (one gesture) |
| Add / remove channel, preview toggle | Setup (one gesture each; remove is a 1.5 s hold, undoable) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Step tap / hold | 3 | 3 | 3 | 3 | 12 | one key, the step is the key under the finger; cell flips in the frame | yes |
| Morph / dice / undo | 3 | 3 | 3 | 3 | 12 | knob and button labels are on screen; toast names the layer and the way back | yes |
| Mute / preview / remove | 3 | 2 | 3 | 2 | 10 | tap, then a hold ladder; the hint names each stage; remove is undoable | yes (setup) |
| Add channel | 3 | 3 | 3 | 2 | 11 | SELECT one past the last, toast says what next | yes |
Pass: performance >= 11, setup >= 9, no sub-score < 2.

## 5. Sync check
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (tables) | key_map PERF W01-W16 tap/hold, B01-B04, B09, B10; button_modes PERF FX SCL ENV LFO EDIT ARP SEQ OCTDN OCTUP; knob_map PERF SELECT, K1-K4 | yes, read through ui_tables.h; no row changed |
| UI / screen | SCREEN_SPEC_V2 4-6 (message slot, lifecycle) | yes, v12 renders unchanged (tests/v12_compare.py) |
| UX / help | untouched | n/a |
| Workflow | S01 steps 5-9, S04 | yes except the gaps in 6 |

## 6. Live checklist
- [x] Reachable from the main screen, no menu: yes.
- [x] Undoable in one press or self-reverting: dice, step edits, page add via EDIT (sq_undo); a removed channel via EDIT; the morph by turning back; mute by tap.
- [x] Lands on a musical boundary: edits are read at play time (sq_eval); the dice rebuild stages for the next step; no note is cut by the UI.
- [x] Screen shows the value within one frame: test `tap redraws in the next frame`, K1 tile hot in the same frame.
- [ ] One-handed: tap and morph yes; hold ladders need one finger held while the other hand is free: HW-1 to confirm.
- [x] Does not change another channel: every call takes the selected channel; the M keys name their own channel on screen.
- [x] Survives stop/start, clock change, preset load: state is in sq_* (reads after SQ_FRAME_SYNC); the lane follows a load.
- [x] Cost: build line `RAM .data+.bss 29192 B of 73728` (headroom 44536 B).
- HW-1 (FB-015) item: on AF_PAIRING algorithms (ACID) a tap or an ON edit made during step p-1 is heard from the next pass, not at step p (the pairing snapshot is taken one step ahead). Not a bug; confirm it feels acceptable.
- Not wired in this task (gaps, see the task log): a tap past LENGTH shows no `PAST THE END` toast and `64 STEPS = MAX` is silent (no message row in MESSAGES); a step turned ON while stopped does not sound once; the hard-tap ACCENT; SOL / FIL / ROL / RST / CHN keys; the ALGO and PRESETS encoders; the preview has no toast.

Design locked: cited from the locked tables (P4-030) and SCREEN_SPEC_V2 (ADR-0006); Gate A by the executor, the reviewer confirms.

## 7. Walk-through (after the build)
Reviewer 2026-11-13, host only (tests/ui_sq_perf_test.c passes inside CHECK PASS; no device).
Steps 1-2, 4-8, 10-12 behave as predicted (test cases for tap, K1 hot tile, dice toasts, EDIT undo/redo, M tap/preview/1.4 s/1.5 s, SELECT add, PG+/PG-). Differences:
- Step 3: NOTE_EDIT is a stand-in that closes when the key is released (P4-014).
- Step 9: removal leaves the channel's preview bit ON, so a removed channel can still be heard on the built-in synth (finding F1).
- Step 9 twice in a row: the first channel's EDIT=UNDO is lost (one-slot undo, finding F2).
- Not wired: CHN key, ALGO encoder (P4-032), PAST THE END, step-on-while-stopped, preview toast.

## 8. Actual ease score
| Function | V | I | O | L | /12 | Pass? |
| --- | --- | --- | --- | --- | --- | --- |
| Step tap / hold | 3 | 3 | 3 | 3 | 12 | yes (stopped: no sound, so O=2 there; still 11) |
| Morph / dice / undo | 3 | 3 | 3 | 3 | 12 | yes |
| Mute / preview / remove | 3 | 2 | 2 | 2 | 9 | yes (setup), O drops: no preview toast, removed channel still audible |
| Add channel | 3 | 3 | 3 | 2 | 11 | yes |

## 9. Sync re-check and outcome
Buttons/tables agree for the wired controls (ui_hold_rows 400 ms on W and B01-B04, ui_remove_rows 1500 ms); renders unchanged. check_scenarios / help untouched. Live checklist: "does not change another channel" is not true for removal (finding F1 keeps a channel audible); "no hung notes" holds (note-offs follow the slot's own flags, sq_core.c sq_emit). One-handed hold ladders await HW-1.
Outcome: Fail (F1), see the task Log. Debt filed as P4-032.
