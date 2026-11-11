---
id: P2-027
title: Record the owner-confirmed headphone jack in the preview design (decision 112)
phase: P2
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P2-025]
gate: none
---
## Goal
Decision 112: the FM-1 has a speaker master output and a headphone jack at the back. PREVIEW.md currently says no headphone output appears in the docs/firmware and leaves it for HW-1. Fix that so the design states the owner-confirmed hardware.

## Read first
- docs/project/OWNER_QUESTIONS.md decision 112
- docs/design/PREVIEW.md (grep headphone, cue bus, HW-1), docs/design/SYSTEM_MAP.md, CLAUDE.md hardware facts

## Deliverable
- PREVIEW.md: preview of every channel with PREVIEW ON goes to the master output (speaker or headphones, whichever the device routes); owner-confirmed 2026-11-11; no separate cue bus is designed; whether the speaker mutes when headphones are plugged in is an HW-1 check (not known). Mute controls = MIDI gate only (already so).
- Any other doc/task that says "no headphone output" or defers the headphone question (grep docs/ and backlog/) corrected; HW-1 wording adjusted to check the plug-in behaviour only.
- Add a line to CLAUDE.md hardware facts: "Audio out: speaker plus a rear headphone jack (owner-confirmed 2026-11-11)".

## Acceptance criteria
- [ ] Greps for 'no headphone' / 'cue bus' claims show only corrected text
- [ ] Design checkers pass (check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh, backlog.py check)

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware, builds, tools/check.sh; files owned by the active P3-003 executor.

## Log
