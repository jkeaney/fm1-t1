# 4. Ratchet, pace, gate and velocity slope

**Input:** `repeats`, `pace`, `time`, `offset`, `velocity`, `sustain`, `delay_ticks`, and the length of the current step in ticks.
**Output:** up to 16 repeats per pulse, each with a start tick, a gate length and a velocity.

## 4.1 Repeat timing and the pace warp **[V]**

For repeat *k* of *n* (`repeats`), the evenly spaced position `x = k/n` (0 to 1, in Q15 fixed point) is warped to `y`, then scaled to the step length:

```
bell    = x * (1 - x)                         # Q15, peaks at 0.25 at x = 0.5
y       = x - (pace / 64) * bell              # clipped to 0..1
start_k = y_k * step_ticks                    # offset from the pulse start
end_k   = y_{k+1} * step_ticks
interval_k = max(end_k - start_k, 1)
```
This is a **quadratic bell warp**, not the "logarithmic / exponential Q15 table" the guide describes. Properties:
- `pace = 0`: linear, evenly spaced.
- `pace < 0`: **accelerating** (early intervals long, later ones short, like a bouncing ball). `y > x`.
- `pace > 0`: **decelerating** (burst first, then spread). `y < x`.
- The maximum displacement is `|pace|/64 × 0.25` of the step (about 25% at ±63 or −64), and the curve stays monotonic over the whole range, so repeats never reorder.

Measured start ticks for 4 repeats in a 192-tick step (gate in brackets, `time = 64`):

| pace | repeat 1 | repeat 2 | repeat 3 | repeat 4 |
| --- | --- | --- | --- | --- |
| −64 | 0 (g42) | 84 (g30) | 144 (g18) | 180 (g6) |
| −32 | 0 (g33) | 66 (g27) | 120 (g21) | 162 (g15) |
| 0 | 0 (g24) | 48 (g24) | 96 (g24) | 144 (g24) |
| +32 | 0 (g15) | 30 (g21) | 72 (g27) | 126 (g33) |
| +63 | 0 (g6) | 12 (g18) | 48 (g30) | 108 (g42) |

A `sub_tick` (the fractional part of the start tick, in 1/16384 tick units) is also computed; the pipeline's own dispatcher ignores it (it only orders events inside the queue).

## 4.2 Gate length **[V]**

```
gate_k = interval_k * time / 128            # time 1..127 (0 → 64); 128 would be 100% of the interval
if sustain > 0:  gate_k = gate_k * sustain / 100       # sustain 1..255 %
gate_k = max(gate_k, 1)
note_off = note_on + gate_k
```
- With `repeats = 1` the "interval" is the whole step.
- With the defaults (`time` 0 → 64 = 50%, `sustain` 50%) a note lasts **25% of its interval**.
- `sustain` above 100% makes the gate longer than the interval, so notes overlap the next repeat or step. The default bass uses `time = 127, sustain = 90`, giving gates of about 89% of the step.

## 4.3 Velocity slope **[V]**

```
vel_k = clamp( base + k * offset / (n - 1), 1, 127 )        # n > 1; base = velocity (0 → 100)
```
`offset` is the total change from the first to the last repeat. Example: 8 repeats, base 100, offset −40 gives `100 95 89 83 78 72 66 60`.

The pipeline then shifts every repeat by the amount the pulse's velocity moved from its base (p-lock, LFSR modulation and accent), so those effects apply equally to all repeats of a pulse.

## 4.4 Micro-delay **[V]**

`delay_ticks` (−12 to +12) is added to each repeat's start. Because the result is clamped to **not earlier than the current tick**, a negative delay has **no effect on the first hit** (its offset is 0) and only pulls later repeats earlier. A positive delay shifts every hit later. The CC event is not delayed.

## 4.5 Invariants (tests)

- `repeats = 1` produces one hit at offset 0.
- Offsets are non-decreasing in `k`; the first is always 0.
- `pace = 0` gives `offset_k = k × step / n` (up to integer division).
- Gate lengths are at least 1 tick.
- Velocities stay in 1 to 127.
