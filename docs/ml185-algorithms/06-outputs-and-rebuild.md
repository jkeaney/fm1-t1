# 6. Outputs, rebuild checklist, tests and open questions

## 6.1 Outputs **[V]**

| Output | Source | Format |
| --- | --- | --- |
| Note-on | stage row (pitch + coarse tune, velocity) | `makenote` → `noteout` |
| Note-off | `makenote`, after `duration_ticks` | automatic |
| Aux | stage `extra2` | CC (number = Aux CC) or pitch bend if CC = −1 |
| MIDI thru | incoming MIDI | passed to `midiout` through a switch |

Only one `makenote` voice path exists; notes from consecutive pulses can overlap if the duration exceeds the pulse interval, but the maximum duration (119 ticks) is shorter than the default pulse
interval (120 ticks), so at 1/16 or 1/8 they never overlap. At 1/32 (60 ticks) long notes can overlap the next pulse. ⚠ Check on the device.

## 6.2 Data model

```c
typedef struct { uint8_t pitch, vel, dur /*0..15*/, aux; uint8_t mode /*0..3*/, len /*1..8*/, div /*1..4*/; } Stage;
typedef struct {
    Stage    st[8];
    uint8_t  direction;      // 0..3
    uint8_t  fixed_len;      // 0 = variable
    uint8_t  seq_len;        // fixed-length dial
    uint8_t  clock;          // 0..2  (1/32 1/16 1/8)
    uint8_t  swing;          // 0..200
    int8_t   coarse;         // semitones
    int8_t   aux_cc;         // -1 = pitch bend
} Ml185Pattern;
```
About 70 bytes per pattern, well inside the 6×6×6×6 budget in [PLAN.md](../../PLAN.md).

## 6.3 Build order

1. Stage records and the cumulative thresholds ([03 §3.1](03-stage-engine.md)).
2. Pulse counter with total length and direction ([05](05-timing-swing-direction.md)).
3. Stage counters and mode rules ([03 §3.2](03-stage-engine.md)); test against the worked example.
4. Note emission: pitch, velocity, duration, note-off ([04](04-pitch-velocity-aux.md)).
5. Aux output.
6. Random.
7. Swing.
8. Global step division handler.

## 6.4 Invariant tests

| Test | Assertion |
| --- | --- |
| Ownership | for `p = 1..sum(L)`, `stage_for_pulse(p)` is defined and non-decreasing |
| Total length | `sum(L) ≤ 64` for all legal settings |
| Rest mode | stage with Mode `0` produces no note-on over a full cycle |
| Single mode | Mode `I` produces exactly 1 note-on per stage visit |
| Division | Mode `II`, `L = 8`, `Div = 3` → pulses 1, 4, 7 |
| Worked example | table in [03 §3.3](03-stage-engine.md) |
| Duration | `dur = 15` → 119 ticks; `dur = 0` → −1 (⚠ treat as minimum 1 tick) |
| Random ranges | pitch in 24..95, velocity in 0..126 |
| Swing | `swing = 0` → pulses exactly on the grid; `swing = 200` → odd steps one full step late |
| Reverse | direction `<` visits `p = N … 1` |

## 6.5 Open questions to confirm on the running device

| # | Question | How to check |
| --- | --- | --- |
| 1 | Does Mode `I` really fire only on the first pulse of a stage? | Stage length 4, Mode I; count the notes in one visit. |
| 2 | Is the Mode `I-` duration really the maximum, and is the wrap equal to the stage length? | Compare Mode I and I- note lengths in a clip. |
| 3 | What are the `extra1` default values for Modes I and II? | Record MIDI at several modes and read note lengths. |
| 4 | Fixed-length mode: what plays beyond the last stage if N is larger than the sum? | Set a short total with long fixed length. |
| 5 | Meaning of the direction values 3 and 4 | Watch the LEDs in `><` and `R`. |
| 6 | Swing Amount range | Drag the control, observe the max and the effect. |
| 7 | Aux CC default and how aux and pitch bend scale | Map aux to CC and record. |
| 8 | Behaviour of the first note after Stop (help lists this as a known issue) | Stop and restart mid-bar. |

## 6.6 FM-1 porting notes

- The whole engine is integer counters and comparisons: one pulse counter, a handful of per-stage counters, and an 8-entry threshold list. No floating point is needed
  except in the swing offset, which can be integer ticks.
- Run the pulse clock from the existing 192 PPQN tick: `pulse = tick / ticks_per_pulse`. 192/4 = 48 ticks per 1/16.
- Decide whether stage length is in **pulses** (as here) or in steps of the host's pattern grid. Keeping pulses preserves the cumulative-sum design.
- The "play stage k" interface (address a stored row rather than step through it) maps neatly onto FM-1's track/cycle structure.
