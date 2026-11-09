# 5. Timing, swing and direction

## 5.1 Transport and tick base **[V]**

`p Transportation` polls the host `transport` object every 10 ticks and builds a `time` message for the pulse clock from the bar, beat, units, resolution and time
signature. The tick base is **480 ticks per quarter note** (a 32nd = 60 ticks). The pulse clock is therefore locked to the host song position,
not free-running.

## 5.2 Clock Timing → pulse interval **[V]**

| Clock Timing | Interval (ticks) | Pulses per quarter note |
| --- | --- | --- |
| 1/32 | 60 | 8 |
| 1/16 (default) | 120 | 4 |
| 1/8 | 240 | 2 |

A stage of length `L` therefore lasts `L` pulses of this interval. One stage can last from one 32nd to eight 1/8 notes (one bar of 4/4 is 8 pulses at 1/8).

## 5.3 Direction **[V mapping, ⚠ semantics]**

| Direction tab | Label | Value sent to the clock |
| --- | --- | --- |
| 0 | `>` forwards | 0 |
| 1 | `<` backwards | 1 |
| 2 | `><` pendulum | 3 |
| 3 | `R` random | 4 |

Two small remaps (`3 → 4`, then `2 → 3`) skip value 2 in the clock's own direction list. ⚠ The meaning of clock direction values is Max's `live.step`;
treat `3` as "forward then back" and `4` as "random" as the likely reading.

Direction acts on the **pulse index**, not on the stage list:
- backwards plays pulse `N` first, so stages come out last-to-first **and** the pulses inside each stage are visited in reverse.
- the Mode `I` / `II` counters still count upward within a stage, so the *firing* positions inside a reversed stage can differ from the forward case.

The device's help lists as a known issue that Random "doesn't hold the entire length of a stage" (a random pulse each tick breaks the stage-length contract), and that
the fixed-length mode behaves differently when not running forward.

## 5.4 Total length: variable vs fixed **[V structure, ⚠ details]**

A gate-switch (`Length Mode`) chooses what `loop 1 $1` tells the pulse clock:

| Length Mode | Loop length N |
| --- | --- |
| Variable | `sum(L1..L8)` (computed with `zl sum` over a packed list of stage lengths) |
| Fixed | `Sequence Length × 4` (⚠ the dial value times 4 pulses; default 8 → 32) |

In fixed mode the loop can be shorter than the sum of the stage lengths (cutting later stages off), or longer (leaving a tail with no owning stage ⚠ which plays nothing). The clock always
has a maximum of 64 steps.

## 5.5 Swing **[V structure, ⚠ exact feel]**

`swingCalc.js` computes eight swung boundary times for one quarter note and the `p swinger` sub-patch uses them as thresholds to turn raw
ticks into a swung 32nd-note index.

```
base grid (ticks within a quarter): 0, 60, 120, 180, 240, 300, 360, 420        # eight 32nds
swing = Swing Amount / 200                                                      # 0..1
# the offset depends on the Clock Timing base:
1/32:  offset = 60  * swing ;  every odd 32nd  += offset
1/16:  offset = 120 * swing ;  every odd 16th  += offset   (pairs of 32nds move together)
1/8:   offset = 240 * swing ;  every odd 8th   += offset
```
- Swing therefore delays the **off-beat** pulse by up to one full step (`swing = 1` moves it to the next grid position), and it moves *all* the 32nds inside that step with it.
- Positions that fall off the end of the quarter are clamped to 480.
- Thresholds are compared with the incoming tick (`>=` tests); the number of passed thresholds gives the swung 32nd index, which is then scaled to the pulse clock's own interval.
- The swing amount arrives as an integer; dividing by 200 means the control range is 0 to 200 → 0 to 1. ⚠ confirm the UI range.

Swing applies to the pulse stream, so it affects **every** note and every stage equally.

## 5.6 Per-tick order of operations

1. Host ticks → swung 32nd index (swinger).
2. Pulse clock applies Clock Timing, Direction and loop length → pulse `p`.
3. Cascade → stage `k`; counter → fire or not ([03](03-stage-engine.md)).
4. Note store → pitch/velocity/duration/aux → MIDI ([06](06-outputs-and-rebuild.md)).
