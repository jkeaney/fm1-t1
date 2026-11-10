---
id: P2-021
title: TB-3 profile v2: all verified params, select kinds and alignment
phase: P2
type: task
status: done
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Extend the hardware support model with what decision 106 proved on the owner's TB-3 (all 11 CCs work, scatter type is a mode select linked to scatter depth) and design the alignment: how the player chooses which of 11 parameters sit on the 4 wires of a channel (decision 100) in at most 2 gestures. Serves VISION rules 1 and 4 and the owner's emphasis: control the ones we need and want.

## Read first
- docs/project/OWNER_QUESTIONS.md decisions 96-106; docs/design/hardware/HW_SUPPORT_HIGHLEVEL.md, HW_SUPPORT_LOWLEVEL.md (2.1-2.4, 3, 4), TB3_PROFILE.md, tools/hwprof/tb3.tsv
- docs/design/scenarios/S21-rig-device-knobs.md; docs/design/ALGORITHM_SET.md 4 (dice layers); the six algorithm SPECs and docs/design/knob_map.tsv (morph rows)
- docs/design/SEQUENCER_UX.md 8.1 (L-score rubric); tests/design/width_probe.sh

## Deliverable
- HW_SUPPORT_LOWLEVEL.md: profile format v2 (parameter KIND CONT/SEL, LINK, per-parameter lane, named wire sets), SELECT lane and HAND settle, three alignment options with mockups and scores, one recommendation, H-RIG20, owner questions (section 9); the old ENV/TUNE mockups of 4.2 and 4.3 fixed
- TB3_PROFILE.md and tools/hwprof/tb3.tsv: 11 parameters with provenance (level 2), four wire sets, scatter section, dice-layer and morph maps per algorithm family, test sheet text; the ACC-on-slide-destination note in section 4
- Scenario S21 re-walked, new S23; knob_map, control_map, page_states, gen_help.py, help_text.tsv, widths.tsv rows; S22 and COVERAGE regenerated

## Acceptance criteria
- [x] Profile format has parameter kinds (CONT, SEL with N zones), linked parameters, budgeted bytes and static asserts; names at most 5 letters and 38 px
- [x] All 11 CCs are in tb3.tsv with provenance; nothing copied from the midi.guide CSV
- [x] Three alignment options with 240x240 mockups, gesture counts, L-scores and one recommendation
- [x] Default dice-layer and morph maps for ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS; the select-driving rule (HITS picks the type, ACCNT rides the depth)
- [x] An H-test for scatter linkage (H-RIG20) with what to listen for; owner questions with lettered options and a recommendation
- [x] The two P2-020 review notes are fixed
- [x] `check_scenarios.py`, `check_help.py`, `check_sequencer_ux.py`, the width probe and `backlog.py check` pass

## Verification
```sh
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py
python3 tools/check_sequencer_ux.py && sh tests/design/width_probe.sh | tail -1
python3 tools/backlog.py index && python3 tools/backlog.py check
```

## Out of scope
- Firmware, the engine (P4), other devices, tools/gen_hwprof.py, locked decisions 96-106, files of P3-002, P3-014, FB-003, SYSTEM_MAP

## Log

### 2026-11-11 executor (sonnet)
Design done; see HW_SUPPORT_LOWLEVEL.md section 9 and TB3_PROFILE.md sections 2, 3, 12, 13.
- Count note: 7 of the 11 CCs were already in the P2-020 table (vendor chart); they move from [I] to [V-ear]. The NEW entries are 4: CC 1 PAD-Z, 11 PAD-Y, 68 SCTYP, 69 SCDEP (own test only). Provenance level 1 -> 2.
- Format v2: HwParam 10 -> 14 B (kind, link, dcfg, dmove), HwSet 10 B, HwProf 192 -> 316 B; flash +992 B, .bss +0 B (RigRT.rsv reused for the HAND settle timer); static asserts in LOWLEVEL 1.8.
- SELECT: zone centres, one zone per detent, CC sent 150 ms after the last detent, SEQ changes only on a bar line, forms DICE/BAR, MOVE = spread. Linked pair = two ordinary wires side by side; flags :G :R :D stay off until H-RIG20. Unknowns U1-U7 and the test are in LOWLEVEL 9.3 and 9.8.
- Alignment: Option 1 wire sets on ALGO (BITE WET PADS PUSH; 2 gestures; 10), Option 2 SWAP latch on ROW 3 (9, fails, changes a locked row), Option 3 palette and FOLLOW on the RIG page (10 at 2 gestures). Recommended: 1 + 3.
- Tables edited in place (no new controls): knob_map RIG ALGO/K1/K3/K4 and PERF R1-R4, control_map EN_ALGO/K1/K3/K4 on RIG, page_states RIG, gen_help.py + help_text.tsv, widths.tsv (+34 rows), S21 step 2, S22 regenerated, S23 new, COVERAGE regenerated.
- P2-020 review notes fixed: TB3_PROFILE.md section 4 states that the ACC form covers the inaudible slide-accent case; LOWLEVEL 4.2 and 4.3 mockups now show CUT/RESO/ENV-X/ENV-Y.
- Owner questions A-I: LOWLEVEL 9.9 (A sets on ALGO, B stand-in layer at set application, C FOLLOW, D SEL behaviour, E run H-RIG20 before P4-024, F the four sets, G strict gesture count of a swap, H provenance level 2, I ROW 3 SWAP latch). None changes a locked decision.
```
check_scenarios.py -> scenarios: 23 files, 490 control-map rows, 0 with no scenario
check_help.py -> OK: 392 help rows, 2695 strings measured
check_sequencer_ux.py -> OK (10 pages, 490 control rows)
width_probe.sh -> 268 strings, 0 over
```

### 2026-11-11 review (sonnet)
Verdict: PASS on all checkable criteria; gate is human-review, so the task stays at `review`.
- Re-ran check_scenarios (23 files, 0 unwalked), check_help (392 rows OK), check_sequencer_ux (OK), width probe (268 strings, 0 over), backlog index and check (0 errors): all pass.
- tb3.tsv holds 11 CCs with kind/link/layer/form/move, four sets (BITE WET PADS PUSH), names at most 5 letters, provenance level 2; TB3_PROFILE.md states nothing is copied from the midi.guide CSV.
- LOWLEVEL 9.4 has three options with mockups, gesture counts, V/I/O/L scores and a recommendation (1 + 3); 9.8 H-RIG20; 9.9 owner questions A-I with lettered options. 4.2/4.3 mockups now show CUT/RESO/ENV-X/ENV-Y.
- Scope: only P2-021 files in the commit; P3-002/FB-003 left untouched.
- Minor: Option 3 at 3 gestures (R10) exceeds the setup cap and is flagged as owner question G, which is honest.
Awaiting owner: read LOWLEVEL 9.4 mockups and decide owner questions A-I (esp. A sets on ALGO, C FOLLOW, G swap gesture count).

### 2026-11-11 orchestrator
Owner review: partly accepted (3A 4A 7A 8A 9A, 5B). Wire sets on ALGO (1, 2, 6) redirected to the new modulation-layer redesign P2-022 (decision 107). Approved as input: owner 2026-11-11.
