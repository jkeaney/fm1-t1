---
id: P2-029
title: Amend ADR-0003 and HARMONY.md after the P2-008 prototypes (ACID range, HarmChord size, drone_pitch)
phase: P2
type: task
status: todo
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
- [ ] `sh tests/design/run_protos.sh` prints PROTOS PASS and golden.txt is unchanged unless the log explains why
- [ ] `python3 tests/design/harmony_proto.py --check` prints CHECK OK
- [ ] ADR-0003 and HARMONY.md contain no remaining 16 B or 0..24 claim that contradicts the prototype

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
