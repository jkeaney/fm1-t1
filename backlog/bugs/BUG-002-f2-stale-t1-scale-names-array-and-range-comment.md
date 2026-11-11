---
id: BUG-002
title: F2 - stale T1_SCALE_NAMES array mislabels the scale masks
phase: P3
type: bug
status: done
priority: 3
executor: haiku
depends_on: [P3-001]
gate: none
severity: low
found_in: bea0fd3 (+ working tree, T1_ISSUES review 2026-11-09)
---
## Goal
Fix: remove the stale `T1_SCALE_NAMES` so that `T1_SCALE_MASKS` is the one scale table and nothing mislabels the masks.

## Read first
- docs/T1_ISSUES.md, entries F2 (residue) and F4 (header comment note)
- firmware/src/torso_t1_algo.c (line ~242) and torso_t1_algo.h (line ~75)
- docs/project/decisions/ADR-0003-sequencer-core.md section 13 (`T1_SCALE_MASKS` reused as the one scale table, delete `T1_SCALE_NAMES`) and section 16 prerequisite 3

## Deliverable
- Delete `T1_SCALE_NAMES` (declaration and definition; it is referenced nowhere). Correct the `range ... in octaves (0 to 7)` comment in `torso_t1_algo.h` to semitones. The new names live with the sequencer scale table (ADR-0003 section 5), not here.

## Steps to reproduce
1. Read `T1_SCALE_NAMES`: it lists the old order (Chromatic, Major, Minor, Dorian, Mixolydian, Maj Pent, ...), which no longer matches the masks.

## Expected
No second, disagreeing names table exists.

## Actual
`T1_SCALE_NAMES` is declared and defined but unused, and its order is wrong.

## Acceptance criteria
- [x] `grep -rn T1_SCALE_NAMES firmware tests` finds nothing.
- [x] `tools/check.sh` passes.

## Verification
```sh
grep -rn T1_SCALE_NAMES firmware tests
sh tools/check.sh
```

## Log
Created by P2-006 (triage of F2: `T1_SCALE_MASKS` and its file survive, so the residue is tracked). May be done inside ADR-0003 prerequisite 3.

### 2026-11-11 executor (claude-haiku-5-5)
Changes: removed the `T1_SCALE_NAMES` definition from `firmware/src/torso_t1_algo.c` and its declaration from
`firmware/src/torso_t1_algo.h`. Changed the header `range` comment from "in octaves (0 to 7)" to "in semitones".
No new names table was added here, as ADR-0003 section 13 requires.

Commands and output:
```
$ grep -rn T1_SCALE_NAMES firmware tests
(no output, exit 1)
$ sh tools/check.sh
== build (log: build/check/build.log)
== ram headroom
RAM .data+.bss 23060 B of 73728
== tests (log: build/check/tests.log)
== backlog (log: build/check/backlog.log)
CHECK PASS
```
Decisive lines: `ALL HOST TESTS PASSED` (tests.log), `131 items, 0 errors` (backlog.log). The word "fail" in tests.log
only matches test names such as "erase failure keeps old data".

### 2026-11-11 review (sonnet)
Verdict: PASS (gate none). Reran `sh tools/check.sh`: CHECK PASS. `./build.sh`: `RAM .data+.bss 23060 B of 73728` (headroom 50668 B, above 8192).
Findings: `T1_SCALE_NAMES` is gone from firmware, tests, tools and web; remaining mentions are only docs/backlog text describing it as stale or deleted (docs/design/*, ADR refs), which is accurate history. The UI uses its own table (`torso_t1_ui.c:23`, CHROM..), so nothing used the deleted array. Header comment now says semitones, matching `torso_t1_algo.c:325` (`max_semitones = range`). Scope: only the 3 expected files changed. docs/T1_ISSUES.md F2 set to FIXED and Counts updated (5 fixed, 1 partial); the "AEOL duplicates MINOR" note is a design point, not part of F2's mislabel defect.
