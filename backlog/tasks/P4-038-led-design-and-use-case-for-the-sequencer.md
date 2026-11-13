---
id: P4-038
title: LED design and use case for the sequencer
phase: P4
type: task
status: todo
priority: 3
executor: sonnet
depends_on: [P4-037]
gate: human-review
---
## Goal
Specify what every LED does on every page so the panel mirrors the sequencer (blinking, solid or dark by what the active channels' playback and sequence are doing), and get the owner to approve the scheme (Gate A) before it is built.

## Read first
- /tmp/readiness_review.md section E (current LEDs, state exposed by the core, proposed rules, hazards)
- docs/design/LED_EXPERIMENT_NOTE.md (P4-037)
- docs/design/TIMING_SPEC.md 5.4
- docs/design/SEQUENCER_UX.md (LED rows per page, 6.3)
- docs/design/SCREEN_SPEC_V2.md
- firmware/src/ui_input.c:170-247 (`ui_leds`, LEDS menu modes), firmware/hal/fm1_input.h
- docs/project/UX_GATE.md and docs/design/features/_TEMPLATE.md

## Deliverable
- docs/design/features/P4-038-led-design.md from `_TEMPLATE.md`, Gate A filled: a table of every LED (14 buttons, 27 keys, PLAY green) by page and state
- Rules to specify: white keys lit when step ON, dark when OFF, dark past LENGTH, playhead inverts, REC cursor and NOTE_EDIT step blink, held key lit; M1-M4 lit unmuted with a flick on each hit, dark muted, breathe when MIDI OFF; CHN lit on CHAN, breathes when MIDI OFF; PG- / PG+ glow when the page exists, PG+ breathes in the add-page prompt; PLAY green mid with a beat flash per TIMING_SPEC 5.4 and 2 Hz blink when waiting for external start; REC lit while recording; dice buttons glow when the algorithm has that layer; EDIT lit when undo is available; OCT+ lit on ROW 2
- A mapping for the MENU LEDS modes DIM HI, DIM LO, OFF and INV for every rule above
- An OWNER RULING item: the old rule 'never a hard blink in a dark room' (comment in ui_input.c:177, cites #119; OWNER_QUESTIONS #119 now reads as the lane-bar decision, so the number is stale) conflicts with blinking. Proposal to rule on: hard on/off blinks only for the playhead and the beat flash, breathing for every attention cue
- Frame budget and latency rules, a host-test bitmap list for P4-039, and the force-reading hazard (lit neighbours) answered from P4-037
- An entry in docs/project/OWNER_QUESTIONS.md for the ruling once given

## Acceptance criteria
- [ ] Every LED has a row for every page it appears on, with INV/DIM/OFF behaviour
- [ ] Sync table (buttons, screen, help, workflow) filled; predicted ease scores pass the UX_GATE rule
- [ ] The hard-blink ruling is recorded as an owner decision with its number
- [ ] Design locked: owner line in the Log

## Verification
```sh
python3 tools/backlog.py check
grep -c 'INV' docs/design/features/P4-038-led-design.md   # at least 1
```

## UX gate
Producer-facing. The use case is the deliverable: `docs/design/features/P4-038-led-design.md`; Gate A is filled here, Gate B after P4-039.

## Out of scope
- Code
- The force scan blanking (P6-005)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Gate: owner must approve the scheme and rule on hard blinks vs breathing.

- 2026-11-13 owner ruling (AskUserQuestion): hard on/off blink only for the playhead and the beat flash; everything else solid, dark or slow breathing. Overrides the old "never a hard blink in a dark room" for those two only.
