<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 05 Findings: every open defect, with status and where the evidence is

Source of truth: `critique_v3_2/ASSESSMENT_A_v3_2.md` (design review, 27/40) and `critique_v3_2/EVIDENCE_B_v3_2.md`
(measurements); the v3 equivalents are in `critique_v3/`. Status values: **OPEN** (not fixed in any mock), **FIXED-IN-v3.2**
(drawn correctly in the v3.2 mock but the owner rejected v3.2 as a whole), **SPEC** (the fix is a change to a TSV or
spec doc, not to the drawing), **DECIDED** (the owner decided; not yet drawn).

## 1. From the independent design review of v3.2 (27/40)

| Sev | Finding | Status |
| --- | --- | --- |
| P1 | **Lane bars show velocity, not pitch**: dicing NOTES/SHAPE changes nothing visible; soft notes (velocity 1-3) look like rests | **DECIDED** (119: pitch height; accent a shape; minimum bar). Not drawn |
| P1 | **The channel setup screen (CHN key) is never drawn.** The cue says `CHN K2 = MIDI CHANNEL` while the knob card still reads `SLIDE 6`. A MIDI channel steal sets another channel to OFF without a toast (the spec requires one) | **OPEN** (needs a designed page + use case) |
| P1 | **Risky moments mislabelled or unflagged:** the preset toast says `SAVE: KEEP HERE` but SAVE overwrites the pending slot with the current state (`button_modes.tsv` B_SAVE tap); `CLOCK LOST` appears only in a transient cue (header identical to internal clock); the dice hint appears in only 14 of 36 toasts; a selected channel that is muted + MIDI OFF + recording gives no warning | **OPEN** |
| P2 | **Map accuracy and layout:** map B omits the `PRV1..4` holds on M1-M4 (`key_map.tsv:211,224,237,250`) and the `RIG` hold on CHN (`key_map.tsv:263`); map A labels OCT+ `ROW+` (spec `ROW 2`, `button_modes.tsv:237`) and HOME `CANCEL`/`MENU` (spec `HOME`/`HLP/MN`, `:110-111`); ARP SPAN reads `1` (spec `1 OCT`, `knob_map.tsv:36`); SHAPE toasts omit `+ MOD` (`ALGORITHM_SET.md:136`); **7 spec toasts are not drawn**; the 2 x 6 button panel is drawn 4 x 3; map A's printed names are AF_S in mid grey | **OPEN** |
| P2 | **HOME timing conflict:** v3.2 opens MENU after a 1.5 s hold plus ~1 s countdown; `button_modes.tsv` still says "release after 0.7 s opens the MENU" (every look at the map would land in the menu if the TSV wins) | **SPEC** (decision 118 not yet applied to the TSVs) |
| P2 | **Playing column hides information:** inside the playhead column the accent cap and bar are both background-coloured; ratchets 5-8 are pixel-identical to 4 (the data model only supports ratchet 1..4: `sq_types.h:83`, so 5-8 should not occur: the mock draws values the firmware cannot produce); the slide chain collapses to one stripe | **OPEN** |
| P2 | **Grid and contrast:** channels, step groups and knobs share one 4-column grid and read as related columns; vertical gaps 1,1,1,5,2,2,2 px; the dimmed numeral of a muted selected channel is < 4.5:1 on 6 of 9 palettes (3.17 on PAPER); 24 of 79 non-text pairs are < 3:1 on GREY | **OPEN** |
| P2 | **Lost clock:** the header for `EXT lost` is pixel-identical to the internal-clock header (only the cue says so); FOLLOW off has no persistent indicator beyond a ghost column + page boxes | **OPEN** |
| P3 | **Mock bugs that would mislead a review:** `lane_muted` shows CH1 muted while the cue says CH2; `cue_mute_held` shows the *tap* text, not the hold text; `state_first_run` shows a lane full of edited steps; 5 cue strings appear in no cue-table render | **OPEN** (fix in the mock source `proposal_v3_2.py`) |
| P3 | **Missing cues:** none for `B_GLO` hold, `B_HOME` hold, `CHN` hold (RIG), the white-key STEP tap, `EDIT` hold, both-OCT hold | **OPEN** |
| P3 | `1->2` (MIDI routing on the tile) is undecodable for a first-timer; seven cue names refer to black keys the unit may label differently (`M2 SOL CHN RST`...) | **OPEN** (see open question 6) |
| P3 | The first-run frame is flush against the MIDI arrow; MIDI 10-16 plus a mute chip leaves 2 px; the preset settle bar is a constant 17 of 34 px when no toast is up | **OPEN** |

## 2. Findings that v3.1/v3.2 *did* fix (so you do not re-fix them)

From the v3 critique, drawn correctly in v3.2 (see `proposals/v3.2/NOTES.md` section 1, a 53-row table):
the HOME gesture; `ROW 1/2/3` shown in AF_M; mute and MIDI-OFF as separate marks; lane key; SNAKE path as a picture and
other geometric enums as pictures; square corners; toasts that lead with the printed button name; undo/redo/preset
toasts that state consequence; the unlabelled bottom band replaced by a hand-following cue; the 5 px grid replaced by a
**measured** 15 x 4 px module; a 15-state sheet; palette tokens defined on all nine palettes with a mono fallback;
beat shading instead of 6 x 3 px pips; MIDI channel on every tile; accent as a shape that survives colour-blindness.

## 3. From the original measured evidence (the shipping screen), still true as *requirements*

- The default palette `GREY` has no accent colour (`T_ACCENT = T_REC = T_TEXT`).
- Text pairs under 4.5:1 on GREY: `T_DIM` on `T_RAISE` 1.67:1 (a muted channel's legend label); `T_DIM` on `T_BG` 2.87:1
  (muted cells, the dimmed knob row, page-map digits, absent dice layers).
- The firmware today draws **raw integers** for enumerated knobs (`DIR 0`); a value-name table does not exist.
- Only the `ACID` row of `ALGOS[]` is filled; five rows are NULL until P5-002..P5-004/P5-007/P5-008 (FB-014 added a
  null-row guard to P4-012).

## 4. Facts I verified in the specs (use them; they were easy to get wrong)

- `B_SAVE` hold (0.7 s) = **undo the last preset load** (state and header slot go back, applied at the next step; again
  = redo); while a slot is pending: **cancel it instead** (`button_modes.tsv:129`). It is **not** "BACK".
- `B_SAVE` tap saves the whole device state to the header slot, or **to the pending slot and cancels its load**.
- `B_OCTUP` tap: ROW 1 -> ROW 2; ROW 2 -> **ROW 3 (the RIG row)** only when the channel has a device with a wire that is
  not `DEV`; on ROW 3 inert with the toast `LAST ROW: OCT- = ROW 1` (`button_modes.tsv:237`).
- `PRESETS` pending/load model: `Pnn?` pending; stopped: loads 1.5 s after the last detent (`SQ_LOAD_SETTLE`); playing:
  on the first 768-tick bar that starts at least that long after.
- A held mute key + `ALGORITHM` turn = that channel's **preview sound** (`control_map.tsv:152`).

## 5. The open design questions the v3.2 builder left (its NOTES section 9)

ARP K3 `SPAN` vs `OCTS`; map split A/B and how to switch pages (OCT while HOME is held collides with touch-for-help);
what is printed on the encoders; dice feedback = toast then cue / toast only / cue only; toast height 40 px vs 32 px;
MIDI channel as `[arrow]n` and OFF as `--`; `UNDO L` vs `UNDO LD` (the latter does not fit); REC ring/disc; a persistent
FOLLOW indicator; what ROW 3 shows (RIG wires or the MOD page). See `08_OPEN_QUESTIONS.md`.
