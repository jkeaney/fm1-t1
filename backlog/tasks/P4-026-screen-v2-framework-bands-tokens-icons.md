---
id: P4-026
title: Screen v2 framework: bands, palette tokens, icon blits, strip plan (replaces the ui_sq.c zones)
phase: P4
type: task
status: review
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
- [x] host test renders the empty frame of `playing` and the band boxes match SCREEN_SPEC_V2 section 2
- [x] every palette has the new tokens (generator test)
- [x] icon blits are pixel-identical to the mock for every icon (host test)
- [x] `sh tools/check.sh` ends CHECK PASS; RAM headroom >= 8 KB

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- The element drawing (P4-028, P4-029); the algorithm descriptor (P4-027); flashing.

## Log

### 2026-11-13 executor (sonnet)
Commands: `sh tools/check.sh` -> `RAM .data+.bss 23844 B of 73728` (headroom 49884 B, >= 8 KB), `CHECK PASS`.
Decisive lines (build/check/tests.log): `icons: 21 1-bpp icons pixel-identical to icons_v12.py`, `ui_sq_test: OK (0 failures)`.
What changed:
- ui_sq.c: Z0..Z7 and the three blits removed; band constants (SCREEN_SPEC_V2 2), the two-strip plan (0-119, 120-239, drawn in screen coordinates with `cv_oy = -y0`), and `uisq_draw_frame()` (knob tiles, button tiles with the six dice in T_DICE, 16 white keys, 11 black keys). The old per-zone element drawing is deleted (P4-028/029 redraw it); the v1 label/lane helpers (uisq_btn_*, uisq_h4_text, uisq_lane_read, toast state) are kept un-drawn because tests/ui_sq_test.c still measures them; P4-028/029 reuse or prune.
- gfx.c: tokens T_CH1..4 / T_CH(n), T_CHF1..4 / T_CHF(n), T_PH, T_DICE, T_OFF (ux fields, set in palette_set from the generated UI_PAL_X table; CUSTOM uses HI-CON's, PAPER's on a light bg); `cv_icon(x, y, id, fg)`.
- tools/gen_ui_palettes.py: UI_PAL_X[palette][11] on all 10 palettes (HI-CON = spec values; dark palettes share them; PAPER darkens channels to 62 %; GREY = luma grey; MONO = white + the one grey), generator checks (contrast >= 3.0 of CH/OFF on bg, MONO/GREY constraints), `UI_HICON_INDEX`. panel.c: factory default palette is now HI-CON.
- tools/gen_sq_icons.py (+ tools/build.py generate()): build/gen/ui_sqicons.h from icons_v12.py (all 21 icons; el.IC read with ast, no Pillow); `--ref` writes build/gen/sq_icons_ref.txt for the test, which run_tests.sh produces.
- tests/ui_sq_test.c: band table checks, no text band crosses y 120, token values, icon pixel-identity, empty `playing` frame box checks, signature tests (2 strips); PNG build/ui_sq/frame_playing.png.
Cost: ui_sq.c is not yet in felucca.c, so the device image only gains the tokens and tables: RAM +22 B (.bss, 11 uint16 in `ux`); flash about +750 B (UI_PAL_X 220 B, icon rows 444 B + table 84 B, once cv_icon is used; unused statics are dropped by the linker).
Look at: the PAPER/GREY token choices (not in the spec, mine); HI-CON as default changes first-boot look (settings_test/theme_test still pass).
