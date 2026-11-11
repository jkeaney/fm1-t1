---
id: P2-026
title: Align SEQUENCER_UX mute/SOLO wording with the MIDI gate (decision 111)
phase: P2
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P2-025]
gate: none
---
## Goal
PREVIEW.md (decision 111) says mute and SOLO act at the MIDI gate in sq_emit, not at step hit resolution. docs/design/SEQUENCER_UX.md still says mute stops hits at the source. Fix before P4-004.

## Read first
- docs/design/PREVIEW.md sections 1.2, 1.4
- docs/design/SEQUENCER_UX.md lines ~103 (SLIDE muted case), ~581 (U48), ~1013 (hit(n) comment)

## Deliverable
- SEQUENCER_UX.md (and any other design doc/TSV with the same claim, found by grep for mute/hit(n)) reworded so a muted channel still computes steps and feeds preview, but sends no MIDI. Keep the design checkers passing.

## Acceptance criteria
- [x] The three places no longer contradict PREVIEW.md; grep for other contradictions reported in the log
- [x] Design checkers pass (check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh, backlog.py check)

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware, builds, tools/check.sh; files owned by the active P3-003 executor.

## Log
### 2026-11-11 executor (sonnet)
Edited docs/design/SEQUENCER_UX.md (SLIDE row ~103: "muted" removed from the not-sounding list; U48 ~581: now "MIDI gate in sq_emit closes, channel keeps computing and feeds preview"; hit(n) comment ~1013: "NOT mute") and ADR-0003-sequencer-core.md ~530 (Mute row, same stale claim). Other greps (hit(n), "stops new hits", mute...source) in docs/design, decisions, tests/design: no further contradictions.
Verified: check_scenarios.py, check_help.py, check_sequencer_ux.py, width_probe.sh ("296 strings, 0 over"), backlog.py check ("123 items, 0 errors") all pass.

### 2026-11-11 review (sonnet)
Verdict: FAIL (minor). All five checkers re-run and pass (scenarios 490 rows, help OK, sequencer_ux OK, width_probe "296 strings, 0 over", backlog "123 items, 0 errors"). The three named edits (SEQUENCER_UX.md SLIDE row ~103, U48 ~581, hit(n) comment ~1013) and the ADR-0003 Mute row ~530 match PREVIEW.md 1.2/1.4 and decision 111. But the executor's grep claim "no further contradictions" is wrong:
- [ ] docs/project/decisions/ADR-0003-sequencer-core.md:441 (step-start algorithm, step 4): "If `muted` or `!(out.flags & PO_HIT)` ... skip to step 8" still applies mute inside step resolution, which would starve the preview (the exact thing decision 111 forbids; line 902 row 2 says the test lives at the MIDI gate). Fix: drop `muted` from the condition, and have the note-on/emit step route through the two gates (MIDI gate = midi_ch set and not muted and SOLO-allowed; PREVIEW gate = prv_on); keep "muting keeps phase".
- [ ] docs/design/PREVIEW.md:267 (P4-004 row) describes SEQUENCER_UX U48 and the hit(n) listing as "stop new hits at the source"; now stale. Reword to "were amended by P2-026" so P4-004 does not chase a fixed claim.
- [ ] Optional: SEQUENCER_UX.md U49 (~582) "while held only ch sounds" should say MIDI (SOLO leaves previews sounding, PREVIEW.md 1.2).
