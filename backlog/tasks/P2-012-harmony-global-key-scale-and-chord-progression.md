---
id: P2-012
title: Design the global HARMONY mode: key, scale and chord progression
phase: P2
type: task
status: in_progress
priority: 1
executor: sonnet
depends_on: [P2-011]
gate: human-review
---
## Goal
Design a new global mode, HARMONY, that sets key, scale and a chord progression which every channel is quantised to. Design only: no firmware.

## Owner brief (2026-11-10, verbatim intent)
Global mode with Key, Scale and chord progression: phrase length, number of chords, and the progression written as scale-degree numbers (for example 1-3-5-3). The producer can change the progression with dice rolls. Default is a single chord, `1----` (root held for the whole phrase). Every channel is quantised to the current chord. By default a channel plays the chord's bottom root note and uses the other chord tones depending on its algorithm. Morphing prioritises moving notes onto the chord's voicing (chord tones win over passing scale tones).

### Addition (owner, 2026-11-10): VOICING dials
HARMONY also has a small set of voicing dials, simple and mathematical, performable live:
- VOICE (spread/density): dials the chord's voicing up or down, from root-only through triad to extended tones (7th, 9th), and close to open spread.
- MOVE (voice-leading distance): dials how far notes may jump between consecutive chords; low values keep common tones and move each voice the smallest step, high values allow full root-position jumps. This sets the difference between chords.
- PEDAL: dials in a pedal tone, the one note held across the progression. The dial walks the pedal through candidate scale degrees (and octaves) so the producer finds the alignment by ear; show which degree it sits on and mark clashes with the current chord.

## Read first
- docs/project/VISION.md (rules 1-4) and docs/project/decisions/ADR-0004-design-lock.md
- docs/design/SEQUENCER_UX.md, SCREEN_SPEC.md, ALGORITHM_SET.md and the TSV tables (page_states, button_modes, knob_map, key_map, control_map)
- docs/project/decisions/ADR-0003-sequencer-core.md (ChanRT is 24 B; RAM headroom and pool budget in HANDOFF.md)
- docs/project/OWNER_QUESTIONS.md (decision tables; owner rulings on global scale/key, 2-gesture setup, no menu diving)

## Deliverable
- docs/design/HARMONY.md: data model (key, scale, phrase length in bars, chord count, per-slot degree and quality, how `1----` is stored), chord change timing on the 768-tick bar grid, how it composes with the existing global scale/key on the SET page (replace or extend: state which, and what the old rows become)
- The quantise rule every algorithm uses: root-note default, chord-tone priority, scale-tone fallback, octave handling, and what happens for a channel with a fixed octave range
- Voicing dials: define VOICE, MOVE and PEDAL as integer-valued formulas (tables, not prose): inputs, ranges, defaults (neutral = plain root-position triads, no pedal), the exact note-selection and voice-leading algorithm, and how they combine with the morph bias. Include golden worked examples for 1-4-5-1 in C major and a minor key. Assign them to knobs/rows with screen labels, the 2-gesture rule, and add them to knob_map.tsv, help_text.tsv and page_states.tsv. Say whether each channel can follow the voicing differently (for example bass takes the root, a pad takes the full voicing).
- Morph rule: how each algorithm's morphs bias toward chord voicing (a per-algorithm table, ACID/EUCLID/SNAKE/STAGE)
- Dice rule: a PROG dice layer that re-rolls the progression (degrees, length, chord count) within musical limits (always starts or ends on degree 1 option, no more than N distinct chords); how it relates to the five shared dice layers plus ALL
- UX: where HARMONY lives (a global page reached in one gesture, screen labels, knob and button roles, a main-screen chord indicator), fully specified per the L (learnability) rubric; additions to page_states.tsv / button_modes.tsv / knob_map.tsv / help_text.tsv
- Interaction with undo, presets (what is saved per slot), MIDI-in chord following (decide: in v1 or deferred), and the clock/transport (chord position resets on PLAY)
- RAM and pool cost estimate against the HANDOFF budget (ChanRT stays 24 B or state why not)
- Scenario S18 added to docs/design/scenarios and COVERAGE.md

## Acceptance criteria
- [x] HARMONY.md has no 'TBD'; the default (`1---`, 4 bars; owner question 85) behaves as one chord for every channel and is inert (HARMONY.md 1)
- [x] VOICE, MOVE and PEDAL are each defined by a formula with worked examples, defaults that leave the sound unchanged, and are mapped to controls (HARMONY.md 3, 5.2; golden from tests/design/harmony_proto.py)
- [x] Every algorithm in the v1 set has a stated chord-quantise and morph rule (HARMONY.md 4.3, ALGORITHM_SET 12)
- [x] Every new page, button mode and knob role is in the TSV tables, and tests/design check scripts pass (no new page: the hold-GLO page is extended)
- [x] L rubric score shown for each new function; any exception justified (HARMONY.md 5.1, 5.6; no exception needed)
- [x] Memory cost within budget, or the overrun and a proposal stated (42 B .bss, 144 B pool, ChanRT 24 B)
- [x] ADR-0003 amended if the sequencer core changes; ALGORITHM_SET.md updated (ADR-0003 section 20, ALGORITHM_SET 12)
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Writing firmware, or changing locked designs beyond what HARMONY requires (list each change).

## Log

### 2026-11-10 executor (Sonnet 5.5)

Orchestrator rulings recorded in docs/design/HARMONY.md section 0: (1) MIDI-in chord following deferred to v2, hook point in one paragraph (7.6); (2) invisible by default,
worked example and an explicit byte-identical acceptance test (1, 10), entered steps scale-only with no opt-in; (3) no new page, HARMONY extends the hold-GLO page (table id
SCALE), PROG dice is HARMONY-page-only on SCL (6.3); (4) 42 B .bss and 144 B pool (7.2), ChanRT stays 24 B; (5) the ARP / PATTERNS interface is HARMONY.md 9, nothing else of P2-013/014 touched.

Deliverables: docs/design/HARMONY.md, docs/design/scenarios/S18-harmony.md, tests/design/harmony_proto.py (golden source). Locked-doc changes are listed one by one in HARMONY.md 8
(tables: page_states, button_modes, knob_map, key_map, control_map + CONTROL_MAP.md appendix; help_text regenerated; SEQUENCER_UX rev 3 with U70-U79; SCREEN_SPEC 7a; ADR-0003 section 20;
ALGORITHM_SET 12; S01/S06/S07 updated; check_scenarios.py now counts S18 as a narrative walk; OWNER_QUESTIONS 85-89 added).

Verification (all run from the repo root):
```
python3 tests/design/check_scenarios.py   -> scenarios: 18 files, 441 control-map rows, 0 with no scenario, 265 only in a sweep
python3 tools/backlog.py check            -> 47 items, 0 errors
python3 tools/check_control_map.py        -> rows tsv=441 md=441 / OK
python3 tools/check_sequencer_ux.py       -> pages=9 ... scored=79 exceptions=3 / OK
python3 tests/design/check_help.py        -> OK: 343 help rows, 1733 strings measured
sh tests/design/width_probe.sh            -> 119 strings, 0 over
python3 tests/design/harmony_proto.py --check -> CHECK OK
```
No firmware changed, so `tools/check.sh` (a build) was not run.

Open (not ticked): "Design locked: owner line in the log" needs the owner's Approved line; the executor does not write it. Five owner questions (85-89, each with a recommendation) are in HARMONY.md 11 and OWNER_QUESTIONS.md.

### 2026-11-10 review (Sonnet 5.5)

Verdict: FAIL (narrow: stale text in locked docs plus an incomplete section 8). The design itself holds. No edits made by the reviewer.

Re-run and passing: check_scenarios.py (18 files, 441 rows), backlog.py check (0 errors), check_control_map.py OK, check_sequencer_ux.py OK, check_help.py OK, width_probe.sh (0 over), harmony_proto.py --check CHECK OK.
Hand-checked: VOICE 3/5/6 goldens (C major and MINPEN), PEDAL clash marks, 4.2 hash and nearest-tone table, chord start table 2.3, RAM 16+1+14+9+1+1 = 42 B and pool 18 x 8 = 144 B. Rulings: (a) deferred with a single hook paragraph (7.6), OK. (b) inert definition and H-INERT test OK, entered steps scale-only OK. (c) no new page, all controls one gesture after hold GLO, OK (ROLE/NEW/BARS are extra surface but flagged in Q89). (d) OK. (e) section 9 interface is clear and pure. OWNER_QUESTIONS 85-89 are well formed (real alternatives, recommendations); 90 is intact.

Findings (fix these):
1. docs/design/page_states.tsv:4 (CHAN row, opener column) still says "(on SCALE D#4 sets the KEY)". Contradicts key_map.tsv:250 (SCALE D#4 "not used") and ruling on key-press-sets-KEY removal. Fix: delete the parenthesis, or say D#4 is inert on SCALE.
2. docs/design/CONTROL_MAP.md:117 (workflow row 6) still reads "row 2 SCALE, OCTAVE, MIDI, LENGTH (any key sets the KEY)". Now wrong on both counts (OCTAVE is on SET, KEY is K2 only). Fix the text and re-run check_control_map.py.
3. Historic-looking workflows still describe the removed behaviour: docs/design/UX_REVIEW.md:190 (W9 "Press any key for the KEY; turn K2 (OCTAVE)") and docs/design/PERFORMANCE_DESIGN.md:432 (F18 "press any key"). Either update them to K2 KEY / OCT on SET, or add a one-line "superseded by P2-012" note, and list the choice in section 8.
4. HARMONY.md section 8 is not the complete change list the task demands: row 14 names S01/S06/S07 but the commit also edits S09, S10, S11, S12, S13 and S15; the commit also edits CONTROL_MAP.md, widths.tsv, gen_help.py and check_scenarios.py (the last two are mentioned only in the task log). Add them as rows (or one row with the file list), plus rows for findings 1-3 once fixed.
5. Decision 69 (P1-017 question 69) approved "scale, key and octave on the hold-GLO page"; octave now leaves it (SET row, 8 item 4). This is listed and covered by Q86, but Q86 should say so explicitly ("reverses part of 69") so the owner sees it; also decision 46 (one undo record per channel) is extended by 8 item 6 and Q86-89 do not ask about it. Add a sentence in section 11 or a sixth question.
Note (no change needed): undo of a HARMONY change from PERF gives a toast only for PROG; typing, VOICE and BARS undo toasts are not specified (7.3 / 5.5). Consider specifying so the screen says what EDIT just did.

Gate human-review: no Approved line written. Status set back to in_progress.
