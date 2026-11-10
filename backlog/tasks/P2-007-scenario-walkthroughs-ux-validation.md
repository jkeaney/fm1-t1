---
id: P2-007
title: Scenario walkthroughs: validate the UX design end to end
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P2-001, P2-003, P1-013, P1-014, P1-017]
gate: human-review
---
## Goal
Script the target workflow as step-by-step scenarios so the control map and screen spec are proven complete before development (ADR-0004).

## Read first
- docs/design/SEQUENCER_UX.md and the TSV tables page_states.tsv / button_modes.tsv / knob_map.tsv (P1-017: the locked sequencer functionality and UX; every page needs a screen spec)
- docs/design/PERFORMANCE_DESIGN.md (P1-014: performance-first controls; supersedes anything it changes in FULL_CONTROL.md)
- docs/design/FULL_CONTROL.md (P1-013: shift row, up to 8 morphs per algorithm, live two-row labels)
- docs/project/decisions/ADR-0004-design-lock.md
- docs/project/VISION.md
- docs/design/CONTROL_MAP.md and the screen spec from P2-001 and P2-003

## Deliverable
- docs/design/scenarios/S*.md, one per scenario, each a numbered table: step | user action | exact screen text | MIDI out | control-map row id
- docs/design/scenarios/COVERAGE.md mapping every control-map row to the scenarios that use it

## Acceptance criteria
- [x] Scenarios cover: TB-3 first-run flow; step entry on white keys; dialing a sequence with knobs; morph; dice per layer; switching algorithm; muting; save/load
- [x] Every control-map row appears in COVERAGE.md (rows with no scenario listed as gaps and sent back to P2-001)
- [x] No step contains 'TBD', 'choose' or 'e.g.'
- [x] Any menu-diving step (>1 press away) is flagged as a vision violation
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tools/backlog.py check
grep -rEn 'TBD|choose|e\.g\.' docs/design/scenarios || echo clean
```

## Out of scope
- Changing the locked designs, or touching firmware outside docs/design and tests/design.

## Log

### 2026-11-10 executor (sonnet)
Deliverables: docs/design/scenarios/S01..S17, COVERAGE.md, FINDINGS.md; tests/design/check_scenarios.py (checker and
COVERAGE writer) and tests/design/gen_sweeps.py (generates the sweeps S09..S17 from the locked tables). No firmware touched.
- S01 TB-3 first run, S02 dial in ACID and perform, S03 step entry / NOTE_EDIT / pages, S04 channels / algorithms / mute,
  S05 save / load, S06 clock (Ableton master, clock lost, FM-1 master with clock out), S07 SET / SCALE / REC,
  S08 RESET / HELP / MENU / first-run; S09..S17 one generated sweep per page context.
- Widths were measured with a probe built from firmware/src/gfx.c text_w and build/gen/ui_fonts.h (reproduces the 45/42/34/28 px of SCREEN_SPEC 6.2).
- Gaps are reported, not fixed: 19 defects F-01..F-19 and 4 owner questions Q1..Q4 in FINDINGS.md. The ones that matter
  most: F-01 cue rows 13/13a/13b can never show; F-03 no toast zone; F-04 default screen strings overflow their slots
  (`MIDI 2 MUTE` 68 px in a 60 px cell, `KEYS ONLY` 62 in 51, header H4/H5 with two-digit MIDI channels or 3-4 pages);
  F-06 no HELP text table; F-02 MENU control roles disagree between control_map and SCREEN_SPEC; Q1 whether two-gesture
  functions satisfy VISION rule 1 (no function needs the menu).
```
$ python3 tests/design/check_scenarios.py --write
scenarios: 17 files, 441 control-map rows, 0 with no scenario, 268 only in a sweep      (exit 0)
$ python3 tools/backlog.py check
42 items, 0 errors
$ grep -rEn 'TBD|choose|e\.g\.' docs/design/scenarios || echo clean
clean
```
- Coverage: every one of the 441 control-map rows is cited (0 gaps); 268 of them only by a page sweep, 173 also by a narrative scenario.
- Criteria: the last one ("Design locked: owner line") is left unticked. It needs the owner's own line; no Approved line was written.
  The scenarios do not change any locked table; the findings above need an owner decision and a re-lock before P2-005.
