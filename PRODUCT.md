# Product

## Register

product

## Users
Music producers who sequence hardware synths (the reference case is one Roland TB-3 on MIDI channel 2) from the M-VAVE FM-1. They sit in a studio at arm's length from the unit, hands busy on the knobs, buttons and keys, glancing at the 240x240 screen between actions and not reading it for long. Live performance is the harder case: every screen must still pass the test of being understood at a glance while playing.

## Product Purpose
The FM-1 firmware is a sequencer, not a synth. Four channels each send MIDI, each with an algorithm (ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS) and a pattern length. The producer enters steps on the white keys or dials a sequence in with the knobs, then performs it: knobs morph the pattern, buttons dice individual layers. The screen's job is to show what the device is doing and what every control does right now. Success is a producer who gets a sequence playing within seconds and never goes into a menu.

## Brand Personality
Fast, legible, confident. A dense parametric instrument screen in the spirit of the Torso T-1 and Elektron: page-based, every encoder labelled with its parameter and value, quick to read once learned. Plain and exact, with colour that carries meaning.

## Anti-references
- A menu-diving synth LCD: tiny text, nested pages, values hidden several screens deep.
- A busy spreadsheet: everything at equal weight so nothing stands out while playing.
- A toy or gamer skin: neon, glow and decoration at the cost of legibility.
- A phone-app look: rounded cards, soft shadows and web chrome on a hardware screen.

## Design Principles
1. **Glance first.** Anything needed mid-performance (what is playing, what each knob and button does now, what is muted) must read in a fraction of a second at arm's length. Detail is for the second look.
2. **Every control is labelled where the eye already is.** The screen names the current role of each knob, button and performance key, and a role that changes shows its new role while it is changed.
3. **Hierarchy over density.** The T-1 style is dense, but the live state (playhead, selected channel, mutes, current knob row) always outranks reference information.
4. **Colour never works alone.** Mute, selection, accent, slide and similar states also differ by shape, position or text.
5. **Respect the fixed medium.** The bitmap fonts, the 240x240 RGB565 panel and the 6-character label rule are the design material. A label that does not fit is rewritten, never shrunk to unreadable.

## Accessibility & Inclusion
- Never rely on colour alone to carry a state.
- Text contrast floor measured from the real RGB565 palette: at least 4.5:1 for body labels and 3:1 for large or bold text.
- A minimum glyph height for anything needed mid-performance, set by the fixed font sizes available (AF_S, AF_M, AF_L).
