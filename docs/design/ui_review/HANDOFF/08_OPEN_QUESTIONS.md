<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 08 Open questions (what we do not know)

## 1. What is "not good enough"? (do this before anything else)

The owner said "It's not good enough" after seeing the v3.2 renders and gave **no reason**. Do not guess. **Interview
with pictures**: send the v3.2 images one at a time (see `09_ASSET_INDEX.md`) and for each ask at most three things:
*what does your eye go to first? what do you try to read and cannot? what is missing?* Then ask the one summary question:
*"If you could change one thing, what?"*

### Hypotheses to test (these are my guesses, **not** facts; rank them by asking)

| # | Hypothesis | Evidence for it |
| --- | --- | --- |
| H1 | **The buttons should be visible on the main screen.** The owner's own sketch showed both button rows always visible ("the two rows of buttons are always visible to what they do"; "everything the user can access is documented on the screen"). The glance choice may have been made without seeing that alternative | `03_DECISIONS.md` section C row 1. **Test by drawing both and asking** |
| H2 | **The other three channels' patterns should be visible** as small dots/markers with mute state (the owner's sketch: "pattern distribution for the current page ... whether it's muted or not") | v3.2 shows only the selected channel's lane |
| H3 | **Still too busy / too many items** (28 strings, 11 text roles, many AF_M items of equal weight) or **too small** (AF_M is 9.6 arcmin; the owner may not read it at their distance) | independent critique: "tile soup", reference data at knob-label weight |
| H4 | **No identity / generic look**: grey tiles, one orange accent; not the Torso/Elektron feel the owner named | critique AI-slop verdict |
| H5 | **The cue band, toasts or the hold-HOME map are not how the owner imagines learning the device** (a held map is a new gesture; toasts cover parts of the screen) | owner chose the architecture from text only |
| H6 | **Something specific is wrong in a picture** (lane, playhead, tile, colours, a label) that the critique did not weigh | possible: the critique tests our workflow, not the owner's taste |
| H7 | **The sizes are still too small on the real unit**, which the owner could see but we cannot | no on-device check was ever done |

## 2. Open design questions left by the v3.2 builder (with my default if the owner has no view)

| # | Question | My default |
| --- | --- | --- |
| a | ARP K3: `SPAN` (firmware label) or `OCTS` (v3.1 rename)? | `SPAN` (no table change); ask the owner |
| b | MAP: split in two pages (A buttons/encoders, B keys)? How to switch pages (OCT while HOME is held collides with touch-for-help)? | split; decide the switch with the owner |
| c | What is printed on the encoders (`MASTER`, `SELECT`, `PRESETS`, `ALGO`/`ALGORITHM`)? `ALGORITHM` is 65 px in AF_S and does not fit a 58 px tile | confirm on the unit |
| d | Dice feedback: toast (1.2 s) then cue (3 s), toast only, or cue only? | cue-only keeps the whole screen visible; test with the owner |
| e | Toast height: 40 px slab or a 32 px AF_M-only preset form? | 40 px; hides empty tile padding only |
| f | MIDI channel on the tile as `[arrow]n`, OFF as `--`? | `->2` style; the word `OFF` does not fit |
| g | `UNDO L` (the TSV label) or `UNDO LD` (does not fit)? | `UNDO L` |
| h | REC: ring when armed and stopped, solid disc when recording? | keep; owner to confirm |
| i | FOLLOW off: a persistent indicator? | yes (small); the ghost column alone is weak |
| j | ROW 3: which names/ranges does it show (RIG wires, or the MOD page of `docs/design/MODULATION.md`)? | RIG wires per `button_modes.tsv`; content is a TB-3 placeholder |
| k | HOME escalation timings (map at once; countdown at 1.5 s; menu ~1 s later?) | match decision 118; confirm exact times |

## 3. Facts the design session must establish (the last session could not)

1. **What is printed on the 11 black keys of the unit** (`docs/controls.jpg` shows `OP1..OP6 PIT GLD MONO POLY`, the
   stock labels). Our cue names `M1..M4`, `SOL`, `CHN`, `RST` etc. A first-timer reading the unit will not find those
   words. **Ask the owner to read what is printed** and decide whether the screen's black-key roles need to relate to
   the printed words or the owner will label the keys.
2. **The owner's real viewing distance and whether they ever lean in.** (30 mm is known; 50-60 cm is assumed.)
3. **How many text items feel right** (give a number or a reference screen: "like the Torso T-1 page" / "like an
   Elektron Digitakt").
4. **Which palette the owner uses** (GREY is the firmware default; others exist) and whether the owner **wants colour**
   at all. Colour was never rejected, but every colour decision so far was mine.
5. **How the owner expects to learn the roles of the 12 buttons** (printed words + the screen? a manual? by trying?).
6. **How many more mock rounds the owner will tolerate**, and whether they would rather react to 3-4 images per round.
7. **What ROW 3 / RIG shows in practice** (MODULATION.md, `docs/design/PREVIEW.md`, HARDWARE/TB-3 profile): the
   screen must not invent labels; check the algorithm specs under `docs/design/algorithms/*/SPEC.md`.

## 4. Technical unknowns (state their cost if you rely on them)

- **A font size between AF_M and AF_L** (a candidate fix for "algorithm names are 9.6 arcmin"): font data + flash +
  generator; the last session measured that no 4-letter algorithm name fits AF_L in 56 px.
- **Icon/glyph blits** (transport shapes, enum pictures, hold chevrons): drawn from primitives or tiny 1-bit bitmaps;
  the mock tool has a `glyph1` helper; the firmware needs a matching routine.
- **New palette tokens** (v3.2 needs two or three) must have values on all nine palettes plus a monochrome fallback.
- **RAM/flash** for string tables (cue, toast, value names) and any preview frames.
- **The legacy T-1 screen** (`ui_t1.c`) is what runs on the unit today; the new screen is not on hardware at all.
