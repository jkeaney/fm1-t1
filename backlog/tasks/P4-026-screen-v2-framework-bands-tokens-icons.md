---
id: P4-026
title: Screen v2 framework: bands, palette tokens, icon blits, strip plan (replaces the ui_sq.c zones)
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-011]
gate: none
---
## Goal
Rebuild `ui_sq.c`'s drawing frame on the locked screen v2 (ADR-0006) so every later screen task draws into the right
bands with the right tokens: the base every element of SCREEN_SPEC_V2 needs.

## Read first
- docs/design/SCREEN_SPEC_V2.md sections 1-3 and 7; docs/project/decisions/ADR-0006-screen-architecture-v2.md
- docs/design/ui_review/proposals/v12/v12.py (reference implementation), icons_v12.py, ../v5/el.py (painter), ../v7/icons_v7.py
- firmware/src/ui_sq.c, gfx.c (cv_* API, palette_set), build/gen/ui_palettes.h, tools/gen_ui_palettes.py

## Deliverable
- Band geometry constants and a strip plan (0-119, 120-239) in ui_sq.c; the old Z0..Z7 zones removed.
- New tokens T_CH1..T_CH4, T_CHF1..4, T_PH, T_DICE, T_OFF with a value on every palette (HI-CON values per spec section 3)
  plus the mono fallback; HI-CON becomes the default palette.
- 1-bpp icon table generated from icons_v12.py art (all ICONS used by v12) with a `cv_icon(x, y, id, fg)` blit.
- RAM/flash cost line in the Log.

## Acceptance criteria
- [ ] host test renders the empty frame of `playing` and the band boxes match SCREEN_SPEC_V2 section 2
- [ ] every palette has the new tokens (generator test)
- [ ] icon blits are pixel-identical to the mock for every icon (host test)
- [ ] `sh tools/check.sh` ends CHECK PASS; RAM headroom >= 8 KB

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- The element drawing (P4-028, P4-029); the algorithm descriptor (P4-027); flashing.

## Log
