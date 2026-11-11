---
id: BUG-004
title: P3-008 rehearsal patch does not apply to HEAD (core.h UP_SLOTS comment and felucca_web.c upreset include changed by P3-011)
phase: P3
type: bug
status: done
priority: 1
executor: sonnet
depends_on: [P3-008]
gate: none
severity: medium   # high: wrong notes, crash, a core feature broken · medium: a control misbehaves · low: cosmetic or latent
found_in: 0f080ac (HEAD when P3-008 was run)
---
## Goal
Fix: docs/design/rehearsal/P3-008.patch must apply cleanly to HEAD (`git apply --check` exits 0), so that P3-008 can run
its deliverable as written. Either regenerate the rehearsal patch against HEAD, or confirm the owner's intent for the two hunks.

## Read first
- docs/design/rehearsal/P3-008.patch (hunks for firmware/src/core.h:18 and web/emu/felucca_web.c:202)
- backlog/tasks/P3-008-remove-the-drum-and-phys-engines-r6a.md (Deliverable step 1: stop and file a BUG if the check fails)
- backlog/tasks/P3-011-*.md (the P3-011 change that removed upreset.c and rewrote the UP_SLOTS comment)

## Deliverable
- A patch (or a corrected rehearsal patch) that `git apply --check` accepts on HEAD, with the same drum/phys removal effect.
- No hunk other than the drum/phys removal is added.

## Steps to reproduce
1. `cd` to the repo at HEAD 0f080ac.
2. `git apply --check docs/design/rehearsal/P3-008.patch`

## Expected
Exit 0, no output.

## Actual
```
error: while searching for:
#define UP_SLOTS 32u             /* user presets (upreset.c) */      <- HEAD has: "removed in P3-011: the constant stays for favorites.c"
error: patch failed: firmware/src/core.h:18
error: firmware/src/core.h: patch does not apply
error: while searching for:
#include "../../firmware/src/upreset.c"                             <- P3-011 deleted this include
error: patch failed: web/emu/felucca_web.c:202
error: web/emu/felucca_web.c: patch does not apply
```
All other files in the patch (engines.c, midi_control.c, params.c, seq.c, settings_persist.c, ui.c, voice.c, tests/*) pass `--check`.

## Acceptance criteria
- [ ] `git apply --check docs/design/rehearsal/P3-008.patch` exits 0 on HEAD.
- [ ] `sh tools/check.sh` passes after P3-008 is applied on top of the corrected patch.

## Verification
```sh
git apply --check docs/design/rehearsal/P3-008.patch
```

## Log

### 2026-11-11 orchestrator (sonnet)
- Cause confirmed: the rehearsal patches were made before P3-011. core.h:18 (UP_SLOTS comment) and web/emu/felucca_web.c:202
  (upreset.c include) drifted. Fixed by regenerating P3-008, P3-009 and P3-010 in order in scratch copies of HEAD d03c6c8
  (the real tree's firmware was not touched). Same patch format (plain unified diffs, delete lists unchanged).
- Running the whole suite also found that the rehearsal patches left real test breakages that P3-008 and P3-009 would have hit at
  `check.sh`: P3-008 (hostsim clap-into-DRUM-lane check, audio_test drum overload case, mod_test PHYS cost test and
  `MOD_COST_MAX` 6.0 -> 7.0, goldens) and P3-009 (`MIDI_CC_ENG` rows of the removed engines, goldens); P3-010 needed hostsim
  fixes (poly pad, ANALOG lead, mix cost block, `xfade_test` dropped). All are now in the patches; the tasks say so.
- Verified chain from HEAD: apply 008 + deletes, 009 + deletes, 010 + deletes gives the same tree as the scratch run; full
  `tests/run_tests.sh` passes after 008 (GOLDEN_UPDATE applied once, only `song/*` + removals + regenerated union_golden changed),
  after 009 and after 010 (ALL HOST TESTS PASSED); all tests/*.c syntax-compile (uac_test and usb_desc_test need -D flags as before).
- Build lines: 008 `image 347112 B; RAM .data+.bss 34340 B; pool 144720 B`, 009 `image 189808 B; RAM 26676 B; pool 144052 B`,
  010 `image 169084 B; RAM 23060 B; pool 92500 B`. The old rehearsal numbers were 16 B smaller in the image (P3-011 base shift).
- Residual acceptance-grep hits (comments, gen_tables.py FM6 tables) are listed in the P3-008 and P3-009 tasks for the executor.
- Result: `git apply --check docs/design/rehearsal/P3-008.patch` exits 0 on HEAD. Done.

