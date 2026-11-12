<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 01 Brief: what the screen is for

Sources: `PRODUCT.md` (repo root), `docs/project/VISION.md`, `CLAUDE.md`, `docs/design/SEQUENCER_UX.md`. Read those; this
file is the design-relevant digest.

## The product

A **UX-first sequencer firmware for the M-VAVE FM-1** (a fork of Felucca 1.6 via tensor-programming/fm1-t1). It is a
**sequencer, not a synth**: four channels each send MIDI out; each channel has an **algorithm** (ACID, EUCLID, SNAKE,
STAGE, ARP, PATTERNS) and a pattern length (1..64 steps, up to 4 pages of 16). The producer enters steps on the 16 white
keys *or* dials a sequence in with the knobs, then **performs it**: the four knobs **morph** the pattern, buttons
**dice** (re-roll) individual layers (ALL, NOTES, ACCNT, SHAPE, ARTIC, HITS), mute keys mute channels, EDIT undoes.

## The user and the moment

A music producer with hardware synths (the reference rig: one Roland TB-3 listening on MIDI channel 2, driven through a
USB-MIDI host box). They sit at arm's length from the unit with **hands busy on knobs, buttons and keys**, glancing at
the screen between actions and not reading it for long. **Live performance is the harder case**: every screen must be
understood at a glance. (Owner, 2026-11-12: the screen is read at "studio at arm's length, hands busy"; live is the
test it must also pass.)

## The workflow test (the acceptance test of the whole product)

A producer with one TB-3 should be able, **from the screen alone**, to:
1. select channel 1 and set its MIDI channel (the TB-3 default is 2): the only setup;
2. choose an algorithm for channel 1;
3. either enter steps on the 16 white keys *or* dial a sequence in with the knobs and dice buttons;
4. press PLAY and **perform**: morph with the knobs, dice a layer, mute/switch channels, undo, change preset, **without
   leaving the main screen and without a menu**.

## The owner's rules (priority order; from CLAUDE.md / VISION.md)

1. **User experience first. No menu diving.** Every musical function is reachable from the panel. The screen always
   shows what the device is doing and what each control does *right now*. MENU is only for set-and-forget settings.
   Gesture budget: a **setup** function may take two gestures (a page open plus one control); a **performance** function
   takes one.
2. It is a **sequencer, not a synth**.
3. Channels drive MIDI (unique MIDI channels; giving a used one turns the other channel's MIDI to OFF).
4. Algorithms are **dialled in and performed** (knobs morph, buttons dice; generation is deterministic).
5. **Speed to music**: a new sequence plays within seconds.

## Product register and personality (PRODUCT.md)

Register: **product** (design serves the task). Personality: **fast, legible, confident**; a dense parametric
instrument screen in the spirit of the **Torso T-1 and Elektron** (page-based, every encoder labelled with parameter and
value, quick to read once learned). Plain and exact; colour carries meaning.

**Anti-references (what it must NOT be):** a menu-diving synth LCD; a busy spreadsheet; a toy or gamer skin (neon, glow);
a phone-app look (rounded cards, soft shadows).

**Design principles (PRODUCT.md):** (1) Glance first. (2) Every control is labelled where the eye already is.
(3) Hierarchy over density. (4) Colour never works alone. (5) Respect the fixed medium (bitmap fonts, 240x240 RGB565,
6-character labels).

**Accessibility:** never colour alone; text contrast >= 4.5:1 (>= 3:1 large/bold) measured from the real RGB565 palette;
a minimum glyph height for anything needed mid-performance, set by the fixed fonts.

## The tension at the heart of the design (name it, do not hide it)

Two of the owner's own requirements pull against each other:
- "**Everything the user can access is documented on the screen** so they know what a press will do" (the owner's words,
  2026-11-12), and the owner's original sketch (`assets/owner_grid_sketch.png`) put the two rows of 12 buttons on the
  main screen so "the two rows of buttons are always visible to what they do".
- "**Too much information, too small**" (the owner's verdict on the current screen and on mock-up v1) on a screen that is
  **30 mm** wide, where the smallest font is 1.1 mm tall at arm's length.

The last session resolved it by asking the owner to choose; the owner chose *"Glance screen + full map on demand"*
(main screen shows only live state, big; a full labelled map appears while HOME is held; toasts and a one-line cue
document controls in use). That is the architecture of v3 / v3.1 / v3.2. **The owner has now said the result is not good
enough without saying why.** It is entirely possible that the *resolution itself* (no button labels on the main screen)
is what is wrong. Test that first (see `08_OPEN_QUESTIONS.md`).
