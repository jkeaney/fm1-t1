---
id: FB-002
title: P3-002 deliverable names too few callers to delete slicer.c and the fx buses
phase: P3
type: feedback
status: done
priority: 2
executor: human
depends_on: [P3-001]
gate: none
session: 2026-11-10, HEAD df9568a, source inspection by executor (haiku); no build run
---
## Goal
Decide how P3-002 reaches its acceptance criteria. The deliverable and SYSTEM_MAP row 13/14 name only fx.c as the caller to edit. The source has more callers, so the task as written cannot delete slicer.c and keep the build green.

## Read first
- docs/design/SYSTEM_MAP.md §4 rows 13-15, §5 edge 7, §6 R5
- backlog/tasks/P3-002-remove-the-fx-effect-buses-and-the-slicer-insert-r.md

## Observation
Callers of slicer.c symbols outside fx.c (`slicer_start`, `sl_pattern`, `sl[]`, `sl_buf`, `sl_lent`, `sl_enter`, `P_SLCR..P_SLDEPTH`):
- `firmware/src/fx.c:229` includes `perform.c`. perform.c uses `sl_buf` (perform.c:440-591), `sl[]` and `sl_lent` (perform.c:213, 229-231, 552-554). perform.c is removed only in P3-007, so after slicer.c is deleted the unity build breaks unless perform.c is also cut loose from fx.c.
- `firmware/src/seq.c:851` calls `slicer_start()`. Not in the deliverable.
- `firmware/src/ui_graph.c:483-486, 1222-1223, 2295` (graph_slicer, the GR_SLCR page) use `sl_pattern`, `sl[]` and `P_SLCR`. Not in the deliverable.
- `firmware/src/midi_control.c:194` writes `sl[track]`. Not in the deliverable.
- `firmware/src/core.h:57`, `params.c:129-132, 454`, `ui.c:1269` and `tests/upreset_test.c:284-289` define and use the P_SL* parameters. Removing them renumbers the parameter table (upreset_test asserts `P_SLCR == 45`), which touches the project format and is not named.
- Test references in `tests/ui_test.c`, `tests/ui_render.c`, `tests/swing_test.c`, `tests/perform_test.c`, `tests/midi_control_test.c`, `tests/hostsim.c`, `tests/regress.c`, `tests/sound14_test.c`.

Also, the fx.c effect-bus code is entangled with perform.c's THROW macros (perform.c:395-420 write the sends), and `core.h:181, 381-383` (`fx[4]`, `dist_*`, `tail`, `ins_run`) plus `mod.c` MD_DIST..MD_REV are the DIST/INSERT state. The deliverable does not say whether those fields and the P_DIST..P_REV / P_ITYPE..P_IMIX params are removed. The acceptance grep ("returns nothing") would require removing them.

## Deliverable
- Owner answers to questions A-D, recorded in the log, with P3-002 or SYSTEM_MAP amended to match, or the task re-scoped.

## Questions for the owner
A. Pull P3-007 (perform.c, R4) forward into P3-002, or stub the perform include out of fx.c for now?
B. Who edits seq.c `slicer_start`, ui_graph.c GR_SLCR and midi_control.c `sl[]`: add them to P3-002's deliverable, or to a new task?
C. Are the P_DIST..P_IMIX params and their project/format slots removed in P3-002 (renumbering P_SLCR and upreset_test), or kept as dead slots until a format task?
D. Acceptance grep: does "nothing" include `tests/` files that P3-002 deletes (swing_test slicer part, perform_test, ui_test cases), with the rest deferred?

## Acceptance criteria
- [ ] Owner answers A-D, and the P3-002 deliverable or the SYSTEM_MAP row is amended to name every caller above.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-10 executor (haiku): P3-002 set blocked on this finding. No files deleted or edited.
- 2026-11-10 orchestrator (sonnet): verified every listed caller with grep; also found `fx.c` users outside the list: `ui_graph.c` `graph_insert` (uses `ins_shape/ins_bits_sh/ins_mixq`), `params.c`/`tests/ui_test.c` comments naming `delay_samples`, `tests/{fold155,mod,sound14,sound15}_test.c` memset the fx buffers, `perform_test`/`reverb_test` use `fx_buses`/`master_out`, and `perform.c` also uses `pf.ew1` inside `fx_buses`. No product decision changes (decision 80 order kept: slicer, then buses, before every other removal).
  - A. Neither pull P3-007 forward nor stub the include (seq.c, ui_layer.c and ui_input.c need perform.c's symbols). P3-002 gives perform.c its own 32768 B `pf_buf` pool buffer in place of `sl_buf` and drops its slicer hooks; perform.c, `pf_buf` and the `send_d`/`send_r` scratch in fx.c go in P3-007. Perform_test is deleted in P3-002 (it needs the slicer and buses).
  - B. Add them to P3-002's deliverable: `seq.c:851`, `midi_control.c:194`, `ui_graph.c` graph_slicer/GR_SLCR, plus the named test edits. The INSERT graph is in the bus task.
  - C. Kept as dead slots: all `P_DIST..P_REV`, `P_SLCR..P_SLDEPTH`, `P_ITYPE..P_IMIX`, `G_RTYPE..G_RDAMP`, `MD_DIST..MD_REV` and midi_control CC 91/93/94, so no renumbering (`upreset_test` `P_SLCR == 45` stays true; project format untouched). Pruned in the v2 params prune. The `track_t` fields `dist_*`, `tail`, `ins_run` are removed in P3-014 with `rsv_sz` grown so `sizeof(track_t)` is unchanged.
  - D. "Nothing" in the grep includes tests: the tests that exercise the removed code are deleted (slicer_test, perform_test in P3-002; insert_test, reverb_test in P3-014) and the other test files are edited by exact lines named in the tasks. Only the P_* slot names are exempt, as they stay.
  - Result: P3-002 re-scoped to the slicer; new P3-014 (fx buses, depends_on P3-002, haiku); P3-003 now depends_on P3-014; P3-006 also removes the dead FX/SLICER/INSERT/REVERB page rows; P3-007 names the perform callers and `pf_buf` and no longer lists perform_test; P3-011 lists the dead slots as out of scope; P3-012 and PROGRAM_PLAN updated; SYSTEM_MAP rows 13-15, edge 7, R5 and section 8 amended. P3-002 set back to todo.
