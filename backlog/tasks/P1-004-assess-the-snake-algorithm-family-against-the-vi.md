---
id: P1-004
title: Assess the snake algorithm family against the vision
phase: P1
type: spike
status: review
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Snake polymetric grid sequencer becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/snake-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/snake.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [ ] All 8 template sections are present and filled
- [ ] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [ ] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [ ] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [ ] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/snake.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/snake.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Wrote docs/design/algorithms/snake.md (8 sections, modelled on sting.md; design only, no code run beyond a throwaway
Python transcription in /tmp, not committed, used to check the shape tables and the default pattern).
Verification:
```
for h in '## 1.' ... '## 8.'; do grep -q ... || echo missing; done   -> prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' snake.md                          -> one line, section 8: "**INCLUDE in v1.** ..."
```
Throwaway check: all 14 shape tables are permutations of 1..16; shapes 5-8 and 11-14 contiguous (Manhattan 1); channels
0..5 defaults all give 10 hits in 16 steps. Criteria: 8 sections present; section 5 has 5 scored rows with reasons;
section 6 has 4 morphs (PATH, LEN, DENS, RANGE; labels <= 6 chars) and 4 dice layers (NOTES, GATES, VEL, POLY);
section 8 = INCLUDE in v1; every design point touching a source claim is in the section 7 table.
Owner questions:
1. Scale fallback: SNAKE uses minor pentatonic when SCL scale is off (ACID uses chromatic). Per-algorithm or global?
2. Is a uniformly random pitch lane acceptable as a bass? (lower default RANGE or weighted pitches are the fallback.)
3. Custom modulation lanes dropped; should a CC lane come back post-v1?
4. Shared channel settings (DIV, GATE, SWING, SCL) still have no spec; reference impl uses the defaults in section 7.
5. Owner re-lock after golden files exist (ADR-0004).
