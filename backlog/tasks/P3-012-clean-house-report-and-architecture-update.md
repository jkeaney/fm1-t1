---
id: P3-012
title: Clean-house report and architecture update
phase: P3
type: task
status: done
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
- [x] CLEAN_HOUSE_REPORT.md exists with one row per step R1-R7
- [x] `grep -nE 'ui_layer|song_chain|editor\.c|eng_drum|upreset|eng_mem|eng_fm6|perform\.c' CLAUDE.md BUILDING.md` returns nothing
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

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

### 2026-11-11 executor (claude-haiku-5-5)
Verification run on HEAD 97b93ce / fefe7ab (BUG-001, BUG-002 in):
- `./build.sh | grep RAM` (first run): `ok    image 168876 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640`
- `sh tools/check.sh` (first run): `CHECK PASS` (tests: `ALL HOST TESTS PASSED`; regress `24 golden renders (0 changed, 0 gone)`; backlog stage ok).
- Acceptance grep: `grep -nE 'ui_layer|song_chain|editor\.c|eng_drum|upreset|eng_mem|eng_fm6|perform\.c' CLAUDE.md BUILDING.md` -> no output, `grep rc=1`.
- Second `sh tools/check.sh` after the doc edits: `CHECK PASS`; its build line `ok    image 168892 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640`. The image varies by 16 B between builds of the same tree (same as P3-011); RAM and pool do not.
- `python3 tools/backlog.py check`: `131 items, 0 errors`.
- Headroom: 73728 - 23060 = 50668 B >= 8192 B, so no E9 FB is filed.
- Inventory re-run: `objdump -h/-t` (Docker, toolchain at /opt/jieli) then `attrib.py`, `cov.py`, `deps.py`, `gen_table.py`; output in build/inventory/ (git-ignored). Totals .text+.ram_text+.data 168892, RAM 23056, pool 92500.

What was done (docs only, no code):
- Created docs/design/CLEAN_HOUSE_REPORT.md: one row per step (R1, R2, R3a/b, R4, R5a/b, R6a/b/c, R7) with build before/after, measured freed, SYSTEM_MAP section 9 prediction and difference; totals that add up exactly for RAM and pool (flash within 16 B of the P3-001 baseline); the final build line; the ADAPT inventory table; residue list; the FB-005 open item.
- CLAUDE.md: include order rewritten from `felucca.c`; the audio ISR, MIDI-out and UI-structure paragraphs; `song.grid` (values 1 and 3 belong to removed modes); T-1 always on; host tests link the real sources (P3-001) instead of stubs; BUILDING/report pointers. `ui_layer`, `ui_name`, `editor` and `ui_draw_page` order updated.
- BUILDING.md: release list (no sample attribution is produced), FELUCCA_FLASH row (no presets), removed FELUCCA_SLICE row (the flag is gone from felucca.c and build.py), emulator storage description, test list (no DRUM/PHYS/FM6/SLICE/chord/FX/reverb/DaisySP tests).
- HANDOFF.md 'Budget facts': measured headroom and pointer to the report.

Not done, on purpose:
- web/emu: edited in P3-006/009/011 to drop deleted references; Emscripten is not installed, so the emulator build is UNVERIFIED.
- CREDITS.md left as history (the audit said keep the names; owner's call).
- Code residue (seq.c song.grid 1/3 branches, comments naming removed code, gen_table.py ROWS, tools/build.py line 10) is listed in the report for v2. No code changed.
- FB-005 open item: decision 114 (OWNER_QUESTIONS, 2026-11-11) already drops the installer's SysEx backup flow. Measured behaviour (static reading only; not run on a device or in a browser): the firmware has no backup handler, so the page's first request times out ("Backup timed out ...") and the "does not support" text is not reached. Recommendation and follow-up in CLEAN_HOUSE_REPORT section 5.

Reviewer: please check the R5b pool (+5026 over the bus figure) and the 16 B flash gap to the P3-001 baseline (both noted, not explained); and the BUG-001/002 attribution of -208 B (inferred from commit order, not rebuilt at 29c1b2b).

### 2026-11-11 review (sonnet)
Verdict: PASS (gate none), set to done. Checked independently:
- `sh tools/check.sh` = CHECK PASS; `./build.sh`: `ok    image 168876 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640`. Headroom 50668 B (>= 8192). build/felucca.fwsc produced (610086 B). `git status` clean.
- Per-step table: every before/after build line matches the Logs of P3-001..P3-011 and P3-014 (chain is continuous; flash steps sum to 354124, + 208 BUG + 16 baseline gap = 354348; RAM and pool sum exactly). Acceptance grep returns nothing. Include order in CLAUDE.md matches felucca.c includes; no removed subsystems described as present in CLAUDE.md or BUILDING.md.
- R5b pool +5026 over 145042: 145042 = dly_buf 131072 + rev_comb 9874 + cho_buf 4096 only. 338167d also removes the `hl` and `ins[NTRK]` .pool state (task text names "HALL/INSERT state"), which accounts for the difference. Plausible, the report says "not split"; not byte-verified.
- 16 B gap (523224 vs 523208): this is the known 16 B image wobble between builds of the same tree, not a real difference.
- -208 B for BUG-001/002: I rebuilt 29c1b2b from `git archive`: `image 169068 B; RAM 23060; pool 92500`. HEAD builds 168876. Like-for-like the two T-1 edits freed 192 B, not 208 (208 mixes in the 16 B wobble: the P3-010 Log's 169084 was a wobbled build). Minor doc inaccuracy; the report marks it inferred; RAM and pool conclusions unaffected. Optional tidy: change 208 to 192 in CLEAN_HOUSE_REPORT section 1.
- FB-005: section 5 records decision 114 (OWNER_QUESTIONS row 114 confirmed, "decided") and a concrete follow-up scope, but no backlog task file exists yet for it; the orchestrator should file one after P3-013.
- Web emulator marked UNVERIFIED as required.
