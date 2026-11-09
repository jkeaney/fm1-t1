---
id: P1-007
title: Assess the rhythmic-steps algorithm family against the vision
phase: P1
type: spike
status: done
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

### 2026-11-09 review (sonnet)
Verdict: PASS (gate none), set done.
- Both Verification commands re-run: all 8 headings present; only LATER matches the verdict (exactly one; INCLUDE/REJECT appear nowhere).
- Template: 8 sections; section 5 has 5 scored rows with reasons (3/2/1/1/3); 4 morphs (DENS LEN DRIFT ROLL), 5 dice layers; all labels <= 6 chars (CHANCE is 6); 21 uses of the warning tag, section 7 lists the ones touched. No TBD/choose/decide left; spec is promotable under ADR-0004 (seeds, salts, tables, procedure, RAM, flex points, latency all normative).
- Re-derived with a fresh throwaway Python model (xorshift32 per sting 6.2, mix32 per steparp 6.2): firing masks, rolled steps, chance/alt-gated 16-step worked-example rows for channels 0, 1, 3 match the doc exactly (templates OFF, BACK, KICK); len 12 and len 5 rows match; DENS 1..16 gives 1..16 hits, monotonic; DENS 6->9->6 restores; NUDGE nd 0/1/2 on ch3 VEL matches and step-back is exact by construction; nh=(den*len+15)/16 equals ceil for all den/len; salts collide with no other doc.
- Overlap table 6.7: every named counterpart (ACID HITS/DENS/ACCNT/OCT, SNAKE GATES/DENS/VEL/POLY/LEN, STAGE RESTS/DENS/VEL/RATCH/TOTAL mode II, ARP HITS/DENS/VEL/RATCH/CHANCE) exists in the other four docs. Consistent with owner decisions 3 and 4 (scale, GATE/DIV/SWING global).
- Euclid claim confirmed: torso_t1_algo.c has t1_euclidean_bjorklund (:25), _rotate (:99) and _eval_cycle (:116). For P1-010 it is the only even-spacing generator in the candidate set; rework through the same density/rotate morph interface. Added as owner question 21.
- Minor, not blocking: ch0 example marks step 10 as rolled but the chance gate silences it; correct per procedure, just slightly confusing.
