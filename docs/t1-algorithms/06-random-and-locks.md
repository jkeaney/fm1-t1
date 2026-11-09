# 6. Randomness, probability, accent and parameter locks

## 6.1 The Galois LFSR **[V]**

A 16-bit Galois linear-feedback shift register, polynomial x¹⁶ + x¹⁴ + x¹³ + x¹¹ + 1 (tap mask `0xB400`):

```
lsb = state & 1;  state >>= 1;  if (lsb) state ^= 0xB400;  return state
```
- **Full period, verified:** from seed 0x1234 the state returns to the start after exactly **65,535** steps.
- **One LFSR per track**, seeded `0x1234 + 0x1111 × track`, and reseeded at every PLAY (`t1_lfsr_reseed`).
- It is **stepped only when something draws from it**, so the number and order of draws per pulse matter for reproducibility (§6.2).

### Looping the random line **[V]**

```
at each pass of the cycle (step_index wraps to 0):  counter++ ;  if counter >= loop_period: counter = 0, state = seed
```
`loop_period` comes from the cycle's `random_rate` **at PLAY** (`t1_lfsr_init`):

| random_rate | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 … |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| loop (passes) | 1 | 2 | 3 | 4 | 6 | 6 | 1 | 2 | 3 … (`rate mod 6 + 1`) |

Verified with a 50% probability pattern of 16 pulses: with rate 0 every bar is identical (`xx.x.x...x.xx..x`); with rate 3 bars 1 to 4 differ and bars 5 to 8 repeat them exactly.
Two consequences: changing `random_rate` while playing has no effect until the next PLAY, and the rate is read from the cycle that is active at PLAY. The RND RATE screen shows `/N` with N = `random_rate` (0 shows `/1`), which is **not** the loop length for N ≥ 3 (rate 3 shows `/3` but loops over 4 passes).

## 6.2 Probability **[V]**

```
prob = p-lock(PROB) if present else cycle.probability
if prob < 100:  roll = lfsr_next() mod 100 ;  fire only if roll < prob
```
The roll is the **first** draw for a pulse. Because the LFSR loops, a given pass skips the same pulses every time it repeats (see above).

## 6.3 Random modulation of parameters **[V]**

`t1_lfsr_modulate(targets, flag, base, depth, min, max)`: if `depth > 0` and the target bit is set, draw one value and return

```
clamp(base + (rnd mod (depth + 1)) - depth/2, min, max)
```
That is a roughly uniform offset in **[−depth/2, +depth/2]**; for odd depth it is slightly asymmetric: measured **−60 to +64 at depth 127**, −12 to +12 at depth 24. At depth 127 a pitch can move by ±63 semitones.

Targets the pipeline actually reads (bit mask `random_targets`):

| Bit | Name | Read by the pipeline? |
| --- | --- | --- |
| 0 | VELOCITY | yes (after the p-lock) |
| 1 | PITCH | yes (before the scale snap) |
| 2 | SUSTAIN | **no** |
| 3 | ACCENT | yes (chance of accenting a pulse) |
| 4 | DELAY | **no** |
| 5 | REPEATS | **no** |
| 6 | CC | yes (CC value) |
| 7 | VOICING | **no** |

Draw order for one fired pulse: probability roll (1 step) → pitch (1) → velocity (1) → accent (8, via `t1_lfsr_byte`) → CC (1). Draws are made only when the corresponding condition holds.

**Caveat (F19):** because `t1_lfsr_next` moves one bit per call, the pitch, velocity and CC draws are not independent. Measured: `rnd2 mod 128 == (rnd1 mod 256) >> 1` in 20,000 of 20,000 consecutive pairs, and pitch/velocity jitter at depth 127 correlate at 0.50. Only the accent draw was given fresh bits.

## 6.4 Velocity and accent **[V]** (current tree)

```
vel = p-lock(VEL) or cycle.velocity (0 → 100)
vel = modulate(VELOCITY, vel, depth, 1..127)
if accent > 0:
    accented = (targets & ACCENT) ? t1_lfsr_byte() mod 10 < 3         # about 3 pulses in 10, independent of random_depth
                                   : step_index == 0                    # otherwise only the cycle's first step
    if accented: vel = min(vel + accent, 127)
```
- **Changed during the review:** the accent chance used to be `(lfsr mod 128) < random_depth`. It now uses `t1_lfsr_byte` (8 fresh LFSR bits) and a fixed 30% chance, so it no longer scales with RND DEPTH. The new helper's own comment explains why: consecutive `t1_lfsr_next` values are nearly the same number (finding F19).
- Without the ACCENT target an accent is applied **only on step 0 of the cycle** (a downbeat), not on "accented pulses" in general.
- The accent draw consumes **8** LFSR steps, so enabling the ACCENT target changes every later random value in the pass.

## 6.5 Parameter locks **[V]**

An 88-byte arena per cycle holds **11 entries of 8 bytes**: `{step_idx, param_id, value:int16, flags, 3 reserved}`. `step_idx = 0xFF` marks an empty slot.

| Operation | Behaviour |
| --- | --- |
| `set(step, param, value)` | update an existing (step, param) entry, else take the first free slot; **returns false when all 11 are used** |
| `get(step, param)` | linear scan of the 11 entries |
| `clear(step, param)` / `clear_all` | mark entries empty |

Parameter ids are bare numbers shared between the UI and the pipeline: **57 = pitch, 67 = velocity, 68 = probability**. Only these three are read by the engine. `flags` is never used.
Notes:
- `t1_system_init` fills every arena with `0xFF`. A **zero-filled** arena looks full (every entry is step 0 / param 0), so new locks are refused (verified), and (0, 0) reads as locked; this matters only for patterns not created by `t1_system_init`.
- The device UI cannot create locks (no caller sets the edit step) and cannot remove them (no UI calls `clear`): finding F12.

## 6.6 Invariants (tests)

- LFSR never reaches state 0 and has period 65,535.
- With `random_rate = r`, the output over `p` passes repeats exactly every `loop(r)` passes.
- `probability = 100` consumes no draw and always fires; `0` never fires.
- A pulse's LFSR steps equal the enabled single-draw targets (pitch, velocity, CC) plus 1 if `probability < 100` plus 8 if the ACCENT target is on with `accent > 0`.
- Arena capacity is 11; the 12th distinct lock is refused.
