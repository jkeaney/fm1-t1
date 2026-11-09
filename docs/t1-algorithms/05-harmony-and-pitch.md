# 5. Harmony and pitch

**Input:** the cycle's `pitch_offset`, `phrase`, `range`, `style`, `scale_id`, `root_note`, `voicing`, `chord_size`; the pattern's `master_scale`, `master_root`; the step index and pulse counter.
**Output:** MIDI note numbers for each chord voice of each repeat.

## 5.1 The pitch chain, in order **[V]** (`torso_t1_pipeline.c`)

```
note  = 60                                           # middle C
note += pitch_offset                                  # -48..+48
note += phrase_contour(phrase, range, step, steps)    # §5.3
note += style_index(style, pulse_counter, pool = 7)   # §5.4   ← added on EVERY pulse, even for style 0
note  = lfsr_modulate(PITCH, note, depth)             # ±depth/2, only if the PITCH target bit is set  (§06)
for each repeat k:
    base   = note + (style > 0 ? style_index(style, pulse_counter * repeats + k, 7) : 0)    # a second time for style > 0
    base   = quantise(base, root, scale)              # snap DOWN into the scale (master_* override if master_scale > 0)
    chord  = chord_from(base, chord_size, scale, root)
    notes  = voicing(chord)
    notes[0] = p-lock pitch if one exists               # unquantised, after voicing
```
Two things here are surprising and are covered in [09](09-review-findings.md): the style index is added even for style 0 ("UP"), and for styles above 0 it is added twice (F5).

## 5.2 Scale quantiser **[V]**

A scale is a 12-bit mask over pitch classes **relative to the root** (bit 0 = the root). `t1_harmony_quantize(note, root, scale)`:

```
if scale is chromatic: return note
repeat up to 12 times:
    pc = (note - root) mod 12
    if mask bit pc is set: return note
    note -= 1                       # snap DOWNWARD to the nearest in-scale pitch
```
It always moves down (never up), so a pitch is pulled to the scale tone **at or below** it. Measured, root C, raw 60 to 71 (current tree):

| Scale | Output |
| --- | --- |
| Major (id 1) | 60 60 62 62 64 65 65 67 67 69 69 71 |
| Minor (id 2) | 60 60 62 63 63 65 65 67 68 68 70 70 |
| Pentatonic major (id 9) | 60 60 62 62 64 64 64 67 67 69 69 69 |

> **History:** when first reviewed, five of the sixteen masks held the wrong notes and the on-screen order disagreed with the engine's (findings F1 and F2: Major returned `… 64 64 66 67 …`, allowing F♯). Both were fixed in the working tree during the review; this chapter now describes the fixed tree. The original table is kept in `verification/probe.before-fixes.out.txt`.

### The 16 scales (`T1_SCALE_MASKS`, current order) **[V]**

Bit *n* = *n* semitones above the root. The order is the UI's `SCALE_NAMES` and the guide's list. A Phase 2 test checks every mask against its interval list.

| id | Name on screen | Mask | Intervals |
| --- | --- | --- | --- |
| 0 | CHROM | `0x0FFF` | 0 1 2 3 4 5 6 7 8 9 10 11 |
| 1 | MAJOR | `0x0AB5` | 0 2 4 5 7 9 11 |
| 2 | MINOR | `0x05AD` | 0 2 3 5 7 8 10 |
| 3 | DORIAN | `0x06AD` | 0 2 3 5 7 9 10 |
| 4 | PHRYG | `0x05AB` | 0 1 3 5 7 8 10 |
| 5 | LYDIAN | `0x0AD5` | 0 2 4 6 7 9 11 |
| 6 | MIXOL | `0x06B5` | 0 2 4 5 7 9 10 |
| 7 | AEOL | `0x05AD` | 0 2 3 5 7 8 10 (the same as MINOR) |
| 8 | LOCR | `0x056B` | 0 1 3 5 6 8 10 |
| 9 | P-MAJ | `0x0295` | 0 2 4 7 9 |
| 10 | P-MIN | `0x04A9` | 0 3 5 7 10 |
| 11 | BLUES | `0x04E9` | 0 3 5 6 7 10 |
| 12 | WHOLE | `0x0555` | 0 2 4 6 8 10 |
| 13 | H-MIN | `0x09AD` | 0 2 3 5 7 8 11 |
| 14 | M-MIN | `0x0AAD` | 0 2 3 5 7 9 11 |
| 15 | DIM | `0x06DB` | 0 1 3 4 6 7 9 10 (half-whole) |

Residue: `T1_SCALE_NAMES` in `torso_t1_algo.c` still lists the **old** order and is unused (see `../T1_ISSUES.md`, F2). The earlier "Dim W-H" scale no longer exists, and AEOL duplicates MINOR.

## 5.3 Melodic contour (phrase) **[V]**

```
max = range                                           # semitones (changed from range × 7 during the review, finding F4)
step i of N:  0 FLAT: 0
              1 SINE:  parabolic sine approximation, ±max
              2 TRI:   0 → +max at the midpoint → 0
              3 SAW U: 0 → +max (rises over the cycle)
              4 SAW D: +max → 0
              5 ARCH:  intended +max at the middle        ← actually constant +max (F3)
              6 INV A: intended dip in the middle          ← actually constant 0   (F3)
              7 RAND:  deterministic hash of the step index, -max..+max
range = 0 or N <= 1 → 0
```
Measured for `range = 7` (max 7 semitones), 16 steps (these values are unchanged by the F4 fix, which only changed how `range` maps to `max`):

| Shape | Offsets, steps 0 to 15 |
| --- | --- |
| SINE | 0 3 5 6 7 6 5 3 0 −3 −5 −6 −7 −6 −5 −3 |
| TRI | 0 0 1 2 3 4 5 6 7 6 5 4 3 2 1 0 |
| SAW U | 0 0 0 1 1 2 2 3 3 4 4 5 5 6 6 7 |
| SAW D | 7 6 6 5 5 4 4 3 3 2 2 1 1 0 0 0 |
| ARCH | 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 |
| INV A | 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 |
| RAND | −7 −7 0 0 7 7 −1 −1 6 6 −2 −2 5 5 −3 −3 |

ARCH and INV A are constant because `(norm × norm) / (N² + 1)` is integer division and is always 0 (F3, **still open**). With the current tree a constant ARCH at `range = 1` is a fixed +1. RAND repeats each value twice and follows a regular progression (a hash artefact).
`range` is now in semitones as the UI says: the device default bass (`range = 12`) has a contour of up to 12 semitones (it was 84 before the F4 fix).

## 5.4 Arpeggio style (note order) **[V]**

`t1_harmony_eval_style(style, counter, pool)` returns an index 0..pool−1:

| Style | Order for pool 4 (counter 0 to 11) |
| --- | --- |
| 0 UP | 0 1 2 3 0 1 2 3 0 1 2 3 |
| 1 DOWN | 3 2 1 0 3 2 1 0 3 2 1 0 |
| 2 UP-DN | 0 1 2 3 2 1 0 1 2 3 2 1 |
| 3 DN-UP | 3 2 1 0 1 2 3 2 1 0 1 2 |
| 4 CNVRG | 0 3 1 2 0 3 1 2 0 3 1 2 |
| 5 DIVRG | 2 1 3 0 2 1 3 0 2 1 3 0 |
| 6 RAND | 3 0 1 2 3 0 1 2 3 0 1 2 (a fixed permutation cycle, not random) |
| 7 ORDER | same as UP (there is no "order of entry" memory) |

The index is **added to the pitch in semitones** with a fixed `pool = 7`, before the scale snap. It is not an index into a held-note list or chord. With style 0 and a chromatic scale a pattern rises
`60 61 62 63 64 65 66 60 61 …` (measured); with a scale, the snap folds those into scale tones.

## 5.5 Chords and voicing **[V]**

`chord_size` 1 to 4. Voice 0 is the quantised base note; each next voice is the **second in-scale tone above the previous** (a "third" in the scale's own steps; for a pentatonic that is a larger interval). Then a **voicing** (0 to 11) displaces voices by octaves:

| Voicing | Effect on voice *i* of *n* |
| --- | --- |
| 0 ROOT | none |
| 1 INV 1 | voice 0 up an octave (n > 1) |
| 2 INV 2 | voices 0 and 1 up an octave (n > 2) |
| 3 INV 3 | voices 0 to 2 up an octave (n > 3) |
| 4 DROP2 | voice *n−2* down an octave |
| 5 DROP3 | voice *n−3* down an octave (n ≥ 3) |
| 6 SPRD | odd voices up an octave |
| 7 / 8 | all voices up / down an octave |
| 9 SPRD W | voice 0 down, last voice up an octave |
| 10 / 11 | all voices up / down two octaves |

All results are clamped to 0 to 127. A **pitch p-lock** replaces voice 0 only, after voicing and without quantising.

## 5.6 Master harmony link **[V]**

If `master_scale > 0`, every track uses `master_root` / `master_scale` instead of its own. **No control on the FM-1 sets `master_scale`** (the HARMONY shift knob 1 edits only `master_root`), so on the device the link is never active (F11).
Despite the plan's wording ("relative to Track 1's tonal center") the link is a global override, not a projection.

## 5.7 Invariants (tests)

- Chromatic: output = input (clamped 0 to 127).
- For a correct mask, every quantised pitch class (relative to root) is in the scale; the quantiser never moves a pitch up.
- Chord voices are strictly ascending before voicing.
- Contour offsets are within ±`range × 7`; TRI starts and ends at 0.
- **Scale test (missing today):** for every scale id, the set of mask bits must equal the intended interval list.
