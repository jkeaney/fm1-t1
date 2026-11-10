<!-- SPDX-License-Identifier: GPL-3.0-only -->
# TB3_PROFILE: the Roland TB-3 profile (phase 3 of the hardware support model)

Task P2-020 (owner decision 96; model in [HW_SUPPORT_LOWLEVEL.md](HW_SUPPORT_LOWLEVEL.md), phase 2). Status: **for owner review** (gate `human-review`). Design and data only: no firmware.
The data file is [`tools/hwprof/tb3.tsv`](../../../tools/hwprof/tb3.tsv) (format of LOWLEVEL 2.4); `tools/gen_hwprof.py` arrives with P4-024.

Tags: **[V]** verified (document or hardware), **[I]** inferred or unconfirmed, with its source in the row. "V-doc" = read in the vendor document; "V-ear" = heard on the owner's TB-3 on 2026-11-11 (firmware version not recorded; true of that unit only).

## 1. Sources and the rule applied (decision 99)

| Source | Use |
| --- | --- |
| **Roland, *TB-3 MIDI Implementation*, v1.10, 2014-07-04**, "3. Parameter Address Map, Controller" (https://cdn.roland.com/assets/media/pdf/TB-3_MI_1.pdf; text extracted and read in P2-015/P2-020) | the CC numbers and names: the only second source. It is a SysEx map that **annotates** each controller with a CC number; it has no receive-CC table and does not say which channel or whether every CC is accepted from a host |
| The owner's own tests on the owner's TB-3, 2026-11-11 (decisions 104, 105) | accent, slide, receive channel |
| midi.guide `tb3-midi-guide.csv` and the CDM article | **cross-check only, not copied**: nothing in this file or the TSV comes from the CSV. Where the CSV lists something the vendor chart does not (CC 1, 11, 68, 69, 102, 103) it is **not used** (single source) |

Rule used: a CC enters the profile only if the vendor chart names it. A CC that only the CSV lists is left out and left to a later H-test.

## 2. The parameter table (7 entries; the four suggested wires are marked)

Names are at most 5 letters and measured in AF_S with `tests/design/width_probe.sh` (limit 38 px): CUT 24, RESO 29, ENV-X 34, ENV-Y 34, ACC 24, EFFEC 34, TUNE 31 px. Range 0..127 for all [I]: the vendor map's own range is 0..255 (two nibbles, SysEx), a CC carries 0..127, so the FM-1 sends the full CC range and the device scales it (**not verified on hardware**: H-RIG1 confirms that 0 and 127 reach the ends).

| CC | Name | Vendor name [V-doc] | Range | Home | Suggested wire | Tag of the function |
| ---: | --- | --- | --- | ---: | --- | --- |
| 74 | CUT | CUTOFF | 0..127 | 64 | **wire 1** | [V-doc] number and name; [I] that it is the audible filter cutoff on a host CC (H-RIG1) |
| 71 | RESO | RESONANCE | 0..127 | 40 | **wire 2** | as above |
| 12 | ENV-X | ENV MOD X | 0..127 | 64 | **wire 3** | [V-doc] number and name; [I] what it does by ear (the pad X axis of the env mod; the vendor map gives only the name). The name stays neutral until H-RIG1 |
| 13 | ENV-Y | ENV MOD Y | 0..127 | 64 | **wire 4** | as ENV-X |
| 16 | ACC | ACCENT | 0..127 | 64 | no | [V-doc] number; [I] it is the **amount** of the accent knob, not a per-note accent (the name is all the chart gives). Kept in the table so a user can ride it; not suggested |
| 17 | EFFEC | EFFECT | 0..127 | 0 | no | [V-doc]; the effect amount |
| 104 | TUNE | TUNING | 0..127 | 64 | no | [V-doc]; an unmusical live move (pitch), so not suggested |

`home` is the neutral the park uses (LOWLEVEL 3.4). **[I]**: no source gives the device's resting value; 64 is the middle, RESO 40 and EFFEC 0 are the placeholders of S21 and are confirmed or changed by H-RIG2 (set the knob by hand, read the number). It is a one-line TSV change.

**Not in the table, left out on purpose** (single source: the CSV only, so unconfirmed): CC 1 (pad Z), 11 (XY pad Y), 68 / 69 (scatter type / depth), 102 / 103 (pattern slide / pattern accent; the names say they act on the TB-3's own pattern, [I]). They are reachable on GENERIC. H-RIG1 may promote any of them.

## 3. The four suggested wires and the lane mapping

`cfg` of the TSV is `layer << 4 | form << 2`; all four start in **HAND** at `home`, and send nothing until the player turns a knob (decision 100).

| Wire | Param | Layer | Form | MOVE | Why this layer rides this wire |
| ---: | --- | --- | --- | ---: | --- |
| 1 | CUT 74 | ACCNT | **ACC** | 48 | the classic 303 gesture: an accented note opens the filter. ACC follows the accent steps themselves (edits, the ACCNT dice and the ACCNT morph for free), so it needs no seed. Value = LEVEL +48 on an accent, -48 otherwise. Dicing ACCNT moves sound and filter together |
| 2 | RESO 71 | ARTIC | WALK | 24 | a slow wander that changes when the gate and slide layer is diced: articulation and resonance read together as "the note's character". Small depth, because a wide resonance move on a held note rings |
| 3 | ENV-X 12 | SHAPE | RAMP | 32 | a sweep over the loop that the SHAPE dice re-aims (direction and start), the build every bassline player does with the env mod |
| 4 | ENV-Y 13 | none | RAND | 0 | an armed but silent wire: MOVE 0 so it does nothing until the player raises it; it is the fourth knob for hand-play and the place a later suggestion changes after H-RIG1 |

If the algorithm lacks the layer the lane freezes (LOWLEVEL 3.1, ALGORITHM_SET 4.4). ACC form is independent of the algorithm, so wire 1 always works.

## 4. Accent (`acc_mode`, `acc_arg`)

- **Method: velocity threshold.** `acc_mode` = 0 (VEL), `acc_arg` = **100**. [V-ear, decision 104/105] notes at 80, 90, 95 do not accent; 100, 105, 110, 120 do (owner's TB-3, channel 2).
- The sequencer's flat non-accent velocity is 90 and accent 120 (decision 104), both on the right side of the threshold.
- CC 16 is **not** the accent method: it is the amount of the accent effect [I, name only]; whether it scales the accented sound by ear is left to H-RIG3. Therefore no `CC` mode is needed and `acc_arg` carries 100, not a CC number.
- An accent (velocity 120) on a note that is slid into sounds like an unaccented one [V-ear, decision 105]. The sequencer need not avoid it, but the ACCNT dice may show a slide-destination accent as inaudible (a UI note for P4, not a change here).

## 5. Slide (`slide_mode`, `slide_arg`)

- **Method: overlap.** `slide_mode` = 0 (OVERLAP), `slide_arg` = 0 (unused). [V-ear, decision 105] a note-on sent **before** the previous note-off glides (no second envelope); a gap gives two separate notes. This is the 303 behaviour the core already assumes (ADR-0003 7.3), so the TB-3 needs no special case.
- CC 102 / 103 are not used for slide or accent on incoming notes (CSV-only names, [I] that they act on the internal pattern); H-RIG4 confirms they do nothing to played notes.
- Timing consequence for P4: the overlap must be at least one scheduler tick, and the previous note-off must not be sent before the next note-on (the glide ends if it is).

## 6. Receive channel

Default **MIDI channel 2** [V-ear, decision 105 (notes on ch2) and CLAUDE.md]. The chart does not state a receive-channel rule [V-doc: none]. The profile does not set the channel (the user sets the channel's MIDI channel, VISION rule 3; a TB-3 whose own channel was changed needs the FM-1 channel changed to match). Rule shown to the player: **the CC goes on the channel's MIDI channel**, so notes and CCs always share a channel; H-RIG9 checks that a TB-3 set to another channel (or OMNI) behaves.

## 7. Provenance record (the `HwProv` fields)

| Field | Value (24 characters at most) |
| --- | --- |
| `src` | `vendor MIDI chart+own test` |
| `how` | `chart; accent+slide by ear` |
| `cred` | `J. Keaney` |
| `level` | 1 (the CC numbers are in the vendor chart and in an independent database; accent and slide are verified on hardware by a named person, but the CC *functions* are not) |
| date | 2026-11 |
| MENU > INFO row | `HW DATA: TB-3`  `L1 JK 2611` |

Credit: the vendor chart is Roland's; the accent and slide facts are the owner's own measurements; midi.guide (pencilresearch/midi, CC BY-SA 4.0) and the CDM article are credited as cross-checks in the ABOUT credits, not as a data source. Level 2 follows once H-RIG1 has been run (every CC heard moving).

## 8. What is not known (left to the H-tests)

| Unknown | Tag | Resolved by |
| --- | --- | --- |
| Whether each of the 7 CCs, sent from a host, moves its knob audibly, and at both ends of 0..127 | [I] | H-RIG1 |
| What ENV-X / ENV-Y do by ear; the true resting value of each parameter | [I] | H-RIG1, H-RIG2 |
| Whether CC 16 changes the accent amount, and by how much | [I] | H-RIG3 |
| Whether CC 102 / 103 affect played notes | [I] | H-RIG4 |
| Zipper noise at a detent step of 1 and 4, at 10 ms | [I] | H-RIG5 |
| Receive channel / OMNI behaviour of a TB-3 with another setting; firmware differences (the tested unit's version was not recorded) | [I] | H-RIG9 |
| The slide overlap length that glides reliably at 300 BPM and 1/32 | [I] | H-RIG4 (add the tempo sweep) |

## 9. The H-tests for the TB-3 (pass criteria)

H-RIG1..H-RIG19 are defined in LOWLEVEL 6.3. The ones that fill this profile:

| # | Test | Pass |
| --- | --- | --- |
| H0 (done) | accent threshold by ear, decision 95 | **done 2026-11-11**: 80, 90, 95 no; 100 and above yes; `acc_arg` = 100 |
| H-RIG1 | each CC of section 2 sent on ch 2, 0 / 64 / 127, from the FM-1 through the host box | every CC moves its parameter audibly; one that does not is struck from the TSV |
| H-RIG3 | accent: velocity 99 versus 100 on every step; CC 16 swept at 0 / 64 / 127 while accented notes play | 99 plain, 100 accented; the CC 16 effect is written in section 4 |
| H-RIG4 | slide by overlap: overlap 1 tick, 5 ms, 20 ms; gap 0 (note-on at the note-off) ; CC 102 / 103 during play | overlap glides, gap 0 does not; CC 102/103 do not slide played notes. **Slide by overlap is already [V-ear] on this unit (decision 105); H-RIG4 repeats it from the FM-1 itself** |

## 10. H-RIG TB-3 test sheet (text for HARDWARE_TEST.md; P4-020 merges it)

> **H-RIG, TB-3 (owner runs it; about 20 minutes).** Rig: FM-1 USB to a host box, host box DIN to the TB-3 MIDI IN; the TB-3 on MIDI channel 2; CH1 MIDI 2, the device TB-3 picked on the RIG page, all four wires HAND.
> 1. (H-RIG1) Turn each ROW 3 knob from 0 to 127 and back: CUT and RESO must be audible on a held note. For ENV-X and ENV-Y (and, on GENERIC, CC 16, 17, 104) write down what you hear. Struck: any that does nothing.
> 2. (H-RIG2) Set the TB-3's own CUT knob by hand, read the position, and compare with the number on screen after sending the same value: they must agree within 2 detents. Record each parameter's resting value for `home`.
> 3. (H-RIG3) Play steps at velocity 99, then 100: only 100 accents. Sweep CC 16 with accented notes playing; record whether the accent gets stronger.
> 4. (H-RIG4) Play two notes overlapped by 1 tick, 5 ms and 20 ms, then a note-on exactly at the note-off, then a 20 ms gap: overlaps glide, the gap does not. Send CC 102 and 103 during play: no slide or accent on played notes.
> 5. (H-RIG5) With HAND on CUT hold a note and turn K1 at one detent per step, then four: smooth, no zipper. If not, widen the 10 ms rate limit.
> 6. (H-RIG8) Turn the TB-3's own CUT, press PLAY: the HAND wire does not overwrite it; set CUT to SEQ and PLAY: it does.
> 7. (H-RIG9) Set the TB-3 to MIDI channel 3, then OMNI if it has it: record whether the notes and CCs follow the FM-1's CH1 MIDI channel.
> Write the results in the profile (section 2 and 8) and the TSV; level goes to 2 when step 1 passes.

## 11. S21 re-walk

[S21](../scenarios/S21-rig-device-knobs.md) now uses the real names: wires `CUT` (CC74, home 64, ACCNT, ACC, M48), `RESO` (CC71, 40, ARTIC, WALK, M24), `ENV-X` (CC12, 64, SHAPE, RAMP, M32), `ENV-Y` (CC13, 64, none, RAND, M0). TUNE (the placeholder fourth wire) is out of the suggestions. `check_scenarios.py` passes with the TB-3 rows (step 2 wire list, step 6 ROW 3 cells, step 14).
