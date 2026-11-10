---
id: P2-013
title: Design the ARP algorithm over the chord progression
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-012]
gate: human-review
---
## Goal
Promote ARP from deferred to a v1 algorithm that arpeggiates the HARMONY chord progression, with shape, direction, speed and more dialled in and performed live. Design only.

## Owner brief (2026-11-10)
New algorithm ARP: uses the chord feature (P2-012) and produces an arpeggio of the chords. Arp shape, direction, speed etc. can all be dialled in and performed.

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
- Which chord voicing the arp follows, including the P2-012 VOICE, MOVE and PEDAL dials (a pedal tone as part of the arp cycle), and what the arp adds on top of them
- Direction set (up, down, up-down, random, as-played, chord-order), shape set, rate set tied to the global clock (including triplets), and how the pattern length (1..64 steps) interacts with the arp cycle
- UX: knob and button roles per row, screen labels, performance gestures; additions to the TSV tables and help_text.tsv; L rubric score
- Interaction with a single held note when HARMONY is `1----`
- Cost: ChanRT / pool impact
- Scenario S19; ALGORITHM_SET.md updated (v1 set becomes 5 algorithms; confirm channel/algorithm picker still fits the screen)

## Acceptance criteria
- [ ] SPEC.md has no 'TBD', parameters and morph and dice mappings as a table the prototype will read
- [ ] All new pages, modes and knob roles in the TSV tables; design check scripts pass
- [ ] Prototype and golden outputs are scheduled as an addition to P2-008 (update P2-008 scope), not done here
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Prototype code (P2-008), firmware.

## Log
