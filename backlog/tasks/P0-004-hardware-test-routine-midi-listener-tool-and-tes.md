---
id: P0-004
title: Hardware test routine: MIDI listener tool and test checklist
phase: P0
type: task
status: review
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
- [x] `.venv/bin/python tools/fm1_midi_listen.py --help` exits 0 and lists --port --seconds --out
- [x] With no FM-1 connected, the tool prints a clear 'no FM-1 MIDI port found' message and exits 2 (no traceback)
- [x] HARDWARE_TEST.md contains the checklist as `- [ ]` items, and a 'Filing feedback' section that names backlog/_templates/feedback.md

## Verification
```sh
.venv/bin/python tools/fm1_midi_listen.py --help
.venv/bin/python tools/fm1_midi_listen.py --seconds 1; echo exit=$?   # without the FM-1: exit=2
```

## Out of scope
- Changing the bridge.
- Any firmware change.

## Log

### 2026-11-09 executor (haiku-5.5)
Commands and output:
```
$ .venv/bin/python tools/fm1_midi_listen.py --help; echo exit=$?
usage: fm1_midi_listen.py [-h] [--port PORT] [--seconds SECONDS] [--out OUT]
  --port PORT        input port name (part of it); default: the FM-1 (JIANT FM / FM-1 / Felucca)
  --seconds SECONDS  how long to listen, default 120
  --out OUT          log file path
  --list             list MIDI input ports and exit
exit=0
$ .venv/bin/python tools/fm1_midi_listen.py --seconds 1; echo exit=$?
no FM-1 MIDI port found (inputs now: ['IAC Driver Bus 1']). Plug in the FM-1 over USB-C, or pass --port.
exit=2
$ .venv/bin/python tools/fm1_midi_listen.py --seconds 1 --out /tmp/x.log; echo exit=$?
exit=2
$ python3 tools/backlog.py check
29 items, 0 errors
```
Files: tools/fm1_midi_listen.py (new, SPDX header), docs/project/HARDWARE_TEST.md (new: setup diagram, smoke checklist as `- [ ]` items, running, Filing feedback section naming backlog/_templates/feedback.md).
Not run: no firmware change, no host suite, no hardware. The smoke checklist items are unticked on purpose: they are for the owner's hardware session.
Not verified: the SUMMARY line on a live FM-1 (no device connected here). Reviewer: the listener's SUMMARY is only written on normal exit; Ctrl-C prints the console summary only.
Acceptance criteria 1-3 ticked on the evidence above.
Final commit subject: `P0-004: Hardware test routine: MIDI listener tool and test checklist`
