---
id: P4-015
title: CHAN page: MIDI channel, algorithm, length
phase: P4
type: task
status: done
priority: 2
executor: haiku
depends_on: [P4-013]
gate: none
---
## Goal
Set up a channel in at most two gestures: MIDI channel (unique across channels), algorithm, pattern length and pages.

## Read first
- docs/design/SEQUENCER_UX.md §2.3, §4.4 (unique MIDI channels), §1.3, §1.6, §11.3 (first-run step 'D#4 K2: MIDI CHANNEL')
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c CHAN page: K2 MIDI channel with the 0.6 s settle, which also commits on ANY exit from CHAN (PLAY, a page button, HOME), via `sq_set_midi`; the unique rule toast naming the channel set to OFF; ALGO encoder picker over the `ALGOS[]` names (six after P5-008); length and page add
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [x] Committing the MIDI channel happens on 0.6 s stillness AND on every exit path (test covers PLAY, GLO, HOME, D#4 exits)
- [x] Taking a used MIDI channel sets the other channel OFF and one EDIT undo restores both (decision 77)
- [x] Scenario S01 steps for CHAN reproduce
- [x] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-015-chan-page-midi-channel-algorithm-length.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).

### 2026-11-13 executor (sonnet)
Files: firmware/src/ui_sq.c (CHAN section before ui_sq_home; state uisq_cm_*), firmware/src/ui_sq_glue.c, tests/ui_sq_chan_test.c (new), tests/run_tests.sh, tests/ui_sq_v12_test.c (footer check now uses uisq_cue_split), docs/design/features/P4-015-*.md (Gate A and B filled).
- `sh tools/check.sh` -> `CHECK PASS`. `ui_sq_chan_test: 0 failure(s)`; `ui_sq_v12_test: OK (0 failures)`; `v12_compare: 44 scenes/sheets, 0 failed` (chan_page and chan_midi_clash included).
- `./build.sh | grep RAM` -> `ok    image 192464 B; RAM .data+.bss 29224 B of 73728; pool 78520 B of 368640` (headroom 44504 B, needs >= 8192).
- Criteria. Exits: the test commits on HOME tap, the D#4 key, PLAY, GLO, REC, SAVE (ui_sq_other_button), a dice button, SELECT, ui_sq_page_set(MENU) and a hold on a step key (NOTE_EDIT), plus HOME pressed (hold). Steal: CH1 takes MIDI 2 from CH2, CH2 OFF, toast `[2] MIDI OFF  2 TO CH1`, one EDIT restores both, again redoes (decision 77). S01 steps 2-4 (D#4, K2, HOME) are in the test. Strings: the CHAN footer halves (page_states cue via uisq_cue_split), the page title (ui_msgs ENC_CHN_CH2_SETUP), the toast words and the ALGO touch row come from ui_tables.h. Residue for the reviewer's grep: the CHAN tile labels / value names (`LENGTH MIDI SOURCE FOLLOW`, `OWN`, `OFF ON`, 1..16) are the pre-existing MorphDesc rows `uisq_chan_row` / `uisq_n_*` (P4-028, mock-exact); the test asserts the labels equal the knob_map rows. `uisq_foot_cue` (setup cue) and the REC / steps footers are P4-029 literals, untouched.
- Behaviour. K2: pending value, tile + header follow at once, commit after 0.6 s of stillness (UISQ_MIDI_SETTLE_MS) or on any exit; no wrap; the commit clears the setup cue's new bit when the result is a real MIDI channel. K1 LENGTH 1..64 (a fast turn of 3+ detents steps 4), K4 FOLLOW, OCT-/OCT+ = -PAGE / +PAGE (sq_len_set / sq_page_add; EDIT undoes the add), SELECT clamps (no add on CHAN), ALGO encoder (ui_sq_algo_turn: no wrap, skips unwritten rows, strip + `ALGO <short> AT STEP`), white keys, M keys, dice and EDIT as on PERF.
- Hazards checked (bounds / stale state / index): pending value is 0..16 or 0xFF and its channel is re-validated (`< SQ_NCH`, in use) in uisq_chan_commit and in the view (corrupt values ignored, test); the pending value is dropped when its channel is removed or leaves use; SELECT commits before moving so the value stays with its channel; no new stack or queue (the existing uisq_gone stack is cleared by a MIDI change, algorithm change and page add, as a dice does, so EDIT undoes the newest action); LENGTH clamped 1..64, viewed page clamped to the page count after every length change (test: 4 pages to 1, 64 to 36, shortened while playing: playhead < len); sq_len_set keeps hidden edits (test); toast hint built into an 18 B buffer from the table row, two-digit MIDI `16 TO CH1` still fits 136 px (test); all three CHN / HOME / key paths go through ui_sq_chan_open/close; the commit also runs from ui_sq_input and ui_sq_page_set so no page change can leave it pending. The 0.6 s timer uses a signed difference (wrap safe).
- What P4-032 must wire: the CHN key tap is ALREADY handled in uisq_key_up (D#4 tap on release, under the 0.4 s hold row of key_map where one exists: PERF / NOTE_EDIT -> ui_sq_chan_open(), CHAN -> ui_sq_chan_close()). P4-032 adds the CHN hold (0.4 s -> RIG) in uisq_keys_tick (the hold row KEY_B05_DS4 400 already exists; a release at or over it does nothing now) and widens the page test in ui_sq_algo_turn from CHAN to PERF (the glue already passes the ALGO encoder every frame). SET -> CHAN switching (D#4 on SET) belongs to the SET task: call ui_sq_chan_open() after allowing that page in its first line.
- Not built (debt, for the reviewer to file as FB): K3 SOURCE (list of offered blocks; sq_page_src_set exists); the toasts `ONE PAGE: K1 = SHORTER` and `64 STEPS = MAX` (no MESSAGES row in v12.py: both buttons are silently inert at the ends); the dim styling of an unsettled K2 value; GLO / REC / SAVE only commit from CHAN (their pages are other tasks); the displaced channel's header `MOFF` shows as the existing MIDI OFF chip.

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none; CHAN picture locked in v12, no owner line required).
- `sh tools/check.sh` -> CHECK PASS (re-run). `./build.sh | grep RAM` -> `ok    image 192464 B; RAM .data+.bss 29224 B of 73728; pool 78520 B of 368640` (headroom 44504 B, needs >= 8192).
- Hazard scan, one pass. Commit paths: K2 stillness (uisq_pg_chan, signed diff), HOME down, HOME/D#4 tap (chan_close), every button (ui_sq_button, ui_sq_other_button), SELECT (commit before move), ui_sq_page_set, ui_sq_input for any non-CHAN page; pending value re-validated (<=OFF, ch<SQ_NCH, in use) at commit; dropped in uisq_do_remove. Steal: sq_set_midi writes one UndoMidi, commit clears uisq_gone so one EDIT restores both; OFF/same-value paths are safe. No new stack. ISR: UI calls are main-loop (sq_chan.c contract), len is a byte and the scheduler clamps it (sq_core.c:330); viewed page clamped after K1/OCT-; sq_len_set keeps hidden edits. ALGO: uisq_algo_ok guards ALGOS[].name[0] (name is an inline char[8], rows 1-5 are zeroed, not NULL pointers), walk stops at ends, count capped. Literals: uisq_chan_row/uisq_n_* equal knob_map labels and are pre-existing P4-028 mock-exact rows; footer, title, toast words come from tables.
- Findings, all non-blocking, filed as FB-018: (1) ONE PAGE / 64 STEPS toasts missing: no MESSAGES row exists, adding one is a design/table change, not for this task; (2) K3 SOURCE inert: new pages copy the source of the page before, so safe; (3) a steal can hit a removed channel (toast names a channel not on screen); (4) EDIT after a later K1 length change swaps LENGTH (core, documented not undoable) and the view falls to page 1.
- Note: EDIT while a K2 value is pending commits it then undoes it (net cancel, toast UNDO); acceptable.
- Gate B: steps 1-7 walked via tests/ui_sq_chan_test.c (0 failures) and build/ui_sq/chan_*.png; actual scores MIDI 12, ALGO 11, LENGTH 12 (equal to predicted, pass rules met). Five surfaces agree (v12_compare 44 scenes 0 failed; knob_map/button_modes/key_map CHAN rows match except the debt above). Live checklist re-answered from the tests: all yes. Hardware not yet checked (HW-1), per task gate none this is not a blocker.
