---
id: BUG-007
title: Preview note hangs when ChanSave.pad[0] bit 7 (PREVIEW) changes while a note sounds
phase: P4
type: bug
status: review
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
- [x] The new test fails on the old code and passes on the new.
- [x] The 100000-tick fuzz, now varying `pad[0]` bit 7, passes on 3 seeds under ASan/UBSan.
- [x] `sh tools/check.sh` ends with `CHECK PASS`; RAM line in the Log with headroom of at least 8192 B.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, voice bookkeeping; the producer-visible preview toggle has its own use case.

## Log

### 2026-11-12 executor (sonnet)
Fix: option 1, no size change. `snd_chan[i]` bit 7 was unused (the ON stores `chan & 0x6F`); it now records "this ON went to trk". All three
trk_note_off sites (same-pitch off-before-on, the OFF branch, sq_all_off) test that bit instead of SQ_PRV_ON; sq_off_now masks it off
before re-scheduling. ChanRT stays 24 B. BUG-006 fix (sq_evq_purge_off_after) untouched.
Tests: g11_preview_flag_flips (flag falls ON->OFF; flag rises ON->OFF) and the fuzz now toggles pad[0] bit 7 (r < 8).
Before the fix (same test file): `FAIL sq_core_test.c:347 preview OFF still sent after the flag fell`, `:355 no preview OFF for a note whose ON never went to the voice`, `fuzz: 60 violations`, `preview ON 3702 != OFF 3681`.
After, ASan/UBSan, seeds 1, 0x1234567, 7: each `fuzz: 100000 ticks ...` then `SQ_CORE PASS` (goldens 1-10 unchanged, incl. g10).
`sh tools/check.sh`: `RAM .data+.bss 23828 B of 73728` (headroom 49900 B), `CHECK PASS`.
