---
id: P1-012
title: UX unification: hardware controls x algorithms x workflow, with an ease score
phase: P1
type: spike
status: done
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
- [x] Every one of the 14 buttons, K1-K4, 3 encoders and the 27 keys has a role in every context, with no control left unassigned and no two roles conflicting in one context
- [x] No function requires a menu dive; hold-HOME menu is only for set-and-forget settings
- [x] Rubric defined with the 0-3 definitions above; every workflow step has a current and a proposed score; every performance function passes (>= 8, no sub-score below 2) or is listed as an unavoidable failure with a reason
- [x] Each v1 algorithm's four morphs map to K1-K4 and its dice layers map to the shared dice buttons, labels at most 6 characters
- [x] ONE global scale and key, settable from the panel, used by all four algorithms (decision S5)
- [x] S1-S4 and S6 each have a stated resolution with score evidence
- [x] docs/design/control_map.tsv exists and agrees with UX_REVIEW.md
- [x] No 'TBD', 'choose' or 'decide' wording in the deliverables
- [x] The owner approved it (log line 'Approved: <name> <date>'), which also covers re-approving P1-010

## Verification
```sh
test -f docs/design/UX_REVIEW.md && test -f docs/design/control_map.tsv && echo ok
grep -nEi 'TBD|choose|decide' docs/design/UX_REVIEW.md docs/design/control_map.tsv || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Writing firmware code; flashing; changing the algorithm procedures themselves (only their knob/dice mappings and labels).

## Log

### 2026-11-10 executor (opus)
Design only (no firmware, no flash, no push; `tools/check.sh` and the test suite not run because no code changed).
Wrote docs/design/UX_REVIEW.md and docs/design/control_map.tsv; amended docs/design/ALGORITHM_SET.md (change note at
top); appended owner questions 31-36 to docs/project/OWNER_QUESTIONS.md.

Verification:
```
$ test -f docs/design/UX_REVIEW.md && test -f docs/design/control_map.tsv && echo ok
ok
$ grep -nEi 'TBD|choose|decide' docs/design/UX_REVIEW.md docs/design/control_map.tsv || echo clean
clean
$ python3 tools/backlog.py check
31 items, 0 errors
```
Extra checks (python): control_map.tsv = 343 rows, 49 controls (14 buttons, 3 encoders, MASTER, K1-K4, 27 keys) x 7
contexts, no empty field; the 1.3 table and the TSV were emitted by one generator, so they agree. Score table
arithmetic re-computed: 30 functions, current 84/270 (26 fail), proposed 251/270 (0 fail); every performance function
>= 8 with no sub-score below 2.

Key design: SELECT = channel, ALGORITHM = algorithm, PRESETS = preset slot; K1-K4 = DENSITY/FORM/PITCH/SPICE morphs;
dice SCL tap NOTES, SEQ HITS, ENV ACCNT, ARP ARTIC, LFO SHAPE, FX ALL, EDIT tap UNDO/REDO; holds: SCL = global
KEY/SCALE/OCTAVE, EDIT = MIDI channel of CH1-4, GLO = GATE/DIV/SWING/BPM, HOME = menu; REC = step entry (white key =
step, hold + K3 = pitch); black keys mute/select/all-on/restart/tap. S1 one press GATE+SLIDE; S2 keep SHAPE + per-
algorithm sub-labels; S3 keep slots, ACID TYPE relabelled PEDAL; S4 SP6 on everything incl. pins and live keys; S6
confirmed.

Reviewer attention: (1) K1 ranges now start at 0 (`KEYS ONLY`), a range edit on all four algorithms. (2) 4 channels
assumed (question 31). (3) Felucca groovebox kept only as MENU > SYSTEM > MODE GROOVE (question 32). (4) Menu rows
removed (TCH1-4, SELECT, BPM LOCK, T1 SEQ -> MODE) will change tests/ui_test.c row counts in the dev task.
Owner-approval criterion left unticked: no 'Approved:' line written (human-review gate).


### 2026-11-10 review (sonnet)
Verdict: **PASS on content; gate human-review, so left at `review`. Awaiting owner: approve P1-012 (also re-approves P1-010) and answer the decisions below.** No Approved line written.

Checked independently:
- TSV script: 343 rows, 49 controls x 7 contexts, all 14 buttons, K1-K4 (as EN_K1..4), 3 encoders, MASTER, 27 keys present; no duplicate (control, context); no empty fields; no TBD/choose/decide (one stale "decide" in ALGORITHM_SET Q22 fixed). Labels over 6 chars exist only for non-knob, non-dice cells (HOME, PLAY/STOP, `+sub` markers); knob and sub-labels are all <= 6.
- Physical names match panel.c B_NAME/E_NAME and CONTROL_INVENTORY.md (ALGORITHM was the track selector; now algorithm).
- Score arithmetic: all 60 V/I/O totals re-added correctly; sums 84 and 251 correct as submitted; pass rule applied mechanically correctly.
- Workflow W1-W15 and the VISION TB-3 workflow are executable with the stated controls.
Fixes made by reviewer:
- Score inflation: F20 (mute) and F26 (audition) had visual 3, yet the black/white-key roles are only labelled on the KEYS legend (the same reason F28 was given 2). Both now 2/3/3 = 8 (still pass); proposed total 251 -> 249/270; TSV score column updated; R9 text said the channel strip shows each channel's key, which section 1.4 does not say, changed to "KEYS legend".
- ALGORITHM_SET toast text aligned to UX_REVIEW (`NO SHAPE IN EUCLID`); Q22 "decide" reworded.
Residual concerns for the owner (not blockers):
- Dice on printed FX/ENV/LFO/ARP/SEQ/SCL: mnemonics are decent for SCL=NOTES, SEQ=HITS, ENV=ACCNT, weaker for ARP=ARTIC and LFO=SHAPE; learnable only because the 2x6 on-screen button picture labels each. Visual 3 on that band is plausible (240/6 = 40 px cells) but unproven until a mock-up (P2-003). SCL and EDIT taps act on release and must be under 0.4 s or they open a layer instead.
- Lost in SEQ mode (kept only in MENU > SYSTEM > MODE GROOVE, Q32): Felucca ARP, SEQ tools (EUCLID/RANDOM/COOK), SCL CHORD/VOICING, GLO SONG and SOLO, FX performance effects, ENV/LFO sound editing, LFO-held MOMENTARY (Q36). Consistent with VISION rule 2 but UX_REVIEW does not list them all; Q32 should state this.
- Q31-Q36 are well formed; none is blocking for the doc itself.

### 2026-11-10 owner
Approved: James Keaney 2026-11-10 (P1-010 and P1-012 approved together; answers to the open points recorded in docs/project/OWNER_QUESTIONS.md).
