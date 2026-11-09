# Keybed expression reference (P1-015, owner requirement R4)

Researched 2026-11-10. Repos were cloned shallow into new empty directories under /tmp (`/tmp/p1015_*`) and only read; nothing was built or run. Tags: **[V]** verified in source I read, **[I]** inferred or taken from prose I could not confirm in code, **unconfirmed** = nobody has shown it.

## 1. Which project unlocked this

**Answer: Charles Vestal's "Velo" (`fm1-velocity`).** Confidence: high that it is the project; high that it is *velocity + polyphonic aftertouch*, not "MPE" in the protocol sense.

| Project | URL | Commit read | Licence | Role |
| --- | --- | --- | --- | --- |
| Velo (the origin) | https://github.com/charlesvestal/fm1-velocity | 260b978 (2026-11-07) | GPL-3.0-only (LICENSE head, README) | Reads velocity and polyphonic aftertouch from the keybed through the SoC ADC. Built on Felucca's platform layer, so GPL-compatible with this repo. |
| MONK-1 | https://github.com/charlesvestal/fm1-monk | e8e9200 | GPL-3.0 | Re-uses Velo's sensing; aftertouch moves vowel/pitch [V README lines 9-11, 28-29]. |
| Scintilla | https://github.com/joakimeriksson/scintilla-fm1 | 314d449 | GPL-3.0 | Synthex firmware; README says velocity and aftertouch from the keys, "played on a real FM-1 (2026-11-01)" [I: README only, sensing code not read]. |
| Felucca MPE | https://github.com/monch1962/mvave-fm1-mpe | 2aaeae3 | GPL-3.0 | NOT keybed. MPE *input* from an external controller (per-note bend/pressure/timbre) [V README]. Name collision is probably why the first search was confusing. |
| FM1 Open (official) | https://github.com/m-vave/fm1_open | 4c7496d | GPLv3 + FM1 SDK linking exception | Grep of `app/` and `sdk/` for velocity/aftertouch/pressure hit only midi_router.c, instrument.h, presets.c, screen.c, fm1.h; I did not read them for keybed sensing. No evidence it senses key force [I]. |

Index of FM-1 projects: https://github.com/cicloid/awesome-fm-1 (README lines 77, 79, 181, 203 list Velo, MONK-1, Scintilla, Felucca MPE). Reverse engineering: https://github.com/AL-255/FM-1-RE (`docs/io/07-input.md`: key scan, 41 key positions, 3-tick debounce, long-press at 41 ticks [V in that doc, "med" confidence by its own tags]; nothing on key force).

Nobody has implemented real MPE (per-note pitch bend / CC74 slide) from the keybed. Unconfirmed: that the hardware could. Nothing sends per-note-channel MPE from the FM-1's own keys in anything I read.

Checked and not conclusive: the Korean zeli.app story (summary only; it mentions no keybed detail and links the repos above), synthtopia/matrixsynth/sonicstate/Elektronauts pages (search snippets and the matrixsynth post only; none mention key pressure or MPE). The Elektronauts thread (30+ pages) was not read in full.

## 2. How Velo works

Source: `README.md`, `notes/01-lab.md`, `firmware/src/app/sense.c`, `firmware/src/app/ui.c`, `firmware/hal/fm1_input.h` (all [V], read).

- **Hardware:** keys are rubber domes with a carbon pill and a single contact each, scanned as a diode matrix (on/off) [V README]. A pill's resistance falls with force.
- **Trick:** the SARADC can read two of the four key rows: PA7 = ADC ch0 (notes 9..19 plus), PA8 = ch1 (notes 0..8 and OCT-/OCT+) [V notes/01-lab.md table]. After latching a column, the 10 kHz scan tick starts a conversion on PA7 (even frames) or PA8 (odd frames) and reads it next tick, so each analog key is sampled every ~2.2 ms [V fm1_input.h comment, notes]. Result: 1023 open, ~170-200 seated.
- **Coverage:** notes 0..19 (F3 to C5) are analog; the top 7 keys (PA6, notes 20..26) have no ADC. Velo estimates them by probing through the LED line PA10 and reports the probe saturates (firm presses all ~44-45), a compressor-like response [V notes].
- **ADC settings found necessary:** clock divider 7 and start delay 2; any faster divider returns rails [V notes "What the device said"]. Switching off the rows' digital input buffers breaks the scan (OCT-/OCT+ read as held, which triggers update mode) [V notes].
- **Velocity:** the dome snaps so touch-to-seat time carries no velocity (0 or 2.2 ms for every press) [V notes]. Velocity comes from how deep the reading is right after landing, relative to each key's learned floor (hard ~185, gentle 270..360). Later "DRIVE" mode samples every other sample with the row driven high by a weak pin driver, turning the pad into an ohmmeter on the pill; log-scale mapping gives quartiles 40/59/74 on regular playing [V README, sense.c header].
- **Aftertouch:** polyphonic, from the held reading, 5 ms updates smoothed ~30 ms. It is "pressing harder than the note landed" in drive mode; in pull-up mode only easing off from the floor [V sense.c]. Verdict in the notes: relaxing a held key moves the pull-up reading only a few counts to tens, so its range is small [V notes verdict].
- **Latency:** touch to note 3-5 ms; drive mode adds ~2.2 ms to the note and up to ~4 ms to release [V notes].
- **MIDI emitted:** note on/off on channel 1 with velocity; polyphonic aftertouch `0xA0 n p` (at_mode 1) or channel pressure `0xD0` of the highest key (at_mode 2) [V ui.c lines 318-358]. Release velocity measured but not sent [V README]. No pitch bend, no CC74, no per-note channels.
- **Status:** author "working; parked 2026-10-14" with per-key calibration "not yet right"; owner's own verdict: one chord's fingers read 59/76/127/104, "which no player can control" [V README, notes]. Yet MONK-1 and Scintilla later shipped using it, so it is usable but noisy.
- **Cost (Velo vs this repo):** per-key state ~60 bytes x 27 (sense_key_t) plus a 48-sample capture; ISR work each 100 us tick for one ADC start/read. No measured CPU figure was published [I]. Our `fm1_input.h` has no analog hook yet [V: `grep` of firmware/hal/fm1_input.h shows only digital scan and `fm1_adc.h` is a blocking polled reader used by master.c/main.c]. Porting means adding Velo's `FM1_INPUT_ANALOG/DRIVE` hooks and reconciling with the MASTER pot slot (Velo hands the OCT- odd-frame slot to the pot) [V Velo fm1_input.h]; the BLE radio also uses the ADC [I notes item 6].

## 3. What the FM-1 keybed can and cannot sense

| Capability | Status | Evidence |
| --- | --- | --- |
| On/off per key, all 27, plus polyphony | Can | [V] this repo `firmware/hal/fm1_input.h` `fm1_in.notes` (27 bits), KEYMAP; debounced |
| Press and release timing (first-closed to debounced press, hold time) | Can | [V] `fm1_in.note_t0[27]` and `press_*` stats in fm1_input.h; hold time is simply frame counting |
| Velocity, F3-C5 (20 keys) | Can, rough | [V] Velo sense.c; ~1..127 spread but not repeatable across a chord |
| Velocity, top 7 keys | Weak | [V] Velo notes: probe saturates; otherwise fixed |
| Polyphonic aftertouch, F3-C5 | Can, small range | [V] Velo ui.c sends 0xA0; [V] notes verdict on range |
| Aftertouch, top 7 keys | Unconfirmed | Velo `sense_is_analog(i)` skips them [V ui.c line 331] |
| Release velocity | Can measure, not sent | [V] README |
| Per-key position (X/Y slide on key) | Unlikely | [I] Velo notes: "pressing one end of a key reads softer": a crude lever effect, not a position sensor |
| True MPE (per-note bend, CC74) | Not shown by anyone | unconfirmed |
| Capacitive touch/hover | Cannot | [V] notes: capacitance probe tried and closed |
| Key LEDs as light sensors | Cannot | [V] notes (tried and closed) |
| Whether our pre-existing 4 KB headroom rule is met by porting | Unknown | RAM ~2 KB for 27 x sense_key_t estimate [I]; measure at build |

This repo's current scan: 10 kHz tick, one column per tick, 11 columns, i.e. a full matrix frame about 1.1 ms; keys are boolean and a press has first-closed and debounced timestamps [V fm1_input.h lines 12-22, 134, 144, 166-195]. The firmware has no velocity input; `kb_vel` is a fixed menu value (`seq.c` line 41 [V]).

## 4. Candidate gestures for a sequencer step/key

Hold-time and tap gestures need only the digital keys (certain); anything force-based depends on porting Velo.

| Gesture | Input needed | Feasibility | Cost |
| --- | --- | --- | --- |
| Hold-to-mute / hold a step key to toggle mute | hold time on a white key | certain (digital) | negligible; use `note_t0`-style timestamp or frame count |
| Double-tap = ratchet (tap count 1..4 within a window sets ratchet) | key press edges + time | certain | negligible; no new hardware |
| Hold + knob = slide/ratchet/probability for that step | key held + K1-K4 | certain | UI only; this is the P1-014 pattern |
| Press velocity -> step accent | Velo velocity on F3-C5 | likely on 20 keys, unlikely repeatable; top 7 keys weak | ISR work per tick + ~2 KB RAM [I]; port effort; chord evenness poor |
| Pressure while held -> ratchet rate / slide amount / probability | Velo poly aftertouch | likely but small range; same coverage limit | as above, plus 5 ms main-loop update |
| Soft vs hard press -> mute vs accent | velocity threshold, two bins | likely (two bins are more forgiving than 127 levels) | as velocity |
| Per-key pitch bend / slide on the key (true MPE) | per-key X/Y | unlikely (no sensor found) | not recommended |
| Key-down duration as note length / gate | hold time | certain | negligible |

## 5. Recommendation

1. **Design the gestures on digital input first** (certain): tap count for ratchet, hold for mute, step key held + knobs for slide / probability / accent per P1-014. These work on all 27 keys with zero new hardware use and no RAM risk.
2. **Treat force sensing as an optional later layer** (a separate backlog task, not part of v1 core): velocity as a coarse two-bin accent/mute choice on F3-C5 only, and aftertouch for one continuous parameter. Do not require it; the top 7 keys lack it and consistency across a chord is poor. Make the force mapping a data table (ADR-0004 addendum).
3. **Fallback if the hardware proves only velocity/hold-time:** the digital set in item 1 covers ratchet, slide, mute, accent and probability by gestures. Nothing in R3 requires force.
4. Reuse Velo's code only by porting under GPL-3.0 (compatible); a hardware-test gate is needed because sensing depends on ADC timing.

## 6. Owner questions

1. Is Velo (fm1-velocity, Charles Vestal) the project you meant by "MPE-like keybed expression"? It is the only one I found that senses force from the keybed; Felucca MPE is input-side only.
2. Is a force layer worth a port (two-bin accent via velocity, optional pressure), given the author's own "parked" verdict, an uneven chord response and the top 7 keys not covered? Or hold-time/tap gestures only for v1? Recommendation: tap/hold only for v1, force as a later optional task.
3. Do you have a Velo or Scintilla firmware flashed anywhere? If so, you could judge the feel before we commit (I did not flash or run anything).
