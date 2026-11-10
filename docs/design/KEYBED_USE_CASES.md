<!-- SPDX-License-Identifier: GPL-3.0-only -->
# KEYBED_USE_CASES: what force sensing does to the sound (P2-015)

Design only. Replaces the mechanism list of SEQUENCER_UX 6.1 (FS1-FS4) with use cases a producer would ask for. Hardware limits are in [KEYBED_EXPRESSION_REFERENCE.md](KEYBED_EXPRESSION_REFERENCE.md) (noisy velocity, small aftertouch range, only F3..C5 sense force, lit LEDs skew readings). Tags: **[V]** verified in a source I read, **[I]** inferred, **⚠** unconfirmed. Calibration, LED handling and costs (SEQUENCER_UX 6.2-6.4) are unchanged except where 8 says so.

## 1. What the TB-3 answers to over MIDI

| Fact | Tag | Source |
| --- | --- | --- |
| CC 74 = CUTOFF, CC 71 = RESONANCE, CC 16 = ACCENT (the Accent amount), CC 17 = EFFECT, CC 12 / 13 = ENV MOD X / Y, CC 104 = TUNING; each is a 0..255 controller in the SysEx "Controller" map | [V] | Roland, *TB-3 MIDI Implementation* v1.10, 2014-07-04, "Parameter Address Map, Controller" (https://cdn.roland.com/assets/media/pdf/TB-3_MI_1.pdf; text extracted and read) |
| CC 16 is the **amount** of the accent knob, not a per-note accent; the chart names no per-note accent message | [I] | same table (name only) |
| Whether note-on velocity >= 100 triggers the TB-3 accent | [V] | verified by ear 2026-11-11 (decision 104): 80, 90, 95 do not accent, 100 and above do; threshold = 100 |
| Slide by CC 102 and accent by CC 103 | ⚠ | one search snippet citing Roland update notes; conflicts with CC 16 above; not used here |
| Slide from overlapping notes, receive range C0..C8 | ⚠ | search snippets, TB-03 and TB-3 mixed; not used here |
| No aftertouch, pitch bend or CC absorbed per note | ⚠ | none appears in the chart I could read; assume the TB-3 ignores aftertouch |
| TB-3 receives on MIDI channel 2 by default | [V] | CLAUDE.md (owner-observed) |

Consequence: the only force result that is certain to change the TB-3 sound without a CC is **which steps get the accent and how many notes play**. Velocity-as-level matters on the TB-3 only through the accent threshold, which the HT-FORCE gate must confirm by ear (item H0).

**Resolved (decision 104):** the TB-3 accent threshold is 100 (verified by ear 2026-11-11), so the generators' flat velocity is 90 (TIMING_SPEC, snake/arp/patterns dyn 0). Written REC notes and the fallback use 90 (see 7); HARD 120 accents, MED 90 does not.

## 2. The use cases, ranked

All run on the **selected channel's MIDI channel** (the TB-3 on 2 by default). "Poly" means a generic poly synth on its own channel. No use case needs a reading finer than 3 bins (section 6).

### UC1 "Make this one bite" (hard tap = accent). Was FS1. Rank 1

- **Gesture:** PERF / NOTE_EDIT, tap a white step key F3..C5 hard to turn the step ON. The cell shows `ACC` while the key is down.
- **TB-3:** that step plays with the 303 accent: louder, brighter, a shorter filter sweep, on the beat the producer chose. **Poly:** a louder note at velocity 120.
- **MIDI:** `ch n NOTE ON key vel 120` on that step (stored as VELO 120, no new data); every other step unchanged. No CC.
- **Screen:** `ACC` in the pressed cell before release, `!` glyph after.
- **Fallback:** NOTE_EDIT `ENV` or `K3 VELO`; D5..G5 always use it.
- **Why it works:** 2 bins; the bin is fixed at landing and is visible before it counts (the tap acts on release). The only misread is a soft tap shown as plain: no harm, one more try.

### UC2 "Build into the drop" (lean on FILL). Was FS3. Rank 2

- **Gesture:** hold G#4 FILL, lean harder.
- **Sound (both):** the line gets busier the harder the producer leans; release and it falls back to the dialled density. The classic push before a break.
- **MIDI:** more note-ons per bar on the selected channel (density morph); the extra steps use the generator's own velocity and pitch, so on the TB-3 nothing but the rhythm changes. No CC.
- **Screen:** `FILL 80%` live; the held cell inverted.
- **Fallback:** maximum density.
- **Why it works:** light = halfway, firm = maximum is 2 effective levels plus the continuous smoothing; the readout makes the result visible before the next step. No finer reading is needed.

### UC3 "Roll it" (lean on ROLL). Was FS2. Rank 3

- **Gesture:** hold A#4 ROLL, lean for 2x, 3x, 4x.
- **Sound:** every step becomes a flam, triplet or 16th-roll; on the TB-3 a stuttering bass, with the **first hit of an accented step keeping its accent and the repeats at 90** so the roll does not become one long accent.
- **MIDI:** per step, n note-ons spread evenly (ratchet spacing rule, SEQUENCER_UX 10), gate = half the sub-step. First hit at the step velocity; repeats at `min(step velocity, 90)`. Same channel, no CC. This repeat-velocity rule is **new**; it adds no per-step data.
- **Screen:** `ROLL 3x` live.
- **Fallback:** fixed 2x.
- **Why it works:** 3 bins with 10 % hysteresis (gate G6).

### UC4 "Play the line in with feel" (REC dynamics). Was FS4. Rank 4

- **Gesture:** in REC, play any of F3..C5 (20 keys) soft, medium or hard.
- **Sound:** the recorded line has dynamics: ghost notes, normal notes, accents. TB-3: soft and medium both play unaccented (verified by ear, decision 104), hard accents. Poly: three loudness levels.
- **Change from before:** the landing force is **snapped to three levels, SOFT 64 / MED 90 / HARD 120**, not 1..127. The reference shows one chord's fingers reading 59/76/127/104 (KEYBED_EXPRESSION_REFERENCE 2); three levels absorb that noise and make the TB-3 result predictable.
- **MIDI:** the live note and the written note use the level velocity. HARD = accent (>= `ACC_THR`).
- **Screen:** the REC band shows `v MED 90` after each key.
- **Fallback:** velocity 90 (was 100, which is the verified accent threshold), or 120 with K2 ACC ON.
- **Why it works:** edges are fractions of each key's calibrated range (section 6); the value is visible after the key, and an unwanted level is fixed by NOTE_EDIT `K3`.

### UC5 "Swell the held chord" (REC aftertouch, poly synths only). New. Rank 5

- **Gesture:** in REC, after landing, keep pressing a held key (F3..C5) harder.
- **Sound:** a poly or pad synth that maps aftertouch swells, opens or vibratos while the note is held. The TB-3 (⚠) ignores it, so this is a **poly-synth** use case and ships OFF.
- **MIDI:** channel pressure `0xDn p` (n = the channel's MIDI channel, p 0..127) of the highest pressed key, sent when p moves by >= 8, at most once per 20 ms; one `0xDn 0` at release. Range: the pressure above the landing reading in DRIVE mode, linear, clamped. Never written into steps. The gate is the table flag `REC_AT` (ADR-0004 constants), default OFF; no menu row.
- **Screen:** `v MED 90  AT 64` on the REC band while the key is down.
- **Fallback:** flag OFF: plain notes.
- **Why it works:** no finer reading is promised, the receiver smooths it, and the small range only costs depth.
- **Weakest case:** it needs DRIVE mode and a poly synth, so it is the first to cut if the gate is hard to pass.

(Fewer than six: none of the other candidates beat the section 3 reasons for dropping.)

## 3. The earlier "not used" items, reconsidered

| Item | Verdict | Reason |
| --- | --- | --- |
| Pressure sent to MIDI as a **CC** (for example CC 74 cutoff while a FILL is leaned on) | **Drop for v1** | Absolute CCs cannot be undone: the FM-1 does not know where the TB-3 knob is, so release would leave the filter somewhere unintended. No force-capable black key is free (B01-B08 are all taken). The TB-3 knobs are under the producer's other hand. Table kept in section 1 for a later task that captures a home value |
| Pressure as **aftertouch** over MIDI | **Add as UC5**, REC only, OFF | The only poly-synth use that needs no new key and no restore problem |
| Pressure to a **morph** knob value (beyond FILL) | **Drop** | No free held key; the knobs are exact and faster; the pressure range cannot land a chosen value repeatably |
| Pressure to the **HARMONY dial** (VOICE / MOVE / DRONE) | **Drop** | The dials are set-and-forget with 7 to 15 positions (HARMONY.md); 3 bins cannot select among them, and a change would not return on release |
| Release velocity | **Drop** | Not sent by the reference; no receiver uses it |
| Per-step ratchet / slide / chance amounts from force | **Drop** | Unchanged: NOTE_EDIT knobs are exact |

## 4. Input UX summary

| Use case | See before it counts | Fallback |
| --- | --- | --- |
| UC1 | `ACC` in the pressed cell | NOTE_EDIT ENV / K3 |
| UC2 | `FILL 80%` live | maximum |
| UC3 | `ROLL 3x` live | 2x |
| UC4 | `v MED 90` after the key | velocity 90 / ACC |
| UC5 | `AT 64` live | flag OFF |

`FORCE OFF` turns every force reading into its fallback; nothing depends on force.

## 5. Output summary

| Use case | Message | Values | Channel |
| --- | --- | --- | --- |
| UC1 | note-on velocity | 120 on that step | the channel's MIDI channel |
| UC2 | more note-ons | density morph | same |
| UC3 | repeated note-ons | 2..4 per step, repeats at min(vel, 90) | same |
| UC4 | note-on velocity (live and written) | 64 / 90 / 120 | same |
| UC5 | channel pressure | 0..127, >= 8 change, 20 ms | same |

## 6. Bins against the hardware

- **Landing bins (UC1, UC4):** the existing key `floor` (hardest seated reading) and the gentle default (SEQUENCER_UX 6.2). HARD = within the top 25 % of the range as before; UC4 adds MED = 25 % to 60 %, SOFT = the rest. The chord noise of KEYBED_EXPRESSION_REFERENCE 2 (59..127) spans more than the 3 bins, so the bins are tested per key by ear (G1, G7), never promised as exact.
- **Pressure bins (UC2, UC3, UC5):** DRIVE mode only; UC3 has 3 bins with hysteresis, UC2 is smoothed and bounded to two useful levels, UC5 is coarse (>= 8 of 127). If DRIVE fails G5 or G6, UC2 / UC3 / UC5 use their fallbacks (SEQUENCER_UX 6.6).
- **Coverage:** F3..C5 only (20 keys); D5..G5 and the top black keys have the fallbacks.

## 7. Cost and changes to other sections

- **RAM: no new per-step data.** UC4 and UC5 reuse the 242 B force state (SEQUENCER_UX 6.4); the repeat-velocity rule (UC3) and the 3 levels are constants.
- **Changes to SEQUENCER_UX 6:** FS4 becomes 3 levels; FS5 (UC5) added; the "not used" paragraph replaced by section 3; gate G7 (REC levels) and H0 added. Section 6.2-6.3 untouched. The REC fallback velocity moves from 100 to 90 in `key_map.tsv`, `control_map.tsv`, CONTROL_MAP.md and scenario S14 (and S07 text) so that it does not equal `ACC_THR`.
- **Not touched:** the 6.2 calibration, LED measures H1-H3, ADC schedule.

## 8. Hardware-test checklist (HT-FORCE, judged by ear on the TB-3, channel 2; UC5 on a poly synth)

| ID | Test | Pass |
| --- | --- | --- |
| H0 | Send note-ons at velocity 80, 99, 100 and 120 on ch 2 | DONE 2026-11-11 (decision 104): accent starts at 100; `ACC_THR` 100 and `ACC_VEL` 120 stand |
| H1 (UC1) | Hard-tap 10 steps in a running line with the PERF LEDs lit | The accent is audible on >= 9 of 10 hard taps; no soft tap accents |
| H2 (UC2) | Lean on FILL through a bar | The pattern audibly gets busier in two distinct amounts; release restores it within one step |
| H3 (UC3) | Lean on ROLL: 2x, 3x, 4x | Each reached on purpose in >= 8 of 10 tries; an accented step keeps one accent only |
| H4 (UC4) | Record 16 notes: 4 soft, 8 medium, 4 hard | >= 85 % land in the intended level; on playback the hard ones accent and the rest do not |
| H5 (UC5) | `REC_AT` ON, poly synth with aftertouch mapped to filter | A held chord's filter follows pressure; stopping the press returns to 0; no stuck value after release |

G7 (SEQUENCER_UX 6.6) is the numeric form of H4.
