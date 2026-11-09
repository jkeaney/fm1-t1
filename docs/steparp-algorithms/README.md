# Step Arp: step-sequenced arpeggiator, algorithm reference

A functional specification of the algorithms in the Ableton Max for Live device **Step Arp** (from the *Sequencers* pack,
built in Max 8.6), written so the behaviour can be rebuilt for the FM-1 sequencer.

## Purpose and source

- **Purpose:** personal study notes for a university assignment on algorithmic sequencing.
- **Source:** analysis of `Step Arp.amxd` (patcher about 556 KB, about 30 subpatchers, plus four embedded helper patchers). All the musical logic
  is in the patch graph; the only embedded script is a small UI helper (`VisibleBang.js`).
- **Citation:** Ableton AG. *Step Arp* [Max for Live device, Sequencers pack]. Check the pack page for version and year.
- The docs describe behaviour in original wording and pseudocode, and do not reproduce the patch.
  Companion sets: [sting](../sting-algorithms/README.md), [snake](../snake-algorithms/README.md), [ml185](../ml185-algorithms/README.md).

## Confidence legend

| Tag | Meaning |
| --- | --- |
| **[V]** | Verified by tracing objects and connections. |
| **[I]** | Inferred from graph shape plus names; very likely right. |
| **⚠** | Not fully traced. Treat as a hypothesis and confirm against the running device. |

## Reading order

| # | File | Answers |
| --- | --- | --- |
| 1 | [01-architecture.md](01-architecture.md) | Stages and the path from a held key to a MIDI note. |
| 2 | [02-inputs.md](02-inputs.md) | Every control, range and default. |
| 3 | [03-note-engine.md](03-note-engine.md) | **The core:** held notes → sorted list → octave expansion → step or chord arp, hold, scale snap. |
| 4 | [04-step-lanes.md](04-step-lanes.md) | The per-step lanes: on/off, velocity, length, chance, ratchet, transpose, CC. |
| 5 | [05-timing.md](05-timing.md) | Division, dotted/triplet, swing, step direction, loop length, transport lock, key retrigger. |
| 6 | [06-outputs-and-rebuild.md](06-outputs-and-rebuild.md) | Order of processing, data model, build order, tests, open questions. |

## One-paragraph summary

Step Arp is two machines joined together. The **note engine** (`ArpHeart`) takes the currently held notes, sorts them (played order, up, down, chord,
random, follow), expands them across 1 or more octaves (up, down, up/down, random), and hands out **one note (or one chord) per trigger**. The **step engine** is a
16-step clock that fires those triggers; each step has an on/off switch and its own **velocity, length, chance, ratchet, transpose** and a **CC value**, all edited as 16-slider lanes.
The chain is: pitch → transpose → scale snap → ratchet → chance → velocity and length → MIDI.

## What is distinctive (useful for the write-up)

1. **Note choice and rhythm are decoupled:** the note engine does not know about steps; it simply answers "give me the next note". Step on/off decides whether to ask.
2. **Six per-step lanes share one widget:** each lane is a 16-value list read by the current step number, with a min/max range and a Random button.
3. **Ratchet before chance:** every ratchet repeat rolls its own probability, so ratchets can thin out.
4. **"Follow" and "mountain" modes:** note order can follow the octave direction, and with up/down octaves the arp climbs then descends as one continuous line.
5. **Scale-aware transposition:** per-step transpose is applied before the scale snap, so transposed notes are pulled back into the scale.
