<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Session prompt: paste this into a new Claude Code session opened in the repo root

```
You now own the screen design for the M-VAVE FM-1 sequencer firmware. Repo root:
/Users/jameskeaney/Dev/apps/FM-1 - T1 - Bassline (branch t1-hardware-bringup).

Start by reading, in this order: CLAUDE.md, then docs/design/ui_review/HANDOFF/00_START_HERE.md and every file it lists
(01_BRIEF through 09_ASSET_INDEX), then PRODUCT.md and docs/design/ui_review/mock/README.md. The package is the
single source of truth for what has been tried, what the owner said, and what is still open.

SITUATION. The screen is a 240x240 px LCD that is only 30 mm wide. A previous session produced mock-ups v1, v3, v3.1,
v3.2. The owner rejected v1 and the shipping layout as "too much information, too small", approved the direction of a
sparse "glance" screen, then said of v3.2: "It's not good enough." They gave no reason. Decision 120 (freeze v3.2) is
withdrawn. Nothing about the look is locked, and no firmware may be built from any mock until the owner says "good enough".

YOUR JOB. Own the design until the owner says "good enough" about pictures, then hand it to the build as a spec, table
changes, an ADR, and backlog tasks (each with a use case and the UX gate). You design; you do not write firmware, flash
the device, or push to git. Ask before any of those.

DO THIS FIRST, before drawing anything.
1. Interview the owner with PICTURES. Send the v3.2 renders listed in 09_ASSET_INDEX.md one at a time with SendUserFile
   (load it with ToolSearch "select:SendUserFile") and ask what is wrong in each: what the eye goes to first, what they
   cannot read, what is missing. Then ask the one question "if you could change one thing, what?". Test the hypotheses in
   08_OPEN_QUESTIONS.md section 1, especially H1 (the owner wanted the two button rows visible on the main screen) and H2
   (they wanted a small pattern view of ALL FOUR channels). Show both alternatives as images before asking the owner to choose.
2. Write the owner's one-line definition of "good enough" into 07_ACCEPTANCE.md.
3. Resolve the physical unknowns listed in 08 section 3 (black-key printing on the unit, viewing distance, how many
   text items feel right, palette, whether colour is wanted).

THEN WORK IN SHORT LOOPS. A vertical slice (the main screen in 3-4 states) at 1x and 4x, sent as images, one round at a
time; do not build 70-scene proposals. Render only with the pixel-exact tool (docs/design/ui_review/mock/); never
draw by hand. Verify every on-screen label against docs/design/button_modes.tsv, knob_map.tsv, key_map.tsv before you show it.
Count text items by script, not by eye.

EVIDENCE BEFORE YOU CALL IT DONE. Run the Impeccable critique once per round in the form described in 06_PROCESS.md
section 4 (two independent agents: a design review that has not read the builder's notes, and a measured-evidence pass;
snapshot with critique-storage.mjs using the same path each time). Look at the renders yourself. Do not tell the
owner something is good until the independent critique and the owner both say so. Report what the critique found wrong.

RULES THAT BIND. 30 mm, 8 px per mm; fonts AF_S/AF_M/AF_L only (there is no other size); glance-screen text that is
read while playing is AF_L or a big shape, AF_M only for short position-identified names, AF_S only on held views;
labels at most 6 characters; colour never works alone; text >= 4.5:1; the roles of every control come from the TSVs;
the owner's workflow test (select channel 1 and set its MIDI channel, pick an algorithm, dial in with knobs and dice,
PLAY, morph/dice/mute/undo/preset without leaving the main screen) must pass from the screen alone.

HAZARD. `python3 tools/backlog.py next` returns P4-012, which would wire the OLD dense layout into the new core. Tell the
owner, and let them choose between blocking it and splitting the cut-over (07_ACCEPTANCE.md section D). Do not run it.

STYLE. Short, visual, plain language. The owner answers in a word. Use AskUserQuestion for decisions, with pictures first.
Commit locally with explicit paths and the Co-Authored-By / Claude-Session trailers. Ask before flashing or pushing.
```
