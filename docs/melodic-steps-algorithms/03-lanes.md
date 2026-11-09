# 3. The lanes (the core algorithm)

**Input:** a clock tick per lane; the lane's 16 faders and length.
**Output:** the lane's current level (0 to 6).

## 3.1 One lane **[V]**

```
class Lane:
    fader[1..16]          # levels 0..6
    length                # 2..16
    pos = 0

    on_tick():
        pos = pos % length + 1        # a `counter 1 16` whose max is `length`
        return fader[pos]

    on_resync() / on_transport_start(): pos = 0     # next tick reads fader[1]
```
The counter is driven by the shared tick (`---trigg`) and restarted by the shared transport and resync messages. Faders beyond `length` are kept but not read.

## 3.2 Five lanes with independent lengths **[V]**

| Lane | Level means | Length parameter |
| --- | --- | --- |
| **Octave** | octave shift | `OctLength` |
| **Transpose** | scale degree | `TransLength` |
| **Length** | note length (multiplier index) | `LenLength` |
| **Velocity** | loudness | `VeloLength` |
| **Chance** | probability the note plays | `ChanceLength` |

All five advance on every tick. The combined pattern repeats after

```
period = lcm(OctLength, TransLength, LenLength, VeloLength, ChanceLength)
```
For example, the *Dorian* preset has lengths 5, 9, 8, 7, 6, so it repeats only every 2,520 steps (more than 157 bars of 16ths).
Using co-prime lane lengths is the main way the device makes a short melody feel endlessly varied.

## 3.3 Edit actions **[V]**

Each lane has three buttons. They share one handler that receives a "StepControl" command number:

| Command | Action | Effect on the 16 faders |
| --- | --- | --- |
| 1 | **Reset** | every fader = the lane's neutral value (below) |
| 2 | **Not-so-random** | each fader `+= random(3) − 1` (so −1, 0 or +1), clamped to 0..6 by the slider range |
| 3 | **Random** | each fader = `random(7)` (uniform 0..6) |

**Neutral (reset) values per lane** **[V]** (sent at load):

| Lane | Reset value | Meaning |
| --- | --- | --- |
| Octave | 3 | no octave shift (levels 0..6 map to −3..+3 octaves) |
| Transpose | 0 | scale degree 1 (the root) |
| Length | 2 | multiplier 4 (a 16th note) |
| Velocity | 5 | velocity about 102 |
| Chance | 6 | always plays |

So Reset produces a plain "play the root on every step, a 16th long, fairly loud" lane, and the other lanes shape it.
The Length lane also has a **LenRndEven** button; ⚠ its exact rule (random values on even steps only or random with an even bias) was not traced.

## 3.4 Resync and transport **[V]**

Two messages reset every lane counter to the first step together:
- the **transport** message (play/stop edge), and
- the **resync** message from the `resync` block (when speed or triplet mode changes, or when Re-Sync is pressed).

Without this, changing speed mid-bar would leave lanes with different phases.

## 3.5 Level scales (summary; details in [05](05-note-output.md))

| Level | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Octave shift | −3 | −2 | −1 | 0 | +1 | +2 | +3 |
| Transpose (degree index) | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| Length multiplier (64ths) | 1 | 2 | 4 | 8 | 12 | 16 | 32 |
| Velocity | 15 | 32 | 50 | 67 | 85 | 102 | 120 |
| Chance % | 0 | 17 | 33 | 50 | 67 | 83 | 100 |

## 3.6 Invariants (tests)

- All fader levels are integers 0..6.
- Counter positions are always in `1..length`.
- After Reset: Octave all 3, Transpose all 0, Length all 2, Velocity all 5, Chance all 6.
- Not-so-random changes each fader by at most 1 and stays in 0..6.
- Random is uniform over 0..6.
- The combined period equals the LCM of the five lengths.
