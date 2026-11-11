---
id: P2-008
title: Prototype harness, sq_types/sq_prims reference and the ACID prototype with golden output
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P1-010, P2-004, P1-013, P1-014, P1-017, P2-012]
gate: none
---
## Goal
Build the host prototype harness the dev tasks will copy from, and prove ACID in it. ADR-0003 §14: the prototype IS the firmware file (the same `build`/`step` functions against `sq_types.h`/`sq_prims.h`), so P4-001/P4-002/P5-001 only copy, add `static` and wire tables. Design only: nothing in firmware/.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §3 (exact C), §4 (vtable, AlgoDesc tables), §5, §14 (golden matrix, purity and table tests), §20 (HARMONY stage)
- docs/design/SEQUENCER_UX.md §9.1-9.3 (StepEdit, ChanSave 344 B, step resolution, determinism)
- docs/design/HARMONY.md §2, §4, §9 (HarmonySave, chord stage, query interface) and tests/design/harmony_proto.py
- docs/design/ALGORITHM_SET.md §4-§6 (layers, knob convention, shared primitives SP1-SP17)
- docs/sting-algorithms/ (ACID source spec) and docs/design/algorithms/ (existing assessment notes)
- docs/t1-algorithms/verification/ (probe pattern to copy)

## Deliverable
- **Added by P2-013 (ARP):** `sq_types.h` and `sq_prims` carry the four core additions of docs/design/algorithms/arp/SPEC.md 6.10 so P2-017 only fills ARP: `AF_CHORD 0x02`, `step(cache, g, const HarmChord *hc, PulseOut *out)` (ignored by ACID, EUCLID, SNAKE, STAGE), `AlgoDesc.sub_note`, `harm_chord_at_bar` reachable from the step test harness, the `mix` hash and a seeded 24-element shuffle in `sq_prims`; the purity test also varies `hc`. The ARP prototype and goldens themselves are P2-017 (SPEC 11 lists the golden cases).
- docs/design/algorithms/common/sq_types.h: every constant, struct and `_Static_assert` of ADR-0003 §3 as amended by SEQUENCER_UX 9.1 and HARMONY 2; `SQ_NALGO` 6 (decision 82) with ARP/PATTERNS fields marked `/* P2-017 fills */` where unknown
- docs/design/algorithms/common/sq_prims.h + sq_prims.c: SP1 xorshift + salts, SP2, SP3, SP4, SP5 `sq_ratchet`, SP6/SP7/SP9 `sq_gbuild`/`sq_note`/`sq_snap_abs`, `<stdint.h>` only
- docs/design/algorithms/common/harm.c: `harm_apply()` and the HARMONY.md §9 query functions in C, compared bit for bit against harmony_proto.py tables (H-GOLDEN)
- docs/design/algorithms/acid/SPEC.md (parameters, procedure, AlgoDesc table: morphs, knob map, layers, sub-labels, seed defaults), proto.c (build/step), golden.txt (ADR-0003 §14 matrix, t = 0..127)
- tests/design/run_protos.sh: compiles each algorithm folder with common/, runs the purity and table tests of ADR-0003 §14, diffs against golden.txt, prints `PROTOS PASS`
- Golden matrix extension: each algorithm also runs once through `harm_apply` with the default HARMONY state (must equal the plain output: H-INERT) and with 1-4-5-1 in C major, ROLE FULL

## Acceptance criteria
- [x] `sh tests/design/run_protos.sh` prints `PROTOS PASS`; running it twice gives byte-identical golden output
- [x] `cc -std=c11 -Wall -Wextra -Werror -fsyntax-only` passes on every common/ and acid/ file with only `<stdint.h>`/`<stddef.h>`/`<stdbool.h>`
- [x] ACID golden covers morph min/mid/max on every ROW 1 and ROW 2 knob and each of its five dice layers
- [x] harm.c output equals harmony_proto.py for every table in HARMONY.md 3.1, 3.2, 3.3, 4.2, 6.1
- [x] ACID SPEC.md has no 'TBD'; its AlgoDesc table is the one proto.c reads (remap = table edit)
- [x] Music-theory checks pass (every note in scale; range within `hi()`); where theory and a fixture disagree, theory wins

## Verification
```sh
sh tests/design/run_protos.sh   # PROTOS PASS
python3 tests/design/harmony_proto.py --check
python3 tools/backlog.py check
```

## Out of scope
- firmware/ (dev tasks copy these files later)
- EUCLID/SNAKE/STAGE (P2-016) and ARP/PATTERNS (P2-017)

## Log

### 2026-11-12 executor (sonnet)
Files: docs/design/algorithms/common/{sq_types.h,sq_prims.h,sq_prims.c,harm.h,harm.c}; docs/design/algorithms/acid/{SPEC.md,proto.c,golden.txt}; tests/design/{run_protos.sh,proto_main.c,prims_test.c,harm_test.c,acid_ref.py}; harmony_proto.py gains a `--dump` mode (the existing `--check` is unchanged). Nothing in firmware/.
```
$ sh tests/design/run_protos.sh          (run twice, plus GOLDEN_UPDATE once to create golden.txt)
prims: primitive checks OK
harm: query interface checks OK
H-GOLDEN: 3566 table lines equal harmony_proto.py
ACID: matrix, purity, table and HARMONY checks OK
acid_ref: 53 cases, 0 mismatches
acid: 58 golden sections, golden.txt equal, two runs byte-identical
PROTOS PASS
$ md5 golden.txt after two further runs: 512de3f83ac09adb219209929fda6af8 both times
$ python3 tests/design/harmony_proto.py --check     -> CHECK OK
$ python3 tools/backlog.py check                    -> 132 items, 0 errors
$ cc -std=c11 -Wall -Wextra -Werror -fsyntax-only <every common/*.c, acid/proto.c>   -> ok (headers via -x c-header, inside run_protos.sh)
```
Criteria: golden covers morph min/mid/max for all 8 ROW 1 and ROW 2 morphs (24 cases) and the five dice layers x two seeds; the SPEC AlgoDesc block is diffed against proto.c by run_protos.sh; SPEC has no TBD; theory checks (every note in scale, inside -12..hi(m)) run on every dumped step.
Extra assurance: tests/design/acid_ref.py is an independent re-implementation from the SPEC and matches all 53 plain golden cases.
**For the reviewer (details in acid/SPEC.md section 7):**
1. ACID reaches semitone -12..+36 (sting.md 6.3: octave -1..+2); ADR-0003 section 20 / HARMONY 4.2 assume 0..24. I followed sting.md, added a `lo` fold floor to `harm_apply` (spec tables unchanged, bit-exact vs Python). Owner/reviewer choice: keep, or clamp octave to +1.
2. HarmChord compiles to 14 B (HARMONY.md says 16). harmony_proto.drone_pitch disagrees with HARMONY 3.3 text above p 7 on 5-note scales (C follows the text; not in any documented table).
3. `sq_ratchet` (SP5) is generic; EUCLID's pace curve is left to P2-016. ARP/PATTERNS fields are left as `P2-017 fills` (NULL sub_note, ids 4 and 5, st[4] kept at 4 per SEQUENCER_UX 9.1). AF_CHORD, the `hc` argument, `sub_note`, `sq_mix` and `sq_seeded_perm` are in place and the purity test also varies hc.
