---
id: FB-016
title: P4-011: key_map black-key labels (MUTE1..4, CHAN, RESTRT) and the PERF OCT tag text differ from SCREEN_SPEC and do not fit the legend
phase: P4
type: feedback
status: todo
priority: 2
executor: human
depends_on: []
gate: none
session: 2026-11-12, P4-011 executor, host render build/ui_sq/*.png
---
## Goal
Triage this observation into tasks or bugs, or drop it with a reason.

## Read first
- docs/design/SCREEN_SPEC.md 2.4, 2.6, 3 (Z5 legend, Z3 tag column), F3
- docs/design/key_map.tsv, button_modes.tsv

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## Observation
Found while building the P4-011 framework; the framework draws what the generated tables say and does not decide between them.

1. **Z5 legend (SCREEN_SPEC 2.6, 3, mock-up 7):** the spec shows `M1 M2 M3 | M4 CHN | SOL FIL ROL | PG- PG+ | RST`. `key_map.tsv` (PERF, tap) says `MUTE1 MUTE2 MUTE3 MUTE4 CHAN`, `PG-`, `PG+`, `RESTRT`; SOLO / FILL / ROLL have a hold label only (no tap row). A legend cell is 20 px; `MUTE1` is about 40 px, so by the F3 rule it is cut to `MU...`, and the four mute keys cannot be told apart (the digit is the cut part). 50 of the 154 page x black-key labels are cut. The framework falls back to the hold label when a key has no tap label.
2. **Z3 tag column (SCREEN_SPEC 2.4):** the spec's PERF tag is `OCT-` over `h:peek`; `button_modes.tsv` has tap `ROW 1` / `ROW 2` (hold label equal to the tap), and page_states.tsv PERF says "the rail shows 1 2 3". `ROW 1` is 36 px in the 36 px column, and `RST R1` (RESET) is 38 px. The framework draws the table label, wrapped on its space (`ROW` / `1`, `RST` / `R1`), `+PAGE` / `-PAGE` (36 px) flush; no `h:peek` is drawn because the hold label equals the tap.
3. **Cue strip:** `page_states.tsv` has a cue_strip column, but `tools/gen_ui_tables.py` does not emit it, so ui_sq.c keeps a small static copy (UisqPage.cue). Suggest emitting it.
4. **Descenders:** AF_S ink runs from line-top + 3 to line-top + 14, so a lowercase cue (`morph`, `hold=`) at the top of the 12 px Z6 slot spills into Z7 (which the spec keeps blank). The framework draws Z6 text 3 px up.

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, triage item; any resulting label change goes through its own task with Gate A.

## Log
