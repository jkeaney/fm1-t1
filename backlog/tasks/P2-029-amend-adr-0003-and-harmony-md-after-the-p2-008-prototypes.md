---
id: P2-029
title: Amend ADR-0003 and HARMONY.md after the P2-008 prototypes (ACID range, HarmChord size, drone_pitch)
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P2-008]
gate: none
---
## Goal
Make the specs say what the reviewed prototypes do, so P4-001 and P4-002 copy files that agree with their docs.

## Read first
- docs/design/algorithms/acid/SPEC.md section 7 and the P2-008 review Log
- docs/project/decisions/ADR-0003-*.md section 20; docs/design/HARMONY.md sections 3.3, 4.2 (lines ~443 and ~507)
- docs/design/algorithms/common/harm.c, docs/design/algorithms/common/sq_types.h, tests/design/harmony_proto.py

## Deliverable
- ADR-0003 section 20 and HARMONY.md 4.2 state ACID reaches -12..+36 semitones (octave -1..+2) and `harm_apply` has the `lo` fold floor (ACID -12, others 0); or, if the owner prefers, the prototype is changed to clamp at +1 octave. Ask the owner via a `human` question in the Log if the +2 ceiling looks wrong; default to the sting.md range.
- HARMONY.md says `HarmChord` is 14 B (lines ~443, ~507).
- `drone_pitch` in tests/design/harmony_proto.py matches HARMONY.md 3.3 above degree 7 on 5-note scales (or the text is corrected to match, whichever the spec intends), and `harmony_proto.py --check` still prints CHECK OK.

- From the P2-016 review: ADR-0003 section 20 `hi(m)` bounds become EUCLID `SPAN+3`, SNAKE and STAGE `RANGE+1` (prototype values; the old `max(...,12)` is too low with jitter and snap-up).
- From the P2-016 review: ALGORITHM_SET.md 5.2 and FULL_CONTROL.md 3.2/3.4 knob order is updated to match `knob_map.tsv` (EUCLID `{0,4,5,3,1,2,6,7}`, STAGE `{0,1,7,3,4,5,6,2}`).

## Acceptance criteria
- [x] `sh tests/design/run_protos.sh` prints PROTOS PASS and golden.txt is unchanged unless the log explains why
- [x] `python3 tests/design/harmony_proto.py --check` prints CHECK OK
- [x] ADR-0003 and HARMONY.md contain no remaining 16 B or 0..24 claim that contradicts the prototype

## Verification
```sh
sh tests/design/run_protos.sh
python3 tests/design/harmony_proto.py --check
python3 tools/backlog.py check
```

## UX gate
n/a, specification text and an internal prototype fix; no control or screen changes.

## Out of scope
- Firmware changes.

## Log

### 2026-11-12 executor (sonnet)
- ACID range: ADR-0003 section 20 row 4 and HARMONY.md 4.2 now state ACID -12..+36 (octave -1..+2), `harm_apply(ch,t,strong,s,lo,hi,pull,tick)` with the `lo` fold floor (ACID -12, others 0); prototype unchanged (default sting.md range, no `human` question needed).
- HarmChord 14 B in HARMONY.md (two places) and ADR section 20 row 3. `hi(m)`: EUCLID SPAN+3, SNAKE/STAGE RANGE+1, ARP/PATT 60.
- `drone_pitch` in tests/design/harmony_proto.py now follows HARMONY.md 3.3 (n' = min(7, scale size), clamp 2n'); the compared dump range for 5-note scales grew to p 1..11 in both harmony_proto.py and tests/design/harm_test.c (C already followed the spec). Only the harmony dump grew; no algorithm golden changed.
- Knob order: ALGORITHM_SET 5.2 / 5.2b (EUCLID, STAGE rows) and FULL_CONTROL 3.2 / 3.4 (panel-slot lines) match knob_map.tsv.
- Lock scope: new ADR-0003 section 22 (preset 1556 B v4, ChanSave 384, UndoRec 94, PO_TIE, AlgoDesc.mod instead of PulseOut.mod_a/b, AF_CHORD, hc argument, sub_note, sq_mix, sq_seeded_perm, 38 B EUCLID cache); decision 115 (EUCLID pace two-way, one pace bit) recorded in section 22 row 7 and the PO_PACE_DEC comments (ADR, sq_types.h comment only). The P2-030 stale-EUCLID-SHAPE rows untouched. web/, check.sh, build.sh untouched. Other modified files in the tree (usb.c, tests/*, P4-003) are another agent's and not committed here.
```
sh tests/design/run_protos.sh        -> ... golden.txt equal ... algorithms: 6 / PROTOS PASS
python3 tests/design/harmony_proto.py --check -> CHECK OK
python3 tools/backlog.py check       -> 134 items, 0 errors
```
