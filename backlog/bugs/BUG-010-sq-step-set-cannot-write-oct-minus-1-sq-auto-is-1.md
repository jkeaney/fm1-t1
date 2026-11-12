---
id: BUG-010
title: sq_step_set cannot write OCT -1 because SQ_AUTO is -1
phase: P4
type: bug
status: review
priority: 2
executor: sonnet
depends_on: []
gate: none
severity: medium   # a control misbehaves: NOTE_EDIT OCT- could not reach -1 or -2
found_in: bb1c058
---
## Goal
Fix: `sq_step_set(ch, p, SQ_F_OCT, -1)` so that it writes an octave shift of -1 instead of clearing the field.

## Read first
- firmware/src/sq_chan.c (SQ_AUTO, sq_step_set); SEQUENCER_UX 1.1 (OCT -2..+2)

## Deliverable
- `SQ_AUTO` is no longer an in-range value (now -32768), found while building the NOTE_EDIT page (P4-014); it was committed with that task because
  the page's OCT- button could not leave 0. Every user of the macro is symbolic (sq_edit_test, sq_chan_test, ui_sq.c), so nothing else changes.

## Steps to reproduce
1. `sq_step_set(0, 0, SQ_F_OCT, -1)`.

## Expected
`ed[0].fl >> 5 == 2` (value + 3).

## Actual
The field is cleared (`fl >> 5 == 0`): -1 was the AUTO sentinel.

## Acceptance criteria
- [x] The new test fails on the old code and passes on the new: tests/ui_sq_note_test.c (OCT- reaches -1 and -2); tests/sq_edit_test.c still passes.
- [ ] `tools/check.sh` passes.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, a fix inside P4-014's page (docs/design/features/P4-014-note-edit-page.md).

## Log
- 2026-11-13 found and fixed in P4-014 (one line in sq_chan.c). Reviewer: confirm the change is acceptable outside that task's named files, or revert it and OCT- will stop at 0.
