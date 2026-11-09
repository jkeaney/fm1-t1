# 1. Architecture and data flow

## Stage diagram

```
 MIDI in (held keys) ─► Hold ─► NotesHeld flag
                         │
                         ▼
   ┌───────────────── ArpHeart (note engine) ──────────────────┐
   │ GetPitchList ─► NotesSort ─► AddOctavesToNotes ─► StepArp │──► one note per "generate" bang
   │ (held, in played order)  (Note Order)  (Octave Mode,     │    (or ChordArp: the whole chord)
   │                                         Octave Steps)     │
   └─────────────────────────────▲─────────────────────────────┘
                                 │ "generate a new note" bang
 transport / BPM ─► Triggers ─► MasterClock ─► Steps (on/off) ──┘            step number s ─► per-step lanes
  (phasor, division, swing)     (step 1..LoopLength, direction)                     Velo · Length · Chance · Ratchet · Trans · Map/CC

   note ─► Transpose (+ lane value) ─► Scales (snap) ─► Ratchet (repeats) ─► Chance (probability) ─► makenote (velocity, length) ─► noteout
```

## Blocks **[V]**

| Block | Role | Doc |
| --- | --- | --- |
| `p Hold`, `p NotesHeld`, `p Retrig`, `p Lock` | latch held notes, held flag, key retrigger, transport lock | [03](03-note-engine.md), [05](05-timing.md) |
| `p ArpHeart` (+ `GetPitchList`, `NotesSort`, `OctaveSort`, `GetOctaveOffset`, `AddOctavesToNotes`, `StepArp`, `ChordArp`) | note engine | [03](03-note-engine.md) |
| `p Triggers`, `p TimeDiv`, `p Tempo`, `p MasterClock` | the clock and the step counter | [05](05-timing.md) |
| `Steps1` | 16 step on/off switches and the loop length | [04](04-step-lanes.md) |
| `Velo`, `Length`, `Chance`, `Ratchet`, `Trans`, `Mapp` (the shared lane widget) | per-step value lanes | [04](04-step-lanes.md) |
| `p Transpose`, `p Scales` | pitch post-processing | [03](03-note-engine.md) |
| `p Ratchet`, `p Chance`, `p Length` | note-level effects | [04](04-step-lanes.md) |
| `makenote`, `noteout`, `flush` | output with automatic note-off | [06](06-outputs-and-rebuild.md) |
| `p UiManagement`, `p PushBanks`, `p Lametta`, `p CustomTab` | UI, Push, tab handling | not algorithmic |

## Event flow for one step **[V]**

1. The clock fires; `MasterClock` advances the step number `s` (1 to LoopLength) and sends it to `Steps1` and every lane.
2. If step `s` is **on**, a "generate" bang is sent to `ArpHeart`.
3. `ArpHeart` returns the next pitch (Step Arp) or the whole chord (Chord Arp).
4. Pitch + the step's **transpose** value → **scale snap** → **ratchet** (1 to 4 hits within the step) → each hit passes the **chance** test.
5. A surviving hit becomes a MIDI note with the step's **velocity** and **length**, then a note-off.
6. In parallel, the step's **CC/Map** value is sent on the chosen CC and to any mapped Live parameters.
