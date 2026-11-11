---
id: P3-007
title: Remove song chain, motion, chord and perform (R4)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-006]
gate: none
---
## Goal
Remove the groovebox sequencing features after cutting their callers (SYSTEM_MAP §5 edges 5, 9, 16).

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R4, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: firmware/src/song_chain.c, motion.c, chord.c, perform.c
- Edit callers: seq.c (`chain.running`, `chain_start/stop`, `chain_motion`, `motion_begin/end`, `perf_key` lines named in SYSTEM_MAP rows 15/19/22), main.c `chain_defaults`, midi_control.c `mchord_of` (row 21: adapt first), ui_input.c remnants, fx.c perform calls (`perf_begin`, `perf_pre`, `perf_master`, `perf_block`, `perf_mute*`, `perf_throw`, `pf.*`, the `send_d`/`send_r` scratch arrays P3-014 kept), seq.c `perf_key*`/`perf_start` (660-720, 772-817, 852), ui_input.c:411,1488,1533-1538, ui_layer.c remnants, project.c song/motion sections. perform.c also owns the 32768 B `pf_buf` pool buffer that P3-002 gave it: it goes with the file
- Tests (perform_test was already deleted in P3-002): motion_test, chord_test, seq155_test, fun10_test, ratchet_test, the song/motion parts of project_test
- tools/size_fns.py: remove "song_chain.c", "motion.c", "chord.c" from `SIZE_FILES`. **A deleted file still listed there crashes ./build.sh** (FileNotFoundError); found in FB-005.
- tests/target_budget.py + target_budget.txt: delete the removed functions (perf_begin, perf_mute, perf_pre, perf_block, perf_master, perf_flanger, perf_phaser, perf_throw) from `FUNCS` and their rows from target_budget.txt; a function missing from the disassembly makes the budget step FAIL ("not found"). No BUDGET_UPDATE of other rows (FB-004 A).
- tests/hostsim.c line ~40: delete `#include "../firmware/src/song_chain.c"` (every host test that includes hostsim.c depends on it); tests/ui_test.c names song_chain/chord; tests/run_tests.sh has the chord_test and motion_test runs (and header comments) to delete; tests/motion_test.c also names ui_events (P3-006 deletes that file first).
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] Pool used drops by at least 32000 B (pf_buf)
- [ ] P3-001's tests/t1_link_test.c still passes
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
