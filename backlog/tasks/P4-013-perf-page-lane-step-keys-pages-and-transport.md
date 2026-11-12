---
id: P4-013
title: PERF page: lane, step keys, pages and transport
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-012]
gate: none
---
## Goal
The home page the device boots into: the selected channel's lane, step keys tap ON/OFF, hold 0.4 s opens NOTE_EDIT, page keys, PLAY/STOP, channel cells.

## Read first
- docs/design/SEQUENCER_UX.md §1.5, §1.7, §2.1, §3, §4.3, §5 (traces U-rows for PERF), §7
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c PERF page: lane with playhead and FOLLOW, step key handling (U14 tap, hold -> NOTE_EDIT named `hold=EDIT`), PG-/PG+, PLAY/STOP via the transport request flag, channel cells Z1, preview of the selected channel
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/
- Live checklist item for the HW-1 sheet (FB-015): on AF_PAIRING algorithms (ACID) a tap or an ON edit made during step p-1 is heard from the next pass, not at step p: the pairing snapshot is taken one step ahead, the same one-step latency the old slide pairing had. Not a bug; confirm it feels acceptable.
- (P4-024 review) the 0.4 s hold that opens NOTE_EDIT must call `sq_note_edit_begin(ch,p)` on the hold, so the session's writes share ONE undo record; without it every `sq_step_set` / `sq_step_clear` is a record of its own. Call `SQ_FRAME_SYNC()` (sq_chan.c) once per frame before input (P4-011/P4-012 wire it). `sq_global_set` returns 0 both for "no change" and for "a load is pending": the SET/SCALE pages need no toast for it (the pending load replaces every global).

## Acceptance criteria
- [x] Scenario S01 steps 1-6 reproduce in the host test (screen strings and API calls)
- [x] A tap on a step key toggles it via `sq_step_set/clear` and the lane redraws within one frame
- [x] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-013-perf-page-lane-step-keys-pages-and-trans.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).
- 2026-11-13: from FB-017 item 2, this task must also wire: SELECT adds a channel (PERF only), M-key hold remove/preview via ui_remove_rows, white keys as steps, K1-K4 via sq_morph_turn, the dice buttons, and decide what the keys do (song.grid 2 keeps them silent now).

### 2026-11-13 executor (Sonnet 5.5)
Built: ui_sq.c PERF controls (ui_sq_keys, ui_sq_button, ui_sq_knob, ui_sq_select_turn, hold timers in ui_sq_input), wired by ui_sq_glue.c; new tests/ui_sq_perf_test.c (in run_tests.sh); use case docs/design/features/P4-013-perf-page-lane-step-keys-pages-and-trans.md (Gate A filled; Gate B for the reviewer).
- `sh tools/check.sh` -> `CHECK PASS` (v12 renders still equal the mock; ui_sq_perf_test: 0 failures).
- `./build.sh | grep RAM` -> `image 182192 B; RAM .data+.bss 29192 B of 73728; pool 78520 B of 368640` (headroom 44536 B, above 8192).
- Wired: white keys (tap = sq_step_tap on release, hold 0.4 s = sq_note_edit_begin + NOTE_EDIT), M1-M4 (tap mute, hold + release < 1.5 s toggles the preview bit pad[0]&0x80, 1.5 s removes: out of use + muted, EDIT restores), SELECT (move / add past the last), K1-K4 via sq_morph_turn on the live row, dice FX/SCL/SEQ/ENV/ARP/LFO (absent layer inert), EDIT undo/redo, OCT-/OCT+ rows, PG-/PG+ (FOLLOW off, PG+ twice adds a page). PLAY was already wired. Strings come from ui_msgs (digits and the algorithm short name are put into table rows).
- Not done, for follow-up (not in tables/MESSAGES or out of scope): no `PAST THE END` / `64 STEPS = MAX` message row (a tap past LENGTH is silent); a step turned ON while stopped does not sound (sq_live_* are ISR-side); hard-tap ACCENT; CHN/RIG, SOL/FIL/ROL, RST keys; ALGO and PRESETS encoders (drained); preview toggle has no toast; ADD_PAGE overlay is the toast only; NOTE_EDIT is a stand-in that lasts while the opening key is held (P4-014 replaces uisq_pg_note_edit); channel removal is UI state (uisq_used + mute), not in presets.
- Reviewer: look at uisq_key_up / uisq_keys_tick (hold ladder) and the removal semantics.

### 2026-11-13 review (sonnet)
Verdict: FAIL, back to todo. `sh tools/check.sh` = CHECK PASS; `RAM .data+.bss 29192 B of 73728` (headroom 44536 B, OK). Gate B filled in docs/design/features/P4-013-*.md.
Hold ladder (uisq_key_up / uisq_keys_tick) is correct: the 0.4 s and 1.5 s come from ui_hold_rows / ui_remove_rows, `fired` stops a tap after a hold, mute-on-tap vs preview-on-release split is right, ISR only reads muted/pad[0] (single bytes), note-offs follow each slot's own flags so removal cannot hang a note. Findings to fix:
- [ ] F1 (must fix) ui_sq.c uisq_do_remove (~line 893): removal only mutes MIDI. `pad[0] & 0x80` (preview) stays set and sq_emit's preview gate (sq_core.c:148) ignores `muted`, so a removed channel with PREVIEW on keeps sounding on the built-in synth, and a removed channel's preview toggles are invisible. Fix: on remove clear bit 7 of pad[0] (save it with uisq_gone_muted, restore on EDIT undo); add a test that after remove with preview on, sq_emit/the preview gate is off, and EDIT restores it.
- [ ] F2 uisq_gone is one slot: removing a second channel before EDIT overwrites the first, so the first cannot be undone (and any dice or PG+ clears it, ui_sq.c ~906-940). Keep a per-channel gone mask (uisq_gone_mask + muted/preview bytes) or document the single-level rule in key_map and test it; EDIT should restore the most recent.
- [ ] F3 uisq_key_up returns early when page != PERF, before ui_sq_held_m_set(0) / ui_sq_hold_end(). Hold M1, then hold a white key 0.4 s (NOTE_EDIT), release M1: the PREVIEW/REMOVE hint stays. Clear the M hint before the page check.
- [ ] F4 (minor) NOTE_EDIT stand-in never ends the sq note-edit session; P4-014 must close it. Note it in P4-014's Log when picked up.
Gaps triage: legitimately later: NOTE_EDIT body (P4-014), MIDI channel and CHAN page (P4-015), RST (P4-018), SOL/FIL/ROL (P6-003), PRESETS encoder (P7-002), hard-tap ACCENT (needs key force; P4-014/P4-017). Must exist before HW-1 and were nobody's: CHN key (D#4) entry to CHAN, PERF ALGO encoder, PAST THE END / 64 STEPS = MAX, stopped step sounds once, preview toast. Filed as P4-032 (depends P4-013, P4-015) and added to P4-018's depends_on so it precedes P4-020 and HW-1 (P4-021).

### 2026-11-13 executor (Sonnet 5.5), review fixes
- F1: `uisq_do_remove` clears the channel's preview bit (pad[0]&0x80) and EDIT undo restores it; test "removed channels have the preview off".
- F2: removals form a stack (up to 4: channel, mute, preview); each EDIT pops one. A dice, a page add, or an EDIT that undoes something else empties it.
- F3: `uisq_key_up` ends the M-key hint (held_m, hold hint) before the non-PERF return; test added.
- F4: for P4-014: replace `uisq_pg_note_edit` (stand-in that ends with the opening key) and own the NOTE_EDIT session end.
- `sh tools/check.sh` -> CHECK PASS; ui_sq_perf_test 0 failures.

### 2026-11-13 review 2 (sonnet)
Verdict: FAIL, back to todo. F1 (preview bit cleared, restored by undo), F3 (M hint ends before the non-PERF return) and the stack semantics of F2 are correct and tested. `sh tools/check.sh` = CHECK PASS; `RAM .data+.bss 29192 B of 73728` (headroom 44536 B, OK).
- [ ] F5 (must fix) ui_sq.c:897 uisq_do_remove pushes without a bound, and the SELECT add (ui_sq.c:871) does not empty the stack. Remove CH2 (stack 1), SELECT past the last re-adds CH2 (stack still 1), remove again (2) ... the 5th push writes uisq_gone_ch/muted/prv[4], past the SQ_NCH=4 arrays, corrupting adjacent statics. Fix: set `uisq_gone_n = 0` where a channel is added by SELECT (a stale entry would also restore the wrong state), and guard the push with `if (uisq_gone_n < SQ_NCH)`. Add a test: remove, add, repeated 6 times, then EDIT, no overflow and uisq_gone_n <= 4.
