# PREVIEW: per-channel preview and six demo presets (ACID LEAD BASS PAD STRINGS PIANO)

Task P2-023 (owner decision 109), **amended by P2-024 (owner decision 110, 2026-11-11)**. Design only: no firmware. The table rows this design
needs are now added (section 7). **Status: owner decision 110 applied; awaiting human review of P2-024.** Inputs: VISION rule 2 (the synth stays a
preview tool), decisions 80, 98-110, SYSTEM_MAP section 2 and rows 4-12, SEQUENCER_UX 4.4, ADR-0003, MODULATION.md, `firmware/src/eng_analog.c`,
`core.h`, `voice.c`, `mod.c`.

Numbers marked **[M]** are measured from the source or the generated headers; **[E]** are estimates that a P3/P4 task must measure;
nothing here is [V] on hardware (a hearing test is part of HW-1, section 7).

## 0. In one paragraph

Each of the four channels owns one **preview voice part** (`trk[ch-1]`, the four parts the core already has: no new RAM). A channel's
PREVIEW is **ON or OFF** (all four are **OFF** at power-on, decision 110/2) and has one of **six sounds**. **Preview and MIDI are always the same
output** (decision 110/7): every note, velocity, gate and slide the channel sends as MIDI, and every mod-slot value it sends as a CC, is
what the preview part plays; there is no speaker-only or MIDI-only behaviour of the notes themselves (a channel's PREVIEW switch decides
only whether its part is heard). A channel whose MIDI is OFF sends nothing, so its preview is silent too. Turning PREVIEW on or off is **one
gesture** (hold the channel's mute key 0.4 s); choosing the sound is **one held gesture** (hold the same key and turn ALGO). The level is the MASTER
pot only. All six sounds are ANALOG patches; PIANO is an **ANALOG e-piano approximation** (decision 110/1A), and section 2.7 says plainly what
that cannot do. The internal synth maps everything the channel is set up to send, best effort (section 3).

This **supersedes** ADR-0003's rule "the preview synth plays the selected channel on `trk[0]`" and SEQUENCER_UX 4.4's "OFF means no MIDI out only; the
selected channel is still heard". Both are amended (section 6, decision 8A, done in this task).

## 1. The preview model (deliverable 1)

### 1.1 State

| Item | Rule |
| --- | --- |
| Per channel | `prv_on` (1 bit) and `prv_snd` (0..5: ACID LEAD BASS PAD STRINGS PIANO). One byte in `ChanSave` (`prv = on << 7 | snd`), stored in the 16 presets (+4 B per `SeqPreset`) and restored by autosave. Not part of undo (set-and-forget choice) |
| Runtime | the part `trk[ch-1]` is loaded from a **patch** (section 2) when `prv_snd` changes or a preset loads; the slot-CC overrides of section 3 are the only other runtime state (a few bytes per part). Voices are the existing 8, shared by all four parts (`NVOICE` 8, voice.c steals the oldest) |
| Global | **the MASTER pot is the preview level** (decision 110/4A; CONTROL_MAP: it already sets "preview-synth and USB-audio volume"). Nothing else is global: no engine, no effects, no menu row |
| Defaults at power-on | **all four channels OFF** (decision 110/2). The sounds are pre-assigned so that switching a channel on already sounds suitable: CH1 ACID, CH2 BASS, CH3 LEAD, CH4 PAD. A fresh PLAY is silent on the speaker and sends MIDI to the TB-3 as in the reference workflow; the first-run guide (S01) shows how to switch it on |

### 1.2 Output routing: one output, one switch

Preview is not a second behaviour. The channel produces one event stream; `sq_emit` hands every event to MIDI out **and** to the channel's
preview part when `prv_on`. There are only these cases:

| State | Result |
| --- | --- |
| `prv_on`, MIDI 1..16 | the notes go to MIDI out and to the speaker and USB audio, identical events |
| not `prv_on`, MIDI 1..16 | MIDI only: nothing from the speaker (the reference workflow with the TB-3) |
| MIDI OFF (`prv_on` or not) | **the channel sends nothing, so the preview plays nothing**: it keeps running and showing its lane, silent on both. The old "speaker only" mode and ADR-0003's "OFF still previews" are removed (decision 110/7). Turning PREVIEW on for such a channel works and is remembered; the toast says why nothing sounds (`CH2 MIDI OFF: PREVIEW SILENT`, 169 px), and the Z1 icon is drawn dim |

A **muted** channel is silent in both (MUTE is unchanged: the preview part gets note-offs, the lane keeps running). SOLO behaves as for MIDI.
Stop, panic and MIDI disconnect release the preview notes with the same `sq_all_off()` call (a note-off per sounding `snd[]` entry on the
part it started on). Because the preview plays exactly the MIDI event stream, there is no preview-only logic to test apart from the
patch table and the CC map (section 3).

### 1.3 Keys and audition

The 27 keys (REC, step audition, the playing keyboard) send on the **selected channel's** MIDI channel as before and sound on that channel's
preview part **only when its PREVIEW is ON** (same events, same rule). Selecting a channel whose PREVIEW is OFF makes the keys MIDI-only, which is
the point of the TB-3 workflow. A key on a channel with MIDI OFF sends and plays nothing; the same toast `CH2 MIDI OFF: PREVIEW SILENT` explains it
(once per second at most). The keyboard's own velocity and force reach the preview as the MIDI velocity, so a hard press accents the ACID patch
exactly as it accents the TB-3.

### 1.4 Gestures: at most 2 each, none in the menu, with on-screen role labels

| Function | Gesture | Count | Why this control |
| --- | --- | ---: | --- |
| PREVIEW on/off of channel n | **hold the black mute key of channel n for 0.4 s** (F#3 G#3 A#3 C#4); fires at 0.4 s; toast `CH1 PREVIEW ON: STRINGS` / `CH4 PREVIEW OFF` | **1** | the mute keys are the per-channel keys on every page that has them; a tap (under 0.4 s, acts on release) stays MUTE. Works on PERF, NOTE EDIT, CHAN and SET, for any channel, selected or not. (On REC and SCALE the black keys are notes or the KEY; press HOME first.) |
| Choose the sound of channel n | **hold the same key and turn ALGO**; each detent steps ACID, LEAD, BASS, PAD, STRINGS, PIANO (no wrap) and **turns PREVIEW ON** if it was OFF; releasing the key after a turn does not toggle MUTE or PREVIEW | **2** (a hold, then a turn of one encoder, with the thumb on the key) | ALGO is the "what is this channel" encoder; held-modifier roles are shown on screen (VISION 1). One held key, one encoder: no menu, no page |

**Re-examination against decision 110/a ("really easy to switch preview on and off per channel and to change the preset per channel").** The
alternatives were: a tap on a mute key (rejected: the tap is MUTE, performance-critical, and a mute is not a preview switch), a CHAN page cell
(rejected: adds a page visit, steals a knob; it is a setup route, not "easy" mid-performance), a button (all 14 are owned, SYSTEM_MAP rows), the
menu (breaks rule 1). Hold-the-mute-key is the only control that is **per channel, always one reach away, and already mentally "this channel's
switch"**, so 6A stands (also the owner's answer 6A). It meets the target: on/off = 1 gesture, sound = 2 (hold + turn, below the setup budget of
VISION 1), and both work **while the sequence plays**, with no page change.

**On-screen role labels (VISION 1).** While a mute key is held past 0.25 s: the strip (Z6) reads `HOLD KEY + ALGO = SOUND` (152 of 204 px) and
the ALGO cell shows that channel's **preview sound name** in place of the algorithm (the six names are the cell's values), so the encoder's
changed role is on screen while it is changed. The held key does not change the channel's algorithm. At 0.4 s the toast confirms the
on/off. The speaker icon of the channel cell (1.5) shows the state before and after, so a user never has to remember it. HELP (HOME held,
touch the key) explains it (`help_text.tsv`, section 7).

### 1.5 What is shown

| Where | What |
| --- | --- |
| Z1 channel cell (60x28) | a **6x6 speaker pictogram** (two `cv_rect`s and a wedge, no font glyph needed) at the right end of line 1 while PREVIEW is ON: T_ACCENT while a preview note sounds, T_TEXT otherwise, dim in a muted cell or when MIDI is OFF. Line 2 is unchanged (`MIDI 2`, `MIDI OFF`, `MUTE 2`). Absent = MIDI only. A text fallback was tried and rejected: `PRV STRINGS` is 72 px against a 56 px limit (measured) |
| Toasts and strip (Z6, 204 px limit; all measured in `widths.tsv`) | `CH1 PREVIEW ON: STRINGS` 148, `CH4 PREVIEW OFF` 99, `CH3 PREVIEW: PIANO` 114 (a sound change), `HOLD KEY + ALGO = SOUND` 152, `PREVIEW: MUTED CHANNEL` 151 (an ON toggle on a muted channel), `CH2 MIDI OFF: PREVIEW SILENT` 169 (ON toggle or audition on a MIDI OFF channel) |
| CHAN page | no new knob (K1..K4 are full); the icon and toasts are the whole display (decision 110/3A) |
| HELP | `help_text.tsv` rows for the four mute keys carry the hold; EN_ALGO rows carry the held-key role (section 7) |

### 1.6 Global versus channel settings

Channel: ON/OFF, sound. Global: the MASTER pot is the level (decision 110/4A). The menu stays for set-and-forget settings and gets **no** PREVIEW row
(SPEAKER EQ and USB LEVEL, already in AUDIO, apply to the preview as the only audio source).

## 2. The six presets (deliverable 2)

### 2.1 What the hardware can do

| Resource | Fact | Consequence |
| --- | --- | --- |
| Voices | `NVOICE` 8 shared by all parts [M] (voice.c steals; with NPART 4 one voice per part is protected) | PAD and STRINGS are **POLY with at most 4 notes**, never UNISON (UNISON spends every voice of the part on one note [M, voice.c:307]). With four channels previewing, chord sounds can steal from lines; MONO sounds cost one voice each |
| ANALOG voice | 2 band-limited oscillators (SAW SQR TRI SIN PWM, SYNC, SUB), noise, drive, trapezoidal SVF LP/BP/HP, ADSR, per-part LFO and 4-slot mod matrix [M] | all five synth sounds fit |
| Effects | the chorus, delay and reverb buses are **removed by P3-014** [M, decision 80]: the old STRINGS / SOFT PAD presets carry 60-70 % chorus and reverb sends and **lose them** | PAD and STRINGS get their width from OSC 2 detune (DTN), PWM width LFO and the stereo SPREAD (`P_SPRD`, kept [M, fx.c:146]); they sound drier than the old presets (stated; accepted by decision 110/5A) |
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
**LEGATO with GLIDE about 12**, so that an overlapping note-on/off pair (the slide, decision 105) glides in the preview as on the TB-3 and a gap is two notes (the `SQR BASS` and `SUB BASS` rows give the range). 1 voice. Feasible; it is what a TB-3 user previews first.

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


### 2.7 PIANO: an ANALOG e-piano approximation (decision 110/1A)

The owner chose option A: no sample or FM6 engine is kept, so the ANALOG-only removal chain (P3-008..P3-010) proceeds. PIANO is the name in the UI
and in the six-sound list, as asked; this section is the plain statement of what it is.

Starting values (from the `SINE KEY` and `PLUCK` rows of `ANALOG_PRESETS[]`, `eng_analog.c` 202-215 [M]; tuned by ear in HW-1, **not [V]**): WAVE SIN
with a second oscillator as the second partial: DTN +6 cents (MIX 50), CUT 70 with KTR 64 so the tone brightens up the keyboard, RES 0, DRV 0;
ATK 0 DEC 88 SUS 0 REL 60 (a struck tone that dies away whether or not the key is held); ENV->FILTER +30 (a bright attack that mellows, the main piano cue
the voice has); voice POLY 4 notes; velocity to LEVEL (voice default) and the kept mod matrix velocity->cutoff slot so hard keys are brighter; a very slow
tremolo (LFO 0.5 Hz to amplitude, depth 6) for the electric-piano shimmer.

**What ANALOG cannot do for a piano** (stated, not hidden): no inharmonic string partials and no hammer noise (the NOISE source shares the amplitude envelope, so
it cannot be a click only); no per-note decay time that follows pitch (the envelope is the same for all notes; KTR moves only the filter); no sympathetic
resonance, damper or release noise; no pedal; 4 notes at once. It reads as a tine or Wurlitzer-style electric piano, not as an acoustic grand. If the owner later
wants a real piano on the device, that is a new decision (it was option B: +47 KB flash for SAMPLE with the PIANO set, rejected by decision 110).

## 3. Mapping the channel's setup to the internal synth (deliverable 2 of P2-024)

Principle (decision 110/b): **the preview plays the channel's MIDI stream.** The internal synth takes what the channel sends and maps every
event to a synth parameter if one exists, and says "no equivalent" if not. Nothing is added to the pattern for the preview.

### 3.1 Notes and the note-level events

| Channel event | Synth action | Per preset |
| --- | --- | --- |
| Note on/off, pitch | `trk_note_on/off(&trk[ch-1], note, vel)` | all six |
| Gate / length (note-off time) | the note-off starts REL of the amplitude envelope; a gate shorter than ATK+DEC cuts the note in the attack/decay | all six; PIANO's SUS 0 means a long gate adds nothing after the decay (a piano does the same) |
| Velocity | voice amplitude (the voice's velocity scaling, `voice.c`) and the kept mod matrix slot `VEL -> cutoff` (`mod.c`, AMT in the patch) | all six; the cutoff amount is strongest in ACID, medium in LEAD, BASS and PIANO, 0 in PAD and STRINGS (a pad should not wah with velocity) |
| **Accent** (velocity >= 100, flat velocity 90, decision 104) | the same velocity: the preview has **no threshold at 100**; accent is the *continuous* VEL->cutoff and amplitude step from 90 to 120. The hardware's threshold-and-bite is **approximated, not reproduced** | ACID: the patch is tuned so 120 opens the filter clearly (the 303 gesture); BASS, LEAD: audible; PIANO: a harder key; PAD, STRINGS: louder only |
| **Slide** (next note-on before the previous note-off, decision 105) | on a MONO or LEGATO part an overlapping note-on glides from the held pitch (`P_GLIDE`, no new envelope trigger) and a gap is two notes | **ACID, LEAD, BASS: yes** (LEGATO or MONO with GLIDE 12 to 20). **PAD, STRINGS, PIANO: no equivalent**: they are POLY, so the overlap plays two notes together |
| Ratchet, retrigger | extra note-on/off pairs in the stream | all six |
| Mute, MIDI OFF, stop, panic | notes released through `sq_all_off()`; a muted or MIDI OFF channel produces no events | all six |
| Program change, pitch bend, pressure | not sent by the sequencer (nothing to map) | n/a |

### 3.2 Device CCs from the four mod slots (MODULATION.md)

The slot value is computed in the ISR and sent by `sq_emit` as a CC. The preview applies the same value to the channel's part through a **map
keyed by (device, CC)**: a new column `prv` in the device profile (`tools/hwprof/<dev>.tsv`, P4-024) naming a target code (CUT, RES, ENVF, DEC, ACCB,
LFOP, LEV, PAN, ATK, REL, GLD or `-`). The override lives while the slot is active and the base (patch) value returns when the slot parks (STOP,
mute: MODULATION decision 100/101), when the device or slot changes, and when the sound changes. Slot values are channel-local (each channel has
its own part), so CH1's CUT slot never moves CH2's filter. The map is a flash table of about 3 B per entry (about 40 B for both devices), no RAM beyond
one override byte per target per part. Nothing changes on the screen: the user sees the same MOD page and hears the same movement.

**TB-3 profile (the 11 CCs of MODULATION.md section 1):**

| CC | TB-3 name | Synth target (`prv`) | What it does on the preview | Per preset |
| ---: | --- | --- | --- | --- |
| 74 | CUTOFF | CUT = `P_E4` | filter cutoff, slot value 0..127 replaces the patch base | all six |
| 71 | RESONANCE | RES = `P_E5` | filter resonance; the range is capped at about 110 on the POLY sounds so a chord cannot self-oscillate [E] | all six |
| 12 | ENV MOD X | ENVF = `P_ED_FLT` | envelope-to-filter amount | all six (PAD, STRINGS and PIANO base is low, so it adds a bloom or bite) |
| 13 | ENV MOD Y | DEC = `P_DEC` | the shared envelope's decay time (ANALOG has one ADSR for amplitude and filter) | all six |
| 16 | ACCENT | ACCB = `P_M1AMT` | the amount of the velocity-to-cutoff slot, i.e. how much an accent bites | ACID, LEAD, BASS, PIANO; PAD, STRINGS: **no equivalent** (slot AMT is 0) |
| 11 | PAD Y | none | **no equivalent**: the TB-3's touch pad is an assignable effects controller the preview cannot imitate | none |
| 1 | PAD Z | none | **no equivalent**, as PAD Y | none |
| 17 | EFFECT | none | **no equivalent**: the effects buses are removed (P3-014); there is nothing to send to | none |
| 104 | TUNING | none | **no equivalent in v1**: the part has no fine tune (only integer `P_TRANS` and the global `G_TUNE`, which would move all four channels) | none |
| 68 | SCATTER TYPE | none | **no equivalent** (no scatter effect) | none |
| 69 | SCATTER DEPTH | none | **no equivalent** | none |

**GENERIC device (standard General MIDI / XG controller meanings; a CC the player sets by number):**

| CC | Meaning | Synth target | CC | Meaning | Synth target |
| ---: | --- | --- | ---: | --- | --- |
| 1 | mod wheel | LFOP = `P_LD_PIT` (vibrato depth) | 72 | release time | REL = `P_REL` |
| 5 | portamento time | GLD = `P_GLIDE` | 73 | attack time | ATK = `P_ATK` |
| 7 | channel volume | LEV = `P_LEVEL` | 74 | brightness | CUT = `P_E4` |
| 10 | pan | PAN = `P_PAN` | 75 | decay time | DEC = `P_DEC` |
| 11 | expression | LEV = `P_LEVEL` | 71 | resonance | RES = `P_E5` |
| 91, 93, 94 | reverb, chorus, detune sends | none (effects removed) | any other | | none |

LFOP, LEV, PAN, ATK, REL and GLD apply to all six sounds. For GLD on the POLY sounds the value is accepted but has no audible effect.

### 3.3 What ANALOG cannot do (summary)

- **No threshold accent.** Accent is a continuous velocity response, not the TB-3's switch at 100.
- **No slide on PAD, STRINGS, PIANO** (POLY parts); an overlap is two notes.
- **No effects:** TB-3 EFFECT, scatter type and depth, and the GENERIC sends 91/93/94 have nothing to drive.
- **No fine tune** (TB-3 TUNING), no touch-pad axes.
- **One envelope** for amplitude and filter, so ENV MOD X and Y share the amplitude ADSR.
- **No sample-accurate pitch glide curve match:** the glide is the voice's exponential portamento, tuned by ear to sound like the 303 (HW-1).
- **PIANO limits:** section 2.7.
- 8 voices shared by all four previews: a chord on CH3 and CH4 together steals the oldest note.

## 4. Effects on tasks (deliverable 4)

**P3-008, P3-009, P3-010 may proceed** exactly as written, **ANALOG only**: option A of the old question 1 is the owner's decision (110/1A), so the
branches for keeping SAMPLE or FM6 do not apply. Their `depends_on: [P2-023]` may be cleared by the reviewer/orchestrator when P2-024 passes
review. No sample, FM6, DRUM or PHYS engine is needed by any preset (the PIANO decision above); `NENGINES` can reach 1. The held status in
SYSTEM_MAP rows 5-9 ("depends on P2-023") is lifted.

| Task | Change |
| --- | --- |
| **P3-014** (fx buses) | none, but note that the chorus and reverb sends of the STRINGS / SOFT PAD presets are gone (2.1) and the `fx[]` fields become dead |
| **P4-001 / P4-005** (types, channel state API) | `ChanSave` gains `prv` (1 B; re-check the 344 B assert and the `SeqPreset` size, +4 B for 4 channels); API `sq_prv_set(ch, on, snd)`, `sq_prv_get(ch)`; default table **all OFF**; sounds CH1 ACID, CH2 BASS, CH3 LEAD, CH4 PAD |
| **P4-004 / P4-012** (core, cut-over) | `sq_emit`: replace "if `chan == sq_sel` play `trk[0]`" by "if `prv_on[chan]` and MIDI is not OFF, play `trk[chan-1]`" (the MIDI OFF test is the same test that gates the MIDI packet: **one condition for both**); the CC emit path calls `prv_cc(ch, target, value)` from the profile's `prv` column; `sq_all_off` releases all four parts; the t1_glue single-part path goes. ADR-0003 text already amended (section 6) |
| **P4-024** (hwprof tools) | `tools/hwprof/<dev>.tsv` gets the column `prv` (target code or `-`); `tb3.tsv` as in 3.2, plus a built-in GENERIC table; `gen_hwprof.py` emits the 3 B entries |
| **P4-010** (UI tables) | the rows are **already added** (section 7); the task only has to keep the checkers green when it renames contexts |
| **P4-011** (UI framework) | the Z1 speaker icon (dim for MIDI OFF) and the held-key ALGO cell role; strip text `HOLD KEY + ALGO = SOUND` |
| **P4-013** (PERF page) | mute-key hold (0.4 s) and the ALGO-while-held modifier |
| **P4-015** (CHAN page) | none (3A: no sound name there) |
| **P4-021** (HW-1 first flash) | add: all channels OFF at boot; hold F#3 switches CH1 PREVIEW ON and ACID plays through the speaker and MIDI at once; slide and accent audible on ACID; tune the six patches by ear; check the voice cap with 4 channels |
| **New task** (proposed P4-0xx, before P4-021) | `preview.c`: the patch table, `prv_load(part, snd)` (all-notes-off, then set the part's parameters), `prv_cc` and the (device, CC) map, host tests: **T-PRV1** with PREVIEW on, the preview note events equal the MIDI note events (same notes, velocities, order, same off times) and a MIDI OFF channel makes neither; T-PRV2 no note leaked on a sound change; T-PRV3 slot CC overrides apply and park; goldens for the six patches; the ISR cost (`BUDGET_UPDATE`) |
| **P5 algorithm tasks** | **no change**: the algorithms already emit velocity (90/120), gate and overlap; the preview follows the stream. ACID's accent and slide layers are the ones the preview must show, so P5 acceptance tests need not add a preview check, but P5-001 (ACID) should list "audible on ACID preview" in its HW checklist |
| **P6-003** (mute keys, solo, fill) | the hold on a mute key is now PREVIEW, not a second mute role: 0.4 s threshold shared with the HOLD menu setting |
| **P7-001..P7-002** (flash map, preset store) | `SeqPreset` +4 B; load restores `prv` and loads the parts at the bar-queued load |
| **P8-002** (user guide) | one paragraph: preview on/off (hold the mute key), six sounds (hold + ALGO), preview is the same output as MIDI |

## 5. Decisions honoured, point by point (decision 110)

| Decision 110 point | How |
| --- | --- |
| 1A: PIANO = ANALOG e-piano approximation; no sample / FM6 engine kept; ANALOG-only chain proceeds | 2.7 (starting values and the stated limits); section 4 lifts the hold on P3-008..P3-010 |
| 2: all channels preview OFF by default | 0, 1.1 (defaults row), section 4 P4-001 and P4-021 |
| 3A: speaker icon plus toasts only | 1.5 |
| 4A: the MASTER pot is the preview level | 0, 1.1, 1.6 (no menu row, no per-channel trim) |
| 5A: PAD and STRINGS drier without chorus and reverb | 2.1 and 2.6 |
| 6A: hold the mute key, hold + ALGO for the sound | 1.4, re-examined against "really easy" with the alternatives listed |
| 7: preview and MIDI always the same output; whatever the channel sends as MIDI the preview plays; no separate preview behaviour | 0, 1.2 (one condition, the MIDI OFF case stated), 1.3, 3 (the preview takes the MIDI stream), T-PRV1 |
| 8A: amend ADR-0003 and SEQUENCER_UX 4.4 | section 6 (done in this task) |
| (a) easy per-channel on/off and preset change | 1.4: on/off one gesture, sound a hold + turn, no menu, no page change, on-screen labels |
| (b) the internal synth maps everything the channel sends: notes, accents, slides, gates, filter and the other device CCs, best effort | section 3: tables per event and per (device, CC), a "no equivalent" entry where there is none, 3.3 lists what ANALOG cannot do |
| 98, 108 (device with 4 mod slots) | the preview reads the same slots (3.2), inert with no device |
| 104, 105 (accent >= 100, slide by overlap) | 3.1 |
| 80, 102, 107, 109 | patches are flash tables; fx removal is the premise of 5A; the preview is a consumer of the mod layer |
| VISION 2 | no synth editing, no engine browsing, no per-patch menu |
| VISION gesture budget | on/off 1, sound 2, none in the menu |

## 6. Amendments made (decision 8A)

Done in this task (not "to be amended"):

1. **ADR-0003**: addendum section 21 "Preview per channel (P2-024)" and the contradicted lines (the event-path diagram, 8.1 preview bullet, the key
   path in 9, `ChanSave.midi_ch` comment, boot and defaults rows, the `synth_note_on/off` row) now say: preview = each channel with `prv_on` on `trk[ch-1]`,
   the same event stream as MIDI out, silent on MIDI OFF.
2. **SEQUENCER_UX 4.4** "Preview synth" row: "OFF means no MIDI out and no preview. A channel is heard only if its PREVIEW is ON, and then it plays exactly what it sends
   as MIDI; muted channels stay silent on both, SOLO ignores OFF".
3. Not edited (they follow at P4): SYSTEM_MAP section 2 wording ("one fixed bass-suited ANALOG patch", question 40: six patches now), `SCREEN_SPEC.md` 2.2 (the icon).

The reference workflow in VISION is unchanged: a TB-3 user leaves PREVIEW OFF (the default), or holds F#3 once to hear the line.

## 7. Table rows added

- `key_map.tsv`: the four mute keys (F#3 G#3 A#3 C#4) on PERF, NOTE_EDIT, CHAN, SET and RIG gain a `hold` row (`0.4 s: PREVIEW on/off of CH n`; with ALGO turned: the sound); `control_map.tsv` and `help_text.tsv` carry the same text for those keys.
- `control_map.tsv` / `help_text.tsv` `EN_ALGO` on PERF, NOTE_EDIT, CHAN, SET, SCALE, REC: while a mute key is held, the encoder chooses that channel's preview sound.
- `page_states.tsv` PERF: the Z1 speaker icon, the held-key strip and the ALGO cell role.
- `tests/design/widths.tsv`: the six toast and strip strings of 1.5 (measured, 0 over).
- Scenarios not edited here: S01 (first run) and S04 (channels) should gain "hold F#3: CH1 PREVIEW ON, ACID plays on the speaker" and "hold + ALGO: sound change"; their coverage lines are in the P4 table task. The checkers pass without them (every control row already has a scenario).

## 8. Owner sheet

**No real choice remains.** Decision 110 settled every question. Two small design consequences for the owner's information (no answer needed unless you disagree):

1. A channel with MIDI OFF is silent on the preview too (the direct reading of 110/7). The earlier "speaker only" mode is gone.
2. The preview has no threshold accent and no fine tune, no pad or effect equivalent (3.3). They are stated limits of best-effort mapping on the ANALOG engine.

## 9. Checks run for this document

`python3 tests/design/check_scenarios.py`, `python3 tests/design/check_help.py`, `python3 tools/check_sequencer_ux.py`, `sh tests/design/width_probe.sh`,
`python3 tools/backlog.py check`: results are in the task log of P2-024.
