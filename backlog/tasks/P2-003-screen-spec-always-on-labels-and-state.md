---
id: P2-003
title: Screen spec: always-on labels and state
phase: P2
type: task
status: review
priority: 2
executor: sonnet
depends_on: [P2-001, P1-013, P1-014, P1-017]
gate: human-review
---
## Goal
VISION rule 1, made concrete: every screen always shows what the device is doing and the current role of every control.

## Read first
- docs/design/SEQUENCER_UX.md and the TSV tables page_states.tsv / button_modes.tsv / knob_map.tsv (P1-017: the locked sequencer functionality and UX; every page needs a screen spec)
- docs/design/PERFORMANCE_DESIGN.md (P1-014: performance-first controls; supersedes anything it changes in FULL_CONTROL.md)
- docs/design/FULL_CONTROL.md (P1-013: shift row, up to 8 morphs per algorithm, live two-row labels)
- docs/design/CONTROL_MAP.md
- firmware/src/ui.c layout constants (H_HEAD, Y_LABEL, CARD_H, Y_GRAPH, Y_FOOT)
- firmware/src/gfx.c (fonts AF_S/M/L, theme colours, CV_MAX 240x124)
- firmware/src/ui_t1.c (current T-1 screen)

## Deliverable
- docs/design/SCREEN_SPEC.md: a fixed zone layout for 240x240 (status bar, knob-label row, main view, button/footer hints) with pixel rectangles; the content of each zone per mode; how modifier-held states relabel; mock-ups as ASCII grids; text length limits per zone, with fonts

## Acceptance criteria
- [x] Every control in CONTROL_MAP has a defined on-screen label location for each mode
- [x] The zone rectangles tile 240x240 without overlap (listed as x,y,w,h)
- [ ] The owner approved it (log line)
- [x] Screen layouts are specified as data (regions, label slots, state fields) so a label or layout change is a table edit; a 'Flex points' section lists them
- [x] Every knob/button role shown on screen for each context (VISION rule 1)
- [x] A 'Boot and first use' section: power-on goes straight to the sequencer main screen with a default channel, algorithm and global scale/key, and PLAY is audible with no setup; no groovebox or mode choice appears anywhere (owner decision 32b)

## Verification
```sh
grep -c 'x,y,w,h\|x=' docs/design/SCREEN_SPEC.md
```

## Out of scope
- Implementing the drawing code.

## Log

### 2026-11-10 executor (sonnet)
- Wrote docs/design/SCREEN_SPEC.md: zones Z0..Z7 tile 240x240 (heights 20+28+60+52+52+12+12+4 = 240; area sum 57600), 3 canvas blits each <= 240x124, slot tables, control-to-slot table for all 49 controls x 9 pages, modifier relabels, length limits, mock-ups, flex points, boot section. Widths measured with a probe built from the real gfx.c `text_w` and build/gen/ui_fonts.h.
- Folded in FB-001 (owner scope): SCREEN_SPEC section 6 (INT / EXT LIVE / EXT SYNC / EXT WAIT, 500 ms timeout = the existing seq.c rule, clock out MASTER when source INT / FOLLOW otherwise). Added matching text: SEQUENCER_UX 2.8, cue rows 8a / 13a, locked item 12, flex row; page_states.tsv PERF and SET screen fields; ADR-0003 change note, section 5 row, new 7.4, flex row. Nothing locked was changed.
- Findings for the reviewer (SCREEN_SPEC 9): the locked mock-ups are not drawable as printed (AF_M labels too wide; PERF row 2 needs ~326 px). Resolved with AF_S, OCT labels in the band tag column, a hold mark in place of `h:`. Open owner questions in SCREEN_SPEC 11.
- Verification:
  - `grep -c 'x,y,w,h\|x=' docs/design/SCREEN_SPEC.md` -> 7
  - `python3 tools/check_sequencer_ux.py` -> `pages=9 ... scored=69 exceptions=3` / `OK`
  - `python3 tools/check_control_map.py` -> `rows tsv=441 md=441` / `OK`
  - `python3 tools/backlog.py check` -> `39 items, 0 errors`
  - tools/check.sh not run: docs-only change, no code.
- Unticked: "The owner approved it" (human gate; no Approved line written).
