---
id: P4-029
title: Screen v2 elements: button picture, keybed footer, message slot (toasts, hold hints, touched names, setup cue)
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-026]
gate: none
---
## Goal
Document every control where the eye is, as SCREEN_SPEC_V2 sections 4.5, 4.6 and 5: the icon buttons, the keybed in its
mode, and the one message slot with its priority rules and timing.

## Read first
- docs/design/SCREEN_SPEC_V2.md sections 4.5, 4.6, 5, 6; proposals/v12/v12.py (buttons, keybed, topbar, MESSAGES, check_messages)
- docs/design/button_modes.tsv, key_map.tsv (hold times 0.4 / 0.7 s); firmware/src/ui_sq.c toast queue

## Deliverable
- Button picture (dice tint, absent layer dimmed, pressed inverted, REC armed); keybed footer (step fronts, playing bar,
  black-key glyphs, held states, CHAN BACK, REC plain keys, footer lines); message slot with toast / touched-control /
  hold-hint / page-title priority, hold bars timed from the TSV hold times, the setup cue.
- The MESSAGES strings as a generated table (width-checked by P4-027's check).

## Acceptance criteria
- [ ] host renders of the remaining SCREEN_SPEC_V2 section 7 scenes match the v12 mock (or listed and approved differences)
- [ ] every message fits (check passes); a too-long one fails the build
- [ ] `sh tools/check.sh` ends CHECK PASS

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- Header/knobs/music (P4-028); table rows (P4-030).

## Log
