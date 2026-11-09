---
id: P1-008
title: Assess the melodic-steps algorithm family against the vision
phase: P1
type: spike
status: done
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Melodic Steps five-lane polymetric melody becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/melodic-steps-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/melodic-steps.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [x] All 8 template sections are present and filled
- [x] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [x] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [x] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [x] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/melodic-steps.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/melodic-steps.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Verification: the 8-heading loop printed nothing; `grep -E 'INCLUDE in v1|LATER|REJECT'` hits section 8 (**LATER**) and the verdict preview.
Section 6 morphs: LEN, LEAP, SPAN, DENS (all <= 6 chars); dice: NOTES, RESTS, OCT, GATE, ACCNT (5 layers) + dice-all/undo. Section 5 scored all 5 rows (3/3/2/1/3).
Informative check in 6.3 was produced by a throwaway Python transcription of the procedure (not committed).
Overlap with the existing five assessments is tabulated in 6.7 for P1-010 (rank-order DENS, velocity/accent lane, gate lane, octave lane, LEN all duplicate/near-duplicate). Genuinely new: contour pitch walk (LEAP/SPAN), per-note lane clock `hc`, pins that do not consume the contour.
All source ⚠ claims touched are listed in section 7 (none relied on).
Owner questions (also appended to docs/project/OWNER_QUESTIONS.md as #22-25):
22. Lift the contour walk into SNAKE/STAGE NOTES as a pitch-source option (answers #5)?
23. Note lanes per sounding note (hc) or per step (t)?
24. Should a typed/pinned step consume a contour note (recommend no)?
25. Cheaper v1 option: SNAKE + a LEAP morph instead of a separate MELODY (only if #22 yes)?

### 2026-11-09 review (sonnet)
Verdict: PASS (gate none).
- Verification loop prints nothing; 8 sections present; one verdict (LATER). Section 5 scored 3/3/2/1/3 with reasons; 4 morphs (LEN LEAP SPAN DENS), 5 dice layers, all labels <= 6 chars; section 7 lists the ⚠ items.
- Re-derived the procedure with a throwaway script: deg, rank, olen/oct, glen/gate, alen/vel and the 16-step example in 6.3 match exactly (including pinned steps not consuming hc by construction). Property sweep over 1,770 seeds x LEAP 0..5 x SPAN 0..14: deg[0]=0, neighbours within LEAP, all inside SPAN, 0 violations.
- PRNG salts: no collision with the other five docs (case-insensitive scan). 6.7 overlap rows spot-checked against snake/sting/steparp/rhythmic-steps (POLY 3..12, ACID gate set, JUMP, CHANCE, DRIFT): consistent.
- ADR-0004 readiness: spec complete; no TBD/choose/decide placeholders. Owner questions 22-25 follow #21 with no numbering clash.
- Not touched outside the deliverable and OWNER_QUESTIONS.md.
