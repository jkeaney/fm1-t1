# Rhythmic Steps: layered drum-step sequencer, algorithm reference

A functional specification of the algorithms in the Ableton Max for Live device **Rhythmic Steps** (from the *Sequencers* pack, built in Max 8.6),
plus a decoded review of its **19 factory presets**. Written so the behaviour can be rebuilt for the FM-1 sequencer.

## Purpose and source

- **Purpose:** personal study notes for a university assignment on algorithmic sequencing.
- **Source:** analysis of `Rhythmic Steps.amxd` (patcher about 800 KB, 47 subpatchers plus 6 embedded helper patchers) and the 19 `.adv` device presets in
  the same folder. The only embedded scripts are UI helpers (`VisibleBang.js`, `sliderRangeControl.js`); all musical logic is in the patch graph.
- **Citation:** Ableton AG. *Rhythmic Steps* [Max for Live device, Sequencers pack]. Check the pack page for version and year.
- The docs describe behaviour in original wording and pseudocode, and do not reproduce the patch. Companion sets:
  [sting](../sting-algorithms/README.md), [snake](../snake-algorithms/README.md), [ml185](../ml185-algorithms/README.md), [steparp](../steparp-algorithms/README.md).

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Verified by tracing objects and connections, or read directly from the preset files. |
| **[I]** | Inferred from graph shape plus names; very likely right. |
| **⚠** | Not fully traced. Treat as a hypothesis and confirm against the running device. |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | Four lanes, five layers each, and how a hit is made. |
| 2 | [02-inputs.md](02-inputs.md) | All 59 parameters and the pattern data. |
| 3 | [03-clock-and-shuffle.md](03-clock-and-shuffle.md) | The step clock, per-step time shuffle, clock speed, auto retrigger. |
| 4 | [04-lane-layers.md](04-lane-layers.md) | **The core:** layer sequencers with independent lengths, the 0 to 6 level scales, Random / Not-so-random / Reset. |
| 5 | [05-note-output.md](05-note-output.md) | Making a note: chance, alt note, velocity, roll, slide, delay, MPE channel. |
| 6 | [06-rebuild-checklist.md](06-rebuild-checklist.md) | Data model, build order, tests, open questions. |
| 7 | [07-presets.md](07-presets.md) | **All 19 presets decoded and reviewed.** |

## One-paragraph summary

Rhythmic Steps is a four-voice drum-pattern generator built around **layers with independent lengths**. Each of the 4 lanes (one drum pad each) runs **five separate 16-step
layer sequences**: *Chance* (does this step trigger), *Velocity*, *Alt* (play the alternate drum), *Roll* (add a quick repeat) and *Slide* (an MPE modulation value). Every layer has its own
length from 2 to 16, so the layers drift against each other and produce long evolving patterns from short loops (**polymetric layering**). Every value is a small integer **0 to 6**,
which keeps the controls fast to edit. A step clock fires on 16th notes with an 8-step **time-shuffle** table (each step moved by up to ±0.1 beat), then each lane builds a note by rolling its
chance, choosing the main or alternate pad, scaling velocity, adding repeats and delaying by a per-lane offset.

## What is distinctive (useful for the write-up)

1. **Independent layer lengths:** the pattern repeats only after the least common multiple of its layer lengths, so a 16-step lane with layers of 13 and 3 repeats every 624 steps. 
2. **Probability as a 7-level step value:** Chance 0 to 6 means 0, 17, 33, 50, 67, 83, 100% (`level > random(6)`), a coarse scale chosen for quick editing.
3. **Everything is level 0 to 6:** one number format for chance, velocity, alt probability, slide and (nearly) roll; one random generator fills all of it.
4. **Not-so-random:** a mutation tool that nudges every step by −1, 0 or +1, for variation that stays close to the current pattern.
5. **Per-step time shuffle with a global amount:** eight sliders move individual steps ahead of or behind the grid, rather than one swing percentage.
6. **Presets are tiny:** the whole pattern state is 80 small integers per lane plus 59 parameters, about 3 KB compressed.
