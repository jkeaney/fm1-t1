# 3. The rhythm engine (the core algorithm)

**Input:** a cycle's `steps`, `pulses`, `rotate`, manual masks, `division`, and the pattern `swing`.
**Output:** for each track, a stream of *steps*, and for each step whether it is a *pulse*.

## 3.1 Euclidean (Bjorklund) distribution **[V]**

`t1_euclidean_bjorklund(steps, pulses)` returns a 64-bit mask; bit *k* set means step *k* is a pulse. It is the classic iterative Bjorklund algorithm: start with `pulses` groups "1" and `steps − pulses`
groups "0", repeatedly append zero-groups onto one-groups, until at most one zero-group remains, then concatenate.

```
E(p, s):  pulses spread as evenly as possible over s steps
edge cases:  s = 0 → 0 ;  p = 0 → 0 ;  p ≥ s → all s bits set ;  s > 64 → 64
```

**Verification (run on the real code):** for 12 (steps, pulses) pairs the output equals an independent floor-based reference **up to rotation** in every case, and always has exactly `pulses` bits set.

| E(p,s) | Pattern (x = pulse) |
| --- | --- |
| E(3,8) | `x..x..x.` (tresillo) |
| E(5,8) | `x.xx.xx.` |
| E(4,16) | `x...x...x...x...` |
| E(5,16) | `x..x..x..x..x...` |
| E(7,16) | `x..x.x.x..x.x.x.` |
| E(9,16) | `x.xx.x.x.xx.x.x.` |
| E(5,12) | `x..x.x..x.x.` |
| E(23,64) | `x..x.x..x..x..x..x.x..x..x..x.x..x..x..x..x.x..x..x..x.x..x..x..` |

The first step is always a pulse when `pulses ≥ 1`.

## 3.2 Rotation **[V]**

```
r = ((rotate mod steps) + steps) mod steps
rotated = ((mask >> r) | (mask << (steps - r))) & step_mask
```
A positive `rotate` moves each pulse to a **lower** step index (bit *k* → *k − r*, wrapping). Pulse count is preserved. `rotate` is −64 to +64 and wraps around the cycle length, so for `steps = 16` a rotation of +20 equals +4.

## 3.3 Manual pulses ("Step Mod") **[V]**

```
mask = (rotated | (active_mask_hi << 32 | active_mask_lo)) & step_mask
```
The manual mask is **OR-ed in**, so it can only **add** pulses; it cannot mute one (the guide says "muting or forcing"; only forcing is possible). The UI edits `active_mask_lo` only (steps 1 to 32). See finding F8 for how the knob selects the bit.

## 3.4 Step length: divisions **[V]**

192 ticks per quarter note. `division` indexes this table:

| Index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Name | 1/1 | 1/2 | 1/4 | 1/8 | 1/16 | 1/32 | 1/8T | 1/16T | 1/32T | 1/8D | 1/16D |
| Ticks | 768 | 384 | 192 | 96 | 48 | 24 | 64 | 32 | 16 | 144 | 72 |

An out-of-range index plays as 1/16 (48 ticks).

## 3.5 Swing **[V]**

```
if swing <= 50: length = base
else:           swing = min(swing, 75)
                delta = base * (swing - 50) / 100
                even-numbered step (0, 2, …): length = base + delta       # lengthened
                odd-numbered step:             length = base - delta      # shortened
```
Swing delays the **odd** steps, and a pair of steps always totals `2 × base`. Measured note-on ticks for a 16-pulse 1/16 pattern (base 48):

| swing | note-on ticks (first 5 steps) |
| --- | --- |
| 50 | 0 48 96 144 192 |
| 60 | 0 **52** 96 **148** 192 |
| 70 | 0 **57** 96 **153** 192 |
| 80 (clamped to 75) | 0 **60** 96 **156** 192 |

`swing` is a **pattern-level** value (default 50). Note: **no control on the FM-1 sets it** (F11), and in the host tests only.

## 3.6 Per-track step counter **[V]**

Each track keeps a runtime (`T1TrackRuntime`): `step_index`, `tick_accum`, `current_step_len`, `pulse_counter`, `cycle_pass`, a cached mask, and an LFSR.

```
on a tick, for each non-muted track:
    if step_index == none:                  # transport start
        step_index = 0; step_len = swung(division, 0); accum = 0; evaluate step 0
    elif accum >= step_len:                 # the step is over
        step_index = (step_index + 1) mod steps
        if step_index == 0: cycle_pass++, LFSR cycle boundary
        step_len = swung(division, step_index); accum = 0; evaluate the step
    else:
        accum++ ; done
    accum++ after evaluating
```
- Steps are therefore `step_len` ticks apart, and changing `division` takes effect at the **next step boundary**.
- Each track has its own `steps` and `division`, so tracks of different lengths drift against each other: a **polymeter**. Two tracks with 16 and 12 steps at the same division realign every 48 steps.
- The Euclidean mask is **cached** per track and only recomputed after `t1_pipeline_invalidate_masks()`; any edit that changes `steps`, `pulses`, `rotate` or the active cycle must call it (finding F7).
- There is **no automatic cycle chaining**: `cycle_count` is unused and the active cycle changes only when you select one. `cycle_pass` counts loops and is used only for the LFSR boundary.
- A **muted** track is skipped entirely, including its counters, so it **freezes** and falls out of phase with the others when un-muted (finding F10).
- `master_length_override` (MAX LEN) is stored by the UI but not read by the engine (F11).

## 3.7 Invariants (tests)

- `popcount(E(p, s)) = min(p, s)` for `s ≤ 64`; E(p, s) with p ≥ s is all ones.
- Rotation preserves pulse count; rotating by `s` is the identity.
- With `swing ≤ 50` every step lasts `division_ticks`; with `swing = s` odd steps last `base − base·(s−50)/100` ticks.
- Two tracks with lengths a and b (same division) realign after `lcm(a, b)` steps.
