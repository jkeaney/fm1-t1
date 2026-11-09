# 2. Inputs

Ranges and defaults are read from the device and preset files **[V]** unless tagged.

## Lane controls (×5: Octave, Transpose, Length, Velocity, Chance)

| Control | Values | Notes | Doc |
| --- | --- | --- | --- |
| **Faders 1 to 16** | integer level 0 to 6 | one per step | [03](03-lanes.md) |
| **Lane length** (`OctLength`, `TransLength`, `LenLength`, `VeloLength`, `ChanceLength`) | integer 2 to 16 | how many leading faders are played | [03](03-lanes.md) |
| **Random** button | action | fills the lane with random levels | [03](03-lanes.md) |
| **Not-so-random** button | action | nudges each level by −1, 0 or +1 | [03](03-lanes.md) |
| **Reset** button | action | sets the whole lane to its neutral level | [03](03-lanes.md) |

Default lane lengths (device and preset files agree): Octave 2, Transpose 3, Length 8, Velocity 15, Chance 4. These are deliberately co-prime-ish values, so a freshly loaded device already drifts against itself.

## Global controls

| Control | Values | Default | Used by | Doc |
| --- | --- | --- | --- | --- |
| **Root Note** | MIDI note 0 to 127 | 36 | base pitch | [05](05-note-output.md) |
| **Scales** | `Chromatic`, `Major`, `Dorian`, `Phrygian`, `Lydian`, `Mixolydian`, `Minor`, `Locrian` | `Major` (index 1) | degree table | [05](05-note-output.md) |
| **Speed menus** | straight: `2n 4n 8n 16n 32n 64n` · triplet: `1nt 2nt 4nt 8nt 16nt 32nt` | `16n` (index 3) | step interval | [04](04-clock-and-timing.md) |
| **Triplets** | Off / On | Off | choose the triplet menu | [04](04-clock-and-timing.md) |
| **Double Time** / **Half Time** | buttons | none | speed index ±1 | [04](04-clock-and-timing.md) |
| **Swing** | 0 to 127 (shown as %) | 0 | delay of off-beat steps | [04](04-clock-and-timing.md) |
| **Re-Sync** | button | none | restart the lanes on the grid | [04](04-clock-and-timing.md) |
| **Length Multiplicator** | `1.`, `2.`, `4.`, `8.`, `12.`, `16.`, `32.` | the Length lane drives this menu | note length | [05](05-note-output.md) |
| **MIDI** (MIDI-in mode) | off / on | off | root from MIDI, clock gating | [05](05-note-output.md) |
| **Note Out** | display | 36 | shows the current output note | not algorithmic |
| **Playback** button | UI | none | enables the fader-edit mouse tracking | not algorithmic |

## Pattern data (preset blob) **[V]**

```
Octave[1..16]  Transpose[1..16]  Length[1..16]  Velocity[1..16]  Chance[1..16]   # each 0..6
Root Note   Scales   Swing   Triplets   MIDI
```
stored as hex-encoded JSON in the preset; the five lane lengths and Resync are ordinary preset parameters. A full pattern is 80 levels plus 5 lengths plus a few globals.

## Performance input

- **Incoming MIDI** (when MIDI mode is on): the held note becomes the root; note-on also starts the clock and note-off stops it (see [05](05-note-output.md)). ⚠ confirm note-off stops the clock.
- **Host transport:** play/stop starts and stops the clock and resets the lane counters.

## Input handling rules

- Lane length changes never alter stored faders.
- Random, Not-so-random and Reset rewrite stored faders only.
- Scale, Root and Speed changes affect playback only; Speed changes trigger a resync of the lanes.
