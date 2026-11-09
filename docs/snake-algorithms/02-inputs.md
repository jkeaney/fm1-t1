# 2. Inputs

Ranges and defaults are from the device's saved parameter data **[V]** unless tagged. "×lane" means the control exists once per lane
(Notes, Gates, Velocities, Custom ×3).

## Per-lane sequencing controls

| Control (saved name `<Lane>_Sequencer_…`) | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Length** | 1 to 16 | 16 | clock loop length; limits Steps Shift | [06](06-playback-timing.md) |
| **Speed** | `32n`, `16n`, `8n`, `4n`, `2n` | `16n` | clock interval | [06](06-playback-timing.md) |
| **Direction** | forwards, backwards, fore/back, back/fore, random | forwards | clock direction | [06](06-playback-timing.md) |
| **Snake Shape** | 1 to 14 | 1 (⚠ not stored; first item) | Snake Core lookup | [03](03-snake-shapes.md) |
| **Steps Shift** | 0 to 100 (dial), mapped to `0..(16 − Length)` | 0 | start offset | [06](06-playback-timing.md) |
| **Random** | button | none | fills the 16 cells randomly | [05](05-lane-generation.md) |
| **Reset** | button | none | restores the lane's cells (⚠ to defaults) | [05](05-lane-generation.md) |
| **Random Range** (`…_Random_Range`) | two-handle slider | Notes: 0 to 83 | bounds for Random | [05](05-lane-generation.md) |

## Per-step values (16 per lane)

| Widget | Values | Default | Doc |
| --- | --- | --- | --- |
| `Note_01..16` | continuous 0 to <84 | ascending: `0, 1.96, 3.27, 5.23, 6.54, 8.0, 11.78, 12.65, 14.57, 15.45, 17.37, 19.42, 21.04, 22.26, 25.11, 25.60` | [04](04-pitch-quantise.md) |
| `Gate_01..16` | off / on | all on | [05](05-lane-generation.md) |
| `Velocity_01..16` | 0 to 127 | 64 | [05](05-lane-generation.md) |
| `Custom_01..16` (×3 lanes) | continuous | 0 | [05](05-lane-generation.md) |

The default Note dial values were chosen so that, under C major, they play exactly
`24 26 28 29 31 33 35 36 38 40 41 43 45 47 48 50` (C1 up the major scale to D3). **[V]** (all 16 verified)

## Global controls

| Control | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Scales** | 37 entries, `Chromatic` to `User` | `Major` (index 1) | quantiser table | [04](04-pitch-quantise.md) |
| **Fondamental** (root) | 0 to 11 (C to B) | 0 | added after quantise | [04](04-pitch-quantise.md) |
| **Swing** | 0 to 1 | 0 | swing stage | [06](06-playback-timing.md) |
| **Note length** (saved as `Direction_convert`) | `64n`, `32n`, `16n`, `8n`, `4n` | `32n` (index 1) | note-off delay (⚠ inferred from wiring) | [06](06-playback-timing.md) |
| **Reset All** / **Random All** | buttons | none | broadcast to every lane | [05](05-lane-generation.md) |
| **MidiInMode** | two states | first | enables MIDI-in control | [07](07-outputs-and-rebuild.md) |
| Lane selector tabs (`live.tab` 1 to 3), lane-section selectors | UI only | none | show/hide panels | not algorithmic |

## Performance inputs (MIDI in) **[V structure]**

| Incoming note | Effect |
| --- | --- |
| 72 to 83 (C5 to B5) | sets the root: `root = note − 72` |
| 60 to 71 (C4 to B4, shown as "C3 to B3" in the UI) | enters pitch classes into the **User** scale table |
| 48 | triggers **Random All** |

MIDI channel handling and exact note-name labels are Live conventions; confirm octave naming on the device. ⚠

## Input handling rules

- **Shape, Length, Shift, Direction, Speed** change *playback only*; stored cell values are untouched.
- **Scale and Root** change *pitch only*; stored dial values are untouched.
- **Random** and **Reset** change *stored values only*.
- **Parameter automation:** every control above is a Live parameter, so all of it can be automated or modulated.
  Only the three Custom lanes drive Live parameters out (see [05](05-lane-generation.md)).
