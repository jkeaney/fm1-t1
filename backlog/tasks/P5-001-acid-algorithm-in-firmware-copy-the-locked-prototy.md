---
id: P5-001
title: ACID algorithm in firmware (copy the locked prototype)
phase: P5
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-002, P2-017]
gate: none
---
## Goal
Land ACID exactly as prototyped (ADR-0003 §14: the prototype is the firmware file).

## Read first
- docs/design/algorithms/acid/SPEC.md, proto.c, golden.txt
- docs/project/decisions/ADR-0003-sequencer-core.md §4 (vtable, AlgoDesc), §14 (purity and table tests)
- firmware/src/sq_algos.c (the ALGOS[] table; created by P5-001)

## Deliverable
- firmware/src/sq_algo_acid.c: copy of proto.c with `static` on every function, SPDX header, included in felucca.c after sq_prims.c and before sq_algos.c
- `ALGOS[]` row for ACID in sq_algos.c (P5-001 creates sq_algos.c with the table and `SQ_NALGO` entries)
- tests/sq_algo_acid_test.c: includes the firmware file, prints the golden matrix, diffs against docs/design/algorithms/acid/golden.txt; runs the generic purity and table tests (ADR-0003 §14); run_tests.sh step

- Orchestrator decision 2026-11-12 (resolves the P5-001 block): the algorithm golden is checked in two parts. This task diffs only the plain matrix part of golden.txt (everything before the `H-INERT` section); the `H-INERT` and `HARM 1-4-5-1` sections need `harm.c` and are checked by P5-005's test, whose acceptance criterion gains them. `static` is not applied to typedefs. Rows of the `ALGOS[]` table for algorithms that do not exist yet are filled by their own tasks (the unity build must still compile: use the real rows only, with `SQ_NALGO` and the table sized per the ADR). If the Use case file `docs/design/features/<ID>-*.md` named in the UX gate section does not exist, create it from `_TEMPLATE.md` using the algorithm's SPEC.md section 1 and the scenario steps that exercise it, with Gate A marked 'cited from the locked design (P2-017)'.

## Acceptance criteria
- [ ] `diff` of the test output against docs/design/algorithms/acid/golden.txt is empty
- [ ] Purity tests (a)-(e) and table tests pass
- [ ] sq_algos.c exists with ACID as entry 0 and placeholders that fail the build if a later algorithm row is missing only where SQ_NALGO requires it
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P5-001-acid-algorithm-in-firmware-copy-the-lock.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other algorithms
- UI
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (Haiku 5.5)

Status: **blocked** before any firmware file was written. No code, table row or test was added; no `tools/check.sh` or `./build.sh` run.

Read: CLAUDE.md, VISION.md, this task, SPEC.md, proto.c, golden.txt (head and section index), ADR-0003 sections 3, 4, 14, 19-22, `common/sq_types.h`, `common/sq_prims.h/.c`, `firmware/src/felucca.c` include list.

Checked and consistent:
- `firmware/src/sq_types.h` is identical to `docs/design/algorithms/common/sq_types.h` (`diff` empty). `firmware/src/sq_prims.c/.h` are the `static` copies of the common files.
- proto.c's `AlgoDesc` fields (`.hi`, `.mod`, `.sub_note`, `AF_PAIRING`, `PO_TIE`-free flags) match the amended types in ADR-0003 section 22 and `sq_types.h`.
- `sq_note`, `sq_dir_map`, `sq_rng_init`, `sq_rnd`, `sq_shuffle`, `sq_rank_build`, `sq_bern`, `sq_accent_pick` are present in `firmware/src/sq_prims.h`.
- `felucca.c` already includes `sq_types.h`, `sq_prims.h`, `sq_prims.c` after `torso_t1_algo.c`.
- `firmware/src/sq_algos.c` does not exist yet (this task creates it).

Conflict (the reason for blocking):

```
$ grep -n 'harm_' tests/design/proto_main.c | head -3
76:{   harm_bind(g, h); harm_reset();
238:static void harm_run(...)
254:  x = harm_apply(c->ch, ..., harm_lo(A.id), A.hi(c->m), harm_pull(A.id, c->m), tick);
$ cat tests/design/run_protos.sh  (line 63)
... "$CMN/sq_prims.c" "$CMN/harm.c" "$d/proto.c"
```

- `docs/design/algorithms/acid/golden.txt` lines 6891-7282 (`## H-INERT ...` and `## HARM 1-4-5-1 ...`, about 390 lines) are produced by `harm_run`, which needs `harm_bind`, `harm_apply`, `harm_pull`, `harm_lo` from `docs/design/algorithms/common/harm.c`.
- `harm.c` is not in `firmware/src/` and is P5-005's file. The acceptance criterion "diff of the test output against golden.txt is empty" cannot pass without it, and copying it here is ahead of its task.
- The plain matrix (golden lines 1-6890, the base, morph, dice and combo cases) needs only proto.c, `sq_prims`, and `sq_types`. It does not need harm.c.

Other points for the owner or orchestrator to decide when unblocking (not blockers on their own):
1. The task says "`static` on ... the cache type" (SPEC 9). A C `typedef` cannot take `static`; the typedef `AcidCache` stays as it is.
2. The deliverable's "placeholders that fail the build if a later algorithm row is missing only where SQ_NALGO requires it" is unclear. SQ_NALGO is 6 (sq_types.h); a static assert requiring all 6 rows would fail the build now. Needs a rule.
3. The UX gate file `docs/design/features/P5-001-*.md` does not exist (features/ holds only P2-016, P2-017 and _TEMPLATE.md). The locked SPEC.md may be cited instead, as the task allows.

Options for the owner:
- (a) Split the golden check: this task diffs golden lines 1-6890 (plain) and the H-INERT / HARM sections move to P5-005's test, with the acceptance criterion amended to say so. My recommendation.
- (b) Bring `harm.c` forward into P5-005's scope, which the orchestrator has said not to do without a decision.
- (c) Hold P5-001 until P5-005 is done.

Acceptance criteria: none ticked (none verified).
