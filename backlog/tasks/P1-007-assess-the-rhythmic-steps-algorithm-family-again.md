---
id: P1-007
title: Assess the rhythmic-steps algorithm family against the vision
phase: P1
type: spike
status: review
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Rhythmic Steps layered drum steps becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/rhythmic-steps-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/rhythmic-steps.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [x] All 8 template sections are present and filled
- [x] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [x] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [x] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [x] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/rhythmic-steps.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/rhythmic-steps.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Design/assessment only; no code, tests, flashing or push. Python model of the procedure used for the worked example
(throwaway, /tmp, not committed).
```
for h in '## 1.' ... '## 8.'; do grep -q ... || echo missing; done   -> (no output)
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/rhythmic-steps.md  -> only LATER lines (verdict)
```
Checks run in the model: DENS 1..16 gives 1..16 hits and is monotonic; DENS 6->9->6 restores the pattern; default
multiplier 0x9E37 gives four different templates on channels 0..3; NUDGE nd-1 restores nd-1 exactly.
Verdict: LATER (score 3/2/1/1/3). Overlap table for P1-010 is in section 6.7. No rhythm generator (Euclidean-style)
in this family; Euclid exists only in the T-1 code (torso_t1_algo.c). Two ideas worth lifting regardless: groove-template
HITS order, and NUDGE.
Owner questions (also added to docs/project/OWNER_QUESTIONS.md as 17-20):
17. Fixed base note 36 / alt +12 for a pitchless algorithm, or a drum note setting (breaks rule 3)?
18. Pin at or past LEN silent until LEN is raised: acceptable?
19. Add NUDGE to the global dice vocabulary for all algorithms?
20. Is a drum-voice channel a v1 goal? Lift the groove-template HITS order into the shared density primitive?
