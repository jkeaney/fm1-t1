---
id: P2-019
title: Hardware support model: low-level design (phase 2)
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Phase 2 of owner decision 96: turn the approved hardware support model (HW_SUPPORT_HIGHLEVEL.md, Option B, decisions 97-100) into a buildable low-level design: exact data layouts and budgets, the profile format and its provenance, the engine integration (lanes from the dice seeds, scheduler, ring, rate budget, transport edges), the full screen design for PERF ROW 3 and the RIG page with the owner's three-rows condition, the control-table rows and scenarios, the P4/P5/P6 task breakdown, the H-tests and the open questions. No TB-3 specifics (that is P2-020). Serves VISION rules 1, 3 and 4.

## Read first
- CLAUDE.md, docs/project/PROCESS.md, docs/project/OWNER_QUESTIONS.md (decisions 96-100)
- docs/design/hardware/HW_SUPPORT_HIGHLEVEL.md (approved), docs/design/hardware/README.md, backlog/tasks/P2-018-*.md (Log: owner conditions)
- docs/design/SEQUENCER_UX.md (2, 4.2, 9, 11), SCREEN_SPEC.md (2), TIMING_SPEC.md (M2-M5), ALGORITHM_SET.md (4), algorithm SPECs
- docs/design/control_map.tsv, knob_map.tsv, page_states.tsv, help_text.tsv, key_map.tsv, button_modes.tsv
- tests/design/check_scenarios.py, check_help.py, gen_help.py, gen_sweeps.py, width_probe.sh; tools/check_sequencer_ux.py
- docs/design/hardware/tb3-midi-guide.csv (reference only, CC BY-SA 4.0)

## Deliverable
- docs/design/hardware/HW_SUPPORT_LOWLEVEL.md (sections 1-7 of the owner request; proposals for the P4/P5/P6 tasks inside the document only)
- New RIG rows in control_map.tsv, knob_map.tsv, key_map.tsv, button_modes.tsv, page_states.tsv, help_text.tsv (via gen_help.py); checkers extended to know the RIG page; scenarios S21 (narrative) and S22 (generated sweep); COVERAGE.md regenerated
- docs/design/hardware/README.md index updated; backlog/tasks/P2-020-*.md created (todo)
- No firmware, no tests of firmware

## Acceptance criteria
- [x] HW_SUPPORT_LOWLEVEL.md has sections 1 to 7 (data structures, profile format, engine integration, UX, table rows and scenarios, task breakdown and H-tests, risks and owner questions) and the "Hardware belongs to its owners" statement
- [x] Every struct has exact offsets and sizes with a static-assert list; RAM numbers checked against the .bss and pool budgets
- [x] The three-knob-rows condition is met on screen: persistent row indicator, transition cue, row and page always shown; strings width-probed
- [x] `python3 tests/design/check_scenarios.py`, `python3 tests/design/check_help.py` and `python3 tools/check_sequencer_ux.py` pass
- [x] `python3 tools/backlog.py check` passes
- [x] No locked decision changed; any change needed is an owner question, not applied

## Verification
```sh
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py && python3 tools/check_sequencer_ux.py
python3 tools/backlog.py index && python3 tools/backlog.py check
sh tests/design/width_probe.sh tests/design/widths.tsv
```

## Out of scope
- TB-3 wires, lane mapping, accent and slide behaviour (P2-020), firmware, P3-002, P3-014, FB-003, SYSTEM_MAP

## Log
2026-11-11 sonnet (designer): wrote docs/design/hardware/HW_SUPPORT_LOWLEVEL.md (sections 1-8) and updated hardware/README.md. Added the RIG page (tenth page) to the design tables: page_states +1, control_map +49, button_modes +28, knob_map +11, key_map +32, help_text +49 (gen_help.py); PERF rows for OCT-/OCT+/K1..K4/CHN amended (ROW 3 appended, locked text kept). Extended check_help.py, check_scenarios.py, gen_help.py, gen_sweeps.py for RIG; scenarios S21 (hand-written, 14 steps) and S22 (generated sweep); S09 and COVERAGE.md regenerated; 74 width-probed strings added to tests/design/widths.tsv. Created P2-020 (TB-3 profile and alignment, todo).
Findings: Rig stored as a sibling RigSave in SeqPreset v4 (ChanSave and UndoRec asserts unchanged); new .bss 274 B (HL said 184); pool +2016 B does not fit before R6 (1380 B short; SQ_NSLOT=8 escape); the `~` suffix on dice sub-labels does not fit (button row 2 would need 247 of 240 px) so a ride dot replaces it; CC lead reduced to 2 ahead + the rest after the note because DIN needs 0.96 ms per CC; ring needs bounded out-of-order insertion (P4-028). No locked decision changed; 8 owner questions A..H (all refinements, recommendation A in each).
Verification: check_scenarios.py (22 files, 490 rows, 0 gaps), check_help.py OK (392 rows), tools/check_sequencer_ux.py OK (10 pages), width_probe.sh widths.tsv 236 strings 0 over, backlog.py check 0 errors.
Awaiting: owner human-review ("Approved: <name> <date>"); read HW_SUPPORT_LOWLEVEL.md sections 4 and 7.

### 2026-11-11 review (sonnet)
Verdict: PASS on the checkable criteria; gate is human-review, so the task stays at `review`.
- Re-ran: check_scenarios.py (22 files, 490 rows, 0 without scenario), check_help.py OK (392 rows, 2695 strings), check_sequencer_ux.py OK (10 pages), backlog.py check 115 items 0 errors, width_probe.sh widths.tsv 236 strings 0 over. build/ and check.sh not run, as instructed. Commit touches only docs, design tests and backlog; no firmware.
- Sections 1-7 present (plus 8 checks). Byte arithmetic re-added: .bss 112+128+26+8 = 274; pool 1508 x 18 = 27144 (+2016), shortfall before R6 disclosed with the SQ_NSLOT=8 escape. Static-assert list (1.8) covers Wire/Rig/RigSave/RigRT/Hw* and pins ChanSave/UndoRec unchanged.
- Owner condition 1 (three knob rows): met in 4.1 - persistent rail with `ROW n/3`, live row 26 px vs 13 px others (geometry tiles Z3), 150 ms flash plus 2 s toast on change, standing cue, and a row/page table for every screen; H-RIG16 human readability test added.
- Owner condition 2 (hardware belongs to its users, provenance): met in 2.3 - the statement is written out, generator rules enforce provenance per profile, the CSV is never an input, midi.guide is cross-reference only, credits shown in MENU > INFO.
- Minor, not blocking: the proposed layout puts a lot in a 36 px rail and 13 px lines; legibility on the real panel is only provable on hardware (covered by H-RIG16).
Awaiting owner: read HW_SUPPORT_LOWLEVEL.md sections 4 and 7 and answer owner questions A..H ("Approved: <name> <date>").
