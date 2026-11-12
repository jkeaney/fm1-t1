---
id: BUG-011
title: A MIDI-channel steal hits a removed (unused) channel and the toast names a channel that is not on screen
phase: P4
type: bug
status: todo
priority: 3
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
- [ ] Steps above give no steal toast; re-adding CH3 later gives a sensible MIDI (not silently OFF without notice).
- [ ] `sh tools/check.sh` ends CHECK PASS.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, bug fix; no new producer-visible element.

## Log
- 2026-11-13: filed from FB-018.
