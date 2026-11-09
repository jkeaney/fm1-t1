---
id: P1-005
title: Assess the ml185 algorithm family against the vision
phase: P1
type: spike
status: review
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the ML-185 stage sequencer becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/ml185-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/ml185.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [x] All 8 template sections are present and filled
- [x] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [x] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [x] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [x] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/ml185.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/ml185.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Wrote docs/design/algorithms/ml185.md (algorithm STAGE) following _TEMPLATE.md, in the style of sting.md and snake.md. Design only, no code.
Verification:
```
for h in '## 1.' ... '## 8.'; do grep -q "$h" docs/design/algorithms/ml185.md || echo missing "$h"; done   -> prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/ml185.md   -> one line: "**INCLUDE in v1.** STAGE brings a rhythmic idea ..."
```
Also checked with a throwaway Python transcription of 6.3 (/tmp, not in repo): sum(L)==TOTAL and L in 1..8 for all TOTAL 8..64 on 6 channels; growing TOTAL adds exactly one pulse to one stage; the source worked example (03 3.3) fire row reproduced; stage 0 always sounds for all DENS/RATCH. Informative check and RAM table (193 B/channel) come from it.
Design summary: 4 morphs TOTAL/DENS/RATCH/RANGE, 5 dice layers NOTES/LENS/RESTS/MODE/VEL (+dice-all, undo). Stateless playback p = t % TOTAL, so dice/knob changes land at the next pulse (no next-step commit; section 6.3a). Scale and GATE/DIV/SWING treated as global per owner decisions. Section 7 lists the [I]/warning reliance (mode I first-pulse, mode I- meaning, counter reset, direction, fixed length, swing, aux).

Owner questions (new, for OWNER_QUESTIONS.md):
1. Pins: white keys can only pin pulses 0..15; with TOTAL > 16 the later pulses cannot be entered by hand. Acceptable for v1, or should step entry get a page/shift?
2. Mode I- (long hold of the whole stage) ignores the global GATE setting. Acceptable?
3. TOTAL has 57 values on a relative knob; confirm the control map gives a coarse step (4) or acceleration.
4. Include STAGE in v1 as the third algorithm (recommended), or defer (LATER) to keep v1 to ACID + SNAKE + T-1 set?
5. Hardware: are defaults (TOTAL 16, DENS 6, RATCH 2, RANGE 12) good on a TB-3? Mode I behaviour is inferred, not verified.
