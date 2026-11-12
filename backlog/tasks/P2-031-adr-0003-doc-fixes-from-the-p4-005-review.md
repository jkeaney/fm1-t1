---
id: P2-031
title: ADR-0003 doc fixes from the P4-005 review (bad preset rejects, sq_chan_sync, dice entropy)
phase: P2
type: task
status: review
priority: 2
executor: sonnet
depends_on: [P4-005]
gate: none
---
## Goal
Make ADR-0003 say what the reviewed code does (FB-012), so later tasks copy a correct contract.

## Read first
- backlog/feedback/FB-012-*.md, backlog/tasks/P4-005-*.md Log
- docs/project/decisions/ADR-0003-sequencer-core.md sections 3.1, 6, 10, 12

## Deliverable
- Section 10: an invalid version, CRC or algorithm id REJECTS the load (state untouched, `sq_preset_load` returns -1); the storage layer (P7-002) decides what an empty or corrupt slot shows
- Sections 6 and 10: the ISR only bumps `sq_epoch`; the main loop's `sq_chan_sync()` clears the UndoRecs; name `sq_chan_sync` in the P4-011 / P4-012 hook lists (task files)
- Section 3.1: dice entropy is the free-running sample counter mixed with a dice counter; no `fm1_ms` hook

## Acceptance criteria
- [x] ADR-0003 sections 3.1, 6, 10 and 12 agree with sq_chan.c
- [x] P4-011 and P4-012 Deliverables name `sq_chan_sync()` and its per-frame call
- [x] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, specification text.

## Out of scope
- Firmware changes.

## Log

### 2026-11-12 executor (sonnet)
Edited ADR-0003 sections 3.1 (dice entropy), 6 (sq_chan_sync), 10 (reject on bad version/CRC/algo id; UndoRec clear moved to sq_chan_sync), 12 step 6 (rejected load keeps defaults); P4-011 and P4-012 Deliverables name sq_chan_sync per frame. Section 22 untouched. Checked against sq_chan.c (sq_entropy, sq_dice_n, sq_chan_sync). `python3 tools/backlog.py check`: see below.

### 2026-11-12 review (sonnet)
Verdict: FAIL (one doc/code mismatch). Docs only; no build run (P4-023 owns it). `python3 tools/backlog.py check`: 148 items, 0 errors.
Verified OK: section 3.1 (sq_chan.c:85 sq_entropy = sq_entropy_ctr mixed with ++sq_dice_n, no fm1_ms; sq_core.c:81,459 counter runs while stopped); section 6 (sq_chan_sync at sq_chan.c:354 clears sq_undo_rec on epoch change; ISR only bumps sq_epoch); section 10 apply path (epoch bump, sq_chan_sync clears UndoRecs); section 12 step 6 clause consistent with section 10 and with sq_preset_load returning -1 (sq_chan.c:301-302); section 22 untouched (diff has no hunk past line 675; section 22 starts at 912); P4-011 and P4-012 Deliverables name sq_chan_sync() and the once-per-frame call.
Findings:
- [ ] ADR-0003-sequencer-core.md:573 says an invalid CRC makes `sq_preset_load` return -1. The code does not: sq_preset_load (sq_chan.c:300-302) rejects only a wrong version or algo id; the CRC is never seen there (sq_chan.c:14,277 and the ADR's own SeqPreset field comment at line 159 say the storage layer, P7, computes and checks it). Fix: say "an invalid `version` or algorithm id rejects the load (-1, state untouched); the storage layer (P7-002) checks the CRC before calling `sq_preset_load` and decides what a corrupt slot shows". Section 12 step 6's "a rejected load returns -1" stays correct.
- Minor (optional): section 3.1 says "low 16 bits of a free-running sample counter"; sq_entropy also folds in `ctr >> 16`.

### 2026-11-12 executor (sonnet), review fix
Dropped CRC from the reject list (sections 10; 12 step 6 only refers to section 10): sq_preset_load rejects a wrong version or algo id; the storage layer (P7-002) checks the CRC first. Section 3.1 now says the counter's low 16 bits folded with its high 16 bits. Section 22 unchanged.
