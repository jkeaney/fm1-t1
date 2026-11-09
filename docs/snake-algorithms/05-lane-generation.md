# 5. Lane generation: Random, Reset, gates, velocity, Custom

Snake has little "generation" logic in the sense of Sting; it is a **performance sequencer**. The only
generators are per-lane **Random** and **Reset**, plus the global Random All / Reset All.

## 5.1 Random (per lane) **[V]**

Every cell is filled independently and uniformly.

```
function randomise(lane, lo, hi):          # lo, hi from the lane's Random Range slider
    for k in 1..16:
        r = rand_int(127)                  # 0..126
        lane[k] = lo + (hi - lo) * r / 127 # linear map; hi is approached but not exactly reached
```

| Lane | Range source | Notes |
| --- | --- | --- |
| Notes | `Notes_Sequencer_Random_Range`, saved 0 to 83 | continuous float, quantised later ([04](04-pitch-quantise.md)) |
| Velocities | `Velocities_Sequencer_Random_Range` | 0 to 127 sensible |
| Custom ×3 | `Custom-Sequencer_Random_Range[…]` | bounds the modulation values |
| **Gates** | none | uses a 2-way draw (`random 2`): each cell is on or off with p = 0.5 |

Properties (tests): all 16 outputs fall in `[lo, hi)`; the Notes lane's expected value is the midpoint; the
Gates lane has expected density 8 of 16.

The Random draw writes the stored cell values, not the playback order. Shape, Length and Shift are unaffected.

## 5.2 Reset (per lane) **[I]**

Restores the lane's cells to their defaults ⚠ (confirm by pressing Reset on the device):

| Lane | Expected default |
| --- | --- |
| Notes | the ascending default set in [02](02-inputs.md) |
| Gates | all 16 on |
| Velocities | all 64 |
| Custom | all 0 |

## 5.3 Random All / Reset All **[V]**

Single buttons that broadcast to every lane. A MIDI note 48 into the device also fires Random All, with a deferred
call so it lands after the current event. Random All is also a hook for performance: pressing it re-rolls every
lane at once while the clocks keep running.

## 5.4 Gate lane (the rhythm) **[V]**

- Cell widget: a toggle. Default on.
- A Snake Core pulse for cell `k` passes through a one-way gate that is controlled by `Gate_k`. If `Gate_k` is on, the pulse
  leaves the lane as a **trigger**; otherwise it is dropped.

```
on_gate_lane_step(s):
    k = cell_for_step(gate_shape, s)
    if gate[k]: emit TRIGGER
```
So the Gates lane is a pure on/off pattern. It has no pitch or velocity of its own.

## 5.5 Velocity lane **[V]**

- Cell widget: dial 0 to 127, default 64.
- On each step of the Velocities lane's own clock, the selected cell's value is latched (an `int` register). That value is
  what the next trigger will use.

```
on_velocity_lane_step(s):  k = cell_for_step(vel_shape, s);  latched_velocity = vel[k]
```

## 5.6 Notes lane (pitch latch) **[V]**

Same pattern: on each step of the Notes lane's own clock, the selected cell's *quantised note* is latched
(a stored message box). At trigger time the latched note is sent.

```
on_notes_lane_step(s):    k = cell_for_step(notes_shape, s);  latched_note = quantise(notes[k], ...)
```
Quantisation actually happens when the dial changes (or when scale/root change) and the result is stored per cell, so the step
only has to release it. Equivalent behaviour; compute either way.

## 5.7 Custom lanes (parameter modulation) **[V structure]**

Three extra lanes (`Custom`, `Custom[1]`, `Custom[2]`) of 16 continuous values, sequenced exactly like the others,
but their output is a **control signal** sent to any Live parameter you map.

```
on_custom_lane_step(s):
    k = cell_for_step(custom_shape, s)
    v = scale(custom[k], 0..127 → 0..1)          # normalise
    ramp to v over 10 ms                          # `line~` glide, then downsampled to control rate
    param_value = target_min + v * (target_max - target_min)   # via RangeAndName block
```

- **Map** button: clicking it then clicking any Live parameter selects the target; the target's name, min and max are read via the
  Live API (`live.object`) and shown. (The map button auto-clears after a successful map.)
- **Range:** user Min/Max percent values restrict the sweep inside the target's range (and can be inverted by swapping, via a `* −1`).
- **Glide:** a fixed 10 ms ramp between successive values avoids zipper noise.
- Only one parameter can be mapped per Custom lane (three total).

## 5.8 Invariants (tests)

- Gates: with all toggles off, a lane never emits a trigger.
- Random with `lo == hi` yields a constant.
- A Custom value of 0 maps to the target's min and 127 maps to its max (within the Min/Max range).
