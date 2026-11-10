---
id: P2-015
title: Keybed expression: use cases for velocity and pressure that change the sound
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-012]
gate: human-review
---
## Goal
Rework force sensing (SEQUENCER_UX section 6) around musical use cases, so velocity and pressure clearly improve live performance with a TB-3-class synth. Today the design lists mechanisms (FS1-FS4). The owner wants each one justified by what it does to the sound. Design only.

## Owner brief (2026-11-10, open-items 6A)
Keep force sensing in v1, but it needs really good use cases for how it elevates performing:
1. **Sound:** what each gesture does to the sound the listener hears.
2. **Input UX:** how the producer plays it, what they see before it counts, and the fallback.
3. **Output:** what changes in the MIDI out (velocity, accent, CC, note length, repeats), and on which channel.

## Read first
- docs/project/VISION.md and docs/design/SEQUENCER_UX.md section 6 (FS1-FS4, calibration, LED disturbance, gate 6.6)
- docs/design/KEYBED_EXPRESSION_REFERENCE.md (hardware limits: noisy velocity, small aftertouch range, F3..C5 only, LED interference)
- docs/design/HARMONY.md (P2-012): pressure could drive a voicing dial
- docs/design/ALGORITHM_SET.md (morphs and dice layers), docs/design/TIMING_SPEC.md

## Deliverable
- docs/design/KEYBED_USE_CASES.md: at most 6 use cases, ranked by musical value. For each: the musical intent (one line a producer would say, for example "push the acid line harder for the drop"), the gesture, what the listener hears on a TB-3 and on a generic poly synth, the exact MIDI output change (message, range, curve, channel), the screen feedback, the fallback, and why it works within the hardware limits (coarse bins, visible before it counts).
- Research which expressive inputs the TB-3 responds to over MIDI (velocity and accent threshold, any CCs such as cutoff or envelope), with sources. Mark each claim [V], [I] or ⚠.
- Reconsider the earlier "not used" items (pressure sent to MIDI as a CC or as aftertouch, pressure to a morph or a HARMONY dial) against the use cases. Keep, add or drop each, with a reason.
- Update SEQUENCER_UX section 6 and the TSV tables (knob_map, help_text, page_states, button_modes) to match. Remove gestures that have no strong use case.
- Hardware-test checklist items for the HT-FORCE gate: one per use case, judged by ear on the TB-3.

## Acceptance criteria
- [ ] Every shipped force gesture names its sound result, its MIDI output and its fallback, and scores at least L2
- [ ] No gesture needs a finer reading than the hardware gives (the bins are justified against KEYBED_EXPRESSION_REFERENCE)
- [ ] TB-3 MIDI facts carry sources and tags
- [ ] TSV tables updated; `python3 tests/design/check_scenarios.py` and `python3 tools/backlog.py check` pass
- [ ] RAM cost stated (target: no new per-step data)
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Firmware, and the calibration and LED-disturbance mechanics (section 6.2-6.3) unless a use case needs a change. List any change.

## Log
