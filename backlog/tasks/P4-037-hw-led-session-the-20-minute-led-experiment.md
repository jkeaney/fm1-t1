---
id: P4-037
title: HW-LED session: the 20-minute LED experiment
phase: P4
type: task
status: todo
priority: 1
executor: human
depends_on: [P4-021]
gate: hardware-test
---
## Goal
Settle by experiment on the real unit how the LEDs can show playback and sequence (lit, blink, dark), so the LED design (P4-038) is built on measured facts, not guesses.

## Read first
- /tmp/readiness_review.md section E (hardware facts and the unknowns)
- firmware/hal/fm1_input.h:41-90 (LED levels, scan)
- firmware/src/ui_input.c:170-247 (`ui_leds`, the old 'never a hard blink in a dark room' comment)
- docs/design/TIMING_SPEC.md 5.4
- docs/design/SEQUENCER_UX.md 6.3 (force and lit neighbours)
- docs/project/HARDWARE_TEST.md (add a short HW-LED section if the owner wants one)

## Deliverable
- A test image or a temporary local patch is NOT part of this task: the orchestrator supplies a throwaway build with a pattern generator (stored outside the repo or on a scratch branch) and asks the owner before flashing
- A short note `docs/design/LED_EXPERIMENT_NOTE.md` with the answers below and the scheme chosen
- The owner's `Approved:` line in the Log

## Acceptance criteria
- [ ] Playhead shown as a lit step, as a dark blink and as an inverted step: which reads best at 120 and 200 BPM, 1/16 (answered)
- [ ] Whether PLAY green has a mid level (answered yes/no)
- [ ] Mid vs dim vs lit distinguishable at the dimmest MENU LEDS level (answered per level)
- [ ] Black-key vs white-key columns: any brightness or visibility difference (answered)
- [ ] Force reading disturbance by lit neighbours: ADC counts with 0 / 1 / 4 neighbours lit (numbers)
- [ ] 2 Hz blink for armed/waiting is readable and not irritating (answered)
- [ ] Frame period during full redraws, in ms, and the visible playhead lag at 1/16 and 1/32 (numbers)
- [ ] Scheme chosen for P4-038, one paragraph
- [ ] Approved: owner line

## Verification
```sh
# orchestrator-supplied throwaway build; owner flashes only after asking
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
```

## UX gate
Gate B on real hardware: the owner watches the LEDs and answers the questions above; no producer-facing behaviour ships from this task.

## Out of scope
- Production LED code (P4-039)
- Any change to firmware/ in the repo from this task
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Depends on P4-021 only so the unit and TB-3 session is already set up; run it in the same sitting if the owner prefers.
