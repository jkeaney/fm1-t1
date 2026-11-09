# Torso T-1 sequencer (this project): algorithm reference and code review

A functional specification of the **T-1 algorithmic sequencer** implemented in this repository (`firmware/src/torso_t1*.c`, `t1_glue.c`, `ui_t1.c`), plus a **review of the code against its own
design documents** (`PLAN.md`, `TORSO_T1_GUIDE.md`). Same format as the six device studies, so the algorithms can be compared directly:
[sting](../sting-algorithms/README.md), [snake](../snake-algorithms/README.md), [ml185](../ml185-algorithms/README.md), [steparp](../steparp-algorithms/README.md),
[rhythmic-steps](../rhythmic-steps-algorithms/README.md), [melodic-steps](../melodic-steps-algorithms/README.md).

## Purpose and method

- **Purpose:** study notes for a university assignment, and a record of what the code really does (as opposed to what the guide says).
- **Method:** I read every T-1 source file in full, then **compiled the real sources on the host and probed them** with three throwaway harnesses (kept in [verification/](verification/)). The working tree changed during the review (scale masks, `range`, accent), so statuses in [../T1_ISSUES.md](../T1_ISSUES.md) are re-verified against the latest tree.
  Claims tagged **[V]** below were either read directly from the code or reproduced by running it. Nothing in the firmware was changed.
- **Test status in this checkout:** the project's five T-1 suites build and run. Phases 1, 2, 4 and 5 pass (869 + 98 + 3,082 + 85 assertions). Phase 3 (1,640 assertions) passes
  except for its **wall-clock ISR benchmark**, which fails intermittently on a busy host (13 to 37 µs when quiet, 87 to 163 µs when something else is running; budget 80 µs). That measures the host OS, not the target. **[V]**
- Companion data: [verification/](verification/) holds the probe sources and their captured output.

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Read from the code, or reproduced by running the real sources. |
| **[I]** | Inferred from code structure and comments. |
| **⚠** | Not confirmed (hardware or behaviour I could not run). |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | What runs on the device vs only in host tests; the clock; the data flow. |
| 2 | [02-inputs.md](02-inputs.md) | The data model, every control, and which fields really do something. |
| 3 | [03-rhythm-engine.md](03-rhythm-engine.md) | **Core:** Euclidean (Bjorklund) rhythms, rotation, divisions, swing, per-track step counters. |
| 4 | [04-ratchet-and-pace.md](04-ratchet-and-pace.md) | Repeats, the pace warp, gate length and velocity slope. |
| 5 | [05-harmony-and-pitch.md](05-harmony-and-pitch.md) | The pitch chain: phrase, style, scale quantise, chords, voicing. |
| 6 | [06-random-and-locks.md](06-random-and-locks.md) | The Galois LFSR, probability, accent, and parameter locks. |
| 7 | [07-state-and-storage.md](07-state-and-storage.md) | [TEMP], [GLOBAL], flash layout, what is and is not saved. |
| 8 | [08-output-and-dispatch.md](08-output-and-dispatch.md) | The micro-event queue, voice allocation, MIDI out. |
| 9 | [09-review-findings.md](09-review-findings.md) | **The review:** 19 findings with evidence, severity and suggested fixes. The maintained issue list is [../T1_ISSUES.md](../T1_ISSUES.md). |
| 10 | [10-tests-and-open-questions.md](10-tests-and-open-questions.md) | Test gaps, recommended new tests, hardware checks. |

## One-paragraph summary

The T-1 sequencer gives each of **6 tracks** a bank of **6 cycles**; a cycle is a 128-byte block of about 30 parameters. A track plays its active cycle as a loop of up to 64 steps, where the
**rhythm is a Euclidean pattern** (Bjorklund distribution of *pulses* across *steps*, rotated, with optional forced extra pulses) and each step lasts a **note division** (11 values from 1/1 to 1/32T and dotted) with optional **swing**.
Each pulse can fire a **probability roll**, builds a **pitch** (60 + offset + melodic contour + arpeggio index, optional random, then snapped **down** into a scale), forms a **chord** of 1 to 4 stacked scale tones with a **voicing**,
and expands into 1 to 16 **ratchet repeats** whose timing follows a quadratic "pace" warp and whose velocity follows a linear slope. Randomness comes from a per-track **16-bit Galois LFSR** that can be
reset every 1 to 6 passes so the "random" line loops. Track 1 plays the FM-1's own synth; tracks 2 to 6 go to **USB-MIDI** on their own channels. Live performance uses **[TEMP]** (snapshot and revert), **[GLOBAL]** (relative edits across all tracks) and **parameter locks**.

## What is distinctive (useful for the write-up)

1. **Cycle blocks as a unit of variation:** each track has 6 whole parameter sets (cycles) it can switch between, rather than 6 sequences of notes.
2. **Rhythm by mathematics, not entry:** a pattern is two numbers (pulses, steps) plus a rotation; everything else is derived.
3. **Looping randomness:** the LFSR is re-seeded at pass boundaries, so a "random" melody repeats exactly every N passes (verified: probability pattern identical each pass; a 4-pass loop repeats exactly).
4. **A tiny deterministic core:** no heap, no floating point, integer-only; the whole pattern is 4,672 bytes and a revert is a `memcpy`.
5. **A gap between design documents and shipped behaviour** (see [09](09-review-findings.md) and [../T1_ISSUES.md](../T1_ISSUES.md)): several documented controls are inert. The five wrong scale masks and the `range` unit found in the first pass have since been fixed in the working tree.
