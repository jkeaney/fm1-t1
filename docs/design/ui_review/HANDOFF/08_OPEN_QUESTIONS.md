<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 08 Open questions (what we do not know)

## 0. Interview answers (design session, 2026-11-12, owner shown v3.2 `glance_playing_4x` and `map_A_4x`)

- **What is wrong with the v3.2 main screen:** still too busy/small; looks generic; only one channel's pattern. In the
  owner's words: *"it needs to scale, sometimes a producer will just have 1 channel, add another, and another. it should
  only showcase 4 channels when they are set up and in use."* and *"the bottom row 1 row 2 and the long text, whats that
  mean, its a waste of space. a user of this device needs to know how it works instantly."*
  => H2 confirmed in a new form (**the screen scales with the number of channels in use**); H3 and H4 confirmed;
  the ROW strip and the AF_M cue line are rejected. H1 (button rows on the main screen) was **not** picked as the main fault.
- **Held map:** *"no wrong idea. if we have an instruction screen it needs to be shaped like the device so the user can
  actually navigate it, when i look at this, i have no idea what its telling me. also we should use icons to help instead
  of text. dice image. play image. etc."* => any control documentation must be **shaped like the panel** and use
  **icons** (dice, play, ...) over words. H5 confirmed.
- **On a press:** both a brief large flash **and** a visible highlight of what changed.
- **Knob row:** a **small marker** (dot/number), not a text strip.
- **Colour:** **a colour per channel** (used for its pattern and tile).
- **Black keys:** the unit carries the **stock labels** (OP1..OP6, PIT, GLD, MONO, POLY); open question 3.1 stands: the
  screen must relate black-key roles to those positions, not to our names.
- **Viewing distance:** **closer, about 30-40 cm** (unit on the desk). At 35 cm: AF_S 1.1 mm = ~10.8 arcmin, AF_M 1.4 mm =
  ~13.7 arcmin, AF_L 2.6 mm = ~25.5 arcmin. AF_M is now comfortably glanceable; AF_S is glance-marginal.
- **"Good enough" means:** instant to understand (recorded in `07_ACCEPTANCE.md` section A).
- **Backlog:** P4-012 **blocked** by the owner until the design is locked.

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

## 0b. Design rounds in the design session (2026-11-12)
- v4 (`proposals/v4/`): scaling rows, colour per channel. Owner: "cleaner, but much less ... not enough info";
  asked for a process: space and grid, every function and its context, the smallest novel encoding per item, options
  section by section, then fit.
- v5 (`proposals/v5/`): options per section + layouts A/B. Owner picks: music = lanes but **one slim row**, each step
  split 1-4 by the channels playing there; knobs = **K-D tile is the meter**; buttons = **icons only**; layouts: neither
  (the keybed strip and the footer).
- v6 (`proposals/v6/`): footer options F-1..F-3: "none of these are helpful ... id like to know about the keybed, and
  which mode the black and white keys are in" -> **F-4 keybed footer** (white keys = the selected channel's steps,
  black keys = role glyphs, a mode line; REC turns them into note keys): owner **"Yes"**.
- v7 (`proposals/v7/`): icon review (a die frame = re-roll, the inside = the layer). Owner: icons **"Yes"**; v7 overall
  **"Good enough"** to go to the independent critique and the remaining states. Not yet locked.
- Independent critique v7: **20/40** (`critique_v7/`). v8 (`proposals/v8/`) fixed its list and moved every
  algorithm-specific item into one registry (`algos_v8.py`, parsed from knob_map.tsv; owner asked for extensibility).
  Independent critique v8: **19/40** (`critique_v8/`): P0 MIDI OFF nearly invisible; REC keeps the PERF knob roles
  (REC = CURSOR ACC SLIDE OCT); the MIDI setup route is not discoverable. P1: 3 px music slots lose accents/mutes at 4
  channels; hold notch invisible; dice amber vs CH1 orange; enum knobs get a meaningless meter; value names/formats/
  lane mode are not in AlgoDesc. Evidence: 0-px reproduction, 8 TSV mismatches (ARP DIR parse, PICK `HOUSE 3`, STAGE
  `(n)`, REC cue text, header vs pending frame collisions).
- Owner decisions after critique v8 (2026-11-12): **music row = the selected channel full width** (pitch bars +
  marks), the other channels as thin hit strips under it; **hold actions = a hint while held** (0.25 s: the top bar
  names the hold action before it fires; the corner notch goes); **a channel is added by turning SELECT past the
  last channel** (`+` becomes a new channel; needs a button_modes/knob_map row).
- v9 (`proposals/v9/`, NOTES.md has the AlgoDesc UI-field proposal). Independent critique v9: **24/40, no P0**
  (`critique_v9/`). P1: left encoders (SELECT/PRESETS/ALGO) and OCT-/OCT+ not named on the main screen; the knob-row
  marker is too small and far from the knobs; other-channel hit strips 2 px, colour-only; music row only 15% of the
  screen (buttons + keybed 43%); channel colours are not theme tokens yet; extensibility needs a MorphDesc/AlgoDesc
  contract change (ADR). Evidence: 0-px reproduction, 964 knob values fit, 832 channel-set renders clean; P02? pill
  overlaps the playhead bar on steps 1-3; GLO/SAVE hold hints not rendered; REC cue missing `REC=done`.
- Owner decisions after critique v9 (2026-11-12): encoders and OCT-/OCT+ are named **only while touched** (top bar
  while turning/holding; no permanent legend); **shrink the button tiles** to give the music row about 1.5x height;
  **write the ADR amendment** adding the UI fields to AlgoDesc/MorphDesc (value names, unit, zero word, lane type,
  optional value-text hook) as part of the hand-off.
- v10 (`proposals/v10/`). Independent critique v10: **27/40** (`critique_v10/`). P0 (reviewer A): MIDI setup still not
  discoverable from the screen (only CHN key glyph; `CHN=SET` cue appears only after a conflict; CHAN page not drawn).
  P1: hit strips lack fixed identity; row/page squares unlabelled (~8 arcmin); ARTIC/HITS die icons unreadable; toasts
  hide the header (incl. MIDI OFF); deuteranope CH1/CH3 clash. Evidence: 0-px reproduction, 1572 knob renders and 546
  state renders clean; check() misses value lists >= 14 entries (dots 1 px or 0); `turn_algo` shows the new algorithm
  before it lands; ROW 3 should show the device's parameter names; REC write cursor not drawn; "SELECT past last adds"
  conflicts with knob_map "clamped, no wrap" (TSV row needed). Coverage 92% of PERF rows incl. hints/toasts.
- Owner decisions after critique v10 (2026-11-12): **MIDI setup cue** as the one exception to "named only while
  touched": when a channel has just been added or has MIDI OFF, the footer reads `CHN KEY = MIDI` and the CHN key
  lights; **toasts and hold hints use the right side of the top bar only** (play/BPM/channel chips stay visible);
  **hit strips get a fixed row per channel** (CH1-CH4 always in the same row, the selected channel's row empty).
- v11 (`proposals/v11/`). Independent critique v11: **26/40, no P0** (`critique_v11/`; v10 27, v9 24, v8 19). P1:
  other-channel strips 3 px (~3.7 arcmin); boxed digits used for channels, knob rows and page; playhead column 2.6:1
  and its cap looks like the accent mark; MIDI prompt absent for an existing channel (factory CH1); full knob fills read
  as "selected". Evidence: 0-px reproduction, 8704 state renders clean; 15 of 75 possible top-bar messages would be cut
  (`CH1 PATTERNS`, `LAST ROW: OCT- = ROW 1`, ...); mismatches: SELECT add vs "clamped", ALGO_PICK cue list, Pnn> state,
  REC CURSOR 1..LENGTH, hold-bar timing, button sub-labels. The score has plateaued at 24-27 across three rounds.
- **Owner, 2026-11-12, on v11: "Good enough, move on"** (shown the v11 sheet and the 26/40 critique with its five P1s).
  The v11 direction is accepted; next: draw the remaining states on it, then the hand-off. The design is NOT yet
  "Design locked" (ADR-0004): that line is written after the owner sees the full state set.
