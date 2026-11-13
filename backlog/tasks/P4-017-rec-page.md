---
id: P4-017
title: REC page
phase: P4
type: task
status: review
priority: 2
executor: haiku
depends_on: [P4-014]
gate: none
---
## Goal
Live step recording: all 27 keys play and write notes, cursor when stopped, nearest step when playing.

## Read first
- docs/design/SEQUENCER_UX.md §2.5, §5 (U54, U67), §6.1 FS4 fallback (velocity 90, or 120 with K2 ACC)
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c REC page: `sq_rec_write(ch,key,vel)`, cursor, K2 ACC, K3 SLIDE, octave keys; the fixed-velocity fallback (force comes in P6-006)
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [x] Scenario steps citing REC reproduce (S07 steps 19-28, 30, 32 as rewritten; host test `ui_sq_rec_test` walks each; SAVE step 29 is P7's)
- [x] Stopped: each key writes at the cursor and advances; playing: writes at the nearest step (test: `ui_sq_rec_test`, all 27 keys and the nearest-step cases)
- [x] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps): the REC block of ui_sq.c has no string literal; the old literals (rec rows, footer, page cue override) now read knob_map, note_edit_text and the page_states cue
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-017-rec-page.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).

### 2026-11-13 executor (Sonnet 5.5)
Built the REC page in `firmware/src/ui_sq.c` (REC block before `ui_sq_other_button`; hooks in the key, knob, button, SELECT, ALGO, HOME handlers) and `tests/ui_sq_rec_test.c` (registered in tests/run_tests.sh). Scope notes:
- **ISR end missing (FB-019, priority 1, before HW-1):** "Out of scope: ISR code", so the keys write steps and queue their notes in a 16-slot ring (`uisq_aud_push`, consumer `ui_sq_aud_next`); nothing plays the queue yet. On a device REC writes silently until FB-019 lands. The UI guarantees every on has its off (owed until queued, retried on every page, also after leaving REC).
- Written step = ON + PITCH + VEL (90, or 120 with ACC) + SLIDE (K3 ON; OFF writes AUTO) in one note-edit session = one undo record; pitch stored as semitones above 36 + KEY with OCTAVE cancelled, folded into 0..96. Playing: the nearest step = `sq_rt.p` (next) if its `grid` is within half a DIV step (`SQ_DIV_TICKS`), else the sounding one.
- Table edits to match as built: GLO tap in REC = slot `GLO REC = DONE` (new v12.py MESSAGES row, `UI_MSG_ENC_GLO_REC_DONE`) instead of the two old toasts (`LEAVE REC FIRST`, `REC: tap REC to finish`: EDIT hold and PRESETS turn are now plain inert); OCT-/OCT+ act on the press; K1 while playing only frames the tile.
- Page = `UI_PG_REC` with `uisq_rec` = 1 (opened from PERF / NOTE_EDIT (cursor on the edited step) / CHAN / SET; closed by REC or a HOME tap). ALGO turn widened to REC (knob_map REC row).

Commands and decisive output:
```
sh tools/check.sh
  RAM .data+.bss 29576 B of 73728          (headroom 44152 B; before the task 29352)
  CHECK PASS
./build.sh | grep RAM
  ok    image 196708 B; RAM .data+.bss 29576 B of 73728; pool 78520 B of 368640
build/check/tests.log:
  ui_sq_rec_test: OK (0 failures)
  ui_sq_v12_test: OK (0 failures)
  v12_compare: 52 scenes/sheets, 0 failed; 12782 pixels differ only by the T_CHF rounding; ...
python3 tests/design/check_scenarios.py   -> 23 files, 490 control-map rows, 0 with no scenario
python3 tests/design/check_help.py        -> OK: 392 help rows
python3 tools/check_sequencer_ux.py       -> OK
python3 tools/check_control_map.py        -> FAIL 78 (the same 78 as before the task: RIG rows and the ALGO hold clause)
python3 tools/backlog.py check            -> 165 items, 0 errors
```
Host render vs v12 REC scenes (`rec_armed`, `rec_playing`): 0 differences (v12_compare). Stale-state and bound hazards checked: use case section 6 (list of 4 items, each with its host test).
For the reviewer: (1) FB-019 is the one real gap; (2) the K1 meter spans 1..16 whatever LENGTH is; (3) `sq_rt` / `sq_now` are read plain from the main loop (nearest step), see use case hazard 4; (4) ui_sq_perf/note/chan/set tests are unchanged and pass.
