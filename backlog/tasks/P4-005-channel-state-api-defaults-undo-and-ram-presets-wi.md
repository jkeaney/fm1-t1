---
id: P4-005
title: Channel state API, defaults, undo and RAM presets with the invariant fuzz
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-004]
gate: none
---
## Goal
Everything the UI may call (ADR-0003 §6): morph turn, dice, undo, set MIDI channel / algorithm / mute, rebuild with the cache flip and epoch guard, SQ_DEFAULTS, preset save/load to RAM with validation.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §3.1, §6, §9, §10, §12
- docs/design/SEQUENCER_UX.md §1.2 (what dice/morph/algorithm changes do to edits), §4.4 (unique MIDI channels, decision 77)
- docs/project/OWNER_QUESTIONS.md decisions 46, 47, 73, 77

## Deliverable
- sq_core.c (or sq_chan.c included after it): `sq_morph_turn`, `sq_dice_layer`, `sq_dice_all`, `sq_undo`, `sq_set_midi` (unique rule: taking a used channel sets the other to OFF, one undo restores both), `sq_set_algo`, `sq_set_mute`, `sq_rebuild`, `SQ_DEFAULTS` (CH1-4 = MIDI 1-4, ACID/EUCLID/SNAKE/STAGE, ADR-0003 §12), `sq_preset_save/load` (RAM) with version and range validation
- tests/sq_chan_test.c: table-driven tests of SEQUENCER_UX 1.2 rows, unique-MIDI cases, undo/redo swap, preset round trip and corrupt-preset rejection; the fuzz extended to random API calls for 100000 ticks under sanitizers

## Acceptance criteria
- [ ] sq_chan_test passes, including the fuzz
- [ ] Defaults match ADR-0003 §12 exactly (test asserts each field)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Flash storage (P7)
- UI
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
