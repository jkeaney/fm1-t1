# 5. Making a note (`MakeNote`, `Roll`, `MpeNoteGen`)

**Input:** the five layer values for this lane and step, the lane's pad notes, Delay, Mute and Roll Rate.
**Output:** MIDI notes (and a CC74) on the lane's channel.

## 5.1 Chance: does the step play? **[V]**

```
r = random(6)                         # 0..5
plays = chance_level > r
```
| Chance level | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| P(plays) | 0 | 1/6 | 2/6 | 3/6 | 4/6 | 5/6 | 1 |
| % | 0 | 17 | 33 | 50 | 67 | 83 | 100 |

A second output tells the UI whether this step "will trigger". The slide value is only updated when a trigger happens. **[V]** (the patch comment says so)

## 5.2 Main or alternate pad note **[V]**

```
r = random(6)
note = alt_note if alt_level > r else main_note        # same 0..6 level scale as Chance
```
The two note numbers are stored in a two-item list and the comparison result (0 or 1) indexes it.

## 5.3 Velocity **[V]**

```
velocity = scale(velo_level, 0..6 → 10..127)            # linear
```
| Level | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Velocity | 10 | 29 | 49 | 68 | 88 | 107 | 127 |

Level 4 = 88 is the default (Reset sets it).

## 5.4 Slide **[V]**

```
slide = scale(slide_level, 0..6 → 0..127)
```
Sent as **CC74** (MPE slide / timbre) just before the note-on, so each hit can carry a different timbre value.

## 5.5 Roll (repeats) **[V structure, ⚠ count]**

When a roll level `n` (0 to 4) is present, a metronome tied to **Roll Rate** fires extra hits of the same note after the first one. Each repeat is **70% of the previous velocity**:

```
v0 = velocity
for k in 1..n:  wait roll_interval;  v_k = v_(k-1) * 0.7;  play note at v_k
```
- The roll interval comes from Roll Rate (`1/8.` … `1/32t`, 9 values); it is a note value in the host tempo.
- ⚠ Whether level `n` gives `n` extra hits or `n+1` total depends on the order two connected branches fire; verify by recording a roll level 2 hit.
- Rolls are interrupted when a new hit starts on the same lane. ⚠ (retriggers the same metro)

## 5.6 Delay and Mute **[V]**

- **Delay n** (0 to 50 ms) holds each hit for that long (a `pipe`), letting you push a pad behind the grid for a "laid back" feel (e.g. 4.7 ms on the pedal hat in *Dembow Rhythm*).
- **Mute n** silences the lane's output.

## 5.7 MPE note generator **[V]**

```
on note (pitch, velocity, slide):
    channel = lane_number + 1                  # lanes 1..4 → MIDI channels 2..5
    send CC74 = slide on channel
    send note-on(pitch, velocity) on channel
    after 1/16 note: send note-off              # `del 16n` into a flush
```
- **Note length is fixed at a 16th note** (host-synced). There is no gate-length control.
- Each lane has its own channel, so the CC74 slide of one lane does not affect the others.
- The `flush` object guarantees hanging notes are released at the next event or when the device is disabled.

## 5.8 Order of processing for one hit

1. Read five layer values for the lane.
2. Roll `plays` from Chance; stop if false.
3. Pick note (main/alt) by Alt level.
4. Velocity from Velo level; slide from Slide level.
5. Apply Delay.
6. Send CC74, note-on; schedule roll repeats; schedule note-off after a 16th.

## 5.9 Invariants (tests)

- Chance 6 always plays; Chance 0 never plays; Chance 3 plays about half the time.
- Velocity table above; level 4 → 88.
- Alt 0 never alt; Alt 6 always alt.
- Every note-on is followed by exactly one note-off after a 16th.
- Roll repeats have decreasing velocity (×0.7 each).
- A muted lane produces no notes.
