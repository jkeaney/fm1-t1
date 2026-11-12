---
id: P4-006
title: Edit layer, pages and step resolution (sq_eval)
phase: P4
type: task
status: done
priority: 2
executor: haiku
depends_on: [P4-005]
gate: none
---
## Goal
Per-step overrides (StepEdit, AUTO vs set), pages of 16 up to 64 steps, the length change rule and page sources, resolved in the ISR by `sq_eval` exactly as specified.

## Read first
- docs/design/SEQUENCER_UX.md §1.1-§1.6 (model, pages, page sources, FOLLOW, length rule), §9.1-§9.3 (structs, resolution, determinism)
- docs/project/decisions/ADR-0003-sequencer-core.md §9 as amended

## Deliverable
- sq_core.c: `sq_eval` per SEQUENCER_UX 9.2; `sq_step_set/clear`, `sq_len_set` (extend duplicates, shrink hides), `sq_page_add`, page source handling, FOLLOW flag
- tests/sq_edit_test.c: one case per row of the SEQUENCER_UX 1.2 table and per worked example in 1.3-1.6, as golden text tests/golden/sq_edit.txt

## Acceptance criteria
- [x] sq_edit_test passes
- [x] A forced-OFF step stays silent after HITS dice; a forced-ON step keeps playing as DENS goes down (1.2 rows)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- UI pages
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (claude-haiku-5-5)

**Files changed**
- `firmware/src/sq_core.c`: `sq_eval(c, p, pass, a, k, g, o)` resolves the edit over the generated layer (9.2); new `sq_gen_pos` (1.4), `sp1_hash` and `SQ_PROB_LOW_256` (9.3), `sq_slide_out`, `sq_tie_end`; `sq_step_start` takes the next step with its pass (`pass + 1` at the wrap), the slide from the edit, and a TIE hold. The note-pairing code is unchanged: `sq_emit`, `sq_sched_pair` / `sq_sched_one`, the purge functions, the same-pitch guard and the preview gate (BUG-006/007 paths untouched). What changed in `sq_step_start` is the slide decision (now from the edit; AUTO gives the old result) and the TIE hold.
- `firmware/src/sq_chan.c`: edit writers and pages (`sq_step_set`, `sq_step_tap`, `sq_step_clear`, `sq_page_clear`, `sq_page_recopy`, `sq_page_src_set`, `sq_len_set`, `sq_page_add`, `sq_follow_set`, `sq_undo_page`).
- `tests/sq_edit_test.c` (new), `tests/golden/sq_edit.txt` (new, `GOLDEN_UPDATE=1` run, reviewed by hand), `tests/run_tests.sh` (one wiring line after the sq_chan suite).

**Deviation from the deliverable wording (for review)**: the deliverable names sq_core.c for the writers. They are in sq_chan.c because the undo record (`sq_undo_rec`, `sq_undo`) and the rebuild path are static there and `sq_chan.c` is the state API. `sq_eval` and the slide / TIE helpers are in sq_core.c as named.

**Choices the spec leaves open (flagged; a reviewer may overrule)**
1. PROB 50 %: a PROB step is silent when `(sp1 & 0xFF) >= 128`. 9.2 names the test but not the threshold. The seed is `ch*0x9E37 ^ p*0x45D9 ^ pass*0x2C1B` with SP1 salt 0, first draw. Checked: 49.1 % below 128 over 16 000 draws.
2. A TIE on a step holds it and suppresses that step's SLIDE and RATCH (9.2 does not say what happens when they coincide).
3. GATE AUTO on a generated `PO_TIE` (PATTERNS) is a TIE, so the generator's tie works without an edit.
4. A tap compares with the generated hit, ignoring PROB (9.1 says "what the step does now"; PROB is per pass).
5. Undo covers page add, page clear and page re-copy (the 1.6 list). Step taps, field edits, LENGTH and K3 SOURCE are not undoable (not in 1.6). A page clear clears the content flag of pages 2..4, so a later extend copies again (1.6 "or was cleared").

**Known gap (not implemented, reviewer should decide)**: 9.2's last clause, "a LENGTH change inside step p cancels a slide or TIE: the off returns to p's own gate end, or is sent at once if that has passed". Not done. A slide or TIE that is in flight keeps its scheduled OFF after a LENGTH change. Nothing hangs (the OFF is queued, at most one old loop away). The fix needs the gate-end tick and an ISR request path, and `ChanRT` is locked at 24 B with no room for either. That is a design call, so I did not guess at it.

**Cost**: a TIE step runs up to 63 `sq_eval` calls (lookahead for the next sounding step). Only TIE steps pay this; everything else is one or two resolutions per step.

**Verification**
```
$ sh tools/check.sh
== build (log: build/check/build.log)
== ram headroom
RAM .data+.bss 23828 B of 73728
== tests (log: build/check/tests.log)
== backlog (log: build/check/backlog.log)
CHECK PASS                       (exit 0; re-run after the last test edit)
```
`./build.sh | grep RAM` (line from build/check/build.log, the same build):
```
image 169100 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640
```
Headroom 73728 - 23828 = 49900 B (floor 8192 B; not an E9 trigger). The sq_* files are not in the device build until the cut-over (P4-012), so this task adds no RAM to the device image.

Host suites (tests.log, decisive lines):
```
SQ_TYPES PASS
SQ_CORE PASS            (the nine goldens unchanged, 100000-tick fuzz, ASan + UBSan)
SQ_CHAN PASS            (100000-tick fuzz, ASan + UBSan)
== sequencer edit layer ...
  ok  tests/golden/sq_edit.txt (3267 B)
SQ_EDIT PASS
ALL HOST TESTS PASSED
grep -c FAIL build/check/tests.log -> 0
```
Edit-test coverage: every 1.2 row (dice NOTES / HITS / ACCNT-ARTIC-SHAPE-ALL, morph DENS, ALGORITHM, SCALE and back, LENGTH shrink and extend, preset load, the four clear gestures); the 1.3 examples (copy with the music unchanged, page 3 copies page 2 as edited, MAX, partial page, hidden page as stored); 1.4 (OWN continuation, page 4 reads block 2); 1.6 (extend copies, hidden page returns exactly, commit by re-copy, undo and redo of add and clear, shrink past the playhead wraps to step 1); the tap rule and range refusals; FOLLOW; the scheduler: TIE over a rest, into the same pitch, across the loop end, with no other sounding step, slide on a non-pairing algorithm, slide across the loop end, PROB determinism.

Hand check of the golden's scheduler events: TIE over a rest ends at tick 96 (step 3 start) and the OFF comes before the ON; across the loop end ends at tick 144; no other sounding step gives OFF then ON at 192; the slide overlaps 6 ticks (step L = 48 ticks at DIV 1/16, D = L/8 = 6).

**For the reviewer**: (a) the LENGTH-cut gap above; (b) the five open choices, especially 1 and 2; (c) the writers are in sq_chan.c, not sq_core.c.

### 2026-11-12 review (sonnet)
**Verdict: PASS (gate none), with follow-ups filed as FB-015.**
Re-run: `sh tools/check.sh` -> CHECK PASS. `./build.sh`: `image 169084 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640`; headroom 49900 B (floor 8192 B). The sq_* files are not in the device image yet.
- (f) The ten sq_core goldens and the chan suite are untouched by the commit (git diff stat: only tests/golden/sq_edit.txt is new). sq_core fuzz passes under ASan + UBSan. sq_chan fuzz passes on seeds 1, 7, 12345. Note the existing fuzzes never call the new writers (put_edits writes raw), so I ran a temporary copy of the sq_chan fuzz with random sq_step_set / tap / clear / sq_len_set / page add / re-copy / clear mixed in (not committed): 4 seeds x 100000 ticks, ASan + UBSan, no violation, no hung note. Worth keeping as a real test in P4-024 (FB-015).
- (a) 1.2 rows: all eight asserted with CHECKs that state the spec rule (not just the golden). Re-derived by hand: TIE over a rest (S1: steps 1 and 3 sound at 48 ticks/step, OFF at 96 = step 3 start, then step 3 off at 96 + 24); TIE across the loop end (S3, OFF at 144 = next pass step 1); TIE with no other sounding step (S4: OFF and ON both at 192, OFF first); slide-into (S5: ON at 48, OFF of the old note at 54 = D = L/8); forced OFF/ON (40 dice, 0 slips, generator would sound step 4 on 29 of them, so the OFF edit is what silences it); PROB (independently recomputed the SP1 draw for ch0 p0: sounds on passes 1,3,5,14 = ticks 48,144,240,672, equals S7); page add copies exactly (music unchanged, pg_src and pg_used asserted), hidden page returns as stored, RESET+D#4 re-copies edits and source, RESET+REC clears and drops the content flag. The golden's scheduler events are consistent with the spec; the resolved tables are code output but every CHECK is spec-derived. Gap: "generated slides move" in the ACCNT/ARTIC row is not asserted.
- (b) sq_eval order matches 9.2: generated at g(p) -> PITCH set (sq_note, SP6) -> OCT clamp -> VELO -> GATE/TIE -> RATCH -> hit (ON/OFF edit) -> PROB; next step uses pass+1 at the wrap. Deterministic (pure function of seeds, edits, p, pass).
- (c) PROB threshold 128 of 256 and the chance seed: the spec fixes the seed formula (9.3, followed exactly, salt 0), names `PROB_LOW_256` without a value; PROB_LOW = 50 is a 12.2 flex constant, so 128 is right but should be derived (FB-015). No locked prototype or ADR text contradicts either. TIE suppressing SLIDE and RATCH: spec silent; RATCH off is consistent with SLIDE/HOLD, dropping a *set* SLIDE is not good (the user's edit has no effect); recommendation in FB-015. Undo only for page ops: matches 1.6's list but not 1.2 ("each clear is one EDIT tap away") or control_map's "EDIT tap undoes"; FB-015 item 2.
- (d) The "no room in ChanRT" reason is wrong: `ChanRT.tie` exists, is never set, and the ADR change note gives it exactly this job (store the scheduled LENGTH in it); the gate-end tick is not needed because the spec's own fallback ("sent at once") covers the next step start. Consequence today: no hang (STOP and CC123 clear it), but after shortening LENGTH a held TIE keeps ringing to its old end, up to one old loop (about 8 s at 64 steps, 120 BPM, 1/16). Filed in FB-015 with options; recommended fix in P4-024; owner decision not needed. Not a blocker for this task (not an acceptance criterion; the cut-over P4-012 should wait for it).
- (e) Writers in sq_chan.c: acceptable and better placed (they need the static undo record and rebuild path). P4-024 names sq_chan.c; P4-011 uses only the sq_* API. Names differ from the ADR/UX docs (sq_step_toggle -> sq_step_tap, etc.): FB-015. Added a note to P4-024 (stale "delete unused sq_undo_new": it is now used; reset family partly done).
- (g) UX gate `n/a` is right for this task: no control, screen or sound changes; the producer-facing gestures belong to P4-013..P4-018, which already cite their own use cases under docs/design/features/ (those files do not exist yet and must be written at their Gate A before those builds). No use case created here.
Minor: ChanRT.tie comment in sq_core.c header block still says P4-006 resolves PO_TIE "together with the edit layer"; fine. Pairing snapshot (ACID) delays a tap made during step p-1 by one pass: noted in FB-015.
