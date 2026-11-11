---
id: P2-023
title: Per-channel preview mode and six demo presets (ACID LEAD BASS PAD STRINGS PIANO)
phase: P2
type: task
status: todo
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
- [ ] All five deliverables present
- [ ] The six presets each map to a concrete engine and parameters, or are flagged as not feasible with the reason
- [ ] Design checkers pass: tests/design/check_scenarios.py, check_help.py, tools/check_sequencer_ux.py, tests/design/width_probe.sh, backlog.py check
- [ ] Decisions 80, 98-109 honoured or explicitly reopened in the owner sheet

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes. Do not run tools/check.sh or builds. Do not touch the P3-014/BUG-003 files (review in progress).

## Log
