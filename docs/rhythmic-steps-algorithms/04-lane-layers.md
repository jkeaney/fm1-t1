# 4. Lane layers (the core algorithm)

**Input:** a step bang per lane; per-layer length; the layer's 16 stored levels.
**Output:** the current value of each of the five layers.

## 4.1 One layer sequencer **[V]**

Every layer is the same small machine: a counter that runs 1 to *length*, and a list of 16 stored values read at that counter position.

```
class Layer:
    values[1..16]       # levels 0..6
    length              # 2..16
    pos = 0

    on_step():
        pos = pos % length + 1        # 1..length (a `counter 1 16` whose max is set to the layer length)
        return values[pos]

    on_resync(): pos = 0              # next step reads values[1]
```
Cells beyond `length` are stored but ignored (they come back if the length is raised).

## 4.2 Five layers per lane, each with its own length **[V]**

| Layer | Meaning of the level (0 to 6) | Length parameter |
| --- | --- | --- |
| **Chance** | probability the step plays | `Length Chance n` |
| **Velo** | loudness | `Length Velocity n` |
| **Roll** | number of quick repeats | `Length Roll n` |
| **Slide** | MPE slide (CC74) value | `Length Mod n` |
| **Alt** | probability of the alternate drum | `Length Alternate n` |

**Lock Length n:** when on, all five layers of lane *n* use `Global Length n` instead of their own lengths. ⚠ The device keeps both sets of values; which takes priority when
lock is off is inferred from the presets (a locked lane has its layer-length parameters left at 16).

## 4.3 Polymeter through layer length **[V]**

Because each layer wraps at its own length, the combination of layers repeats only after the **least common multiple** of the five lengths:

```
period(lane) = lcm(len_chance, len_velo, len_roll, len_slide, len_alt)
```
Examples from the factory presets: lengths 16, 13, 3 → period 624 steps; 16, 11, 10 → 880; 16, 15, 14 → 1680. A lane with 16/16/16/16/16 loops every 16 steps. With Auto Retrigger the
long periods are cut off at the retrigger interval.

This is the main creative mechanism: a short **Velo** layer of length 3 gives an accent cycle that slides across a 16-step **Chance** pattern, so the same rhythm gets a different accent on every pass.

## 4.4 Setters (edit actions) **[V]**

These rewrite stored values for one layer (all 16 cells).

**Random**
```
for i in 1..16: values[i] = random(7)          # uniform 0..6
```

**Not-so-random** (mutation)
```
for i in 1..16: values[i] = clip(values[i] + random(3) - 1, 0, 6)      # each step moves -1, 0 or +1
```
Repeated presses perform a random walk away from the starting pattern, with all values bounded to 0..6.

**Reset**
```
Velo layer:  values[i] = 4  for all i             # a medium-loud default
all others:  values[i] = 0  for all i             # silence / no alt / no roll / no slide
```
Reset is per layer, so a freshly reset Chance layer is silent and a reset Velo layer is flat at level 4.

## 4.5 Level scales (summary; details in [05](05-note-output.md))

| Layer | Level 0 | Level 3 | Level 6 |
| --- | --- | --- | --- |
| Chance | never | 50% | always |
| Alt | never alt | 50% alt | always alt |
| Velo | velocity 10 | 68 | 127 |
| Slide | 0 | 64 | 127 |
| Roll | none | 3 repeats | (device caps at 4) |

## 4.6 Statistics from the factory presets **[V]** (computed from the 19 presets)

- Chance levels in use: 0 (800 cells), 6 (295), 1 (50), 3 (50), 2 (13), 4 (4), 5 (4). The presets are mostly "on/off" patterns with a small number of 50% and 17% steps.
- Velo levels: 4 is the commonest (756 of 1,216 cells ≈ 62%); 0 and 6 are rare.
- Roll: 1,179 of 1,216 cells are 0; only 37 steps use a roll at all.
- Alt: 1,047 zeros; the alt note is used sparingly.

## 4.7 Invariants (tests)

- All stored levels in 0..6.
- Counter positions always in `1..length`.
- Lane period equals the LCM of layer lengths (with lock off) or `Global Length` (with lock on).
- Reset: Velo all 4, other layers all 0.
- Not-so-random: every changed cell differs by exactly 1 and stays in 0..6.
- Random: over many runs every level 0..6 appears with equal frequency.
