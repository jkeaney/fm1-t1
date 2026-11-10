# Program plan: from locked design to finished, flashed v1 firmware

Written 2026-11-10 by the technical architect (Opus), as the deliverable of P2-005. The owner approves it in the P2-005 Log.
After approval, a **Sonnet orchestrator** runs the backlog (runbook: [ORCHESTRATOR.md](ORCHESTRATOR.md)) with **Haiku**
executing most tasks, and Sonnet only where a task needs judgement across files, ISR timing, or design.

Every task below already exists in `backlog/` with full Definition-of-Ready content. `python3 tools/backlog.py list`
is the live view; this page is the map.

## 1. What "finished" means

v1 is done when P8-004 is signed: the release image is flashed on the owner's FM-1, boots straight into the sequencer,
and the owner has run the target workflow (VISION rule 4) on the TB-3: pick a channel, set its MIDI channel, choose one
of six algorithms, enter or dial a sequence, and perform it with morph knobs, dice, mute and HARMONY, with presets
surviving a power cycle. No menu is needed for any musical function.

## 2. Architecture in one page (the shape every task builds toward)

```
 keys/knobs/buttons ──▶ ui_input.c ──▶ ui_sq.c pages (PERF NOTE_EDIT CHAN SET HARMONY REC RESET HELP)
                                         │  call only the sq_* API (ADR-0003 §6); read tables generated from docs/design/*.tsv
                                         ▼
 main loop  ─────────────────────────  sq_core.c channel API ── double-buffered state ──┐
                                                                                         │ read once per block
 audio ISR: events_block ─▶ sq_block ─▶ sq_tick (192 PPQN) ─▶ ALGOS[ch].step() ─▶ harm_apply() ─▶ sq_note()
                                         │                        (pure, golden)     (HARMONY)     (scale)
                                         ▼
                                       evq[96] ─▶ sq_emit ─▶ txr[128] stamped ─▶ TIMER5 release ─▶ USB MIDI out
                                                         └─▶ preview: trk[0] ANALOG (selected channel)
 clock: AUTO source (INT master + clock out | EXT follow via PLL), visible as the header dot on every screen
 storage: 16 preset slots (SeqPreset v3, 1396 B) + settings, in a reviewed flash region (P7-001)
```

Principles the tasks enforce:
- **Algorithms are pure** (`build`/`step`); the P2-008/016/017 prototypes *are* the firmware files. Dev tasks copy them,
  add `static`, wire the `ALGOS[]` row, and reproduce the golden text byte for byte. No re-implementation.
- **Everything performable is a table** (AlgoDesc, knob_map/button_modes/key_map/help_text TSVs compiled to C by
  `tools/gen_ui_tables.py`). A workflow change after a hardware test is a table edit, not code.
- **UI never touches ISR state**; it calls the `sq_*` API. Multi-byte swaps use the double buffer or `fm1_irq_off/on`.
- **Clean house first** (decision 80): removing the groovebox frees ~28 KB RAM and ~190 KB pool, so the 16 preset slots,
  HARMONY, ARP and PATTERNS all fit with margin.
- **Hardware early and often**: a flash after clean-house (HW-0), at first boot of the new core (HW-1), timing (HW-T),
  algorithms (HW-2), performance (HW-3), force (HT-FORCE), storage (HW-4) and release (P8-004).

## 3. The plan, phase by phase

Model: **S** = Sonnet, **H** = Haiku, **owner** = human gate. Gate: **HR** = human-review, **HW** = hardware-test.

### P2 Design (remaining)
| ID | Task | Model | Gate | Depends on |
| --- | --- | --- | --- | --- |
| P2-012 | HARMONY design (fixing review findings) | S | HR | P2-011 |
| P2-013 | ARP algorithm design | S | HR | P2-012 |
| P2-014 | PATTERNS library design (research, famous-song + genre patterns) | S | HR | P2-012 |
| P2-015 | Keybed expression use cases | S | HR | P2-012 |
| P2-008 | Prototype harness, `sq_types.h`/`sq_prims` reference, ACID prototype + golden | S | none | P2-012 |
| P2-016 | EUCLID, SNAKE, STAGE prototypes + goldens | S | none | P2-008 |
| P2-017 | ARP + PATTERNS prototypes + goldens; owner locks all prototypes | S | HR | P2-013, P2-014, P2-016 |
| P2-005 | This plan | Opus | HR | (approved by owner) |

P2-013, P2-014 and P2-015 run in parallel. P2-008 can start as soon as P2-012 is approved (it needs only HARMONY's rules),
so prototype work overlaps the three parallel designs.

### P3 Clean house (removal order of SYSTEM_MAP §6, with the fx buses first per decision 80)
| ID | Task | Model | Depends on |
| --- | --- | --- | --- |
| P3-001 | Host tests link the real seq/ui/T-1 sources (decision 42) | S | P2-005 |
| P3-002 | Remove the fx effect buses and the slicer insert (R5, first) | H | P3-001 |
| P3-003 | Remove the dead DIGITAL engine and the FM4 flag (R1) | H | P3-002 |
| P3-004 | Remove the web editor (R2) | H | P3-003 |
| P3-005 | Menu rows 37 -> 28, T-1 always on (R3a) | H | P3-004 |
| P3-006 | Remove the groovebox UI files and page code (R3b) | S | P3-005 |
| P3-007 | Remove song chain, motion, chord, perform (R4) | H | P3-006 |
| P3-008 | Remove DRUM and PHYS engines (R6a) | H | P3-007 |
| P3-009 | Remove the sample family and FM6 (R6b) | H | P3-008 |
| P3-010 | Remove the remaining engines and `eng_mem`; ANALOG only (R6c) | H | P3-009 |
| P3-011 | Remove user presets, slice_store, FUN1-FUN10 formats (R7) | H | P3-010 |
| BUG-002 | Delete the stale `T1_SCALE_NAMES` (F2) | H | P3-001 |
| BUG-001 | Fix ARCH / INV A phrase contours (F3) | S | P3-001 |
| P3-012 | Clean-house report: bytes freed, CLAUDE.md architecture | H | P3-011, BUG-001, BUG-002 |
| P3-013 | **HW-0** owner flash: the cleaned build boots and the T-1 still plays the TB-3 | owner, HW | P3-012 |

### P4 Sequencer core (ADR-0003 §16 order, TIMING_SPEC §9)
| ID | Task | Model | Depends on |
| --- | --- | --- | --- |
| P4-001 | `sq_types.h` (structs, constants, static asserts) | H | P3-012, P2-017 |
| P4-002 | `sq_prims.c/.h` with golden tests | H | P4-001 |
| P4-003 | `usb.c` 128-entry out ring, `midi_out_free`, reserve rule | H | P3-012 |
| P4-004 | `sq_core.c` scheduler + event-level goldens (mock algorithms) | S | P4-002, P4-003 |
| P4-005 | Channel state API, defaults, undo, RAM presets, invariant fuzz | S | P4-004 |
| P4-006 | Edit layer, pages, step resolution (`sq_eval`) | H | P4-005 |
| P4-007 | Timing T-C: stamped release ring, TIMER5 release, tempo constant | S | P4-004 |
| P4-008 | Timing T-B: `tools/fm1_timing.py` | H | P2-005 |
| P4-009 | Timing T-D: AUTO clock source, PLL, Start/SPP | S | P4-007 |
| P4-010 | `tools/gen_ui_tables.py`: TSV tables -> `build/gen/ui_tables.h` | H | P3-012 |
| P4-011 | `ui_sq.c` framework: zones Z0-Z6, toasts, labels, page dispatch | S | P4-010, P4-005 |
| P4-012 | Cut-over R8: `sq_block`, `sq_boot`, delete the T-1 pipeline/state/UI | S | P4-006, P4-009, P4-011, P5-001 |
| P4-013 | PERF page | H | P4-012 |
| P4-014 | NOTE_EDIT page | H | P4-013 |
| P4-015 | CHAN page (unique MIDI channels, commit on any exit, algorithm picker) | H | P4-013 |
| P4-016 | SET page + tempo-source visibility (T-E) | H | P4-013 |
| P4-017 | REC page | H | P4-014 |
| P4-018 | RESET, HELP, first run, cue strip, ADD_PAGE, MOVE_HELD | H | P4-015, P4-016, P4-017 |
| P4-019 | Timing T-A: timing-test image | H | P4-012, P4-007 |
| P4-020 | Rewrite HARDWARE_TEST.md for the new sequencer (TB-3 checklists) | H | P4-018 |
| P4-021 | **HW-1** first flash of the new core: workflow up to PLAY on the TB-3 | owner, HW | P4-020 |
| P4-022 | **HW-T** timing gate G1-G7 (T-F) | owner, HW | P4-019, P4-008, P4-021 |

### P5 Algorithms (E5 split per decision 82: classic four, then HARMONY/ARP/PATTERNS)
| ID | Task | Model | Depends on |
| --- | --- | --- | --- |
| P5-001 | ACID | H | P4-002, P2-017 |
| P5-002 | EUCLID | H | P5-001, BUG-001 |
| P5-003 | SNAKE | H | P5-001 |
| P5-004 | STAGE | H | P5-002, P5-003 |
| P5-005 | HARMONY engine (`harm_apply`, goldens against `harmony_proto.py`) | H | P5-004, P4-005 |
| P5-006 | HARMONY page (hold GLO) and the chord indicator | H | P5-005, P4-016 |
| P5-007 | ARP | H | P5-005 |
| P5-008 | PATTERNS (algorithm + pattern data in flash) | H | P5-005 |
| P5-009 | **HW-2** all six algorithms + HARMONY on the TB-3 | owner, HW | P5-006, P5-007, P5-008, P4-021 |

### P6 Performance UX
| ID | Task | Model | Depends on |
| --- | --- | --- | --- |
| P6-001 | Morph knobs, ROW 1/ROW 2, band labels | H | P4-018, P5-008 |
| P6-002 | Dice: five layers + ALL, sub-labels, undo/redo | H | P6-001 |
| P6-003 | Performance keys: MUTE1-4, SOLO, FILL, ROLL, PG-/PG+, RESTRT | H | P6-002 |
| P6-004 | LEDs | H | P6-003 |
| P6-005 | Keybed scan: DRIVE mode, calibration, LED blanking, FORCE row | S | P6-004, P2-015 |
| P6-006 | Force gestures per KEYBED_USE_CASES | H | P6-005 |
| P6-007 | **HT-FORCE** hardware gate | owner, HW | P6-006 |
| P6-008 | **HW-3** full workflow without the menu, morph/dice/mute on the TB-3 | owner, HW | P6-004, P5-009 |

### P7 Storage
| ID | Task | Model | Gate | Depends on |
| --- | --- | --- | --- | --- |
| P7-001 | Flash map: safe region for 16 presets + settings (ADR-0005) | S | HR | P3-012 |
| P7-002 | Preset store: save, queued load, empty-slot rule, undo of a load | H | none | P7-001, P5-005, P6-002 |
| P7-003 | Settings persistence: globals, force floors, first-run bit | H | none | P7-002, P6-005 |
| P7-004 | **HW-4** power-cycle keeps presets and settings | owner | HW | P7-003 |

### P8 Release
| ID | Task | Model | Gate | Depends on |
| --- | --- | --- | --- | --- |
| P8-001 | Hardening: long fuzz, sanitizers, stuck notes, disconnects, T1_ISSUES closed | H | none | P7-003, P6-007 |
| P8-002 | User guide | H | HR | P8-001 |
| P8-003 | Release package: version, changelog, images, checksums, install/restore | H | none | P8-002 |
| P8-004 | **Release flash and acceptance session** | owner | HW | P8-003, P7-004, P6-008, P4-022 |

E9 (v2 clean-up) stays a placeholder. Its trigger (RAM headroom under 8 KB) is checked on every dev review.

## 4. Critical path and the owner's touch points

Critical path: P2-012 -> P2-013/014 -> P2-017 -> P4-001 -> P4-002 -> P4-004 -> P4-005 -> P4-006 -> P4-012 -> P4-013 ->
P4-015..018 -> P6-001..004 -> P7-002 -> P7-003 -> P8-001..004. Clean house (P3) runs in parallel with the P2 prototypes
only in calendar terms: the orchestrator runs one dev task at a time, so P3 fills the time while design tasks wait on
owner approval.

Owner touch points (batched by the orchestrator in the open-items format):
1. Approvals: P2-012, P2-013, P2-014, P2-015, P2-017, P2-005 (this plan), P7-001, P8-002.
2. Hardware sessions (each needs a flash OK): HW-0 (P3-013), HW-1 (P4-021), HW-T (P4-022), HW-2 (P5-009),
   HW-3 (P6-008), HT-FORCE (P6-007), HW-4 (P7-004), release (P8-004). HW-T and HW-1 can share a session, and so can
   HW-3 and HT-FORCE.
3. Pushes (ask each time).

## 5. Budgets every review checks

| Resource | Today | After P3 | v1 design adds | Floor / trigger |
| --- | ---: | ---: | ---: | --- |
| RAM .data+.bss free | 17984 B | ~46496 B | 6861 B + ~600 B timing + ARP/PATTERNS (P2-013/014 state it) | 4 KB floor; under 8 KB pulls E9 forward |
| Pool free | 25764 B | ~217975 B | 25128 B presets + ARP/PATTERNS | must fit 16 slots (decision 80) |
| Flash image | 523032 B | ~288458 B | ~6 KB + pattern data | the image limit in build.py |
| CPU (audio `max_us`) | baseline | baseline | timing +1 pp, force scan +3 % | TIMING_SPEC G6, HT-FORCE G4 |

## 6. Risks and their answers

| Risk | Answer in the plan |
| --- | --- |
| A removal breaks boot (the device then needs mask-ROM recovery) | HW-0 flashes the cleaned build before any new core work; `restore/` holds the stock image; every removal ends with check.sh |
| The nested TIMER5 release (TIMING M3) is unsafe | P4-007 implements the fallback behind one flag; HW-T decides |
| Haiku can't finish a task | Escalation rule (ORCHESTRATOR.md 4): blocked, or failed review twice -> re-run on Sonnet, logged |
| Force sensing fails on hardware | HT-FORCE fallbacks: FORCE OFF still ships every function (SEQUENCER_UX 6.6) |
| The six-algorithm picker does not fit | P2-013/P2-014 must show the screen; the names are one table |
| Memory grows with ARP/PATTERNS | P2-013/014/017 state bytes; the pattern data is const flash; E9 trigger on every review |
| The design and the dev task disagree | Dev executors stop and file BUG/FB (ADR-0004); never guess |
