# T-1 sequencer: issues found in review

A single record of every defect, inconsistency and gap found while reviewing the Torso T-1 sequencer (`firmware/src/torso_t1*.c`, `t1_glue.c`, `ui_t1.c`).
Background and the algorithm write-ups are in [t1-algorithms/](t1-algorithms/README.md); the reproduction programs and their captured output are in [t1-algorithms/verification/](t1-algorithms/verification/).

## Snapshot this list is based on

| Item | Value |
| --- | --- |
| Reviewed | 2026-11-09 |
| Base commit | `bea0fd3` (2026-10-14), **plus uncommitted changes** to 13 firmware files and 2 test files, and new untracked `t1_glue.c`, `torso_t1_port.c`, `ui_t1.c` |
| Method | Read every T-1 source; built and ran the five `torso_phase*_test` suites; compiled the real sources on the host and probed them (`verification/probe*.c`) |
| Re-verified | Every issue below was re-checked against the working tree **after** your scale-mask and `range` fixes landed during the review; statuses reflect the tree as of the last run |
| Firmware changed by me | **No.** Only files under `docs/` were written |

**Status key:** `OPEN` reproduces in the current tree · `FIXED` was found, then fixed in the working tree (uncommitted) and re-verified · `PARTIAL` mostly fixed with a residue.
**Severity:** High = wrong notes or a documented core feature does not work · Medium = a control misbehaves or does nothing · Low = latent, cosmetic or hygiene.

## Summary

| ID | Status | Severity | Area | One-line description |
| --- | --- | --- | --- | --- |
| [F1](#f1) | FIXED | High | Harmony | Five scale masks held the wrong notes |
| [F2](#f2) | PARTIAL | High | Harmony / UI | UI scale names and engine order disagreed; a stale name array remains |
| [F3](#f3) | OPEN | Medium | Harmony | ARCH and INV A contours are constant |
| [F4](#f4) | FIXED | High | Harmony | `range` was multiplied by 7 semitones |
| [F5](#f5) | OPEN | Medium | Harmony | Arpeggio style added on every pulse, twice above style 0; RAND/ORDER wrong |
| [F6](#f6) | OPEN | High | UI | GLOBAL mode: unmapped knobs change STEPS on all tracks |
| [F7](#f7) | OPEN | High | UI / engine | Selecting a cycle by key does not refresh the cached rhythm |
| [F8](#f8) | OPEN | Medium | UI | RND TGT and STEP MOD knobs reach only one bit with normal turning |
| [F9](#f9) | OPEN | Medium | Dispatch | Event-queue overflow is silent and can lose note-offs |
| [F10](#f10) | OPEN | Low | Engine | Muting freezes a track's phase |
| [F11](#f11) | OPEN | Medium | Engine / UI | Several controls are inert or unreachable |
| [F12](#f12) | OPEN | High | Product | Patterns are not saved; p-locks cannot be created or cleared on the device |
| [F13](#f13) | PARTIAL | Medium | Docs | Guide and plan describe things the code does not do |
| [F14](#f14) | OPEN | Low | Code | Duplicate dead dispatcher and voice allocator, with different policies |
| [F15](#f15) | OPEN | Low | Storage | Bank erase vs 64 KB sector is untested (unused on device) |
| [F16](#f16) | OPEN | Low | Random | Modulation range is asymmetric; large depths move pitch by tens of semitones |
| [F17](#f17) | OPEN | Low | P-locks | A zero-filled lock arena refuses every new lock |
| [F18](#f18) | FIXED | Low | Tests | The Phase 3 timing test is wall-clock and flaky |
| [F19](#f19) | OPEN | Medium | Random | Consecutive LFSR draws are dependent: pitch and velocity jitter are tied together |

Counts: 19 issues: 3 fixed (F1, F4, F18), 2 partial (F2, F13), 14 open. The 14 open items are 3 High (F6, F7, F12), 6 Medium (F3, F5, F8, F9, F11, F19) and 5 Low (F10, F14 to F17).

---

<a id="f1"></a>
## F1. Five scale masks held the wrong notes: FIXED

- **Severity / area:** High, harmony.
- **Where (original):** `torso_t1_algo.c` `T1_SCALE_MASKS`, ids 1, 4, 8, 9, 11.
- **What was wrong:** the hex values did not match the pitch lists in their own comments.

| Scale | Original mask | Pitch classes it actually held | Correct |
| --- | --- | --- | --- |
| Major | `0x0AD5` | 0 2 4 **6** 7 9 11 (this is Lydian) | `0x0AB5` = 0 2 4 5 7 9 11 |
| Mixolydian | `0x06D5` | 0 2 4 **6** 7 9 10 | `0x06B5` = 0 2 4 5 7 9 10 |
| Phrygian | `0x056D` | 0 **2** 3 5 **6** 8 10 | `0x05AB` = 0 1 3 5 7 8 10 |
| Lydian | `0x0AF5` | 0 2 4 **5 6** 7 9 11 (8 notes) | `0x0AD5` = 0 2 4 6 7 9 11 |
| Melodic Minor | `0x0AD7` | 0 **1 2** 4 6 7 9 11 (8 notes) | `0x0AAD` = 0 2 3 5 7 9 11 |

- **Reproduction (original):** quantising raw 60 to 71 in "Major" returned `60 60 62 62 64 64 66 67 67 69 69 71`: F♯ allowed, F pulled down to E. (`probe.before-fixes.out.txt`, section 3.)
- **Why the tests missed it:** `torso_phase2_test.c` asserted `T1_SCALE_MASKS[1] == 0x0AD5`, which checks the constant against itself, not the musical scale.
- **Current state:** all 16 masks now equal their interval lists, and a new Phase 2 test checks every one (`tests/torso_phase2_test.c`; suite now 100 assertions, passing). Major now quantises raw 60 to 71 to `60 60 62 62 64 65 65 67 67 69 69 71`.

---

<a id="f2"></a>
## F2. Scale names and engine order disagreed: PARTIAL

- **Severity / area:** High, harmony and UI.
- **Original problem:** the on-screen names (`torso_t1_ui.c` line 22) and the guide listed CHROM, MAJOR, MINOR, DORIAN, PHRYG, LYDIAN, MIXOL, AEOL, LOCR, P-MAJ, P-MIN, BLUES, WHOLE, H-MIN, M-MIN, DIM, but the masks and names in `torso_t1_algo.c` were in a different order for ids 4 to 15.
  Ids 4 to 15 displayed one name and played another.
- **Visible consequence:** the device default bassline (`t1_glue.c` line 52, `scale_id = 10`, comment "minor pentatonic") actually played **A Locrian**. Raw 57 to 69 with root A returned `57 58 58 60 60 62 63 63 65 65 67 67 69`, containing B♭ and E♭.
- **Current state:** the masks were re-ordered to the UI order and each mask now matches its name. The default bass is now genuinely A minor pentatonic (raw 57 to 69 → `57 57 57 60 60 62 62 64 64 64 67 67 69`), and a test asserts `quantize(70, A, 10) == 69`.
- **Residue (OPEN, Low):** `T1_SCALE_NAMES` in `torso_t1_algo.c` (line 242) still lists the **old** order (Chromatic, Major, Minor, Dorian, Mixolydian, Maj Pent, …), so it now mislabels the masks. Nothing references it (declared in `torso_t1_algo.h` line 75, used nowhere), but it will mislead anyone who uses it.
  The old table also had both a "Dim H-W" and a "Dim W-H"; the new table keeps only Half-Whole, and Aeolian (id 7) duplicates Natural Minor (id 2).
- **Fix:** delete `T1_SCALE_NAMES`, or reorder it to match, and expose a single names-plus-masks table to both the engine and the UI.

---

<a id="f3"></a>
## F3. ARCH and INV A phrase contours are broken: OPEN

- **Severity / area:** Medium, harmony.
- **Where:** `torso_t1_algo.c` `t1_harmony_eval_phrase`, case 5 (line ~363) and case 6 (line ~370).
- **What:** `quad = (norm * norm) / (total_steps * total_steps + 1)` uses integer division. For every step `norm² < total² + 1`, so `quad` is always 0. ARCH is therefore a constant `+max` and INV A is a constant 0. Neither has any shape.
- **Reproduction:** `probe.c` section 4, 16 steps. With `range = 1` (now 1 semitone): ARCH = `1 1 1 … 1`, INV A = `0 0 0 … 0`. (Originally 7s and 0s.)
- **Impact:** two of the eight contours are indistinguishable from FLAT (INV A) or a fixed offset (ARCH).
- **Fix:** multiply before dividing, for example `quad = norm*norm*max / (total*total)`, and use a proper parabola for ARCH: `max − quad`.
- **Suggested test:** each contour must have more than one distinct value for `range ≥ 2` and `steps ≥ 4`; ARCH must peak near the middle and INV A must dip near the middle.
- **Related (Low):** RAND contour (case 7) repeats each value twice and walks a regular sequence (`-1 -1 0 0 1 1 -1 -1 …`) because of an LCG modulus artefact.

---

<a id="f4"></a>
## F4. `range` was multiplied by 7 semitones: FIXED

- **Severity / area:** High, harmony.
- **Original problem:** `max_semitones = range * 7` (`t1_harmony_eval_phrase`). The UI calls RANGE "0 to 127 semitones" and the header comment said octaves 0 to 7.
- **Visible consequence:** the device default bass (`range = 12`, TRI contour) had a contour of up to **+84 semitones**. An 8-bar run produced notes from **22 to 115** (25 distinct pitches), against a stated "A minor pentatonic around A1".
- **Current state:** `max_semitones = range` (`torso_t1_algo.c` line 331). The TRI contour at `range = 12` now peaks at 12 semitones, and the same 8-bar run now gives notes **21 to 57** (15 distinct pitches; 50 note-ons, with accented hits at velocity 120 after the accent change).
- **Note:** the header comment in `torso_t1_algo.h` (`range … in octaves (0 to 7)`) still describes the old unit.

---

<a id="f5"></a>
## F5. Arpeggio style is applied unexpectedly: OPEN

- **Severity / area:** Medium, harmony.
- **Where:** `torso_t1_pipeline.c` line 237 (`note += t1_harmony_eval_style(cp->style, rt->pulse_counter, 7u)`) and line 311 (`rep_base += … style(pulse_counter * repeats + k)`); `torso_t1_algo.c` `t1_harmony_eval_style`.
- **What:**
  1. The style index is added to the pitch in semitones on **every** pulse, **including style 0 (UP)**. A new track climbs 0 to 6 semitones per pulse.
  2. For styles above 0 the index is added **again** per repeat, so the offset is doubled.
  3. The pool size is hard-coded to 7; the index is a semitone offset, not a position in a chord or note list.
  4. Style 6 (RAND) is a fixed permutation cycle, not random (pool 7: `3 5 0 2 4 6 1 …`).
  5. Style 7 (ORDER) behaves as UP; there is no "order of entry" memory.
- **Reproduction:** `probe.c` sections 5, 5b, 9, 9b, 9c. Default-initialised track, 4 pulses, chromatic: `60 61 62 63 64 65 66 60 61 62 63 64`. Style DOWN: `72 70 68 66 64 62 60 72 70 …`.
- **Impact:** a track with the default style cannot repeat a pitch; styles other than UP produce doubled, hard-to-predict offsets.
- **Fix:** apply the style once, to a defined note pool (the chord or the scale degrees), make 0 mean "no offset" if that is intended, and give RAND and ORDER a real meaning or remove them.
- **Suggested test:** style 0 with one pulse per bar gives the same pitch each bar (or document the sweep); style > 0 applies the offset exactly once.

---

<a id="f6"></a>
## F6. GLOBAL mode edits the wrong parameter: OPEN

- **Severity / area:** High, UI.
- **Where:** `torso_t1_ui.c` `t1_ui_on_encoder_turn`, GLOBAL branch, line 143: `T1ParameterID pid = T1_PARAM_STEPS;`, with unmapped knobs falling through.
- **What:** knobs with no mapping keep `pid = STEPS` and change the **step count of all six tracks**. Affected: MOD/CC page knobs 1, 3 (no shift) and 4, and RHYTHM page knob 4.
- **Reproduction:** `probe.c` section 11. GLOBAL on page 4, knob 1 `+3`: `steps 16 → 19` on a track. RHYTHM knob 4 in GLOBAL: `steps 16 → 20`.
- **Impact:** a performer reaching for a MOD/CC knob while holding GLO silently changes every track's loop length, which also changes pattern phase relationships.
- **Fix:** initialise `pid` to a "none" value and return before `t1_global_apply_delta` when unmapped.
- **Suggested test:** for every page × shift × knob in GLOBAL mode, at most the intended parameter changes.

---

<a id="f7"></a>
## F7. Selecting a cycle with the keys does not refresh the rhythm: OPEN

- **Severity / area:** High, UI and engine.
- **Where:** `torso_t1_ui.c` `t1_ui_select_cycle` (line 79) sets `active_cycle` but does not call `t1_pipeline_invalidate_masks()`. It is called from the white keys 7 to 12 in `ui_t1.c` (line 98).
- **What:** each track caches its Euclidean mask (`mask_valid`). After a key-select the old cycle's mask is kept until another edit invalidates it, so the **new cycle's other parameters play over the old cycle's rhythm**.
- **Reproduction:** `probe2.c` section C. Cycle 0 (4 pulses) plays 4 per bar; after `select_cycle(1)` (12 pulses) it still plays 4; after an explicit mask invalidate it plays 12. The knob path (RHYTHM knob 4) does invalidate, so only the key path is affected.
- **Impact:** switching cycles is the T-1's central performance gesture, and it does not change the rhythm until you touch another control.
- **Fix:** call `t1_pipeline_invalidate_masks()` inside `t1_ui_select_cycle`, or invalidate wherever `active_cycle` is written.
- **Suggested test:** after `t1_ui_select_cycle(n)`, the next bar contains cycle *n*'s pulse count.

---

<a id="f8"></a>
## F8. Single-detent turns reach only one bit: OPEN

- **Severity / area:** Medium, UI.
- **Where:** `torso_t1_ui.c` line 407 (`random_targets ^= 1 << (|steps| % 8)`) and the STEP MOD line (about 276, `active_mask_lo ^= 1 << (|steps| % 32)`).
- **What:** the bit toggled is chosen by the **size** of the turn, not by the knob position. A single detent (±1) always toggles bit 1. Faster turns reach other bits (8 detents in one frame wraps to bit 0).
- **Reproduction:** `probe2.c` sections D and E. Six single detents toggle `0x02` on and off; 2, 3, 4 … 8 detents in one frame toggle bits 2, 3, 4 … 0.
- **Impact:** with normal slow turning only the PITCH random target can be selected; the others (velocity, accent, CC) need fast spins. STEP MOD can only toggle forced step 2.
- **Fix:** keep a cursor bit that the knob moves through, and toggle on a press or a second gesture.

---

<a id="f9"></a>
## F9. The event queue drops events silently: OPEN

- **Severity / area:** Medium, dispatch.
- **Where:** `torso_t1.c` `t1_micro_queue_push` (line ~257: `dropped_events++`, returns false); callers in `torso_t1_pipeline.c` (lines 337, 351, 373) ignore the result.
- **What:** the queue holds 256 events. One pulse at 16 repeats × 4 voices queues 128 events (64 note-ons and 64 note-offs). When the queue is full, events are dropped one at a time, so a note-on can be accepted while its note-off is dropped.
- **Reproduction:** `probe2.c` section A. Six tracks, 16 pulses, 16 repeats, 4-voice chords, two bars: **16,416 events dropped**; after draining, **32 of 2,048 MIDI note-ons never received a note-off**. The internal FM path stayed balanced. Moderate load (4 repeats, 2 voices, six tracks) dropped nothing.
- **Impact:** hung notes on an external synth under extreme settings, until the next stop (stop sends All Notes Off, CC 123). The dropped count is not shown anywhere.
- **Fix:** queue a note-on and its note-off together or not at all; cap `repeats × voices` per pulse; surface `dropped_events`.

---

<a id="f10"></a>
## F10. Muting freezes a track's phase: OPEN

- **Severity / area:** Low, engine.
- **Where:** `torso_t1_pipeline.c` line 169 (`if (trk->muted) continue;`) runs before the step counters advance.
- **What:** a muted track stops counting, so when un-muted it resumes where it stopped, behind its siblings.
- **Reproduction:** `probe2.c` section F. After 30 ticks muted: track 0 at step 2 / accum 48 versus track 1 at step 3 / accum 30. They do not realign.
- **Impact:** the groove between tracks changes every time a track is muted and un-muted.
- **Fix:** advance the counters of muted tracks and only suppress their events.

---

<a id="f11"></a>
## F11. Controls that do nothing or cannot be reached: OPEN

- **Severity / area:** Medium, engine and UI.

| Control / field | Location | State |
| --- | --- | --- |
| GROOVE (`groove_id`) | UI `torso_t1_ui.c` ~335 | stored and shown; **never read** by the engine |
| MAX LEN (`master_length_override`) | UI ~229 | stored and shown; **never read** (a 16-pulse bar still gave 16 note-ons with the override at 4; `probe.c` section 13) |
| Pattern `swing` | engine `torso_t1_pipeline.c` `t1_calc_swung_len`; default set at `torso_t1.c` 528 | live in the engine, **no control on the FM-1 sets it** (stays 50 = straight) |
| `master_scale` | engine `torso_t1_pipeline.c` 246 | live in the engine, **no control sets it**; M-TRANS edits `master_root`, which therefore has no effect |
| `soloed`, `cycle_count`, `reserved_flags` | `Track` / `CycleParams` | unused |
| Random targets SUSTAIN (bit 2), DELAY (4), REPEATS (5), VOICING (7) | `torso_t1_algo.h` | defined, never read |
| `delay_ticks` < 0 | pipeline line 298 clamps `on_time` to the current tick | no effect on the first hit of a pulse |
| `random_rate` | `t1_glue.c` `t1_lfsr_reseed` | read only at PLAY from the active cycle; changing it while playing does nothing |
| `active_mask_hi` | engine supports it | no control edits it (steps 33 to 64 cannot be forced) |

- **Impact:** the guide documents most of these as working.
- **Fix:** wire each to the engine, or remove it from the UI and the guide.

---

<a id="f12"></a>
## F12. Patterns are not saved; p-locks unreachable: OPEN

- **Severity / area:** High, product.
- **Where:** `t1_glue.c` header comment ("Nothing here touches the flash"); `torso_t1_ui.c` `t1_ui_set_edit_step` (line 127, never called by `ui_t1.c`); `t1_plock_clear` (no UI caller).
- **What:**
  1. The T-1 pattern is not stored anywhere on the device: not in projects, presets or flash (searched `project.c`, `upreset.c`, `params.c`). Only the on/off setting `SYSTEM > T1 SEQ` (`PREF_T1`) is saved. Every boot restores the default pattern and **edits are lost at power-off**.
  2. The p-lock arena works in the engine, but nothing on the device sets the step being edited, so locks cannot be created. The guide says "hold a step key 1 to 16"; those keys select tracks and cycles.
  3. No UI calls `t1_plock_clear`, so a lock could not be removed.
  4. Only three parameters can be locked (pitch, velocity, probability).
- **Fix:** wire the step keys to `t1_ui_set_edit_step`, add a clear gesture, and persist patterns (the flash layout exists but is unused).

---

<a id="f13"></a>
## F13. Documentation versus code: PARTIAL

- **Severity / area:** Medium, docs. (`TORSO_T1_GUIDE.md`, `PLAN.md`)

| Document says | Code does | Status |
| --- | --- | --- |
| Hardware 192 PPQN timer ISR; FreeRTOS tasks and priorities | clocked from the audio block (about 0.7 ms granularity); no FreeRTOS on the device; the task table is descriptive | open |
| TRS-MIDI UART output | USB-MIDI only; the UART ring is drained and discarded | open |
| Pace uses a Q15 table with logarithmic / exponential curves | a quadratic bell warp in Q15 arithmetic | open |
| Step Mod mutes or forces steps | forced pulses only (OR) | open |
| Random Rate "/1 to /16 cycles" | loop 1, 2, 3, 4, 6 passes, then wraps; the screen shows `/rate`, wrong for rates above 2 | open |
| Keys 1 to 16 hold a step for p-lock | keys select tracks and cycles | open |
| Master Harmony Link projects relative to Track 1 | a global scale/root override, unreachable from the UI | open |
| "[TEMP] keeps the snapshot intact" | entering TEMP overwrites the snapshot with the current pattern | open |
| "ISR < 80 µs", "revert 0.081 µs" | host-measured or design targets; not measured on the FM-1 | open |
| Scale list order | now matches the masks | **fixed** |
| `range` described in octaves / semitones inconsistently | now semitones in code and UI (header comment in `torso_t1_algo.h` still says octaves) | partial |
| Accent "added on accented pulses" with RND TGT ACCENT scaled by depth | now a fixed ~30% chance per pulse, independent of `random_depth` (see the note in F19) | changed in tree |

---

<a id="f14"></a>
## F14. Duplicate dead code with different policies: OPEN

- **Severity / area:** Low, code.
- **Where:** `torso_t1.c`: `t1_micro_queue_dispatch` (line 383), `t1_fm_voice_alloc` (323), `t1_timer_192ppqn_isr` (153), the task table, timing statistics.
- **What:** the device uses the pipeline's own dispatcher and allocator. The copies in `torso_t1.c` are compiled into the firmware but never called, and differ:
  - the pipeline allocator steals the **lowest-velocity** voice (ties by age); the copy steals the **oldest**;
  - the copy's dispatcher compares sub-ticks and, called with sub-tick 0, would delay sub-tick events by a tick.
- **Impact:** code size, and two behaviours for one job that the tests may exercise differently.
- **Fix:** delete the unused copies or make the pipeline call them.

---

<a id="f15"></a>
## F15. Flash bank erase vs sector size: OPEN (latent)

- **Severity / area:** Low, storage. Unused on the device.
- **Where:** `torso_t1_state.c` `t1_flash_save_bank` (line 345, one `erase_sector(bank_offset)` call); `torso_t1_state.h` `T1_FLASH_SECTOR_SIZE` (line 111) is 64 KB while banks sit every 32 KB.
- **What:** two banks share each 64 KB erase sector (banks 0 and 1, 2 and 3, 4 and 5). The test's simulated driver erases only the 32 KB around the offset, so the sharing case is never exercised. A real 64 KB sector erase would wipe the neighbouring bank.
- **Fix:** erase and rewrite both banks of a sector together, or change the layout to one bank per sector.

---

<a id="f16"></a>
## F16. LFSR modulation range: OPEN

- **Severity / area:** Low, random.
- **Where:** `torso_t1_algo.c` `t1_lfsr_modulate`, line 534.
- **What:** `(rnd % (depth + 1)) − depth/2` is slightly asymmetric for odd depths: measured **−60 to +64 at depth 127**, −12 to +12 at depth 24. At high depths a pitch can move by tens of semitones before the scale snap.
- **Fix:** document the range, or compute a symmetric offset; consider capping pitch modulation at an octave.

---

<a id="f17"></a>
## F17. Zero-filled p-lock arena refuses locks: OPEN (latent)

- **Severity / area:** Low, p-locks.
- **Where:** `torso_t1_algo.c` `t1_plock_set` (line 559 looks for `step_idx == 0xFF`).
- **What:** `t1_system_init` fills every slot with `0xFF`. A zero-filled arena (for example from a pattern loaded without initialisation) looks fully occupied by (step 0, param 0): `set` is refused and `get(0, 0)` reports a lock.
- **Reproduction:** `probe2.c` section G: `set(step 3, param 57)` on a zeroed arena returns 0.
- **Impact:** none today, since patterns are never loaded; it matters as soon as persistence (F12) is added.
- **Fix:** treat an all-zero entry (or a version flag) as empty, or initialise on load.

---

<a id="f18"></a>
## F18. The Phase 3 timing test is flaky: FIXED

- **Severity / area:** Low, tests.
- **Where:** `tests/torso_phase3_test.c` line 500, `max_us < 80`.
- **What:** it measures wall-clock microseconds on a general-purpose OS. Across repeated runs the maximum was 13, 30, 37, 87, 124 and 163 µs, and it failed again on the latest run. It cannot say anything about the FM-1.
- **Fix:** replace with a bounded operation count per tick, or a cycle-counter measurement on the target.
- **Status fix:** the two wall-clock assertions (`torso_phase3_test.c`, `torso_phase4_test.c`) now run only when `T1_BENCH=1` is set; otherwise the measured value is printed and the test notes it is not enforced.

---

<a id="f19"></a>
## F19. Consecutive LFSR draws are dependent: OPEN (new)

- **Severity / area:** Medium, random.
- **Where:** `torso_t1_pipeline.c` draws for probability (line 232), pitch (250), velocity (275) and CC (370) use `t1_lfsr_next`/`t1_lfsr_modulate`. `t1_lfsr_next` advances the register **one bit** per call. A newly added `t1_lfsr_byte` helper (line 164) fixes this for the accent draw only, and its own comment states the problem.
- **What:** consecutive 16-bit outputs are the same number shifted by one bit (`s2 = (s1 >> 1) ^ taps`). Anything that takes both a pitch and a velocity (or CC) value from one pulse gets values that are tied together.
- **Reproduction:** `verification/probe3.c`.
  - `rnd2 mod 128 == (rnd1 mod 256) >> 1` in **20,000 of 20,000** consecutive pairs.
  - With pitch and velocity modulation at depth 127, the correlation between the two offsets is **0.500**; velocity jitter is, in effect, half the pitch jitter.
  - For `mod 100` draws (probability then another) the dependence is not visible in a simple conditional test (P = 0.500 vs 0.502).
- **Impact:** the "random" pitch, velocity and CC movements are not independent; they look like one random value copied three times.
- **Fix:** draw a fresh byte or word per decision (as `t1_lfsr_byte` does), or decorrelate with a multiply-and-shift.
- **Side effect to note:** `t1_lfsr_byte` uses 8 LFSR steps per accent draw, which changes the draw count per pulse and therefore every pattern that uses the ACCENT target. That is fine, but it breaks any previously recorded random line.

---

## Observations that are not defects

- **Euclid, rotation, swing, division maths, pace warp, velocity slope:** verified correct (12 Euclid patterns match an independent reference up to rotation).
- **LFSR looping:** full period (65,535) and exact repeat over `loop_period` passes (verified for rates 0 and 3).
- **[TEMP] revert/commit:** behave as designed; memory layout is statically asserted.
- **Unit tests:** four of five suites pass; Phase 3 passes apart from the timing test (F18).

## Appendix: quirks seen in the third-party devices studied (not project issues)

These are noted in the matching `docs/*-algorithms/` folders, listed here for completeness.

- **Snake V3.2.3:** the `Minor` scale table is `0 2 3 5 7 9 11` (melodic minor) with extra repeated entries in the saved data; the pitch lookup divides the octave position by 11, not 12, so the top slice of each octave band can index past the table. See `snake-algorithms/04-pitch-quantise.md`.
- **Sting 2.08:** scale snap uses linear (not circular) distance and ties resolve to the earlier list entry. See `sting-algorithms/03-pitch-generation.md`.
- **Rhythmic Steps:** seven preset display names differ from their filenames (for example "DnB Steppa 165 bpm" stored as "Steppa 175 bpm"). See `rhythmic-steps-algorithms/07-presets.md`.
- **Melodic Steps:** the `Chromatic` scale spans only 0 to 6 semitones because levels 0 to 6 index the first seven entries. See `melodic-steps-algorithms/05-note-output.md`.
