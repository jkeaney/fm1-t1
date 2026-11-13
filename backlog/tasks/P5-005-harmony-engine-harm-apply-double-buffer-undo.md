---
id: P5-005
title: HARMONY engine: harm_apply, double buffer, undo
phase: P5
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P5-004, P4-005]
gate: none
---
## Goal
The chord stage every channel passes through (HARMONY.md §4), copied from the locked C reference and tested against harmony_proto.py.

## Read first
- docs/design/HARMONY.md §2, §3, §4, §6, §7, §9, §10 (H-INERT, H-GOLDEN, H-RAM, H-UNDO)
- docs/project/decisions/ADR-0003-sequencer-core.md §20
- docs/design/algorithms/common/harm.c (the reference to copy)
- tests/design/harmony_proto.py

## Deliverable
- firmware/src/sq_harm.c: copy of harm.c with `static`; `harm_db[2]` double buffer read once per block, `HarmRT`, `HarmUndo`, `undo_last`; `harm_apply()` called between `step()` and `sq_note()` in sq_core.c; `AlgoDesc.hi()` per algorithm; PROG dice
- One shared `chord at this step` function, used by the scheduler (sq_core.c ~266, ~416), sq_chan.c ~310 and the screen (ui_sq.c ~441), replacing the four `sq_hc_inert` call sites, so the screen never shows other notes than the ones played
- API for the UI: set progression slot, VOICE, MOVE, DRONE, BARS, ROLE per channel, PROG dice, undo (EDIT swaps the later of channel and harmony records)
- tests/sq_harm_test.c: H-GOLDEN tables equal harmony_proto.py output (checked-in text), H-INERT for all four algorithms against their goldens, ROLE OFF identity for any progression, H-UNDO

- Orchestrator decision 2026-11-12: this task's test also checks the `H-INERT` and `HARM 1-4-5-1` sections of every locked algorithm golden (ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS) that P5-001..P5-004, P5-007 and P5-008 leave unchecked because they need harm.c.

## Acceptance criteria
- [ ] sq_harm_test passes all four H- checks
- [ ] `.bss` growth is at most 42 B (build line before/after in the Log)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P5-005-harmony-engine-harm-apply-double-buffer.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- The HARMONY page (P5-006)
- MIDI-in chord following (deferred to v2)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: Deliverable extended (shared chord-at-step function) from the POC readiness review, finding 5.
