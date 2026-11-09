# 5. Making a note

**Input:** the five lane levels for this tick, Root Note, Scale, tempo.
**Output:** a MIDI note-on and note-off (or nothing, if Chance fails).

## 5.1 Pitch **[V]**

```
octave_semis = scale(Octave_level, 0..6 → -3..+3) * 12          # -36 .. +36
degree_semis = scale_table[Scale][ Transpose_level ]            # level 0..6 indexes the scale's degrees
pitch        = root + degree_semis + octave_semis
```
Octave level 3 means no shift. The device shows the final pitch in the **Note Out** display.

### Scale tables **[V]** (8 values each; the 8th is the octave and the levels 0..6 only reach the first 7)

| Scale | Degrees (semitones) |
| --- | --- |
| Chromatic | 0 1 2 3 4 5 6 (and 7) |
| Major | 0 2 4 5 7 9 11 (12) |
| Dorian | 0 2 3 5 7 9 10 (12) |
| Phrygian | 0 1 3 5 7 8 10 (12) |
| Lydian | 0 2 4 6 7 9 11 (12) |
| Mixolydian | 0 2 4 5 7 9 10 (12) |
| Minor (natural) | 0 2 3 5 7 8 10 (12) |
| Locrian | 0 1 3 5 6 8 10 (12) |

Notes:
- **Chromatic is a list of 0 to 7 semitones**, not 0 to 11. With 7 usable levels the chromatic scale spans only 0 to 6 semitones above the root (a tritone). The *Neurothmic Drums* preset uses this, mapping seven levels to seven adjacent drum pads. **[V]**
- Levels 0..6 therefore always address the first seven degrees. Reaching the octave degree (12) needs the Octave lane.
- The lookup is keyed by the level. ⚠ The table is loaded into a `coll` through a `funnel 15`; the key base (0 or 1) was not confirmed. Check that Transpose level 0 returns the root, and level 6 returns the 7th degree.

## 5.2 Velocity **[V]**

```
velocity = scale(Velocity_level, 0..6 → 15..120)     # linear
```
Levels: 15, 32, 50, 67, 85, 102, 120.

## 5.3 Chance **[V]**

```
chance_pct = scale(Chance_level, 0..6 → 0..100)       # 0, 16.7, 33.3, 50, 66.7, 83.3, 100
r = random(100) + 1                                    # 1..100
plays = r <= chance_pct
```
So level 0 never plays, level 6 always plays, level 3 plays 50% of the time. A failed roll produces no note at all (no note-on, no note-off).

## 5.4 Note length **[V]**

The Length lane's level selects a multiplier from the `Length Multiplicator` menu:

| Level | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Multiplier (64th notes) | 1 | 2 | 4 | 8 | 12 | 16 | 32 |

```
unit_ms     = (0.98 × sixteenth_ms) / 4        # one 64th note, with a 2% gap
length_ms   = unit_ms × multiplier
```
Equivalent note values: 1/64, 1/32, 1/16, 1/8, 3/16, 1/4, 1/2. The 2% reduction leaves a tiny gap so back-to-back notes do not overlap.
The note-off is produced by a delay (`pipe`) of `length_ms` in the MIDI message block. Note length is in **milliseconds from the live tempo**, so it follows tempo changes. A note longer than the step interval overlaps following steps.

## 5.5 MIDI messages (`MIDI_msgs`) **[V]**

```
on tick (after buddy):
    if chance passes:
        send note-on  (pitch, velocity)                    # via midiformat
        after length_ms: send note-on (pitch, 0)           # a zero-velocity note-on as the note-off
```
- Note-offs are zero-velocity note-ons (the `pack pitch 0` path), channel 1.
- The pitch is stored when the note is made, so a root or scale change while a note is held does not strand the note-off.

## 5.6 Incoming MIDI (root, start) **[V structure]**

With MIDI mode on, `ddg.mono` tracks the most recently held key. That note becomes the **Root Note** (and the Note Out display); note-on starts the clock, note-off stops it. Without MIDI mode the root is the Root Note control.

## 5.7 Order of processing for one note

1. Five lane values collected.
2. Pitch = root + degree + octave.
3. Chance roll; stop on failure.
4. Velocity from level; length from multiplier × unit.
5. Note-on now; note-off after `length_ms`.

## 5.8 Invariants (tests)

- Pitch for Octave level 3, Transpose 0 equals Root Note.
- Pitch for Octave level 0 is root − 36 (clamped to the MIDI range by the receiver).
- Chance 6 always plays; Chance 0 never plays; mean play rate for level 3 ≈ 50%.
- Length multipliers are `1,2,4,8,12,16,32` 64ths.
- Every note-on has exactly one note-off after the computed length.
- Output pitch class (pitch − root) mod 12 always belongs to the chosen scale (for non-chromatic scales).
