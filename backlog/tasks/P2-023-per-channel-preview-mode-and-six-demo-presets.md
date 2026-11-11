---
id: P2-023
title: Per-channel preview mode and six demo presets (ACID LEAD BASS PAD STRINGS PIANO)
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Per owner decision 109: each sequencer channel has a PREVIEW mode that plays the channel's pattern through the FM-1 speaker/audio while also sending MIDI out, so patterns can be tested and demoed without external gear. Six demo presets: ACID, LEAD, BASS, PAD, STRINGS, PIANO. The built-in synth stays a preview tool (CLAUDE.md rule 2).

## Read first
- docs/project/OWNER_QUESTIONS.md decisions 80, 102, 108, 109
- docs/design/SYSTEM_MAP.md, SYSTEM_INVENTORY.md (what the engines are and what the removal chain P3-008..P3-010 deletes)
- docs/design/SEQUENCER_UX.md, SCREEN_SPEC.md, CHANNEL pages; docs/design/MODULATION.md (device/slot model)
- firmware/src/ engines/voice code (read-only), backlog tasks P3-008, P3-009, P3-010
- docs/project/VISION.md, CLAUDE.md

## Deliverable
Design docs only (no firmware), in docs/design/PREVIEW.md plus table/scenario edits as needed:
1. Preview model: per channel, on/off and preset; how it coexists with the device/MIDI out (both, speaker only, MIDI only); how it is shown on screen; gestures (at most 2 for on/off and for choosing a preset); where it lives among global vs channel settings.
2. The six presets: for each, which engine/voice and parameters realise it on the existing synth, honest about what the hardware can do (72 KB RAM, polyphony, CPU budget). Say which engines survive the removal chain; if STRINGS/PIANO/PAD cannot be done on ANALOG alone, say what must be kept from P3-008..P3-010 and the RAM/pool/CPU cost.
3. Interaction with the mod layer: does preview respond to the same mod slots (filter etc.)?
4. Effects on tasks: list changes needed to P3-008..P3-010 and P4/P5 tasks.
5. Owner question sheet with lettered options and a recommendation.

## Acceptance criteria
- [x] All five deliverables present
- [x] The six presets each map to a concrete engine and parameters, or are flagged as not feasible with the reason
- [x] Design checkers pass: tests/design/check_scenarios.py, check_help.py, tools/check_sequencer_ux.py, tests/design/width_probe.sh, backlog.py check
- [x] Decisions 80, 98-109 honoured or explicitly reopened in the owner sheet

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes. Do not run tools/check.sh or builds. Do not touch the P3-014/BUG-003 files (review in progress).

## Log

### 2026-11-11 executor (sonnet)
Deliverable: docs/design/PREVIEW.md (sections 1-8: preview model and gestures, six presets with engine and parameters, mod-layer interaction, task effects, owner sheet of 8 questions). No firmware, TSV or scenario rows edited; proposed rows are in PREVIEW.md section 7. Six toast strings measured with width_probe on a scratch file (all within 204 px), not added to widths.tsv.
Key findings: ACID/LEAD/BASS/PAD/STRINGS fit ANALOG (PAD/STRINGS lose chorus/reverb to P3-014); PIANO does not (question 1: A ANALOG e-piano, B keep SAMPLE PIANO set ~41.4 KB data + ~5 KB code, C keep FM6). P3-009/P3-010 stay held on question 1. ADR-0003 and SEQUENCER_UX 4.4 preview rules are superseded (not edited; question 8).
Verification:
- check_scenarios.py -> scenarios: 23 files, 490 control-map rows, 0 with no scenario
- check_help.py -> OK: 392 help rows, 2695 strings measured
- check_sequencer_ux.py -> OK
- width_probe.sh -> 286 strings, 0 over
- backlog.py check -> 120 items, 0 errors

### 2026-11-11 review (sonnet)
Verdict: PASS on the mechanical and design checks; gate is human-review, so the task stays at `review`.
- Re-ran all five checkers: check_scenarios (23 files, 490 rows, 0 unscenarioed), check_help OK (392 rows), check_sequencer_ux OK, width_probe 286 strings 0 over, backlog check 120 items 0 errors.
- Commit 795ccde touches only docs/design/PREVIEW.md and this task file. No BUG-003/P3-014 files, no firmware.
- Presets: ACID, LEAD, BASS, PAD, STRINGS each map to ANALOG with concrete E0..E7, ADSR, mode and LFO values; spot-checked against ANALOG_PRESETS in eng_analog.c:202-215 (values match; SPREAD is POLY/UNISON only, as the doc uses it). PIANO is honestly flagged as not feasible on ANALOG, with options A/B/C and costs.
- Labelling: [M] measured vs [E] estimate is declared up front and applied per figure (41.4 KB PIANO data [M], eng_sample ~5 KB [E], CPU [E], nothing [V]).
- Superseded rules: ADR-0003 and SEQUENCER_UX 4.4 are named as superseded in section 0 and 6, not edited, and put to the owner (question 8).
Awaiting owner: answer questions 1-8 in PREVIEW.md section 8 (chiefly Q1 PIANO: A/B/C, which unblocks P3-009/P3-010, and Q8 amending ADR-0003 / SEQUENCER_UX 4.4). Also confirm the hold-mute-key gesture.
