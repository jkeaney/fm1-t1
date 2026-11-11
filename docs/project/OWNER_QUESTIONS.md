# Open questions for the owner

Collected from design tasks; resolve at the matching human-review gate (P1-010 / P2-001..P2-004).

| # | From | Question | Needed by |
| --- | --- | --- | --- |
| 1 | P1-003 sting | Is ACID without slide (TB-3 portamento) acceptable for v1? | P1-010 |
| 2 | P1-003 sting | How does the shared scale-quantise (SCL QNT) setting relate to ACID's own scale snap? | P2-001 |
| 3 | P1-003 sting | VISION rule 3 says a channel's only setting is its MIDI channel, but the algorithm docs assume shared GATE / DIV / SWING. Are those global, per-algorithm, or dropped? | P2-001 / P2-004 |
| 4 | P1-004 snake | The global scale control: what is its default value, and is "off" (chromatic) a setting? SNAKE's random pitches sound arbitrary on chromatic; recommend a minor-pentatonic default or a non-chromatic first scale. | P2-001 |
| 5 | P1-004 snake | Is a uniformly random pitch lane acceptable as a bass? Fallbacks: lower default RANGE (12) or scale-weighted pitches. | P1-010 |
| 6 | P1-004 snake | Should the dropped Custom modulation lanes return post-v1 as a CC lane? | P1-010 |
| 7 | P1-011 slide | SLIDE has no amount control (fixed ~16% of steps, only dice changes it) because the 4 knobs are taken. Acceptable, or give it a shifted-knob/button gesture to turn slide off? Recommend acceptable for v1; dice until none. | P1-010 |
| 8 | P1-011 slide | Slide is sent as overlapping notes (no CC). Confirm on the TB-3 that this glides (hardware test); if not, we try CC65. | first hardware test |
| 9 | P1-011 slide | A dice or knob change now takes effect one step later (next-step commit, up to ~125 ms at 120 BPM). Acceptable? | P1-010 |
| 10 | P1-005 ml185 | STAGE loops can be 8..64 pulses but the 16 white keys reach only pulses 0..15. Acceptable (long loops come from the generator), or should step entry get a page/shift to reach pulses 16..63? | P1-010 / P2-003 |
| 11 | P1-005 ml185 | STAGE long holds (mode I-) ignore the global GATE so a long note stays long when GATE is shortened. Acceptable? (A table edit if not.) | P1-010 |
| 12 | P1-005 ml185 | Are STAGE's defaults (TOTAL 16, DENS 6, RATCH 2, RANGE 12, base C2) good on a TB-3, and is a rhythm-first 8-pitch riff generator (no accent or slide lane) enough? | first hardware test |
| 13 | P1-006 steparp | ARP is recommended LATER (its rhythm half duplicates SNAKE/STAGE; only CHORD, ORDER, OCT are new). If v1 needs a harmonic algorithm, promote it as a fourth algorithm on the shared rank-order/ratchet primitives, or offer CHORD/ORDER as a pitch option for other algorithms instead? | P1-010 |
| 14 | P1-006 steparp | Is a 2..4-note chord pool enough for a TB-3 bass, or does it want a root-on-downbeat bias? | first hardware test (only if promoted) |
| 15 | P1-006 steparp | Chord snap on sparse scales turns major into minor in minor pentatonic. Acceptable? (Relates to #2 and #4.) | P2-001 |
| 16 | P1-006 steparp | Chord-tone snap direction: at or below (as inferred in the source) or nearest? | P2-001 |
| 17 | P1-007 rhythmic-steps | Is a fixed base note 36 with the alt at +12 acceptable for the pitchless RHYTHM algorithm, or must a drum channel carry a note setting (breaks rule 3)? | P1-010 |
| 18 | P1-007 rhythmic-steps | A pin on a step at or past LEN is silent until LEN is raised. Acceptable (same shape as #10)? | P1-010 / P2-003 |
| 19 | P1-007 rhythmic-steps | Add NUDGE (reversible small mutate, 0..8 steps) to the global dice vocabulary for all algorithms, independent of whether RHYTHM ships? | P1-010 / P2-004 |
| 20 | P1-007 rhythmic-steps | Is a drum-voice channel a v1 goal at all? If not, RHYTHM stays LATER; also lift the groove-template HITS order into the shared density primitive? | P1-010 |
| 21 | P1-007 rhythmic-steps (review) | None of the five assessed families places hits by even spacing (Euclidean). Björklund already exists in the T-1 code (`torso_t1_algo.c`: `t1_euclidean_bjorklund`, `_rotate`). Should P1-010 carry a reworked Euclidean algorithm into v1 as the rhythm-first choice (pitch from the global scale), and should the groove-template HITS order be offered as a second density source? | P1-010 |
| 22 | P1-008 melodic-steps | If MELODY is not promoted, should its contour walk (LEAP/SPAN, starting on the root) become a pitch-source option for SNAKE's and STAGE's NOTES layers? (Answers #5.) | P1-010 |
| 23 | P1-008 melodic-steps | Should note lanes advance per sounding note (hit counter, keeps the tune in order when thinned or pinned) or per step (`t`, as SNAKE/STAGE)? Shared primitive if per note. | P1-010 / P2-001 |
| 24 | P1-008 melodic-steps | Should a typed (pinned) step consume a contour note? Recommended no. | first hardware test |
| 25 | P1-008 melodic-steps | Cheaper v1 option: SNAKE plus a LEAP morph on its pitch lane instead of a separate MELODY algorithm (only if #22 is yes)? | P1-010 |
| 26 | P1-009 t1 | Is a 16-step maximum (LEN 2..16) acceptable for EUCLID, or are 32/64 steps (the T-1's range) wanted? (Relates to #10.) | P1-010 |
| 27 | P1-009 t1 | Is RATCH acceptable as a dice layer with fixed odds and no amount knob, or should it take a knob from SPAN or ACCNT? | P1-010 |
| 28 | P1-009 t1 | Should ROTATE be a knob (replacing LEN) rather than a dice layer? | P1-010 |
| 29 | P1-009 t1 | Is a root-anchored contour (never below the root, base 36, ceiling 73) right for a TB-3 bass, or centred on the root? | first hardware test |
| 30 | P1-009 t1 | Should EUCLID be the fourth v1 algorithm (recommended), or only a shared Euclidean placement source inside another algorithm? | P1-010 |
| 31 | P1-012 UX | The control map assumes **4 channels** (SELECT CH1-4, black keys F#3-C#4 mute, D#4-A#4 select, EDIT layer K1-K4 = their MIDI channels). The T-1 had 6 tracks. Is 4 right for v1? Recommend 4 (one per knob on the CHAN layer); 6 would need a second CHAN row and moves the select keys (a table edit). | P1-012 review |
| 32 | P1-012 UX | Keep Felucca's groovebox reachable as a set-and-forget legacy mode (MENU > SYSTEM > MODE `SEQ`/`GROOVE`, default `SEQ`), or remove it? Recommend keep for v1 (no code removed, the sequencer is the default). | P1-012 review |
| 33 | P1-012 UX | FX, ENV, LFO, ARP, SEQ are printed with synth names but act as dice buttons (labels on screen). Acceptable, or do you want a printed overlay/sticker set for the button block? Recommend screen labels only for v1, sticker as an optional extra. | P1-012 review |
| 34 | P1-012 UX | A preset load (PRESETS turn) is queued to the next loop start (immediate when stopped), and SAVE held 0.7 s undoes it. Acceptable? | P1-012 review |
| 35 | P1-012 UX | K1 at 0 = `KEYS ONLY` (only typed steps play) is how "enter steps instead of an algorithm" works. Acceptable, or should there be a fifth ALGORITHM choice `STEPS`? Recommend K1 = 0 (no new algorithm). | P1-012 review |
| 36 | P1-012 UX | Felucca's LFO-held MOMENTARY (knob moves snap back on release) is dropped in SEQ mode because LFO is now the SHAPE dice. Bring back a momentary-morph gesture after the first hardware test? | first hardware test |
| 37 | P2-002 system map | Remove the web editor (SysEx editor, backup/restore, `web/editor.html`) in v1? Recommend yes; the update path stays. | P2-002 review |
| 38 | P2-002 system map | Keep the CDC serial console (936 B) and USB audio of the preview in v1? Recommend keep both, revisit in v2. | P2-002 review |
| 39 | P2-002 system map | Drop the CONTROL-tab row `CHORD ENTRY` and chord.c (it targets groovebox step editing)? Recommend yes (CONTROL tab 4 rows). | P2-002 review |
| 40 | P2-002 system map | Preview patch: is ANALOG preset 0 acceptable as the one fixed boot patch, and does TUNE stay in the AUDIO tab? | P2-002 review |
| 41 | P2-002 system map | Keep MIDI learn (`LEARN CLEAR`, 458 B) in a sequencer, or remove it? | P2-002 review |
| 42 | P2-002 system map | Sequencer is permanent, so host tests must link the T-1 sources (now stubbed): make that a prerequisite task? Recommend yes. | P2-002 review |
| 43 | P2-004 ADR-0003 | Swing: odd steps start `2*(swing-50)`% of a step late (66 = triplet feel, 75 = dotted), instead of the T-1's `(swing-50)`%. OK? (a constant) | P2-004 review |
| 44 | P2-004 ADR-0003 | GATE 10..100 % defaults to 50 %: the ADR makes 50 % neutral (x1.0 of each algorithm's own gate, 100 % = x2.0 capped) via a table. Or should 100 % be neutral? | P2-004 review |
| 45 | P2-004 ADR-0003 | "Next loop start" for a queued preset load = every 16 steps (one bar at 1/16), since channels have different loop lengths. OK? Reviewer note: 16 steps is half a bar at 1/32 and four bars at 1/4; a constant of 768 ticks (always one 4/4 bar) is the DIV-independent alternative (one table constant, `SQ_BAR_STEPS` becomes `SQ_BAR_TICKS`). | P2-004 review |
| 46 | P2-004 ADR-0003 | Undo is one record per channel (the last dice / dice-all / pin-clear), tap again = redo, instead of one previous-seed per seed. OK? | P2-004 review |
| 47 | P2-004 ADR-0003 | An algorithm or MIDI-channel change does not cut sounding notes (pending note-offs go to the old channel); only stop, panic and USB reconnect cut. OK? | P2-004 review |
| 48 | P1-013 full control | The row switch is OCT- (ROW 1) and OCT+ (ROW 2): tap latches, hold and turn is momentary. This moves the keyboard octave to hold SCL + OCT-/OCT+ (setup, scores 7). Acceptable? Alternative: put the row switch on another button (a table row edit) and keep the keyboard octave on OCT. | P1-013 review |
| 49 (answered by P1-014: the RESET layer, hold UNDO) | P1-013 full control | There is no per-morph 'reset to default' gesture in v1 (a hold-a-knob gesture would collide with the layer holds); ROW 2 defaults are neutral. Add one after the first hardware test? | first hardware test |
| 50 | P1-013 full control | STAGE S1 `OFFSET` has no source parameter (the ML-185 follows the host position); it is there so S1 means 'where the loop starts' on all four algorithms. Keep, or give that slot to a ninth idea (for example the `LONG` hold odds)? | P1-013 review |
| 51 | P1-013 full control | The knob band grows from 40 to 52 px, so the sequence band shrinks from 80 to 68 px (labels need two rows of two text lines). Acceptable for the 16-step lane, or drop the value line of the dimmed row? | P2-003 |
| 52 | P1-013 full control | With DIR (backward / pendulum) active, ACID's slide flag means 'glide into this step from the step played just before', and a slide can cross the loop point in BWD. Acceptable? | first hardware test |
| 55 | P1-014 performance (revised) | Approve scheme B2, the printed-word layout: the dice stay where P1-012 put them (`FX` ALL, `SCL` NOTES, `SEQ` HITS, `ENV` ACCNT, `ARP` ARTIC, `LFO` SHAPE, all on press), `EDIT` = UNDO (hold = RESET layer), `GLO` = SET page, `REC` = RECORD (play the keys, they write), HOME acts on release. Honest scoring: the approved P1-013 scheme fails 25 of 56 functions, the first P1-014 version (layer-order rows) fails 5 of 54 once L is corrected, B2 fails 0 of 54 (with one classification change, question 60). Fallbacks: B (first version) or A (approved plus cues, 13 of 22 compared functions fail). | P1-014 review |
| 56 | P1-014 performance | White keys are the steps in PERF (no STEP mode, no live notes in PERF): a press pins and sounds the step; tap again clears; UNDO takes a stray pin back. Accept the risk that a hand brushing a white key sets a pin? The pitch workflow cost is answered in the revision: tap REC and the 27 keys play pitches and write them (the audition path), and hold a step key + press another white key = that scale degree; K1 NOTE per detent remains for fine edits. (A `KEYS LOCK` menu setting is possible as a table row if hardware testing shows brushing is a problem.) | first hardware test |
| 57 | P1-014 performance (revised) | After B2, five buttons still differ from their printing or fit it loosely: `EDIT` (undo), `OCT-`/`OCT+` (row switch, owner decision 48, the one hard override), the black keys (printed OP1..OP6 PIT GLD MONO POLY), and the fair fits ENV/LFO/ARP/SEQ/SCL/FX (L2). Accept screen labels plus the HOME-held HELP, or add a removable overlay strip (reverses decision 33)? | P1-014 review |
| 58 | P1-014 performance | Scenes: two (A, B) on C#5 / D#5, volatile (lost at power-off), recalled on the bar. Persist them with the preset slots (about 15 KB more pool, an E7 decision) and/or add a third scene by giving up ROLL? Tap tempo is given up as a key (BPM on SET K4 only) to make room for PAGE: acceptable? (Unchanged by the revision.) | P1-014 review |
| 59 | P1-014 performance | Pattern LENGTH (R1): the gestures are fixed (hold PAGE + press the last step key; SET row 2 K4). P1-016 must fix the range (assumed 1..64), the default (16) and how it relates to the algorithm `LEN` / `TOTAL` morph. Per-step performance (R3) is scored as step editing (input 1: hold a step key, turn a knob); a latched step cursor would score it as a performance function but adds a mode. Keep hold-and-turn? | P1-016 |
| 60 | P1-014 revision | ROW 2 morphs (START, ARTIC, DYNAMICS, READ) are input 2 (tap OCT+, then turn) and L2, total 10, so they cannot pass as performance functions by honest scoring. B2 classes them as setup dial-ins (pass at 10 against 9). Accept, or move the four morphs you perform most onto ROW 1 (a table edit of the knob table)? | P1-014 review |
| 61 | P1-014 revision | HOME now acts on release: tap = home, hold + touch a control = HELP (no time window), release after 0.7 s with nothing touched = MENU (about 0.1 s later than stock). Accept? | P1-014 review |
| 62 | P1-014 revision | MUTATE (hold to re-roll everything, snap back on release) is removed: scored I2 / L1, and REC is now record. Dice + EDIT tap does "try and take back". Bring it back after the first hardware test? It needs a free control (today none). | first hardware test |
| 63 | P1-014 revision | REC writes chromatic, unquantised pitches (a note outside the scale is allowed) and, while playing, writes to the step nearest the playhead. Accept, or snap pitches to the global scale and to the next step? | first hardware test |
| 64 | P1-014 revision | The hold + press pitch gesture edits ONE step at a time (the first version let all held keys receive one edit). Accept dropping multi-select? | P1-014 review |
| 65 | P1-017 sequencer UX lock | NOTE EDIT chance has two levels (100 / 50 %) on LFO so it stays one press. Accept, or a four-level PROB on a knob (pushes RATCH onto button presses; a 4-press cycle fails the 2-gesture rule)? | P1-017 review |
| 66 | P1-017 | A new page is an exact copy of the page BEFORE it (A, A', A''): its edits and its source (the generator block it reads; OWN lets the algorithm continue). Confirm. | P1-017 review |
| 67 | P1-017 | A step's SLIDE means 'this note glides into the next step' (TB-303 rule; after the last step into step 1, across the loop end; no glide if that step is silent). Confirm on the TB-3 (with question 8). | first hardware test |
| 68 | P1-017 | Force sensing ships behind the HT-FORCE hardware gate, FORCE OFF until it passes, in Velo's DRIVE mode (+~2.2 ms on a REC note); v1 set = hard-tap accent (scored 9, kept as optional extra E3), pressure ROLL / FILL, REC velocity, each with a fallback. Accept this set? | P1-017 review |
| 69 | P1-017 | Scale, key and octave on a SCALE page opened by HOLDING GLO (SET stays on the tap), not by holding SCL (which would delay the NOTES dice to the release). Accept? | P1-017 review |
| 70 | P1-017 | SNAKE's long polymeter cycles are cut at the channel length (max 64 steps) so the lane always shows what plays. Accept? | P1-017 review |
| 71 | P1-017 | Exception E1: a preset turned while playing lands on the first bar that starts at least 1.5 s (SQ_LOAD_SETTLE) after the last detent, so a SAVE tap into the pending slot never races the bar (decision 34 plus revision 2; O1 by the letter of the rubric). Keep (recommended by the reviewer and the revision), or load at the first step after the settle (SQ_LOAD_AT = STEP, scores 11)? | P1-017 review |
| 72 | P1-017 revision | The L rule is restated (SEQUENCER_UX 8.1): a role reached through a hold scores L2 when the hold is named permanently on the page where it starts (new `h:` sub-labels on PERF) and opens a labelled page; otherwise L1 as in P1-014. Accept? Under the P1-014 letter 23 setup functions fall to 9 (43 of 69 pass; revision 2 adds U18 step octave, reached only through the step hold); under the restated rule 66 of 69 pass with three listed exceptions (E1 preset on the bar, E2 HOME-held HELP per decisions 57/61, E3 hard-tap accent). | P1-017 review |
| 73 | P1-017 revision | Default MIDI channels CH1..CH4 = 2, 3, 4, 5 (channel number plus one), so CH1 reaches a TB-3 at its factory channel with no setup; any channel changes in 2 gestures (D#4, K2). Accept, or keep 1..4 and make first-run step 1 'D#4 K2: MIDI CHANNEL'? ANSWERED 2026-11-10: keep 1..4 plus the first-run prompt (decision 73). | answered |
| 77 | P2-011 (scenario finding F-14, FINDINGS Q4) | Two unmuted channels on the same MIDI channel (CH1 set to MIDI 2 for the TB-3, CH2's default is 2): warn, allow, or refuse? ANSWERED 2026-11-10: MIDI channels are UNIQUE across the 4 sequencer channels, clashes are impossible. Giving a channel a MIDI channel another sequencer channel already uses REMOVES it from that other channel: its MIDI channel becomes OFF (no MIDI output), shown clearly on screen and in a toast naming the displaced channel; undo restores both as one undo record. Defaults stay CH1..CH4 = 1..4. | answered |
| 78 | P2-011 (scenario finding Q1) | Is VISION rule 1's 'one press, hold or turn away' met by 'at most two gestures' (R-5)? ANSWERED 2026-11-10 (option a): setup functions (MIDI channel, length, tempo, scale, first ROW 2 morph, edit a note, add a page, save to another slot, reset one morph, ask what a control does) may take 2 gestures; performance functions stay at 1. Recorded in VISION.md rule 1. | answered |
| 79 | P2-011 (scenario finding F-15) | **Turning PRESETS to a slot that was never saved.** What loads? Options: (a) nothing: the pattern is unchanged, the header returns to the current slot and the toast says `P09 EMPTY: NOTHING LOADED`; a SAVE tap while `P09?` is pending still fills it (recommended: a stray detent cannot wipe the music); (b) the factory defaults of ADR-0003 12 load (a fast reset to a blank start, but one detent too many destroys the current pattern, undone by SAVE hold). | **Decided 2026-11-10: (a)** nothing loads; toast `P09 EMPTY: NOTHING LOADED` |

## Decisions (owner, 2026-11-09)
| # | Decision |
| --- | --- |
| 1 | P0-005 flash: owner will flash in the morning (2026-11-10). Task restored to todo, human-run. |
| 2 | ACID gets slide as a sixth dice layer in v1 (task P1-011). |
| 3 | Scale is ONE control, used by every algorithm; global by default (not per-channel). |
| 4 | GATE, DIV, SWING (and scale) are GLOBAL performance settings on the panel, not per channel. A channel still has only its MIDI channel and algorithm. |
| 5 | Branch t1-hardware-bringup pushed to origin (jkeaney/fm1-t1). |

## Decisions (owner, 2026-11-10)
| # | Decision |
| --- | --- |
| P1-010 | Algorithm set NOT approved as-is: approve with changes. A whole rethink of the hardware buttons and knobs is needed first (task P1-012, Opus), and P1-010 is re-approved together with it. |
| S1 | Slide on the ARTIC button, decided inside the UX review (P1-012), not separately. |
| S2 | SHAPE button naming/scope: decided inside P1-012. |
| S3 | Knob fit (ACID TYPE on K2, SNAKE PATH on K4): decided inside P1-012. Every knob must align to the hardware. |
| S4 | Scale snap rule: decided inside P1-012 (UX review). |
| S5 | Each algorithm adheres to ONE global scale and key that can be set on the device (a panel control, not a menu dive). Goes in the UX review. |
| S6 | Defaults (minor pentatonic, SNAKE RANGE 12): confirm during the UX review. |
| S7 | Per-algorithm morph values and seeds are KEPT when a channel switches algorithm (switching back restores). |
| P | Push approved and done. |
| F | Flash approved (P0-005), run with the owner. |

## Decisions (owner, 2026-11-10, second round)
| # | Decision |
| --- | --- |
| P1-010 / P1-012 | APPROVED together (4 algorithms ACID/EUCLID/SNAKE/STAGE; new control scheme, 249/270). |
| 31 | 4 channels for v1. |
| 32 | Keep Felucca groovebox as a legacy menu mode in v1; PLAN a v2 cleaned-up firmware (epic E9) that removes it and other unused subsystems. |
| 33 | Screen labels only for the dice buttons; no stickers. |
| 34 | Preset load queued to next loop start; SAVE hold 0.7 s undoes. |
| 35 | K1 at 0 = KEYS ONLY (no fifth algorithm). |
| 36 | Momentary-morph gesture: revisit after the first hardware test. |
| P0-005 | Listen test on the TB-3 deferred (device is flashed with FM-1_900); task stays open. |
| Push | Approved. |

## Decision (owner, 2026-11-10, third round)
| # | Decision |
| --- | --- |
| Mem | If we run out of RAM or flash, re-prioritise the groovebox clean-up (epic E9) ahead of the remaining feature work. Trigger: `tools/check.sh` fails its ram stage, or headroom falls under 8 KB (the hard floor is 4 KB), or flash/pool no longer fits. The orchestrator then files a high-severity BUG, raises E9 to priority 0, and plans the removal before continuing. |

## Decision (owner, 2026-11-10, fourth round): supersedes #32
| # | Decision |
| --- | --- |
| 32b | The groovebox is completely OUT of the UI in v1. The device boots straight into the new sequencer UI and works immediately (no setup, no mode choice). No MODE GROOVE menu row. Groovebox-only subsystems are removed (or compiled out) in v1 per the system map; E9 only cleans up what could not be removed safely. Follow-up edits needed: remove the MODE GROOVE mention in UX_REVIEW.md / CONTROL_MAP.md / control_map.tsv; add 'boot straight to a working sequencer' to the screen spec (P2-003) and architecture (P2-004). |

## Superseding note (2026-11-10, P2-002)
| # | Decision |
| --- | --- |
| 32 (superseded) | The groovebox is NOT kept as a legacy menu mode: it is completely out of the UI in v1 (device boots straight to the sequencer, no MODE row). Groovebox-only subsystems are REMOVE in v1; v2 (E9) is the clean-up of what v1 could not safely remove. docs/design/SYSTEM_MAP.md follows this; CONTROL_MAP / UX_REVIEW still mention `MODE SEQ/GROOVE` and need a one-line update. |

## Decisions (owner, 2026-11-10, fifth round)
| # | Decision |
| --- | --- |
| P2-002 | SYSTEM_MAP approved. |
| 37 | Remove the web editor in v1 (update path stays). |
| 38 | Keep the CDC console and USB audio in v1. |
| 39 | Drop CHORD ENTRY and chord.c (CONTROL-tab 4 rows); CONTROL_MAP/UX_REVIEW/TSV edited by task P2-009. |
| 40 | ANALOG preset 0 is the fixed boot preview patch; TUNE stays in the AUDIO tab. |
| 41 | Keep MIDI learn. |
| 42 | Add a prerequisite dev task: host tests link the real T-1/sequencer sources (P2-005 must create it first in the removal chain). |
| P0-005 | Listen test deferred. The owner will connect the TB-3 once the work here is done to test the sequencer interface: control of steps, notes, slides, accents etc. P2-005 must plan hardware-test gated tasks with a TB-3 checklist covering exactly those (steps, notes, slides, accents, morph, dice). |
| Push | Approved. |

## Decision (owner, 2026-11-10, sixth round): full algorithm control
| # | Decision |
| --- | --- |
| Shift | Every algorithm needs FULL control. A SHIFT for the knobs is approved: it doubles the usable knobs (K1-K4 plus a shift row of 4 = up to 8 morphs per algorithm), and the screen instructs the change live (both rows of labels always visible; the active row and the shift state are unmistakable). This supersedes the 4-morph cap and the 'no shift page' stance. Easy access is the requirement: the shift must score >= 8 on the ease rubric with no sub-score below 2. Task P1-013. |

## Decisions (owner, 2026-11-10, seventh round): ADR-0003 questions
| # | Decision |
| --- | --- |
| 43 | Swing doubled: odd steps start 2*(swing-50)% of a step late. |
| 44 | GATE 50% is neutral (x1.0 of each algorithm's own gate); the screen labels it as a share of the algorithm's gate, not of a step (P2-003 must do this). |
| 45 | Queued preset load takes effect on a fixed 768-tick bar. |
| 46 | One undo record per channel; tap again = redo. |
| 47 | Algorithm or MIDI-channel change does not cut sounding notes; only stop, panic and USB reconnect do. |
| Push | Approved. |
(ADR-0003 itself is approved together with P1-013.)

## Decisions (owner, 2026-11-10, eighth round)
| # | Decision |
| --- | --- |
| P1-013 / P2-004 | APPROVED together (OCT-/OCT+ row switch, 8 morphs per algorithm, ADR-0003 as amended). |
| 48,50,51,52 | As recommended (OCT-/OCT+ rows; keep STAGE OFFSET; 52 px knob band; ACID slide under BWD accepted, test on TB-3). |
| 49 | CHANGED: design the per-morph reset gesture NOW, as part of a performance-first design pass (task P1-014). |
| Perf | Design for performance. All knobs accounted for; the panel buttons align to the algorithm controls; the keyboard aligns to performance features and step controls; all within the UX guidelines and scored with the ease rubric. Controls available, perform, fun to play. The user must never feel lost and must be able to play it WITHOUT a manual. P1-014. |
| Push | Approved. |

## Requirements (owner, 2026-11-10, ninth round): pattern length, pages, per-step performance, keybed expression
| # | Requirement |
| --- | --- |
| R1 | Each channel has its own pattern length (step count to fill), not fixed at 16. This amends VISION rule 3: a channel's settings are its MIDI channel, its algorithm and its pattern length (plus anything the design proves necessary and scores well). |
| R2 | Viewing a channel, the 16 white keys align to its steps. The pattern is PAGINATED by its step length (page 1 = steps 1-16, page 2 = 17-32 ...), with a way to swap pages; the screen always shows the page and the playhead. |
| R3 | Selecting a step has performative functionality: per-step ratchet, slide, mute, accent, probability etc. |
| R4 | Use the keybed for expression: reference the open-source project that unlocked MPE-like capability from the FM-1 keybed, so playing a key gives performative options (ratchet, slide, mute ...). The project has not been identified yet (a first web search did not find it): task P1-015 identifies it and the keybed hardware capability; owner to confirm which project. |

## P1-015 findings (2026-11-10): keybed expression (R4)
| # | Question |
| --- | --- |
| 53 | The project that reads force from the FM-1 keybed is Velo (charlesvestal/fm1-velocity, GPL-3.0) and its shipped successor CTL-1 (charlesvestal/fm1-ctl, MPL-2.0; also used by MONK-1 and Scintilla). It gives velocity and polyphonic aftertouch via the ADC, not MPE; Felucca MPE is MPE input only. Is Velo the project you meant? |
| 54 | Force sensing covers F3-C5 only (20 keys), is uneven across a chord, and Velo's author parked it (CTL-1 ships it, with caveats: edge presses read soft, LEDs disturb readings). Recommendation: v1 uses tap-count and hold gestures (certain, all 27 keys); force is a later optional port. Agree? See docs/design/KEYBED_EXPRESSION_REFERENCE.md. |

## Decisions (owner, 2026-11-10, tenth round): rethink of the sequencer UX
| # | Decision |
| --- | --- |
| 55 | B2 approved as the BUTTON LAYOUT (printed-word fit). |
| 57 | Screen labels plus hold-HOME help; no overlay strip. |
| 60 | Move the four most-performed morphs to ROW 1 (knob table edit, done in P1-017). |
| 61,62 | HOME acts on release; MUTATE stays removed. |
| 56,63,64 | RETHINK (not accepted as designed): the device must be easy and never need deep learning or controls. The sequencer model is now: dial an algorithm in, then control the steps that were set: turn steps on/off, morph a step; set the first 16 steps, then duplicate them into the second 16 and change those, and so on up to 64 steps; hold a step to open a NOTE EDIT page where the knobs change pitch etc. |
| 58 | NO scenes and NO tap tempo. Tempo is set on the global settings page. Spend the effort on easily entering and sequencing up to 64 steps from a 16-step pattern. |
| 59 | Pattern length and its gestures need a whole rethink (as above). |
| 53,54 | Keybed force sensing (Velo / CTL-1) IS wanted in v1 ('this is it'): the next design pass must make excellent use of velocity and polyphonic aftertouch. |
| Process | Framework for the next pass: every state is a page; each button has a mode (press, hold ...); each state has a screen; each screen shows what the knobs control; input, processing and output all aligned and mapped; held to a high bar of usability, understanding, UX, performance, fun and musical output. An Opus UX-designer / technical-architect pass (P1-017) reviews this feedback and LOCKS the sequencer functionality and UX. P1-016 is superseded by P1-017. |
| Push | Approved. |
| 65-70 | P1-017 questions 65-70 accepted as written (65 NOTE EDIT chance two levels; 66 a new page is an exact copy of the page before it; 67 SLIDE glides into the next step; 68 force sensing behind the HT-FORCE gate with the v1 set; 69 SCALE page on HOLD GLO; 70 SNAKE cycles cut at the channel length). |
| 71 | Keep the bar-aligned preset load: a preset turned while playing lands on the first bar that starts at least 1.5 s (SQ_LOAD_SETTLE) after the last detent (exception E1 stands). |
| 72 | Accept the restated L rule (SEQUENCER_UX 8.1): a role reached through a hold scores L2 when the hold is named permanently on the page where it starts and opens a labelled page. |
| 73 | REJECTED the 2..5 default. Default MIDI channels stay CH1..CH4 = MIDI 1..4 (channels aligned with channel numbers), and step 1 of the first run is 'D#4 K2: MIDI CHANNEL', a visible prompt on the main screen. VISION.md keeps its original wording. |
| 74 | v1 sends MIDI clock out (FM-1 as master): YES; see FB-001. |

| 75 | P2-003 screen spec | Approved. Tempo source AUTO (internal default with own tempo kept separately; a heard MIDI clock becomes the source; back to internal when it stops); K1 inert in EXT; clock out automatic, no menu row; PLAY on EXT waits for the clock to start; clock/source shown by blinking colour-coded dots; hold-mark glyph accepted | owner 2026-11-10 | decided, to be specified in P2-010 |
| 76 | P2-010 timing spec | Approved with all recommended answers: CLOCK row = AUTO / INTERNAL ONLY; clock lost while playing stops; PLAY again cancels WAIT; host Start takes over a playing FM-1; PLAY LED mid-level with beat flash; timing test is a separate build; LEAD 2.90 ms | owner 2026-11-10 | decided |
| 77 | Q4/F-14: MIDI channels are unique across the 4 channels; assigning one that another channel uses sets that channel to OFF (screen label and toast name it; one undo record restores both). Defaults stay 1..4. |
| 78 | Q1: setup functions may take 2 gestures; performance functions stay 1. Closed; written into VISION.md rule 1. |
| 80 | Memory (2026-11-10, open-items 1C): v1 keeps **16 preset slots**. The fx-bus removal moves to the **front of E3**, the first dev work after P2-005, so the pool is freed before HARMONY/ARP/PATTERNS/preset storage land. |
| 81 | PATTERNS categories (open-items 2C): the research decides the categories. The owner wants **famous-song patterns** as well as **classic genre patterns** (house bass, funky bass, tech house bass, rolling bass and the like). See P2-014. |
| 82 | v1 has **six algorithms**: ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS (open-items 3A). E5 splits into two dev tasks; P2-013 and P2-014 must show the algorithm picker still fits the screen. |
| 83 | Ableton Sync check on the JIANT FM port and the P0-005 TB-3 listening test: owner runs both together on 2026-11-11 on the build already flashed (open-items 4B, 5A). |
| 84 | Keybed force sensing stays in v1 (open-items 6A), but needs **strong use cases**: what each gesture does to the sound, the input UX, and what it changes in the MIDI output. New design task P2-015 before P2-005. |
| 85 | P2-012 | HARMONY default phrase length: A) 4 bars (`1---`), B) 5 bars to match the literal `1----`. Recommend A. **Decided 2026-11-10: A (4 bars)** |
| 86 | P2-012 | (partly reverses decision 69) HARMONY one-gesture page costs: the keys become chord keys (F3-E4 = 1-7), so "press any key = KEY" and the 27-key picture go (KEY is K2) and OCTAVE moves to OCT-/OCT+ on SET. A) accept, B) keep key-press = KEY on the 20 other keys. Recommend A. **Decided 2026-11-10: A** |
| 87 | P2-012 | ACID's ROW 2 morph is also called `PEDAL`. A) keep both, B) rename the HARMONY one `DRONE`. Recommend A. **Decided 2026-11-10: B (rename the HARMONY dial to `DRONE`)** |
| 88 | P2-012 | FULL role is monophonic (contour mapped onto the voiced notes). A) monophonic in v1, B) polyphonic chords in v1 (grows `ChanRT.snd[2]` and the event queue). Recommend A. **Decided 2026-11-10: A (monophonic in v1)** |
| 89 | P2-012 | ROLE (OFF AUTO ROOT FULL) is a per-channel setting beyond VISION rule 3. A) accept as a harmony-page setting, B) drop it. Recommend A. **Decided 2026-11-10: A** |
| 90 | PATTERNS famous-song patterns (open-items 1B): ship the exact notes under style names (for example `FUNK 3`), never song titles; references only in RESEARCH.md. |
| 91 | P2-012 | Extends decision 46: EDIT tap also undoes the last HARMONY change (the later of channel and HARMONY wins). A) accept, B) channel only. Recommend A. **Decided 2026-11-10: A** |
| 92 | P2-013 | ARP design locked (open-items 2A): ASPL/CHORD names kept; ADR-0003 amendment for the four core additions to be written as a follow-up task | owner 2026-11-11 | decided |
| 93 | P2-014 | PATTERNS design locked (open-items 3A): categories and tags confirmed; the 13 idiom patterns accepted and judged by ear on live hardware | owner 2026-11-11 | decided |
| 94 | P2-015 | Keybed use cases locked with UC1-UC4. UC5 (REC channel aftertouch) parked to v2; core workflow first | owner 2026-11-11 | decided |
| 95 | P2-015 | TB-3 accent threshold test H0 is run by ear in the P0-005 flash session; the flat velocity 100 vs 90 is set from the result | owner 2026-11-11 | decided |
| 96 | New | Hardware support model: the sequencer drives per-channel hardware parameters (CC, accents, slides, filter, resonance on dice and morph), starting with the TB-3. Owner wants a high-level design and UX first, then a lower-level design, then TB-3 specifics. Reference: midi.guide (CC BY-SA 4.0), cdm.link AIRA article | owner 2026-11-11 | decided, see 97-100 |

| 97 | P2-018 | Hardware support UX = Option B (third knob row on PERF + RIG page via hold CHN). CONDITION: the screen must show clearly that there are three knob rows, show the change when the row changes, and always show which row/page the user is on | owner 2026-11-11 | decided |
| 98 | P2-018 | A channel may carry an optional device (RIG); amends VISION rule 3; inert with no device | owner 2026-11-11 | decided |
| 99 | P2-018 | Profile data is our own (built-in table + GENERIC); midi.guide is a cited reference only. Docs and profiles say hardware is owned by its users, not the manufacturer, and credit community-gathered data with per-profile provenance | owner 2026-11-11 | decided |
| 100 | P2-018 | Questions F-M accepted as recommended: STOP parks SEQ wires at LEVEL; 4 wires; undo = seeds only; no per-step lock in v1; chosen device starts wires in HAND; SEQ-wire cap 8; no REC of ROW 3 in v1; phase 2 = low-level design of the model and Option B | owner 2026-11-11 | decided |
| 101 | P2-019 | Hardware low-level design locked; questions A-H as recommended (ROW 3 via two OCT+ taps; MOVE on RIG page; compact non-live rows; DEV leaves values; 5-letter names; ride dot; mute parks SEQ wires; tap cycles / hold selects) | owner 2026-11-11 | decided |
| 102 | P2-019 | fx-bus removal (P3-014) precedes preset storage; v1 keeps 16 slots (SQ_NSLOT=8 only a fallback) | owner 2026-11-11 | decided |
| 103 | P2-020 | TB-3 specifics start after the H0 accent-threshold test | owner 2026-11-11 | decided |
| 104 | P2-015 / H0 | TB-3 accent threshold measured by ear 2026-11-11 (notes at 80, 90, 95, 100, 105, 110, 120 on ch2): 80, 90, 95 do NOT accent; 100 and above DO. So velocity >= 100 accents. The flat non-accent velocity in the algorithms must be 90 or lower (not 100); accent stays 120 (or >= 100). [V] on this unit | owner 2026-11-11 | decided; design fix routed |
| 105 | P2-020 / TB-3 | Slide and accent measured by ear on the owner's TB-3, 2026-11-11 (notes on ch2 via rtmidi): next note-on BEFORE the previous note-off = SLIDE (pitch glides, no second envelope) [V]; a gap between notes = two separate notes [V]; velocity >= 100 accents (decision 104) [V]; an accent (vel 120) on the note slid INTO sounds the same as unaccented [V on this unit]; so the profile slide method is OVERLAP and accent on a slide destination does nothing audible. Unit: owner's TB-3, firmware version not recorded | owner 2026-11-11 | decided |
| 106 | P2-020 / H-RIG1 | CC receive validated by ear on the owner's TB-3, 2026-11-11, ch2, slow 0-127-0 sweeps under a repeating note: ALL 11 CCs work [V on this unit]: 74 cutoff, 71 resonance, 12 env mod X, 13 env mod Y, 16 accent, 17 effect, 104 tuning, 68 scatter type, 69 scatter depth, 11 pad Y, 1 pad Z. Owner notes: scatter type (68) is a MODE SELECT, scatter depth (69) is the depth of that effect; the two are linked. The key design work is aligning these into the sequencer so the user controls the ones they need and want | owner 2026-11-11 | decided; design follow-up P2-021 |

| 107 | P2-021 | Owner review of P2-021 sheet 2026-11-11: 3A, 4A, 7A, 8A, 9A accepted; 5B (scatter zone count is not a priority; ship the placeholder). 1, 2 and 6 REDIRECTED: not "wire sets on ALGO" but a new MODULATION LAYER, designed as part of the algorithm engine (not a bolt-on to the RIG/ROW 3 wires). Requirements: (a) global settings AND channel settings; each channel has a device; each device has its controls mapped, in the device's real parameter names, easy to set up; (b) device knobs keep working: we only push our values. Check whether CC is remote-control or mod-control, or full control only; (c) alongside pattern gen, note gen and quantise, add a modulation gen: per-step modulation or an LFO, for the knobs; (d) the knobs are NOT external controls: pick per device which params are sequenced, diced and morphed; (e) revisit docs/snake: snakes drive pitch and velocity AND a mod snake can map to another param (e.g. filter). Model this for EVERY algorithm: each algo's pattern/engine gets new mod lane(s) for what is externally controlled; (f) setup UX: about 4 slots (sensible, not everything); page 3 becomes the control-setup page: K1 sets slot 1 (turn to select the filter), K2 sets slot 2 (turn until you find the effect), etc. Routed to P2-022 | owner 2026-11-11 | decided; redesign P2-022 |
| 108 | P2-022 | Modulation layer design (docs/design/MODULATION.md) locked; questions 1-9 all as recommended: LFO dice re-rolls SHAPE and mod seed together; DEPTH = hold OCT+ and turn; fixed lane per slot (odd = step, even = sweep); ALGO picks device on page 3; start depth 0; VISION rule 3 reworded (owner edits VISION.md); SCTYP uses two adjacent slots; cap 8 active slots; select changes on bar lines only. Supersedes the wire/ROW 3 design of 97-101 as set out in MODULATION.md 5.3 | owner 2026-11-11 | decided |
| 109 | Open-items 2026-11-11 | Owner answers: push after BUG-003/P3-014 pass review (1B); Ableton Sync check goes in the HW-0 hardware session (2A); fx-bus removal sound change accepted (3A). NEW REQUIREMENT: this device is a sequencer for external gear, and each channel gets a PREVIEW mode that plays its pattern live through the speaker as well as sending MIDI. Ship a handful of demo presets: ACID, LEAD, BASS, PAD, STRINGS, PIANO. Routed to design task P2-023; engine-removal tasks P3-008..P3-010 (DRUM/PHYS/sample/remaining engines, ANALOG only) are held until P2-023 says which engines the six presets need | owner 2026-11-11 | decided; design P2-023 |
| 110 | P2-023 | Preview design answers 2026-11-11: 1A (PIANO = ANALOG e-piano approximation; no sample/FM6 engine kept, so the ANALOG-only removal chain P3-008..P3-010 proceeds); 2 ALL channels preview OFF by default; 3A, 4A (MASTER pot is the preview level), 5A, 6A; 7: preview and MIDI are always the SAME output (whatever a channel sends as MIDI the preview plays; no separate preview behaviour); 8A (amend ADR-0003 and SEQUENCER_UX 4.4). ADDED: (a) a really easy per-channel on/off switch for preview on the device, and an easy per-channel preset change; (b) the internal synth maps everything the channel is set up to send: notes, accents, slides, gates, filter and the other device CCs, best effort. Routed to P2-024 | owner 2026-11-11 | decided; amend P2-024 |
| 111 | P2-024 | Owner 2026-11-11: 1B, preview is INDEPENDENT of MIDI out: a channel can preview with MIDI muted/OFF, like a DJ CUE system (headphones/speaker preview of a pattern, dial it in, hear how it fits with the others, then unmute the MIDI or turn up the synth). Amends 110's "one output" to: same events, separate destinations (MIDI gate and preview gate independent). 2A, TB-3 controls with no equivalent stay unmapped, BUT TUNING (CC 104) IS pitch: map it to a synth pitch offset. P2-024 stays in review until P2-025 lands | owner 2026-11-11 | decided; P2-025 |
| 112 | P2-025 | Owner 2026-11-11 (answer to the one-shared-mix question, effectively 1A): the FM-1 has a master output via the speaker AND a headphone jack at the back. Preview simply always outputs every channel whose preview is ON, to the speaker or the headphones (whatever the device routes the master to). No cue bus is needed. The per-channel mute controls are the MIDI out gate. P2-024 and P2-025 approved with this clarification; PREVIEW.md must state the headphone jack as owner-confirmed hardware (not "undocumented") | owner 2026-11-11 | decided; P2-027 |
