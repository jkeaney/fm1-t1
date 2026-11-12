---
id: P4-023
title: ModSave storage: s_mod saved and loaded with the preset
phase: P4
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P4-005]
gate: none
---
## Goal
Stop `s_mod` (the SHAPE/MOD seed, one per channel and algorithm) being lost on a preset round trip: today it lives in `sq_smod[ch][algo]` in sq_chan.c and `SeqPreset` has no field for it (FB-011 b).

## Read first
- backlog/feedback/FB-011-*.md (b), backlog/tasks/P4-005-*.md Log
- ADR-0003 section 22 (ModSave, SeqPreset 1556 B) and docs/design/MODULATION.md
- firmware/src/sq_chan.c, firmware/src/sq_types.h

## Deliverable
- ModSave storage in sq_chan.c and its save/load with `sq_preset_save` / `sq_preset_load`, with a round-trip test in tests/sq_chan_test.c. First check ADR-0003 section 22: if ModSave is already inside the 1556 B layout, use it; if saving it needs the SeqPreset size to change, STOP and report blocked with the exact byte count (that is an ADR amendment, not yours to make).

## Acceptance criteria
- [ ] A preset round trip preserves `s_mod` for every channel and algorithm (new test fails before, passes after)
- [ ] `sizeof(SeqPreset)` is unchanged at 1556 unless the log shows an owner-approved ADR amendment
- [ ] `sh tools/check.sh` ends with `CHECK PASS`; the RAM line is pasted into the Log with headroom of at least 8192 B.


## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, preset storage plumbing; the producer-visible effect (SHAPE survives save/load) is covered by the P7 hardware gate.

## Out of scope
- Flash storage of presets (P7-001/P7-002).

## Log
