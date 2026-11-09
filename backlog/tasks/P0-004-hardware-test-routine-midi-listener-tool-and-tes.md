---
id: P0-004
title: Hardware test routine: MIDI listener tool and test checklist
phase: P0
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P0-001]
gate: none
---
## Goal
Make hardware sessions repeatable and recorded, so owner feedback flows into the backlog (docs/project/PROCESS.md, Bugs and feedback loop).

## Read first
- /tmp/t1tools/listen.py (the listener used in the bring-up session; if it's gone, write from the spec below)
- tools/fm1_bridge.py
- backlog/_templates/feedback.md
- CLAUDE.md (Flashing, restoring and testing)

## Deliverable
- tools/fm1_midi_listen.py (SPDX header, mido): `--port` (default: a name containing 'JIANT FM'/'FM-1'/'Felucca', excluding 'Update'), `--seconds` (default 120), `--out` log path; logs every non-clock message with a timestamp; ends with a SUMMARY line: note-on count per MIDI channel, note histogram, velocity histogram, count of CC123
- docs/project/HARDWARE_TEST.md: setup diagram (FM-1 → Mac bridge → TB-3; FM-1 → host box → TB-3), the smoke checklist (boot, T1 SEQ on, PLAY, notes on ch2, STOP sends CC123, knobs change cards, SEQ pages, keys select tracks, mute), how to run the listener and the bridge, and how to file an FB- item from the template

## Acceptance criteria
- [ ] `.venv/bin/python tools/fm1_midi_listen.py --help` exits 0 and lists --port --seconds --out
- [ ] With no FM-1 connected, the tool prints a clear 'no FM-1 MIDI port found' message and exits 2 (no traceback)
- [ ] HARDWARE_TEST.md contains the checklist as `- [ ]` items, and a 'Filing feedback' section that names backlog/_templates/feedback.md

## Verification
```sh
.venv/bin/python tools/fm1_midi_listen.py --help
.venv/bin/python tools/fm1_midi_listen.py --seconds 1; echo exit=$?   # without the FM-1: exit=2
```

## Out of scope
- Changing the bridge.
- Any firmware change.

## Log
