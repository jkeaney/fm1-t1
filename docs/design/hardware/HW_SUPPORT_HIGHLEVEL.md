# HW_SUPPORT_HIGHLEVEL: the hardware support model, phase 1 (high-level design and UX)

Task P2-018 (owner decision 96, 2026-11-11). Status: **for owner review** (gate `human-review`). Design only: no firmware, no
tests, no `control_map.tsv` rows (they are proposed in section 12). **Phase 1 only:** this is the model and the UX. The low-level
design (wire formats, state machines, code) is phase 2; the TB-3 profile and its settings are phase 3. Nothing here is locked until
the owner writes "Approved" in the P2-018 log, and then "Design locked" per ADR-0004.

Tags follow the repo convention: **[V]** verified (in a document or on hardware), **[I]** inferred, **(warning)** unconfirmed, a hypothesis
to test on the device (the H-tests of section 10).

Inputs read: CLAUDE.md, VISION.md, PROCESS.md, SEQUENCER_UX.md (1, 2, 8.1, 9), ALGORITHM_SET.md (4), HARMONY.md, ARP and PATTERNS
SPEC.md, KEYBED_USE_CASES.md, SCREEN_SPEC.md, control_map / knob_map / key_map / page_states, TIMING_SPEC.md, ADR-0003,
OWNER_QUESTIONS.md, `hardware/tb3-midi-guide.csv`, midi.guide, and the CDM article on the AIRA CC list.

## 1. The problem and the opportunity, in plain words

**Today** a channel sends notes: pitch, velocity, length, and (through the core) a slide. Everything the producer does to the sound of the
device (open the filter, push the resonance, change the accent amount) is done by hand on the device, away from the FM-1. The dice and morph
controls make the notes alive; the sound stays still. A bassline on a TB-3 lives half in the notes and half in the filter, so the sequencer
is only half a performance tool.

**The opportunity.** The device listens to MIDI, and the FM-1 already knows when each step happens, which steps are accented, and which dice
layer was just re-rolled. Sending a few control changes (CC) *with* the notes lets the same gestures move the sound: dice ACCNT and the
filter opens on different steps; turn a knob and the cutoff moves. No new concept for the player: **the buttons still dice, the knobs
still morph, and now they can reach the device as well.**

**The hard part is not the plumbing, it is control.** A sequencer that silently writes CCs fights the player's own hand on the device.
The whole UX therefore rests on one question the screen must always answer, per parameter: **who is in control, the device, you, or the
sequencer?** Section 2.2 gives that three words, section 6 shows them, and setting them is two gestures at most.

**Rules this design must keep** (VISION): UX first and no menu diving (1); a sequencer, not a synth (2: this adds control of an *external* device, never sound design on the FM-1); a channel is a MIDI channel, an
algorithm and a length (3: the rig is a small, optional addition; it needs the owner's nod, question A); algorithms are dialled in with
knobs and dice (4); a plain sequencer with no device chosen is unchanged.

## 2. The model: concepts and names

### 2.1 Vocabulary

| Name | What it is | Where it lives |
| --- | --- | --- |
| **PROFILE** | A description of a device: its name, how it wants accents and slides asked for, and up to 16 named **PARAMS** (name, CC number, range, a neutral `home` value). Read-only data. A built-in `GENERIC` profile has no names: the user types a raw CC number. | flash table (section 8) |
| **RIG** | A channel's choice of device: one profile id plus up to **4 WIRES**. Part of the channel and of the preset, like LENGTH. Rig = "what this channel is plugged into". | `ChanSave` (section 7) |
| **WIRE** | One device parameter the sequencer may touch: a CC number (its name comes from the profile), a **STATE**, a **LEVEL**, a **MOVE** depth, a **LANE FORM** and the **dice layer** that re-rolls it. 4 per channel, because there are 4 knobs. | the rig |
| **STATE** | Who is in control of the wire: `DEV` (the device; the FM-1 never sends it), `HAND` (you: it sends only when you turn its knob), `SEQ` (the sequencer: a lane moves it around your LEVEL on every step). | the wire; shown everywhere (section 6) |
| **LEVEL** | The wire's resting value 0..127: the value your knob sets, and the centre the lane moves around. The sequencer's version of the device's own knob position. | the wire |
| **MOVE** | How far the lane moves away from LEVEL, 0..127. 0 = the lane is silent. | the wire |
| **LANE** | The per-step value stream of a wire in state `SEQ`, **computed, never stored**: `value(p) = LEVEL + MOVE x lane(p)`, clamped to the param's range. The lane is a pure function of (the dice layer's seed, the step position, the lane form), so it is deterministic (VISION rule 4) and costs no RAM per step. | the core (rig_eval) |
| **LANE FORM** | The shape of the lane: `RAND` (a fresh value per step), `WALK` (a random walk, smooth), `RAMP` (rises over the loop, the classic filter sweep), `ACC` (follows the resolved accent: open on accented steps). Two bits. | the wire |
| **RIDE** | Optional: a wire's MOVE is scaled by one of the channel's eight morphs (for example PUNCH), so a morph knob you already know also deepens the filter movement. 0 = none. | the wire |
| **PLOCK** | A per-step lock of one wire's value (Phase 2 idea, not v1; section 12, question I). | StepEdit extension |
| **LANES beyond note** | The pattern already carries three: **accent** (velocity at or above the device threshold), **slide** (glide into the next step, SEQUENCER_UX 1.1), **gate**. The rig adds up to four **CC lanes**. The *profile* says how accent and slide are sent (section 8.3), so algorithms never learn about devices. | PulseOut (existing) + rig lanes |

**The key architectural choice: algorithms stay oblivious.** Euclid, Acid, Snake, Stage, Arp and Patterns keep emitting `PulseOut`
exactly as locked (goldens unchanged). The rig is evaluated *beside* `sq_eval` (SEQUENCER_UX 9.2) from the **same layer seeds** that
ALGORITHM_SET 4.2 already binds to the five dice buttons. A wire "rides" a layer by hashing that layer's seed with the wire number and
the generator position `g(p)`. So **dice ACCNT re-rolls the accent steps and, in the same press, whatever wire rides ACCNT** with no
algorithm change and no new seed. A layer an algorithm lacks (EUCLID has no SHAPE) is dimmed (ALGORITHM_SET 4.4): a wire riding it freezes
its lane, honestly, and the sub-label shows it.

### 2.2 The three control states (the answer to "what is in control")

| STATE | Word on screen | Chip | The FM-1 sends | The player's mental model |
| --- | --- | --- | --- | --- |
| Device | `DEV` | `-` | nothing, ever | "I turn that knob on the box." |
| Hand | `HAND` | `H` | the value when *you* turn its FM-1 knob; nothing at PLAY or on a dice | "The FM-1 knob is a remote for the device's knob." |
| Sequencer | `SEQ` | `~` | LEVEL + lane on every step; re-sent at PLAY; parked at STOP | "The sequencer plays this knob; I steer it with LEVEL, MOVE and dice." |

Why three and not two: `HAND` is what makes the feature safe to try. Choosing a device and turning a knob must not start a light show;
`SEQ` is one more tap and is a deliberate act. `DEV` is the default for every wire and is exactly today's behaviour.

## 3. How the controls reach the device

| Source | Binds to | Behaviour | Gestures |
| --- | --- | --- | --- |
| **Morph knob row (K1..K4)** | A new **ROW 3 "RIG"** of the knob band: K1..K4 = LEVEL of wires 1..4, labelled with the device's param names (`CUTOFF`, `RESO`). The knobs are relative detents (CLAUDE.md), so there is no pickup jump when the row changes [V]. | A turn sends a CC at once (HAND) or moves the lane's centre from the next step (SEQ). | Same as ROW 2: OCT+ tap, then turn (a 2nd OCT+ tap reaches ROW 3 only when the channel has a wire that is not `DEV`). |
| **Morph by RIDE** | MOVE of a wire scales with one of the 8 morphs. | Turning PUNCH changes the notes *and* how far the filter wanders. Shown by a `~` on the morph's cell. | 0 new: the knob the user already turns. |
| **Dice layer (SCL ENV ARP SEQ LFO, FX = ALL)** | Each wire rides one layer (or none). | The press re-rolls the layer's seed: the notes layer *and* the lanes riding it. EDIT tap undoes both together (seeds are the undo unit already, ALGORITHM_SET 4.3). | 1 (an existing press). |
| **Step p-lock** | One wire's value on one step. | Beats the lane for that step (like a StepEdit field). **Not in v1** (question I); the data hook is reserved (section 7, 11). | 2 (hold step, turn) |
| **Performance gesture** (FILL, ROLL, SOLO) | A wire may take an offset while the gesture is held, restored on release. | Parked as a v2 idea: KEYBED_USE_CASES 3 dropped pressure-to-CC for v1 because an absolute CC cannot be undone; a *held offset with restore* fixes that, and is the reason RIG keeps a home value. | 1 (hold) |

### 3.1 The five dice layers and the K1..K4 / S1..S4 morphs, concretely

Illustrative wiring for a filter-and-accent synth (the real defaults are phase 3):

| Dice button | Algorithm effect (locked) | A wire riding it could be | Musical result |
| --- | --- | --- | --- |
| `ALL` (FX) | every layer present | all wires | a whole new phrase *and* a new filter motion |
| `NOTES` (SCL) | new pitches | a slow `WALK` on cutoff | the line and the timbre drift together |
| `HITS` (SEQ) | where the rhythm falls | rarely used | (usually none) |
| `ACCNT` (ENV) | which steps are loud | cutoff or accent-amount in form `ACC` | accents open the filter: the 303 gesture |
| `ARTIC` (ARP) | gate and slide | resonance `RAND` | squelch changes with the slide pattern |
| `SHAPE` (LFO) | register jumps, slot lengths | cutoff `RAMP` | a sweep that restarts on the new shape |

Morphs: `RIDE` is chosen from the algorithm's own eight morph names, so the cell on ROW 1 / ROW 2 reads `PUNCH 40~`. Switching algorithm changes
which morph a slot means, so RIDE binds to the **slot (0..7), not the name**, and the `~` follows the slot; the algorithm change rule is in section 9.

## 4. What the player sees and does: the one-paragraph version

Pick a device for the channel (hold `CHN`, turn the ALGO encoder): four wires appear in `HAND`, named after the device. Turn **ROW 3**
and the cutoff moves. Tap a wire's white key on the RIG page once and it becomes `SEQ`: it now moves with the music, and the ACCNT
dice re-rolls it. Tap again and it is `DEV` again: the FM-1 lets go. With no device chosen the whole thing does not exist on screen.

## 5. Failure first: how this goes wrong (and the rule that prevents it)

| Failure | Cause | Rule |
| --- | --- | --- |
| Device ignores the CC | wrong MIDI channel, device in a mode that ignores CC, a CC the device does not have (warning: TB-3 list is unofficial, section 8) | The FM-1 cannot know [V: MIDI out is one way]. The wire shows what it *sent* (`out`), never what the device did; first-use cue `RIG SENDS ONLY: CHECK THE DEVICE`; H-test H1. |
| Slide semantics | a slide is a *note* property (overlap), not a CC; the TB-3 may not accept it from MIDI (warning; section 8.3) | Slide stays in the note lane, with the profile's `slide` method. If the profile says `NONE` the lane dims the `~` glyph on that channel so the promise is honest. |
| CC flood | 4 channels x 4 wires x every step, plus HAND turns; the 128-entry ring (TIMING_SPEC 3) and the DIN link (31.25 kbit/s, about 1000 three-byte messages/s, about 1 ms each [V: MIDI spec]) | Send **only on change**; lane CCs stamped just before their note; HAND turns coalesced to at most 1 per 10 ms per wire; a global cap of 8 `SEQ` wires; CCs drop **before** notes (ring reserve below). |
| Stuck values | STOP in the middle of a sweep leaves the filter at the last lane value | **Park:** at STOP the FM-1 sends each `SEQ` wire's LEVEL once, so the device rests where the knob says. `HAND` and `DEV` wires are untouched. |
| Overwriting the player's hand | the player turned the device knob, then PLAY re-sends | `HAND` never sends at PLAY; only `SEQ` does. Choosing `SEQ` is the consent. |
| Knob positions lost | the FM-1 cannot read the device's knob | LEVEL is the FM-1's own memory, saved in the preset. On load/PLAY it is sent only for `SEQ`. |
| Wrong device | the user picks a profile that does not match | one tap of the wire key returns the wire to `DEV`; RESET R3 puts the row back to profile defaults. |

**Ring rule (proposed amendment of the TIMING_SPEC reserve rule, hook in P4-003):** with fewer than 24 free entries new CCs are dropped
(counted); with fewer than 8, NOTE_ONs are dropped as today; NOTE_OFF, CC123 and clock are always kept. A dropped lane CC is stale by the next step
anyway; a dropped `park` or `PLAY` CC is retried (the wire stays "dirty" until sent).

## 6. UX options (three, distinct)

All mock-ups are 40 columns of about 6 px, like SEQUENCER_UX 2; P2-003 owns pixels. Device and param names are illustrative. Every
string was width-checked with `tests/design/width_probe.sh` (section 13). Gesture counts are from PERF. L-scores use the rubric of
SEQUENCER_UX 8.1: V (visual), I (input), O (output), L (learnable); performance functions need 11 of 12, setup functions 10 of 12, no
sub-score below 2, and at most 2 gestures (setup) or 1 (performance, owner ruling 2026-11-10). Scores are the designer's, not machine-checked.

The functions scored: **R1** choose the device (setup), **R2** set who is in control of a wire (setup), **R3** pick which parameter a wire
drives (setup), **R4** move a device parameter live (performance), **R5** dice the device lanes (performance), **R6** see what is in control
at a glance (performance, no input).

### 6.1 Option A: RIG PAGE (a patch bay; everything on one page)

Hold black key `CHN` (D#4) to open the **RIG page**. All device work happens there; PERF is unchanged except a header chip.

```
+----------------------------------------+
|> 120 P01  1 ACID M2  [1][2]. .   L32   |
| RIG   CH1   DEVICE: SYNTH-A  (ALGO)    |
| 1 CUTOFF  74  SEQ   ACCNT  RAND        |  one row per wire: key, name, CC, STATE, layer, form
| 2 RESO    71  HAND  -      -           |
| 3 ENVMOD  12  DEV   -      -           |
| 4 ACCENT  16  DEV   -      -           |
|LEVEL 88   |MOVE 40 |LAYER   |FORM      |  K1..K4 = wire 1 live: LEVEL MOVE LAYER FORM (wire = last key tapped)
|   ... buttons as PERF ...              |
| M1 M2 M3|M4 BCK|SOL FIL ROL|PG- PG+|RST|
| RIG: KEYS 1-4 = WHO DRIVES  HOME=back  |
+----------------------------------------+
```

White keys 1..4 cycle `DEV -> HAND -> SEQ -> DEV`; the last key tapped selects the wire for K1..K4. Live LEVEL needs the page open.

| Function | Gestures | V | I | O | L | Total | Verdict |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| R1 choose device | 2 (hold CHN, turn ALGO) | 3 | 2 | 3 | 2 | 10 | pass (setup) |
| R2 who is in control | 2 (open, tap key) | 3 | 2 | 3 | 2 | 10 | pass |
| R3 pick a parameter | 3 (open, tap key, turn K) | 3 | 1 | 3 | 2 | 9 | fails the 2-gesture bar |
| R4 move a param live | 2 (open the page, turn) | 3 | 2 | 3 | 2 | 10 | **fails** performance bar (11, 1 gesture) |
| R5 dice the lanes | 1 | 3 | 3 | 3 | 2 | 11 | pass |
| R6 see who is in control | 0 | 2 | 3 | 2 | 2 | 9 | fails |

Strength: tidy, one place, nothing on PERF to learn. Weakness: the filter is a *performance* control and it is two pages away; the
player leaves the main screen, which VISION rule 4 and 8.3 try to avoid.

### 6.2 Option B: RIG ROW (a third knob row on PERF, plus the page for setup) - RECOMMENDED

The knob band gains **ROW 3 = RIG**: LEVEL of wires 1..4, labelled with the device's names, with a state chip on each cell
(`~` SEQ, `H` HAND, `-` DEV). It is reached like ROW 2 (OCT+ tap; the *second* OCT+ tap, only when a wire is not `DEV`). The RIG page is kept for setup
(parameter, layer, form, MOVE, RIDE, state), but **the state can also be flipped without leaving PERF** through the RIG page's shortcut (hold `CHN` and tap a key) and every performance gesture lives in the band.

PERF with ROW 3 live:

```
+----------------------------------------+
|> 120 P01  1 ACID M2  [1][2]. .  L32 ~  |  header chip ~ = a wire is SEQ on this channel
| [1 ACID] 2 EUCL m  3 SNAK m  4 STAG m  |
| PG 1/2  FOLLOW  hold=EDIT        >P2   |
| #  .  #' #  .  #! .  #~ #  .  #  .  #x2|
|R3|CUTOFF 88~|RESO 40H|ENVMOD -|ACCENT - |  ROW 3 live: name, value, chip   K1..K4
|1 |DENS 12   |SLIDE 4 |JUMP 5  |ACCNT 3~ |  ROW 1 dim; the ~ on ACCNT = a lane rides this layer
| ALL  NOTES ACCNT~ SHAPE UNDO  SET      |
|  -   notes ACCNT~ OCTAVE h:RST h:HARM  |  sub-label ACCNT~ : the press also re-rolls a wire
| HOME SAVE  ARTIC HITS  PLAY  REC       |
| M1 M2 M3|M4 CHN|SOL FIL ROL|PG- PG+|RST|
| ROW 3 = DEVICE KNOBS  OCT- = ROW 1     |
+----------------------------------------+
```

RIG page (hold `CHN`; one wire selected by its white key):

```
+----------------------------------------+
|> 120 P01  1 ACID M2  [1][2]. .   L32   |
| RIG  CH1  SYNTH-A   (ALGO = DEVICE)    |
| 1 CUTOFF  SEQ   2 RESO  HAND           |
| 3 ENVMOD  DEV   4 ACCENT DEV           |
|PARAM CUT|LAYER AC|FORM ACC|MOVE 40     |  K1..K4 of the selected wire
|  RIDE-  PUNCH   RIDE+  (OCT- / OCT+)   |
| ... buttons as PERF ...                |
| M1 M2 M3|M4 BCK|SOL FIL ROL|PG- PG+|RST|
| RIG: KEYS 1-4 = DEV/HAND/SEQ  HOME=back|
+----------------------------------------+
```

White keys 1..4: tap = cycle that wire's state and select it; the cue strip names it. On first use of a device (`DEV` -> `HAND` happens automatically for the
profile's four suggested params when the device is chosen; section 12, question J).

| Function | Gestures | V | I | O | L | Total | Verdict |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| R1 choose device | 2 (hold CHN, turn ALGO) | 3 | 2 | 3 | 2 | 10 | pass |
| R2 who is in control | 2 (hold CHN, tap key 1..4) | 3 | 2 | 3 | 2 | 10 | pass |
| R3 pick a parameter | 2 (hold CHN, turn K1 of the wire the key selected) | 3 | 2 | 3 | 2 | 10 | pass |
| R4 move a param live | 1 once ROW 3 is live; else 2 as every ROW 2 morph | 3 | 2 | 3 | 3 | 11 | pass (same score as the ROW 2 morphs) |
| R5 dice the lanes | 1 | 3 | 3 | 3 | 2 | 11 | pass (sub-label `~` teaches it) |
| R6 see who is in control | 0 | 3 | 3 | 3 | 2 | 11 | pass |

Strength: the device's knobs are where the morphs are, with the same gesture, the same labels, the same dice. The setup page is
small and used rarely. Weakness: a third row means a third OCT+ meaning (amends SEQUENCER_UX 4.2, question B) and a held `CHN` (new hold on a black key, question C).

### 6.3 Option C: RIDERS (no new row, no new page of its own; the device rides the controls you have)

No LEVEL knobs and no ROW 3. A wire is *attached to a control the player already uses*: a morph (its value scales the CC) and/or a dice layer
(its seed makes the lane). Setup is a short list on the CHAN page (second screen of it): each of the eight morphs and five dice layers can be
tagged with one device param.

```
+----------------------------------------+
|> 120 P01  1 ACID M2  [1][2]. .   L32 ~ |
| CHANNEL 1  ACID  DEVICE: SYNTH-A       |
| RIDERS (OCT+ = next list)              |
| DENS --   |SLIDE --|JUMP --  |ACCNT  ACC|  K1..K4 = what each ROW 1 morph also drives
| PUNCH CUT |PEDAL --|OFFSET --|DIR  --   |  (dim row = ROW 2)
| layers: NOTES -  HITS -  ACCNT AMT      |
|   ... buttons as PERF ...              |
| RIDERS: TURN K = PICK PARAM  HOME=back |
+----------------------------------------+
```

PERF shows a `~` on any morph or dice label that also drives the device; there is no per-wire state, only a channel switch `RIG ON/OFF`.

| Function | Gestures | V | I | O | L | Total | Verdict |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| R1 choose device | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R2 who is in control | 2 (the whole rig on/off only) | 2 | 2 | 2 | 2 | 8 | fails: no per-parameter ownership |
| R3 pick a parameter | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R4 move a param live | 1 (the morph you know) | 2 | 3 | 2 | 1 | 8 | **fails**: the device param is not named on the knob, and the note morph and the filter move together |
| R5 dice the lanes | 1 | 3 | 3 | 3 | 2 | 11 | pass |
| R6 see who is in control | 0 | 2 | 3 | 2 | 2 | 9 | fails |

Strength: smallest surface; no row, no hold. Weakness: **coupling**. You cannot move the cutoff without moving the notes, which is exactly the
loss of control the problem section warns about. It cannot express `HAND` at all.

### 6.4 Comparison and recommendation

| | A: RIG PAGE | B: RIG ROW | C: RIDERS |
| --- | ---: | ---: | ---: |
| Functions at or above their bar | 3 of 6 | 6 of 6 | 3 of 6 |
| Sum of totals (max 72) | 59 | 63 | 56 |
| Live device control | page change | 1 gesture, named knobs | coupled to notes |
| "In control or not" in 2 gestures | yes | yes, and visible on PERF | no (channel-wide only) |
| New things to learn | 1 page | 1 row + 1 page | 1 list |
| Changes a locked decision | 1 (hold on CHN) | 2 (hold CHN, third row) | 0 |

**Recommendation: Option B.** It is the only option where the device's own knobs behave like the morph knobs (same gesture, labelled, undone
by turning back) and where "who is in control" is on the main screen at all times; it spends two small, listed amendments of the locked
design (questions B and C) to do it, and its setup page doubles as Option A, so nothing is thrown away if the owner prefers the page.

## 7. Data model sketch and byte costs

```c
/* One device parameter a channel may drive. Saved: 6 B. The NAME and range come from the PROFILE, so a CC survives a profile change. */
typedef struct {
    uint8_t cc;       /* 0 = empty wire, else 1..127 (the control change number sent) */
    uint8_t cfg;      /* bits0..1 STATE 0 DEV 1 HAND 2 SEQ; bits2..3 FORM RAND/WALK/RAMP/ACC; bits4..6 LAYER 0 none, 1..5 NOTES..SHAPE; bit7 spare */
    uint8_t level;    /* 0..127 resting value (profile `home` after RESET) */
    uint8_t move;     /* 0..127 lane depth */
    uint8_t ride;     /* 0 none, 1..8 = morph slot that scales MOVE */
    uint8_t spare;    /* reserved: PLOCK flag / the wire's lane multiplier (phase 2) */
} Wire;
typedef struct { uint8_t prof; uint8_t nw; Wire w[4]; uint8_t pad[2]; } Rig;   /* 2 + 24 + 2 = 28 B */
_Static_assert(sizeof(Rig) == 28, "Rig");
```

`ChanSave` (SEQUENCER_UX 9.1, 344 B) gains `Rig rig` (28 B): **372 B**; `SeqPreset` (1396 B with HarmonySave) becomes **1396 + 4 x 28 = 1508 B**.

| Item | Where | Bytes | Note |
| --- | --- | ---: | --- |
| `Rig` in the live `ChanSave`, x4 | .bss | 112 | the whole saved model |
| Rig runtime per wire: last sent value (1), last-sent tick (2), dirty/parked bits (1), x16 wires | .bss | 64 | the rate limiter and the park/PLAY re-send |
| Rig UI state: selected wire, page open, ROW 3 latch, first-use cue, a per-device state chip cache | .bss | 8 | inside the 3072 B UI allowance |
| Undo | .bss | 0 | seeds are the undo unit; LEVEL/MOVE/state edits are not undone by EDIT (question H) |
| **Total new .bss** | | **184** | against headroom ~17 KB now (11123 B after the full SEQUENCER_UX build before removals, SEQUENCER_UX 9.4) and ~46 KB after the planned removals: 184 B is 1.1 % of today's headroom and does not move the 8 KB trigger |
| `SeqPreset` +112 B in each of the 18 preset-sized copies (16 slots + staging + undo-load) | pool | 2016 | **prerequisite: the fx-bus removal (R6) lands first.** Pool free today 25764 B; the SEQUENCER_UX set needs 25128, so +2016 does not fit until R6 (192847 B free after); with `SQ_NSLOT` = 8 it fits (the same escape as SEQUENCER_UX 9.5) |
| PLOCK array (v1.1): 64 B per channel for one wire | .bss / pool | 256 / +4608 | not in v1; fits after R6 only |
| Built-in profile table: 6 profiles x (16 params x 12 B + 16 B header) | flash | about 1250 | plus strings, about 600 B; flash freed by removals is more than 140 KB |
| CPU | ISR | about 20 integer ops per wire per step (a hash and a clamp) | 16 wires = 320 ops per step instant, well inside the +5 % of ADR-0003 |

Message budget (the CC rate): `SEQ` lane CCs change at most once per step: 4 channels x 4 wires x 8 steps/s (120 BPM, 1/16) = **128 CC/s worst
case, 13 % of a DIN link** and 4 x 4 x 8 / 1000 of a USB frame budget. A burst of 16 CCs at one step boundary smears 16 ms on a DIN host box, so
lane CCs are **stamped ahead of their note and spread** (one CC per tick, wire k at `due - 2(k+1)` ticks of 100 us), with the note-on at its own stamp, and the
global cap of 8 `SEQ` wires bounds the burst to 8 ms. `HAND`: 1 CC per 10 ms per wire, last value wins.

## 8. Where device profiles come from

### 8.1 Options

| Source | What | For | Against |
| --- | --- | --- | --- |
| **Built-in table in flash** (recommended v1) | 5 or 6 profiles compiled from `tools/` data we write: TB-3 first, a few popular synths, plus `GENERIC` (raw CC number) | zero setup; names fit the 51 px knob cell; reviewed once; the model is exercised on real hardware | only the devices we write; adding one is a firmware release |
| **User-defined on the device** | `GENERIC` plus a typed CC number per wire | works with any gear, no data | numbers instead of names; 2 gestures per wire |
| **Import a midi.guide-style CSV** | a host tool turns the open CSV into a profile table appended to the firmware package (`tools/fm1_profile.py`, then the normal install) | any device in the database; the data is open | needs a host and a flash cycle (no SD slot, USB-C is device-only); **licence** (8.2); the CSV has no accent/slide semantics, no `home`, and ranges can be wrong |
| **Learn from the device** | listen for CCs on MIDI IN and bind them | no tables | the jack is MIDI IN only and the TB-3 sends its knob turns only on its own USB/DIN out (warning: not a path the FM-1 can read from the TB-3 directly); not for v1 |

Recommendation: built-in table for v1 (A), `GENERIC` as the user-defined path, CSV import as a later host tool (phase 3 or v2).

### 8.2 Licensing flag (owner question E)

The midi.guide database (repo `pencilresearch/midi`) is **CC BY-SA 4.0**; this firmware is **GPL-3.0**. Share-alike plus attribution
terms mean copying that CSV into the firmware image, even converted, may impose licence terms that the GPL does not satisfy, and the
compatibility of CC BY-SA 4.0 data inside a GPL-3.0 binary is not something this design can settle (warning: needs a decision, not an assumption).
CC numbers themselves are facts [I]; the safe route is to write our profile data from the device's own MIDI implementation chart or
from our own tests on the hardware, and to cite midi.guide only as a cross-check. **This design copies nothing.** The repo already holds
`docs/design/hardware/tb3-midi-guide.csv` for reference (documentation only, not built into any image): it should carry the CC BY-SA attribution in the new
`hardware/README.md`, and must not be read by `gen_ui_tables.py`.

### 8.3 Accent and slide semantics: what is and is not known

| Claim | Status | Source |
| --- | --- | --- |
| The TB-3 responds to CC 74 (cutoff), 71 (resonance), 16 (accent), 17 (effect), 12 / 13 (env mod X / Y), 1 / 11 (XY), 68 / 69 (scatter), 104 (tuning), 102 / 103 (pattern slide / accent) | **[V]** that these appear in the midi.guide table and in the CDM article; **(warning)** that every one works as a *receive* on every firmware | `tb3-midi-guide.csv`; CDM (the author calls the list unofficial and possibly incomplete) |
| The note velocity decides the TB-3 accent | **(warning)** existing hypothesis H0 (threshold near 100), decision 95: tested by ear in the P0-005 session | OWNER_QUESTIONS 95; KEYBED_USE_CASES |
| TB-3 slide can be produced from MIDI notes (overlapping notes) | **(warning)** unconfirmed; the existing core assumes the 303-style overlap (ADR-0003 7.3) | no source |
| "You can't transmit slide changes when playing patterns; slide is recorded in step mode but not in real time" | **[V]** that the CDM author says so and says Roland confirmed it; **[I]** that it describes the TB-3 as a *sender* (recording out), not whether it *accepts* slide from incoming notes | CDM article |
| CC 102 / 103 (pattern slide / accent) act on the TB-3's internal pattern, not on incoming notes | **[I]** from the names | `tb3-midi-guide.csv` |

So the profile carries two small fields, filled by H-tests, not by guessing: `acc` = how an accent is asked for (`VEL` with a threshold, `CC` with an
amount, `NONE`) and `slide` = how a slide is asked for (`OVERLAP`, `CC`, `NONE`). The sequencer's existing accent and slide lanes then need no change
of meaning: `ACC_THR` moves from a global constant into the profile, which is where decision 95's result naturally lands.

## 9. Behaviour in the situations that matter

| Situation | Behaviour |
| --- | --- |
| **No device chosen** (`prof` = 0, the power-on state, and every existing preset) | Nothing changes: no ROW 3, no chip, no CC, no byte of traffic. The RIG page shows `DEVICE: NONE`, the ALGO encoder picks one. The plain sequencer stays exactly as locked. |
| **Channel MIDI OFF or muted** | No CC (a wire speaks on the channel's MIDI channel; no channel, no CC). Muting keeps the lane phase like it keeps the pattern phase (ADR-0003 7.3). LEVEL turns on a muted channel still send (`HAND`), because the player is steering a sound. |
| **PLAY** | After the count-in and before the first note: each `SEQ` wire sends LEVEL + lane(step 0) once, stamped before the first NOTE_ON. `HAND` and `DEV` wires send nothing. |
| **STOP** | After the note-offs and CC123: each `SEQ` wire sends LEVEL once (**park**), so the device does not rest at a lane value. `HAND`/`DEV` untouched. (Question F: park / leave / profile neutral.) |
| **Preset load** (lands on a bar, SEQUENCER_UX 4.1) | The preset carries the rig. At the bar the new rig's `SEQ` wires send their first values; a wire that was `SEQ` in the old preset and is not in the new one is parked at the profile's neutral `home` (so the old sweep does not stay on the device). `HAND` values are not re-sent. |
| **Algorithm change** | The rig belongs to the channel, like edits (SEQUENCER_UX 1.2); it is kept. A wire riding a layer the new algorithm lacks freezes its lane (the layer is dimmed, ALGORITHM_SET 4.4) and un-freezes when the layer returns. `RIDE` binds to the morph *slot*, so it follows the algorithm's new morph there; the `~` on ROW 1/2 is redrawn. LEVEL, MOVE and the state do not change, so there is no CC burst. |
| **Device change** | New profile: wires are refilled with its four suggested params in `HAND`; LEVEL starts at each param's neutral `home` and *nothing is sent* until a knob is turned. Old `SEQ` wires are parked first. |
| **Dice / morph** | The seed or morph changes at the next step boundary, like the notes (next-step commit). The lane for the new step uses the new value. |
| **Length change, pages** | The lane is a function of `g(p)`, so a new page that copies a page copies its lane, exactly (SEQUENCER_UX 1.4). |
| **External clock / tempo** | Lanes follow steps, so they follow the clock. |
| **HAND turn on a stopped sequencer** | Sends at once (the player is tweaking a held note). |
| **RESET** | RESET + OCT+ from ROW 3 (`RST R3`) returns LEVEL to profile `home` and MOVE to 0, state `HAND`, as one undo record. RESET + a dice button resets that layer's seeds including any rig lane (they are the same seeds). |
| **REC** | Turning ROW 3 while recording is allowed and is not written (the pattern stores notes; question L: a later "record the knob" idea). |
| **Power loss** | Rig is saved in the preset; the device's own state is the device's. |

## 10. What must be verified on real hardware (H-tests for the hardware gate; add to HARDWARE_TEST.md later)

| # | Check | Pass |
| --- | --- | --- |
| H-RIG1 | A CC from the FM-1 over USB host box reaches the TB-3 on its MIDI channel and the named knob moves audibly | each of the CCs of phase 3 moves; any that does not is struck from the profile |
| H-RIG2 | A held lane value on the device matches what the screen says (the screen shows what was sent) | no drift between the displayed LEVEL and the device |
| H-RIG3 | Accent by velocity threshold (decision 95) and by CC 16: which does the accent, and what does CC 16 do to the accent amount | the profile's `acc` field is filled from the result |
| H-RIG4 | Slide by overlapping notes (the core's current method) is heard as a glide; CC 102 / 103 do not affect incoming notes | the profile's `slide` field is filled; if `NONE`, the `~` is dimmed on that profile |
| H-RIG5 | Resolution and zipper noise: a HAND detent step of 1 and of 4, and a rate of 1 CC per 10 ms | smooth enough; if audible, widen the rate limit or the step |
| H-RIG6 | CC flood: 8 `SEQ` wires on 4 channels, 1/32 steps, against a note-timing capture (`tools/fm1_timing.py`) through the host box and DIN | note jitter stays within the timing gate G1..G7; if not, lower the SEQ cap |
| H-RIG7 | STOP parks at LEVEL; a preset load parks wires that disappear | no wire rests at a lane value |
| H-RIG8 | The player turns the device's own knob, then PLAY: `HAND` does not overwrite it, `SEQ` does | as stated |
| H-RIG9 | The device with its own MIDI channel set (global/receive channel) ignores or accepts the CC as expected | note in the profile |
| H-RIG10 | The ring never starves notes under the worst-case CC load (drop counters from P4-003) | note drops 0 |

## 11. Impact on the locked designs and the task list

### 11.1 Locked designs and the prototypes P2-013 .. P2-017

| Item | Effect |
| --- | --- |
| P2-013 ARP, P2-014 PATTERNS, P2-016 EUCLID / SNAKE / STAGE, P2-017 prototypes and goldens | **None.** Algorithms stay oblivious (section 2.1): `step()`, `PulseOut`, seeds and goldens are untouched. Patterns that carry accent and slide flags keep them; only the profile decides how they are asked for. |
| P2-015 keybed use cases | The velocity 100 versus 90 flat value (decision 95) becomes the TB-3 profile's accent threshold. Pressure-to-CC stays dropped for v1; RIG makes the v2 idea (a held offset with restore, section 3) possible. |
| SEQUENCER_UX 4.2 (knob rows) | Amended by Option B: ROW 3 (question B). |
| VISION rule 3 / glossary "Channel" | A channel gains an optional **device (rig)**; amendment text for the owner (question A). |
| ALGORITHM_SET section 3, "CC / modulation lane (SNAKE Custom lanes): LATER, needs the engine to carry CC events" | This design is that engine; the deferral row is closed by it. |
| TIMING_SPEC 3 (M5 ring) | Reserve rule gains a CC threshold (24 free) and the CC pre-stamp (section 5, 7). |
| `key_map.tsv` | `KEY_B05_D#4` PERF hold: RIG (currently no hold). The legend cell for CHN reads `CHN h:RIG` (R-8: every hold is named where it starts; widths to verify, section 13). |

### 11.2 Where the hooks go, so the core is not reworked later

The rule: **add the fields and the call sites now, with the feature inert** (`prof = 0`), so later phases fill in behaviour without changing structs or save formats.

| Task | Hook (reserve now, fill later) |
| --- | --- |
| P4-001 sq types | `Wire`, `Rig` and `ChanSave.rig` with static asserts; preset version bump; `SQ_NWIRE` = 4. |
| P4-003 out ring | `midi_out_free()` also exposes the CC threshold (24); CC packets droppable, counted separately. |
| P4-004 sq_core | `rig_eval(ch, p, g, pass, &RigOut)` called next to `sq_eval`; `sq_emit` gets a `CC` kind with a `pre` stamp; PLAY and STOP call `rig_play` / `rig_park`. Inert while `prof = 0` (one compare per step). |
| P4-005 channel state API | `sq_rig_set_prof / sq_wire_set(ch,i,field,value) / sq_wire_state`, defaults (all `DEV`), reset records. |
| P4-006 sq_eval | the lane evaluation reads the layer seeds through the same accessor as the dice (no copy of the seed table). |
| P4-007 timing | the pre-stamp rule for lane CCs (`due - 2(k+1)` ticks); spread. |
| P4-010 gen_ui_tables | a profile table generator reading `tools/` data (not the midi.guide CSV); a `RIG` page row in `page_states.tsv`. |
| P4-011 ui framework | the knob band supports a third row; the band label for a row tag (`R3`). |
| P4-013 PERF page | ROW 3, chips, the `~` on dice sub-labels and morph cells, header chip. |
| P4-015 CHAN page | the device chip on the channel cell; the CHN hold route to the RIG page. |
| P4-018 reset / help | `RST R3`; help texts for the rig rows (run `check_help.py`). |
| P4-020 / P4-021 hardware test | the H-RIG list of section 10. |
| P5-001 .. P5-008 algorithms | **No hook.** They must not read the rig. |
| P5-009 hw-2 | H-RIG1 .. H-RIG4 join the algorithm session. |
| P6-002 dice | the toast names the wires that rode (`ACCNT + CUTOFF`); undo is unchanged. |
| P6-003 / P6-006 performance keys, force | v2 hook only: the gesture-offset source of section 3 reads the same wire fields. |
| P7-001 / P7-002 flash map and presets | slot size 1396 -> 1508 B; the pending-load INIT/park rule of section 9. |
| P7-003 settings | the park rule if the owner chooses a global switch (question F). |
| P8-002 user guide | a RIG chapter. |

## 12. Open owner questions

Proposed control-map additions (**not added**; check_scenarios would demand scenario steps for each): `B_OCTUP PERF` second tap = ROW 3 (RIG, only when a wire is
not `DEV`); `KEY_B05_D#4 PERF` hold = RIG; new page `RIG` (context column in the TSVs: `K1` PARAM, `K2` LAYER, `K3` FORM, `K4` MOVE; `KEY_W01..W04` tap = wire state; `B_OCTDN` / `B_OCTUP` = RIDE -/+; `B_FX..` buttons keep PERF roles; HOME = back); `B_OCTUP RESET` from ROW 3 = `RST R3`; a scenario S21 (set a device, make cutoff `SEQ`, dice ACCNT, STOP parks) with it.

| # | Question | Options | Recommendation |
| --- | --- | --- | --- |
| A | May a channel gain an optional **device (rig)** beyond MIDI channel, algorithm and length (amends VISION rule 3 and the glossary, like R1 and decision 89)? | A) yes, optional, inert with no device; B) no | **A** |
| B | The UX option | A) RIG PAGE; B) RIG ROW (+ page); C) RIDERS | **B**, section 6.4 |
| C | A **hold on black key CHN** opens the RIG page (new hold, named `h:RIG` on the legend) | A) yes; B) instead a RIG key on the CHAN page (3 gestures for setup, over the bar) | **A** |
| D | Profile source | A) built-in table + `GENERIC` raw CC; B) A plus a host tool to import CSV later; C) built-in only, no GENERIC | **A**, then B |
| E | Licensing of midi.guide data (CC BY-SA 4.0) inside a GPL-3.0 project | A) write our own profile data from vendor charts and tests, cite midi.guide only; B) bundle with attribution after a licence check; C) do not bundle any device data | **A**; please also confirm that `tb3-midi-guide.csv` in `docs/` stays reference-only with attribution |
| F | STOP behaviour of a `SEQ` wire | A) park at LEVEL; B) leave at the last lane value; C) park at the profile neutral | **A** |
| G | Wires per channel | A) 4 (one knob row); B) 6 or 8 (two rows) | **A** |
| H | Does EDIT-tap undo cover LEVEL / MOVE / state? | A) no: the seeds only (cheaper; a knob turns back by hand); B) yes (adds a rig image to `UndoRec`, +28 B x4) | **A** |
| I | Per-step lock (PLOCK) of a wire in NOTE_EDIT | A) not in v1, hook reserved; B) in v1 (needs a NOTE_EDIT knob row; +256 B .bss, +4608 B pool) | **A** |
| J | Choosing a device fills the four suggested wires | A) in `HAND` (silent until touched; ROW 3 works at once); B) all `DEV` (user wires everything); C) two in `SEQ` (instant movement, but unrequested CCs) | **A** |
| K | The global cap on `SEQ` wires (CC load) | A) 8; B) 16 (no cap); C) 4 | **A**, tuned by H-RIG6 |
| L | Record the ROW 3 knob into the pattern (REC) | A) v2; B) v1 as a step lock | **A** |
| M | Phase 2 scope | A) low-level design for the model and Option B only; B) include profile import | **A** |

## 13. Checks run for this document

- Width probe (`sh tests/design/width_probe.sh -`), AF_S, strings used in the mock-ups: knob label `ENVMOD` 49 of 51; value line `RESO 40H` 46 of 51; sub-label `ACCNT~` 46 of 47 (the
  cell width of `ACCNT` is 42 in SCREEN_SPEC 2.5, so `ACCNT~` needs the cell widened by 4 px: the PERF row-1 sum is 209 of 240, room exists); `SHAPE~` 44; `h:RIG` 28 of 41; `DEV` 22;
  cue `RIG: TAP 1-4 = WHO DRIVES  HOME=back` 221 of 240. **Over:** `CUTOFF 127` (69 of 51, so the label and the value are two lines, as in SCREEN_SPEC 2.4) and a single-line
  `CUTOFF 74` (62 of 60): the mock-ups' one-line rows are schematic. P2-003 re-measures in the phase-2 task.
- `python3 tools/backlog.py check`; `check_scenarios.py` and `check_help.py` were **not** affected (no `control_map.tsv` or `help_text.tsv` row was added).
