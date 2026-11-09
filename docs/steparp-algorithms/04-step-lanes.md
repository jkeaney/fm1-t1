# 4. Per-step lanes

All lanes share one widget: **16 slider values** read by the current step number, plus a min/max range, a Random button and (for some) an on/off.

## 4.1 The lane widget **[V]**

```
lane.values[1..16]                         # one value per step
lane.min, lane.max                         # slider range (e.g. velocity 27..100)
value_for(step) = lane.values[step]        # `zl nth`: list item at the step number

random(lane): for i in 1..16: lane.values[i] = uniform(lane.min, lane.max)
```
- Changing the range re-scales the sliders (`setminmax`).
- The step number is the master clock's current step (1 to LoopLength), so every lane stays in sync.
- Each lane is edited by dragging across the sliders; there is no per-lane loop length: all lanes use the global `LoopLength`.

## 4.2 Step on/off (`Steps1`) **[V]**

Each step has a toggle (default on). The clock sends the step number; if the step's toggle is on, a "generate" bang goes to the note engine, otherwise the
step is silent and the note engine's counter does **not** advance. LoopLength sets how many steps exist (the rest are hidden and unused).

## 4.3 Velocity **[V]**

`velocity = Velo.values[step]` (0 to 127). Passed straight to the note's velocity.

## 4.4 Length **[V]**

```
note_length_ms = (Length.values[step] / 127) * step_period_ms          # step_period_ms = 1000 / step_frequency_hz
```
So the lane is a **gate fraction** of the step: 127 = full step, 64 ≈ half. Because the step frequency follows Division and tempo, lengths stay musical. Length can exceed none (max 100% in the formula); overlap with the next step is not possible.

## 4.5 Chance **[V]**

```
if ChanceOn:
    r = random(100)                         # 0..99
    pass = r < Chance.values[step]          # percent
else:
    pass = true
```
Applied **per hit**, after ratchet. A chance of 0 never plays, 100 always plays.

## 4.6 Ratchet **[V structure, ⚠ off behaviour]**

The lane value `n` (0 to 3 by default) is the **number of ratchets**. A small repeat engine fires `n` extra hits spaced evenly through the step:

```
hit_interval_ms = step_period_ms / (n + 1)
start a timer at hit_interval_ms; fire once per tick; stop after n hits
```
- The base hit is the normal trigger; the ratchet adds `n` evenly spaced repeats (so `n = 0` is a single hit, `n = 3` is four hits per step). ⚠ The inner counter wiring is subtle;
  confirm the hit count (n or n+1 total) on the device.
- Each repeat has the same pitch, goes through **Chance** independently, and uses the step's velocity.
- When `RatchetOn` is off the lane is ignored. ⚠ The bypass wiring for the off state wasn't fully traced; expect a single hit.

## 4.7 Transpose **[V]**

`pitch' = pitch + Trans.values[step]` when `TransposeOn`. Range is the lane's min/max (default −7 to +7 for Random). `TrnspAll` sets every step to one value; `TranspReset` zeros the lane.
The shift is applied **before** scale snap and ratchet.

## 4.8 Map / CC lane **[V structure]**

`Data.values[step]` (0 to 127) is sent each step as:
- a MIDI **CC** (number from `MIDICC`, Off or 1 to 128) via `ctlout`, and/or
- a value to any of up to 8 mapped **Live parameters** (eight `Abl.Map` voices), with an optional `Smooth` glide.

The Map lane is independent of the note engine (it keeps sending even for off steps ⚠), and the held-notes flag is multiplied in so the output is muted when no notes are held.

## 4.9 Invariants (tests)

- Velocity output is always `values[step]` (clamped 0..127).
- Length output ∈ [0, step_period].
- Chance 100 → always plays; 0 → never plays; mean play rate → value/100.
- Ratchet n produces hits at multiples of `step_period/(n+1)` inside the step.
- Random never leaves [min, max].
