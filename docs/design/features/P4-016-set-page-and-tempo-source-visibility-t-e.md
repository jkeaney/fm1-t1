<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-016: SET page and tempo-source visibility (T-E): use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-016-set-page-and-tempo-source-visibility-t-e.md   Spec: SCREEN_SPEC_V2 (header 4.1, knob tiles 4.2, message slot 5), SEQUENCER_UX 2.4 and 2.8, SCREEN_SPEC 6.2 and 6.5 (strings only; zones superseded)   Scenario: S07 steps 1-6, S06 steps 3-4, 11-12, S12

**STATUS: GATE A PROPOSAL, AWAITING OWNER SIGN-OFF. Nothing is built.** SET is not drawn in SCREEN_SPEC_V2 (its section 8 lists it as not yet drawn), so this page is a new design in the locked v12 idiom. Build starts only after the owner approves the renders below (section 6, last line).

## Proposed renders (the owner reviews these)
Source: `docs/design/ui_review/proposals/v12/v12_set.py` (imports the v12 reference implementation; the locked v12 scene set and the firmware comparison are untouched). Regenerate from that directory: `../../../../../.venv/bin/python ../../mock/mock.py render v12_set.py --out out_set`. Each scene has `.png` (1x), `_4x.png` and `_grid.png` in `docs/design/ui_review/proposals/v12/out_set/`:

| PNG (prefix `v12_set_set_`) | What it shows |
| --- | --- |
| `int` | SET, own clock, playing: BPM 120 SWING 50% GATE 50% DIV 1/16, status `OUT MASTER`, `OCT 0`, footer `KEYS = STEPS` / `GLO = BACK` |
| `int_stopped` | the same stopped: header stop square, status `OUT ON PLAY` |
| `turn_swing` | K2 turned: the tile frames itself in the channel colour (as PERF), old level ticks |
| `octave` | OCT+ touched: slot `OCT+ OCTAVE +1`, status `OCT +1` |
| `ext_live` | host clock accepted: header `EXT` over `124`; K1 tile becomes `HOST 124` (dashed frame, no fill, MID digits); status `EXT USB  OUT FOLLOW`; footer `HOST SETS TEMPO` |
| `ext_k1_turned` | K1 turned while EXT: nothing changes, tile frames in white, slot `K1 HOST SETS TEMPO` |
| `ext_wait` | PLAY pressed, host not started: header `EXT` / `WAIT`, tile `HOST --`, footer `WAIT FOR HOST START` / `PLAY=CANCEL` |
| `ext_lost` | clock vanished: header `EXT` / `--` in red, tile `HOST --`, footer `CLOCK LOST: STOPPED` in red / `PLAY=START` |

Design points (all from existing locks unless marked NEW):
- Bands, fonts (AF_S / AF_M / AF_L), tokens, 12 icon buttons, keybed and the music row are exactly PERF's: steps and moves stay live on SET (page_states SET row).
- Message slot while open: `GLO  ALL CHANNELS` (the CHAN pattern `CHN  CHn SETUP`; it yields to hold hint, touched name and toast).
- Knob tiles per knob_map SET rows: K1 BPM 40..240 (AF_L, meter), K2 SWING 50..75 %, K3 GATE 10..100 %, K4 DIV 1/32 1/16T 1/16 1/8T 1/8 1/4 (a 6-name list: dots, no meter). One font per knob for its whole range (`value_font`); the percent knobs resolve to AF_M. Every tile is a global value: the meter uses the neutral amber of the selected channel as on PERF (NEW question 3).
- Status line (NEW, replaces PERF's ROW boxes and PAGE which have no meaning here): preset `Pnn` left; centre = clock out (`OUT MASTER` while playing as master, `OUT ON PLAY` stopped, `OUT FOLLOW` on any external source; SCREEN_SPEC 6.4) preceded, when external, by `EXT USB` or `EXT TRS` (the port the clock came from); right = `OCT -2..+2` (OCTAVE moved here by HARMONY, P2-012; OCT- / OCT+ are named in the slot while touched, as on PERF).
- Tempo source in the header on every page (SCREEN_SPEC_V2 4.1, already drawn by P4-028 in `ui_sq.c`): INT = `120`; EXT = `EXT` over the BPM; waiting = `EXT` / `WAIT`; lost = `EXT` / `--` in `T_REC`. This task adds the host test of the four strings and their widths, not new header code.
- EXT: K1 is a read-only mirror, not a control. The shape changes (label `HOST`, no fill, dashed frame), not only the colour. Turning it names why (toast). The user's own tempo is never touched and returns when the host stops (SEQUENCER_UX 2.8; TIMING_SPEC).

## 1. Who and when
A producer wants to set the tempo, swing, gate length and step division for all channels, and to see at a glance whether the tempo is their own or a host's, without a menu.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Tap GLO (B_GLO, PERF / NOTE_EDIT / CHAN) | SET: slot `GLO  ALL CHANNELS`, tiles BPM SWING GATE DIV, status `P01  OUT MASTER  OCT 0`, footer `KEYS = STEPS` / `GLO = BACK` | none |
| 2 | Turn K1 (EN_K1), INT | BPM tile and the header tempo show the new number in the same frame | tempo changes at once (`beat_samples`) |
| 3 | Turn K2 / K3 / K4 | SWING %, GATE %, DIV name; the tile frames itself while turning | from the next step |
| 4 | Tap OCT+ / OCT- (B_OCTUP / B_OCTDN) | slot `OCT+ OCTAVE +1` while touched; status `OCT +1` (clamps at -2 / +2) | from the next step the notes sit an octave away |
| 5 | Host clock starts (S06): header becomes `EXT` / `124` | K1 tile becomes `HOST 124`, footer `HOST SETS TEMPO`, status `EXT USB  OUT FOLLOW` | the FM-1 follows the host |
| 6 | Turn K1 while EXT | nothing changes; slot `K1  HOST SETS TEMPO` | none |
| 7 | PLAY while EXT and the host has not started | header `EXT` / `WAIT`; tile `HOST --`; footer `WAIT FOR HOST START` / `PLAY=CANCEL` | none until the host's Start (P4-031 owns the transport) |
| 8 | The host clock stops | header `EXT` / `--` in red for the lost period; footer `CLOCK LOST: STOPPED`; then back to INT: the own tempo (unchanged) is in the BPM tile | transport stopped, note-offs |
| 9 | Tap GLO or HOME | back to PERF | none |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| BPM, SWING, GATE, DIV, OCTAVE (page open + turn / tap) | Setup (2 gestures) |
| Read the tempo source (header, every page) | Performance (0 gestures: always on screen) |
| Read the host tempo on SET (EXT) | Setup (0 beyond opening the page) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| BPM / SWING / GATE / DIV | 3 | 3 | 3 | 3 | 12 | tile labels are the printed names; value and header BPM move in the same frame; the page names itself in the slot; footer says `GLO = BACK` | yes |
| OCTAVE | 2 | 3 | 3 | 3 | 11 | OCT- / OCT+ are printed on the panel and named in the slot while touched; the value is on the status line (small AF_M at the far right) | yes |
| Tempo source (header) | 3 | 3 | 3 | 3 | 12 | two-line `EXT` / number differs in shape from the single INT number; red `--` is a shape plus colour | yes |
| EXT on SET | 3 | 3 | 3 | 3 | 12 | the tile changes label and shape (no fill, dashed), the footer says who sets the tempo, a K1 turn is answered by a toast | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
The design is not signed off, so the locked tables are NOT edited yet. Edits that the build commit makes (a listed row, a changed cell), so no surface is left describing the older screen:
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs | knob_map SET K1 (`value shows EXT` -> tile `HOST n`, read-only); button_modes SET OCT- / OCT+ (`the tag column reads OCT -2..+2` -> status line `OCT n`; no tag column exists in v12); the SET SELECT row (`CH n`) unchanged | to edit at build |
| UI / screen | page_states.tsv SET row (`SET band ... 2 status lines TEMPO / OUT ... tag column` -> v12 description above); SCREEN_SPEC_V2 section 8 (SET moves from not covered to covered; section 4.2 gains the SET band) and section 5 (`GLO  ALL CHANNELS` page title); SCREEN_SPEC.md 6.5, 7 (SET mock-ups) and 7.1 gain a `superseded by SCREEN_SPEC_V2` header (done now: the pointer only); SEQUENCER_UX 2.4 mock-up and 2.8 `Pixels` sentence | to edit at build (pointers added now) |
| UX / help | help_text.tsv EN_K1..K4 SET rows (K1 `your own tempo / inert while a host clock runs`, K2 SWING, K3 GATE, K4 DIV) checked: they still agree with the proposal (K1 tile reads `HOST` while EXT; the help words stay true) | yes |
| Workflow | S07 steps 1-6, S06 steps 3-4 / 11-12 (the `TEMPO INT n` status line and tag column are gone), S12 (generated from page_states / knob_map by tests/design/gen_sweeps.py: regenerate), COVERAGE.md | to edit at build |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: GLO tap
- [x] Undoable in one press or self-reverting: values are turned back; EDIT = undo is a no-op for globals (not an undo-stack action; says nothing); OCT by tapping the other
- [x] Lands on a musical boundary, no hung or cut notes: BPM is read by the scheduler (beat length); SWING, GATE, DIV, OCTAVE are read at the next step (sq_core reads `g->s` per step); no note is cut
- [x] Screen shows the value within one frame; sound changes by the next step (BPM: at once)
- [x] One-handed
- [x] Does not change another channel (or says so on screen): the page is labelled `ALL CHANNELS`
- [x] Survives stop/start, clock-source change, preset load: the tile and header re-read the source each frame; the user's tempo is never overwritten by the host; a preset load changes the global values and the tile shows them next frame
- [ ] Cost recorded (RAM headroom >= 8 KB): measured at build (the page adds a handful of bytes of view state; the strings are `ui_tables` data)

Hazards checked in the design (scan for bounds, stale state, surface sync):
1. Bounds: BPM 40..240 (clamped; fast = 5), SWING 50..75, GATE 10..100, DIV 0..5, OCTAVE -2..+2 (stored as a signed int8 in `GlobalSave`; every turn and tap clamps, no wrap), the `EXT` host tempo shown is the clamped 40..240 integer (never 3 digits + sign). No stack grows: SET keeps no list or history.
2. Stale state: the BPM tile mirror must read the source each frame (`clk_ext_bpm`, never a cached copy) and `bpm_own` when INT; `ui.force` / the view signature must include the clock state and the octave, or the tile and status line stay old; a pending K1 touch frame must clear on leaving SET; EXT -> INT must put the own tempo back in the tile in the same frame; LOST has no last-host-tempo state (SCREEN_SPEC 6.2).
3. Surface sync: the table rows and scenarios named in section 5 are the ones that still describe the older screen (the `TEMPO INT n` status line and the OCT tag column); the build commit edits them and re-runs `check_scenarios.py` / `gen_sweeps.py`.
4. Width: every new string is measured with the mock's `text_w`: `GLO ALL CHANNELS`, `K1 HOST SETS TEMPO`, `OCT+ OCTAVE +1`, `OCT- OCTAVE -2` fit the 136 px slot (`check_messages()` returns no failure); the status line asserts the clock-out blocks end before the OCT readout (`EXT USB  OUT FOLLOW` is the widest); the footers are asserted to fit 240 px (`WAIT FOR HOST START` + `PLAY=CANCEL` in AF_S).

Design locked: not yet (owner sign-off pending).

### Questions for the owner (answer with the PNGs)
1. The task text asks for a blinking colour-coded clock dot (SCREEN_SPEC 6.3). SCREEN_SPEC_V2 4.1 (locked) has no dot: the tempo source is the text `EXT` / `WAIT` / `--` and the transport icon. The proposal follows V2 (no dot, so no beat blink). Keep it that way, or add the dot to the header?
2. SET's status line drops ROW and PAGE (meaningless here) for `OUT` and `OCT`. OK, or should PAGE stay?
3. Tile meters on SET use the selected channel's colour; the values are global. Alternative: white (as the REC band). Which?
4. EXT SYNC (first 7 pulses, `EXT --` in SCREEN_SPEC 6.2) is not in V2's list: shown as the same `HOST --` tile as WAIT. OK?
5. WAIT and LOST footers use AF_S (the strings are too wide for AF_M with their right-hand hint). OK at 30-40 cm?

## 7. Walk-through (after the build)
Not yet done (Gate B: the reviewer).

## 8. Actual ease score
Not yet done.

## 9. Sync re-check and outcome
Not yet done.
- Approved: pending (owner)
