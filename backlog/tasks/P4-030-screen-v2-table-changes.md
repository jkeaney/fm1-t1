---
id: P4-030
title: Screen v2 table changes: add/remove channel, M-key hold timing, page_states and help rows, widths
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Bring the authoritative tables in line with the locked screen (owner decisions 121-126) so the firmware, the checkers and
the screen agree.

## Read first
- docs/design/SCREEN_SPEC_V2.md sections 5 and 6; docs/project/OWNER_QUESTIONS.md rows 121-126
- docs/design/knob_map.tsv (PERF ENC_SEL "clamped, no wrap"), key_map.tsv (KEY_B01..B04 hold = PREVIEW at 0.4 s),
  control_map.tsv, page_states.tsv (PERF/CHAN/REC/FIRST_RUN screen and cue_strip columns), help_text.tsv, tests/design/widths.tsv
- tests/design/check_scenarios.py, check_help.py, gen_help.py, gen_sweeps.py, width_probe.sh; tools/gen_ui_tables.py

## Deliverable
- SELECT: one detent past the last channel in use adds the next free channel (toast `[n] NEW  ALGO=PICK`).
- M1-M4 hold: release before 1.5 s = PREVIEW on/off (on release); 1.5 s = remove the channel (undoable, toast `[n] GONE  EDIT=UNDO`).
- page_states PERF/CHAN/REC/FIRST_RUN rows rewritten to SCREEN_SPEC_V2 (message slot, setup cue `CHN KEY, K2 = MIDI`, REC cue).
- help_text rows for the message strings; widths.tsv rows for the message slot.
- The owner signs the M-key timing in the Log (decision 125 timing was a design proposal).

## Acceptance criteria
- [ ] gen_help.py, gen_sweeps.py, check_help.py, check_scenarios.py, width_probe.sh, `tools/gen_ui_tables.py --check` all pass
- [ ] owner sign-off line in the Log

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- Firmware drawing; other pages' layouts.

## Log
