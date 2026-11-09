# 2. Inputs

Ranges and defaults are from the saved parameter data **[V]** unless tagged.

## Note-engine controls

| Control | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Note Order** | `Played`, `Up`, `Down`, `Chord`, `Random`, `Follow` | `Up` | NotesSort / arp type | [03](03-note-engine.md) |
| **Octave Mode** | `Up`, `Down`, `UpDown`, `Random` | `Down` (saved index 1; ⚠ confirm) | OctaveSort | [03](03-note-engine.md) |
| **Octave Steps** | integer | 1 | number of octaves | [03](03-note-engine.md) |
| **Hold** | `Not Held`, `Hold` | off | latch notes | [03](03-note-engine.md) |
| **Scale** | `Off`, `On` | off | scale snap using Live's current scale | [03](03-note-engine.md) |

## Timing controls

| Control | Values | Default | Doc |
| --- | --- | --- | --- |
| **Division** | `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`, `1/64` | `1/16` (index 4) | [05](05-timing.md) |
| **Pulse Sync Dot** / **Triplet** | Off / Dotted, Off / Triplet | off | [05](05-timing.md) |
| **Time Mode** | `Sync`, `Freq` | `Sync` | [05](05-timing.md) |
| **BPM** | 5 to 999 | 120 (manual tempo when not synced) | [05](05-timing.md) |
| **Swing** | 0 to 1 | 0 | [05](05-timing.md) |
| **StepDirection** | `Forwards`, `Backwards`, `Alternate`, `Random` | `Forwards` | [05](05-timing.md) |
| **LoopLength** | 1 to 16 | 16 | [05](05-timing.md) |
| **Key Retrig** | off / on | off | [05](05-timing.md) |

## Per-step lanes (16 values each)

| Lane | Value range | Random range defaults (min / max) | Enable | Doc |
| --- | --- | --- | --- | --- |
| **Steps** (on/off) | off / on | none | always | [04](04-step-lanes.md) |
| **Velocity** | 0 to 127 | 27 / 100 | always | [04](04-step-lanes.md) |
| **Length** | 0 to 127 (fraction of step) | 27 / 100 | always | [04](04-step-lanes.md) |
| **Chance** | 0 to 100 (%) | 20 / 80 | `ChanceOn` | [04](04-step-lanes.md) |
| **Ratchet** | 0 to 3 | 0 / 3 | `RatchetOn` | [04](04-step-lanes.md) |
| **Transpose** | −? to +? semitones (display range ±; ⚠ widget range) | −7 / +7 | `TransposeOn` | [04](04-step-lanes.md) |
| **Map / CC** (`Data`) | 0 to 127 | 20 / 80 | MIDICC ≠ Off | [04](04-step-lanes.md) |

The lane tabs are `Velocity, Length, Chance, Ratchet, Transpose, Map/CC`. Each lane has: a **Random** button (fills all 16 values uniformly in its min/max), a **min/max** range, and
(Transpose, Chance, Ratchet, Map) an on/off. Transpose also has `TrnspAll` (offset all) and `TranspReset`. Map has `Smooth` and `MIDICC` (Off or CC 1 to 128).

## Performance inputs

- **MIDI notes in:** the held keys are the source of pitches. Note-off removes a note unless Hold is on.
- **Host transport:** play state decides whether the clock is locked to the song position.
