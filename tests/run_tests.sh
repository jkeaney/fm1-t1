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
# UI renders (tests/ui_render.c, P3-006, P4-012): the screens that are left, every palette, from the real drawing code: the sequencer screen
#                   (pages, SHIFT / GLOBAL / TEMP, mute, track, a lit card), the menu (every row, ABOUT, INFO, CALIBRATION), UPDATE
#                   MODE, the splash, the dialog, the count-in, the header's states and batteries. The layout lint (no text
#                   off the screen, cut, hidden, overlapping or spilling out of its cell / card; only free text
#                   ellipsised), GREY gray, MONO neutral, the draw cost, the text audit (build/ui_new/text_audit.tsv);
#                   PNGs of GREY MONO GREEN PAPER NIGHT in build/ui_new (tests/ui_render.py), the findings in build/ui_new/report.txt
#                   (the sequencer screen's own findings are listed there and do not fail: FB-007);
#                   the alignment: every text / icon / keycap meant to be centred, or on its neighbours' line, by its
#                   ink against its box (cells, chips, buttons, rows, the keycaps' pills), every screen in FLAT and LINE and
#                   every palette, over every value it can show; 1 px off or more fails (build/ui_new/align.txt).
# UI (tests/ui_test.c): the UI sources against stub display / buttons / knobs: the sequencer screen (PERF page stub) is the page (HOME held opens the
#                   MENU, a dialog takes the screen), the menu's tabs and rows, SCREEN OFF / WAKE, the LEDs (idle, scale), the
#                   header, STYLE, KNOB ACCEL, CLICK / USB / retired settings, GREY gray, MONO neutral.
# Audio / persistence: the real C paths against simulated DMA and NOR flash: bounded overload
#                   fades, shared-voice limits, deferred settings and retries, failed-save rollback, the autosave
#                   (1.2: when it writes, the power-on restore, damaged copies, a write cut short, wear over a session),
#                   transport-stop timeouts, MIDI and UART recovery.
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
# Browser emulator (web/emu, when emcc is there): the firmware in WebAssembly (build/emu) boots, plays keys and MIDI,
#                   draws, lights its LEDs, keeps a save across instances, plays a song bit for bit as the same file
#                   built with cc (web/emu/native_check.c); the cost of 1 s of a heavy song against real time.
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

$CC -o "$OUT/input_test" tests/input_test.c
run "keys and buttons: fast press, long release, bouncy contacts (one note each), glitches, encoders" "$OUT/input_test"

$CC -o "$OUT/midi_uart_test" tests/midi_uart_test.c
run "TRS MIDI parser" "$OUT/midi_uart_test"

$CC -O2 -Ibuild/gen -Ifirmware/src -o "$OUT/torso_phase2_test" tests/torso_phase2_test.c firmware/src/torso_t1_algo.c -lm
run "T-1 algorithm functions the new core keeps (Bjorklund, rotate, pace curve, scale masks, phrase contours)" "$OUT/torso_phase2_test"

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

# USB-MIDI out ring (ADR-0003 8.3, P4-003): 128 entries, NOTE_ONs dropped and counted below the reserve of 8,
# NOTE_OFF / clock / CC kept, order kept across the wrap
$CC -o "$OUT/usb_ring_test" tests/usb_ring_test.c
run "USB MIDI out ring: 128 entries, reserve rule (ONs dropped first, offs and clock kept), order, drop counter" "$OUT/usb_ring_test"

# Sequencer data model (P4-001, ADR-0003 section 22): sq_types.h compiles as C11 with -Wall -Werror and its locked sizes hold
cc -std=c11 -Wall -Werror -Ifirmware/src -o "$OUT/sq_types_test" tests/sq_types_test.c
run "sequencer data model: sq_types.h as C11 (-Wall -Werror), locked sizes (ChanSave 384, ChanRT 24, UndoRec 94, SeqPreset 1556, HarmonySave 8)" "$OUT/sq_types_test"

# UI tables (P4-010, ADR-0004): tools/gen_ui_tables.py compiles docs/design/*.tsv into build/gen/ui_tables.h; the row counts equal the TSV line counts
tl() { echo $(( $(wc -l < "docs/design/$1.tsv") - 1 )); }
python3 tools/gen_ui_tables.py --check && python3 tools/gen_ui_tables.py || { echo ">>> STEP FAILED: gen_ui_tables <<<"; fail=1; }
cc -std=c11 -Wall -Wextra -Werror -Ibuild/gen -DT_PAGE_STATES=$(tl page_states) -DT_CONTROL_MAP=$(tl control_map) -DT_BUTTON_MODES=$(tl button_modes) -DT_KNOB_MAP=$(tl knob_map) -DT_KEY_MAP=$(tl key_map) -DT_HELP_TEXT=$(tl help_text) -o "$OUT/ui_tables_test" tests/ui_tables_test.c
run "UI tables: build/gen/ui_tables.h row counts equal the design TSV line counts, ids and names agree" "$OUT/ui_tables_test"

# Shared primitives (P4-002): the firmware copy (static) prints the dump of the prototype (tests/design/prims_test --dump), byte for byte
cc -std=c11 -Wall -Wextra -Werror -O1 -fno-strict-aliasing -Ifirmware/src -Itests -o "$OUT/sq_prims_test" tests/sq_prims_test.c
sq_prims_cmp() { "$OUT/sq_prims_test" > "$OUT/sq_prims_out.txt" && cmp "$OUT/sq_prims_out.txt" tests/sq_prims_golden.txt; }
run "shared primitives: firmware sq_prims.c dump equals tests/sq_prims_golden.txt byte for byte" sq_prims_cmp

# Scheduler (P4-004): firmware sq_core.c against mock hooks and a mock ALGOS table: the nine event goldens of ADR-0003 section 14
# (tests/golden/sq_core_*.txt) and the 100000-tick invariant fuzz, under ASan + UBSan when the compiler has them
SQC="-std=c11 -Wall -Wextra -Werror -Wno-unused-function -Wno-missing-field-initializers -O1 -fno-strict-aliasing -Ifirmware/src"
printf 'int main(void){return 0;}\n' > "$OUT/sq_san_probe.c"
if ${CC%% *} -fsanitize=address,undefined -o "$OUT/sq_san_probe" "$OUT/sq_san_probe.c" 2>/dev/null && "$OUT/sq_san_probe"; then
    SQC="$SQC -g -fsanitize=address,undefined -fno-sanitize-recover=undefined"
    export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
fi
${CC%% *} $SQC -o "$OUT/sq_core_test" tests/sq_core_test.c
run "sequencer scheduler: sq_core.c event goldens 1-9 (tests/golden/sq_core_*.txt) + 100000-tick invariant fuzz" "$OUT/sq_core_test"

# Channel state API (P4-005): firmware sq_chan.c (morph, dice, undo, unique MIDI, algorithm, rebuild, defaults, RAM presets) against a test ALGOS
# table with the v1 morph / seed / layer tables; SEQUENCER_UX 1.2 rows, unique-MIDI table, defaults field by field, and a 100000-tick API fuzz
${CC%% *} $SQC -o "$OUT/sq_chan_test" tests/sq_chan_test.c
run "sequencer channel API: sq_chan.c UX 1.2 rows, unique MIDI, undo/redo, defaults, presets + 100000-tick API fuzz" "$OUT/sq_chan_test"

# Edit layer, pages and step resolution (P4-006): sq_eval (SEQUENCER_UX 9.2) over the generated layer, the UX 1.2 rows, the 1.3-1.6 examples,
# slide and TIE timing in the scheduler; the resolved tables and event logs are tests/golden/sq_edit.txt
${CC%% *} $SQC -o "$OUT/sq_edit_test" tests/sq_edit_test.c
run "sequencer edit layer: sq_eval 9.2, UX 1.2 rows, pages and length 1.3-1.6, slide / TIE scheduling (tests/golden/sq_edit.txt)" "$OUT/sq_edit_test"

# ACID in firmware (P5-001): firmware sq_algo_acid.c + the ALGOS[] row of sq_algos.c: the plain golden matrix of docs/design/algorithms/acid/golden.txt (everything
# before its H-INERT line; the harm sections belong to P5-005), the purity tests (a)-(g) and the table tests of ADR-0003 section 14
${CC%% *} $SQC -o "$OUT/sq_algo_acid_test" tests/sq_algo_acid_test.c
sq_acid_cmp() { "$OUT/sq_algo_acid_test" > "$OUT/sq_algo_acid_out.txt" || return 1
    n=$(grep -n '^## H-INERT' docs/design/algorithms/acid/golden.txt | head -1 | cut -d: -f1)
    head -n $((n - 1)) docs/design/algorithms/acid/golden.txt | cmp - "$OUT/sq_algo_acid_out.txt"; }
sq_acid_desc() { sed -n '/ALGODESC-BEGIN/,/ALGODESC-END/p' docs/design/algorithms/acid/SPEC.md | grep -v ALGODESC | sed -e '1d' -e '$d' > "$OUT/acid_spec_desc.txt"
    sed -n '/ALGODESC-BEGIN/,/ALGODESC-END/p' firmware/src/sq_algo_acid.c | grep -v ALGODESC | sed -e '1d' -e '$d' -e 's/ \\$//' > "$OUT/acid_fw_desc.txt"
    diff "$OUT/acid_spec_desc.txt" "$OUT/acid_fw_desc.txt"; }
run "ACID algorithm: the descriptor in sq_algo_acid.c equals SPEC.md section 5 (modulo the macro backslashes)" sq_acid_desc
run "ACID algorithm: firmware copy of the prototype, plain golden matrix equals acid/golden.txt before H-INERT, purity (a)-(g) and table tests" sq_acid_cmp

# Sequencer UI framework (P4-011): firmware ui_sq.c on the real gfx.c / sq_core.c / sq_chan.c and build/gen/ui_tables.h: zone table tiles 240x240, every label
# fits its zone by the real text_w (header slots, channel cells, band, tag column, every Z4 cell of every page, legend, cue, widths.tsv), redraw by
# signature, the 2 s toast; renders every page and zone to build/ui_sq/*.png
mkdir -p build/ui_sq
python3 tools/gen_sq_icons.py --ref build/gen/sq_icons_ref.txt || { echo ">>> STEP FAILED: gen_sq_icons --ref <<<"; fail=1; }
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_test" tests/ui_sq_test.c -lm
run "sequencer UI framework: ui_sq.c screen v2 bands + two-strip plan + empty frame, colour tokens, icons pixel-identical to the mock, labels fit (real text_w), redraw by signature, toast; PNGs in build/ui_sq" "$OUT/ui_sq_test" build/ui_sq

# PERF page controls (P4-013): ui_sq.c's key / button / knob / SELECT handlers on the host: step taps, hold = NOTE_EDIT, M-key mute / preview / remove, SELECT add, K1-K4, dice, EDIT, PG-/PG+, OCT rows
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_perf_test" tests/ui_sq_perf_test.c -lm
run "PERF page controls: tap step ON/OFF, hold 0.4 s NOTE_EDIT, M key tap/preview/remove, SELECT adds a channel, K1-K4 morph, dice + EDIT undo, PG-/PG+ with the add prompt, OCT rows; PNGs in build/ui_sq/perf_*" "$OUT/ui_sq_perf_test" build/ui_sq

# NOTE_EDIT page (P4-014): hold a step key opens it, K1-K4 AUTO / set with AUTO at the low end, the layer buttons, OCT-/OCT+, EDIT undo of the one session record, the editor moving, HOME / SELECT leaving and closing the session, bounds; PNGs in build/ui_sq/note_*
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_note_test" tests/ui_sq_note_test.c -lm
run "NOTE_EDIT page: open by hold, K1-K4 AUTO / set, layer buttons, OCT, EDIT undo / redo, move the editor, HOME / SELECT leave and close the session, field bounds, strings fit; PNGs in build/ui_sq/note_*" "$OUT/ui_sq_note_test" build/ui_sq

# CHAN page (P4-015): D#4 opens it, K2 MIDI with the 0.6 s settle and the commit on every exit, the unique-channel steal (toast, other channel OFF, one EDIT undo), OFF, K1 LENGTH clamp and page count change, -PAGE / +PAGE, FOLLOW, the ALGO encoder, SELECT without add, bounds; PNGs in build/ui_sq/chan_*
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_chan_test" tests/ui_sq_chan_test.c -lm
run "CHAN page: D#4 open / close, K2 MIDI settle + commit on every exit, MIDI steal toast + one EDIT undo, OFF, LENGTH clamp + page count, -PAGE / +PAGE, FOLLOW, ALGO picker, SELECT without add, pending-value bounds; PNGs in build/ui_sq/chan_*" "$OUT/ui_sq_chan_test" build/ui_sq
# SET page (P4-016, SCREEN_SPEC_V2 4.7, SEQUENCER_UX 2.4 / 2.8): GLO open / close, K1 BPM (request for song.g[G_BPM]) K2 SWING K3 GATE K4 DIV clamps, OCT-/OCT+ = OCTAVE, K1 inert under an external clock, the four tempo-source states of the header, live keys, bounds; PNGs in build/ui_sq/set_*
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_set_test" tests/ui_sq_set_test.c -lm
run "SET page: GLO open / close, K1 BPM clamp + fast + request, K2 SWING K3 GATE K4 DIV clamps, OCTAVE -2..+2 + slot text, K1 inert under EXT (HOST n, own tempo kept), header tempo-source states and widths, live keys, exits, bounds; PNGs in build/ui_sq/set_*" "$OUT/ui_sq_set_test" build/ui_sq
# REC page (P4-017, SCREEN_SPEC_V2 4.2 / 4.4 / 4.6, SEQUENCER_UX 2.5 / U54 / U67): REC opens / closes (PERF, NOTE_EDIT, CHAN, SET, HOME), 27 keys write ON + PITCH + VEL at the cursor (stopped, advances, wraps) or the nearest step (playing) in one undo record, K2 ACC K3 SLIDE K4 / OCT-/OCT+, K1 CURSOR, EDIT undo / redo, dice, GLO inert, SELECT / ALGO, the note queue (every on has its off, full queue, late release), bounds; PNGs in build/ui_sq/rec_*
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_rec_test" tests/ui_sq_rec_test.c -lm
run "REC page: open / close from every page, 27 keys write at the cursor (stopped) or the nearest step (playing), ACC 120 / SLIDE / OCT and the stored pitch, K1 CURSOR wrap + step audition, EDIT undo / redo + cursor, dice, GLO inert, SELECT / ALGO, note queue (ON/OFF pairs, full queue, release after leaving), bounds; PNGs in build/ui_sq/rec_*" "$OUT/ui_sq_rec_test" build/ui_sq

# RESET, HELP, FIRST_RUN, the cue priority, ADD_PAGE and MOVE_HELD (P4-018, SCREEN_SPEC_V2 4.9 / 4.10, SEQUENCER_UX 2.6 / 11.2 / 11.3) and the sq_reset_* family: hold EDIT opens RESET (one shot, per page beneath), HOME held + a touch is HELP (no action, no MENU), the first-run steps, the 19 cue rows, the add-page prompt, SOL / FIL / ROL named while held, S01 end to end, every control has a help row, a random key / knob / button / HOME / EDIT run keeps the invariants; PNGs in build/ui_sq/safety_*
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_safety_test" tests/ui_sq_safety_test.c -lm
run "RESET / HELP / FIRST_RUN / cue priority / ADD_PAGE / MOVE_HELD + sq_reset_*: hold EDIT, HELP routing, first-run steps, 19 cue rows, help rows for every control, S01, random hazard run; PNGs in build/ui_sq/safety_*" "$OUT/ui_sq_safety_test" build/ui_sq
# Algorithm UI descriptor (P4-027, ADR-0007): the real ui_sq.c draws the knob tiles of ACID (real row) and a dummy DEMO row through the descriptor alone
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/sq_algodesc_test" tests/sq_algodesc_test.c -lm
run "algorithm UI descriptor: value text / one font / dots / short name from AlgoDesc, a dummy algorithm's knob tiles through ui_sq.c unchanged" "$OUT/sq_algodesc_test"

# Screen v2 elements (P4-028, P4-029, SCREEN_SPEC_V2 4.1-4.6, 5): the real ui_sq.c draws header, knob tiles, status line, music row, buttons, keybed, footer and message slot for every scene of the v12 mock
# (tests/v12_export.py turns v12.py's scenes into C data); tests/v12_compare.py compares the 240x240 frames with the mock's own render (exact, bar the
# listed T_CHF rounding) and with the committed goldens (glyph anti-aliasing only). Frames in build/ui_sq/v12, the mock's render in build/ui_sq/v12_ref
python3 tests/v12_export.py build/gen/v12_scenes.h || { echo ">>> STEP FAILED: v12_export <<<"; fail=1; }
${CC%% *} -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o "$OUT/ui_sq_v12_test" tests/ui_sq_v12_test.c -lm
run "screen v2 elements: ui_sq.c draws every v12 scene (header, knob tiles, status line, music row, buttons, keybed, footer, message slot) + view fill + message priority and timing + no algorithm named in the screen code" "$OUT/ui_sq_v12_test" build/ui_sq/v12
run "screen v2 elements: host renders equal the v12 mock pixel for pixel (tests/v12_compare.py)" python3 tests/v12_compare.py build/ui_sq/v12

# Timing T-C (P4-007, TIMING_SPEC 6.3): the real usb.c release ring + sq_core.c stamps: tick position, stamp, ordering, clock out count,
# release, tempo constant; then the same rows with the M3 fallback (TIMER5 releases only between audio interrupts)
${CC%% *} $SQC -o "$OUT/timing_test" tests/timing_test.c
run "timing: tick position, stamps, ordering, clock out, release ring, tempo constant (nested TIMER5 release)" "$OUT/timing_test"
${CC%% *} $SQC -DFELUCCA_TX_NESTED=0 -o "$OUT/timing_test_fb" tests/timing_test.c
run "timing: the same rows with FELUCCA_TX_NESTED=0 (the M3 fallback)" "$OUT/timing_test_fb"

# Timing T-D (P4-009, TIMING_SPEC M11, M12, section 4): the real clk_ext.c (clock source model, PLL, Start / Continue / SPP) with usb.c's in ring and
# sq_core.c: PLL jitter / burst / ramp / no loss, rules R1..R16, Start-then-pulses, SPP + Continue, clock out silent in every EXT state
${CC%% *} $SQC -o "$OUT/clock_test" tests/clock_test.c -lm
run "clock follow: PLL, source rules R1-R16, Start / Continue / SPP, clock out silent in EXT" "$OUT/clock_test"

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
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/speaker_test" tests/speaker_test.c -lm
    run "SPEAKER EQ: FLAT / LOWCUT / BASS+ responses, BASS+ harmonics of the bass, the sub cut, no offset after, headroom on a full-scale kick (#180)" "$OUT/speaker_test"
    run "DSP render (ANALOG preset 0)" "$OUT/hostsim" 0 0 1 "$OUT/render.wav"
    mkdir -p build/tracks_demo
    run "TRACKS: 4-track pattern, live recording (lengths, swing), voice budget, engine switch, cost" env TRACKS=build/tracks_demo "$OUT/hostsim" 0 0 1 "$OUT/tracks.wav"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/project_test" tests/project_test.c -lm
    run "project record (FUN10: round trip, name, FM6 patches, hash)" "$OUT/project_test"
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
    run "UI (P3-006, P4-012): the sequencer screen and the menu: tabs, rows, SCREEN OFF / WAKE, LEDS, header, STYLE, GREY gray, MONO neutral" "$OUT/ui_test"
    $CC -w -Ibuild/gen -Ifirmware/src -o "$OUT/sq_link_test" tests/sq_link_test.c -lm
    run "sequencer link (P4-012): the real seq.c + sq_*.c + ui_sq.c: 2 bars of ACID on MIDI channel 1, STOP sends offs then CC123, empty ALGOS rows are inert (FB-014)" "$OUT/sq_link_test"
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
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/swing_test" tests/swing_test.c -lm
    run "SWING: track + global at most 100, sequencer step lengths, the SWG display" "$OUT/swing_test"
    $CC -O2 -w -Ibuild/gen -Ifirmware/src -o "$OUT/regress" tests/regress.c -lm
    run "regression: golden renders, health, voices, CPU budget" "$OUT/regress" tests/golden.txt tests/cpu_baseline.txt
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
