# 2. Inputs

Ranges and defaults are read from the device's saved data **[V]** unless tagged.

## Per-stage controls (×8)

| Control | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Stage n Mode** | `0`, `I`, `II`, `I-` | `I` (index 1) | trigger rule | [03](03-stage-engine.md) |
| **Stage n Length** | 1 to 8 (pulses) | 1 | cumulative cascade | [03](03-stage-engine.md) |
| **Stage n Div** | 1 to 4 | 1 | counter wrap (Mode II only) | [03](03-stage-engine.md) |
| **Pitch dial n** | MIDI-note display, centred on 60 | 0 offset | note pitch | [04](04-pitch-velocity-aux.md) |
| **Velocity dial n** | 0 to 127 | 120 | note velocity | [04](04-pitch-velocity-aux.md) |
| **Aux dial n** | continuous | 0 | CC value or pitch bend | [04](04-pitch-velocity-aux.md) |
| **Stage n LED** | UI | none | display only | not algorithmic |

The three dial layers are shown one at a time through the `Pitch / Vel / Aux Layer` tab; they are three separate sets of parameters.

## Global controls

| Control | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Global Step Division** | 1 to 4 | 1 | sets every stage's Div | [03](03-stage-engine.md) |
| **StepDiv Reset** (hidden) | toggle | off | resets per-stage divisions after using the global one | [03](03-stage-engine.md) |
| **Direction** | `>`, `<`, `><`, `R` | `>` | pulse clock direction | [05](05-timing-swing-direction.md) |
| **Length Mode** | two states | ⚠ not stored | variable (sum of stages) vs fixed | [05](05-timing-swing-direction.md) |
| **Sequence Length** | numeric dial, default 8 | 8 | fixed length (×4 pulses ⚠) | [05](05-timing-swing-direction.md) |
| **Clock Timing** | `1/32`, `1/16`, `1/8` | `1/16` | pulse interval | [05](05-timing-swing-direction.md) |
| **Swing Amount** | numeric, 0 to 200 mapped to 0 to 1 | 0 | swing offset | [05](05-timing-swing-direction.md) |
| **Coarse Tune** | integer semitones | 0 | added to pitch | [04](04-pitch-velocity-aux.md) |
| **Random** | button | none | randomises the current dial layers | [04](04-pitch-velocity-aux.md) |
| **Aux CC no.** | integer; `-1` = pitch bend | ⚠ not stored | aux destination | [04](04-pitch-velocity-aux.md) |
| **MIDI Thru** | on/off | ⚠ | passes incoming MIDI to the output | [06](06-outputs-and-rebuild.md) |

## Input handling rules

- **Mode, Length, Div** change playback only; they never change the stored pitch/velocity/aux.
- Changing a stage Length re-bangs the length totals, so the cumulative thresholds update immediately. **[V]**
- **Random** rewrites stored dial values only.
- The Length tab and Div tab each also output their label (1 to 8, 1 to 4), which is what the engine uses; the Mode tab outputs its index (0 to 3). **[V]**
