---
id: P4-024
title: Core writers: sq_global_set, the reset family, and the per-frame sq_chan_sync call
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-005, P4-006]
gate: none
---
## Goal
Provide the core entry points the UI tasks will call but no task owned (FB-011 a, c, d, f): so P4-011..P4-018 only call APIs.

## Read first
- backlog/feedback/FB-011-*.md and FB-012-*.md
- docs/design/SEQUENCER_UX.md 1.2 and 2.x (the reset rules), ADR-0003 sections 6 and 10, docs/design/TIMING_SPEC.md
- firmware/src/sq_chan.c, sq_core.c

## Deliverable
- `sq_global_set` writers for KEY, SCALE, OCTAVE, GATE, DIV, SWING (rebuild all four caches into the inactive buffers, then flip `gdb`)
- the reset family of SEQUENCER_UX: reset one field, one step, the viewed page, re-copy the previous page (beyond P4-006's `sq_step_set`/`sq_step_clear`)
- `sq_undo` and the dice entry point call `sq_chan_sync()` first, closing the frame-wide window where a stale UndoRec could land on a freshly loaded preset (FB-012)
- a documented hook name for the main-loop per-frame call of `sq_chan_sync()` for P4-011/P4-012 to use (do not wire it into the main loop: that is the cut-over)
- delete the unused `sq_undo_new` (sq_chan.c:95)
- confirm in the Log that the selected channel is UI state (ui_sq.c) and needs no core writer
- host tests for each writer including the epoch race

- (FB-015 option A, orchestrator 2026-11-12) LENGTH change cuts a held TIE: store the LENGTH the hold was scheduled under (1..64, 0 = none) in the existing `ChanRT.tie` byte (no struct change); at each step start the ISR compares it with `sq_ch[c].len` and on a mismatch purges the queued TIE OFF (`sq_evq_purge_off`) and calls `sq_off_now`. Host test: TIE on step 1 of 8, `sq_len_set(2)` during the hold, the OFF arrives within one step.
- (FB-015) step-edit undo: add `sq_note_edit_begin(ch, p)` and a `what = 8` UndoRec taken on the first write of a NOTE_EDIT session and on `sq_step_clear`, so EDIT tap undoes a field toggle or a step clear (SEQUENCER_UX 1.2, control_map NOTE_EDIT rows). Directed tests including redo.
- (FB-015) a set SLIDE on a TIE step whose next step sounds wins (slide overlap S1 + D); otherwise TIE; TIE always cancels RATCH. Add one line to ADR-0003 9.2 and a test.
- (FB-015) derive `SQ_PROB_LOW_256` from the `PROB_LOW` flex constant: `(PROB_LOW * 256 + 50) / 100` (= 128).
- (FB-015) amend ADR-0003 and SEQUENCER_UX to the code's API names (`sq_step_tap`, `sq_page_recopy`, `sq_len_set`, `sq_page_src_set`); `sq_step_dice` is still unimplemented: say which task owns it (P6-002) in the ADR.
- (FB-015) add to the P4-013 live checklist: on AF_PAIRING algorithms (ACID) a tap or ON edit made during step p-1 is heard from the next pass, the same one-step latency the old slide pairing had.

## Acceptance criteria
- [ ] Each writer has a test that passes; a global change never produces a half-built cache (fuzz or directed)
- [ ] The 100000-tick fuzz of sq_chan_test passes on 3 seeds under ASan/UBSan
- [ ] The LENGTH-cuts-TIE test and the step-edit undo/redo tests fail on the old code and pass on the new
- [ ] No `-Wunused-function` warning from sq_chan.c (note: `sq_undo_new` is now used by `sq_undo_page`: keep it, and the warning check is on other functions)
- [ ] `sh tools/check.sh` ends with `CHECK PASS`; the RAM line is pasted into the Log with headroom of at least 8192 B.


## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, core API plumbing; producer-visible behaviour is gated in the P4 page tasks and HW-1.

## Out of scope
- Wiring into the main loop and events_block (P4-012). The preview toggle writer (BUG-007).

## Log

### 2026-11-12 note from the P4-006 review (reviewer, not a status change)
- P4-006 already delivered, in sq_chan.c: `sq_step_set` (one field, AUTO via `SQ_AUTO`), `sq_step_clear` (one step), `sq_page_clear`, `sq_page_recopy`, `sq_page_src_set`, `sq_len_set`, `sq_page_add`, `sq_follow_set`. The reset family here is therefore only what is missing: undo for step-level edits (see FB-015) and any wiring the UI needs.
- The Deliverable line "delete the unused `sq_undo_new` (sq_chan.c:95)" is stale: `sq_undo_page` (P4-006) now calls it. Keep it.
- FB-015 recommends folding the TIE-cut on LENGTH change and step-edit undo (`sq_note_edit_begin`) into this task.
