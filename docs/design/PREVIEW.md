# PREVIEW: per-channel preview and six demo presets (ACID LEAD BASS PAD STRINGS PIANO)

Task P2-023 (owner decision 109). Design only: no firmware, no table rows are changed here (the proposed rows are listed in section 7, for
the P4 table task, as MODULATION.md did). **Status: proposed, awaiting owner review.** Inputs: VISION rule 2 (the synth stays a
preview tool), decisions 80, 98-109, SYSTEM_MAP section 2 and rows 4-12, SEQUENCER_UX 4.4 (MIDI OFF), ADR-0003 (preview = selected
channel on `trk[0]`), MODULATION.md, `firmware/src/eng_analog.c`, `core.h`, `voice.c`, the generated `build/gen/felucca_samples.h`.

Numbers marked **[M]** are measured from the source or the generated headers; **[E]** are estimates that a P3/P4 task must measure;
nothing here is [V] on hardware (a hearing test is part of HW-1, section 7).

## 0. In one paragraph

Each of the four channels owns one **preview voice part** (`trk[ch-1]`, the four parts the core already has: no new RAM). A channel's
PREVIEW is **ON or OFF** and has one of **six sounds**. When ON, the channel's notes go to the speaker and USB audio **and** to MIDI out
(unless the channel's MIDI is OFF, which gives "speaker only"). When OFF the channel is MIDI only, which is today's reference workflow.
ON/OFF is one gesture (hold the channel's mute key 0.4 s); choosing the sound is one more (hold the same key and turn ALGO). The six
sounds are tiny ANALOG patches, except PIANO, which is honest: ANALOG alone cannot play a piano, and the owner chooses between an
analogue electric-piano approximation and keeping a trimmed copy of the SAMPLE engine with the PIANO set (section 2.7, question 1).

This **supersedes** ADR-0003's rule "the preview synth plays the selected channel on `trk[0]`" and SEQUENCER_UX 4.4's "OFF means no MIDI
out only; the selected channel is still heard". Section 6 lists the consequences.

## 1. The preview model (deliverable 1)

### 1.1 State

| Item | Rule |
| --- | --- |
| Per channel | `prv_on` (1 bit) and `prv_snd` (0..5: ACID LEAD BASS PAD STRINGS PIANO). One byte in `ChanSave` (`prv = on << 7 | snd`), stored in the 16 presets (+4 B per `SeqPreset`) and restored by autosave. Not part of undo (it is a set-and-forget choice, like MIDI assignment's sibling, but nothing else depends on it) |
| Runtime | the part `trk[ch-1]` is loaded from a **patch** (section 2) when `prv_snd` changes or a preset loads; no other runtime state. Voices are the existing 8, shared by all four parts (`NVOICE` 8, voice.c steals the oldest) |
| Global | **the MASTER pot** is the one global preview level (CONTROL_MAP: it already sets "preview-synth and USB-audio volume"). Nothing else is global: no engine, no effects, no menu row in v1 |
| Defaults at power-on | CH1 ON ACID; CH2 OFF BASS; CH3 OFF LEAD; CH4 OFF PAD (the sounds are assigned so that switching a channel ON already sounds suitable; question 2 asks whether all four should default ON) |

### 1.2 Output routing: three modes, no new setting

| Mode | State | Result |
| --- | --- | --- |
| Both | `prv_on` and MIDI 1..16 | notes to MIDI out and to the speaker (default for CH1) |
| Speaker only | `prv_on` and MIDI OFF | notes to the speaker only. Removes the ADR-0003 rule that OFF "still previews the selected channel": now it previews only if that channel's PREVIEW is ON |
| MIDI only | not `prv_on` and MIDI 1..16 | the reference workflow with the TB-3: nothing from the speaker |
| (silent) | not `prv_on` and MIDI OFF | the channel runs and shows its lane but is inaudible (as today's OFF) |

A **muted** channel is silent in both (MUTE is unchanged: the preview part gets note-offs, lane keeps running). SOLO behaves as for MIDI.
Stop, panic and MIDI disconnect release the preview notes with the same `sq_all_off()` call (a note-off per sounding `snd[]` entry on the
part it started on).

### 1.3 Keys and audition

The 27 keys (REC, step audition, the playing keyboard) sound on the **selected channel's** MIDI channel as before, and on that channel's
preview part **only when its PREVIEW is ON**. (Before: always on `trk[0]`.) Selecting an OFF channel makes the keys MIDI-only, which is the
point of the TB-3 workflow; when an audition would be completely silent (PREVIEW OFF and MIDI OFF) the toast reads `CH3: MIDI OFF + PREVIEW OFF` (not in the measured set; P4-011 measures it).

### 1.4 Gestures (at most 2, none in the menu)

| Function | Gesture | Count | Why this control |
| --- | --- | ---: | --- |
| PREVIEW on/off of channel n | **hold the black mute key of channel n for 0.4 s** (F#3 G#3 A#3 C#4); fires at 0.4 s; toast `CH1 PREVIEW ON: STRINGS` / `CH1 PREVIEW OFF` | 1 | the mute keys are already the per-channel keys on every page; a tap (under 0.4 s, acts on release) stays MUTE. No other function owns the hold of those keys (checked in `key_map.tsv`: rows for F#3..C#4 are tap only) |
| Choose the sound of channel n | **hold the same key and turn ALGO**; each detent steps ACID, LEAD, BASS, PAD, STRINGS, PIANO (no wrap) and **turns PREVIEW ON** if it was OFF; releasing the key after a turn does not toggle MUTE or PREVIEW | 2 | ALGO is the "what is this channel" encoder; held modifier roles are shown on screen (VISION 1). Works for any channel, selected or not |

Neither needs the menu, both are within the setup budget (two gestures; on/off is one). While a mute key is held past 0.25 s the strip (Z6)
reads `HOLD KEY + ALGO = SOUND` (152 of 204 px); the ALGO cell shows the preview sound name instead of the algorithm while the key is
down, and the 6 names are the cell's values. The held key does not change the channel's algorithm: the ALGO encoder is a modifier here,
like OCT+ held (MODULATION 5.2). HELP (HOME held, touch the key) explains it.

### 1.5 What is shown

| Where | What |
| --- | --- |
| Z1 channel cell (60x28) | a **6x6 speaker pictogram** (two `cv_rect`s and a wedge, no font glyph needed) at the right end of line 1 while PREVIEW is ON: T_ACCENT while a preview note sounds, T_TEXT otherwise, dim in a muted cell. Line 2 is unchanged (`MIDI 2`, `MIDI OFF`, `MUTE 2`). Absent = MIDI only. A text fallback was tried and rejected: `PRV STRINGS` is 72 px against a 56 px limit (measured) |
| Toasts (Z6, 204 px limit, all measured) | `CH1 PREVIEW ON: STRINGS` 148, `CH4 PREVIEW OFF: MIDI ONLY` 162 (MIDI on), `CH2 SPEAKER ONLY: MIDI OFF` 162 (MIDI OFF, PREVIEW ON), `CH3 PREVIEW: PIANO` 114 (a sound change), `HOLD KEY + ALGO = SOUND` 152, `PREVIEW: MUTED CHANNEL` 151 (an ON toggle on a muted channel, so the silence is explained) |
| CHAN page | no new knob (K1..K4 are full). The sound name is shown in the page's channel line `CH2 MIDI 3 +SPK BASS`-style only if the P4-015 layout has room; otherwise the Z1 icon and the toasts are the whole display (P4-015 decides, question 3) |
| HELP | `help_text.tsv` gets one row per new gesture (P4 table task) |

### 1.6 Global versus channel settings

Channel: ON/OFF, sound. Global: MASTER pot level only. The menu stays for set-and-forget settings and gets **no** PREVIEW row in v1
(SPEAKER EQ and USB LEVEL, already in AUDIO, apply to the preview as the only audio source). Question 4 asks about an AUDIO row
`PREVIEW LEVEL` (a trim against the MASTER pot) if the six patches turn out uneven in loudness.

## 2. The six presets (deliverable 2)

### 2.1 What the hardware can do

| Resource | Fact | Consequence |
| --- | --- | --- |
| Voices | `NVOICE` 8 shared by all parts [M] (voice.c steals; with NPART 4 one voice per part is protected) | PAD and STRINGS are **POLY with at most 4 notes**, never UNISON (UNISON spends every voice of the part on one note [M, voice.c:307]). With four channels previewing, chord sounds can steal from lines; MONO sounds cost one voice each |
| ANALOG voice | 2 band-limited oscillators (SAW SQR TRI SIN PWM, SYNC, SUB), noise, drive, trapezoidal SVF LP/BP/HP, ADSR, per-part LFO and 4-slot mod matrix [M] | all five synth sounds fit |
| Effects | the chorus, delay and reverb buses are **removed by P3-014** [M, decision 80]: the old STRINGS / SOFT PAD presets carry 60-70 % chorus and reverb sends and **lose them** | PAD and STRINGS get their width from OSC 2 detune (DTN), PWM width LFO and the stereo SPREAD (`P_SPRD`, kept [M, fx.c:146]); they sound drier than the old presets (stated, question 5) |
| RAM | `trk[]` 7296 B already holds the four parts [M, SYSTEM_MAP row 4]; headroom after P3-014 is about 18.5 KB of `.bss`, about 46 KB after the whole removal chain [M, SYSTEM_MAP section 9] | the patches are **flash tables, 0 B RAM** |
| CPU | not measured for 4 parts previewing 8 voices [E] | the regress budget (`BUDGET_UPDATE`) must be re-baselined in the dev task; ADR-0003's +5 % ISR rule applies. The worst case is 8 PWM or SUB voices through the SVF with the LFO on |

### 2.2 The patch record

Each patch is a flash record, about 30 B [E]: the 8 engine parameters (`P_E0..E7`), `ATK DEC SUS REL`, ENV->FILTER, voice mode (MONO,
LEGATO, POLY), GLIDE, DTN, SPREAD, LFO wave/rate/depth to shape or filter, LEVEL, FTYPE, and the velocity-to-cutoff slot of the kept
mod matrix. The six patches total about 180 B flash. They start from the existing `ANALOG_PRESETS[]` (the values quoted are those rows,
`eng_analog.c` 202-215 [M]) and are tuned by ear in HW-1; the numbers below are the **starting values, not [V]**.

### 2.3 ACID (TB-303-like)

ANALOG, WAVE SAW, DTN 0, MIX 0, NOIS 0, CUT 50, RES 100, DRV 25, KTR 64; ATK 0 DEC 55 SUS 20 REL 30; ENV->FILTER +48 (the existing `ACID` row);
voice mode **LEGATO** with GLIDE about 20 so that the algorithm's overlapping note-on/off pair (the slide, decision 105) glides in the
preview just as on the TB-3 (a gap = two notes); FTYPE LP; velocity to cutoff so the 120-velocity accent (>= 100, decision 104) opens the
filter (the kept matrix, SYSTEM_MAP row 11). Mono: 1 voice. Feasible.

### 2.4 LEAD

ANALOG, SAW, DTN 12, MIX 64, CUT 90, RES 30, DRV 10, KTR 64; ATK 4 DEC 70 SUS 100 REL 50; ENV->FILTER +20 (the `SAW LEAD` row); MONO with
GLIDE 20 (glide on legato notes); a 5 Hz LFO to pitch, depth 5, faded in (`P_LFADE`) for vibrato. 1 voice. Feasible.

### 2.5 BASS

ANALOG, WAVE **SUB** (saw plus an octave-down square, MIX = the sub level 64), CUT 45, RES 20, DRV 30, KTR 40; ATK 0 DEC 60 SUS 60 REL 20; ENV->FILTER +30;
MONO, no glide (the `SQR BASS` and `SUB BASS` rows give the range). 1 voice. Feasible; it is what a TB-3 user previews first.

### 2.6 PAD and STRINGS (ANALOG, no effects)

| | PAD | STRINGS |
| --- | --- | --- |
| Wave | PWM | SAW |
| E parameters (WAVE DTN MIX NOIS CUT RES DRV KTR) | 4, 20, 64, 4, 60, 10, 0, 32 | 0, 25, 64, 0, 70, 10, 0, 32 |
| ATK DEC SUS REL | 80 90 110 95 (slow in, long out) | 70 90 115 90 |
| Width | LFO (0.3 Hz) to the PWM width, depth 40; SPREAD 60 | DTN 25 cents; slow filter LFO depth 10; SPREAD 70 |
| Voice | POLY, 4 notes | POLY, 4 notes |
| Source rows | `SOFT PAD` / `PWM STR` | `STRINGS` |

Feasible with a **caveat**: both lose the chorus and reverb sends the old rows carry (P3-014), so they are thinner and drier. They read as
"pad" and "string ensemble" in a demo, not as a recording. 4 voices each when a chord plays; with CH3 and CH4 both previewing chords the
8-voice cap steals (voice.c steals the oldest, one voice per part protected).

### 2.7 PIANO: the one that does not fit ANALOG

ANALOG has no inharmonic partials, no hammer noise and no per-note decay that tracks pitch. Options (question 1):

| Option | What | Cost | Honest result |
| --- | --- | --- | --- |
| **A. ANALOG E-piano** | SIN WAVE, DTN +1 cent pair, a second partial via PWM or SYNC at low depth, fast ATK 0, DEC 88, SUS 0, REL 60 (the `PLUCK` / `SINE KEY` shape), cutoff following the key (KTR 64) and velocity | 0 B | a tine/e-piano pluck; not a piano. Renamed in the UI would be `E.PNO`; the owner asked for PIANO, so this is a downgrade to say aloud |
| **B. Keep SAMPLE with the PIANO set only (recommended)** | engine SAMPLE on that part, SET PIANO: 5 zones across the keyboard, IMA ADPCM at 22 kHz, the real piano of Felucca 1.0 to 1.5 | flash: PIANO data about **41.4 KB** [M, `SMP_ZONES` offsets 0..41350 + the last zone]; plus `eng_sample.c` (263 lines) about 5 KB [E]; RAM about 0.5 KB [E] once the three user slots go (`usr_zone` 3x16 zones); pool 0 (the file never uses `ENG_MEM` [M]); CPU: ADPCM decode per voice, one voice per note [E, measure] | a real piano; the other sets (FLUTE, SAX, BREAK, 3 user slots, GRAIN, SLICE) still go |
| C. Keep FM6 with a DX-style E-piano voice | 6-operator FM, patch of 155 B | flash 11.5 KB, RAM 1.5 KB, pool 0.5 KB [M, SYSTEM_MAP rows 7]; CPU heaviest of the three [E] | a good electric piano, not an acoustic one; the editor that loads patches goes |

Option B costs about 47 KB of the 234 KB the removal chain frees (20 %); the flash image after the chain would be about 335 KB instead of 288 KB [E]
and RAM headroom stays above 45 KB. A and B are not exclusive: A is the fallback if B's CPU does not fit.

## 3. Interaction with the mod layer (deliverable 3)

The mod layer (MODULATION.md) drives an **external device** through four slots per channel: each slot is a device parameter, its
value goes out as a CC (TB-3 CC74 cutoff, CC71 resonance, CC12 env mod, ...). **Preview responds to the same slots, for the parameters the
preview patch has**, because the slot value is already computed in the ISR and `sq_emit` already sends it:

| Slot CC (device profile) | Preview target on the channel's part | Behaviour |
| --- | --- | --- |
| 74 (cutoff, filter) | `P_E4` CUT | the slot's 0..127 value sets the filter live (the preview patch value is the base; the slot replaces it while active) |
| 71 (resonance) | `P_E5` RES | as above |
| 12 (env mod X / env amount) | ENV->FILTER | as above |
| any other CC (accent 16, tuning 104, scatter 68/69, pad 11/1, effect 17) | none | the preview ignores it, and the slot's toast/ride dot is unchanged |

Rules: a table of 3 CCs in the preview module (about 12 B flash), applied only when the channel's `prv_on`; the base value returns when the slot
parks (STOP, mute: MODULATION decision 100/101), when the device or slot changes, and when the sound changes. The slot values are
channel-local (each channel has its own part), so CH1's CUT slot never moves CH2's filter. No new gesture and no screen change: the user
sees the same MOD page and hears the same movement on the speaker, which is the point of the preview. Cost: one compare and an index in
the CC emit path, no new RAM. A GENERIC device that uses CC74 / 71 also moves the preview; a device with other numbers is silent in the preview.

## 4. Effects on tasks (deliverable 4)

| Task | Change |
| --- | --- |
| **P3-008** (DRUM, PHYS) | none: neither is needed by any preset. Its `depends_on: [P2-023]` can be cleared once the owner answers question 1 |
| **P3-009** (sample family, FM6) | **depends on question 1.** A: unchanged. B: keep `eng_sample.c` and the PIANO set; delete FLUTE, SAX, BREAK, user slots, `eng_grain.c`, `eng_slice.c`, `slice data`, `fm6*`; `tools/gen_samples.py` emits PIANO only; the criterion "flash image shrinks by at least 140000 B" becomes "at least 90000 B" [E]; the grep list drops `eng_sample.c` and its public symbols; sample_test is kept cut to PIANO. C: keep `eng_fm6.c`, `fm6_core.c` and a built-in patch table; delete the vbank, `fm4_convert.c` and the editor hooks |
| **P3-010** (remaining engines, ANALOG only) | A: unchanged. B: the goal becomes "ANALOG plus SAMPLE (PIANO)": `NENGINES` 2 (ANALOG 0, SAMPLE 1; indices need not stay stable because the project formats that stored them go in R7), `eng_idx()` still returns 0 for a bad index; the `grep -c ENG_MEM` criterion still 0; the pool drop stays about 51552 B. Re-baselined regress renders cover ANALOG and the PIANO part. P3-010's title and the P3-008 / P3-009 / P3-010 chain note in SYSTEM_MAP rows 5-9 change accordingly |
| **P3-014** (fx buses, in review) | none, but its Log should note that the chorus and reverb sends of the STRINGS / SOFT PAD presets are gone (section 2.1); the preview patches here do not use the `fx[]` fields, which become dead |
| **P4-001 / P4-005** (types, channel state API) | `ChanSave` gains `prv` (1 B, padded: re-check the 344 B assert and the 1496 B `SeqPreset`, +4 B for 4 channels); API `sq_prv_set(ch, on, snd)`, `sq_prv_get(ch)`; default table CH1 ON ACID, CH2 BASS, CH3 LEAD, CH4 PAD |
| **P4-004 / P4-012** (core, cut-over) | `sq_emit`: replace "if `chan == sq_sel` play `trk[0]`" by "if `prv_on[chan]` play `trk[chan-1]`"; `sq_all_off` releases all four parts; the t1_glue single-part path goes; ADR-0003 4.x text amended (ADR addendum, owner sign-off) |
| **P4-010** (UI tables) | rows for the new gestures (section 7) and the toasts in `widths.tsv`; HELP rows |
| **P4-011** (UI framework) | the Z1 speaker icon and the held-key ALGO cell role; strip text `HOLD KEY + ALGO = SOUND` |
| **P4-013** (PERF page) | mute-key hold (0.4 s) and the ALGO-while-held modifier |
| **P4-015** (CHAN page) | optionally shows the sound name (question 3); no knob change |
| **P4-021** (HW-1 first flash) | add: PREVIEW ON on CH1 plays ACID through the speaker and MIDI at once; tune the six patches by ear; check the voice cap with 4 channels |
| **New task** (proposed id P4-0xx, before P4-021) | `preview.c`: the patch table, `prv_load(part, snd)` (all-notes-off, then set the part's parameters; PIANO switches the engine through the existing `eng_req` fade), the 3-CC mod map, host tests: determinism (same state, same events), no note leaked on a sound change, goldens for the six patches, the ISR cost (`BUDGET_UPDATE`) |
| **P5 algorithm tasks** | none (a preview patch is independent of the algorithm; the channel keeps its sound when ALGO changes) |
| **P6-003** (mute keys, solo, fill) | the hold on a mute key is now PREVIEW, not a second mute role: 0.4 s threshold shared with the HOLD menu setting |
| **P7-001..P7-002** (flash map, preset store) | `SeqPreset` +4 B; load restores `prv` and loads the parts at the bar-queued load |
| **P8-002** (user guide) | one paragraph: preview on/off, six sounds, speaker only |
| `SEQUENCER_UX.md` 4.4 row "Preview synth", ADR-0003 preview text, `SCREEN_SPEC.md` 2.2, `knob_map`/`control_map`/`key_map.tsv`, scenarios S04/S01 | superseded or extended as in section 7; **not edited here** (the owner has not approved this design) |

## 5. Decisions honoured (deliverable 5 check)

| Decision | How |
| --- | --- |
| 80 (16 slots; fx-bus removal first) | patches are flash tables; `SeqPreset` +4 B only. The removal of the buses is the premise of the PAD / STRINGS caveat |
| 98, 108 (device with 4 mod slots) | preview reads the same slots (section 3), inert with no device |
| 99-101, 103-106 (profiles, TB-3 facts) | untouched: ACID uses decision 104/105 (accent >= 100, slide by overlap) only inside the preview |
| 102 (fx removal precedes preset storage) | unchanged |
| 107 (modulation layer as one design) | preview is a consumer, not a bolt-on |
| 109 (this task) | per-channel PREVIEW, six presets, MIDI as well; the PIANO limit is stated, not hidden, and the engine-removal tasks stay held on question 1 |
| VISION 2 (synth is a preview tool) | the six patches are preview voices; no synth editing UI, no engine browsing, no per-patch menu |
| VISION gesture budget | on/off 1, sound 2, none in the menu |

## 6. Consequences for existing rules (to be amended after the owner answers)

1. ADR-0003: preview = selected channel on `trk[0]` becomes preview = each channel with `prv_on` on `trk[ch-1]`.
2. SEQUENCER_UX 4.4 "Preview synth" row: "OFF means no MIDI out only. The selected channel is still heard" becomes "OFF means no MIDI out; the channel is heard only if its PREVIEW is ON".
3. SYSTEM_MAP section 2 ("one fixed bass-suited ANALOG patch", question 40) becomes six patches; row 5 (ANALOG only) depends on question 1.
4. The reference workflow in VISION is unchanged: a TB-3 user sets CH1 PREVIEW OFF once, or leaves it on to hear the line.

## 7. Proposed table rows (for the P4 table task; not added here)

- `key_map.tsv`: `KEY_B01_F#3 .. KEY_B04_C#4`, every page, gesture `hold`: `0.4 s: PREVIEW on/off of CH n (toast)`; and `hold+turn ALGO`: choose the sound.
- `control_map.tsv` `EN_ALGO` (every page): while a mute key is held, the encoder chooses that channel's preview sound.
- `widths.tsv`: the six toast strings of 1.5 (measured with `tests/design/width_probe.sh`).
- Scenario: S01 (first run) gains "CH1 PREVIEW ON plays ACID on the speaker"; S04 (channels) gains hold-mute and sound change.

## 8. Owner questions (lettered options, a recommendation each)

1. **PIANO.** A: ANALOG e-piano approximation, zero cost, not a real piano. B: keep SAMPLE with the PIANO set only, +47 KB flash, +0.5 KB RAM, a real piano. C: keep FM6 with an E-piano patch, +13 KB flash, +2 KB RAM, a DX-style electric piano. **Recommend B**, with A as the fallback if its CPU does not fit. Until you answer, P3-009 and P3-010 stay held.
2. **Defaults.** A: CH1 ON, others OFF (recommended: a TB-3 user hears one preview, PLAY is audible with no setup). B: all four ON. C: all OFF (silent first run; contradicts SYSTEM_MAP section 3).
3. **Show the sound name where?** A: speaker icon in the channel cell plus toasts only (recommended: no layout change). B: also a name on the CHAN page (P4-015 decides). C: a fifth cell label in the channel cell (does not fit 56 px: `PRV STRINGS` is 72 px).
4. **Preview level.** A: only the MASTER pot (recommended). B: also an AUDIO menu row `PREVIEW LEVEL` (a trim, set and forget).
5. **PAD and STRINGS without chorus and reverb.** A: accept the drier sound (recommended; it is a preview). B: ask for a small chorus kept inside the voice code for these two (a separate task, costs a delay line of about 4 KB RAM or pool, against the decision to remove the chorus).
6. **Gesture.** A: hold the mute key 0.4 s, and hold + ALGO for the sound (recommended). B: a CHAN page cell (adds one gesture, steals a knob). C: a menu row (breaks "no menu diving" for a performance-adjacent function).
7. **Keys and audition while PREVIEW is OFF.** A: MIDI only (recommended: consistent). B: the keys always preview on the selected channel's part even if OFF (louder surprise, more code).
8. **Amend ADR-0003 and SEQUENCER_UX 4.4** as in section 6 now (A, recommended once you accept this design) or leave them until the P4 task.

## 9. Checks run for this document

`tests/design/check_scenarios.py`, `tests/design/check_help.py`, `tools/check_sequencer_ux.py`, `sh tests/design/width_probe.sh`,
`tools/backlog.py check`: results are in the task log. The six toast strings were measured with the probe on a scratch file (all within
204 px); they are not added to `widths.tsv` until the owner accepts them.
