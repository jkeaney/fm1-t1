# 7. Performance state and storage

## 7.1 [TEMP] snapshot **[V]**

Two whole patterns sit in RAM: `active_pattern` (what plays) and `snapshot_pattern` (the revert copy), 4,672 bytes each.

| Action | Code | Effect |
| --- | --- | --- |
| Enter (SAVE or LFO pressed) | `t1_temp_enter` | `snapshot = active` (copies the current pattern in), mode TEMP |
| Edit | any knob | changes `active` only |
| Revert (button released) | `t1_temp_revert` | `active = snapshot` (4,672 B memcpy), **invalidate cached Euclid masks**, mode NORMAL |
| Commit (OCT+ while held) | `t1_temp_commit` | `snapshot = active`, mode NORMAL (shows "KEPT") |

Notes:
- The guide says entering TEMP "keeps the snapshot intact"; the code actually **overwrites it with the current pattern** on entry (which is what makes the revert point correct).
- On the device the copy runs with interrupts off (`fm1_irq_off`) so the audio ISR never sees a half-copied pattern.
- Revert restores parameters only. It does **not** rewind step positions or the LFSR; playback continues from where it was, with the restored pattern.
- The reported revert time (`last_revert_duration_us`) is measured by the host clock; the design's "0.081 µs" and "< 6 µs" figures are targets and estimates. ⚠ Not measured on hardware.
- A second TEMP edit session after a commit uses the committed pattern as its new baseline (verified: commit then edit then revert returns to the committed values).

## 7.2 [GLOBAL] relative edit **[V]**

`t1_global_apply_delta(param, delta)` adds `delta` to one parameter of the **active cycle of each of the six tracks**, clamping each to its own legal range, so differences between tracks are kept.
Example from the guide: with 16 and 12 steps, `+4` gives 20 and 16.

Supported parameter ids (21): STEPS, PULSES, ROTATE, DIVISION, REPEATS, OFFSET, TIME, PACE, PITCH, SCALE, ROOT, VOICING, STYLE, PHRASE, RANGE, VELOCITY, SUSTAIN, PROBABILITY, DELAY, RANDOM_DEPTH, CHORD_SIZE.
Clamping notes: STEPS and PULSES keep `pulses ≤ steps` (steps only lowers pulses, never raises); ROOT wraps modulo 12; SUSTAIN to 255.
Mask-affecting params (STEPS, PULSES, ROTATE) invalidate the cached masks.

### Routing from the knobs (`t1_ui_on_encoder_turn`) **[V]**

| Page | Knob 1 | Knob 2 | Knob 3 | Knob 4 |
| --- | --- | --- | --- | --- |
| RHYTHM | STEPS | PULSES / ROTATE (shift) | DIVISION / SUSTAIN (shift) | **STEPS** (should be cycle) |
| RATCHET | REPEATS / OFFSET | TIME / PACE | VELOCITY (shift ignored) | PROBABILITY (shift ignored) |
| HARMONY | PITCH | ROOT / SCALE | VOICING / STYLE | PHRASE / RANGE |
| MOD/CC | **STEPS** | RANDOM_DEPTH | **STEPS** (DELAY with shift) | **STEPS** |

The bold entries are unmapped and fall through to the starting value `T1_PARAM_STEPS`: turning those knobs in GLOBAL mode changes the step count of all six tracks (finding F6; verified).

## 7.3 Flash layout (host model) **[V]**

| Item | Value |
| --- | --- |
| Flash region | 256 KB |
| Banks | 6, one every 32 KB (`0x8000`) |
| Patterns | 6 per bank at `bank + n × 4,672` |
| Bank allocation | 6 × 4,672 = 28,032 B (fits a 32 KB slot; 4,736 B spare) |
| Two banks per 64 KB sector | banks (0,1), (2,3), (4,5) |
| `t1_flash_verify_boundaries()` | returns true: each bank lies inside one 64 KB sector |

Save/load go through a `T1FlashDriver` of three callbacks (read, write, erase). `t1_flash_save_bank` calls `erase_sector(bank_offset)` once, then writes 28,032 bytes.
- Whether that erase also wipes the **neighbouring bank** depends on the driver's erase size. The test's simulated driver erases only the 32 KB around the offset, which matches the bank stride but **not** the 64 KB sector the header documents (F15). ⚠ A real 64 KB sector erase would destroy the bank sharing the sector.
- The "1.17 ms to stream 4,672 B at 32 MHz SPI" figure is raw transfer time (4,672 × 8 / 32 Mbit/s) and ignores command and setup overhead.

## 7.4 What is saved on the FM-1 **[V]**

- **Nothing of the T-1 pattern is saved.** The flash functions are never called, and the T-1 pattern is not part of Felucca's project or preset saves (searched `project.c`, `upreset.c`, `params.c`).
- The only persisted item is the on/off setting `SYSTEM > T1 SEQ` (`PREF_T1`), saved with the settings.
- Every boot starts from `t1_system_init()` plus `t1_default_pattern()`, so edits are **lost at power-off** (F12).

## 7.5 Invariants (tests)

- `sizeof(Pattern) = 4,672`, `sizeof(Track) = 776`, `sizeof(CycleParams) = 128` (static asserts).
- Revert after edit restores every byte; commit then revert restores the committed bytes.
- GLOBAL delta preserves the per-track difference unless a clamp is hit.
- **Missing:** a test that every GLOBAL knob route changes only the intended parameter (would have caught F6).
