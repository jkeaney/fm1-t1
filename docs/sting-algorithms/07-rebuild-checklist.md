# 7. Rebuild checklist, tests and open questions

## 7.1 Build order

Each step is independently testable. Stop when the invariants pass.

1. **PRNG + primitives:** `rand(n)`, `bern(p)`, `urn(n)` on one seeded generator.
2. **Classic (v1) pitch generator** ([03 §3.2](03-pitch-generation.md)).
3. **v2 pitch generator** (§3.3).
4. **Scale snap** with `pitch_orig` / `pitch` pair (§3.5). Include the linear-distance and tie quirks.
5. **Octave lane** ([04 §4.1](04-rhythm-and-dynamics.md)).
6. **Gate lane** (§4.2).
7. **Accent + velocity lane** (§4.3).
8. **Type replacement pass** (§3.4), after you have confirmed the open questions below.
9. **Step counter, direction, length, offset** ([05](05-playback-timing.md)).
10. **Swing, then Jitter.**
11. **Output assembly** ([06](06-outputs.md)).
12. **CakeWalk drift** (optional extra).

## 7.2 Determinism (important for FM-1)

- Use a single seeded PRNG; derive sub-streams per lane (e.g. `seed ^ lane_id`) so changing one lane's code doesn't reshuffle the others.
- Never read wall-clock or hardware noise inside generation.
- Test: same seed + same controls = byte-identical `StingPattern`.

## 7.3 Invariant tests (no device needed)

| Test | Assertion |
| --- | --- |
| Classic permutation | For many seeds, `pitch[0..11]` is a permutation of 0..11. |
| Classic window | No repeated pitch class inside any aligned 12-window. |
| v2 pitch set | Distinct pitch classes in output are 3, 4 or 5. |
| v2 balance | Each motif note appears `floor(16/N)` or `ceil(16/N)` times (± truncation). |
| Snap membership | With Scale On, every `pitch[i]` is in the reference list. |
| Snap reversible | Turning Scale off restores `pitch_orig` exactly. |
| Snap tie rule | Equidistant note maps to the earlier reference entry. |
| Octave range | All values in −2..+2; mean non-zero rate rises with step index. |
| Gate set | All values in {30, 70, 100, 127}; `gate[0] ∈ {70, 100}`. |
| Accent uniqueness | K accented steps are distinct positions. |
| Direction | L = 4: Fwd `0 1 2 3`, Bwd `3 2 1 0`, Pendulum `0 1 2 3 2 1`. |
| Division table | `steps_per_beat` matches §5.1 for all 15 divisions. |
| Output range | `midi_note` within 0 to 127 for any controls (clamp). |

## 7.4 Open questions to confirm on the running device

These are the ⚠ items. Each can be settled quickly in Ableton with the device loaded.

| # | Question | How to check |
| --- | --- | --- |
| 1 | What note replaces steps in the Type pass, and how many steps for each Type? | Type 16, press Gen 10 times; read the notes (use Gen Clip and look at the MIDI). |
| 2 | Is the never-replaced slot step 0 or step 15? | Same experiment; see which index never repeats the repeat-note. |
| 3 | What does Density change (count of steps, accents, or probability)? | Gen at Density 0, 8, 16 and compare note counts and accents. |
| 4 | Accent neutral point and which direction raises accents? | Gen Clip at Accent min, mid, max and read velocities. |
| 5 | Swing unit and strength. | Swing 1.0 on 1/16 at 120 BPM; measure the odd-step delay in the clip or on a scope. |
| 6 | Pendulum end repeats? | Steps 4, Pendulum; log the played step order. |
| 7 | Does incoming MIDI replace or add to Root? | Play a MIDI note into the device, with Scale on, and watch the scale. |
| 8 | Poly Reset meaning. | Set to 5 and listen or log the step wrap. |
| 9 | Gate dial role (global scale vs min length). | Gen Clip at Gate 50 and 100; compare durations. |

## 7.5 FM-1 porting notes

- Everything here is integer arithmetic except the pendulum/gate curves (`^1.2`, `^0.6`); precompute
  small lookup tables (16 entries each) instead of using `pow` at runtime.
- Total generation cost is tiny (a few hundred random draws), so it can run in the UI task rather
  than the 192 PPQN ISR; only the lane reads at §5.6 need to be on the sequencer tick path.
- Map the device's controls onto FM-1's existing parameter model rather than copying its UI. A sensible
  minimum set is Algo, Type, Density, Steps, Root, Scale, Direction, Swing, Gen.
