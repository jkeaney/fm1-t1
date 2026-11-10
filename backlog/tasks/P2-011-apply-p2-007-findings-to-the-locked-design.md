---
id: P2-011
title: Apply the P2-007 scenario findings to the locked design docs
phase: P2
type: task
status: todo
priority: 0
executor: sonnet
depends_on: [P2-007]
gate: human-review
---
## Goal
The scenario walkthroughs (P2-007) found 19 defects (F-01..F-19) in the locked design docs, all confirmed real by the independent review. Fix them so the design is implementable, without changing any owner decision.

## Read first
- docs/design/scenarios/FINDINGS.md (the defects, with evidence), docs/design/scenarios/COVERAGE.md
- docs/design/SCREEN_SPEC.md, SEQUENCER_UX.md, TIMING_SPEC.md, the TSVs, ADR-0003
- docs/project/OWNER_QUESTIONS.md rows 65-76 (decided; do not reopen)
- the P2-007 review in backlog/tasks/P2-007-scenario-walkthroughs-ux-validation.md Log

## Deliverable
- Every FINDINGS defect fixed in the owning doc/table or, where a fix needs an owner choice, raised as a numbered owner question with options (do not decide silently). Needed at minimum: the cue-strip row order (13a/13b reachable, F-01), a toast zone in SCREEN_SPEC (F-03), overflowing strings re-fitted with measured widths (F-04: MIDI 2 MUTE, KEYS ONLY, header slots H4/H5), a HELP text table (F-06), MENU roles reconciled (F-02), the unmuted-channel MIDI clash (F-14, hold as an owner question). Q2 and Q3 in FINDINGS are already answered by OWNER_QUESTIONS 76: mark them answered and remove the 'no way to start on internal' wording from S06; fix the S06 step 2 arithmetic (139 ms); disclose the gen_sweeps.py respelling and that sweep-only rows are table echoes; commit the F-04 width probe under tests/design/.
- FINDINGS.md updated with the status of each defect (fixed / owner question).
- Checks stay green: tools/check_sequencer_ux.py, tools/check_control_map.py, tests/design/check_scenarios.py, backlog.py check.

## Acceptance criteria
- [ ] All 19 defects marked fixed or raised as an owner question, none left open silently
- [ ] No owner decision (OWNER_QUESTIONS 65-76) changed
- [ ] The three check scripts and `backlog.py check` pass
- [ ] Owner approval

## Verification
```sh
python3 tools/check_sequencer_ux.py && python3 tools/check_control_map.py
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Firmware changes. Q1 (does the 2-gesture rule satisfy VISION) and Q4 (warn on shared MIDI channel) are owner questions, not part of this fix pass unless answered.

## Log
- 2026-11-10 created by the orchestrator from the P2-007 findings.

(Owner 2026-11-10, P2-007 gate: 1B approve P2-007 together with P2-011; 2A two gestures allowed for setup functions; 3 MIDI channels unique per sequencer channel: assigning a used MIDI channel removes it from the other (OFF); 4B push after P2-011; 5A P0-005 deferred; 6A Ableton Sync check to be done by owner later.)
