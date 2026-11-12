---
id: P7-002
title: Preset store: save, queued load, empty-slot rule, undo of a load
phase: P7
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P7-001, P5-005, P6-002, P4-023]
gate: none
---
## Goal
16 preset slots in flash at the ADR-0005 region: SAVE stores the current state; turning PRESETS queues a load on the 768-tick bar after the 1.5 s settle; an empty slot loads nothing; SAVE hold undoes the last load.

## Read first
- docs/project/decisions/ADR-0005-preset-flash-map.md
- docs/project/decisions/ADR-0003-sequencer-core.md §10
- docs/design/SEQUENCER_UX.md (preset rows, U69), §9.5
- docs/project/OWNER_QUESTIONS.md decisions 45, 71, 79 (toast `P09 EMPTY: NOTHING LOADED`)
- firmware/src/project.c, storage.c

## Deliverable
- project.c: `SQ_NSLOT` 16 preset objects (SeqPreset v3 incl. HarmonySave), save, validated load (version, ranges), staging copy and undo-last-load copy in `.pool` (initialised explicitly: pool is not zeroed)
- ui_sq.c: PRESETS encoder with pending slot display, bar-aligned apply, empty-slot toast, SAVE tap/hold
- tests/preset_test.c: round trip of all 16 slots, power-loss mid-write (fake storage), corrupt/old version rejected, empty slot leaves the pattern unchanged, queued load applies on the bar

## Acceptance criteria
- [ ] preset_test passes
- [ ] Pool use grows by about 25128 B (build line in the Log)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P7-002-preset-store-save-queued-load-empty-slot.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Settings (P7-003)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
