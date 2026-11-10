---
id: P2-014
title: Design the PATTERNS library: top-20 preset patterns quantised to scale and chords
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-012]
gate: human-review
---
## Goal
Design PATTERNS: a selectable library of ready-made patterns (top 20 each for acid lines, bass lines, melody lines and other categories) that quantise to the global scale and chord progression. Design only.

## Owner brief (2026-11-10)
New algorithm PATTERNS: rather than a generator it is a preset pattern library to select from, fitting and quantising to the global scale and chords. Top 20 patterns for each of: ACID lines, bass lines, melody lines, etc.

## Read first
- docs/project/VISION.md (rules 1-4) and docs/project/decisions/ADR-0004-design-lock.md
- docs/design/SEQUENCER_UX.md, SCREEN_SPEC.md, ALGORITHM_SET.md and the TSV tables (page_states, button_modes, knob_map, key_map, control_map)
- docs/project/decisions/ADR-0003-sequencer-core.md (ChanRT is 24 B; RAM headroom and pool budget in HANDOFF.md)
- docs/project/OWNER_QUESTIONS.md (decision tables; owner rulings on global scale/key, 2-gesture setup, no menu diving)
- docs/design/HARMONY.md (from P2-012)
- docs/design/ALGORITHM_SET.md and docs/design/SEQUENCER_UX.md (how a channel's pattern is stored, steps/pages)

## Deliverable
- docs/design/algorithms/patterns/SPEC.md: categories (confirm the list with the owner: ACID, BASS, MELODY, plus proposals such as ARP-like, RHYTHMIC, PADS), 20 patterns per category chosen as the best-known idioms, each described as degree-relative data (step, scale degree or chord-tone index, octave, gate, accent, slide, tie) so it transposes to any key/scale/chord
- The authoring rule that avoids copying any copyrighted melody: patterns are generic idioms and rhythms, not transcriptions of songs (state the rule and how it is checked)
- Storage format and size per pattern and for the whole library; flash vs pool placement; against the pool budget (24,984 B for presets vs 780 B spare today, so state the cost and whether the library needs the fx-bus removal first)
- Selection UX: how a pattern is picked and previewed in one or two gestures (encoder browse with a category switch), what the knobs morph while a pattern is loaded (variation, density, octave span, humanise, chord-tone bias), whether a loaded pattern becomes editable steps (and how that differs from an algorithm channel)
- Behaviour when the global scale or chord changes (re-quantise live), and the dice layers mapping
- TSV table additions, help_text.tsv, L rubric score, scenario S20, ALGORITHM_SET.md update

## Acceptance criteria
- [ ] 20 patterns for each confirmed category listed in a TSV the prototype and firmware can read
- [ ] Every pattern passes a scale/range check in all 12 keys for the supported scales (script in tests/design)
- [ ] Storage cost stated and within budget, or an overrun and proposal given
- [ ] Owner confirmed the category list (record it in the log)
- [ ] All new UX in the TSV tables; check scripts pass; no 'TBD'
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Firmware, and prototype code (add to P2-008 scope).

## Log
