---
id: P6-002
title: Dice: five layers plus ALL, sub-labels, undo and redo
phase: P6
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P6-001]
gate: none
---
## Goal
The dice buttons re-roll one layer each; FX = ALL; the screen names the algorithm's layer under each button; EDIT tap undoes, tap again redoes.

## Read first
- docs/design/ALGORITHM_SET.md §4 (layers, mapping matrix, dice actions, absent layers)
- docs/design/SEQUENCER_UX.md §3 (button modes), §1.2
- docs/project/OWNER_QUESTIONS.md decision 46
- firmware/src/ui_sq.c

## Deliverable
- ui_sq.c: dice button handling to `sq_dice_layer`/`sq_dice_all`, sub-labels (<= 6 chars) in Z4, toast, absent layer shows `-` and does nothing, EDIT undo/redo

## Acceptance criteria
- [ ] Each algorithm x each layer button calls the right layer or is inert when absent (table-driven test)
- [ ] Undo then redo restores byte-identical ChanSave (test)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P6-002-dice-five-layers-plus-all-sub-labels-und.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
