---
id: P2-001
title: Design the global control map v1 (no menu diving)
phase: P2
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P1-001, P1-010, P1-012]
gate: none
---
## Goal
OWNER-APPROVED INPUT: P1-012 already designed and the owner approved the control scheme (docs/design/UX_REVIEW.md, control_map.tsv, 2026-11-10). This task CONSOLIDATES it into the firmware-facing CONTROL_MAP.md; it must not redesign. Any conflict or gap found goes to docs/project/OWNER_QUESTIONS.md as a question and the task stops at `blocked` with the reason in the log.

Assign every button, knob, encoder and key to the vision: channel select, algorithm select, step entry, morph, dice, transport, mute. Every musical function one gesture away, with on-screen labels.

## Read first
- docs/design/UX_REVIEW.md and docs/design/control_map.tsv (P1-012: the owner-directed hardware/UX rethink; build on it, do not redo it)
- docs/project/OWNER_QUESTIONS.md (owner decisions: scale, GATE/DIV/SWING are global; ACID has slide)
- docs/design/CONTROL_INVENTORY.md
- docs/design/ALGORITHM_SET.md
- docs/project/VISION.md (reference workflow)
- docs/controls.jpg

## Deliverable
- docs/design/CONTROL_MAP.md: (1) modes, if any, and how you always know which one you're in; (2) a table Control | Tap | Hold | Combo | Screen label; (3) keys: steps vs channels vs dice; (4) the reference workflow as a gesture-by-gesture walkthrough; (5) what stays in the MENU (settings only); (6) the conflicts resolved from the T-1 guide

## Acceptance criteria
- [x] The reference workflow walkthrough reaches 'performing' without any MENU step
- [x] Every control from CONTROL_INVENTORY has a row (or 'unused, reserved')
- [x] No gesture needs more than one held modifier
- [x] CONTROL_MAP.md agrees with control_map.tsv row for row (a script check is logged); no behaviour differs from the approved UX_REVIEW.md
- [x] Control map is a machine-readable table (docs/design/control_map.tsv or similar) that firmware can load as a const table; a 'Flex points' section lists what is re-mappable by editing a row
- [x] Every row meets VISION rule 1: reachable without menu diving, with its on-screen label

## Verification
```sh
for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "$b" docs/design/CONTROL_MAP.md || echo missing $b; done   # prints nothing
```

## Out of scope
- Screen pixel layout (that's P2-003).

## Log

### 2026-11-10 executor (sonnet)
Consolidated UX_REVIEW.md + control_map.tsv into docs/design/CONTROL_MAP.md (no redesign; tap < 0.4 s, 4 channels, flex points, T-1 guide conflicts). Appendix A repeats all 343 TSV rows; new script tools/check_control_map.py compares every (control, context) action and label and checks each control appears in section 2.
```
$ python3 tools/check_control_map.py
rows tsv=343 md=343
OK
$ for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "$b" docs/design/CONTROL_MAP.md || echo missing $b; done
(no output)
```
Criteria: workflow section 4 reaches performing with no MENU step; all 49 controls have rows (none reserved/unused); only one held modifier per gesture (section 1); layout and rows agree with the TSV (script above); flex points section 7; labels from TSV. No new owner questions (none found).

### 2026-11-10 review (sonnet)
Verdict: PASS (gate none). Re-ran `python3 tools/check_control_map.py`: rows tsv=343 md=343, OK; 14-button grep prints nothing. Spot-checked 40+ TSV rows against CONTROL_MAP.md section 2/3/4 (all encoders and knobs in SCL/CHAN/GLOB/STEP_HELD, SCL/EDIT/GLO/HOME/SAVE/REC/OCT hold and combo rows, first/last white keys, black keys B01/B05/B09/B10/B11): behaviour matches, no redesign. Every combo is one held thing plus a turn or press; tap-vs-hold (0.4 s default via HOLD setting, 0.7 s HOME/SAVE/REC) stated consistently. Reference workflow reaches PERFORM with no MENU step (only EDIT hold). 4 channels, flex points (section 7), and T-1 conflicts (REC no-op, P-LOCK unwired, OCT+ was SHIFT) match CONTROL_INVENTORY. No TBD wording ("Choose an algorithm" is a gesture name).
Fix by reviewer: tools/check_control_map.py section-2 check was a bare substring test over the text before Appendix A; it now searches only section 2 with whole-token matching (so C4 is not satisfied by C#4). Re-run: OK.
