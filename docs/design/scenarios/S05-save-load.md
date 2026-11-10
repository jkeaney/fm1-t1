# S05: save and load whole-device presets (P01..P16)

SEQUENCER_UX W7 and U56 to U58, U69. Start: playing, CH1 ACID MIDI 2, header slot `P01`, 120 BPM. Notation as S01.
The state saved is the whole device: channels, algorithms, morphs, seeds, step edits, lengths, pages, mutes, globals.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Tap SAVE | Toast `SAVED P01`; HD slot `P01` | none; the music is unchanged | B_SAVE/PERF |
| 2 | Turn K1 (DENS) two detents clockwise so that the state differs from the saved one | K1R `DENS 14` (from 12) | from the next step: 14 hits | EN_K1/PERF |
| 3 | Turn PRESETS one detent clockwise | HD slot `P02?` at once; CUE `P02 AT BAR  SAVE = save here` | none yet | EN_PRESET/PERF |
| 4 | Tap SAVE before the load lands (the window is at least 1.5 s after the last detent) | Toast `SAVED P02`; HD slot `P02`; the pending load is cancelled | none; the music is unchanged | B_SAVE/PERF |
| 5 | Turn PRESETS one detent counter-clockwise to `P01` and let go | HD `P01?`; after 1.5 s the load is staged and HD reads `P01>` with the bar countdown; CUE `P01 AT BAR  SAVE = save here` | none until the first bar starting at least 1.5 s after the last detent | EN_PRESET/PERF |
| 6 | Wait for the bar | HD `P01`; K1R `DENS 12` (the saved state); CUE `SAVE hold=undo` | the P01 pattern plays from the bar start; any notes of the old state end by their queued offs | EN_PRESET/PERF |
| 7 | Hold SAVE for 0.7 s | Toast `LOAD UNDONE: P02`; HD `P02`; K1R `DENS 14` | the pre-load state returns at the next step | B_SAVE/PERF |
| 8 | Hold SAVE for 0.7 s again | the P01 state returns (redo); the redo toast text is not locked (FINDINGS F-16) | the P01 pattern at the next step | B_SAVE/PERF |
| 9 | Turn PRESETS one detent clockwise to `P02`, then one back to `P01` before the settle time ends | HD `P02?` then back to `P01` (the current slot): the pending state clears | none | EN_PRESET/PERF |
| 10 | Turn PRESETS two detents clockwise, hold SAVE for 0.7 s while the slot is pending | the pending slot is cancelled instead of undoing a load; HD returns to `P01` | none | B_SAVE/PERF |
| 11 | Press PLAY (stop), turn PRESETS one detent clockwise to `P02`, wait 1.5 s | HD `P02?`, CUE `P02: SAVE = save here` during the wait; then the load lands at once and HD reads `P02` | `FC`, `B1 7B 00` on stop; no notes | B_PLAY/PERF, EN_PRESET/PERF |
| 12 | Turn PRESETS to a slot that was never saved (`P09`) and wait | HD `P09?`; what is loaded from an empty slot is not locked (FINDINGS F-15) | none | EN_PRESET/PERF |

Power-cycle: ADR-0003 section 12.6 restores the last state from a clean shutdown (autosave), but SEQUENCER_UX and
SCREEN_SPEC lock no screen text or cue for it, so no step above shows it (FINDINGS F-15).
