# S08: take it back (UNDO, RESET), ask what a control does (HELP), the menu, and the rest of the first-run strip

SEQUENCER_UX 2.6, 2.7, 3, 11.3 and U59 to U65. Start: playing, CH1 ACID on MIDI channel 2, the S02 morphs applied, page 1
of 1, first-run strip at step 4 (`PRESS FX: NEW PATTERN  EDIT=undo`). Notation as S01.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Press FX | first-run strip advances to `TAP A WHITE KEY: STEP ON/OFF`; toast `ALL`; CUE `EDIT takes it back` for 2 s first | next step: a new line | B_FX/PERF |
| 2 | Tap white key 3 | first-run strip advances to `HOLD A WHITE KEY: EDIT THAT NOTE` | step 3 sounds at its next pass | KEY_W03_A3/PERF |
| 3 | Hold white key 5 for 0.4 s, then tap HOME | NOTE_EDIT opens (strip `HOLD...` becomes `HOME=done`); after HOME the strip reads `PG+ TWICE: ADD PAGE 2 (A COPY)` | audition of step 5 | KEY_W05_C4/PERF, B_HOME/NOTE_EDIT |
| 4 | Tap D#5 twice | page 2 exists (S03 steps 23 and 24); the first-run strip ends and never returns (bit in `ui_prefs2`) | none audible | KEY_B10_D#5/PERF |
| 5 | Tap HOME on PERF | toast `HOME  hold: HELP / MENU`; nothing opens | none | B_HOME/PERF |
| 6 | Hold EDIT for 0.4 s | Page RESET. Banner `RESET`; every resettable cell carries an `RST` tag; K1R cells read `RST` over their values; tag column `RST R1` over `RST R2`; BTN `RST AL` `RST NT` `RST AC` `RST SH` `CANCEL` `-` / `HOME` `-` `RST AR` `RST HT` `PLAY` `RST PG`; white keys read `CLR`; LEG D#4 reads `COPY P`; ALGO and PRESETS read `-`; CUE `RESET: TOUCH WHAT TO RESET  EDIT=no` | playback continues | B_EDIT/PERF |
| 7 | Turn K2 one detent | `SLIDE` returns to `4`; RESET closes after this one reset; CUE `EDIT takes it back` | next step: the default slide density | EN_K2/RESET |
| 8 | Tap EDIT | toast `UNDO`; `SLIDE 6` is back | next step | B_EDIT/PERF |
| 9 | Hold EDIT, tap SEQ | HITS seed back to the factory seed (`RST HT`); RESET closes | next step: the factory rhythm | B_EDIT/PERF, B_SEQ/RESET |
| 10 | Hold EDIT, tap FX | the whole algorithm back to factory: 8 morphs and every seed; edits kept (`RST AL`) | next step: the factory ACID line | B_FX/RESET |
| 11 | Hold EDIT, tap OCT- | the four ROW 1 morphs back to default (`RST R1`) | next step | B_OCTDN/RESET |
| 12 | Hold EDIT, tap SCL / ENV / LFO / ARP in turn (four resets, each after a new hold) | that layer's seeds back to factory (`RST NT`, `RST AC`, `RST SH`, `RST AR`); each closes RESET | next step | B_SCL/RESET, B_ENV/RESET, B_LFO/RESET, B_ARP/RESET |
| 13 | Hold EDIT, tap OCT+ | the four ROW 2 morphs back to default (`RST R2`) | next step | B_OCTUP/RESET |
| 14 | Hold EDIT, tap white key 3 | all edits of step 3 cleared; toast `STEP 3 CLEARED  EDIT=undo` | step 3 follows the generator again | KEY_W03_A3/RESET |
| 15 | Hold EDIT, tap REC | every edit of the viewed page cleared (`RST PG`) | next pass | B_REC/RESET |
| 16 | Hold EDIT, tap D#4 | the page before is copied onto the viewed page (page 1: its edits are cleared) | next pass | KEY_B05_D#4/RESET |
| 17 | Hold EDIT, tap EDIT | `CANCEL`: nothing reset, back to PERF | none | B_EDIT/RESET |
| 18 | Hold EDIT, tap GLO, then hold EDIT, tap SAVE, then hold EDIT, turn PRESETS | each shows toast `NOTHING TO RESET` and the page stays RESET until cancelled | none | B_GLO/RESET, B_SAVE/RESET, EN_PRESET/RESET, EN_ALGO/RESET |
| 19 | In RESET tap PLAY, tap white key 7 | PLAY still starts or stops; the key clears step 7 | start / stop bytes as S01 step 5 and 10 | B_PLAY/RESET, KEY_W07_E4/RESET |
| 20 | Tap HOME | back to PERF | none | B_HOME/RESET |
| 21 | Hold HOME, do not touch anything, watch from 0.25 s | CUE `HOME: touch=HELP  let go=MENU`; the page stays | none | B_HOME/PERF |
| 22 | While HOME is down, press SEQ | Page HELP: the page beneath dimmed, a two-line help box over y=108 to y=212 naming SEQ and what it does on PERF; the control does not dice; the text in the mock-up is `SEQ = HITS: re-roll where the notes fall`; CUE `HELP: SEQ  let go = back` (the strip shows line 1 of its row in `docs/design/help_text.tsv`, `HITS: re-roll where notes fall`, and the box shows both lines; all 343 control-page rows are written, FINDINGS F-06 fixed) | none | B_SEQ/HELP |
| 23 | While HOME stays down, turn K1, then SELECT, then ALGORITHM, then PRESETS | the box explains `DENS` (both rows), `SELECT`, `ALGORITHM`, `PRESETS`; nothing changes | none | EN_K1/HELP, EN_SELECT/HELP, EN_ALGO/HELP, EN_PRESET/HELP |
| 24 | While HOME stays down, press PLAY | the box explains PLAY and the transport starts or stops | start / stop bytes as S01 | B_PLAY/HELP |
| 25 | Release HOME | back to PERF exactly as before (no menu, because a control was touched) | none | B_HOME/HELP |
| 26 | Hold HOME for 0.7 s touching nothing and release | the Felucca menu opens with its own hints: `HOME CLOSE` in the header, `ALGO TAB  PRESETS ROW  OCT- / OCT+ VALUE` in the footer; SCREEN_SPEC 3 now agrees with the control map (ALGORITHM = tab, PRESETS = row; SELECT keeps its page role and is not drawn in the menu; FINDINGS F-02 fixed) | none | B_HOME/MENU |
| 27 | In MENU turn ALGORITHM one detent, PRESETS one detent, K1 one detent | tab changes, row changes, the row's value changes (control map); OCT- / OCT+ give previous and next value (control map) but SCREEN_SPEC 3 says they read `close` | none | EN_ALGO/MENU, EN_PRESET/MENU, EN_K1/MENU, B_OCTDN/MENU, B_OCTUP/MENU |
| 28 | In MENU press SAVE | the menu closes; nothing is saved | none | B_SAVE/MENU |
| 29 | Tap HOME | back to PERF | none | B_HOME/MENU |

The first-run strip, steps 1 to 7 of SEQUENCER_UX 11.3, advanced in S01 steps 3 to 7, S08 steps 1 to 4. Step 5 of this
file shows PERF's HOME is a toast only, so a reflexive double HOME cannot land on a dead page.
