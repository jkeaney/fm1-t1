<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 03 Decisions that bind the design (and their status)

The project-wide decision log is `docs/project/OWNER_QUESTIONS.md` (decision tables at the end; rows 116-120 are from
this session). This file lists the decisions **from the UI review** with the owner's words where I have them, and flags
which ones the later mock-ups quietly **did not honour**. Owner quotes are verbatim from the 2026-11-12 session.

## A. Decisions recorded in OWNER_QUESTIONS.md

| # | Decision | Status |
| --- | --- | --- |
| 116 | The screen is 30 mm wide; AF_S is not used for anything read while playing; the main screen is a **glance screen** showing only: the four knob values, the selected channel's pattern with a clear playhead, the four channels' mute state, the selected channel and its algorithm. ~8-11 text roles, no AF_S | **decided**; but see B1: the owner may no longer want this |
| 117 | Controls are documented by (a) large **toasts** after a press/turn that lead with the printed button name and state the consequence and exit, (b) a one-line AF_M **cue** in the bottom band that follows the hand, (c) a full labelled **MAP** while HOME is held. No button grid / black-key legend on the glance screen | **decided**; same caveat |
| 118 | HOME hold shows the MAP at once; keep holding to escalate to the MENU (countdown on the map); letting go earlier returns to the main screen. **Replaces** "release after 0.7 s opens the MENU". Exact times open | **decided (timings open)** |
| 119 | Lane bar **height = the note's pitch** within the pattern's range; accent is a marked **shape**; soft notes keep a minimum bar so they never look like rests | **decided** (the v3.2 renders still show velocity; not re-rendered) |
| 120 | Freeze the v3.2 visual language | **WITHDRAWN**: the owner said v3.2 is "not good enough" |

## B. The owner's words that shape the design (chronological)

1. **Context of use:** "Studio at arm's length, hands busy." Live is "the harder case". References: **Torso T-1 /
   Elektron** ("a dense parametric sequencer screen: page-based, every encoder labelled with its parameter and value,
   fast to read once learned"). Anti-references: *a menu-diving synth LCD; a busy spreadsheet; a toy or gamer skin; a
   phone-app look.* Accessibility: *never colour alone; a text contrast floor; a minimum glyph size.*
2. **"the screen is 30mm. those screens you shared with me are fails for me. too much information, too small."**
   (verdict on the shipping screen's mock-ups and on the first proposal.)
3. **The owner's own concept, a 6x6 grid sketch** (`assets/owner_grid_sketch.png`), offered as "a concept to critique,
   not a requirement": top row `ALG | CH | INFO`; then `K1 K2 K3 K4`; then "representation of the 4 ch"; then **two
   rows of six buttons** (`FX SE EN LF ED GL` / `HO SA AR SE PL RE`); then "White Keys"; then "Black Keys". In the owner's
   words:
   - "for me, the layout should be divided into a grid ... **this way the two rows of buttons are always visible to what
     they do**."
   - "**the actual musical output of all channels should not take much space, all the producer needs to see is the
     pattern distribution for the current page of the sequence, as dots or markers, and whether it's muted or not.**"
   - "**the key here is that everything the user can access is documented on the screen so they know what a press will
     do.**"
   - "one thing missing is what the select, presets and algo knobs do. ideally the algo knob changes the algo, but not
     sure what select or presets do." (Answer: `SELECT` = channel, `ALGORITHM` = the selected channel's algorithm,
     `PRESETS` = preset slot P01..P16 with a pending/load model; see `02_CONSTRAINTS.md` section 3.)
4. **Documenting the controls given 30 mm:** asked to choose between (a) glance screen + full map on demand, (b) the
   screen follows your hands, (c) two screens PERFORM/MAP, (d) everything on one bigger screen, the owner chose **(a)**.
5. **At a glance, pick four:** the owner picked all four offered: **the four knob values; where the pattern is
   (playhead + steps); which channels are muted; the selected channel and its algorithm.**
6. **Menu gesture:** "Keep holding to escalate (Recommended)".
7. **Bottom-band legend:** "Medium-font cue that follows the hand (Recommended)", i.e. not a small-font legend.
8. **Scope of the last pass:** "Everything including the P2 and P3 items." **Priority:** "Reassurance at the risky
   moments" (dice, undo, preset).
9. **Lane bars:** "Pitch height, accent as a shape (Recommended)."
10. **"Perfect just means that the UX is communicated cleanly to the user and the device is usable."** and, earlier,
    "**You need to critique every single thing on the page. And challenge every pixel. Until it's perfect.**"
11. **Process wishes:** use the **Impeccable** skill to review and propose the best UX/UI ("Colours. Fonts. How to lay
    out."); "Every single element needs to be lined out. Its function. Purpose."; show mock-ups **in the chat** ("I'll
    give a simple it's good enough or not"); and finally: "**It's not good enough. Please write a package with all the
    inputs and processes and requirements for a Claude design session to pick up and own.**" (this package)

## C. Where the later mock-ups did NOT honour what the owner said (check these first)

These are not reconstructions of intent; they are mismatches between the owner's words above and what v3.1 / v3.2 draw.
Any of them could be the reason for "not good enough".

| Owner said | v3.2 does | Why it may matter |
| --- | --- | --- |
| "the two rows of buttons are **always visible** to what they do" | **No button labels on the glance screen at all** (only a hand-following cue + toasts + a held map) | The owner's central requirement may have been traded away in the choice in B4; the choice was a multiple-choice question with long option text. Re-test it with pictures |
| "pattern distribution for the current page ... as dots or markers ... **of all channels**, and whether it's muted" | Shows the **selected** channel's lane big; the other three channels show only a number, algorithm, MIDI channel and mute mark: **no pattern for them** | The owner wanted a small overview of all four patterns |
| Owner's grid: one coherent 6-column button area mirroring the panel | The panel's 2 x 6 button block appears only on map page A, rearranged **4 x 3** | Mirroring the panel was the point of the grid concept |
| "divided into a grid" | v3.2 uses a 15 x 4 px module and one 4-column grid for channels, steps and knobs, which reads as related columns (critique P2) | A visible, honest grid was asked for; this grid is arbitrary |
| "everything ... documented on the screen" | Black-key roles exist only on map page B; the cue names black keys that the unit's silkscreen may label differently | See open question 6 |

## D. Earlier project decisions that still bind (pointers, not a full list)

`docs/project/OWNER_QUESTIONS.md`: **32b** boot straight into the new sequencer UI; **48 / 97** OCT-/OCT+ are the knob
**row** buttons (three rows of four, ROW 3 = the RIG row); **B2 printed-word fit** (the dice roles follow the printed
button words: FX=ALL, SCL=NOTES, ENV=ACCNT, LFO=SHAPE, ARP=ARTIC, SEQ=HITS; EDIT=UNDO, GLO=SET); CH1-CH4 default MIDI 1-4
with a first-run step; MIDI channels are unique across channels; tempo source AUTO; preset load model (settle 1.5 s /
next bar); **EUCLID pace is two-way** (115, a data-model limit, not a UI choice). `docs/project/VISION.md` rules 1-5.
