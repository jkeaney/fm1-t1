# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

A **UX-first sequencer firmware for the M-VAVE FM-1**. It's a fork of Felucca 1.6 (hugelton/Felucca, GPL-3.0) via
tensor-programming/fm1-t1, which added a Torso T-1-style algorithmic sequencer. Remotes: `origin` = jkeaney/fm1-t1,
`upstream` = tensor-programming/fm1-t1.

Product rules, in priority order. Every design decision is judged against these:

1. **User experience comes first. No menu diving.** Every musical function is reachable from the panel. The screen
   always shows what the device is doing and what each knob and button does right now, with on-screen labels for
   the current knob and button roles. MENU is only for set-and-forget settings.
2. **It is a sequencer, not a synth.** The built-in synth engines exist only to preview sequences.
3. **Channels.** Each sequencer channel sends MIDI out; its settings are its MIDI channel, its **algorithm** and its
   pattern length (1..64 steps, up to 4 pages of 16). The algorithm drives its sequence. You either enter steps on the 16 white keys or dial a sequence in with the
   knobs and buttons, then perform it live. The knobs **morph** the pattern, and buttons **dice** (re-roll) individual
   layers of the algorithm.
4. **Target workflow:** one synth (for example a Roland TB-3) → select channel 1 → set its MIDI channel → choose an
   algorithm → enter steps or dial one in → perform with the knobs and buttons. A sequence should come together fast.

The algorithms come from the catalogue in `docs/`. Each must be **assessed and redesigned** as an algorithm choice
that fits these UX rules, not ported literally. The T-1 algorithms already in the firmware are the starting set, to
be improved into this workflow.

## How work is done here

All work goes through the backlog. Don't patch firmware outside a task.

- **Handoff (read first after a context reset):** docs/project/HANDOFF.md
- **Vision:** docs/project/VISION.md
- **Process:** docs/project/PROCESS.md (lifecycle, Definition of Ready and Done, gates, bugs and feedback, model
  routing)
- **Roadmap:** docs/project/ROADMAP.md (phases P0–P8)
- **Decisions:** docs/project/decisions/ (ADRs)
- **Design outputs:** docs/design/
- **UX gate:** docs/project/UX_GATE.md (Gate A before, Gate B after, use cases in docs/design/features/). Every open task has a `## UX gate` section.
- **Tasks:** `backlog/` (one file per task, bug or feedback item). Use `python3 tools/backlog.py next|list|show|set|check|index`.
- **`/next-task`** runs one cycle: the `fm1-task-executor` agent (model per the task's `executor` field) does it,
  then `fm1-task-reviewer` checks it.
- Human gates (`human-review`, `hardware-test`) need the owner's sign-off in the task log.
- New problems become `backlog/bugs/BUG-*.md` or `backlog/feedback/FB-*.md`, created from `backlog/_templates/`.

## Hardware facts that shape everything

- JieLi AC79 (pi32v2 core), **72 KB RAM**. The build fails if less than 4 KB of `.bss` headroom remains. Large static
  buffers go in the 360 KB `.pool` region with `__attribute__((section(".pool")))`. The pool is NOLOAD and **not
  zeroed**, so initialise it explicitly.
- 240×240 RGB565 screen. Controls are the panel buttons (`B_FX B_SCL B_ENV B_LFO B_EDIT B_GLO B_HOME B_SAVE B_ARP
  B_SEQ B_PLAY B_REC B_OCTDN B_OCTUP`, see panel.c), K1–K4 plus the SELECT/ALGO/PRESET encoders (all relative
  detents, `panel_enc()`), and 27 keys F3–G5 (16 white, 11 black; bit 0 = F3). There is no MENU button: holding
  HOME opens the menu.
- **USB-C is a USB device only.** It carries MIDI in/out, USB audio and firmware updates. **The 3.5 mm TRS jack is
  MIDI IN only**, in hardware; `hal/fm1_uart.h` configures RX only. Driving DIN gear such as the TB-3 without a
  computer needs a USB-MIDI host box (FM-1 USB → host → DIN → TB-3 MIDI IN). The TB-3 receives on **channel 2** by
  default.
- Audio out: speaker plus a rear headphone jack (owner-confirmed 2026-11-11); headphones auto-mute the speaker (verified 2026-11-11).
- Stock firmware (official V15) reports `FM-1_015` on the MIDI port "FM-1" (observed 2026-11-10). Felucca builds report `FM-1_9xx` on the port "JIANT FM", and the update loader shows as "JIANT Update", so the installer needs `--port "JIANT"` for a Felucca install but `--port "FM-1"` for `--info` on stock. After a factory restore the device may need a power-cycle.

## Commands

The build runs the Linux x86-64 JieLi toolchain in Docker (`linux/amd64`), so Docker must be running. The toolchain
is in `~/.jieli/toolchain` and the AC79 SDK in `~/fw-AC79_AIoT_SDK`. Python tools use the local venv `.venv/`
(already has mido, python-rtmidi, Pillow, fonttools; Homebrew `libraqm` is needed for the font rasteriser).

```sh
sh tools/check.sh                            # build + RAM headroom + host tests + backlog check; ends CHECK PASS or CHECK FAIL: <stage>
./build.sh                                   # -> build/felucca.bin, build/loader/ota.bin, build/felucca.fwsc
                                             # prints "image … RAM .data+.bss N of 73728; pool …": watch the RAM line
export AC79_SDK=~/fw-AC79_AIoT_SDK PATH="$PWD/.venv/bin:$PATH"
sh tests/run_tests.sh                        # all host tests (run after ./build.sh; uses build/)
GOLDEN_UPDATE=1 sh tests/run_tests.sh        # accept an intended sound change (tests/golden.txt)
BUDGET_UPDATE=1 sh tests/run_tests.sh        # accept an intended CPU-cost change
```

To run one host suite, look up its compile line in `tests/run_tests.sh`.

Known failures that don't come from code changes:
- The "web pages" step fails because this folder's path contains spaces: the test opens a `%20`-encoded path.
- Wall-clock benchmarks fail intermittently on a busy host.

Flashing, restoring and testing on hardware:

```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"           # identity; read-only
.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes
.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"   # back to stock
.venv/bin/python tools/fm1_bridge.py         # test bridge: FM-1 MIDI -> Mac -> TB-3 over USB (blocks clock/transport)
```

- **Always ask the user before flashing.**
- In update mode the device enumerates as **"JIANT Update"**, which the installer's port filter doesn't match. Pass
  `--port "JIANT"` so both names match. If an install stops at "update loader did not appear", rerun with that port;
  the device waits in update mode.
- `restore/` and `.venv/` are excluded in `.git/info/exclude`.
- If a build crashes at boot, the app can't enter update mode, and recovery needs FM-1-transporter (the JieLi
  mask-ROM USB boot).
- The browser emulator (`web/emu/build.sh`) needs Emscripten, which isn't installed.

## Architecture

**Unity build.** `firmware/src/felucca.c` `#include`s every `.c` file into one translation unit, so **include order
is the dependency order**. Everything is `static`. A file can only use what was included before it, unless it adds a
forward declaration. The order is (as `felucca.c` includes them; nested includes in brackets):
HAL headers → libc/lcd/gfx → sound core (`engines.c` [dsp.c, eng_analog.c], params, mod, voice, fx [click.c]) → usb
[uac_fir.h], midi_uart, seq [midi_control.c, midi_clock.c] → torso_t1_port, torso_t1_algo → the sequencer core (sq_types.h, sq_prims,
sq_algo_acid, sq_algos, sq_core, sq_chan; clk_ext sits before seq) → audio → panel, ui [favorites.c, midi_learn.c], menu_items,
icons, ui_draw, ui_menu, ui_input, ui_sq, ui_sq_glue → fm1_flash, storage_hw, storage, project [settings_persist.c] → ota, ota_hw →
console → master → main.c. The web editor (P3-004) and the groovebox pages and layers (P3-006) are gone from the build.

**The target is freestanding.** There is no hosted libc: `libc.c` provides only memset/memcpy/memcmp, and only
`<stdint.h>`, `<stddef.h>`, `<stdbool.h>` and `<stdarg.h>` are available (no `assert.h`, `stdio.h`, `time.h` or
`string.h`). Code shared with the host tests guards hosted includes with `#ifndef T1_DEVICE`. `T1_DEVICE` is defined
at the top of `felucca.c`. `torso_t1_port.c` supplies a minimal `snprintf`. The build also checks that RAM-resident
code (`.ram_text`) makes no calls.

**Two threads of execution:**
- **Audio ISR.** `audio.c` calls `events_block(n)` in `seq.c` once per control block. It handles transport, MIDI in,
  the keyboard (`keyboard_block`), and the sequencer core (`sq_block`, once `sq_ready`), which runs in place of
  the pattern tracks' `seq_tick`.
- **Main loop** (main.c, ~60 fps): `ui_input()` → settings and autosave → `ui_leds()` → `ui_draw()`.
- The UI talks to the ISR through one-shot `volatile uint8_t` request flags that the ISR consumes and clears
  (`transport_req`, `panic_req` in seq.c). Single-byte state like `song.grid` is read at the start of a block.
- A multi-byte state swap must be wrapped in `fm1_irq_off()/fm1_irq_on()` (see the `sq_chan.c` state swaps).
- **Timing comes from samples**, not a hardware timer: `beat_samples()` (fx.c) is samples per quarter note and
  follows the internal BPM or an external MIDI clock. Sequencer time is `events_block`'s `seq_n`.

**MIDI out:** `midi_out_event(pkt)` in usb.c queues a 32-bit USB-MIDI packet `CIN | status<<8 | d1<<16 | d2<<24` into
a 64-entry ring, and drops it if no host is configured. The pattern tracks do not run, so MIDI out comes from the
sequencer core (`sq_emit`) and from live key presses.

**UI structure:**
- `ui_input()` in ui_input.c runs the button tap/hold timers (`btn_hold`), the menu, then the page. The sequencer
  screen (`ui_sq_glue.c`) is the only page and takes over with an early return.
- `ui_draw_page()` in ui_draw.c picks the screen in priority order: uboot → menu → count-in → confirm → sequencer.
- Drawing uses a 240×124-max canvas: `cv_begin/cv_rect/cv_rrect/cv_text*/cv_blit`, fonts `AF_S`/`AF_M`/`AF_L`, and
  theme colours `T_*`. Redraw only when a state signature changes or `ui.force` is set.
- `song.grid` decides what the keys do: 0 notes, 2 silent (owned by a mode: the sequencer screen sets it). Values 1 (drum grid)
  and 3 (song slots) belong to removed modes; their branches are still in seq.c (v2 clean-up, see
  docs/design/CLEAN_HOUSE_REPORT.md).
- Two-valued menu settings are bits in `ui_prefs`/`ui_rec_prefs`/`ui_prefs2` (ui.c), declared in `MENU_FLAGS`
  (menu_items.c). Adding a menu row changes the tab row counts asserted in `tests/ui_test.c`.

**The sequencer** (the T-1 pipeline, state, UI and glue files were removed in the cut-over, P4-012):
- `sq_types.h` data model; `sq_prims.c` shared primitives; `sq_algo_acid.c` + `sq_algos.c` (the `ALGOS[]` table; only
  ACID is filled, other rows are inert until P5); `sq_core.c` scheduler and event queue (`sq_block`, run from
  `events_block` in the audio ISR once `sq_ready`); `sq_chan.c` channel state API, undo, presets; `clk_ext.c` clock
  source model. `torso_t1_algo.c` keeps only what the new core uses (Bjorklund, rotate, pace, scale masks, contour).
- `ui_sq.c` draws the screen (SCREEN_SPEC_V2: bands, tiles, music row, buttons, keybed, message slot; PERF, NOTE_EDIT,
  CHAN, SET pages); `ui_sq_glue.c` wires controls and the per-frame sync (`sq_frame`). Screen code names no algorithm:
  algorithms describe their own UI through `AlgoDesc`/`MorphDesc` (ADR-0007).
- Host tests link the real `seq.c`, `ui*.c`, `sq_*.c` (see `tests/run_tests.sh`); a new device hook needs a host
  counterpart in `tests/host_sq_hooks.h` or the tests fail to link.

## Docs and the algorithm catalogue

- `docs/*-algorithms/` are functional specs of the algorithms in seven devices: sting (acid lines), snake, ml185
  (stage sequencer), steparp, rhythmic-steps, melodic-steps, and t1 (this code).
- Claims are tagged **[V]** verified, **[I]** inferred, or **⚠** unconfirmed. Treat ⚠ items as hypotheses.
- `docs/t1-algorithms/verification/` holds host probe programs that compile the real T-1 sources.
- `docs/T1_ISSUES.md` is the live defect list for the T-1 code (F1–F19, each OPEN/FIXED/PARTIAL with a
  reproduction). Check it before changing T-1 code, and update a status when you fix an issue.
- `TORSO_T1_GUIDE.md` and `PLAN.md` describe the upstream T-1 design. They don't always match the code: the guide's
  control mapping conflicts with itself, and the code ignores some documented parameters. Trust the code and
  T1_ISSUES.md.
- `BUILDING.md` documents the build flags (`FELUCCA_*`), the emulator and the test coverage.
- `docs/design/CLEAN_HOUSE_REPORT.md` records the v1 removals (R1-R7): predicted against measured flash, RAM and pool,
  the final build line and the inventory of the ADAPT rows that remain.

## Conventions

- New files carry the `SPDX-License-Identifier: GPL-3.0-only` header.
- Match Felucca's comment style: dense, explaining why, naming the cross-file hooks.
- When music theory and a test disagree, the theory wins: fix the fixture, as was done for the scale masks.
- Keep generation deterministic: random lines come from per-track seeded LFSRs and repeat by `random_rate`.
- `t1_lfsr_next` shifts only one bit per call, so take independent decisions from fresh bits (`t1_lfsr_byte`).
- Every change must be checked on a real FM-1 before it counts as done. Host tests render the screen to PNG (build/ui_sq/) but hardware is the judge.
