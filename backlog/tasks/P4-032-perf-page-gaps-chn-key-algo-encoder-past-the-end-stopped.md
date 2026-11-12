---
id: P4-032
title: PERF page gaps: CHN key, ALGO encoder, PAST THE END, stopped step sounds, preview toast
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-013, P4-015]
gate: none
---
## Goal
Close the PERF controls P4-013 left unwired that the HW-1 workflow (select CH1, set its MIDI channel, pick an algorithm, enter steps, PLAY on the TB-3) needs: nothing on PERF reaches the CHAN page or changes the algorithm until this lands.

## Read first
- backlog/tasks/P4-013-perf-page-lane-step-keys-pages-and-transport.md (Log: "Not done")
- docs/design/key_map.tsv (PERF KEY_B05_D#4 CHN; PERF KEY_W01..W16 tap; KEY_B10_D#5 last page at 64), knob_map.tsv (PERF * ALGO), docs/design/SCREEN_SPEC_V2.md
- firmware/src/ui_sq.c (ui_sq_keys, uisq_key_down/up, ui_sq_select_turn), ui_sq_glue.c (ui_sq_controls drains EN_ALGO)
- tests/ui_sq_perf_test.c

## Deliverable
- PERF B05 (D#4, CHN) tap opens the CHAN page of the selected channel (the page itself is P4-015)
- PERF ALGO encoder turn: algorithm of the selected channel over ALGOS[] names via sq_set_algo, clamped, toast/touch line per knob_map; lands at the next step
- PAST THE END: a white-key tap at a step >= LENGTH shows the toast (add the row to the MESSAGES source table and regenerate ui_tables.h; no literal string in page code); PG+ on page 4 at LENGTH 64 shows `64 STEPS = MAX`
- Stopped: a step turned ON by a tap sounds once (key_map.tsv), via a main-loop request the ISR consumes (a one-shot volatile flag, as transport_req); the note-off is guaranteed (no hung note)
- Preview toggle toast (the M-key hold released before 1.5 s)
- tests in tests/ui_sq_perf_test.c for each

## Acceptance criteria
- [ ] D#4 tap on PERF opens CHAN; ALGO turn changes sq_ch[sel].algo and the screen names it in the same frame
- [ ] A tap past LENGTH toasts PAST THE END and changes nothing; PG+ at 64 steps toasts 64 STEPS = MAX
- [ ] A step turned on while stopped sounds once and ends (host test counts NOTE_ON and NOTE_OFF)
- [ ] Every string comes from ui_tables.h
- [ ] `sh tools/check.sh` ends with `CHECK PASS`; paste the `RAM .data+.bss N of 73728` line, headroom at least 8192 B

## Verification
```sh
sh tools/check.sh
./build.sh | grep RAM
```

## UX gate
Producer-facing; extend docs/design/features/P4-013-perf-page-lane-step-keys-pages-and-trans.md steps 1, 11 and add the CHN and ALGO steps; Gate B by the reviewer.

## Out of scope
- Hard-tap ACCENT (needs key force; belongs with NOTE_EDIT P4-014 and REC P4-017), SOL/FIL/ROL (P6-003), RST (P4-018), PRESETS encoder (P7-002)
- The CHAN page itself (P4-015)

## Log
- 2026-11-13: filed by the P4-013 reviewer (gaps found in the P4-013 Log "Not done"; sits before HW-1 via P4-018 -> P4-020 -> P4-021).
