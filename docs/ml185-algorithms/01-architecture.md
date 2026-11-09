# 1. Architecture and data flow

## Stage diagram

```
host transport ─► Transportation (+ swinger.js) ─► pulse clock  ──► pulse index p (1..N)
                  (ticks → swung 32nd positions)   live.step, 64 steps,            │
                                                   interval 60/120/240 ticks,       ▼
                                                   direction, loop 1..N      cumulative-sum cascade
                                                                             stage = first k with p ≤ L1+…+Lk
                                                                                    │ pulse inside stage k
                                                                                    ▼
                                                              per-stage counter (reset at stage change)
                                                              fires when counter == 1   (Mode and Division
                                                                                          decide the counter's wrap)
                                                                                    │ "play stage k"
                                                                                    ▼
                         note store `live.step` (8 steps: pitch, velocity, duration, aux) ──► makenote ──► noteout
                                                                                       └──► aux → ctlout / bendout
```

## The two `live.step` objects **[V]**

| Object | Used as | Steps | What it does |
| --- | --- | --- | --- |
| `live.step[2]` | **pulse clock** | 64 | steps through pulses at the Clock Timing rate; `direction` and `loop 1 N` set how it traverses |
| `live.step[1]` | **note store** | 8 | holds pitch / velocity / duration / aux for the 8 stages and outputs the stage's row when told to "play stage k" |

The note store is not clocked by time. It is **addressed**: the stage engine sends it the stage number and it emits that row.
That is why the stages can last different numbers of pulses.

## Components

| Block | Role | Doc |
| --- | --- | --- |
| `p Transportation`, `p swinger` + `swingCalc.js` | convert host ticks into swung 32nd-note positions and drive the pulse clock | [05](05-timing-swing-direction.md) |
| `p stage1 … p stage8` | per-stage control: mode, length, division → messages to the engine | [03](03-stage-engine.md) |
| cascade of `if $i1<=$i2 …` | pulse index → stage number | [03](03-stage-engine.md) |
| per-stage `counter` | picks which pulses inside a stage fire | [03](03-stage-engine.md) |
| `DialLayer` | pitch, velocity and aux dials for the 8 stages, Random | [04](04-pitch-velocity-aux.md) |
| `makenote`, `noteout`, `ctlout`, `bendout` | MIDI out | [06](06-outputs-and-rebuild.md) |
| `p StepDivisionButtonHandler` | the global step-division switch and its (hidden) reset | [03](03-stage-engine.md) |
| `p LEDdriver` | UI feedback only | not algorithmic |

## Event flow for one note **[V structure]**

1. The pulse clock outputs pulse index `p`.
2. The cascade finds the stage `k` that owns `p` and emits a pulse for that stage.
3. Stage `k`'s counter increments; if it equals 1, "play stage k" is sent.
4. The note store outputs stage `k`'s row. A `makenote` gives it a duration and the note-off.
5. Aux (extra2) goes out as CC or pitch bend.

When the pulse moves to a new stage the stage-change message resets that stage's counter, so every stage starts counting from its first pulse.
