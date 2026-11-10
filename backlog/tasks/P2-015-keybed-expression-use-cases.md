---
id: P2-015
title: Keybed expression: use cases for velocity and pressure that change the sound
phase: P2
type: task
status: review
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
- [x] Every shipped force gesture names its sound result, its MIDI output and its fallback, and scores at least L2
- [x] No gesture needs a finer reading than the hardware gives (the bins are justified against KEYBED_EXPRESSION_REFERENCE)
- [x] TB-3 MIDI facts carry sources and tags
- [x] TSV tables updated; `python3 tests/design/check_scenarios.py` and `python3 tools/backlog.py check` pass
- [x] RAM cost stated (target: no new per-step data)
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Firmware, and the calibration and LED-disturbance mechanics (section 6.2-6.3) unless a use case needs a change. List any change.

## Log

### 2026-11-10 executor (sonnet)
- Wrote docs/design/KEYBED_USE_CASES.md: 5 use cases (UC1 hard-tap accent, UC2 FILL lean, UC3 ROLL lean, UC4 REC 3-level dynamics, UC5 REC channel aftertouch, OFF), TB-3 CC facts from the Roland MIDI Implementation v1.10 [V], note-velocity accent threshold marked unconfirmed, reconsidered list, hardware checklist H0-H5.
- Changes: REC force is now 3 levels SOFT 64 / MED 90 / HARD 120 (was continuous 1..127); REC fallback velocity 100 -> 90 because 100 equals ACC_THR; ROLL repeats play at min(step vel, 90); new FS5 (flag REC_AT, OFF); gate G7 and H0 added to 6.6. Edited SEQUENCER_UX 6, key_map/knob_map/control_map/page_states, CONTROL_MAP.md, scenarios S07, S14. help_text unchanged. RAM: no new per-step data.
- Flag for the owner/reviewer: algorithms write a flat velocity 100 (TIMING_SPEC 424); if H0 shows the TB-3 accents at 100, those need 90 in an algorithm task. Calibration and LED sections 6.2-6.3 not changed.
- `python3 tests/design/check_scenarios.py` -> 441 control-map rows, 0 with no scenario; `python3 tools/check_control_map.py` -> OK; `python3 tools/check_sequencer_ux.py` -> OK; `python3 tests/design/check_help.py` -> OK; `python3 tools/backlog.py check` -> 0 errors.
- Design lock (owner line) still needed: criterion left unticked.
