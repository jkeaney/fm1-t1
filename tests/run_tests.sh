#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
# Host tests of the Felucca sources (no hardware). Run from the repo root after ./build.sh:
#   tests/run_tests.sh
#
# Regression suite (tests/regress.c, tests/target_budget.py; details at the top of regress.c):
#   golden renders  every engine x preset, the GM map on DRUM, voice modes, FX sends, a 4-track mix: one hash
#                   each in tests/golden.txt. A change of the sound fails with the list of renders.
#   health          clipping, DC, peak level, voices free after the release, silence at the end.
#   CPU             instructions / sample per preset and mix (tests/cpu_baseline.txt, +25 %), ns printed;
#                   target: loop instructions of the render functions in the pi32v2 disassembly
#                   (tests/target_budget.txt, +10 %; exact, static).
#   voices          the budget of 8, steal fades, MONO / LEGATO / UNISON keep their note, the VOICE cap,
#                   no hanging notes on any MIDI / key routing.
# ENGINE MEMORY (tests/union_test.c, 1.5): engine switches on a part (every pair and A -> B -> A of the engines that
#                   share the part's state region, with others), hashed against 1.4's (tests/union_golden.txt).
# UI renders (tests/ui_render.c, P3-006): the screens that are left, every palette, from the real drawing code: the T-1 screen
#                   (pages, SHIFT / GLOBAL / TEMP, mute, track, a lit card), the menu (every row, ABOUT, INFO, CALIBRATION), UPDATE
#                   MODE, the splash, the dialog, the count-in, the header's states and batteries. The layout lint (no text
#                   off the screen, cut, hidden, overlapping or spilling out of its cell / card; only free text
#                   ellipsised), GREY gray, MONO neutral, the draw cost, the text audit (build/ui_new/text_audit.tsv);
#                   PNGs of GREY MONO GREEN PAPER NIGHT in build/ui_new (tests/ui_render.py), the findings in build/ui_new/report.txt
#                   (the T-1 screen's own findings are listed there and do not fail: FB-007);
#                   the alignment: every text / icon / keycap meant to be centred, or on its neighbours' line, by its
#                   ink against its box (cells, chips, buttons, rows, the keycaps' pills), every screen in FLAT and LINE and
#                   every palette, over every value it can show; 1 px off or more fails (build/ui_new/align.txt).
# UI (tests/ui_test.c): the UI sources against stub display / buttons / knobs: the T-1 screen is the page (HOME held opens the
#                   MENU, a dialog takes the screen), the menu's tabs and rows, SCREEN OFF / WAKE, the LEDs (idle, scale), the
#                   header, STYLE, KNOB ACCEL, CLICK / USB / retired settings, GREY gray, MONO neutral.
# Audio / persistence: the real C paths against simulated DMA and NOR flash: bounded overload
#                   fades, shared-voice limits, deferred settings and retries, failed-save rollback, the autosave
#                   (1.2: when it writes, the power-on restore, damaged copies, a write cut short, wear over a session),
#                   transport-stop timeouts, MIDI and UART recovery.
# CHORD (tests/chord_test.c): the chord keys (src/chord.c): diatonic triads / sevenths of several scales and roots,
#                   the fixed shapes and voicings (at most 4 notes), names, MONO plays the root, a release ends
#                   exactly what its key / MIDI note started, recording, the ARP, MIDI IN, kits ignore CHRD.
# MOD (tests/mod_test.c): the modulation matrix: slots that do nothing are bit-identical, every source on each
#                   kind of destination, clamping, MIDI CC1 / CC11 / aftertouch routing, the cost of 4 active
#                   slots (at most +6 %), demos in build/mod_demo/.
# SOUND (tests/sound14_test.c; #132 #104 #148): the matrix's S&H / SLEW sources (a value per LFO cycle, the glide) and
#                   DEPTH destination (the LFO's gain; none = untouched), ANALOG WAVE SYNC / SUB (pitch, no DC, bounded,
#                   the band-limited restart), SPREAD (SPRD 0 / MONO / LEGATO bit for bit, a note hard left / right, the
#                   mono sum, UNISON, the mute key on both sides, mono sends), the cost; demos in
#                   build/sound14_demo/.
# SOUND 1.5 (tests/sound15_test.c; #104 #175): ANALOG's filter TYPE (LP bit for bit, every other engine ignores it, BP / HP
#                   responses around the cutoff, RES, bounded at the corners), ENV SYNC (OFF bit for bit, the note values
#                   on the knob, attack / decay / release times at 60 / 120 / 173 BPM and on the external clock), the cost;
#                   demos in build/sound15_demo/ (analog_bp, analog_hp, env_sync).
# PHASE FOLD 1.5.5 (tests/fold155_test.c; #104): E7 FOLD (0..127, default 0, the presets at 0), FOLD 0 bit for bit with
#                   1.5.1's loop, bounded and without DC at FOLD 1..127 (every WAVE, MIX / RING, SUB, notes 12..127),
#                   brighter with FOLD, the gain falling with the pitch above A4, the cost; build/fold155_demo/phase_fold.wav.
# SLICE (tests/slice_test.c): slice tables, AUTO onsets of a user-slot loop, reverse, keys, modes, the MAN slices
#                   (SLICES page) and their store in the slot (src/slice_store.c); the presets and the loop;
#                   demos in build/slice_demo/.
# SAMPLE (tests/sample_test.c, 1.5): PIANO is the 5-zone piano of 1.0 .. 1.1.5 again: zones, splits, pitch over the
#                   keyboard (YIN), 0.75 s at a root, the aliases, GRAIN SRC 0; SLICE's PIANO its middle C zone (shared);
#                   1.2's PIANO HD user-slot file (gen_samples.py --user-slot: build/piano_hd/PIANO_HD), as old users may
#                   have installed it: valid in USR1 and bit for bit as SET 0.
# INPUT (tests/input_test.c): the key / button debounce of hal/fm1_input.h against the TIMER5 scan and bouncing
#                   contacts: a press within 2 scans (<= 2.3 ms), one note per bouncy press, no early or hanging
#                   release, stray samples ignored, fast repeats, the encoders' detents; the LED scan: lit LEDs every
#                   frame, dim ones a short pulse (the second line write) every frame, each only on its own column;
#                   the breath (#119): dark .. ~60 % of lit (DIM LO ~30 %), smooth, no dark run over ~10 ms near its peak;
#                   the power-on sweep (1.1, hal/fm1_led_anim.h): its length, every LED every frame at its level, the
#                   head left to right with its tail, the buttons, the end on the idle glow, the tick as before after;
#                   the idle animation (1.2, MENU > ANIM IDLE): endless, every LED every frame, the light there and back,
#                   the buttons breathing, a key down ends it before its next frame.
# CLICK (tests/click_test.c, 1.1): the metronome (src/click.c) and the count-in (src/seq.c) through audio.c: each beat at
#                   the first sample of its block, with a 1/16 track's steps 1 5 9 13 (internal and external clock, DIV 1/8
#                   and SWING too), the accent, the frequencies, length, LEVEL, MASTER, OFF / REC / ON, USB audio bit for bit
#                   without it; COUNT-IN 1 / 2 BARS (step 1 exactly N bars after PLAY, STOP cancels, PLAY again nothing,
#                   never with an external clock), notes in its last eighth onto step 1; build/click_demo/.
# USB audio (tests/uac_test.c): the UAC1 descriptors as a host parses them (with and without CDC; 44100 and
#                   48000 Hz), the ring and packetiser: 44.1 frames per packet, every frame in order (bit for bit),
#                   underrun / overrun, restart; SET_CUR; the 44.1 -> 48 kHz resampler (every frame against a
#                   double-precision one, response, SNR, cost); the stream at 48 kHz (47..49 frames per packet,
#                   fast and slow I2S clocks, switching rates mid-stream).
# web (web/test_backup.mjs): the backup capture, validation before writes, restore order.
# Browser emulator (web/emu, when emcc is there): the firmware in WebAssembly (build/emu) boots, plays keys and MIDI,
#                   draws, lights its LEDs, keeps a save across instances, plays a song bit for bit as the same file
#                   built with cc (web/emu/native_check.c); the cost of 1 s of a heavy song against real time.
# PHYS (tests/phys_test.c): stability over the whole parameter and pitch range, the worst-case cost against
#                   the heaviest factory preset, demos in build/phys_demo/; tests/phys_ref.cpp compares the
#                   fixed-point models with DaisySP's float originals when DaisySP is there (DAISYSP=path).
# DRUM (tests/drum_test.c): the drum voices (src/drum_voice.c): pitch, decay, centroid and level against
#                   Felucca's targets, the controls' directions, no clipping, DC, retriggers, the hat choke, the
#                   kick on a small speaker; the DRUM engine (src/eng_drum.c): its key map, the 8 lanes together,
#                   one hit per lane, the choke between lanes; the cost per voice; demos in build/drum_demo/.
# NOISE (tests/noise_test.c): the engine (src/eng_noise.c): COLR's slope (white, pink, brown), the filter and the
#                   register clock following the key, META periodic at the key, no DC, no clipping at the
#                   corners, a note from silence the same twice, the cost per voice; demos in build/noise_demo/.
# DIGITAL -> FM6: the DIGITAL engine was removed in P3-003 (its DSP and fm4_test.c, digital_test.c, fm4_div0_test.c,
#                   ui_test_fm4 and its render with it). src/fm4_convert.c stays until R6 (SYSTEM_MAP).
# FM6 (tests/fm6_test.c): the 6-operator FM engine (src/eng_fm6.c, src/fm6_core.c): the 32 algorithms' carriers, the
#                   operator envelopes (stages, rates, the voice ending), bit-stable notes, a click-free retrigger, no DC /
#                   clipping over the factory patches, the macros' directions, PTCH, pack / unpack and the SysEx
#                   layouts, the 6-voice cap, the cost per voice; demos in build/fm6_demo/.
# Sanitizers (ASan + UBSan, when the compiler has them; SANITIZE=0 skips): the stored-data and protocol tests again
#                   (loader, M-UPGRADE entry, projects, fun10).
#                   UBSan leaves out the DSP's intended wraps (signed overflow, shifts) and the XIP rebase of a user zone's
#                   offset (bounds, object-size, pointer-overflow: correct on the device's flat flash, not in C's model).
# Change baseline entries only for reviewed, intentional differences in sound or cost;
# retain every unaffected golden / CPU / target entry. VERBOSE=1: every render.
set -e
export AC79_SDK="${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}"
cd "$(dirname "$0")/.."
OUT=build/host
mkdir -p "$OUT"
CC="${CC:-cc} -O1 -Wall -Wno-unused-function"
fail=0
run() { step="$1"; echo "== $1"; shift; "$@" || { echo ">>> STEP FAILED: $step <<<"; fail=1; }; }

$CC -o "$OUT/storage_test" tests/storage_test.c
run "flash storage (A/B, torn writes)" "$OUT/storage_test"

$CC -o "$OUT/upreset_test" tests/upreset_test.c
run "user presets (UP_PUT parser, bank round trip, versions, PHYS DRUM and SAMPLE PERC -> DRUM, grid records, DIGITAL kept)" "$OUT/upreset_test"

$CC -o "$OUT/input_test" tests/input_test.c
run "keys and buttons: fast press, long release, bouncy contacts (one note each), glitches, encoders" "$OUT/input_test"

$CC -o "$OUT/midi_uart_test" tests/midi_uart_test.c
run "TRS MIDI parser" "$OUT/midi_uart_test"

$CC -O2 -Ibuild/gen -Ifirmware/src -o "$OUT/torso_phase1_test" tests/torso_phase1_test.c firmware/src/torso_t1.c -lm
run "Torso T-1 Phase 1 (environment baseline, RAM reservation, 192 PPQN timer, FreeRTOS priority hierarchy, micro-queue)" "$OUT/torso_phase1_test"

$CC -O2 -Ibuild/gen -Ifirmware/src -o "$OUT/torso_phase2_test" tests/torso_phase2_test.c firmware/src/torso_t1.c firmware/src/torso_t1_algo.c -lm
run "Torso T-1 Phase 2 (Bjorklund Euclidean rhythms, pace/ratchet, harmony/voicing, Galois LFSR, p-locks)" "$OUT/torso_phase2_test"

$CC -O2 -Ibuild/gen -Ifirmware/src -o "$OUT/torso_phase3_test" tests/torso_phase3_test.c firmware/src/torso_t1.c firmware/src/torso_t1_algo.c firmware/src/torso_t1_pipeline.c -lm
run "Torso T-1 Phase 3 (dual output routing, 6-track 192 PPQN pipeline, polyphony stress, ISR latency)" "$OUT/torso_phase3_test"

$CC -O2 -Ibuild/gen -Ifirmware/src -o "$OUT/torso_phase4_test" tests/torso_phase4_test.c firmware/src/torso_t1.c firmware/src/torso_t1_algo.c firmware/src/torso_t1_pipeline.c firmware/src/torso_t1_state.c -lm
run "Torso T-1 Phase 4 (TEMP mode, snapshot revert latency, global track offsets, SPI flash layout)" "$OUT/torso_phase4_test"

$CC -O2 -Ibuild/gen -Ifirmware/src -o "$OUT/torso_phase5_test" tests/torso_phase5_test.c firmware/src/torso_t1.c firmware/src/torso_t1_algo.c firmware/src/torso_t1_pipeline.c firmware/src/torso_t1_state.c firmware/src/torso_t1_ui.c -lm
run "Torso T-1 Phase 5 (UI & control surface, track/cycle navigation, 4-page encoder dispatch, card formatting, p-lock step editing)" "$OUT/torso_phase5_test"


HALF=$(sed -n 's/^#define HALF_FRAMES \([0-9]*\).*/\1/p' firmware/src/core.h)
$CC -O2 -DT_CDC=1 -DHALF_FRAMES=$HALF -o "$OUT/uac_test" tests/uac_test.c -lm
run "USB audio input: descriptors (with CDC), ring and packets, 44.1 -> 48 kHz resampler, 48 kHz stream" "$OUT/uac_test"
$CC -O2 -DT_CDC=0 -DHALF_FRAMES=$HALF -o "$OUT/uac_test_nocdc" tests/uac_test.c -lm
run "USB audio input: descriptors (without CDC), ring and packets, resampler, 48 kHz stream" "$OUT/uac_test_nocdc"
# USB descriptor layouts (#67): CDC UAC LAYOUT CDC-presented 48K; layout 0 and the console left out = 1.0's bytes
# (+ the 48 kHz rate with 48K = 1, the default; 48K = 0: byte for byte; + the AC collection's audio streaming
# interface first, FELUCCA_UAC_AS_FIRST = 1, the default since 1.6)
for v in 1.1.0.1.1 1.1.1.1.1 1.1.2.1.1 1.1.3.1.1 1.1.0.0.1 1.1.2.0.1 0.1.0.1.1 1.1.0.1.0 1.1.0.0.0 1.1.2.1.0 0.1.0.1.0 \
         1.0.0.1.1 1.0.1.1.1 1.0.0.0.1 0.0.0.1.1; do
    IFS=. read -r t_cdc t_uac t_lay t_on t_48 <<EOF
$v
EOF
    $CC -DT_CDC="$t_cdc" -DT_UAC="$t_uac" -DT_LAYOUT="$t_lay" -DT_ON="$t_on" -DT_48K="$t_48" -o "$OUT/usb_desc_test" \
        tests/usb_desc_test.c
    run "USB descriptors: CDC $t_cdc (presented $t_on), UAC $t_uac (48 kHz $t_48), layout $t_lay" "$OUT/usb_desc_test"
done
# FELUCCA_UAC_AS_FIRST=0 (#67): the AC collection of 1.0 .. 1.5 (MIDI first), 1.0's bytes; the parser model of the
# kernel AppleUSBAudio (macOS <= 15) stops before the audio streaming interface there
for v in 1.1.0.1.1 1.1.0.0.1 1.1.0.1.0 1.1.0.0.0 0.1.0.1.1; do
    IFS=. read -r t_cdc t_uac t_lay t_on t_48 <<EOF
$v
EOF
    $CC -DT_CDC="$t_cdc" -DT_UAC="$t_uac" -DT_LAYOUT="$t_lay" -DT_ON="$t_on" -DT_48K="$t_48" -DT_ASF=0 \
        -o "$OUT/usb_desc_test" tests/usb_desc_test.c
    run "USB descriptors (AC collection MIDI first): CDC $t_cdc (presented $t_on), UAC $t_uac (48 kHz $t_48)" \
        "$OUT/usb_desc_test"
done

[ -f build/felucca.fwsc ] || { echo "run ./build.sh first"; exit 1; }

$CC -DOWN_PKG=1 -o "$OUT/ota_test" tests/ota_test.c
run "M-UPGRADE entry (own loader)" "$OUT/ota_test" build/felucca.fwsc

if [ -f "$AC79_SDK/cpu/wl82/tools/uboot.boot" ]; then
    head -c 200000 build/felucca.bin > "$OUT/old_app.bin"
    python3 tools/fm1pkg_make.py "$OUT/old_app.bin" build/loader/ota.bin "$OUT/old.fwsc" >/dev/null
    $CC -o "$OUT/ldr_test" tests/ldr_test.c
    run "update loader: other app -> this build" "$OUT/ldr_test" "$OUT/old.fwsc" build/felucca.fwsc
else
    echo "== skip loader update test (no JieLi AC79 SDK in $AC79_SDK)"
fi

if [ -f build/gen/felucca_tables.h ]; then
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/hostsim" tests/hostsim.c -lm
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/scale_test" tests/scale_test.c -lm
    run "scales: white-key mapping and note lifecycle" "$OUT/scale_test"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/chord_test" tests/chord_test.c -lm
    run "chord keys: diatonic and fixed chords, voicings, MONO root, releases, recording, ARP, MIDI IN, kits" "$OUT/chord_test"
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/speaker_test" tests/speaker_test.c -lm
    run "SPEAKER EQ: FLAT / LOWCUT / BASS+ responses, BASS+ harmonics of the bass, the sub cut, no offset after, headroom on a full-scale kick (#180)" "$OUT/speaker_test"
    run "DSP render (ANALOG preset 0)" "$OUT/hostsim" 0 0 1 "$OUT/render.wav"
    mkdir -p build/tracks_demo
    run "TRACKS: 4-track pattern, live recording (lengths, swing), voice budget, engine switch, cost" env TRACKS=build/tracks_demo "$OUT/hostsim" 0 0 1 "$OUT/tracks.wav"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/project_test" tests/project_test.c -lm
    run "project formats (FUN1..FUN5 -> FUN6, the grid and song chain; DIGITAL tracks -> FM6, SAMPLE PERC -> DRUM)" "$OUT/project_test"
    $CC -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/midi_control_test" tests/midi_control_test.c -lm
    run "USB/TRS clock, bend, sustain, ownership and panic recovery" "$OUT/midi_control_test"
    $CC -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/theme_test" tests/theme_test.c -lm
    run "themes: contrast, text blending and font metrics" "$OUT/theme_test"
    $CC -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/text_ref_test" tests/text_ref_test.c -lm
    run "text: pens, kerning and pixels equal the reference renderer" "$OUT/text_ref_test"
    if python3 -c "import PIL" 2>/dev/null; then
        run "text spacing: glyph gaps against the font's own (16x supersampled), S M <= 0.25 px, L <= 0.5 px" \
            python3 tests/text_spacing_test.py build/gen/ui_fonts.h
    fi
    $CC -O1 -w -Ibuild/gen -o "$OUT/settings_test" tests/settings_test.c
    run "settings: PER1..PER4 migration, palette ids and preference preservation" "$OUT/settings_test"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_test" tests/ui_test.c -lm
    run "UI (P3-006): the T-1 screen and the menu: tabs, rows, SCREEN OFF / WAKE, LEDS, header, STYLE, GREY gray, MONO neutral" "$OUT/ui_test"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/t1_link_test" tests/t1_link_test.c -lm
    run "T-1 link (P3-001): the real seq.c + torso_t1*.c + t1_glue.c + ui_t1.c: 2 bars on MIDI channel 2, STOP sends CC123" "$OUT/t1_link_test"
    FV=$(sed -n 's/^#define FELUCCA_VERSION "\([^"]*\)".*/\1/p' firmware/src/felucca.c)   # (the splash, ABOUT: the real version)
    $CC -O1 -w "-DFELUCCA_VERSION=\"$FV\"" -Ibuild/gen -Ifirmware/src -Itests -o "$OUT/ui_render" tests/ui_render.c -lm
    mkdir -p build/ui_new/ppm build/ui_slot
    run "UI renders: layout lint (every screen and palette, every page, engine and column value), GREY gray, MONO neutral, alignment by ink (1 px fails), draw cost" \
        "$OUT/ui_render" build/ui_new build/ui_slot
    if python3 -c "import PIL" 2>/dev/null; then python3 tests/ui_render.py build/ui_new build/ui_slot; fi
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/audio_test" tests/audio_test.c -lm
    run "audio: overload protection, bounded fades and DMA diagnostics" "$OUT/audio_test"
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/click_test" tests/click_test.c -lm
    run "metronome and count-in: beats sample for sample (internal and external clock), accent, level, MASTER, not in USB, count-in timing, notes onto step 1" "$OUT/click_test"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/persistence_test" tests/persistence_test.c -lm
    run "persistence: deferred settings, retry and failed-save rollback" "$OUT/persistence_test"
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/mod_test" tests/mod_test.c -lm
    mkdir -p build/mod_demo
    run "modulation matrix: off = bit-identical, the math, MIDI CC1 / CC11 / aftertouch, cost, demos" "$OUT/mod_test" build/mod_demo
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/sound14_test" tests/sound14_test.c -lm
    mkdir -p build/sound14_demo
    run "sound items: matrix S&H / SLEW / DEPTH, ANALOG SYNC / SUB, SPREAD (bit for bit at 0, L / R, mono sum), cost, demos" \
        "$OUT/sound14_test" build/sound14_demo
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/sound15_test" tests/sound15_test.c -lm
    mkdir -p build/sound15_demo
    run "1.5 sound items: ANALOG filter TYPE LP / BP / HP (responses, LP and other engines bit for bit), ENV SYNC (times at several tempi), cost, demos" \
        "$OUT/sound15_test" build/sound15_demo
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/fold155_test" tests/fold155_test.c -lm
    mkdir -p build/fold155_demo
    run "PHASE FOLD (1.5.5): FOLD 0 bit for bit, bounded, no DC, brighter with FOLD, alias-aware gain, cost, demo" \
        "$OUT/fold155_test" build/fold155_demo
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/swing_test" tests/swing_test.c -lm
    run "SWING: track + global at most 100, sequencer step lengths, the SWG display" "$OUT/swing_test"
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/regress" tests/regress.c -lm
    run "regression: golden renders, health, voices, CPU budget" "$OUT/regress" tests/golden.txt tests/cpu_baseline.txt
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/union_test" tests/union_test.c -lm
    run "engine memory (1.5): every switch between the engines with per-part state sounds as in 1.4 (720 sequences)" \
        "$OUT/union_test" tests/union_golden.txt
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/phys_test" tests/phys_test.c -lm
    mkdir -p build/phys_demo
    run "PHYS: stability C-1..G9 over the parameter corners, worst-case cost against PHASE WIRE, demos" "$OUT/phys_test" build/phys_demo
    D=${DAISYSP:-vendor/DaisySP}/Source
    if [ -d "$D/PhysicalModeling" ] && command -v c++ >/dev/null 2>&1; then
        $CC -O2 -w -Ibuild/gen -Ifirmware/src -Itests -c -o "$OUT/phys_fixed.o" tests/phys_fixed.c
        c++ -O2 -std=c++14 -w -I"$D" -I"$D/Utility" -o "$OUT/phys_ref" tests/phys_ref.cpp \
            "$D/PhysicalModeling/modalvoice.cpp" "$D/PhysicalModeling/resonator.cpp" "$D/PhysicalModeling/stringvoice.cpp" \
            "$D/PhysicalModeling/KarplusString.cpp" "$D/Filters/svf.cpp" "$D/Utility/dcblock.cpp" \
            "$D/Dynamics/crossfade.cpp" "$OUT/phys_fixed.o"
        run "PHYS: the fixed-point models against DaisySP's float originals (mode frequencies, decays, Svf)" "$OUT/phys_ref"
    else
        echo "== skip PHYS reference test (no DaisySP: set DAISYSP to a checkout)"
    fi
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/drum_test" tests/drum_test.c -lm
    mkdir -p build/drum_demo
    run "DRUM: voice targets, controls, no clipping, retrigger, hat choke, the kick on a small speaker, keys, 8 lanes, cost, demos" "$OUT/drum_test" build/drum_demo
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/noise_test" tests/noise_test.c -lm
    mkdir -p build/noise_demo
    run "NOISE: colour slopes, key-tracked filter and clock, META period, DC, clipping, retrigger, cost, demos" "$OUT/noise_test" build/noise_demo
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/fm6_test" tests/fm6_test.c -lm
    mkdir -p build/fm6_demo
    run "FM6: algorithms, envelopes, retrigger, DC, clipping, macros, patch formats, voices, cost, demos" "$OUT/fm6_test" build/fm6_demo
    # SLICE is in the standard build (firmware/src/core.h): its test always runs (after #22 by andreahaku)
    if grep -q '^#define SLC_BREAK_BPM ' build/gen/felucca_samples.h; then
        mkdir -p build/slice_demo
        python3 tests/slice_loop.py build/slice_demo/loop
        python3 tools/fm1_sample_upload.py build LOOP build/slice_demo/loop build/slice_demo/loop.wav:60 >/dev/null
        $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/slice_test" tests/slice_test.c -lm
        run "SLICE: tables, onsets, reverse, keys, modes, MAN slices and their store, demos" "$OUT/slice_test" \
            build/slice_demo/loop build/slice_demo
    else
        echo "== SLICE: build/ was made with FELUCCA_SLICE=0 (no BREAK); run ./build.sh without it first"
        echo ">>> STEP FAILED: SLICE not in samples <<<"
        fail=1
    fi
    # SAMPLE's PIANO (1.5: as 1.1.5), SLICE's PIANO (its middle C zone), 1.2's PIANO HD in a user slot
    if grep -q '^    {"PIANO", 0, 5},' build/gen/felucca_samples.h; then
        mkdir -p build/sample_demo build/piano_hd
        python3 tools/gen_samples.py --user-slot PIANO "PIANO HD" build/piano_hd/PIANO_HD >/dev/null
        $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/sample_test" tests/sample_test.c -lm
        run "SAMPLE: PIANO (5 zones as 1.1.5: splits, pitch, length, GRAIN), SLICE's PIANO, a PIANO HD slot" "$OUT/sample_test" \
            build/piano_hd/PIANO_HD build/sample_demo
    else
        echo "== skip SAMPLE's PIANO (no CC0 library: assets/samples-cc0, tools/fetch_cc0.py)"
    fi
else
    echo "== skip hostsim (run ./build.sh once)"
fi

# ASan + UBSan: the stored-data and protocol paths (fixed inputs: the same run every time)
SAN="-O1 -g -fsanitize=address,undefined -fno-sanitize=signed-integer-overflow,shift,bounds,object-size,pointer-overflow"
SAN="$SAN -fno-sanitize-recover=undefined -w"
san_ok() {
    printf 'int main(void){return 0;}\n' > "$OUT/san_probe.c" &&
        ${CC%% *} $SAN -o "$OUT/san_probe" "$OUT/san_probe.c" 2>/dev/null && "$OUT/san_probe"
}
if [ "${SANITIZE:-1}" = 0 ]; then
    echo "== skip the sanitizer runs (SANITIZE=0)"
elif [ ! -f build/gen/felucca_tables.h ]; then
    echo "== skip the sanitizer runs (run ./build.sh once)"
elif ! san_ok; then
    echo "== skip the sanitizer runs (the compiler has no ASan / UBSan)"
else
    mkdir -p "$OUT/asan"
    A="$OUT/asan"
    SCC="${CC%% *} $SAN -Ibuild/gen -Ifirmware/src"
    export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
    if [ -f "$OUT/old.fwsc" ]; then
        $SCC -o "$A/ldr_test" tests/ldr_test.c
        run "ASan/UBSan: update loader (other app -> this build)" "$A/ldr_test" "$OUT/old.fwsc" build/felucca.fwsc
    fi
    $SCC -DOWN_PKG=1 -o "$A/ota_test" tests/ota_test.c
    run "ASan/UBSan: M-UPGRADE entry (own loader)" "$A/ota_test" build/felucca.fwsc
    for t in project_test; do
        $SCC -o "$A/$t" tests/$t.c -lm
        run "ASan/UBSan: $t" "$A/$t"
    done
fi

run "regression: target cost of the render loops (pi32v2 disassembly)" python3 tests/target_budget.py \
    build/felucca.dis tests/target_budget.txt

run "installer CLI (fm1_install.py) against a simulated FM-1" python3 tests/install_test.py

run "timing tool self-test (P4-008): synthetic logs through the TIMING_SPEC 6.2 verdicts" python3 tests/fm1_timing_selftest.py

if command -v node >/dev/null 2>&1; then
    run "web backup: capture, validation before writes, restore order" node web/test_backup.mjs
else
    echo "== skip web tests (no node)"
fi

if ! command -v emcc >/dev/null 2>&1; then
    echo "== skip the browser emulator (no emcc: Emscripten builds web/emu)"
elif ! command -v node >/dev/null 2>&1 || [ ! -f build/gen/felucca_tables.h ]; then
    echo "== skip the browser emulator (needs node and build/gen)"
else
    run "browser emulator: the firmware to WebAssembly (web/emu/build.sh -> build/emu)" sh web/emu/build.sh
    cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src -o "$OUT/emu_native" web/emu/native_check.c -lm
    run "browser emulator: the same song from the native build" "$OUT/emu_native" "$OUT/emu_native.f32"
    run "browser emulator: boot, keys, MIDI, screen, LEDs, PLAY, a save kept across instances, = native, cost" \
        node web/emu/emu_test.mjs build/emu/felucca.wasm "$OUT/emu_native.f32"
fi

[ $fail -eq 0 ] && echo "ALL HOST TESTS PASSED" || { echo "HOST TESTS FAILED"; exit 1; }
