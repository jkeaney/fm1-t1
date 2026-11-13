# Roadmap

> **Status 2026-11-13:** P0-P3 done. P4: the new core and screen are built and HW-1 is signed off (proof of concept done). P4 now continues with ACID finish, LEDs, unwired controls, RIG/MOD and timing; then P5-P8. All phases are broken into tasks: see [PROGRAM_PLAN.md](PROGRAM_PLAN.md). Runbook: [ORCHESTRATOR.md](ORCHESTRATOR.md). Handoff: [HANDOFF.md](HANDOFF.md).


Phases run in order. Each ends at a **gate**, and the next phase's tasks are only broken down once that gate passes
(rolling wave). Task-level order and dependencies live in the task files; `python3 tools/backlog.py list` shows them.

```
P0 Foundation ─▶ P1 Mapping & assessment ─▶ P2 Design (gated) ─▶ P3 Clean house ─▶ P4 Sequencer core ─▶ P5 Algorithms ─▶ P6 Performance UX ─▶ P7 Storage ─▶ P8 Release
                   │                           │
                   │                           └─ P2-005 breaks P3–P6 down into tasks
                   └─ controls · systems · 7 algorithm families
```

| Phase | Goal | Exit gate | Status |
| --- | --- | --- | --- |
| **P0 Foundation** | Work committed, one green check command, a hardware test routine, the fixed bassline build flashed | `tools/check.sh` passes; owner has heard the fixed build on the TB-3 | tasks defined |
| **P1 Mapping, assessment and UX design** | Know what we have (controls, subsystems, 7 algorithm families), choose the v1 algorithms, then design the controls and sequencer UX: P1-010 algorithm set, P1-012 UX unification, P1-013 shift row, P1-014 performance-first scheme, P1-015 keybed force reference, P1-017 sequencer UX lock | Inventories and assessments reviewed; algorithm set approved; sequencer UX locked and approved by the owner (P1-017) | P1-017 in review loop |
| **P2 Design** | Decide what we build: global control map, keep/adapt/remove system map, screen spec, sequencer core architecture and algorithm interface, T-1 bug triage | Owner approves CONTROL_MAP, SYSTEM_MAP, SCREEN_SPEC and ADR-0003; P3–P6 broken into tasks | tasks defined |
| **P3 Clean house** | Remove what SYSTEM_MAP marks REMOVE (synth editing, sampling, song chain, editor protocol …), keeping the build and tests green after each removal | The firmware holds only UI, system, algorithms and setup; RAM and flash freed is reported | epic E3 |
| **P4 Sequencer core** | Channels (MIDI channel only), 16-step entry, transport, MIDI out per channel, preview synth, the new main screen | The reference workflow up to "press PLAY" works on hardware with steps | core and screen built; HW-1 signed off 2026-11-13 (POC done); ACID finish, LEDs, unwired controls, RIG/MOD, external clock and timing remain (execution order below) |
| **P5 Algorithms** | The shortlisted algorithms behind one interface; the improved T-1 Euclidean first | Each algorithm has host tests and is heard on hardware | epic E5 |
| **P6 Performance UX** | Morph knobs, per-layer dice buttons, mute/switch, always-on labels, LEDs | The whole reference workflow runs without opening the menu | epic E6 |
| **P7 Storage** | Save and load sequences to a safe flash region | Power-cycle keeps sequences; the flash map is reviewed | epic E7 |
| **P8 Release** | Hardening, a user guide drawn from the screen spec, a release package | Release checklist passes | epic E8 |
| **P9 v2 clean-up** | Sequencer-only firmware: remove the legacy groovebox and every subsystem marked 'v2 REMOVE' in the system map | Owner approves the removal list; check.sh green; flash and RAM freed reported | epic E9 |

Current state and next steps: docs/project/HANDOFF.md.

## Execution order (agreed 2026-11-13)

The `priority:` number in each task file is the position in this list (lower = first); `python3 tools/backlog.py next`
returns the first ready task in it. Human gates (marked H) stop the line until the owner has done them.

| # | Tasks | What |
| --- | --- | --- |
| 1 | P4-040 | ACID finish (draw generated slides, gate/hold/tie, visible dice; clearer hints PG+=ADD and K1=LEN); owner approves the marks |
| 2 | P4-037 (H) | HW-LED session: the 20-minute LED experiment (a throwaway build; ask before flashing) |
| 3-4 | P4-038, P4-039 | LED design (owner approves) then LED-1 build |
| 5 | P4-043 | Unwired controls (ALGO on NOTE_EDIT/SET, OCT peek, preview pick) |
| 6 | P4-045 | RIG and MOD design reconciliation, ADR-0008, device-profile format, mocks (docs; may run while LED builds; owner approves) |
| 7-10 | P4-046 .. P4-049 | Device-profile data and TB-3 profile, MOD engine (ISR), ROW 3 knobs, RIG page |
| 11 | P4-050 (H) | HW-RIG on the TB-3 |
| 12 | P4-031 + BUG-009 + P4-025 | External clock, USB out ring clear, TRS song position (one batch) |
| 13-14 | P4-019, P4-022 (H) | Timing test image (after the MOD engine, so the ISR cost includes it), HW-T gate |
| 15 | P4-033 | CHAN page K3 SOURCE |
| 16-20 | P5-002+003+004, P5-005, P5-006, P5-007+008, P5-009 (H) | EUCLID, SNAKE, STAGE; HARMONY engine; HARMONY page; ARP, PATTERNS; HW-2 |
| 21-28 | P6-001 .. P6-008 | Morph, dice, performance keys, LED-2, keybed force, HT-FORCE (H), HW-3 (H) |
| 29-31 | P7-002 .. P7-004 | Presets, settings persistence, HW-4 (H) |
| 32-35 | P8-001 .. P8-004 | Hardening, user guide, release package, release acceptance (H) |
| 36 | E9 | v2 clean-up (after release; the owner decides) |

Dependencies added on 2026-11-13: P4-019 waits for P4-047 (the timing image must carry the MOD output's ISR cost);
P6-001 waits for P4-048 (both change OCT row switching: ROW 1 / 2 / 3); P6-008 waits for P4-050 (HW-3 is the
whole-workflow acceptance and the RIG/ROW 3 knobs are part of performing); P7-002 waits for P4-047 (ModSave is
stored with the preset); P8-004 waits for P4-050. P5-002..P5-008 follow RIG/MOD by order only, not by dependency, so a
pending owner gate does not freeze them; the owner can reorder them by priority.

## Already done (before this process existed)

- The T-1 engine was made freestanding and wired to the clock, transport and USB MIDI out, with its own screen and
  controls. MENU → SYSTEM → T1 SEQ.
- Flashed and verified on hardware: channel 2 notes and All Notes Off on stop.
- Fixes, not yet on the device: the scale table, RANGE semitones, random pitch/velocity/accent, and independent
  accent draws (see T1_ISSUES F1 and F4).
