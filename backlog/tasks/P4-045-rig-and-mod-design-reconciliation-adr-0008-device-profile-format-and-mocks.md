---
id: P4-045
title: RIG and MOD design reconciliation: ADR-0008, device-profile data format, ROW 3 and RIG mocks
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-021]
gate: human-review
---
## Goal
Turn the four older RIG / MOD designs into ONE buildable design that matches the code now built, so tasks P4-046 to P4-050 can be built without guessing. Docs only: no firmware, no tools, no build.

The owner ruled on 2026-11-13: MOD output is IN v1, and a FULL RIG is in v1: "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."

So the design must deliver: device profiles are DATA (a new device only fills in a table or file, no per-device code); the TB-3 is the first complete profile; ROW 3 knobs R1-R4 and the RIG page (hold CHN) are wired; and the per-algorithm MOD lanes (`s_mod`, MODULATION.md) are output as CC to the channel's device.

**The conflict this task must resolve (do not skip it).** The sources disagree:
- P2-018/019/021 (`HW_SUPPORT_HIGHLEVEL.md`, `HW_SUPPORT_LOWLEVEL.md`, `TB3_PROFILE.md`) designed a RIG page (hold CHN) with per-wire PARAM / LAYER / FORM (RAND WALK RAMP ACC) / MOVE on K1..K4, ROW 3 R1-R4 = wire LEVEL, and HAND / SEQ / DEV states. Those rows are still in `knob_map.tsv`, `key_map.tsv`, `button_modes.tsv`, `control_map.tsv`, `help_text.tsv`, scenarios S21-S23 and SCREEN_SPEC_V2 4.2 and 7 (`row3_rig`).
- P2-022 (`MODULATION.md`, owner decision 107/108, approved 2026-11-11) SUPERSEDED that: no RIG page and no CHN hold; ROW 3 IS the MOD page (K pick, hold OCT+ DEPTH, hold OCT- LEVEL, ALGO = device); lanes A (step, leans on accents) and B (sweep) from `s_mod` and the algorithm's own indices; no HAND/SEQ/DEV; `ModSave` 25 B.
- The code built so far follows MODULATION.md: `sq_mod_lane`, `sp18_cell`, `sp19_lfo`, `sp20_lean` in `sq_prims.c`, `ModIdx` and `AlgoDesc.mod` in `sq_types.h`, `acid_mod`, `s_mod` seed and `UndoRec` 94 B, `ModSave` 25 B in `sq_chan.c`; nothing calls `->mod`.
- The owner's 2026-11-13 ruling and P4-043's old text want BOTH ROW 3 knobs R1-R4 AND the RIG page on CHN hold.

Produce a reconciliation that keeps the owner's ruling (ROW 3 live + RIG page setup + MOD output) on top of the built MODULATION.md engine. The proposed starting point (change it only with a stated reason): RIG page = SETUP surface per wire (K1 PARAM pick, K2 LANE source (A step / B sweep, today fixed by slot parity: MODULATION owner question 3), K3 FORM, K4 DEPTH (= the old MOVE), ALGO = device, SELECT = channel); ROW 3 R1-R4 = live LEVEL of wires 1-4, hold OCT+ on ROW 3 = DEPTH peek is kept only if the owner wants it; HAND = DEPTH 0. FORM maps onto the built primitives: RAND = `sp18_cell` per step, ACC = `sp18_cell/2 + sp20_lean` (lane A today), RAMP = `sp19_lfo` over the algorithm's cycle (lane B today), WALK = new, `sp18_cell` linearly interpolated between steps (the only new primitive). State what each choice costs in `ModSave` bytes (a form needs 2 bits: pack into the param byte, keep 3 B per slot and 25 B per channel, or grow the assert and the 100 B sibling array, and say which).

## Read first
- docs/design/MODULATION.md (all; sections 2, 3, 4, 5.3, 7, 9, 10 matter most) and docs/design/hardware/HW_SUPPORT_HIGHLEVEL.md, HW_SUPPORT_LOWLEVEL.md (sections 1.8, 3, 4, 9), docs/design/hardware/TB3_PROFILE.md and tools/hwprof/tb3.tsv
- backlog tasks P2-018, P2-019, P2-020, P2-021, P2-022 (Logs and Deliverables; the owner approvals and decisions 97-108 are in the Logs)
- docs/project/decisions/ADR-0003-sequencer-core.md (ModSave sibling array, sections 10 and 22; UndoRec; the CC lead rules) and ADR-0007-algorithm-ui-descriptor.md (no screen branch names an algorithm or device; line ~26 "a device profile supplies ROW 3 labels the same way")
- docs/design/SCREEN_SPEC_V2.md 4.2 (ROW 3 tile line), 4.3 (the `ROW n` boxes), 7 (`row3_rig`), 8 (the RIG page placeholder, line ~174)
- docs/design/knob_map.tsv rows PERF R1-R4 and RIG K1-K4 / SELECT / ALGO / PRESET; docs/design/key_map.tsv (PERF CHN hold, line ~263, and the 32 RIG key rows); docs/design/button_modes.tsv RIG rows; control_map.tsv RIG context; page_states.tsv
- docs/design/algorithms/acid/SPEC.md section 2.1 (the mod lanes and `s_mod`); the other five SPEC.md files' mod sections
- firmware/src/sq_prims.c ~120-150 (`sp19_lfo`, `sp20_lean`, `sq_mod_lane`), sq_types.h (ModIdx, AlgoDesc.mod, UndoRec, SQ_SEED_MOD), sq_chan.c ~25-60 and 580-680 (ModSave, `sq_smod`, preset save/load), sq_core.c (`sq_emit`, `sq_eval`, `sq_tick`, `sq_all_off`), ui_sq.c (`uisq_rig_row` ~250, the CHN hold ~2144, OCT row switching ~2068)
- tools/gen_ui_tables.py (how the TSVs become C tables, the `--check` and `--selftest` pattern) and tests/design/widths.tsv, width_probe.sh
- docs/design/ui_review/proposals/v12/v12.py (the `DEVICES` table ~111, `rig_row`, `row3_rig` scene ~607, `MESSAGES`) and NOTES.md
- backlog/tasks/P4-043-*.md (the RIG and MOD decisions were removed from it; it now points here)
- /tmp/readiness_review.md sections B, D and finding 6 (may be gone; the findings are copied into the Log below)

## Deliverable
1. **Reconciliation doc** `docs/design/hardware/RIG_MOD_V1.md`: one page of truth that says, for every item of the table below, KEPT / CHANGED / DROPPED from which source and why. Items: RIG page (existence, entry by CHN hold, controls K1-K4 / SELECT / ALGO / PRESET, keys, buttons), ROW 3 (R1-R4 meaning, entry by OCT+, rail `ROW n/3`), HAND / SEQ / DEV (and its replacement), lane forms (RAND WALK RAMP ACC vs lane A / B), DEPTH / MOVE, LEVEL, device choice (ALGO on the RIG page; CHAN page gains nothing), park on stop / mute, active-slot cap (8), `ModSave` bytes and `UndoRec`, dice (SHAPE + `s_mod`, toast `+ MOD`), REC of ROW 3 (none). Then the exact list of tables, scenarios and SCREEN_SPEC_V2 passages that change (the changes themselves are made by P4-048 / P4-049, except the tables that P4-046 owns).
2. **Device-profile data format decision**: profiles are data, generated into a flash table the way `gen_ui_tables.py` does it. Decide and write down: source location (recommended: `docs/design/hardware/profiles/<device>.tsv`, one file per device, moved from `tools/hwprof/tb3.tsv`, plus one shared header block per file), the columns (id, long name <= 16, short name <= 5 letters and <= 38 px in AF_S, CC, kind cont / select, zones, home, default depth, pick order, link, park / neutral value on stop, provenance fields), the generator (`tools/gen_hwprof.py` -> `build/gen/hwprof.h`, with `--check` and `--selftest` like `gen_ui_tables.py`, and what it rejects), the C table shape (`HW_PROFS[]`, `HwProf`, `HwParam`; flash only, no RAM), how a channel stores the device (an id in `ModSave`), the rule "no per-device or per-algorithm branch in firmware", and the exact steps to add a second device (file + one list line; nothing else). Include the licence and provenance rule from P2-019 2.3 (our own facts, midi.guide only a cited reference).
3. **ADR-0008** `docs/project/decisions/ADR-0008-rig-mod-v1.md` (next free id): decision, context (the conflict above), the reconciled model, `ModSave` / `UndoRec` / `ModRT` byte costs against the 8192 B RAM headroom rule, the ISR cost bound target (<= about 25 integer operations per active slot per step, max 8 active slots, and where `->mod` is called so the main-loop `uisq_view_fill` path never calls it), the CC rate rule (at most one CC per 10 ms per wire, dedupe, CC stamped just ahead of its note), and consequences. Update `docs/project/decisions/README.md`.
4. **Gate A use case** `docs/design/features/P4-045-rig-and-mod-for-the-tb-3.md` from `_TEMPLATE.md`: the steps from "I picked a TB-3 on CH1" to "the cutoff follows the accents and I turn it by hand", gesture counts, predicted ease score, five-surface sync, live checklist. Target: setup of one wire <= 2 gestures from PERF, performance (LEVEL, dice, undo) = 1 gesture.
5. **Mocks for owner approval**: in `docs/design/ui_review/proposals/v12/v12.py` (and `NOTES.md`) draw (a) ROW 3 tiles for the TB-3 (names from the profile data, the `ROW 3` rail, live-row state, HAND vs driven marker), (b) the RIG page (header, four wire cells, status lines, cue strip, the held / touched states), (c) the toasts. Render at 1x and 4x into `.../v12/out/` (follow the existing render command in NOTES.md), add the scenes to the SCREEN_SPEC_V2 state catalogue list in the task log (the spec itself is edited by P4-048 / P4-049 after approval). Every string measured with the real-font width probe (`tests/design/widths.tsv`, `sh tests/design/width_probe.sh`, 0 over).
6. **Verified-parameter list**: a table in RIG_MOD_V1.md of exactly which TB-3 parameters and CCs are verified and how, using this base and correcting it from the sources: verified by the Roland TB-3 MIDI Implementation v1.10 chart AND by the owner's ear on 2026-11-11 (decision 106): CC 74 CUTOFF, 71 RESONANCE, 12 ENV MOD X, 13 ENV MOD Y, 16 ACCENT, 17 EFFECT, 104 TUNING; verified by the owner's own ear test only: CC 1 PAD-Z, 11 PAD-Y, 68 SCATTER TYPE, 69 SCATTER DEPTH; accent threshold velocity 100 or more and slide by note overlap (decisions 104, 105). Mark UNVERIFIED: the zone count of SCTYP (8 is a placeholder), SCDEP at 0 and a type change, the homes RESO 40 and EFFEC 0, knob-versus-CC pickup (H-MOD1), whether the TB-3 sends its knob moves on MIDI OUT (H-MOD3), CC timing with 4 active slots (H-MOD4), receive channel 2 (CLAUDE.md default) on this unit. Nothing is copied from `tb3-midi-guide.csv`.
7. Update P4-046 .. P4-050 if the decisions change their scope (edit their Goal / Deliverable; do not re-order), and record any new owner question with lettered options and a recommendation in the Log and in `docs/project/OWNER_QUESTIONS.md`.

## Acceptance criteria
- [ ] RIG_MOD_V1.md exists and resolves every row of the item list in Deliverable 1; no row says "TBD"
- [ ] The data format is specified so that a second device is "a new .tsv plus one list line" and the document names every file a new device touches (none of them firmware C)
- [ ] ADR-0008 exists with byte costs, the ISR bound and the rate rule; `decisions/README.md` lists it
- [ ] Gate A use case exists with gesture counts; setup <= 2 gestures, performance = 1
- [ ] v12 renders of ROW 3 (TB-3) and the RIG page exist at 1x and 4x in `proposals/v12/out/`; width probe 0 over
- [ ] The verified / unverified TB-3 list is present and matches TB3_PROFILE.md and MODULATION.md sections 1, 2 and 9
- [ ] `python3 tools/backlog.py check` shows 0 errors; `sh tests/design/width_probe.sh`, `python3 tests/design/check_scenarios.py`, `python3 tests/design/check_help.py`, `python3 tools/check_sequencer_ux.py` still pass (no table was edited by this task)
- [ ] Owner approves the reconciliation and the mocks (human-review): a line `Approved: <name> <date>` in the Log

## Verification
```sh
python3 tools/backlog.py check
sh tests/design/width_probe.sh        # needs build/gen/ui_fonts.h from an earlier ./build.sh; do NOT run a build in this task
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py && python3 tools/check_sequencer_ux.py
sh tools/check_docs.sh
```

## UX gate
Producer-facing design. This task WRITES the Gate A use case for the chain (P4-048 and P4-049 cite it). Gate B for the chain happens in P4-048, P4-049 and P4-050.

## Out of scope
- Any firmware, tools, tests or table edit (the tables are edited by P4-046 / P4-048 / P4-049)
- A second real device profile (a dummy one in tests is P4-046)
- Algorithms other than ACID (their `mod` rows are P5; this task only states the contract they follow)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created. Owner rulings 2026-11-13 (AskUserQuestion), recorded verbatim: (1) "MOD output is IN v1." (2) FULL RIG is in v1: "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."
- Findings copied from the POC readiness review (/tmp/readiness_review.md, 2026-11-13): the CHN hold is silently ignored (ui_sq.c ~2144); `uisq_rig_row` returns 0 (~250); the MOD hook `sq_prims.c` ~141 is never called; EUCLID's SHAPE layer is `s_mod` only, so its SHAPE dice does nothing audible without MOD; ISR CPU is unmeasured (sq_tie_end, P4-042); `uisq_view_fill` runs 64 `sq_eval` per frame so `->mod` must not be called from it.
- Known conflict flagged by the planner: tables and scenarios still carry the P2-019/021 RIG design while MODULATION.md and the built engine carry the P2-022 MOD page. Resolve it here; do not leave both.
