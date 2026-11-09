---
id: P1-017
title: Sequencer functionality and UX lock: step-centric editing, 64-step pages, force-sensing keys, page/state/mode framework
phase: P1
type: spike
status: todo
priority: 0
executor: opus
depends_on: [P1-014, P1-015]
gate: human-review
---
## Goal
You are the UX designer and technical architect for this instrument. The owner has rejected the step/pitch-entry, pattern-length, pages and scenes parts of the P1-014 design and asked for a whole rethink that LOCKS the sequencer functionality and its UX at a high bar: easy, never any deep learning or deep controls, fun, musical, and powerful. Produce the complete, locked sequencer spec that a Haiku developer can implement literally and that scores to the rubric.

## The owner's requirements (verbatim intent, read OWNER_QUESTIONS.md 'tenth round' and R1-R4)
1. Dial an algorithm in first. THEN the user controls the steps that were generated: turn steps ON/OFF, morph a step.
2. Get the first 16 steps right; then the second 16 are a DUPLICATE of the first and can be changed independently; and so on, up to 64 steps (4 pages of 16). Easily entering and sequencing 64 steps out of a 16-step pattern is the key effort. Each channel has its own length; no fixed 16.
3. To change a note: HOLD the step; the screen shows a NOTE EDIT page and the knobs change pitch, octave, gate length, velocity, ratchet, slide, probability etc.
4. NO scenes. NO tap tempo. Tempo is set on the global settings page. Reclaim any controls they used.
5. The keybed force sensing (Velo / CTL-1: velocity + polyphonic aftertouch, see docs/design/KEYBED_EXPRESSION_REFERENCE.md) is IN v1: design excellent use of it (for example per-step accent/ratchet/slide/probability amounts from press force, expressive step editing, performance moves), with explicit handling of the limits: only F3-C5 (20 keys) have force; the top 7 keys do not; lit LEDs disturb neighbouring key readings (step keys are lit!); ADC shared with the MASTER pot; CPU/RAM cost. Give a fallback gesture for every force gesture so the instrument works on all 27 keys and if force is poor.
6. The device must be easy and never need deep learning or controls: every function reachable in at most 2 gestures; no hidden modes.
7. The FRAMEWORK to follow: every state is a PAGE; each button has defined modes (press, hold, double ...); each page/state has a SCREEN; each screen showcases what the knobs are controlling; input -> processing -> output are all aligned and mapped (a state/page table, a button-mode table, a knob-map table, and an input/processing/output trace for every function). Held to a high bar of usability, understanding, UX, performance, fun and musical output.
8. Already decided (do not reopen): B2 button layout with printed-word fit (FX SCL SEQ ENV ARP LFO dice, EDIT = UNDO with hold RESET, GLO = SET, REC = record, OCT-/OCT+ row switch); screen labels plus hold-HOME help, no overlay; HOME acts on release; MUTATE removed; four most-performed ROW 2 morphs move to ROW 1 (re-lock the knob table: 3B); 4 channels; global scale/key and GATE/DIV/SWING/tempo; boot straight into the sequencer; per-algorithm saved morphs; four v1 algorithms ACID EUCLID SNAKE STAGE with 8 morphs. You MAY change a decided item only if the score demands it, and then say so explicitly with evidence.

## Read first
- docs/project/OWNER_QUESTIONS.md (ALL; especially R1-R4, the eighth, ninth and tenth rounds), VISION.md (rule 3 now: a channel has MIDI channel, algorithm and pattern length), ADR-0004
- docs/design/PERFORMANCE_DESIGN.md (B2, the scoring rubric V/I/O/L and the three schemes), FULL_CONTROL.md, UX_REVIEW.md, CONTROL_MAP.md, control_map.tsv, tools/check_control_map.py
- docs/design/KEYBED_EXPRESSION_REFERENCE.md and, if you need it, the Velo/CTL-1 sources in a fresh /tmp directory (read-only; never run downloaded code)
- docs/design/ALGORITHM_SET.md and section 6 of docs/design/algorithms/{sting,snake,ml185,t1}.md (how patterns are generated; how generated steps and manual edits interact when the user re-dices or morphs)
- docs/project/decisions/ADR-0003-sequencer-core.md (Step/Channel structs, RAM table), docs/design/SYSTEM_MAP.md (headroom; pool pressure), firmware/src/panel.c, docs/controls.jpg (look at it)

## Deliverable
- docs/design/SEQUENCER_UX.md: the locked spec, containing at least:
  1. **Model**: layers of a pattern: the generated layer (from the algorithm) and the user edit layer (per-step overrides: on/off, pitch, octave, velocity, gate, ratchet, slide, probability ...). Exactly how re-dice and morph interact with edits (do edits survive? how does the user see which steps are edited? how to clear an edit), and how pages work: 1 to 4 pages of 16, page 2..4 start as a duplicate of page 1, edited independently, per-channel length, page-swap gesture, the playhead and follow rules, what the 16 white keys show on each page, length change rule (extend duplicates, shrink truncates, nothing lost until the user commits).
  2. **State/page inventory**: every page (PERFORM, STEP SELECT, NOTE EDIT, PAGE view, SET (global incl. tempo, scale/key, gate, div, swing), CHAN, RESET, HELP, FIRST-RUN, any others) with its ASCII mock-up of the 240x240 screen showing the knob labels and what each of K1-K4 and SHIFT-row knobs controls on that page, the cue strip and legends; the transitions between pages (a state diagram) and the single escape.
  3. **Button-mode table**: every button x every mode (press, hold, double if used) x every page, with action, on-screen label, and the principle that makes it guessable.
  4. **Knob-map table** per page; **key-map table** (27 keys) per page including the force-sensing gestures and their fallbacks.
  5. **Input -> processing -> output traces** for every function (a table), including MIDI emitted, timing, and what the screen shows.
  6. **Force-sensing design**: gestures, thresholds, calibration, LED-disturbance handling for lit step keys (for example dim/turn off LEDs while sampling, or sample between LED refreshes), fallbacks, CPU/RAM/ADC-sharing cost with numbers, risks, and the hardware-test gate that must pass before it is relied on.
  7. **Workflows** (numbered, gesture by gesture): first run to a playing TB-3 bassline; dial in an algorithm; extend 16 to 64 steps and vary the second 16; edit one note on NOTE EDIT; mute/solo; change tempo; save and load. Each must be ~2 gestures per function.
  8. **Score** every function with V/I/O/L (UX_REVIEW/PERFORMANCE_DESIGN rubric); high-bar pass rule: performance functions >= 11/12 and setup >= 10/12, no sub-score below 2; no deep controls (max 2 gestures). Be HONEST: do not inflate L. Compare at least two genuinely different step-editing models (for example hold-to-edit with a NOTE EDIT page vs. a latched step cursor) and choose by score and fun.
  9. **Data model and budget**: per-step override record, per-channel pages up to 64 steps x 4 channels, undo, the preset size; re-add the RAM and pool budget against ADR-0003/SYSTEM_MAP (RAM floor 4 KB; the pool has little spare: say which removals are prerequisites; propose compact encodings if needed).
  10. **Musical output review**: what the user hears for the key operations (edit a step, extend, dice with edits present, ratchet/slide emission, swing across pages, note-off guarantees across page boundaries and on stop), checked against the timing in ADR-0003.
  11. **Confusion audit**: every state in which a first-time user could be lost and the cue that rescues them; the first-minute walkthrough.
  12. **Flex points** (ADR-0004): what is re-mappable by a table edit.
  13. **Locked statement**: the list of what is now LOCKED, so later tasks can implement without choices.
- Machine-readable tables: docs/design/page_states.tsv, docs/design/button_modes.tsv, docs/design/knob_map.tsv (and extend control_map.tsv/CONTROL_MAP.md), with a script tools/check_sequencer_ux.py that checks completeness (every button x mode x page defined, every knob labelled on every page, no conflicts) and prints OK.
- Updated consistently, with change notes: PERFORMANCE_DESIGN.md, CONTROL_MAP.md/control_map.tsv (check_control_map.py OK), FULL_CONTROL.md (row 1/row 2 morph reassignment per 3B), ALGORITHM_SET.md and the four algorithm docs (how the length/pages and edits relate to each algorithm's own lengths; keep generation deterministic), ADR-0003 (Step/Channel/override structs and sizes with _Static_assert plan, RAM re-added), VISION.md rule 3 wording.
- New owner questions (continue numbering after 64; keep them few and real).

## Acceptance criteria
- [ ] Every owner requirement 1-7 above is satisfied or explicitly argued against with score evidence
- [ ] Pages: up to 4 x 16 steps per channel, duplicate-then-edit, page swap, length per channel fully specified; white keys align to the viewed page
- [ ] NOTE EDIT page specified with its knob map; step on/off and per-step morph specified
- [ ] No scenes, no tap tempo; tempo on the SET page; freed controls reassigned
- [ ] Force-sensing gestures specified with fallbacks, LED handling, costs and a hardware-test gate
- [ ] State/page, button-mode, knob-map and key-map tables exist as TSVs; tools/check_sequencer_ux.py prints OK; tools/check_control_map.py prints OK
- [ ] Every function traced input -> processing -> output and scored; all pass the high-bar rule honestly; two editing models compared
- [ ] RAM/pool budget re-added; 4 KB floor respected; prerequisites named
- [ ] ADR-0003 and algorithm docs consistent with the locked spec
- [ ] No 'TBD', 'choose' or 'decide' wording; a 'Locked' section lists what is locked
- [ ] The owner approved it (log line 'Approved: <name> <date>')

## Verification
```sh
python3 tools/check_sequencer_ux.py
python3 tools/check_control_map.py
grep -nEi 'TBD|choose|decide' docs/design/SEQUENCER_UX.md || echo clean
python3 tools/backlog.py check
```

## Out of scope
- Firmware code; flashing. (Downloaded code is read-only and never executed.)

## Log
