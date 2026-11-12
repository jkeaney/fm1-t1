<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 04 History: what was tried, what scored what, what we learned

All on branch `t1-hardware-bringup`, committed, **not pushed**. Commit hashes are the ones to `git show`.

## Chronology

| Step | What | Commit | Outcome |
| --- | --- | --- | --- |
| 0 | P4-011 delivered `ui_sq.c`: the **old dense layout** (Z0..Z7 zones) from SCREEN_SPEC, rendered to `build/ui_sq/*.png`. Reviewed and passed with debt (legend labels truncated: `C... S... R...`) | `0444534` | A mock render of the layout the firmware was about to ship exposed the problem |
| 1 | **Assessment A of the shipping layout** (my informal design review, `ASSESSMENT_A.md`): 29/40; the first priority issues (values same size as labels, weak playhead, unreadable legend, muted channels vanish, duplicated dice labels) | in `815bf75` | superseded by measurements |
| 2 | **Assessment B evidence for the shipping layout** (`EVIDENCE.md`, `elements.tsv`, `contrast.py`, `fonts.tsv`, `palettes.tsv`, `text_pairs.tsv`, `truncations.tsv`): 69 text draws at idle (75 playing), only AF_S, 64 truncated legend labels, lowest text contrast 1.67:1 (`T_DIM` on `T_RAISE`), T_DIM on T_BG 2.87:1, no accent colour in GREY (T_ACCENT = T_REC = T_TEXT) | `49b3ad8` | I had **under-estimated** the text count (guessed ~55) |
| 3 | **Pixel-exact mockup tool** (`mock/`): real bitmap fonts + palettes parsed from `build/gen/`, reproduces the shipping render with **0 differing pixels** after a fresh render | `4b84bfb` | the foundation for everything after |
| 4 | **Owner: "the screen is 30mm ... too much information, too small."** Also: their own **6x6 grid sketch**, offered as a concept to critique | (chat) | converts "tidy up a dense screen" into "design for a 1.2-inch screen" |
| 5 | **v1** (`proposals/v1/`): kept the zone layout; knob VALUES large (AF_M), drawn transport shapes, struck-through muted keys, glyph legend, colour per dice layer (6 new tokens), blue bars | `28feaf6` | my own critique: better hierarchy, but cramped dice row, raw enum numbers, decorative blue bars, still 60-75 small items. **Owner: fail** |
| 6 | **v2-grid** (the owner's concept rendered on a 5 px module) | stopped before output | stopped because the owner's 30 mm / "too small" feedback made a 12-button grid at 7.6 arcmin pointless; the idea survived in the *held map* |
| 7 | **Owner decisions:** glance screen + full map on demand; four at-a-glance facts (see `03_DECISIONS.md`) | (chat) | **the decision was made from a multiple-choice question, without a picture of the alternative (buttons visible)** |
| 8 | **v3-glance** (`proposals/v3-glance/`): 4 big tiles, 16-step lane with a full-height amber playhead, 4 big knob cells with meters, toasts, a one-page MAP that mirrors the panel (held HOME). 8 text roles / 20 strings, no AF_S on the main screen | `77049c3` | I called it "the first version I'd call a real answer". **Independent critique: 24/40** (`critique_v3/`, `cae5069`, `ddb71e0`) |
| 9 | **v3.1** (`proposals/v3.1/`): fixes for my own v3 critique: HOME escalation, `ROW 1/2` labels, separate mute/OFF marks (OFF became a ring symbol), lane key, SNAKE path as a picture, square corners, 32 px bottom-band toast, palette tokens on all palettes | `d9bdd9a` | superseded by v3.2 |
| 10 | **v3.2** (`proposals/v3.2/`): answers every critique finding (53-row table in its NOTES.md): hand-following AF_M cue, toasts that lead with the printed button and state consequence/exit, map split in two pages, 15 x 4 px module (99 of 99 elements on it), 15-state sheet, MIDI channel on the tile, accent as a shape. 11 text roles / 28 strings while playing | `4d69407` | **Independent critique: 27/40** (`critique_v3_2/`, `4a88dd0`, `5b97263`). Renders 0-pixel reproducible, no clipped text |
| 11 | I recorded decisions 116-120, then sent the v3.2 renders to the owner | `815bf75` | **Owner: "It's not good enough."** (no reason given) and asked for this package. Decision 120 (freeze) withdrawn |

## Scores (independent critiques, 10 Nielsen heuristics 0-4)

| Version | Total | Notes |
| --- | --- | --- |
| shipping layout (informal, pre-measurement) | 29/40 | not an independent run; do not compare directly |
| v3 | **24/40** | 4 P1 findings; moderate cognitive load (2 failed checks) |
| v3.2 | **27/40** | 3 P1 findings (lane shows velocity not pitch; CHN setup screen never drawn; mislabelled/unflagged risky moments); density verdict: **did not regress**, but ~12 non-brief items at knob-label weight |

27/40 is "Acceptable", the top of its band. The owner's judgement ("not good enough") is **consistent with the score**,
and the score is only part of it: the critique tests the screen against *our* workflow test, not against whether the
owner likes how it looks.

## What the owner has actually seen

- Explicitly sent as files (`SendUserFile`): the v3.2 set (playing, idle, row 2, mutes, stress, dice strip, preset
  toast, undo toast, cue series, map A, map B, states sheet).
- Earlier mock-ups (current, v1, v3) I displayed in the session with the file reader and described in text; I cannot be
  sure the owner saw each image. **Assume the owner has seen v3.2 and may only have *read about* the others.**
- The owner has **not** seen the screens on the real unit.

## Lessons (process honesty; these are the reasons this package exists)

1. **I over-praised v3** ("a real answer") before an independent critique scored it 24/40. Do not tell the owner
   something is good until a fresh-eyes critique has said so, and always say what the critique found wrong.
2. **I let multiple-choice questions make big design decisions.** The owner chose "glance screen + map on demand" from
   a text description. The thing they cared most about (buttons always visible) may have been traded away without their
   seeing the alternative. **Show pictures of each option before asking.**
3. **Agents build what the brief says, and report their own work glowingly.** Each mock-up agent said what it was
   "least sure about" (good), but also mislabelled controls (`SAVE` hold = `BACK`), drew scenes with bugs (the
   independent reviewer found three in v3.2), and invented names (`SNK3`). Verify against the TSVs and look at the
   images yourself.
4. **Rounds were expensive and slow** (several 15-30 minute agent builds per version). Consider shorter loops with
   the owner looking at a handful of 1x images, instead of 70-scene proposals.
5. **I estimated text counts by eye and was wrong** (55 vs 69-75). Count by script (`mock/` + `critique_v3*/scripts/`).
6. **I never put a render on the real unit.** The owner has now said "30 mm", and all legibility numbers are rules of
   thumb. An on-device preview is the most reliable next step (see `07_ACCEPTANCE.md`).
7. The two-assessment critique (design review + measured evidence) found real defects that the builder missed in every
   round. Keep it, but run it **once per round with a smaller scope** so it does not consume the whole session.
