<!-- SPDX-License-Identifier: GPL-3.0-only -->
# TB3_PROFILE: the Roland TB-3 profile (phase 3 of the hardware support model)

Task P2-020 (owner decision 96; model in [HW_SUPPORT_LOWLEVEL.md](HW_SUPPORT_LOWLEVEL.md), phase 2). Status: **for owner review** (gate `human-review`). Design and data only: no firmware. **Revised by P2-021 (2026-11-11, decision 106): all 11 CCs verified by ear; select and linked parameters; wire sets (sections 2, 3, 7-14).**
The data file is [`tools/hwprof/tb3.tsv`](../../../tools/hwprof/tb3.tsv) (format of LOWLEVEL 2.4); `tools/gen_hwprof.py` arrives with P4-024.

Tags: **[V]** verified (document or hardware), **[I]** inferred or unconfirmed, with its source in the row. "V-doc" = read in the vendor document; "V-ear" = heard on the owner's TB-3 on 2026-11-11 (firmware version not recorded; true of that unit only).

## 1. Sources and the rule applied (decision 99)

| Source | Use |
| --- | --- |
| **Roland, *TB-3 MIDI Implementation*, v1.10, 2014-07-04**, "3. Parameter Address Map, Controller" (https://cdn.roland.com/assets/media/pdf/TB-3_MI_1.pdf; text extracted and read in P2-015/P2-020) | the CC numbers and names of the **seven** SysEx-listed controllers (74, 71, 12, 13, 16, 17, 104). It is a SysEx map that **annotates** each controller with a CC number; it has no receive-CC table and does not say which channel or whether every CC is accepted from a host |
| The owner's own tests on the owner's TB-3, **2026-11-11** (decisions 104, 105, **106**) | accent, slide, receive channel, and now **all 11 CCs heard moving their parameter** (slow 0-127-0 sweeps under a repeating note, channel 2), including the four the vendor chart does not list: **CC 1 (pad Z), 11 (pad Y), 68 (scatter type), 69 (scatter depth)**. The owner's reading of 68 (a mode select) and 69 (its depth, linked) is also from this test |
| midi.guide `tb3-midi-guide.csv` and the CDM article | **cross-check only, not copied**: nothing in this file or the TSV comes from the CSV. The CSV was the pointer to what to test for CC 1, 11, 68, 69, 102, 103; the facts and the names here are our own |

Rule used (decision 99, widened by P2-021): a CC enters the profile if the vendor chart names it **or** a named person heard it on hardware. The four CCs that enter by the second road have one source (the owner's test) and that is stated in each row. CC 102 / 103 (pattern slide / pattern accent in the CSV) were not part of the 2026-11-11 test and stay out (H-RIG4).

## 2. The parameter table (11 entries)

Names are at most 5 letters and measured in AF_S with `tests/design/width_probe.sh` (limit 38 px): CUT 24, RESO 29, ENV-X 34, ENV-Y 34, PAD-Y 33, PAD-Z 34, ACC 24, EFFEC 34, TUNE 31, SCTYP 37, SCDEP 37 px. Range 0..127 for all (the vendor map's own range is 0..255, two nibbles of SysEx; a CC carries 0..127 and the device scales it; **0 and 127 both audible** by the 2026-11-11 sweeps [V-ear]). `kind` and `link` are LOWLEVEL 9.2 and 9.3; the lane (`layer form move`) is what a wire gets when it takes the parameter.

| CC | Name | Vendor / owner name | Kind | Home | Lane when wired (layer form move) | Source and tag |
| ---: | --- | --- | --- | ---: | --- | --- |
| 74 | CUT | CUTOFF | C | 64 | ACCNT ACC 48 | number and name [V-doc]; audible [V-ear] |
| 71 | RESO | RESONANCE | C | 40 | ARTIC WALK 24 | as CUT |
| 12 | ENV-X | ENV MOD X | C | 64 | SHAPE RAMP 32 | as CUT; what it does musically is the env mod's X axis [I] (by ear: it moves; the vendor map gives only the name) |
| 13 | ENV-Y | ENV MOD Y | C | 64 | - RAND 0 (armed, silent) | as ENV-X |
| 11 | PAD-Y | pad Y (owner's term) | C | 64 | SHAPE WALK 32 | **own test only** [V-ear]; not in the vendor chart. Name is ours |
| 1 | PAD-Z | pad Z (owner's term) | C | 0 | ACCNT ACC 40 | **own test only** [V-ear]; "pad Z" is the pressure axis of a touch pad [I] |
| 16 | ACC | ACCENT | C | 64 | - RAND 0 | number and name [V-doc]; audible [V-ear]; it is the **amount** of the accent effect [I], not a per-note accent (decision 104: velocity is) |
| 17 | EFFEC | EFFECT | C | 0 | - RAND 0 | [V-doc], [V-ear]; the effect amount |
| 104 | TUNE | TUNING | C | 64 | - RAND 0 | [V-doc], [V-ear]; in no wire set (an unmusical live move) |
| 68 | SCTYP | scatter type (owner) | **S8** (placeholder) | 8 (centre of zone 1) | HITS DICE 40 | **own test only** [V-ear] that it acts; the owner reads it as a **mode select**. Zone count 8 and equal edges are [I] (H-RIG20) |
| 69 | SCDEP | scatter depth (owner) | C, **linked to SCTYP** | 0 | ACCNT ACC 40 | **own test only** [V-ear]; the depth of the scatter effect, linked to the type (section 12) |

`home` is the neutral the park uses (LOWLEVEL 3.4). **[I]**: no source gives the device's resting value; 64 is the middle, RESO 40, EFFEC 0, PAD-Z 0, SCDEP 0 are placeholders confirmed or changed by H-RIG2 (set the knob by hand, read the number). It is a one-line TSV change.
The lane is the *default* a wire gets; the player changes it on the RIG page and a wire keeps its changes (the profile is read only when a wire takes the parameter).

## 3. The wire sets and the lane mapping

Four wires carry eleven parameters (decision 100). The profile names **four sets**, each four parameters; a freshly picked device starts with the first (BITE). All wires start in **HAND** at `home` and send nothing until the player turns a knob (decision 100). How the player picks a set (hold CHN, turn ALGO) is LOWLEVEL 9.4.

| Set | Wire 1 | Wire 2 | Wire 3 | Wire 4 | What it is for | Layers that move it |
| --- | --- | --- | --- | --- | --- | --- |
| **BITE** | CUT | RESO | ENV-X | ENV-Y | the classic 303 filter hands: the filter, its edge, the envelope's two axes (this is the P2-020 set, unchanged) | ACCNT -> CUT, ARTIC -> RESO, SHAPE -> ENV-X |
| **WET** | CUT | EFFEC | SCTYP | SCDEP | space and glitch: cutoff plus the effect amount and the **scatter pair side by side** | ACCNT -> CUT and SCDEP, HITS -> SCTYP |
| **PADS** | PAD-Y | PAD-Z | CUT | RESO | the touch pad as lanes: pad Y wanders over the loop, pad Z leans on the accents | SHAPE -> PAD-Y, ACCNT -> PAD-Z and CUT, ARTIC -> RESO |
| **PUSH** | CUT | ACC | EFFEC | RESO | drive harder: the filter opens on accents, the accent amount and the effect by hand | ACCNT -> CUT, ARTIC -> RESO; ACC and EFFEC armed, silent |

Every parameter is in at least one set except TUNE (reachable with K1 on the RIG page). Why the lanes sit where they do:
- **CUT, ACCNT, ACC, MOVE 48.** The classic 303 gesture: an accented note opens the filter. The ACC form follows the accent steps themselves (edits, the ACCNT dice and the ACCNT morph for free), so it needs no seed. Value = LEVEL +48 on an accent, -48 otherwise. Dicing ACCNT moves sound and filter together.
- **RESO, ARTIC, WALK, 24.** A slow wander that changes when the gate and slide layer is diced: articulation and resonance read together as the note's character. Small depth, a wide resonance move on a held note rings.
- **ENV-X, SHAPE, RAMP, 32.** A sweep over the loop that the SHAPE dice re-aims (direction and start): the build every bassline player does with the env mod. **PAD-Y** gets SHAPE with WALK (drift, not a ramp) so a set that has both does not sweep twice.
- **PAD-Z, ACCNT, ACC, 40.** The pressure axis leans on accents, the way a finger would.
- **ENV-Y, ACC, EFFEC, TUNE: armed but silent** (layer none, MOVE 0): the wire does nothing until the player raises MOVE; each is the knob for hand-play.
- **SCTYP, HITS, DICE, spread 40; SCDEP, ACCNT, ACC, 40:** section 12.
If the algorithm lacks the layer the lane freezes (LOWLEVEL 3.1); set application writes the stand-in layer instead (section 13, owner question B). The ACC form is independent of the algorithm, so CUT, PAD-Z and SCDEP always work.

## 4. Accent (`acc_mode`, `acc_arg`)

- **Method: velocity threshold.** `acc_mode` = 0 (VEL), `acc_arg` = **100**. [V-ear, decision 104/105] notes at 80, 90, 95 do not accent; 100, 105, 110, 120 do (owner's TB-3, channel 2).
- The sequencer's flat non-accent velocity is 90 and accent 120 (decision 104), both on the right side of the threshold.
- CC 16 is **not** the accent method: it is the amount of the accent effect [I, name only; it moves audibly, decision 106]; whether it scales the accented sound, and by how much, is left to H-RIG3. Therefore no `CC` mode is needed and `acc_arg` carries 100, not a CC number.
- An accent (velocity 120) on a note that is slid into sounds like an unaccented one [V-ear, decision 105]. The sequencer need not avoid it, but the ACCNT dice may show a slide-destination accent as inaudible (a UI note for P4, not a change here).
- **The ACC form on CUT covers the inaudible slide-accent case.** A velocity accent on the note slid into does nothing audible [V-ear], but the ACC lane of CUT (and of PAD-Z and SCDEP) is a **CC** sent before the note, not a velocity effect: the filter still opens on that step, so the accent is not lost on a slide destination; the only part that is inaudible is the TB-3's own velocity accent (the extra envelope bite). Nothing in the lane design needs to avoid or special-case slide destinations.

## 5. Slide (`slide_mode`, `slide_arg`)

- **Method: overlap.** `slide_mode` = 0 (OVERLAP), `slide_arg` = 0 (unused). [V-ear, decision 105] a note-on sent **before** the previous note-off glides (no second envelope); a gap gives two separate notes. This is the 303 behaviour the core already assumes (ADR-0003 7.3), so the TB-3 needs no special case.
- CC 102 / 103 are not used for slide or accent on incoming notes (CSV-only names, [I] that they act on the internal pattern); H-RIG4 confirms they do nothing to played notes.
- Timing consequence for P4: the overlap must be at least one scheduler tick, and the previous note-off must not be sent before the next note-on (the glide ends if it is).

## 6. Receive channel

Default **MIDI channel 2** [V-ear, decision 105 (notes on ch2) and CLAUDE.md]. The chart does not state a receive-channel rule [V-doc: none]. The profile does not set the channel (the user sets the channel's MIDI channel, VISION rule 3; a TB-3 whose own channel was changed needs the FM-1 channel changed to match). Rule shown to the player: **the CC goes on the channel's MIDI channel**, so notes and CCs always share a channel; H-RIG9 checks that a TB-3 set to another channel (or OMNI) behaves.

## 7. Provenance record (the `HwProv` fields)

| Field | Value (24 characters at most) |
| --- | --- |
| `src` | `vendor chart+own tests` (22) |
| `how` | `11 CCs+acc+slide by ear` (23) |
| `cred` | `J. Keaney` |
| `level` | **2** (decision 106: every CC in the table, and the accent and slide methods, were heard on hardware by a named person; seven CC numbers are also in the vendor chart, four rest on that test alone) |
| date | 2026-11 |
| MENU > INFO row | `HW DATA: TB-3`  `L2 JK 2611` |

Credit: the vendor chart is Roland's; the accent, slide and CC facts are the owner's own measurements on one unit (firmware version not recorded); midi.guide (pencilresearch/midi, CC BY-SA 4.0) and the CDM article are credited as cross-checks in the ABOUT credits, not as a data source. Level 2 follows the HwProv scale of LOWLEVEL 2.1 ("verified on hardware by a named person"). It is a fact about **that** unit; the resting values (`home`) and the scatter zones stay [I] and are listed in section 8.

## 8. What is not known (left to the H-tests)

| Unknown | Tag | Resolved by |
| --- | --- | --- |
| What ENV-X / ENV-Y / PAD-Y / PAD-Z do musically (they move audibly, decision 106; what the player hears is not described) | [I] | the owner's ear; names stay neutral |
| The true resting value (`home`) of each parameter | [I] | H-RIG2 |
| Scatter: the number of types, the zone edges, whether depth 0 or type zone 1 means off, behaviour of a type change under depth, whether the depth needs resending, clock dependence | [I] | **H-RIG20** (LOWLEVEL 9.8) |
| Whether CC 16 changes the accent amount, and by how much | [I] | H-RIG3 |
| Whether CC 102 / 103 affect played notes | [I] | H-RIG4 |
| Zipper noise at a detent step of 1 and 4, at 10 ms; whether 150 ms is a good SEL settle | [I] | H-RIG5, H-RIG20 |
| Receive channel / OMNI behaviour of a TB-3 with another setting; firmware differences (the tested unit's version was not recorded) | [I] | H-RIG9 |
| The slide overlap length that glides reliably at 300 BPM and 1/32 | [I] | H-RIG4 (add the tempo sweep) |

## 9. The H-tests for the TB-3 (pass criteria)

H-RIG1..H-RIG19 are defined in LOWLEVEL 6.3, **H-RIG20** in LOWLEVEL 9.8. The ones that fill this profile:

| # | Test | Pass |
| --- | --- | --- |
| H0 (done) | accent threshold by ear, decision 95 | **done 2026-11-11**: 80, 90, 95 no; 100 and above yes; `acc_arg` = 100 |
| H-RIG1 (done by sweep) | each CC sent on ch 2, slow 0-127-0 under a repeating note | **done 2026-11-11, decision 106: all 11 move their parameter** (from the host); repeat from the FM-1 itself in P5-010 |
| H-RIG3 | accent: velocity 99 versus 100 on every step; CC 16 swept at 0 / 64 / 127 while accented notes play | 99 plain, 100 accented; the CC 16 effect is written in section 4 |
| H-RIG4 | slide by overlap: overlap 1 tick, 5 ms, 20 ms; gap 0 (note-on at the note-off); CC 102 / 103 during play | overlap glides, gap 0 does not; CC 102/103 do not slide played notes. **Slide by overlap is already [V-ear] (decision 105); H-RIG4 repeats it from the FM-1 itself** |
| H-RIG20 | scatter linkage (LOWLEVEL 9.8) | zone count and edges, `:G` `:D` `:R` written into `tb3.tsv` |

## 10. H-RIG TB-3 test sheet (text for HARDWARE_TEST.md; P4-020 merges it)

> **H-RIG, TB-3 (owner runs it; about 30 minutes).** Rig: FM-1 USB to a host box, host box DIN to the TB-3 MIDI IN; the TB-3 on MIDI channel 2; CH1 MIDI 2, the device TB-3 picked on the RIG page (hold CHN, turn ALGO to `TB-3 BITE`), all four wires HAND.
> 1. (H-RIG1) Turn each ROW 3 knob from 0 to 127 and back: CUT and RESO must be audible on a held note. Then turn ALGO to `TB-3 WET`, `PADS`, `PUSH` and do the same: every one of the 11 knobs must move something (decision 106 heard them from a host; this repeats it from the FM-1). TUNE is not in a set: hold CHN, select a wire, turn K1 until the palette shows TUNE. Struck: any that does nothing.
> 2. (H-RIG2) Set the TB-3's own knob by hand, read the position, and compare with the number on screen after sending the same value: they must agree within 2 detents. Record each parameter's resting value for `home`.
> 3. (H-RIG3) Play steps at velocity 99, then 100: only 100 accents. Sweep CC 16 with accented notes playing; record whether the accent gets stronger.
> 4. (H-RIG4) Play two notes overlapped by 1 tick, 5 ms and 20 ms, then a note-on exactly at the note-off, then a 20 ms gap: overlaps glide, the gap does not. Send CC 102 and 103 during play: no slide or accent on played notes.
> 5. (H-RIG5) With HAND on CUT hold a note and turn K1 at one detent per step, then four: smooth, no zipper. If not, widen the 10 ms rate limit.
> 6. (H-RIG8) Turn the TB-3's own CUT, press PLAY: the HAND wire does not overwrite it; set CUT to SEQ and PLAY: it does.
> 7. (H-RIG9) Set the TB-3 to MIDI channel 3, then OMNI if it has it: record whether the notes and CCs follow the FM-1's CH1 MIDI channel.
> 8. (**H-RIG20, scatter linkage**) Pick `TB-3 WET`; wires 3 and 4 are SCTYP and SCDEP. Put the channel on a held or repeating note and turn the TB-3's own scatter off by hand first. Listen for, and write down: **(a) zones:** with SCDEP up (turn K4 on ROW 3 until the effect is clear), turn K3 (SCTYP) one detent at a time: how many different effects do you hear before it wraps, and does every detent give a new one (if two detents sound the same, the zone is wider than the profile thinks)? **(b) depth:** with SCTYP on a clearly audible type, turn SCDEP from 0 up: does 0 mean no effect, is it smooth? **(c) off:** SCTYP on its first detent and SCDEP at full: is there any effect? (that is the gated case). **(d) change under depth:** SCDEP high, turn SCTYP across three detents: click, hang, restart, silence? Does it wait for a beat? **(e) resend:** SCDEP high, change SCTYP, then nudge SCDEP one detent: does the effect jump (the device needed the depth again)? **(f) bar line:** SEQ on SCTYP (hold CHN, tap A3 until the chip reads `~`, tap D#4 for BACK), press HITS dice while playing: the type changes only on the next bar line, never between, and never in a burst. Write the answers next to `SCTYP` / `SCDEP` in section 12 and the TSV (`S<n>`, `:G`, `:D`, `:R`).
> 9. (S-set) Turn ALGO through `BITE WET PADS PUSH` while stopped: each change shows the toast and the four names; nothing is sent (the TB-3's knobs do not move); after each, turn one ROW 3 knob and hear it.
> Write the results in the profile (sections 2, 8 and 12) and the TSV; level 2 stays when step 1 passes from the FM-1.

## 11. S21 re-walk

[S21](../scenarios/S21-rig-device-knobs.md) uses the real names of the BITE set: wires `CUT` (CC74, home 64, ACCNT, ACC, M48), `RESO` (CC71, 40, ARTIC, WALK, M24), `ENV-X` (CC12, 64, SHAPE, RAMP, M32), `ENV-Y` (CC13, 64, none, RAND, M0); the second ALGO detent now lands on `TB-3 BITE`. New scenario [S23](../scenarios/S23-rig-sets-scatter.md) walks WET, the scatter pair, a SEQ select on the bar line, the HITS dice and the palette swap. `check_scenarios.py` passes with the TB-3 rows.

## 12. Scatter: a select and its depth (CC 68 and CC 69)

| Fact | Tag |
| --- | --- |
| CC 68 moves the scatter **type** audibly; the owner reads it as a mode select (discrete) | [V-ear, decision 106] |
| CC 69 moves the scatter **depth** audibly; linked to the type | [V-ear, decision 106] |
| Neither is in the vendor SysEx chart; the names `SCTYP` / `SCDEP` are ours | [V-doc: absent] |
| The number of types, the zone edges, off behaviour, behaviour of a change under depth, resend, clock | [I], H-RIG20 |

What the profile does about it, whatever the unknowns turn out to be (LOWLEVEL 9.2 and 9.3):
- **SCTYP is kind S8**: eight equal zones of 16 CC values (placeholder). A knob turn moves one zone per detent and sends the CC at the centre of the zone 150 ms after the last detent; the ROW 3 cell shows `#3`. SEQ picks the zone from the HITS seed, within `spread` zones of the base zone, only on a bar line (form DICE: until the layer is diced; form BAR: and by bar number mod 4).
- **SCDEP is kind C, linked to SCTYP**: an ordinary wire that rides ACCNT with the ACC form (accent steps scatter deeper). The set WET puts it next to SCTYP.
- **Why HITS drives the type.** The type is a *where-and-how* choice: HITS is the dice layer present in all six algorithms, and one press then re-rolls the rhythm and the scatter flavour together. SHAPE would freeze on EUCLID; ARTIC on SNAKE.
- **Why the depth rides ACCNT.** The accent steps are where a producer wants the glitch; the lane is the same ACC form as the cutoff, so it follows edits and the ACCNT dice for free.
- **Nothing sweeps through modes and nothing bursts**: one CC per bar line at most, the centre of a zone, and `MOVE` 0 pins the type.
- **Flags that stay off until H-RIG20:** `:G` (depth cell dims while the type is in zone 1), `:R` (resend the depth 1 ms after each type change), `:D` (send depth 0, the type, then the depth).

## 13. Dice layers and morph rows per algorithm family

### 13.1 Which layers exist (ALGORITHM_SET 4.2; sub-labels are the algorithm's own words)

| Layer | ACID | EUCLID | SNAKE | STAGE | ARP | PATTERNS |
| --- | --- | --- | --- | --- | --- | --- |
| NOTES | NOTES | NOTES | NOTES | NOTES | VARY | VARY |
| HITS | HITS | ROTATE | GATES | RESTS | RESTS | ORDER |
| ACCNT | ACCNT | ACCNT | VELO | VELO | VELO | ACCNT |
| ARTIC | GT+SLD | RATCH | **absent** | RT+HLD | LENGTH | SLIDES |
| SHAPE | OCTAVE | **absent** | DRIFT | LONG | PHASE | UP/DN |

### 13.2 What each dice button re-rolls on the device (default lanes)

| Dice | Wire(s) it re-rolls | Same in every algorithm? |
| --- | --- | --- |
| NOTES | none (pitch changes leave the sound alone) | yes |
| HITS | **SCTYP** (new scatter flavour at the next bar line) | yes (HITS exists in all six) |
| ACCNT | **CUT, PAD-Z, SCDEP** (the ACC lanes follow the accents) | yes |
| ARTIC | **RESO** (and EFFEC if the player gave it a lane) | SNAKE has no ARTIC: the button is dim; set application writes SHAPE instead |
| SHAPE | **ENV-X, PAD-Y** | EUCLID has no SHAPE: set application writes HITS instead (ENV-X then re-aims with the rhythm) |
| ALL | every wire with a layer | yes |

### 13.3 Suggested RIDE per family (the morph that scales MOVE; **default is none**, so S21's numbers hold)

Slots: K1..K4 = 1..4, S1..S4 = 5..8 (OCT-/OCT+ row 1 / 2). The player sets RIDE on the RIG page (OCT-/OCT+); this is what to suggest, one line per role.

| Role (wires) | ACID | EUCLID | SNAKE | STAGE | ARP | PATTERNS |
| --- | --- | --- | --- | --- | --- | --- |
| accent lanes (CUT, PAD-Z, SCDEP): the accent-depth morph | K4 ACCNT | K4 ACCNT | S3 DYN | S3 DYN | S3 DYN | K4 ACCNT |
| texture lane (RESO): the articulation morph | K2 SLIDE | K3 RATCH | none (no ARTIC) | K4 RATCH | S2 LEGATO | S2 SLIDE |
| sweep lanes (ENV-X, PAD-Y): the range morph | K3 JUMP | S2 SPAN | K3 RANGE | S4 RANGE | K3 SPAN | K3 SPAN |
| select (SCTYP): none (a select has a spread, not a depth to scale) | - | - | - | - | - | - |

### 13.4 What each set does per family (the lanes that move; "freeze" = the dice button is dim and the lane is silent, unless the stand-in of owner question B is written)

| Set | ACID | EUCLID | SNAKE | STAGE | ARP | PATTERNS |
| --- | --- | --- | --- | --- | --- | --- |
| BITE | all three lanes | ENV-X rides ROTATE (stand-in) | RESO rides DRIFT (stand-in) | all three | all three | all three |
| WET | CUT, SCTYP, SCDEP | same | same | same | same | same (all use ACCNT and HITS) |
| PADS | all | PAD-Y rides ROTATE (stand-in) | RESO rides DRIFT (stand-in) | all | all | all |
| PUSH | CUT, RESO | CUT, RESO | CUT; RESO rides DRIFT (stand-in) | CUT, RESO | CUT, RESO | CUT, RESO |

**WET is the one set that behaves identically on all six algorithms**: it was built from the layers that exist everywhere.

## 14. Owner questions raised by P2-021

The list with lettered options and recommendations is LOWLEVEL 9.9 (A sets on ALGO, B stand-in layers, C FOLLOW, D SEL behaviour, E run H-RIG20 first, F the four sets, G the strict gesture count of a swap, H provenance level 2, I the ROW 3 SWAP latch). None changes a locked decision.
