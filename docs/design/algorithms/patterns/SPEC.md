# PATTERNS: the preset pattern library (P2-014)

Task P2-014. Status: **for owner review** (gate `human-review`). Design only: no firmware is changed. The maths is executable in
[`tests/design/patterns_proto.py`](../../../../tests/design/patterns_proto.py) (`--check` runs every check quoted here; every golden is its output). Data:
[`patterns.tsv`](patterns.tsv) (120 patterns, read by the prototype and, via the P5-008 generator, by the firmware). Research and sources:
[`RESEARCH.md`](RESEARCH.md). Goldens: [`patterns_golden.txt`](patterns_golden.txt). Scenario: [S20](../../scenarios/S20-patterns.md).

## 0. Summary

PATTERNS is the sixth v1 algorithm (owner decision 82; short name `PATT`). It is **not a generator**: it plays one of **120 library patterns** (6 categories x 20)
written as degree-relative data, quantised to the global scale and, while HARMONY is active, to the chord progression. Every pattern marks which steps are
**anchors** (they hold its identity) and which are **decoration** (ghost notes, ornaments, slides) or **optional adds**, so the same eight morphs and five dice
layers as every other algorithm thin, thicken, vary, shift and accent it without destroying it.

| Question | Answer | Where |
| --- | --- | --- |
| Categories and counts | `ACID` `HOUSE` `FUNK` `TECHNO` `DUB` `MELODY`, 20 each = 120 | 1 |
| Naming (decision 90) | `<CATEGORY> <n>` style names only; song titles live in RESEARCH.md and a test greps the firmware strings for them | 2 |
| Data and size | 2 B per step, 4 B per pattern: 4472 B of `static const` flash; **0 B RAM, 0 B pool beyond the per-algorithm `AlgoState`** | 3, 8 |
| Pick UX | `OCT+` tap, turn the knob under `PICK` (S1); a fast detent jumps a category. 2 gestures (setup); stopped it auditions | 5 |
| Morphs | `DENS VARY SPAN ACCNT` / `PICK SLIDE DYN CTONE`; every default reproduces the stored pattern | 4 |
| Scale or chord change | re-quantised at the next step (no resolved pitch is stored); inert HARMONY = byte-identical | 6 |
| Dice | NOTES = which decoration notes vary, HITS = thinning order, ACCNT, ARTIC = slides, SHAPE = up or down | 7 |
| Editable? | not copied into steps: it stays generated and the per-step edit layer sits on top, exactly as for the other algorithms | 5.4 |

## 1. Categories (chosen by the research, owner addition 2C)

| Tag (cell) | Name (header, toast) | Count | Idioms covered (owner's list) | Famous-idiom (`F`) patterns |
| --- | --- | ---: | --- | ---: |
| `ACD` | `ACID` | 20 | acid house, acid techno, 303 accent / slide / octave lines | 4 |
| `HSE` | `HOUSE` | 20 | offbeat, deep, tech house, garage / UKG, Italo house | 0 |
| `FNK` | `FUNK` | 20 | funky bass, disco octaves, soul, boogie, walking | 4 |
| `TEC` | `TECHNO` | 20 | techno, minimal, electro, EBM, rolling bass, trance gallop (`TECHNO 8` is 12 steps) | 2 |
| `DUB` | `DUB` | 20 | dub, reggae, DnB / reese, jungle, dubstep half-time | 1 |
| `MEL` | `MELODY` | 20 | melody lines, synth-pop and Italo arps, call and answer, rising / falling sequences (the only `DEG` category) | 2 |

Why six x 20: RESEARCH.md 1 (each idiom has at least twenty distinct, widely used shapes; 20 per category is one 4-detent sweep of a "page" of the list and gives a
uniform PICK range of 1..120). The cell shows the 3-letter tag (`HSE 3`) because `HOUSE 20` is 56 px and the knob cell has 47 px of ink; the header, toast and status lines use the full name.
**Owner confirmation of this list is open item O2 (log).**

## 2. Naming rule and how it is checked (owner decision 90)

- Famous-song patterns ship their notes under the same `<CATEGORY> <n>` name as every other pattern. The `src` column of `patterns.tsv` is `F` (written in the idiom of a famous record) or `G` (generic). No title appears in `patterns.tsv`.
- The title list is the `<!-- TITLES -->` block of RESEARCH.md (13 titles). `python3 tests/design/patterns_proto.py --check` fails if any title appears (case-insensitive) in `patterns.tsv`, in any `firmware/src` `.c`/`.h`/`.tsv`, or in any `docs/design/*.tsv`. The P5-008 dev task adds the same grep over the generated `build/gen` pattern table.
- **Caveat the owner must see (open item O1):** decision 90 asks for "exact notes". They were reconstructed from memory, not verified against transcriptions, so the `F` patterns are idiom-faithful, not guaranteed note-exact, and they are one or two bars of bass or riff ostinato, never a melody of a song. Verify against a source the owner trusts or accept them as idiom.

## 3. Data format

### 3.1 The `patterns.tsv` grammar (source of truth)

Columns: `id cat num name mode src len steps`. `steps` is `len` whitespace-separated tokens (`|` between groups of four is cosmetic). A token is `.` (rest) or

```
[~] pitch [flags] [^rank]        pitch: integer; flags: A accent  S slide  T tie  s short gate  l long gate  C chord-tone flag;  rank: 1..3
```

| Part | Meaning |
| --- | --- |
| no `~`, no rank | an **anchor**: sounds at DENS 1..16, never removed or varied |
| rank `1`..`3` (flags then digit, or `^n`) | **decoration**: sounds at the stored density; removed as DENS falls (rank 3 first), varied by VARY. Rank 1 is the most structural decoration |
| `~` prefix | an **optional add**: a rest in the stored pattern that DENS above 8 fills in (rank 1 first); rank defaults to 2 |
| `C` | the step follows the chord progression (CTONE, 4.5) |
| `S` | glide into the next step (SEQUENCER_UX: "this note glides into the next step", crossing the loop end) |
| `T` | tie: the gate holds into the next sounding step |
| `s` `l` | short (about 25 %) or long (about 95 %) gate class; none = normal. The global GATE multiplies it (50 % neutral) |

### 3.2 Pitch modes

| Mode | Stored value | Resolved |
| --- | --- | --- |
| `SEMI` (ACID HOUSE FUNK TECHNO DUB) | semitones above the key, 0..48 | `snap(s)`, SP6: nearest scale tone, ties lower. The root, fifth and octave stay the root, fifth and octave wherever they are in the scale; a tone missing from the scale moves to its neighbour |
| `DEG` (MELODY) | scale degree 0..14 | `tones[d mod n] + 12 * (d div n)` with `n` the scale size, so the contour survives any scale (a 5-note scale gives a 5-note-per-octave version of the same shape) |

SEMI keeps harmonic identity, DEG keeps melodic contour; the category decides, so a bass line never changes its fifth and a melody never collapses its shape. Offsets are never negative (a
register below the root is not used) and a pattern spans less than 4 octaves, so `36 + key + 12 * octave + s` is inside 0..127 for every key and octave (checked, section 9).

### 3.3 Binary format (flash)

```c
/* 2 bytes per step. rest = 0x0000 */
/* b0: bit7 ON (1 = sounds)  bit6 TIE  bits0-5 pitch (0..48 SEMI, 0..14 DEG)
   b1: bit0 ACC  bit1 SLD  bit2 CT  bits3-4 RANK (0 anchor, 1..3)  bits5-6 GATE (0 short, 1 normal, 2 long)  bit7 OPT (optional add; ON is 0) */
typedef struct { uint16_t first; uint8_t len; uint8_t flags; } PatHead;  /* 4 B; flags bit0 DEG, bit1 F (idiom), bits2-7 0 */
_Static_assert(sizeof(PatHead) == 4, "PatHead");
static const PatHead pat_head[120];  static const uint16_t pat_step[1996];  /* id = 20 * category + n - 1 */
```

### 3.4 Storage cost (PROGRAM_PLAN 5)

| Item | Bytes | Where |
| --- | ---: | --- |
| 1996 steps x 2 B | 3992 | flash `static const` |
| 120 heads x 4 B | 480 | flash |
| category tags and names (6 x 12 B) | 72 | flash |
| **Library** | **4544** | flash (of the ~6 KB + pattern data line; image has about 235 KB of room after P3) |
| Per-channel state | `AlgoState` 20 B (8 morphs incl. `PICK`, 6 seeds) is the existing per-algorithm slot | pool (ADR-0003); no new field |
| Algorithms 4 to 6 | +20 B per channel per algorithm that is stored: PATTERNS adds 20 B x 4 channels = 80 B per preset, 1280 B over 16 slots (ARP adds the same; the P2-005 budget is 25128 B of presets in a 217975 B free pool) | pool, `ChanSave` |
| RAM `.bss` | **0 B** (no working buffers: the step is a pure function of the state, 4.7) | |
| CPU | at most one pass over `len` (32) steps per evaluated step: about 32 hashes and 32 x 32 rank comparisons, under 12 000 integer operations, once per 16th step | ISR budget TIMING_SPEC G6 untouched |

## 4. The algorithm

### 4.1 Pipeline (per evaluated step, position `i = g mod len`, `g` the generator position of ALGORITHM_SET)

```
sounding  <- anchors; + decoration kept by DENS; + optional adds taken by DENS            (HITS seed orders decoration / adds)
pitch     <- resolve (SEMI snap | DEG degree) -> SPAN fold -> VARY neighbour -> SP6 snap  (NOTES seed picks which, SHAPE seed picks up / down)
pitch     <- CTONE pull to the chord, only while HARMONY is active                       (flagged steps first; NOTES seed orders the rest)
accent    <- stored flag, ACCNT removes / adds                                           (ACCNT seed)
slide     <- stored flag, SLIDE removes / adds                                           (ARTIC seed)
velocity  <- 120 / 80 compressed or humanised by DYN                                     (ACCNT seed for the jitter)
gate      <- stored class (TIE kept), global GATE multiplies; note = SP9(pitch)
```

`h16(seed, i, salt)` (16-bit integer mixer, `patterns_proto.py`) is the only random source: the same state and seeds play the same pattern (VISION rule 4). Seeds: `s_notes s_hits s_acc s_art s_shape`.

### 4.2 The eight morphs (K1 to K4 ROW 1, S1 to S4 ROW 2; defaults reproduce the stored pattern)

| Knob | Label | Range, default | Effect (defined by the prototype) |
| --- | --- | --- | --- |
| K1 | `DENS` | 0..16, **8** | 0 = KEYS ONLY (nothing generated; entered steps still play). 1 = anchors only. 1 to 7: the decoration steps are ordered by (rank, HITS hash); the first `nDeco * (DENS-1) / 7` are kept, so rank 3 goes first. 8 = the pattern as stored. 9 to 16: the first `nOpt * (DENS-8) / 8` optional adds (rank 1 first) join |
| K2 | `VARY` | 0..8, **0** | each decoration or taken-optional step is replaced by the next scale tone above or below when `h16(notes) mod 8 < VARY` (direction `h16(shape)`). Anchors are never varied |
| K3 | `SPAN` | 1..4, **4** | octaves the pattern may reach above its own lowest note: a note `12 * SPAN` or more above it folds down by octaves. 1 = everything inside one octave (the TB-3 friendly squash); 4 = as written (every library pattern is under 4 octaves) |
| K4 | `ACCNT` | 0..8, **4** | below 4: remove `nAcc * (4-ACCNT) / 4` accents (hash order); above 4: add accents to `nPlain * (ACCNT-4) / 8` unaccented sounding steps |
| S1 | `PICK` | 1..120, **1** | the library pattern (section 5). Not a morph, but it sits in the morph store (`morph[4]`) so it is saved, kept per algorithm and reset like one |
| S2 | `SLIDE` | 0..8, **4** | below 4: remove `nS * (4-SLIDE) / 4` stored slides; above 4: add slides on non-slid steps that are followed by a sounding step |
| S3 | `DYN` | 0..14, **10** | 0..10: velocity contrast scaled `DYN / 10` (0 = flat 100); 10 = 120 / 80 as stored; 11..14: contrast full plus a fixed per-step velocity offset of +-3 x (DYN-10) / 2 (the **humanise**: deterministic, from the ACCNT seed, same every pass). Timing humanise is not offered (TIMING_SPEC; rule 4) |
| S4 | `CTONE` | 0..8, **2** | chord-tone bias, audible only while HARMONY is active. 0 = none, 1 = half of the `C`-flagged steps, 2 = all flagged steps (as authored), 3 to 8 = flagged plus `ceil(nOther * (CTONE-2) / 6)` more (NOTES-seed order, lowest rank first), 8 = every sounding step |

Deviations from the knob convention (ALGORITHM_SET 5.1): S1 is `PICK`, not `OFFSET`, because a library needs a selector and OFFSET (rotation) was the cheapest slot to give up: a rotated pattern is a different groove, and the producer who wants it
can rotate it by editing steps or choose another pattern; S4 is `CTONE`, not `READ`, because reversing a library pattern changes its identity and the chord-tone bias is the dial the owner asked for. K1..K4 keep DENSITY / FORM(VARY) / PITCH(SPAN) / SPICE(ACCNT).

### 4.3 Golden cases (min / mid / max)

`patterns_golden.txt` holds, for the first pattern of the golden set of **each of the six categories** (`ACID 1`, `HOUSE 3`, `FUNK 4`, `TECHNO 1`, `DUB 3`, `MELODY 1`), every morph at min, mid and max: DENS 1 / 8 / 16, VARY 0 / 4 / 8, SPAN 1 / 2 / 4,
ACCNT 0 / 4 / 8, SLIDE 0 / 4 / 8, DYN 0 / 10 / 14, CTONE 0 / 2 / 8 (against the chord G Bb F). It is MINPEN, key C, seeds 1 2 3 4 5. `--check` fails on any difference; `--golden` rewrites it for an intended change. Example (`HOUSE 3`, MINPEN, MIDI = 36 + s):

| Dials | One bar (16 steps) |
| --- | --- |
| as stored | `. 36 36 36 . 36 36 36 . 36 36 36 . 36 36 48` |
| DENS 5 | `. 36 . 36 . 36 . 36 . 36 36 36 . 36 36 48` |
| DENS 5, VARY 4 | `. 36 . 36 . 36 . 36 . 36 46 36 . 36 36 48` |
| DENS 5, VARY 4, SPAN 1 | `. 36 . 36 . 36 . 36 . 36 46 36 . 36 36 36` |
| same, chord G Bb Eb (chord 4) | `. 39 . 36 . 36 . 36 . 36 46 36 . 36 36 36` (the one `C`-flagged step is pulled) |

### 4.4 Anchor and decoration statistics

Every pattern has at least one anchor and at least one decoration or optional step, and at least one `C`-flagged step (`--check`). The library has 1996 steps; the golden shows that DENS 1 is exactly the anchor set for all 120 patterns (`neutral_identity`).

### 4.5 CTONE and the chord stage

PATTERNS opts out of the generic AUTO pull of HARMONY (`harm_apply`) and does its own through the same interface: while `harm_active()` it resolves a pulled step with `harm_snap(&chord, s, HQ_PC)` using the chord of the step's own tick (HARMONY 2.3, `harm_chord_at_bar(bar+1)` for a step evaluated one step ahead). Role OFF turns CTONE off; ROOT and FULL act as for every algorithm.
Candidates are kept at or above the key's base octave (the prototype's `chord_pull`) so a tie never falls below 0. With HARMONY inert (`harm_active() == 0`) PATTERNS is byte-identical to its goldens (H-INERT, section 9).

### 4.6 Loop and length

The generator loops over `len` (12, 16 or 32); the channel LENGTH is separate. A pick whose `len` is larger than the channel's LENGTH raises LENGTH to a multiple of 16 that holds it (toast `LEN 32`, SOURCE `OWN` for the new pages); LENGTH never shrinks on a pick. A 12-step pattern in LENGTH 16 repeats (the classic 12-over-16 shift) because the generator loop is its own `len`.

### 4.7 Statelessness

The pattern is a pure function of (pick, morphs, seeds, scale, key, chord, position): nothing is cached, so any change lands within one step, the global-scale re-quantise needs no stored pitch, and RAM is unchanged.

## 5. Selection UX

### 5.1 The control

| Gesture | Result |
| --- | --- |
| Turn ALGO to `PATT` | the six-name picker (5.5); the PERF screen of PATTERNS (5.2) |
| Tap `OCT+` (ROW 2 live), turn the knob under `PICK` (K1) | one detent = the next or previous pattern in the whole list (1..120); the cell reads `HSE 3`, the lane header names it in full |
| A detent less than 40 ms after the previous one | jumps to the first pattern of the next category (clockwise) or the previous category (counter-clockwise), clamped at the ends. A two-detent flick therefore moves one pattern then one category |
| Hold `OCT+` and turn K1 | the momentary peek of ROW 2: the same `PICK`, one control |
| Tap `OCT-` | back to the performance row (DENS VARY SPAN ACCNT) |

The first pattern is `ACID 1`. A pick lands at the next step (no note cut, decision 47) while playing. Pick is a **setup** function: 2 gestures, score 10 (5.6).

### 5.2 PATTERNS on PERF

```
 > INT 120   P01   1 PATT M2          <- Z0; H4 `1 PATT M2` is 71 px of 76
 [1 PATT][2 EUCL][3 SNAK][4 STAG]     <- Z1 cells; the selected cell is filled
 HSE 3   16 STEPS  PG 1/1  hold=EDIT  <- Z2 header: pattern tag and number, step count, page, hold role
 |lane: 16 cells, velocity bars, ' ! ~ glyphs, playhead|
 OCT- |  DENS 8 |  VARY 0 |  SPAN 4 | ACCNT 4      <- ROW 1 (live when filled)
 OCT+ |  PICK HSE 3 | SLIDE 4 |  DYN 10 | CTONE 2   <- ROW 2
 [FX ALL][SCL NOTES/VARY][ENV ACCNT][LFO SHAPE/UP/DN][EDIT][GLO] ...
 PICK: HOLD OCT+ AND TURN K1                         <- cue strip (first time a PATT channel is shown)
```

Toast when the pick changes: `PATTERNS: HOUSE 12  LEN 16` (162 px of 224). The band status of a single-row page is not used on PERF. Widths: `tests/design/widths.tsv` (all within their limits).

### 5.3 Preview and audition

- Playing: the new pattern is heard from the next step and plays on its own steps (the lane redraws at once).
- Stopped: 0.4 s after the last PICK detent the pattern auditions once, 16 steps at the current tempo through the channel's MIDI and the internal synth, then stops (the same audition path as a typed key, HARMONY 5.3). A new detent cancels it.

### 5.4 Is a loaded pattern editable steps?

**No copy is made.** The pattern stays generated, like every algorithm's output, so the morphs and dice keep working on it. The producer's own changes (tap a white key for ON / OFF, hold it for NOTE EDIT, REC) are **edits** in the per-step edit layer (SEQUENCER_UX 1.2): a set field is never touched by a morph, a dice or a PICK.
Difference from an algorithm channel: none in storage or editing; the difference is the source of the generated layer (a table row, not a seeded generator). Consequence: picking another pattern keeps the edits (as algorithm switching does, S7) and `EDIT` undoes the pick as one record. A step the producer turned OFF stays off in the new pattern. There is no "bake the pattern into steps" gesture in v1 (open item O3: a bake would cost the morphs; if wanted it is a later table edit).

### 5.5 The algorithm picker with six algorithms (decision 82)

Turning ALGO draws, for 1.2 s after the last detent, the picker over the lane header line (Z2, 14 px, AF_S). Six slots of 40 px, the current one inverted; the short names are the H4/Z1 names:

```
x=0       40       80       120      160      200     240
[ ACID ][ EUCL ][ SNAK ][ STAG ][ ARP  ][[PATT]]      <- PATT current (inverted)
```

Widths (px of ink, limit 40): ACID 26, EUCL 29, SNAK 30, STAG 29, ARP 22, PATT 27 (`tests/design/widths.tsv`); the six slots tile 240 exactly. The order is the ALGO encoder order (clamped, no wrap). The band, buttons and cue behind it keep drawing, so nothing else on the screen is lost. The same strip is the design for the ARP agent's six-algorithm text (P2-013 changed the EN_ALGO strings; this task owns only the strip).

### 5.6 L rubric score (setup pass >= 10, no sub-score below 2, at most 2 gestures; performance >= 11)

| Function | Path | G | V | I | O | L | Total | Why |
| --- | --- | ---: | --- | --- | --- | --- | --- | --- |
| Choose PATTERNS | ALGO | 1 | 3 | 3 | 3 | 3 | 12 | picker names all six; the printed word ALGORITHM |
| PICK a pattern | OCT+ tap, K1 | 2 | 3 | 2 | 3 | 2 | 10 | cell `PICK HSE 3`, header names it; tap + turn = two actions (I2); the OCT+ role is named `h:peek` on PERF (L2) |
| PICK category jump | the same knob, a fast detent | 2 | 3 | 2 | 3 | 2 | 10 | the cell and header change category; a cue names it the first time |
| PICK, peek form | hold OCT+, K1 | 1 | 3 | 1 | 3 | 2 | 9 | modifier + control (I1); a convenience, not the scored path |
| Thin / thicken (DENS) | K1 | 1 | 3 | 3 | 3 | 3 | 12 | one knob; the lane shows the notes appear and go |
| VARY, SPAN, ACCNT | K2, K3, K4 | 1 | 3 | 3 | 3 | 3 | 12 | one knob each, labels on screen |
| SLIDE, DYN, CTONE | OCT+ tap, S2..S4 | 2 | 3 | 2 | 3 | 2 | 10 | the row-2 convention of the other algorithms |
| Dice a layer | SCL SEQ ENV ARP LFO | 1 | 3 | 3 | 3 | 3 | 12 | the shared dice buttons; sub-labels `VARY ORDER ACCNT SLIDES UP/DN` |

## 6. When the global scale or chord changes

- **Scale or key change (SET/SCALE page K1/K2):** the very next evaluated step is resolved with the new mask: nothing is stored resolved. SEMI patterns snap to the nearest tone (golden: `HOUSE 3` VARY 4, MINPEN `46`, penta-major `45`); DEG patterns re-map degree by degree, so the contour is exact in every scale. Two SEMI notes may collapse onto one tone in a sparse scale (a documented consequence of quantising; DEG patterns do not collapse).
- **Chord change:** the step uses the chord of its own start tick; changes land at the bar boundary (HARMONY 2.3). Only `C`-flagged steps (and, with CTONE above 2, more) move; the rest stay on the scale.
- **Octave change (SET OCT-/OCT+):** a transposition of the whole line by SP9; the pattern is unchanged.
- **HARMONY inert:** no chord stage, byte-identical (H-INERT, section 9).

## 7. Dice layers (the five shared buttons)

| Button | Shared name | PATTERNS seed | What it re-rolls | Sub-label |
| --- | --- | --- | --- | --- |
| SCL | `NOTES` | `s_notes` | which decoration notes VARY changes, and the order in which CTONE pulls the non-flagged steps | `VARY` |
| SEQ | `HITS` | `s_hits` | which decoration notes DENS drops first and which optional notes it adds first (within a rank) | `ORDER` |
| ENV | `ACCNT` | `s_acc` | which accents ACCNT removes or adds, and the DYN humanise offsets | `ACCNT` (drawn once) |
| ARP | `ARTIC` | `s_art` | which slides SLIDE removes or adds | `SLIDES` |
| LFO | `SHAPE` | `s_shape` | the up or down direction of each VARY replacement | `UP/DN` |
| FX | dice all | all five | every layer; **the pattern pick is never diced** (the pick is a choice, not a layer) | `ALL` |

All five layers are present, so none is dimmed (ALGORITHM_SET 4.4). Dice never changes the PICK, the morph values or any edit. Undo (`EDIT`) swaps each seed with its previous seed; a PICK is its own undo record.

## 8. Budget summary (PROGRAM_PLAN 5)

Flash +4544 B of data (+ about 1.5 KB code, an estimate for P5-008). RAM +0 B. Pool: the existing `AlgoState` slot, +80 B per preset (1280 B over 16 slots), recorded in P2-005 as part of ARP/PATTERNS. Nothing here moves a floor.

## 9. Checks (all run by `python3 tests/design/patterns_proto.py --check`)

| Check | Result |
| --- | --- |
| 20 patterns in each of the 6 categories; unique names; lengths 12 / 16 / 32 | pass |
| at least one anchor, one decoration or optional step, one `C` flag per pattern; pitches in range; span under 4 octaves | pass |
| neutral identity: the dials at their defaults give exactly the stored pattern (SEMI snapped, DEG resolved) in MAJOR, MINPEN and CHROMATIC; DENS 1 is exactly the anchors | pass |
| range and scale sweep: 120 patterns x 16 scales x 12 keys x 5 octaves x every dial extreme x HARMONY inert and a chord: all notes in 0..96 and `36 + key + 12 * octave + s` in 0..127; with HARMONY inert every note is in the scale | pass, 492000 note checks |
| morph goldens at min / mid / max for one pattern per category | pass (`patterns_golden.txt`) |
| song-title grep over patterns.tsv, firmware sources and design TSVs | pass, 13 titles absent |

## 10. Table additions

`knob_map.tsv`: eight `PERF PATTERNS K1..S4` rows. `help_text.tsv` is generated: the `{r1}` / `{r2}` placeholders expand with the new labels (`DENS VARY SPAN ACCNT`, `PICK SLIDE DYN CTONE`) and `check_help.py` passes. `widths.tsv`: the PATTERNS strings. `ALGORITHM_SET.md`: a PATTERNS section. `S20-patterns.md`: the scenario. `control_map.tsv`, `button_modes.tsv`, `key_map.tsv`, `page_states.tsv` need no new row: PATTERNS adds no control, only a ROW 1/ROW 2 reading and the picker strip, and the EN_ALGO text already names the six algorithms.
