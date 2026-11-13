---
id: P4-043
title: Unwired controls: ALGO on NOTE_EDIT/SET, OCT peek, preview sound pick (RIG and MOD moved to P4-045..050)
phase: P4
type: task
status: todo
priority: 5
executor: sonnet
depends_on: [P4-021]
gate: human-review
---
## Goal
Close the controls that have no owner so no panel control is silently dead. The RIG and MOD decisions are made (owner, 2026-11-13: both are IN v1) and their work is in the chain P4-045 (design), P4-046 (profile data), P4-047 (MOD engine), P4-048 (ROW 3 knobs), P4-049 (RIG page, hold CHN), P4-050 (HW-RIG session). This task keeps the other unwired controls.

## Read first
- /tmp/readiness_review.md section B (control wiring audit) and finding 8
- firmware/src/ui_sq.c ~1538 (ALGO accepted on CHAN, REC, PERF only)
- firmware/src/ui_sq_glue.c ~83 (OCT acts on release; no hold-to-peek)
- docs/design/key_map.tsv, button_modes.tsv, knob_map.tsv
- docs/design/PREVIEW.md

## Deliverable
- ALGO encoder on NOTE_EDIT and SET (knob_map says it changes the algorithm; code accepts CHAN, REC, PERF only)
- OCT- / OCT+ hold = peek, tap acting as today (on release)
- M-key hold + ALGO = preview-sound pick
- Control tables updated to match; a task per remaining piece if it turns out large (file it, do not grow this one)
- Decided and moved out (do NOT build here): CHN hold = RIG page -> P4-049; PERF ROW 3 knobs R1-R4 -> P4-048; MOD output (the `sq_prims.c` hook, EUCLID's LFO/SHAPE dice) -> P4-047; device profiles -> P4-046; design -> P4-045. If P4-045 changes the OCT row behaviour, follow its ADR-0008 for the peek rows.

## Acceptance criteria
- [ ] Every control named in the Deliverable either works (host test) or is marked v2 in the three tables and shows a toast
- [ ] The CHN hold and ROW 3 rows are NOT marked v2 here: they belong to P4-048 / P4-049 and stay as the tables have them until those tasks run
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
- The RIG page, ROW 3 knobs and MOD output (P4-045..P4-050)
- Algorithm rows (P5)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Decision step first: the executor of step 1 is the owner via the orchestrator; step 2 may be reassigned to sonnet after the rulings.

- 2026-11-13 owner ruling (AskUserQuestion): MOD output IS in v1. RIG decision pending: owner asked for plain-language context (asked again).
- 2026-11-13 owner ruling (AskUserQuestion), verbatim: (1) "MOD output is IN v1." (2) FULL RIG is in v1: "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."
- 2026-11-13 planner: both decisions made; executor changed to sonnet. RIG and MOD work removed from this task and filed as P4-045 (design), P4-046 (profile data), P4-047 (MOD engine), P4-048 (ROW 3), P4-049 (RIG page; the CHN-hold item now lives here), P4-050 (HW-RIG). Remaining here: ALGO on NOTE_EDIT/SET, OCT peek, preview sound pick.
