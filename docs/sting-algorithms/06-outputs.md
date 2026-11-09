# 6. Outputs

Two output paths exist: live note events during playback, and a one-shot clip write ("Gen Clip").

## 6.1 Final pitch **[V]**

```
midi_note = pitch[i] + 36 + 12 * octave[i] + transpose
```
- `pitch[i]` is the (possibly scale-snapped) pitch class 0 to 11.
- Base of 36 puts the lowest pitch at C2 (MIDI 36) by default.
- Vector addition is element-wise over the whole 16-step list (a `vexpr` with scalar broadcast for transpose).

## 6.2 Note event fields

| Field | Source | Formula |
| --- | --- | --- |
| pitch | 6.1 | `midi_note` |
| start | step index and Division | `step_index × step_length_beats` (beats), plus any swing delay |
| duration | gate lane | `gate[i] / 127 × 0.25` beats (one 1/16 at 127). ⚠ plus a global Gate-dial scale |
| velocity | vel lane | `vel_high` if accented else `vel_low`, see [04](04-rhythm-and-dynamics.md) |
| mute | none | always 0 |

## 6.3 Clip generation ("Gen Clip") **[V structure]**

1. Find an empty clip slot (counts scenes and clips, checks for a free slot).
2. Create a clip of length from the table below, named `sting_<n>`.
3. Build a note dictionary with the fields above for all active steps and add them to the clip.
   - The note dictionary must **not** contain a `note_id`; the host assigns it.
   - `pitch` and `mute` are integers; the rest are floats.

**Clip length for 16 steps, by Division (beats)** **[V]**

| Division | 1/48 | 1/32 | 1/24 | 1/16 | 1/12 | 1/8 | 1/6 | 3/16 | 1/4 | 5/16 | 1/3 | 3/8 | 1/2 | 3/4 | 1 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Beats | 1.375 | 2 | 2.75 | 4 | 5.375 | 8 | 10.75 | 12 | 16 | 20 | 21.5 | 24 | 32 | 48 | 64 |

The exact values (`16 × step_length`) would be 1.333, 2, 2.667, 4, 5.333, 8, 10.667, 12, 16, 20, 21.333,
24, 32, 48, 64. The device rounds the triplet-type values up by about 1/24 beat (⚠ a deliberate loop-end
fudge). Use exact values on FM-1 unless you hit loop-end drift.

## 6.4 Per-step edit writes

Editing a step in the UI writes straight into the lane (`pitch_orig` and `pitch` for notes) without
regenerating. Notes edited while Scale is On are re-snapped.

## 6.5 What a rebuilt module must expose

```
struct StingPattern {
    uint8_t pitch_orig[16];   // 0..11
    uint8_t pitch[16];        // 0..11 after snap
    int8_t  octave[16];       // -2..+2
    uint8_t gate[16];         // 30/70/100/127
    uint8_t vel[16];          // MIDI velocity
    uint8_t length;           // 1..16
    int8_t  offset;
};
```
Memory is tiny (about 80 bytes per pattern). It fits comfortably in the 6×6×6×6 budget in [PLAN.md](../../PLAN.md).
