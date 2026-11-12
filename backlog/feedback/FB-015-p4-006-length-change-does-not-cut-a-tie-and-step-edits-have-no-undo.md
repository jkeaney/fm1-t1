---
id: FB-015
title: P4-006 leaves two spec rules open: a LENGTH change does not cut a TIE, and step edits have no undo
phase: P4
type: feedback
status: done
priority: 1
executor: sonnet
depends_on: []
gate: none
session: 2026-11-12, review of P4-006, code reading plus host runs
---
## Goal
Close two places where sq_eval / the writers of P4-006 differ from the locked spec, before the P4-012 cut-over puts the edit layer in front of a producer. Triage into one task (recommended: fold into P4-024, which already owns the reset family and depends on P4-006) or drop with a reason.

## Read first
- docs/design/SEQUENCER_UX.md 1.2 (clear table: "one EDIT tap away from coming back"), 1.6 (LENGTH rule, Undo row), 9.2 (last clause of `slideout` and the TIE paragraph)
- docs/project/decisions/ADR-0003-sequencer-core.md change note P1-017 (the `tie` flag of ChanRT "closed at the next step start by a LENGTH change"; `sq_note_edit_begin`)
- firmware/src/sq_core.c `sq_step_start`, `sq_tie_end`; firmware/src/sq_chan.c `sq_step_set`, `sq_step_clear`, `sq_undo_page`, `sq_undo` (the `what & 8` single-step path already exists)

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## UX gate
n/a, triage of internal work; the resulting task states its own gate.

## Observation
**1. LENGTH change inside a TIE (9.2: "cut only by STOP or a LENGTH change while it holds").** The executor did not implement the cut and wrote that the locked 24 B ChanRT has no room for the gate-end tick. That reason does not hold:
- `ChanRT.tie` (offset 7) is already in the struct and is only ever cleared (`sq_rt_run`), never set. The ADR change note assigns it exactly this job. Storing the LENGTH the hold was scheduled under (1..64, 0 = none) in it needs no new byte; at each step start the ISR compares it with `sq_ch[c].len`.
- The gate-end tick is not needed. The step start after a LENGTH change is at or past the end of p's own gate (gate <= 100 % of one step), so the spec's own fallback ("sent at once if that has passed") applies: purge the queued TIE OFF (`sq_evq_purge_off`) and `sq_off_now`. Only a change inside the first part of step p could honour "p's own gate end", and one step of lateness there is inaudible.
- A slide in flight needs nothing: its OFF is at most S1 + D, one step away.
User-visible consequence today: nothing hangs (the OFF is queued, STOP and CC123 still clear it), but after the user shortens LENGTH the held note keeps ringing to its original end, up to one OLD loop (a 64-step pattern at 120 BPM, 1/16: about 8 s; slower at larger DIV) while the shortened loop restarts underneath it. It is the one control in the set that sounds wrong rather than merely late. It is not a hardware-only issue; a host test can show it (TIE on step 1 of 8, `sq_len_set(2)` during the hold, expect the OFF within one step).

**2. Step edits are not undoable.** 1.2 says every clear (one field, one step) is "one EDIT tap away from coming back", and control_map NOTE_EDIT rows say "EDIT tap undoes" for each field toggle. P4-006 records an UndoRec only for page add / page clear / page re-copy (1.6 lists only those). `sq_step_set`, `sq_step_tap` and `sq_step_clear` write nothing to the undo record although `sq_undo` already restores a single step (`what & 8`, `ed[0]`). `sq_note_edit_begin(ch,p)`, named in the ADR change note, exists nowhere and no backlog item owns it.

**3. Smaller points (decide in the same task; none blocks).**
- TIE and a set SLIDE on the same step: the code drops the SLIDE (the user's explicit edit has no effect), and also drops RATCH. 9.2 is silent. Recommended rule: a set SLIDE ON whose next step sounds wins (slide overlap, S1 + D), TIE otherwise; TIE always cancels RATCH (as SLIDE and HOLD already do). Add one line to 9.2.
- `SQ_PROB_LOW_256` is a hard-coded 128. 12.2 lists `PROB_LOW` 50 (%) as a flex constant; derive the threshold from it, `(PROB_LOW * 256 + 50) / 100` (= 128). The chance seed (`ch*0x9E37 ^ p*0x45D9 ^ pass*0x2C1B`, salt 0, first draw, low byte) matches 9.3 and gives 49.9 % over 65 536 draws (reviewer measured); no locked prototype defines it, so nothing conflicts.
- API names differ from ADR-0003 / SEQUENCER_UX: code has `sq_step_tap`, `sq_page_recopy`, `sq_len_set`, `sq_page_src_set`; the docs say `sq_step_toggle`, `sq_page_copy`, `sq_set_length`, `sq_set_source`. `sq_step_dice` is also unimplemented. Amend the docs to the code names (no UI task cites the old names).
- On AF_PAIRING algorithms (ACID) the pairing snapshot taken one step earlier overrides the hit and note of the next step, so a tap or an ON edit made during step p-1 is heard from the next pass, not at step p. Same one-step latency the old slide pairing had; note it in the P4-013 live checklist.

## Options and recommendation (owner decision needed: no)
- A (recommended): implement 1 with the `tie` byte as above and 2 with `sq_note_edit_begin` plus a `what = 8` record taken on the first write of a NOTE_EDIT session and on `sq_step_clear`, both in P4-024 (same files, it already depends on P4-006). Cost: about 25 lines and two directed tests; no struct change, no RAM.
- B: accept the shortfall for v1 and amend 9.2 and 1.2 to say so. Cheaper, but the cut is a one-line rule the owner locked and a held note across a LENGTH change will be heard on the TB-3.
- C: a separate small task. Same work, more overhead.

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
### 2026-11-12 filed by the P4-006 reviewer

- 2026-11-12 orchestrator triage: option A. Folded into P4-024 (tie cut on LENGTH change, step-edit undo, SLIDE-on-TIE rule, PROB_LOW derivation, ADR API-name amendments, P4-013 checklist line). No owner decision needed.
