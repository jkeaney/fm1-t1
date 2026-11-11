---
id: P2-028
title: Record that headphones auto-mute the speaker (decision 113)
phase: P2
type: task
status: todo
priority: 3
executor: haiku
depends_on: [P2-027]
gate: none
---
## Goal
Decision 113: on the owner's FM-1, plugging in headphones switches the speaker off automatically [V]. Update the docs that treated this as an open HW-1 check.

## Read first
- docs/project/OWNER_QUESTIONS.md decision 113
- docs/design/PREVIEW.md (grep "headphone", "HW-1"), CLAUDE.md hardware facts line "Audio out", backlog tasks mentioning the speaker-mute check (grep -rn "headphone" backlog docs)

## Deliverable
- PREVIEW.md: state that preview plays on the speaker, or the headphones when plugged in (hardware auto-switch, verified on the owner's unit 2026-11-11); remove the open check.
- CLAUDE.md "Audio out" line: add "headphones auto-mute the speaker (verified 2026-11-11)".
- Any HW-1 task/doc wording that lists this as a check to do: mark it answered.

## Acceptance criteria
- [ ] grep -rn "speaker mutes when headphones" docs backlog CLAUDE.md shows only answered/historical text
- [ ] python3 tools/backlog.py check passes; python3 tests/design/check_sequencer_ux.py passes

## Verification
```sh
python3 tools/backlog.py check; python3 tools/check_sequencer_ux.py; python3 tests/design/check_help.py
```

## Out of scope
- Firmware, builds; files owned by the active P3-003 executor.

## Log
