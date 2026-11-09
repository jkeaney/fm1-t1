# Sting-style acid line generator: algorithm reference

A functional specification of the generative algorithms in the Max for Live device **Sting 2.08**
(an "acid line generator"), written so the behaviour can be rebuilt for the FM-1 sequencer.

## Purpose and source

- **Purpose:** personal study notes for a university assignment on generative sequencing.
- **Source:** analysis of `sting 2.amxd` (Sting 2.08, by Iftah Gabbai, 2025), by reading the
  device's patcher graph and its embedded JavaScript helpers.
- **Citation:** Gabbai, I. (2025). *Sting 2.08* [Max for Live device].
- The docs describe what the algorithms do, in original wording and pseudocode, and do not
  reproduce the patch or JS source.

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Verified by tracing objects and connections in the patch. |
| **[I]** | Inferred from the graph shape plus parameter names; very likely right. |
| **⚠** | Not fully traced. Treat as a hypothesis and confirm against the running device before relying on it. |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | What are the stages, and how does data flow between them? |
| 2 | [02-inputs.md](02-inputs.md) | Every control, its range, default, and which stage consumes it. |
| 3 | [03-pitch-generation.md](03-pitch-generation.md) | Classic (v1) and v2 note generators, Type morph, scale snapping. |
| 4 | [04-rhythm-and-dynamics.md](04-rhythm-and-dynamics.md) | Gate, velocity and accent, octave generators. |
| 5 | [05-playback-timing.md](05-playback-timing.md) | Clock, divisions, direction, swing, jitter, length, offset. |
| 6 | [06-outputs.md](06-outputs.md) | The pattern data model and the note events produced. |
| 7 | [07-rebuild-checklist.md](07-rebuild-checklist.md) | Build order, invariant tests, FM-1 porting notes, open questions. |

## One-paragraph summary

Sting generates a 16-step pattern in four parallel lanes: **pitch** (0 to 11), **octave** (-2 to +2),
**gate length** and **velocity**. Pitch comes from "urn" sampling (random draws without replacement),
so a pattern never repeats a pitch class before using up the set. A *Type* control morphs the
pattern from fully random towards "acid" by replacing steps with repeated notes. An optional
scale stage snaps pitches to the nearest scale degree without losing the original notes. Octave,
gate and velocity are small step-position-weighted coin flips. Playback applies direction, offset,
swing and a probabilistic step jitter. Parameters can also drift by themselves using a random
walk ("CakeWalk").
