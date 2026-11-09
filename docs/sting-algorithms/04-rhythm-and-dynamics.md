# 4. Rhythm and dynamics lanes

Three lanes, each regenerated on **Gen**, each independent. All use step position `i` (0 to 15) to
bias the odds, so later steps tend to differ from the first.

## 4.1 Octave lane **[V structure]**

**Output:** `octave[16]`, integer −2 to +2.

```
for i in 0..15:
    p     = 10 + 20 * (i / 16)              # percent: 10% at step 0 rising to ~29% at step 15
    a     = bern(p)                         # two independent coin flips
    b     = bern(p)
    dir   = rand(3) - 1                     # -1, 0 or +1
    octave[i] = dir * (a + b)               # 0, ±1 or ±2
```

- Later steps get more octave jumps. Most steps stay at 0.
- ⚠ Density is wired into this lane as a regenerate/length trigger. Its exact scaling of `p` (if any) is not confirmed.
- Output octave in semitones = `12 * octave[i]`.

## 4.2 Gate lane **[V structure, ⚠ meaning of values]**

**Output:** `gate[16]`, one of 30, 70, 100, 127.

```
gate[0]  = (rand(2) == 0) ? 100 : 70             # first step: coin flip
for i in 1..15:
    pa = lerp(i, 0..15 → 5..50)                   # linear, 5% → 50%
    pb = 10 + 30 * ((i/15) ^ 1.2)                 # scale 10..40 with exponent 1.2 (curved)
    s  = bern(pa) + bern(pb)                      # 0, 1 or 2
    gate[i] = [30, 70, 127][s]
```

- Short gates (30) dominate early steps; full-length (127) becomes steadily more likely later.
- ⚠ The exact step index that the two scale curves use (0 to 15 vs 1 to 15) is not confirmed.
- **Meaning of the values:** at output the code scales 0..127 to 0..0.25 beats: `duration = gate/127 * 0.25`
  (one 16th note at 127). So 30 ≈ 24% of a 16th, 70 ≈ 55%, 100 ≈ 79%, 127 = full step. The top-level
  **Gate** dial (default 100) is a further global scale. ⚠ Exact role not confirmed.

## 4.3 Velocity lane and accent **[V structure, ⚠ final mapping]**

Two parts: *which steps are accented*, then *how loud accented and normal steps are*.

**Accent pattern**
```
accent[0..15] = 1                       # 1 = normal, 2 = accented
for each of K draws from urn16:         # unique step positions, no repeats
    accent[position] = 2
```
- `K` comes from the Density control (⚠). One step gets a coin flip (`1 + rand(2)`) for its level.
- The unique-draw approach means accents never double-hit the same step.

**Bipolar Accent control** (−1 to +1, shown 0 to 100, default 50 = neutral ⚠)
```
a = accent_control                       # -1 .. +1
if a >= 0:   scale a (0..1) onto 100 → 127     # accented steps get louder
if a <  0:   scale -a (0..1) onto 100 → 40     # normal steps get quieter
```
- It publishes two velocities: `vel_high` (for accented steps) and `vel_low` (for normal steps).
- Endpoints verified: 100→127 and 100→40. ⚠ Which end maps to which role and the neutral point are not fully
  confirmed (the graph duplicates the scaling objects for the positive and negative branches).
- Then `velocity[i] = accent[i] == 2 ? vel_high : vel_low`.

## 4.4 Density **[I]**

A 0 to 16 value, default 14. Feeds the octave, gate and velocity generators as both a regenerate trigger
and (⚠) a quantity: likely the count of active/accented steps. If rebuilding, a safe interpretation is
"number of steps (out of 16) that play". Verify on the device by sweeping Density with Gen held.

## Invariant checks
- `octave[i] ∈ {-2,-1,0,1,2}`, `gate[i] ∈ {30,70,100,127}`.
- Over many generations, the mean number of non-zero octave steps grows with step index.
- No step is accented twice by the K-draw (K distinct positions).
