# 3. Clock, shuffle and resync

**Input:** host beat position (in beats), play state, Clock Speed, Shuffle 1 to 8, Shuffle Length, Global Shuffle, Auto Retrigger.
**Output:** a "next step" bang at each step time.

## 3.1 The step grid **[V]**

The sequencer works on a **2-beat cycle of 8 steps** (a 16th-note grid at 100% speed):

```
cycle position  c = (beat_position × speed) mod 2          # in beats
step i is due when c reaches threshold[i]                   # i = 0..7
threshold[i] (no shuffle) = i × 0.25                        # 0, 0.25, 0.5 … 1.75 beats
```
Each threshold is tested by a comparison and each step is allowed to fire only once per cycle (a `onebang` per step, re-armed at the start of the next cycle).
The same circuit re-arms all eight steps when the transport position jumps backwards (loop or locate), so patterns restart correctly on a transport jump. **[V]**

## 3.2 Per-step time shuffle (`TimeShuffler`) **[V]**

Eight shuffle sliders move steps off the grid. Each slider is scaled by Global Shuffle, then mapped onto a **±0.1 beat** window around its step:

```
g = Global Shuffle / 100
step 0 :  threshold[0] = scale(Shuffle1 × g,   0..50  → 0 .. 0.1)           # step 1 can only be late
step k :  threshold[k] = scale(Shuffle(k+1) × g, -50..50 → (k·0.25 - 0.1) .. (k·0.25 + 0.1))    # k = 1..7
```
Concrete windows (beats): step 2: 0.15 to 0.35, step 3: 0.40 to 0.60, step 4: 0.65 to 0.85, step 5: 0.90 to 1.10, step 6: 1.15 to 1.35, step 7: 1.40 to 1.60, step 8: 1.65 to 1.85.
So the maximum displacement is 0.1 beat (40% of a 16th) either way. At −50 the step lands 0.1 beat early, at +50 0.1 beat late.

- **Shuffle Length (2 to 8)**: how many of the 8 shuffle values are active; the active ones are repeated across the cycle and the rest are zeroed. ⚠ Confirm the repeat rule on the device.
- **Global Shuffle** is a master amount; 0% gives a straight grid whatever the sliders say.
- A **Randomize** and **Reset** button exist for the shuffle sliders.

Because the shuffle thresholds can pass each other (a late step 1 beyond an early step 2), the "fire once per cycle" rule keeps the order stable. ⚠ Behaviour when two thresholds cross is not documented.

## 3.3 Clock Speed **[V structure, ⚠ mapping]**

A menu of six percentages scales how fast the cycle position advances (`cycle_position = beats × speed`). 100% = 16th notes; 200% = 32nds; 50% = 8ths.
The saved menu labels are `50 %, 66 %, 75 %, 100 %, 150 %, 200 %`, but the factor messages in the patch are 50, 75, 100, 125, 150, 200. ⚠ So the menu-to-factor mapping is not one-to-one by label;
confirm each setting by measuring the step rate.

Step counters in the lanes advance once per step bang, so Clock Speed also scales the effective length of every layer in time.

## 3.4 Auto Retrigger **[V structure]**

With **Auto Retrigger** on, every layer counter in every lane is reset to step 1 once every `Auto Trigger Rate` bars (1 to 4). The reset is sent as a "resync per-type sequencers" bang, and the device also resyncs on the next time-shift loop of the host.
This guarantees that long polymetric patterns re-align with the song every few bars. 11 of the 19 factory presets turn it on.

## 3.5 Per-tick order of operations

1. Host position → `c` using Clock Speed.
2. Test each of the 8 thresholds; a step fires the first time `c` passes it in the cycle.
3. Step bang → each lane advances its five layer counters ([04](04-lane-layers.md)).
4. If Auto Retrigger is due → reset all layer counters first, so step 1 plays.

## 3.6 Invariants (tests)

- With all shuffles 0: step times are `0, 0.25, … 1.75` beats.
- With `Global Shuffle = 0`: identical to the straight grid.
- At most 8 step bangs per 2-beat cycle at 100% speed.
- A backwards jump in beat position re-arms all steps (no steps are skipped after a loop).
- Shuffle displacement magnitude never exceeds 0.1 beat.
