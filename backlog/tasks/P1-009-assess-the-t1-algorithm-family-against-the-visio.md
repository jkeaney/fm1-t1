---
id: P1-009
title: Assess the t1 algorithm family against the vision
phase: P1
type: spike
status: review
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
- [x] All 8 template sections are present and filled
- [x] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [x] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [x] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [x] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/t1.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/t1.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Deliverable: docs/design/algorithms/t1.md (algorithm EUCLID). Also appended owner questions 26-30 to docs/project/OWNER_QUESTIONS.md.
Verification:
```
for h in '## 1.' ... '## 8.'; do grep -q "$h" docs/design/algorithms/t1.md || echo missing "$h"; done   -> prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/t1.md  -> one line: "**INCLUDE in v1.** EUCLID is the one algorithm ..."
```
Criteria: 8 sections present; section 5 has 5 scored rows (3,3,2,3,3) with reasons; section 6 has 4 morphs (HITS, LEN, SPAN, ACCNT) and 4 dice layers (ROT, NOTES, ACCNT, RATCH), all labels <= 6 chars; section 8 is INCLUDE in v1 only; ⚠ reliance is in section 7 (items 1, 2, 5, 6).
Summary: reuse unchanged = t1_euclidean_bjorklund, _rotate, t1_harmony_quantize, T1_SCALE_MASKS; reuse after change = ratchet maths (scalar signature), eval_phrase (after F3 fix); retire = eval_cycle, LFSR, p-locks, style, chord/voicing (kept for a later ARP), pipeline tick, T-1 UI/state. Defects mapped per function in 6.8. Overlap with all six other assessments in 6.7. Recommendation: Euclidean algorithm IN v1 (only even-spacing placement; RHYTHM lacks it).
No code, tests, flashing or push (docs-only task, so no check.sh).

Owner questions: 26 LEN max 16 vs 32/64; 27 RATCH as fixed-odds dice vs knob; 28 ROTATE knob instead of LEN; 29 root-anchored vs centred contour; 30 EUCLID as 4th v1 algorithm vs shared placement source. Existing relevant: 16 (snap direction), 22, 24, 6.
