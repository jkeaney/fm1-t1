# 4. Pitch: dial value → scale degree → MIDI note

**Input:** a stored note value `x` (continuous, `0 ≤ x < 84`), the selected scale table, the root.
**Output:** a MIDI note number.

## 4.1 The conversion **[V]**

Each stored note value encodes *both* an octave band and a position within the octave. The position is
turned into an index into the scale table, so a fixed dial range always spans the same number of octaves
no matter how many notes the scale has.

```
function quantise(x, table, root):
    band   = floor(x / 12)                       # 0..6   (seven 12-wide bands over 0..83)
    pos    = (x - 12*band) / 11 * 100            # 0..100 across the band (note: divides by 11)
    N      = length(table)                       # number of scale degrees
    idx    = floor(pos * N / 100)                # 0..N, truncated
    degree = table[min(idx, N-1)]                # ⚠ out-of-range index assumed to clamp
    octave = 12 * (band - 3)                     # band 0..6 → -36, -24, -12, 0, +12, +24, +36
    return 60 + degree + octave + root
```

**Verified:** this formula reproduces all 16 default note values stored in the device exactly
(`24 26 28 29 31 33 35 36 38 40 41 43 45 47 48 50` for C major, root 0). **[V]**

Order of additions in the device: scale degree + 60, plus the octave offset, then the root last.

## 4.2 Behaviours to copy or deliberately change

1. **Band width 12, but position scaled by 11.** `pos` reaches 100 at `x − 12·band = 11`. In the top 1/12 of each
   band (11 ≤ offset < 12) `pos` exceeds 100 and `idx` can reach `N`. The device indexes a Max `table`,
   which returns the last entry for an out-of-range index (⚠ assumed), so that sliver plays the scale's top degree.
2. **Integer inputs are uneven for N = 12.** With `Chromatic`, `idx = floor(12·k/11)`, so integer `x = 11` maps past the table.
   Dial values are continuous, so in practice this is only a small bias, but a rebuild that stores integer
   semitones should simply use `idx = offset` for chromatic.
3. **Quantise is late.** The stored `x` is never rewritten by the scale, so changing scale or root re-pitches the
   same stored sequence. (The dial widget also receives the quantised value back for display. ⚠ it shows the snapped
   value but the stored value is what Random writes.)
4. **Number of degrees varies**, so the same `x` lands on different degrees in different scales (e.g. a 5-note
   pentatonic vs a 12-note chromatic). That is by design.
5. **Range:** `x ≥ 84` selects no band and produces no note.

## 4.3 Root (`Fondamental`)

Integer 0 to 11 semitones added after the 60 base and octave offset. It can be set from the UI or by MIDI-in (note 72 to 83).

## 4.4 The 37 scale tables **[V]**

Menu order (index = `Scales` value). Values are semitones from the root.

| # | Name | Degrees | N |
| --- | --- | --- | --- |
| 0 | Chromatic | 0 1 2 3 4 5 6 7 8 9 10 11 | 12 |
| 1 | Major | 0 2 4 5 7 9 11 | 7 |
| 2 | Minor | 0 2 3 5 7 9 11 ⚠ see note below | 7 |
| 3 | Dorian | 0 2 3 5 7 9 10 | 7 |
| 4 | Mixolydian | 0 2 4 5 7 9 10 | 7 |
| 5 | Lydian | 0 2 4 6 7 9 11 | 7 |
| 6 | Phrygian | 0 1 3 5 7 8 10 | 7 |
| 7 | Locrian | 0 1 3 5 6 8 10 | 7 |
| 8 | WholeTone | 0 2 4 6 8 10 | 6 |
| 9 | HalfWholeDim | 0 1 3 4 6 7 9 10 | 8 |
| 10 | WholeHalfDim | 0 2 3 5 6 8 9 11 | 8 |
| 11 | MinorBlues | 0 3 5 6 7 10 | 6 |
| 12 | MinorPentatonic | 0 3 5 7 10 | 5 |
| 13 | MajorPentatonic | 0 2 4 7 9 | 5 |
| 14 | HarmonicMinor | 0 2 3 5 7 8 11 | 7 |
| 15 | HarmonicMajor | 0 2 4 5 7 8 11 | 7 |
| 16 | Dorian#4 | 0 2 3 6 7 9 10 | 7 |
| 17 | PhrygianDominant | 0 1 4 5 7 8 10 | 7 |
| 18 | MelodicMinor | 0 2 3 5 7 9 11 | 7 |
| 19 | LydianAugmented | 0 2 4 6 8 9 11 | 7 |
| 20 | LydianDominant | 0 2 4 6 7 9 10 | 7 |
| 21 | SuperLocrian | 0 1 3 4 6 8 10 | 7 |
| 22 | 8ToneSpanish | 0 1 3 4 5 6 8 10 | 8 |
| 23 | Bhairav | 0 1 4 5 7 8 11 | 7 |
| 24 | HungarianMinor | 0 2 3 6 7 8 11 | 7 |
| 25 | Hirajoshi | 0 2 3 7 8 | 5 |
| 26 | InSen | 0 1 5 7 10 | 5 |
| 27 | Iwato | 0 1 5 6 10 | 5 |
| 28 | Kumoi | 0 2 3 7 9 | 5 |
| 29 | PelogSelisir | 0 1 3 7 8 | 5 |
| 30 | PelogTembung | 0 1 5 7 8 | 5 |
| 31 | Messiaen3 | 0 2 3 4 6 7 8 10 11 | 9 |
| 32 | Messiaen4 | 0 1 2 5 6 7 8 11 | 8 |
| 33 | Messiaen5 | 0 1 5 6 7 11 | 6 |
| 34 | Messiaen6 | 0 2 4 5 6 8 10 11 | 8 |
| 35 | Messiaen7 | 0 1 2 3 5 6 7 8 9 11 | 10 |
| 36 | User | up to 12 pitch classes entered over MIDI | variable |

> **Note on "Minor" (#2):** the stored initial values are `0 2 3 5 7 9 11`, which is *melodic minor (ascending)*, not natural
> minor (`0 2 3 5 7 8 10`). The saved table data for that entry is also inconsistent: it has 12 slots with repeated values
> (`... 7 7 9 9 11`), so depending on how the table loads, its length may be 12, not 7. This looks like a quirk in the source
> device. If you rebuild, define natural minor as `0 2 3 5 7 8 10` and decide explicitly what you want. ⚠
> Some other names (e.g. "Messiaen3") follow the author's labelling; the degrees are exactly as listed above.

## 4.5 User scale **[V structure]**

Selecting `User` switches the table name to a scratch table. MIDI notes 60 to 71 played into the device are stored as pitch classes
(`note − 60`) in the order played; the table length follows the number of notes held. ⚠ Whether duplicates are removed
or sorted is not confirmed (a `keepone`/`borax` helper suggests only distinct held notes are kept).

## 4.6 Rebuild notes

- Store `const int8_t SCALE[37][12]` plus `uint8_t SCALE_N[37]`; the largest named scale has 12 degrees.
- Keep pitch as a stored 8-bit or 16-bit control (0..83 range) and quantise at trigger time. On FM-1 this costs one table lookup
  and a few integer operations per note.
- Use an explicit clamp for the out-of-range index instead of relying on container behaviour.
