# 3. Pitch generation

**Input:** Algo (v1/v2), Type (1 to 16), Root, Scale, Live scale intervals.
**Output:** `pitch[16]`, values 0 to 11 (pitch classes; octave is a separate lane).

## 3.1 The urn primitive **[V]**

Max's `urn N` draws integers `0..N-1` in random order **without repeats**; after N draws it
signals "empty", and Sting immediately clears it so the next draw starts a fresh shuffle.

```
urn_init(n):  bag = [0..n-1]; shuffle(bag); pos = 0
urn_draw():   if pos == n: shuffle(bag); pos = 0
              return bag[pos++]
```
(Any unbiased shuffle works, e.g. Fisher-Yates.)

## 3.2 Algo v1, "Classic" **[V]**

Fill 16 steps from a 12-note urn, regrouped into one list.

```
function classic():
    out = []
    repeat 16 times:
        out.append(urn12.draw())        # pitch class 0..11, no repeats within each 12
    return out
```

Properties (use as tests):
- Steps 0 to 11 are a permutation of 0..11.
- Steps 12 to 15 are 4 distinct values drawn from a fresh shuffle.
- No pitch class appears twice within any aligned window of 12 draws.

## 3.3 Algo v2 **[V structure, I details]**

Builds a short motif and tiles it, then shuffles the whole result.

```
function v2():
    N      = 3 + rand(3)                 # motif length: 3, 4 or 5
    motif  = [urn12.draw() for _ in N]   # N distinct pitch classes
    reps   = floor(16 / N) + 1           # enough repeats to cover 16 steps
    tiled  = (motif repeated reps times)[0:16]   # first 16 items
    return shuffle(tiled)                # random permutation of the 16 items
```

Properties:
- At most 3 to 5 distinct pitch classes appear in the pattern (a restricted pitch set, which gives the
  more "riff-like" character compared with Classic).
- Each motif note appears about `16/N` times, in random positions.
- ⚠ Whether the tile truncates to 16 or overshoots and is regrouped is not fully confirmed. The
  graph tiles `floor(16/N)+1` copies then groups to 16, so truncation to 16 is the expected result.

## 3.4 Type: the random → acid morph **[V structure, ⚠ replacement value]**

After the generator runs, the **Type** value (1 to 16) controls how many steps are overwritten with
*repeated notes*. The author's own note in the patch: "Add repeating notes based on sequence type"
and "which notes in the random sequence will be replaced by the 2nd generator and how often".

```
function apply_type(pitch, type):          # type in 1..16
    k       = type_to_count(type)          # ⚠ exact mapping: see below
    indices = draw k unique indices from urn15   # urn 15 → 0..14: step 15 is never chosen... ⚠ or step 0 is
    for i in indices:
        pitch[i] = repeat_note              # ⚠ which note
```

What is known:
- Indices are chosen **without replacement** from a 15-slot urn, so one slot always keeps its generated note.
- At Type 1 the pattern is the raw generator output (label `RANDOM`); at Type 16 it is `xACIDx`, mostly
  repeats. The 16 labels progressively "freeze" letters, so the number of replaced steps rises with Type.
- ⚠ `repeat_note` is most likely the pattern's first note (root-like), which is what gives acid lines their
  hammering feel. To confirm: set Type to 16, press Gen several times and read the resulting notes.
- ⚠ `type_to_count` is most likely `type - 1` or `type`. Confirm the same way.

## 3.5 Scale snap (nearest-degree quantise) **[V]**

Runs only when **Scale = On**. Keeps the original notes so it is reversible.

**Reference list**
- Scale Off: reference = `[0,1,2,3,4,5,6,7,8,9,10,11]` (identity, no change).
- Scale On: reference = the scale's interval list from the host, each shifted by Root and wrapped:
  `ref[j] = (interval[j] + root) % 12`. Default if no host scale is available: major `[0,2,4,5,7,9,11]`.

**Snap**
```
function snap(note, ref):
    best = ref[0]; bestd = abs(note - ref[0])
    for j in 1 .. len(ref)-1:
        d = abs(note - ref[j])
        if d < bestd:             # strict <  → ties keep the earlier ref entry
            best = ref[j]; bestd = d
    return best
```

Behaviours to copy faithfully (or deliberately change):
1. **Distance is linear, not circular.** Pitch class 11 is 11 away from 0, not 1. In a scale containing 0 but not 11, note 11 may snap to 10 or 9 rather than 0.
2. **Ties go to the earlier entry in the reference list**, whose order depends on how Root wrapped it, so tie direction can change with Root.
3. **Two copies of the lane:** `pitch_orig` (never snapped) and `pitch` (snapped). Changing Root or Scale re-snaps from `pitch_orig`. Turning Scale off restores the originals.

## 3.6 Root and transpose handling

- **Root** affects only the scale reference (3.5). It does not transpose the pitches themselves.
- **Transpose** is added at output (see [06](06-outputs.md)).
- Incoming MIDI note `n` yields `n % 12`, which feeds the root path. ⚠ Whether it replaces Root or adds to it is untraced.

## CakeWalk (parameter drift) **[V structure]** {#cakewalk}

Auto-modulation of a dial, run separately for Density and for Type.

```
state: v (current value, integer), amount (0..1), active (bool)
amount_steps = scale(amount, 0..1 → 1..16)        # width of the walk
each tick while active:
    delta = drunk(4) - 2          # random walk step; drunk 4 gives 0..3, so delta in -2..+1 (⚠ exact range)
    v     = clamp(v + delta, within [0, amount_steps])
output v to the dial
```

- The walk is only active when the modulation sources (velocity, aftertouch, slider assignment values) sum to non-zero. Otherwise the dial's modulation display is hidden and the dial stays manual.
- Used for the *dial's displayed value* and its colour feedback, and (⚠) to feed the generator when it changes.
