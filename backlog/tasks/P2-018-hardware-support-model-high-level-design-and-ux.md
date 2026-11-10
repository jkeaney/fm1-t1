---
id: P2-018
title: Hardware support model: high-level design and UX (phase 1)
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Owner decision 96 (2026-11-11): each sequenced channel gets a hardware support layer so the algorithms' dice and morph controls can drive a target device's own parameters (filter, resonance, accent amount ...) as well as notes, and the user can see and set in two gestures what the sequencer controls and what it leaves alone. Phase 1 is the high-level model and UX only, for owner approval (VISION rules 1, 3 and 4). The low-level design and the TB-3 specifics are separate, later tasks and are not started here.

## Read first
- CLAUDE.md, docs/project/VISION.md, docs/project/PROCESS.md (Definition of Ready and Done)
- docs/design/SEQUENCER_UX.md (sections 1, 2, 8.1 rubric, 9 data model and budget), docs/design/ALGORITHM_SET.md (section 4 layers), docs/design/HARMONY.md
- docs/design/algorithms/arp/SPEC.md, docs/design/algorithms/patterns/SPEC.md, docs/design/KEYBED_USE_CASES.md (section 3: why pressure to CC was dropped), docs/design/SCREEN_SPEC.md
- docs/design/control_map.tsv, knob_map.tsv, key_map.tsv, page_states.tsv (locked controls)
- docs/design/hardware/tb3-midi-guide.csv (data from midi.guide, CC BY-SA 4.0)
- docs/design/TIMING_SPEC.md (out ring, reserve rule), docs/project/OWNER_QUESTIONS.md (decision 96)
- https://midi.guide/ and https://cdm.link/aira-secrets-get-midi-control-sequence-externally-download-free-ableton-m4l-patches/

## Deliverable
- docs/design/hardware/HW_SUPPORT_HIGHLEVEL.md (problem, model, dice/morph mapping, data model with byte costs, profile sources and licensing, three UX options with mock-ups and L-scores, recommendation, edge behaviours, failure modes, H-test list, impact on P2-013..017 and P4/P5/P6/P7 hooks, owner questions)
- docs/design/hardware/README.md (index of the folder)
- No firmware, no control_map rows (they are proposed in the document only)

## Acceptance criteria
- [ ] HW_SUPPORT_HIGHLEVEL.md has all the sections listed in the owner request (grep the headings)
- [ ] Exactly three UX options, each with a 240x240 mock-up, gesture counts, an L-score table and the "what is in control" gesture (at most 2)
- [ ] One recommendation with reasons; byte costs for RAM (.bss), pool and flash are stated and checked against the headroom figures
- [ ] Claims about the TB-3 and the AIRA article are tagged [V] / [I] / unconfirmed (warning sign)
- [ ] The licensing question is flagged and no midi.guide data is copied into firmware
- [ ] Open owner questions are lettered with a recommendation
- [ ] `python3 tools/backlog.py check` passes; width probe run on the new on-screen strings

## Verification
```sh
python3 tools/backlog.py index && python3 tools/backlog.py check
sh tests/design/width_probe.sh -   # with the strings of section 7 on stdin
```

## Out of scope
- Low-level design (wire formats, state machines, code), TB-3 specifics beyond naming it as the first target, firmware, tests, control_map.tsv edits
- Files of P3-002, P3-014, FB-003, SYSTEM_MAP

## Log
2026-11-11 sonnet (designer): wrote docs/design/hardware/HW_SUPPORT_HIGHLEVEL.md and hardware/README.md. Model: per-channel RIG (device profile + 4 WIRES), three control states DEV / HAND / SEQ, lanes computed from the existing dice-layer seeds so algorithms and goldens are untouched. Three UX options scored with the 8.1 rubric; recommendation Option B (ROW 3 on PERF for device knobs, RIG page via hold CHN for setup). Cost: 184 B .bss, +2016 B pool (needs the fx-bus removal first), about 1.9 KB flash. 13 owner questions (A..M), 10 H-tests. Licensing of midi.guide data (CC BY-SA 4.0 vs GPL-3.0) flagged, nothing copied. No control_map rows added (proposed only), so check_scenarios and check_help are unaffected.
Verification: `python3 tools/backlog.py check` -> 113 items, 0 errors; width probe run on the new strings (3 one-line schematics over, noted in section 13).
Awaiting: owner human-review ("Approved: <name> <date>").

### 2026-11-11 review (sonnet)
Verdict: PASS on the checkable criteria; gate is human-review, so the task stays at `review`.
- Verified: HW_SUPPORT_HIGHLEVEL.md has the required sections (problem, model, dice/morph mapping 3.1, data model and byte costs 7, profiles and licensing 8, three options 6, recommendation 6.4, edge behaviours 9, failure modes 5, H-tests 10, impact 11, questions A..M with recommendations 12). Exactly three options, each with a mock-up, gesture counts and an L-score table; the totals (59 / 63 / 56) re-add correctly. Byte costs are stated for .bss (184), pool (+2016, with the fx-bus-removal prerequisite) and flash. TB-3 and CDM claims are tagged [V] / [I] / warning. Licensing is flagged (8.2, question E) and nothing is copied into firmware. `python3 tools/backlog.py check`: 113 items, 0 errors. Commit touches only the task file and docs/design/hardware/; no firmware or control_map changes. Build and check.sh not run, as instructed.
- Minor, not blocking: the one-line mock-up rows are schematic and exceed the width probe (disclosed in section 13). Option B amends two locked decisions (questions B and C), which the owner must approve.
Awaiting owner: read HW_SUPPORT_HIGHLEVEL.md sections 6 and 12 and answer questions A..M ("Approved: <name> <date>" in this Log).
