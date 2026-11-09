# 5. Playback and timing

**Input:** transport position, Division, Steps, Offset, Direction, Swing, Clock mode, Poly Reset, jitter amount.
**Output:** for each step tick, a `position` (0 to 15) used to read all four lanes, and a delay for swing.

## 5.1 Division → steps per beat **[V]**

| Division | 1/48 | 1/32 | 1/24 | 1/16 | 1/12 | 1/8 | 1/6 | 3/16 | 1/4 | 5/16 | 1/3 | 3/8 | 1/2 | 3/4 | 1 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Steps per quarter note | 12 | 8 | 6 | 4 | 3 | 2 | 1.5 | 1.33 | 1 | 0.8 | 0.75 | 0.67 | 0.5 | 0.33 | 0.25 |

Rule: `steps_per_beat = 4 / (division × 4)`, i.e. a 1/16 step is 0.25 beat. The device stores the
rounded values above; rebuild from the formula and compare.

## 5.2 Step counter **[V]**

```
tick  = host transport position (quarter-note ticks)
raw   = floor(tick * steps_per_beat)         # free-running step count
if poly_reset != ∞:  raw = raw mod poly_reset    # ⚠ inferred
```
- **Clock = Cont:** `raw` always advances with transport.
- **Clock = Gate:** `raw` advances only while a MIDI note is held (the note-held flag gates the clock).
- Counting is quantised to a metro on the 8th grid in the device, which is a Max implementation detail; for FM-1 use the native sequencer tick.

## 5.3 Length, offset and direction **[V]**

```
L = Steps                       # 1..16
if Direction == Fwd:      pos =  raw mod L
if Direction == Bwd:      pos = (L-1) - (raw mod L)
if Direction == Pendulum: pos =  triangle(raw, L)     # bounces 0..L-1..0
```
- `triangle` is a triangle wave of period `2(L-1)`: `m = raw mod (2(L-1)); pos = m < L ? m : 2(L-1) - m`.
  ⚠ The device uses a single `expr` with absolute value; the end-point behaviour (whether ends repeat)
  is not confirmed. Test with L = 4: expected `0 1 2 3 2 1 0 1 …` (no repeated ends).
- **Offset** rotates the lanes before reading: `lane' = rotate(lane, offset)`. All four lanes rotate together.
  Changing Offset does not regenerate.
- **Steps** below 16 uses the first L slots; setting Steps also resets Offset to 0 in the Push helper (UI behaviour, not in the core).

## 5.4 Swing **[V structure, ⚠ units]**

```
swing_amount = scale(swing, 0..1 → 0..125)
if (step_index mod 2) == 1:       # every second step
    delay output by swing_amount   # `pipe`
else:
    no delay
```
- ⚠ The unit of 125 (ms vs ticks vs % of step) is not confirmed. A musical version: delay the off-steps by `swing × 0.5 × step_length`.

## 5.5 Jitter (probabilistic step displacement) **[V structure, ⚠ exact numbers]**

Randomly moves the step *index* before it reads the lanes, creating variation.

```
amount in 0..1
trigger_prob = (1001 - scale(amount^0.6, 0..1 → 1001..200)) / 1000   # amount skewed by ^0.6
if rand(1000) > threshold:       # fires
    max_shift = amount * 15
    shift     = round(0.5 * max_shift) * (rand(2) ? +1 : -1)
    pos       = clip(abs(pos + shift), 1, 16)
else:
    pos unchanged
```
- Higher amount → fires more often and shifts further. `pow 0.6` lifts small amounts so a little jitter is audible.
- ⚠ The indexing base (1 to 16 vs 0 to 15) and clip are as the device does it; for 0-based storage use `0..L-1` and wrap.

## 5.6 Order of operations per tick

1. `raw` from transport and Division.
2. Apply Poly Reset.
3. Apply Direction + Steps → `pos`.
4. Apply Jitter to `pos`.
5. Apply Offset rotation to the lane read.
6. Read `pitch, octave, gate, vel` at `pos`.
7. Apply Swing delay on odd steps.
8. Emit the note (see [06](06-outputs.md)).
