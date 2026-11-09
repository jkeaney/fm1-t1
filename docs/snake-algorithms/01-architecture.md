# 1. Architecture and data flow

## Stage diagram

```
 per lane (×4 + 2 extra Custom):                                                       shared
┌────────────────────────────────────────────────────────────────────────────────┐   ┌────────────────────────────┐
│ host transport ─► live.step clock ─► step idx ─► + shift ─► +1 ─► SNAKE CORE   │   │ Scale + Root ─► quantise   │
│   (speed, direction, length)                                  (shape lookup)   │   │ Swing + note length        │
│                                                                    │           │   │ MIDI in: root, user scale, │
│                         cell k whose shape[k] == step number ◄─────┘           │   │          random-all        │
│                                   │                                            │   └────────────────────────────┘
│                    bang cell k's stored value                                  │
└───────────────────────────────────┼────────────────────────────────────────────┘
                                    ▼
   Notes lane ──► quantise ──► latched pitch ───────┐
   Velocities lane ──► latched velocity ────────────┼──► released by ──► swing delay ──► note-on ──► note-off
   Gates lane ──► (cell toggle ON) ► TRIGGER pulse ─┘    the trigger      (+ delay)     (flush)     (after length)
   Custom lanes ──► scale 0..1 ──► 10 ms glide ──► mapped Live parameter (modulation, no notes)
```

## The four lanes **[V]**

| Lane | Cell value | Range | Role |
| --- | --- | --- | --- |
| **Notes** | continuous dial `Note_01..16` | 0 to <84 | pitch (before quantise) |
| **Gates** | toggle `Gate_01..16` | off / on | **the trigger**: only an ON cell fires a note |
| **Velocities** | dial `Velocity_01..16` | 0 to 127 (default 64) | note velocity |
| **Custom** ×3 | dial `Custom_01..16` | continuous (mapped) | modulation of any Live parameter |

Each lane is the same sub-machine (`p Snake Core` + `p Random` + `p start` + `p midi seq` + 16 cell widgets),
copied per lane. So the algorithm only needs to be built once and instanced.

## Per-lane sub-machine

| Block (patch name) | Job | Doc |
| --- | --- | --- |
| `p midi seq` (a Max `live.step` clock) | produces the step index from host transport using Speed, Direction, Length | [06](06-playback-timing.md) |
| `p start` | works out the maximum Steps Shift for the chosen Length (`16 − length`) | [06](06-playback-timing.md) |
| `+ 0`, `+ 1` | add the shift, convert to a 1-based step number | [06](06-playback-timing.md) |
| `p Snake Core` | maps step number → which cell plays, using the selected shape table | [03](03-snake-shapes.md) |
| cell widgets | hold the 16 stored values; banged by Snake Core to emit their value | [05](05-lane-generation.md) |
| `p Random` | fills all 16 cells with random values in a chosen range | [05](05-lane-generation.md) |
| `p quantize` (Notes lane only, ×16) | converts a stored dial value to a MIDI note through the scale | [04](04-pitch-quantise.md) |

## Event flow for one note **[V]**

1. The **Gates** lane clock ticks. Snake Core finds the cell for this step. If that cell's toggle is ON, a pulse leaves the lane.
2. The pulse goes to the **swing** stage, which delays off-beat pulses.
3. The delayed pulse releases the **latched pitch** and **latched velocity** into `noteout` through a `flush` object.
4. A second timer (note length) bangs `flush`, which sends the note-off.

The latches are updated *independently* by the Notes and Velocities lanes on their own clocks. A note is
therefore built from "whatever pitch and velocity were last selected" at the instant the gate fires.

## Ordering and dependencies

- The lanes share nothing except the transport, the scale/root, and the swing/length settings.
- Generation (Random) and playback are decoupled: Random only rewrites the 16 stored values.
- Changing Shape, Length or Shift never changes stored values.
