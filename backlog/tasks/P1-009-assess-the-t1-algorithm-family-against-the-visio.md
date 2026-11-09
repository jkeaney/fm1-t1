---
id: P1-009
title: Assess the t1 algorithm family against the vision
phase: P1
type: spike
status: todo
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Torso T-1 (this firmware's current engine) becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/t1-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md
- docs/T1_ISSUES.md (known defects)
- firmware/src/torso_t1_algo.c, torso_t1_pipeline.c

## Deliverable
- docs/design/algorithms/t1.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [ ] All 8 template sections are present and filled
- [ ] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [ ] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [ ] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [ ] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/t1.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/t1.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log
