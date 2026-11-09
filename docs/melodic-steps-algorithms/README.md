# Melodic Steps: five-lane polymetric melody sequencer, algorithm reference

A functional specification of the algorithms in the Ableton Max for Live MIDI effect **Melodic Steps** (from the *Creative Extensions* pack, built in Max 8.0),
plus a decoded review of its **7 factory presets**. Written so the behaviour can be rebuilt for the FM-1 sequencer.

## Purpose and source

- **Purpose:** personal study notes for a university assignment on algorithmic sequencing.
- **Source:** analysis of `Melodic Steps.amxd` (patcher about 350 KB, 9 subpatchers, plus 8 embedded helper patchers) and the 7 `.adv` presets in the same folder.
  No embedded scripts; all logic is in the patch graph.
- **Citation:** Ableton AG. *Melodic Steps* [Max for Live MIDI effect, Creative Extensions pack]. Check the pack page for version and year.
- The docs describe behaviour in original wording and pseudocode, and do not reproduce the patch. Companion sets:
  [sting](../sting-algorithms/README.md), [snake](../snake-algorithms/README.md), [ml185](../ml185-algorithms/README.md), [steparp](../steparp-algorithms/README.md), [rhythmic-steps](../rhythmic-steps-algorithms/README.md).

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Verified by tracing objects and connections, or read directly from the preset files. |
| **[I]** | Inferred from graph shape plus names; very likely right. |
| **⚠** | Not fully traced. Treat as a hypothesis and confirm against the running device. |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | Five lanes, one clock, and how a note is built. |
| 2 | [02-inputs.md](02-inputs.md) | All controls and the pattern data. |
| 3 | [03-lanes.md](03-lanes.md) | **The core:** five independent lane counters, 0 to 6 levels, Random / Not-so-random / Reset. |
| 4 | [04-clock-and-timing.md](04-clock-and-timing.md) | Note-value speeds, triplets, double/half time, swing, resync, MIDI-clocked mode. |
| 5 | [05-note-output.md](05-note-output.md) | Scale degrees, octave, length, velocity, chance, and the MIDI messages. |
| 6 | [06-rebuild-checklist.md](06-rebuild-checklist.md) | Data model, build order, tests, open questions. |
| 7 | [07-presets.md](07-presets.md) | **All 7 presets decoded and reviewed.** |

## One-paragraph summary

Melodic Steps builds a melody from **five parallel lanes of 16 faders**: *Octave*, *Transpose* (a scale degree), *Length*, *Velocity* and *Chance*. Each fader is a level from 0 to 6, and each lane has its **own length from 2 to 16**, so
the lanes cycle at different rates and combine into a long evolving pattern (a polymeter). On every clock step all five lanes advance; their five values are collected together and turned into one note:
pitch = root + scale degree + 12 × octave, velocity and chance from linear scales, and note length chosen from a table of note values. The scale (8 choices) and root note
can be set on the device or from incoming MIDI.

## What is distinctive (useful for the write-up)

1. **Melody as five independent layers:** pitch is split into octave and scale-degree lanes, so rhythm-free melodic contour, accent and probability patterns can each have a different loop length.
2. **Scale degrees, not semitones:** the Transpose lane holds an index into the chosen scale, so every value is always in key; switching scale re-colours the whole pattern.
3. **Seven levels everywhere:** one 0 to 6 value format for all lanes; one Random, one Not-so-random and one Reset action per lane, with a lane-specific neutral reset value.
4. **Note length from a table of 64th-note multiples** (1, 2, 4, 8, 12, 16, 32) rather than a continuous gate time.
5. **Tiny presets:** the whole pattern is 80 small integers plus root, scale and swing.
