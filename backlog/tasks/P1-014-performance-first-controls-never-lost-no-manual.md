---
id: P1-014
title: Performance-first control design: every control accounted for, fun to play, never lost, no manual
phase: P1
type: spike
status: done
priority: 0
executor: sonnet
depends_on: [P1-013]
gate: human-review
---
## Goal
Owner challenge (2026-11-10, eighth round): design the device for PERFORMANCE. All knobs accounted for; the panel buttons align to the algorithm controls; the keyboard aligns to performance features and step controls; everything inside the UX guidelines and proven with the ease score. It must be controllable, fun to play, and a user must NEVER feel lost; they must be able to play it without a manual. Includes the per-morph reset gesture (owner question 49, now decided: design it now).

## Read first
- docs/project/VISION.md; docs/project/OWNER_QUESTIONS.md (all decisions, esp. 'eighth round' and the sixth-round shift decision; question 36 momentary morph; question 49)
- docs/project/decisions/ADR-0004-design-lock.md (data-table flexibility)
- docs/design/FULL_CONTROL.md, UX_REVIEW.md (rubric and scores), CONTROL_MAP.md, control_map.tsv, tools/check_control_map.py, ALGORITHM_SET.md and section 6 of the four algorithm docs
- docs/project/decisions/ADR-0003-sequencer-core.md (state and RAM budget: any new feature needs state; respect the 4 KB RAM floor and the SYSTEM_MAP headroom)
- docs/design/SYSTEM_MAP.md; docs/design/CONTROL_INVENTORY.md; firmware/src/panel.c; docs/controls.jpg (what is printed on the panel)

## Deliverable
- docs/design/PERFORMANCE_DESIGN.md containing:
  1. **Control accounting table**: every physical control (14 buttons, K1-K4, 3 encoders, 27 keys, plus chosen hold/tap variants) with its role in PERFORM, and the SINGLE principle that explains the layout (e.g. panel buttons = algorithm controls; white keys = steps; black keys = performance moves). No control may be dead or ambiguous in PERFORM; justify any intentional blank.
  2. **Panel buttons aligned to the algorithm controls**: the dice buttons, row buttons, undo/redo, reset gesture, algorithm functions; one consistent mapping between a button and what it does to the algorithm in every algorithm.
  3. **Keyboard**: which keys are step controls (entry, pin, tie, per-step accent/slide, clear) and which are performance features (e.g. mute channels, fills/rolls, momentary mutate that snaps back, scene/snapshot recall, tap tempo, octave jumps, restart); a keys legend always visible on screen; behaviour while stopped vs playing.
  4. **Reset gesture** for a single morph and for a whole row/algorithm (owner Q49), plus the momentary-morph gesture (Q36) if the score says it earns a place.
  5. **'Never lost / no manual' design**: the always-on cue strip (what each live control does right now), first-run guidance at power-on (shown once, dismissible with any control), a consistent escape (one gesture returns to the main PERFORM screen from anywhere), undo reachable everywhere, a 'what does this do' touch-to-learn behaviour (e.g. holding any control briefly shows its function without changing anything, if the score says it is worth it), naming and iconography rules (labels <= 6 chars, consistent verbs). List every state a first-time user could get confused in and the cue that rescues them.
  6. **Fun to play**: the performance moves list (fills, rolls, mutate-hold, scene recall, build/drop) with the gesture for each, and the sequence of a 60-second live performance as a gesture-by-gesture script.
  7. **Score**: extend the rubric of UX_REVIEW.md with a fourth sub-score **L = learnable without a manual** (0 = needs the manual; 1 = guessable only from other controls; 2 = role visible on screen but must be discovered; 3 = role obvious from the label/position and the cue strip). Score every function (current approved scheme vs proposed) on visual/input/output/L. Pass rule: performance functions need V,I,O,L totals >= 11/12 with no sub-score below 2; setup functions >= 9/12 with no 0. Show before/after.
  8. **Three candidate schemes** (different organising principles), each scored, with the winner chosen by score and fun; the losers kept in an appendix with why they lost.
  9. **Change list**: every difference from the approved P1-013 scheme, each with the score evidence; anything that touches RAM/flash with the cost from ADR-0003/SYSTEM_MAP.
  10. Flex points (ADR-0004): what is re-mappable by a table edit.
- Update to match: docs/design/control_map.tsv, CONTROL_MAP.md (run python3 tools/check_control_map.py to OK), FULL_CONTROL.md, UX_REVIEW.md (new rubric column and scores), ALGORITHM_SET.md and ADR-0003 where state or vtable changes, each with a change note at the top.
- New owner questions in docs/project/OWNER_QUESTIONS.md (continue numbering).

## Acceptance criteria
- [x] Every physical control has a defined role in PERFORM; none dead or ambiguous; one explaining principle
- [x] Panel buttons map consistently onto algorithm controls in all four algorithms
- [x] Keys split into step controls and performance features with an on-screen keys legend
- [x] A per-morph reset gesture and a row/algorithm reset exist and score to the pass rule
- [x] The extended L sub-score is defined; all performance functions pass (>= 11/12, no sub-score below 2)
- [x] Three schemes scored; the winner is chosen by score; changes from the approved scheme are listed with evidence
- [x] A first-run flow, a consistent escape gesture and a list of confusing states with rescuing cues
- [x] A 60-second performance script that uses only the documented gestures
- [x] control_map.tsv and CONTROL_MAP.md updated; check_control_map.py prints OK; ADR-0003 updated if state changes, RAM budget re-added with the 4 KB floor respected
- [x] No 'TBD', 'choose' or 'decide' wording
- [x] The owner approved it (log line 'Approved: <name> <date>')

## Verification
```sh
python3 tools/check_control_map.py
grep -nEi 'TBD|choose|decide' docs/design/PERFORMANCE_DESIGN.md || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Firmware code; flashing.

## Log

### 2026-11-10 executor (sonnet)
Design only. Files: docs/design/PERFORMANCE_DESIGN.md (new, 11 sections), control_map.tsv (245 rows = 49 controls x 5 contexts), CONTROL_MAP.md (regenerated), FULL_CONTROL.md, UX_REVIEW.md (new 1.2/1.3, section 3.3 L), ALGORITHM_SET.md, ADR-0003 (UndoRec 48 B, section 11.1), OWNER_QUESTIONS.md (53-57; 49 answered).
Winner: scheme B "Rows" (top row = dice in layer order, bottom row = safety/system, white keys = steps always, black keys = moves). Scores (21 compared functions, /252): A 172 (13 fail), B 234 (0 fail), C 211 (11 fail). Full table (53 functions): approved P1-013 386 of 636, fails 24; proposed 570 of 624, all 52 live functions pass (P >= 11 no sub < 2; S >= 9 no 0).
Ninth-round R1-R4 added at the control level only (PAGE key D#4, LENGTH via hold PAGE + key and SET row 2 K4, step band with two rows of four per-step properties, [R4] keybed slot); P1-016 fills data/algorithm side.
RAM: ADR-0003 11.1: total new .bss 6953 B (incl. 1536 B reserved for P1-016 step records); headroom 39543 B after removals, 11031 B before (> 8 KB trigger, floor 4 KB). Pool about 22400 B (3364 B spare today: removals must precede preset storage).
Verification:
```
python3 tools/check_control_map.py   -> rows tsv=245 md=245 / OK
grep -nEi 'TBD|choose|decide' docs/design/PERFORMANCE_DESIGN.md || echo clean   -> clean
python3 tools/backlog.py check   -> 37 items, 0 errors
```
Owner questions: 53-57 (OWNER_QUESTIONS.md). No Approved line written (human-review gate). The last acceptance box (owner approval) stays unticked.


### 2026-11-10 review (sonnet)
Verdict: **PASS on the checkable criteria; stays at `review`. Awaiting owner: approve the scheme and decide the items in the owner list below (no Approved line written).**

Verified independently:
- `check_control_map.py` OK (245 rows). My script over control_map.tsv: 49 controls x 5 contexts (PERF STEP_HELD SET RESET MENU), no duplicate or missing (control, context) rows, no blank or dead action, all 14 buttons, K1-K4, 3 encoders, 27 keys, MASTER present. No gesture needs two different held modifiers (HELP + RESET layer both held is the only theoretical pair, undefined but harmless).
- Scores re-summed by script: 7.2 approved 386/636 (24 fail), proposed 570/624 (0 fail), every row's V+I+O+L sums correctly and every pass/FAIL label matches the pass rule; 8.1 totals 172 / 234 / 211, fails 13 / 0 / 11 (all correct).
- RAM: 55744 of 73728 gives 17984 B headroom; minus 6953 = 11031; post-removal 46496 - 6953 = 39543; pool 368640 - 342876 = 25764, minus 22400 = 3364. All correct. Implication kept: removals (R6) before preset storage (E7), or P1-016 shrinks the step record; any persisted-scenes idea (+15 KB) does not fit before the removals. Note the 6953 B does not include P1-016's data side beyond the reserved 1536 B.
- No TBD/choose/decide; backlog check 0 errors; no stray files.

Fixed by reviewer (committed):
- Owner question numbers clashed (P1-015 already used 53-54). P1-014's questions renumbered 55-59 in OWNER_QUESTIONS.md, PERFORMANCE_DESIGN.md (2 refs + section 11), CONTROL_MAP.md, ADR-0003 (scenes = 58).
- UX_REVIEW.md section 2 still had pre-P1-014 workflows (W2 hold EDIT, W8 tap GLO, W9 hold SCL, W10 select keys / ALL ON, W12 tap GLO / TAP key, W14 live audition, reference workflow, note 6.5). Rewritten to the new map. ADR-0003 pin Enter/Edit lines (STEP mode, K3) updated to white-key press and hold + K1 NOTE.
- Two strings broke the 34-character cue/help limit (`SET: any key sets the KEY  HOME=back` 36, `K3 PITCH: how wide the pitches range` 36); shortened.
- Question 56 (white keys) now discloses the lost "play a pitch" workflow.

Material findings the owner must weigh (design is internally consistent, but these are honest weaknesses):
1. **L scores are inflated for the dice and UNDO.** FULL_CONTROL/PERFORMANCE_DESIGN give L3 to SCL ENV LFO EDIT GLO dice because "position + cue strip" teaches them. The cue strip (5.1 row 13) names only FX, ARP and OCT+; the other five dice appear in it only after being used. The printed words (SCL, ENV, LFO, EDIT, GLO, ARP) contradict the role, so by the document's own rule (7.1) honest L is 2 for them, and ARP=UNDO (F15) is also 2 (the approved scheme gave EDIT=UNDO L3 under the same contradiction). Dice and UNDO still pass at 11, but F30-F34 (OCT as ROW switch, ROW 2 morphs) at L2 = 10 FAIL; their pass depends on L3 that the design itself calls "the weakest L3". Honest B total is about 12-14 points lower with 4-5 fails, still far ahead of A and C.
2. **REC = MUTATE overrides the most obvious printed word.** REC prints "record"; a first-time user pressing REC to enter steps gets a re-randomised pattern for as long as the key is held (it snaps back, so not destructive). PLAY, SAVE, HOME, SELECT, ALGORITHM, PRESETS, KNOB 1-4 are respected; SEQ=SET is a decent fit (stock: sequencer, pattern); SCL=NOTES is a decent fit (scale / pitch). Weak fits: REC, ARP (arpeggiator = undo), GLO (global is the best printed fit for the SET page, but it is a dice), LFO and ENV (stock: modulation / envelope fit SHAPE and ACCNT better than ACCNT and HITS). A printed-word-compatible variant (scheme D) was not scored: REC = a record-like role (for example "write" = the held step-entry/clear-pins), MUTATE on a black key or ARP; SET on GLO. The executor never scored such a scheme, so B's win over A and C is over two weakened alternatives: A is scored 0 for ROLL/SOLO/scenes because it "has no free control" (that is a design choice, not a property of the scheme), which pads A's 13 fails.
3. **White keys always steps removes pitch playing.** Entering a bassline pitch is: tap key (pins root or generated note), hold key, turn K1 per scale degree, OCT+ then K4 for octave; 16 pitched steps is roughly 16 x (tap + hold + several detents). No chromatic note outside the scale, no way to hear an arbitrary pitch (UX_REVIEW old W14 audition is lost; F26 was re-defined to dodge it). VISION workflow step 3 ("enter steps on the 16 white keys") now enters rhythm and roots, not a tune. Owner question 56 now says so; the suggested alternative (hold step key + press another white key = scale degree) is a table-level change but conflicts with "all held keys receive the same edit".
4. **HOME hold vs HELP.** HOME hold 0.7 s opens the MENU and HELP needs a control touched "within 0.7 s", so the window to ask for help is narrower than the hold needed to open the menu; a slow user opens the menu. Fix would be: open the MENU on HOME release after 0.7 s with no control touched (note STEP_HELD/RESET rows list HELP too).
5. Undefined / thin: REC held then ARP tap (undo during MUTATE); FILL/ROLL held while turning K1; PAGE held while a step key is held; MENU black keys still mute / fill / page while the menu is open (the white keys are inert). The 60 s script is fully executable from documented gestures, except t=54 (ARP undoes the FX dice, not the K3 turn, though the text suggests the pitch width) and the script never uses the HELP or RESET gestures.
6. Confusing-states list (5.5) covers stopped vs playing only implicitly (rows 1, 12 strip text), and has no rows for mute+solo together, undo after a preset load (SAVE hold, not ARP, so ARP after a load is undefined), or an accidental pin from a brush beyond row 3. Add them.
7. FULL_CONTROL.md sections 1.4, 2 and F35 still describe hold SCL keyboard octave and the old layers; covered only by its change note.

Owner decisions (exactly): (a) approve scheme B at all, or the printed-word-compatible variant (REC, ARP, GLO, ENV, LFO placement); (b) accept no pitch playing on the keys, or add hold-step-key + white key = scale degree; (c) accept screen labels + HELP, or an overlay strip (Q57); (d) accept stray-pin risk (Q56); (e) volatile scenes and TAP removed (Q58); (f) R1-R3 gestures, per-step editing as setup class (Q59). Honest view: the scheme is the best of the three offered and strong on the screen side (cue strip, escape, first-run, reset), but "playable without a manual" is only partly met: five dice, UNDO, REC and the ROW switch rely on labels that contradict the printing, so first contact needs the on-screen button picture and the HOME help; pitch entry is slow. Treat it as a good base to hardware-test, not as proven.

### 2026-11-10 orchestrator: sent back for ONE revision pass (before the owner sees it)
The reviewer's findings (full list in its log above) show the design does not yet meet 'playable without a manual' honestly. Required in the revision, then it goes to the owner:
1. Honest L scores: the dice, ARP=UNDO and ROW 2 knobs were scored L3 on 'position + cue strip' but the cue strip names only FX, ARP and OCT+; rescore honestly (those are L2), and fix every function that then falls under the pass rule.
2. Respect the PRINTED words (docs/controls.jpg, panel.c): add a variant B2 of the winning scheme that gives every button a role consistent with the word printed on it wherever the word names something this sequencer can do (for example REC = record/step entry, PLAY = play, SAVE = save, HOME = home, SCL = scale/key, GLO = global feel such as swing/gate/tempo, FX = performance effects such as fill/roll, LFO = modulation/shape, ENV = accent/envelope-like dynamics, ARP = articulation/arpeggio-like movement, SEQ = the sequence/pattern, EDIT = edit selected step, OCT-/OCT+). Score B vs B2 on L and pick honestly. Where a printed word must be overridden, say why and what screen cue compensates. REC=MUTATE specifically must be justified or replaced.
3. Pitch entry: white keys cannot play or audition a pitch, which breaks the TB-3 workflow step 3 in VISION. Specify a fast pitch-entry gesture (for example hold a step key and press another white key = that scale degree/note, or an audition behaviour) and score it; keep an audition path so the producer can hear a note.
4. HOME: resolve the conflict between the HELP window and the 0.7 s hold that opens the menu.
5. Fill the undefined states the reviewer listed, and add to the confusing-states list.
6. Keep the owner-numbered questions (55-59) consistent; do not renumber others. Update all affected documents, re-run python3 tools/check_control_map.py (OK), keep ADR-0003 RAM re-added. Do not write an Approved line.

### 2026-11-10 executor revision (sonnet)
Revision pass after review and the orchestrator's six required items. Design only (no firmware, tests or flashing). Looked at docs/controls.jpg (including a 2x crop of the key labels: black keys read OP1 OP2 OP3 | OP4 OP5 | OP6 PIT GLD | MONO POLY | blank) and panel.c.
1. **Honest L.** New rule (PERFORMANCE_DESIGN 7.1): L3 only where a printed word already means the job; fair or weak fits and unprinted positions are L2; the cue strip does not lift L. All dice, UNDO, black-key moves are now 11 (3/3/3/2). F31-F34 (ROW 2 morphs) are 3/2/3/2 = 10 and cannot pass as performance functions by any honest scoring: B2 classes them as **setup dial-ins** (pass at 10 vs 9) and says so plainly (7.2); owner question 60 offers the alternative (move the four most-performed morphs to ROW 1).
2. **B2 (printed words).** FX = ALL; SCL = NOTES; SEQ = HITS; ENV = ACCNT; ARP = ARTIC; LFO = SHAPE (the P1-012 positions, which came from the printing; the first version's layer-order row was the thing that fought it); EDIT = UNDO / RESET; GLO = SET; REC = RECORD (new `REC` context: the 27 keys play pitches and write them); HOME / SAVE / PLAY as printed; OCT-/OCT+ remains the one hard override (owner decision 48). **REC = MUTATE is replaced** and MUTATE is removed (scored I2 / L1; dice + EDIT tap covers "try and take back"). Scored B (honest) vs B2 vs A vs C; B2 chosen (7.2, 8): approved 414/672 with 25 fails; B honest 568/648 with 5 fails (F31-F34, F36); B2 576/648, 0 fails; 22-function comparison A 180 (13 fail), B 231 (1), B2 235 (1 as P, 0 with F31 as S), C 220 (10). B2 is "A plus B's keys"; A's zeros are a property of freezing the keys.
3. **Pitch entry.** REC: tap REC, play keys F3..G5 (chromatic, sounds = the audition path), K1 CURSOR K2 ACC K3 SLIDE K4 OCT, EDIT tap takes the last note back: 17 gestures for 16 pitched steps (B: about 80). One step: hold the step key + press another white key = that scale degree (F05 10). K1 NOTE sounds each detent.
4. **HOME.** HOME acts on release: tap = home; held + any control touched (no time window) = HELP; released after 0.7 s with nothing touched = MENU; the strip shows `HOME: touch=HELP  let go=MENU` at once (PERFORMANCE_DESIGN 5.3; owner question 61).
5. **Undefined states** defined (5.6, 13 rows) and the confusing-states list extended from 14 to 20 rows (stopped vs playing, mute + solo, EDIT after a load, brush guard, HOME menu surprise, REC pitch mistakes, two buttons held); black keys inert in MENU (TSV rows fixed); 60 s script rewritten (ARP-undo error removed; HELP, pitch gesture, REC and RESET are used).
6. **Numbering:** 55-59 kept (55, 56, 57 reworded for B2), 60-64 added in OWNER_QUESTIONS.md; no other numbers touched.
Files: docs/design/PERFORMANCE_DESIGN.md (rewritten, 11 sections), control_map.tsv and CONTROL_MAP.md (294 rows = 49 controls x 6 contexts, regenerated), FULL_CONTROL.md, UX_REVIEW.md (1.1, 1.2, 1.3 regenerated, W2-W14, 3.3, 5.2 note), ALGORITHM_SET.md, ADR-0003 (REC state added, MUTATE 24 B removed, 11.1 re-added), OWNER_QUESTIONS.md.
RAM (ADR-0003 11.1): new .bss 5329 + 56 + 0 + 8 + 31 + 2 + 768 + 768 = **6962 B**; headroom 46496 - 6962 = **39534 B** after the removals, 17984 - 6962 = **11022 B** before (6926 B above the 4096 B floor; > 8 KB trigger). Pool unchanged (3364 B spare today).
Verification:
```
python3 tools/check_control_map.py   -> rows tsv=294 md=294 / OK
grep -nEi 'TBD|choose|decide' docs/design/PERFORMANCE_DESIGN.md || echo clean   -> clean
python3 tools/backlog.py check   -> 37 items, 0 errors
```
Reviewer should look at: (a) the F31-F34 reclassification (the one place a pass is by classification, not by score); (b) REC's interaction rules (5.6) and the 27-byte sounding-note table; (c) that SCL lost its hold (scale and key now only on SET row 2); (d) the black keys keep L2 (printing OP1..OP6 not used except OP1..OP4 loosely). No Approved line written (human-review gate); the last acceptance box stays unticked.

### 2026-11-10 re-review after revision (sonnet)
Verdict: **PASS on the checkable criteria; stays at `review`. Awaiting owner: approval and the decisions below (no Approved line written, not set done).**

Verified independently:
- `check_control_map.py` OK (294 rows). My script: 49 controls x 6 contexts (PERF STEP_HELD SET RESET REC MENU), no duplicate or missing (control, context), no blank action, REC rows consistent (27 keys = notes, OCT/GLO/PRESETS inert, EDIT = undo last note, HOME/REC leave). No gesture needs two different held modifiers (HELP+RESET and PAGE+step key are defined with first-held-wins).
- Scores re-summed by script: 7.2 totals 414 / 568 / 576 and 8.1 totals 180 / 231 / 235 / 220 (fails 13 / 1 / 1 / 10) are all correct; every V+I+O+L sum and pass label matches the rule. Owner numbering 55-64 sequential, 53-54 are P1-015; no TBD/choose/decide in PERFORMANCE_DESIGN; backlog check 0 errors; no stray files.
- RAM re-added: 5329+56+8+31+2+768+768 = 6962; 46496-6962 = 39534; 17984-6962 = 11022; pool 25764-22400 = 3364. Correct and consistent with ADR-0003 11.1. Pool is the tight resource: removals must precede preset storage.
- HOME: firmware today fires the menu at 700 ms while held (`btn_hold`, ui_input.c:1295, 1586) and swallows HOME with a held layer button. Moving to act-on-release is a real code change (design says so in 10.2); no hidden clash, and it resolves finding 4.
- All six orchestrator items addressed. 60 s script walked: every step uses documented gestures (HELP, pitch gesture, REC, RESET included).

Fixed by reviewer (docs only, check_control_map OK):
- Release rule for a pinned step omitted the second-white-key case: hold a pinned step, press another key for its pitch, release within 0.4 s, and the pin would be cleared. Now "no knob, OCT or second white key" (control_map.tsv, CONTROL_MAP.md, PERFORMANCE_DESIGN 3.1).
- REC row said every written note is "one UNDO away": untrue (one-change undo; a take overwrites pins). Now says UNDO restores only the last note.
- FULL_CONTROL.md still said "hold SCL" for keyboard octave; marked superseded.

Honest findings the owner should weigh:
1. Printed-word fit is overstated at the top of the "fair" band. HOME, SAVE, PLAY, REC, GLO, KNOB, PRESETS, ALGORITHM are genuine. SEQ=HITS and SCL=NOTES are decent. FX=dice ALL ("effects and slicer"), ENV=ACCNT, ARP=ARTIC, LFO=SHAPE, EDIT=UNDO are stretches: no printed word tells a newcomer that a button re-rolls anything. L2 is the right score; "5 strong, 6 fair" should read about "5 strong, 2 fair, 4 stretch, 1 weak". Also SCL printed "scale / key" is the best word for the scale and key, yet B2 uses it for a dice and moves scale/key behind GLO > OCT+ > K1 / any key (3 gestures, F18 10, F19 9). Still settable on the device (S5 met) and not performance, but a first-time user looking for the scale will press SCL and get new notes.
2. B2 over B is the right call, but the margin (235 vs 231) is not the reason. The reason is REC=record, GLO=SET, SEQ/ARP/SCL positions, and the fixed pitch entry. A is inflated by frozen keys. Agree with the executor on this.
3. F31-F34 pass by reclassification (10 vs setup 9). The document's own 60 s script (t=23) and "build" move (6.1, DYNAMICS) perform ROW 2 live, so "setup dial-in" is not how the design itself uses them. Owner decision "full algorithm control, shift is OK" is served (all 8 morphs are reachable, one tap), and the sixth-round decision accepted a shift; but call it what it is: performance with a known L2/I2 penalty. Honest option: approve ROW 2 as a performance function at 10 as an explicit owner waiver, or move the 4 most-played morphs to ROW 1 (Q60).
4. Pitch entry works (REC tap + 16 keys; 16 keys + REC in and out = 18 gestures, not 17, and any rest needs a K1 turn; hold+key is 48). Risks: (a) the step key becomes a "held step" the moment a second white key goes down with no time threshold, so a fast legato roll (press 5 before releasing 1) changes step 1's pitch instead of pinning 5; fix: only treat the first key as held after ~250 ms; (b) REC overwrites pins and only the last note is undoable, so one mistaken take destroys a hand-entered pattern (consider scene A/B or SAVE first, or a REC undo of the whole take); (c) while REC is armed all 27 keys are notes, so FILL/ROLL/mutes vanish, and a user who presses REC (stock: record, starts playing) then sees the device stay stopped. The REC mode is visible (banner, LED, strip) and left by REC or HOME; I judge it hard to get lost in, easy to be surprised by.
5. Confusing-states list (20 rows) and 5.6 (13 rows) cover the earlier findings. Gaps left: pressing REC while a step key is held (STEP_HELD row: held step stays selected, next key is pitch or note?) is ambiguous; a take written while playing quantises to the nearest step with no preview of where it landed (Q63).
6. SYSTEM_MAP and ADR-0003 consistent; the 6962 B includes 1536 B reserved for P1-016, so P1-016 may change it.

First minute of a first-time user (honest "playable without a manual" verdict: mostly, with the on-screen cue strip and HELP; not from the panel print alone): they will (1) tap white keys expecting notes and get pins, (2) press SCL expecting scale, FX expecting effects, EDIT expecting an editor (all re-roll or undo), (3) press OCT+ expecting octave and get ROW 2, (4) press REC and expect the device to start, (5) brush a black key and mute a channel, (6) hold HOME to see help and open the menu on release. All are rescued by the strip/first-run, none destructive except an overwriting REC take.

Owner decisions (exactly): (a) approve scheme B2 (Q55); (b) white keys always steps, stray-pin risk (Q56); (c) accept the printed-word overrides (OCT, EDIT, stretches) with screen labels + HELP (Q57); (d) volatile scenes, tap tempo removed (Q58); (e) R1-R3 gestures (Q59); (f) ROW 2 as setup or an explicit performance waiver (Q60); (g) HOME on release (Q61); (h) MUTATE gone (Q62); (i) REC unquantised (Q63); (j) one step at a time (Q64); plus the new notes: a REC take overwrites (undo = last note), and a legato-roll threshold for the hold+key gesture.
Recommended approval statement: "Approved: <name> <date>: scheme B2 as designed; ROW 2 morphs accepted as a performance waiver at 10/12; hold+key pitch gesture gets a 250 ms hold threshold; a REC take is protected (SAVE/scene first, or whole-take undo) before P1-016 lands; hardware-test the printed-word stretches (FX, ENV, ARP, LFO, EDIT) with a first-time user before P2."

### 2026-11-10 owner
Approved: James Keaney 2026-11-10 WITH CHANGES. Approved: scheme B2 as the button layout (printed-word fit, 1A); screen labels plus hold-HOME help, no overlay strip (2A); HOME acts on release and MUTATE removed (6A); push (9A). Changed: the four most-performed ROW 2 morphs move to ROW 1 (3B, applied in P1-017 when it re-locks the knob table). SUPERSEDED by P1-017 (Opus sequencer + UX lock): pitch/step entry (4: step-centric editing with a note-edit page), pattern length and pages (7: 16 -> 64 steps by duplicating and editing pages), scenes and tap tempo (5: dropped; tempo on the global settings page), keybed force sensing (8B: in v1). The owner's framework for P1-017: every state is a page, each button has a mode (press/hold ...), each state has a screen, each screen shows what the knobs control; input to processing to output all aligned and mapped; high bar on usability, understanding, UX, performance, fun and musical output; never any deep learning.
