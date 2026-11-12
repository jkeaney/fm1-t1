<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 07 Acceptance: what "done" means for the design, and the hand-off into the build

## A. The gate that matters

**The owner says "good enough" about pictures.** That is the only gate that closes the design. Write the owner's one-line
definition here once you have it (the last session's: *"Perfect just means that the UX is communicated cleanly to the
user and the device is usable."*): ______________________________.

Everything below is evidence you bring so that the owner's "good enough" is well-founded. None of it replaces it.

## B. Evidence the design must carry (the design is not ready to show as "final" without it)

These are the checks the independent critique runs. Targets are **suggestions to agree with the owner**, derived from the
last session; the owner may relax or tighten them.

| # | Check | How measured | Target |
| --- | --- | --- | --- |
| 1 | **The workflow test passes from the screen alone** (select channel 1 + set its MIDI channel; choose an algorithm; dial in with knobs and dice; PLAY; morph, dice, mute, switch channel, undo, change preset, without leaving the main screen) | walk it in scenes; the independent critique scores Recognition, Visibility, Error recovery | every step passes; no step needs the printed manual |
| 2 | **The owner's "everything accessible is documented"** | list every control x tap/hold on PERF from `button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`; show where each is documented (main screen / toast / cue / map) | 100% of controls have a documented place; the owner agrees the place is right (see `08`, question 1) |
| 3 | **The four glance facts** (knob values; pattern + playhead; channel mute state; selected channel + algorithm) readable at arm's length | arcmin table at 50 cm; no AF_S on a glance screen | AF_L or big shape for anything read while playing; AF_M only for position-identified labels |
| 4 | **Density**: not "too much information" | script-counted text items per screen (use `critique_v3_2/scripts/`) | cap agreed with the owner (v3.2: 28 strings / 11 roles playing; the owner did not accept that). **Ask what number feels right** |
| 5 | **Contrast** | `mock.py contrast`, every meaning-bearing pair, all 9 palettes + mono | text >= 4.5:1 (large >= 3:1); non-text meaning >= 3:1; colour never alone |
| 6 | **Accuracy against the tables** | spot-check >= 12 labels/holds/toasts against `button_modes.tsv`, `key_map.tsv`, `knob_map.tsv`, `control_map.tsv` | 0 mismatches |
| 7 | **No clipped / ellipsised / overlapping text** in any state | the tool's warnings + the edge-case sweep | 0 |
| 8 | **States catalogue** | one render per state: idle, playing, row 2/3, mutes, selected+muted+OFF, EXT/WAIT/lost, REC, FOLLOW off, first run, ADD PAGE, past length, preset pending/loading, every toast, held solo/fill/roll, map | all drawn, all correct |
| 9 | **Independent critique** | the two-assessment method (`06_PROCESS.md` section 4) | agree a target with the owner; suggest **>= 32/40 with no P1** |
| 10 | **Real-size check on the unit** | see section E | the owner has seen the main screen on the real 30 mm display and still says "good enough" |

## C. Deliverables from the design session (in this order)

1. **The owner's reasons** (interview notes; `08_OPEN_QUESTIONS.md` updated).
2. **A small vertical slice for the owner to react to**: the main screen in 3-4 states (idle, playing, row 2, muted),
   at 1x and 4x, **with the buttons question answered in pictures** (see `08`, question 1), sent as images.
3. **The agreed design**, rendered: all states + toasts + cue + map(s).
4. **`docs/design/SCREEN_SPEC_V2.md`**: the implementable spec (zone table with x,y,w,h on the stated module, font per
   element, string tables for cue/toast/map, colour tokens per palette with fallback, behaviours: HOME escalation timing,
   toast duration and dismissal, lane bar meaning, states), citing the final renders. It replaces `SCREEN_SPEC.md` for
   the PERF screen.
5. **The table changes** (`button_modes.tsv` HOME hold; `help_text.tsv`; `page_states.tsv`; `knob_map.tsv` for value
   names and the ARP K3 label; `widths.tsv`) with `gen_help.py`, `gen_sweeps.py`, `check_help.py`, `check_scenarios.py`,
   `width_probe.sh`, `tools/gen_ui_tables.py --check` all passing.
6. **An ADR** (`docs/project/decisions/ADR-0006-*.md`): the screen architecture (glance / toast / cue / map) and the
   30 mm rule, so later tasks cite it.
7. **Feature use cases** in `docs/design/features/` (Gate A) for each screen the build will touch, from `_TEMPLATE.md`.
8. **Owner sign-off line**: `Design locked: <owner> <date>` in the use case / task log (ADR-0004).

## D. Hand-off into the firmware backlog

Do **not** write firmware. File or re-slice tasks (Sonnet planner for design/planning, Haiku for exact edits, see
PROCESS.md). What already exists and what changes:

| Existing | Status | What the new design does to it |
| --- | --- | --- |
| `P4-011` `ui_sq.c` framework | **done**, old zones | the glance design **replaces its zones**; reuse the page enum, toast queue, table-driven labels |
| `P4-012` cut-over (new core drives the screen) | **ready** | **hazard**, see below |
| `P4-013..P4-018` PERF, NOTE_EDIT, CHAN, SET, REC, RESET/HELP/FIRST_RUN pages | todo, written for the old layout | re-point to `SCREEN_SPEC_V2`; the **CHN page is the unresolved P1** (never drawn) |
| `P5-006` HARMONY page, `P6-001..P6-006` morph knobs, dice, performance keys, LEDs, keybed UI | todo | their screen parts follow the new design (value-name tables, dice toasts, hold feedback) |
| `P4-020` HARDWARE_TEST.md, `P4-021` HW-1, `P5-009` HW-2, `P6-008` HW-3 | todo | their checklists must test the new screen on the unit |

Likely **new** tasks (the design session decides): glance framework v2 (zones, fonts, palette tokens, icon blits);
lane renderer (pitch bars, playhead, accent/slide/ratchet marks, off-page playhead); knob band (values, enum pictures,
row styles); channel tiles; toast + cue engine with the string tables; map pages + HOME escalation; value-name tables
(raw integers today); the CHN setup page; palette tokens on all palettes. Each needs a use case and the UX gate.

### Backlog hazard (act on this early)

`python3 tools/backlog.py next` currently returns work that would wire the **old dense layout** into the new core
(`P4-012`; its dependencies P4-006, P4-009, P4-011, P5-001 are done). `/next-task` would pick it. Options, **for the owner
to choose**: (a) set `P4-012` to `blocked` until the design is locked (also stalls P4-019/P4-021 HW-1, the first
hardware test of the new core); (b) **split the cut-over** so the core can drive MIDI on the unit with a minimal
functional screen while the design is finished, then replace the screen. I recommend (b) if the owner wants hardware
validation of the timing/scheduler soon: it decouples the core's hardware risk from the design. I did **not** change the
backlog.

## E. On-device preview (the most valuable validation, and not done)

All legibility numbers so far are rules of thumb applied to PNGs on a laptop. A **spike task** could build a tiny
preview firmware that shows the rendered frames (RLE or palettised, a few frames, cycled with a button) on the **real
unit at real size**, so the owner judges "good enough" on the actual 30 mm display. Costs to scope in the spike: a frame
is 240 x 240 x 2 = 115,200 B raw (the app image is ~169 KB; flat-colour frames compress very well); it needs an
**owner-approved flash** (never flash without asking) and a way back (`restore/FM-1-official-V15.fwsc` via
`tools/fm1_install.py --port "JIANT"`, then power-cycle). The mock tool already writes the exact RGB565 values; an
exporter to a C array is small. If the spike proves too big, the fallback is the owner holding a printed 30 mm image
against the unit.

## F. Out of scope for the design session

The sequencer core, timing, MIDI, storage, algorithms (all done/queued under their own tasks); flashing; pushing.
