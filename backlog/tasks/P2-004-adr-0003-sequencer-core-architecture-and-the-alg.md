---
id: P2-004
title: ADR-0003: sequencer core architecture and the algorithm interface
phase: P2
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P1-010, P2-002]
gate: human-review
---
## Goal
Fix the data model and interfaces that P4–P6 build on: channels, steps, the algorithm plugin interface (layers, seeds, morph parameters), the event router to MIDI and the preview synth, timing, memory budget.

## Read first
- docs/project/OWNER_QUESTIONS.md (owner decisions: scale, GATE/DIV/SWING are global; ACID has slide)
- docs/design/ALGORITHM_SET.md
- docs/design/SYSTEM_MAP.md
- docs/t1-algorithms/01-architecture.md, 08-output-and-dispatch.md
- firmware/src/t1_glue.c, torso_t1_pipeline.c
- CLAUDE.md (Architecture)

## Deliverable
- docs/project/decisions/ADR-0003-sequencer-core.md: C structs for Channel and Step (with sizes, and a _Static_assert plan), the algorithm vtable (e.g. `init`, `on_step`, `morph`, `dice(layer)`), the seed model (per channel, per layer), the morph parameter ranges, the event path ISR→MIDI ring→midi_out_event and to the preview synth, RAM and pool budget per channel, and what of the T-1 code is reused, rewritten or dropped
- docs/project/decisions/README.md updated

## Acceptance criteria
- [x] Every v1 algorithm in ALGORITHM_SET fits the vtable (a short mapping table)
- [x] The RAM budget sums to at most the headroom freed by SYSTEM_MAP plus the current headroom, minus 4 KB
- [x] The owner approved it (log line)
- [x] The algorithm interface declares layers, morph targets and dice targets as data tables so remapping a knob/button needs no algorithm code change; 'Flex points' section included
- [x] The architecture states the boot sequence into the sequencer and the power-on defaults (owner decision 32b); no groovebox mode switch exists

## Verification
```sh
test -f docs/project/decisions/ADR-0003-sequencer-core.md
```

## Out of scope
- Writing the implementation.

## Log

### 2026-11-10 executor (sonnet)
- Wrote docs/project/decisions/ADR-0003-sequencer-core.md (structs with sizes and _Static_assert plan, AlgoDesc vtable and data tables, seed model, timing 192 PPQN with swing/ratchet/slide ordering, event path, note-off guarantees, pins/KEYS ONLY, presets, RAM table, boot/defaults, T-1 reuse table, host-test strategy, flex points, 12 numbered dev prerequisites). Updated decisions/README.md (0003 proposed, 0004 added).
- RAM: core 2185 B + UI allowance 3072 = 5257 B .bss vs 42400 B allowed (46496-4096) after E3, and 13888 B today; pool 6408 B. Struct sizes hand-computed (PulseOut 6, ChanSave 86, UndoRec 34, SeqPreset 356, SeqEvent 8, ChanRT 24); not compiled (design only).
- Verification: `test -f docs/project/decisions/ADR-0003-sequencer-core.md` -> exit 0.
- Not ticked: "The owner approved it" (no Approved line; human gate).
- New owner questions 43-47 (swing formula, GATE neutral point, bar-quantised preset load, per-channel undo record, no cut on algo/MIDI change) added to OWNER_QUESTIONS.md.

### 2026-11-10 review (sonnet)
Verdict: **pass on the design checks; leaves at `review`. Awaiting owner: approve ADR-0003 and answer questions 43-47** (write "Approved: <name> <date>" in this log).
- Compiled all section 3 structs and `_Static_assert`s in a throwaway host file (cc -std=c11 -Wall -Wextra): all sizes hold (PulseOut 6, MorphDesc 12, LayerDesc 3, SeqGlobal 18, AlgoState 16, ChanSave 86, UndoRec 34, SeqPreset 356, ChanRT 24, SeqEvent 8); `sq_note` and the swing arithmetic (66 % at 1/16 = 15 ticks) check out.
- Budget re-added from the real build line (RAM 55744 of 73728 -> 17984 free; 27232 -> 46496 after E3; pool 25764 / 217975 free): 2185 + 3072 = 5257 was correct, now 2193 + 3072 = 5265 after the fixes below; still far inside 42400 / 13888.
- Mapping tables of all four algorithms (seeds, `seed_k`, layer -> seed lists incl. ACID ARTIC = {GATE, SLIDE}, morph ranges/defaults, absent layers) match ALGORITHM_SET 4.2/5.2 and the assessments. usb.c ring claim is right (`midi_out_q[MQ]` is already a separate array, +256 B for 128). T-1 function citations (8 spot-checked) are real; F3 is the open ARCH/INV A integer division and is correctly a prerequisite of EUCLID.
- Fixed in the ADR (committed with this review):
  1. Stuck-note hole: a tie announced one step ahead leaves the note with no queued off; a mute, a note change or an algorithm switch before the next step would hang it. Added `ChanRT.open` and the flush in 7.3 steps 4-5 (struct still 24 B).
  2. `sq_cache` was a byte array cast to structs (misaligned 16/32-bit access on pi32v2): now `uint32_t [4][2][16]`; added `sq_calgo[4][2]` so the ISR reads a cache with the algorithm that built it, not `ChanSave.algo` (a switch between flip and algo store was a race). +8 B budget.
  3. Ratchet repeats were bounded by `[S, S+L)`, which overruns the next step on a swung odd step (overlap = unintended glide on a mono synth): now `[S, S1-1]`.
  4. A dropped NOTE_ON (ring reserve rule) left its NOTE_OFF; now an off is sent only if the note is in `snd[]`, so the stream never has an off without an on.
  5. Smaller: START sets `sq_acc = beat_samples()` as `t1_on_start` does; entropy counter increments every block; slide snapshot carries the algorithm id; seq.c forward declarations noted for the unity build; `GATE_MUL` wording in 17c; the ram_text rule is untouched (only flash routines live there).
- Departures in section 17 judged against musical practice: swing 2x (pair-ratio, MPC-style 50-75 %) and the single undo record are sound. GATE neutral 50 % is sound in effect but the screen must not read "GATE 50 %" as 50 % of a step (the note is at the algorithm's own gate): P2-003 should label it as a percentage of the algorithm's gate. Preset load every 16 steps is the weakest: half a bar at 1/32, four bars at 1/4; suggested 768 ticks (always one bar), noted on question 45. No note cut on change is sound (events carry their MIDI channel).
- Prerequisites 1-12 are ordered and executable (types -> prims -> F3 -> usb -> scheduler on a mock table -> state API -> algorithms -> cut-over -> UI -> store -> hardware). No TBD/choose/decide wording found.
- Not verified here (no tooling): behaviour on the TB-3 (question 8), actual CPU cost of `sq_tick`.

### 2026-11-10 owner
Approved: James Keaney 2026-11-10 (P1-013 and ADR-0003 approved together; rulings 48,50,51,52 as recommended; ruling 49 changed to: design a per-morph reset gesture NOW, and a whole performance-first control pass in P1-014, which may amend this scheme).
