---
target: UI proposal v3-glance (FM-1 240x240 screen)
total_score: 24
p0_count: 0
p1_count: 4
timestamp: 2026-11-12T04-31-16Z
slug: esign-ui-review-proposals-v3-glance-proposal-v3-py
---
# Critique: UI proposal v3-glance (FM-1 240x240 screen, 30 mm wide)

Sources: docs/design/ui_review/critique_v3/ASSESSMENT_A_v3.md (independent design review), EVIDENCE_B_v3.md (measurements, edge-case sweep). Detector (detect.mjs) scanned zero files: not web markup, so it is a non-scan, not a clean result.

Design Health Score 24/40 (Acceptable). Heuristics: 1 Visibility 3, 2 Match real world 2, 3 Control 3, 4 Consistency 2, 5 Error prevention 2, 6 Recognition 2, 7 Flexibility 3, 8 Minimalist 3, 9 Error recovery 2, 10 Help 2.
Cognitive load: 2 of 8 checklist failures (minimal choices, working memory) on the perform side.

## Priority issues
- [P1] The perform loop is undocumented while playing: no button or key is labelled, HOLD HOME = MAP disappears on PLAY leaving a 20 px empty band; dice toasts do not name the printed button (SCL) and arrive after the re-roll. Command: clarify. Constraint: an AF_S legend (7.6 arcmin) would contradict the owner's too-small verdict; use AF_M cue and printed-name-first toasts instead.
- [P1] High-stakes toasts do not state consequence or exit: preset pending says P04? but not that it loads by itself at the bar, that SAVE keeps it, HOME cancels; undo toast does not say undone/which channel/EDIT again = redo. A knob turn must dismiss any toast. Command: clarify.
- [P1] MAP errors and legibility: SAVE hold labelled BACK (button_modes.tsv: undo last preset load, or cancel a pending slot); OCT+ omits ROW 3 (RIG row); all ~70 labels in AF_S; K1-K4 drawn as a vertical list, black keys do not show 3-2-3-2-1 grouping. Command: layout.
- [P1] Lane data fidelity (evidence B): in the playing column bars, accent triangle and slide tie draw in one dark colour so accent/slide of the current step vanish and the slide chain breaks; velocity 1-3 step is pixel-identical to a rest; ratchet 6-8 identical to none; playhead on an unviewed page leaves 0 amber px and only a 4x4 marker. Command: layout, colorize.
- [P2] Knob row mode signal weak (I/II glyphs 3 px, only a 10 px strip changes; no ROW 3; undocumented white hot-knob frame reads as selection). Command: bolder.
- [P2] Static info outranks missing info: channel numerals 1-4 in AF_L never change; each channel MIDI channel (the TB-3 workflow first step) is nowhere on the main screen; OFF chip clips its frame; a selected+muted+MIDI OFF channel shows no M chip. Command: layout.
- [P2] Accent cyan bar vs normal bar 1.15:1 (1.00 mono) reads as a cursor; accent survives only as an 11x6 px triangle. Command: colorize.
- [P2] Stated 5 px module not met: 0 of 35 measured elements on it; channel numeral ink 14 px left of cell centre; lane group edges 1 px off cell edges. Command: layout.
- [P3] Undefined states: EXT --, WAIT, EXT lost, P16> loading, MIDI-channel toast, per-layer dice toasts, first-run, SOL/FIL/ROL held, RESET latch, FOLLOW off. Enum names UP/P-P/RP+/OCV/THB read as code; 20 of 42 enum names fall to AF_M.

## Personas
Alex (power user), Sam (low vision, colour-blind), Jordan (first-timer), Dana (studio producer, hands busy): see ASSESSMENT_A_v3.md.
