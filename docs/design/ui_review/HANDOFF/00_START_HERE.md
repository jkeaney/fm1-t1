<!-- SPDX-License-Identifier: GPL-3.0-only -->
# FM-1 screen design: handoff package (read this first)

Written 2026-11-12 by the Claude session that ran the UI review, for a Claude design session that will **own the screen
design from here**. Everything you need is in this folder or linked from it. Paths are relative to the repo root
(`/Users/jameskeaney/Dev/apps/FM-1 - T1 - Bassline`); the branch is `t1-hardware-bringup`.

## The one-paragraph situation

The FM-1 is a hardware step sequencer whose firmware we are rebuilding around a **UX-first** screen. The screen is a
**240 x 240 px LCD that is only 30 mm wide**. We spent a long session reviewing the existing (shipping) screen and
iterating mock-ups (v1, v3, v3.1, v3.2) with a pixel-exact render tool and an independent two-part critique
(design review + measured evidence). The owner rejected the first two as "**too much information, too small**", approved
the *direction* of a sparse "glance" screen, and then, shown the latest render (v3.2), said: **"It's not good enough."**
They did **not** say what is wrong. The design is **not frozen**. Your job is to own it from here: find out what is
wrong, fix it, and get the owner's simple "**good enough**" on pictures before any firmware is built.

## Your first three moves (in order)

1. **Interview the owner before drawing anything.** Show them the v3.2 renders one at a time (see
   `09_ASSET_INDEX.md`; send them with `SendUserFile`) and ask, per image, "what is wrong here?" Do not guess: the last
   session guessed, and the guess was not good enough. Hypotheses to test are in `08_OPEN_QUESTIONS.md` (section 1).
2. **Read `04_HISTORY.md` and `05_FINDINGS.md`** so you do not repeat rejected designs or re-find known defects.
3. **Set the bar with the owner in one sentence** ("good enough" means ...) and write it into `07_ACCEPTANCE.md`
   before iterating. The owner's own definition so far: *"Perfect just means the UX is communicated cleanly to the user
   and the device is usable."*

## What is in this package

| File | What it is |
| --- | --- |
| `00_START_HERE.md` | this file |
| `01_BRIEF.md` | the product, the user, the job the screen does, the workflow test, the owner's rules |
| `02_CONSTRAINTS.md` | the physical and firmware limits that shape every pixel (30 mm, three fonts, canvas, palettes, RAM) |
| `03_DECISIONS.md` | every owner decision that binds the design, with its status (one is withdrawn) |
| `04_HISTORY.md` | the versions tried, scores, what the owner said, what each taught us |
| `05_FINDINGS.md` | every open defect and critique finding, with status and where the evidence is |
| `06_PROCESS.md` | how to work: tools, commands, the critique method, delegation rules, repo rules, the owner's style |
| `07_ACCEPTANCE.md` | what "done" means for the design, and the hand-off into firmware tasks |
| `08_OPEN_QUESTIONS.md` | what we do not know, starting with "what is not good enough?" |
| `09_ASSET_INDEX.md` | every file that matters, with a one-line purpose |
| `SESSION_PROMPT.md` | a ready-to-paste prompt that starts the design session |
| `assets/` | the owner's grid sketch and the panel photo |

## Ground rules that are easy to forget

- **Do not flash the device and do not push to git without asking the owner.** (CLAUDE.md.)
- **Do not build firmware from any mock until the owner says "good enough".** Decision 120 (freeze v3.2) is withdrawn.
- **Show pictures, not descriptions.** The owner reviews images and answers in a word or two. Use `SendUserFile`.
- **Do not trust an agent's own "this is perfect" or its summary.** Look at the renders yourself, and get an
  independent critique before telling the owner something is good. The last session over-praised v3 and the
  independent review scored it 24/40.
- **One thing is time-sensitive in the backlog:** task `P4-012` (the cut-over that wires the *old* dense UI into the new
  core) is **ready** and would be picked by `/next-task`. Do not let it run before the design lands. See
  `07_ACCEPTANCE.md`, section "Backlog hazard".
