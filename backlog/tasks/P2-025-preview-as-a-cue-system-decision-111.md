---
id: P2-025
title: Preview as a CUE system (decision 111): independent preview gate, tuning as pitch
phase: P2
type: task
status: done
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
- [x] Deliverables 1-5 present
- [x] Design checkers pass (check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh, backlog.py check)
- [x] Decision 111 honoured; hardware limits stated honestly

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes, tools/check.sh, builds. Do not touch files owned by the P3-003 executor (running in the repo).

## Log

### 2026-11-11 executor (sonnet)
Edited docs/design/PREVIEW.md (sections 0, 1.2-1.5, 3.1-3.3, 4, 5, 7-9), SEQUENCER_UX.md 4.4 row, ADR-0003 (8.1, diagram, section 21 row 2), S04 step 14, key_map/control_map/help_text/page_states.tsv, tests/design/widths.tsv (removed 2 stale strings, added 6).
Two independent gates (MIDI: channel set, not muted, SOLO; PREVIEW: prv_on); four states with icon + `MIDI n/MUTE n/MIDI OFF/CUE n/CUE OFF` labels; hardware: speaker + USB audio only, one mix, no cue bus, no headphone jack documented (stated, HW-1 to confirm). TB-3 TUNING -> per-part PIT, +-50 cents [E]; other no-equivalent rows unmapped. Fixed P2-024 errors (section 7 S01/S04, 3.1 MONO/LEGATO). Task effects: mute/SOLO must move to the MIDI gate in sq_emit (P4-004/P4-012, P6-003), P4-011 icon/labels, new preview task adds P_FINE and T-PRV4.
Owner note (section 8): default is that all previewing channels share the one mix; a "cue only" exclusive mode is not designed.
```
check_scenarios.py: 23 files, 490 control-map rows, 0 with no scenario
check_help.py: OK: 392 help rows, 2695 strings measured
check_sequencer_ux.py: OK
width_probe.sh: 296 strings, 0 over
backlog.py check: 122 items, 0 errors
```

### 2026-11-11 review (sonnet)
Verdict: PASS on content; gate is human-review, so left at `review`. Awaiting owner: read docs/design/PREVIEW.md 1.2 (two gates, four states, one mix and no cue bus), 1.4 (tap = MIDI mute, hold 0.4 s = PREVIEW), 3.2 (TUNING = PIT, +-50 cents [E]) and 8 (default: all previewing channels share the mix; no "cue only" mode), and confirm.
- Re-ran all five checkers: scenarios 23 files, 0 with no scenario; help OK 392 rows; check_sequencer_ux OK; width_probe 296 strings, 0 over; backlog check 122 items, 0 errors. Design only, no build.
- Decision 111 checked: independent gates (PREVIEW.md 1.2, 3.1 Mute row, P4-004 "no shared condition"); CUE use defined; hardware stated honestly (speaker + USB audio, one mix, no cue bus, no headphone jack documented, HW-1 to confirm); gestures: preview on/off 1 hold, MIDI mute 1 tap, sound 2 (hold + ALGO); tap/hold on one key do not conflict (0.4 s threshold); four states with icon and MIDI n / MUTE n / MIDI OFF / CUE n / CUE OFF labels. TUNING = P_FINE pitch with [E]; other rows unmapped. P2-024 errors fixed (section 7, 3.1 retrigger text at line 201).
- Contradiction (not blocking now): SEQUENCER_UX.md:581 (U48 "new hits stop at once"), :1013 (`hit(n)` comment lists mute) and :103 (SLIDE "muted" case) still describe mute as stopping hits at the source. PREVIEW.md P4-004/P4-012 and P6-003 rows already say mute/SOLO move to the MIDI gate in `sq_emit`, and 4.4 (line 527) is amended. It must be fixed before P4-004 starts (add the same note to U48 and the `hit(n)` line, or have P4-004's executor treat PREVIEW.md as overriding); a mute-only slide glide cancel needs a decision (does a muted channel's slide still glide in the preview? default: yes, same events).

### 2026-11-11 orchestrator
Approved: owner 2026-11-11 (decision 112: one shared mix on the master output, speaker or headphone jack; mute = MIDI gate).
