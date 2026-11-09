# 7. Preset review (all 7 factory presets, decoded)

The 7 `.adv` presets are gzip-compressed XML. Only the five **lane lengths** and Resync are stored as ordinary parameters; **everything else** (the 5 × 16 fader values, Root Note, Scale, Swing, Triplets, MIDI mode) is stored in a text blob (hex-encoded JSON).
The tables below were decoded from those files. Fader levels are 0 to 6 (see [03](03-lanes.md) and [05](05-note-output.md)).

**Reading a lane:** 16 characters, one per fader. Only the first *length* characters of each lane are played; the rest are stored but unused.

## Melodic Steps Aeolian

- Root note: **45 (A1)** · Scale: **Minor (natural)** (degrees [0, 2, 3, 5, 7, 8, 10]) · Swing: 0 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 4 · Transpose 7 · Length 6 · Velocity 5 · Chance 8 → **combined pattern period 840 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `2453333333333333` | `2453` (4) |
| Transpose | `0033311000000000` | `0033311` (7) |
| Length | `1132331332112311` | `113233` (6) |
| Velocity | `1432055555555555` | `14320` (5) |
| Chance | `3630333300000000` | `36303333` (8) |

Mean chance over the played faders: 50% · Transpose levels used: [0, 1, 3] · Octave levels used: [2, 3, 4, 5]

## Melodic Steps Chromatic

- Root note: **36 (C1)** · Scale: **Chromatic** (degrees [0, 1, 2, 3, 4, 5, 6]) · Swing: 10 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 6 · Transpose 7 · Length 5 · Velocity 12 · Chance 16 → **combined pattern period 1680 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `3233343333333333` | `323334` (6) |
| Transpose | `3304411000000000` | `3304411` (7) |
| Length | `1221222222222222` | `12212` (5) |
| Velocity | `5535535535535555` | `553553553553` (12) |
| Chance | `4515435554241134` | `4515435554241134` (16) |

Mean chance over the played faders: 58% · Transpose levels used: [0, 1, 3, 4] · Octave levels used: [2, 3, 4]

## Melodic Steps Dorian

- Root note: **38 (D1)** · Scale: **Dorian** (degrees [0, 2, 3, 5, 7, 9, 10]) · Swing: 0 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 5 · Transpose 9 · Length 8 · Velocity 7 · Chance 6 → **combined pattern period 2520 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `3435444444444444` | `34354` (5) |
| Transpose | `0011223340000000` | `001122334` (9) |
| Length | `2222222222222222` | `22222222` (8) |
| Velocity | `6421642444444444` | `6421642` (7) |
| Chance | `6262140000000000` | `626214` (6) |

Mean chance over the played faders: 58% · Transpose levels used: [0, 1, 2, 3, 4] · Octave levels used: [3, 4, 5]

## Melodic Steps Locrian

- Root note: **40 (E1)** · Scale: **Locrian** (degrees [0, 1, 3, 5, 6, 8, 10]) · Swing: 10 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 12 · Transpose 7 · Length 5 · Velocity 6 · Chance 16 → **combined pattern period 1680 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `5243622321143333` | `524362232114` (12) |
| Transpose | `3210432000000000` | `3210432` (7) |
| Length | `2111122222222222` | `21111` (5) |
| Velocity | `3215100000000000` | `321510` (6) |
| Chance | `0105035054201103` | `0105035054201103` (16) |

Mean chance over the played faders: 31% · Transpose levels used: [0, 1, 2, 3, 4] · Octave levels used: [1, 2, 3, 4, 5, 6]

## Melodic Steps Mixolydian

- Root note: **41 (F1)** · Scale: **Mixolydian** (degrees [0, 2, 4, 5, 7, 9, 10]) · Swing: 0 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 9 · Transpose 8 · Length 6 · Velocity 15 · Chance 7 → **combined pattern period 2520 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `4132424224444444` | `413242422` (9) |
| Transpose | `6400001300000000` | `64000013` (8) |
| Length | `2121202222222222` | `212120` (6) |
| Velocity | `3454324423322110` | `345432442332211` (15) |
| Chance | `2460603000000000` | `2460603` (7) |

Mean chance over the played faders: 50% · Transpose levels used: [0, 1, 3, 4, 6] · Octave levels used: [1, 2, 3, 4]

## Melodic Steps Phrygian

- Root note: **43 (G1)** · Scale: **Phrygian** (degrees [0, 1, 3, 5, 7, 8, 10]) · Swing: 15 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 4 · Transpose 7 · Length 6 · Velocity 15 · Chance 3 → **combined pattern period 420 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `2453333333333333` | `2453` (4) |
| Transpose | `0103010000000000` | `0103010` (7) |
| Length | `1132331332112311` | `113233` (6) |
| Velocity | `6453565666455445` | `645356566645544` (15) |
| Chance | `6030000000000000` | `603` (3) |

Mean chance over the played faders: 50% · Transpose levels used: [0, 1, 3] · Octave levels used: [2, 3, 4, 5]

## Neurothmic Drums

- Root note: **36 (C1)** · Scale: **Chromatic** (degrees [0, 1, 2, 3, 4, 5, 6]) · Swing: 0 · Triplets: off · MIDI-in mode: off
- Lane lengths: Octave 2 · Transpose 16 · Length 16 · Velocity 16 · Chance 16 → **combined pattern period 16 steps**

| Lane | Faders 1–16 | Played (first n) |
| --- | --- | --- |
| Octave | `3333333333333333` | `33` (2) |
| Transpose | `0656265016566242` | `0656265016566242` (16) |
| Length | `6600216425535102` | `6600216425535102` (16) |
| Velocity | `6434542453245424` | `6434542453245424` (16) |
| Chance | `6666666666566566` | `6666666666566566` (16) |

Mean chance over the played faders: 98% · Transpose levels used: [0, 1, 2, 4, 5, 6] · Octave levels used: [3]

## Cross-preset summary

| Preset | Root | Scale | Lengths O/T/L/V/C | Period (steps) | Mean chance | Swing | Triplets |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Melodic Steps Aeolian | A1 | Minor (natural) | 4/7/6/5/8 | 840 | 50% | 0 | off |
| Melodic Steps Chromatic | C1 | Chromatic | 6/7/5/12/16 | 1680 | 58% | 10 | off |
| Melodic Steps Dorian | D1 | Dorian | 5/9/8/7/6 | 2520 | 58% | 0 | off |
| Melodic Steps Locrian | E1 | Locrian | 12/7/5/6/16 | 1680 | 31% | 10 | off |
| Melodic Steps Mixolydian | F1 | Mixolydian | 9/8/6/15/7 | 2520 | 50% | 0 | off |
| Melodic Steps Phrygian | G1 | Phrygian | 4/7/6/15/3 | 420 | 50% | 15 | off |
| Neurothmic Drums | C1 | Chromatic | 2/16/16/16/16 | 16 | 98% | 0 | off |


## Review notes

These observations come from the decoded files, not from listening to the presets.

1. **Six melodic presets and one rhythmic one.** The six *Melodic Steps* presets are named for a mode (Chromatic, Dorian, Locrian, Mixolydian, Phrygian, Aeolian). *Neurothmic Drums* is a different use of the same engine (see 4 below).
2. **Roots ascend with the preset list:** the stored roots are C1 (Chromatic), D1 (Dorian), E1 (Locrian), F1 (Mixolydian), G1 (Phrygian), A1 (Aeolian). The modes are therefore *not* the white-key modes of those roots
   (for example Locrian on E and Phrygian on G); each preset is simply "this scale from this root".
3. **Every melodic preset uses lane lengths that share few factors.** Combined periods are 420 (Phrygian), 840 (Aeolian), 1,680 (Chromatic and Locrian) and 2,520 (Dorian and Mixolydian) steps. At a 16th-note speed the shortest of these is over 26 bars, so the melody effectively never repeats during a normal loop.
   The Phrygian preset has the shortest period because its Chance lane is only 3 long. **[V]**
4. **Neurothmic Drums is a drum-pattern preset.** It uses the *Chromatic* scale (7 adjacent semitones), a 2-step Octave lane fixed at the neutral level 3, and 16-step Transpose, Length, Velocity and Chance lanes with Chance 6 almost everywhere (mean 98%).
   The Transpose lane (`0656265016566242`) therefore selects among seven neighbouring pads (36 to 42) and the Length lane (`6600216425535102`) sets note length, so the device acts as a one-bar drum sequencer. Its combined period is 16 steps. **[V]**
5. **Chance is moderate in the melodic presets:** the mean over the played faders is 31% (Locrian), 50% (Aeolian, Mixolydian, Phrygian) and 58% (Chromatic, Dorian), so roughly half of the steps are silent and which ones vary on every pass, since chance is random, not deterministic.
6. **Swing is used lightly:** 0 in four presets, 10 in Chromatic and Locrian, 15 in Phrygian. No preset uses Triplets or MIDI mode. **[V]**
7. **Unused faders hold the lane's neutral level.** In *Aeolian* the Velocity lane is `14320` (5 long) followed by `5`s, and the Octave lane is `2453` followed by `3`s. Those are exactly the Reset values from [03](03-lanes.md) (Velocity 5, Octave 3), so the unused faders appear to be left at their reset value. This is consistent with the authors shortening a lane rather than clearing it (inferred, not observed).
8. **Not exercised by any preset:** Triplets, MIDI-in mode, Resync, Double/Half time and the lengths other than the five lane lengths. Those code paths need your own tests.
