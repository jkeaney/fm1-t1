---
id: P2-003
title: Screen spec: always-on labels and state
phase: P2
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P2-001]
gate: human-review
---
## Goal
VISION rule 1, made concrete: every screen always shows what the device is doing and the current role of every control.

## Read first
- docs/design/CONTROL_MAP.md
- firmware/src/ui.c layout constants (H_HEAD, Y_LABEL, CARD_H, Y_GRAPH, Y_FOOT)
- firmware/src/gfx.c (fonts AF_S/M/L, theme colours, CV_MAX 240x124)
- firmware/src/ui_t1.c (current T-1 screen)

## Deliverable
- docs/design/SCREEN_SPEC.md: a fixed zone layout for 240x240 (status bar, knob-label row, main view, button/footer hints) with pixel rectangles; the content of each zone per mode; how modifier-held states relabel; mock-ups as ASCII grids; text length limits per zone, with fonts

## Acceptance criteria
- [ ] Every control in CONTROL_MAP has a defined on-screen label location for each mode
- [ ] The zone rectangles tile 240x240 without overlap (listed as x,y,w,h)
- [ ] The owner approved it (log line)

## Verification
```sh
grep -c 'x,y,w,h\|x=' docs/design/SCREEN_SPEC.md
```

## Out of scope
- Implementing the drawing code.

## Log
