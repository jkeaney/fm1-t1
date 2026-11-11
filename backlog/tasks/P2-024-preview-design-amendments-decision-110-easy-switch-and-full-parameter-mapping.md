---
id: P2-024
title: Preview design amendments (decision 110): easy on/off, preset per channel, full parameter mapping
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-023]
gate: human-review
---
## Goal
Apply owner decision 110 to docs/design/PREVIEW.md and the rules it supersedes, so the preview design is final.

## Read first
- docs/project/OWNER_QUESTIONS.md decisions 104, 105, 106, 108, 109, 110
- docs/design/PREVIEW.md (esp. section 8 and 6), docs/design/MODULATION.md, docs/design/hardware/TB3_PROFILE.md (accent = velocity >= 100, slide = overlap)
- docs/design/SEQUENCER_UX.md 4.4, docs/project/decisions/ ADR-0003, firmware engine code eng_analog.c (read-only)

## Deliverable
1. PREVIEW.md updated: PIANO = ANALOG e-piano approximation (parameters); all channels OFF by default; level = MASTER pot only; hold-mute gesture (6A) re-examined against "really easy to turn preview on/off per channel and change the preset per channel": keep 6A, but confirm at most 2 gestures for each and show on-screen role labels; preview and MIDI are always the SAME output (remove the speaker-only / MIDI-only split and question 7 text accordingly; say what a channel with MIDI OFF does).
2. New section: internal synth parameter mapping, best effort, from the channel's setup: notes, gates/length, accent (velocity >= 100), slide (overlap => glide), filter cutoff/resonance, env mod, and other mod-slot CCs (MODULATION.md). Table: each device CC / event -> synth parameter or "no equivalent", per preset. State what ANALOG cannot do.
3. Amend ADR-0003 and SEQUENCER_UX 4.4 as PREVIEW.md section 6 describes (decision 8A); add the toast strings to tests/design/widths.tsv and the proposed rows to control_map/key_map/help/page_states so the design checkers pass.
4. Task effects: confirm P3-008..P3-010 may proceed (ANALOG only) and list any P4/P5 changes.
5. Short owner sheet only if a real choice remains.

## Acceptance criteria
- [ ] Deliverables 1-4 present
- [ ] Design checkers pass (check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh, backlog.py check)
- [ ] Decision 110 honoured point by point

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes, tools/check.sh, builds. Do not touch the P3-003 executor's files (it is running tests in the repo).

## Log
