---
id: P2-020
title: TB-3 profile and alignment (phase 3 of the hardware support model)
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-019]
gate: human-review
---
## Goal
Fill the hardware support model with its first device: the Roland TB-3 as a built-in profile, written from owner-verifiable facts (own tests, the vendor's public MIDI implementation chart, community databases cited as references only; decision 99), so that a bassline channel on a TB-3 gets named knobs, sensible suggested wires and honest accent and slide behaviour. Serves VISION rule 4 (the TB-3 target workflow).

## Read first
- docs/design/hardware/HW_SUPPORT_LOWLEVEL.md (profile format, section 2; engine hooks, section 3; H-tests, section 6)
- docs/design/hardware/HW_SUPPORT_HIGHLEVEL.md (8.3 what is and is not known about accent and slide), docs/design/hardware/tb3-midi-guide.csv (reference only)
- docs/project/OWNER_QUESTIONS.md decisions 95, 99, 100, 104, 105 (104 and 105 are hardware facts measured by ear on the owner's TB-3 on 2026-11-11: accent at velocity >= 100; slide = overlap; accent on a slide destination inaudible; tag them [V], owner's unit); docs/design/KEYBED_USE_CASES.md (accent threshold H0)
- The TB-3 owner's manual and MIDI implementation chart (Roland, public); https://midi.guide/ and the CDM article as cross-checks

## Deliverable
- docs/design/hardware/TB3_PROFILE.md: the exact wire list (CC number, name of at most 5 letters, range, home value, suggested layer, form and move for each of the four suggested wires), the lane mapping for the bassline (which dice layer rides which wire and why, with the ACC form on the cutoff), accent behaviour (velocity threshold versus CC 16; the value of `acc_arg`), slide behaviour (overlap versus anything else; the value of `slide_mode`), the receive channel rule (default channel 2), the provenance record for the profile (source, how verified, credit), and what is not known and is left to the H-tests
- tools/hwprof/tb3.tsv (the profile data file in the format of HW_SUPPORT_LOWLEVEL 2.4; no firmware change)
- The H-RIG TB-3 test sheet text for HARDWARE_TEST.md (proposal in the document; P4-020 merges it)
- Scenario S21 re-walked with the real names and values; a TB-3 rows pass in tests/design/check_scenarios.py

## Acceptance criteria
- [ ] Every TB-3 claim is tagged [V] verified (document or hardware), [I] inferred or unconfirmed, with its source
- [ ] No data copied from the midi.guide CSV; every CC number has a second source (vendor chart or own test) or is marked unconfirmed and left out of the suggested wires
- [ ] Names of at most 5 letters and at most 40 px (width probe); the four suggested wires and their defaults are listed
- [ ] The provenance record has source, how verified and credit fields filled
- [ ] The H-tests for the TB-3 are listed with pass criteria, including accent threshold (H0, decision 95) and slide by overlap
- [ ] `python3 tests/design/check_scenarios.py`, `check_help.py` and `python3 tools/backlog.py check` pass

## Verification
```sh
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py
python3 tools/backlog.py index && python3 tools/backlog.py check
```

## Out of scope
- Firmware, the engine (P4), other devices, a CSV import tool, files of P3-002, P3-014, FB-003, SYSTEM_MAP

## Log
