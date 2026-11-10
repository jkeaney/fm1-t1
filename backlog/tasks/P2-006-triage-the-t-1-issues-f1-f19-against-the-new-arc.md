---
id: P2-006
title: Triage the T-1 issues (F1–F19) against the new architecture
phase: P2
type: task
status: done
priority: 2
executor: sonnet
depends_on: [P2-004]
gate: none
---
## Goal
Each open T-1 defect either becomes a BUG item (the code survives into the new design) or is dropped (the code is being replaced).

## Read first
- docs/T1_ISSUES.md
- docs/project/decisions/ADR-0003-sequencer-core.md (what of the T-1 code is reused)
- backlog/_templates/bug.md

## Deliverable
- One backlog/bugs/BUG-nnn-*.md per surviving issue, with its F-number in the title and repro copied from T1_ISSUES; a line in T1_ISSUES.md under each issue: 'Tracked as BUG-nnn' or 'Dropped: <reason>'

## Acceptance criteria
- [x] Every OPEN or PARTIAL issue in T1_ISSUES.md has a 'Tracked as' or 'Dropped' line
- [x] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
grep -cE 'Tracked as BUG-|Dropped:' docs/T1_ISSUES.md
python3 tools/backlog.py check
```

## Out of scope
- Fixing the bugs.

## Log

### 2026-11-10 executor (sonnet)
Triage by ADR-0003 section 13 (reuse/rewrite/drop) and section 19 / TIMING_SPEC. Survive: F2 -> BUG-002, F3 -> BUG-001. Dropped (with reasons citing ADR-0003 s13): F5, F6, F7, F8, F10, F11, F12, F14-F17, F19; F9 and F14 and F13's timing rows link TIMING_SPEC / ADR-0003 s19. F1, F4, F18 are FIXED and untouched.
```
$ grep -cE 'Tracked as BUG-|Dropped:' docs/T1_ISSUES.md
16        (= every OPEN/PARTIAL issue: F2, F3, F5-F17, F19)
$ python3 tools/backlog.py check
42 items, 0 errors
```
Judgement call for the reviewer: F13 (docs vs code) marked Dropped; its one surviving residue (stale `range` comment in torso_t1_algo.h) was folded into BUG-002. F12 dropped on the grounds that the preset store (ADR-0003 s10) replaces T-1 persistence.

### 2026-11-10 review (sonnet)
Verdict: PASS (gate none -> done).
- Verification re-run: grep count 16 (= F2, F3, F5-F17, F19; F1, F4, F18 untouched, diff is additions only); `backlog.py check` 42 items, 0 errors.
- Read ADR-0003 s13, s8.2/8.4, s19 and TIMING_SPEC M1-M5; checked 12+ Dropped reasons (F5, F6, F7, F9, F10, F11, F12, F13, F14, F16, F17, F19) against the s13 table rows and the sources. F10: the new design keeps mute phase (ADR-0003 s7.3 step 4), so dropping is right. F9: pair-or-nothing and note-off guarantee are in s8.2/8.4. F12: SeqPreset/preset store is s10. F13: guide/plan are superseded; timing rows covered by TIMING_SPEC/s19; stale `range` comment confirmed present at torso_t1_algo.h:99 and carried by BUG-002.
- BUG-001 (F3) and BUG-002 (F2): repro copied from T1_ISSUES, phase P3 (matches s16 prerequisites 4 and 3, correctly overriding the template default P2), front matter valid, depends_on empty. T1_SCALE_NAMES confirmed unused (declared algo.h:75, defined algo.c:242).
