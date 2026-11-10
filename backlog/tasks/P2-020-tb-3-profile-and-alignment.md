---
id: P2-020
title: TB-3 profile and alignment (phase 3 of the hardware support model)
phase: P2
type: task
status: review
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
- [x] Every TB-3 claim is tagged [V] verified (document or hardware), [I] inferred or unconfirmed, with its source
- [x] No data copied from the midi.guide CSV; every CC number has a second source (vendor chart or own test) or is marked unconfirmed and left out of the suggested wires
- [x] Names of at most 5 letters and at most 40 px (width probe); the four suggested wires and their defaults are listed
- [x] The provenance record has source, how verified and credit fields filled
- [x] The H-tests for the TB-3 are listed with pass criteria, including accent threshold (H0, decision 95) and slide by overlap
- [x] `python3 tests/design/check_scenarios.py`, `check_help.py` and `python3 tools/backlog.py check` pass

## Verification
```sh
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py
python3 tools/backlog.py index && python3 tools/backlog.py check
```

## Out of scope
- Firmware, the engine (P4), other devices, a CSV import tool, files of P3-002, P3-014, FB-003, SYSTEM_MAP

## Log

### 2026-11-11 executor (sonnet)
Deliverables: docs/design/hardware/TB3_PROFILE.md, tools/hwprof/tb3.tsv, S21 re-walked (wires CUT 74 / RESO 71 / ENV-X 12 / ENV-Y 13; TUNE dropped from the suggestions).
Second source for the CC numbers: fetched and read the Roland PDF (TB-3_MI_1.pdf v1.10) with pdftotext: "CUTOFF (CC# 74)", "RESONANCE (CC# 71)", "ACCENT (CC# 16)", "EFFECT (CC# 17)", "ENV MOD X (CC# 12)", "ENV MOD Y (CC# 13)", "TUNING (CC# 104)". CC 1, 11, 68, 69, 102, 103 are CSV-only: left out. Nothing copied from the CSV.
Width probe (limit 40, rule 38): CUT 24, RESO 29, ENV-X 34, ENV-Y 34, ACC 24, EFFEC 34, TUNE 31, all OK.
```
python3 tests/design/check_scenarios.py -> scenarios: 22 files, 490 control-map rows, 0 with no scenario
python3 tests/design/check_help.py -> OK: 392 help rows, 2695 strings measured
python3 tools/backlog.py index && check -> 115 items, 0 errors
```
For the reviewer: homes (RESO 40, EFFEC 0, others 64) are [I] placeholders until H-RIG2; the vendor map is a SysEx map with CC annotations (0..255), not a receive-CC table, so CC functions from a host are [I] until H-RIG1; provenance level set to 1. Level was not set to 2 because the CC functions were not heard.
