---
id: P4-048
title: PERF ROW 3 knobs R1-R4: live level of each wire, OCT row switching, toasts
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-047, P4-043]
gate: human-review
---
## Goal
Wire ROW 3 on the PERF page: with a device on the selected channel, K1-K4 on ROW 3 (R1-R4 in the tables) are the live LEVEL of wires 1-4, drawn from the profile data, reachable by the OCT row switch only when the channel's device has wires. A HAND wire sends its CC at once. The screen code has no per-device and no per-algorithm branch (ADR-0007).

Owner ruling 2026-11-13: FULL RIG is in v1, ROW 3 knobs R1-R4 wired, TB-3 the first complete device.

## Read first
- docs/design/hardware/RIG_MOD_V1.md, ADR-0008 and docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md (Gate A, owner-approved mocks)
- docs/design/SCREEN_SPEC_V2.md 4.2 (ROW 3 tile line), 4.3 (`ROW` boxes, 2 or 3), 5 (messages), 7 (state list incl. `row3_rig`)
- docs/design/knob_map.tsv PERF R1-R4 (as rewritten by P4-045 decisions) and the K1-K4 note "on ROW 3"; button_modes.tsv OCT-/OCT+ rows; help_text.tsv, control_map.tsv
- firmware/src/ui_sq.c: row model and `uisq_rig_row` (~250, returns 0 today), OCT handling (~2068-2072), the knob tile drawing, toasts; ui_sq_glue.c ~44, 83 (OCT acts on release today)
- firmware/src/sq_hw.c (P4-046) and the slot API of P4-047 (`sq_slot_set`, `hw_scale`)
- docs/design/ui_review/proposals/v12/v12.py (`rig_row`, `row3_rig`), tests/v12_compare.py, tests/v12_export.py, tests/ui_test.c
- docs/project/UX_GATE.md; backlog/tasks/P4-040-*.md as a model for a screen task with pixel compare

## Deliverable
- Row model: ROW 3 exists only when the selected channel's device has at least one wire (a table read from the profile, `hw_prof(...)`), so OCT+ walks ROW 1 -> ROW 2 -> ROW 3 -> ROW 1 only then; with no device it stays 2 rows. Rail `ROW n/3` per the SCREEN_SPEC_V2 4.3 change made in this task, the live row filled, the toast `ROW 3 OF 3: <device>` / `OCT+ ROW 3 TB-3` (text per the approved mock)
- R1-R4 turn: sets the LEVEL of the wire in that column; a HAND wire (DEPTH 0) sends the CC at once through P4-047's rate limit (<= 1 CC per 10 ms per wire, last value wins); a SEQ wire (DEPTH above 0) moves the centre of its lane from the next step; a select parameter steps one zone per detent and sends 150 ms after the last detent (only if the P4-045 design keeps selects on ROW 3); knob tiles show the profile's short name over the value, with the driven / HAND mark from the approved mock (graphics, no extra text); wires beyond the device's count are blank
- Selection switching: a channel change (SELECT) or a device change redraws ROW 3 from that channel's slots; a channel with no device falls back to ROW 1 with the toast `NO DEVICE` only if it was on ROW 3
- Dice integration: the SHAPE dice toast adds `+ MOD` when a wire is driven (P4-047 behaviour), per the approved wording and the 136 px message slot
- Tables updated to match (knob_map, button_modes, control_map, help_text via `tests/design/gen_help.py`, key_map if touched), SCREEN_SPEC_V2 4.2 / 4.3 / 7 edited (the `row3_rig` scene now comes from profile data), `tests/design/widths.tsv` rows added, scenarios regenerated or edited so `check_scenarios.py` shows 0 gaps; `docs/design/forbidden_terms.txt` gets the old `WIRE1..4` / HAND-SEQ wording if it is now stale
- v12 mock scenes for ROW 3 (HAND wire, driven wire, no device, device with 2 wires) added to v12.py and exported; host render equals the mock pixel for pixel; the existing 52 scenes still identical
- Host tests in `tests/ui_test.c` (or `ui_sq_row3_test.c`): row walk with and without a device, R1-R4 set LEVEL, HAND sends one CC (check the MIDI queue), rate limit under a fast sweep, channel switch, no-device fallback; a second (dummy) device with 2 wires in the test shows 2 wires and no code change
- Grep proof in the Log: no device name, no CC number and no algorithm name in `ui_sq.c` / `ui_sq_glue.c` for this feature

## Acceptance criteria
- [ ] ROW 3 appears only for a channel whose device has wires; the OCT walk is correct in both cases (host test)
- [ ] R1-R4 turn sets the LEVEL; a HAND wire sends its CC at once, at most one per 10 ms per wire, the final value always sent (host test on the MIDI queue)
- [ ] `python3 tests/v12_compare.py`: existing scenes unchanged, new ROW 3 scenes pass
- [ ] check_scenarios / check_help / check_sequencer_ux / width probe / `sh tools/check_docs.sh` all pass; control rows for R1-R4 are no longer in the `check_control_map.py` baseline of failures (the baseline must not grow)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)
- [ ] Owner approves the ROW 3 screens on the unit or on the renders (human-review): `Approved: <name> <date>`

## Verification
```sh
sh tools/check.sh                    # last line: CHECK PASS
./build.sh | grep 'RAM'              # paste into the Log
python3 tests/v12_compare.py         # all scenes pass
python3 tools/check_control_map.py   # baseline must not grow
```

## UX gate
Producer-facing. Gate A: `docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md` (steps: OCT+ OCT+ then turn R1, gesture counts). Gate B after the build: the live checklist on renders now, on the unit in P4-050. Update the PERF use case `docs/design/features/P4-013-*.md` with the ROW 3 rows.

## Out of scope
- The RIG page and device / lane / form / depth setup (P4-049)
- The MOD engine itself (P4-047)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created. Owner ruling 2026-11-13: "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."
- 2026-11-13 planner: P4-043 (OCT peek) is a dependency because both tasks change the OCT row handling in ui_sq.c / ui_sq_glue.c; one agent builds at a time.
