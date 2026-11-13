# Handoff: current state of the project (read this first after a context reset)

Updated 2026-11-13. If this disagrees with a task file, the task file and `python3 tools/backlog.py list` win.
Decisions: `docs/project/OWNER_QUESTIONS.md`. Older handoffs: `docs/project/sessions/HANDOFF-*-archive.md`.
**Update this file at the close of every `/next-task` cycle (state + next + open owner items).**

## What we are building
A UX-first sequencer firmware for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules:
`docs/project/VISION.md`. Branch `t1-hardware-bringup`; **nothing pushed since f68d3df. Ask before any push or flash.**

## State
- **Done:** P0-P3, P4-001..016 and P4-026..030, BUG-001..005, BUG-010, BUG-011. The cut-over (P4-012) is done: the
  device build boots the new sequencer core (`sq_*.c`) and the new screen (`ui_sq.c`, `ui_sq_glue.c`). Screen design
  is LOCKED: `docs/design/SCREEN_SPEC_V2.md`, ADR-0006/0007, renders `docs/design/ui_review/proposals/v12/`.
  Pages built: PERF, NOTE_EDIT, CHAN, SET. Only the ACID algorithm row is filled.
- **Host-verified only. Nothing from the new core has been flashed.** The unit still runs the HW-0 build (old T-1).
- RAM `29352 B of 73728` (about 44 KB headroom; floor 4 KB, E9 trigger 8 KB).
- Interrupt CPU cost is unmeasured (static estimate only) until HW-1 / P4-019 / P4-022.

## Path to the first flash (HW-1)
P4-017 REC page -> P4-032 PERF gaps -> P4-018 RESET/HELP/first run -> P4-020 rewrite HARDWARE_TEST.md -> **P4-021
HW-1 (owner flashes; ask first)**. Owner rulings (2026-11-13): external clock P4-031, K3 SOURCE P4-033 wait until
after HW-1; HW-1 is the first timing measurement.
After HW-1: P4-019/P4-022 timing, P5 algorithms + harmony + HW-2, P6 morph/dice/keys + HW-3, P7 presets + HW-4, P8.

## Known debts
External clock transport/tempo not via clk_ext (P4-031); midi_clock.c overwrites BPM under a host clock; K3 SOURCE inert
(P4-033); PERF ALGO encoder / CHN hold / PAST THE END toasts (P4-032); SOL/FIL/ROL (P6-003); PRESETS encoder (P7-002);
hard-tap ACCENT; GLO hold = HARMONY (P5-006); live MIDI IN notes share slots with sequence notes; `--selftest` of
gen_ui_tables not in check.sh; `check_control_map.py` has a pre-existing 78-failure baseline.

## Process lessons
- One agent builds at a time (shared `build/`); never run two `tools/check.sh` at once.
- Ask reviewers for ONE complete findings list (hazards + docs sync) per pass; screen work runs on Sonnet.
- Stray-file trap: writing to a shell glob with `>>` creates a literal `*` file; resolve the real path first.
- Human gates need an "Approved:" line from the owner's own words.
- Hardware facts are tested live and logged as decisions. Runbook: `docs/project/ORCHESTRATOR.md`.

## Hardware / lab
Stock V15 shows as `FM-1_015` on port "FM-1"; Felucca builds as `FM-1_9xx` on "JIANT FM" (loader "JIANT Update"; install
with `--port "JIANT"`). Restore: `restore/FM-1-official-V15.fwsc` (gitignored). TB-3 receives on channel 2.
USB-C is device-only, TRS is MIDI IN only, so the TB-3 needs a host path (`tools/fm1_bridge.py` or a USB-MIDI host box).
