# Vision: a UX-first sequencer for the FM-1

The north star for every task. When a task's acceptance criteria and this document disagree, this document wins:
raise it as feedback (see [PROCESS.md](PROCESS.md)), don't silently pick one.

## Product rules (priority order)

1. **User experience lives or dies on the panel.**
   - No menu diving: every musical function is one press, hold or turn away. The menu holds only set-and-forget
     settings.
   - The screen always shows what the device is **doing** (transport, channel, algorithm, what is playing) and what
     it **can do** right now: the current role of every knob and button, labelled on screen.
   - A control whose role changes (for example with a held modifier) shows its new role while it is changed.
   - Gesture budget (owner ruling 2026-11-10): a **setup** function (MIDI channel, length, tempo, scale, adding a page and the like) may take two gestures, a page open plus one control; a **performance** function (morph, dice, mute, undo) takes one. Neither needs the menu.
2. **A sequencer first, not a synth.** The internal synth exists only to preview sequences. Synth editing,
   sampling, effects authoring and sound design are out of scope unless they serve previewing.
3. **Channels drive MIDI.**
   - The sequencer has channels, and each channel sends MIDI out.
   - A channel's settings are its MIDI channel, its algorithm and length, and (optional) a device with four mod slots (decisions 98, 108). MIDI channels are unique across the four channels: giving a channel one that another channel uses turns that other channel's MIDI to OFF (owner ruling 2026-11-10).
   - A channel's sequence comes from steps entered on the 16 white keys, or from an algorithm.
4. **Algorithms are dialled in and performed.**
   - Each channel picks an algorithm.
   - The knobs **morph** it continuously.
   - Buttons **dice** (re-roll) individual layers of it, such as rhythm, pitch, accents and length, without touching
     the other layers.
   - Generation is deterministic: the same settings and seed play the same pattern.
5. **Speed to music.** A new sequence should play within seconds, then be performable live.

## Reference workflow (the acceptance test for the whole product)

A producer has one synth, a Roland TB-3, connected:

1. Select **channel 1** and set its MIDI channel (the TB-3 default is 2). That's the only setup.
2. Choose an **algorithm** for channel 1.
3. Either enter steps on the 16 white keys, **or** dial a sequence in with the knobs and dice buttons.
4. Press PLAY and **perform**: morph with the knobs, dice the layers, and mute or switch channels, all without
   leaving the main screen.

## Glossary

| Term | Meaning |
| --- | --- |
| **Channel** | One sequencer lane that sends MIDI on its own MIDI channel. It has steps and/or an algorithm. |
| **Algorithm** | A generator that produces a channel's notes, for example Euclidean, acid line, stage or snake. Chosen per channel. |
| **Layer** | One independently generated part of an algorithm's output: rhythm, pitch, accent, length or gate. It can be diced on its own. |
| **Morph** | A continuous knob transform of an algorithm's output (density, range, variation, and so on). Undoable by turning back. |
| **Dice** | Re-roll one layer's seed. The previous seed can be recalled. |
| **Preview synth** | The FM-1's internal sound engine, used to hear a channel without external gear. |

## Hardware constraints (fixed)

- **USB-C is a USB device.** Driving DIN gear without a computer needs a USB-MIDI host box. **The 3.5 mm jack is
  MIDI IN only.**
- 72 KB RAM (keep at least 4 KB spare), a 360 KB pool, and a 240×240 screen.
- Controls: 14 buttons, 4 knobs plus 3 encoders, and 27 keys (16 white). See [../design/](../design/) once mapped.

## Starting point

- The firmware is a fork of Felucca 1.6, a full groovebox. The Torso T-1-style sequencer that was added first has
  been replaced (cut-over P4-012) by the sequencer core `sq_*.c` and the screen `ui_sq*.c`; only the T-1's Euclidean,
  pace, scale and contour helpers survive in `torso_t1_algo.c`. The core drives a TB-3 on channel 2 (HW-1, 2026-11-13).
- The T-1 algorithms were the **starting algorithm set**, assessed and redesigned (docs/design/algorithms/).
- The old T-1 defect list is history: [../T1_ISSUES.md](../T1_ISSUES.md).
- The algorithm catalogue to assess lives in `docs/*-algorithms/`.
