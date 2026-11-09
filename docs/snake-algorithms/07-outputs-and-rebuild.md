# 7. Outputs, rebuild checklist, tests and open questions

## 7.1 Output events **[V]**

| Output | Source | Format |
| --- | --- | --- |
| MIDI note-on | Gates trigger + latched pitch + latched velocity, after swing | pitch 0 to 127, velocity 0 to 127 |
| MIDI note-off | delayed bang of `flush` after the note-length delay | for all held notes |
| Live parameter writes | three Custom lanes | normalised 0..1 mapped into the target's range, 10 ms glide |

`noteout` is the only note output (via a `flush` object, which also guarantees no stuck notes on transport stop or when the device is disabled).
There is no clip-writing mode in this device; it plays live.

## 7.2 Data model for a rebuilt module

```c
typedef struct {
    uint8_t  shape;          // 1..14
    uint8_t  length;         // 1..16
    uint8_t  speed;          // 0..4  (32n 16n 8n 4n 2n)
    uint8_t  direction;      // 0..4
    uint8_t  shift_pct;      // 0..100
    uint8_t  order[17];      // derived: order[step] = cell
} LaneClock;

typedef struct {
    LaneClock notes_clk, gates_clk, vel_clk, custom_clk[3];
    uint16_t note_x[16];     // fixed-point of 0..84, e.g. ×256 (continuous)
    uint16_t gate_bits;      // 16 toggles
    uint8_t  vel[16];        // 0..127
    uint8_t  custom[3][16];  // 0..127
    uint8_t  scale;          // 0..36
    uint8_t  root;           // 0..11
    uint8_t  swing;          // 0..127 or 0..100
    uint8_t  note_len;       // 0..4  (64n 32n 16n 8n 4n)
} SnakePattern;
```
About 160 bytes per pattern. Comfortably inside the 6×6×6×6 budget in [PLAN.md](../../PLAN.md).

## 7.3 Build order (each step is independently testable)

1. **Shape tables + inverse** ([03](03-snake-shapes.md)).
2. **Single-lane clock:** length, direction, speed, shift → step number ([06](06-playback-timing.md)).
3. **Lane value store + cell lookup** (Velocity first; it is the simplest).
4. **Quantiser + scale tables** ([04](04-pitch-quantise.md)) with the default-value check below.
5. **Gate lane + trigger**, then **latched pitch/velocity** combine into a note.
6. **Random / Reset** ([05](05-lane-generation.md)).
7. **Swing and note-off** ([06](06-playback-timing.md)).
8. **Polymeter test:** run all lanes with different lengths.
9. **Custom lanes** (optional; FM-1 can map them to its own parameters instead of Live's).
10. **MIDI-in control** (root, user scale, random-all).

## 7.4 Invariant tests (no device needed)

| Test | Assertion |
| --- | --- |
| Shape permutation | For each of the 14 shapes, `sort(shape) == 1..16`. |
| Shape inverse | `order[shape[k]] == k` for all k. |
| Contiguous path | For shapes 5 to 8 and 11 to 14, consecutive steps are grid neighbours (Manhattan distance 1). |
| Identity | Shape 1: `order[s] == s`. |
| **Quantiser golden** | With C major, root 0, the 16 default note values produce `24 26 28 29 31 33 35 36 38 40 41 43 45 47 48 50`. |
| Scale membership | For any `x`, `(note − 60 − root) mod 12` is a degree of the scale. |
| Octave bands | `x` in `[0,12)` gives notes 24..35 under Chromatic-like scales; `[36,48)` gives 60..71. |
| Length/shift bound | For all `L`, `shift ≤ 16 − L` and `step ∈ [1,16]`. |
| Gate off | All gates off → zero note-ons for any number of ticks. |
| Polymeter period | With lengths 5 and 3 (same speed), the combined pattern repeats every 15 steps. |
| Random range | All Random outputs lie in `[lo, hi)`. |
| Note-off | Every note-on is followed by exactly one note-off after the note length. |

## 7.5 Open questions to confirm on the running device

| # | Question | How to check |
| --- | --- | --- |
| 1 | Is the out-of-range scale index clamped to the last degree? | Set a note dial near the top of an octave band; read the pitch. |
| 2 | What does **Reset** restore, per lane? | Randomise, press Reset, read each lane. |
| 3 | Ping-pong end behaviour (fore/back): do end steps repeat? | Length 4, fore/back, log step order or watch the grid. |
| 4 | Random direction: can it repeat the same step? | Watch 50 ticks. |
| 5 | Swing exact timing at 0, 0.5, 1. | Record MIDI to a clip and measure the off-beat offset. |
| 6 | Is the "Note length" menu really `Direction_convert`? | Change it and listen or measure the note durations. |
| 7 | User scale: sorted? de-duplicated? length? | Play notes in odd order, with repeats, and read the resulting pitches. |
| 8 | Which octave names do MIDI-in notes 60 to 71 and 72 to 83 use in Live? | Play the notes from a MIDI keyboard. |
| 9 | Does the "Minor" scale have 7 or 12 entries at runtime? | Compare the output of Minor with MelodicMinor. |
| 10 | Default shape at load. | Open a fresh instance. |

## 7.6 FM-1 porting notes

- The whole engine is table lookups and integer math: no floating point is needed except the swing phase, which can be a 16-bit fixed-point fraction of the step.
- The polymetric lanes are cheap: one counter and one lookup each. Run them from the same 192 PPQN tick (`step = floor(tick / ticks_per_step)`)
  so they stay phase-locked like the original.
- Decide early whether the **Gates lane stays the sole trigger** (as in Snake) or whether each lane may trigger notes. The Snake design depends on that separation.
- Think about how to map Custom lanes to FM-1's parameter locks rather than to Live parameters.
