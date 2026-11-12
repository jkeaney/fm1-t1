<!-- SPDX-License-Identifier: GPL-3.0-only -->
# v12: the locked design (Design locked: James Keaney 2026-11-12)

The accepted v11 direction plus the remaining states. Spec: docs/design/SCREEN_SPEC_V2.md. ADRs 0006, 0007.
Render: `python3 ../../mock/mock.py render v12.py --out out && python3 sheets_v12.py`; message fit check:
`python3 -c "import sys; sys.path[:0]=['../../mock','.']; import mock; mock.palette_set('HI-CON'); import v12; print(v12.check_messages())"`.
Added over v11: tempo source (EXT / WAIT / lost), FOLLOW off, past LENGTH, the CHAN page (title in the message slot,
BACK on D#4, `K2 = MIDI`), MIDI clash toast naming the channel, held SOLO/FILL/ROLL, first-run cue, remove a channel by
a long press on its mute key (owner), UNDO/REDO toasts, the playhead pointer, the ROW label, the MESSAGES table + check.
