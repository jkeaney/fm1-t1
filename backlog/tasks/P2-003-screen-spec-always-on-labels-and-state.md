---
id: P2-003
title: Screen spec: always-on labels and state
phase: P2
type: task
status: review
priority: 2
executor: sonnet
depends_on: [P2-001, P1-013, P1-014, P1-017]
gate: human-review
---
## Goal
VISION rule 1, made concrete: every screen always shows what the device is doing and the current role of every control.

## Read first
- docs/design/SEQUENCER_UX.md and the TSV tables page_states.tsv / button_modes.tsv / knob_map.tsv (P1-017: the locked sequencer functionality and UX; every page needs a screen spec)
- docs/design/PERFORMANCE_DESIGN.md (P1-014: performance-first controls; supersedes anything it changes in FULL_CONTROL.md)
- docs/design/FULL_CONTROL.md (P1-013: shift row, up to 8 morphs per algorithm, live two-row labels)
- docs/design/CONTROL_MAP.md
- firmware/src/ui.c layout constants (H_HEAD, Y_LABEL, CARD_H, Y_GRAPH, Y_FOOT)
- firmware/src/gfx.c (fonts AF_S/M/L, theme colours, CV_MAX 240x124)
- firmware/src/ui_t1.c (current T-1 screen)

## Deliverable
- docs/design/SCREEN_SPEC.md: a fixed zone layout for 240x240 (status bar, knob-label row, main view, button/footer hints) with pixel rectangles; the content of each zone per mode; how modifier-held states relabel; mock-ups as ASCII grids; text length limits per zone, with fonts

## Acceptance criteria
- [x] Every control in CONTROL_MAP has a defined on-screen label location for each mode
- [x] The zone rectangles tile 240x240 without overlap (listed as x,y,w,h)
- [ ] The owner approved it (log line)
- [x] Screen layouts are specified as data (regions, label slots, state fields) so a label or layout change is a table edit; a 'Flex points' section lists them
- [x] Every knob/button role shown on screen for each context (VISION rule 1)
- [x] A 'Boot and first use' section: power-on goes straight to the sequencer main screen with a default channel, algorithm and global scale/key, and PLAY is audible with no setup; no groovebox or mode choice appears anywhere (owner decision 32b)

## Verification
```sh
grep -c 'x,y,w,h\|x=' docs/design/SCREEN_SPEC.md
```

## Out of scope
- Implementing the drawing code.

## Log

### 2026-11-10 executor (sonnet)
- Wrote docs/design/SCREEN_SPEC.md: zones Z0..Z7 tile 240x240 (heights 20+28+60+52+52+12+12+4 = 240; area sum 57600), 3 canvas blits each <= 240x124, slot tables, control-to-slot table for all 49 controls x 9 pages, modifier relabels, length limits, mock-ups, flex points, boot section. Widths measured with a probe built from the real gfx.c `text_w` and build/gen/ui_fonts.h.
- Folded in FB-001 (owner scope): SCREEN_SPEC section 6 (INT / EXT LIVE / EXT SYNC / EXT WAIT, 500 ms timeout = the existing seq.c rule, clock out MASTER when source INT / FOLLOW otherwise). Added matching text: SEQUENCER_UX 2.8, cue rows 8a / 13a, locked item 12, flex row; page_states.tsv PERF and SET screen fields; ADR-0003 change note, section 5 row, new 7.4, flex row. Nothing locked was changed.
- Findings for the reviewer (SCREEN_SPEC 9): the locked mock-ups are not drawable as printed (AF_M labels too wide; PERF row 2 needs ~326 px). Resolved with AF_S, OCT labels in the band tag column, a hold mark in place of `h:`. Open owner questions in SCREEN_SPEC 11.
- Verification:
  - `grep -c 'x,y,w,h\|x=' docs/design/SCREEN_SPEC.md` -> 7
  - `python3 tools/check_sequencer_ux.py` -> `pages=9 ... scored=69 exceptions=3` / `OK`
  - `python3 tools/check_control_map.py` -> `rows tsv=441 md=441` / `OK`
  - `python3 tools/backlog.py check` -> `39 items, 0 errors`
  - tools/check.sh not run: docs-only change, no code.
- Unticked: "The owner approved it" (human gate; no Approved line written).

### 2026-11-10 review (sonnet)
Verdict: **FAIL, sent back to todo.** Re-ran: `check_sequencer_ux.py` OK (pages=9, scored=69, exceptions=3), `check_control_map.py` OK (441=441), `backlog.py check` 0 errors, grep -c = 7. Zone table verified: heights 20+28+60+52+52+12+12+4 = 240, area sum 57600, blits 240x108 / 240x104 / 240x28 all <= 240x124; Z0 slots 238+2, Z3 36+4x51, LEG_X 11 entries. 49 controls = 48 panel + MASTER; spot-checked 12 rows against control_map / knob_map / button_modes (OCT labels on all pages, SET/REC/CHAN knobs, GLO BACK, EN_PRESET in REC): consistent. Glyph widths re-measured with a probe on the real text_w and ui_fonts.h: the quoted widths are correct (ACCENT S46/M58, RESTRT S42, EXT 126 S45/M57, h:HLP/MN 53, h:UNDO L 52, TAP STEP... 203, TEMPO strings 112/118/84/72/73).

Required fixes (numbered, for the next executor):
1. **ADR-0003 is structurally damaged.** The commit replaced the heading `### 7.1 Why 192 PPQN` with the new `### 7.4` block and inserted a second `## 8. The event path` (docs/project/decisions/ADR-0003-sequencer-core.md lines 388-396). Now section 7 starts at 7.4, the 192-PPQN paragraph sits under "## 8", 7.2/7.3 follow it, and "## 8. The event path" appears twice (lines 396 and 469); section 5 still cites "section 7.1". Restore `### 7.1 Why 192 PPQN` above its paragraph, delete the stray `## 8` at 396, and put 7.4 after 7.3 (before the real `## 8`).
2. **SCREEN_SPEC 6.1/6.2 misstate seq.c.** (a) It says the internal BPM `song.g[G_BPM]` "is never overwritten by the external tempo [V, fx.c]". It is: `midi_clock_pulse` (midi_clock.c) writes `song.g[G_BPM] = clamp(.., 40, 240)` every quarter. So EXT WAIT cannot "revert to the internal BPM" and K1-on-EXT "sets INT BPM" cannot work without a code change (a separate external-tempo field, or save/restore). State that as a requirement for the dev task, or specify the readout from the present behaviour. (b) `clk_bpm` range "20..300" is wrong: accepted range is 40..240 (dt 62..375 ms). (c) `tempo_valid` is a flag set on the first pulse; the BPM is first computed on the 7th pulse (6 pulses later). The spec says "valid after 6 pulses (tempo_valid)". Fix the wording; EXT SYNC lasts about one quarter note, as written.
3. **Timeout arithmetic wrong (6.1).** 500 ms is 24 pulses at 120 BPM (not 12), and 0.25 beat at 30 BPM (not "3.6 beats"; 30 BPM is below the firmware's 40 floor anyway). The 500 ms check in seq.c:1410 runs only while `song.playing` and `clock_mode`; EXT WAIT while stopped (row 13a) needs a new `clk_live` derivation. Say so instead of implying [V] covers it.
4. **F2 row-2 fit does not add up (SCREEN_SPEC 2.5 / 9).** A 5 px mark replacing `h:` (9 px) saves 4 px, not 9. The quoted cells 47+46+46+45+30+26 = 240 equal the label widths minus the full 9 px with zero mark width. With the 5 px mark plus a 1 px gap on the HOME and SAVE cells the row needs about 252 px, 12 over. Choose a fix that sums to <= 240 (cell padding 3 -> 1 px gives 12 back exactly; or a 1-2 px mark; or move a hold label) and show the sum with the mark included. The printed 326 px figure for the old row is plausible but its breakdown is not shown; list the 8 cells.
5. **A locked decision is changed, not only added.** knob_map.tsv SET K1 (locked): "tempo (internal clock; ignored while an external MIDI clock runs, value shows EXT)". SEQUENCER_UX 2.8 / SCREEN_SPEC 6.4 make K1 edit the internal BPM while EXT. Either keep K1 inert in EXT (and drop 8a's "K1 SETS INT BPM"), or list it as an explicit amendment with an owner question and update knob_map.tsv and the "nothing locked was changed" claims (SCREEN_SPEC header, task log).
6. Smaller gaps against "a label slot for every control on every page": (a) MENU: control_map labels (`close`, `tab`, `row`) have no slot; the spec only says "own title". (b) RESET: OCT- / OCT+ read `RST R1` / `RST R2` and ALGO / PRESET are `-` in control_map; the RESET row of the page table omits both. (c) NOTE_EDIT and REC replace Z1 with a banner, so the `CH n` (EN_SELECT) label slot is absent; name it in the banner. (d) Z5 text says a 2 px gap between key groups; LEG_X gives 3 px (42+20 -> 65).

Not at fault: the FB-001 clock-out master/follow rule is coherent with the ADR 5 row (INT only emits; follow silent) and 96 packets/s at 240 BPM is right; RAM impact (~8 B) is plausible but unchecked against ADR 11.2.
Awaiting owner: nothing yet; fix the above first. No Approved line written.

### 2026-11-10 executor (sonnet), rework after review
Fixed all six findings:
1. ADR-0003: restored `### 7.1 Why 192 PPQN`, removed the stray `## 8`, 7.4 now follows 7.3 (headings 7.1, 7.2, 7.3, 7.4, one `## 8`). Added a line to 7.4 on the tempo overwrite and a pointer to P2-010.
2. SCREEN_SPEC 6.1/6.2 rewritten from the firmware: `midi_clock_pulse` overwrites `song.g[G_BPM]` clamped 40..240 (`midi_clock.c:65`); `tempo_valid` is a first-pulse flag, BPM first computed on the 7th pulse (new `clk_known` bit required); EXT WAIT shows the last host tempo (`126 EXT?`), not the internal one. The lost-internal-BPM consequence is owner question 5 (separate field), related clock work pointed to P2-010. SEQUENCER_UX 2.8 / 11.2 aligned.
3. Timeout: 500 ms = 24 pulses at 120 BPM, 8 pulses at 40 BPM; `seq.c:1410` runs only while playing; the stopped case needs a new `clk_live` derivation from `midi_clock.last_ms` (stored on every 0xF8) plus a `clk_seen` bit, stated as a dev-task requirement.
4. PERF row 2: recomputed with a probe on the real `text_w` (HOME 6+44, SAVE 6+42, ARP 43, SEQ 42, PLAY 28, REC 22 = 233; padding 1 px x 6 = 239 <= 240; cells 51/49/44/43/29/24); mark saves 3 px (5+1 vs 9), not 9; row 1 table (sum 240) and the old 326 px breakdown stated.
5. Locked SET K1 restored: ignored while an external clock runs; cue 8a now `EXT: K1 OFF, HOST SETS TEMPO`; "K1 sets INT BPM" removed everywhere; owner question 5 raises the alternative.
6. Added MENU control labels (footer line), RESET `RST R1`/`RST R2` and `-` encoders, `CH n` in the NOTE_EDIT and REC banners, legend group gap 3 px.
Verification:
  - `grep -c 'x,y,w,h\|x=' docs/design/SCREEN_SPEC.md` -> 7
  - `python3 tools/check_sequencer_ux.py` -> `pages=9 ... scored=69 exceptions=3` / `OK`
  - `python3 tools/check_control_map.py` -> `rows tsv=441 md=441` / `OK`
  - `python3 tools/backlog.py check` -> `40 items, 0 errors`
Owner approval still pending (no Approved line).

### 2026-11-10 review 2 (sonnet)
Verdict: **PASS on content; gate human-review, so it stays at `review`.** Re-ran: `check_sequencer_ux.py` OK (pages=9, scored=69, exceptions=3), `check_control_map.py` OK (441=441), `backlog.py check` 40 items 0 errors. No Approved line written; no 20-minute suite run.
All six previous findings verified fixed:
1. ADR-0003 headings now 7.1, 7.2, 7.3, 7.4 (line 461) and a single `## 8` (line 470); `git diff 03f3710 HEAD` of the ADR is purely additive (change note, section 5 row, 7.4, flex row), no damage elsewhere.
2. Tempo claims match firmware: midi_clock.c:65 `song.g[G_BPM] = clamp(.., 40, 240)` overwrite; first pulse only sets `tempo_valid` (midi_clock.c:~34-37), interval closed at `++tempo_n == 6` so BPM first computed on the 7th pulse; accepted dt 62..375 ms (lines ~44-45); seq.c:1410 `fm1_ms - last > 500u` inside `if (song.playing)` under `clock_mode`. SCREEN_SPEC 6.1 states all of this, and flags `clk_known` / `clk_seen` / `clk_live` as new dev requirements.
3. Timeout arithmetic correct: 500 ms = 24 pulses at 120 BPM, 8 pulses at 40 BPM (62.5 ms each).
4. PERF row 2: 50+48+43+42+28+22 = 233 + 6 x 1 padding = 239 <= 240; cells 51+49+44+43+29+24 = 240; row 1 cells 24+41+42+47+36+50 = 240, each >= label + 3. Mark saves 3 px (6 vs 9) stated honestly. Widths consistent with my earlier real-glyph measurement.
5. Locked SET K1 restored (K1 inert in EXT; knob_map.tsv line 42 unchanged). The only TSV change in the whole task is additive screen text in page_states.tsv (PERF/SET screen fields); no locked decision altered. Alternative raised as owner question 5.
6. MENU footer line, RESET `RST R1`/`RST R2` and `-` encoders, `CH n` in NOTE_EDIT and REC banners, 3 px group gap (LEG_X 42+20=62 -> 65) all present.
Awaiting owner: read docs/design/SCREEN_SPEC.md (zones, PERF cell layout, section 6 tempo-source states, section 11 questions 3-5) and add an `Approved` log line. Notable owner decision: question 5 (keep the internal BPM separate from the host tempo; K1 stays inert during EXT).
