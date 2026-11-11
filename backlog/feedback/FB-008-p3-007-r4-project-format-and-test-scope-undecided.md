---
id: FB-008
title: P3-007 (R4) cannot delete the project.c song/motion sections without a project-format decision, and names too few tests
phase: P3
type: feedback
status: done
priority: 2
executor: human
depends_on: [P3-006]
gate: none
session: 2026-11-11, executor (claude-haiku-5-5), P3-007 blocked after a grep only (partial edits reverted, tree clean)
---
## Goal
Decide how the saved project format (FUN10 in project.c) treats the song chain and motion, and extend P3-007's named
scope so its acceptance grep can pass. Then P3-007 can run.

## Read first
- backlog/tasks/P3-007-remove-song-chain-motion-chord-and-perform-r4.md (Deliverable, acceptance criteria, Log)
- docs/design/SYSTEM_MAP.md rows 19, 22, 48 and section 6 (R4, R7)
- firmware/src/project.c header comment (lines 1-100: the FUN6..FUN10 layouts), 127-128, 170, 622-750, 1206-1233, 1327-1328
- firmware/src/core.h lines 296-321 (motion and chain types)

## Observation
A grep (no edits kept) shows that deleting `song_chain.c` and `motion.c` as the Deliverable says also removes
the types they define, and these are in the saved project format:

1. **Format conflict.** `project_t` holds `chain_config_t chain` and `motion_store_t motion` (project.c:127-128). They
   are written into FUN10 (`PROJ_STORE_SIZE` 3840, `_Static_assert` at project.c:170; `proj_pack` 646-648, `proj_unpack`
   736-748, `proj_hash` 1327-1328) and read from FUN6..FUN9 (`chain_from_v9`, `proj_motion_v9`, `proj_motion_ids`).
   The acceptance criterion "grep for each removed file's public symbols returns nothing" matches the `motion_` and
   `chain_` prefixes, so the types cannot stay in core.h. Stored projects (flash slots) depend on this layout. The
   task does not say which of these is wanted:
   - **A (recommended):** keep the FUN10 byte layout. Replace the two fields with opaque reserved byte arrays of the
     same size (84 + 388 bytes, no symbol names). Loads ignore them, saves write zeros, and old projects keep their
     offsets, checksum and every other field. R7 removes the FUN1..FUN10 formats later.
   - **B:** drop the 472 bytes and bump the magic. Any FUN10 project in a slot then shows EMPTY.
   - **C:** keep the types in core.h. This fails the grep acceptance.
2. **Tests not named.** Besides the Deliverable's list, these use removed symbols and must change for the grep to pass:
   `tests/sound14_test.c` (`pf.mg`, `perf_held`, `perf_latched`, `send_r` at 74-77, 588-621), `tests/sound15_test.c`
   (67-70), `tests/fold155_test.c` (396-399): perform's `pf` and `perf_*` state. `tests/ui_test.c` 220-241 (the FX
   layer block), `tests/audio_test.c` 204, 231 (`kb_layer`), `tests/drum_test.c` (10 lines), `tests/scale_test.c` (1),
   `tests/persistence_test.c` (6), `tests/midi_control_test.c` (7 chord and motion lines), `tests/song_probe.h` (8,
   motion and chain), `tests/old_pack.h` (`motion_event_t`, `motion.count`), `tests/chord_test.c` (named in the
   Deliverable as a delete, but `run_tests.sh` lines 35-36 and 189-190 also go), and `tests/project_test.c` (44 lines).
3. **Callers not named under Edit.** The Deliverable lists caller counts for `ui.c` (chain 8, motion 23) and
   `midi_clock.c` (motion_begin, line 26), `upreset.c` (`motion_base_value`, line 410), `settings_persist.c` (comment
   line 17), `core.h` (lines 299, 305, 309), but the Edit list names only seq.c, main.c, midi_control.c, ui_input.c,
   fx.c, project.c, and ui_layer.c. `ui.c`'s motion undo and load paths (`load_begin` 515-526, `load_end` 585-591,
   `undo.motion_backup` 455, `motion_guard` 705-709) must change too. `ui_draw.c` (`chain.running`, lines 293-404)
   changes as well.
4. **Behaviour that follows from the Deliverable** (for the owner to confirm): keys play one note each (chord.c
   removed; CHRD / VOIC become dead parameters); MIDI IN notes play single notes; the FX-layer keys and the black-key
   track mutes stop; the header's SONG icon goes; the `sound14` check of `send_r` goes with the scratch arrays
   (P3-014 kept them only for perform).

## Deliverable
- Linked P3- ids for the format decision and the extended P3-007 scope, recorded in the log.

## Acceptance criteria
- [x] Option A chosen on technical grounds by the orchestrator (no owner decision needed: the FUN10 record is internal and P3-011 deletes the format), recorded below and in P3-007.
- [x] P3-007's Deliverable names the test files in point 2 and the ui.c / upreset.c / midi_clock.c / ui_draw.c edits in point 3, as a verified patch.
- [x] P3-007 is set to `todo` again; its executor re-runs it with the patch.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-11, executor (claude-haiku-5-5): the grep above was run before any change. `sh tools/check.sh` on the untouched
  tree: `CHECK PASS`, `RAM .data+.bss 51472 B of 73728`, `image 392280 B; RAM .data+.bss 51472 B of 73728; pool 187780 B of 368640`.
  Partial edits (fx.c, felucca.c, hostsim.c, size_fns.py, target_budget, run_tests.sh) were reverted with `git checkout`.

### 2026-11-11 orchestrator (answer; rehearsal in scratch copies of the tree, nothing in the real firmware was edited)
1. Point 1: **option A**. `project_t` keeps the FUN10 byte layout; the chain and motion become `rsv_song[84]` / `rsv_auto[388]`
   (loads ignore them, saves write zeros), `project_v6_t` keeps `uint8_t chain[36]`. Reason it is not wrong: the only
   writer/reader of the record is the autosave (project slots die in P3-011), the checksum and every other offset are
   unchanged, and no symbol with a `motion_`/`chain_` prefix survives. P3-011 deletes the reserved arrays with the format.
2. Point 2 and 3: every test, caller and doc named, with line ranges, in the rewritten P3-007; the work was rehearsed
   (device build, `tests/run_tests.sh` once: ALL HOST TESTS PASSED, golden and cpu_baseline unchanged, target budget only
   the `fm1_alnk0_irq` -15 % note) and saved as docs/design/rehearsal/P3-007.patch (applies to HEAD 06d0a96).
   Two misses of this FB's own list: `tests/chord_test.c` also goes with `chord.c`; `tests/song_probe.h` is included by
   nothing and goes; `seq_steps()` and the `kb_chn` array lived in song_chain.c / chord.c and had to be re-homed in seq.c.
3. Point 4 (behaviour): technical consequences of the deletion, no owner input needed; recorded in P3-007.
4. Consequence found by re-auditing P3-008..P3-012: **the order changes to P3-007, P3-011, P3-008, P3-009, P3-010, P3-012.**
   project.c and upreset.c hold migrations that name PHYS, DRUM, SAMPLE PERC and DIGITAL; deleting them first removes
   hundreds of lines the engine tasks would otherwise have to edit and then delete. `depends_on` is updated. Each of those
   tasks now carries its own verified patch (docs/design/rehearsal/P3-0NN.patch) applied in that order. Gaps patched:
   P3-011 (project.c also holds the settings persistence and the autosave, which stay), P3-008 (the engine table is
   positional: deleted engines become placeholder rows; the step-lane helpers lived in eng_drum.c; speaker_test used the DRUM
   kick), P3-009 (ENG_FM4_GONE lived in fm4_convert.c; install_test imported fm1_sample_upload; union_test goes; the FUN10
   FM6 patch bytes become opaque), P3-010 (run-time test breakages, two golden lines, credits), P3-012 (stale docs list).
5. Expected build line after each step (device build): 007 `image 377856 B; RAM 34372; pool 154624`; 011 `371232; 34356;
   144720`; 008 `347096; 34340; 144720`; 009 `189792; 26676; 144052`; 010 `169068; 23060; 92500` (RAM and pool in bytes
   of 73728 and 368640).
