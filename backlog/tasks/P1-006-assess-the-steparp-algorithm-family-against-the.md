---
id: P1-006
title: Assess the steparp algorithm family against the vision
phase: P1
type: spike
status: todo
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Step Arp step-sequenced arpeggiator becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/steparp-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/steparp.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [ ] All 8 template sections are present and filled
- [ ] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [ ] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [ ] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [ ] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/steparp.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/steparp.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log
