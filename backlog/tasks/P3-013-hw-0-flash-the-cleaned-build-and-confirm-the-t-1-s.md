---
id: P3-013
title: HW-0: flash the cleaned build and confirm the T-1 still plays the TB-3
phase: P3
type: task
status: done
priority: 1
executor: human
depends_on: [P3-012]
gate: hardware-test
---
## Goal
Catch any boot or audio break from the removals before new core work starts (a build that crashes at boot needs mask-ROM recovery).

## Read first
- docs/project/HARDWARE_TEST.md (smoke checklist)
- CLAUDE.md (Flashing, restoring and testing)

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner runs the smoke checklist of HARDWARE_TEST.md: boot, identity, PLAY, notes on TB-3 channel 2, STOP sends CC123, knobs, mute
- Owner listens to the preview synth (ANALOG) on the speaker

## Acceptance criteria
- [ ] Boots to the T-1 screen without a reboot or crash
- [ ] `fm1_install.py --info --port "JIANT"` reports `FM-1_9xx`
- [ ] TB-3 plays on channel 2; STOP silences it
- [ ] Approved: owner line with date and firmware SHA in the Log

## Verification
```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
.venv/bin/python tools/fm1_midi_listen.py --seconds 120 --out /tmp/hw0.log
```

## Out of scope
- Any code change; problems become FB-nnn files

## Log

- 2026-11-12: Flashed build/felucca.fwsc (identity FM-1_900, port JIANT FM; image 168876 B, RAM 23060 B of 73728, pool 92500 B). `--info --port "JIANT"` reports FM-1_900 [running]. Booted to the T-1 screen. Owner confirmed PLAY/STOP and the HOME-hold menu. Captures (/tmp/hw0.log, /tmp/hw0stop.log): 689 note-ons in 120 s, all on MIDI channel 2, velocities 80 and 120. STOP sent CC123 on channels 1-5 and notes ceased, with no hung notes. TB-3 plays through tools/fm1_bridge.py (Ableton is used only for TB-3 audio monitoring). The new UX is not in this build (P4), so the screen is the old T-1 one, as expected.
- Not exercised: knob and mute behaviour, preview synth on the speaker, Ableton Sync check (decision 83); carry these to a later hardware task.
- Approved: owner, 2026-11-12, firmware SHA-1 fb0a4da83d8362646050a0a9f10a257b1c794d9d (build/felucca.fwsc).
