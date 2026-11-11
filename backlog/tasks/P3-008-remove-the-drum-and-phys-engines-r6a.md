---
id: P3-008
title: Remove the DRUM and PHYS engines (R6a)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-007, P2-023]
gate: none
---
## Goal
Remove the drum kit and physical models (sound design, not preview).

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R6, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: eng_drum.c, drum_voice.c, eng_phys.c, phys_dsp.c, phys_symp.c
- Edit callers: voice.c `ENGI_DRUM` branch, seq.c `drum_track()`, ui_input.c DRUM lines, engines.c table rows, params.c pages for them
- Tests: drum_test, phys_test, phys_ref; regress golden lines for these engines removed
- tests/target_budget.py + target_budget.txt: delete the removed functions (phys_render, drum_render, px_modal_block, px_modal_run, px_memb_block, px_string_excite, px_string_run, px_symp_run, dv_metal_run, dv_kick_run, dv_snare_run, dv_clap_run, dv_hat_run, dv_tom_run, dv_rim_run, dv_bell_run, dv_cym_run, dv_out, dv_metal_mix) from `FUNCS` and their rows from target_budget.txt; a function missing from the disassembly makes the budget step FAIL ("not found"). No BUDGET_UPDATE of other rows (FB-004 A).
- tests/run_tests.sh: the drum_test, drumfuzz (if still present) and phys_test runs and their header comments; tests/ui_test.c and tests/regress.c name eng_drum/drum_voice: grep and edit. Check LICENSING.md rows that list eng_drum/drum_voice/phys_* (licence of a removed file; update the table, do not leave a dead path).
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
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
