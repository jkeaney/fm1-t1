# 1. Architecture and data flow

## Stage diagram

```
 host transport (beats) ─► Sequencer ─────────────► "next step" bang (8 steps per 2-beat cycle, shuffled)
   (Clock Speed, Auto Retrigger)  ▲
                                  └── TimeShuffler: 8 per-step time thresholds (Shuffle 1..8, Global Shuffle, Shuffle Length)

 For each of 4 lanes (Lane 1..4), on every step bang:
   ┌ Chance layer ─┐
   │ Velocity layer│   five independent counters, each with its own length (2..16)
   │ Roll layer    │──► MakeNote(note, alt note, chance, velocity, roll, slide, delay)
   │ Slide layer   │         │
   │ Alt layer     │         ├─ roll chance:  level > random(6) ?
   └───────────────┘         ├─ pick main or alternate pad note
                             ├─ velocity = level scaled 10..127
                             ├─ roll: extra repeats at Roll Rate, each ×0.7 velocity
                             └─ delay (ms)
                                    ▼
                       MPE note-gen: CC74 (slide) then note-on on lane's channel; note-off after a 16th ─► MIDI out
```

## Blocks **[V]**

| Block (patch name) | Role | Doc |
| --- | --- | --- |
| `p Sequencer` | host beat position → step bangs, with per-step shuffle thresholds | [03](03-clock-and-shuffle.md) |
| `settings` (`TimeShuffler`) | Clock Speed, Shuffle 1 to 8, Shuffle Length, Global Shuffle → thresholds | [03](03-clock-and-shuffle.md) |
| `p Resync` | restarts the layer counters at the Auto Retrigger interval | [03](03-clock-and-shuffle.md) |
| `Lane` ×4 (embedded lane patcher) | one drum pad: 5 layer sequences + note maker | [04](04-lane-layers.md), [05](05-note-output.md) |
| `p PerTypeSequencer` | the five layer sequences of one lane | [04](04-lane-layers.md) |
| `p MakeNote`, `p Roll` | note assembly and repeats | [05](05-note-output.md) |
| `p MpeNoteGen` | MIDI output with slide CC and note-off | [05](05-note-output.md) |
| `p Setters` (`Random`, `NotSoRandom`, `Reset`) | fill / mutate / clear a layer | [04](04-lane-layers.md) |
| `p StepsStore` | holds and recalls the step data (saved as the preset blob) | [02](02-inputs.md) |
| `p PushToDevice`, `p DeviceToPush`, `p GetPush`, `p PushBanks`, … | Ableton Push control surface integration | not algorithmic |
| `p MainColors`, `p UiSliderPosition`, `manual` | UI | not algorithmic |

## The central design: lanes × layers

```
              step 1  2  3 … 16
Lane 1  Chance  [ ][ ][ ] …          length Lc1
        Velo    [ ][ ][ ] …          length Lv1
        Roll    [ ][ ][ ] …          length Lr1
        Slide   [ ][ ][ ] …          length Ls1
        Alt     [ ][ ][ ] …          length La1
Lane 2 …    (same five layers, own lengths)
Lane 3 …
Lane 4 …
```
Each cell holds a level 0 to 6. **Every layer has its own position counter** that advances once per step bang and wraps at that layer's length.
A lane's "current" values at any bang are the five layer values at their own counter positions.

## Event flow for one step **[V structure]**

1. The clock fires step bang `n` (one of the 8 shuffled positions in the 2-beat cycle).
2. Each lane advances its five counters and reads the five level values.
3. `MakeNote` rolls the Chance level; if it fails, nothing is played (the Slide value is not updated either).
4. On success it picks the pad note (main or alt), maps velocity, schedules any roll repeats, applies the lane delay.
5. The MPE note generator sends the slide value as CC74 and the note on the lane's MIDI channel, then the note-off a 16th later.
