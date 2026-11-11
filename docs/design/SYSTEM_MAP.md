# System map: keep / adapt / remove for every firmware subsystem (v1 clean house)

Task P2-002. Inputs: [SYSTEM_INVENTORY.md](SYSTEM_INVENTORY.md) (measured sizes, build at `c44670e`), [CONTROL_MAP.md](CONTROL_MAP.md),
[UX_REVIEW.md](UX_REVIEW.md), [ALGORITHM_SET.md](ALGORITHM_SET.md), ADR-0001, ADR-0004, VISION. Design and analysis only: nothing here
removes code (that is P3 / epic E3). **Status: awaiting owner review. Not approved.**

## 0. Governing decision (read first)

Owner update 2026-11-10 (**supersedes decision 32**): the Felucca groovebox is **completely out of the UI in v1**. The device
boots straight into the sequencer UI. There is no `MODE` row and no panel route to the groovebox. Consequences used throughout:

1. A subsystem only the groovebox uses is **REMOVE** in v1. Where removal needs risky edits to the audio ISR or to a file that the
   sequencer shares, the row says **ADAPT** (strip the groovebox parts) and the leftover goes to the v2 column.
2. A subsystem the sequencer, the preview synth or the platform core needs is **KEEP** or **ADAPT**.
3. **v2 column** = clean-up of whatever could not be removed safely in v1 (it is the scope note for epic E9). `-` means nothing left to do.
4. Unreachable code still costs flash, so "hide it" is not an option: every REMOVE is deleted from the build (or, for the one
   case where deletion is unsafe, compiled out by a flag in v1 and deleted in v2: stated in the row).

Words: **KEEP** unchanged. **ADAPT** stays, with the stated change. **REMOVE** deleted with its tests. **REMOVE-ON-REPLACE**
(a form of REMOVE) is deleted by the dev task that lands its replacement (E4/E6), not by the E3 clean-house task, because the
T-1 mode must keep working on the TB-3 until then.

## 1. Rule for what the sequencer needs (the KEEP test)

The sequencer in SEQ operation needs exactly: boot and platform core; panel input; LCD and canvas; the audio ISR as timebase;
USB MIDI out/in and update path; TRS MIDI in; storage for settings and presets; **one preview synth voice path**
(`trk[0]` -> `voice.c` -> `fx.c mix_part` -> `audio.c`), driven by `t1_glue.c synth_note_on/off`; the menu for set-and-forget settings.
Source evidence for the preview path: `t1_glue.c` calls `trk_note_on(&trk[0], ...)` (voice.c:363); `t1_glue.c` uses `beat_samples()`
(fx.c:219), `midi_out_event` (usb.c) and `song.playing`; `ui_t1.c` draws with `cv_*`, `draw_head()` (ui_draw.c:324) and reads `panel_enc()`.

## 2. The preview synth: ANALOG stays

Engine choice: **ANALOG** (`eng_analog.c`, `ENGINES[0]`). Reasons, all from the source and the inventory:
- It is index 0 and the fallback: `eng_idx()` returns 0 for any bad engine number (engines.c), so the device always has a valid engine.
- It is the smallest useful one: 5746 B flash, 0 B RAM, 0 B pool (inventory appendix). It keeps no per-part state in the engine
  memory pool (`grep -c ENG_MEM eng_analog.c` = 0), so `eng_mem` (51552 B pool) is not needed once the other engines go.
- It is a subtractive mono-friendly voice (oscillator, filter, envelope; filter TYPE LP/BP/HP added in 1.5) which suits the bass
  line a TB-3 user previews. FM6 (11507 B, bank in flash) and the sample family (140537 B) cost far more for no sequencing benefit.
- Its tests survive (`regress.c` golden renders can be cut to ANALOG presets).
The preview patch itself (one fixed bass-suited ANALOG patch at boot, no engine/preset browsing) is a P2-003/E4 detail: see owner question 40.

## 3. Boot and first-use

What the device does at power-on in v1 (the target; the screen is specified in P2-003, the defaults come from UX_REVIEW section 2):

1. `main.c` boots the platform (HAL, WDT/boot-loop guard, LCD, gfx, MASTER pot read, storage + settings restore, audio ISR start, USB enumerate).
2. `t1_glue_init()` runs `t1_default_pattern()` equivalents: **4 channels**: CH1 ACID on MIDI 1 unmuted; CH2 EUCLID MIDI 2, CH3 SNAKE MIDI 3,
   CH4 STAGE MIDI 4, all three muted; global KEY C, SCALE MINPEN, OCTAVE 0, GATE 50 %, DIV 1/16, SWING 50 %, BPM 120; preset P01; keyboard OCT 0.
   (UX_REVIEW section 2. Note the TB-3 default is MIDI 2, so step 2 of the reference workflow still sets CH1 to 2.)
3. The device enters the sequencer main screen directly. `t1_mode` is permanently 1 (retire `PREF_T1`, `MI_T1SEQ`); HOME held 0.7 s opens the menu.
4. **PLAY is audible with no setup**: the selected channel's notes go to MIDI out and to the preview synth (`trk[0]`, ANALOG) on the
   speaker and USB audio, volume from the MASTER pot. If no host is configured, MIDI out is dropped (usb.c) but the preview still sounds.

Subsystems this depends on (all KEEP/ADAPT): libc, lcd, gfx, HAL, main (ADAPT), audio, panel (ADAPT), sound core (ADAPT), ANALOG engine,
params (ADAPT), voice, fx (ADAPT), usb, midi_uart, midi clock/control (ADAPT), seq core (ADAPT: keyboard, transport, `events_block`),
T-1 core + glue (ADAPT), ui_t1 (ADAPT: becomes the main UI), ui draw/input (ADAPT: header, frame, hold timers, menu dispatch), menu,
storage + settings_persist + favorites (settings live in `favorites.factory[15][..]`, see 4.3), project.c (ADAPT: the preset store), click, master, ota.
Removing any of them breaks "boots to the sequencer and plays". Not needed at boot: everything marked REMOVE.

## 4. Subsystem table

Every row of SYSTEM_INVENTORY. Sizes are the inventory's measured bytes (flash = .text+.ram_text+.data; RAM = .data+.bss).
"Hidden edges" were found in the source beyond the inventory's heuristic dependency columns (section 5).

| # | Subsystem (files) | v1 | Reason / what changes | Flash B | RAM B | Pool B | v2 (clean-up of what v1 could not remove) |
|---|---|---|---|---:|---:|---:|---|
| 1 | libc (libc.c) | KEEP | compiler-emitted memset/memcpy; platform core | 258 | 4 | 0 | - |
| 2 | lcd (lcd.c) | KEEP | the screen is the UX (VISION 1) | 432 | 548 | 0 | - |
| 3 | gfx (gfx.c, fonts, keycaps, palettes) | KEEP | canvas + text for every screen; `cv_px` is 61228 B pool | 53743 | 74 | 61228 | ADAPT: drop font sizes / keycap and palette data the final screens do not use (needs the P2-003 screen spec to know; unconfirmed) |
| 4 | sound core (core.h, dsp.c, engines.c) | ADAPT | keep `track_t`/`trk[0]`, the DSP helpers and ANALOG. Delete the engine table entries of removed engines (`NENGINES` 1), the factory sequence patterns (SAVE > PHRASES), `eng_mem` (51552 B, ANALOG uses none). Indices "append, never reorder" only mattered for editor/projects/user presets, all removed | 1329 | 7304 | 51552 | ADAPT: shrink `trk[]` (7296 B) from 4 groovebox tracks to what the preview needs; remove NPART > 1 |
| 5 | engines: ANALOG (eng_analog.c) | KEEP | the preview synth (section 2) | 5746 | 0 | 0 | - |
| 5b | engines: PHASE LOFI TRIO WHEEL VOICE NOISE (6 files) | REMOVE | sound design, not preview (VISION 2). Only the engine table, params names and tests reach them | 17013 | 3588 | 0 | - |
| 6 | engines: sample family (eng_sample, eng_grain, eng_slice, felucca_samples.h) | REMOVE | sampler/slicer: out of scope (VISION 2). 126717 B of ADPCM data. Callers found: engines.c, params.c, ui_slice.c (ui.c:1763), editor, category.c/settings_persist.c mention it (check at removal) | 140537 | 5883 | 0 | - |
| 7 | engine FM6 (eng_fm6, fm6_core, fm6_vbank, fm4_convert, felucca_fm6.h) | REMOVE | 6-op FM with a flash voice bank; `fm4_convert.c` only converts old DIGITAL projects, which no longer load. Bank storage (`fm6_vbank.c`) uses storage, not the reverse | 11507 | 1561 | 525 | - |
| 8 | engine PHYS (eng_phys, phys_dsp, phys_symp) | REMOVE | physical models: sound design | 9329 | 0 | 0 | - |
| 9 | engine DRUM (eng_drum, drum_voice) | REMOVE | 8-lane drum kit + grid. Hidden edges: `voice.c:188 ENGI_DRUM` branch, `seq.c:94 drum_track()`, `ui_input.c:531,898`, ui_draw grid | 14352 | 4 | 0 | - |
| 10 | params (params.c) | ADAPT | keep the descriptor table that `voice.c` reads (`P_LWAVE`, `P_LSYNC`, `P_LRATE`...) for ANALOG; drop the other engines' pages, sample/DRUM/FM6 value formatters | 6102 | 2 | 0 | ADAPT: prune to the parameters the preview patch uses |
| 11 | mod matrix (mod.c) | KEEP | `voice.c` calls `mod_note` (378), `mod_voice` (687), `mod_sh` (67) on every note and block: removing it edits the ISR render path, which changes the preview sound for no UX gain. With no slots set it is bit-identical to off (mod_test) | 1858 | 52 | 0 | REMOVE together with voice ADAPT (strip the three call sites) |
| 12 | voice (voice.c) | ADAPT | keep note on/off, ANALOG render, `track_render`. Remove the DRUM branch (188) and chord/mono-mode hooks that need removed code | 3284 | 181 | 0 | ADAPT: reduce voice modes (MONO/LEGATO/UNISON) to what the preview needs; strip LFO/matrix |
| 13 | slicer insert (slicer.c) | REMOVE | groovebox per-track gate/stutter. Task P3-002. Callers (FB-002): fx.c 3 call sites (`slicer_busy` 842, `slicer_track` 843/856, `slicer_track_sd` 811), `seq.c:851 slicer_start`, `midi_control.c:194 sl[track]`, `ui_graph.c` `graph_slicer`/GR_SLCR (483, 1222, 2294), `perform.c` (borrows `sl_buf`: given its own `pf_buf` until R4). The `P_SLCR..P_SLDEPTH` param slots stay dead (project format). 32768 B pool | 1614 | 164 | 32768 | - |
| 14 | fx (fx.c) | ADAPT | keep `mix_part`, `beat_samples()` (the timebase, fx.c:219), master level/SPEAKER EQ (`fx_lowcut`, `spk_bass`), USB level. Task P3-014 removes the DIST/INSERT, chorus, delay, reverb buses (and `dist_*`/`tail`/`ins_run` in `track_t`, `graph_insert`); P3-002 removes the slicer calls (811, 842-858). The perform calls stay until R4, so fx.c keeps `send_d`/`send_r` scratch for `perf_throw`/`perf_pre`; the `P_DIST..P_REV`, `P_ITYPE..P_IMIX`, `G_RTYPE..G_RDAMP` slots and `MD_DIST..MD_REV` stay dead. Pool freed = `dly_buf` 131072 + `rev_comb` 9874 + `cho_buf` 4096. **Correction (FB-003 audit): the presets carry CHO/DLY/REV sends, so the preview sound does change** (95 of 97 golden renders, 12 ANALOG presets); intended by decision 80, goldens are re-baselined in P3-014 | 6469 | 1682 | 150066 | ADAPT: flash/RAM of the removed effect code is unconfirmed (<= 6469 / <= 1682; the inventory cannot split a file) |
| 15 | perform (perform.c) | REMOVE | the FX-hold layer: the FX button is now DICE ALL (CONTROL_MAP). Callers: `seq.c:660-720,772-817,852 perf_key*/perf_start`, `fx.c:813,858,884,930-941`, `ui_layer.c`, `ui_input.c:411,1488,1533-1538` (all groovebox-only). Until R4 it owns a 32768 B `pf_buf` pool buffer (was `sl_buf`) | 7254 | 1877 | 0 | - |
| 16 | click (click.c) | KEEP | CLICK, CLICK LEVEL, COUNT-IN stay in the AUDIO menu (CONTROL_MAP section 5) | 257 | 4 | 0 | - |
| 17 | usb (usb.c, uac_fir.h) | KEEP | MIDI out/in, USB audio of the preview, update SysEx path (`ota_req` 152/794, `ota_wire_send` 876) | 10591 | 5393 | 0 | ADAPT only if the owner drops USB audio or CDC (question 38) |
| 18 | midi_uart (midi_uart.c) | KEEP | TRS MIDI IN is the only computer-free input | 0 | 156 | 0 | - |
| 19 | song chain (song_chain.c) | REMOVE | groovebox song rows (song.grid = 3). **Hidden edges**: `seq.c:236,375,841,891,1077` (`chain.running`, `chain_start/stop`, `chain_motion`), `main.c:102 chain_defaults`, `ui_draw.c:293`, `ui_input.c:274,345 chain_busy`, `project.c` FUN6. 13096 B of RAM | 1302 | 13180 | 0 | - |
| 20 | seq core (seq.c) | ADAPT | keep `keyboard_block` (775), scale/key helpers, transport, `events_block`, `seq_start/stop`, the `t1_*` hooks (58-71, 858, 893, 1420). Remove the 4-track pattern sequencer, ARP, recording, drum recording, song_key, chain/motion/perf calls. Keys: grid 0 notes, 2 silent | 7398 | 233 | 0 | ADAPT: remnants (`song.grid`, `seq_mode`) once ui no longer reads them |
| 21 | seq: chord (chord.c) | REMOVE | CHRD/VOIC chord keys: not in the control map. **Hidden edge**: `midi_control.c:59,86 mchord_of` (chord.c:185) -> ADAPT midi_control first. Menu row `CHORD ENTRY` (MI_CHADD = keys add to the cursor step) goes with it: question 39 | 846 | 27 | 0 | - |
| 22 | seq: motion (motion.c) | REMOVE | step automation/p-locks are dropped (CONTROL_MAP section 6). **Hidden edges**: `seq.c:840,892 motion_begin/end`, `ui_input.c:524 motion_capture`, `project.c`, `ui_events.c`, `ui_layer.c` | 3028 | 1021 | 0 | - |
| 23 | seq: midi clock/control (midi_clock.c, midi_control.c) | ADAPT | keep external clock follow, Start/Stop (`midi_clock.c:18,29 seq_start/stop`), the MIDI IN routing row; remove the chord-aware note tracking (`mchord_of`) and the per-groovebox-track channel table (`MI_TCH`) | 3102 | 2320 | 0 | ADAPT: `midi_notes[][]` table (2280 B in .bss) sized for the groovebox routing; shrink |
| 24 | T-1 core: torso_t1_port.c | KEEP | freestanding `snprintf` shim | 498 | 0 | 0 | - |
| 25 | T-1 core: torso_t1.c / torso_t1.h | ADAPT | 6 tracks x 6 cycles (128-byte `CycleParams`) -> 4 channels, no cycles (CONTROL_MAP section 6). Keep the micro event queue and MIDI rings (SP14). `.pool` buffers 13980 B shrink (amount unconfirmed) | 1970 | 36 | 13980 | - |
| 26 | T-1 core: torso_t1_algo.c | ADAPT | keep `t1_euclidean_bjorklund/_rotate` (SP11), scale masks, pace curve (SP5), phrase shapes (SP10). Replace LFSR by SP1 xorshift, drop p-locks, snap-down by nearest-ties-lower (SP6) | 2508 | 0 | 0 | - |
| 27 | T-1 core: torso_t1_pipeline.c | ADAPT | the 192 PPQN evaluator is rewritten around SP12/SP14 and the four algorithms (E4/E5); `synth_note_on/off` hook stays | 1956 | 280 | 0 | - |
| 28 | T-1 core: torso_t1_state.c | REMOVE-ON-REPLACE | TEMP snapshot and GLOBAL offsets are dropped (CONTROL_MAP section 6); the flash layout was never wired up. Presets P01-P16 go through project.c (E7). Delete in the E4/E7 task that lands the new state | 1622 | 28 | 0 | - |
| 29 | T-1 ui helper (torso_t1_ui.c) | REMOVE-ON-REPLACE | knob-to-parameter mapping and card text are replaced by the data-table control map and knob tables (SP15, ADR-0004). Delete when E6 lands the tables | 3612 | 9 | 0 | - |
| 30 | T-1 glue (t1_glue.c) | ADAPT | keep clock accumulator, start/stop, all-notes-off, CC123; replace `t1_default_pattern()` by the 4-channel defaults (section 3); `trk[0]` preview plays the selected channel | 340 | 18 | 0 | - |
| 31 | T-1 screen (ui_t1.c) | ADAPT | becomes the main UI (PERF, STEP, CHAN, GLOB, SCL); always on (`t1_mode` = 1). Hooks in ui_input.c:1510, ui_draw.c:1509 (**hidden edges**: the inventory says nothing depends on it) | 88 (+ inlined, unconfirmed) | 3 | 0 | - |
| 32 | audio (audio.c) | KEEP | I2S ISR and the sequencer's clock source | 6516 | 2069 | 0 | - |
| 33 | panel (panel.c) | ADAPT | printed-label table now also carries the on-screen role labels (FX = ALL, SCL = NOTES, ...) | 469 | 6 | 0 | - |
| 34 | ui core: ui.c | ADAPT | keep prefs, `ui_force`, messages, menu/confirm state, power-on. Remove pages, browse, undo of sound loads, step-entry state, user-preset calls (`up_used` ui.c:12,37) | 8930 | 105 | 1549 | ADAPT: further trimming of `ui` state once pages are gone |
| 35 | ui core: favorites.c | ADAPT | **hidden edge**: `favorites.factory[16][32]` is the settings blob (`ui.c:45 ui_prefs = favorites.factory[15][30]`, `settings_persist.c`). Keep the array, delete the favorites/filter logic | 420 | 0 | 520 | ADAPT: rename to a settings struct (changes the persisted format: needs a migration decision) |
| 36 | ui core: category.c | REMOVE | sound categories for PRESETS > LIST | 222 | 0 | 0 | - |
| 37 | ui core: midi_learn.c | KEEP | CONTROL_MAP keeps the `LEARN CLEAR` row, so the learn table stays; question 41 asks whether CC learn belongs in a sequencer | 458 | 0 | 0 | REMOVE if question 41 = no |
| 38 | ui core: ui_events.c | REMOVE | automation list (motion) | 1592 | 0 | 0 | - |
| 39 | ui core: ui_slice.c | REMOVE | EDIT > SLICES (`#include` at ui.c:1763) | 842 | 450 | 0 | - |
| 40 | menu (menu_items.c, ui_menu.c) | ADAPT | rows: remove LATCH, BPMLOCK, SELECT, T1-T4 CHANNEL, T1 SEQ, and CHORD ENTRY (question 39); **no MODE row**. 37 -> 28 rows, see section 7 | 4254 | 4 | 0 | - |
| 41 | icons (icons.c, ui_icons.h) | ADAPT | tab icons (`ui_menu.c:189`) and header/battery icons (`ui_draw.c:39,48`) are still drawn; the parameter icons for knob cards go (`FELUCCA_ICONS`) | 10103 | 0 | 0 | ADAPT: drop every unused icon cell (needs the final P2-003 screens) |
| 42 | ui graphs (ui_graph.c, ui_fm6op.c) | ADAPT | remove FM6 operator pages (ui_fm6op.c 276 B REMOVE), piano roll, browse, drum grid, sample pages; keep the scope only if P2-003 keeps SCOPE (DISPLAY row) | 26400 | 618 | 1024 | ADAPT: unconfirmed split of the 26124 B; whatever P2-003 does not draw |
| 43 | ui draw (ui_draw.c, ui_help.c) | ADAPT | keep `draw_head`, `draw_frame`, footer, HELP hints used by ui_t1 and the menu; remove every groovebox page draw | 30508 | 18 | 0 | ADAPT: unconfirmed split; the largest remaining UI file |
| 44 | ui input (ui_input.c) | ADAPT | keep button hold timers, menu dispatch, the T-1 early return (1510); remove page, step-entry, grid, record, layer code | 27802 | 41 | 56 | ADAPT: unconfirmed split |
| 44b | ui input: ui_name.c | REMOVE | NAME screen for user presets/projects; sequencer presets are numbered P01-P16 | 897 | 0 | 56 | - |
| 45 | ui layers: ui_layer.c | REMOVE | the quick layers (FX GLO SCL EDIT held) are replaced by the CONTROL_MAP contexts implemented in ui_t1.c (T-1 mode returns early before the layer code, ui_input.c:1510). Check `ui_help.c` first (it is used by `draw_layer`) | 3109 | 96 | 0 | - |
| 45b | ui layers: ui_tools.c | REMOVE | SEQ TOOLS (EUCLID, DOUBLE/HALF) on pattern data | 2380 | 0 | 2184 | - |
| 46 | storage (storage_hw.c, storage.c) | KEEP | A/B sector store: settings and presets (E7) | 1082 | 1 | 3840 | - |
| 47 | user presets (upreset.c, up_fm6.c) | REMOVE | 32 groovebox sound presets. Callers: `ui.c up_used`, `project.c`, editor | 3230 | 0 | 9888 | - |
| 48 | project (project.c, settings_persist.c, slice_store.c) | ADAPT | settings_persist.c KEEP (settings). project.c becomes the sequencer preset store (P01-P16 as storage objects, E7): delete the FUN1..FUN10 groovebox formats, song chain, motion, patterns; slice_store.c REMOVE (282 B) | 9057 | 7 | 9238 + .noinit 15360 | ADAPT: unconfirmed split; `proj_slot` 15360 B .noinit resized to the preset size |
| 49 | ota (ota.c, ota_hw.c) | KEEP | the in-app update entry (`ota_service`, M-UPGRADE handshake used by `tools/fm1_install.py`). It does **not** depend on the editor (`grep -c 'ed_\|ED_' ota.c` = 0) | 1128 | 94 | 2688 | - |
| 50 | editor (editor*.c, 7 files) | REMOVE | web-editor SysEx for the groovebox (patches, FM6 bank, backup, palette). Only `main.c:246,286 ed_service` calls it; it calls `ota_frame_get/done`, not the reverse. `web/editor.html`, `web/EDITOR_PROTOCOL.md`, `fm1backup.js` become orphaned (question 37) | 14962 | 659 | 1748 | - |
| 51 | console (console.c) | KEEP | read-only CDC diagnostics (936 B); every change is hardware-tested (CLAUDE.md) and the console shows crash records and audio load | 864 | 72 | 0 | revisit (question 38) |
| 52 | master (master.c) | KEEP | MASTER pot read before audio start | 60 | 4 | 0 | - |
| 53 | main (main.c) | ADAPT | remove `ed_service` calls (246, 286) and `chain_defaults` (102); boot straight to the sequencer; keep the boot guard, update-mode gesture, calibration | 18114 | 1 | 0 | ADAPT: the boot-time groovebox state init that remains |
| 54 | HAL + tables | KEEP | register-level drivers (key scan, flash driver in `.ram_text`), `felucca_tables.h` | 17115 | 396 | 0 | ADAPT: drop `felucca_tables.h` entries (11816 B generated) that only removed engines read; unconfirmed |
| 55 | (symbols not matched) / (unnamed bytes) | n/a | not subsystems: closed by removing/adapting their owners | 709 / 12761 | 594 / 4865 | 11 / 11 | - |
| 56 | engine DIGITAL (eng_digital.c, `FELUCCA_FM4`) | REMOVE | not compiled today (FM4 = 0), so 0 B freed; delete the dead source, the flag and the FM4 test builds | 0 | 0 | 0 | - |

The platform core (inventory "Platform core" table) is **all KEEP or ADAPT-only-where-named**: boot/linker/libc, input (HAL +
panel), LCD (lcd + gfx), audio ISR/timebase, USB, TRS MIDI IN, storage, update loader + ota.c, MIDI clock/transport. The two
ADAPTs inside it (panel labels, seq.c/midi_clock/midi_control stripping, main.c boot) only remove groovebox code and keep every
platform function. The update loader (`loader/`, 8216 B image, ota.bin 6930 B) and the installer path (usb SysEx -> ota_service -> loader,
`tools/fm1_install.py`) are **not touched**.

## 5. Dependency edges verified in the source (inventory columns were heuristic)

Method: grep of `firmware/src`, line numbers cited. 16 edges checked; 6 are corrections to the inventory.

| # | Edge | Result |
|---|---|---|
| 1 | usb -> ota (`usb.c:152` `ota_req`, 794, 876 `ota_wire_send`) | true; ota must stay while usb stays |
| 2 | editor -> ota (`editor.c:56,915 ota_wire_send / ota_frame_get`); ota -> editor | true one way; `ota.c` has 0 references to `ed_`: **editor can go without breaking updates** |
| 3 | main -> editor / ota / console (`main.c:246,286 ed_service`, 247 `ota_service`, 270 `cdc_task`) | true; the only caller of the editor is main.c (plus `felucca.c` include) |
| 4 | t1_glue -> voice/sound core (`t1_glue.c:25-36 trk_note_on/off`, voice.c:363,410) | true; this is the preview-synth path (KEEP chain) |
| 5 | seq core -> song chain (`seq.c:236,375,841,891,1077`) | **missing in the inventory** (it lists song chain depending on seq, not the reverse): seq.c must be edited before song_chain.c is removed |
| 6 | ui_input / ui_draw -> T-1 screen (`ui_input.c:1510 t1_input`, `ui_draw.c:1509 t1_draw`, `ui.c:141-148` stubs) | **missing**: the inventory lists T-1 screen as depended on by nothing |
| 7 | fx -> slicer, perform (`fx.c:811,813,842-858`); also perform -> slicer (`perform.c` uses `sl_buf`, `sl[]`, `sl_lent`), seq -> slicer (`seq.c:851`), midi_control -> slicer (`:194`), ui_graph -> slicer/fx (GR_SLCR, `graph_insert` uses `ins_*`) | true, and the last four are **missing in the first pass (FB-002)**; slicer.c's references to `fx.c` are comments only, so the inventory's "slicer depends on fx" and "voice depends on fx" (voice.c:579 is a comment, `spread_on` is voice.c's own) are **false edges**; the real direction is fx -> voice/slicer |
| 8 | voice -> mod matrix (`voice.c:67,378,687`) | true; why mod stays in v1 (row 11) |
| 9 | midi_control -> chord (`midi_control.c:59,86 mchord_of`, defined `chord.c:185`) | **missing**: chord removal needs a midi_control edit |
| 10 | ui_prefs -> favorites (`ui.c:45`; `settings_persist.c` stores `favorites`) | **missing**: favorites.c cannot be removed, only emptied |
| 11 | ui core -> ui_slice -> sample family (`ui.c:1763`; `ui_slice.c`), params -> ENGINES (`params.c:228,243`) | true; params names the sample engine, so params is edited with the engine removal |
| 12 | voice/seq/ui -> DRUM (`voice.c:188`, `seq.c:94`, `ui_input.c:531,898`) | **missing** from the engine DRUM row's "depended on by" for seq and voice |
| 13 | console -> storage/audio (`console.c:153 st_read`, 200 `felucca_dbg`) | true; console is a leaf (nothing calls it but main) |
| 14 | audio -> fx (`audio.c:34 fx_usb_fixed`) | true |
| 15 | midi_clock -> seq (`midi_clock.c:18,29 seq_start/stop`) | true (clock follow KEEP implies seq core ADAPT keeps those entry points) |
| 16 | ui core -> user presets (`ui.c:12,37 up_used`); ui_input -> motion (`ui_input.c:524`), ui_input -> chain (`274,345`) | true; callers are all groovebox pages |

## 6. Removal order (leaves first; nothing a KEEP/ADAPT row needs is removed before it is cut loose)

Each step ends with `tools/check.sh` (build + tests) green; steps 1-3 are independent of the algorithm work and can run before E4.

| Step | Remove | Why it is safe here (edge numbers from section 5) | Test impact (section 8) |
|---|---|---|---|
| R1 | engine DIGITAL source + `FELUCCA_FM4` flag (row 56) | not compiled; a leaf | digital_test, fm4_test, fm4_div0_test, `ui_test_fm4`, FM4 UI render go |
| R2 | editor (row 50) | leaf (edge 3); ota is independent (edge 2) | editor_test, fuzz_ed, backup_test, robust_test (malformed requests part) go |
| R3 | groovebox UI: ui_tools, ui_layer, ui_name, ui_events, ui_slice, category, FM6 op pages, drum/step/pattern/browse pages, icon cells, `MI_` rows (rows 36,38,39,41-45b) | UI is the top of the dependency stack; nothing KEEP depends on these pages; `ui_t1.c` returns early before them | ui_test, ui_render, ui155_test, detail_test, step15_test, v16_test, v156_test, drumfuzz_test shrink or go |
| R4 | song chain, motion, chord, perform (rows 19,22,21,15) after editing their callers (edges 5, 9, 16): seq.c, main.c, ui_input, midi_control, fx.c | callers are the R3 pages and seq.c / fx.c / midi_control.c, which are ADAPTed in the same step | motion_test, chord_test, seq155_test, fun10_test, ratchet_test, project_test (song/motion) |
| R5a/R5b | slicer insert (row 13, P3-002) then fx effect buses (row 14, P3-014), after fx.c, seq.c, midi_control.c, ui_graph.c are edited and perform.c has its own buffer (edge 7) | perform.c is cut loose from slicer.c first (own `pf_buf`), so R5 does not wait for R4; the buses go second | slicer_test, insert_test, reverb_test, swing_test (slicer part) |
| R6 | engines: DRUM, PHYS, sample family, FM6 (+ fm4_convert, vbank), PHASE/LOFI/TRIO/WHEEL/VOICE/NOISE, in that order (rows 5b-9): first `voice.c` / `seq.c` DRUM branches (edge 12), then `params.c`, then `engines.c`'s table, then `eng_mem` | voice/seq/ui no longer name them after R3-R4 | drum_test, phys_test/phys_ref, sample_test, slice_test, fm6_test, fm6bank_test, noise_test, fold155_test, sound14 (non-ANALOG parts), union_test, regress goldens (re-baseline to ANALOG) |
| R7 | user presets, slice_store, project formats FUN1-FUN10 (rows 47, 48) | `up_used` callers and project loaders were cut in R3/R4/R6 | upreset_test, project_test, backup_test remnants |
| R8 | (on replacement, E4/E6/E7) torso_t1_state.c, torso_t1_ui.c (rows 28, 29) | the T-1 screen and glue keep calling them until the replacement is in | torso_phase4_test, torso_phase5_test replaced by the new golden/scenario tests |

Order check: no step removes a row that a KEEP/ADAPT row still calls. Chain and motion (R4) wait for seq.c's edits (edge 5), fx effects (R5)
wait for the fx.c edit, DRUM (R6) waits for voice/seq/ui (edge 12), the editor (R2) is a leaf, and ota/usb/loader/installer are never touched.

## 7. Menu rows and `tests/ui_test.c`

Current tabs (menu_items.c): DISPLAY 8, CONTROL 8, AUDIO 6, MIDI 8, SYSTEM 7 = 37 rows (the test asserts "8 8 6 8 7").
Rows removed from the menu: MI_LATCH, MI_BPMLOCK, MI_SELECT, MI_TCH1..4, MI_T1SEQ (CONTROL_MAP section 5) **and, because the groovebox is gone, no
MODE row is added** (CONTROL_MAP and UX_REVIEW 1 assumed a `MODE SEQ/GROOVE` row: they need a one-line update, owner decision 32 is superseded).
Also MI_CHADD (CHORD ENTRY) if chord.c goes (question 39). MI_PREVIEW (STEP PREVIEW) stays: it auditions pinned steps.

| Tab | Before | After | Rows |
|---|---:|---:|---|
| DISPLAY | 8 | 8 | COLOR STYLE LARGE ANIM LEDS SCREEN OFF SCOPE HOME |
| CONTROL | 8 | 4 | HOLD, KNOB ACCEL, SCALE LEDS, STEP PREVIEW (5 if CHORD ENTRY stays) |
| AUDIO | 6 | 6 | SPEAKER EQ, USB LEVEL, CLICK, CLICK LEVEL, COUNT-IN, TUNE (TUNE is `song.g[G_TUNE]`: the preview's tuning: keep, or move to the preview patch: question 40) |
| MIDI | 8 | 4 | MIDI IN, AUTO SELECT, SCREEN WAKE, LEARN CLEAR |
| SYSTEM | 7 | 6 | USB SERIAL, RESTORE LAST, HELP, CALIBRATION, INFO, ABOUT |
| Total | 37 | 28 | `MI_COUNT` 37 -> 28; `MI_VALUES` shifts |

`tests/ui_test.c` changes (dev task of R3, not now): the title string and assertions at ~1668 ("8 8 6 8 7 rows") become "8 4 6 4 6";
`mtab_rows(MTAB_CONTROL) == 8` -> 4, `MTAB_MIDI` 8 -> 4, `MTAB_SYSTEM` 7 -> 6; the navigation script (~1672-1690) uses `MI_LATCH`,
`MI_TCH1`, `MI_SELECT`, `MI_SERIAL`, `MI_HOME`, `MI_LCLEAR` as anchors: LATCH and TCH1 vanish, so the walk is rewritten with CONTROL = HOLD..STEP PREVIEW and MIDI = MIDI IN, AUTO SELECT, SCREEN WAKE, LEARN CLEAR. Also changed: `ui155_test.c:132-134`
(SELECT row), `midi_control_test.c:361-370` (`MI_TCH1..4`), `ui_render.c:1980-2025` (loops over `MI_COUNT`, adapts itself), `editor_test.c` (removed with the editor).
Because host tests build without `T1_DEVICE`, the stubs `t1_mode = 0` hide the T-1 from them; once the groovebox is gone the stubs go and the host
tests must link `t1_glue.c`/`ui_t1.c` (new coverage, ADR-0004 scenarios): a P3/E4 task, flagged in question 42.

## 8. Host test suites, per removal

Listed from `tests/run_tests.sh` (suite = one run step). "go" = deleted with the code; "change" = edited; "stay" = unaffected.

| Suite | Fate | With step |
|---|---|---|
| storage_test, input_test, midi_uart_test, uac_test (x2), usb_desc_test (matrix), ota_test, ldr_test, audio_test, click_test, persistence_test, settings_test, theme_test, text_ref_test, text_spacing_test | stay (settings_test/persistence_test change if the favorites blob is renamed) | - |
| torso_phase1_test (micro queue), phase2_test (algo; snap lines 212-224 hold under SP6), phase3_test (pipeline), phase4_test (TEMP/state), phase5_test (ui helper) | change: 1-3 follow the 4-channel model; 4 and 5 go with R8 | R8 |
| midi_control_test | change (chord, `MI_TCH`) | R3/R4 |
| scale_test | change: `scale_snap` -> SP6 | E4 |
| speaker_test (SPEAKER EQ in fx.c) | stay | - |
| mod_test, sound14_test (ANALOG SYNC/SUB stay, matrix S&H stay while mod stays), sound15_test (ANALOG filter TYPE), mod_test demos | stay / trim non-ANALOG cases | R6 |
| regress (golden renders, CPU budget), union_test, tests/golden.txt, cpu_baseline.txt, target_budget.txt | change: ANALOG only; `GOLDEN_UPDATE=1`, `BUDGET_UPDATE=1` re-baseline; union_test goes (one engine) | R6 |
| ui_test, ui_render (+PNG sweeps), ui155_test, detail_test, step15_test, v156_test, v16_test, drumfuzz_test (+ song variant) | ui_test and ui_render change to the new menu and the T-1 screen; the others go | R3 |
| seq155_test, ratchet_test, fun10_test, motion_test, project_test, chord_test | go (groovebox features) | R4 |
| slicer_test, perform_test (moved here from R4: it needs the slicer and the buses) | go | R5a (P3-002) |
| insert_test, reverb_test | go | R5b (P3-014) |
| swing_test | change: the slicer part goes, the sequencer swing parts stay | R5a |
| drum_test, phys_test, phys_ref, noise_test, fm6_test, fold155_test, slice_test, sample_test, sound14 non-ANALOG | go | R6 |
| fm6bank_test, upreset_test, backup_test, fuzz_proj, fuzz_smp, robust_test | go (robust_test's engine-number cases) | R6/R7 |
| editor_test, fuzz_ed | go | R2 |
| digital_test, fm4_test, fm4_div0_test, `ui_test_fm4` and its render | go | R1 |
| hostsim (render + TRACKS demos), ASan/UBSan reruns | change with regress | R6 |

Known failures that predate this work (spaced path "web pages" step; T-1 phase 3/4 wall-clock benchmarks) are untouched.

## 9. Flash and RAM freed

Measured sizes from the inventory (build `c44670e`: image 523032 B; RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640).

**v1 REMOVE (E3, steps R1-R7), firm numbers** (row sizes quoted above; sums by `python3`):

| Region | Freed | After | Notes |
|---|---:|---:|---|
| Flash image | 234574 B (44.8 %) | 288458 B | engines 192738 (synth 17013 + sample 140537 + FM6 11507 + PHYS 9329 + DRUM 14352), editor 14962, perform 7254, user presets 3230, motion 3028, ui_tools 2380, song chain 1302, ui_layer 3109, slicer 1614, ui_events 1592, ui_name 897, chord 846, ui_slice 842, category 222, slice_store 282, ui_fm6op 276 |
| RAM .data+.bss | 28512 B | 27232 B (headroom 46496 B, was 17984) | song chain 13180, sample 5883, synth 3588, perform 1877, FM6 1561, motion 1021, editor 659, ui_slice 450, slicer 164, ui_layer 96, others |
| Pool | 192211 B | 150665 B of 368640 | fx buses 145042, slicer 32768, user presets 9888, editor 1748, ui_tools 2184, FM6 525, ui_name 56 |

On top, **not counted because the inventory cannot split a file** (all upper bounds): `eng_mem` 51552 B pool (if ANALOG needs none: `grep -c ENG_MEM eng_analog.c` = 0),
fx.c flash/RAM of the removed effects (<= 6469 / <= 1682), groovebox pages inside `ui_draw` / `ui_input` / `ui_graph` / `ui` / `project` (<= 30508 / 27802 / 26400 / 8930 / 9057 flash),
parameter icons (<= 10103), `proj_slot` 15360 B .noinit. Later, on replacement (R8): torso_t1_state 1622 + torso_t1_ui 3612 = 5234 B flash, 37 B RAM (the new algorithm code adds
back an amount not yet known). The T-1 sources' own 13980 B pool shrinks with 4 channels (unconfirmed).

**v2 clean-up** (what v1 did not remove; column "v2" above): mod matrix 1858 / 52, the ADAPT residue of ui_draw / ui_input / ui_graph / ui / icons / project / params / fx /
gfx / HAL tables / sound core (`trk[]` 7296 B) and `midi_notes` 2280 B. These are all "<= row size" upper bounds from the inventory: **firm v2 total from the
inventory = mod 1858 B flash + 52 B RAM**; the rest is measured only after v1 lands (re-run `docs/design/inventory/` scripts: ADAPT rows then show their real size).
This is the epic E9 scope; if v1 leaves enough headroom E9 may stay a small epic.

Safety: RAM headroom after v1 is ~46 KB (floor 4 KB, E9 trigger 8 KB); the build check `RAM .data+.bss ... of 73728` and `.ram_text` no-calls stay as they are.

## 10. Flex points (ADR-0004 addendum)

- Menu row set and tab assignment: `MI_TAB` / `MENU_FLAGS` tables; changing a row is a table edit plus the `ui_test` counts.
- Preview engine: `ENGINES[]`; a different single engine (for example PHASE for a brighter preview) is a one-line swap while the others are still in the tree; once R6 is done it is a new task.
- Boot defaults (section 3): a table in `t1_glue.c`/the ADR-0003 channel table.
- Cannot be changed by table edit: which source files are deleted (R1-R8), `NENGINES`, the persisted settings format (favorites blob).

## 11. Owner questions raised here (copied to OWNER_QUESTIONS.md, numbers 37-42)

37. Remove the web editor (and `web/editor.html`, `fm1backup.js`, backup/restore over SysEx) in v1? Recommend yes; the update path (`fm1ota.js`, `fm1_install.py`) is separate and stays. Alternative: keep it until v2.
38. Keep the CDC serial console (936 B) and USB audio of the preview in v1? Recommend keep both (hardware testing, hearing the preview over USB); revisit in v2.
39. `CHORD ENTRY` menu row (CONTROL tab) and chord keys: the control map keeps the row, but it adds notes to a groovebox step cursor. Recommend remove it with chord.c (CONTROL tab 4 rows). Confirm.
40. Preview patch: one fixed ANALOG bass-suited patch at boot, no browsing; is the existing ANALOG preset 0 acceptable or does the owner want a chosen patch? And does TUNE (cents) stay in the AUDIO tab?
41. MIDI learn (CC learn, `LEARN CLEAR` row, 458 B): keep for CC control of the sequencer, or remove with the groovebox?
42. The sequencer becomes permanent, so host tests must link the T-1 sources (today they are stubbed). OK to make that an E3/E4 prerequisite task?
