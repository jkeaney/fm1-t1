# 6. Playback and timing

Each lane has its own clock. Nothing is shared between lanes except the transport, so lanes with different
Length, Speed or Direction drift against each other (polymeter and polyrhythm).

## 6.1 Transport sync **[V structure]**

A small `Time` sub-patch reads the host transport (a polled `transport` object, a metro firing every 10 ticks while running)
and builds a message `time <bar> <beat> <units> <resolution> <timesig>` for the lane's `live.step` clock.

Because the step is computed from the host position (not by counting pulses), every lane is **phase-locked to the song position**.
Jumping the transport re-aligns all lanes, so they all restart together at bar 1.

## 6.2 Step clock (`live.step`) **[V messages, ⚠ exact semantics]**

Messages sent to each lane's `StepSeq`:

| Message | Source control | Meaning |
| --- | --- | --- |
| `nstep N` | Length | number of steps |
| `interval I` | Speed | step duration: `32n`, `16n`, `8n`, `4n`, `2n` |
| `direction D` | Direction | traversal mode, below |
| `loop 1 L` | Length | loop window over the first `L` steps |
| `time …` | transport | position sync |

**Direction menu → mode** (order from the menu):

| Index | Label | Behaviour (for L = 4) |
| --- | --- | --- |
| 0 | forwards | 1 2 3 4 1 2 3 4 … |
| 1 | backwards | 4 3 2 1 4 3 2 1 … |
| 2 | fore/back | forwards then backwards (ping-pong; ⚠ whether ends repeat) |
| 3 | back/fore | backwards then forwards |
| 4 | random | random step each tick (⚠ with or without repeats) |

Output of the clock is a zero-based position (a `− 1` is applied after the clock's own 1-based index).

## 6.3 Steps Shift and the final step number **[V]**

```
max_shift = 16 - L                                  # `p start` maps Length n → 16 - n
shift     = scale(shift_dial, 0..100 → 0..max_shift)
step      = clock_position + shift + 1              # 1-based; always within 1..16
cell      = order[step]                             # see 03-snake-shapes.md
```
- Shift slides the play window along the shape's path. With L = 16 the shift is always 0.
- Because shift is limited to `16 − L`, `step` never exceeds 16.

## 6.4 Speed table

`interval` is a note value relative to the beat in the host's tempo:

| Speed | Steps per quarter note |
| --- | --- |
| 32n | 8 |
| 16n | 4 (default) |
| 8n | 2 |
| 4n | 1 |
| 2n | 0.5 |

Only these five are offered; there are no triplet divisions. The swing stage also looks up a "ticks per step" value for the
Gates lane's speed (a stored table), so swing always follows the *Gates* lane speed. ⚠

## 6.5 Swing **[V structure, ⚠ units]**

Swing applies to the trigger pulses (Gates lane), not to pitch or velocity lanes.

```
swing_phase = zmap(swing, 0..1 → 0.5 .. 0.744)      # smoothed with a one-pole filter
cycle       = 2 * ticks_per_step                    # one swing cycle = two steps
```

- A host-locked phasor runs one cycle per two steps. The first pulse of each pair is on the cycle start; the second is released
  when the phasor passes `swing_phase`.
- `swing = 0` → `0.5`: the second step sits exactly halfway (straight). `swing = 1` → `0.744`: the second step is pushed 74.4%
  of the way through the pair. `0.667` would be a triplet shuffle.
- ⚠ The exact comparators in the signal chain (four thresholds at 0, 0.25, 0.5, 0.75) are not fully traced, so treat the
  mapping `0.5 → 0.744` as the verified range and the reading "position of the second hit within a two-step cycle" as inferred.
- The result is delivered as a signal-rate edge, so swung timing has audio-rate resolution rather than tick resolution.

## 6.6 Note length **[I]**

After the trigger, a delay of `Note length` (64n to 4n, saved under the misleading name `Direction_convert`) bangs a `flush`
object, which sends note-offs for any held notes. Length is a fixed note value and is not scaled by the gate lane or by tempo
changes beyond the host's musical time.

## 6.7 Per-tick order of operations (one lane)

1. Host position → clock position (apply Speed, Direction, Length).
2. Add Steps Shift, add 1.
3. Snake Core lookup: `cell = order[step]`.
4. Lane-specific action: Gate (conditional trigger) / Notes (latch pitch) / Velocities (latch velocity) / Custom (ramp parameter).
5. On a Gates trigger only: swing delay → note-on with the latched pitch and velocity → delayed note-off.
