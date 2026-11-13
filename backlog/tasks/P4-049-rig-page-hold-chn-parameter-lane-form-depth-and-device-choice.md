---
id: P4-049
title: RIG page: hold CHN opens it; wire select, PARAM, LAYER, FORM, depth, device choice, undo and reset, HELP
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-048]
gate: human-review
---
## Goal
Build the RIG page, the setup surface for the selected channel's device: hold CHN opens it, the four wires are chosen and shaped with K1-K4, and the device is picked per channel. Everything on it comes from the profile data (P4-046) and the slot API (P4-047). No per-device and no per-algorithm branch in the screen code.

Owner ruling 2026-11-13: FULL RIG is in v1: "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."

## Read first
- docs/design/hardware/RIG_MOD_V1.md, ADR-0008 and docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md (the approved RIG mock, the final K1-K4 meaning)
- docs/design/knob_map.tsv RIG K1 PARAM, K2 LAYER, K3 FORM, K4 MOVE (or their P4-045 renames), SELECT, ALGO, PRESET; key_map.tsv PERF CHN hold (~263) and the RIG key rows; button_modes.tsv RIG rows; control_map.tsv, page_states.tsv, help_text.tsv
- docs/design/SCREEN_SPEC_V2.md 4.x, 5 (messages), 7, 8 (line ~174: the RIG page placeholder this task replaces)
- docs/design/hardware/HW_SUPPORT_LOWLEVEL.md sections 4 (page layout, three knob rows rule) and 7; MODULATION.md 5.2 (setup gestures)
- firmware/src/ui_sq.c (page table ~285-300 with the RIG stub, `uisq_rig_row` ~250, CHN hold ignored ~2144-2146, `uisq_short`, toasts, the cue strip), ui_sq_glue.c, ui_input.c; firmware/src/sq_hw.c; slot API of P4-047; sq_chan.c (UndoRec / reset family `sq_reset_algo`)
- docs/design/features/P4-015-chan-page-*.md (the CHN key and the CHAN page it must not break)
- docs/design/ui_review/proposals/v12/v12.py, tests/v12_compare.py; backlog/tasks/P4-040-*.md as a screen-task model

## Deliverable
- Hold CHN (0.4 s per key_map; latched, stays open on release; a tap still opens CHAN exactly as today) opens the RIG page of the selected channel; HOME or CHN tap leaves; the header names `RIG CHn <device>`
- Wire select: a wire is selected by touching its R-knob on ROW 3 (the page opens on the wire touched last) or by pressing the wire's cell key per the approved mock; the selected wire is drawn in the channel colour
- K1 PARAM: walks the device's pick order by name (`hw_next_param`, wraps, skips parameters already on another wire, `-` = none); status line shows the long name and CC; a select parameter linked to another (SCTYP -> SCDEP) takes the second wire as P4-045 decides, with the approved toast
- K2 LAYER / lane source, K3 FORM, K4 DEPTH (or the P4-045 renames): exactly as approved in P4-045 (proposed: lane A or B, RAND / WALK / RAMP / ACC, 0..127 where 0 gives the knob back to the player with the toast `DEPTH 0: THE KNOB IS YOURS`); a layer or form the algorithm lacks is shown dim and freezes the lane (data from `ALGOS[].layer[]`, no algorithm name)
- SELECT chooses the channel (CH1..CH4, clamped); ALGO chooses the DEVICE (`NONE`, `GENERIC`, the profiles in list order; the cue strip names the role `ALGO = DEVICE` while it moves; applies after the settle time, then the device's default slots at DEPTH 0); the device choice is stored per channel in `ModSave`; a device change on a playing channel parks the old driven wires (P4-047 rule) and starts the new ones from the next step
- Undo and reset: EDIT undo covers the seeds only (P4-047 rule, decision 100); RESET on the RIG page resets the four slots of the selected channel (not other channels), with the same confirm pattern as the existing RESET; the `RST R3` wording follows P4-045
- HELP: touching any RIG control in HELP shows its help lines (help_text.tsv rows regenerated), and the cue row at the bottom names the last-pressed or held control (ADR / owner decision 124); toasts for every change within the 136 px message slot
- Tables updated to match (knob_map, key_map, button_modes, control_map, page_states, help_text, overlay_text if used), SCREEN_SPEC_V2 4.x and 7 edited (the RIG page leaves "not covered here"), scenarios S21-S23 replaced by the walk of P4-045, widths.tsv rows; `forbidden_terms.txt` gets the stale RIG wording
- v12 mock scenes for the RIG page (device chosen, wire selected, select parameter pair, DEPTH 0, empty device NONE, HELP) added and exported; host render equals the mock pixel for pixel; existing scenes unchanged
- Host tests: hold CHN opens, tap CHN still opens CHAN, K1-K4 and ALGO change the slot state exactly (`sq_slot_set` called with the expected values), skip-used-parameter, linked pair, device change parks, RESET scope, a dummy 2-parameter device walks with no code change, a save/load round trip of the device and slots through the preset
- Grep proof in the Log: no device name, CC number or algorithm name in the new screen code

## Acceptance criteria
- [ ] Hold CHN opens the RIG page; tap CHN still opens CHAN; HOME leaves (host test)
- [ ] Every RIG control row in the tables is wired (host test) or marked v2 with a toast; the `check_control_map.py` baseline shrinks by the RIG rows and does not grow
- [ ] Setup of one wire from PERF takes <= 2 gestures (hold CHN + one knob turn on the opened wire), measured in the Gate B walk
- [ ] `python3 tests/v12_compare.py`: existing scenes unchanged, RIG scenes pass; check_scenarios / check_help / check_sequencer_ux / width probe / `sh tools/check_docs.sh` pass
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)
- [ ] Owner approves the RIG screen and behaviour (human-review): `Approved: <name> <date>`

## Verification
```sh
sh tools/check.sh                    # last line: CHECK PASS
./build.sh | grep 'RAM'              # paste into the Log
python3 tests/v12_compare.py
python3 tools/check_control_map.py   # baseline must not grow
```

## UX gate
Producer-facing. Gate A: `docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md`. Gate B after the build on renders; on the unit in P4-050. Update the CHAN use case `docs/design/features/P4-015-*.md` for the CHN tap / hold split.

## Out of scope
- ROW 3 knobs (P4-048) and the MOD engine (P4-047)
- A real second device profile
- Preset slot storage in flash (P7); only the RAM round trip
- The other unwired controls of P4-043 (ALGO on NOTE_EDIT / SET, OCT peek, preview pick)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created. Owner ruling 2026-11-13: "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."
