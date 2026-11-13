---
id: P4-050
title: HW-RIG: TB-3 cutoff, resonance and the others follow ROW 3 and the MOD lanes; stop leaves no stuck CC
phase: P4
type: task
status: todo
priority: 1
executor: human
depends_on: [P4-049]
gate: hardware-test
---
## Goal
The owner runs the RIG and MOD chain on the real FM-1 and TB-3 and signs it off. Cutoff, resonance and the other wired parameters audibly follow ROW 3 (by hand) and the MOD lanes (by sequence); the unverified TB-3 facts are settled; stop leaves no stuck CC.

## Read first
- docs/project/HARDWARE_TEST.md (add the section below to it; it is the sheet the owner follows)
- docs/design/MODULATION.md section 9 (H-MOD1..H-MOD6, H-RIG20) and docs/design/hardware/TB3_PROFILE.md section 10 (H-RIG sheet)
- docs/design/hardware/RIG_MOD_V1.md (the verified / unverified list from P4-045) and docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md (the live checklist)
- tools/fm1_bridge.py (FM-1 USB MIDI -> Mac -> TB-3; the TB-3 receives on channel 2) and tools/fm1_midi_listen.py (CC monitor)
- backlog/tasks/P4-021-*.md (the HW-1 sheet format) and backlog/tasks/P4-037-*.md (a hardware-session task model)

## Deliverable
- The executor (sonnet, orchestrator side, before the session) adds a `## HW-RIG (RIG and MOD on the TB-3)` section to docs/project/HARDWARE_TEST.md with the checklist below, pass criteria and a result table; the OWNER then runs it and writes the results and an `Approved:` line in this Log
- Checklist (each with an expected result and a pass / fail box):
  - R1 ROW 3: OCT+ twice on a TB-3 channel shows `ROW 3`; turning R1 (CUT) audibly moves the filter at once (HAND); R2 (RESO) the resonance; R3 / R4 ENV-X / ENV-Y
  - R2 setup: hold CHN opens the RIG page; K1 picks other parameters (PAD-Y, PAD-Z, ACC, EFFEC, TUNE); each audibly moves its parameter; K4 DEPTH above 0 starts the lane
  - R3 MOD lanes: with ACID playing, accents open the filter (lane A), the resonance sweep lines up with the bar (lane B) (H-MOD5); LFO dice changes the figure; EDIT restores it; DEPTH 0 gives the TB-3 knob back (H-MOD2: no CC74 on a monitor for 60 s)
  - R4 knob versus CC (H-MOD1): turn the TB-3 knob while a slot is active and idle; record whether our next CC overwrites it and whether the knob is ignored until moved
  - R5 stop: PLAY then STOP with two driven wires leaves each at its park value once (H-MOD6); mute does the same; PLAY again re-sends at the first step; no stuck CC after PANIC
  - R6 rate (H-MOD4): 4 active wires at 1/16 and 180 BPM through the host box: no note delay audible or measured above 1 ms, no dropped notes (ring drop counters)
  - R7 scatter (H-RIG20): SCTYP / SCDEP linked pair; does SCDEP 0 mean scatter off; does a type change reset the depth; how many zones SCTYP has
  - R8 readability: ROW 3 and the RIG page are legible at playing distance (H-RIG16)
  - R9 device NONE: the sequence is byte-identical to before (no CC at all on a monitor)
  - R10 power cycle is not part of this task (presets are P7)
- Findings: every failure becomes a `BUG-` or `FB-` item from the templates. The unverified list from P4-045 is updated with the measured answers (zones count, homes RESO / EFFEC, pickup behaviour); profile data corrections are a data-only change to `docs/design/hardware/profiles/tb3.tsv` (a follow-up task)

## Acceptance criteria
- [ ] The HW-RIG section is in HARDWARE_TEST.md with every item above
- [ ] The owner has run R1-R9 and recorded pass / fail with notes in this Log
- [ ] The TB-3 cutoff and resonance audibly follow ROW 3 and the MOD lanes (R1, R3)
- [ ] Stop and mute leave no stuck CC (R5)
- [ ] Every failure is filed as BUG / FB; the unverified TB-3 list is updated with the results
- [ ] Owner `Approved: <name> <date>` line in this Log

## Verification
```sh
python3 tools/backlog.py check
.venv/bin/python tools/fm1_midi_listen.py     # CC monitor during R3, R5, R9
.venv/bin/python tools/fm1_bridge.py          # FM-1 -> Mac -> TB-3 (ask the owner before flashing: always)
```

## UX gate
Gate B on the real unit for P4-048 and P4-049 (use case `docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md` live checklist), measured gesture counts.

## Out of scope
- Fixing findings (separate tasks)
- A second device
- Flashing without the owner's say-so (CLAUDE.md: always ask)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created. Owner rulings 2026-11-13: "MOD output is IN v1." and "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."
