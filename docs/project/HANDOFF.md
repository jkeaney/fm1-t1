# Handoff: current state of the project (read this first after a context reset)

Updated 2026-11-13 (POC close-out, P4-036). If this disagrees with a task file, the task file and `python3 tools/backlog.py list` win.
Decisions: `docs/project/OWNER_QUESTIONS.md`. Older handoffs: `docs/project/sessions/HANDOFF-*-archive.md`.
**Update this file at the close of every `/next-task` cycle (state + next + open owner items).**

## What we are building
A UX-first sequencer firmware for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules:
`docs/project/VISION.md`. Branch `t1-hardware-bringup`, pushed to origin at efe0dc9. **Ask before any further push or flash.**

## State
- **Done:** P0-P3, P4-001..016 and P4-026..030, BUG-001..005, BUG-010, BUG-011. The cut-over (P4-012) is done: the
  device build boots the new sequencer core (`sq_*.c`) and the new screen (`ui_sq.c`, `ui_sq_glue.c`). Screen design
  is LOCKED: `docs/design/SCREEN_SPEC_V2.md`, ADR-0006/0007, renders `docs/design/ui_review/proposals/v12/`.
  Pages built: PERF, NOTE_EDIT, CHAN, SET. Only the ACID algorithm row is filled.
- **HW-1 (first flash, 2026-11-13): the new core is flashed and working on the TB-3 at efe0dc9** (build sha1 8a929084..., `FM-1_900`). Owner: it works. P4-021 stays open until the owner writes his own `Approved:` line and the checklist sections B onward are run; findings go to P4-035.
- Readiness review (read-only, 2026-11-13): core and screen are sound and ADR-0007 holds; ACID is not yet FINISHED (generated slides and ARTIC dice invisible on screen, no end-to-end MIDI golden, no HARMONY hooks, no MOD output), the LEDs still show legacy state, and the interrupt cost is unmeasured.
- RAM `29640 B of 73728` (about 44 KB headroom; floor 4 KB, E9 trigger 8 KB).
- Interrupt CPU cost is unmeasured (static estimate only) until HW-1 / P4-019 / P4-022.

## Next steps (plan after HW-1)
1. Owner: sign off P4-021 (`Approved:` line), then P4-035 triage of his findings.
2. P4-042 bound the tie search in the ISR (before P5-008), P4-044 channel in-use, P4-041 ACID MIDI test, P4-040 ACID finish (owner approves new marks).
3. LEDs: P4-037 HW-LED experiment -> P4-038 design (owner approves, rules on hard blinks) -> P4-039 LED-1 build. Requirement: the device LEDs follow the sequencer (blinking, solid or dark by what the active channels' playback and sequence are doing).
4. P4-043 unwired controls (owner rules on RIG and MOD output in v1), P4-031 external clock, P4-019/P4-022 timing.
5. P5 algorithms + harmony + HW-2, P6 morph/dice/keys + LED-2 (P6-004) + HW-3, P7 presets + HW-4, P8.

## Open owner items
P4-021 `Approved:` line; the hard-blink vs breathing ruling for LEDs (P4-038, replaces the old "never a hard blink in a dark room" comment in ui_input.c); RIG in v1? and MOD output in v1? (P4-043); P7-001 flash map `Design locked:` (blocks nothing until P7-002).

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
