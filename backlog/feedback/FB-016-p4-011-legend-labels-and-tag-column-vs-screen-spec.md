---
id: FB-016
title: P4-011: key_map black-key labels (MUTE1..4, CHAN, RESTRT) and the PERF OCT tag text differ from SCREEN_SPEC and do not fit the legend
phase: P4
type: feedback
status: dropped
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
### 2026-11-12 review of P4-011 (sonnet): ruling per item
1. **Legend, data bug, FIXED.** `key_map.tsv`, `control_map.tsv`, `CONTROL_MAP.md`: `MUTE1..4` -> `M1..M4`, `CHAN` -> `CHN`, `RESTRT` -> `RST`, hold-only `SOLO/FILL/ROLL` -> `SOL/FIL/ROL` (the SCREEN_SPEC mock-up 7 text). ui_sq_test now fails if a mute label is cut. **Remaining, owner's choice:** measured with the real AF_S, `M1` 17 and `FIL` 15 px fit the 18 px ink, but `CHN` 25, `SOL` 22, `ROL` 22, `PG-` 20, `PG+` 23, `RST` 21 do not, so the mock-up legend is not achievable in a 20 px cell and shows `C... S... R... P... P... R...`. Options: (A) accept the cut, full word in HELP (status quo, L = 2); (B) 1-2 glyph labels: `CH SO FI RO < > RS` (all <= 17 px; recommended: readable at a glance, no layout change, no ellipsis); (C) steal Z7's 4 px for a 16 px legend with two lines (costs the margin, the F3 alternative); (D) a narrower legend font (new asset). Recommend B.
2. **PERF tag `ROW 1` / `ROW 2` vs `OCT-` / `h:peek`: stale spec text, not a data bug.** button_modes.tsv, control_map.tsv, page_states.tsv and SEQUENCER_UX 3 (decision 97, three rows) agree on `ROW n`; SCREEN_SPEC 2.4, 2.5, F2 and mock-up 7 predate it. The tables are the generated source and win. A trial of the hold sub-label `peek` (SEQUENCER_UX R-8: every hold is named where it starts) showed `ROW 1` filling the whole 36 px tag, touching `DENS`, and `peek` in T_MID on the filled live row is ghosted, so I reverted it. **Owner:** (A) keep `ROW n` and no `peek` (status quo; the hold peek is unscored), or (B) tag `OCT-`/`OCT+` as printed with `h:peek` and `ROW n` only on the rail digits (recommended if R-8 must hold). Either way SCREEN_SPEC 2.4/2.5/F2 and mock-up 7 need a one-line edit after the ruling.
3. **Cue strip generator gap, FIXED.** gen_ui_tables.py now emits `cue` in `ui_page_rows`; ui_sq.c draws it (`uisq_page_cue`), keeping its own text only for HELP, MENU, FIRST_RUN, ALGO_PICK where the column is prose. ui_sq_test asserts the PERF and RIG cues come from it.
4. **Z6 drawn 3 px up: acceptable, a spec note not a defect.** AF_S ink is 12 rows (line-top +3..+14) in a 12 px slot, so it must start at the slot top; the render shows the legend ink ending at row 223 and cue ink 224..232, Z7 blank. Add "Z6 text is drawn 3 px above the line top" to SCREEN_SPEC 5 whenever it is next edited.

- 2026-11-13: dropped, obsolete: SCREEN_SPEC_V2 / ADR-0006 has no legend or tag column (icons only, keybed footer), so the 20 px legend and ROW-tag questions no longer exist.
