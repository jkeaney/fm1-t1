---
id: P1-011
title: Add a slide dice layer to the ACID design
phase: P1
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P1-003]
gate: none
---
## Goal
Owner decision: ACID gets slide (TB-3 portamento) as a sixth dice layer in v1, with no guessing left for a dev task (ADR-0004).

## Read first
- docs/design/algorithms/sting.md
- docs/project/OWNER_QUESTIONS.md (decisions 2-4: scale and GATE/DIV/SWING are global)
- docs/sting-algorithms/ (how slide works in the source; mind [V]/[I]/⚠)
- docs/project/decisions/ADR-0004-design-lock.md

## Deliverable
- docs/design/algorithms/sting.md updated: a SLIDE dice layer (label at most 6 chars) with its own seed, probability table, undo, procedure and how slide is emitted as MIDI for a TB-3 (overlapping note-on before note-off, legato rules, and any CC); parameters table row; RAM update
- Section 2/6/7 updated; scale wording changed to use the single global scale (no ACID-private scale snap controls)

## Acceptance criteria
- [ ] Section 6 lists 6 dice layers including SLIDE; still at most 4 knob morphs; labels at most 6 chars
- [ ] The slide MIDI emission rule is fully specified, with a worked 8-step example showing note-on/off order
- [ ] No TBD/choose/decide wording; any ⚠ reliance listed in section 7
- [ ] Scale and GATE/DIV/SWING are described as global settings

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/sting.md || echo missing "$h"; done   # prints nothing
grep -n 'SLIDE' docs/design/algorithms/sting.md
```

## Out of scope
- Other algorithms; code.

## Log
