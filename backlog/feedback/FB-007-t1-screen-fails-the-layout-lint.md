---
id: FB-007
title: The T-1 screen (ui_t1.c) fails the layout lint: its hint line runs off the screen and two card labels spill
phase: P3
type: feedback
status: todo
priority: 3
executor: human
depends_on: []
gate: none
session: 2026-11-11, executor (sonnet), P3-006: ui_render now draws the T-1 screen for the first time
---
## Goal
Triage the findings into the P4-011 screen spec (the T-1 screen is replaced there), or into a small fix task, or drop them.

## Read first
- docs/project/VISION.md
- firmware/src/ui_t1.c (t1_draw)
- tests/ui_render.c (the T-1 scenes; their findings are reported, not failing)

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## Observation
Before P3-006 `tests/ui_render.c` never drew the T-1 screen (the host ran Felucca's pages), so its layout was never linted.
With the T-1 screen the only page, the lint reports on every palette (build/ui_new/report.txt, "LINT ... t1*"):
1. The hint line `SEQ PAGE  OCT+ SHIFT  GLO ALL  SAVE TEMP` (ui_t1.c, canvas 2, x 8, y 72) is 242 px wide: it runs
   off the right edge (ink to x 250 on a 240 px screen) and is cut by its canvas.
2. On pages 4 (RND DEPTH) and with OCT+ shift (STEP MOD) a card label is wider than its 55 px card.
3. The screen also draws no LINE-style rules and its cards use SURF (invisible in STYLE LINE).
Expected: every text on the device's only performance screen fits the screen and its card.
P3-006 does not change ui_t1.c (out of scope: no behaviour change to the T-1 path); ui_render lists these findings in
its report and does not fail on them, so a new regression elsewhere still fails the build.

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
