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

### Addition (owner, 2026-11-10, open-items 2C): the research decides the categories
The owner wants two families in the library, with the research choosing the final category list and the count per category:
- **Famous-song patterns:** the bass, acid and melody lines producers recognise and want to start from.
- **Classic genre patterns:** house bass, funky bass, tech house bass, rolling bass, and the like (for example acid house, techno, electro, disco, garage/UKG, DnB/reese, minimal, dub, Italo and synth-pop arps). The research proposes the full list per category.

Owner ruling (2026-11-11, decision 85): famous-song patterns **ship with their exact notes under style names** (for example `FUNK 3`, `HOUSE 2`), never under song titles. Song references appear only in RESEARCH.md. This overrides the earlier "generic idiom only" line in the research step above for famous-song patterns.

The algorithm picker must still fit the screen with six algorithms (owner decision 82). Show the screen.

## Read first
- docs/project/VISION.md (rules 1-4) and docs/project/decisions/ADR-0004-design-lock.md
- docs/design/SEQUENCER_UX.md, SCREEN_SPEC.md, ALGORITHM_SET.md and the TSV tables (page_states, button_modes, knob_map, key_map, control_map)
- docs/project/decisions/ADR-0003-sequencer-core.md (ChanRT is 24 B; RAM headroom and pool budget in HANDOFF.md)
- docs/project/OWNER_QUESTIONS.md (decision tables; owner rulings on global scale/key, 2-gesture setup, no menu diving)
- docs/design/HARMONY.md (from P2-012)
- docs/design/ALGORITHM_SET.md and docs/design/SEQUENCER_UX.md (how a channel's pattern is stored, steps/pages)

### Addition (owner, 2026-11-10): research first, two kinds of pattern
The task starts with a research step, recorded in docs/design/algorithms/patterns/RESEARCH.md:
1. **Reference patterns:** the best-known patterns per type (acid, bass, melody, etc.) as researched from external sources (books, tutorials, well-known pattern collections, the TB-303/TB-3 community). Record each with its source (a link or citation) and what makes it a classic. These are references for study and for checking our work; do not copy a protected melody into the library. Where a pattern is a famous recording, describe its idiom (rhythm, degree shape, accent and slide placement) in generic terms only.
2. **Common patterns:** the widely used generic patterns per type (for example offbeat bass, root-fifth-octave bass, 16th acid with slides on weak steps, walking bass, call-and-answer melody, rising/falling sequences) that we ship as the starting point. These are our own generic idioms, written as degree-relative data.
Each shipped pattern is **built to be morphed and tuned**: mark in the data which steps are structural (anchors that hold the identity) and which are decoration (ghost notes, ornaments, slides), so the algorithm's morph can thin, thicken, shift or vary the decoration without destroying the pattern, and the tune/variation dials (density, octave span, accent, slide amount, chord-tone bias) have defined effects. Provide golden cases for morph at min/mid/max on at least one pattern per category.

## Deliverable
- docs/design/algorithms/patterns/SPEC.md: categories (confirm the list with the owner: ACID, BASS, MELODY, plus proposals such as ARP-like, RHYTHMIC, PADS), 20 patterns per category chosen as the best-known idioms, each described as degree-relative data (step, scale degree or chord-tone index, octave, gate, accent, slide, tie) so it transposes to any key/scale/chord
- The authoring rule that avoids copying any copyrighted melody: patterns are generic idioms and rhythms, not transcriptions of songs (state the rule and how it is checked)
- Storage format and size per pattern and for the whole library; flash vs pool placement; against the pool budget (24,984 B for presets vs 780 B spare today, so state the cost and whether the library needs the fx-bus removal first)
- Selection UX: how a pattern is picked and previewed in one or two gestures (encoder browse with a category switch), what the knobs morph while a pattern is loaded (variation, density, octave span, humanise, chord-tone bias), whether a loaded pattern becomes editable steps (and how that differs from an algorithm channel)
- Behaviour when the global scale or chord changes (re-quantise live), and the dice layers mapping
- TSV table additions, help_text.tsv, L rubric score, scenario S20, ALGORITHM_SET.md update

## Acceptance criteria
- [ ] RESEARCH.md lists reference patterns with sources for every category, and separately the common starter patterns
- [ ] Every shipped pattern marks anchor versus decoration steps, and morph/tune behaviour has golden cases
- [ ] No shipped pattern is a transcription of a protected melody (check recorded in the log)
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
