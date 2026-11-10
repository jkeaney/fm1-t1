# HARMONY: global key, scale and chord progression (P2-012)

Task P2-012. Status: **for owner review** (gate `human-review`). Design only: no firmware is changed. The maths below is executable in
[`tests/design/harmony_proto.py`](../../tests/design/harmony_proto.py) (`--check` asserts the golden values quoted here); every worked example is its output.
Where this document and the locked tables disagree, section 8 lists each locked line it changes; nothing else in the lock moves.

## 0. Summary and the five orchestrator rulings

HARMONY is one global state: **a progression** (1 to 8 chords, each a scale degree 1..7, spread over 1 to 8 bars), **three voicing dials** (`VOICE`, `MOVE`, `DRONE`) and **a
per-channel ROLE** (OFF, AUTO, ROOT, FULL). It lives on the existing hold-GLO page (table id `SCALE`, screen name `HARMONY`) next to the global SCALE and KEY: one gesture
(hold GLO) opens it, one more control changes anything on it. Every channel passes its generated notes through one **chord stage** after the algorithm and the scale quantise.

| # | Ruling | Where it is answered |
| --- | --- | --- |
| 1 | MIDI-in chord following is **deferred to v2**; only the hook point is specified | 7.6 (one paragraph) |
| 2 | **Invisible by default.** With `1---`, `VOICE 2`, no `DRONE` and every ROLE AUTO the device sounds and behaves exactly as the locked design; steps the producer enters are only ever scale-quantised, never pulled to the chord; there is no per-channel opt-in for entered steps in v1 | 1 (with the worked example) |
| 3 | **Small control surface, one gesture.** No new top-level page: HARMONY extends the hold-GLO page; PROG dice is HARMONY-page-only | 5 (page), 6.3 (dice scheme and reason) |
| 4 | **Memory.** 42 B of RAM (target 64 B), 0 B in the per-preset pool beyond the saved 8 B per slot (144 B in all) | 7.2 |
| 5 | P2-013 (ARP) and P2-014 (PATTERNS) are untouched except for the chord and voicing query interface | 9 |

## 1. Invisible by default (ruling 2)

### 1.1 What "inert" means

HARMONY is **inert** when all of: one chord (`nchords = 1`), its degree is 1, `VOICE = 2`, `DRONE = 0`. (`BARS`, `MOVE` and the ROLEs do not matter for the test:
with one chord there is nothing to lead between.) Then:

- a channel with ROLE AUTO (the default) or OFF gets **exactly** the note its algorithm and SP6 produced: `harm_apply()` returns its input, no anchor, no pull;
- the PERF screen draws nothing for HARMONY (no chord chip, no changed label);
- the 768-tick bar grid, the clock and every other behaviour are untouched; `harm_apply()` costs one byte test per sounding step.

ROOT and FULL are explicit per-channel choices made on the HARMONY page, so they apply even while the rest is inert.

HARMONY becomes **active** the moment the producer does something that only HARMONY can do: types a progression other than `1`, turns `VOICE` off 2, or walks a `DRONE`.
The page shows the state in words: the lane header reads `HARMONY` always; PERF draws the chord chip only while active (5.4). Returning every dial to its default makes it inert
again (RESET + PROG puts the progression back to `1---`, 6.3).

### 1.2 Entered steps are scale-only

A step whose `PITCH` is set (typed on the keys in REC, or set in NOTE EDIT) is quantised to the scale (SP6) and **never** moved by the chord stage, in every role and every
state. A step whose `PITCH` is AUTO follows the generated note and is therefore harmonised like any generated note. There is no per-channel opt-in to chord-snap entered steps in
v1 (considered and left to v2: it would need a fifth ROLE value and a visible mark on every entered step). The producer who wants a chord tone on a step types the chord tone.

### 1.3 Worked example: the target workflow, bytes unchanged

The reference workflow (VISION): select CH1, MIDI 2, algorithm ACID, enter steps on F3..G5 or dial one in, PLAY, perform on a TB-3.

| Step of the workflow | HARMONY state | Result |
| --- | --- | --- |
| power-on, `D#4` `K2` 2, ALGORITHM ACID | default `{0x03,0,0,0,0x82,0,0,0}` = `1---`, VOICE 2, MOVE 8, no drone, AUTO | inert |
| REC, type steps C2 (s = 0), G2 (s = 7), Bb2 (s = 10) | entered steps: scale-only | ACID's generated notes between them are unchanged |
| PLAY | key C, MINPEN, octave 0, base 36 | ACID emits s = 0, 3, 7, 10 ... exactly as the P2-008 golden; MIDI `ch2 ON 36`, `39`, `43`, `46` ... |
| morph K1..K4, dice, mute | not touched | not touched |

Equality is a test, not a promise: the dev task for the chord stage must run the P2-008 golden for all four algorithms with the default HARMONY state and require **byte-identical
MIDI** (acceptance criterion H-INERT, section 10). The same test with ROLE OFF on every channel and a non-inert progression must also be byte-identical.

Then the producer opens HARMONY (hold GLO), types `F3 B3 C4 F3` (chords 1 4 5 1): in MINPEN that is C, G, Bb, C. Only now do AUTO channels change (S18).

## 2. Data model

### 2.1 `HarmonySave` (8 B, saved per preset slot)

```c
typedef struct {
    uint8_t len;     /* bits0..2 bars-1 (0..7); bits3..5 nchords-1 (0..7); bits6..7 0. Rule: nchords <= bars */
    uint8_t deg[3];  /* 8 slots x 3 bits: (deg[0] | deg[1]<<8 | deg[2]<<16) >> (3*i) & 7 = degree-1 of slot i (0..6) */
    uint8_t vm;      /* bits0..3 VOICE 0..6 (default 2); bits4..7 MOVE 0..8 (default 8) */
    uint8_t drone;   /* 0 = off; 1..14 = the candidate walked to (2.5) */
    uint8_t roles;   /* 2 bits per channel (bit 2c): 0 AUTO, 1 OFF, 2 ROOT, 3 FULL */
    uint8_t rsv;     /* 0; version / future */
} HarmonySave;
_Static_assert(sizeof(HarmonySave) == 8, "HarmonySave");
```

Key, scale and octave are **not** repeated: they stay in `GlobalSave` (ADR-0003, `key`, `scale`, `octave`) and remain the single source for SP6 and SP9.
The 8 B joins `SeqPreset` (1388 -> 1396 B, version 3).

**Defaults:** `{0x03, 0,0,0, 0x82, 0, 0, 0}`: `bars = 4`, `nchords = 1`, `deg[0] = 0` (degree 1), `VOICE 2`, `MOVE 8`, no drone, all roles AUTO. This is `1---`.
A preset of version 2 loads with these defaults; `sq_preset_load` clamps `bars/nchords` to 1..8 with `nchords <= bars`, degrees to 0..6, `VOICE <= 6`, `MOVE <= 8`, `drone <= 14`, roles to 0..3.

### 2.2 Notation and the `1---` default

A progression is shown as a strip of **bar cells** (one per bar of the phrase): the degree numeral at each bar where a chord starts, `-` in a bar where the previous chord holds.
So `1---` is one chord held for 4 bars (the owner's `1----` is the same with 5 bars; the default is 4, owner question 85), `1351` is four chords of one bar each (the owner's 1-3-5-3
style), `1-4-` is two chords of two bars. Typing keys writes degrees (5.3); there is no separate dash key because the hold lengths follow from the two numbers `bars` and `nchords`.

### 2.3 Chord timing on the 768-tick bar grid

`bar = (sq_bar / 768) % bars`, where `sq_bar` is the ticks since PLAY (ADR-0003 section 10), so PLAY resets the position and an external clock drives it exactly as it drives the steps.
Chord `i` (0-based) starts at bar `start(i) = (i * bars) / nchords` (integer division) and holds until the next chord's start, the last one until the phrase wraps.

| bars | nchords | starts (bar index) | strip |
| --- | --- | --- | --- |
| 4 | 1 | 0 | `1---` |
| 4 | 2 | 0, 2 | `1-4-` |
| 4 | 3 | 0, 1, 2 | `145-` (the last chord holds two bars) |
| 4 | 4 | 0, 1, 2, 3 | `1451` |
| 8 | 3 | 0, 2, 5 | `1-4--5--` |
| 5 | 4 | 0, 1, 2, 3 | `1451-` |

The chord changes **at the bar boundary tick** (`tick % 768 == 0`) and never mid-bar. A step uses the chord of its own start tick (the swung start), also when the step was evaluated
one step ahead (`nxt`, ADR-0003 7.3): `harm_apply` takes the step's tick and, when it lies in the next bar, asks `harm_chord_at_bar(bar + 1)` instead of the current `HarmRT`.
Sounding notes are never cut at a boundary (decision 47, notes end by their queued offs).

### 2.4 Chord quality comes from the scale (no quality is stored)

The slot stores a degree only. A chord is built by **stacking templates snapped to the scale**: with `r` the root (the degree's scale tone, semitones above the key) and the template
`T = [0, 4, 7, 11, 14]` (root, third, fifth, seventh, ninth), tone `k` is `snap(r + T[k])`, where `snap(x)` is the nearest scale tone by absolute semitone distance, **ties to the lower**
(SP6). A duplicate pitch class is dropped (the lowest copy stays). This gives true diatonic chords in 7-note scales (C major: C E G, F A C, G B D with a dominant seventh G B D F),
the scale's own colour in minor scales, and consonant open chords in the default minor pentatonic (C Eb G; G Bb Eb; Bb Eb F). It needs no per-scale table: only `T_SCALE_MASKS`.
Degrees above the scale's size wrap with an octave (`deg 6` in a 5-note scale is tone 1 up an octave); the progression dice never produces a degree above `min(7, scale size)`.

The chord's **quality** for display (the chip's case) is the interval to the snapped third: 3 semitones lower case (`ii`), 4 upper case (`IV`), otherwise upper case.

### 2.5 State and ISR copy

`HarmonySave harm_db[2]` is double buffered like `sq_gdb` (the UI writes the idle buffer, flips `harm_gidx`; the ISR reads it once per block). `HarmRT` (14 B) is derived by the ISR
at a bar boundary or on a flip (the flip is taken at the next 48-tick step grid so a dial lands on the next step, not the next bar): `slot`, the voiced notes `vt[6]` and count, the
pitch-class mask of the chord tones and drone, the root, the drone pitch, and one `anchor` bit per channel. Rebuilding the voice-leading chain for slot `k` costs at most `k` iterations
of the 3 x n candidates of 3.2 (under 1500 integer operations in the worst case, once per edit or per bar).

## 3. The voicing dials (simple, integer, performable live)

All three are integer tables. **Neutral** (all defaults) is plain root-position triads with no drone, which is the inert state of 1.

### 3.1 VOICE (K3), range 0..6, default 2

The chord tones that are built and used. Tone `k` of `T` is placed at `snap(r + T[k]) + 12 o` with octave shift `o`; duplicates by pitch class dropped; the result sorted ascending.

| VOICE | Reads | Tones (k, o) | Sounds like |
| --- | --- | --- | --- |
| 0 | `0` | root | root only |
| 1 | `1` | root, fifth | power chord |
| 2 | `2` (default) | root, third, fifth | plain close triad |
| 3 | `3` | + seventh | seventh chord |
| 4 | `4` | + ninth | ninth chord |
| 5 | `5` | root, fifth, seventh, ninth, **third + 12** | open ninth |
| 6 | `6` | root, seventh, **third + 12, fifth + 12, ninth + 12** | wide |

Golden, C major, chords 1 4 5 1, MOVE 8 (semitones above the key):

| VOICE | I | IV | V | I |
| --- | --- | --- | --- | --- |
| 0 | `0` | `5` | `7` | `0` |
| 1 | `0 7` | `5 12` | `7 14` | `0 7` |
| 2 | `0 4 7` | `5 9 12` | `7 11 14` | `0 4 7` |
| 3 | `0 4 7 11` | `5 9 12 16` | `7 11 14 17` | `0 4 7 11` |
| 4 | `0 4 7 11 14` | `5 9 12 16 19` | `7 11 14 17 21` | `0 4 7 11 14` |
| 5 | `0 7 11 14 16` | `5 12 16 19 21` | `7 14 17 21 23` | `0 7 11 14 16` |
| 6 | `0 11 16 19 26` | `5 16 21 24 31` | `7 17 23 26 33` | `0 11 16 19 26` |

VOICE changes the chord tones every AUTO channel is pulled to, what FULL channels play, and what ARP and PATTERNS read through the interface (9). ROOT channels ignore it.

### 3.2 MOVE (K4), range 0..8, default 8: the voice-leading distance

MOVE decides how the voiced chord is **placed** after the chord before it: root position (8) or the smoothest inversion (0).

Inputs: the previous voicing `P` (sorted, the first chord has none), the root-position voicing `W` of the next chord (3.1), `m = MOVE`.
Algorithm (integer, deterministic):

1. If there is no `P`, or `m = 8`: the voicing is `W`.
2. Otherwise for each inversion `i` in `0 .. n-1` (raise the lowest `i` notes of `W` by 12, re-sort) and each shift `q` in `{0, -12}`, form `C = inversion(W, i) + q`. Skip a `C` with a note below 0 or above 48.
3. `dist(C) = sum over j < min(len C, len P) of |C[j] - P[j]|`.
4. `cost(C) = dist(C) * (8 - m) + m * 4 * i`.
5. Choose the `C` with the least `(cost, i, |q|, q)` (ties: fewer inversions, then no shift, then the lower shift).

So `m = 8` costs only the inversion number (root position wins), `m = 0` is pure smallest movement, and in between the voice-leading gain must beat `4 m` per inversion.
The chain restarts at slot 0 on every PLAY and every phrase wrap, so each pass of the loop is identical (determinism). The drone does not take part (3.3).

Golden, C major, VOICE 2, chords 1 4 5 1 (`dist` = the sum of the voice movements over the three changes):

| MOVE | I | IV | V | I | dist |
| --- | --- | --- | --- | --- | --- |
| 0 to 4 | `0 4 7` | `0 5 9` | `2 7 11` | `4 7 12` | 12 |
| 5 | `0 4 7` | `5 9 12` | `7 11 14` | `4 7 12` | 30 |
| 6 to 8 | `0 4 7` | `5 9 12` | `7 11 14` | `0 4 7` | 42 |

C natural minor (key C, NATMIN), the same chords (i, iv, v, i):

| MOVE | i | iv | v | i |
| --- | --- | --- | --- | --- |
| 8 (root position) | `0 3 7` (C Eb G) | `5 8 12` (F Ab C) | `7 10 14` (G Bb D) | `0 3 7` |
| 0 to 4 | `0 3 7` | `0 5 8` (C F Ab) | `2 7 10` (D G Bb) | `3 7 12` (Eb G C) |

C minor pentatonic (the device default, key C, MINPEN), chords 1 4 5 1 = C, G, Bb, C:

| VOICE, MOVE | I | IV | V | I |
| --- | --- | --- | --- | --- |
| 2, 8 | `0 3 7` (C Eb G) | `7 10 15` (G Bb Eb) | `10 15 17` (Bb Eb F) | `0 3 7` |
| 2, 0 | `0 3 7` | `3 7 10` (Eb G Bb) | `3 5 10` (Eb F Bb) | `3 7 12` (Eb G C) |
| 3, 8 | `0 3 7 10` | `7 10 15 17` | `10 15 17` (the seventh doubles the root's pitch class and is dropped) | `0 3 7 10` |

MOVE only changes the **placement** of the voiced notes: AUTO channels are pulled to pitch classes (any octave) and ROOT channels play the root, so MOVE is audible on **FULL
channels** and through the interface (ARP, PATTERNS). The strip shows the voiced chord of the touched chord only as an audition when a key is typed.

### 3.3 DRONE (OCT-/OCT+ on the HARMONY page), range 0..14, default 0

The producer walks the drone tone through the candidates by tapping OCT+ (next) and OCT- (previous), and finds the alignment by ear. `n' = min(7, size of the scale)` is the number of
degrees offered (5 in the default pentatonic).

| p | Drone tone | Octave shift |
| --- | --- | --- |
| 0 | off | |
| 1 .. n' | the scale tone of degree `p` | +0 |
| n'+1 .. 2n' | the scale tone of degree `p - n'` | +12 |
| above 2n' | clamped to 2n' | |

The drone pitch is `degree_root(p) + 12 * shift` semitones above the key (`drone_pitch` in the prototype). What it does:

- **ROOT channels** play the drone instead of the chord root (a drone point in the bass);
- **AUTO channels** treat the drone's pitch class as one more chord tone in every chord (it enters the pitch-class mask, so notes are pulled onto it too);
- **FULL channels** get the drone as an extra voice of the voicing (up to 6 notes);
- the anchor (first hit of a chord) is still the chord root.

**Clash marks.** For each chord of the loop the status line shows one mark: `+` the drone's pitch class is a tone of that chord's voicing; `X` it is not, and a chord tone lies one
semitone from it (a real clash); `-` it is not in the chord but nothing is a semitone away (a passing tone, usually fine). Golden, C major, chords I IV V I, VOICE 2:

| DRONE | Tone | I | IV | V | I |
| --- | --- | --- | --- | --- | --- |
| 1 | C (0) | `+` | `+` | `X` (B is a semitone away) | `+` |
| 5 | G (7) | `+` | `-` | `+` | `+` |
| 8 | C (12) | `+` | `+` | `X` | `+` |
| 4 | F (5) | `X` | `+` | `-` | `X` |

C natural minor (i iv v i): DRONE 1 `+ + - +`; DRONE 5 (G) `+ X + +`; DRONE 3 (Eb) `+ - X +`. MINPEN (C G Bb C): DRONE 1 `+ - - +`; DRONE 2 (Eb) `+ + + +`; DRONE 4 (G) `+ + - +`.

### 3.4 How the dials combine with the morph bias

The morph bias (4.3) sets **how often** a note is pulled (the pull amount, 0..8, from the algorithm's own pitch-width morph). The dials set **to what**: VOICE sets which pitch classes
count as chord tones, MOVE sets where the exact voiced notes sit (FULL only), DRONE adds one more target. Nothing about a dial changes the pull amount, and nothing about a morph
changes a dial: the two are independent, so turning K1..K4 on PERF never moves the voicing and turning K3/K4 on HARMONY never moves a morph.

### 3.5 Each channel can follow the voicing differently: ROLE

ROLE is per channel (the ARP button on the HARMONY page, for the channel SELECT points at; shown as `CHn ROLE x` on status line 2):

| ROLE | A generated note becomes | Typical use |
| --- | --- | --- |
| `AUTO` (default) | pulled toward the chord tones by the rule of 4 (first hit of a chord = its root); the chord tones are the pitch classes of the VOICE chord plus the drone | lead, acid line |
| `OFF` | untouched: never harmonised | a channel that drives drums by pitch, or a part that must stay on the scale |
| `ROOT` | the chord root (the drone if set), in the octave of the algorithm's note | bass |
| `FULL` | the voiced chord note chosen by the line's contour (4.2) | pad, stabs, a second voice |

## 4. The chord stage (the quantise rule every algorithm uses)

### 4.1 Where it runs

`harm_apply(ch, t, strong, s, hi, tick)` runs once per sounding generated step, in `sq_eval` after the algorithm's `step()` and SP6, before SP9 (`sq_note`). It does not run for a step whose
PITCH is set (1.2), nor for the audition. Inputs: the channel, the step position `t` within the channel pattern (0..len-1), `strong`, the scale-snapped semitone `s` (0..96), the
algorithm's range ceiling `hi` (`AlgoDesc.hi(m)`, ADR-0003 section 20), the tick. It returns a semitone in `0..hi`.

### 4.2 The rule, in order

Let `inert` be 1.1, `role` the channel's ROLE, `C` the chord of the step's tick (voiced notes `vt[]`, pitch-class mask `pcs`, root `r`, drone pitch `drone`), `pull` the amount of 4.3.

1. `role = OFF`, or `role = AUTO` and `inert`: return `s`.
2. `role = ROOT`: `x = (drone if set else r) % 12 + 12 * (s / 12)`.
3. `role = FULL`: `x = vt[ min(n-1, (s * n) / (hi + 1)) ]` (the line's contour is mapped onto the voiced notes from the bottom voice to the top one; the drone is one of the voices).
4. `role = AUTO` (active), by priority:
   - **Root default (anchor):** if this channel has an `anchor` bit set (set at PLAY and at every chord change) this is the **first sounding generated hit** of the chord: `x = r % 12 + 12 * (s / 12)`; clear the bit.
   - **Chord-tone priority:** otherwise `peff = min(8, pull + 4 * strong)`; `h = ((t * 0x9E3779B1) >> 29)` (3 bits, a fixed hash of the step position, no random number consumed so the generator's seeds never shift); if `h < peff` then `x` is the pitch of the chord-tone set (all octaves of `pcs`) nearest to `s`, ties to the lower.
   - **Scale-tone fallback:** otherwise `x = s` (a passing scale tone stays, because `s` is already on the scale).
5. Octave handling: chord tones exist in every octave, so the register of `s` is kept. After the choice fold into the range: `while x > hi: x -= 12; while x < 0: x += 12`. A channel with a **fixed octave range** (RANGE, SPAN or JUMP kept small) therefore never leaves it, and the pulled note is the nearest chord tone that fits.
6. Return `x`; the core adds key, octave and base with `sq_note` as before.

`strong` is table data per algorithm (the table below). The root default needs no extra state beyond the one bit per channel.

Golden (C major, key C, bar 2 of `1451`, the IV chord F A C, an ACID channel with the default JUMP 5 so `pull = 3`, `hi = 24`, the generated line s = 0 2 4 5 7 9 11 12 at step positions
t = 16..23, strong when `t % 4 = 0`):

| t | s | h(t) | strong | peff | rule | x |
| --- | --- | --- | --- | --- | --- | --- |
| 16 | 0 | 7 | yes | 7 | anchor (first hit of IV) | 5 |
| 17 | 2 | 4 | no | 3 | 4 >= 3, scale tone kept | 2 |
| 18 | 4 | 0 | no | 3 | 0 < 3: nearest of {0,5,9} to 4 is 5 | 5 |
| 19 | 5 | 5 | no | 3 | kept (already a chord tone) | 5 |
| 20 | 7 | 2 | yes | 7 | 2 < 7: 5 and 9 tie, the lower wins | 5 |
| 21 | 9 | 7 | no | 3 | kept | 9 |
| 22 | 11 | 4 | no | 3 | kept | 11 |
| 23 | 12 | 1 | no | 3 | 1 < 3: 12 is a chord tone | 12 |

Out: `5 2 5 5 5 9 11 12`. With the minor pentatonic default (the same IV chord G Bb Eb, a line s = 0 3 3 7 10 12 7 5): out `7 3 3 7 10 12 7 3`.

### 4.3 The pull amount and the strong steps: the morph rule per algorithm

The pull comes from the pitch-width morph the algorithm already has (ALGORITHM_SET 12 repeats this table): the wider the line the producer allows, the more the strong notes are drawn to the
chord. No knob is added and no morph changes meaning.

| Algorithm | Pull (0..8) | Default | Strong steps (+4) | Anchor | Other morphs |
| --- | --- | --- | --- | --- | --- |
| `ACID` | `3 + JUMP * 3 / 20` | 3 | accented steps, octave-jump notes | yes | the `PEDAL` morph repeats the chord root; slides glide to the pulled pitch |
| `EUCLID` | `2 + SPAN * 6 / 24` | 5 | accented hits; hit ordinal 0 of each cycle | yes | the contour is built first and then pulled; ratchets share the pulled pitch |
| `SNAKE` | `2 + RANGE * 6 / 48` | 3 | lane notes with velocity >= 100 | yes | the traversal is read first; each lane note is pulled by its lane position, so PATH changes keep each lane note's pitch |
| `STAGE` | `2 + RANGE * 6 / 36` | 4 | first pulse of a stage; long holds (`I-`) | yes | a ratcheted stage shares one pulled pitch |

The pull is 0 while HARMONY is inert and for roles other than AUTO. The dice: NOTES re-rolls the underlying pitches and the stage pulls the new ones; HITS, ACCNT, ARTIC and SHAPE
never touch pitch, so they leave the pull alone (ACCNT changes which steps are strong).

## 5. UX: where HARMONY lives and how it is operated

### 5.1 Placement (rule 3, one gesture; L rubric favours extending the page)

HARMONY is the hold-GLO page (table id `SCALE`), screen name `HARMONY`. The alternatives scored under the L rubric (PERFORMANCE_DESIGN 7.1; V I O L, pass = 10 for a setup function):

| Option | V | I | O | L | Total | Gestures | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| **A: extend the hold-GLO page** (scale and key are already there; `h:HARM` is named on PERF) | 3 | 2 | 3 | 2 | 10 | 2 (hold GLO, then the control) | **chosen** |
| B: a new top-level page | 3 | 2 | 3 | 1 | 9 | 3 (no free opener; D#4 / GLO / REC taken) | fails (L1: nothing prints it) |
| C: put it on the SET page (tap GLO) | 3 | 2 | 3 | 2 | 10 | 2 | rejected: SET is four tempo-feel knobs with two status lines and has no room for 4 knobs + strip |
| D: two knob rows on the HARMONY page (OCT-/OCT+) | 3 | 1 | 3 | 2 | 9 | 3 (open, row, turn) | fails the 2-gesture rule |

To fit one row of four knobs, the page gives its OCT-/OCT+ pair to the DRONE walk and moves OCTAVE to the same buttons on SET (printed octave, L3). KEY stays on K2 and the 27 keys
become the chord keys (5.3): this costs the "press any key = KEY" convenience and the 27-key picture (owner question 86).

### 5.2 Controls on the HARMONY page (hold GLO, table id SCALE)

| Control | Label | Role | Range / default | Gestures from PERF |
| --- | --- | --- | --- | --- |
| K1 | `SCALE` | global scale (unchanged) | 16 scales, MINPEN | 2 |
| K2 | `KEY` | global key (unchanged; the keys no longer set it) | C..B, C | 2 |
| K3 | `VOICE` | 3.1 | 0..6, 2 | 2 |
| K4 | `MOVE` | 3.2 | 0..8, 8 | 2 |
| OCT- / OCT+ | `OCT-` `OCT+` (tag column shows `DRONE`) | 3.3 drone walk, one candidate per tap | 0..14, 0 | 2 |
| SCL | `PROG` | dice the progression (6) | | 2 |
| LFO | `BARS` | phrase length 1..8 bars, one per press, wraps to the chord count (never fewer bars than chords) | 1..8, 4 | 2 |
| ARP | `ROLE` | the selected channel's ROLE: OFF, AUTO, ROOT, FULL, wrapping | AUTO | 2 |
| SEQ | `NEW` | the next chord key starts a new progression at chord 1 | | 2 |
| keys F3 G3 A3 B3 C4 D4 E4 | `DEG` | type chord 1..7 (5.3) | | 2 (like U54) |
| the other 20 keys | `-` | inert | | |
| FX, ENV, EDIT, GLO, HOME, SAVE, PLAY, REC, SELECT, ALGO, PRESETS | unchanged | FX ALL and ENV ACCNT still dice the selected channel; EDIT also undoes a HARMONY change (7.3) | | |

Everything on the page is one control after the one hold (rule: gesture budget, VISION 1). The lane stays live on HARMONY: it is the SET/SCALE convention (the page does not stop the
music), so turning VOICE or pressing PROG is also a **performance** gesture of one control once the page is open (rows U77, U78).

### 5.3 Typing a progression

The first seven white keys carry the degrees: F3 = 1, G3 = 2, A3 = 3, B3 = 4, C4 = 5, D4 = 6, E4 = 7 (the legend under the strip names them). A key writes its degree at the cursor and moves the
cursor on. **The first key after opening the page, or after `NEW`, starts a new progression** (one chord, then each further key appends one, up to 8; `bars` grows to hold the chords, up to 8;
a 9th key is ignored with the toast `8 CHORDS MAX`). Each key also **sounds** its chord on the selected channel's MIDI channel as a quick ascending arpeggio (80 ms apart, 120 ms long),
so a monophonic synth such as the TB-3 still reveals the chord. So `F3 B3 C4 F3` types `1451`: one open gesture, then the digits, as REC and a line of notes (U54).
Typing alone never changes the sound until the stage is active (typing a single `1` leaves the device inert).

### 5.4 The main-screen chord indicator

While HARMONY is active, the PERF lane header gets a **chord chip** right-aligned just left of `>P2`: the roman numeral of the chord playing now (`I`, `iv`, `V`, `vii`; lower case for a minor
third, 2.4), `T_ACCENT` in the bar where the chord changes. Measured 11 px for `vii`, budget 34 px (widths.tsv). While inert nothing is drawn. On the HARMONY page the strip fills the cell of
the bar that is playing.

### 5.5 Screen and strings (SCREEN_SPEC 7a has the zones)

```
+----------------------------------------+
|> 120 P01  CH1 ACID M2  [1][2]. .  L16  |
| HARMONY  C MINPEN  OCT 0               |  Z2 lane header
|  1   4   5   1       <- bar cells      |  strip: numerals AF_L, playing bar filled
|  C   G   Bb  C       <- chord names    |
|  F3 G3 A3 B3 C4 D4 E4 = 1..7           |  legend
|OCT- |SCALE   |KEY     |VOICE   |MOVE    |  Z3: tag column + K1..K4
|DRONE|MINPEN  |C       |2       |8       |
| DRONE 1 C +0  + - - +                  |  status line 1
| CH1 ROLE AUTO                          |  status line 2
| ALL PROG ACCNT BARS UNDO HARM          |  Z4 row 1
| HOME SAVE ROLE NEW PLAY REC            |  Z4 row 2
| HARM: KEYS F3-E4 = CHORD 1-7           |  Z6 cue
+----------------------------------------+
```

Undo toasts (EDIT tap, shown on every page): `UNDO: PROG`, `UNDO: CHORDS` (a typing run), `UNDO: VOICE`, `UNDO: MOVE`, `UNDO: DRONE`, `UNDO: BARS`, `UNDO: ROLE`; a second tap redoes with the same words prefixed `REDO:`. Toasts added to the screen spec's toast list: `PROG` (dice), `UNDO: PROG`, `NEW: TYPE CHORDS`, `BARS n`, `VOICE n`, `8 CHORDS MAX`, `DRONE OFF`, `RST PROG`. Strings are measured in
`tests/design/widths.tsv` (all inside their budgets).

### 5.6 L rubric score of each new function

Setup scores V/I/O/L (pass >= 10, no sub-score below 2, <= 2 gestures); P rows need >= 11. The table is the same one that U70..U79 add to SEQUENCER_UX 8.2 (checked by `tools/check_sequencer_ux.py`).

| Function | Path | V | I | O | L | Total | Why |
| --- | --- | --- | --- | --- | --- | --- | --- |
| VOICE (U70) | hold GLO, K3 | 3 | 2 | 3 | 2 | 10 | value and chord on screen; the page is named on PERF by `h:HARM`, so L2 by the restated rule |
| MOVE (U71) | hold GLO, K4 | 3 | 2 | 3 | 2 | 10 | as U70 |
| DRONE (U72) | hold GLO, OCT-/OCT+ | 3 | 2 | 3 | 2 | 10 | printed word OCT overridden (decision 48, as on PERF); the drone tone and clash marks are on screen |
| PROG dice (U73) | hold GLO, SCL | 3 | 2 | 3 | 2 | 10 | printed SCL = scale; the cell reads `PROG` |
| Type a progression (U74) | hold GLO, keys | 3 | 2 | 3 | 2 | 10 | digits and key names drawn; every key sounds its chord |
| BARS (U75) | hold GLO, LFO | 3 | 2 | 3 | 2 | 10 | cell reads `BARS`; the strip changes |
| ROLE (U76) | hold GLO, ARP | 3 | 2 | 3 | 2 | 10 | cell reads `ROLE`; status line 2 names the channel and role |
| Perform VOICE / MOVE / DRONE (U77) | page open: K3, K4, OCT | 3 | 3 | 3 | 2 | 11 | one control; the lane stays live |
| Perform PROG (U78) | page open: SCL | 3 | 3 | 3 | 2 | 11 | one control |
| Know the chord (U79) | none (display) | 3 | 3 | 3 | 3 | 12 | chip on PERF, strip on HARMONY |
| Octave (U47, changed) | tap GLO, OCT-/OCT+ | 3 | 2 | 3 | 3 | 11 | printed OCT = octave, on SET |

No exception is needed: every row passes, none needs more than 2 gestures.

## 6. The PROG dice

### 6.1 What it re-rolls

Degrees, bars and chord count, within musical limits; it never touches KEY, SCALE, VOICE, MOVE, DRONE, ROLEs or any channel layer. One press = one deterministic draw from a stream seeded by
the low 16 bits of the audio sample counter at the press (SP13), kept for undo:

- RNG: xorshift32 (13, 17, 5) seeded with `(seed16 << 16 | seed16) ^ 0x4841524D`, `rnd(n) = (next() >> 8) % n` (SP1; salt `HARM`).
- `bars = [2,4,4,4,4,4,8,8][rnd(8)]`; `nchords = 2 + rnd(min(bars,4) - 1)`, so at most **4 distinct chords** (`SQ_PROG_MAX = 4`) and always at least 2.
- each degree: weighted draw over 1..`dmax` (`dmax = min(7, scale size)`) with weights `[2,2,2,4,4,3,1]`; a degree equal to its predecessor is redrawn up to 4 times, then moved to the next degree.
- **Starts or ends on degree 1:** `a = rnd(4)`: `a < 3` sets the first chord to degree 1, otherwise the last chord to degree 1 (a neighbour that would also be 1 becomes degree `min(5, dmax)`).

Golden (xorshift, MAJOR / MINPEN):

| Seed (low 16) | MAJOR (7 degrees) | MINPEN (5 degrees) |
| --- | --- | --- |
| 0x0001 | 4 bars: `1 6 3` | 4 bars: `1 4 5` |
| 0x1234 | 4 bars: `1 5` | 4 bars: `1 4` |
| 0xBEEF | 8 bars: `6 5 1` | 8 bars: `1 5 1` |
| 0xFFFF | 4 bars: `1 6 1` | 4 bars: `1 4 2` |

### 6.2 Undo, reset

EDIT tap swaps the progression with its previous value (7.3). Hold EDIT then touch SCL (the RESET page over HARMONY) puts the progression back to `1---`.

### 6.3 How it relates to the five dice layers plus ALL: **HARMONY-page-only**

It **does not replace** a layer (NOTES, HITS, ACCNT, ARTIC, SHAPE are per-channel and all five stay on PERF), and it **does not join ALL**, because ALL re-rolls "every layer of the selected
channel" (ALGORITHM_SET 4.3) and the progression is global: a PERF `FX` press that re-wrote the chord loop for all four channels would break the invisible default (ruling 2) and the rule
that dice touch one channel's layers. So the PROG dice is the `SCL` button on the HARMONY page only, where its printed word (scale) fits, and the other dice buttons that page does not use
(`LFO`, `ARP`, `SEQ`) become `BARS`, `ROLE`, `NEW`. `FX` (ALL) and `ENV` (ACCNT) keep their channel roles there, so a channel can still be diced while HARMONY is open.

## 7. State, memory, undo, presets, transport

### 7.1 Where the state lives

`HarmonySave` is part of the preset (2.1). Nothing else in HARMONY is saved: the typing cursor, the pending anchor bits and `HarmRT` are runtime.

### 7.2 RAM and pool (ruling 4)

| Item | B | Where |
| --- | ---: | --- |
| `HarmonySave harm_db[2]` | 16 | .bss |
| `harm_gidx` | 1 | .bss |
| `HarmRT`: slot 1, count 1, `vt[6]` 6, pcmask 2, root 1, drone 1, anchor bits 1, epoch 1 | 14 | .bss |
| `HarmUndo`: previous `HarmonySave` 8 + valid 1 | 9 | .bss |
| `undo_last` (0 channel record, 1 harmony record) | 1 | .bss |
| typing cursor (0..8) | 1 | .bss |
| **Total .bss** | **42** | target <= 64 met |
| `HarmonySave` in each of the 18 preset-sized pool copies (16 slots, staging, undo-last-load) | 144 | .pool |

`ChanRT` stays **24 B**, `ChanSave` 344, `UndoRec` 92. Stack: `harm_chord_at_bar` uses `HarmChord` (16 B) plus two 6-byte voicings, under 64 B. Pool spare: 780 - 144 = 636 B before the
fx-bus removal (HANDOFF budget; the removal lands before preset storage, decision 80). RAM headroom after: 39635 B with the removals, 11123 B without (SEQUENCER_UX 9.4).

### 7.3 Undo

One undo record for HARMONY: `HarmUndo` holds the `HarmonySave` before the last change; EDIT tap swaps it with the live one (undo, then redo, as the channel record does). Which record EDIT
swaps is decided by `undo_last`: the most recent change wins, whether it was a channel change (dice, morph, step edit) or a HARMONY change. A "change" is one touch session: all the keys of a
typing run, all the detents of one dial, one PROG press, one BARS press, one ROLE press; touching a different HARMONY control starts a new record. EDIT works on every page (PERF, CHAN, SET, HARMONY), so
the user can leave the page and still take a PROG dice back. A preset load clears `HarmUndo` as it clears every `UndoRec` (ADR-0003 section 10); the load itself is undone by hold SAVE as before.

### 7.4 Presets

Per slot: the progression, bars, VOICE, MOVE, DRONE and the four ROLEs (8 B). A preset load validates and clamps them (2.1) and takes effect with the rest at its bar. A version-2 preset loads as
inert defaults.

### 7.5 Clock and transport

The chord position is `(sq_bar / 768) % bars`; PLAY and the external start reset `sq_bar`, so **PLAY puts the progression at chord 1**. STOP freezes nothing HARMONY owns. RESTRT (F#5) restarts
the channels' steps but not `sq_bar`, so it does not restart the progression; the anchor bits are set again on PLAY and on every chord change. A preset load lands on a bar boundary and the new progression
is read from the same `sq_bar`. An external clock changes only how fast `sq_bar` runs.

### 7.6 MIDI-in chord following: DEFERRED to v2 (ruling 1)

Not built in v1. The hook point is `events_block` (seq.c), where MIDI-in notes already arrive: v2 adds `harm_follow(uint16_t held_pcmask, uint8_t on)` that, while keys are held on the
chosen input channel, replaces the current chord's pitch-class mask and root in `HarmRT` (`HarmRT` reserves `epoch` for the override) until the next bar boundary or the note-off, without
touching `HarmonySave`, so presets, undo and the progression strip stay as they are. Nothing in v1 depends on it: RAM 0 B, no table row, no control.

## 8. Changes to locked docs (each one)

| # | Locked item | Change | Why |
| --- | --- | --- | --- |
| 1 | page_states.tsv `SCALE`: screen, knob band, keys, cue | the 27-key picture replaced by the progression strip; band SCALE KEY VOICE MOVE; keys F3-E4 = chord degree; cue `HARM: KEYS F3-E4 = CHORD 1-7`. Table id stays `SCALE`, screen name `HARMONY` | the one-gesture HARMONY page (ruling 3) |
| 2 | knob_map.tsv `SCALE` K2/K3/K4 | K2 `KEY` text drops "press any key"; K3 `OCTAVE` becomes `VOICE`; K4 inert becomes `MOVE` | |
| 3 | button_modes.tsv + control_map.tsv `SCALE` | `SCL` `PROG`, `LFO` `BARS`, `ARP` `ROLE`, `SEQ` `NEW`; `OCT-`/`OCT+` = DRONE walk; GLO `BACK` text | 5.2 |
| 4 | `SET`: `OCT-`/`OCT+` | inert becomes global OCTAVE -/+1 (page_states SET, SCREEN_SPEC 3) | OCTAVE had to leave the SCALE page |
| 5 | `h:SCALE` -> `h:HARM`, `B_GLO` hold label `SCALE` -> `HARM`, SET/PERF cue `hold=HARM` | PERF button picture, SEQUENCER_UX 2.1, 2.4, 3, SCREEN_SPEC | the hold must be named (R-8) |
| 6 | `B_EDIT` tap on PERF, CHAN, SET, SCALE | also undoes the last HARMONY change when it came later | 7.3 |
| 7 | `RESET` rows `SCL`, `OCT-`, `OCT+` | from HARMONY: progression to `1---`; DRONE OFF | |
| 8 | key_map.tsv `SCALE` | 27 rows: W01..W07 `DEG`, the other 20 keys `-` (were all `KEY`) | 5.3 |
| 9 | help_text.tsv (generated by tests/design/gen_help.py) | the SCALE and SET rows above, all within 34 characters and 224 px (`check_help.py` OK) | |
| 10 | SEQUENCER_UX | revision 3 note; 1.7, 2.4, 3, 4.1, 4.3 rows; U46, U47 paths; U70..U79; totals (79 functions, 76 pass); 9.1 `SeqPreset` 1396 B; 9.4 RAM +42; 9.5 pool +144 | |
| 11 | SCREEN_SPEC | rows in 3, 7, new 7a HARMONY screen; chord chip | |
| 12 | ADR-0003 | new section 20 (chord stage, `HarmonySave`, `AlgoDesc.hi`); `ChanRT` unchanged | |
| 13 | ALGORITHM_SET | new section 12 (SP18 chord stage, per-algorithm table) | |
| 14 | scenarios | S07 steps 1, 6, 11, 13..17 and S01 / S06 sub-label updated; S18 added; COVERAGE regenerated (S18 counts as a narrative walk) | |
| 15 | Stale text fixed after review | page_states.tsv CHAN opener (`on SCALE D#4 is inert`), CONTROL_MAP.md workflow row 6, and a "superseded by HARMONY.md 5 (P2-012)" note on UX_REVIEW.md W9 and PERFORMANCE_DESIGN.md F18 (history kept, not rewritten) | |
| 16 | Files changed beyond the rows above | CONTROL_MAP.md appendix A (50 rows, kept equal to control_map.tsv); scenarios S09, S10, S11, S12, S13, S15 (regenerated sweeps), S01, S06, S07 (hand edits), S18 (new), COVERAGE.md; tests/design/gen_help.py (HARMONY overrides), check_scenarios.py (S18 counts as a narrative walk), widths.tsv (HARMONY strings), harmony_proto.py (new); OWNER_QUESTIONS.md 85-91 | |
| 17 | VISION | **unchanged**; HARMONY serves rules 3 and 4 (the algorithm drives the sequence; knobs morph). The one tension is the per-channel ROLE, a fourth channel setting stored on the global page: owner question 89 | |

## 9. The chord and voicing query interface (for P2-013 ARP and P2-014 PATTERNS)

Both tasks call only this; they never read `HarmonySave` directly. All functions are pure (no globals written), integer-only, safe from the ISR and the main loop; cost is bounded by
8 slots x 18 candidates.

```c
#define HARM_MAXV 6
typedef struct {
    uint8_t  active;          /* 0 = HARMONY inert (1.1): callers should behave as before */
    uint8_t  slot, n;         /* chord index 0..7 and the number of voiced notes in vt[] (1..6) */
    uint8_t  deg;             /* degree 1..7 */
    int8_t   root;            /* chord root, semitones above the key (0..11 + octave carry) */
    int8_t   drone;             /* drone pitch, semitones above the key, or -1 */
    int8_t   vt[HARM_MAXV];   /* voiced notes ascending, semitones above the key (VOICE + MOVE + drone) */
    uint16_t pcs;             /* 12-bit pitch-class set of the chord tones and the drone, bit k = key+k */
} HarmChord;                  /* 16 B */

uint8_t  harm_active(void);                                   /* ruling 2: false for the default state */
void     harm_chord_at_bar(uint16_t bar, HarmChord *out);     /* the chord of bar number `bar` (any size; taken % bars); builds the voice-leading chain */
void     harm_chord_now(HarmChord *out);                      /* the chord of the current bar (reads HarmRT; ISR) */
uint16_t harm_ticks_to_change(uint32_t sq_bar_ticks);         /* ticks until the next chord starts (0..bars*768) */
int16_t  harm_snap(const HarmChord *c, int16_t s, uint8_t mode); /* mode HQ_PC: nearest chord tone, all octaves; HQ_VOICED: nearest exact voiced note;
                                                                    HQ_ROOT: chord root in the octave of s; ties lower; result clamped to 0..96 */
int16_t  harm_tone(const HarmChord *c, uint8_t idx);          /* idx-th chord tone upward: vt[idx % n] + 12 * (idx / n); for chord-relative patterns */
```

- **ARP (P2-013)** walks `harm_tone(c, k)` for its CHORD/ORDER/OCT figure over `harm_chord_now()`, re-reads it at `harm_ticks_to_change()`, and uses `n` as the pool size; `HQ_VOICED` gives it MOVE's exact placement.
- **PATTERNS (P2-014)** stores notes as scale degrees or as chord-tone indices and resolves them through `harm_snap` / `harm_tone`, so a stored pattern is quantised to the current chord at play time (and is
  unchanged when `harm_active() == 0`).
- Both must treat `active == 0` as "no chord stage" and stay byte-identical to their own golden output in that state (the H-INERT rule of section 10 applies to them too).

## 10. Acceptance for the dev tasks, golden files and risks

- **H-INERT:** default HARMONY state gives byte-identical MIDI for all four algorithms against the P2-008 golden; ROLE OFF on every channel gives byte-identical MIDI for any progression.
- **H-GOLDEN:** the tables of 3.1, 3.2, 3.3, 4.2 and 6.1 are compared bit for bit with `harmony_proto.py` (the C implementation is tested against the Python).
- **H-RAM:** 42 B .bss; `ChanRT` 24 B; the headroom line is reported on every dev review.
- **H-UNDO:** EDIT swaps the later of the channel and harmony records; a preset load clears both.
- Risks: the drone walk by two buttons is less direct than a knob (accepted, the owner asked for "walk through candidates"); AUTO's pull is a seeded-free hash, so identical positions repeat
  identically each pass (by design: rule 4); a monophonic synth hears only the last note of a chord audition unless it is an arpeggio (hence 5.3); the label `PEDAL` is kept only for ACID's ROW 2
  morph, which is why the HARMONY dial is called `DRONE` (owner decision 87B).

## 11. Owner decisions (2026-11-10, approved; OWNER_QUESTIONS 85-89 and 91)

85A: default phrase length 4 bars (`1---`). 86A: chord keys replace press-a-key-sets-KEY and OCTAVE moves to SET (partly reverses decision 69). 87B: the HARMONY dial is named `DRONE`; ACID's ROW 2 morph
keeps `PEDAL`. 88A: FULL is monophonic. 89A: ROLE is a harmony-page setting. 91A: EDIT also undoes the last HARMONY change (extends decision 46).
