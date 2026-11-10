---
id: P2-012
title: Design the global HARMONY mode: key, scale and chord progression
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-011]
gate: human-review
---
## Goal
Design a new global mode, HARMONY, that sets key, scale and a chord progression which every channel is quantised to. Design only: no firmware.

## Owner brief (2026-11-10, verbatim intent)
Global mode with Key, Scale and chord progression: phrase length, number of chords, and the progression written as scale-degree numbers (for example 1-3-5-3). The producer can change the progression with dice rolls. Default is a single chord, `1----` (root held for the whole phrase). Every channel is quantised to the current chord. By default a channel plays the chord's bottom root note and uses the other chord tones depending on its algorithm. Morphing prioritises moving notes onto the chord's voicing (chord tones win over passing scale tones).

## Read first
- docs/project/VISION.md (rules 1-4) and docs/project/decisions/ADR-0004-design-lock.md
- docs/design/SEQUENCER_UX.md, SCREEN_SPEC.md, ALGORITHM_SET.md and the TSV tables (page_states, button_modes, knob_map, key_map, control_map)
- docs/project/decisions/ADR-0003-sequencer-core.md (ChanRT is 24 B; RAM headroom and pool budget in HANDOFF.md)
- docs/project/OWNER_QUESTIONS.md (decision tables; owner rulings on global scale/key, 2-gesture setup, no menu diving)

## Deliverable
- docs/design/HARMONY.md: data model (key, scale, phrase length in bars, chord count, per-slot degree and quality, how `1----` is stored), chord change timing on the 768-tick bar grid, how it composes with the existing global scale/key on the SET page (replace or extend: state which, and what the old rows become)
- The quantise rule every algorithm uses: root-note default, chord-tone priority, scale-tone fallback, octave handling, and what happens for a channel with a fixed octave range
- Morph rule: how each algorithm's morphs bias toward chord voicing (a per-algorithm table, ACID/EUCLID/SNAKE/STAGE)
- Dice rule: a PROG dice layer that re-rolls the progression (degrees, length, chord count) within musical limits (always starts or ends on degree 1 option, no more than N distinct chords); how it relates to the five shared dice layers plus ALL
- UX: where HARMONY lives (a global page reached in one gesture, screen labels, knob and button roles, a main-screen chord indicator), fully specified per the L (learnability) rubric; additions to page_states.tsv / button_modes.tsv / knob_map.tsv / help_text.tsv
- Interaction with undo, presets (what is saved per slot), MIDI-in chord following (decide: in v1 or deferred), and the clock/transport (chord position resets on PLAY)
- RAM and pool cost estimate against the HANDOFF budget (ChanRT stays 24 B or state why not)
- Scenario S18 added to docs/design/scenarios and COVERAGE.md

## Acceptance criteria
- [ ] HARMONY.md has no 'TBD'; the default (`1----`) behaves as one chord for every channel
- [ ] Every algorithm in the v1 set has a stated chord-quantise and morph rule
- [ ] Every new page, button mode and knob role is in the TSV tables, and tests/design check scripts pass
- [ ] L rubric score shown for each new function; any exception justified
- [ ] Memory cost within budget, or the overrun and a proposal stated
- [ ] ADR-0003 amended if the sequencer core changes; ALGORITHM_SET.md updated
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Writing firmware, or changing locked designs beyond what HARMONY requires (list each change).

## Log
