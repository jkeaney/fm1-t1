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

## Acceptance criteria
- [ ] Each writer has a test that passes; a global change never produces a half-built cache (fuzz or directed)
- [ ] The 100000-tick fuzz of sq_chan_test passes on 3 seeds under ASan/UBSan
- [ ] No `-Wunused-function` warning from sq_chan.c
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
