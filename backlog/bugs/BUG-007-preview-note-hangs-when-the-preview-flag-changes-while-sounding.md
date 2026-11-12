---
id: BUG-007
title: Preview note hangs when ChanSave.pad[0] bit 7 (PREVIEW) changes while a note sounds
phase: P4
type: bug
status: todo
priority: 2
executor: sonnet
depends_on: [P4-005]
gate: none
severity: medium   # latent today: nothing toggles the bit yet; real once the preview toggle or a preset load changes it
found_in: P4-005 review, e5e8214
---
## Goal
Fix: `sq_emit` gates `trk_note_off` on the current value of `SQ_PRV_ON(c)`, so clearing the bit between a note ON and its OFF leaves the preview voice hanging (and a later ON hits the host mock's "double ON"). Expected: every preview ON gets its OFF.

## Read first
- backlog/feedback/FB-013-*.md, docs/design/PREVIEW.md
- firmware/src/sq_core.c (`sq_emit` lines ~89, ~106, ~307) and ChanRT.snd

## Deliverable
- The fix: either always send `trk_note_off` for a note whose ON went to `trk` by recording the destination in `ChanRT.snd`, or send a note-off for a sounding preview note when the bit falls (main-loop request flag consumed by the ISR). Choose the one that keeps ChanRT at 24 B or explain the size change.
- A sq_core test where the bit falls while a note sounds, and a fuzz that varies `pad[0]` bit 7 (the current fuzz keeps it constant).

## Steps to reproduce
1. Channel with PREVIEW on, a note ON sent to `trk[c]`.
2. Clear bit 7 of `pad[0]` (a preset load or the future toggle) before the note's OFF falls due.
3. Run to the OFF time.

## Expected
`trk_note_off` is sent; no voice hangs.

## Actual
The OFF is gated on the new bit value and never reaches `trk[c]`.

## Acceptance criteria
- [ ] The new test fails on the old code and passes on the new.
- [ ] The 100000-tick fuzz, now varying `pad[0]` bit 7, passes on 3 seeds under ASan/UBSan.
- [ ] `sh tools/check.sh` ends with `CHECK PASS`; RAM line in the Log with headroom of at least 8192 B.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, voice bookkeeping; the producer-visible preview toggle has its own use case.

## Log
