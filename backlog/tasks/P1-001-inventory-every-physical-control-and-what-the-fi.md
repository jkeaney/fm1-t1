---
id: P1-001
title: Inventory every physical control and what the firmware does with it today
phase: P1
type: task
status: todo
priority: 1
executor: haiku
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
- [ ] Every enum value in panel.c's B_* and EN_* lists appears in the tables (check by grepping each name)
- [ ] Every row cites at least one file:line
- [ ] The menu-diving list names every MI_* row in menu_items.c and says whether it's musical (should move to the panel) or a setting

## Verification
```sh
for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "B_$b" docs/design/CONTROL_INVENTORY.md || echo missing B_$b; done   # prints nothing
grep -c 'MI_' docs/design/CONTROL_INVENTORY.md   # ≥ 36
```

## Out of scope
- Proposing the new mapping (that's P2-001).

## Log
