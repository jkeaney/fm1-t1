---
id: P4-024
title: Core writers: sq_global_set, the reset family, and the per-frame sq_chan_sync call
phase: P4
type: task
status: review
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
- [x] Each writer has a test that passes; a global change never produces a half-built cache (fuzz or directed)
- [x] The 100000-tick fuzz of sq_chan_test passes on 3 seeds under ASan/UBSan
- [x] The LENGTH-cuts-TIE test and the step-edit undo/redo tests fail on the old code and pass on the new
- [x] No `-Wunused-function` warning from sq_chan.c (note: `sq_undo_new` is now used by `sq_undo_page`: keep it, and the warning check is on other functions)
- [x] `sh tools/check.sh` ends with `CHECK PASS`; the RAM line is pasted into the Log with headroom of at least 8192 B.


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

### 2026-11-12 executor (Sonnet 5.5)
What changed
- `sq_core.c`: step 0 of `sq_step_start`: if `ChanRT.tie` (now the LENGTH the hold was scheduled under) differs from `len`, purge the queued OFF (`sq_evq_purge_off`) and `sq_off_now`; `tie = tiehold ? len : 0` set after the rest branch (struct unchanged, 24 B). A set SLIDE ON whose next step sounds beats TIE (the glide's S1 + D); otherwise TIE; TIE still cancels RATCH. `SQ_PROB_LOW_256` = `(PROB_LOW*256+50)/100` = 128 (static-asserted in sq_edit_test).
- `sq_chan.c`: `sq_global_set(field, v)` (SQ_G_KEY/SCALE/OCTAVE/GATE/DIV/SWING; clamps; builds the other SeqGlobal + `sq_gbuild` + all four caches into the inactive buffers, flips `sq_gidx` and the four `sq_cidx` in ONE `SQ_IRQ_OFF` window, retries if the epoch moved, refused while a load is pending). `sq_note_edit_begin` + `sq_edit_record` (what = 8 UndoRec; one record per NOTE_EDIT session, own record for a tap, a clear of a non-AUTO step, or a write with no session; refused/no-change writes take none). `sq_undo`, `sq_dice_layer`, `sq_dice_all`, `sq_set_algo`, `sq_undo_new` call `sq_chan_sync()` first. `SQ_FRAME_SYNC()` is the documented per-frame hook (not wired into main.c).
- Docs: SEQUENCER_UX (API names, U16, 9.2 TIE/SLIDE/LENGTH line), ADR-0003 change note names + section 22 row 9, P4-013 task gets the AF_PAIRING live-checklist line (its feature doc does not exist yet).
- Reset family: one field = `sq_step_set(.., SQ_AUTO)`, one step = `sq_step_clear`, viewed page = `sq_page_clear`, re-copy = `sq_page_recopy`: all already delivered by P4-006; no new function needed (step ones are now undoable).
- Selected channel (FB-011 f) is UI state in ui_sq.c; no core writer. `sq_undo_new` kept (used by `sq_undo_page`), not deleted.

Tests first (fail on the old code, pass on the new)
- Old sq_core.c/sq_chan.c from HEAD plus stub `sq_global_set`/`sq_note_edit_begin` (in /tmp), new tests: `sq_edit_test`: FAIL C1 (tied note cut: OFF at tick 96, not <= 48), FAIL C3 (SLIDE+TIE: OFF at 48 not 54), FAIL U1, U3, U4 x3, U5 x2, U6, U7 x2 (13 failures). `sq_chan_test`: FAIL global field writes (all six), FAIL "sq_undo after a load landed", "sq_undo synced first", "a MIDI record from before the load is not swapped".
- New code: `SQ_EDIT PASS`, `SQ_CHAN PASS`, `SQ_CORE PASS`. Goldens (ten sq_core, sq_edit.txt, sq_chan) unchanged; no GOLDEN_UPDATE used. New edit tests add no golden lines.

Verification
```
sh /tmp/p24/b.sh sq_chan_test chan_new   # cc -std=c11 -Wall -Wextra -Werror -O1 -fsanitize=address,undefined ...
for s in 1 7 12345: /tmp/p24/chan_new $s  -> SQ_CHAN PASS (100000-tick fuzz, now with random sq_global_set and step-edit calls) x3
for s in 1 7 12345: /tmp/p24/core_new $s  -> SQ_CORE PASS x3
sh tools/check.sh -> CHECK PASS
RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640   (headroom 49900 B)
```
- `-Wunused-function` (tests build with `-Wno-unused-function`; I also compiled without it): sq_chan_test reports only functions that test does not call (sq_page_*, sq_follow_set: tested by sq_edit_test), sq_edit_test only sq_set_midi, sq_set_mute, sq_global_set, sq_defaults (tested by sq_chan_test). Nothing is dead across the two; sq_chan.c is not in felucca.c yet, so the firmware build is unaffected.
- Notes for the reviewer: (1) `sq_global_set` is refused (returns 0) while a queued load is pending, because the inactive gdb/cache buffers then hold the staged load. (2) An undo of a step edit does not clear `pg_used` bits that the write set (harmless content flag). (3) Writes made without `sq_note_edit_begin` are one record each, so the UI should call begin on key-down for the one-record-per-session behaviour. (4) The task's "ADR-0003 9.2" is SEQUENCER_UX 9.2 (ADR section 9 is the retired pin model); the line was added there and summarised in ADR section 22 row 9. (5) A LENGTH change inside the first part of the tied step cuts one step late at most (the step start), as FB-015 allowed.
