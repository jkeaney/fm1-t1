# 10. Tests, gaps and open questions

## 10.1 Existing coverage **[V]**

| Suite | Assertions | Result here | Covers |
| --- | --- | --- | --- |
| `torso_phase1_test.c` | 869 | pass | struct sizes/offsets, 192 PPQN period maths, task table, micro-queue, MIDI rings |
| `torso_phase2_test.c` | 100 | pass (two assertions added: all 16 scale masks, A♯ in A minor pentatonic) | Euclid, rotation, pace/ratchet, harmony, LFSR, p-lock arena |
| `torso_phase3_test.c` | 1,640 | pass; the ISR benchmark fails intermittently (F18) | division and swing, pipeline, polyphony stress |
| `torso_phase4_test.c` | 3,082 | pass | [TEMP], [GLOBAL], flash layout (simulated driver) |
| `torso_phase5_test.c` | 85 | pass | page/encoder dispatch, card text, p-lock editing |

These are strong on **structure and arithmetic** (sizes, offsets, formulas) and weaker on **musical and integration correctness**. Examples: the scale test asserts a hex constant (F1);
nothing checks that the on-screen scale name matches the played scale (F2); nothing exercises the device glue (`t1_glue.c`, `ui_t1.c`).

## 10.2 Recommended new tests (each would have caught a finding)

| Test | Catches |
| --- | --- |
| *(Added in the working tree)* every `scale_id` mask equals its interval list | F1 |
| UI `SCALE_NAMES[i]` equals the engine's name for the same id | F2 |
| Phrase ARCH: offsets rise then fall and peak in the middle; INV A: the mirror; each shape has more than one distinct value | F3 |
| Phrase excursion never exceeds the documented unit for `range = 1, 12, 127` | F4 |
| Style 0 with `pulses = 4` yields the same pitch on every pulse (or document the sweep); style > 0 applies the offset once | F5 |
| For each page × shift × encoder in GLOBAL mode, exactly one parameter changes (or none) | F6 |
| After `t1_ui_select_cycle(n)` the next bar plays cycle *n*'s pulse count | F7 |
| Each knob position toggles a distinct bit of RND TGT / STEP MOD | F8 |
| Fill the queue; every accepted note-on has its note-off, or both are dropped | F9 |
| Mute for N ticks then un-mute; track phase equals an un-muted twin | F10 |
| Set GROOVE, MAX LEN, swing, master scale; behaviour changes (or the control is removed) | F11 |
| Save then power-cycle (host: re-init and reload); the pattern survives; a lock can be set and cleared via the UI path | F12 |
| A zero-filled `CycleParams` can still take locks | F17 |
| Replace wall-clock timing with a bounded work count per tick | F18 |
| Host test that drives `t1_glue.c` + a fake `seq.c` clock for 8 bars and compares MIDI output to a golden file | integration gap |

## 10.3 Open questions (need the hardware)

| # | Question | How to check |
| --- | --- | --- |
| 1 | Does the default bass really leave the audible range of a TB-3 (notes above 100)? | Play the default pattern into a MIDI monitor for 8 bars. |
| 2 | Real worst-case tick cost on the FM-1 | Read the cycle counter around `t1_pipeline_tick` with the stress pattern of [08](08-output-and-dispatch.md). |
| 3 | Real [TEMP] revert time with interrupts off | Time the `memcpy` of 4,672 B on target. |
| 4 | Audio dropout or sync jitter at 300 BPM with all tracks busy | Run the stress pattern with an external clock. |
| 5 | Does the SPI flash used elsewhere erase in 4, 32 or 64 KB units (relevant if banks are ever persisted)? | Check the flash driver's erase granularity. |
| 6 | Does Live (as host) honour CC 123 on every channel used by tracks 2 to 6? | Stop mid-note with a long gate. |
| 7 | Intended unit of `range` and the intended meaning of STYLE (a note pool?) | Ask the author / decide the design. |

## 10.4 Suggested fix order

1. ~~**F1, F2** scale tables~~ **Done in the working tree** (masks corrected and reordered; test added). Remaining: remove or reorder the stale `T1_SCALE_NAMES`.
2. **F7, F6** cycle refresh and GLOBAL routing: small changes in the UI file, large effect on usability.
3. **F5, F3, F19** style semantics, the broken contours, and LFSR draw independence (F4, the `range` unit, is done).
4. **F12** wire step keys to p-lock editing and persist patterns.
5. **F9, F10** queue back-pressure and mute phase.
6. **F8, F11, F13** controls and documentation clean-up.
7. **F14 to F18** hygiene.

## 10.5 How to reproduce

```
cd "<repo root>"
cc -O1 -Ifirmware/src -o /tmp/t1_probe docs/t1-algorithms/verification/probe.c \
   firmware/src/torso_t1.c firmware/src/torso_t1_algo.c firmware/src/torso_t1_pipeline.c \
   firmware/src/torso_t1_state.c firmware/src/torso_t1_ui.c -lm
/tmp/t1_probe        # same for probe2.c
```
The captured outputs are in `verification/probe.out.txt` and `verification/probe2.out.txt`.
