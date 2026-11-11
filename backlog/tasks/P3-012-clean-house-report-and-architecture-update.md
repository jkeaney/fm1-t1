---
id: P3-012
title: Clean-house report and architecture update
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-010, BUG-001, BUG-002]
gate: none
---
## Goal
Record what P3 freed and leave CLAUDE.md describing the cleaned firmware, so P4 starts from true numbers.

## Read first
- docs/design/SYSTEM_MAP.md §9 (predicted bytes)
- docs/design/inventory/ (the scripts that measured the inventory)
- CLAUDE.md (Architecture section)
- git log of P3-002..P3-011 and P3-014 (their Log build lines)

## Deliverable
- docs/design/CLEAN_HOUSE_REPORT.md: a table of predicted vs measured flash/RAM/pool freed per removal step, the final build line, and the re-run inventory of ADAPT rows
- CLAUDE.md Architecture and include-order text updated to the files that remain (no mention of removed engines, editor, layers, chain)
- docs/project/HANDOFF.md 'Budget facts' updated with the measured headroom

- **Audit of 2026-11-11 (rehearsal of P3-007..P3-011; these docs still describe removed code after those tasks):**
  BUILDING.md line 49 (the sample attribution), 64 (the `FELUCCA_SLICE` row), 80-81 (browser emulator: projects, user
  presets, user sample slots), 92-97 (the test list: DRUM/NOISE/PHYS/FM6/SLICE, chord keys, FX layer, reverbs, SLICER,
  DaisySP); CREDITS.md lines 7-16 and 21 (PHASE waveforms, DRUM voices and Sample Pack, samples, VOICE, PHYS, FM6, SLICE
  contributors stay as history if the owner wants them: keep the names, mark the code as removed); CLAUDE.md include-order text
  (lines 110-111 still name `ui_layer`, `storage/ota/editor`, engines/voice/fx) and the UI paragraph at line 135 (`ui_layer.c`);
  HANDOFF.md 'Budget facts'. The final build line to expect after P3-007, P3-011, P3-008, P3-009, P3-010 in that order is
  `image 169068 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640` (headroom 50668 B); report the measured one.
- CLEAN_HOUSE_REPORT.md: the order of the steps was R4 (P3-007), R7 (P3-011), R6a/b/c (P3-008..P3-010), with the build line
  after each in the Logs; use the rehearsal numbers in the Logs of those tasks for predicted-versus-measured. State that
  web/emu (the browser emulator) was edited to drop deleted references but is UNVERIFIED (no emcc here) and that the
  FUN10 record, the settings object and the autosave are what persistence is left (P7-002 replaces the record).
- Report one open item for the owner (FB-005): web/index_pkg.html + fm1backup.js still offer a SysEx backup before returning to official V15; the cleaned firmware no longer answers it. State the measured behaviour and a recommendation.

## Acceptance criteria
- [ ] CLEAN_HOUSE_REPORT.md exists with one row per step R1-R7
- [ ] `grep -nE 'ui_layer|song_chain|editor\.c|eng_drum|upreset|eng_mem|eng_fm6|perform\.c' CLAUDE.md BUILDING.md` returns nothing
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Code changes
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).
- Patch fallback: if `git apply --check` fails only on comment or context-only hunks (no code line differs), you may apply with `git apply --3way` (or `patch` with fuzz) and must list in the Log which hunks differed. Any failing code hunk is still a stop: file a BUG (as BUG-004, BUG-005).

## Log
