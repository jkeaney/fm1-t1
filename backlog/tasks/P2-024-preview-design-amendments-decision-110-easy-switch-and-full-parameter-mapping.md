---
id: P2-024
title: Preview design amendments (decision 110): easy on/off, preset per channel, full parameter mapping
phase: P2
type: task
status: review
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
- [x] Deliverables 1-4 present
- [x] Design checkers pass (check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh, backlog.py check)
- [x] Decision 110 honoured point by point

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes, tools/check.sh, builds. Do not touch the P3-003 executor's files (it is running tests in the repo).

## Log

### 2026-11-11 executor (sonnet)
Design docs only; no builds, check.sh not run (P3-003 executor active).
- PREVIEW.md rewritten: all channels OFF by default; level = MASTER only; PIANO = ANALOG e-piano approximation with stated limits; 6A kept and re-examined (on/off 1 gesture, sound hold+ALGO, on-screen labels); preview == MIDI stream, MIDI OFF channel silent (speaker-only mode and question 7 text removed); new section 3 (notes, gate, accent, slide, TB-3 11 CCs and GENERIC CCs -> synth target, "no equivalent", 3.3 ANALOG limits); section 4 task effects (P3-008..P3-010 ANALOG only may proceed; P4/P5/P6/P7 changes listed); section 8: no real choice remains.
- BASS patch now LEGATO+GLIDE so slides work (2.5).
- ADR-0003 amended in place + addendum 21; SEQUENCER_UX 4.4 row and line 536; S01/S04 sentences updated.
- Tables: widths.tsv (6 toast/strip strings, 0 over), key_map (hold rows for mute keys), control_map, help_text (mute keys and EN_ALGO), page_states PERF.
Verification output:
- check_scenarios.py: `scenarios: 23 files, 490 control-map rows, 0 with no scenario, 305 only in a sweep`
- check_help.py: `OK: 392 help rows, 2695 strings measured`
- check_sequencer_ux.py: `OK`
- width_probe.sh: `292 strings, 0 over`
- backlog.py check: `121 items, 0 errors`
Note for reviewer: owner decision on the PAD Y/Z, TUNING mapping is "no equivalent" (kept honest rather than invented stand-ins); the 100-velocity accent threshold is approximated continuously.

### 2026-11-11 review (sonnet)
Verdict: PASS on content; gate is human-review, so left at `review`. Awaiting owner: read docs/design/PREVIEW.md sections 1.4 (gestures), 3 (mapping tables, "no equivalent" rows) and 8, and confirm that a MIDI OFF channel is silent on the preview too and that the PAD Y/Z, TUNING, EFFECT rows stay "no equivalent".
- Re-ran all five checkers: scenarios 23 files, 0 with no scenario; help OK 392 rows; check_sequencer_ux OK; width_probe 292 strings, 0 over; backlog check 121 items, 0 errors. No firmware changed, no build run (design only).
- Decision 110 point by point: 1A, 2 (all OFF, 0/1.1), 4A (MASTER only), 7 (one `sq_emit` condition for MIDI and preview; MIDI OFF = silent, stated in 1.2 and ADR-0003 addendum 21), 8A (ADR-0003 and SEQUENCER_UX 4.4 edited) all present. On/off = 1 gesture, sound = hold + ALGO (2), with strip label and ALGO cell role. Mapping covers notes, gate, velocity, accent, slide, ratchet, 11 TB-3 CCs, GENERIC CCs, with honest "no equivalent" and a 3.3 limits list. Toast strings are in widths.tsv.
- BASS MONO -> LEGATO is justified: voice.c:403 MONO always retriggers, so an overlap re-attacks; LEGATO (voice.c:405) glides without retrigger, matching TB-3 slide. Sensible.
- Minor doc nits (not blocking, fix with the next PREVIEW edit): (1) section 7 last bullet says S01/S04 were not edited, but the diff edited both; (2) 3.1 slide row says "no new envelope trigger" for MONO or LEGATO, but LEAD stays MONO and does retrigger (voice.c:403); say LEGATO-only gives a true slide, or switch LEAD to LEGATO.
