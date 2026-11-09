---
id: P1-012
title: UX unification: hardware controls x algorithms x workflow, with an ease score
phase: P1
type: spike
status: todo
priority: 0
executor: opus
depends_on: [P1-001, P1-011]
gate: human-review
---
## Goal
A whole rethink of how the hardware buttons, knobs, encoders and keys drive the algorithms, so the producer can select, morph, tune and dice an algorithm easily and buttons take them to the right page or own the sequence. Unite the UX, the algorithms and the hardware in one design, and prove it with a score (owner directive 2026-11-10). Short, focused pass: aim to finish in about 30 minutes.

## Read first
- docs/project/VISION.md (rules 1-4, the TB-3 workflow)
- docs/project/OWNER_QUESTIONS.md (all decisions, esp. 'Decisions 2026-11-10': S1-S7)
- docs/project/decisions/ADR-0004-design-lock.md (incl. flexibility addendum: control map, layers, morph and dice mappings are data tables)
- docs/design/CONTROL_INVENTORY.md (every control today, with file:line)
- docs/design/ALGORITHM_SET.md (the P1-010 draft: 4 algorithms ACID/EUCLID/SNAKE/STAGE, 5 shared dice layers NOTES/HITS/ACCNT/ARTIC/SHAPE, knob convention K1 density K2 form K3 pitch width K4 spice)
- docs/design/algorithms/*.md (sections 6: each algorithm's morphs and dice layers)
- firmware/src/panel.c (physical controls: B_FX B_SCL B_ENV B_LFO B_EDIT B_GLO B_HOME B_SAVE B_ARP B_SEQ B_PLAY B_REC B_OCTDN B_OCTUP, K1-K4, SELECT/ALGO/PRESET encoders, 27 keys; hold HOME opens the menu)

## Deliverable
- docs/design/UX_REVIEW.md containing, in order:
  1. **Hardware map**: every physical control (14 buttons, K1-K4, 3 encoders, 27 keys, hold-HOME) and the physical labels printed on the panel, with the role each should have in each context (channel select, step entry, perform, algorithm edit, global). Decide which encoder selects which thing (channel, algorithm, preset/scale), and make every knob's role match its position and any panel printing. Buttons either take the user to a page or own a part of the sequence (e.g. dice a layer); state which for each.
  2. **Workflows** as numbered step lists with the exact controls pressed: select channel 1; set its MIDI channel; choose an algorithm; enter steps on the 16 white keys; dial a sequence in with knobs; morph; dice each layer; tune the algorithm; set the global scale and key (decision S5: ONE global scale + key on the device, reachable from the panel without a menu dive, used by every algorithm); mute/switch; save/load.
  3. **Ease score**: a rubric and the scores. For each workflow step or musical function score 0-3 on each of: (a) VISUAL CUE: is the control's current role shown on screen where the eye already is (0 not shown, 1 shown on another page, 2 shown but small/ambiguous, 3 labelled on the main screen); (b) INPUT EFFORT: gestures needed (0 menu dive or hidden combo, 1 modifier+control or more than 2 presses, 2 two actions, 3 one direct control); (c) OUTPUT FEEDBACK: what the user sees/hears after (0 none, 1 delayed over 1 step or not shown, 2 audible next step, 3 audible within one step AND the screen shows the new value). Total /9 per function. Pass rule: performance functions (morph, dice, mute, select algorithm) need >= 8 with no 0 or 1 sub-score; setup functions need >= 6 with no 0. Score the CURRENT firmware per CONTROL_INVENTORY and the PROPOSED design in two columns, and show the delta.
  4. **Redesign** wherever a function fails the pass rule, with the new control assignment and the re-score. Iterate until all pass or list the unavoidable failures with a reason.
  5. **Algorithm alignment**: a table mapping each of the 4 v1 algorithms' morphs and dice layers onto the physical K1-K4 and dice buttons, with the on-screen label (at most 6 characters) and score. Resolve the open items by evidence from the score: S1 slide on ARTIC, S2 the SHAPE button, S3 the two stretched knob fits (ACID TYPE on K2, SNAKE PATH on K4), S4 scale snap rule, S6 defaults (minor pentatonic; SNAKE RANGE default 12). State which algorithm doc edits follow.
  6. **Global scale and key**: where it is set on the device, its default, how each algorithm adheres (root and scale mask feed the pitch layer of every algorithm), and how the screen always shows it.
  7. **Flex points** (per ADR-0004): what can be re-mapped by editing a table row.
- docs/design/control_map.tsv: the proposed map as a machine-readable table, one row per (control, context): control, context, action, on-screen label, page-or-owns-sequence, score.
- docs/design/ALGORITHM_SET.md amended to match (layer vocabulary, knob convention, edit table), with a change note at the top.
- Add any new owner questions to docs/project/OWNER_QUESTIONS.md (continue numbering).

## Acceptance criteria
- [ ] Every one of the 14 buttons, K1-K4, 3 encoders and the 27 keys has a role in every context, with no control left unassigned and no two roles conflicting in one context
- [ ] No function requires a menu dive; hold-HOME menu is only for set-and-forget settings
- [ ] Rubric defined with the 0-3 definitions above; every workflow step has a current and a proposed score; every performance function passes (>= 8, no sub-score below 2) or is listed as an unavoidable failure with a reason
- [ ] Each v1 algorithm's four morphs map to K1-K4 and its dice layers map to the shared dice buttons, labels at most 6 characters
- [ ] ONE global scale and key, settable from the panel, used by all four algorithms (decision S5)
- [ ] S1-S4 and S6 each have a stated resolution with score evidence
- [ ] docs/design/control_map.tsv exists and agrees with UX_REVIEW.md
- [ ] No 'TBD', 'choose' or 'decide' wording in the deliverables
- [ ] The owner approved it (log line 'Approved: <name> <date>'), which also covers re-approving P1-010

## Verification
```sh
test -f docs/design/UX_REVIEW.md && test -f docs/design/control_map.tsv && echo ok
grep -nEi 'TBD|choose|decide' docs/design/UX_REVIEW.md docs/design/control_map.tsv || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Writing firmware code; flashing; changing the algorithm procedures themselves (only their knob/dice mappings and labels).

## Log
