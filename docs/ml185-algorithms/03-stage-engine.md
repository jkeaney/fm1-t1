# 3. The stage engine (the core algorithm)

**Input:** pulse index `p` (1 to N, from the pulse clock), and per-stage `Length`, `Mode`, `Div`.
**Output:** "play stage k" events.

## 3.1 Pulse index → stage (cumulative-sum cascade) **[V]**

Let `L1..L8` be the stage lengths (each 1 to 8). Stage thresholds are cumulative sums:

```
C1 = L1
C2 = L1 + L2
...
C8 = L1 + ... + L8              # total pulses in the sequence (≤ 64)
```
A chain of comparators tests the pulse index against each threshold. The first test that passes decides the stage:

```
function stage_for_pulse(p):
    for k in 1..8:
        if p <= C[k]: return k
    return none                  # beyond the end (fixed-length mode only)
```
In the patch each test is `if p <= C then <stage k> else pass p on to the next`. The cumulative adders are a chain of `+` objects, and the stage 1
threshold is held in a number box that is re-banged whenever any stage length changes, so thresholds always follow the latest lengths. **[V]**

Pulse position **inside** the stage is implicit: the stage's counter (below) counts pulses since the stage began.

## 3.2 Per-stage counter and trigger **[V structure]**

Every stage has a counter that counts up from 1 and wraps at a maximum `M_k`. A pulse arriving for stage `k` increments
its counter. **A note fires when the counter reads 1.** At the start of each new stage, a stage-change message resets that counter so
the first pulse reads 1.

```
on_pulse_for_stage(k):
    counter[k] = counter[k] % M[k] + 1          # 1..M[k]
    if counter[k] == 1: play_stage(k)

on_stage_change(k):  counter[k] = 0              # so the first pulse makes it 1
```

`M[k]` depends on the stage's Mode:

| Mode | `M[k]` | Result | Source |
| --- | --- | --- | --- |
| `0` | 0 (counter held at 0) | never fires: a **rest** | **[V]** a zero is sent as the division and the counter min is set to 0 |
| `I` | `L[k]` (the stage length) | fires once, on the **first pulse** | **[I]** a "single" message sets the wrap to the stage length |
| `II` | `Div[k]` (1 to 4) | fires on pulses 1, 1+Div, 1+2·Div … : a **repeat/ratchet** | **[V]** the division is sent as the wrap |
| `I-` | `L[k]` (like `I`) **and** duration forced to the maximum | fires once, with a **long note** | **[I]** same single message, plus an "extra1 = 15" message |

Mode summary, as the device's own help describes it: *4 play modes, 8 lengths, and the middle switch works as a clock divider for play mode II*.

## 3.3 Worked example

Lengths `L = [3, 2, 4, 1, 1, 1, 1, 1]`, modes all `II`, `Div = [1, 2, 1, 1, 1, 1, 1, 1]`:

| Pulse p | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Stage | 1 | 1 | 1 | 2 | 2 | 3 | 3 | 3 | 3 | 4 | 5 | 6 | 7 | 8 |
| Counter | 1 | 1 | 1 | 1 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| Fires | ✓ | ✓ | ✓ | ✓ | – | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

(Stage 1 with `Div = 1` re-fires on every pulse; stage 2 with `Div = 2` fires on its 1st pulse only, because its length is 2.)
Use this table as a unit test. ⚠ The exact counter reset timing at stage boundaries should be confirmed on the device.

## 3.4 Global Step Division **[V structure]**

The `Global Step Division` switch sets the `Div` of **all** stages at once through a small handler; the individual stage switches can
then override one stage. The hidden `StepDiv Reset` returns every stage to the global value. The handler only moves values between
the 9 switches; it does not change the engine.

## 3.5 Invariants (tests)

- Total pulses = sum of stage lengths, never more than 64.
- For any `p`, exactly one stage owns it (or none, past the total in fixed-length mode).
- Mode `0` stages never fire.
- Mode `I` stages fire exactly once per stage visit.
- Mode `II` with `Div = d` fires `ceil(L/d)` times per visit.
