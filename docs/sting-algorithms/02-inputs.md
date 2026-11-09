# 2. Inputs

All user-facing controls found on the device, what they feed, and the algorithm file that
describes the processing. Ranges and defaults are read from the device's saved parameter data **[V]**
unless tagged.

## Generation controls

| Control | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Algo** | `v2`, `v1` (v1 = "Classic") | not stored | pitch generator selection | [03](03-pitch-generation.md) |
| **Type** | 16 steps, labelled `RANDOM` → `xACIDx` | 11 | number of notes replaced by repeats | [03](03-pitch-generation.md) |
| **Density** | 0 to 16 (⚠ exact range) | 14 | octave, velocity, gate generators | [04](04-rhythm-and-dynamics.md) |
| **Gen** | button | none | triggers full regenerate | [01](01-architecture.md) |
| **Gen Clip** | button | none | writes the pattern to a clip | [06](06-outputs.md) |
| **Poly Reset** | `∞`, 1 to 16 | ∞ (0) | clock: resets step counter every N steps (⚠ inferred from the clock subpatch) | [05](05-playback-timing.md) |

The 16 Type labels, in order, show the intended morph (letters "freeze" from RANDOM to ACID):
`RANDOM, xANDOM, RxNDOM, RAxDOM, RANxOM, RANDxM, RANDOx, AANDxO, ACNxOM, ACxEOM, AxEEOM, xACEOM, AxCEEM, AAxIEM, AACxxx, xACIDx`.

## Pitch and scale controls

| Control | Values | Default | Feeds | Doc |
| --- | --- | --- | --- | --- |
| **Root** | C to B (0 to 11) | C | scale reference shift, transposing the scale | [03](03-pitch-generation.md) |
| **Scale** | Off / On | Off | enables nearest-degree snap | [03](03-pitch-generation.md) |
| **Transpose** | integer semitones | 0 | added to every output pitch | [06](06-outputs.md) |
| Live scale intervals | read from the host's scale | C major `[0,2,4,5,7,9,11]` if none | the reference list for the snap | [03](03-pitch-generation.md) |

## Time controls

| Control | Values | Default | Doc |
| --- | --- | --- | --- |
| **Division** (Rate) | 15 values: `1/48 1/32 1/24 1/16 1/12 1/8 1/6 3/16 1/4 5/16 1/3 3/8 1/2 3/4 1` | `1/16` (index 3) | [05](05-playback-timing.md) |
| **Steps** | 1 to 16 | 16 | [05](05-playback-timing.md) |
| **Offset** | integer rotate | 0 | [05](05-playback-timing.md) |
| **Direction** | `Fwd`, `Bwd`, `Pendulum` | `Fwd` | [05](05-playback-timing.md) |
| **Swing** | 0 to 1 continuous | 0 | [05](05-playback-timing.md) |
| **Clock** | `Cont` (continuous), `Gate` (runs only while a MIDI note is held) | `Cont` | [05](05-playback-timing.md) |
| Jitter (internal amount) | 0 to 1 | not exposed as a named control in the top level (⚠) | [05](05-playback-timing.md) |

## Dynamics controls

| Control | Values | Default | Doc |
| --- | --- | --- | --- |
| **Gate** | 0 to 100 (%, ⚠ exact role) | 100 | [04](04-rhythm-and-dynamics.md) |
| **Accent** | bipolar −1 to +1 shown as 0 to 100 | 50 (= neutral, ⚠) | [04](04-rhythm-and-dynamics.md) |

## Performance and automation inputs

| Input | Effect | Doc |
| --- | --- | --- |
| Incoming MIDI notes | pitch class of the played note feeds the root/transposition path; with Clock = Gate the held-note state gates the clock (⚠) | [05](05-playback-timing.md) |
| Host transport | provides the tick/beat position that drives the step counter | [05](05-playback-timing.md) |
| **CakeWalk** | random-walk auto-modulation of the Density and Type dials; active only when the modulation sources (velocity/aftertouch/slider assignments) sum to non-zero | [03](03-pitch-generation.md#cakewalk) |
| Per-step edits | writing a value to a single step in any lane (note, octave, gate, velocity) | [06](06-outputs.md) |

## Input handling rules **[V]**

- Parameter changes that affect the *shape* of a pattern (Algo, Type, Density, Gen) regenerate lanes.
- Parameter changes that affect only *playback* (Direction, Swing, Offset, Steps, Division) never touch the stored lanes.
- Toggling **Scale** never destroys the original pitches; it switches between the stored `pitch_orig` and a snapped copy.
