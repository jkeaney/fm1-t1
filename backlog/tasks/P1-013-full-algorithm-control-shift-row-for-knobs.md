---
id: P1-013
title: Full algorithm control: a shift row doubles the knobs (up to 8 morphs per algorithm)
phase: P1
type: spike
status: review
priority: 0
executor: sonnet
depends_on: [P1-012, P2-001, P2-002]
gate: human-review
---
## Goal
Owner decision (2026-11-10, 'Shift'): algorithms need full control. Add a SHIFT for the knobs so K1-K4 give two rows of four (up to 8 morphs per algorithm), with the screen instructing the change live. Make access easy: the producer must always see which row is live, what each of the eight knobs does, and be able to reach the second row in one gesture. Bring back from the source algorithms (docs/sting-algorithms, snake, ml185, t1-algorithms) every parameter that gives real musical control and was cut only because of the 4-knob cap.

## Read first
- docs/project/OWNER_QUESTIONS.md ('Decision ... sixth round: full algorithm control' and all earlier decisions)
- docs/project/VISION.md; docs/project/decisions/ADR-0004-design-lock.md (data-table flexibility)
- docs/design/UX_REVIEW.md (rubric, scores, section 'Algorithm alignment'), docs/design/CONTROL_MAP.md, docs/design/control_map.tsv, tools/check_control_map.py
- docs/design/ALGORITHM_SET.md and the four v1 algorithm docs docs/design/algorithms/{sting,snake,ml185,t1}.md, section 6 of each, plus the source specs in docs/*-algorithms/ for what was cut
- docs/project/decisions/ADR-0003-sequencer-core.md (DRAFT under review: the morph array, per-algorithm saved state, RAM table)
- firmware/src/panel.c (which controls exist and how they are printed)

## Deliverable
- docs/design/FULL_CONTROL.md: (1) the SHIFT control: pick ONE existing control (check control_map.tsv for conflicts: a free or low-value button, or a held-state of one) and justify it with the ease rubric; define hold-to-shift vs tap-to-latch (and what happens if shift is held while turning), how it relates to the existing holds; (2) the live-screen rule: both label rows always visible with the live row highlighted, the exact label text zone and character limits (<= 6 chars per label), a toast/indicator while shifted, value readout on turn; (3) per algorithm (ACID, EUCLID, SNAKE, STAGE) a table of up to 8 morphs: slot (K1-K4 / SHIFT+K1-K4), label, parameter, range, default, step per detent, and which source parameter it restores (cite doc + section); explicitly list parameters that stay cut and why; keep the K1 density / K2 form / K3 pitch width / K4 spice convention for the base row and define a matching convention for the shift row (e.g. timing/feel, articulation, variation); (4) ease-score rows for the shift access and for turning each of the 8 knobs (visual/input/output 0-3, pass rule: performance >= 8 with no sub-score < 2), current (4-knob) vs proposed; (5) the knob value-change rule for relative detent knobs so 8 morphs stay precise (step sizes, acceleration) and that morph values are kept per algorithm when switching (decision S7); (6) Flex points (ADR-0004).
- Update to match: docs/design/control_map.tsv and CONTROL_MAP.md (+ run python3 tools/check_control_map.py to OK), UX_REVIEW.md (scores, workflow steps that use knobs), ALGORITHM_SET.md (knob convention, layer vocabulary unchanged unless needed), section 6 of each of the four algorithm docs (morph tables and any new procedure steps with exact formulae and defaults, so a prototype can reproduce them), and ADR-0003 (morph array size, per-algorithm saved-state size, RAM table re-added, vtable) with a change note at the top of each edited file.
- New owner questions appended to docs/project/OWNER_QUESTIONS.md (continue numbering).

## Acceptance criteria
- [x] ONE shift control chosen with no conflicting role in any context, scored >= 8 with no sub-score below 2
- [x] The live-screen rule shows both rows and the live row at all times; every label <= 6 chars
- [x] Each of the four algorithms has up to 8 morphs fully specified (range, default, step, formula) and a list of restored vs still-cut source parameters with reasons
- [x] Every performance knob function scores >= 8 with no sub-score below 2
- [x] control_map.tsv and CONTROL_MAP.md updated; check_control_map.py prints OK
- [x] ADR-0003 and the four algorithm docs updated consistently (morph counts, RAM)
- [x] No 'TBD', 'choose' or 'decide' wording
- [ ] The owner approved it (log line 'Approved: <name> <date>'); this also covers re-approving P2-004

## Verification
```sh
python3 tools/check_control_map.py
grep -nEi 'TBD|choose|decide' docs/design/FULL_CONTROL.md || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Firmware code; flashing.

## Log

### 2026-11-10 executor (sonnet)
Design: docs/design/FULL_CONTROL.md. Shift = OCT- (ROW 1) / OCT+ (ROW 2): tap latches, hold and turn is momentary; the keyboard octave moved to SCL held + OCT-/OCT+. Scores F30 = 9, F31-F34 = 8 (3/2/3), F35 = 7; all 32 knob functions >= 8, no sub-score < 2. Two knob rows (8 morphs) for ACID, EUCLID, SNAKE, STAGE; every ROW 2 default is neutral so base-row goldens are unchanged. Edited: control_map.tsv, CONTROL_MAP.md, UX_REVIEW.md, ALGORITHM_SET.md (+SP16, SP17), sting/t1/snake/ml185.md section 6, ADR-0003 (SQ_NMORPH 8, AlgoState 20 B, ChanSave 102 B, SeqPreset 420 B, RAM table: .bss +5329 B, pool +7560 B; seventh-round decisions 43-47 applied, bar = 768 ticks).
Verification:
- `python3 tools/check_control_map.py` -> `rows tsv=343 md=343` / `OK`
- `grep -nEi 'TBD|choose|decide' docs/design/FULL_CONTROL.md || echo clean` -> `clean`
- `python3 tools/backlog.py check` -> `34 items, 0 errors`
Owner questions 48-52 added to OWNER_QUESTIONS.md. Last criterion (owner approval) left unticked: no Approved line written.
