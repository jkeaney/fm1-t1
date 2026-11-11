---
id: P7-003
title: Settings persistence: globals, force floors and the first-run bit
phase: P7
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P7-002, P6-005]
gate: none
---
## Goal
Keep set-and-forget settings across power cycles without wearing flash: globals, force floors (20 x 2 B), FORCE level, first-run bit.

## Read first
- firmware/src/settings_persist.c, favorites.c (the settings blob), ui.c `ui_prefs*`
- docs/design/SEQUENCER_UX.md §6.2, §11.3
- docs/project/decisions/ADR-0005-preset-flash-map.md

## Deliverable
- settings_persist.c: the new fields with a version bump and defaults for old blobs; autosave policy (write only on change, debounced)
- settings_test/persistence_test extended for the new fields and the old-version upgrade

## Acceptance criteria
- [ ] settings_test and persistence_test pass with the new cases
- [ ] An old settings blob loads with defaults for new fields (test)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P7-003-settings-persistence-globals-force-floor.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Preset format
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
