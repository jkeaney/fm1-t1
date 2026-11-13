---
id: P4-044
title: Channel in-use persistence: start with CH1 only
phase: P4
type: task
status: done
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

### 2026-11-13 review (sonnet)
Verdict: FAIL (three code findings, one docs-sync finding). `sh tools/check.sh` -> CHECK PASS; `RAM .data+.bss 29640 B of 73728` (headroom 44088 B, >= 8192 B). Checked and fine: pad[1] bit use (ISR never reads it; sq_preset_load masks to bit 0; SeqPreset 1556 B and ChanSave 384 B unchanged; no other consumer of pad[1]); `#define uisq_used sq_used` is a plain alias, no shadowing; `sq_used` is a single byte, main-loop only (the ISR gates on `muted`, `midi_ch`, `pad[0]` only); queued load applies the mask in sq_chan_sync after the ISR lands it at the bar; addable() is a table read with the NULL-row guard; removal of CH1 allowed while another is in use; a landed queued load clears the remove-undo stack and moves the selection. Old presets (pad[1] was zeroed on load, never saved to flash yet) now load as CH1 only; no version bump, acceptable until P7, mention it in the P7 task.
- [x] F1 (BUG-011 class, breaks S01). Power-on CH2..CH4 are out of use but still hold MIDI 2..4 (sq_chan.c:553 `sq_default_chan`). The target workflow sets CH1 to MIDI 2 (S01 step 3): `sq_set_midi` (sq_chan.c:214-225) takes it from the out-of-use CH2 and `uisq_chan_commit` (ui_sq.c:1467) raises the toast `CH2 IS MIDI OFF: CH1 TAKES 2` about a channel that is not on the screen. BUG-011's rule was "not in use = holds no MIDI channel". Fix: out-of-use channels hold MIDI OFF (set in sq_default_chan for c > 0, and forced for every channel not in `sq_used` when a load lands, see F2); re-add already starts at OFF with the setup cue. Update t_defaults (sq_chan_test.c:390 asserts midi_ch == c+1) and ADR-0003 12 / SEQUENCER_UX U01 to match. (Alternative: skip out-of-use channels in sq_set_midi, but then re-add can duplicate a number; BUG-011 chose OFF.) Add a ui test: power-on, CH1 -> MIDI 2, no toast, CH2 added later has OFF.
- [x] F2 (hidden audible channel). A load never forces a not-in-use channel silent: sq_chan_sync/sq_used_clean (sq_chan.c:540-545, 691) leave `muted`, `pad[0]` bit 7 (PREVIEW) and `midi_ch` as loaded, and the ISR gates only on `muted` (sq_core.c:147). A hostile or zero-pad preset with CH2 `muted=0` therefore plays on MIDI and the built-in synth while the screen shows only CH1. Fix: in sq_chan_sync, after sq_used is set, for every channel not in use: `muted = 1`, `pad[0] &= 0x7F`, `midi_ch = SQ_MIDI_OFF`; add the case to t_inuse (preset with CH2 unmuted and no bit) and check no CH2 note-on is emitted.
- [x] F3 (stale state, latent until P7). The immediate load path (`when == 0` or stopped) calls `sq_chan_sync()` inside sq_preset_load (sq_chan.c:~668), which advances `sq_seen_epoch`; the next frame's `sq_frame` (ui_sq_glue.c:22-27) then sees `seen == sq_seen_epoch`, so the remove-undo stack is not cleared and the selection is not moved onto a channel in use. Only the ISR-landed (queued) path is covered by the tests. Fix: have sq_chan_sync set a `sq_load_landed` flag that sq_frame consumes (or compare `sq_used` to the last seen mask), and add a ui_sq_perf_test case that does an immediate load while CH3 is selected and CH3 is not in the loaded mask.
- [x] F4 (docs sync, stale after the change). docs/design/scenarios/S04-channels-algorithms-mute.md:3-4 and step 1 (line 8: SELECT one detent now ADDS CH2 with toast `NEW ALGO=PICK`, it no longer selects an existing muted CH2); S01-tb3-first-run.md:30 (step 3 toast and `CHB cell 2` text, change with F1); docs/design/SEQUENCER_UX.md:568 (U01) and :747 (power-on state still list CH2 EUCLID / CH3 SNAKE / CH4 STAGE as channels in use); check S02/S03/S05/S07 and docs/design/scenarios/COVERAGE.md for CH2..CH4 assumptions and regenerate anything generated from them. ADR-0003 12, SCREEN_SPEC_V2 6 and the P4-015 Gate A row 0 are correct.
Second round only needs to verify F1-F4 (and the hint the first-run strip still reads `CHN KEY, K2 = MIDI` on CH1).

### 2026-11-13 executor (sonnet), round 2 (F1-F4)
- F1: `sq_default_chan` gives CH2..CH4 MIDI OFF (CH1 MIDI 1); remove already sets OFF; ADD (ui_sq.c) sets OFF explicitly with the setup cue. Tests: ui_sq_perf_test (power-on MIDI values, CH1 -> MIDI 2 via the commit: no steal, no toast, CH2 still OFF; ADD gives OFF; remove then re-add gives OFF); sq_chan_test t_defaults now expects OFF. The older UI/core scenarios that assume four channels on MIDI 1..4 set that up in their world/boot helper (boot_mock, reset_world). First-run strip `CHN KEY, K2 = MIDI` untouched.
- F2: `sq_chan_sync`, after the mask is set, forces every channel not in use to muted, preview off, MIDI OFF. sq_link_test: a preset with an unmuted, preview-on, MIDI 6 ACID CH2 without its bit loads silent; 2 bars play nothing on MIDI channel 6.
- F3: `sq_load_landed` (set by sq_chan_sync) is consumed by `sq_frame`, so the clear of the remove-undo stack, tempo take and selection fix run on both the immediate and the queued path. sq_link_test: immediate load with CH3 selected and not in the mask -> stack empty, selection CH1, runs once. Mutation (flag removed) makes it FAIL.
- F4 docs: S04 (start state, step 1 adds CH2, step 2), S01 step 3 (no clash, no toast), SEQUENCER_UX U01, section 7 power-on and the reference-workflow sentence, ADR-0003 12 (CH2..CH4 MIDI OFF), SYSTEM_MAP boot line. Grep terms: `CH2 EUCLID`, `MIDI 2-4`, `MIDI 2 / SNAKE`, `CH2 held MIDI 2`, `IS MIDI OFF: CH1 TAKES`, `default MIDI channel of a channel`, `CH2 (default 2`, `default 2`. Left as is (correct for channels both in use): SEQUENCER_UX 4.4 / U40 / row 28, SCREEN_SPEC `CH2 IS MIDI OFF: CH1 TAKES 2`. Other docs/design hits are historic ui_review evidence, not specs. S02/S03/S05/S07/COVERAGE were not edited: no CH2..CH4 assumption found by those terms.
Hazards checked: unused-channel state after load/remove/add (F1-F3 tests, 4000-step UI fuzz, 20000-step core fuzz); stale stack/selection after load (F3); flags across paths (sq_load_landed cleared on consume).
```
sh tools/check.sh -> CHECK PASS ; RAM .data+.bss 29640 B of 73728 (headroom 44088 B)
```

### 2026-11-13 review round 2 (sonnet)
Verdict: PASS (gate none). Verified ad939c6 only. `sh tools/check.sh` -> CHECK PASS; `RAM .data+.bss 29640 B of 73728` (headroom 44088 B).
- F1: sq_default_chan gives CH2..CH4 MIDI OFF; ADD sets OFF (ui_sq.c:1940); remove already sets OFF; the S01 step 3 case (CH1 -> MIDI 2, no toast, CH2 untouched) is a test in ui_sq_perf_test. BUG-011 intact: the steal toast can no longer name an unused channel, the undo-of-removal restore rule (only if free) is untouched. Tests that assumed MIDI 1..4 now set it explicitly.
- F2: sq_chan_sync forces muted, PREVIEW off and MIDI OFF on every channel out of use after a landed load.
- F3: `sq_load_landed` is set in sq_chan_sync (both paths), consumed once in sq_frame (flag or epoch change); two loads before a frame give one cleanup, which is correct because the mask is the latest; test covers the immediate path and a second frame doing nothing.
- F4: S01, S04, SEQUENCER_UX U01 / power-on, ADR-0003 12, SYSTEM_MAP consistent. I fixed the remaining stale one-liners myself: SCREEN_SPEC.md:456, SEQUENCER_UX.md:482, 556, 1257, 1298 (decision 73 note). First-run strip text `CHN KEY, K2 = MIDI` still right for CH1 MIDI 1.
- Minor, non-blocking (P7 note): a hand-made preset where an out-of-use lower channel holds the MIDI number of an in-use one loads the in-use one as OFF (sq_preset_load uniqueness runs before the out-of-use reset). Presets saved by the device are consistent, so only hostile files hit it.
P4-041 status confirmed done.
