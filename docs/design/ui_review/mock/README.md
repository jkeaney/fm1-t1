<!-- SPDX-License-Identifier: GPL-3.0-only -->
# FM-1 pixel-exact mockup tool

Renders UI proposals for the 240x240 RGB565 screen with the REAL firmware fonts (AF_S / AF_M / AF_L, parsed from
`build/gen/ui_fonts.h`, glyph phases, kerning and the 4-bit alpha ramp included), the real palettes (`build/gen/ui_palettes.h`,
derived as `gfx.c palette_set` does) and the real label tables (`build/gen/ui_tables.h`). Pillow only; deterministic. Run
`./build.sh` once so `build/gen/` exists (or point `FM1_GEN` at a copy). Use the repo venv: `.venv/bin/python`.

Colour expansion RGB565 to sRGB is the one of the `png_write()` in `tests/ui_sq_test.c` (r5\*255/31, g6\*255/63, b5\*255/31, rounded
down), so a mock and a `build/ui_sq/*.png` are comparable pixel for pixel. Default palette GREY (index 0, what the test renders).

## Commands (from the repo root)

```sh
M=docs/design/ui_review/mock
.venv/bin/python $M/mock.py render $M/example_proposal.py --out /tmp/mock_out      # NAME.png, NAME_4x.png, NAME_grid.png per scene
.venv/bin/python $M/mock.py diff A.png B.png --out diff.png                          # count + clusters; red = differing pixel (4x)
.venv/bin/python $M/mock.py measure "HOLD=EDIT NOTE" --font S --cell 36             # advance, ink box, cap height, fits?
.venv/bin/python $M/mock.py contrast TEXT BG --palette GREY                          # WCAG ratio (tokens, 0xRGB565 or #rrggbb)
.venv/bin/python $M/mock.py palette GREY                                             # a palette's tokens as RGB565 / hex
.venv/bin/python $M/check_fidelity.py --regen                                        # the gate: current_perf.py vs the shipping render
```

`render` writes, per scene: `NAME.png` (1x, 240x240), `NAME_4x.png` (nearest neighbour) and `NAME_grid.png` (4x with a pixel grid,
every 10 px firm and numbered every 20; the zones and cells you `mark()` outlined and labelled `name x,y,w,h`; every text's ink
box in cyan, red when the text was cut or ellipsised). `diff` exits 1 when the images differ. Warnings (text cut by the canvas,
text ellipsised, canvas over the 240x124 strip) print while rendering.

## Defining a proposal

A proposal is a python file exposing `draw(canvas, state)` and a `STATE` dict (optionally `PALETTE = "AMBER"`):

```python
from mock import T, AF_S, AF_M, AF_L, text_w, measure, tables, fixture

PALETTE = "GREY"
STATE = {                                # one render, or "scenes": {name: {...}} for several (each merged over the top-level keys)
    "scenes": {"playing": fixture("perf_playing")},
}

def draw(cv, S):
    cv.begin(240, 108, T.BG)             # like cv_begin: size and clear the strip (the firmware's is at most 240x124)
    cv.text_fit(2, 5, AF_S, "1 ACID", T.TEXT, T.BG, 56)
    cv.rect(0, 20, 60, 28, T.SEL)
    cv.glyph1(10, 6, "#.#\n.#.\n#.#", T.ACCENT)   # 1-bit icon from ASCII art ('#' set, '.' clear); warns outside 1..10 px
    cv.mark("Z1 cell", 0, 20, 60, 28)    # outline + label in NAME_grid.png
    cv.blit(0, 0)                        # like cv_blit; with no blit at all the strip is blitted at 0,0
```

Primitives (names and semantics as `firmware/src/gfx.c`; colours are RGB565 ints, theme tokens `T.BG T.SURF T.TEXT T.THEME T.ACCENT T.MID
T.DIM T.LINE T.SEL T.TINT T.INK T.REC T.RAISE T.KEY T.LANE T.GRID` are read at use):

| call | firmware | note |
| --- | --- | --- |
| `begin(w, h, bg)` | `cv_begin` | clears; coordinates are canvas coordinates |
| `rect frame pset line line_t rrect` | `cv_rect cv_frame cv_pset cv_line cv_line_t cv_rrect` | clipped to the strip; rrect r<=8, corners blended against `under` |
| `text(x, y, f, s, fg)` / `text_on(.., bg)` | `cv_text` / `cv_text_on` | y = line top (baseline y + asc); returns the pen x at the end |
| `text_r(xr, y, f, s, fg, bg)` | `cv_text_r` | the advance ends at xr |
| `text_in(x0, y, w, f, s, fg, bg)` | `cv_text_in` | the INK centred across x0..x0+w |
| `text_fit(x, y, f, s, fg, bg, maxw)` | `cv_text_fit` | never a smaller font; the end becomes the ellipsis glyph (0x85) like the firmware, 48-char buffer |
| `blit(x, y)` | `cv_blit` | the strip to the panel |
| `glyph1(x, y, art, color, under=None)`, `paste(canvas, x, y)`, `mark(name, x, y, w, h)` | (mock only) | icons, composition, annotation |

Helpers: `text_w(font, s)` (the firmware advance), `measure(s, font, cell)`, `contrast_ratio(a, b)`, `tables()` (label rows:
`knob_label(page, ctx, knob)`, `btn_label`, `key_label`, `page_cue`, enum ids in `.enums`), `fixture(name)` (the test state, below).
Fonts: AF_S 12 px / 400 (line 15, caps rows 3..11), AF_M 15 px / 500 (line 19, caps rows 4..14), AF_L 28 px / 600 (digits and capitals only).

### STATE fixtures

`fixture("perf" | "perf_row2" | "perf_playing" | "perf_snake_mute")` returns the state `tests/ui_sq_test.c` had when it wrote
`build/ui_sq/<name>.png` (made by `fixtures/gen_fixtures.sh` from `fixtures/probe.c`, a host probe; it does not touch the firmware):
`page sel vpage row playing bpm follow preset preset_pend playhead hot[4]`, `channels[4] {algo midi_ch(16=OFF) muted len pg_used snd0 p}`,
`morph[8]` (the selected channel's morph values), `lane[16] {on vel ed acc slide rep}` (the resolved steps of the viewed page),
`algos[6] {name sub[5] layer_n[5] knob_morph[8]}` (the test's ALGOS table). A proposal may build its own dict instead; the keys are
only a convention that `current_perf.py` reads. Knob / button / key labels are not in the state: they are looked up in the tables.

`current_perf.py` is the shipping screen as a line-by-line port of the draw code of `ui_sq.c`; copy it, or import it as
`example_proposal.py` does and redraw one zone.

## Diffing

`mock.py diff A.png B.png --out diff.png` prints the number of differing pixels and their 8-connected clusters (`x y w h px`), and writes a
4x PNG of A dimmed with differing pixels red and each cluster boxed in yellow. Compare a proposal with the shipping render, a
proposal with another, or the mock with a firmware PNG:

```sh
.venv/bin/python $M/mock.py diff /tmp/mock_out/example_proposal_perf.png /tmp/mock_out/example_proposal_playing.png --out /tmp/d.png
```

## Fidelity

`check_fidelity.py` renders `current_perf.py` and diffs the four scenes against `perf.png perf_row2.png perf_playing.png
perf_snake_mute.png` (exit 0 = 0 differing pixels). `build/ui_sq/*.png` are only as fresh as the last test run: if `build/gen/ui_fonts.h` is
newer they differ by glyph anti-aliasing; `--regen` recompiles and runs `tests/ui_sq_test.c` into a temp dir (not `tools/check.sh`,
not `build.sh`) and compares against that. Not reproduced: keycaps (`cv_keycap`, KC_ data), the icon sets (ui_icons.h), `cv_flip`, the
ramp cache's 6 slots (a cache; no pixel effect) and the CUSTOM palette. None is used by `ui_sq.c`'s PERF screen.

## Palette parser fix and the v2 base palette

The palette parser used to skip `HI-CON` and mis-index `MONO`; both are fixed. `HI-CON` is the base palette of screen v2 (SCREEN_SPEC_V2, ADR-0006); the v12 reference renders use it.
