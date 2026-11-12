---
id: P4-013
title: PERF page: lane, step keys, pages and transport
phase: P4
type: task
status: review
priority: 2
executor: haiku
depends_on: [P4-012]
gate: none
---
## Goal
The home page the device boots into: the selected channel's lane, step keys tap ON/OFF, hold 0.4 s opens NOTE_EDIT, page keys, PLAY/STOP, channel cells.

## Read first
- docs/design/SEQUENCER_UX.md §1.5, §1.7, §2.1, §3, §4.3, §5 (traces U-rows for PERF), §7
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c PERF page: lane with playhead and FOLLOW, step key handling (U14 tap, hold -> NOTE_EDIT named `hold=EDIT`), PG-/PG+, PLAY/STOP via the transport request flag, channel cells Z1, preview of the selected channel
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/
- Live checklist item for the HW-1 sheet (FB-015): on AF_PAIRING algorithms (ACID) a tap or an ON edit made during step p-1 is heard from the next pass, not at step p: the pairing snapshot is taken one step ahead, the same one-step latency the old slide pairing had. Not a bug; confirm it feels acceptable.
- (P4-024 review) the 0.4 s hold that opens NOTE_EDIT must call `sq_note_edit_begin(ch,p)` on the hold, so the session's writes share ONE undo record; without it every `sq_step_set` / `sq_step_clear` is a record of its own. Call `SQ_FRAME_SYNC()` (sq_chan.c) once per frame before input (P4-011/P4-012 wire it). `sq_global_set` returns 0 both for "no change" and for "a load is pending": the SET/SCALE pages need no toast for it (the pending load replaces every global).

## Acceptance criteria
- [x] Scenario S01 steps 1-6 reproduce in the host test (screen strings and API calls)
- [x] A tap on a step key toggles it via `sq_step_set/clear` and the lane redraws within one frame
- [x] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-013-perf-page-lane-step-keys-pages-and-trans.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).
- 2026-11-13: from FB-017 item 2, this task must also wire: SELECT adds a channel (PERF only), M-key hold remove/preview via ui_remove_rows, white keys as steps, K1-K4 via sq_morph_turn, the dice buttons, and decide what the keys do (song.grid 2 keeps them silent now).

### 2026-11-13 executor (Sonnet 5.5)
Built: ui_sq.c PERF controls (ui_sq_keys, ui_sq_button, ui_sq_knob, ui_sq_select_turn, hold timers in ui_sq_input), wired by ui_sq_glue.c; new tests/ui_sq_perf_test.c (in run_tests.sh); use case docs/design/features/P4-013-perf-page-lane-step-keys-pages-and-trans.md (Gate A filled; Gate B for the reviewer).
- `sh tools/check.sh` -> `CHECK PASS` (v12 renders still equal the mock; ui_sq_perf_test: 0 failures).
- `./build.sh | grep RAM` -> `image 182192 B; RAM .data+.bss 29192 B of 73728; pool 78520 B of 368640` (headroom 44536 B, above 8192).
- Wired: white keys (tap = sq_step_tap on release, hold 0.4 s = sq_note_edit_begin + NOTE_EDIT), M1-M4 (tap mute, hold + release < 1.5 s toggles the preview bit pad[0]&0x80, 1.5 s removes: out of use + muted, EDIT restores), SELECT (move / add past the last), K1-K4 via sq_morph_turn on the live row, dice FX/SCL/SEQ/ENV/ARP/LFO (absent layer inert), EDIT undo/redo, OCT-/OCT+ rows, PG-/PG+ (FOLLOW off, PG+ twice adds a page). PLAY was already wired. Strings come from ui_msgs (digits and the algorithm short name are put into table rows).
- Not done, for follow-up (not in tables/MESSAGES or out of scope): no `PAST THE END` / `64 STEPS = MAX` message row (a tap past LENGTH is silent); a step turned ON while stopped does not sound (sq_live_* are ISR-side); hard-tap ACCENT; CHN/RIG, SOL/FIL/ROL, RST keys; ALGO and PRESETS encoders (drained); preview toggle has no toast; ADD_PAGE overlay is the toast only; NOTE_EDIT is a stand-in that lasts while the opening key is held (P4-014 replaces uisq_pg_note_edit); channel removal is UI state (uisq_used + mute), not in presets.
- Reviewer: look at uisq_key_up / uisq_keys_tick (hold ladder) and the removal semantics.
