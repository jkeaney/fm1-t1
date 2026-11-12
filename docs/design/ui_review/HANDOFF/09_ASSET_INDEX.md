<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 09 Asset index: every file that matters

Paths are relative to the repo root `/Users/jameskeaney/Dev/apps/FM-1 - T1 - Bassline`.
`UI` = `docs/design/ui_review`.

## Project context (read, do not edit without cause)

| Path | Purpose |
| --- | --- |
| `CLAUDE.md` | project rules, hardware facts, commands, architecture (unity build, freestanding C) |
| `PRODUCT.md` | design context for the Impeccable skill (register `product`, users, personality, anti-references, principles, accessibility): **written this session from the owner's answers** |
| `docs/project/VISION.md` | product rules and the reference workflow |
| `docs/project/HANDOFF.md` | project-wide handoff (read after this package) |
| `docs/project/PROCESS.md`, `UX_GATE.md`, `ORCHESTRATOR.md` | lifecycle, the UX gate, the orchestrator runbook |
| `docs/project/OWNER_QUESTIONS.md` | the decision log; rows 116-120 are this session (120 withdrawn) |
| `docs/design/SCREEN_SPEC.md` | the **old** zone spec (dense layout) that `ui_sq.c` implements; to be replaced for PERF by `SCREEN_SPEC_V2.md` |
| `docs/design/SEQUENCER_UX.md`, `UX_REVIEW.md`, `PERFORMANCE_DESIGN.md`, `FULL_CONTROL.md`, `CONTROL_MAP.md` | the UX specs and the existing **ease score** (V/I/O/L) |
| `docs/design/button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, `control_map.tsv`, `page_states.tsv`, `help_text.tsv` | **authoritative control roles and strings** (generated into `build/gen/ui_tables.h`) |
| `tests/design/widths.tsv` and `tests/design/*.py|sh` | width limits and the checkers |
| `docs/design/scenarios/` | scripted walkthroughs (S01..S23) with the exact screen text per step |
| `docs/design/algorithms/*/SPEC.md`, `LOCK.md` | the six locked algorithms (knob names/ranges, dice layers per algorithm) |
| `docs/controls.jpg` (= `HANDOFF/assets/panel_photo.jpg`) | the physical panel photo with the stock function card |
| `firmware/src/ui_sq.c`, `gfx.c`, `build/gen/ui_fonts.h`, `ui_tables.h`, `ui_palettes.h` | the old layout, the canvas/theme code, the real fonts, the generated tables, the palettes |
| `docs/design/features/_TEMPLATE.md` | the Gate A/B use-case template; existing use cases: `P4-011-*.md` etc. |

## This package

`UI/HANDOFF/00_START_HERE.md` ... `09_ASSET_INDEX.md`, `SESSION_PROMPT.md`, `assets/owner_grid_sketch.png` (the owner's 6x6
concept), `assets/panel_photo.jpg`.

## The tool

| Path | Purpose |
| --- | --- |
| `UI/mock/README.md` | **how to use the pixel-exact mockup tool** |
| `UI/mock/mock.py` | render / diff / measure / contrast / palette |
| `UI/mock/current_perf.py` | the shipping screen reproduced with 0 differing pixels |
| `UI/mock/check_fidelity.py` | the fidelity gate (`--regen`) |
| `UI/mock/example_proposal.py` | a template proposal |

## Evidence about the shipping (old dense) screen

`UI/ASSESSMENT_A.md` (informal design review, 29/40), `UI/EVIDENCE.md` (+ `elements.tsv` 86 rows, `fonts.tsv`,
`palettes.tsv`, `text_pairs.tsv`, `truncations.tsv`, `contrast.py`, `measure.sh`, `build_evidence.py`).

## The proposals (newest last)

| Path | What |
| --- | --- |
| `UI/proposals/v1/` | zones kept; values large; shapes; colour per layer. `idle_before_after.png`, `playing_`, `row2_`, `snake_mute_`, `long_names_`, `undo_toast_before_after.png`; `NOTES.md` |
| `UI/proposals/v3-glance/` | the first glance screen + one-page held map. `*_before_after.png` (current left, proposal right), `proposal_v3_*.png`, `NOTES.md` |
| `UI/proposals/v3.1/` | v3 fixes: `*_v3_vs_v3_1.png`, `NOTES.md` |
| **`UI/proposals/v3.2/`** | **the latest (rejected as "not good enough")**: `proposal_v3_2_<scene>.png` (1x), `_4x.png`, `_grid.png`; `<scene>_v3_1_vs_v3_2.png`; contact sheets `states_sheet_1x/2x.png`, `cue_series_sheet.png`, `toast_dice_strip.png`, `lane_accent_test.png`; `geometry_v3_2.tsv`; `proposal_v3_2.py` (the source); `NOTES.md` (53-row finding table, text/geometry/toast specs, firmware cost, owner questions) |
| v2-grid | never built (stopped); the owner's concept survives only in `03_DECISIONS.md` and `assets/owner_grid_sketch.png` |

### The v3.2 scene names (use with `proposal_v3_2_<scene>_4x.png`)

`glance_idle glance_playing row2 row3 mutes stress stress_row2 stress_pal_paper stress_pal_ice stress_pal_mono`;
lane: `lane_full_playhead lane_full_playhead_first lane_muted lane_offpage lane_ratchet_sweep lane_vel_low`;
cue: `cue_idle_stopped cue_idle_playing cue_dice_all|notes|accnt|shape|artic|hits cue_mute_held cue_solo cue_fill cue_roll
cue_oct_latch cue_page cue_save_hold cue_undo cue_table_1..3`; toasts: `toast_dice_all|notes|accnt|shape|artic|hits
toast_dice_artic_snake toast_undo toast_redo toast_preset_pending toast_preset_pending_stopped toast_preset_loading
toast_algo toast_midi_channel toast_midi_off`; states: `state_add_page state_ext_lost state_ext_sync state_fil
state_first_run state_follow_off state_midi_channel state_past_length state_rec_playing state_rec_stopped state_reset
state_rol state_sel_muted_off state_sol state_wait`; maps: `map_A map_A_row3 map_B map_menu_countdown map_touch`;
`stage_total`.

### The images the owner has seen as files (v3.2)
`glance_playing_4x`, `glance_idle_4x`, `row2_4x`, `mutes_4x`, `stress_4x`, `toast_dice_strip.png`,
`toast_preset_pending_4x`, `toast_undo_4x`, `cue_series_sheet.png`, `map_A_4x`, `map_B_4x`, `states_sheet_2x.png`.

## The critiques

| Path | What |
| --- | --- |
| `UI/critique_v3/ASSESSMENT_A_v3.md` | design review of v3: **24/40** (+ `EVIDENCE_B_v3.md`, `scripts/`, `edge/` fixtures) |
| `UI/critique_v3_2/ASSESSMENT_A_v3_2.md` | design review of v3.2: **27/40**, element verdicts, personas, emotional journey, density verdict |
| `UI/critique_v3_2/EVIDENCE_B_v3_2.md` | measurements: render reproduction, text inventory, contrast, grid, edge-case sweep (156 knob cells, 408 channel tiles, 36 dice toasts), cue fit, accuracy vs TSVs, status of v3's ten findings; `scripts/` + `edge/` |
| `.impeccable/critique/*.md` | the stored snapshots (v3 24, v3.2 27) from the Impeccable skill |

## The Impeccable skill

`/Users/jameskeaney/.claude/plugins/cache/impeccable/impeccable/3.8.0/skills/impeccable/` (`SKILL.md`,
`reference/critique.md`, `reference/product.md`, `scripts/critique-storage.mjs`). Install new versions through the plugin
manager (`npx impeccable update` finds no skill folders in this repo).

## Git (nothing here is pushed)

Branch `t1-hardware-bringup`. Key commits: `4b84bfb` mock tool; `49b3ad8` evidence (shipping screen); `28feaf6` v1;
`77049c3` v3-glance; `cae5069`/`ddb71e0` critique v3; `d9bdd9a` v3.1; `4d69407` v3.2; `4a88dd0`/`5b97263` critique v3.2;
`815bf75` decisions 116-120 + PRODUCT.md + snapshots.
