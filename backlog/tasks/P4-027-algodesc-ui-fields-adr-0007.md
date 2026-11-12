---
id: P4-027
title: AlgoDesc / MorphDesc UI fields (ADR-0007) and the build-time width check
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P5-001]
gate: none
---
## Goal
Let every algorithm describe its UI (short name, lane type, value names, unit, zero word, value-text hook) so the screen
has no per-algorithm code and new algorithms reach every screen element (owner requirement, ADR-0007).

## Read first
- docs/project/decisions/ADR-0007-algorithm-ui-descriptor.md; ADR-0003 section 4; docs/design/algorithms/LOCK.md section 5
- firmware/src/sq_types.h (AlgoDesc, MorphDesc), sq_algo_acid.c (ALGO_ACID_ROW), sq_algos.c
- docs/design/ui_review/proposals/v12/algos_v12.py (the mock registry, `check()`, `value_text`, `value_font`, `is_list`)
- docs/design/knob_map.tsv PERF rows; tools/gen_ui_tables.py

## Deliverable
- sq_types.h: the fields of ADR-0007 with asserted sizes; ACID's row filled from knob_map.tsv.
- tools/gen_ui_tables.py: the `check()` rules over every algorithm row and every SCREEN_SPEC_V2 MESSAGES string; fails the build on a misfit.
- tests: a host test that registers a dummy algorithm and renders its knob tiles through the screen code unchanged.

## Acceptance criteria
- [x] struct size asserts updated and documented in the Log (ADR-0003 amendment)
- [x] `python3 tools/gen_ui_tables.py --check` passes and fails on a deliberately too-wide label (tested)
- [x] the dummy-algorithm host test passes with no screen-code change
- [x] `sh tools/check.sh` ends CHECK PASS

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- EUCLID..PATTERNS rows (their P5 tasks fill the new fields); screen drawing.

## Log

### 2026-11-13 executor (sonnet)
**ADR-0003 amendment (ADR-0007), struct sizes** (firmware/src/sq_types.h and its docs/design/algorithms/common copy, identical):
- `MorphDesc`: 12 B -> 24 B on the device (adds `names`, `unit`, `zero` pointers); 40 B on a 64-bit host. Asserted as `sizeof(void*)==4 ? 24 : 40` plus the `names` offset (12 / 16).
- `AlgoDesc`: adds `short_name[7]`, `lane`, and the `value_text` hook; asserted 312 B device (32-bit), 472 B host. The device assert compiled in ./build.sh (unity build includes sq_types.h), so 312 is confirmed.
- Nothing stored changes (ALGOS[] is const flash; ChanSave 384 / SeqPreset 1556 / UndoRec 94 untouched, sq_types_test PASS). RAM 23844 B of 73728 (unchanged).
- New constants: `SQ_LANE_PITCHED/SINGLE`, `SQ_NAMES_MAX 10`, `SQ_VTEXT_MAX 8`.

**ACID row** (sq_algo_acid.c, plus the identical edits in acid/SPEC.md section 5 and acid/proto.c so the ALGODESC diffs still hold): `.short_name="ACID"`, `.lane=SQ_LANE_PITCHED`, DENS `zero="KEYS"`, DIR `names={FWD,BWD,PEND}`.

**Screen code reads the descriptor** (firmware/src/ui_sq.c, not in the device build yet): `uisq_short_of()` (descriptor short_name, else the old placeholder table until the P5 rows fill it), `uisq_morph_text` (hook > zero > names > number+unit), `uisq_morph_font` (one font per knob), `uisq_morph_dots`, `uisq_knob_tile` (label, value, dots / meter from the descriptor alone). `uisq_draw_frame` and the page logic are untouched; wiring the tiles into the frame is P4-028.

**tools/gen_ui_tables.py**: `algo_check()` ports the mock's check() to every algorithm (TSV PERF rows, measured with the real fonts): short name <= 6 chars and <= 60 px, all 8 slots filled, label <= 6 chars and <= 58 px AF_M, widest value <= 54 px AF_L else <= 58 px AF_M, names count equals the range; plus a TSV <-> firmware ACID row cross-check (label, range, default, zero, names), plus all 34 SCREEN_SPEC_V2 MESSAGES (parsed with ast from proposals/v12/v12.py) against the 136 px slot. Runs in `--check` and in a normal run. `--selftest` proves it fails on a 7-char label, a 6-wide-char label, a 7-char short name, a too-wide value, a too-long message and a missing slot. Hook texts for PATTERNS PICK / STAGE TOTAL are measured from a table in the tool (the P5 hooks must stay within it). The 58 px limit for AF_M values is the tile width (the mock used 56); all rows pass at either.

**Tests**: new tests/sq_algodesc_test.c (in run_tests.sh): the real ui_sq.c, unchanged, draws the knob tiles of ACID (real row) and DEMO (a dummy row with names, unit, zero-less number, a value_text hook, lane SINGLE); asserts the text, fonts, dots, short name and that no ink lands in the tile gaps. Because the new MorphDesc pointer fields trip -Wmissing-field-initializers on the positional `{label,min,max,def,step,fast}` rows, `-Wno-missing-field-initializers` was added to SQC in tests/run_tests.sh and CF in tests/design/run_protos.sh (reviewer: your call whether to prefer padding every row instead).

```
python3 tools/gen_ui_tables.py --check     -> gen_ui_tables: OK (6 algorithms, widths measured; ...)
python3 tools/gen_ui_tables.py --selftest  -> gen_ui_tables selftest: OK (6 algorithms, 34 messages; 6 deliberate misfits caught)
  (also: a TSV label DENSITYX and a TSV default edit each made --check exit 1; both reverted)
build/host/sq_algodesc_test                -> SQ_ALGODESC PASS
sh tests/design/run_protos.sh              -> PROTOS PASS
sh tools/check.sh                          -> RAM .data+.bss 23844 B of 73728 ... CHECK PASS
```
The selftest is not yet called from check.sh / run_tests.sh (`--check` is); the reviewer may want that added.

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none). Re-ran `sh tools/check.sh` -> `RAM .data+.bss 23844 B of 73728` (headroom 49884 B), CHECK PASS; `gen_ui_tables.py --check` OK; `--selftest` OK (6 misfits caught); sq_algodesc_test in the suite. Diff limited to Deliverable files.
Executor flags: (1) -Wno-missing-field-initializers in run_tests.sh SQC and run_protos.sh CF: accepted, host-test-only, firmware build unaffected. (2) --selftest not in check.sh: accepted; criterion "fails on a too-wide label (tested)" is met by the selftest run. Suggest wiring it into run_tests.sh in a later task. (3) AF_M value limit 58 vs mock 56: accepted; ADR-0007 sets no value limit for AF_M (58 is the label limit/tile width) and all rows pass at 56.
