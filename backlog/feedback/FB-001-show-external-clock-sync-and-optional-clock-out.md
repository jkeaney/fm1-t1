---
id: FB-001
title: Screen must show when tempo is set by external MIDI clock (and decide on clock out)
phase: P2
type: feedback
status: todo
priority: 2
executor: sonnet
depends_on: []
gate: none
session: 2026-11-10, Ableton -> FM-1 over USB, Felucca build FM-1_900
---
## Goal
Triage this observation into tasks or bugs, or drop it with a reason.

## Read first
- docs/project/VISION.md
- docs/design/SEQUENCER_UX.md (SET page, main screen)
- firmware/src/midi_clock.c

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## Observation
Owner: "the fm1 needs to showcase when its bpm is being set by the external clock". Ableton sync to the FM-1 works
(the FM-1 follows incoming clock and Start/Stop). Nothing on screen says the tempo is external. Related finding: the
firmware has no MIDI clock OUT (midi_clock.c is receive only), so the FM-1 cannot be the clock master.

## Acceptance criteria
- [ ] The main screen and SET page show an "EXT" tempo state (and the incoming BPM) while external clock is driving, and revert to the internal BPM readout when the clock stops (needs a timeout rule). Added to the screen spec (P2-003) and the page tables.
- [ ] Clock out in v1 (OWNER DECIDED YES, 2026-11-10): the FM-1 sends 24-per-quarter MIDI clock plus Start/Stop/Continue over USB when it is the master (internal tempo); it does not echo clock when following external. Needs: design in SEQUENCER_UX/ADR-0003 (source of tempo: INT or EXT, a SET-page master/follow state), a hardware-test gate with Ableton and TB-3, and a dev task in E4.
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-10 created by orchestrator from owner feedback. Fold the EXT indicator into P2-003 (screen spec); clock-out is an owner question.
- 2026-11-10 owner answered YES to clock out in v1. Triage must add the design item (P2) and an E4 dev task.
- 2026-11-10 executor: criterion 1 (EXT state on main screen + SET, timeout rule, in the spec and page tables) done in SCREEN_SPEC 6, SEQUENCER_UX 2.8, page_states.tsv. Criterion 2 design part done (SEQUENCER_UX 2.8, ADR-0003 section 5 row + 7.4, SCREEN_SPEC 6.3). Still open for the orchestrator: the hardware-test gate (Ableton + TB-3) and the E4 dev task, and criterion 3 (converting to backlog ids). Not closed.

