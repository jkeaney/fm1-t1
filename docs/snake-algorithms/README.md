# Snake: polymetric grid sequencer, algorithm reference

A functional specification of the algorithms in the Max for Live device **MDD Snake V3.2.3**
("Maxime Dangles Device Snake", built in Max 8.1.7), written so the behaviour can be rebuilt for the
FM-1 sequencer.

## Purpose and source

- **Purpose:** personal study notes for a university assignment on generative and algorithmic sequencing.
- **Source:** analysis of `MDD___SNAKE_V3.2.3.amxd` by reading its patcher graph (about 3.7 MB, 177 nested
  subpatchers) and the embedded saved state. The only embedded JavaScript is `M4L.chooser.js`, a stock
  Cycling '74 parameter-mapping helper, so **all the musical logic is in the patch graph**.
- **Citation:** Dangles, M. *MDD Snake V3.2.3* [Max for Live device]. (Author credit is taken from the
  device's own title; check the device page for the full reference and year.)
- The docs describe what the algorithms do, in original wording and pseudocode, and do not reproduce the
  patch. Companion set: [../sting-algorithms/](../sting-algorithms/README.md).

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Verified by tracing objects and connections, or checked numerically against the device's stored data. |
| **[I]** | Inferred from the graph shape plus parameter names; very likely right. |
| **⚠** | Not fully traced. Treat as a hypothesis and confirm against the running device. |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | What are the lanes and how does data flow from step to MIDI note? |
| 2 | [02-inputs.md](02-inputs.md) | Every control, range, default, and which stage consumes it. |
| 3 | [03-snake-shapes.md](03-snake-shapes.md) | **The core idea:** the 14 traversal shapes over the 4×4 grid, with tables. |
| 4 | [04-pitch-quantise.md](04-pitch-quantise.md) | Dial value → scale degree → MIDI note, plus all 37 scale tables. |
| 5 | [05-lane-generation.md](05-lane-generation.md) | Random, Reset, gate, velocity and the Custom modulation lanes. |
| 6 | [06-playback-timing.md](06-playback-timing.md) | Per-lane clock, speed, direction, length, shift, swing, note length. |
| 7 | [07-outputs-and-rebuild.md](07-outputs-and-rebuild.md) | Note output, MIDI-in control, data model, build order, tests, open questions. |

## One-paragraph summary

Snake has **four independent 16-step lanes**: Notes, Gates, Velocities and Custom (a parameter
modulation lane, instanced three times). Each lane holds 16 values laid out as a 4×4 grid, but the order in
which the cells are *played* is set by a **Snake Shape**, one of 14 permutations of the grid (rows,
columns, serpentines, diagonals and spirals). Each lane has its own length, speed, direction and
step-shift, so the lanes drift against each other like polymeters. The **Gates lane is the trigger**: when
its step lands on a cell whose gate is on, the current pitch (from the Notes lane) and velocity
(from the Velocities lane) are released as a note. Pitch is stored as a continuous dial value and converted at
play time through a chosen scale table (37 scales, plus a user scale played in over MIDI). Swing and note
length are applied at the output.

## What is distinctive (useful for the write-up)

1. **Order is data.** Playback order is a lookup table (a permutation), separate from the stored values,
   so changing the shape re-reads the same 16 values in a new order without changing them.
2. **Lane independence = polymeter for free.** Every lane runs its own clock, with no shared bar-length constraint.
3. **Pitch is stored as a continuous control, quantised late.** The same stored values play differently under
   a different scale or root.
4. **Trigger / pitch / velocity separation.** Rhythm, pitch and dynamics are separate sequences that are
   sampled together at each trigger.
