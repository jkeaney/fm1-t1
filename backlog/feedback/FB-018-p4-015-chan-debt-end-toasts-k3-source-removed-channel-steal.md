---
id: FB-018
title: P4-015 CHAN debt: no ONE PAGE / 64 STEPS toasts, K3 SOURCE inert, a steal can name a removed channel
phase: P4
type: feedback
status: todo
priority: 3
executor: human
depends_on: [P4-015]
gate: none
session: 2026-11-13, host tests only (no device), P4-015 reviewer
---
## Goal
Triage the three CHAN gaps left by P4-015 into tasks, or drop them with a reason.

## Read first
- docs/project/VISION.md
- backlog/tasks/P4-015-chan-page-midi-channel-algorithm-length.md (Log), firmware/src/ui_sq.c (CHAN section)

## Deliverable
- Linked P-ids for each item below, or a drop reason, recorded in the log.

## Observation
1. **End-of-range toasts.** OCT- at LENGTH <= 16 and OCT+ at 64 are silent (button_modes CHAN OCTDN says toast `ONE PAGE: K1 = SHORTER`, key_map PG+ says `64 STEPS = MAX`). The v12 MESSAGES table has no row for either, so the page cannot draw them without a literal. Needs the rows added to v12.py/ui_tables.h (design change), best folded into P4-032 (PERF past-the-end gaps) or P4-018 so PERF and CHAN share one wording.
2. **K3 SOURCE** is drawn (`OWN`) and inert. sq_page_src_set exists; a new page copies the source of the page before, so inert is musically safe, but the producer cannot choose P1..P3. Needs its own task before HW-1 if the owner wants page sourcing on the first hardware pass.
3. **Steal from a removed channel.** sq_set_midi scans all four ChanSave rows, so a removed (not in use) channel that still holds MIDI n is set to OFF and named in the toast `[k] MIDI OFF  n TO CHm` for a channel the screen does not show; re-adding it later gives it MIDI OFF. Harmless to sound, confusing in the toast. Fix options: the removal clears the channel's MIDI to OFF, or the commit skips unused channels (needs a core parameter).
4. (minor) EDIT after a page add that K1 later lengthened swaps LENGTH back to the record's value, and the viewed page then falls to page 1 (uisq_view_page fallback); LENGTH itself is documented not undoable (SEQUENCER_UX 1.6).

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
