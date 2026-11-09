# 4. Clock and timing

**Input:** host tempo and transport; Speed menus; Triplets; Double/Half Time; Swing; Re-Sync.
**Output:** a tick per step (and the resync message).

## 4.1 Step interval **[V structure, ⚠ arithmetic]**

The step length is a note value chosen from two menus. A Triplets switch selects which menu is active:

| Straight (Triplets off) | `2n` | `4n` | `8n` | `16n` (default) | `32n` | `64n` |
| --- | --- | --- | --- | --- | --- | --- |
| Triplet (Triplets on) | `1nt` | `2nt` | `4nt` | `8nt` | `16nt` | `32nt` |

The menu index (0 to 5) is shared: **Double Time** adds 1 to it (twice as fast), **Half Time** subtracts 1, clamped to 0..5; switching Triplets keeps the same index so the speed stays comparable (a `16n` ↔ `8nt` swap).
The interval in milliseconds comes from the tempo and the selected note value: `ms = 60000 / BPM / (steps per beat)`. A triplet value is expected to be 2/3 of the matching straight value (a `8nt` at 120 BPM = 166.7 ms).
⚠ The patch also contains a `× 1.5` branch on the triplet path whose role I could not resolve, so measure a triplet setting before relying on that arithmetic.
The clock runs from the host transport (`plugsync~`) when it is playing. When the transport is not running, an internal metro on the selected value keeps the device audible (offline playback speed). ⚠ confirm the not-playing behaviour on the device.

## 4.2 Step length helper (`p Step Length`) **[V]**

The Live tempo is read through the Live API (`live_set` → `tempo`) and converted to the duration of a 16th note:

```
sixteenth_ms = 15000 / BPM              # e.g. 125 ms at 120 BPM
```
Two derived values leave this block: `0.98 × sixteenth_ms` (used for note length, [05](05-note-output.md)) and `10 × (15 / BPM)` (used as the unit of swing delay).

## 4.3 Swing **[V]**

Swing delays **every second step**. The Clock counts steps with a toggle; on the off-beat step the output is delayed:

```
delay_ms = Swing × (150 / BPM)          # Swing as the control's number; 150/BPM = sixteenth_ms / 100
```
So at Swing = 100 the off-beat step is pushed back by one full 16th note (125 ms at 120 BPM). At 0 it is straight.
The control's stored range is 0 to 127 (shown with a percent style), so the maximum may be 127, about 1.27 sixteenths. ⚠ confirm the range on the device.
Swing therefore delays the **note** and not the lane counters; the delay is applied to the outgoing step message. The shuffle is relative to the tempo, so it stays musical when the tempo changes.

## 4.4 Resync **[V structure]**

`p resync` watches the host clock position against the current speed's grid:

```
bar_position / grid_size  → watch for the position where the grid and the lanes align
when the speed menu or Triplets changes (or Re-Sync is pressed):
    wait until the next grid boundary (pipe 50 ms),
    send a resync message that resets every lane counter to step 1
```
The grid size used is one 8th for the straight speeds (`/ 0.125`) and one 6th for triplet speeds (`/ 0.166666`). ⚠ the exact boundary rule is inferred from those two constants.

## 4.5 MIDI-clocked mode **[V structure]**

With **MIDI** on, the transport is not used. Incoming MIDI notes drive the device:

```
note-on:   root = incoming note ;  start the clock (metro runs)
note-off:  stop the clock          # ddg.mono tracks the held note; held-state gates the metro
```
The mono tracker (`ddg.mono`) ensures only the most recent held key counts. The step metro runs at the same note-value interval as in synced mode. ⚠ confirm in the running device.

## 4.6 Per-tick order of operations

1. Clock fires (transport-locked or metro).
2. If the step is an off-beat step, delay it by the swing amount.
3. Lane counters advance (or reset if a resync message just arrived).
4. `buddy` collects the five lane values.
5. The note is built and sent ([05](05-note-output.md)).

## 4.7 Invariants (tests)

- At 120 BPM, `16n` → 125 ms between ticks; `8nt` → 166.7 ms.
- Swing 0 → no delay; Swing 100 → off-beat delayed by one sixteenth (125 ms at 120 BPM).
- Double Time halves the interval; Half Time doubles it.
- After a resync, all five lane positions are 1 on the same tick.
