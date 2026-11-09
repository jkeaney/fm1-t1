# 3. The note engine (`ArpHeart`)

**Input:** held notes (pitch, velocity), Note Order, Octave Mode, Octave Steps; and a "generate" bang from the step clock.
**Output:** the next pitch (Step Arp) or a chord (Chord Arp) per generate bang.

## 3.1 Held notes → pitch list **[V]**

`GetPitchList` keeps a store of currently held pitches **in the order they were pressed**. A note-on adds a pitch, a note-off removes it.
When all notes are released the store is cleared, so the *next* chord starts a new played order. With **Hold** on, note-offs are ignored
(latched) until Hold is turned off, which releases everything.

```
pitches = []                          # played order
on note_on(p):  if p not in pitches: pitches.append(p)
on note_off(p): if not hold: pitches.remove(p)
on all_released: pitches = []
```

## 3.2 Note Order (`NotesSort`) **[V]**

| Note Order | Sorted list |
| --- | --- |
| `Played` | pitches as pressed |
| `Up` | ascending (`zl sort 0`) |
| `Down` | descending (`zl sort -1`) |
| `Chord` | as played (the whole list is played together, see 3.5) |
| `Random` | a random permutation (`zl scramble`) |
| `Follow` | takes its order from **Octave Mode**: Up → ascending, Down → descending, UpDown → ascending ("mountain", 3.4), Random → permutation |

The sort runs when the held-note list or either mode changes, **not on every trigger**. So `Random` is a permutation that is fixed until the notes
(or mode) change. ⚠ confirm on the device.

## 3.3 Octave list (`GetOctaveOffset`, `OctaveSort`) **[V]**

```
offsets = [0, 12, 24, 36, 48, 60][0 : OctaveSteps+1]        # `zl slice`: first (steps+1) offsets
```
(`GetOctaveOffset` adds 1 to the control then slices `0 12 24 36 48 60`, so 1 octave step gives `[0]`... ⚠ the exact "+1" convention: a setting of 1 is the default and yields 1 octave of range.)

`OctaveSort` orders the offsets:

| Octave Mode | Order |
| --- | --- |
| `Up` | ascending |
| `Down` | descending |
| `UpDown` | ascending then back down without repeating the ends (`0 12 24 12`) |
| `Random` | permutation, re-shuffled when the notes stop (a new run starts) |

```
pingpong(sorted): return sorted + reverse(sorted)[1:-1]
```

## 3.4 Combining notes and octaves (`AddOctavesToNotes`) **[V]**

**Normal mode:** for each octave offset in order, append the whole sorted note list shifted by that offset.

```
list = []
for o in octave_order:
    for n in sorted_notes: list.append(n + o)
```
e.g. notes `[C, E, G]`, octaves `[0, 12]` → `C E G C+12 E+12 G+12`.

**Mountain mode** (only when Note Order = `Follow` **and** Octave Mode = `UpDown`): the up half of the octave list is built as above, then the same list is reversed,
the first and last items are chopped off, and the result is appended. The arp climbs through all notes across all octaves, then descends back as one continuous line without repeating the peak or the start.

```
up   = [n + o for o in octave_up_half for n in sorted_notes]
list = up + reverse(up)[1:-1]
```

## 3.5 Step Arp vs Chord Arp **[V]**

Selected by Note Order: `Chord` → Chord Arp, anything else → Step Arp.

**Step Arp:** one note per generate bang.
```
on_generate():  p = list[counter];  counter = (counter + 1) % len(list)
on list becoming non-empty (first key down): counter = 0
```

**Chord Arp:** the whole held chord is output at once, and the counter steps through octaves.
```
chord_size = len(list) / n_octaves
on_generate():  chord = list[counter*chord_size : (counter+1)*chord_size];  counter = (counter + 1) % n_octaves
```

## 3.6 Transpose **[V]**

After the note engine, the pitch is shifted by the current step's Transpose lane value when `TransposeOn` is set:

```
pitch' = pitch + transpose_lane[step] * TransposeOn
```
Applied to every pitch if a chord is returned.

## 3.7 Scale snap (`Scales`) **[V structure, ⚠ detail]**

When **Scale** is on, the device reads the current Live Set's `scale_intervals` and builds a 12-entry lookup: for each chromatic pitch class 0 to 11, the scale tone to use.
The table is built by walking the 12 semitones with a counter that advances each time a scale interval is reached and otherwise keeps the last scale tone, so a non-scale pitch class
maps to the **nearest scale tone at or below it** (⚠ inferred; confirm on the device).

```
table[pc] = largest scale_interval <= pc            # for pc in 0..11
pitch' = (pitch // 12) * 12 + table[pitch % 12]     # keeps the octave
```
The root note of Live's scale is not applied here (only the interval list is read). ⚠ confirm against a non-C root.
A "scales changed" notification re-flushes the output so hanging notes are released when the scale changes.

## 3.8 Invariants (tests)

- `Up` order is non-decreasing; `Down` is non-increasing.
- `Random` order is a permutation of the held notes (same multiset).
- With octave steps 1, the list equals the sorted notes.
- `UpDown` octave list for offsets `[0,12,24]` is `[0,12,24,12]`.
- Mountain list for notes `[a,b]`, octaves up-half `[0,12]` is `a b a+12 b+12 a b ...` (peak not repeated). ⚠ verify the exact list on the device.
- With Scale on, every output pitch class is in the scale.
