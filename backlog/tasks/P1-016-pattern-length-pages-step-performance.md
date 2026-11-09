---
id: P1-016
title: Pattern length per channel, paged keyboard steps, per-step performance and keybed expression: integrate into the design
phase: P1
type: spike
status: dropped
priority: 0
executor: sonnet
depends_on: [P1-014, P1-015]
gate: human-review
---
## Goal
Owner requirements R1-R4 (ninth round): each channel has its own pattern length; the 16 white keys are paginated pages of the viewed channel's steps with page swapping; selecting a step has performative functionality (ratchet, slide, mute, accent, probability ...); keybed expression per the reference found in P1-015. Integrate all of it into the design with the ease score (including L, learnable without a manual) and keep everything inside the UX rules.

## Read first
- docs/project/OWNER_QUESTIONS.md (R1-R4 and all decisions), VISION.md (rule 3 amended by R1), ADR-0004
- docs/design/PERFORMANCE_DESIGN.md (P1-014), FULL_CONTROL.md, UX_REVIEW.md, CONTROL_MAP.md, control_map.tsv, tools/check_control_map.py
- docs/design/KEYBED_EXPRESSION_REFERENCE.md (P1-015)
- docs/design/ALGORITHM_SET.md and section 6 of docs/design/algorithms/{sting,snake,ml185,t1}.md (each algorithm's own length: ACID/SNAKE lane lengths, EUCLID LEN 2..16, STAGE TOTAL 8..64: reconcile with a channel pattern length)
- docs/project/decisions/ADR-0003-sequencer-core.md (Step, Channel structs, RAM table)

## Deliverable
- docs/design/PATTERN_AND_STEPS.md: (1) pattern length: range (for example 1..64), default, where it is set (a knob/shift row slot? a hold?), how each algorithm treats it (its own lengths become divisors/lanes of the channel length, or the channel length replaces them: specify per algorithm with formulas), what happens when length changes (truncate/extend, pinned steps kept), and score; (2) pagination: page size 16, page swap control (gesture), how the screen shows page N of M and the playhead across pages, follow-playhead vs hold-page rule, and how white keys map on partial last pages; (3) selecting a step: gesture, what the screen shows, and the per-step performative properties (ratchet count, slide, mute, accent, probability, pitch nudge ...), each with value range, how it is entered (hold step key + knob, keybed expression if P1-015 supports it, else fallback), and storage cost per step; (4) keybed expression: the chosen gestures and fallback, with MIDI emitted; (5) full ease/learnability scoring (V, I, O, L) for every new function, pass rule as in P1-014; (6) 60-second performance script updated; (7) Flex points.
- Updated consistently, with change notes: ADR-0003 (Step and Channel structs, per-step flags, length, RAM re-added against the 4 KB floor, preset size), the four algorithm docs section 6, ALGORITHM_SET.md, control_map.tsv and CONTROL_MAP.md (check_control_map.py OK), PERFORMANCE_DESIGN.md, UX_REVIEW.md, VISION.md rule 3 wording (owner R1).
- New owner questions appended.

## Acceptance criteria
- [ ] Pattern length per channel specified end to end (range, default, control, per-algorithm behaviour with formulas, change rule)
- [ ] Pagination and page swap specified with on-screen state; white keys always align to the viewed page's steps
- [ ] Per-step performative properties specified with storage cost; RAM re-added, 4 KB floor respected
- [ ] Keybed expression based on P1-015 evidence, with a fallback
- [ ] All new functions pass the ease/learnability rule
- [ ] control_map.tsv/CONTROL_MAP.md updated and check_control_map.py prints OK; ADR-0003 and algorithm docs consistent
- [ ] No 'TBD', 'choose' or 'decide' wording
- [ ] The owner approved it (log line 'Approved: <name> <date>')

## Verification
```sh
python3 tools/check_control_map.py
grep -nEi 'TBD|choose|decide' docs/design/PATTERN_AND_STEPS.md || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Firmware code; flashing.

## Log

### 2026-11-10 orchestrator
Superseded by P1-017 (Opus sequencer + UX lock) after the owner's tenth-round rethink. Dropped.
