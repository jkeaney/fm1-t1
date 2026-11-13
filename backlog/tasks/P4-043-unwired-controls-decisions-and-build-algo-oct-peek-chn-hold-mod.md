---
id: P4-043
title: Unwired controls: decisions, then ALGO on NOTE_EDIT/SET, OCT peek, preview pick, CHN hold, MOD
phase: P4
type: task
status: todo
priority: 2
executor: human
depends_on: [P4-021]
gate: human-review
---
## Goal
Close the controls that have no owner: ask the owner whether RIG and MOD output ship in v1, then build what is in v1 and park the rest explicitly, so no panel control is silently dead.

## Read first
- /tmp/readiness_review.md section B (control wiring audit) and finding 8
- firmware/src/ui_sq.c ~1538 (ALGO accepted on CHAN, REC, PERF only), ~2144 (CHN hold ignored), ~250 (`uisq_rig_row` returns 0)
- firmware/src/ui_sq_glue.c ~83 (OCT acts on release; no hold-to-peek)
- firmware/src/sq_prims.c ~141 (the MOD output hook nothing calls)
- docs/design/key_map.tsv, button_modes.tsv, knob_map.tsv
- docs/design/PREVIEW.md

## Deliverable
- STEP 1 (executor human, decision first): ask the owner and record in docs/project/OWNER_QUESTIONS.md: (a) does RIG ship in v1 (placeholder decision if not: CHN hold shows a 'RIG: v2' toast and the RIG rows are marked v2 in the tables); (b) is MOD output in v1 (EUCLID's LFO/SHAPE button does nothing audible without it)
- STEP 2 (build, sonnet, after the decisions): ALGO encoder on NOTE_EDIT and SET; OCT- / OCT+ hold = peek with tap acting as today; M-key hold + ALGO = preview-sound pick; CHN hold = RIG or the placeholder; MOD output hook called or its dice labelled v2, per the ruling
- Control tables updated to match; a task per remaining piece if the ruling makes it large (file it, do not grow this one)

## Acceptance criteria
- [ ] The owner's two rulings are in OWNER_QUESTIONS.md with numbers
- [ ] Every control named above either works (host test) or is marked v2 in the three tables and shows a toast
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)
- [ ] Owner approves the behaviour (human-review)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
python3 tools/check_control_map.py   # the pre-existing baseline must not grow
```

## UX gate
Producer-facing. Use cases to update: P4-013 (PERF), P4-014 (NOTE_EDIT), P4-016 (SET), P4-015 (CHAN); Gate A cited from the locked tables, Gate B after the build.

## Out of scope
- Building the RIG page itself (separate task if ruled in)
- Algorithm rows (P5)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Decision step first: the executor of step 1 is the owner via the orchestrator; step 2 may be reassigned to sonnet after the rulings.

- 2026-11-13 owner ruling (AskUserQuestion): MOD output IS in v1. RIG decision pending: owner asked for plain-language context (asked again).
