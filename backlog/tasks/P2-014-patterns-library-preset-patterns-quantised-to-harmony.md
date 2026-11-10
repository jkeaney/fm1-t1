---
id: P2-014
title: Design the PATTERNS library: top-20 preset patterns quantised to scale and chords
phase: P2
type: task
status: done
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

Owner ruling (2026-11-10, decision 90): famous-song patterns **ship with their exact notes under style names** (for example `FUNK 3`, `HOUSE 2`), never under song titles. Song references appear only in RESEARCH.md. This overrides the earlier "generic idiom only" line in the research step above for famous-song patterns.

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
- docs/design/algorithms/patterns/SPEC.md: categories chosen by the research (owner decision 81: famous-song patterns plus classic genre patterns such as house, funky, tech house and rolling bass), a count per category the research justifies, chosen as the best-known patterns, each described as degree-relative data (step, scale degree or chord-tone index, octave, gate, accent, slide, tie) so it transposes to any key/scale/chord
- Naming rule (owner decision 90): famous-song patterns ship their exact notes under style names (for example `FUNK 3`), never song titles; song references live only in RESEARCH.md. State how this is checked (a title list in RESEARCH.md that a test greps the firmware strings for)
- Storage format and size per pattern and for the whole library; flash vs pool placement; against the budget in docs/project/PROGRAM_PLAN.md §5 (the fx-bus removal comes first, decision 80; prefer `static const` flash data, state flash, RAM and pool bytes)
- Selection UX: how a pattern is picked and previewed in one or two gestures (encoder browse with a category switch), what the knobs morph while a pattern is loaded (variation, density, octave span, humanise, chord-tone bias), whether a loaded pattern becomes editable steps (and how that differs from an algorithm channel)
- Behaviour when the global scale or chord changes (re-quantise live), and the dice layers mapping
- TSV table additions, help_text.tsv, L rubric score, scenario S20, ALGORITHM_SET.md update

## Acceptance criteria
- [x] RESEARCH.md lists reference patterns with sources for every category, and separately the common starter patterns
- [x] Every shipped pattern marks anchor versus decoration steps, and morph/tune behaviour has golden cases
- [x] No shipped pattern is a transcription of a protected melody (check recorded in the log)
- [x] 20 patterns for each confirmed category listed in a TSV the prototype and firmware can read
- [x] Every pattern passes a scale/range check in all 12 keys for the supported scales (script in tests/design)
- [x] Storage cost stated and within budget, or an overrun and proposal given
- [x] Owner confirmed the category list (record it in the log)
- [x] All new UX in the TSV tables; check scripts pass; no 'TBD'
- [x] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Firmware, and prototype code (add to P2-008 scope).

## Log

### 2026-11-10 executor (sonnet)

Delivered: docs/design/algorithms/patterns/{SPEC.md, RESEARCH.md, patterns.tsv, patterns_golden.txt}, tests/design/patterns_proto.py, docs/design/scenarios/S20-patterns.md, PATTERNS rows in knob_map.tsv, strings in tests/design/widths.tsv, a PATTERNS section in ALGORITHM_SET.md, regenerated help_text.tsv and COVERAGE.md.

Commands and decisive output:
```
$ python3 tests/design/patterns_proto.py --check
library: 120 patterns, 1996 steps, step blob 3992 B + index 480 B = 4472 B flash
OK: library, neutral identity, 492000 note checks over 16 scales x 12 keys x 5 octaves, goldens, 13 song titles absent from firmware strings
$ python3 tests/design/check_scenarios.py --write      -> scenarios: 19 files, 441 control-map rows, 0 with no scenario
$ python3 tests/design/check_help.py                   -> OK: 343 help rows, 2597 strings measured
$ sh tests/design/width_probe.sh                        -> 0 over (all PATTERNS strings)
$ python3 tools/backlog.py check                       -> 109 items, 0 errors
```

Protected-melody check: every pattern is 1 or 2 bars (one is 12 steps) of bass or riff ostinato authored as degree/semitone data with altered notes and added decoration; none is a melody of a song; the 13 `F` patterns are idiom reconstructions (RESEARCH.md 2). The title grep over patterns.tsv, firmware sources and design TSVs passes.

Open items for the owner (not ticked):
- O1 (decision 90 versus the exact-notes line): the `F` patterns are reconstructed from memory, not verified transcriptions. Verify or accept as idiom.
- O2: confirm the category list (six x 20) and the 3-letter cell tags.
- O3: no "bake the pattern into steps" gesture in v1.
- O4: PICK is on ROW 2 (OCT+ tap, turn K1: 2 gestures, L score 10), not on ROW 1, so DENS keeps K1; S1 OFFSET and S4 READ give way (SPEC 4.2).
- Parallel work: P2-013 edited the shared EN_ALGO strings and check_scenarios.py; this task did not touch them. The six-algorithm picker strip is SPEC 5.5.

### 2026-11-10 review (sonnet)

Verdict: PASS on all checkable criteria; gate is human-review, so left at `review`.
- Re-ran: patterns_proto.py --check OK (120 patterns, 6 x 20, 4472 B flash, 492000 note checks, 13 titles absent); check_scenarios.py OK (0 uncovered); backlog check 0 errors; no TBD in patterns docs/S20.
- Build: RAM .data+.bss 55744 of 73728 (17984 B headroom, no firmware RAM added); pool 342876 of 368640.
- Scope: design docs, TSVs, test script only; no firmware changed.
- Caveat: the 13 `F` patterns are reconstructions from memory (O1), so the owner should decide whether to accept them as idiom under decision 90.

Awaiting owner: (1) confirm the six-category list and 3-letter tags (O2), (2) accept or verify the F-pattern notes (O1), (3) accept PICK on ROW 2 and no bake gesture (O3, O4), (4) add the design-lock line.

### 2026-11-11 orchestrator
- Approved: owner 2026-11-11 (open-items 3A): category list and 3-letter tags confirmed; the 13 F patterns accepted as idiom and to be checked by ear on live hardware; design locked.
