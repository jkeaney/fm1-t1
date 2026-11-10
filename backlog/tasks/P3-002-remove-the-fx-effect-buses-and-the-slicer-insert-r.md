---
id: P3-002
title: Remove the fx effect buses and the slicer insert (R5, first per decision 80)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-001]
gate: none
---
## Goal
Free ~178 KB of pool (fx buses 145042 B + slicer 32768 B) first, so the 16 preset slots, HARMONY, ARP and PATTERNS fit (owner decision 80).

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R5, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: firmware/src/slicer.c; the DIST/INSERT, chorus, delay and reverb bus code and their `.pool` buffers inside fx.c
- Edit callers: fx.c: the slicer call sites (`slicer_busy`, `slicer_track`, `slicer_track_sd`) and the perform calls at the effect stage (perform.c itself goes in P3-007); keep `mix_part`, `beat_samples()`, master level, SPEAKER EQ (`fx_lowcut`, `spk_bass`), USB level
- Tests: delete slicer_test, insert_test, reverb_test and the slicer part of swing_test from tests/run_tests.sh; `GOLDEN_UPDATE=1 sh tests/run_tests.sh` and `BUDGET_UPDATE=1 sh tests/run_tests.sh` once, and the Log lists which golden lines changed (only cases that used the removed effects may change)
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] `beat_samples` and `mix_part` still exist in fx.c
- [ ] Pool used drops by at least 170000 B against the pre-task build line
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
