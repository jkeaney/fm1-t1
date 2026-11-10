---
id: P3-001
title: Host tests link the real seq, ui and T-1 sources
phase: P3
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P2-005]
gate: none
---
## Goal
Owner decision 42 and ADR-0003 §16.1: today the host tests build seq.c/ui*.c with `t1_mode` stubbed to 0, so none of the T-1 path is tested. Make the host tests compile the real T-1 sources (torso_t1*.c, t1_glue.c, ui_t1.c) with host hooks, before any removal, so every later removal is guarded by tests.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §13 (last row), §14
- docs/design/SYSTEM_MAP.md §7 (last paragraph), §8
- tests/run_tests.sh, tests/ui_test.c, tests/torso_phase*_test.c
- firmware/src/seq.c lines around the `#ifndef T1_DEVICE` stubs (t1_block, t1_mode), firmware/src/ui.c stubs

## Deliverable
- A host-hook header tests/host_t1_hooks.h (or equivalent) supplying what the T-1 needs from the device (midi_out_event capture, beat_samples) so `seq.c`, `ui*.c` and the T-1 files link in host tests
- The `#ifndef T1_DEVICE` stubs in seq.c/ui.c replaced by the real symbols in host builds (keep the guard only where a device-only header is involved)
- A new host test tests/t1_link_test.c: run `events_block` for 2 bars with T-1 on, assert that channel-2 note-ons reach the captured `midi_out_event` log, and that stop emits CC123
- tests/run_tests.sh step for it

## Acceptance criteria
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] `grep -n 't1_mode' firmware/src/seq.c firmware/src/ui.c` shows no `#define t1_mode 0` stub left
- [x] tests/t1_link_test.c passes and is in run_tests.sh
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Any removal (P3-002 onwards)
- Changing T-1 behaviour
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-10 executor (sonnet)
- Added tests/host_t1_hooks.h (real torso_t1*.c + t1_glue.c, T1_HOST_GLUE) included by tests/hostsim.c after seq.c; ui_t1.c added to tests/ui_test.c and web/emu/felucca_web.c. seq.c/ui.c stubs removed (declarations unconditional); torso_t1.c weak synth stubs skipped under T1_HOST_GLUE. midi_out_event capture = usb.c's own midi_out_q ring with usb.config = 1.
- tests/t1_link_test.c (in run_tests.sh): 2 bars, `t1_link: 2 bars: ch2 note-ons 15, note-offs 14`, nothing on other channels, STOP CC123 on ch 2, `all checks ok`.
- `sh tools/check.sh` -> `CHECK PASS` (tests.log: ALL HOST TESTS PASSED)
- `grep -n 'define t1_mode' firmware/src/seq.c firmware/src/ui.c` -> no match
- Build line: `image 523224 B; RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640` (headroom 17984 B >= 8192)

### 2026-11-10 review (sonnet)
Verdict: PASS (gate none).
- Re-ran `sh tools/check.sh`: CHECK PASS (tests.log: ALL HOST TESTS PASSED).
- Build line: `image 523224 B; RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640` -> headroom 17984 B (>= 8192).
- seq.c:60 and ui.c: no `#define t1_mode 0` stub left; declarations unconditional. torso_t1.c change only skips weak stubs under T1_HOST_GLUE (no behaviour change on device).
- tests/t1_link_test.c in run_tests.sh and passing: ch2 note-ons 15 reach captured log, nothing on other channels, STOP sends CC123.
- Scope: only hooks/tests/stub removal changed. Note: unrelated uncommitted P4-008 edits in the working tree (run_tests.sh timing self-test) were present during the run; not part of this commit.
