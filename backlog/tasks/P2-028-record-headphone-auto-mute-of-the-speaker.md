---
id: P2-028
title: Record that headphones auto-mute the speaker (decision 113)
phase: P2
type: task
status: done
priority: 3
executor: haiku
depends_on: [P2-027]
gate: none
---
## Goal
Decision 113: on the owner's FM-1, plugging in headphones switches the speaker off automatically [V]. Update the docs that treated this as an open HW-1 check.

## Read first
- docs/project/OWNER_QUESTIONS.md decision 113
- docs/design/PREVIEW.md (grep "headphone", "HW-1"), CLAUDE.md hardware facts line "Audio out", backlog tasks mentioning the speaker-mute check (grep -rn "headphone" backlog docs)

## Deliverable
- PREVIEW.md: state that preview plays on the speaker, or the headphones when plugged in (hardware auto-switch, verified on the owner's unit 2026-11-11); remove the open check.
- CLAUDE.md "Audio out" line: add "headphones auto-mute the speaker (verified 2026-11-11)".
- Any HW-1 task/doc wording that lists this as a check to do: mark it answered.

## Acceptance criteria
- [x] grep -rn "speaker mutes when headphones" docs backlog CLAUDE.md shows only answered/historical text
- [x] python3 tools/backlog.py check passes; python3 tools/check_sequencer_ux.py passes

## Verification
```sh
python3 tools/backlog.py check; python3 tools/check_sequencer_ux.py; python3 tests/design/check_help.py
```

## Out of scope
- Firmware, builds; files owned by the active P3-003 executor.

## Log

### 2026-11-11 executor (claude-haiku-5-5)
Edits:
- docs/design/PREVIEW.md 1.2 paragraph: "HW-1 check (the one thing not known)" replaced with the settled decision 113 statement (speaker off when headphones plugged in, no open check).
- docs/design/PREVIEW.md 1.2 "Hardware limits" sentence: preview goes to speaker, or headphones when plugged in.
- docs/design/PREVIEW.md section 9 row: "HW-1 checks only whether the speaker mutes on plug-in" replaced with decision 113 answered.
- CLAUDE.md "Audio out" line: added "headphones auto-mute the speaker (verified 2026-11-11)".
- Remaining "whether the speaker mutes" text is only in P2-027 (task and log, historical) and this task.

Verification:
```
$ grep -rn "speaker mutes when headphones\|speaker mutes on plug-in\|whether the speaker mutes" docs backlog CLAUDE.md
  -> only P2-027 (lines 20, 39, historical) and P2-028 itself
$ python3 tools/backlog.py check
125 items, 0 errors          rc=0
$ python3 tools/check_sequencer_ux.py
pages=10 button-rows=282 knob-rows=122 key-rows=359 control-rows=490 scored=79 exceptions=3
OK
$ python3 tests/design/check_help.py
OK: 392 help rows, 2695 strings measured
```
Not run: `sh tools/check.sh` (docs only, no code changed). No firmware touched.

### 2026-11-11 review (sonnet)
Verdict: PASS (gate none). Docs only, no firmware changed, so no build and no RAM line (P3-003 executor active).
- Re-ran: backlog.py check -> "125 items, 0 errors"; tools/check_sequencer_ux.py -> OK (scored=79, exceptions=3); tests/design/check_help.py -> OK (392 help rows).
- PREVIEW.md 1.2 (line 63-69) and section 9 row (line 335) state decision 113 correctly (speaker off when headphones plugged in, never both, verified 2026-11-11); no open HW-1 check remains. CLAUDE.md line 58 correct. OWNER_QUESTIONS.md 113 consistent.
- Grep of docs, backlog and CLAUDE.md: remaining "whether the speaker mutes" wording is only historical (P2-027 task and log). P2-025 log line 58 is historical.
- Fixed the acceptance line's wrong checker path text in this task.
- Note (non-blocking): PREVIEW.md line 9 says nothing is [V] on hardware; it concerns the numbers, and decision 113 is a separate hardware fact.
