# 2. Inputs

All 59 device parameters, read from the preset files **[V]**, and the pattern data (the blob).

## Per-lane parameters (×4, numbered 1 to 4)

| Parameter | Type / range | Default | Used by | Doc |
| --- | --- | --- | --- | --- |
| **Pad Note n** | int 36 to 51 | 39, 42, 38, 36 (lanes 1 to 4) | main drum note | [05](05-note-output.md) |
| **Pad Alt Note n** | int 36 to 51 | 37, 46, 40, 41 | alternate drum note | [05](05-note-output.md) |
| **Mute n** | off / on | off | silences the lane | [05](05-note-output.md) |
| **Delay n** | float 0 to 50 (ms) | 0 | delays each hit | [05](05-note-output.md) |
| **Global Length n** | int 2 to 16 | 16 | length of all layers when locked | [04](04-lane-layers.md) |
| **Lock Length n** | off / on | off | link all layer lengths to Global Length | [04](04-lane-layers.md) |
| **Length Chance n** | int 2 to 16 | 16 | Chance layer length | [04](04-lane-layers.md) |
| **Length Velocity n** | int 2 to 16 | 16 | Velocity layer length | [04](04-lane-layers.md) |
| **Length Roll n** | int 2 to 16 | 16 | Roll layer length | [04](04-lane-layers.md) |
| **Length Mod n** | int 2 to 16 | 16 | Slide ("Mod") layer length | [04](04-lane-layers.md) |
| **Length Alternate n** | int 2 to 16 | 16 | Alt layer length | [04](04-lane-layers.md) |

Default pad notes in a standard drum mapping: lane 1 = 39 (clap), lane 2 = 42 (closed hat), lane 3 = 38 (snare), lane 4 = 36 (kick). ⚠ labels only.

## Global parameters

| Parameter | Type / range | Default | Used by | Doc |
| --- | --- | --- | --- | --- |
| **Clock Speed** | enum `50 %`, `66 %`, `75 %`, `100 %`, `150 %`, `200 %` | 100 % | step rate | [03](03-clock-and-shuffle.md) |
| **Global Shuffle** | float 0 to 100 (%) | 100 | scales all shuffle sliders | [03](03-clock-and-shuffle.md) |
| **Shuffle 1** | float 0 to 50 | 0 | step 1 delay | [03](03-clock-and-shuffle.md) |
| **Shuffle 2 … 8** | float −50 to +50 | 0 | steps 2 to 8 offset | [03](03-clock-and-shuffle.md) |
| **Shuffle Length** | int 2 to 8 | 4 | how many shuffle steps are used | [03](03-clock-and-shuffle.md) |
| **Roll Rate** | enum, 9 values: `1/8.` `1/8` `1/8t` `1/16.` `1/16` `1/16t` `1/32.` `1/32` `1/32t` | `1/32` (index 7) | spacing of roll repeats | [05](05-note-output.md) |
| **Auto Retrigger** | off / on | off | restart layers periodically | [03](03-clock-and-shuffle.md) |
| **Auto Trigger Rate** | enum `1 Bar` … `4 Bars` | 1 Bar | retrigger interval | [03](03-clock-and-shuffle.md) |
| **Timing Page** | enum `1`, `2` | 1 | UI page only | not algorithmic |

## Pattern data (the preset blob) **[V]**

20 arrays of 16 integers, each 0 to 6:

```
Chance-Steps[1..4]   Velo-Steps[1..4]   Roll-Steps[1..4]   Slide-Steps[1..4]   Alt-Steps[1..4]
```
stored as hex-encoded JSON text inside the preset (a blob parameter). So a full pattern is `4 lanes × 5 layers × 16 steps = 320` small integers.
Value meanings are in [04](04-lane-layers.md) and [05](05-note-output.md).

## Edit actions (per layer)

| Action | Effect | Doc |
| --- | --- | --- |
| **Random** | fills the layer with uniform values 0 to 6 | [04](04-lane-layers.md) |
| **Not-so-random** | nudges each value by −1, 0 or +1, clipped to 0..6 | [04](04-lane-layers.md) |
| **Reset** | Velocity layer → 4, every other layer → 0 | [04](04-lane-layers.md) |
| Step edit | click or drag a step (also Push pads) | not algorithmic |

## Input handling rules

- Changing a **length** never changes stored values; it only changes how many leading cells are read.
- **Random / Not-so-random / Reset** rewrite stored values only.
- **Shuffle** and **Clock Speed** change timing only.
- Mute silences the lane's notes but the layer counters keep advancing (⚠ inferred, matches "mute" semantics).
