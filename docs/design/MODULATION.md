<!-- SPDX-License-Identifier: GPL-3.0-only -->
# MODULATION: the modulation layer (mod gen, mod lanes, 4 slots, the MOD page)

Task P2-022 (owner decision 107, 2026-11-11). Status: **for owner review** (gate `human-review`). Design only: no firmware, no `control_map.tsv` rows (proposed in section 8), no scenario rows. It **replaces** the RIG / ROW 3 "wire" bolt-on of P2-018 to P2-021 ([HW_SUPPORT_HIGHLEVEL.md](hardware/HW_SUPPORT_HIGHLEVEL.md), [HW_SUPPORT_LOWLEVEL.md](hardware/HW_SUPPORT_LOWLEVEL.md), [TB3_PROFILE.md](hardware/TB3_PROFILE.md), scenarios S21 to S23): each of those now carries a banner pointing here, and section 5.3 lists exactly what is kept and what is dropped.

Tags: **[V]** verified (vendor document, or heard on the owner's TB-3 on 2026-11-11, true of that unit only), **[I]** inferred, **[D]** design choice made here. Anything not tagged [V] is a hypothesis until an H-MOD test (section 9) has run.

## 0. The idea in one paragraph

A channel's algorithm already makes a pitch lane, a velocity lane and a rhythm. The **mod gen** adds two more lanes, **A** and **B**, made by the *same engine* with the same seeds, the same step position and the same morphs, and read out as device parameters (the TB-3's filter, say). A channel has **4 slots**; each slot names one device parameter (in the device's own words), a LEVEL (the centre we swing around) and a DEPTH (how far). Slot value = `LEVEL + lane x DEPTH`. The lanes are **dice'd** with the LFO button (the SHAPE layer) and **morphed** by the algorithm's own morphs, so they stay in step with the music; the player sets the 4 slots on **page 3 (ROW 3, "MOD")** by turning K1..K4 to select the parameter by name. A slot with DEPTH 0 sends nothing: the device's own knob is the player's. The knobs of ROW 1 and ROW 2 are not external controls; they morph the pattern and, through it, the lanes.

## 1. Settings model (deliverable 1)

Two lists, and each setting lives where it is used. **Global** = one value for the whole device. **Channel** = one value per sequencer channel (4 channels).

| Level | Setting | Where it is edited (existing page) | New in P2-022 |
| --- | --- | --- | --- |
| GLOBAL | tempo and clock source, DIV, swing, scale, key, octave | SET page (GLO) and SCALE, as locked | no |
| GLOBAL | **MOD SEND**: master on/off for all slots (default ON) | MENU, set-and-forget (a `ui_prefs2` bit) | yes, one bit |
| GLOBAL | CC rate guard (thresholds 24 / 12 / 8 free ring entries, LOWLEVEL 3.3) | none (a constant, not a setting) | no |
| CHANNEL | MIDI channel (unique across channels, VISION 3), mute | CHAN page | no |
| CHANNEL | algorithm, LENGTH 1..64, pages and their sources | CHAN page, ALGO encoder | no |
| CHANNEL | **device** (`NONE`, `GENERIC`, `TB-3`, ...); `NONE` = no slots, the whole layer is inert (decision 98) | MOD page, **ALGO encoder** while page 3 is live | the RIG page is gone |
| CHANNEL | **4 slots**: parameter, LEVEL, DEPTH each | MOD page (section 5) | yes |
| CHANNEL | per algorithm: `s_mod` (the mod seed) | LFO dice, FX dice-all (section 3.4) | one new seed per algorithm |

**A device profile lists its controls in the device's real names.** Each profile entry carries a **long name** (the device's panel or manual word, up to 16 characters, shown in the status line and the help) and a **short name** (at most 5 letters, 38 px in AF_S, shown in cells); both come from the profile source `tools/hwprof/<dev>.tsv`. TB-3 (names from the Roland *TB-3 MIDI Implementation* v1.10 [V-doc] and the owner's tests [V-ear]):

| Pick order | Short | Long | CC | Kind | Home | Default DEPTH when picked | How the player is likely to use it |
| ---: | --- | --- | ---: | --- | ---: | ---: | --- |
| 1 | CUT | CUTOFF | 74 | cont | 64 | 48 | the 303 gesture; lane A leans on accents, so accents open the filter |
| 2 | RESO | RESONANCE | 71 | cont | 40 | 24 | a slow wander on lane B; small, a wide move on a held note rings |
| 3 | ENV-X | ENV MOD X | 12 | cont | 64 | 32 | a sweep over the loop (lane B) |
| 4 | ENV-Y | ENV MOD Y | 13 | cont | 64 | 32 | decay-side partner of ENV-X |
| 5 | PAD-Y | PAD Y | 11 | cont | 64 | 32 | the touch pad as a lane |
| 6 | PAD-Z | PAD Z | 1 | cont | 0 | 40 | pressure axis, leans on accents (lane A) |
| 7 | ACC | ACCENT | 16 | cont | 64 | 24 | accent amount |
| 8 | EFFEC | EFFECT | 17 | cont | 0 | 32 | effect amount |
| 9 | SCTYP | SCATTER TYPE | 68 | select, 8 zones [I] | zone 1 | 2 zones | mode select; **linked to SCDEP** (section 6.2) |
| 10 | SCDEP | SCATTER DEPTH | 69 | cont | 0 | 40 | depth of the selected scatter; linked to SCTYP |
| 11 | TUNE | TUNING | 104 | cont | 64 | 4 | tiny pitch drift; the default DEPTH is small because pitch moves are loud |

The pick order is the order K1..K4 find when you turn from the start of the list: the likely wants first. This table replaces the "wire sets" of TB3_PROFILE section 3 (BITE, WET, PADS, PUSH): the sets were a second thing to learn; picking by name is one. Home, kind and zone count are unchanged from `tools/hwprof/tb3.tsv` (zone count stays a placeholder, decision 107 item 5B).

## 2. Remote control versus mod control: what we can and cannot do (deliverable 2)

What a MIDI Control Change is: an **absolute value for a parameter, pushed to the device**. MIDI CC has no "offset to the knob" form. So:

| Question | Answer | Status |
| --- | --- | --- |
| Is CC remote-control or mod-control? | **Remote (full) control of one absolute value.** The TB-3 takes CC 74, 71, 12, 13, 16, 17, 104, 68, 69, 11, 1 on its receive channel and moves the parameter to the value sent; there is no way to say "add 10 to whatever the knob is". | CC receive of all 11: **[V-ear]** (decision 106). "No offset form" is a property of the MIDI CC message **[V-doc]** |
| So how do we get "mod"? | **We emulate it around a centre we own.** Our value is `LEVEL + lane x DEPTH / 127`, clamped to the parameter's range. The LEVEL is the slot's, not the device knob's. | [D] |
| Can we read the device's knob position? | **No.** The FM-1 sends CC out only; the 3.5 mm jack is MIDI IN only; there is no read-back path in v1. We never know where the player left the TB-3 knob. | [V] hardware facts (CLAUDE.md). That the TB-3 *transmits* its knob moves as CC on its MIDI OUT is **[I]** (the Roland chart is a SysEx map and does not settle it), and routing it back to the FM-1 would need the host box to forward it over USB: a v2 idea, test H-MOD3 |
| What happens when the player turns the TB-3's own knob while a slot is active? | The knob sets the parameter to its physical position's value at once; our **next** CC then overwrites it. A step lane sends a CC on most steps, so the knob is overruled within one step; a slow lane overrules it at its next value change; a value equal to the last one sent is not re-sent (dedupe), so a held value lets the knob win until our value changes. | the sequence is **[I]**: the TB-3 may also ignore its knob until the physical knob is moved ("pickup"); the ear test H-MOD1 settles which |
| Does the device knob keep working? | **Yes when the slot is idle (DEPTH 0, or the parameter on no slot): the slot sends nothing and parks nothing.** While a slot is active the sequencer owns that one parameter and the other 7 parameters on the device stay entirely the player's. This is how decision 107 (b) "we only push our values" is kept. | [D] |
| What is left on the device when we stop? | STOP and mute **park** each active slot once at its LEVEL (a known place; decision 100). A slot made idle sends nothing, so the device keeps the last value we sent. | [D]; park kept from decision 100 |
| What can we *not* do? | Offset the knob, read the knob, restore the knob's value (we cannot know it), or make two sources (a lane and a hand turn) add. Where the player wants the TB-3 knob back, they set DEPTH to 0. | [V] |

Consequence for the UX: the screen must say who owns a parameter. A slot with DEPTH above 0 shows a **driven mark** (a filled dot on the cell; graphics, 0 px of text); DEPTH 0 shows an empty dot and the toast `DEPTH 0: THE KNOB IS YOURS` the moment the depth reaches zero (section 5.2).

## 3. The modulation gen (deliverable 3)

### 3.1 Where it sits

The generators of a channel are, in the order the engine runs them each step:

| Stage | Makes | Seeded by | Dice layer |
| --- | --- | --- | --- |
| pattern gen | which steps sound, their gate, slide, ratchet | `s_hits`, `s_gate`, `s_slide`, ... | HITS, ARTIC |
| note gen | which pitch each step has | `s_notes` (+ `s_oct`) | NOTES, SHAPE |
| quantise | scale and key snap (SP6) | none | none |
| velocity gen | loudness and accent | `s_vel` / `s_acc` | ACCNT |
| **mod gen** (new) | lane A and lane B, two `int8` values in -127..+127 per step, plus a select quantiser for select parameters | **`s_mod`** | **SHAPE (the LFO button)** |

The mod gen is **part of each algorithm**, not a pass beside the sequencer: `AlgoDesc` gains a function `mod(ctx, i_a, i_b)`, called from the algorithm's own `step()` at the position that algorithm already computes, and `PulseOut` gains `int8_t mod_a, mod_b`. Each algorithm decides *which index* reads the lane (section 4); the cell values and shapes come from two shared primitives (SP18, SP19, below). It reads only data the algorithm already has (its cache, its lanes' lengths, the step's velocity, its morphs), so every morph that moves the notes moves the lane in the same way, with no extra mapping.

```c
/* Shared primitives (sq_prims.c, pure; they do not call the SP1 generator, so no algorithm's note output or golden changes) */
int8_t sp18_cell(uint16_t s_mod, uint8_t lane /*0 A,1 B*/, uint8_t slot /*1..4*/, uint8_t i);    /* -127..127, a hash of (s_mod, lane, slot, i & 15) */
int8_t sp19_lfo(uint16_t s_mod, uint8_t slot, uint16_t t, uint16_t period);                       /* -127..127; shape from s_mod bits 0-1: TRI, SAW up, SAW down, SINE (table of 17 values);
                                                                                                    phase = (s_mod >> 4) % period + (slot-1) * period/4; t is the algorithm's own position */
int8_t sp20_lean(uint8_t velocity);                                                               /* (velocity - 90) * 2 clamped to -64..+64; 90 (a plain step, decision 104) = 0, 120 (accent) = +60 */
```

- **Lane A (STEP form, "per-step modulation").** `A = clamp(sp18_cell(...)/2 + sp20_lean(vel))`: a value per step that is held for the step and **leans on loudness**: an accented step pushes the parameter up, a plain step leaves it at its cell value. This is the classic "accents open the filter" behaviour (the old ACC form) without a form menu, and it follows accent edits, the ACCNT dice and the ACCNT morph for free.
- **Lane B (SWEEP form, "LFO").** `B = sp19_lfo(...)`: a slow shape whose **period is the algorithm's own cycle** (section 4), so the sweep always lines up with the loop. Slots 2 and 4 read B; slots 1 and 3 read A [D]. Slot 3 and slot 1 do not play the same curve (the slot is part of the hash); slot 4 and slot 2 are a quarter period apart.
- **Select parameters** (SCTYP): the lane is **quantised to the parameter's zones** (the "quantise" of the mod gen): zone = `base zone + (lane + 127) x (spread + 1) / 255 - spread / 2`, clamped; it is sent only on a bar line and only when the zone changed (S23 rule, kept), so the device never selects a type in the middle of a bar. DEPTH on a select is its spread in zones (0..7).
- **Ratchets and rolls** inside one step share the step's lane value: no CC burst per ratchet.
- **Sent value** per active slot: `v = clamp(LEVEL + lane x DEPTH / 127, lo, hi)` (division truncates toward zero). One CC per slot per step at most, none when `v` equals the last value sent (dedupe, LOWLEVEL 3.3); the CC is stamped 0.2 ms ahead of its note, the rest 1 ms apart (LOWLEVEL 3.5, kept).

### 3.2 Determinism (VISION 4)

`lane = f(s_mod, slot, generator position g, algorithm cache, velocity)`. Same settings and the same seed play the same CC stream. The generator position `g` is the one of SEQUENCER_UX 9.2, so a page that reads another page copies its lanes exactly, and `OWN` pages let them drift.

### 3.3 Morph semantics

There is no new morph slot: the eight morph slots of every algorithm are full (FULL_CONTROL 4.2). The morphs move the lane in three ways:

| Morph kind | Effect on the lanes | Reversible? |
| --- | --- | --- |
| **Reading morphs** (OFFSET, DIR, LEN, GLEN, PATH, TOTAL, PICK, SHAPE of ARP): they change the index the lane is read at | the lane is re-ordered, rotated or re-timed together with the notes: the filter move stays glued to the same notes | yes (pure functions of the morph values, FULL_CONTROL 5) |
| **Loudness morphs** (ACCNT, DYN, PUNCH): they change the velocity the lean reads | lane A swings more or less with the accents | yes |
| **Density and pitch morphs** (DENS, HITS, JUMP, SPAN, RANGE, ...) | the step set changes; the lane at each remaining step does **not** change (its index does not move), so thinning the pattern never re-rolls the filter | yes |

And one **new performance morph, DEPTH**, on page 3: hold OCT+ and turn K (section 5.2). Turning DEPTH to 0 puts the parameter completely back in the player's hand, and turning it back up restores the same curve (the cells are seeded, not stored). That makes the depth knob the "morph the modulation" gesture; the old per-wire RIDE (a morph scaling MOVE) is dropped as one concept too many.

### 3.4 Dice semantics

No sixth dice layer and no new button (ALGORITHM_SET 4: exactly 5 layers). **`s_mod` is a second seed of the SHAPE layer**, exactly as ACID's ARTIC press already re-rolls GATE and SLIDE with two counter reads (ALGORITHM_SET 4.3). The button printed `LFO` re-rolls SHAPE and the mod seed together; dice-all (FX) includes it; EDIT (undo) restores both.

| Algorithm | SHAPE button re-rolls | Sub-label (the button's second line) |
| --- | --- | --- |
| ACID | `s_oct` (register jumps) + `s_mod` | `OCT+MOD` |
| EUCLID | **`s_mod` only** (SHAPE was absent in EUCLID; the button was dim, now it is alive) | `MOD` |
| SNAKE | `s_poly` (velocity-lane length) + `s_mod` | `DRIFT+MOD` |
| STAGE | `s_lens` (long stages) + `s_mod` | `LONG+MOD` |
| ARP | `s_phase` + `s_mod` | `PHASE+MOD` |
| PATTERNS | the up/down seed + `s_mod` | `UP/DN+MOD` |

Cost of the shared button: the player cannot re-roll the filter motion without also re-rolling the register jumps (ACID) or the drift. Owner question 1 offers a separation. A channel with device `NONE` or no active slot: `s_mod` is still rolled (so a later device finds a fresh lane) but nothing is sent and the toast does not mention MOD. The dice toast on an active channel reads `LFO: SHAPE + MOD DICED` (measured, 140 of 204 px).

## 4. Mod lanes for every algorithm (deliverable 4)

Every algorithm has exactly two lanes, A (step) and B (sweep). "Index" is what the cell or the LFO position is read at; "period" is the LFO cycle in steps; both are values the algorithm already has. Lengths count in generator steps `g` (SEQUENCER_UX 9.2).

| Algorithm | Lane A reads cell at (index) | Lane B period (steps) | Morphs that re-order or re-time the lanes | Pitch/velocity analogy |
| --- | --- | --- | --- | --- |
| **ACID** (sting) | the gate position `g mod 16` (the same index as the accent and slide flags); lean from the step's velocity | 16 (the bar) | S1 `OFFSET` rotates, S4 `DIR` reverses, K4 `ACCNT` and S3 `PUNCH` change the lean | a fourth flag per step beside gate, accent, slide |
| **EUCLID** (t1) | the **hit number** within the cycle (`h mod 16`, the k-th hit gets the k-th cell), so the value belongs to the note and stays with it when OFFSET rotates; a hand-turned-ON step with no hit uses `g mod 16` | `LEN` (S1; the cycle length) | K2 `OFFSET`, S1 `LEN`, K4 `ACCNT`, S3 `PUNCH`, S4 `CURVE` (reads the contour order) | as the pitch contour, indexed by hit |
| **SNAKE** (snake) | **mod snake A**: a 4x4 grid of 16 cells stored like the pitch grid and read through the same traversal `order[]`, the same `len` and `dir` as the **pitch lane** (`order[dir_map(u, len, dir)]`) | **mod snake B**: a second 16-cell grid read like the **velocity lane** (`order[u mod vlen]`, length `POLY`), linearly interpolated between neighbours so it glides (an LFO made of cells) | K3 `PATH`/shape re-orders all lanes together, K2 `LEN`, S2 `GLEN`, S4 `DIR`, S3 `DYN` | the two extra lanes the original "Custom lanes x3" were (snake.md rows 34, 95), now read by the same machine as pitch and velocity; it is polymetric: A loops with the pitch, B with the velocity |
| **STAGE** (ml185) | the **stage number** `s` (a value per stage, held for the stage's whole duration, so a long stage holds one filter value; a stage of 4 pulses is one CC, not four) | `TOTAL` pulses (one sweep per loop) | K2 `TOTAL`, K3 `DIR` (stage order), S1 `OFFSET`, S3 `DYN` lean | the stage's note slot gains a mod slot |
| **ARP** | the **figure position** `c mod cyc` (the arpeggio note number inside the figure: each arpeggio note has its own value, so the filter plays the figure) | the channel `LENGTH` | K2 `SHAPE` (figure), S1 `OFFSET`, S4 `DIR`, S3 `DYN` | as the chord-tone order |
| **PATTERNS** | the **library pattern's step number** `pos` (the stored 16 steps; the authored accents give the lean, so a famous bassline keeps its accent-to-filter feel) | the pattern length (16 for most library patterns; `LENGTH` if shorter) | S1 `PICK` changes pattern: the lane follows the new pattern's step positions with the same seed; K1 `DENS` and K2 `VARY` leave the lane at each step unchanged; S4 `CTONE` n/a | the library gives the steps; the seed gives the lane |

Notes:
- A lane exists for **all six**: the table has no empty row (acceptance criterion). Three algorithms have a *native* mod lane that is a reading of an existing structure (SNAKE grids, STAGE stages, ARP figure); three read a flag-like index (ACID, EUCLID, PATTERNS).
- Both lanes cost no morph slot and no button; their only extra state is `s_mod` (2 B per algorithm per channel, section 7). SNAKE needs no extra stored grid: the 4x4 cells of A and B are `sp18_cell` of the cell number, regenerated, so a dice is the seed.
- The player never sees lane A or B as a setting. A slot's lane is fixed by its number (odd slots A, even slots B). If the owner wants the form chosen per slot, that is owner question 3.

### 4.1 Worked example: ACID on a TB-3, slots CUT and RESO

Start: CH1 ACID, MIDI 2, device TB-3, slot 1 = CUT (CC74), LEVEL 64, DEPTH 48; slot 2 = RESO (CC71), LEVEL 40, DEPTH 24; slots 3 and 4 DEPTH 0 (idle). Pattern steps 1 to 4: velocities 120, 90, 90, 120 (steps 1 and 4 accented; accent needs 100 or more, decision 104 [V]). The cells below are **illustrative fixtures** (the real values come from `sp18_cell`; the golden is fixed by the P4 task).

| Step | velocity | `sp20_lean` | cell (slot 1) | lane A = cell/2 + lean | CUT = 64 + A x 48 / 127 | LFO position (period 16, TRI from -127) | lane B | RESO = 40 + B x 24 / 127 |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 120 | +60 | +40 | +80 | 64 + 30 = **94** (`B1 4A 5E`) | 0 | -127 | 40 - 24 = **16** (`B1 47 10`) |
| 2 | 90 | 0 | -80 | -40 | 64 - 15 = **49** (`B1 4A 31`) | 1 | -96 | 40 - 18 = **22** (`B1 47 16`) |
| 3 | 90 | 0 | +10 | +5 | 64 + 1 = **65** (`B1 4A 41`) | 2 | -64 | 40 - 12 = **28** (`B1 47 1C`) |
| 4 | 120 | +60 | +100 | +110 | 64 + 41 = **105** (`B1 4A 69`) | 3 | -32 | 40 - 6 = **34** (`B1 47 22`) |

(`B1` = control change on MIDI channel 2. LFO: `-127 + 254 x i / 8` truncated, rising to +127 at step 9.) What the player hears: accented steps open the filter (94 and 105 against 49 and 65) while the resonance creeps up across the bar. What the buttons do next:

| Gesture | Effect |
| --- | --- |
| LFO (SHAPE) dice | new register jumps **and** new cells and a new LFO shape; the CUT/RESO figure changes at the next step; EDIT restores all of it |
| K4 `ACCNT` down | fewer accented steps: the lean disappears with them, so fewer filter peaks |
| S1 `OFFSET` +2 | the pattern and the filter moves rotate together; the CC values stay glued to the same notes |
| K1 `DENS` down | steps drop out; the surviving steps keep their CUT values (the lane index does not move) |
| hold OCT+, turn K1 to 0 | slot 1 DEPTH 0: CC74 stops being sent, the toast reads `DEPTH 0: THE KNOB IS YOURS`, and the TB-3 cutoff knob works; RESO keeps moving |

## 5. Setup UX: four slots and the MOD page (deliverable 5)

### 5.1 The slots

Each channel has **4 slots** (about 4, as asked; the TB-3 has 11 parameters and the player picks the 4 that matter for this channel, which is what keeps the page honest: 11 would be a patch bay). A slot = `{param, level, depth}`, 3 bytes. A freshly picked device starts with its **default slots** (section 6.1) at **DEPTH 0**: nothing is sent and the device's own knobs all work until the player raises a depth (kept from decision 100, "a chosen device starts in HAND"; owner question 5 offers a livelier start).

### 5.2 The MOD page = page 3 = ROW 3

Reached as before: **tap OCT+ twice** from ROW 1 (decision 101). The rail shows `1 2 3` with `3` live in the accent colour; the lane header names the page. The four cells are the four slots; **each K turns the parameter of its slot**: K1 slot 1, K2 slot 2, K3 slot 3, K4 slot 4.

| Control | On the MOD page | Gestures from PERF |
| --- | --- | --- |
| **K1..K4, turn** (default line `PICK`) | select the slot's parameter **by name**, stepping through the device's pick order (section 1) and wrapping; a fast detent skips by 4 (the known knob-step rule, FULL_CONTROL 5.1); `-` is "no parameter" (the slot is empty). A parameter already on another slot is **skipped** (no two slots on one CC). The cell shows the short name; the status line shows `CC74 CUTOFF  LEVEL 64  DEPTH 48` (the device's words). Picking sets LEVEL to the profile home and DEPTH to 0 (depth survives only when the slot keeps its parameter). | OCT+, OCT+, turn K: **setup, 2 gestures** (open + one control) |
| **hold OCT+ + K, turn** | the `DEPTH` line is live while OCT+ is held (the existing momentary-peek idiom of ROW 1 / ROW 2): 0..127, 0 = idle (the toast `DEPTH 0: THE KNOB IS YOURS`), up to 8 active slots across the device (cap `MOD FULL: 8 ACTIVE`, decision 100's SEQ cap kept) | hold OCT+, turn K: **performance, 1 gesture** once the page is open |
| **hold OCT- + K, turn** | the `LEVEL` line is live while OCT- is held: the centre we swing around; a CONT parameter at once sends the value (so the player hears the centre), a SELECT shows its zone | hold OCT-, turn K: 1 gesture |
| **ALGO encoder** | **picks the device** (`NONE`, `GENERIC`, then the profiles) while page 3 is live; the cue strip says `ALGO = DEVICE` for 2 s on entering, so the encoder's role is stated while it is changed (VISION 1). Pages other than page 3 keep ALGO = algorithm. | OCT+, OCT+, turn ALGO: **2 gestures** |
| **LFO** (dice SHAPE + MOD) | as on every page (3.4) | 1 |
| **EDIT** (undo) | as on every page; undo covers the seed, not the PICK / LEVEL / DEPTH edits (decision 100: undo = seeds only) | 1 |
| **HOME / OCT-** tap | leave to PERF | 1 |

What the screen shows (rail, labels): banner `MOD CH1  TB-3  4 SLOTS` (136 px); four cells, each **name over value** (`CUT` over `D48`; the value is the line that is live: `D` DEPTH, `L` LEVEL, nothing on PICK), a driven dot or an empty dot, and a tiny bar for the live value; two status lines (`CC74 CUTOFF  LEVEL 64  DEPTH 48`, 193 px; `STEP LANE A  LEANS ON ACCENTS` or `SWEEP LANE B  16 STEPS`); the cue strip `K PICK  h:OCT+ DEPTH  h:OCT- LEVEL` (200 px) and, while a line is held, `K DEPTH: 0 = THE DEVICE KNOB IS YOURS` or `K LEVEL: THE CENTRE WE SWING AROUND`. All strings are measured in `tests/design/widths.tsv` (the P2-022 block; 0 over).

Gesture count: setup (pick one parameter on a slot, change a device, set a LEVEL) <= 2 gestures from PERF; performance (DEPTH, dice, undo) = 1 once on the page, and the dice and undo are 1 from anywhere. This meets the budget of VISION 1 and decision 107 (f).

### 5.3 Reconciliation with decisions 97 to 101 (what is kept, what is dropped)

| Decision | Content | Status in this design |
| --- | --- | --- |
| 97 | Option B: a third knob row on PERF + a RIG page by holding CHN; the screen must show clearly that there are three rows, show the change and always show which row/page it is on | **Kept in part:** the third row stays (ROW 3, two OCT+ taps, rail `1 2 3`, accent colour, toast `ROW 3 OF 3: MOD`, the live row inverted). It is now the **MOD page** (setup + DEPTH + LEVEL), no longer "external controls". **Dropped:** the separate RIG page and the CHN hold route; the device is chosen on the MOD page with ALGO |
| 98 | A channel may carry an optional device; amends VISION rule 3; inert with no device | **Kept** (section 1); VISION rule 3 text is amended by this design: a channel's settings are its MIDI channel, algorithm, length **and (optionally) a device with 4 slots**. Proposed text in section 10 |
| 99 | Profile data is our own (built-in table + GENERIC); midi.guide is a cited reference; provenance per profile | **Kept unchanged** (`HwProv`, "Hardware belongs to its owners") |
| 100 | STOP parks SEQ wires at LEVEL; 4 wires; undo = seeds only; no per-step lock; a chosen device starts wires in HAND; SEQ-wire cap 8; no REC of ROW 3 | **Kept:** park at LEVEL (also on mute), 4 slots, undo = seeds, no per-step lock, start at DEPTH 0 (= HAND), cap 8 active slots, no REC. **Dropped:** the HAND / SEQ / DEV state machine (the two real states are DEPTH 0 and DEPTH above 0) |
| 101 | Low-level design locked: A ROW 3 via two OCT+ taps; B MOVE on the RIG page; C compact non-live rows; D DEV leaves values; E 5-letter names; F ride dot; G mute parks SEQ wires; H tap cycles / hold selects | **Kept:** A, C (the three rows draw the non-live rows as label lines), E (5 letters, 38 px), G. **Dropped:** B (MOVE becomes DEPTH on OCT+ held), D (no DEV state), F (no RIDE, so no ride dot), H (the key wires on the RIG page are gone; the 16 white keys keep their note/step roles on every page) |
| 102 | fx-bus removal (P3-014) precedes preset storage; 16 slots | **Kept**; the preset is smaller than the RIG design (section 7) |
| 103 to 106 | TB-3 specifics after the accent test; accent threshold 100; slide overlap; all 11 CCs verified | **Kept** and used (sections 1, 3.1, 4.1) |
| 107 | this brief | items (a) to (f) answered in sections 1, 2, 3, 4, 5, 6; 5B kept (6.2) |

## 6. TB-3 alignment (deliverable 6)

### 6.1 Default slot set

A freshly picked TB-3 puts these four on slots 1..4, all at DEPTH 0 with the default depths of section 1 waiting under the K turn (the first raise of DEPTH jumps to the table's default, so the first touch is audible, not 1/127):

| Slot | Parameter | CC | Lane | Why this default |
| ---: | --- | ---: | --- | --- |
| 1 | CUT | 74 | A (step, leans on accents) | the 303 gesture: the filter opens on accents |
| 2 | RESO | 71 | B (sweep) | the resonance wanders over the loop |
| 3 | ENV-X | 12 | A | per-step env-mod variation; a different curve from slot 1 (the slot is in the hash) |
| 4 | ENV-Y | 13 | B | a sweep a quarter-period behind slot 2 |

The other seven (PAD-Y, PAD-Z, ACC, EFFEC, SCTYP, SCDEP, TUNE) are reached by turning that slot's K. Which parameters are **sequenced, diced and morphed** (decision 107 d): all 11 can be sequenced (slot), diced (the LFO press re-rolls every active slot's lane at once), and morphed (DEPTH, plus the algorithm's reading morphs, 3.3). Nothing is "external knobs" any more. Defaults that fit a role: PAD-Z and CUT lean on accents (lane A slots), RESO, ENV-X, PAD-Y wander (lane B), TUNE keeps a small default DEPTH (4) because pitch moves are loud, ACC and EFFEC are best at a LEVEL by hand and a small DEPTH.

### 6.2 Scatter: type (select) linked to depth

- SCTYP is a **select** of 8 zones [I, placeholder, decision 107 item 5B: the zone count is not a priority and ships as the placeholder]; SCDEP is the continuous depth of the selected scatter [V-ear, decision 106].
- **The pair takes two slots.** Picking SCTYP on slot *n* also puts SCDEP on slot *n+1* (replacing whatever was there) and shows the toast `SCTYP TAKES SLOTS 3 AND 4` (159 px); picking SCDEP alone is allowed (the type stays where the device has it); picking SCTYP on slot 4 asks the player to use slot 3 or fewer: the pick is refused with the same toast form (`SCTYP NEEDS SLOT 3 OR LOWER`). Dropping SCTYP from slot *n* leaves SCDEP in place.
- SCTYP's lane: lane A (or B) quantised to the zones, spread = DEPTH (0..7 zones around the LEVEL zone), sent on **bar lines only and only when the zone changed** (S23 steps 5, 8, 9 rules kept), 150 ms after the last hand detent when LEVEL is edited on the page (so a quick LEVEL sweep never selects types on the way).
- SCDEP's lane: a normal slot. The unknown behaviour (does SCDEP at 0 mean "scatter off"? does a type change reset the depth?) is H-RIG20, unchanged; the `:G :R :D` link flags of the profile stay for it.

## 7. Budget (deliverable 7)

| Item | Was (RIG design, LOWLEVEL 1.6) | Now | Note |
| --- | --- | --- | --- |
| Saved per channel | `Rig`: device + 4 wires x 7 B = 28 B (`RigSave` 112 B) | **`ModSave`**: device 1 + 4 slots x 3 B (param, level, depth) + `s_mod` for 6 algorithms x 2 B = **25 B**; 4 channels = **100 B** | `ChanSave` stays 344 B (a sibling array, as RF1) |
| Undo | `UndoRec` stays 92 B | `UndoRec` carries one more seed: **+2 B to 94 B**; the assert in `sq_types.h` changes in P4-001 | undo of the SHAPE layer must restore `s_mod` |
| Runtime (.bss) | 274 B | **about 250 B**: `ModRT` 4 channels x 4 slots x 7 B (last CC, last value sent, pending zone, park, timer) = 112 B; `RigQ` and stats 26 B; UI 8 B; the active-slot count 1 B | well inside the 3072 B UI allowance |
| `.bss` headroom after SEQUENCER_UX 9.4 | 10849 B | **about 10870 B**: above the 8192 B trigger and the 4096 B floor | numbers are estimates until P4-023 |
| Pool: `SeqPreset` | 1396 + 112 = 1508 B x 18 copies = 27144 B | 1396 + 100 = **1496 B x 18 = 26928 B** (216 B less) | still 1164 B too big **before** the fx-bus removal R6 (decision 102), same escape: `SQ_NSLOT` = 8 until R6 |
| Flash: profiles | `HwProf` 316 B x 8 + provenance 76 B x 8 = 3136 B | per-parameter lane, form and MOVE (3 B) and the 4 sets (60 B) are dropped; a long name (up to 16 B) is added: `HwParam` about 27 B; `HwProf` for 16 parameters about 440 B but the TB-3 has 11: about 300 B; **about 3 KB for 8 profiles** | final sizes by P4-023 asserts |
| CPU (ISR) | about 40 integer operations per SEQ wire per step | about 25 per active slot per step (one hash or one table read, a multiply, a clamp, a compare) | well inside ADR-0003's +5 % |
| Goldens | rig goldens separate; core goldens unchanged | `tests/mod_golden.txt` separate; algorithm **note** goldens unchanged (the new `PulseOut` fields are not hashed into them) | T-MOD1 below |

Tests the P4 tasks must write (T-MOD1..T-MOD4): T-MOD1 with every slot idle (or device NONE), the core event stream and the notes goldens are byte-identical to today's; T-MOD2 with active slots, the NOTE events are identical to T-MOD1 and only CC events are added; T-MOD3 `sp18_cell`, `sp19_lfo`, `sp20_lean` are pure (same inputs, same output, any call order); T-MOD4 dicing SHAPE changes the lanes of exactly the active slots and undo restores them.

### 7.1 Tasks this changes

| Task | Was | Now |
| --- | --- | --- |
| P4-023 | `sq_rig.h`: Wire, Rig, RigSave, RigRT, HwProf... | `sq_mod.h`: Slot, `ModSave`, `ModRT`, `HwProf` with long names and the default slot set, asserts incl. `UndoRec == 94` |
| P4-024 | `gen_hwprof.py`, 4 sets, per-param lane fields | the generator reads **long + short names**, `kind`, `link`, `home`, default depth, **one default slot set**; no sets, no layer/form/move; width check on the short names |
| P4-025 | `sq_rig.c`: `rig_mix`, `rig_lane`, `rig_value` | **`sp18_cell`, `sp19_lfo`, `sp20_lean`** in `sq_prims.c` (SP18..SP20) and `mod_value()`; `tests/mod_golden.txt`; T-MOD1..T-MOD4 |
| P4-026 | `rig_step/rig_block/...` | `mod_step/mod_block/mod_play/mod_stop/...` same call sites; the lane values now come from `PulseOut.mod_a/b` |
| P4-027 | main-loop API (wires, cap) | `sq_mod_set_dev / sq_slot_set (param, level, depth)`, cap of 8 active slots, loader clamping |
| New (add to P4-0xx before the six algorithm tasks) | | each algorithm task (P4-012 to P4-017) gains the `mod()` hook and a lane golden; `AlgoDesc.mod` |
| P4-030 | `ui_rig.c`: the RIG page, CHN route, wire keys | **`ui_mod.c`**: the MOD page only (K pick, held OCT+ / OCT- lines, ALGO = device); no CHN route, no key wires |
| P4-031 | cue rows 4a-4c, 13a, 15, 15a, `RST R3` | the strings of section 5.2 and the toasts of 3.4 and 6.2; `RST R3` becomes "reset the four slots of the channel" |
| P6-009 | ROW 3 live: LEVEL knobs, HAND/SEQ | ROW 3 = MOD page behaviour (5.2) |
| P6-010 | dice integration: ride dots, toast `ACCNT + CUT` | the LFO toast `LFO: SHAPE + MOD DICED`; sub-labels of 3.4; no ride dot |
| P5-010 | H-RIG sheet | H-MOD sheet (section 9) plus the unchanged H-RIG20 |
| P2-020/21 outputs | `tools/hwprof/tb3.tsv` v2 with layer/form/move and sets | rewritten as v3 by P4-024 (the data in section 1 above is the content) |
| P7-005 | preset map for 1508 B slots | 1496 B |
| ALGORITHM_SET, FULL_CONTROL, scenarios | RIG rows and S21..S23 | section 8 |

## 8. Design documents changed, and rows to add later

- [ALGORITHM_SET.md](ALGORITHM_SET.md): change note at the top (SHAPE gains `s_mod`; the "CC / modulation lane" LATER row is superseded; EUCLID SHAPE is no longer absent).
- [algorithms/snake.md](algorithms/snake.md): row 34 (Custom lanes x3, "Dropped") and the "must not exist" list get a note: the mod snakes are now lane A and lane B (section 4).
- [hardware/README.md](hardware/README.md), HW_SUPPORT_HIGHLEVEL, HW_SUPPORT_LOWLEVEL, TB3_PROFILE and scenarios S21, S22, S23 carry a **SUPERSEDED BY MODULATION.md** banner. They are kept as history and for `control_map.tsv` coverage: the RIG context rows and S21..S23 remain until P4-030 replaces them, so the scenario checker stays green; no row is edited here.
- `tests/design/widths.tsv`: the P2-022 block (18 strings, measured).
- Proposed `control_map.tsv` rows (for the P4 table task, not added here): `EN_K1..EN_K4/MOD` pick; `EN_K1..EN_K4/MOD` with `B_OCTUP` held (DEPTH) and `B_OCTDN` held (LEVEL); `EN_ALGO/MOD` device; `B_LFO/MOD` dice; `B_EDIT/MOD` undo; `B_HOME/MOD` leave. The `RIG` context and the `KEY_W*/RIG` rows go.
- Scenario to write when the rows exist (S24, replacing S21..S23): the walk of section 4.1 plus the setup walk "OCT+ OCT+, turn K1 to CUT, hold OCT+ and turn K1, press PLAY, press LFO, press EDIT, hold OCT+ and turn K1 to 0".

## 9. What is verified and what must be tested (the H-MOD sheet; the owner runs it on the TB-3)

| Test | What | Pass criterion |
| --- | --- | --- |
| H-MOD1 | **Knob versus CC.** With CUT idle, turn the TB-3 CUTOFF knob while a repeating note plays; then send CC74 from a slot; then turn the knob again | records whether the knob jumps the parameter at once and whether our next CC overwrites it; also whether the knob is ignored until moved (pickup) |
| H-MOD2 | **Idle really is idle.** Slot DEPTH 0 for 60 s with a playing note | the CC74 stream is empty (scope or MIDI monitor) and the knob works |
| H-MOD3 | Does the TB-3 send its knob moves on MIDI OUT? | tells whether a read-back is ever possible (v2 only) |
| H-MOD4 | Rate: 4 active slots at 1/16 and 180 BPM through the host box | no note delay above 1 ms, no drops (the ring counters) |
| H-MOD5 | Ear: lane A leans on accents (CUT opens on accents), lane B period lines up with the bar | the owner hears both |
| H-MOD6 | Park: PLAY then STOP with 2 active slots | each active parameter returns to its LEVEL once |
| H-RIG20 | scatter linkage (unchanged, LOWLEVEL 9.8) | as there |

[V] today: the 11 CC numbers, that each moves its parameter on channel 2, accent at velocity 100 or more, slide by overlap (decisions 104 to 106). Everything about the knob/CC interaction (the middle of section 2) and the timing of 4 slots is [I] until H-MOD1 and H-MOD4.

## 10. Owner questions (lettered options, a recommendation each)

1. **How is the mod lane dice'd?** (A) The LFO button re-rolls SHAPE **and** the mod seed together (this design; no new gesture; ACID's register jumps change with the filter). (B) The LFO button re-rolls only MOD while page 3 is live and SHAPE only elsewhere (the sub-label changes with the page; two roles for one button, stated on screen). (C) A dedicated MOD layer on a hold of LFO (breaks "dice acts on press"). **Recommend A**; if hardware shows the coupling is annoying, switch to B by a table row.
2. **Where is the DEPTH performance knob?** (A) hold OCT+ and turn K on page 3 (this design: owner's "turn to select" is kept on plain K). (B) plain K = DEPTH and hold a slot's white key + K = pick (rejects the literal 107 f). (C) take S4 of each algorithm as a global MOD morph (loses a morph in all six). **Recommend A.**
3. **Lane per slot.** (A) fixed: odd slots lane A (step), even slots lane B (sweep) (this design). (B) the player chooses the lane per slot (one more value to set, +1 gesture). **Recommend A**; B if the owner wants to put the cutoff on a sweep.
4. **Device choice on ALGO while page 3 is live.** (A) ALGO picks the device on the MOD page (this design). (B) the CHAN page gets a DEVICE cell (a fifth CHAN setting; costs a CHAN change). **Recommend A**: one fewer page to learn; B if the owner finds ALGO changing role confusing.
5. **Initial depth when a device is picked.** (A) all four slots DEPTH 0 (decision 100's HAND; the device is untouched until the player acts). (B) slots 1 and 2 at their default depths so the sequence is alive on selection (speed to music, VISION 5). **Recommend A** (decision 107 b); B is a one-line profile flag if the owner wants it.
6. **VISION rule 3.** The glossary "A channel's only setting is its MIDI output channel" has been amended once for the optional device (decision 98). The proposed text: "A channel's settings are its MIDI channel, its algorithm and length, and (optional) a device with four mod slots." (A) accept and edit VISION.md. (B) keep as is. **Recommend A** (the owner edits VISION; this task does not).
7. **Linked pair on two slots.** (A) SCTYP takes slots *n* and *n+1* (this design). (B) a single combined slot `SCATTER` whose LEVEL is the type zone and DEPTH the scatter depth (one slot, no lane on the type). (C) leave unlinked (not recommended: the type is a mode). **Recommend A**; priority is low (5B).
8. **Cap of 8 active slots across the device.** Four channels x four slots would be 16. (A) cap 8 (decision 100, kept). (B) cap 16 and rely on the CC rate guard. **Recommend A** until H-MOD4.
9. **Select quantiser on bar lines only.** (A) yes (S23 rule). (B) on any step. **Recommend A.**

## 11. Checks run for this document

`python3 tests/design/check_scenarios.py`, `check_help.py`, `python3 tools/check_sequencer_ux.py`, `sh tests/design/width_probe.sh` (286 strings, 0 over, including the 18 new ones), `python3 tools/backlog.py check`; results are in the task log. No firmware was touched and no `control_map.tsv` or scenario row was edited.
