---
id: P1-001
title: Inventory every physical control and what the firmware does with it today
phase: P1
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
The ground truth for the global control map (P2-001): every button, encoder and key, every gesture, in every mode, as the code really behaves.

## Read first
- firmware/src/panel.c (B_* and EN_* enums)
- firmware/src/ui_input.c (ui_input: taps, holds, combos)
- firmware/src/ui_layer.c (held-button layers)
- firmware/src/ui_t1.c (T-1 mode controls)
- firmware/src/seq.c keyboard_block and song.grid
- docs/controls.jpg (the panel)
- TORSO_T1_GUIDE.md §2 (the T-1 claims: verify, don't copy)

## Deliverable
- docs/design/CONTROL_INVENTORY.md with: (1) a table of the 14 buttons + 7 encoders/knobs, with columns Control | Tap | Hold | With-modifier combos | In T1 mode | file:line; (2) the 27 keys: what they do per song.grid value and in T1 mode; (3) the LEDs and what drives them; (4) every place a function is reachable only through the MENU (the 'menu-diving list')

## Acceptance criteria
- [x] Every enum value in panel.c's B_* and EN_* lists appears in the tables (check by grepping each name)
- [x] Every row cites at least one file:line
- [x] The menu-diving list names every MI_* row in menu_items.c and says whether it's musical (should move to the panel) or a setting

## Verification
```sh
for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "B_$b" docs/design/CONTROL_INVENTORY.md || echo missing B_$b; done   # prints nothing
grep -c 'MI_' docs/design/CONTROL_INVENTORY.md   # ≥ 36
```

## Out of scope
- Proposing the new mapping (that's P2-001).

## Log

### 2026-11-09 executor (sonnet)
Read-only inventory; no firmware edits, no full test suite, no flashing. Deliverable: docs/design/CONTROL_INVENTORY.md
(sections: hardware facts; 14 buttons and 7 encoders x Tap/Hold/Combos/T1; K1-K4 per page, Felucca and T1; 27 keys per
song.grid, per layer; layer knobs; LEDs; hidden gestures; menu-diving list of all 37 MI_* rows with class; open questions).

Verification:
```
for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "B_$b" docs/design/CONTROL_INVENTORY.md || echo missing B_$b; done   # printed nothing
for e in SELECT ALGO PRESET K1 K2 K3 K4; do grep -q "EN_$e" ...; done   # printed nothing (after adding K2-K4 to the encoder row)
grep -c 'MI_' docs/design/CONTROL_INVENTORY.md   # 43 (>= 36)
every MI_ value in menu_items.c:16-19 (37, excluding MI_COUNT) appears in backticks   # none missing
every table row starting with a backticked control/row name carries a file:line   # none missing
```
Findings worth the reviewer's eye (all [code]-verified unless the doc marks [unconfirmed]):
- EN_ALGO (printed ALGORITHM) selects the TRACK T1-T4 in Felucca mode, not an algorithm.
- The Felucca per-track MIDI out channel and MIDI IN routing exist only in MENU (MI_MIDIIN, MI_TCH1-4); T1 SEQ mode
  itself is toggled only by MI_T1SEQ.
- TORSO_T1_GUIDE.md section 2 is wrong in three places: REC does nothing in T1 mode; step-key P-LOCK is not wired (edit_step
  has no caller); OCT+ is SHIFT, not page navigation. Table in section 1.4 of the inventory.
- ui_leds() has no T1-specific branch; T1 LED feedback is unconfirmed on hardware.
- First draft cited several ui_input.c lines wrongly; all citations were re-checked against grep -n afterwards, but the
  reviewer should spot-check a few.
