---
id: P4-029
title: Screen v2 elements: button picture, keybed footer, message slot (toasts, hold hints, touched names, setup cue)
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-026]
gate: none
---
## Goal
Document every control where the eye is, as SCREEN_SPEC_V2 sections 4.5, 4.6 and 5: the icon buttons, the keybed in its
mode, and the one message slot with its priority rules and timing.

## Read first
- docs/design/SCREEN_SPEC_V2.md sections 4.5, 4.6, 5, 6; proposals/v12/v12.py (buttons, keybed, topbar, MESSAGES, check_messages)
- docs/design/button_modes.tsv, key_map.tsv (hold times 0.4 / 0.7 s); firmware/src/ui_sq.c toast queue

## Deliverable
- Button picture (dice tint, absent layer dimmed, pressed inverted, REC armed); keybed footer (step fronts, playing bar,
  black-key glyphs, held states, CHAN BACK, REC plain keys, footer lines); message slot with toast / touched-control /
  hold-hint / page-title priority, hold bars timed from the TSV hold times, the setup cue.
- The MESSAGES strings as a generated table (width-checked by P4-027's check).

## Acceptance criteria
- [x] host renders of the remaining SCREEN_SPEC_V2 section 7 scenes match the v12 mock (or listed and approved differences)
- [x] every message fits (check passes); a too-long one fails the build
- [x] `sh tools/check.sh` ends CHECK PASS

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- Header/knobs/music (P4-028); table rows (P4-030).

## Log

### 2026-11-13 executor (sonnet)
Done. `firmware/src/ui_sq.c` now draws, from the `UisqView`: the button picture (`uisq_draw_buttons`: dice tint, absent layer DIM via `uisq_btn_dimmask`, pressed
inverted, REC armed a red disc), the keybed (`uisq_draw_keybed`: step fronts, playing bar, M1-M4 / CHN / SOL FIL ROL / PG- PG+ / RST glyphs, held states, CHN = BACK on
CHAN, REC plain keys, past-LENGTH hatch), the footer lines and the inverted setup cue, and the message slot (`uisq_draw_msg`, `uisq_msg_pick`: hold hint > touched
control > toast > page title). `uisq_draw_elements` now lays the frame first. Message state API: `ui_sq_toast_msg/_id`, `ui_sq_touch/_id/_end`, `ui_sq_hold_begin/_ctl/_end`,
`ui_sq_btn_down`, `ui_sq_held_m_set`, `ui_sq_held_mv_set`, `ui_sq_new_set` (nothing calls them before the cut-over, P4-012). The slot is hashed by strip 0 only.
`tools/gen_ui_tables.py` emits `ui_msgs[]` (the MESSAGES of the mock, enum `UI_MSG_*`) and `ui_hold_rows[]` (hold times parsed from the PERF hold rows of
button_modes.tsv / key_map.tsv: EDIT/GLO 400, SAVE 700, keys 400 ms); the width check of P4-027 covers the messages. The M-key remove hold (1.5 s) is
`UISQ_REMOVE_MS` (the key_map row comes with P4-030). The toast time is now 1000 ms (SCREEN_SPEC_V2 5.1; tests updated; the diced hatch keeps 2000 ms as `UISQ_DICED_MS`).
`docs/design/ui_review/proposals/v12/icons_v12.py` gained `left`, `right`, `rst` (copied from el.py `IC`) so `build/gen/ui_sqicons.h` has the black-key arrows; no mock render changes.

Verification
- `python3 tools/gen_ui_tables.py --selftest` -> `message too long -> caught: SCREEN_SPEC_V2 message (toast, 'WWWWWWWWWWWW', 'EDIT=UNDO') needs 225 px, the slot has 136` ... `OK (6 algorithms, 34 messages; 6 deliberate misfits caught)`
- `/tmp/ui_sq_v12_test build/ui_sq/v12` -> `ui_sq_v12: 40 scenes and 4 knob sheets drawn`, `ui_sq_v12_test: OK (0 failures)` (now also: every ui_msgs row fits 136 px, footers fit, hold times
  from the TSVs, priority hold > touch > toast > title, 0.25 s hint start, bar fractions, toast 1000 ms, strip-1 signature unaffected by a toast, cue and dim mask in view_fill)
- `python3 tests/v12_compare.py` -> `44 scenes/sheets, 0 failed` over the WHOLE 240x240 frame now (no mask of the message slot, no y limit); only difference remains the listed T_CHF
  CH2/CH4 rounding (approved in P4-028); the committed goldens differ from today's mock render only by glyph anti-aliasing (largest group 40 px).
- `sh tools/check.sh` -> `RAM .data+.bss 23844 B of 73728` ... `CHECK PASS`
Notes for the reviewer: the past_length quirk (other strips hidden past the selected channel's LENGTH) was left untouched, as instructed. `uisq_draw_frame` is kept (ui_sq_test checks the
empty frame through it). ui_sq.c is still not in felucca.c, so firmware size/RAM is unchanged.

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none). Re-ran `sh tools/check.sh` -> CHECK PASS; `./build.sh` -> `RAM .data+.bss 23844 B of 73728` (headroom 49884 B, > 8192).
- Toast 1000 ms: SCREEN_SPEC_V2 5.1 says "Toast (~1 s)"; ui_sq_test.c updated consistently (1000 ms boundary, 500/950/40 ms walk). Diced hatch keeps 2000 ms separately. OK.
- icons_v12.py gained left/right/rst only (2 lines); no proposals/v12/out render changed; v12_compare 44 scenes 0 failed on the whole frame. OK.
- Hold bars: ui_hold_rows[] generated from the PERF hold rows of button_modes.tsv / key_map.tsv (0.4/0.7 s); uisq_msg_pick order hold > touch > toast > page title, matches spec 5.5. No per-algorithm branch in the diff (only descriptor fields, ALGOS[].layer[].n for dim).
- UISQ_REMOVE_MS 1.5 s constant pending P4-030 key_map rows: accepted, spec 6 states 1.5 s and names P4-030. uisq_draw_frame kept (ui_sq_test uses it): accepted.
- Minor note: the past_length quirk left as instructed. UX gate Gate B: element-level task, ui_sq.c not yet in felucca.c (cut-over P4-012), so nothing user-reachable yet; no sections 7-9 walk possible beyond the host scene renders, which match the mock.
