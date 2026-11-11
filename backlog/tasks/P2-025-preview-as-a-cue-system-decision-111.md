---
id: P2-025
title: Preview as a CUE system (decision 111): independent preview gate, tuning as pitch
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-024]
gate: human-review
---
## Goal
Apply owner decision 111 to docs/design/PREVIEW.md and dependent tables: preview is independent of MIDI out, so a channel can be auditioned (a DJ-style CUE) with its MIDI muted, then unmuted or the synth turned up.

## Read first
- docs/project/OWNER_QUESTIONS.md decisions 109, 110, 111
- docs/design/PREVIEW.md (all), docs/design/SEQUENCER_UX.md (mute/solo rows), docs/design/MODULATION.md, tests/design/*.tsv
- firmware audio output facts (CLAUDE.md, docs: what outputs the FM-1 has: speaker, USB audio, headphones if any). Read-only.

## Deliverable
1. PREVIEW.md: remove the "MIDI OFF => preview silent" rule. Two independent gates per channel: MIDI out and PREVIEW. Same events feed both; only the destinations differ. Define the CUE use: preview on + MIDI muted; the player hears the channel in the speaker/headphone mix alongside other previewing channels, then unmutes MIDI. State what the device's audio outputs actually allow (is there a separate headphone/cue bus, or one mix?). Be honest about hardware limits; do not promise a separate cue bus if there is none.
2. Gestures: at most 2 each for preview on/off, MIDI mute/unmute, and preset change; on-screen role labels; ensure mute-key tap (MIDI) and hold (preview) do not conflict. Show status clearly: icons/strip for MIDI-on/preview-on states (four combinations).
3. Mapping table: TB-3 TUNING (CC 104) maps to a synth pitch offset (range stated, e.g. semitone span from the profile or a stated assumption marked [E]); the other no-equivalent rows stay unmapped (decision 111 2A).
4. Fix two doc errors from the P2-024 review: PREVIEW.md section 7 says S01 and S04 were not edited (they were); section 3.1 says a MONO overlap makes no new envelope trigger (LEAD is MONO and does retrigger).
5. Update affected tables (key_map, control_map, help_text, page_states, widths.tsv) and any scenario so the design checkers pass; list task effects (P4/P5/P6).
6. Owner sheet only if a real choice remains (e.g. whether cue mix includes other previewing channels).

## Acceptance criteria
- [ ] Deliverables 1-5 present
- [ ] Design checkers pass (check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh, backlog.py check)
- [ ] Decision 111 honoured; hardware limits stated honestly

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes, tools/check.sh, builds. Do not touch files owned by the P3-003 executor (running in the repo).

## Log
