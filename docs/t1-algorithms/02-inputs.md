# 2. Inputs: data model and controls

## 2.1 The cycle parameter block (128 bytes) **[V]**

`Pattern → Track[6] → CycleParams[6]`. The status column says whether the engine really reads the field.

| Field | Range | Used by | Status |
| --- | --- | --- | --- |
| `steps` | 1 to 64 (0 → 16) | step counter, Euclid | **live** |
| `pulses` | 0 to steps | Euclid | **live** |
| `rotate` | −64 to +64 | Euclid | **live** |
| `division` | 0 to 10 (11 values) | step length | **live** |
| `repeats` | 1 to 16 | ratchet | **live** |
| `offset` | −64 to +63 | ratchet velocity slope | **live** |
| `time` | 1 to 127 (0 → 64) | ratchet gate density | **live** |
| `pace` | −64 to +63 | ratchet timing warp | **live** |
| `pitch_offset` | −48 to +48 | pitch | **live** |
| `scale_id` | 0 to 15 | quantise, chord | **live** (masks and order fixed during the review) |
| `root_note` | 0 to 11 | quantise, chord | **live** |
| `voicing` | 0 to 11 | chord voicing | **live** |
| `style` | 0 to 7 | arpeggio index | **live** (7 = same as 0, see F5) |
| `phrase` | 0 to 7 | pitch contour | **live** (5, 6 broken, see F3) |
| `range` | 0 to 127 | contour size in semitones | **live** (was × 7 before the F4 fix) |
| `chord_size` (same byte as `reserved_pitch`) | 1 to 4 (0 → 1) | chord | **live** |
| `velocity` | 1 to 127 (0 → 100) | note velocity | **live** |
| `probability` | 0 to 100 | trigger chance | **live** |
| `sustain` | 1 to 255 (%) | gate scale | **live** |
| `accent` | 0 to 127 | velocity boost | **live** |
| `delay_ticks` | −12 to +12 | note onset nudge | **live** (negative inert on a first hit, see F11) |
| `random_depth` | 0 to 127 | LFSR modulation size | **live** |
| `random_rate` | 0 to 15 | LFSR loop length | **live at PLAY only** |
| `random_targets` | bitmask | which params the LFSR touches | **partly live**: bits 0, 1, 3, 6 only |
| `cc_target`, `cc_value` | 0 to 127 | MIDI CC on each pulse | **live** |
| `active_mask_lo/hi` | 32 + 32 bits | forced extra pulses | **live** (UI edits the low 32 only) |
| `groove_id` | 0 to 15 | nothing | **inert** (UI only) |
| `reserved_flags` | | nothing | unused |
| `p_lock_data[88]` | 11 × 8 B | per-step overrides | **live** for 3 parameters |

Pattern level: `bpm_x10` (host init only), `swing` (50 to 75, **live but no UI**), `master_scale` / `master_root` (**live in the engine, no UI sets `master_scale`**), `master_length_override` (**inert**).
Track level: `active_cycle` (live), `muted` (live), `midi_channel` (live), `soloed` and `cycle_count` (**unused**).

## 2.2 Controls on the FM-1 **[V]** (`ui_t1.c`, `torso_t1_ui.c`)

| Control | Action |
| --- | --- |
| Knobs 1 to 4 | the page's four parameters; **OCT+ held** = the second parameter of each knob |
| SEQ / OCT− | next / previous page: RHYTHM, RATCHET, HARMONY, MOD/CC |
| GLO held | knob turns apply a relative delta to **all six tracks** |
| SAVE or LFO held | [TEMP]: edits undone on release; OCT+ while held keeps them |
| SELECT | tempo (BPM) |
| PLAY | start / stop |
| white keys 1 to 6 | select track; white keys 7 to 12 select that track's cycle |
| black keys 1 to 6 | mute / unmute track |

### Page / knob map (normal turn → shift turn)

| Page | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
| --- | --- | --- | --- | --- |
| 1 RHYTHM | STEPS → MAX LEN* | PULSES → ROTATE | DIVISION → GATE LEN (sustain, ×5 per detent) | CYCLE → STEP MOD (toggles one forced pulse) |
| 2 RATCHET | REPEATS → OFFSET | TIME (×2 per detent) → PACE | VELOCITY → ACCENT | PROB (×2 per detent) → GROOVE* |
| 3 HARMONY | PITCH → M-TRANS (master root) | ROOT → SCALE | VOICING → STYLE | PHRASE → RANGE |
| 4 MOD/CC | RND TGT (toggle one bit) → CHANNEL | RND DEPTH → RND RATE | CC NUM → SLEW/DEL (delay) | CC VALUE → VOICES (chord size) |

\* inert in the engine. Detent step sizes: most knobs move 1 per detent, TIME and PROB move 2, GATE LEN moves 5.

### In [GLOBAL] mode **[V]**

Each knob is routed to a parameter id, then `t1_global_apply_delta(pid, steps)` adds the delta to the **active cycle of every track**. Several knobs have no mapping and fall through to the initial id, **STEPS**
(finding F6). The page-by-page routing is in [07](07-state-and-storage.md).

### Parameter lock editing **[V]**

`t1_ui_on_encoder_turn` supports locking only **pitch** (HARMONY knob 1), **velocity** (RATCHET knob 3) and **probability** (RATCHET knob 4) on a step, when `edit_step != 0xFF`.
**Nothing on the device ever sets `edit_step`** (`t1_ui_set_edit_step` has no caller in `ui_t1.c`), so on the FM-1 these locks cannot be created or edited (finding F12).

## 2.3 Defaults **[V]**

`t1_system_init`: 120.0 BPM, swing 50 (straight), every track: 6 cycles of 16 steps, 0 pulses, division 4 (1/16), 1 repeat, velocity 100, probability 100, sustain 50, p-lock arena filled with `0xFF`, MIDI channel = track index.
`t1_default_pattern` then makes track 2 (index 1) a generative bass: E(7,16), pitch offset −27 (A1), root A, `scale_id` 10, TRI contour, range 12, time 127, sustain 90, velocity 80, accent 40,
probability 95, random targets PITCH + ACCENT, depth 24, random rate 3. Track 1 stays silent (0 pulses).
