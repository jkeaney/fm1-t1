# 4. Pitch, velocity, duration and aux

Each of the 8 stages stores four values in the note store (`live.step[1]`): **pitch, velocity, extra1 (duration) and extra2 (aux)**.
The dials set pitch, velocity and extra2; the stage Mode sets extra1.

## 4.1 Pitch **[V]**

```
midi_pitch = dial_pitch_value + coarse_tune
```
- The dial shows MIDI note numbers; the value written into the store is the dial's note value. Random uses the range 24 to 95 (below).
- **Coarse Tune** is an integer number of semitones added to every note just before the note is created. It does not change stored values.
- There is **no scale quantiser**: pitch is whatever the dial gives, chromatically.

## 4.2 Velocity **[V]**

`velocity = dial_velocity_value`, 0 to 127, default 120. No accent logic, no velocity curve.

## 4.3 Duration (extra1) **[V formula, I mode link]**

```
duration_ticks = extra1 * 8 - 1          # ticks of a 480-per-quarter clock
```
- `extra1` runs 0 to 15 (⚠ upper bound inferred from the mode message "extra1 = 15"), so durations run from about 7 ticks to 119 ticks,
  just under a 1/16 note (120 ticks).
- **Mode `I-`** sets the stage's `extra1` to 15 (the longest), which is what makes it the "long note" mode. Other modes use the stored value (⚠ default not saved).
- The duration is passed to `makenote` as a tick value, which then also produces the note-off.

## 4.4 Aux (extra2) **[V]**

The aux value goes out on one of two paths:

```
if aux_cc == -1:   send pitch bend with the aux value
else:              send CC number aux_cc with the aux value
```
It is sent each time the stage plays, alongside the note. The Aux CC number is a separate setting; `-1` selects pitch bend.

## 4.5 Random **[V]**

Random rewrites **all 8 stages at once for the layer you are viewing**:

| Layer | Draw | Result |
| --- | --- | --- |
| Pitch | `random 72`, minus 36, plus 60 | uniform integer from 24 to 95, centred on middle C |
| Velocity | `random 127` | uniform integer 0 to 126 |
| Aux | `random 127` (⚠ maps to extra2) | uniform 0 to 126 |

Each stage is drawn independently (a loop of 8 draws). The draw is uniform; there is no scale, no step bias and no repeat avoidance.

## 4.6 Rebuild notes

- Stage record: `{ pitch:u8, vel:u8, dur:u8(0..15), aux:u8, mode:u8(0..3), len:u8(1..8), div:u8(1..4) }`, 7 bytes × 8 stages.
- Convert `dur` to a duration at the output stage; on FM-1 use its own gate/length model rather than Max ticks.
- Pitch, velocity and aux are independent lanes of 8, edited as separate "layers" in the UI; all three are written by the same Random draw loop.
