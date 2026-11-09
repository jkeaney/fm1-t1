# Roadmap

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
| **P1 Mapping & assessment** | Know what we have: every control, every firmware subsystem, every catalogue algorithm assessed against the vision | Inventories and 7 assessments reviewed; the algorithm shortlist approved by the owner | tasks defined |
| **P2 Design** | Decide what we build: global control map, keep/adapt/remove system map, screen spec, sequencer core architecture and algorithm interface, T-1 bug triage | Owner approves CONTROL_MAP, SYSTEM_MAP, SCREEN_SPEC and ADR-0003; P3–P6 broken into tasks | tasks defined |
| **P3 Clean house** | Remove what SYSTEM_MAP marks REMOVE (synth editing, sampling, song chain, editor protocol …), keeping the build and tests green after each removal | The firmware holds only UI, system, algorithms and setup; RAM and flash freed is reported | epic E3 |
| **P4 Sequencer core** | Channels (MIDI channel only), 16-step entry, transport, MIDI out per channel, preview synth, the new main screen | The reference workflow up to "press PLAY" works on hardware with steps | epic E4 |
| **P5 Algorithms** | The shortlisted algorithms behind one interface; the improved T-1 Euclidean first | Each algorithm has host tests and is heard on hardware | epic E5 |
| **P6 Performance UX** | Morph knobs, per-layer dice buttons, mute/switch, always-on labels, LEDs | The whole reference workflow runs without opening the menu | epic E6 |
| **P7 Storage** | Save and load sequences to a safe flash region | Power-cycle keeps sequences; the flash map is reviewed | epic E7 |
| **P8 Release** | Hardening, a user guide drawn from the screen spec, a release package | Release checklist passes | epic E8 |

## Already done (before this process existed)

- The T-1 engine was made freestanding and wired to the clock, transport and USB MIDI out, with its own screen and
  controls. MENU → SYSTEM → T1 SEQ.
- Flashed and verified on hardware: channel 2 notes and All Notes Off on stop.
- Fixes, not yet on the device: the scale table, RANGE semitones, random pitch/velocity/accent, and independent
  accent draws (see T1_ISSUES F1 and F4).
