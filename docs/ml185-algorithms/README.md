# ML-185: stage-controlled sequencer, algorithm reference

A functional specification of the algorithms in the Max for Live device **ML-185 Sequencer** (v0.9, Nico Starke,
November 2009), written so the behaviour can be rebuilt for the FM-1 sequencer. The device is a software take on the
**RYK M-185** hardware sequencer (Roland System 100m era), where a **stage** system decides how long each note slot lasts and
how it is played.

## Purpose and source

- **Purpose:** personal study notes for a university assignment on algorithmic sequencing.
- **Source:** analysis of `ML-185 Sequencer.amxd` (patcher graph about 580 KB, 14 subpatchers) plus its one embedded JavaScript
  file `swingCalc.js` (about 2 KB). Also read: the device's own help text.
- **Citation:** Starke, N. (2009). *ML-185 Sequencer*, v0.9 [Max for Live device]. After RYK's M-185 sequencer.
- The docs describe the behaviour in original wording and pseudocode, and do not reproduce the patch.
  Companion sets: [../sting-algorithms/](../sting-algorithms/README.md), [../snake-algorithms/](../snake-algorithms/README.md).

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Verified by tracing objects and connections. |
| **[I]** | Inferred from graph shape plus names and the help text; very likely right. |
| **⚠** | Not fully traced. Treat as a hypothesis and confirm against the running device. |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | Two clocks, eight stages, and how a note comes out. |
| 2 | [02-inputs.md](02-inputs.md) | Every control, range and default. |
| 3 | [03-stage-engine.md](03-stage-engine.md) | **The core idea:** pulse index → stage, stage modes, clock divider, triggers. |
| 4 | [04-pitch-velocity-aux.md](04-pitch-velocity-aux.md) | The three dial layers, Random, duration and aux output. |
| 5 | [05-timing-swing-direction.md](05-timing-swing-direction.md) | Pulse clock, direction, fixed/variable length, swing. |
| 6 | [06-outputs-and-rebuild.md](06-outputs-and-rebuild.md) | MIDI output, data model, build order, tests, open questions. |

## One-paragraph summary

ML-185 has **8 stages**, each holding one pitch, one velocity and one aux value. A fast pulse clock (1/32, 1/16 or 1/8)
steps through a pulse count up to 64. Each stage has a **Length** (1 to 8 pulses), so the stages together lay out a sequence of
up to 64 pulses. Which stage owns a given pulse is found by a running (cumulative) sum of the stage lengths. Each stage also has a
**Mode** (0 rest, I single, II repeating, I- long single) and a **Division** (1 to 4, for Mode II only), which decide on which pulses
inside the stage a note fires. When a note fires, the stage's pitch, velocity, duration and aux values go out as MIDI. Direction, fixed or variable total length and swing
shape the pulse stream.

## What is distinctive (useful for the write-up)

1. **Stage lengths in clock ticks, not notes:** a stage is a *duration*, so the total sequence length is the sum of stage lengths and can be any value up to 64.
2. **Cumulative-sum addressing:** one pulse counter plus a chain of threshold tests replaces a table of step times.
3. **Mode and division are per stage:** three small switches (mode, length, division) give many rhythmic variations from only 8 stored notes.
4. **Direction acts on pulses, not stages:** reversing the pulse stream reverses the stage order *and* the order of pulses inside each stage.
