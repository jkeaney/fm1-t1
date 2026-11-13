---
id: P6-005
title: Keybed scan: DRIVE mode, calibration, LED blanking and the FORCE row
phase: P6
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P4-039, P2-015]
gate: none
---
## Goal
Read velocity and pressure from F3-C5 reliably, per SEQUENCER_UX §6.2-§6.4 and the reference ports, with a test-build debug readout for HT-FORCE.

## Read first
- docs/design/KEYBED_EXPRESSION_REFERENCE.md
- docs/design/SEQUENCER_UX.md §6.2-§6.6
- docs/design/KEYBED_USE_CASES.md (P2-015)
- firmware/hal key scan (fm1_input.h) and the scan ISR
- firmware/src/ui_sq.c

## Deliverable
- Scan ISR: DRIVE mode (weak drive every other sample), per-key state, learned floors (decay 1 count/s), HARD bins, H1 LED blanking while sampling (H2/H3 behind flags), MASTER pot sharing kept
- Menu rows FORCE (OFF/LIGHT/NORMAL/FIRM, default OFF until HT-FORCE passes) and KEYS CAL
- `FELUCCA_FORCE_DEBUG` test-build readout page
- Host tests of the bin logic and calibration maths with recorded count traces

## Acceptance criteria
- [ ] Bin/calibration host tests pass
- [ ] With FORCE OFF every function behaves exactly as before (all ui_sq tests unchanged)
- [ ] Scan ISR cost measured by `felucca_dbg` in the Log
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P6-005-keybed-scan-drive-mode-calibration-led-b.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: dependency changed from P6-004 to P4-039 (LED-1): LED blanking builds on the LED frame; P6-004 is now LED-2.
