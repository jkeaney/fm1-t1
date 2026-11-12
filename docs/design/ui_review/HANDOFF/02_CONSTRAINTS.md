<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 02 Constraints: the physical and firmware limits

Every number here was measured or read from the repo; sources are named. **If you change a constraint, say so
explicitly** (for example "I propose adding a font size", with the firmware cost): do not slide past one.

## 1. Physical

| Fact | Value | Source |
| --- | --- | --- |
| Screen | **240 x 240 px, RGB565, 30 mm wide** (owner measured, 2026-11-12) => **8 px per mm** (about 203 ppi) | owner; CLAUDE.md |
| Viewing distance | arm's length, 50-60 cm (hands busy); the owner may lean in for a held view | owner |
| Cap height, AF_S | 9 px = **1.1 mm** = about **7.6 arcmin** at 50 cm | measured |
| Cap height, AF_M | 11 px = **1.4 mm** = about **9.6 arcmin** at 50 cm | measured |
| Cap height, AF_L | 21 px = **2.6 mm** = about **18 arcmin** at 50 cm | measured |
| Reading thresholds used so far | comfortable sustained reading ~16-20 arcmin; a brief glance ~10 arcmin; below ~5 arcmin invisible at arm's length. `arcmin = cap_mm / 500 * 3438` | session working rule; **validate on the unit** |
| Glance-screen font rule | anything read while playing: **AF_L or a big shape**; AF_M only for short names of controls identified by position (K1..K4 labels, algorithm names); **AF_S only on temporary held views** | decision 116 |

**Caution:** the arcmin thresholds are rules of thumb. The single most valuable validation is **seeing the mocks on the
real screen at real size** (see `07_ACCEPTANCE.md`, "On-device preview"); a PNG on a laptop is not the same thing.

## 2. The panel (see `assets/panel_photo.jpg` = `docs/controls.jpg`)

- Left block: **2 x 2 encoders**: top row `MASTER` (analog pot, no screen role) and `SELECT`; bottom row `PRESETS` and
  `ALGORITHM`. Under it, buttons `OCT-` and `OCT+`.
- The **screen** is to the right of that block (about 15% of the device width in the photo).
- Right of the screen: a row of **four knobs** (`KNOB 1..4`), and under them a **2 x 6 button block**:
  row 1 `FX SCL ENV LFO EDIT GLO`, row 2 `HOME SAVE ARP SEQ PLAY/STOP REC`.
- Bottom: the keybed: **16 white keys (F3..G5)** and **11 black keys** in the 3-2-3-2-1 grouping (some labelled
  `OP1..OP6 PIT GLD MONO POLY` by the stock silkscreen: these do not match our roles; **the owner must confirm what
  is printed on the unit**, see open question 6).
- Important geometry: the knobs and buttons are **not under the screen**; they are to its right. The 4 knobs do not
  align with the 6 button columns (about 1.8 columns apart). A screen layout that "mirrors the panel" is therefore a
  **diagram by order and arrangement**, not an overlay.
- Encoders and knobs are **relative detent encoders** (no absolute position; CLAUDE.md lists no push action). Buttons are
  tap/hold; keys are force-sensing (velocity, poly aftertouch) for F3..C5 only.

## 3. Controls and their roles (the screen must document these)

Authoritative tables (do not invent roles; if a mock disagrees, the mock is wrong):
`docs/design/button_modes.tsv` (every button x page x tap/hold), `knob_map.tsv`, `key_map.tsv`, `control_map.tsv`,
`page_states.tsv`, `help_text.tsv`; narrative in `docs/design/SEQUENCER_UX.md`.

Roles on the main (PERF) page, from the tables (as of 2026-11-12):

| Control | Role |
| --- | --- |
| `SELECT` | turn: select channel CH1..CH4 (clamped, no wrap) |
| `ALGORITHM` | turn: algorithm of the selected channel (ACID EUCLID SNAKE STAGE ARP PATTERNS); lands at the next step; morphs and seeds kept per algorithm |
| `PRESETS` | turn: choose a preset slot P01..P16; becomes PENDING (header `Pnn?`); stopped: loads 1.5 s after the last detent; playing: at the next 768-tick bar. `SAVE` tap saves **into the pending slot and cancels its load** |
| `MASTER` | analog volume (preview synth + USB audio); no MIDI effect |
| `K1..K4` | the live **row** of morphs (ROW 1, ROW 2, or ROW 3 = the RIG wires when the channel has a device with a wire) |
| `OCT-` / `OCT+` | row buttons: tap latches ROW 1 / next row; hold + turn = momentary peek |
| `FX SCL ENV LFO` and `ARP SEQ` | the six **dice**: ALL, NOTES, ACCNT, SHAPE, ARTIC, HITS (the roles were chosen to fit the printed words: "B2 printed-word fit") |
| `EDIT` | tap = UNDO/REDO (toggle); hold = RESET layer |
| `GLO` | SET page (GATE, DIV, SWING, BPM; scale, octave, MIDI, length); hold = HARMONY page |
| `HOME` | tap = back to PERF; hold = (current spec) HELP while held, MENU on release after 0.7 s; **owner decision 118 changes this to: map while held, keep holding to escalate to MENU** (the TSV is not updated yet) |
| `SAVE` | tap = save the whole device state to the slot in the header (or the pending slot); **hold 0.7 s = undo the last preset load (again = redo); while a slot is pending: cancel it** |
| `PLAY`, `REC` | transport, record arm |
| 16 white keys | the 16 steps of the viewed page: tap on/off, hold = edit the note; in REC they play pitches |
| 11 black keys | `M1..M4` mute channels (hold = preview), `CHN` (channel setup page; hold = RIG), solo, fill, roll, page-/page+, restart |

## 4. The three fonts (fixed; there is no other)

| Font | Line / cap height | Notes |
| --- | --- | --- |
| `AF_S` | 12 px line / **9 px caps** | average glyph ~6.5 px; `ACCENT` 46 px, `EXT 126` 45 px |
| `AF_M` | 15 px line / **11 px caps** | `EXT 126` 57 px |
| `AF_L` | 28 px line / **21 px caps** | **digits and capitals only** |

Real glyph data: `build/gen/ui_fonts.h` (generated); the mock tool parses it exactly. **There is no size between AF_M
and AF_L.** Adding one is a possible proposal but has a firmware cost (font data, flash, generator) that must be stated.

## 5. Drawing, memory and build rules

- **Canvas:** one blit is at most **240 x 124 px** (`CV_MAX`). A screen is drawn as up to three blit strips (A = Z0-Z2,
  B = Z3-Z4, C = Z5-Z7 in SCREEN_SPEC). Text drawn across a strip edge is clipped (the current screen clips `HLP/MN`
  this way: the mock tool warns about it).
- **Redraw rule:** redraw a zone only when its state signature changes or `ui.force` is set.
- **API:** `cv_begin / cv_rect / cv_rrect / cv_text* / cv_blit`, fonts `AF_S/AF_M/AF_L`, theme tokens `T_*`.
- **Theme tokens:** `T_BG T_SURF T_TEXT T_THEME T_ACCENT T_MID T_DIM T_LINE T_SEL T_TINT T_INK T_REC T_RAISE T_KEY T_LANE
  T_GRID`. **Nine palettes** exist in the generated header: `GREY` (default) `GREEN AMBER ICE VIOLET ROSE PAPER NIGHT
  MONO`. (A `HI-CON` palette is mentioned in the spec but is not in the generated header.) On GREY, `T_ACCENT = T_REC =
  T_TEXT` (all white), so **there is no accent colour in the default palette today**: any new colour is a new token,
  needs a value on every palette, and needs a monochrome fallback (v3.2 did this; see its NOTES section 5-7).
- **Labels:** at most **6 characters** per control label; `tests/design/widths.tsv` holds per-zone width limits; the
  tests fail on a label that does not fit. Labels live in the TSVs and are compiled into `build/gen/ui_tables.h` by
  `tools/gen_ui_tables.py` (which also checks widths and enumerates known contexts).
- **RAM:** the firmware must keep >= 8 KB of `.bss` headroom (E9 trigger); build line at 2026-11-12: `RAM .data+.bss
  23828 B of 73728` (about 49.9 KB free) and the UI code `ui_sq.c` is not in the device build yet, so there is room, but
  any UI state, icon bitmaps or string tables must be costed.
- **Freestanding C:** no libc beyond memset/memcpy/memcmp; no float-heavy rendering; icons are small bitmaps or drawn
  from primitives. Everything is `static` in a unity build.
- **Host tests render the screen:** `tests/ui_sq_test.c` writes PNGs to `build/ui_sq/`; hardware is the final judge
  ("Every change must be checked on a real FM-1").
- **Spec and design tables are generated and checked:** `tests/design/check_scenarios.py`, `check_help.py`,
  `width_probe.sh`, `gen_help.py`, `gen_sweeps.py`. A change to a TSV means re-running them.

## 6. What already exists in firmware (so you know what a design would replace)

- `firmware/src/ui_sq.c` (P4-011, reviewed, **done**, not wired into the device build): the **old dense layout** (zones
  Z0..Z7 per SCREEN_SPEC: header, 4 channel cells, lane, knob band, 12-button picture, black-key legend, cue strip).
  It was built *before* the 30 mm constraint was applied. **The glance design replaces its zones.**
- The new sequencer core (`sq_core.c`, `sq_chan.c`, timing, `sq_algo_acid.c`) is done and reviewed, not yet driving the
  screen. `P4-012` (the cut-over) is the task that wires `ui_sq.c` in; see the backlog hazard in `07_ACCEPTANCE.md`.
- The T-1 legacy screen (`ui_t1.c`) is what is on the unit today.
