---
id: P1-010
title: Choose the v1 algorithm set and the shared layer/morph model
phase: P1
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P1-011, P1-003, P1-004, P1-005, P1-006, P1-007, P1-008, P1-009]
gate: human-review
---
## Goal
One coherent algorithm menu for channels: which algorithms ship in v1, the layers they share (so the dice buttons mean the same thing everywhere) and a consistent knob morph scheme.

## Read first
- docs/project/OWNER_QUESTIONS.md (owner decisions: scale, GATE/DIV/SWING are global; ACID has slide)
- docs/design/algorithms/*.md (all 7 assessments)
- docs/project/VISION.md

## Deliverable
- docs/design/ALGORITHM_SET.md: the v1 list (3–6 algorithms) with a reason for each; the shared layer vocabulary (e.g. RHYTHM, PITCH, ACCENT, LENGTH) and which algorithm supports which; the knob convention (e.g. K1 always density-like, K4 always variation); an implementation order; the rejected or later list

## Acceptance criteria
- [x] Every INCLUDE recommendation from the assessments is either in the v1 list or explicitly deferred with a reason
- [x] The layer vocabulary has at most 5 layers, and every v1 algorithm maps onto it
- [ ] The owner approved it (log line 'Approved: <name> <date>')

## Verification
```sh
test -f docs/design/ALGORITHM_SET.md && grep -c '^|' docs/design/ALGORITHM_SET.md
```

## Out of scope
- Interface code (that's P2-004).

## Log

### 2026-11-09 executor (sonnet)
Wrote docs/design/ALGORITHM_SET.md (design only; no code, assessments not edited).
- `test -f docs/design/ALGORITHM_SET.md && grep -c '^|' docs/design/ALGORITHM_SET.md` -> table-row count printed (see below)
- v1: ACID, EUCLID, SNAKE, STAGE; deferred ARP, RHYTHM, MELODY. 5 shared layers NOTES/HITS/ACCNT/ARTIC/SHAPE. Knobs K1 DENSITY, K2 FORM, K3 PITCH, K4 SPICE (edits table 5.4). 15 shared primitives. 30 owner questions answered as recommendations (+N1..N3).
- Reviewer attention: (1) ACID SLIDE shares the ARTIC button with GATE, which conflicts with owner decision 2 ("sixth dice layer"); flagged as N1. (2) Recommends changing the shared quantiser to nearest-ties-lower (changes T-1 behaviour). (3) Recommends SNAKE RANGE default 24 -> 12. (4) Knob reorders needed in sting, snake, ml185 docs (not applied).
- Third criterion (owner approval) is intentionally unticked; no 'Approved:' line written. Awaiting owner.

### 2026-11-09 review (sonnet)
Verdict: PASS on checkable criteria; **Awaiting owner** (gate human-review; status left at review; no Approved line written).
Verified against the 7 assessments: all 4 INCLUDE (sting, t1, snake, ml185) are in v1; ARP, RHYTHM, MELODY are LATER with reasons. 5 layers; every seed maps (ACID 6 seeds/5 buttons, EUCLID ROT+NOTES+ACC+RAT, SNAKE NOTES+GATES+VEL+POLY, STAGE NOTES+LENS+RESTS+MODE+VEL), seed names/defaults match each 6.1. K1-K4 table matches each assessment's morph ranges/defaults; edit table 5.4 is concrete. 15 primitives SP1-SP15 counted. Small fix made: section 9 now states the snap change keeps the existing phase-2 test assertions green but alters current T-1 pipeline output.
Findings for the owner (judgement, not defects):
- SHAPE is a grab-bag: ACID register jumps, SNAKE velocity-lane length (POLY), STAGE which stages are long (LENS, arguably rhythm); absent in EUCLID. Hard to label understandably on screen (rule 1).
- ACID SLIDE shares ARTIC with GATE: decision 2 ("sixth dice layer") is not met literally; slide cannot be re-rolled alone. Fallback (long press) is a table edit.
- Some K2/K4 fits are stretches (ACID TYPE as FORM, SNAKE PATH as SPICE).
- Snap change, SNAKE RANGE 24->12, pentatonic default scale are recommendations only.
Awaiting owner: approve N1, SHAPE naming, snap rule, default scale, and the 4-algorithm set.
