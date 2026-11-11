---
id: P2-002
title: Decide keep / adapt / remove for every subsystem (system map)
phase: P2
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P1-002, P2-001]
gate: human-review
---
## Goal
Define the 'clean house' target: only UI, system, algorithms and setup that drive the vision remain (ADR-0001).

## Read first
- docs/design/SYSTEM_INVENTORY.md
- docs/design/CONTROL_MAP.md
- docs/design/ALGORITHM_SET.md
- docs/project/decisions/ADR-0001-fork-felucca-and-keep-t1.md

## Deliverable
- docs/design/SYSTEM_MAP.md: each inventory row marked KEEP / ADAPT (what changes) / REMOVE, with a reason; the removal order (leaves first, by the dependency column); the expected flash and RAM freed; the host test suites that go or change with each removal; which engine stays as the preview synth and why

## Acceptance criteria
- [x] Every SYSTEM_INVENTORY row has a decision
- [x] The platform-core list is all KEEP; the update loader and installer path are KEEP (ADAPT only strips groovebox code; loader/ota/installer untouched)
- [x] The removal order never removes something that a KEEP item still depends on (16 edges verified, section 5)
- [x] The owner approved it (log line)
- [x] SYSTEM_MAP has a 'v2' column (redefined by the coordinator update: v2 = clean-up of what v1 could not safely remove): each subsystem is KEEP, ADAPT, REMOVE (in v1) or v2-REMOVE (kept in v1 as the legacy groovebox mode per owner decision 32, removed in v2 epic E9)

## Verification
```sh
grep -cE 'KEEP|ADAPT|REMOVE' docs/design/SYSTEM_MAP.md
```

## Out of scope
- Removing code (that's P3).

## Log

### 2026-11-10 executor (sonnet)
- Mid-task coordinator update (supersedes owner decision 32): groovebox fully out of the UI in v1, no MODE row. Redid the map: groovebox-only subsystems are REMOVE in v1; v2 column = residue of ADAPT rows. Added section 3 "Boot and first-use". Recorded as a superseding note in OWNER_QUESTIONS.md.
- Created docs/design/SYSTEM_MAP.md. Verification: `grep -cE 'KEEP|ADAPT|REMOVE' docs/design/SYSTEM_MAP.md` -> 81.
- v1 REMOVE frees (firm, inventory sizes): flash 234574 B (image 523032 -> 288458), RAM 28512 B (headroom 17984 -> 46496), pool 192211 B. Upper-bound extras listed in section 9. v2 firm figure from the inventory is only mod matrix 1858 B / 52 B RAM; the rest is measurable after v1.
- Preview synth: ANALOG (index 0 and eng_idx fallback, 5746 B, no eng_mem).
- Hidden edges found (inventory said otherwise): seq->song chain, ui->T-1 screen, midi_control->chord, settings stored in favorites array, voice/seq->DRUM; false edges: voice->fx, slicer->fx.
- Menu rows 37 -> 28 (DISPLAY 8, CONTROL 4, AUDIO 6, MIDI 4, SYSTEM 6); ui_test asserts "8 8 6 8 7" become "8 4 6 4 6". CONTROL_MAP/UX_REVIEW still mention MODE SEQ/GROOVE and need a one-line update (not done: out of scope).
- Not run: build, full tests, flashing (design-only). Sizes are cited from the inventory, not re-measured.
- Owner questions 37-42 added to docs/project/OWNER_QUESTIONS.md: editor removal, console/USB audio, CHORD ENTRY row, preview patch/TUNE, MIDI learn, host tests linking the T-1.
- Owner approval: NOT given. Awaiting human-review.

### 2026-11-10 owner
Approved: James Keaney 2026-11-10 (answers to questions 37-42 recorded in docs/project/OWNER_QUESTIONS.md).

### 2026-11-10 review (sonnet)
Verdict: PASS on all checkable criteria; held at `review`. **Awaiting owner:** approve SYSTEM_MAP.md and answer owner questions 37-42 (below). Not set done; no Approved line written.

Verified independently:
- Coverage: scripted comparison of all 87 inventory rows and all 56 `.c` files in firmware/src against SYSTEM_MAP. Every row has a decision. Files not named literally (editor_*.c, six eng_*.c) are covered by grouped rows (50, 5b). The two "unmatched/unnamed bytes" rows are n/a, correctly.
- Platform core (libc, lcd, gfx, HAL, audio, usb, midi_uart, storage, ota, master, console) is KEEP. loader/installer untouched. `ota.c` has 0 refs to `ed_` (editor removable); `usb.c:152,794,876` -> ota stays.
- Numbers re-add: 523032-234574 = 288458; RAM 55744-28512 = 27232, headroom 73728-27232 = 46496; pool 342876-192211 = 150665; the 20 REMOVE row sizes sum to 234574; pool terms 145042+32768+9888+1748+2184+525+56 = 192211. All correct and firm (inventory sizes, not re-measured).
- Edges confirmed in source: seq.c -> chain (236,375,841,891,1077), ui_input.c:1517/ui_draw.c:1510 -> t1_input/t1_draw, midi_control.c:59,86 -> mchord_of (chord.c:185), settings blob `favorites.factory[15]` (favorites.c:38, settings_persist.c), voice.c:188 / seq.c:94,507 / ui_input.c:531,898 -> DRUM, fx.c:811-858 -> slicer/perform, voice.c:67,378,687 -> mod, main.c:102,246,286, t1_glue.c:28 trk_note_on, ui_draw.c:13-16 name_on/ui_help. Order R1-R8 is consistent with them (editor R2 before MI_ row removal R3 is right: editor_menu.c names MI_LATCH/MI_CHADD).
- Preview synth ANALOG: sound (index 0 fallback, no ENG_MEM refs, smallest). Boot defaults match UX_REVIEW section 2 line 184-186 exactly.
- Menu rows 37 -> 28: matches menu_items.c MI_TAB and ui_test.c:1658-1659. Caveat: 8 4 6 4 6 assumes CHORD ENTRY is removed; CONTROL_MAP still keeps it (5 CONTROL rows, 29 total) until question 39 is answered.

Findings (non-blocking, fix in the doc or carry into the R3/R6 dev tasks):
- [ ] Missed hidden edges in KEEP files: `settings_persist.c:76-92` (marked KEEP) names ENGI_DRUM/ENGI_SAMPLE/ENGI_FM6, `FM4_TO_FM6` (fm4_convert.c) and `core.h drum_from_perc`; `main.c:106 fm6_init()`; `voice.c:484,495 eng_mem_claim` (needed before `eng_mem` is deleted); `storage.c` OBJ_FM6BANK/OBJ_UPFM6/OBJ_FM6VB enum slots (do not renumber: flash layout). Add to R6 (settings_persist is ADAPT, not KEEP).
- [ ] Section 8 "stay" is optimistic: 49-52 host tests include the unity chain, so removing engines/groovebox will edit fixtures for most of them (audio_test, settings_test, persistence_test, midi_uart_test, input_test, storage_test), not only the ones listed. Plan for it in R3/R6.
- [ ] torso_phase2_test/phase3_test are not named per step (map says "1-3 follow the 4-channel model" without naming; assign them to E4).
- [ ] R4 deletes song chain/motion but `project.c` (FUN6/motion ids) still references them until R7; R4 must cut those references (or R7 must precede). Say so explicitly.

Doc fix done in separate commit 773f141: removed the MODE SEQ/GROOVE / legacy-groovebox / "SEQ mode" text from CONTROL_MAP.md and UX_REVIEW.md (control_map.tsv had none). `python3 tools/check_control_map.py` -> OK (343/343 rows).

Owner decisions needed (questions 37-42): 37 remove the web editor in v1 (rec yes); 38 keep CDC console and USB audio (rec keep); 39 remove CHORD ENTRY + chord.c (rec yes; fixes menu count at 28); 40 fixed ANALOG preview patch at boot, TUNE row stays?; 41 keep or drop MIDI learn / LEARN CLEAR (458 B); 42 allow host tests to link the T-1 sources as a prerequisite task.
