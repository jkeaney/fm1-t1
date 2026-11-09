# 6. Rebuild checklist, tests and open questions

## 6.1 Data model

```c
typedef struct {
    uint8_t fader[16];       // levels 0..6
    uint8_t length;          // 2..16
    uint8_t pos;             // runtime counter
} MsLane;

typedef struct {
    MsLane  oct, trans, len, vel, chance;
    uint8_t root;            // 0..127
    uint8_t scale;           // 0..7
    uint8_t speed;           // 0..5 (menu index)
    uint8_t triplets;        // 0/1
    uint8_t swing;           // 0..100
    uint8_t midi_mode;       // 0/1
} MelodicStepsPattern;
```
About 100 bytes per pattern. Well inside the 6×6×6×6 budget in [PLAN.md](../../PLAN.md).

## 6.2 Build order

1. Lane with length and wrap ([03 §3.1](03-lanes.md)); test the period property.
2. Five lanes advanced by one tick.
3. Scale tables and pitch ([05 §5.1](05-note-output.md)).
4. Velocity, chance, length ([05](05-note-output.md)).
5. Note-off scheduling.
6. Random / Not-so-random / Reset with per-lane neutral values ([03 §3.3](03-lanes.md)).
7. Clock: note values, triplets, double/half, swing ([04](04-clock-and-timing.md)).
8. Resync.
9. MIDI-in root and clock gating (optional).
10. Load the 7 presets as test data ([07](07-presets.md)).

## 6.3 Invariant tests

| Test | Assertion |
| --- | --- |
| Lane wrap | position cycles `1..length` |
| Period | combined period is the LCM of the five lengths (Dorian preset: 2,520) |
| Reset values | Octave 3, Transpose 0, Length 2, Velocity 5, Chance 6 |
| Not-so-random | per-fader change ∈ {−1,0,+1}, stays in 0..6 |
| Random | uniform over 0..6 |
| Pitch | root + degree + 12 × (octave − 3) |
| Scale membership | pitch class relative to root is in the scale |
| Velocity table | 15, 32, 50, 67, 85, 102, 120 |
| Chance table | 0, 17, 33, 50, 67, 83, 100 % |
| Length table | 1, 2, 4, 8, 12, 16, 32 sixty-fourths |
| Note-off | exactly one per note-on, after the computed length |
| Swing | off-beat step delayed by `swing% × sixteenth` |
| **Preset golden** | all 7 presets have levels in 0..6 and lengths in 2..16 (verified) |

## 6.4 Open questions to confirm on the running device

| # | Question | How to check |
| --- | --- | --- |
| 1 | Transpose level 0 returns the root (coll key base)? | Reset Transpose; read Note Out. |
| 2 | Chromatic scale spans only 0 to 6 semitones? | Transpose levels 0 to 6 on Chromatic. |
| 3 | Does the clock run when the transport is stopped? | Press play in MIDI-off mode with the transport stopped. |
| 4 | Does note-off in MIDI mode stop the clock? | Release the key. |
| 5 | Exact resync boundary | Change speed mid-bar and watch the lane playheads. |
| 6 | What does LenRndEven do? | Press it several times; read the Length lane. |
| 7 | Swing range: is 100% one full 16th, and is the maximum 127? | Measure the off-beat offset at 50, 100, 127. |
| 8 | Does Swing delay only notes or also the lane counters? | Compare with a very long note length. |
| 9 | MIDI output channel | Monitor the output. |

## 6.5 FM-1 porting notes

- The engine is five counters, a 7×8 lookup (scale) and a few linear scales. It needs no floating point except the swing delay; keep levels as 3-bit integers (a whole lane is 6 bytes plus length).
- Pitch from *scale degree plus octave* (rather than semitone) is a good fit for FM-1's keyboard-less editing; every value is musical by construction.
- Because the lanes are independent, FM-1 can expose each lane's length as a track-cycle length. This reproduces the polymeter without any new timing machinery.
- Decide whether to keep note-off as a zero-velocity note-on or use real note-offs.
- Use a seeded PRNG for Chance, and a separate stream for Random so editing a pattern does not change playback randomness.
