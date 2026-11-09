# 1. Architecture and data flow

## Stages

```
 INPUTS                    GENERATE (on Gen / param change)             PLAY (every step)            OUTPUT
┌──────────────┐   ┌──────────────────────────────────────────┐   ┌─────────────────────────┐   ┌──────────────┐
│ Algo, Type,  │   │ Pitch lane  ──► scale snap ──► root      │   │ clock ► step counter    │   │ note events  │
│ Density,     │──►│ Octave lane                              │──►│ direction / length /    │──►│ (pitch, vel, │
│ Steps, Root, │   │ Gate lane                                │   │ offset ► jitter ► swing │   │ start, dur)  │
│ Scale, Accent│   │ Velocity lane (accent pattern + levels)  │   │ read lanes[position]    │   │ or clip dump │
└──────────────┘   └──────────────────────────────────────────┘   └─────────────────────────┘   └──────────────┘
        ▲                         ▲                                         ▲
        │                         │                                         │
   CakeWalk (random-walk drift of Density/Type dials)        user edits of individual steps override lanes
```

**[V]** The lanes are stored independently as 16-slot arrays. Playback reads all four lanes at
the same step position. Editing a step writes into the lane directly and does not trigger a
regenerate.

## The four lanes (pattern data model)

| Lane | Per-step value | Range | Notes |
| --- | --- | --- | --- |
| `pitch[16]` | pitch class | 0 to 11 | Stored twice: `pitch_orig` (as generated) and `pitch` (after scale snap). See [03](03-pitch-generation.md). |
| `octave[16]` | octave offset | -2 to +2 | Added as `12 × octave` semitones. |
| `gate[16]` | gate length code | 30, 70, 100, 127 (⚠ 100 only on step 1) | Scaled to a note duration at output. |
| `vel[16]` | velocity level | accent pattern, then mapped to MIDI velocity | See [04](04-rhythm-and-dynamics.md). |

Pattern length is 16 slots; the playing length is the **Steps** control (1 to 16). **Offset**
rotates all four lanes together.

## Generation trigger sequence

When **Gen** is pressed (and when density/type/algo change, ⚠) the lanes regenerate in this order:

1. Clear the pitch stores and reset the urn.
2. Run the selected generator (Classic or v2) to fill `pitch_orig[0..15]`.
3. Apply the **Type** replacement pass (repeated notes). See [03](03-pitch-generation.md).
4. Copy to `pitch`, then run scale snap if Scale is On.
5. Regenerate octave, gate and velocity lanes (each lane listens for the same "gen" message and is independent).
6. Fire a "done" notification so the UI and any clip writer can read the finished lanes.

Lanes do not depend on each other during generation, so they can be generated in any order.

## Randomness model

Three primitives cover everything:

| Primitive | Meaning | Used by |
| --- | --- | --- |
| `rand(n)` | uniform integer in `[0, n)` | coin flips, probability tests |
| `urn(n)` | draw from `0..n-1` without replacement; when empty, refill and reshuffle | pitch classes, accent positions, replacement indices |
| `bern(p)` | `rand(100) < p` → 0 or 1 | gate, octave, jitter tests |

For FM-1 use one seeded PRNG for all of them so patterns are reproducible; see
[07](07-rebuild-checklist.md).
