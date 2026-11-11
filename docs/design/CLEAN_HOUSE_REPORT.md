# Clean-house report (P3-012)

Measured 2026-11-11 on HEAD after `97b93ce` / `fefe7ab` (BUG-001, BUG-002), by `./build.sh` and `sh tools/check.sh`
(`CHECK PASS`). Sources: the build lines and before/after pairs in the Logs of P3-001..P3-011, P3-014, and the
predictions in [SYSTEM_MAP.md](SYSTEM_MAP.md) section 9 and the rehearsal lines in those Logs. Numbers are bytes.
Predicted values come from SYSTEM_MAP section 9 unless a row says otherwise. "n.a." means the step had no prediction for
that column.

## 1. Removal steps (R1-R7) in the order they landed

R5 and R6 were landed in parts (R5a, R5b, R6a, R6b, R6c), so there are ten rows for the seven steps. R8 (the T-1
state and screen, which are replaced by E4/E6/E7) is not done.

| Step | Task (commit) | Removed | Build before (image / RAM / pool) | Build after | Freed flash / RAM / pool (measured) | Predicted flash / RAM / pool | Difference (measured minus predicted) |
|---|---|---|---|---|---|---|---|
| R5a | P3-002 (`653bf22`) | slicer insert | 523208 / 55744 / 342876 | 520904 / 55584 / 342876 | 2304 / 160 / 0 | 1614 / 164 / 32768 | flash +690; pool 0 here, the buffer goes with R5b |
| R5b | P3-014 (`338167d`) | FX effect buses (DIST/INSERT, chorus, delay, reverb) | 520904 / 55584 / 342876 | 515492 / 55248 / 192808 | 5412 / 336 / 150068 | n.a. / <= 1682 / 145042 | pool +5026 over the bus figure; the Log does not split it |
| R1 | P3-003 (`e7283ec`) | DIGITAL engine, `FELUCCA_FM4` | 515492 / 55248 / 192808 | 515492 / 55248 / 192808 | 0 / 0 / 0 | 0 / 0 / 0 (not compiled) | none |
| R2 | P3-004 (`108ceb1`) | web editor (7 sources, its tests) | 515492 / 55248 / 192808 | 500212 / 54064 / 191048 | 15280 / 1184 / 1760 | 14962 / 659 / 1748 | flash +318, RAM +525, pool +12 |
| R3a | P3-005 (`7a3fe0b`) | menu rows 37 to 28, T-1 always on | 500212 / 54064 / 191048 | 499152 / 54048 / 191048 | 1060 / 16 / 0 | rehearsal 499168 / 54048 / 191048 | flash 16 B off the rehearsal |
| R3b | P3-006 (`3f12cad`) | groovebox UI files and page code (ui_tools, ui_layer, ui_name, ui_events, ui_slice, ui_fm6op, ui_graph, ui_help) | 499152 / 54048 / 191048 | 392280 / 51472 / 187780 | 106872 / 2576 / 3268 | six files 9096 / 96 / 2240, plus upper bounds for the page code in ui_draw 30508, ui_input 27802, ui_graph 26400, ui 8930, project 9057 | the measured flash is inside the sum of the bounds (111793 with the six files); pool +1028 |
| R4 | P3-007 (`25e2897`) | song chain, motion, chord, perform | 392280 / 51472 / 187780 | 377872 / 34372 / 154624 | 14408 / 17100 / 33156 | song chain 1302 + motion 3028 + chord 846 + perform 7254 = 12430 / 16078 / n.a. (rehearsal line 377856 / 34372 / 154624) | flash +1978 (callers edited in the same step); RAM +1022 |
| R7 | P3-011 (`5dbd09b`) | user presets, slice store, FUN1-FUN10 project formats | 377872 / 34372 / 154624 | 371248 / 34356 / 144720 | 6624 / 16 / 9904 | user presets 3230 + slice store 282 = 3512 / n.a. / 9888 | flash +3112 (project FUN code, bound 9057); pool +16 |
| R6a | P3-008 (`7a8b4eb`) | DRUM and PHYS engines | 371248 / 34356 / 144720 | 347112 / 34340 / 144720 | 24136 / 16 / 0 | PHYS 9329 + DRUM 14352 = 23681 / n.a. / 0 | flash +455 |
| R6b | P3-009 (`9ec4767`) | sample family, FM6 | 347112 / 34340 / 144720 | 189808 / 26676 / 144052 | 157304 / 7664 / 668 | sample 140537 + FM6 11507 = 152044 / 7444 / 525 | flash +5260, RAM +220, pool +143 |
| R6c | P3-010 (`29c1b2b`) | remaining synth engines (PHASE, LOFI, TRIO, WHEEL, VOICE, NOISE), `eng_mem`; ANALOG kept | 189808 / 26676 / 144052 | 169084 / 23060 / 92500 | 20724 / 3616 / 51552 | synth 17013 (not split in SYSTEM_MAP; ANALOG's share is not stated) / n.a. / eng_mem 51552 | pool exact; flash +3711 over the synth figure |
| (T-1 fixes) | BUG-001 `5ff01aa`, BUG-002 `97b93ce` | not a removal: phrase contour and scale name code edits | 169084 / 23060 / 92500 | 168876 / 23060 / 92500 | 208 / 0 / 0 | n.a. | attributed to the two T-1 code edits (inferred from the commit order; not rebuilt at `29c1b2b`) |

Totals, from the P3-001 Log baseline (`image 523224 B; RAM 55744 B; pool 342876 B`):

| | Start | End | Freed | Share | SYSTEM_MAP section 9 (E3 total from `c44670e`) |
|---|---:|---:|---:|---:|---:|
| Flash image | 523224 | 168876 | 354348 | 67.7 % | 234574 (44.8 %) |
| RAM .data+.bss | 55744 | 23060 | 32684 | 58.6 % | 28512 |
| Pool | 342876 | 92500 | 250376 | 73.0 % | 192211 |

The step rows add up to the totals: flash 354124 from R-steps plus 208 from the T-1 edits, and a 16 B gap to the
P3-001 baseline (its Log says 523224; the P3-002 "before" line says 523208; not explained). RAM and pool add up exactly.
The totals beat section 9 because section 9 counted only the named files and left the page code, the `eng_mem` pool and
the perform buffer as upper bounds or not at all.

## 2. Final build line

```
ok    image 168892 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640   (check.sh build, and the second ./build.sh)
ok    image 168876 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640   (first ./build.sh of this task)
```

The image varies by 16 B between builds of the same tree (the same wobble P3-011 recorded); RAM and pool do not vary.
The table in section 1 uses the 168876 figure for the total. RAM headroom is 73728 - 23060 = **50668 B**, above the 8192 B E9 trigger and the 4 KB floor. No FB is needed for RAM.
The `.ram_text` no-calls check and the register-access check pass in the same build. `sh tools/check.sh` ends `CHECK PASS`.
A rehearsal of P3-010 in the Log gave the same RAM and pool.

## 3. Re-run inventory of the ADAPT rows

Re-run on the cleaned build (`objdump -h` and `-t` on `build/felucca.elf`, then `docs/design/inventory/attrib.py`,
`cov.py`, `deps.py`, `gen_table.py`; the Docker `objdump` call is the one in SYSTEM_INVENTORY.md). Totals: `.text +
.ram_text + .data` = 168892 (the table sums to the section total), RAM 23056, pool 92500, `.noinit` 204. Sizes are symbol
bytes attributed to a file; "unattributed" rows are the rest.

| SYSTEM_MAP row | Subsystem (files) | Flash B | RAM B | Pool B | Note |
|---|---|---:|---:|---:|---|
| 4 | sound core (core.h, dsp.c, engines.c) | 374 | 7296 | 0 | RAM is `trk[]` |
| 10 | params (params.c) | 3601 | 0 | 0 | |
| 12 | voice (voice.c) | 2662 | 165 | 0 | |
| 14 | fx (fx.c) | 442 | 351 | 0 | `mix_part`, `beat_samples`, master, USB level |
| 20 | seq core (seq.c) | 6171 | 296 | 0 | still holds the song.grid 1 and 3 branches (section 4) |
| 23 | midi clock / control (midi_clock.c, midi_control.c) | 2600 | 2096 | 0 | |
| 25-27 | T-1 core (torso_t1_port, torso_t1, torso_t1_algo, torso_t1_pipeline, torso_t1_state) | 8534 | 392 | 13980 | one row in the inventory; pool is the `torso_t1.h` buffers |
| 28 | T-1 state (torso_t1_state.c) | in row above | | | REMOVE-ON-REPLACE (R8) |
| 29 | T-1 ui helper (torso_t1_ui.c) | 3660 | 9 | 0 | REMOVE-ON-REPLACE (R8) |
| 30 | T-1 glue (t1_glue.c) | 352 | 18 | 0 | |
| 31 | T-1 screen (ui_t1.c) | 42 | 6 | 0 | functions are inlined into ui_input.c and ui_draw.c (the inventory caveat) |
| 33 | panel (panel.c) | 173 | 6 | 0 | `.noinit` 32 |
| 34, 35, 37 | ui core (ui.c, favorites.c, midi_learn.c) | 1366 | 55 | 1517 | |
| 40 | menu (menu_items.c, ui_menu.c) | 2941 | 0 | 0 | |
| 41 | icons (icons.c, ui_icons.h) | 8167 | 0 | 0 | the generated icon table is 7551 of it |
| 42 | ui graphs (ui_graph.c, ui_fm6op.c) | 0 | 0 | 0 | files deleted (R3b) |
| 43 | ui draw (ui_draw.c; ui_help.c deleted in R3b) | 2823 | 0 | 0 | |
| 44 | ui input (ui_input.c) | 3336 | 41 | 0 | |
| 48 | project (project.c, settings_persist.c) | 656 | 6 | 9238 | |
| 53 | main (main.c) | 24520 | 1 | 0 | `.noinit` 4 |

v2 comparison: section 9 gave the mod matrix (a KEEP row) 1858 / 52 as v2 residue; measured 1774 / 4. The
inventory's `gen_table.py` still lists the deleted files (for example `category.c`, `ui_graph.c`, `ui_help.c`, `slicer.c`)
in its ROWS, so those rows show 0. Prune the list before the next inventory run.

## 4. What is left (v2 clean-up, not done here)

Code is out of scope for P3-012, so these are reported and not changed:
- **Live code, still in the build:** `seq.c` keeps the `song.grid` 1 (drum grid) and 3 (song slots) branches and
  `SONG_KEYS` (lines 110, 197, 704, 716), though no code sets grid 1 or 3 now. `core.h` keeps the `NENG_SHOWN`, `G_ENGSEL` and
  `G_ENGGO` entries; `params.c` keeps the retired DIGITAL ids 61..80 as inert values; `project.c` keeps the reserved song, motion and FM6 bytes
  of the FUN10 record (P7-002 replaces the record).
- **Comments naming removed code:** 53 lines in `firmware/src` and `firmware/hal` name the editor, `ui_layer` or motion,
  nearly all of them comments (for example `felucca.c` lines 12, 33, 86, 93; `storage.c` 22-33; `midi_learn.c` 4, 10;
  `ui.c` 9). `tools/build.py` line 10 still mentions "the sample attribution".
- **Persistence left after v1:** the settings object, the autosave, and the FUN10 project record (in `project.c`). P7-002
  replaces the record. The autosave and settings are the parts that stay.
- **Docs left as history:** `CREDITS.md` lines 7-16 and 21 still credit PHASE, DRUM, samples, VOICE, PHYS, FM6, SLICE and
  their contributors. They were kept on purpose (the names stay as history) and are not edited here; marking the code as
  removed is a one-line change if the owner wants it.
- **Web emulator, unverified:** `web/emu/felucca_web.c` (and `web/emu/emu_test.mjs`) were edited in P3-006, P3-009, P3-011
  to drop deleted references. Emscripten is not installed here, so **the emulator build is unverified**.
- **Removed earlier than SYSTEM_MAP said:** `ui_help.c` (section 43 said keep its HINT text for the T-1 and the menu) was
  deleted in P3-006; the HELP and some menu rows now have no reader (accepted by the P3-006 review). `category.c` was
  deleted in P3-011, not P3-006 as the P3-006 Log implies.

## 5. Open item for the owner (FB-005)

**Installer SysEx backup flow.** `web/index_pkg.html` and `web/fm1backup.js` still offer a complete SysEx backup before
the return to official V15 (`stockBackup`, `BackupConnection`, `captureBackup`).

- **Measured behaviour (static reading only; not run on a device or in a browser):** the firmware answers only the
  identity handshake (`cmd 0x11`) and read requests (`cmd 0x30`). `usb.c` `sysex_byte` and `ota.c` `ota_service` /
  `ota_read` have no backup handler. The page's first request (`[1, []]`, a 1200 ms default timeout) gets no reply, so
  it rejects with "Backup timed out. Keep the FM-1 connected and stopped." The "does not support complete backups" text
  (`stockUnsupported`) is reached only if a reply says so, which the cleaned firmware never sends. So the user sees a
  timeout error and no install starts.
- **Owner decision already recorded:** OWNER_QUESTIONS row 114 (2026-11-11): drop the installer's SysEx backup flow
  and keep the plain restore to official V15. P3-012 is code-free, so the change is not made here.
- **Recommendation:** a follow-up task removes the backup branch, `stockBackup` and the backup strings from
  `web/index_pkg.html`, the inlined copy in `web/make_site.py`, `web/fm1backup.js` and `web/test_backup.mjs`, and keeps the
  plain restore with the warning that official V15 cannot open Felucca music. Schedule it as the next task after the
  hardware check (P3-013).

## 6. Documents changed by P3-012

- `CLAUDE.md`: the include-order text, the audio ISR and MIDI-out paragraphs, the UI structure bullets, `song.grid`, the
  T-1 bullets (always on, host tests link the real sources), and the BUILDING.md and report pointers.
- `BUILDING.md`: the release list (sample attribution), the `FELUCCA_FLASH` and `FELUCCA_SLICE` rows, the emulator storage
  description, and the test list.
- `docs/project/HANDOFF.md`: "Budget facts" with the measured headroom and a pointer to this report.
- `docs/design/CLEAN_HOUSE_REPORT.md`: this file.
