---
id: P4-044
title: Channel in-use persistence: start with CH1 only
phase: P4
type: task
status: review
priority: 2
executor: sonnet
depends_on: []
gate: none
---
## Goal
Power-on shows only the channels that exist: CH1 in use, the others inert until their algorithm rows exist, and the 'channel in use' state is stored with the preset so P7 can restore it.

## Read first
- /tmp/readiness_review.md finding 7
- firmware/src/ui_sq.c ~115 (in-use state is UI-only)
- firmware/src/sq_chan.c ~535-539 (inert empty rows) and the SeqPreset code ~586-666
- docs/project/decisions/ADR-0003-sequencer-core.md section 22 (SeqPreset layout)
- backlog/feedback/FB-014-cutover-null-algo-rows.md

## Deliverable
- 'Channel in use' becomes core state (a bit per channel in a struct that is already saved or has room, with the size assertion still true); first boot starts with CH1 only
- Adding a channel is possible only for channels with a filled algorithm row; the behaviour is a table read (`ALGOS[]`), not a name check (ADR-0007)
- Tests: power-on state, add and remove channel, a save/load round trip of the bit

## Acceptance criteria
- [x] Power-on host test shows 1 channel in use
- [x] The in-use bit survives the SeqPreset round trip in the host test
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing but small: update docs/design/features/P4-015-chan-page-*.md Gate A rows for the new power-on state; Gate B on the unit with the next flash.

## Out of scope
- Flash storage (P7)
- New algorithm rows
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).

### 2026-11-13 executor (sonnet)
Layout: no size change. In use = bit 0 of ChanSave.pad[1] (was reserved, ChanSave 384 and SeqPreset 1556 B unchanged). Working copy is core state `sq_used` (sq_chan.c; ui_sq.c's `uisq_used` is now a macro for it). Default: CH1 = 1, CH2..CH4 = 0 (still muted, algorithms EUCLID/SNAKE/STAGE).
- sq_boot: `sq_used = 0x01`. sq_preset_save writes the bit per channel from `sq_used`; sq_preset_load keeps bit 0 of pad[1] (was zeroed); the landed load (sq_chan_sync) rebuilds `sq_used` from the staged bits: a channel whose ALGOS[].build is NULL is dropped, an empty mask becomes CH1. A queued load changes the mask only at the bar. ui_sq_glue sq_frame, after a landed load, clears the remove-undo stack and moves the selection onto a channel in use.
- Add (SELECT past the last) takes the first free channel with `sq_chan_addable()` = ALGOS[algo].build != NULL (table read). Remove/undo (uisq_do_remove, EDIT) unchanged: they edit `sq_used`; CH1 may be removed while another is in use (as before).
- CH1 audible with ACID at boot and the others inert: sq_link_test still passes (2 bars on MIDI channel 1 only). Existing UI tests set `uisq_used = 0x0F` after sq_boot.
- Tests: sq_chan_test t_inuse (power-on, addable, save/load round trip byte-identical, queued load at the bar, hostile preset, 20000-step add/remove/algo/load fuzz), ui_sq_perf_test (power-on one channel, add CH2, remove, last never removed, EDIT undo, 4000 random SELECT/remove/EDIT/key steps: mask valid, selection in use, stack bounded, removed channels silent).
Hazards checked: array bounds (mask & 0xF0, sel % SQ_NCH, gone_n <= 4: fuzz); unused-slot guards (addable, remove of unused: perf test); stale state after remove/add/load (gone stack cleared, selection moved, ui_sq_glue); early returns (none skip cleanup); flags across page change (unchanged).
Docs sync (grep "in use", "all four channels", "power-on", "pad[1]"): ADR-0003 section 12 defaults row, SCREEN_SPEC_V2 section 6, P4-015 feature Gate A step 0, sq_types.h comment.
Not done: nothing known. P7 persistence will carry the bit with the preset.
```
sh tools/check.sh  -> CHECK PASS
image 216276 B; RAM .data+.bss 29640 B of 73728; pool 78520 B of 368640   (headroom 44088 B)
```
