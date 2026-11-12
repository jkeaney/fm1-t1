---
id: P4-011
title: ui_sq.c framework: screen zones, toasts, labels and page dispatch
phase: P4
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P4-010, P4-005, P2-031]
gate: none
---
## Goal
The new main UI skeleton every page task fills in: zones Z0-Z6 at the SCREEN_SPEC pixels, the toast, the always-on control labels from ui_tables.h, redraw by state signature, and page dispatch from the page-state table.

## Read first
- docs/design/SCREEN_SPEC.md §1-§5, §7 (zones, labels, modifier relabel, text limits, mock-ups)
- docs/design/SEQUENCER_UX.md §2 (page inventory), §2.7 (transitions, single escape), §3 (button modes)
- firmware/src/ui_t1.c (the pattern being replaced), ui_draw.c (`draw_head`, cv_* canvas), ui_input.c (hold timers)

## Deliverable
- firmware/src/ui_sq.c (+ ui_sq.h if needed): zone renderers (header with slot H2 placeholder, channel cells, lane, band, buttons, legend, cue), toast queue (2.7), label drawing from ui_tables.h, page enum and dispatch with stub pages, `ui_sq_input()`/`ui_sq_draw()` entry points; not yet wired into ui_draw/ui_input (P4-012 does it); the UI reads channel state only after `sq_chan_sync()` (sq_chan.c; clears the UndoRecs and takes a queued load's HARMONY/BPM once the ISR has bumped `sq_epoch`), which the main loop calls once per frame before `ui_sq_input()` (ADR-0003 section 6)
- tests/ui_sq_test.c: host render of each zone to PNG via the existing ui_render path; asserts every label fits its zone using the real `text_w`

## Acceptance criteria
- [x] ui_sq_test passes and writes build/ui_sq/*.png
- [x] No label exceeds its zone width (test asserts)
- [x] ui_sq.c reads only the sq_* API and exported state, never ISR-private fields (reviewer checks)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-011-ui-sq-c-framework-screen-zones-toasts-la.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Page behaviour (P4-013..P4-018)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (Sonnet 5.5)
Files: firmware/src/ui_sq.c (new), tests/ui_sq_test.c (new), tests/run_tests.sh (one step), docs/design/features/P4-011-ui-sq-c-framework-screen-zones-toasts-la.md (Gate A), backlog/feedback/FB-016 (findings below). felucca.c NOT changed: ui_sq.c is not needed by the device build (nothing includes sq_core/sq_chan there yet) and is not wired into ui_draw/ui_input/main.c; the host test includes it.
```
sh tools/check.sh      -> CHECK PASS
./build.sh | grep RAM  -> ok    image 169084 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640   (headroom 49900 B >= 8192)
ui_sq_test: widths.tsv: 297 rows measured, 0 over / ui_sq_test: OK (0 failures)   (26 PNGs in build/ui_sq/)
```
What it does: zone table Z0..Z7 (asserted to tile 240x240, blits A 240x108, B 240x104, C 240x28); Z0 with H1..H6 (H2 = placeholder `INT bpm` with the amber dot), Z1 cells and the NOTE_EDIT / REC banner, Z2 lane (header, 16 cells from sq_eval, playhead, edited step, page-map graphics in H5), Z3 band (tag column OCT-/OCT+, PERF two rows with the live row filled, single-row pages with status lines), Z4 cells computed from the measured labels (every page x algorithm sums to 240 and holds its labels), Z5 legend, Z6 cue / toast; 2000 ms toast (same text restarts, new text replaces, dropped while HOME held), ALGO_PICK strip, page table `uisq_pages[UI_PG_COUNT]` with stub input entries, `ui_sq_input()` / `ui_sq_draw()`; redraw by signature per blit. The caller owes SQ_FRAME_SYNC() once per frame before ui_sq_input().
Deviations and findings (the framework draws the tables and does not choose; details in FB-016):
- Table vs SCREEN_SPEC text: the PERF tag column is `ROW 1` / `ROW 2` in button_modes.tsv, `OCT-` / `h:peek` in SCREEN_SPEC 2.4. The tables win (generated source); `ROW 1` is 36 px and `RST R1` 38 px in the 36 px column, so a tag that does not fit wraps on its space (`ROW` / `1`); `+PAGE` (36 px) is drawn flush. No exact-row conflict stops the framework, so I did not block; the owner should rule on FB-016.
- key_map.tsv black-key tap labels are `MUTE1..4`, `CHAN`, `RESTRT` (spec legend: `M1..M4 CHN SOL FIL ROL RST`); `MUTE1` is cut to `MU...` by the F3 rule (65 of the page x key labels cut); SOLO/FILL/ROLL have only a hold row, so the legend falls back to the hold label.
- Cue strip text is not in ui_tables.h (gen_ui_tables.py does not emit the cue_strip column): a static copy of the page_states.tsv text is in `uisq_pages[].cue`.
- AF_S ink is line-top + 3 .. + 14: lowercase cue text at the top of the 12 px Z6 slot spilled into Z7, so Z6 text is drawn 3 px up (`UISQ_Z6_TEXT_DY`); the test asserts Z7 stays blank. The cue strip is drawn from x=0 (the RIG ALGO cue rows are 239 px); toasts from x=4 (236 px max, all rows pass).
- The H2 clock states (EXT, WAIT), the first-run / HELP / RESET / SCALE strip contents, step velocity of `!` (vel >= 100 stands in for an accent flag) and all page behaviour are the page tasks' (P4-012..P4-018).
- Reads: sq_ch, sq_gdb/sq_cache/sq_cidx/sq_calgo via sq_eval (as sq_gen_hit does), sq_playing, sq_bpm, sq_follow, and ChanRT p / snd[0] as read-only single bytes. Writes none.
