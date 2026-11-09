# 6. Outputs, processing order, rebuild checklist, tests and open questions

## 6.1 Output **[V]**

| Output | Source |
| --- | --- |
| Note-on | pitch (engine → transpose → scale), velocity (Velo lane) |
| Note-off | `makenote` after the Length-lane duration, via a `flush` object (also releases hanging notes when the scale changes) |
| CC | Map lane value on the chosen CC (`ctlout`) |
| Live parameter writes | Map lane to up to 8 mapped parameters |
| MIDI thru | incoming non-note MIDI passed to `midiout` |

## 6.2 Order of processing for one step

1. Clock trigger (swing applied) → step counter → step number `s`.
2. If step `s` is on → ask the note engine for the next note (or chord).
3. `+ Trans[s]` (if on).
4. Scale snap (if on).
5. Ratchet: `n = Ratchet[s]` repeats (if on).
6. Chance roll per hit (if on).
7. `makenote(pitch, Velo[s], Length[s]/127 × step_period)`.
8. CC / Map value for step `s`.

## 6.3 Data model

```c
typedef struct {
    // note engine
    uint8_t  order;          // 0..5  Played Up Down Chord Random Follow
    uint8_t  octave_mode;    // 0..3  Up Down UpDown Random
    uint8_t  octave_steps;   // 1..6
    uint8_t  hold, scale_on;
    // clock
    uint8_t  div;            // 0..6 (1/1 .. 1/64)
    uint8_t  dotted, triplet;
    uint8_t  swing;          // 0..127
    uint8_t  direction;      // 0..3
    uint8_t  loop_len;       // 1..16
    // lanes: 16 values each
    uint16_t step_on;        // bitmask
    uint8_t  vel[16], len[16], chance[16], ratchet[16], data[16];
    int8_t   trans[16];
    uint8_t  chance_on, ratchet_on, trans_on, cc;   // cc 0 = off
} StepArpPattern;
```
About 100 bytes per pattern.

## 6.4 Build order

1. Held-note list with played order and hold ([03 §3.1](03-note-engine.md)).
2. `NotesSort` + octave list + combination, with mountain mode ([03 §3.2 to 3.4](03-note-engine.md)).
3. Step Arp counter, then Chord Arp ([03 §3.5](03-note-engine.md)).
4. Step counter with direction ([05 §5.4](05-timing.md)); step on/off.
5. Lanes: velocity, length ([04](04-step-lanes.md)).
6. Transpose then scale snap.
7. Chance, then ratchet; test ordering (ratchet before chance).
8. Swing.
9. CC/Map lane.
10. Key retrig and transport lock.

## 6.5 Invariant tests

| Test | Assertion |
| --- | --- |
| Sort | `Up`/`Down` monotonic; `Random` is a permutation |
| Octave expansion | length = notes × octaves; each octave block = notes + offset |
| Pingpong | offsets `[0,12,24]` → `[0,12,24,12]` |
| Step arp wrap | with list length n, output cycles with period n |
| Chord arp | each trigger outputs `chord_size` notes; period = octave count |
| Off step | no output, counter unchanged |
| Chance | long-run rate ≈ value/100 |
| Ratchet before chance | with chance 50% and ratchet 3, mean hits ≈ 2 per step (4 × 0.5) |
| Length | note duration = len/127 × step period |
| Scale | with Scale on, output pitch classes ∈ scale |
| Direction | Backwards L=4: 4 3 2 1 |

## 6.6 Open questions to confirm on the running device

| # | Question | How to check |
| --- | --- | --- |
| 1 | Does `Random` note order reshuffle each trigger or only when notes change? | Hold a chord and listen over several cycles. |
| 2 | Exact octave count for Octave Steps = 1, 2, 3 | Hold one note and read the pitches. |
| 3 | Mountain list at the peak and the start: repeated or not? | Follow + UpDown with 2 notes, 3 octaves. |
| 4 | Scale snap direction (down vs nearest) and root handling | Scale on with non-scale notes and with a non-C root. |
| 5 | Ratchet count (n or n+1 hits) and the Ratchet-off path | Record a clip with ratchet 0, 1, 3. |
| 6 | Alternate and Random StepDirection details | Watch the step LEDs. |
| 7 | Swing mapping | Record at swing 0, 0.5, 1 and measure the off-beat position. |
| 8 | Does the Map lane keep sending for off steps? | Turn a step off and monitor the CC. |
| 9 | Octave Mode default | Open a fresh instance. |

## 6.7 FM-1 porting notes

- Every lane is `uint8_t[16]` read by a single step index, which matches FM-1's per-step parameter locks. The note engine is a small list builder; recompute it only when held notes or modes change.
- Keep **ratchet-then-chance** ordering so ratchet density can be thinned by chance.
- The phasor/edge clock is a Max implementation detail; on FM-1 use two tick offsets per 2-step cycle: `t0 = 0` and `t1 = 0.5 + swing×0.249` of the cycle.
- Hold and the key-retrig reset are state-machine rules, independent of the arp maths.
