# 6. Rebuild checklist, tests and open questions

## 6.1 Data model

```c
typedef struct {
    uint8_t chance[16], velo[16], roll[16], slide[16], alt[16];   // levels 0..6
    uint8_t len_chance, len_velo, len_roll, len_slide, len_alt;    // 2..16
    uint8_t global_len;      // 2..16
    uint8_t lock;            // link layers to global_len
    uint8_t pad_note, alt_note;   // 36..51
    uint8_t delay_ms;        // 0..50
    uint8_t mute;
} Lane;

typedef struct {
    Lane    lane[4];
    uint8_t clock_speed;     // 0..5
    uint8_t shuffle[8];      // stored as -50..50 (shuffle[0] 0..50)
    uint8_t shuffle_len;     // 2..8
    uint8_t shuffle_amount;  // 0..100
    uint8_t roll_rate;       // 0..8
    uint8_t auto_retrig;     // 0/1
    uint8_t auto_rate;       // 0..3 bars
} RhythmicStepsPattern;
```
Per pattern: 4 × (80 + 8) + about 14 ≈ 370 bytes. Comfortably inside the 6×6×6×6 budget in [PLAN.md](../../PLAN.md) (4.5 KB per pattern).

## 6.2 Build order

1. Layer sequencer with length and wrap ([04 §4.1](04-lane-layers.md)); test the period property.
2. Five layers per lane, with Lock ([04 §4.2](04-lane-layers.md)).
3. Note maker: chance, alt, velocity ([05](05-note-output.md)).
4. Setters: Random, Not-so-random, Reset ([04 §4.4](04-lane-layers.md)).
5. Step grid with 8 thresholds, then shuffle ([03](03-clock-and-shuffle.md)).
6. Delay, note-off, roll.
7. Auto retrigger.
8. MPE channel per lane and CC74 (only if FM-1 has a use for them).
9. Load the factory presets as test data ([07](07-presets.md)).

## 6.3 Invariant tests

| Test | Assertion |
| --- | --- |
| Layer period | `lcm` of layer lengths (unlocked) |
| Locked lane | all layers wrap at `global_len` |
| Chance extremes | 6 always, 0 never |
| Chance mean | level `n` → n/6 of hits over many steps |
| Velocity table | levels 0..6 → 10, 29, 49, 68, 88, 107, 127 |
| Not-so-random | per-cell change ∈ {−1,0,+1}, results in 0..6 |
| Reset | velo=4, others=0 |
| Shuffle window | displacement ≤ 0.1 beat; 0 at Global Shuffle 0 |
| Straight grid | thresholds 0, 0.25, … 1.75 beats |
| Retrigger | after the interval, all layer counters are at position 1 |
| **Preset golden** | decode a factory preset and verify all cell values are in 0..6 and lengths in 2..16 (all 19 pass) |

## 6.4 Open questions to confirm on the running device

| # | Question | How to check |
| --- | --- | --- |
| 1 | Do unlocked layers ignore `Global Length`? What does Global Length do when Lock is off? | Set Global Length 5 with Lock off and watch the playhead. |
| 2 | Roll: `n` or `n+1` total hits? | Record a clip with roll level 1, 2, 3. |
| 3 | Clock Speed label-to-factor mapping (66% vs 75%) | Measure step rate at each setting. |
| 4 | Shuffle Length semantics (repeat vs zero) | Set length 3 with distinct shuffle values and measure. |
| 5 | Behaviour when shuffled thresholds cross | Step 2 late +50, step 3 early −50. |
| 6 | Does Mute stop the layer counters? | Mute, unmute, check phase. |
| 7 | Are rolls cut off by a new hit on the same lane? | Roll 4 on consecutive steps. |
| 8 | Alt layer: is the Alt note chosen by the same random draw as Chance? | Statistical check over many hits. |
| 9 | Does Auto Retrigger also reset the shuffle position? | Observe at the retrigger bar. |

## 6.5 FM-1 porting notes

- The five-layer design maps well onto FM-1's per-step parameter locks: each layer is one `uint8_t[16]` with its own wrap length. The engine is five counters per lane and a tiny note maker.
- Integer levels 0..6 can be stored in 3 bits, so a whole lane fits in 40 bytes.
- Use a seeded PRNG for `random(6)`/`random(7)` so patterns are reproducible; keep a separate stream per lane.
- Decide whether FM-1 keeps the **fixed 16th note length** or exposes a gate length (the original has none).
- Treat the factory presets as a ready-made test corpus: they exercise lock, polymeter, roll, alt and shuffle.
