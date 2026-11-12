---
id: P4-023
title: ModSave storage: s_mod saved and loaded with the preset
phase: P4
type: task
status: done
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
- [x] A preset round trip preserves `s_mod` for every channel and algorithm (new test fails before, passes after)
- [x] `sizeof(SeqPreset)` is unchanged at 1556 unless the log shows an owner-approved ADR amendment
- [x] `sh tools/check.sh` ends with `CHECK PASS`; the RAM line is pasted into the Log with headroom of at least 8192 B.


## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, preset storage plumbing; the producer-visible effect (SHAPE survives save/load) is covered by the P7 hardware gate.

## Out of scope
- Flash storage of presets (P7-001/P7-002).

## Log

### 2026-11-12 executor (sonnet)
Contract check: ADR-0003 section 22 row 1 makes ModSave a 100 B SIBLING array of SeqPreset (slot 1556 + 100 = 1656 B), not part of the 1556 B, so saving it does not change sizeof(SeqPreset); no STOP.
- `ModSave` (25 B per channel: dev 1, 4 slots x 3 B reserved zero, s_mod x 6 as little-endian bytes) defined in sq_chan.c, static-asserted 25 B (sq_types.h untouched).
- API change: `sq_preset_save(SeqPreset *, ModSave *ms)` and `sq_preset_load(const SeqPreset *, const ModSave *ms, when)`; ms is SQ_NCH entries, NULL skips (save) or keeps the live s_mod (load). Load stages s_mod in `sq_stage_smod`, the staged caches are built from it, `sq_chan_sync` takes it when the load lands (a queued load keeps s_mod unchanged until the bar). `sq_build_buf` takes an s_mod pointer. ADR-0003 section 6 prototypes (lines 389-390) now differ by the extra parameter: P2-031 may want to note it.
- New `t_modsave` in tests/sq_chan_test.c (every channel x algorithm, queued load, NULL case); the existing preset test now saves/loads ModSave and the old "CH2 s_mod is not in the preset" exemptions are gone. The test did not compile before (no ModSave).
```
sh tools/check.sh -> RAM .data+.bss 23828 B of 73728 (headroom 49900 B)
SQ_CHAN PASS
CHECK PASS
```
Note: sq_*.c is not yet in the felucca.c unity build (cut-over P4-012), so the RAM line is unchanged by this task.

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none -> done).
- check.sh: CHECK PASS. `RAM .data+.bss 23828 B of 73728` (headroom 49900 B >= 8192). sq_*.c is not in the unity build yet.
- (a) ModSave 25 B/channel, 100 B total, static-asserted; dev and slot[] stay zero (memset on save); sizeof(SeqPreset)==1556 asserted in t_preset. Matches ADR-0003 section 22 row 1 (sibling array).
- (b) NULL on save skips, NULL on load keeps live s_mod. A rejected load (-1) returns before staging touches sq_stage_smod. Queued load: staged caches built from sq_stage_smod, sq_chan_sync copies it in under sq_stage_valid at the epoch change, then rebuilds; same path as bpm/harm, no new race.
- (c) Old code could not compile the test, but the assertions are behavioural (round trip of 0x1234-pattern values for 6 ch x 6 algo, byte-identical re-save, s_mod unchanged until the bar). Fuzz 100000 ticks, ASan+UBSan, seeds 1, 7, 12345: all SQ_CHAN PASS.
- Non-blocking: the fuzz always passes NULL for ModSave (tests/sq_chan_test.c:473-479), so the ModSave path is not fuzzed; the queued-test comment says "a dice while pending does not survive the landing" but no dice is issued while pending. Load also accepts s_mod = 0 (zero-filled ModSave); integrity is the P7 CRC's job. Worth a follow-up in P4-012/P7-002 if desired.
- (d) Any 16-bit s_mod is accepted, dev/slots ignored; no out-of-range indexing possible.
- (e) ADR-0003 section 22 already sizes the pool at 18 x 1656 B. P7-001 and P7-002 still said v3/1396 B: fixed (1656 B payload).
- (f) ADR-0003 section 6 prototypes updated to the three-argument forms.
- (g) `ls backlog/tasks | grep -F '*'` is empty.
