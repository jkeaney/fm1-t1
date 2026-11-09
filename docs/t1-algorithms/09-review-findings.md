# 9. Review findings

> **Maintained list:** the up-to-date, per-issue record (status, locations, reproduction, fixes) is [../T1_ISSUES.md](../T1_ISSUES.md). This chapter is the original review narrative.
> **Status update (working tree, 2026-11-09):** F1 (scale masks), F2 (scale order, except a stale unused name array) and F4 (`range` unit) have since been fixed in the working tree and re-verified; F19 was added. The text of F1, F2 and F4 below describes the code **as first reviewed**.

Nineteen findings from reading the T-1 sources and running them. Every item marked **[V]** was reproduced with the real code (probes in [verification/](verification/)); nothing was changed in the firmware.
Severity is my judgement of the effect on a user of the FM-1 (High = wrong notes or a documented core feature not working; Medium = a control misbehaves or does nothing; Low = latent or cosmetic).

## Summary

| ID | Severity | Area | Finding |
| --- | --- | --- | --- |
| F1 | **High** | Harmony | 5 of 16 scale masks contain the wrong notes (Major, Mixolydian, Phrygian, Lydian, Melodic Minor) |
| F2 | **High** | Harmony / UI | On-screen scale names do not match the engine for ids 4 to 15; the default bassline plays Locrian, not minor pentatonic |
| F3 | Medium | Harmony | Phrase contours ARCH and INV A are constant (integer division) |
| F4 | **High** | Harmony | `range` is multiplied by 7 semitones; the default bass spans MIDI 22 to 115 |
| F5 | Medium | Harmony | Arpeggio style index added on every pulse, twice for styles above 0; RAND is a fixed cycle; ORDER = UP |
| F6 | **High** | UI | In GLOBAL mode, unmapped knobs change STEPS on all six tracks |
| F7 | **High** | UI / engine | Selecting a cycle with the keys does not refresh the cached rhythm |
| F8 | Medium | UI | RND TGT and STEP MOD knobs can only reach one bit with normal turning |
| F9 | Medium | Dispatch | Event queue overflow drops events silently; MIDI note-offs can be lost |
| F10 | Low | Engine | Muting freezes a track's phase |
| F11 | Medium | Engine / UI | Several documented controls are inert or unreachable |
| F12 | **High** | Product | Patterns are not saved; parameter locks cannot be created or cleared on the device |
| F13 | Medium | Docs | The guide and plan describe things the code does not do |
| F14 | Low | Code | Duplicate dispatcher and voice allocator, dead on the device, with different policies |
| F15 | Low | Storage | Bank erase vs 64 KB sector is unmodelled by the tests (unused on device) |
| F16 | Low | Random | LFSR modulation is asymmetric; large depths move pitch by tens of semitones |
| F17 | Low | P-locks | A zero-filled lock arena refuses all new locks |
| F18 | Low | Tests | The Phase 3 timing benchmark is wall-clock and flaky |
| F19 | Medium | Random | Consecutive LFSR draws are dependent: pitch and velocity jitter are tied together (added later) |

---

## F1. Five scale masks are wrong **[V]** (now FIXED in the working tree)
- **Where:** `torso_t1_algo.c` `T1_SCALE_MASKS` ids 1, 4, 8, 9, 11.
- **What:** the hex values do not match the pitch lists in their own comments. Major `0x0AD5` contains pitch classes 0 2 4 **6** 7 9 11 (Lydian); Mixolydian has a raised 4th; Phrygian is 0 **2** 3 5 **6** 8 10; Lydian has 8 notes (0 2 4 **5 6** 7 9 11); Melodic Minor has 8 notes (0 **1 2** 4 6 7 9 11). See the full table in [05](05-harmony-and-pitch.md).
- **Evidence:** the probe prints the set bits of each mask; quantising raw 60 to 71 in "Major" returns `60 60 62 62 64 64 66 67 67 69 69 71` (F# allowed, F snapped to E).
- **Why the tests miss it:** `torso_phase2_test.c` asserts `T1_SCALE_MASKS[1] == 0x0AD5`, which checks the constant against itself.
- **Fix:** correct the five constants (Major `0xAB5`, Mixolydian `0x6B5`, Phrygian `0x5AB`, Lydian `0xAD5`, Melodic Minor `0xAAD`) and add a test that compares every mask with an interval list. ⚠ Verify the hex against a table before changing.

## F2. Scale names on screen do not match the engine **[V]** (now FIXED apart from a stale `T1_SCALE_NAMES` array)
- **Where:** `torso_t1_ui.c` `SCALE_NAMES` (and the guide) versus `T1_SCALE_MASKS` / `T1_SCALE_NAMES`.
- **What:** the UI order is CHROM, MAJOR, MINOR, DORIAN, PHRYG, LYDIAN, MIXOL, AEOL, LOCR, P-MAJ, P-MIN, BLUES, WHOLE, H-MIN, M-MIN, DIM. The engine order is Chromatic, Major, Minor, Dorian, Mixolydian, Maj Pent, Min Pent, Harm Minor, Phrygian, Lydian, Locrian, Mel Minor, Blues, Whole Tone, Dim H-W, Dim W-H.
  Ids 0 to 3 agree; ids 4 to 15 show one name and play another (for example id 5 shows LYDIAN and plays major pentatonic; id 10 shows P-MIN and plays Locrian).
- **Consequence for the default pattern:** `t1_default_pattern` sets `scale_id = 10` with the comment "A minor pentatonic". The engine plays **A Locrian** (verified: raw 57 to 69 with root A returns `57 58 58 60 60 62 63 63 65 65 67 67 69`, which includes B♭ and E♭).
- **Fix:** make one table the source of truth (names and masks together).

## F3. ARCH and INV A phrase contours are broken **[V]**
- **Where:** `t1_harmony_eval_phrase` cases 5 and 6.
- **What:** `quad = (norm*norm) / (total*total + 1)` is integer division and is 0 for every step, so ARCH is constantly `+max` and INV A is constantly 0. Measured for 16 steps, range 1: all 7s and all 0s.
- **Fix:** scale before dividing (for example `quad = norm*norm*max / (total*total)`).

## F4. `range` scales by 7 semitones **[V]** (now FIXED: `max_semitones = range`)
- **Where:** `max_semitones = range * 7`.
- **What:** the UI calls RANGE "0 to 127 semitones" and the header comment says octaves 0 to 7; the code uses 7 semitones per unit. The device default bass has `range = 12` with a TRI contour, so the contour alone reaches **+84 semitones**.
- **Evidence:** an 8-bar run of the default pattern produced notes from **22 to 115** (25 distinct pitches), against a stated "A minor pentatonic around A1" (about MIDI 33).
- **Fix:** decide the intended unit (semitones, scale degrees or octaves) and use it in the code, the UI and the guide.

## F5. Arpeggio style is applied unexpectedly **[V]**
- **Where:** `torso_t1_pipeline.c` lines ~237 and ~311.
- **What:** (a) `note += style_index(style, pulse_counter, 7)` runs for **every** style, including 0 (UP), so a freshly created track climbs 0 to 6 semitones per pulse (`60 61 62 63 64 65 66 60 …` measured). (b) For styles above 0 the index is added **again** per repeat (`pulse_counter × repeats + k`), so the offset is doubled (DOWN gave `72 70 68 66 64 62 60`). (c) The pool size is fixed at 7 and the index is a semitone offset, not a position in a note list or chord. (d) RAND is a fixed permutation cycle, not random. (e) ORDER behaves as UP.
- **Fix:** apply the style once, to a defined note pool (the chord or scale degrees), and give ORDER a meaning or remove it.

## F6. GLOBAL mode edits the wrong parameter **[V]**
- **Where:** `t1_ui_on_encoder_turn`, GLOBAL branch: `pid` starts as `T1_PARAM_STEPS`.
- **What:** MOD/CC page knobs 1, 3 (without shift) and 4, and RHYTHM page knob 4, have no mapping, so they change **STEPS on all six tracks**. Measured: GLOBAL on page 4, knob 1 `+3` took `steps 16 → 19` on a track.
- **Fix:** initialise `pid` to a "none" value and return early when unmapped.

## F7. Cycle select by key does not refresh the rhythm **[V]**
- **Where:** `t1_ui_select_cycle` (called from the white keys 7 to 12 in `ui_t1.c`).
- **What:** it sets `active_cycle` but does not call `t1_pipeline_invalidate_masks()`. The cached Euclidean mask stays that of the previous cycle until another edit invalidates it. Measured: cycle 0 (4 pulses) then `select_cycle(1)` (12 pulses) still played 4 pulses per bar until the masks were invalidated, then 12.
  The knob path (RHYTHM knob 4) does invalidate, so only the key path is affected.
- **Fix:** call invalidate in `t1_ui_select_cycle` (and in any place `active_cycle` is written).

## F8. Single-detent turns reach only one bit **[V]**
- **Where:** RND TGT: `random_targets ^= 1 << (|steps| % 8)`; STEP MOD: `active_mask_lo ^= 1 << (|steps| % 32)`.
- **What:** the bit toggled is chosen by the *size* of the turn, not the knob position. One detent (±1) always toggles bit 1. Measured: single detents toggle `0x02` on and off; 2 detents in one frame toggle bit 2, up to 8 detents for bit 0. So normally only PITCH can be selected as a random target, and only forced step 2 can be toggled by a slow turn.
- **Fix:** keep a "cursor" bit that the knob moves through, with a press to toggle.

## F9. The queue drops events silently **[V]**
- **Where:** `t1_micro_queue_push` returns false when full; the pipeline ignores it.
- **What:** under heavy load (6 tracks, 16 repeats, 4-voice chords) 16,416 events were dropped in two bars and **32 MIDI note-ons lost their note-off** (the FM voice allocator was unaffected). Those notes sound until the next stop (stop sends All Notes Off, CC 123).
- **Fix:** drop whole note on/off pairs together, or limit repeats × voices per pulse, or enlarge the queue; surface `dropped_events`.

## F10. Muting freezes phase **[V]**
- **What:** a muted track is skipped before its counters advance. After 30 ticks of mute, the un-muted track resumed 30 ticks behind a track that kept running, and does not catch up. The groove between tracks changes every time a track is muted. Measured: step 2 / accum 48 versus step 3 / accum 30.
- **Fix:** advance the counters for muted tracks and only suppress events.

## F11. Inert or unreachable controls **[V]**
| Control | State |
| --- | --- |
| GROOVE (`groove_id`) | stored and shown; never read |
| MAX LEN (`master_length_override`) | stored and shown; never read (a 16-pulse bar still gave 16 note-ons with the override at 4) |
| Pattern `swing` | live in the engine, **no control sets it** (default 50 = straight) |
| `master_scale` | live in the engine, **no control sets it**; M-TRANS edits `master_root`, which then has no effect |
| `soloed`, `cycle_count`, `reserved_flags` | unused |
| RND TGT bits SUSTAIN, DELAY, REPEATS, VOICING | defined, never read |
| `delay_ticks` below 0 | no effect on the first hit of a pulse (clamped to the pulse tick) |
| `random_rate` | read only at PLAY |
| Step mod above step 32, `active_mask_hi` | engine supports it; no control edits it |

## F12. Patterns are not saved; p-locks unreachable **[V]**
- **What:** (a) the T-1 pattern is not stored anywhere on the device: not in projects, presets or flash; a reboot returns the default pattern. (b) The p-lock arena works in the engine, but `t1_ui_set_edit_step` is never called by the device UI, so a step cannot be selected for locking (the guide says "hold a step key 1 to 16"; the keys select tracks and cycles). (c) No UI calls `t1_plock_clear`. (d) Only 3 of the parameters (pitch, velocity, probability) can be locked at all.
- **Fix:** wire the step keys to `set_edit_step`, add a clear gesture, and persist the pattern (the flash layout exists but is unused).

## F13. Documentation versus code **[V]**
| Document says | Code does |
| --- | --- |
| Hardware 192 PPQN timer ISR, FreeRTOS tasks and priorities | clocked from the audio block; no FreeRTOS on the device (the task table is a descriptive constant) |
| TRS-MIDI UART output | USB-MIDI only; the UART ring is drained and discarded |
| Pace uses a Q15 table, logarithmic and exponential curves | a quadratic bell warp in Q15 arithmetic ([04](04-ratchet-and-pace.md)) |
| Step Mod mutes or forces steps | forced pulses only (OR) |
| Random Rate "/1 to /16 cycles" | loop length 1, 2, 3, 4, 6 (then wraps); the screen shows `/rate`, which is wrong for rates above 2 |
| Scales listed in the guide order | engine order differs (F2) |
| Keys 1 to 16 hold a step for p-lock | keys select tracks and cycles |
| Track 0 internal, tracks 1 to 5 MIDI, channels 1 to 16 | channels are 0-based indexes; default tracks 2 to 6 use MIDI channels 2 to 6 |
| `[TEMP]`: "keep snapshot intact" | entering TEMP overwrites the snapshot with the current pattern (correct, but not as written) |
| "ISR < 80 µs", "revert 0.081 µs" | host-measured or targets; not measured on the FM-1 |
| Master Harmony Link projects relative to Track 1 | a global scale/root override, unreachable from the UI |

## F14. Duplicate dead code **[V]**
`torso_t1.c` contains a second dispatcher (`t1_micro_queue_dispatch`, which also compares sub-ticks and would delay sub-tick events by one tick), a second voice allocator (`t1_fm_voice_*`, steals the *oldest* voice, whereas the pipeline allocator steals the *lowest velocity*),
the timer ISR and its statistics, and the task table. None of it is called on the device, but all of it is compiled into the firmware image. Two policies for one job also invite divergence in the tests.

## F15. Flash bank erase versus sector size ⚠
- `T1_FLASH_SECTOR_SIZE` is 64 KB with banks every 32 KB, so two banks share each erase sector. `t1_flash_save_bank` erases once at the bank offset. The test driver erases only 32 KB, so the sharing case is never exercised.
  If a real driver erases 64 KB, saving one bank would erase its neighbour. Unused on the device today, so latent.

## F16. LFSR modulation range **[V]**
`rnd mod (depth+1) − depth/2` is not symmetric for odd depths (−60 to +64 measured at 127), and at high depths a pitch can move by ±63 semitones, which the scale snap then folds anywhere. The default bass uses depth 24 (±12).

## F17. Zero-filled arena **[V]**
`t1_system_init` sets every lock slot to `0xFF`. Any other initialisation (for example a zeroed pattern loaded from storage) makes all 11 slots look occupied by (step 0, param 0): `set` is refused (measured) and `get(0, 0)` reports a lock. Latent, because patterns are never loaded.

## F18. The Phase 3 benchmark is flaky **[V]**
`test_pipeline_isr_benchmark` measures wall-clock microseconds on a general-purpose OS and fails intermittently (13, 30, 37, 87, 124, 163 µs across six runs against an 80 µs limit). It cannot show target behaviour. A cycle-count measurement on the FM-1, or an operation count, would be a valid replacement.

---

## What is solid **[V]**
- Euclidean generation and rotation (12 cases match an independent reference up to rotation).
- Division and swing arithmetic, including the preserved pair length.
- The pace warp is monotonic and symmetric in its two directions; velocity slope is exact.
- The LFSR has full period and the pass-boundary reseed gives exact loops (verified for rates 0 and 3).
- [TEMP] revert/commit behave as designed; memory layout is statically asserted.
- The firmware design keeps all state in static memory with no heap, and the event ordering rule (offs before ons at the same tick) is correct.

## F19. Consecutive LFSR draws are dependent **[V]** (added after the first pass)
- **Where:** `torso_t1_pipeline.c` draws for probability, pitch, velocity and CC; `t1_lfsr_next` advances one bit per call. A new `t1_lfsr_byte` helper fixes the accent draw only.
- **What:** each 16-bit output is the previous one shifted by a bit. Measured: `rnd2 mod 128 == (rnd1 mod 256) >> 1` in 20,000 of 20,000 consecutive pairs; pitch and velocity modulation at depth 127 correlate at 0.500.
- **Fix:** take a fresh byte/word per decision (as `t1_lfsr_byte` does) for every random target.
- Full detail: [../T1_ISSUES.md#f19](../T1_ISSUES.md#f19).
