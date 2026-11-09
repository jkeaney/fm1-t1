# System inventory: every firmware subsystem, its size, RAM and dependencies

Ground truth for the keep/adapt/remove map (P2-002). This document only **describes**; it removes or recommends
nothing. Task: P1-002. Build inspected: `./build.sh` on the clean tree at commit `c44670e` (2026-11-09), default
flags (FELUCCA_FLASH/OTA/CDC/UAC/UART = 1, FELUCCA_SLICE = 1, FELUCCA_FM4 = 0, `T1_DEVICE` defined).

## Build totals (cited from the build, not estimated)

`./build.sh` printed (log line `ok    image ...`):

```
ok    .ram_text: 916 insns, no calls
ok    image 523032 B; RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640
loader: image 8216 B at 0x1c0a800, bss end 0x1c0f510
loader: ota.bin 6930 B
```

`objdump -h build/felucca.elf` (section sizes in bytes):

| Section | Hex | Bytes | Meaning |
|---|---|---:|---|
| .text | 0x7edf8 | 519672 | code + all const data, runs by XIP from flash at 0x02000120 |
| .ram_text | 0xb6c | 2924 | code copied to RAM (flash driver: it cannot run from flash while writing it) |
| .data | 0x1b4 | 436 | initialised RAM |
| .bss | 0xd800 | 55296 | zeroed RAM |
| .pool | 0x53b5c | 342876 | the 360 KB pool, NOLOAD, not zeroed |
| .noinit | 0x3ccc | 15564 | survives resets (project slots, crash record) |

Reconciliation with the build's own numbers (so the table's totals are exact, not "within 5%"):

- **Flash image** = .text + .ram_text + .data = 519672 + 2924 + 436 = **523032 B**, equal to the build's `image 523032 B`.
- **RAM .data+.bss**: the build reports 55744 B. The section sizes sum to 55732 B; the 12 B difference is alignment
  between the end of .data (0x1c081b4) and the start of .bss (0x1c081c0). The reported figure is `.bss` end
  (0x1c159c0) minus `.data` start (0x1c08000). RAM headroom = 73728 - 55744 = **17984 B** (the build requires >= 4096).
- **Pool** = 342876 B of 368640 (headroom 25764 B). **.noinit** 15564 B is a separate region (0x1c7c000, 0x3d50 long)
  and is not part of either reported RAM figure.
- Flash (XIP) region is `0x8DFBC` = 581564 B (firmware/app.ld:15), so .text leaves 61892 B free. .ram_text region is 0x6000 = 24576 B.

## Re-running (how every number was produced)

From the repo root. Docker must run; paths match tools/build.py (`docker run ... debian:bookworm-slim`, toolchain mounted
at /opt/jieli). The scripts are in docs/design/inventory/ and write to the git-ignored build/inventory/.

```sh
./build.sh                                              # -> build/felucca.elf and the "image ... RAM ..." line above
mkdir -p build/inventory
tcx(){ docker run --rm --platform linux/amd64 -v "$PWD:/work" -v "$HOME/.jieli/toolchain:/opt/jieli:ro" -w /work debian:bookworm-slim "$@"; }
tcx /opt/jieli/common/bin/objdump -h build/felucca.elf > build/inventory/hdr.txt     # section sizes (table above)
tcx /opt/jieli/common/bin/objdump -t build/felucca.elf > build/inventory/syms.txt    # symbol table with sizes
python3 docs/design/inventory/attrib.py build/inventory/syms.txt     # symbol -> defining file; per-file bytes per section
python3 docs/design/inventory/cov.py > /dev/null                     # which host tests compile which file
python3 docs/design/inventory/deps.py                                # file -> file symbol references
python3 docs/design/inventory/gen_table.py build/inventory/hdr.txt   # prints the subsystem table below
```

(`objdump -t` is the same call tools/build.py:226 makes. There is no linker map file; `nm --size-sort` is not in the
toolchain, so the symbol table from `objdump -t` is the size source.)

## Method and caveats (read before trusting a number)

1. **The firmware is one translation unit**, so the ELF has no per-file information. Each symbol (function or data object)
   is matched to the file whose source defines it (regex on the definition line). 1679 names were mapped; the remainder
   is in the "not matched" row.
2. **Inlined code counts toward the file it was inlined into.** Notable consequence: `ui_t1.c` shows 88 B, but its
   `t1_input` / `t1_draw` are called from ui_input.c / ui_draw.c and were inlined there (they have no symbol of their own in
   the ELF), so the screen's true cost is **unconfirmed** and sits inside the "ui input" and "ui draw" rows. Likewise
   `midi_uart.c` shows 0 B of code: none of its functions (e.g. `um_byte`) has an ELF symbol, so they were inlined somewhere
   (the caller was not identified), and its true cost is **unconfirmed**.
3. **.text includes read-only data**, not just code: 126717 B of ADPCM samples, 42904 B of fonts, 11816 B of generated
   tables, 7680 B of the UAC filter, icons, keycaps. See "Biggest items".
4. Symbols nested inside a larger symbol and the compiler's `.L_MergedGlobals*` bundles (about 9.5 KB of .bss, members of
   which are also listed individually) are excluded to avoid double counting. The "not matched" and "unnamed bytes" rows
   close the gap to the section totals (unnamed = string literals, switch tables, padding, asm objects; no symbol).
5. **Dependencies are a heuristic.** "Depends on" = this row's source mentions a function/variable the other row defines and
   that definition comes earlier in the unity include order (or is forward-declared). Macros, types and inlined-away
   names are not seen, and a common name can create a false edge (the top five by reference count are listed). Treat as a
   starting point; verify with the compiler before removing anything.
6. **"Runs in"** is taken from the file headers ("runs in the audio ISR", `main loop`), CLAUDE.md "Two threads of execution",
   and tools/size_fns.py:16-27 (files NOT in `SIZE_FILES` keep -Os because the sound/ISR code lives there). Rows marked
   `ISR+main` are shared. Inferred where noted; it is not derived from a call graph.
7. **Host tests**: `cov.py` follows `#include` chains of tests/*_test.c (+ regress, hostsim, ui_render, fuzz_*, descdump) and the
   `cc ... torso_t1*.c` lines of tests/run_tests.sh. "none (hardware only)" means no host test compiles that file. This says a
   test *compiles* it, not that it asserts anything about it.

## Subsystem table

Sizes in bytes. Flash = .text + .ram_text + .data (what goes into the 523032 B image). RAM = .data + .bss. Pool and .noinit as defined above.

| Subsystem | Files | Purpose | Flash B (.text+.ram_text+.data) | RAM B (.data+.bss) | Pool B | .noinit B | Runs in | Depends on | Depended on by | Host tests covering it |
|---|---|---|---:|---:|---:|---:|---|---|---|---|
| libc | libc.c | memset/memcpy/memcmp the compiler emits (no hosted libc) | 258 | 4 | 0 | 0 | boot+ISR | - | ui core, project, ui layers, T-1 core, editor | 49: audio_test, backup_test, chord_test, click_test ... |
| lcd | lcd.c | ST7789 240x240 panel on SPI1, polled DMA | 432 | 548 | 0 | 0 | main | HAL + tables | ui draw, ui input, gfx, ui core, ui graphs | none (hardware only) |
| gfx | gfx.c ui_fonts.h ui_keycaps.h ui_palettes.h | canvas renderer (cv_*), AA text, palettes; generated font / keycap data | 53743 | 74 | 61228 | 0 | main | lcd | ui draw, ui graphs, menu, ui input, ui layers | 26: backup_test, chord_test, detail_test, drumfuzz_test ... |
| sound core | core.h dsp.c engines.c | track/part structs, DSP helpers, engine table, factory patterns, engine memory pool | 1329 | 7304 | 51552 | 0 | ISR (+data) | engines: sample family | ui core, project, editor, ui layers, user presets | 50: audio_test, backup_test, chord_test, click_test ... |
| engines: synth set | eng_analog.c eng_phase.c eng_lofi.c eng_trio.c eng_wheel.c eng_formant.c eng_noise.c | 7 synth engines (ANALOG PHASE LOFI TRIO WHEEL VOICE NOISE) | 22759 | 3588 | 0 | 0 | ISR | sound core, gfx | icons, ui graphs, ui core | 49: audio_test, backup_test, chord_test, click_test ... |
| engines: sample family | eng_sample.c eng_grain.c eng_slice.c felucca_samples.h | SAMPLE / GRAIN / SLICE engines + 126 KB ADPCM sample data | 140537 | 5883 | 0 | 0 | ISR | sound core | ui core, ui graphs, project, editor, icons | 49: audio_test, backup_test, chord_test, click_test ... |
| engine: FM6 | eng_fm6.c fm6_core.c felucca_fm6.h fm6_vbank.c fm4_convert.c | 6-op FM (msfa), factory patches, voice bank, DIGITAL->FM6 conversion (eng_digital.c is NOT built: FELUCCA_FM4=0) | 11507 | 1561 | 525 | 0 | ISR (+main for bank) | storage, libc, sound core, user presets | ui graphs, ui core, project, editor, user presets | 50: audio_test, backup_test, chord_test, click_test ... |
| engine: PHYS | eng_phys.c phys_dsp.c phys_symp.c | physical-model engine (DaisySP / Rings ports) | 9329 | 0 | 0 | 0 | ISR | sound core | user presets, project | 49: audio_test, backup_test, chord_test, click_test ... |
| engine: DRUM | eng_drum.c drum_voice.c | 8-lane drum kit engine | 14352 | 4 | 0 | 0 | ISR | sound core | ui layers, ui core, ui graphs, ui draw, seq core | 49: audio_test, backup_test, chord_test, click_test ... |
| params | params.c | parameter descriptors, value formatting, page table | 6102 | 2 | 0 | 0 | main (+ISR reads) | libc, engine: FM6, engine: DRUM, sound core, engines: sample family | ui core, ui draw, ui graphs, ui input, project | 49: audio_test, backup_test, chord_test, click_test ... |
| mod matrix | mod.c | 4-slot modulation matrix per track | 1858 | 52 | 0 | 0 | ISR | params, sound core | fx, voice, ui graphs, ui draw, seq: midi clock/control | 49: audio_test, backup_test, chord_test, click_test ... |
| voice | voice.c | per-track voice render / note on-off | 3284 | 181 | 0 | 0 | ISR | sound core, params, mod matrix, engine: DRUM, fx | seq: midi clock/control, seq core, T-1 glue, fx, ui graphs | 49: audio_test, backup_test, chord_test, click_test ... |
| slicer insert | slicer.c | per-track 16-step gate / stutter insert (+32 KB pool buffer) | 1614 | 164 | 32768 | 0 | ISR | sound core | perform, fx, ui draw, ui core, ui graphs | 49: audio_test, backup_test, chord_test, click_test ... |
| fx | fx.c | mix_part, DIST/INSERT, chorus / delay / reverb buses (pool buffers) | 6469 | 1682 | 150066 | 0 | ISR | perform, sound core, mod matrix, slicer insert, voice | audio, ui graphs, menu, seq core, ui input | 49: audio_test, backup_test, chord_test, click_test ... |
| perform | perform.c | FX-hold layer: master performance effects | 7254 | 1877 | 0 | 0 | ISR | slicer insert, sound core, libc, fx | ui layers, seq core, fx, ui input, ui core | 49: audio_test, backup_test, chord_test, click_test ... |
| click | click.c | metronome blip | 257 | 4 | 0 | 0 | ISR | - | ui core, seq core, audio | 49: audio_test, backup_test, chord_test, click_test ... |
| usb | usb.c uac_fir.h | USB device: MIDI in/out rings, UAC audio, CDC, descriptors | 10591 | 5393 | 0 | 0 | ISR (poll) + main | ota, HAL + tables | editor, console, seq core, ota, main | 52: audio_test, backup_test, chord_test, click_test ... |
| midi_uart | midi_uart.c | TRS MIDI IN parser (RX DMA ring, polled from TIMER5) | 0 | 156 | 0 | 0 | ISR | usb | console | 50: audio_test, backup_test, chord_test, click_test ... |
| song chain | song_chain.c | song = rows of 4 saved projects (lanes) | 1302 | 13180 | 0 | 0 | ISR+main | libc, seq core, sound core, fx, seq: motion | editor, project, ui input, ui graphs, ui draw | 49: audio_test, backup_test, chord_test, click_test ... |
| seq core | seq.c | keyboard, scale, arp, pattern sequencer, transport, events_block | 7398 | 233 | 0 | 0 | ISR | perform, sound core, seq: midi clock/control, usb, click | ui input, ui layers, seq: midi clock/control, ui core, T-1 screen | 49: audio_test, backup_test, chord_test, click_test ... |
| seq: chord | chord.c | chord keys (CHRD / VOIC) | 846 | 27 | 0 | 0 | ISR | seq core, libc, sound core, params | ui graphs, seq: midi clock/control, ui input | 49: audio_test, backup_test, chord_test, click_test ... |
| seq: motion | motion.c | step automation + p-locks | 3028 | 1021 | 0 | 0 | ISR | libc, params, seq core, sound core, fx | ui core, editor, ui layers, ui input, ui draw | 49: audio_test, backup_test, chord_test, click_test ... |
| seq: midi clock/control | midi_clock.c midi_control.c | external MIDI clock follower; shared USB/TRS channel controls | 3102 | 2320 | 0 | 0 | ISR | seq core, seq: chord, voice, sound core, seq: motion | seq core, T-1 core, ui input, T-1 glue, T-1 screen | 49: audio_test, backup_test, chord_test, click_test ... |
| T-1 core | torso_t1_port.c torso_t1.c torso_t1_algo.c torso_t1_pipeline.c torso_t1_state.c torso_t1.h | freestanding shims, data model + event queue, algos, 192-PPQN pipeline, TEMP/GLOBAL state (torso_t1.h holds the .pool buffers) | 8554 | 344 | 13980 | 0 | ISR (state/ui helpers: main) | libc, T-1 glue, sound core, seq: midi clock/control | T-1 glue, T-1 ui helper, T-1 screen | 5: torso_phase1_test, torso_phase2_test, torso_phase3_test, torso_phase4_test ... |
| T-1 ui helper | torso_t1_ui.c | knob->parameter mapping and card text | 3612 | 9 | 0 | 0 | main | T-1 core, sound core, seq: midi clock/control, libc | T-1 screen, T-1 glue | 1: torso_phase5_test |
| T-1 glue | t1_glue.c | T-1 clock/transport/MIDI-out in the audio ISR | 340 | 18 | 0 | 0 | ISR | T-1 core, libc, seq core, voice, sound core | T-1 core, seq core | none (hardware only) |
| T-1 screen | ui_t1.c | T-1 screen + controls (its functions are inlined into ui_input.c / ui_draw.c: see caveats) | 88 | 3 | 0 | 0 | main | T-1 ui helper, gfx, T-1 core, seq core, panel | - | none (hardware only) |
| audio | audio.c | I2S ISR, events_block caller, mix to codec / USB | 6516 | 2069 | 0 | 76 | ISR | fx, usb, sound core, click, voice | console, main, ui draw, storage | 2: audio_test, click_test |
| panel | panel.c | button / encoder -> printed-label table, calibration | 469 | 6 | 0 | 32 | main | fx, gfx, project, ui core | ui input, menu, main, editor, project | 24: backup_test, chord_test, detail_test, drumfuzz_test ... |
| ui core | ui.c category.c favorites.c midi_learn.c ui_events.c ui_slice.c | UI state, page logic, categories, favorites, MIDI learn, automation list, slice edit | 12464 | 555 | 2069 | 0 | main | seq: motion, sound core, engines: sample family, libc, user presets | ui input, ui draw, ui graphs, ui layers, editor | 25: backup_test, chord_test, detail_test, drumfuzz_test ... |
| menu | menu_items.c ui_menu.c | MENU rows / settings, shared with the editor; menu screen | 4254 | 4 | 0 | 0 | main | gfx, ui core, libc, panel, usb | editor, ui input, ui layers, main | 22: chord_test, detail_test, drumfuzz_test, editor_test ... |
| icons | icons.c ui_icons.h | Fukiai icon cells (Huffman-coded) | 10103 | 0 | 0 | 0 | main | gfx, engines: synth set, engines: sample family, params, seq core | ui draw, ui graphs, ui layers, ui input, menu | 15: chord_test, detail_test, drumfuzz_test, fun10_test ... |
| ui graphs | ui_graph.c ui_fm6op.c | page graphs, scope, piano roll, FM6 operator pages | 26400 | 618 | 1024 | 0 | main | ui core, gfx, engine: FM6, params, engines: sample family | ui draw, editor, ui layers, menu | 15: chord_test, detail_test, drumfuzz_test, fun10_test ... |
| ui draw | ui_draw.c ui_help.c | header, knob cards, footer, frame, HELP hints | 30508 | 18 | 0 | 0 | main | ui core, gfx, icons, ui graphs, params | ui layers, ui input, ui core, T-1 screen, ui graphs | 15: chord_test, detail_test, drumfuzz_test, fun10_test ... |
| ui input | ui_input.c ui_name.c | LEDs, knob / button handling, step entry, NAME screen | 27802 | 41 | 56 | 0 | main | ui core, seq core, gfx, panel, user presets | ui layers, ui core, editor, ui draw, main | 15: chord_test, detail_test, drumfuzz_test, fun10_test ... |
| ui layers | ui_layer.c ui_tools.c | quick layers (FX GLO SCL EDIT REC held), SEQ TOOLS | 5489 | 96 | 2184 | 0 | main | ui core, perform, gfx, libc, seq: motion | ui input, ui core, ui draw | 15: chord_test, detail_test, drumfuzz_test, fun10_test ... |
| storage | storage_hw.c storage.c | SPI-NOR A/B-sector object store (flash HW side + format) | 1082 | 1 | 3840 | 0 | main | HAL + tables, audio, libc | engine: FM6, editor, user presets, project, ota | 11: backup_test, editor_test, fm6bank_test, fuzz_ed ... |
| user presets | upreset.c up_fm6.c | 32 user presets (RAM mirror + flash), FM6 patches | 3230 | 0 | 9888 | 0 | main | ui core, storage, sound core, engine: FM6, libc | editor, ui core, ui input, ui draw, ui graphs | 24: backup_test, chord_test, detail_test, drumfuzz_test ... |
| project | project.c settings_persist.c slice_store.c | 4 project slots (.noinit + flash), settings persistence, slice store | 9057 | 7 | 9238 | 15360 | main | ui core, sound core, libc, storage, engine: FM6 | editor, ui core, ui input, main, panel | 27: backup_test, chord_test, detail_test, drumfuzz_test ... |
| ota | ota.c ota_hw.c | M-UPGRADE SysEx firmware update entry | 1128 | 94 | 2688 | 0 | main | usb, HAL + tables, storage, lcd, gfx | usb | 2: ldr_test, ota_test |
| editor | editor.c editor_backup.c editor_fm6.c editor_learn.c editor_menu.c editor_palette.c editor_preferences.c | web-editor SysEx protocol | 14962 | 659 | 1748 | 0 | main | ui core, project, user presets, sound core, usb | main | 8: backup_test, editor_test, fm6bank_test, fuzz_ed ... |
| console | console.c | read-only CDC serial console | 864 | 72 | 0 | 0 | main | usb, audio, storage, sound core, voice | - | none (hardware only) |
| master | master.c | MASTER pot read before audio start | 60 | 4 | 0 | 0 | boot+main | HAL + tables | main | 1: audio_test |
| main | main.c | boot sequence and the ~60 fps main loop | 18114 | 1 | 0 | 4 | boot+main | ui core, HAL + tables, usb, sound core, panel | - | none (hardware only) |
| HAL + tables | fm1_adc.h fm1_flash.h fm1_guard.h fm1_input.h fm1_irq.h fm1_lcd_hw.h fm1_sys.h fm1_time.h fm1_led_anim.h felucca_tables.h | register-level hal/*.h (header-only; key/LED scan, flash driver in .ram_text) + generated tables | 17115 | 396 | 0 | 64 | ISR+boot | - | ui input, main, editor, ota, storage | none (hardware only) |
| (symbols not matched to a file) | - | `.L_*` leftovers, compiler-generated, unmatched statics | 709 | 594 | 11 | 28 | - | - | - | - |
| (unnamed bytes) | - | string literals, switch tables, padding, asm objects (crt0, vectors, isr): no symbol | 12761 | 4865 | 11 | 0 | - | - | - | - |
| **Total (objdump -h sections)** | | | **523032** | **55732** | **342876** | **15564** | | | | |

Column sums equal the objdump section totals by construction (flash 523032, RAM 55732, pool 342876, .noinit 15564).

## Biggest items (so the map can be judged by weight)

From `build/inventory/syms.txt` (symbol size) and the per-file appendix:

| Item | Where | Bytes | Region |
|---|---|---:|---|
| ADPCM sample data (5 sets, 11 zones) | felucca_samples.h via engines.c | 126717 | flash |
| Fonts (3 faces) | ui_fonts.h via gfx.c | 42904 | flash |
| Generated tables | felucca_tables.h | 11816 | flash |
| UAC FIR filter | uac_fir.h via usb.c | 7680 | flash |
| Icons / keycaps | ui_icons.h / ui_keycaps.h | 7551 / 5751 | flash |
| FX buses (`dly_buf` 131072, `rev_comb` 9874, `cho_buf` 4096 ...) | fx.c | 150066 | pool |
| Canvas pixel buffer `cv_px` | gfx.c | 59520 | pool |
| Engine memory `eng_mem` | engines.c | 51552 | pool |
| SLICER buffer `sl_buf` | slicer.c | 32768 | pool |
| T-1 buffers | torso_t1.h | 13980 | pool |
| Project slots `proj_slot` | project.c | 15360 | .noinit |
| Song chain `chain` | song_chain.c | 13096 (within the row's 13180) | RAM |
| Track state `trk[]` | core.h | 7296 | RAM |
| USB rings and buffers | usb.c | 5372 | RAM |

## The T-1 sequencer today (rows "T-1 core", "T-1 ui helper", "T-1 glue", "T-1 screen")

Flash 8554 + 3612 + 340 + 88 = **12594 B**, RAM 344 + 9 + 18 + 3 = **374 B**, pool **13980 B** (torso_t1.h). The T-1 screen's
inlined code (caveat 2) is not in these numbers. It is hooked into Felucca by `#ifdef T1_DEVICE` blocks in seq.c (line 59 and
1420) and ui.c (line 141); with `T1_DEVICE` undefined (host tests) those become stubs. No host test compiles t1_glue.c or ui_t1.c.
Everything else in the firmware that the T-1 uses (usb.c MIDI-out ring, voice.c, seq.c clock, midi_control.c, gfx/panel/ui)
is shared, not counted here.

## Platform core: what any firmware on this device needs

"Any firmware" = boots, reads the panel, draws, talks USB-MIDI, can be updated, and keeps time. Based on CLAUDE.md
(Hardware facts, Architecture) and the rows above; it is **a list of what the hardware needs, not a removal plan.**

| Need | Files | Flash B | RAM B | Pool B | Why it is core |
|---|---|---:|---:|---:|---|
| Boot, vectors, linker | hal/fm1_vec.S, hal/fm1_isr.S, crt0.S, app.ld, main.c (boot part), libc.c, hal/fm1_sys.h fm1_guard.h fm1_irq.h fm1_time.h | main 18114 (boot + main loop together), libc 258; asm objects are in "unnamed" | main 1, libc 4 | 0 | start-up (WDT, boot-loop guard, fatal vectors), memset/memcpy |
| Input (buttons, encoders, 27 keys, LEDs) | hal/fm1_input.h (TIMER5 10 kHz scan), panel.c, hal/fm1_adc.h | within "HAL + tables" 17115 (shared with fm1_flash.h etc.), panel 469 | panel 6, HAL 396 | 0 | the only user input |
| LCD | lcd.c, hal/fm1_lcd_hw.h, gfx.c (+ fonts) | lcd 432, gfx 53743 | lcd 548, gfx 74 | 61228 (`cv_px`) | the screen is the UX (VISION rule 1) |
| Audio ISR / timebase | audio.c, hal/fm1_audio.h fm1_timer.h | 6516 | 2069 | 0 | sequencer time is `events_block`'s sample count (CLAUDE.md "Timing comes from samples"), so the audio ISR is the clock even when no sound is wanted |
| USB device (MIDI in/out, firmware updates) | usb.c, hal/fm1_usb.h | 10591 (with uac_fir.h; the UAC audio and CDC parts are inside, see the FELUCCA_UAC / FELUCCA_CDC flags) | 5393 | 0 | MIDI out and the update path |
| TRS MIDI IN | midi_uart.c, hal/fm1_uart.h | 0 (inlined) | 156 | 0 | the only MIDI-in that works without a computer |
| Persistent storage | storage_hw.c, storage.c, hal/fm1_flash.h (.ram_text 2924 B) | 1082 + 2924 | 1 | 3840 | settings, projects |
| Update loader | loader/ (separate 8216 B image, ota.bin 6930 B) + ota.c, ota_hw.c | 1128 (app side) | 94 | 2688 | in-app update entry (CLAUDE.md: a crashing build can't re-enter update mode) |
| MIDI clock / transport | midi_clock.c, midi_control.c (in "seq: midi clock/control") + the clock/transport parts of seq.c | 3102 + part of 7398 | 2320 + part of 233 | 0 | external clock follow, Start/Stop; the parts of seq.c that are core vs the Felucca pattern sequencer are **unconfirmed** (they are one file) |

Not platform core (they exist for Felucca's groovebox role): the engines, voice/mod/fx/slicer/perform/click, song_chain, params,
the pattern sequencer parts of seq.c, chord/motion, upreset/project, the web editor, console, ui_* pages, menu. Which of these the
product keeps is for P2-002.

## Appendix: bytes per source file

Exact output of `attrib.py` (before grouping), same column definitions. eng_digital.c is not compiled (FELUCCA_FM4=0) and so
has no row; `?unmatched` are symbols no definition regex matched.

| File | .text | .ram_text | .data | .bss | .pool | .noinit |
|---|---:|---:|---:|---:|---:|---:|
| ?unmatched | 703 | 0 | 6 | 588 | 11 | 28 |
| audio.c | 6516 | 0 | 0 | 2069 | 0 | 76 |
| category.c | 222 | 0 | 0 | 0 | 0 | 0 |
| chord.c | 846 | 0 | 0 | 27 | 0 | 0 |
| click.c | 256 | 0 | 1 | 3 | 0 | 0 |
| console.c | 864 | 0 | 0 | 72 | 0 | 0 |
| core.h | 392 | 0 | 0 | 7296 | 0 | 0 |
| drum_voice.c | 5970 | 0 | 0 | 0 | 0 | 0 |
| dsp.c | 54 | 0 | 0 | 0 | 0 | 0 |
| editor.c | 13875 | 0 | 0 | 629 | 1176 | 0 |
| editor_backup.c | 309 | 0 | 0 | 30 | 572 | 0 |
| editor_fm6.c | 64 | 0 | 0 | 0 | 0 | 0 |
| editor_learn.c | 248 | 0 | 0 | 0 | 0 | 0 |
| editor_menu.c | 64 | 0 | 0 | 0 | 0 | 0 |
| editor_palette.c | 196 | 0 | 0 | 0 | 0 | 0 |
| editor_preferences.c | 206 | 0 | 0 | 0 | 0 | 0 |
| eng_analog.c | 5746 | 0 | 0 | 0 | 0 | 0 |
| eng_drum.c | 8382 | 0 | 0 | 4 | 0 | 0 |
| eng_fm6.c | 5316 | 0 | 0 | 1561 | 516 | 0 |
| eng_formant.c | 2809 | 0 | 4 | 0 | 0 | 0 |
| eng_grain.c | 4854 | 0 | 0 | 16 | 0 | 0 |
| eng_lofi.c | 2108 | 0 | 0 | 0 | 0 | 0 |
| eng_noise.c | 2456 | 0 | 0 | 0 | 0 | 0 |
| eng_phase.c | 3844 | 0 | 0 | 256 | 0 | 0 |
| eng_phys.c | 1961 | 0 | 0 | 0 | 0 | 0 |
| eng_sample.c | 3252 | 0 | 0 | 1351 | 0 | 0 |
| eng_slice.c | 5714 | 0 | 0 | 4516 | 0 | 0 |
| eng_trio.c | 3474 | 0 | 0 | 0 | 0 | 0 |
| eng_wheel.c | 2318 | 0 | 0 | 3328 | 0 | 0 |
| engines.c | 883 | 0 | 0 | 8 | 51552 | 0 |
| favorites.c | 420 | 0 | 0 | 0 | 520 | 0 |
| felucca_fm6.h | 1152 | 0 | 0 | 0 | 0 | 0 |
| felucca_samples.h | 126717 | 0 | 0 | 0 | 0 | 0 |
| felucca_tables.h | 11816 | 0 | 0 | 0 | 0 | 0 |
| fm1_adc.h | 96 | 0 | 0 | 0 | 0 | 0 |
| fm1_flash.h | 110 | 2924 | 0 | 0 | 0 | 0 |
| fm1_input.h | 1306 | 0 | 4 | 378 | 0 | 0 |
| fm1_irq.h | 468 | 0 | 0 | 0 | 0 | 64 |
| fm1_lcd_hw.h | 48 | 0 | 0 | 4 | 0 | 0 |
| fm1_led_anim.h | 59 | 0 | 0 | 0 | 0 | 0 |
| fm1_sys.h | 258 | 0 | 0 | 10 | 0 | 0 |
| fm1_time.h | 26 | 0 | 0 | 0 | 0 | 0 |
| fm4_convert.c | 2616 | 0 | 0 | 0 | 0 | 0 |
| fm6_core.c | 1753 | 0 | 0 | 0 | 0 | 0 |
| fm6_vbank.c | 670 | 0 | 0 | 0 | 9 | 0 |
| fx.c | 6465 | 0 | 4 | 1678 | 150066 | 0 |
| gfx.c | 4892 | 0 | 16 | 58 | 61228 | 0 |
| icons.c | 2552 | 0 | 0 | 0 | 0 | 0 |
| lcd.c | 432 | 0 | 0 | 548 | 0 | 0 |
| libc.c | 254 | 0 | 4 | 0 | 0 | 0 |
| main.c | 18114 | 0 | 0 | 1 | 0 | 4 |
| master.c | 56 | 0 | 4 | 0 | 0 | 0 |
| menu_items.c | 2334 | 0 | 0 | 4 | 0 | 0 |
| midi_clock.c | 676 | 0 | 0 | 40 | 0 | 0 |
| midi_control.c | 2426 | 0 | 0 | 2280 | 0 | 0 |
| midi_learn.c | 458 | 0 | 0 | 0 | 0 | 0 |
| midi_uart.c | 0 | 0 | 0 | 156 | 0 | 0 |
| mod.c | 1854 | 0 | 4 | 48 | 0 | 0 |
| motion.c | 3028 | 0 | 0 | 1021 | 0 | 0 |
| ota.c | 796 | 0 | 0 | 94 | 2688 | 0 |
| ota_hw.c | 332 | 0 | 0 | 0 | 0 | 0 |
| panel.c | 468 | 0 | 1 | 5 | 0 | 32 |
| params.c | 6102 | 0 | 0 | 2 | 0 | 0 |
| perform.c | 6910 | 0 | 344 | 1533 | 0 | 0 |
| phys_dsp.c | 6358 | 0 | 0 | 0 | 0 | 0 |
| phys_symp.c | 1010 | 0 | 0 | 0 | 0 | 0 |
| project.c | 8032 | 0 | 1 | 6 | 9238 | 15360 |
| seq.c | 7393 | 0 | 5 | 228 | 0 | 0 |
| settings_persist.c | 742 | 0 | 0 | 0 | 0 | 0 |
| slice_store.c | 282 | 0 | 0 | 0 | 0 | 0 |
| slicer.c | 1614 | 0 | 0 | 164 | 32768 | 0 |
| song_chain.c | 1302 | 0 | 0 | 13180 | 0 | 0 |
| storage.c | 798 | 0 | 0 | 0 | 3840 | 0 |
| storage_hw.c | 284 | 0 | 0 | 1 | 0 | 0 |
| t1_glue.c | 340 | 0 | 0 | 18 | 0 | 0 |
| torso_t1.c | 1970 | 0 | 0 | 36 | 0 | 0 |
| torso_t1.h | 0 | 0 | 0 | 0 | 13980 | 0 |
| torso_t1_algo.c | 2508 | 0 | 0 | 0 | 0 | 0 |
| torso_t1_pipeline.c | 1956 | 0 | 0 | 280 | 0 | 0 |
| torso_t1_port.c | 498 | 0 | 0 | 0 | 0 | 0 |
| torso_t1_state.c | 1622 | 0 | 0 | 28 | 0 | 0 |
| torso_t1_ui.c | 3612 | 0 | 0 | 9 | 0 | 0 |
| uac_fir.h | 7680 | 0 | 0 | 0 | 0 | 0 |
| ui.c | 8930 | 0 | 0 | 105 | 1549 | 0 |
| ui_draw.c | 29873 | 0 | 0 | 18 | 0 | 0 |
| ui_events.c | 1592 | 0 | 0 | 0 | 0 | 0 |
| ui_fm6op.c | 276 | 0 | 0 | 2 | 0 | 0 |
| ui_fonts.h | 42904 | 0 | 0 | 0 | 0 | 0 |
| ui_graph.c | 26124 | 0 | 0 | 616 | 1024 | 0 |
| ui_help.c | 635 | 0 | 0 | 0 | 0 | 0 |
| ui_icons.h | 7551 | 0 | 0 | 0 | 0 | 0 |
| ui_input.c | 26905 | 0 | 0 | 41 | 0 | 0 |
| ui_keycaps.h | 5751 | 0 | 0 | 0 | 0 | 0 |
| ui_layer.c | 3109 | 0 | 0 | 96 | 0 | 0 |
| ui_menu.c | 1920 | 0 | 0 | 0 | 0 | 0 |
| ui_name.c | 897 | 0 | 0 | 0 | 56 | 0 |
| ui_palettes.h | 180 | 0 | 0 | 0 | 0 | 0 |
| ui_slice.c | 842 | 0 | 0 | 450 | 0 | 0 |
| ui_t1.c | 88 | 0 | 0 | 3 | 0 | 0 |
| ui_tools.c | 2380 | 0 | 0 | 0 | 2184 | 0 |
| up_fm6.c | 702 | 0 | 0 | 0 | 3728 | 0 |
| upreset.c | 2528 | 0 | 0 | 0 | 6160 | 0 |
| usb.c | 2890 | 0 | 21 | 5372 | 0 | 0 |
| voice.c | 3284 | 0 | 0 | 181 | 0 | 0 |
