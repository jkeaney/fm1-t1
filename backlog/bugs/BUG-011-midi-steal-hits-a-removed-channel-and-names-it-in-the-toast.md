---
id: BUG-011
title: A MIDI-channel steal hits a removed (unused) channel and the toast names a channel that is not on screen
phase: P4
type: bug
status: done
priority: 1
executor: sonnet
depends_on: [P4-015]
gate: none
severity: low
found_in: 2ba2c02
---
## Goal
Fix: sq_set_midi scans all four ChanSave rows, so an unused channel that still holds MIDI n is set to OFF and named in `[k] MIDI OFF  n TO CHm`, so that only channels in use can be stolen from (or a removal clears the MIDI to OFF).

## Read first
- backlog/feedback/FB-018 item 3; backlog/tasks/P4-015 Log; firmware/src/sq_chan.c `sq_set_midi`; ui_sq.c CHAN commit.

## Deliverable
- Either removal clears the channel's MIDI to OFF (restored by the EDIT undo of the removal), or the commit skips unused channels. Pick the one with fewer side effects; document it. A host test that failed before.

## Steps to reproduce
1. Remove CH3 (it held MIDI 4). 2. On CH1 set MIDI 4 in CHAN.

## Expected
CH1 gets MIDI 4 with no toast about CH3.

## Actual
CH3 is set to OFF and the toast names it.

## Acceptance criteria
- [x] Steps above give no steal toast; re-adding CH3 later gives a sensible MIDI (not silently OFF without notice).
- [x] `sh tools/check.sh` ends CHECK PASS.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, bug fix; no new producer-visible element.

## Log
- 2026-11-13: filed from FB-018.

## Log
- 2026-11-13 owner ruled (open-items 3A): fix before the first flash (HW-1). Same ruling: P4-031 and P4-033 stay after HW-1 (1A, 2A); HW-1 is the first timing measurement (4A).

### 2026-11-13 executor (sonnet)
Chose: removal clears the channel's MIDI to OFF (ui_sq.c `uisq_do_remove`); fewer side effects than skipping unused channels in sq_set_midi, because skipping would leave duplicate MIDI channels on re-add and on undo of the removal. Removal also clears `sq_um_valid` (a stale MIDI record must not swap after it). The removal stack gained `uisq_gone_midi[]` (shifts with the stack of 4); EDIT undo restores the MIDI unless an in-use channel holds it by then (then OFF). Any MIDI commit already empties the stack. A re-added channel starts at MIDI OFF with the existing setup cue.
Tests: tests/ui_sq_chan_test.c, 7 new BUG-011 checks (removal sets OFF, steal without toast, EDIT undo restores, no stale restore after a MIDI change, re-add is OFF, three removals/three undos, last channel keeps MIDI); one old check (line 295) updated to expect OFF. Before the fix (firmware stashed) FAIL 295, 298, 300, 304, 306; after:
`sh tools/check.sh` -> `RAM .data+.bss 29352 B of 73728` (headroom about 44 KB), `CHECK PASS`.
Docs synced: docs/design/SCREEN_SPEC_V2.md section 6, docs/design/SEQUENCER_UX.md 4.4 (new row).

### 2026-11-13 review (sonnet)
Verdict: PASS. `sh tools/check.sh` -> CHECK PASS; `RAM .data+.bss 29352 B of 73728` (headroom 44376 B, >= 8192 B).
- Tests fail without the fix: with ui_sq.c from HEAD~1, ui_sq_chan_test reports 5 failures (295, 298, 300, 304, 306); with the fix none.
- Hazards checked: stack of 4 (uisq_gone_midi shifts with the others; at most 3 removals as the last channel cannot go); undo of removal restores MIDI unless an in-use channel holds it (OFF, no duplicate); sq_um_valid cleared at removal; MIDI commit, dice, add and page-add empty the stack (ui_sq.c:1306, 1361, 1383, 1424, 1529, 1578); re-add after a drop-off or an add starts at OFF with the setup cue; pending offs use the channel byte stored at note-on (sq_core.c:157-176), so OFF at removal cannot strand a note.
- Docs: SCREEN_SPEC_V2 section 6 and SEQUENCER_UX 4.4 updated; generated scenarios/key_map/control_map only say "REMOVE CHn (undoable)", still true. No use case in docs/design/features mentions removal or steal.
- Minor, non-blocking: EDIT undo of a removal whose MIDI number was taken meanwhile restores the channel at MIDI OFF with no toast (ui_sq.c ~1536); can only happen if the stack survived a MIDI change, which it cannot (commit empties it), so the branch is effectively unreachable defensive code.
