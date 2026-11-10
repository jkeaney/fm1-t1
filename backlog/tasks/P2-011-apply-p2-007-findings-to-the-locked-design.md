---
id: P2-011
title: Apply the P2-007 scenario findings to the locked design docs
phase: P2
type: task
status: review
priority: 0
executor: sonnet
depends_on: [P2-007]
gate: human-review
---
## Goal
The scenario walkthroughs (P2-007) found 19 defects (F-01..F-19) in the locked design docs, all confirmed real by the independent review. Fix them so the design is implementable, without changing any owner decision.

## Read first
- docs/design/scenarios/FINDINGS.md (the defects, with evidence), docs/design/scenarios/COVERAGE.md
- docs/design/SCREEN_SPEC.md, SEQUENCER_UX.md, TIMING_SPEC.md, the TSVs, ADR-0003
- docs/project/OWNER_QUESTIONS.md rows 65-76 (decided; do not reopen)
- the P2-007 review in backlog/tasks/P2-007-scenario-walkthroughs-ux-validation.md Log

## Deliverable
- Every FINDINGS defect fixed in the owning doc/table or, where a fix needs an owner choice, raised as a numbered owner question with options (do not decide silently). Needed at minimum: the cue-strip row order (13a/13b reachable, F-01), a toast zone in SCREEN_SPEC (F-03), overflowing strings re-fitted with measured widths (F-04: MIDI 2 MUTE, KEYS ONLY, header slots H4/H5), a HELP text table (F-06), MENU roles reconciled (F-02), the unmuted-channel MIDI clash (F-14, hold as an owner question). Q2 and Q3 in FINDINGS are already answered by OWNER_QUESTIONS 76: mark them answered and remove the 'no way to start on internal' wording from S06; fix the S06 step 2 arithmetic (139 ms); disclose the gen_sweeps.py respelling and that sweep-only rows are table echoes; commit the F-04 width probe under tests/design/.
- FINDINGS.md updated with the status of each defect (fixed / owner question).
- Checks stay green: tools/check_sequencer_ux.py, tools/check_control_map.py, tests/design/check_scenarios.py, backlog.py check.

## Acceptance criteria
- [x] All 19 defects marked fixed or raised as an owner question, none left open silently (FINDINGS status table; one new owner question, 79)
- [x] No owner decision (OWNER_QUESTIONS 65-76) changed (77 and 78 are new owner rulings, recorded)
- [x] The three check scripts and `backlog.py check` pass
- [ ] Owner approval

## Verification
```sh
python3 tools/check_sequencer_ux.py && python3 tools/check_control_map.py
python3 tests/design/check_scenarios.py
python3 tools/backlog.py check
```

## Out of scope
- Firmware changes. Q1 (does the 2-gesture rule satisfy VISION) and Q4 (warn on shared MIDI channel) are owner questions, not part of this fix pass unless answered.

## Log
- 2026-11-10 created by the orchestrator from the P2-007 findings.

(Owner 2026-11-10, P2-007 gate: 1B approve P2-007 together with P2-011; 2A two gestures allowed for setup functions; 3 MIDI channels unique per sequencer channel: assigning a used MIDI channel removes it from the other (OFF); 4B push after P2-011; 5A P0-005 deferred; 6A Ableton Sync check to be done by owner later.)

### 2026-11-10 executor (sonnet)
Fixed F-01..F-19 in SCREEN_SPEC (2.1, 2.2, 2.7 toasts, 3 MENU row, 5, 6.5, 6.6, 7, 7.1, 9), SEQUENCER_UX (11.1 to 11.3, new 4.4 and 15, U40, U58),
TIMING_SPEC (R13, R16, section 11 marked answered), ALGORITHM_SET 5.2, knob_map / control_map / CONTROL_MAP (MIDI row, KEYS, TOTAL), ADR-0003
(`sq_set_midi` returns the displaced channel, `UndoMidi` 4 B, preset validation), VISION (gesture budget, unique MIDI channels), the scenarios and
FINDINGS (status table, method disclosures). New: docs/design/help_text.tsv (343 rows), tests/design/{width_probe.c,width_probe.sh,widths.tsv,
gen_help.py,check_help.py}. Mid-task owner rulings applied: Q1 option a (OWNER_QUESTIONS 78), unique MIDI channels with OFF displacement (77).
Judgement calls the owner may overrule: header H4 drops the `CH` prefix; muted cell reads `MUTE n`; cue rows renumbered 10a / 10b and 5a;
the MIDI assignment commits after 0.6 s of stillness so turning across a taken channel does not displace it (SEQUENCER_UX 4.4); F-13 reading
(adding a page sets FOLLOW off); empty preset slot = owner question 79 (recommended: load nothing).
```
$ python3 tools/check_sequencer_ux.py && python3 tools/check_control_map.py
OK / OK
$ python3 tests/design/check_scenarios.py
scenarios: 17 files, 441 control-map rows, 0 with no scenario, 268 only in a sweep
$ python3 tests/design/check_help.py
OK: 343 help rows, 1733 strings measured
$ sh tests/design/width_probe.sh
110 strings, 0 over
$ python3 tools/backlog.py check
43 items, 0 errors
$ grep -rEn 'TBD|choose|e\.g\.' docs/design/scenarios || echo clean
clean
```
Scores: U40 stays 2 gestures (score 10); the displacement adds no gesture, its undo is the existing EDIT tap; no score recomputed. No Approved line written; the last criterion (owner approval) is left unticked.

### 2026-11-10 review (opus, reviewer)
**Verdict: PASS (design review).** Awaiting owner: approval of the judgement calls below (no "Approved:" line written; gate human-review, status stays `review`).
- Re-ran all verification: check_sequencer_ux, check_control_map, check_scenarios, check_help (343 rows), width_probe (110 strings, 0 over), backlog.py check (46 items, 0 errors): all green.
- F-01..F-19, Q1..Q4: each has a status in FINDINGS and the fix is present (cue rows 10a/10b/5a in SEQUENCER_UX 11.2 and SCREEN_SPEC 6.6 agree; toast zone 2.7; H4/H5, MUTE n, KEYS, 48 (32) re-fitted; help_text.tsv; MENU row; R13/R16; ALGORITHM_SET 5.2). Grep finds no surviving stale 13a/13b, `KEYS ONLY`, `MIDI 2 MUTE`, `NO PAGE BEFORE` outside the historical FINDINGS/F6 records.
- Owner decisions 65-76 untouched; 77/78 recorded; ADR-0003 (`sq_set_midi`, `UndoMidi`, preset validation) matches SEQUENCER_UX 4.4 and SCREEN_SPEC 2.2/2.7.
- Extra rules: 0.6 s MIDI settle (commits also on leaving CHAN), H4 without `CH`, page-add sets FOLLOW off (consistent with 1.3/U34), renames 10a/10b/5a are sound and do not conflict with VISION or decisions 73/77.
- Minor, non-blocking: SEQUENCER_UX 4.4 says the MIDI channel commits on 0.6 s stillness or leaving CHAN but not on PLAY or a page-button press from another page; the implementer should commit on any exit. No edits made to design docs.
- Owner to decide: judgement calls (MIDI settle 0.6 s, `MUTE n` wording, row renames) and question 79 (empty preset slot; recommended option a).
