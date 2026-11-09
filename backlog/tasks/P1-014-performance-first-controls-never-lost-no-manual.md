---
id: P1-014
title: Performance-first control design: every control accounted for, fun to play, never lost, no manual
phase: P1
type: spike
status: review
priority: 0
executor: sonnet
depends_on: [P1-013]
gate: human-review
---
## Goal
Owner challenge (2026-11-10, eighth round): design the device for PERFORMANCE. All knobs accounted for; the panel buttons align to the algorithm controls; the keyboard aligns to performance features and step controls; everything inside the UX guidelines and proven with the ease score. It must be controllable, fun to play, and a user must NEVER feel lost; they must be able to play it without a manual. Includes the per-morph reset gesture (owner question 49, now decided: design it now).

## Read first
- docs/project/VISION.md; docs/project/OWNER_QUESTIONS.md (all decisions, esp. 'eighth round' and the sixth-round shift decision; question 36 momentary morph; question 49)
- docs/project/decisions/ADR-0004-design-lock.md (data-table flexibility)
- docs/design/FULL_CONTROL.md, UX_REVIEW.md (rubric and scores), CONTROL_MAP.md, control_map.tsv, tools/check_control_map.py, ALGORITHM_SET.md and section 6 of the four algorithm docs
- docs/project/decisions/ADR-0003-sequencer-core.md (state and RAM budget: any new feature needs state; respect the 4 KB RAM floor and the SYSTEM_MAP headroom)
- docs/design/SYSTEM_MAP.md; docs/design/CONTROL_INVENTORY.md; firmware/src/panel.c; docs/controls.jpg (what is printed on the panel)

## Deliverable
- docs/design/PERFORMANCE_DESIGN.md containing:
  1. **Control accounting table**: every physical control (14 buttons, K1-K4, 3 encoders, 27 keys, plus chosen hold/tap variants) with its role in PERFORM, and the SINGLE principle that explains the layout (e.g. panel buttons = algorithm controls; white keys = steps; black keys = performance moves). No control may be dead or ambiguous in PERFORM; justify any intentional blank.
  2. **Panel buttons aligned to the algorithm controls**: the dice buttons, row buttons, undo/redo, reset gesture, algorithm functions; one consistent mapping between a button and what it does to the algorithm in every algorithm.
  3. **Keyboard**: which keys are step controls (entry, pin, tie, per-step accent/slide, clear) and which are performance features (e.g. mute channels, fills/rolls, momentary mutate that snaps back, scene/snapshot recall, tap tempo, octave jumps, restart); a keys legend always visible on screen; behaviour while stopped vs playing.
  4. **Reset gesture** for a single morph and for a whole row/algorithm (owner Q49), plus the momentary-morph gesture (Q36) if the score says it earns a place.
  5. **'Never lost / no manual' design**: the always-on cue strip (what each live control does right now), first-run guidance at power-on (shown once, dismissible with any control), a consistent escape (one gesture returns to the main PERFORM screen from anywhere), undo reachable everywhere, a 'what does this do' touch-to-learn behaviour (e.g. holding any control briefly shows its function without changing anything, if the score says it is worth it), naming and iconography rules (labels <= 6 chars, consistent verbs). List every state a first-time user could get confused in and the cue that rescues them.
  6. **Fun to play**: the performance moves list (fills, rolls, mutate-hold, scene recall, build/drop) with the gesture for each, and the sequence of a 60-second live performance as a gesture-by-gesture script.
  7. **Score**: extend the rubric of UX_REVIEW.md with a fourth sub-score **L = learnable without a manual** (0 = needs the manual; 1 = guessable only from other controls; 2 = role visible on screen but must be discovered; 3 = role obvious from the label/position and the cue strip). Score every function (current approved scheme vs proposed) on visual/input/output/L. Pass rule: performance functions need V,I,O,L totals >= 11/12 with no sub-score below 2; setup functions >= 9/12 with no 0. Show before/after.
  8. **Three candidate schemes** (different organising principles), each scored, with the winner chosen by score and fun; the losers kept in an appendix with why they lost.
  9. **Change list**: every difference from the approved P1-013 scheme, each with the score evidence; anything that touches RAM/flash with the cost from ADR-0003/SYSTEM_MAP.
  10. Flex points (ADR-0004): what is re-mappable by a table edit.
- Update to match: docs/design/control_map.tsv, CONTROL_MAP.md (run python3 tools/check_control_map.py to OK), FULL_CONTROL.md, UX_REVIEW.md (new rubric column and scores), ALGORITHM_SET.md and ADR-0003 where state or vtable changes, each with a change note at the top.
- New owner questions in docs/project/OWNER_QUESTIONS.md (continue numbering).

## Acceptance criteria
- [x] Every physical control has a defined role in PERFORM; none dead or ambiguous; one explaining principle
- [x] Panel buttons map consistently onto algorithm controls in all four algorithms
- [x] Keys split into step controls and performance features with an on-screen keys legend
- [x] A per-morph reset gesture and a row/algorithm reset exist and score to the pass rule
- [x] The extended L sub-score is defined; all performance functions pass (>= 11/12, no sub-score below 2)
- [x] Three schemes scored; the winner is chosen by score; changes from the approved scheme are listed with evidence
- [x] A first-run flow, a consistent escape gesture and a list of confusing states with rescuing cues
- [x] A 60-second performance script that uses only the documented gestures
- [x] control_map.tsv and CONTROL_MAP.md updated; check_control_map.py prints OK; ADR-0003 updated if state changes, RAM budget re-added with the 4 KB floor respected
- [x] No 'TBD', 'choose' or 'decide' wording
- [ ] The owner approved it (log line 'Approved: <name> <date>')

## Verification
```sh
python3 tools/check_control_map.py
grep -nEi 'TBD|choose|decide' docs/design/PERFORMANCE_DESIGN.md || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Firmware code; flashing.

## Log

### 2026-11-10 executor (sonnet)
Design only. Files: docs/design/PERFORMANCE_DESIGN.md (new, 11 sections), control_map.tsv (245 rows = 49 controls x 5 contexts), CONTROL_MAP.md (regenerated), FULL_CONTROL.md, UX_REVIEW.md (new 1.2/1.3, section 3.3 L), ALGORITHM_SET.md, ADR-0003 (UndoRec 48 B, section 11.1), OWNER_QUESTIONS.md (53-57; 49 answered).
Winner: scheme B "Rows" (top row = dice in layer order, bottom row = safety/system, white keys = steps always, black keys = moves). Scores (21 compared functions, /252): A 172 (13 fail), B 234 (0 fail), C 211 (11 fail). Full table (53 functions): approved P1-013 386 of 636, fails 24; proposed 570 of 624, all 52 live functions pass (P >= 11 no sub < 2; S >= 9 no 0).
Ninth-round R1-R4 added at the control level only (PAGE key D#4, LENGTH via hold PAGE + key and SET row 2 K4, step band with two rows of four per-step properties, [R4] keybed slot); P1-016 fills data/algorithm side.
RAM: ADR-0003 11.1: total new .bss 6953 B (incl. 1536 B reserved for P1-016 step records); headroom 39543 B after removals, 11031 B before (> 8 KB trigger, floor 4 KB). Pool about 22400 B (3364 B spare today: removals must precede preset storage).
Verification:
```
python3 tools/check_control_map.py   -> rows tsv=245 md=245 / OK
grep -nEi 'TBD|choose|decide' docs/design/PERFORMANCE_DESIGN.md || echo clean   -> clean
python3 tools/backlog.py check   -> 37 items, 0 errors
```
Owner questions: 53-57 (OWNER_QUESTIONS.md). No Approved line written (human-review gate). The last acceptance box (owner approval) stays unticked.

