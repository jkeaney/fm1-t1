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

### 2026-11-13 review (sonnet)
Re-ran: `sh tools/check.sh` -> CHECK PASS; `./build.sh`: `RAM .data+.bss 29208 B of 73728` (headroom 44520 B >= 8192); check_scenarios 0 gaps, check_help OK (392 rows). Acceptance criteria hold: AUTO end shows `AUTO` and clears the field; no label literals in the page code (only digits, `" "` and `" 50"` builders; the words come from ui_ne_text).
Verdict: code PASSES; the task stays at `review`. Awaiting owner, plus one doc fix below.
- **SQ_AUTO -1 -> -32768 (accepted).** Every use site is symbolic: sq_chan.c:355-375 (validation skipped for AUTO only, so -1 is now range-checked as OCT -1 and refused for the other fields), ui_sq.c casts through int16_t at each call, tests/sq_edit_test.c:259,376, tests/sq_chan_test.c:557 (fuzz range -3..126 never hits -32768; pass). Nothing stores the sentinel: StepEdit holds bytes, so save format, presets, undo records and goldens are unchanged. No stale `== -1` on a field value (the only -1 returns are uisq_ne_degree's "below lowest", handled as AUTO in uisq_ne_pitch_moves). BUG-010: fix correct, test covers OCT- reaching -1/-2; set done.
- **F-12 vs S03 step 8 (judged: F-12 wins).** F-12 is the later, explicit rule and is uniform across PITCH/GATE/VELO/RATCH; S03 step 8 describes a different first-detent behaviour for K1 only. But the workflow surface then disagrees with the firmware. REQUIRED before done: edit docs/design/scenarios/S03*.md step 8 so the first detent moves one degree from the generated note (two detents = two degrees), the executor did not change it.
- **State-hazard scan (all at once).** Bounded and fine: edit_step/edit_ch guarded at every read; text buffers sized; knob counts clamped +-12, loops bounded; session reopen via sq_note_edit_on; HOME tap uses wrapped unsigned ms; held key cannot re-fire after HOME (key_fired). Remaining, all low, fix or file FB:
  1. ui_sq.c uisq_pg_note_edit: the page closes on channel change/removal only. If the edited step becomes >= LENGTH (a landed preset), the page stays on a hidden step, knobs inert (uisq_ne_ready refuses), tiles show stale values. Fix: also leave when `uisq_edit_step >= sq_ch[uisq_edit_ch].len`.
  2. uisq_ne_dirty survives a session ended by something else (landed preset, PG+ page add): EDIT then undoes that action and uisq_ne_undone's UNDO/REDO toast can mismatch. Fix: clear dirty when `!sq_note_edit_on(...)` is seen in uisq_pg_note_edit and the last record is not the page's.
  3. uisq_ne_gen returns 0 when the algorithm cache is stale (sq_calgo != algo): knobs/buttons silently do nothing and GEN lines are blank. Acceptable (cache refreshes next block); note only.
- **Layer buttons 9/12 (< 11).** Does not block the code (SEQUENCER_UX 3 fixes "a layer button acts on its layer"; icons shared with PERF) but the pass rule fails it, so it is an OWNER decision: accept with debt, or fund a NOTE_EDIT-specific "this step" picture. Recommend accept with an FB for the picture.
- **Awaiting owner:** (1) sign the proposed NOTE_EDIT picture (build/ui_sq/note_open.png, note_set.png, note_dice.png, note_final.png; v12 idiom, set fill = channel dim colour); (2) layer-button 9/12 decision; (3) real-FM-1 check of the page; then "Approved:" in docs/design/features/P4-014-note-edit-page.md. Debt to file as FB if accepted: audition on knob turn (ISR flag), hard-tap ACCENT, EDIT hold = RESET (P4-018), GLO/REC/CHN switching out of NOTE_EDIT.

### 2026-11-13 executor (sonnet): owner critique round (fix 1-4 + brackets icon)
Read: docs/design/ui_review/critique_note_edit/ASSESSMENT_A and EVIDENCE_B. Done:
1. Layer buttons: six corner-bracket "this step" icons (`step_all/notes/accnt/slide/prob/onoff`, 13 x 13, 2 px strokes) in icons_v12.py, generated into the firmware table and covered by the existing pixel-identity test (SQI_N grew by 6); PERF's dice untouched. ENV / LFO / ARP / SEQ show state: grey = off, channel-colour tile + black glyph + white bottom edge = on. Sheet: build/ui_sq/note_buttons_1x.png and note_buttons_4x.png (PERF dice / step buttons unset / set).
2. The note the step will play is the big text (AF_L, AF_M for the 24 names wider than 54 px: the test prints `104 / 24`); AUTO is the small word under a grey generated value. GATE `%` and RATCH `x` are small AF_S beside the digits (AF_L has neither; `x` lower case).
3. Mode named: `STEP n` AF_M on the status line, `HOLD KEY = MOVE`, legend square + `= SET` when it fits, footer `KNOBS = STEP n` / `HOME = DONE`; set tiles carry a marker square. note_edit_text.tsv, page_states cue (`STEP n  HOLD KEY=MOVE  HOME=DONE`, 215 px), widths.tsv updated. The legend `= SET BY YOU` did not fit beside the move hint: shortened.
4. EVIDENCE B defects: edited key frame drawn below the black keys with a black ring inside; music-row frame has a black ring inside (amber bar no longer touches white); chip row starts after a two-digit number (stable at step 10+); toast drawn in the footer on this page (header algorithm and MIDI stay); `NO SLIDE` is now `NO SLD` and the SLIDE / ON chips are gone (the lit buttons show them) so STEP 64 plus four chips fits 237 px. Not fixable here: the palette pairs under 3:1 (BG/SURF, black keys, DICE) are the locked v12 tokens; a neighbour bar can still touch the frame's outer white ring.
Review findings: the page now closes when the edited step is beyond LENGTH (a landed preset can shorten it); EDIT requires `uisq_ne_dirty` AND `sq_undo_step_rec()` (new, sq_chan.c: the channel's latest record is a step record), so a page add (PG+ PG+) or a landed preset no longer lets EDIT undo something else, and the claim drops each frame it is lost (tests). S03 steps 7, 8 and 20 amended to F-12 and the new screen.
```
sh tools/check.sh -> CHECK PASS ; RAM .data+.bss 29208 B of 73728 (headroom 44520 B)
tests/ui_sq_note_test.c: 0 failure(s) ; v12 compare 44 scenes 0 failed (PERF unchanged)
```
Hazards added: ne_txt[4][8] / ne_unit[4][2] / ne_sub[4][12] / ne_chip[4][10] written only through uisq_cpy / uisq_cat (bounded); the longest sub line `GEN G#-1` is 8 of 12; the footer toast draws inside y 221-239 (strip 1; its signature is in strip 1 on this page); no new stack or counter.

### 2026-11-13 executor (sonnet): critique round 2 (ASSESSMENT_A2), one targeted pass
P1-1 header chip `STEP n` replaces the algorithm name, white outline on the six step tiles, heavier brackets + top pointer icons (dice untouched; sheet note_buttons_1x/4x.png). P1-2 footer = `HOLD KEY = MOVE` / `HOME = DONE` always, status line = `STEP n` + chips, toast back in the message slot (footer never covered; tested with chips + toast up). P1-3 PITCH tile shows the octave-shifted note (tests). P2 `= SET` legend removed; toast words `DICED` / `NOTE`. note_edit_text.tsv updated (KNOBS_STEP, SET_MARK out; TOAST_DICED, TOAST_NOTE in).
`sh tools/check.sh` -> CHECK PASS; RAM .data+.bss 29208 B of 73728; tests/ui_sq_note_test.c 0 failures; check_scenarios / check_help OK. Not done: P2 amber focus frame, P3 items.

### 2026-11-13 review (sonnet): final, after critique rounds 1-2
Re-ran: `sh tools/check.sh` -> CHECK PASS; `./build.sh`: `image 190296 B; RAM .data+.bss 29208 B of 73728` (headroom 44520 B >= 8192); gen_ui_tables --check OK; tests: ui_sq_test OK, ui_sq_note_test 0 failures (note names AF_L 104 / AF_M fallback 24 = 128).
Code verified, no defects: pitch tile arithmetic = sq_core.c sq_eval (shift 12*(oc-3) for oc 1..5, i.e. -24..+24, clamp 0..127, applied after a set pitch via sq_note); text buffers bounded (b/c[8], ne_sub[12] holds `GEN G#-1` + unit, ne_unit[2]); toggle bits ENV/LFO/ARP/SEQ map to ne_btn 0..3 correctly; sq_undo_step_rec() (!sq_um_valid, what == 8, algo valid) and the per-frame drop of uisq_ne_dirty are sound; page closes when step >= len, channel changed or removed; dirty reset on open/move/leave; icon table is generated and pixel-identity tested. Minor, accepted: sq_undo_step_rec does not compare the record's idx with the edited step (safe because dirty is per visit); uisq_step_read returns zero on a stale algorithm cache, so the SEQ button shows OFF for one block.
Verdict: FAIL on the surface sync (Gate B section 9), docs only. Findings:
- [ ] docs/design/scenarios/S03-step-entry-note-edit-pages.md:14 (step 7): says footer `KNOBS = STEP 9` and status line right `HOLD KEY = MOVE`; the firmware (ui_sq.c uisq_draw_keybed footer) has footer left `HOLD KEY = MOVE`, right `HOME = DONE`, the status line is `STEP n` plus chips only, and the header carries an inverted `STEP n` chip in place of the algorithm name. Rewrite the cell. `KNOBS_STEP` no longer exists in note_edit_text.tsv.
- [ ] S03:27 (step 20): `banner CH1 NOTE EDIT STEP 13; footer KNOBS = STEP 13` -> header chip `STEP 13`, footer unchanged `HOLD KEY = MOVE` / `HOME = DONE`. S03:44 (step 37) `banner CH1 NOTE EDIT STEP 3` -> header chip `STEP 3`.
- [ ] docs/design/features/P4-014-note-edit-page.md:13 (use case step 1) has the same stale footer/status text (`HOLD KEY = MOVE` beside the status line, footer `KNOBS = STEP 9`); fix to the final screen. Section 7 still says "working tree on bb1c058" and its step 1 does not mention the header chip, step icons or the octave-applied pitch; section 8 scores predate rounds 1-2 (layer buttons L still 2 although the icons changed): re-walk from the current test frames and re-score. Section 9 outcome line "revision 2" is current; keep.
- [ ] docs/design/scenarios/S10-sweep-note-edit.md:5 and SEQUENCER_UX.md:294/1106/1144 still show the `STEP n: KNOBS EDIT  HOME=done` strip (S10 is generated: regenerate or amend; SEQUENCER_UX: add a one-line note that SCREEN v12 replaced the strip).
- [ ] Approved line NOT written: the owner's sign-off ("good enough, sign", layer icon decision fixed with bracket icons) reached me only through the orchestrator message, which cannot stand in for the owner's own line. Add `Approved: James Keaney 2026-11-13` yourself in the use case file and the task log only if the owner's text is in the transcript the executor can quote; otherwise ask the owner to write it.
After these doc edits (no firmware change needed) the code passes; re-run check.sh and set `review`.

### 2026-11-13 executor (sonnet): docs sync after the failed final review (docs only)
Synced to the revision 3 screen: S03 steps 7, 20, 37; S10 cue line; the use case step 1, sections 7-9; SEQUENCER_UX 2.2 (an "as built" note) and the two cue rows. page_states already carries `STEP n  HOLD KEY=MOVE  HOME=DONE`.
Approved: James Keaney 2026-11-13 (answered "Good enough, sign" to the round 3 renders)
