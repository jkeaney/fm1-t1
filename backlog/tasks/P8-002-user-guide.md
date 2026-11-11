---
id: P8-002
title: User guide
phase: P8
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P8-001]
gate: human-review
---
## Goal
A short user guide a producer reads in ten minutes, built from the locked screen spec and help text, starting with the target workflow.

## Read first
- docs/project/VISION.md
- docs/design/SEQUENCER_UX.md §7, §11.4
- docs/design/help_text.tsv
- docs/design/SCREEN_SPEC.md §7 mock-ups
- docs/design/HARMONY.md §5
- docs/design/KEYBED_USE_CASES.md
- docs/design/algorithms/*/SPEC.md (one paragraph each)

## Deliverable
- docs/USER_GUIDE.md: 1 Quick start (the target workflow with a TB-3, gesture by gesture), 2 The screen, 3 Pages, 4 The six algorithms (what each knob and dice layer does), 5 HARMONY, 6 Performing (morph, dice, mute, fill, roll, force), 7 Presets, 8 Clock and sync (Ableton, TB-3 via host box), 9 Install, update and restore, 10 Troubleshooting; every on-screen string quoted exactly from help_text.tsv

## Acceptance criteria
- [ ] Every page id in page_states.tsv appears in the guide
- [ ] Every quoted screen string exists in help_text.tsv or SCREEN_SPEC (a script checks)
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
