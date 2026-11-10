---
id: P2-013
title: Design the ARP algorithm over the chord progression
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P2-012]
gate: human-review
---
## Goal
Promote ARP from deferred to a v1 algorithm that arpeggiates the HARMONY chord progression, with shape, direction, speed and more dialled in and performed live. Design only.

## Owner brief (2026-11-10)
New algorithm ARP: uses the chord feature (P2-012) and produces an arpeggio of the chords. Arp shape, direction, speed etc. can all be dialled in and performed.

### Addition (orchestrator, 2026-11-10, owner decision 82)
v1 has six algorithms. Show the algorithm picker screen with all six, and confirm the names fit the label widths (`tests/design/width_probe.sh`).

## Read first
- docs/project/VISION.md (rules 1-4) and docs/project/decisions/ADR-0004-design-lock.md
- docs/design/SEQUENCER_UX.md, SCREEN_SPEC.md, ALGORITHM_SET.md and the TSV tables (page_states, button_modes, knob_map, key_map, control_map)
- docs/project/decisions/ADR-0003-sequencer-core.md (ChanRT is 24 B; RAM headroom and pool budget in HANDOFF.md)
- docs/project/OWNER_QUESTIONS.md (decision tables; owner rulings on global scale/key, 2-gesture setup, no menu diving)
- docs/design/HARMONY.md (from P2-012)
- the existing steparp spec under docs/ (the deferred ARP spec) and docs/design/ALGORITHM_SET.md
- docs/design/FULL_CONTROL.md (up to 8 morphs per algorithm, K1-K4 plus OCT-/OCT+ row switch)

## Deliverable
- docs/design/algorithms/arp/SPEC.md in the same table form as the other v1 algorithms: parameters (range, default), the five dice layers mapped, up to 8 morphs (shape, direction, speed/rate, octave span, gate, swing/humanise, note order, hold/latch), what chord voicing it follows, and behaviour on chord change mid-pattern
- Which chord voicing the arp follows, including the P2-012 VOICE, MOVE and DRONE dials (a drone/pedal tone as part of the arp cycle), and what the arp adds on top of them
- Direction set (up, down, up-down, random, as-played, chord-order), shape set, rate set tied to the global clock (including triplets), and how the pattern length (1..64 steps) interacts with the arp cycle
- UX: knob and button roles per row, screen labels, performance gestures; additions to the TSV tables and help_text.tsv; L rubric score
- Interaction with a single held note when HARMONY is `1----`
- Cost: ChanRT / pool impact
- Scenario S19; ALGORITHM_SET.md updated (v1 set is 6 algorithms, owner decision 82; confirm channel/algorithm picker still fits the screen)

## Acceptance criteria
- [x] SPEC.md has no 'TBD', parameters and morph and dice mappings as a table the prototype will read
- [x] All new pages, modes and knob roles in the TSV tables; design check scripts pass
- [x] Prototype and golden outputs are scheduled as an addition to P2-008 (update P2-008 scope), not done here
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Prototype code (P2-008), firmware.

## Log

### 2026-11-10 executor (sonnet)
Delivered docs/design/algorithms/arp/SPEC.md (parameters, 8 morphs, 5 dice layers, direction/shape/rate sets, chord-voicing and ROLE behaviour, chord change, pattern length, single held note, cost, UX roles, L rubric, picker), scenario S19, the knob_map.tsv ARP rows, the `ALGO_PICK` page state, widths.tsv rows, ALGORITHM_SET.md (six algorithms, section 13), SCREEN_SPEC.md (picker, always-on chord chip for ARP), the `EN_ALGO` text in control_map/knob_map/CONTROL_MAP/help (six names), gen_help.py, check_scenarios.py (S18 onward are narrative), and the P2-008 / P2-017 scope lines (P2-008: core additions AM-1 to AM-4 in sq_types/sq_prims; P2-017: the ARP golden cases). Worked examples come from a throwaway Python transcription (not committed); P2-017 produces the real goldens.
Commands and decisive output:
```
python3 tests/design/check_scenarios.py   -> scenarios: 20 files, 441 control-map rows, 0 with no scenario, 265 only in a sweep   (exit 0, no FAIL/GAP)
python3 tools/backlog.py check            -> 109 items, 0 errors
python3 tests/design/check_help.py        -> OK: 343 help rows, 2597 strings measured
sh tests/design/width_probe.sh            -> 162 strings, 0 over   (1 PATT MOFF 71/76, ACID EUCL SNAK STAG [ARP] PATT 185/240)
python3 tests/design/harmony_proto.py --check -> CHECK OK
grep -in tbd docs/design/algorithms/arp/SPEC.md -> no match
```
Not ticked: "Design locked: owner line" (human-review gate; the owner writes it).
For the owner (SPEC 8 and 11): (1) the DIR names and meanings `ASPL` (root-first, voiced order) and `CHORD` (stack order), since no held keys exist; (2) default DENS 12; (3) the figure restarts with the loop, not with the chord (stateless; matches ADR-0003's loop rule); (4) the chord chip is always drawn for an ARP channel; (5) four core additions AM-1 to AM-4 (chord argument to `step`, `sub_note` hook, `AF_CHORD`, `SQ_NALGO` 6) need the ADR-0003 amendment that P2-005 plans; (6) `PATT` is a placeholder short form for PATTERNS (P2-014 owns the real one; strip has room for 7 letters). Also noted: S18 shows the chord chip `I` for the minor tonic while HARMONY.md 2.4 says lower case (`i`); S19 follows 2.4, S18 was not edited.

### 2026-11-10 review (sonnet)
Verdict: PASS on the checkable criteria; gate is human-review, so the task stays at `review`.
Re-ran: check_scenarios.py (20 files, 0 FAIL/GAP), backlog.py check (109 items, 0 errors), check_help.py OK, width_probe.sh (162 strings, 0 over; picker line 185/240), `grep -i tbd` on SPEC.md (no match). Commit touches only docs, tests/design scripts and the task file (docs-only; no firmware). SPEC has the parameter, 8-morph and 5-dice tables, direction/shape/rate sets, chord/ROLE/DRONE behaviour, single-note case, cost, UX roles; S19 and the ALGO_PICK page state exist; P2-008 and P2-017 scope lines updated.
Note: `tools/check.sh` ends CHECK FAIL: tests (clang link error). The working tree has uncommitted edits to firmware/src (seq.c, torso_t1.c, ui.c) and two other task files that are not part of this commit, so the failure is not attributable to P2-013. Build line: RAM .data+.bss 55744 B of 73728 (17984 B headroom); pool 342876 of 368640.
Awaiting owner: read SPEC 8 and 11 (the six open points: DIR names ASPL/CHORD, default DENS 12, figure restarts with the loop, always-on chord chip, AM-1..AM-4 core additions, PATT short form) and write the "Design locked" line.
