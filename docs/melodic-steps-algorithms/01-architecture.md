# 1. Architecture and data flow

## Stage diagram

```
 host transport / metro ─► Clock ─► tick ──► (all five lanes advance together)
   (speed, triplets, swing)                  ┌─ Octave    lane  length Lo ─► level 0..6 ─┐
                                             ├─ Transpose lane  length Lt ─► level 0..6 ─┤
                                             ├─ Length    lane  length Ll ─► level 0..6 ─┤──► buddy (wait for all 5)
                                             ├─ Velocity  lane  length Lv ─► level 0..6 ─┤
                                             └─ Chance    lane  length Lc ─► level 0..6 ─┘
                                                                                          │
   MIDI note-on (optional) ──► Root note                                                  ▼
   Scale menu ──► scale table                          pitch  = root + scale[Transpose] + 12 × (Octave − 3)
                                                       vel    = scale(Velocity, 0..6 → 15..120)
                                                       length = (98% of a 16th) / 4 × multiplier[Length]
                                                       play?  = random(100)+1 ≤ scale(Chance, 0..6 → 0..100)
                                                                                          │
                                                               note-on now, note-off after `length` ─► MIDI out
```

## Blocks **[V]**

| Block (patch name) | Role | Doc |
| --- | --- | --- |
| `Fader Octave / Transpose / Length / Velocity / Chance` (5 bpatchers) | one lane each: 16 faders, a counter, a length, edit buttons | [03](03-lanes.md) |
| `p Clock` | tempo-synced step clock with triplet and swing handling | [04](04-clock-and-timing.md) |
| `p Step Length` | reads the Live tempo and computes the duration of a 16th note | [04](04-clock-and-timing.md), [05](05-note-output.md) |
| `p resync` | restarts the lanes together when speed or triplet mode changes | [04](04-clock-and-timing.md) |
| `p scale mod` | scale table and the degree-to-semitone lookup | [05](05-note-output.md) |
| `p MIDI`, `p MIDI_msgs` | incoming MIDI (root note / start) and outgoing note messages with chance | [05](05-note-output.md) |
| `buddy 6` | waits until all five lane values have arrived for a step | [05](05-note-output.md) |
| `p value`, `p colors`, `p fadercolors`, mouse handling | UI and fader editing | not algorithmic |

## The central design: five lanes, one tick

```
tick n:   Octave[ n mod Lo ]   Transpose[ n mod Lt ]   Length[ n mod Ll ]   Velocity[ n mod Lv ]   Chance[ n mod Lc ]
```
Each lane keeps its own position counter (a `counter 1 16` whose maximum is that lane's length). They are all advanced by the same clock tick, and a combined note is formed
only after all five have produced a value (`buddy`). Because the lengths differ, the combination of values repeats only after `lcm(Lo, Lt, Ll, Lv, Lc)` ticks.

## Event flow for one step **[V structure]**

1. The clock fires; `---clock` sends a bang that triggers all five lanes in a fixed order (Octave, Transpose, Length, Velocity, Chance).
2. Each lane advances its counter and outputs its fader value (0 to 6).
3. `buddy` collects the five values.
4. Octave → semitones, Transpose → scale degree → semitones, Length → multiplier, Velocity → MIDI velocity, Chance → percent.
5. A random number is compared with the chance percent. If the note plays, note-on goes out, and a delay of the note length produces the note-off.
