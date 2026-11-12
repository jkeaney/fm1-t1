---
id: P4-030
title: Screen v2 table changes: add/remove channel, M-key hold timing, page_states and help rows, widths
phase: P4
type: task
status: review
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Bring the authoritative tables in line with the locked screen (owner decisions 121-126) so the firmware, the checkers and
the screen agree.

## Read first
- docs/design/SCREEN_SPEC_V2.md sections 5 and 6; docs/project/OWNER_QUESTIONS.md rows 121-126
- docs/design/knob_map.tsv (PERF ENC_SEL "clamped, no wrap"), key_map.tsv (KEY_B01..B04 hold = PREVIEW at 0.4 s),
  control_map.tsv, page_states.tsv (PERF/CHAN/REC/FIRST_RUN screen and cue_strip columns), help_text.tsv, tests/design/widths.tsv
- tests/design/check_scenarios.py, check_help.py, gen_help.py, gen_sweeps.py, width_probe.sh; tools/gen_ui_tables.py

## Deliverable
- SELECT: one detent past the last channel in use adds the next free channel (toast `[n] NEW  ALGO=PICK`).
- M1-M4 hold: release before 1.5 s = PREVIEW on/off (on release); 1.5 s = remove the channel (undoable, toast `[n] GONE  EDIT=UNDO`).
- page_states PERF/CHAN/REC/FIRST_RUN rows rewritten to SCREEN_SPEC_V2 (message slot, setup cue `CHN KEY, K2 = MIDI`, REC cue).
- help_text rows for the message strings; widths.tsv rows for the message slot.
- The owner signs the M-key timing in the Log (decision 125 timing was a design proposal).

## Acceptance criteria
- [x] gen_help.py, gen_sweeps.py, check_help.py, check_scenarios.py, width_probe.sh, `tools/gen_ui_tables.py --check` all pass
- [ ] owner sign-off line in the Log (pending: human-review gate)

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- Firmware drawing; other pages' layouts.

## Log

### 2026-11-13 executor (sonnet)
Changes: key_map.tsv + control_map.tsv M1-M4 hold rows (hint PREVIEW CHn at 0.4 s; release before 1.5 s = PREVIEW on release; "held 1.5 s: REMOVE CHn", toast `[n] GONE  EDIT=UNDO`); knob_map PERF SELECT row (one detent past the last channel adds the next free one, toast `[n] NEW  ALGO=PICK`, never past CH4); page_states PERF/CHAN/REC/FIRST_RUN screen, keys and cue_strip rewritten to SCREEN_SPEC_V2 (setup cue `CHN KEY, K2 = MIDI   CH1` for FIRST_RUN); gen_help.py (SELECT and M-key help) -> help_text.tsv and the sweep scenarios regenerated; widths.tsv gained 15 message-slot rows (136 px); gen_ui_tables.py now emits `ui_remove_rows` (UI_N_REMOVE) parsed from the `held N s: REMOVE` clause, failing if not exactly 4 rows; `UISQ_REMOVE_MS` deleted from ui_sq.c and replaced by `uisq_remove_ms(ctl)` reading the generated table (tests/ui_sq_v12_test.c uses it; tests/ui_sq_test.c PERF cue assertion updated to the new cue_strip). Firmware behaviour (actually adding/removing channels, release-fire PREVIEW) is NOT implemented: out of scope here (tables only).
Verification:
```
sh tools/check.sh                      -> CHECK PASS (RAM .data+.bss 23844 B of 73728)
python3 tools/gen_ui_tables.py --check -> gen_ui_tables: OK (6 algorithms, widths measured; ... key 359, help 392)
python3 tests/design/check_help.py     -> OK: 392 help rows, 2695 strings measured
python3 tests/design/check_scenarios.py-> 490 control-map rows, 0 with no scenario
sh tests/design/width_probe.sh         -> 312 strings, 0 over
```
**Owner must sign (human-review):**
1. The M1-M4 hold timing (decision 125, a design proposal): hint at 0.25 s, `PREVIEW CHn` named from 0.4 s, release before 1.5 s = PREVIEW toggle (now on release, no longer at 0.4 s), hold to 1.5 s = REMOVE the channel, undoable via EDIT. Note this changes today's behaviour (PREVIEW fired at 0.4 s while held).
2. The 1.5 s remove threshold specifically (long enough to avoid accidents mid-performance, vs. delay).
3. SELECT add rule: one detent past the last channel in use adds the next free channel on PERF (clamped at CH4); other pages stay clamped, no add.
4. FIRST_RUN cue wording `CHN KEY, K2 = MIDI   CH1` and PERF/CHAN/REC footer lines as in page_states.tsv.
Sign-off: ______ (owner, date)

### 2026-11-13 review (sonnet)
Verdict: PASS on criteria 1; Awaiting owner (human-review gate), left at `review`.
Re-ran: `sh tools/check.sh` CHECK PASS; `./build.sh` `RAM .data+.bss 23844 B of 73728` (headroom 49884 B, above 8192); gen_ui_tables --check OK; check_help 392 rows OK; check_scenarios 0 unmapped; width_probe 312 strings, 0 over. Generated `ui_remove_rows` in build/gen/ui_tables.h has 4 rows (the four M-key controls, 1500 ms), and the generator fails if the count is not 4.
Checked vs SCREEN_SPEC_V2 sections 5-6 and v12 strings: SELECT add rule (knob_map PERF SELECT, toast `[n] NEW  ALGO=PICK`, never past CH4) matches; M1-M4 hold rows (hint PREVIEW CHn 0.4 s, release before 1.5 s = PREVIEW on release, 1.5 s REMOVE, toast `[n] GONE  EDIT=UNDO`) match section 6; FIRST_RUN cue `CHN KEY, K2 = MIDI   CH1`, CHAN cue `K2 = MIDI   CHN = BACK`, REC cue `PLAY KEYS TO WRITE   REC = DONE`, page title `CHN CHn SETUP` match spec 4.5/5/6. Widths rows are all 136 px slot strings from the spec, all fit.
Wording fix made: page_states.tsv PERF row said a mute key "held 0.25 s names PREVIEW CHn"; spec section 6 and key_map say the PREVIEW name appears from 0.4 s (0.25 s is the generic hint start). Changed to "held 0.4 s names PREVIEW CHn (release before 1.5 s toggles it)". Checks re-run green.
Scope note: firmware behaviour (add/remove, release-fire PREVIEW) is intentionally not implemented here (tables only); the tick box for sign-off stays open.
Awaiting owner: sign the four items in the executor entry above (M-key timing incl. the 1.5 s remove threshold, SELECT add rule, FIRST_RUN/PERF/CHAN/REC cue wording).
