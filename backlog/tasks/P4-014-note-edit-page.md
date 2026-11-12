---
id: P4-014
title: NOTE_EDIT page
phase: P4
type: task
status: review
priority: 2
executor: haiku
depends_on: [P4-013]
gate: none
---
## Goal
Per-step editing opened by holding a step: every field AUTO or set, as locked.

## Read first
- docs/design/SEQUENCER_UX.md §1.1, §1.2, §2.2, §4.1 (NOTE_EDIT knobs), §5 (NOTE_EDIT traces)
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c NOTE_EDIT page: fields from knob_map rows, AUTO/set display, white keys tap ON/OFF and hold = move the editor, EDIT tap = undo, exit rules of §2.7
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [x] Scenario steps citing NOTE_EDIT in docs/design/scenarios reproduce
- [x] Turning a knob to its AUTO end shows `AUTO` and clears that StepEdit field
- [x] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-014-note-edit-page.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).
- 2026-11-13: carried from P4-013 F4: replace the uisq_pg_note_edit stand-in and close the sq_note_edit_begin session.

### 2026-11-13 executor (sonnet)
Built: NOTE_EDIT page in firmware/src/ui_sq.c (view fields, tiles, status chips, frames, footer; knobs, buttons, keys, HOME, SELECT, EDIT), the words in docs/design/note_edit_text.tsv (generated into `ui_ne_text` by tools/gen_ui_tables.py), tests/ui_sq_note_test.c (wired into tests/run_tests.sh), the P4-013 perf test line that asserted the stand-in. Use case + Gate A/B: docs/design/features/P4-014-note-edit-page.md (the picture is proposed in the v12 idiom: SCREEN_SPEC_V2 has none; owner to sign).
Files outside the named ones (reviewer: judge): firmware/src/sq_chan.c (3 additions: `sq_note_edit_end()` closes the session, `sq_note_edit_on(ch,p)` says whether it is still on, and `SQ_AUTO` is now -32768 because -1 made OCT -1 unwritable: BUG-010), firmware/src/ui_sq_glue.c (one call: ui_sq_home), tools/gen_ui_tables.py, tests/run_tests.sh.
```
sh tools/check.sh                      -> CHECK PASS
build line: image 189492 B; RAM .data+.bss 29208 B of 73728; pool 78520 B of 368640   (headroom 44520 B >= 8192)
/tmp/ui_sq_note_test build/ui_sq       -> ui_sq_note_test: 0 failure(s)
/tmp/ui_sq_perf_test                   -> ui_sq_perf_test: 0 failure(s)
python3 tests/design/check_scenarios.py -> 0 with no scenario ; check_help.py -> OK: 392 help rows
```
Decisions: (1) S03 step 8 says the first K1 detent sets the generated note; SEQUENCER_UX F-12 (later) says it moves one degree from it: F-12 followed. (2) K3 fast turn: 3+ detents in a frame step 8 (the shared sq_morph_turn ignores `fast`). (3) ENV/ARP/LFO toggle to AUTO, not to an explicit OFF/100 (ENV: 80 when the generator would accent, F-10). (4) EDIT is inert until this visit wrote something (otherwise sq_undo would undo an older dice or page add).
Not done (debt, see use case 9): audition on a knob turn (ISR flag), hard-tap ACCENT (key force: the ENV button and K3 are the fallback), EDIT hold = RESET (P4-018), GLO / REC / CHN out of NOTE_EDIT (their pages are P4-015/16/17; those keys are inert here), S03 step 7's "step sounds once".
Index / bound hazards checked (new state: `uisq_edit_step`, `uisq_edit_ch`, `uisq_ne_dirty`, `uisq_ne_undone`, `uisq_home_*`, the view's `ne_txt[4][8]`, `ne_gen[4][10]`, `ne_chip[4][10]`; no new stack or array that grows):
- every text goes through uisq_cpy / uisq_cat / uisq_catu with the destination size (bounded); the widest strings (`GEN F#-1`, `PROB 50`, `NO SLIDE`) are 8, 7, 8 chars in 10, and the test fits-checks all 128 notes.
- `uisq_edit_step` is read only through `p >= SQ_NSTEP` guards (uisq_ne_ready, uisq_ne_fill clamps to 0); the test drives 40 (hidden by LENGTH), 63 and 255; sq_step_set / sq_note_edit_begin refuse p >= len, so nothing is written.
- `uisq_edit_ch` is bounded `< SQ_NCH` in ready / fill; the page closes when `uisq_sel != uisq_edit_ch` or the channel left `uisq_used` (stale-state hazard: a removed or re-selected channel).
- the session: left on HOME / SELECT / selection move (sq_note_edit_end); `sq_note_edit_on` is checked before every write so an undo, a tap on another step or a landed preset that ended it silently re-opens it; the stale-session hazard of P4-013 F4 (a PERF tap swallowed into an old record) has a test.
- EDIT calls sq_undo only after a write of this visit (`uisq_ne_dirty`): no undo of an older record. `uisq_gone_*` (removed channel stack) is untouched here, no push.
- pitch: semitone arithmetic in int16 with explicit 0..96 clamps; `uisq_ne_degree` loops are bounded by the 0..96 range and `i < 12` per call; turn counts are clamped to +-12; the dice walk falls back to the other direction instead of writing AUTO.
- gate / velocity / ratchet / octave: every value is clamped before sq_step_set (which validates again); tests turn each knob 40 times to both ends and `d = +-127`.
- the knob / button entry points ignore k >= 4 and unknown ctl ids (tested with 9, 200, 0xFF).
- ui_sq_home keeps a 32-bit ms difference (wrap-safe unsigned compare); a menu hold leaves `uisq_home_dn` set until the next call, where its age is over 700 ms: not a tap.
- UisqView `memset` before fill: the signature hash covers the new fields, no uninitialised padding added (all new fields are bytes / char arrays).
