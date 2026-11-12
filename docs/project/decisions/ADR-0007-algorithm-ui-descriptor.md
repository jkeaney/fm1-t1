<!-- SPDX-License-Identifier: GPL-3.0-only -->
# ADR-0007: Algorithm UI descriptor (amends ADR-0003's AlgoDesc / MorphDesc)

Status: **accepted** (owner, 2026-11-12: "Yes, write the ADR"; contract change to the AlgoDesc layout).

## Context
The owner requires that algorithms can be added and "interface with each element of the screen in how it needs" with no
hard-coded per-algorithm screen code. `AlgoDesc` (firmware/src/sq_types.h) already carries the name, the morph labels and
ranges (`MorphDesc`), the knob remap (`knob_map[]`), the five shared dice layers (`layer[]`) and their sub-labels (`sub[]`).
The screen also needs value names, units, zero words, a lane type and algorithm-only value text. The mock proves the
shape: `docs/design/ui_review/proposals/v12/algos_v12.py` is the screen's only per-algorithm source, and a hypothetical
DEMO algorithm registered in one place renders on every element (header, knob tiles, music row, dice, toasts).

## Decision
Add a UI part to the descriptor (exact field layout and sizes are P4-027's to assert):
| Field | Type (proposed) | Screen use |
| --- | --- | --- |
| `AlgoDesc.short_name[7]` | char, <= 6 + NUL | header; `PATTERNS` -> `PATT` |
| `AlgoDesc.lane` | uint8 (PITCHED, SINGLE) | music row: pitch bars, or a fixed hit block (EUCLID) |
| `MorphDesc.names` | const char *const * or NULL | value lists (FWD BWD PEND...): tile text + position dots (<= 10) |
| `MorphDesc.unit` | const char * or NULL | `2 OCT`-style suffix |
| `MorphDesc.zero` | const char * or NULL | value 0 word (`KEYS`) |
| `AlgoDesc.value_text` | optional hook `(morph id, value, cache, buf) -> len` | algorithm-only text: PATTERNS PICK `HSE 3`, STAGE TOTAL `48(32)` |
| `layer[].n == 0` (exists) | | the matching die is drawn absent |
| `sub[]` (exists) | | toast detail |
A device profile supplies ROW 3 labels the same way (`HwSet` names, HW_SUPPORT_LOWLEVEL 9). A build-time check (extend
`tools/gen_ui_tables.py`) runs the mock's `check()` rules over every row: label <= 6 chars and <= 58 px AF_M; the widest
value fits 54 px in AF_L, else AF_M; lists > 10 entries use a meter; short name fits the header slot.

## Consequences
- `sq_types.h` struct sizes change: assert them; this is the ADR-0003 amendment LOCK.md section 5 requires.
- Every algorithm row (ACID now; EUCLID..PATTERNS in P5-002..P5-008) fills the UI fields; the TSV knob_map stays the
  design source and the check keeps them in step.
- The screen code (P4-028) reads only the descriptor: adding an algorithm needs its row and its TSV rows, nothing else.
