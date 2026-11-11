# SEQUENCER_UX: the locked sequencer functionality and UX (P1-017)

Task P1-017 (owner tenth round, 2026-11-10). Status: **for owner review** (gate `human-review`). Design only: no firmware,
no tests. When the owner approves it, this document is the single spec the dev tasks cite (ADR-0004); it supersedes the
control parts of [PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md) (B2 button layout kept), the step band and R1-R4 reserve
of that document, the pin model of [ADR-0003](../project/decisions/ADR-0003-sequencer-core.md) section 9, and the
ROW 1 / ROW 2 assignment of [FULL_CONTROL.md](FULL_CONTROL.md) section 3 (re-locked here, 3B).

> **Revision 3 (P2-012, HARMONY; for owner review; additive, see [HARMONY.md](HARMONY.md)).** The SCALE page (hold GLO) becomes the HARMONY screen
> (table id `SCALE` is kept so no context is added): K1 SCALE, K2 KEY, K3 `VOICE`, K4 `MOVE`; OCT- / OCT+ walk the `DRONE` (OCTAVE moves to the same
> buttons on SET, which had them inert); `SCL` dices the progression (`PROG`), `LFO` sets `BARS`, `ARP` sets the channel `ROLE`, `SEQ` starts a `NEW`
> progression; keys F3-E4 type chords 1-7 (the KEY is K2 only); the 27-key picture is replaced by the progression strip. Defaults are inert: with
> `1---`, `VOICE 2`, `DRONE` off no note changes (HARMONY.md 1). The GLO hold sub-label is `h:HARM`. EDIT tap also undoes the last HARMONY change.
> New scored rows U70-U79; U46 and U47 change their path. PERF gains a chord chip in the lane header only while HARMONY is active.

> **Revision 1 (2026-11-10, after the independent Opus review; for owner review).** Six fixes, applied to this
> document, the five TSVs, CONTROL_MAP, PERFORMANCE_DESIGN, ADR-0003, ALGORITHM_SET and the four algorithm docs:
> (1) **page sources**: each page stores which generator block it reads (2 bits per page, `pg_src`), a new
> page inherits the source of the page before, so adding a page is an exact copy in every case (1.3, 1.4, 9.1, 9.2);
> (2) **the L rule is restated openly** (8.1): a hold is L2 only when it is named permanently on the page where it
> starts, so PERF now carries hold sub-labels (`h:RST`, `h:HARM`, `h:HLP/MENU`, `h:UNDO L`) and the lane header names
> `hold=EDIT`; the honest count under both rules is in 8.5, and the hard-tap accent is rescored 9 (exception E3);
> (3) **save to another slot**: a turned PRESETS slot is pending (`P05?`) and a SAVE tap saves there instead of loading
> (U69); (4) **slides cross the loop end** (the next step after the last is step 1; cut only by STOP and a LENGTH
> change), and SLIDE means "glide into the next step" everywhere; (5) **HOME follows owner decisions 57 / 61**: tap =
> home, hold + touch = HELP, release after 0.7 s with nothing touched = MENU; the first draft's HOME-tap HELP page and
> its double-HOME dead end are gone; HELP fails the bar on input (I1) and is listed as exception E2 (owner decisions 57 / 61);
> (6) the partial last page, the STAGE `TOTAL` cue, the DRIVE sensing mode and its cost, default MIDI channels
> (owner ruling: CH1..CH4 = MIDI 1..4 plus a first-run MIDI CHANNEL prompt), the NOTE EDIT undo rule and the U28 disclosure. RAM is re-added (6819 B, 9.4).

> **Revision 2 (2026-11-10, after the second review; for owner review).** Six fixes: (1) **source labels tell the
> truth**: CHAN `K3 SOURCE` offers only the generator blocks that earlier pages actually play, each labelled by the
> first page that plays it, plus `OWN`; a block no earlier page plays any more is shown by its positions (`17-32`);
> the heard-span cue counts every unheard shown step, not only the highest block (1.4, W3); (2) **a pending preset waits**: while
> playing, it lands on the first bar that starts at least `SQ_LOAD_SETTLE` 1.5 s after the last detent, so a SAVE tap
> always has at least 1.5 s (4.1, U57 / U68 / U69, W7, 11.1, 11.2); (3) the E1 text matches U57 (11); (4) the count
> under the P1-014 letter is **43 of 69** (U18 step octave falls with the other step-hold rows; under the restated
> rule U18 is L2 = 10); U69 is L2 = 10; sum 734 (8.2, 8.5); (5) the `ChanRT` field order is given and compiled (24 B,
> 9.1); (6) PG+ on page 4 at LENGTH 64 toasts `64 STEPS = MAX`, and a TIE crosses the loop end on the same terms as a
> slide (cut only by STOP and a LENGTH change) (1.3, 9.2, 10).

Machine-readable tables (the source; this document explains them): [page_states.tsv](page_states.tsv),
[button_modes.tsv](button_modes.tsv), [knob_map.tsv](knob_map.tsv), [key_map.tsv](key_map.tsv) and
[control_map.tsv](control_map.tsv) (49 controls x 9 pages = 441 rows, mirrored in [CONTROL_MAP.md](CONTROL_MAP.md)
appendix A). `python3 tools/check_sequencer_ux.py` proves them complete and consistent and checks the score table of
section 8; `python3 tools/check_control_map.py` proves CONTROL_MAP.md agrees with control_map.tsv.

Inputs read: CLAUDE.md, VISION.md, LESSONS.md, OWNER_QUESTIONS.md (all rounds; R1-R4; eighth, ninth, tenth), ADR-0004,
PERFORMANCE_DESIGN.md, FULL_CONTROL.md, UX_REVIEW.md (rubric 3.1), CONTROL_MAP.md, control_map.tsv,
check_control_map.py, KEYBED_EXPRESSION_REFERENCE.md, ALGORITHM_SET.md, section 6 of the sting / t1 / snake / ml185
assessments, ADR-0003, SYSTEM_MAP.md, `firmware/src/panel.c`, and the panel photo `docs/controls.jpg` (looked at: the
16 white keys under 11 black keys grouped 3-2-3-2-1 and printed `OP1 OP2 OP3 | OP4 OP5 | OP6 PIT GLD | MONO POLY | -`;
the reference card prints `HOME hold: Menu`, `SAVE hold: undo the last load`, `OCT-/OCT+: octave`, `REC: arm to record`).

## 0. The instrument in one paragraph

**Dial an algorithm in, then own its steps.** The algorithm writes a 16-step pattern (the *generated layer*); every
white key is one of those steps, and a tap turns it on or off. **Hold** a step and a NOTE EDIT page opens for it and
stays open: the knobs set its pitch, gate, velocity and ratchet, OCT-/OCT+ its octave, and the layer buttons that dice
the whole pattern on the main page toggle that step's on/off, accent, slide and chance. What you set is an *edit*: it
survives every dice and every morph, is marked on the lane, and one RESET touch puts it back. When the first 16 are
right, **PG+ twice** adds page 2 as an exact copy (silently: the music does not change), and you change the copy;
up to 4 pages = 64 steps, each channel its own length. Everything is a **page** with its own screen; every page names
what its knobs, buttons and keys do; **HOME** always comes back. No scenes, no tap tempo (tempo lives on SET), no
hold-while-turning gestures except the HOME-held HELP of owner decisions 57 / 61: a hold **opens** a page, a tap **acts**. Force sensing on the keybed adds accents by a
harder tap, a pressure-controlled ROLL and FILL, and velocity in REC; every force gesture has a plain fallback.

The framework rules, applied everywhere (each is checked by a table or a score row):

| # | Rule | Where it is enforced |
| --- | --- | --- |
| R-1 | Every state is a page (or a named overlay of one) with its own screen and a cue-strip line | page_states.tsv, section 2 |
| R-2 | Each button has exactly two modes, **tap** and **hold**; double-tap is not used anywhere | button_modes.tsv, checker rule 2 |
| R-3 | **Tap acts. Hold opens a page** (NOTE EDIT, SCALE, RESET, MENU) that stays open after release. The only holds that act while held are the three momentary moves SOLO, FILL, ROLL, HOME-held HELP (owner decisions 57 / 61) and the optional OCT row peek | section 3 |
| R-4 | Every knob is labelled on every page; a deliberately inert knob is drawn `-` | knob_map.tsv, checker rule 3 |
| R-5 | Every function is reachable in at most **2 gestures** from where the user is; no hidden modes | score table section 8, checker rule 7 |
| R-6 | Input, processing and output are traced for every function | section 5 |
| R-7 | **HOME tap** is the one escape (on PERF it only cancels a prompt; it never opens a page); EDIT tap is the one undo; the button that opened a page also closes it | button_modes.tsv |
| R-8 | **Every hold is named on the page where it starts**: the `h:` sub-label on the PERF button picture (`h:RST` EDIT, `h:HARM` GLO, `h:HLP/MENU` HOME, `h:UNDO L` SAVE, `h:peek` OCT), `hold=EDIT` in the lane header for the white keys, the black-key legend for SOLO / FILL / ROLL | section 2.1, 8.1 |

## 1. Model

### 1.1 Two layers per step

Each channel has a **pattern** of `LENGTH` steps (1..64, default 16). Every step position `p` (0..LENGTH-1) is
resolved, at the moment it starts, from two layers:

| Layer | What it is | Who changes it | Stored as |
| --- | --- | --- | --- |
| **Generated** | the algorithm's output at the step's *generator position* `g(p)` (1.4): hit, pitch, velocity, gate, ratchet, slide | knobs (morph), dice buttons, ALGORITHM, global SCALE / KEY / OCTAVE | seeds + morphs per algorithm (ADR-0003 `AlgoState`, unchanged) |
| **Edit** | per-step overrides; every field is either **AUTO** (follow the generated layer) or **set** | taps on the white keys, NOTE EDIT, REC, force accent | `StepEdit` 4 B per step, 64 per channel (section 9) |

Fields of an edit, each independently AUTO or set:

| Field | Set values | Resolved as |
| --- | --- | --- |
| `ON` | ON, OFF | AUTO: the generator's hit. A tap writes the opposite of what the step does now; a tap that makes it equal to the generator writes AUTO (so two taps never leave a stale edit) |
| `PITCH` | 0..96 semitones above `36 + KEY` (stored unsnapped) | snapped to the global scale at play time (SP6), so a SCALE change re-snaps reversibly; AUTO: the generated note |
| `OCT` | -2..+2 | added to the resolved pitch (AUTO pitch too: an octave edit on an AUTO pitch follows NOTES dice) |
| `GATE` | 10..100 % of a step, or TIE | multiplied by the global GATE like the generated gate (50 % neutral); TIE holds into the next sounding step (after the last step, across the loop end into step 1, like SLIDE; 9.2) |
| `VELO` | 1..127 | >= `ACC_THR` 100 is an accent on the TB-3 (shown `ACC`); threshold verified by ear 2026-11-11, decision 104 |
| `RATCH` | 1x..4x | repeats inside the step (SP5 spacing); AUTO: the generated repeats |
| `SLIDE` | ON, OFF | **this note glides into the next step** (step p+1; after the last step, step 1: TB-303 rule); if that step does not sound (OFF, chance drew silent) there is no glide and the note plays its own gate; the core turns it into the slide pairing of ADR-0003 7.3 (9.2) |
| `PROB` | 100 %, 50 % (`PROB_LOW`) | at 50 % the step plays on a deterministic draw per pass (9.3) |

`K1 = 0` (`KEYS ONLY`, decision 35) still works: the generator then has no hits and only steps whose `ON` is set play.

### 1.2 What re-dice, morph and algorithm changes do to edits (locked)

**The rule: a dice or a morph rewrites the generated layer only. A set field is never touched by anything except the
user editing it, RESET, UNDO, or a preset load. An AUTO field follows the new generated value at the next step.**

| Action | Generated layer | Set fields | AUTO fields | Example |
| --- | --- | --- | --- | --- |
| Dice NOTES (SCL) | new pitches | kept (an edited pitch stays) | follow | step 5 PITCH set to G: still G; step 6 AUTO gets a new note |
| Dice HITS (SEQ) | new hit placement | `ON` set stays ON / OFF | follow | a step forced OFF stays silent even if the new rhythm has a hit there |
| Dice ACCNT / ARTIC / SHAPE / ALL | new values | kept | follow | an edited SLIDE stays; generated slides move |
| Morph (any knob, any row) | recomputed | kept | follow | DENS down: forced-ON steps keep playing, AUTO steps thin out |
| ALGORITHM change | the new algorithm's layer | kept (edits belong to the channel, not the algorithm, like pins N2) | follow the new algorithm | a hand-made bassline on ACID stays when EUCLID drives the AUTO steps |
| Global SCALE / KEY / OCTAVE | re-snapped | set PITCH re-snapped (stored unsnapped) | follow | turning SCALE back restores every note exactly |
| LENGTH change | unchanged | kept, including on hidden pages | follow | 1.6 |
| Preset load | replaced | replaced (the preset carries the edits) | | SAVE hold undoes the load |

**How the user sees edits.** On the lane (the 16 cells of the viewed page, drawn left to right like the white keys):
a step with any set field carries a **corner tick** (top-right, 3 px); a set field adds its glyph inside the cell
(`!` accent, `~` slide drawn to the right edge toward the next cell, `x2..x4` ratchet, `%` chance, `-` tie, `^`/`v`
octave); a forced-OFF generated hit is drawn as a hollow cell with a slash. On NOTE EDIT every cell shows `AUTO` (dim,
the generated value under it) or the set value (bright, with a dot).

**How the user clears edits** (each is 2 gestures and one EDIT tap away from coming back):

| Clear | Gesture |
| --- | --- |
| One field of one step | NOTE EDIT open, hold EDIT (RESET), touch that field's control |
| Every field of one step | hold EDIT (RESET), tap the step's white key (from PERF); or in NOTE EDIT hold EDIT, tap FX |
| Every edit on the viewed page | hold EDIT (RESET), tap REC |
| Re-copy the page before onto the viewed page | hold EDIT (RESET), tap black D#4 (CHAN) |

### 1.3 Pages

- A channel has `LENGTH` steps, shown as `ceil(LENGTH / 16)` **pages** of 16 (page 1 = steps 1-16, page 2 = 17-32, ...;
  at most 4). The 16 white keys are **always the 16 steps of the viewed page**: key n = step `16 x (page - 1) + n`.
  Keys past `LENGTH` on the last page are drawn dim and tapping one shows `PAST THE END: CHAN K1 = LENGTH`.
- **Swap the viewed page:** black **C#5 `PG-`** and **D#5 `PG+`** (the printed `MONO POLY` pair, freed by removing
  scenes). They never wrap. Changing the view by hand sets `FOLLOW` off (toast `VIEW P2 . FOLLOW OFF`).
- **Add a page (the main extension gesture):** on the last page, `PG+` shows the ADD_PAGE prompt (a ghost page cell
  and `ADD PAGE 2 = COPY OF 1? PG+ = yes`); `PG+` again adds it. Any other control cancels the prompt and does its
  own job. The new page is an **exact copy of the page before it**: its edit layer is copied, and its generator
  source is the source of the page before (1.4), so the music **does not change** when a page is added, whatever the
  page before reads. The view moves to the new page, the lane shows it, and the user changes the copy. Page 3 copies
  page 2 (as edited), page 4 copies page 3, so A, A', A'' build up naturally. `LENGTH` becomes `16 x pages`.
- **The prompt names what PG+ will do** (three cases, nothing silent):

  | Last page is | Prompt (cue strip) | PG+ again |
  | --- | --- | --- |
  | full, and the next page never held content | `ADD PAGE 3 = COPY OF 2? PG+ = yes` | page 3 = copy of page 2 (edits and source), LENGTH 48 |
  | full, and the next page is hidden by a shrink | `SHOW PAGE 3 (HIDDEN)? PG+ = yes` | page 3 comes back as stored, LENGTH 48 |
  | partial (for example LENGTH 20: page 2 shows steps 17-20) | `SHOW ALL OF PAGE 2 (L32)? PG+ = yes` | LENGTH 32: page 2's stored steps 21-32 are shown (they were filled as a copy when page 2 was first entered, or hold the user's earlier edits); the next PG+ PG+ adds page 3 |

  A partial page is always completed before a new page is added, so PG+ never jumps LENGTH by an amount the user
  did not see. An odd length is set on CHAN K1.
- **At the maximum:** on page 4 with LENGTH 64, PG+ shows no prompt and changes nothing; it toasts `64 STEPS = MAX`
  (the page-map cells are all filled, so the limit is visible before the press). On a partial page 4 (LENGTH 49..63)
  the third prompt applies (`SHOW ALL OF PAGE 4 (L64)?`), and the press after that is the `64 STEPS = MAX` toast.
- **Per-channel length:** CHAN page K1 `LENGTH` 1..64 (step 1, fast 4), or OCT- / OCT+ on CHAN = to the page boundary
  below / above (20 -> 16 / 32; 32 -> 16 / 48; at 16 or less OCT- is inert with toast `ONE PAGE: K1 = SHORTER`).
  Each channel loops at its own length, so CH1 at 32 against CH2 at 12 is polymeter.

### 1.4 The generator position: page sources

The generated layer of step `p` is the algorithm's step at the **generator position** `g(p)`. Each page `k` (0-based)
stores a **source** `src(k)` in `0..k`: the **generator block** it reads (block `b` = generator positions
`16b..16b+15`). `src(k) = k` is OWN (the page reads its own block).

```
page k = p / 16 (0-based), i = p % 16
src(0) = 0                       (page 1 is always its own)
g(p)   = 16 * src(k) + i
generated(p) = step(cache, g(p))  // each algorithm reduces g by its own loop (ACID 16, EUCLID LEN, SNAKE lanes, STAGE TOTAL)
```

- **A new page inherits the source of the page before**: `src(k) = src(k-1)`. So the new page reads exactly the
  generator block the page before reads, and with the copied edits it is an exact duplicate, whatever that page's
  source was. Page 2 added after page 1 reads page 1 (`src = 0`, shown `P1`): "page 2 starts as a duplicate".
- **OWN** (`src(k) = k`) lets the algorithm continue past the page: STAGE with `TOTAL` 32 plays pulses 17-32 on an
  OWN page 2; a EUCLID `LEN` 12 or a SNAKE pitch lane of 7 keeps drifting across the page. Page 4 added after an OWN
  page 3 reads page 3's positions (`src = 2`, shown `P3`), so it duplicates page 3 exactly; set it to OWN to let the
  algorithm continue again. ACID's own loop is 16, so for ACID every source sounds the same (the CHAN cell shows the
  source and the toast says `ACID REPEATS EVERY 16`).
- **What K3 offers, and what its labels mean.** On the CHAN page, `K3 SOURCE` of the viewed page `k` offers, in this
  order: the **distinct blocks that the pages before it actually play now** (`src(j)` for `j < k`, duplicates removed), each
  labelled `Pm` by the **first** page `m` that plays it, then `OWN`. So `P2` is offered only when page 2 plays a block
  of its own (page 2 is OWN or reads a block page 1 does not), and it always means "the notes page 2 plays now". A page
  reading `Pm` therefore changes with every dice and morph exactly as page `m`'s generated layer does; the two pages
  differ only by their edits. Page 1 shows `OWN`, fixed (dim).
- **A block nobody before plays any more.** If an earlier page's source is changed later, a page that reads the block
  it used keeps reading it (nothing changes silently). Its K3 value is then shown by the block's positions,
  `17-32` (dim), with toast `P3 READS 17-32: NO PAGE PLAYS IT` (renamed from `PAGE 3 READS 17-32: NO PAGE BEFORE PLAYS IT`, which was 263 px against 240; P2-011, F-04); it stays in the list only while it is the
  current value (turning away drops it; `OWN` or a `Pm` replace it).
- Examples (STAGE `TOTAL` 48): pages 1 and 2 both read block 0 -> page 3 offers `P1`, `OWN`. Page 2 set to OWN ->
  page 3 offers `P1`, `P2` (block 1 = pulses 17-32), `OWN`. Page 3 OWN, page 4 added -> page 4 reads block 2,
  labelled `P3`.
- Two bits per page in one byte (`pg_src`, 9.1); re-copying the page before (RESET + `D#4`) copies its source too.
- **The channel loop resets the generator:** at `p = LENGTH - 1 -> 0` the generator position returns to `g(0) = 0`.
  Generation stays a pure function of (seeds, morphs, globals, sources, p), so the same settings play the same 64
  steps every pass (VISION rule 4). Consequence, stated plainly: SNAKE's long polymeter cycles (for example 112 steps)
  are cut to the channel length (at most 64); owner question 70.
- **Heard span cue (STAGE `TOTAL`, EUCLID `LEN`, SNAKE lanes):** the algorithm's own loop of `T` positions covers the
  blocks `0..ceil(T/16)-1` (at most 4). A position `q` of the loop (`q < T`) is **heard** when some *shown* step
  `p < LENGTH` has `g(p) = q`, so a partial last page counts only the steps it shows (`LENGTH 20` shows 4 steps of
  page 2, not 16); every other position is **unheard**, whether it lies above or between the heard ones. *Heard* = the
  number of heard positions (the count is per step, the toast span is the first run of unheard positions). When any block is unheard, the knob cell reads `TOTAL` over `48 (32)` (the bracket is the heard count: `48 (32 heard)` was 71 px in a 51 px cell; HELP says what it means) and the turn toasts
  the first unheard span, `17-32 NOT HEARD: D#4 K3 = OWN` (with two unheard blocks: `17-32 +1 NOT HEARD`). Examples:
  `TOTAL` 32 on one page -> `(16)`, `17-32 NOT HEARD`; `TOTAL` 48 with pages 1-2 on block 0 and page 3 OWN
  (block 2) -> `(32)`, `17-32 NOT HEARD` (the gap, not the top, is reported); `LENGTH 20` (page 2 OWN, 4 steps
  shown) with `TOTAL` 32 -> `TOTAL 32 (20)`, `21-32 NOT HEARD`. So the user is never left
  wondering why a turn changed little or where part of the loop went.

### 1.5 Playhead, view and FOLLOW

- **The playhead** of the selected channel is the inverted lane cell when it is on the viewed page; the **page map**
  in the header has one small cell per page (outlined = viewed, filled = playing, hollow = hidden page holding stored
  content after a shrink), and `>P3` appears at the lane's right edge when the playhead is on another page.
- **FOLLOW** (CHAN K4, default ON): while playing, the view turns to the page the playhead is on as it crosses a page
  boundary. `PG-` / `PG+` set FOLLOW OFF (the user wants to look somewhere); CHAN K4 sets it back ON (2 gestures).
- **While NOTE EDIT or REC is open the view never moves by itself** (the edited step stays in sight); on leaving, the
  FOLLOW setting applies again. FOLLOW is UI state (not in presets), one per device.
- The view is remembered per channel; SELECT shows the selected channel's view.

### 1.6 Length change rule (extend duplicates, shrink hides, nothing lost)

| Change | What happens |
| --- | --- |
| Extend onto a page that has never held content (or was cleared) | the whole page is filled as a copy of the page before it (edits and source copied), even if LENGTH shows only part of it |
| Extend onto a page that was hidden by a shrink | it comes back exactly as it was (its edits and its source) |
| Shrink | the playing loop ends at the new length at the next step; steps beyond keep their edits, hidden (page-map cell hollow), saved in presets |
| Shrink while the playhead is past the new end | the next step is step 1 (`p` wraps); a sounding note ends by its queued note-off |
| Discard hidden content | only by the explicit commit: extend over it, then RESET + `D#4` (re-copy the page before) or RESET + `REC` (clear the page). Nothing is ever lost by turning K1 or tapping OCT- |
| Undo | EDIT tap undoes the last page add, page re-copy or page clear (the undo record holds one page of edits) |

### 1.7 What the 27 keys are on each page

| Page | 16 white keys | 11 black keys |
| --- | --- | --- |
| PERF | steps of the viewed page: tap ON/OFF, hold = NOTE EDIT (named `hold=EDIT` in the lane header); force: hard tap = accent (F3-C5) | `MUTE1-4` `CHAN` `SOLO` `FILL` `ROLL` `PG-` `PG+` `RESTRT` |
| NOTE_EDIT | tap ON/OFF, hold = move the editor to that step | as PERF (`CHAN` switches page) |
| CHAN | as PERF | as PERF, `D#4` = BACK |
| SET | as PERF | as PERF, `D#4` switches to CHAN |
| SCALE (HARMONY) | white F3-E4 type chord degrees 1-7 (the KEY is K2 only); every other key inert | (inert) |
| REC | all 27 keys play and write notes (F3..G5 + octave); force = level SOFT / MED / HARD | (same) |
| RESET | tap = clear that step's edits | `D#4` = re-copy the page before; the rest inert |
| HELP (HOME held) | explain on the page beneath | explain |
| MENU | inert | inert |

## 2. State / page inventory

Nine pages and three overlays (page_states.tsv). Screen budget per page, as P1-014 fixed it (header 20, channel band
28, lane 60, band 52, button picture 52, black-key legend 12, cue strip 12 = 236 of 240 px); the "band" is the part
that changes per page. The button picture holds two lines per button row: the tap label (AF_M, 13 px) and the
sub-label (AF_S, 9 px), 2 x 22 = 44 of its 52 px; the sub-label is the algorithm's dice sub-label (ALGORITHM_SET 4.4)
for the six dice buttons and the hold role, prefixed `h:`, for the buttons that have one (R-8). Mock-ups are 40 columns of about 6 px; P2-003 owns pixels, these fix content and order.

### 2.1 PERF (home; power-on lands here)

```
+----------------------------------------+
|> 120 P01  CH1 ACID M1  [1][2]. .  L32  |  header: play state, clock dot + BPM (tempo source: INT, or EXT n, WAIT, 2.8), preset, channel, algorithm, MIDI, page map, LENGTH
| [1 ACID] 2 EUCL m  3 SNAK m  4 STAG m  |  channel band: selected inverted, m = muted, activity dot
| PG 1/2  FOLLOW  hold=EDIT        >P2   |  lane header: viewed page / pages, FOLLOW, the white-key hold, playhead page
| #  .  #' #  .  #! .  #~ #  .  #  .  #x2 . |  16 lane cells = white keys 1..16 (# on, . off, ' edited, ! ~ x2 glyphs)
|1|DENS 12|SLIDE 4|JUMP 5 |ACCNT 3|       |  knob band ROW 1 (live, filled)   K1..K4
|2|OFFS 0 |PEDAL 6|PUNCH40|DIR FWD|       |  knob band ROW 2 (dim)            S1..S4
| ALL  NOTES ACCNT SHAPE UNDO  SET       |  button picture, top row FX SCL ENV LFO EDIT GLO (tap label, AF_M)
|  -   notes ACCNT OCTAVE h:RST h:HARM |  line 2 (AF_S): dice sub-label of the algorithm (ACID: ALGORITHM_SET 4.4), or h: = the hold
| HOME SAVE  ARTIC HITS  PLAY  REC  R1 R2|  bottom row HOME SAVE ARP SEQ PLAY REC + OCT-/OCT+ cells
| h:HLP/MENU h:UNDO L GT+SLD HITS h:peek |  line 2: h:HLP/MENU, h:UNDO L, dice sub-labels (ACID: GT+SLD, HITS), OCT h:peek
| M1 M2 M3|M4 CHN|SOL FIL ROL|PG- PG+|RST |  black-key legend in the 3-2-3-2-1 grouping
| TAP STEP=ON/OFF  HOLD=EDIT NOTE         |  cue strip (priority table 11.2)
+----------------------------------------+
```

### 2.2 NOTE_EDIT (hold a step key 0.4 s; stays open)

```
+----------------------------------------+
|> 120 P01  CH1 ACID M1  [1][2]. .  L32  |
| NOTE EDIT  STEP 21  (page 2, key 5)    |  banner replaces the channel band
| #  .  #' #  [#]' .  #! .  #~ #  .  #   |  lane: the edited step boxed
| PITCH G2 *|GATE AUTO|VELO 120*|RATCH AUTO|  K1..K4: set values bright with *, AUTO dim (generated value under it)
|   (gen F2)| (gen 50%)| ACC      | (gen 1x) |
| ALL  NOTE  ACCENT PROB  UNDO  SET       |  FX SCL ENV LFO EDIT GLO for this step
| HOME SAVE  SLIDE ON/OFF PLAY REC OCT- OCT+|  HOME SAVE ARP SEQ PLAY REC, OCT-/OCT+ = step octave
| M1 M2 M3|M4 CHN|SOL FIL ROL|PG- PG+|RST |
| STEP 21: KNOBS EDIT  HOME=done          |
+----------------------------------------+
```

### 2.3 CHAN (tap black D#4)

```
+----------------------------------------+
|> 120 P01  CH1 ACID M1  [1][2]. .  L32  |
| CHANNEL 1   ACID   MIDI 1              |
| pages: [1 OWN][2 P1][3 hidden][  ]      |  page map, large: viewed outlined, playing filled
| LENGTH 32 |MIDI 1  |SOURCE P1 |FOLLOW ON |  K1..K4 (one row)
| -PAGE (OCT-)        +PAGE (OCT+)        |
| ALL NOTES ACCNT SHAPE UNDO SET          |  buttons keep PERF roles
| HOME SAVE ARTIC HITS PLAY REC           |
| M1 M2 M3|M4 BCK|SOL FIL ROL|PG- PG+|RST |  D#4 cell reads BACK
| CH1: K1 LENGTH  OCT=+-PAGE  D#4=back    |
+----------------------------------------+
```

### 2.4 SET (tap GLO) and HARMONY (hold GLO; table id SCALE)

```
+----------------------------------------+   +----------------------------------------+
|> 120 P01  CH1 ACID M1  [1][2]. .  L32  |   |> 120 P01  CH1 ACID M1  [1][2]. .  L32  |
| SET (all channels)                     |   | HARMONY  C MINPEN  OCT 0               |
| lane as PERF (steps stay live)         |   |  1   4   5   1    <- one cell per bar  |
| BPM 120 |SWING 50|GATE 50 |DIV 1/16    |   |  C   F   G   C    <- chord names       |
|  TEMPO INT/EXT n  OUT MASTER/FOLLOW    |   |  keys F3 G3 A3 B3 C4 D4 E4 = 1..7      |
|  tag OCT- / OCT+ = OCTAVE -/+          |   | SCALE MINPEN|KEY C|VOICE 2|MOVE 8    |
| ... buttons as PERF, GLO = BACK,       |   | DRONE OFF (OCT-/OCT+)  CH1 ROLE AUTO   |
|     hold GLO = HARM                    |   | PROG BARS ROLE NEW  (SCL LFO ARP SEQ)  |
| SET: GLO hold=HARM  HOME=back          |   | HARM: KEYS F3-E4 = CHORD 1-7           |
+----------------------------------------+   +----------------------------------------+
```

The HARMONY page is the SCALE page extended (HARMONY.md 5): scale and key stay on K1 / K2; the 27-key picture and "press a key = KEY"
are replaced by the progression strip and the chord keys; OCTAVE moved to OCT- / OCT+ on SET.

### 2.5 REC (tap REC)

```
+----------------------------------------+
|> 120 P01  CH1 ACID M1  [1][2]. .  L32  |
| REC  CH1  writing to page 2            |  red banner, REC LED lit
| #  .  #  #  ^  .  #  .  ...            |  ^ = write cursor (stopped) or the playhead (playing)
| CURSOR 5|ACC OFF |SLIDE OFF|OCT 0      |  one row; force on: last written level shown 'v MED 90'
| KEYS = NOTES (all 27)  OCT-/OCT+ = OCT |
| REC: PLAY KEYS TO WRITE  REC=done      |
+----------------------------------------+
```

### 2.6 RESET (hold EDIT 0.4 s; one shot) and HELP (hold HOME and touch a control; while held)

```
+----------------------------------------+   +----------------------------------------+
| RESET  touch what goes back to default |   | HELP (HOME held)  touch any control    |
| (the page beneath, every resettable    |   | (the page beneath dimmed)              |
|  cell tagged RST)                      |   | +------------------------------------+ |
| RST |RST |RST |RST   (knobs)           |   | | SEQ = HITS: re-roll where the      | |
| RST AL RST NT RST AC RST SH CANCEL -   |   | | notes fall. NOTE EDIT: step ON/OFF | |
| HOME -  RST AR RST HT PLAY RST PG      |   | +------------------------------------+ |
| white keys = CLR step, D#4 = COPY P    |   | PLAY still plays. let go = back        |
| RESET: TOUCH WHAT TO RESET  EDIT=no    |   | HELP: SEQ  let go = back               |
+----------------------------------------+   +----------------------------------------+
```

Before anything is touched, holding HOME shows only the strip `HOME: touch=HELP  let go=MENU` (from 0.25 s, so a
plain tap does not flash it). HELP exists only while HOME is down: releasing HOME after a touch returns to the page
it was held on, with nothing changed; releasing after 0.7 s with nothing touched opens MENU; releasing earlier with
nothing touched is a HOME tap. MENU is Felucca's menu (set-and-forget rows only, SYSTEM_MAP section 7), unchanged;
FIRST_RUN, ADD_PAGE and MOVE_HELD are overlays that only change the cue strip and one cell (page_states.tsv).

### 2.7 Transitions (state diagram) and the single escape

```
                 tap D#4 / tap D#4 (BACK)
        +------------------------------------> CHAN ---+
        |      tap GLO / tap GLO (BACK)                 |  GLO tap -> SET, REC tap -> REC,
        +------------------------------------> SET ----+  hold GLO -> HARMONY (table id SCALE; from any of these)
        |      hold GLO / tap GLO (BACK)                |
        +------------------------------------> SCALE --+
 boot-> PERF --hold a white key 0.4 s----------> NOTE_EDIT --hold another white key--> NOTE_EDIT (moved)
        |  ^   tap REC / tap REC (DONE)                 |  REC tap -> REC at this step
        |  +--- REC <-----------------------------------+  SELECT turn -> PERF
        |      hold EDIT 0.4 s (one reset, then back)
        +------------------------------------> RESET --> the page it came from
        |      hold HOME + touch a control (any page) / let go of HOME -> back to that page
        +------------------------------------> HELP (exists only while HOME is held)
        |      hold HOME 0.7 s touching nothing, release (from any page) / tap HOME
        +------------------------------------> MENU
   EVERY page --tap HOME--> PERF   (PERF: tap HOME only cancels a prompt; it never opens a page)
```

- **One escape:** HOME tap returns to PERF from every page (owner decision 61: tap = home). On PERF it cancels a
  pending prompt (ADD PAGE, a pending preset slot) or skips the first-run strip, and toasts `HOME  hold: HELP / MENU`;
  it never opens a page. HELP lives only while HOME is held (decision 57), so a reflexive double HOME cannot land in a
  page where nothing works: the first draft's dead end is gone.
- Opening a page from another latched page replaces it (one latched page at a time; NOTE_EDIT, CHAN, SET, SCALE switch
  freely). REC and RESET are exclusive: in REC, GLO, PRESETS and EDIT hold are inert and say why.
- Power-on always lands on PERF; REC, NOTE_EDIT, RESET, HELP are never restored.

### 2.8 Tempo source and clock out (added by FB-001 / P2-003, finalised by P2-010 to the owner rulings of 2026-11-10; adds, changes nothing above)

The header and the SET page always say where the tempo comes from and whether the FM-1 sends clock, and a **clock dot**
blinks on the beat. No new control and no new MENU row. The source is AUTO: the FM-1 runs on its own clock; a MIDI
clock it hears (USB or the TRS jack) becomes the tempo source; when that clock stops it returns to its own tempo, which
the host never overwrites. Pixels, strings and colours: [SCREEN_SPEC.md](SCREEN_SPEC.md) section 6; mechanisms, rules
and tests: [TIMING_SPEC.md](TIMING_SPEC.md).

| State | Condition | Header | Dot | Clock out |
| --- | --- | --- | --- | --- |
| INT | no host clock accepted (default) | `INT 120` | amber, solid, blinks on the internal beat | MASTER, while playing: 24 pulses per quarter plus Start / SPP / Continue / Stop over USB |
| EXT SYNC | a host clock accepted, first 7 pulses | `EXT --` | cyan with a centre hole, steady dim | none |
| EXT LIVE | a host clock accepted, tempo measured | `EXT 126` (the incoming BPM) | cyan with a centre hole, blinks on the host beat | none |
| WAIT | PLAY pressed while a host clock is accepted and the host has not started | `WAIT` | cyan ring, blinks at 2 Hz | none |

Timeout rule: 500 ms without a pulse (24 pulses at 120 BPM) returns the source to INT; the check also runs while
stopped. If it happens while playing the transport stops (as it does today). On SET, K1 BPM stays ignored while an
external clock runs, as locked in `knob_map.tsv` (the cell mirrors the incoming BPM); the user's own tempo is kept and
shows again when the host stops. PLAY while a host clock is accepted waits for the host's Start (cue strip
`WAIT FOR HOST START  PLAY=cancel`); it never falls back to the internal clock silently.

## 3. Button modes

Two modes per button (R-2): **tap** and **hold**. A button that has a hold role (EDIT, GLO, HOME, SAVE, OCT-, OCT+)
acts on **release** when tapped (HOLD threshold: menu setting, default 0.4 s; HOME and SAVE 0.7 s); every other
button acts on **press** and its hold does nothing more. Full per-page rows: [button_modes.tsv](button_modes.tsv)
(14 buttons x 9 pages x 2 modes, plus the hidden maintenance row both-OCT 5 s = UPDATE MODE).

| Button | PERF tap | PERF hold | NOTE_EDIT tap | CHAN / SET / SCALE | REC | RESET | Principle |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `FX` | dice ALL `ALL` | - | dice this step (pitch, accent, slide, gate) `ALL` | as PERF | as PERF | algorithm to factory / step to AUTO `RST AL` | printed FX = everything |
| `SCL` | dice NOTES `NOTES` | - | dice this step's pitch `NOTE` | as PERF; SCALE (HARMONY): dice the progression `PROG` | as PERF | NOTES seeds / PITCH AUTO `RST NT`; from HARMONY the progression to `1---` | scale = pitch layer; on HARMONY the scale-level layer |
| `ENV` | dice ACCNT `ACCNT` | - | accent ON/OFF `ACCENT` | as PERF | as PERF | ACCNT seeds / VELO AUTO `RST AC` | envelope = accent |
| `LFO` | dice SHAPE `SHAPE` | - | chance 100/50 % `PROB` | as PERF; SCALE (HARMONY): `BARS` 1..8 | as PERF | SHAPE seeds / PROB AUTO `RST SH` | modulation = shape, chance |
| `ARP` | dice ARTIC `ARTIC` | - | slide ON/OFF (into the next step) `SLIDE` | as PERF; SCALE (HARMONY): channel `ROLE` | as PERF | ARTIC seeds / SLIDE AUTO `RST AR` | arpeggiator = articulation |
| `SEQ` | dice HITS `HITS` | - | step ON/OFF `ON/OFF` | as PERF; SCALE (HARMONY): `NEW` progression | as PERF | HITS seeds / ON AUTO `RST HT` | sequencer = which steps |
| `EDIT` | UNDO `UNDO` (also the last HARMONY change when that came later) | RESET page `RESET` (sub-label `h:RST`) | undo the last step change (9.1: the visit snapshot, or the last other-key tap) | UNDO / RESET | undo last written note; hold inert | `CANCEL` | edit = take back; hold = reset |
| `GLO` | SET `SET` | HARMONY `HARM` (sub-label `h:HARM`) | SET / HARMONY (closes NOTE EDIT) | SET: tap `BACK`, hold HARM; HARMONY: `BACK` | inert | inert | global |
| `HOME` | home: cancel a prompt `HOME` | touch a control = HELP; release after 0.7 s untouched = MENU `HLP/MN` (sub-label `h:HLP/MENU`) | back to PERF `HOME` | back to PERF (hold as PERF) | leave REC | cancel, PERF | home; hold = menu (printed); help while held (decisions 57 / 61) |
| `SAVE` | save to the header slot, or to the pending slot `Pnn?` `SAVE` | undo last load (state and slot; next step) `UNDO L` (sub-label `h:UNDO L`) | as PERF | as PERF | as PERF | inert | printed |
| `PLAY` | start / stop `PLAY` | - | as PERF | as PERF | as PERF | as PERF | printed |
| `REC` | open REC `REC` | - | REC at this step | open REC | leave `DONE` | clear viewed page `RST PG` | printed: arm to record |
| `OCT-` / `OCT+` | latch ROW 1 / ROW 2 | peek the row while turning (`h:peek`) | step octave -1 / +1 `OCT-` `OCT+` | CHAN: to the page boundary below / above `-PAGE` `+PAGE`; SET: OCTAVE -/+1; SCALE (HARMONY): DRONE walk down / up | REC octave -/+1 | ROW reset / OCT AUTO | rows on PERF; printed octave elsewhere |

HELP (HOME held): every control explains its role on the page beneath and does nothing (PLAY still starts and stops). MENU: Felucca's rule (buttons close
the menu, OCT- / OCT+ = previous / next value).

**The one principle that makes NOTE_EDIT guessable:** a layer button acts on its layer. On PERF it re-rolls that
layer for the whole pattern; on NOTE_EDIT it sets that layer for the edited step (SEQ hits = on/off, ENV accent,
ARP articulation = slide, LFO movement = chance, SCL notes = a new note, FX all = everything on this step).

## 4. Knob map and key map

### 4.1 Knobs per page ([knob_map.tsv](knob_map.tsv))

| Page | K1 | K2 | K3 | K4 | Rows |
| --- | --- | --- | --- | --- | --- |
| PERF | live row, column 1 | column 2 | column 3 | column 4 | ROW 1 / ROW 2 by OCT- / OCT+ (4.2) |
| NOTE_EDIT | `PITCH` scale degree per detent, sounds | `GATE` 10..100 %, TIE | `VELO` 1..127 (`ACC` >= 100) | `RATCH` 1x..4x | one row; each starts at AUTO |
| CHAN | `LENGTH` 1..64 | `MIDI` 1..16, OFF (defaults CH1..CH4 = 1..4; **unique** across the four channels, 4.4) | `SOURCE` of the viewed page: the blocks earlier pages play, labelled `Pm` by the first page `m` that plays each, then `OWN` (1.4) | `FOLLOW` ON / OFF | one row |
| SET | `BPM` 40..240 | `SWING` 50..75 % | `GATE` 10..100 % (50 neutral) | `DIV` 1/32..1/4 | one row |
| SCALE (HARMONY) | `SCALE` 16 scales | `KEY` C..B | `VOICE` 0..6 (2) | `MOVE` 0..8 (8) | one row; OCT- / OCT+ = DRONE 0..14 |
| REC | `CURSOR` | `ACC` OFF / ON | `SLIDE` OFF / ON | `OCT` -2..+2 | one row |
| RESET | `RST` that value of the page beneath to default (NOTE_EDIT: to AUTO) | `RST` | `RST` | `RST` | |
| HELP (HOME held) | `?` explain | `?` | `?` | `?` | |
| MENU | value | value | value | value | |

Encoders on every page: `SELECT` = channel (closes NOTE_EDIT; in REC the cursor goes to step 1), `ALGORITHM` =
algorithm of the selected channel (edits kept), `PRESETS` = preset slot, which becomes **pending** (header `P05?`,
strip `P05: SAVE = save here`): stopped, it loads 1.5 s after the last detent (`SQ_LOAD_SETTLE`); playing, on the
**first 768-tick bar that starts at least `SQ_LOAD_SETTLE` after the last detent** (every detent moves the target on;
so the load lands between 1.5 s and 1.5 s + one bar after the user lets go: 1.5..3.5 s at 120 BPM); a SAVE tap at
any time while it is pending (at least 1.5 s in every case) saves the current state into P05 instead and cancels the
load; turning back to the current slot cancels; inert in REC and RESET. MENU: ALGORITHM = tab, PRESETS = row. MASTER = volume, always.
Knob value rule (FULL_CONTROL 5.1, unchanged): relative detents, slow = exact step, detents under 40 ms apart = the
fast step on ranges over 32 values (NOTE_EDIT `VELO` fast 8, CHAN `LENGTH` fast 4, SET `BPM` fast 5).

### 4.2 The PERF knob rows re-locked (3B: the most-performed morphs on ROW 1)

Owner decision 60: the four ROW 2 morphs performed most move up to ROW 1. Morph ids, ranges, defaults and formulas
are unchanged; only `AlgoDesc.knob_morph[8]` (slot -> morph id) changes, so golden files stay valid.

| Algorithm | ROW 1: K1 | K2 | K3 | K4 | ROW 2: S1 | S2 | S3 | S4 | Moved up (why) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ACID | `DENS` | **`SLIDE`** | `JUMP` | `ACCNT` | `OFFSET` | `PEDAL` | `PUNCH` | `DIR` | SLIDE: the acid move; PEDAL (how many steps sit on the root) is a dial-in |
| EUCLID | `HITS` | **`OFFSET`** | **`RATCH`** | `ACCNT` | `LEN` | `SPAN` | `PUNCH` | `CURVE` | rotation and ratchets are how Euclidean rhythms are played; LEN is now a dial-in under the channel LENGTH, SPAN a melodic dial-in |
| SNAKE | `DENS` | `LEN` | `RANGE` | `PATH` | `OFFSET` | `GLEN` | `DYN` | `DIR` | none: ROW 1 already holds its four performed morphs |
| STAGE | `DENS` | `TOTAL` | **`DIR`** | `RATCH` | `OFFSET` | `RDIV` | `DYN` | `RANGE` | direction flips are the ML-185's live move; RANGE is set once |

`knob_morph[]` (slot K1..K4, S1..S4 -> morph id of FULL_CONTROL 3): ACID `{0,5,2,3,4,1,6,7}`, EUCLID
`{0,4,5,3,1,2,6,7}`, SNAKE `{0,1,2,3,4,5,6,7}`, STAGE `{0,1,7,3,4,5,6,2}`. Four morphs move up in total
(ACID 1, EUCLID 2, STAGE 1). ROW 1 functions score 12 as performance; ROW 2 stays a setup dial-in at 10 (section 8).

### 4.3 Keys per page ([key_map.tsv](key_map.tsv), 27 keys x 9 pages)

Force column: **F** = the key senses force (F3..C5: white W01-W12 and black B01-B08), **-** = it does not
(D5 E5 F5 G5 white, C#5 D#5 F#5 black).

| Key (printed) | F | PERF | Force gesture (PERF) | Fallback |
| --- | --- | --- | --- | --- |
| W01..W12 F3..C5 | F | tap: step n ON/OFF; hold 0.4 s: NOTE_EDIT step n | a **hard tap** that turns a step ON writes ACCENT (VELO 120); the cell shows `ACC` while the key is down, before release | NOTE_EDIT `ENV` (ACCENT) or `K3 VELO` |
| W13..W16 D5..G5 | - | as W01..W12 | none | NOTE_EDIT `ENV` / `K3` |
| B01..B04 F#3 G#3 A#3 C#4 (OP1..OP4) | F | tap: MUTE CH1..CH4 | none (a mute is binary) | - |
| B05 D#4 (OP5) | F | tap: CHAN page | none | - |
| B06 F#4 (OP6) | F | hold: SOLO | none | - |
| B07 G#4 (PIT) | F | hold: FILL | **pressure** = density: light = halfway from K1 to maximum, firm = maximum (live readout `FILL 80%`) | fixed maximum |
| B08 A#4 (GLD) | F | hold: ROLL | **pressure bins** 2x / 3x / 4x (readout `ROLL 3x`) | fixed 2x |
| B09 C#5 (MONO) | - | tap: PG- | none | - |
| B10 D#5 (POLY) | - | tap: PG+ (last page: the prompt of 1.3: add a copy, show a hidden page, or complete a partial page; page 4 at LENGTH 64: toast `64 STEPS = MAX`, nothing changes) | none | - |
| B11 F#5 | - | tap: RESTRT (all to step 1 at the next step) | none | - |
| all 27 in REC | F on 20 | play and write | **landing force = written level** SOFT 64 / MED 90 / HARD 120 | velocity 90, or 120 with `K2 ACC` |

The other pages (section 1.7): NOTE_EDIT white keys tap ON/OFF and hold = move the editor (force accent as PERF);
HARMONY (table id SCALE): F3-E4 = chord 1-7, the rest inert; RESET white = clear step, D#4 = re-copy the page before; HELP explain; MENU inert.

**Controls freed and reassigned (owner requirement 4):** scenes A/B on C#5/D#5 -> `PG-`/`PG+`; tap tempo (it had
D#4) and P1-014's `PAGE` on D#4 -> `CHAN`; P1-014's GLO hold "SET while held" -> SCALE page; P1-014's "hold HOME and
touch = HELP" -> **kept** (owner decisions 57 / 61; the first draft's HOME-tap HELP page is reverted in
revision 1); P1-014's "hold a step + press another white key = pitch" -> removed (NOTE_EDIT `K1 PITCH` and REC); the step band's OCT row switch -> step
octave. Scene storage (about 2.2 KB pool) is returned.

### 4.4 MIDI channels are unique (owner ruling 2026-11-10, OWNER_QUESTIONS 77; P2-011)

No two of the four sequencer channels ever send on the same MIDI channel, so a clash is impossible rather than warned about.

| Item | Rule |
| --- | --- |
| Assigning | CHAN K2 shows the value live (the cell and the header `M n` follow the knob at once, dim until settled). The channel's MIDI channel is committed when K2 has been still for 0.6 s (`SQ_MIDI_SETTLE`) or when CHAN is left, so turning from 1 to 3 across 2 never touches the channel on 2. Only the settled value matters |
| Displacement | if another channel `o` already uses the settled value `m`, `o` is set to **OFF** (`SQ_MIDI_OFF`, ADR-0003): it keeps its algorithm, pattern, mute state and edits and **keeps running and showing its lane**, but sends no MIDI. Its notes already sounding end by their queued offs. OFF itself may be shared by any number of channels and is never displaced |
| Showing it | the displaced channel's Z1 cell line 2 reads `MIDI OFF` (`MUTE -` if it is also muted) and its header slot H4 reads `n ALGO MOFF`; the selected channel shows `OFF` in the K2 cell. The toast is `CH2 IS MIDI OFF: CH1 TAKES 2` (names the displaced channel, the taker and the number; 163 px; `CH4 IS MIDI OFF: CH3 TAKES 16` 170 px). When the user sets a channel to OFF themselves there is no toast |
| Undo | EDIT tap undoes the reassignment as ONE record (`UndoMidi`: the channel, its old MIDI channel, the displaced channel, its old MIDI channel, 4 B, ADR-0003); again = redo. It is the latest record until the next undoable action. Toast `UNDO: MIDI 2 BACK ON CH2` |
| Preview synth | amended by P2-024 (decision 110): amended by P2-025 (decision 111): OFF and mute close **only the MIDI gate**. The preview has its own gate: a channel is heard if its PREVIEW is ON (all four are OFF at power-on; hold a mute key 0.4 s to switch it, PREVIEW.md 1.4), and it plays the same events it would send as MIDI, whether or not its MIDI is open. A muted or MIDI OFF channel with PREVIEW on is a CUE (cell label `CUE n` / `CUE OFF`). SOLO acts on the MIDI gate only |
| Presets, autosave | `sq_preset_load` validates uniqueness: if a stored preset has two channels on one MIDI channel the lower-numbered channel keeps it and the others load as OFF. Defaults CH1..CH4 = 1..4 are distinct |
| Gestures and scores | unchanged: U40 stays 2 gestures (D#4, K2) and scores 10; the displacement is a side effect of the same turn, and its undo is the existing EDIT tap (1 gesture). No other function's gesture count changes, so no score is re-computed |

## 5. Input -> processing -> output traces

Every function of the score table (section 8) has one row. `ch` = selected channel; "next step" = the change is heard
when the channel's next step starts (ADR-0003 7.3; at most one step, 125 ms at 120 BPM and 1/16); "frame" = the
screen redraws within one main-loop frame (about 16 ms). MIDI is on the channel's MIDI channel unless OFF; the
each channel whose PREVIEW is ON plays the same notes on its own preview part (all OFF at power-on; ADR-0003 addendum 21). API names are ADR-0003 section 6 plus the additions of section 9.

| Id | Input | Processing | Output: MIDI and timing | Output: screen |
| --- | --- | --- | --- | --- |
| U01 | power on | `sq_boot()`: defaults (ADR-0003 12: CH1 ACID MIDI 1, CH2-CH4 MIDI 2-4 muted), LENGTH 16 x4, edits empty, every `pg_src` 0, FOLLOW on | nothing until PLAY | PERF, header `M1`, first-run strip `D#4 K2: MIDI CHANNEL` |
| U02 | SELECT turn | `sq_select(ch)` | none | channel cell inverted, lane / knobs / header show ch |
| U03 | ALGORITHM turn | `sq_set_algo(ch,a)` keeps `st[]` and edits; rebuild | new AUTO steps from the next step; no note cut (decision 47) | header algorithm name, knob labels, dice sub-labels |
| U04 | K1..K4 turn (ROW 1) | `sq_morph_turn(ch,knob,d)` via `knob_morph[]`, `sq_rebuild` | next step | cell value same frame; lane cells update |
| U05 | OCT- / OCT+ tap | UI row latch | none | gutter digit, filled row, OCT cell inverted, toast `ROW 2` |
| U06 | OCT+ tap, K turn | as U04 on ROW 2 | next step | as U04 |
| U07-U11 | SCL / SEQ / ENV / ARP / LFO press | `sq_dice_layer(ch,L)`: new seed, UndoRec | next step (AF_PAIRING hit/note one step later) | toast `NOTES` + sub-label, lane redraw, strip `EDIT takes it back` |
| U12 | FX press | `sq_dice_all(ch)` | next step | toast `ALL`, lane redraw |
| U13 | EDIT tap | `sq_undo(ch)` swaps UndoRec | next step | toast `UNDO` / `REDO` + what |
| U14 | white key tap | `sq_step_toggle(ch,p,vel)`: `ON` = opposite of now, AUTO if equal to the generator | stopped: the step sounds once now (note-on, off after its gate); playing: at its next pass | cell filled / hollow, corner tick if edited |
| U15 | hard tap (F3..C5) turning a step on (FORCE not OFF) | landing force >= the key's HARD bin -> `VELO` 120 written with `ON` | as U14, at velocity 120 (TB-3 accent) | `ACC` in the cell while the key is down, `!` glyph after; with FORCE on the playing strip alternates in `HARD TAP = ACCENT` |
| U16 | white key held 0.4 s | open NOTE_EDIT on `p`; snapshot the step into UndoRec (bit3) | the step sounds once (audition) | NOTE_EDIT band, step boxed, banner `STEP p` |
| U17 | NOTE_EDIT K1 | `sq_step_set(ch,p,PITCH, degree +- 1)` (first detent: AUTO -> set at the generated note) | every detent: note-on/off of the new pitch on the channel (audition, 120 ms); playing: also at the next pass | `PITCH G2*`, lane glyph |
| U18 | NOTE_EDIT OCT- / OCT+ | `OCT` -1 / +1 (clamp -2..+2) | audition | `OCT+1` on the OCT cell, `^` glyph |
| U19 | NOTE_EDIT K2 | `GATE` 10..100 %, TIE | next pass (tie: no note-off until the next step's start; 7.3 step 7) | `GATE 70%*` / `TIE`, `-` glyph |
| U20 | NOTE_EDIT K3 | `VELO` | next pass | `VELO 96*` / `ACC` |
| U21 | NOTE_EDIT K4 | `RATCH` 1..4 | next pass: up to 4 note-on/off pairs inside the step (SP5) | `RATCH 3x*`, `x3` glyph |
| U22 | NOTE_EDIT ARP | `SLIDE` toggle | next pass: if the next step (step 1 after the last) sounds, this note's off moves to `S1 + D` and the next note starts before it (glide), across the loop end too; cut only by STOP or a LENGTH change inside the step | `SLIDE ON`, `~` glyph to the next cell (on the last step the glyph points off the lane edge and step 1 shows a `~` lead-in) |
| U23 | NOTE_EDIT ENV | `VELO` 120 / 80-or-AUTO | next pass | `ACCENT` cell lit, `!` |
| U24 | NOTE_EDIT LFO | `PROB` 100 <-> 50 % | next pass: plays when the per-pass draw says so (9.3) | `PROB 50`, `%` |
| U25 | NOTE_EDIT SEQ | `ON` toggle (as U14 on the edited step) | next pass | cell filled / hollow |
| U26 | NOTE_EDIT FX / SCL | entropy -> set PITCH (+ accent, slide, gate for FX), stored | audition, then next pass | the new values with `*` |
| U27 | NOTE_EDIT, other white key held | move the editor to that step (new UndoRec snapshot; the last change wins, 9.1) | audition | box moves, banner |
| U28 | NOTE_EDIT open while playing, K turn | as U17-U21, heard every pass | next pass | live values |
| U29 | EDIT hold, white key | `sq_step_clear(ch,p)` -> all AUTO; UndoRec bit3 | next pass | tick gone, toast `STEP 5 CLEARED  EDIT=undo` |
| U30 | NOTE_EDIT: EDIT hold, a control | that field -> AUTO | next pass | cell reads `AUTO` |
| U31 | EDIT hold, REC | `sq_page_clear(ch,page)`; UndoRec holds the 16 records | next pass | every tick on the page gone |
| U32 | EDIT hold, D#4 | `sq_page_copy(ch,page-1 -> page)` (page 1: clear) | next pass | page redrawn |
| U33 | (look) | - | - | corner ticks and glyphs (1.2) |
| U34 | PG- / PG+ tap | `view_page[ch]` +-1, FOLLOW off | none | lane shows the page, `PG 2/4`, toast `FOLLOW OFF` |
| U35 | PG+ on the last page, PG+ | `sq_page_add(ch)`: partial last page -> LENGTH to its end; else LENGTH 64 -> nothing (toast `64 STEPS = MAX`, no prompt); else a hidden next page -> shown as stored; else copy the page before (16 edits + `src`), LENGTH = 16 x pages | none audible (a copy reads the same generator block with the same edits) | the prompt of 1.3, ghost cell, then the page in view, `L48` |
| U36 | D#4, K1 | `sq_set_length(ch,L)` | the loop ends at L from the next step | `LENGTH 24`, page map |
| U37 | D#4, K3 | the UI steps through the offered list of 1.4 (distinct `src(j)`, `j < page`, labelled by the first page that plays each; then OWN; plus the current value while no earlier page plays it); `sq_set_source(ch,page,src)` writes the 2-bit `pg_src` field of the viewed page | from the next pass of that page | `SOURCE P1` / `SOURCE P2` / `SOURCE OWN` / dim `SOURCE 17-32`; heard-span cue on the algorithm's loop knob (1.4); ACID toast `ACID REPEATS EVERY 16` |
| U38 | D#4, K4 | UI FOLLOW | none | `FOLLOW ON` |
| U39 | (look) | - | - | header page map, `PG n/m`, `>P3`, `L32` |
| U40 | D#4, K2 | `sq_set_midi(ch,m)` on the settled value (4.4): if another channel `o` holds `m`, `o` is set to OFF in the same call and ONE `UndoMidi` record keeps both old values | notes after it go to the new channel; sounding notes end on the old one (decision 47); the displaced channel stops sending at once, its sounding notes end by their queued offs | `MIDI 2`, header `M2`; the displaced channel's cell `MIDI OFF`; toast `CH2 IS MIDI OFF: CH1 TAKES 2` |
| U41 | GLO, K1 | `song.g[G_BPM]` | `beat_samples()` changes at once | `BPM 128`, header |
| U42-U44 | GLO, K2 / K3 / K4 | `sq_global_set()` (swing / gate / div) | next step | the cell |
| U45 | GLO hold, K1 | SCALE, `sq_global_set()` | next step: every pitch re-snapped | `SCALE DORIAN`, scale tones lit |
| U46 | GLO hold, any key | KEY = pitch class | next step; the new root sounds once | `KEY F` |
| U47 | GLO hold, OCT- / OCT+ or K3 | OCTAVE | next step | `OCTAVE +1` |
| U48 | black F#3..C#4 tap | `sq_set_mute` | the MIDI gate in `sq_emit` closes at once (the channel keeps computing its steps and feeds its preview; PREVIEW.md 1.2); sounding MIDI notes end by their queued offs | channel cell `m`, legend dim |
| U49 | hold F#4 | `sq_perf(SOLO)` | while held only ch sends MIDI (previews keep sounding) | SOL cell inverted, strip `SOLO CH1: let go` |
| U50 | hold G#4 (+ pressure) | FILL: K1 morph raised by pressure, restored on release | next step | `FILL 80%` live |
| U51 | hold A#4 (+ pressure) | `sq_perf(ROLL)`, repeats = pressure bin | next step: 2-4 triggers per step | `ROLL 3x` live |
| U52 | F#5 tap | `sq_restart()` | all channels to step 1 at the next step | playheads to 1 |
| U53 | PLAY | `sq_req` START / STOP | start: tick 0 at once; stop: every note-off, then CC123 | PLAY LED, header |
| U54 | REC tap, keys | `sq_rec_write(ch,key,vel)`: stopped at the cursor (advances), playing at the nearest step; writes PITCH, ON, VELO (force), SLIDE / ACC from K2 / K3 | the key sounds at once (note-on at landing + ~5 ms, off on release) | written cell, cursor, last velocity |
| U55 | REC or HOME tap in REC | disarm | none | banner gone |
| U56 | SAVE tap | `sq_preset_save` into the header slot Pnn (E7 store) | none | toast `SAVED P03` |
| U57, U68 | PRESETS turn | slot pending (`sq_ui.pend`, deadline = last detent + `SQ_LOAD_SETTLE` 1.5 s, moved on by every detent); stopped: at the deadline, load; playing: at the deadline, stage the slot and queue it to the first 768-tick bar that starts at or after the deadline (only the last slot turned lands); turning back to the header slot cancels | stopped: 1.5 s after the last detent; playing: on the first bar starting >= 1.5 s after the last detent (1.5 s .. 1.5 s + one bar) | header `P05?` at once (settle), then `P05>` with bar countdown dots (playing, once the bar is fixed), then `P05`; strip `P05: SAVE = save here` until the load lands |
| U58 | SAVE hold 0.7 s | restore the pre-load copy and the previous header slot at the next step (again = redo); while a slot is pending: cancel it | next step | toast `LOAD UNDONE: P03`; the redo says `LOAD REDONE: Pnn` (the slot loaded again; P2-011, F-16) |
| U69 | PRESETS turn, SAVE tap while pending (stopped or playing) | `sq_preset_save` into the pending slot; the pending load (settling, or staged for its bar) is cancelled; the header slot becomes it. The window is at least `SQ_LOAD_SETTLE` 1.5 s in both cases (playing: up to 1.5 s + one bar), so a SAVE tap never races the bar | none (the music does not change) | toast `SAVED P05`, header `P05` |
| U70 | Chord voicing (VOICE) | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, K3 (label `VOICE`, HARMONY.md 3) |
| U71 | Voice-leading distance (MOVE) | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, K4 (label `MOVE`) |
| U72 | Pedal tone: walk the candidates | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, OCT- / OCT+ (status line names the degree and marks clashes) |
| U73 | Dice the progression | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, SCL (`PROG`) |
| U74 | Type a progression (1-3-5-3) | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, keys F3-E4 (like U54: one open, then the keys; digits drawn on the strip) |
| U75 | Phrase length in bars | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, LFO (`BARS`) |
| U76 | Channel ROLE (OFF AUTO ROOT FULL) | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, ARP (`ROLE`); SELECT picks the channel |
| U77 | Perform VOICE, MOVE, DRONE with HARMONY open | P | 1 | 3/3/3/2 = 11 | pass | K3, K4, OCT- / OCT+ with the page open (same acts as U70-U72; the lane stays live) |
| U78 | Perform the progression dice with HARMONY open | P | 1 | 3/3/3/2 = 11 | pass | SCL (`PROG`) with the page open (same act as U73) |
| U79 | Know the chord that is playing | S | 0 | 3/3/3/3 = 12 | pass | chord chip in the PERF lane header (shown only while HARMONY is active) and the strip on HARMONY |
| U59 | EDIT hold, knob (PERF) | `sq_reset_morph` | next step | `RST` then the default value |
| U60 | EDIT hold, OCT- / OCT+ | `sq_reset_row` | next step | row values |
| U61 | EDIT hold, FX / dice | `sq_reset_algo` / `sq_reset_layer` | next step | toast |
| U62 | HOME held; touch or turn a control | HELP while HOME is down; table lookup of the control's help lines on the page beneath in [help_text.tsv](help_text.tsv) (control x page, two lines of at most 34 characters, 343 rows, checked by `tests/design/check_help.py`; the cue strip shows line 1, the help box both lines; P2-011, F-06); the control does not act; release -> back | none (PLAY still plays) | strip `HOME: touch=HELP  let go=MENU` from 0.25 s, then the help box |
| U63 | HOME hold 0.7 s, nothing touched | MENU on release | none | menu |
| U64 | HOME tap | back to PERF (on PERF: cancel a prompt) | none | PERF; on PERF toast `HOME  hold: HELP / MENU` |
| U65 | first power-on | strip steps 11.3 (step 1 `D#4 K2: MIDI CHANNEL` is a prompt for U40, not a new gesture); bit in `ui_prefs2` | - | strip text |
| U66 | NOTE_EDIT K1 / REC keys | audition path | each detent / key sounds | value |
| U67 | REC key with force | level = calibrated landing force snapped to SOFT 64 / MED 90 / HARD 120; with `REC_AT` ON, pressing on sends channel pressure (FS5) | written note at that level; HARD is an accent | `v MED 90` |

## 6. Force sensing (Velo / CTL-1, in v1 with fallbacks)

Source facts: [KEYBED_EXPRESSION_REFERENCE.md](KEYBED_EXPRESSION_REFERENCE.md) (Velo `fm1-velocity` and CTL-1
`fm1-ctl`, GPL-3.0 / MPL-2.0; the code is ported, never run from a download). Velocity is usable but noisy (a chord's
fingers read 59/76/127/104); aftertouch has a small range; only F3..C5 (20 keys) sense force; a lit LED changes the
reading of the keys beside it; the ADC is shared with the MASTER pot. The design therefore uses force only where a
**coarse** reading is enough and the result is visible before it counts.

### 6.1 Gestures

| # | Where | Gesture | Reading used | Bins / mapping (table data) | Fallback (always available) |
| --- | --- | --- | --- | --- | --- |
| FS1 | PERF, NOTE_EDIT: white F3..C5 | **hard tap** turning a step ON = accent | landing velocity, 2 bins | `HARD` = the top quarter of that key's calibrated range -> `VELO` 120; else no VELO edit | NOTE_EDIT `ENV` / `K3 VELO`; keys D5..G5 always use it |
| FS2 | PERF: black A#4 ROLL held | **lean** = more repeats | aftertouch in **DRIVE** mode (6.4), 3 bins with 10 % hysteresis | light 2x, medium 3x, firm 4x | 2x fixed |
| FS3 | PERF: black G#4 FILL held | **lean** = denser | aftertouch in **DRIVE** mode (6.4), continuous, 30 ms smoothing | K1 + (max - K1) x pressure, light = halfway | maximum density |
| FS4 | REC: F3..C5 (white and black) | **play harder = a higher level** (P2-015) | landing force, 3 bins | SOFT 64 / MED 90 / HARD 120, written as `VELO`; HARD is an accent | velocity 90, or 120 with `K2 ACC` ON |
| FS5 | REC: F3..C5, only with table flag `REC_AT` (default OFF, no menu row) | **keep pressing a held key = swell** (poly synths) | aftertouch in **DRIVE** mode | channel pressure `0xD0 p`, sent on a change of >= 8, at most one per 20 ms, never written to steps | flag OFF: no aftertouch |

"Lean harder" needs Velo's **DRIVE** sensing mode: in the plain pull-up mode aftertouch only reads easing off from
the landing floor, over a few counts to tens (KEYBED_EXPRESSION_REFERENCE 2), which cannot carry three bins. DRIVE
drives the row high through a weak pin driver on every other scan sample, so the pad reads the pill like an ohmmeter
and pressing past the landing reading is measurable. If DRIVE fails the gate (G3, G5 or G6), the scan stays in
pull-up mode: FS1 and FS4 still work from the landing reading and FS2 / FS3 use their fixed fallbacks.

The musical case for each gesture (sound, exact MIDI, TB-3 facts with sources, fallback, hardware tests H0-H5) is
[KEYBED_USE_CASES.md](KEYBED_USE_CASES.md) (P2-015); FS1 = UC1, FS3 = UC2, FS2 = UC3, FS4 = UC4, FS5 = UC5. FS2 repeats:
the first hit keeps the step velocity, the repeats play at `min(step velocity, 90)`.

Not used, with reasons (reconsidered in KEYBED_USE_CASES 3): per-step ratchet / slide / chance amounts from force
(aftertouch range too small to land a chosen value repeatably; the NOTE_EDIT knobs are exact); pressure as a CC such as
the TB-3 cutoff (an absolute CC cannot be restored on release, and no force-capable black key is free); pressure to a
morph or to a HARMONY dial (too few bins); release velocity (nothing uses it). Channel aftertouch is FS5 (REC only, OFF).

Every force gesture is **visible before it counts**: FS1 shows `ACC` in the pressed cell while the key is down (the
bin is fixed at landing, so lightening does not remove it; the tap acts on release, so keeping the key down past 0.4 s
opens NOTE EDIT instead of toggling, and EDIT tap takes a toggle back); FS2 / FS3 read out `ROLL 3x` / `FILL 80%` live; FS4 shows the
written level (`v MED 90`); FS5 shows `AT 64` live.

### 6.2 Thresholds and calibration

- Per key `floor` (the fully seated reading: the lowest count, i.e. the hardest press; lower counts = harder) is learned automatically: a running minimum of the seated reading,
  decayed 1 count per second toward 1023, stored with the settings (20 x 2 B). A key never pressed uses the
  reference default (hard ~185, gentle 270..360 counts; Velo notes).
- `HARD` bin edge per key = floor + 25 % of (gentle default - floor). `FORCE` menu row (set-and-forget, SYSTEM tab):
  `OFF / LIGHT / NORMAL / FIRM` shifts every bin edge by -15 %, 0, +15 % (OFF = every fallback). Default: `NORMAL` once
  the hardware gate (6.6) passes; `OFF` until then.
- Menu row `KEYS CAL` (optional): the screen asks for 2 soft and 2 hard presses on each force key; it overwrites the
  learned floors. Never needed for the fallbacks.

### 6.3 LED disturbance (step keys are lit)

The sequencer lights the LEDs of ON steps (and the playhead) on the white keys, so the reading of a pressed key can be
skewed by its lit neighbours. Three measures, in order (the gate result (6.6) selects which of them ship):

1. **H1, blank while sampling (CTL-1's method):** the scan ISR switches off the LEDs of the column it is sampling for
   the ADC window (one 100 us scan tick per 1.1 ms frame, about 9 % duty). Visible effect: none expected (the LEDs are
   already multiplexed); measured in the gate.
2. **H2, blank the neighbours while a force key is down:** from key-down to release, the LEDs of the keys either side
   are off. The screen lane still shows the steps. Cost: a short flicker of two LEDs under the finger.
3. **H3, compensation:** per key, the reading offset with each neighbour lit is learned in `KEYS CAL` and subtracted
   (2 x 20 B = 40 B, counted in 6.4 and 9.4). Used only if H1 and H2 together fail the gate.

### 6.4 Costs (numbers)

| Resource | Cost | Basis |
| --- | --- | --- |
| RAM (.bss) | 242 B: 20 keys x 8 B state (floor, peak, current / drive reading, bin) + 20 x 2 B floors + 20 x 2 B H3 neighbour offsets + MASTER cache 2 B | Velo `sense_key_t` is ~60 B x 27; only the fields the four gestures need are kept; DRIVE needs no extra state (the drive reading replaces `current` on its samples) |
| Sensing mode | **DRIVE** (Velo) whenever `FORCE` is not OFF: every other scan sample of a key is a drive sample, so each key gets a pull-up reading and a drive reading every ~4.4 ms instead of one reading every ~2.2 ms | needed by FS2 / FS3; Velo README, sense.c |
| Flash | about 2.5 KB (scan hook, bins, calibration UI strings) | Velo `sense.c` size class |
| CPU, scan ISR | one ADC start + read per 100 us tick at clock divider 7: estimate 1-2 % of the CPU; the gate measures it (limit 3 %) | Velo notes (no published figure; KEYBED_EXPRESSION_REFERENCE 2) |
| CPU, audio ISR | 0: force is read by the scan ISR and consumed in the main loop | |
| ADC sharing | the scan ISR owns the ADC; MASTER is converted in 1 of every 16 odd frames (about every 35 ms, 28 Hz) into a cached value that `master.c` reads instead of the blocking `fm1_adc_read` | Velo hands the OCT- odd-frame slot to the pot (reference 2) |
| Latency | REC note: landing + 3-5 ms, **plus about 2.2 ms in DRIVE** (so up to about 7.2 ms), and up to about 4 ms added to a release; FS1 acts on release (no added latency); FS2/FS3 30 ms smoothing | Velo notes: "drive mode adds ~2.2 ms to the note and up to ~4 ms to release" |
| Pool | 0 | |

### 6.5 Risks

| Risk | Effect | Mitigation |
| --- | --- | --- |
| Uneven readings across keys / edge presses read soft | wrong accent bin | 2 bins only; `ACC` preview before release; fallback |
| Digital input buffers off breaks the scan (OCT-/OCT+ read as held -> UPDATE MODE) | device enters update mode | never switch the row buffers off (Velo note); gate G3 runs 10 minutes watching for a false OCT hold |
| ADC conflict with the MASTER pot or the BLE radio [I] | zipper volume or stuck readings | ISR-owned ADC schedule; BLE stays off in v1 (to be confirmed in the gate) |
| CPU | audio dropouts | gate G4; if over, sample every other frame (5 ms resolution) |
| Owner dislikes the feel | | `FORCE OFF` restores the plain instrument; nothing depends on force |

### 6.6 The hardware-test gate (task HT-FORCE, before `FORCE` defaults to NORMAL)

Run on the owner's FM-1 with a debug readout page (built for the test, removed after):

| Gate | Pass criterion |
| --- | --- |
| G1 accent bins | 20 soft + 20 hard taps on each of W01..W12 with the PERF LED pattern lit: >= 90 % correct per key, >= 95 % overall |
| G2 LED disturbance | with H1 (and H2 if needed) a lit neighbour moves the reading by < 10 % of the soft-hard gap |
| G3 scan integrity | 10 minutes of play: no false OCT- / OCT+ hold, no stuck key, MASTER tracks within 50 ms without zipper |
| G4 CPU | scan ISR load +3 % at most; audio `felucca_dbg.max_us` within +5 % |
| G5 latency | in DRIVE mode: REC note-on within 8 ms of key-down and note-off within 6 ms of release (scope or USB-MIDI timestamp); pull-up mode: within 6 ms |
| G6 pressure | in DRIVE mode: ROLL 2x / 3x / 4x reached on purpose >= 80 % of tries; FILL halfway / maximum distinguishable |
| G7 REC levels | 16 notes (4 soft, 8 medium, 4 hard) on each of 6 force keys: >= 85 % land in the intended level (H4) |
| H0 TB-3 accent | verified by ear 2026-11-11 (decision 104): 80, 90, 95 do not accent, 100 and above do; threshold = 100 (`ACC_THR` 100). Flat non-accent velocity is 90 |

Fail of G1 or G2: FS1 and FS4 stay off (`FORCE OFF`), FS2 / FS3 may ship if G6 passes. Fail of G5 or G6 in DRIVE
mode: the scan stays in pull-up mode, FS2 / FS3 use their fallbacks, FS1 / FS4 are re-run on G1 / G5 in pull-up mode.
Fail of G3 or G4: no force at all in v1. The owner signs the gate in the task log.

## 7. Workflows (gesture by gesture)

Power-on state (ADR-0003 12): CH1 ACID **MIDI 1** unmuted; CH2 EUCLID MIDI 2, CH3 SNAKE MIDI 3, CH4 STAGE MIDI 4,
all muted; ROW 1; LENGTH 16; FOLLOW on. The default MIDI channel of a channel is its number (owner ruling 73), so the
channels line up with the numbers on the screen; a TB-3 at its factory channel 2 needs one visible step, which the
first run prompts for. (The T-1's six tracks are gone: v1 has four channels, decision 31.)
Changing a channel's MIDI channel is 2 gestures from PERF: tap `D#4` (CHAN), turn `K2`. In the reference workflow CH1 goes to 2 and CH2 (default 2, muted) becomes OFF, with a toast that says so (4.4).

**W1. First run to a playing TB-3 bassline (3 gestures to sound the first time: `D#4`, `K2`, `PLAY`; every function at most 2).**
1. first-run strip `D#4 K2: MIDI CHANNEL`: tap `D#4` (CHAN), turn `K2` to 2 (the TB-3's factory channel; header `M2`), the strip moves on to `PRESS PLAY` as soon as K2 turns; tap `HOME` to return to PERF, where the strip shows it. (A synth already on channel 1 skips this: PLAY also advances the strip.)
1b. press `PLAY`: the ACID line plays on the TB-3 (first-run strip moves on to `TURN K1 TO MORPH IT`).
2. turn `K2` (SLIDE) and `K1` (DENS) to taste; press `SCL` until the notes please; `EDIT` takes a roll back.
3. a different synth channel later: tap black `D#4` (CHAN), turn `K2` to its channel, tap `HOME`.

**W2. Dial in an algorithm (one gesture per function).** turn `ALGORITHM` to EUCLID -> `K1 HITS` 5 -> `K2 OFFSET`
to rotate -> `SEQ` (dice HITS = new rotation) -> `ENV` (new accents) -> `OCT+` then `S1 LEN` 12 for a 12-step cycle
-> `OCT-` back.

**W3. Extend 16 to 64 steps and vary the copies.**
1. with page 1 right: `PG+` (prompt `ADD PAGE 2 = COPY OF 1? PG+ = yes`), `PG+`: page 2 exists, in view, sounds
   identical.
2. tap white keys 13 and 15 (steps 29, 31) ON; tap key 4 OFF: page 2 now differs.
3. hold key 16 (step 32): NOTE_EDIT; `OCT+` (an octave up), `ENV` (accent), `K4` to 2x; `HOME`.
4. `PG+`, `PG+`: page 3 = copy of page 2 (with its changes; it reads page 1's generator block, like page 2);
   `D#4`, `K3` to `OWN` (K3 offers only `P1` and `OWN` here, because pages 1 and 2 both play block 0; the algorithm
   continues on page 3: on STAGE with `TOTAL` 48 it plays pulses 33-48, and the STAGE `TOTAL` cell now reads
   `TOTAL 48 (32)` with toast `17-32 NOT HEARD`, because no page plays pulses 17-32; on ACID the toast says
   `ACID REPEATS EVERY 16`). To hear all 48 pulses in order: `PG-` (page 2, still on CHAN), `K3` to `OWN`; the cue
   goes away. `HOME`.
5. `PG+`, `PG+`: page 4 = an exact copy of page 3: its edits, and its source `P3` (it reads page 3's generator
   positions, so it sounds exactly like page 3); vary it the same way, or set it `OWN` too. LENGTH is 64, the header
   map shows 4 pages, FOLLOW shows each page as it plays.

**W4. Edit one note on NOTE_EDIT.** hold key 5 (0.4 s) -> `K1` two detents up (each sounds) -> `K2` to TIE ->
`ARP` (slide on) -> `LFO` (50 % chance) -> `EDIT` tap if wrong (the step returns to how it was when the page opened)
-> `HOME`.

**W5. Mute and solo.** tap `F#3` (MUTE1) / tap again; hold `F#4` (SOLO) for a bar, release.

**W6. Change tempo.** tap `GLO` -> turn `K1` (BPM) -> tap `GLO` (or `HOME`).

**W7. Save and load.**
- Save over the current slot: tap `SAVE` (toast `SAVED P03`).
- Save into another slot (keep P03, store the work as P05): turn `PRESETS` to P05 (header `P05?`, strip
  `P05: SAVE = save here`) -> tap `SAVE` while it is pending (at least 1.5 s after the last detent in every case;
  playing, the load waits for the first bar that starts 1.5 s or more after the last detent): toast `SAVED P05`,
  nothing is loaded, the header reads `P05`. 2 gestures, never a race with the bar.
- Load: turn `PRESETS` to P04 and let go (stopped: loads after 1.5 s; playing: lands on the first bar starting at
  least 1.5 s later, header `P04?` then `P04>` with the bar countdown) ->
  wrong one: hold `SAVE` 0.7 s (the previous state and the header slot P03 return at the next step).

**W8. Clear and recover.** hold `EDIT` -> tap white key 5 (step 5 back to the generator) -> `EDIT` tap brings it back.

## 8. Scores

### 8.1 Rubric and the high bar

V (visual), I (input), O (output) are UX_REVIEW 3.1; L (learnable without a manual) is PERFORMANCE_DESIGN 7.1. Input:
one direct control 3; two actions, or a hold that opens a page, 2; modifier + control (hold while touching or turning
another control) 1. **High bar (this task): performance (P) >= 11/12, setup (S) >= 10/12, no sub-score below 2, at
most 2 gestures** (column G, counted from the page the user is on: PERF unless the function lives on a page, then
"open it" counts). `tools/check_sequencer_ux.py` re-adds every row and enforces the bar; a row that fails must be a
listed exception (8.3).

**The L rule, restated openly (revision 1; owner question 72).** PERFORMANCE_DESIGN 7.1 says: L3 only when a printed
word already means the job, or the lane read against the white keys; a fair fit or a black key is L2; **"a role found
only by a hold or a combo is L1"**; the cue strip never lifts L. The first draft of this document scored hold roles L2
without saying that it had changed the rule. That was not honest, and this revision states the change:

> **Restated rule.** A role reached through a hold is **L2** when (a) the hold is **named permanently on the page
> where it starts** (R-8: the `h:` sub-label on the PERF button picture, `hold=EDIT` in the lane header, the black-key
> legend for SOLO / FILL / ROLL) **and** (b) the hold opens a page that labels every control it changes (NOTE EDIT,
> SCALE, RESET), or, for the one combo, the strip names it the moment the hold starts (`HOME: touch=HELP  let go=MENU`).
> A hold that is not named on its starting page stays **L1**, as in P1-014. Everything else in 7.1 is unchanged; the
> cue strip still never lifts L.

The argument: P1-014's L1 is defined as "guessable only from other controls (the role is hidden until a hold or a
combo is found)". With the sub-labels the role is no longer hidden: it is written beside the control on the home
screen, which is the 7.1 definition of L2 ("role visible on screen but must be discovered"). The rule is therefore
applied by what the screen shows, not by whether a hold is involved. If the owner rejects it, 8.5 gives the count
under the P1-014 letter.

**Disclosure (U28).** U28 "perform a step live" is the same act as U17-U21 (turn a NOTE EDIT knob), seen from a
performance seat once NOTE EDIT is already open; it is scored a second time, as a performance function, because R3
asks for per-step performance. Without U28 the count is 65 of 68 passing.

### 8.2 Every function

| Id | Function | Class | G | V/I/O/L = total | Verdict | Control path |
| --- | --- | --- | --- | --- | --- | --- |
| U01 | Reach the sequencer | S | 0 | 3/3/3/3 = 12 | pass | power-on lands on PERF |
| U02 | Select channel | P | 1 | 3/3/3/3 = 12 | pass | SELECT turn (printed) |
| U03 | Pick an algorithm | P | 1 | 3/3/3/3 = 12 | pass | ALGORITHM turn (printed) |
| U04 | Morph ROW 1 | P | 1 | 3/3/3/3 = 12 | pass | K1..K4 (printed KNOB 1-4) |
| U05 | Switch the knob row | P | 1 | 3/3/3/2 = 11 | pass | OCT- / OCT+ tap (override of the printed word) |
| U06 | Morph ROW 2 (dial-in) | S | 2 | 3/2/3/2 = 10 | pass | OCT+ tap, turn |
| U07 | Dice NOTES | P | 1 | 3/3/3/2 = 11 | pass | SCL |
| U08 | Dice HITS | P | 1 | 3/3/3/2 = 11 | pass | SEQ |
| U09 | Dice ACCNT | P | 1 | 3/3/3/2 = 11 | pass | ENV |
| U10 | Dice ARTIC | P | 1 | 3/3/3/2 = 11 | pass | ARP |
| U11 | Dice SHAPE | P | 1 | 3/3/3/2 = 11 | pass | LFO |
| U12 | Dice ALL | P | 1 | 3/3/3/2 = 11 | pass | FX |
| U13 | Undo / redo | P | 1 | 3/3/3/2 = 11 | pass | EDIT tap |
| U14 | Step ON / OFF | P | 1 | 3/3/3/3 = 12 | pass | tap a white key (the lane is the legend) |
| U15 | Accent a step by a hard tap (optional, FORCE on) | P | 1 | 2/3/3/1 = 9 | EXC E3 | hard tap on F3..C5 (`ACC` shown only once pressed; nothing on PERF names it; 12 of 16 step keys) |
| U16 | Open NOTE EDIT | S | 1 | 3/2/3/2 = 10 | pass | hold a white key 0.4 s |
| U17 | Step pitch | S | 2 | 3/2/3/2 = 10 | pass | hold key, K1 (sounds) |
| U18 | Step octave | S | 2 | 3/2/3/2 = 10 | pass | hold key, OCT- / OCT+ (printed octave, but reached only through the step hold, like U17) |
| U19 | Step gate / tie | S | 2 | 3/2/3/2 = 10 | pass | hold key, K2 |
| U20 | Step velocity | S | 2 | 3/2/3/2 = 10 | pass | hold key, K3 |
| U21 | Step ratchet | S | 2 | 3/2/3/2 = 10 | pass | hold key, K4 |
| U22 | Step slide | S | 2 | 3/2/3/2 = 10 | pass | hold key, ARP |
| U23 | Step accent | S | 2 | 3/2/3/2 = 10 | pass | hold key, ENV |
| U24 | Step chance | S | 2 | 3/2/3/2 = 10 | pass | hold key, LFO |
| U25 | Step ON / OFF inside NOTE EDIT | S | 2 | 3/2/3/2 = 10 | pass | hold key, SEQ (or tap the key) |
| U26 | Dice one step | S | 2 | 3/2/3/2 = 10 | pass | hold key, FX / SCL |
| U27 | Edit the next step | S | 1 | 3/2/3/2 = 10 | pass | in NOTE EDIT hold another white key |
| U28 | Perform a step live | P | 1 | 3/3/3/2 = 11 | pass | NOTE EDIT open while playing, turn a knob |
| U29 | Clear one step | S | 2 | 3/2/3/2 = 10 | pass | hold EDIT, tap the key |
| U30 | Clear one field of a step | S | 2 | 3/2/3/2 = 10 | pass | in NOTE EDIT hold EDIT, touch the control |
| U31 | Clear the viewed page | S | 2 | 3/2/3/2 = 10 | pass | hold EDIT, REC |
| U32 | Re-copy the page before | S | 2 | 3/2/3/2 = 10 | pass | hold EDIT, D#4 |
| U33 | See which steps are edited | S | 0 | 3/3/3/2 = 11 | pass | corner tick and glyphs on the lane |
| U34 | Swap the viewed page | P | 1 | 3/3/3/2 = 11 | pass | C#5 PG- / D#5 PG+ |
| U35 | Add a page (copy) | S | 2 | 3/2/3/2 = 10 | pass | PG+ on the last page, PG+ |
| U36 | Pattern length | S | 2 | 3/2/3/2 = 10 | pass | D#4 (CHAN), K1 |
| U37 | Page source (what an earlier page plays, or OWN) | S | 2 | 3/2/3/2 = 10 | pass | D#4, K3 (viewed page) |
| U38 | FOLLOW on / off | S | 2 | 3/2/3/2 = 10 | pass | D#4, K4 |
| U39 | Know page, length, playhead | S | 0 | 3/3/3/3 = 12 | pass | header page map, PG n/m, >P3 |
| U40 | MIDI channel | S | 2 | 3/2/3/2 = 10 | pass | D#4, K2 |
| U41 | Tempo | S | 2 | 3/2/3/3 = 11 | pass | GLO, K1 (printed global) |
| U42 | Swing | S | 2 | 3/2/3/3 = 11 | pass | GLO, K2 |
| U43 | Global gate | S | 2 | 3/2/3/3 = 11 | pass | GLO, K3 |
| U44 | Division | S | 2 | 3/2/3/3 = 11 | pass | GLO, K4 |
| U45 | Scale | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, K1 |
| U46 | Key | S | 2 | 3/2/3/2 = 10 | pass | hold GLO, K2 (the keys now type chords) |
| U47 | Global octave | S | 2 | 3/2/3/3 = 11 | pass | tap GLO, OCT- / OCT+ (printed octave; moved from the SCALE page) |
| U48 | Mute a channel | P | 1 | 3/3/3/2 = 11 | pass | F#3 G#3 A#3 C#4 |
| U49 | Solo | P | 1 | 3/3/3/2 = 11 | pass | hold F#4 |
| U50 | Fill (with pressure) | P | 1 | 3/3/3/2 = 11 | pass | hold G#4, lean |
| U51 | Roll (with pressure) | P | 1 | 3/3/3/2 = 11 | pass | hold A#4, lean |
| U52 | Restart | P | 1 | 3/3/3/2 = 11 | pass | F#5 |
| U53 | Start / stop | P | 1 | 3/3/3/3 = 12 | pass | PLAY |
| U54 | Play a line in | S | 2 | 3/2/3/3 = 11 | pass | REC, keys (printed record) |
| U55 | Leave REC | S | 1 | 3/3/3/3 = 12 | pass | REC or HOME |
| U56 | Save | S | 1 | 3/3/3/3 = 12 | pass | SAVE |
| U57 | Load a preset (stopped) | S | 1 | 3/3/2/3 = 11 | pass | PRESETS turn (`P05?` at once; loads 1.5 s after the last detent) |
| U58 | Undo a load | S | 1 | 3/2/3/3 = 11 | pass | hold SAVE 0.7 s (printed; state and header slot return) |
| U59 | Reset one morph | S | 2 | 3/2/3/2 = 10 | pass | hold EDIT, turn the knob |
| U60 | Reset a knob row | S | 2 | 3/2/3/2 = 10 | pass | hold EDIT, OCT- / OCT+ |
| U61 | Reset the algorithm or a layer | S | 2 | 3/2/3/2 = 10 | pass | hold EDIT, FX / a dice button |
| U62 | Ask what a control does | S | 2 | 3/1/3/2 = 9 | EXC E2 | hold HOME, touch the control (owner decisions 57 / 61) |
| U63 | Menu | S | 1 | 3/2/3/3 = 11 | pass | hold HOME 0.7 s touching nothing, release (printed) |
| U64 | Escape to PERF | S | 1 | 3/3/3/3 = 12 | pass | HOME tap |
| U65 | First-run guidance (incl. step 1 MIDI CHANNEL prompt; adds no gesture, the gesture is U40) | S | 0 | 3/3/3/3 = 12 | pass | strip on first power-on; the prompt is visible text naming the exact controls (L3), the user need not know that CHAN exists (I stays 3 because the gesture is named) |
| U66 | Hear a pitch before keeping it | S | 2 | 3/2/3/2 = 10 | pass | hold key, K1 (each detent sounds); or REC |
| U67 | Velocity by force in REC | S | 2 | 3/2/3/2 = 10 | pass | REC, play harder |
| U68 | Cue a preset while playing | S | 1 | 3/3/1/3 = 10 | EXC E1 | PRESETS turn (lands on the first 768-tick bar starting >= 1.5 s after the last detent) |
| U69 | Save into another slot (stopped or playing) | S | 2 | 3/2/3/2 = 10 | pass | PRESETS turn (`P05?`), SAVE tap within the pending window (>= 1.5 s both ways; playing: until the bar the load waits for). L2: the window is shown (`P05?`, strip `SAVE = save here`) but not printed |

Totals (restated L rule, with the HARMONY rows U70-U79 of revision 3): **79 functions, 76 pass, 3 listed exceptions** (E1 U68, E2 U62, E3 U15). 23 performance
functions: 22 pass at >= 11, U15 fails (9). 56 setup functions: 54 pass at >= 10, U62 (9, I1) and U68 (10, O1) fail
on a sub-score. No function needs more than 2 gestures. Sum 839 of 948 (recomputed by script; revision 2 lowers U18
and U69 to L2; revision 3 adds 10 rows and lifts U47 to 11).

### 8.3 The exceptions, argued (listed, not hidden)

| Exc | Function | Sub-score | Why it stays | Alternative (one table constant) |
| --- | --- | --- | --- | --- |
| E1 | U68 | O1: heard 1.5 s to 1.5 s + one bar after the last detent (1.5..3.5 s at 120 BPM) | owner decision 34 / 45 queue a load to the bar. The reason is musical **and** ergonomic: PRESETS is a detent encoder, so loading without a settle would play every slot turned through (P02, P03 on the way to P04), and revision 2 adds the `SQ_LOAD_SETTLE` wait so a SAVE tap into the pending slot (U69) never races the bar. The screen answers at once (`P04?`, then `P04>` and a bar countdown), so the user is never in doubt. Stopped loads land 1.5 s after the last detent (U57, 3/3/2/3 = 11, a pass). | `SQ_LOAD_AT = STEP` (the first step that starts at least `SQ_LOAD_SETTLE` after the last detent; the same SAVE window) scores 3/3/2/3 = 11; owner question 71 |
| E2 | U62 | I1: hold HOME while touching another control (a modifier + control) | **owner decisions 57 and 61** fix HELP as "hold HOME and touch a control", HOME acting on release. HELP is not a musical function, it never blocks PLAY, it is named on PERF (`h:HLP/MENU`) and by the strip the moment HOME is held, and it leaves no page behind. The first draft's alternative (HOME tap on PERF opens a latched HELP page, 10) is reverted because it reopened an owner decision and created a double-HOME dead end | none proposed: a latched HELP would need the owner to reopen decision 57 |
| E3 | U15 | V2 / L1: nothing on PERF names the hard tap before it happens; 12 of 16 step keys | an **optional extra outside the bar**: `FORCE` defaults OFF until HT-FORCE passes, and every accent is reachable by U23 (NOTE EDIT `ENV`, 10, pass) and U20 (`K3 VELO`). With FORCE on the playing strip alternates in `HARD TAP = ACCENT`, which by rule does not lift L | dropping FS1 would remove the most direct force gesture without raising any score; owner question 68 |

### 8.4 Editing models compared

Four genuinely different ways to edit steps after dialling an algorithm in, scored on the same functions (honest L;
G = gestures from PERF).

| Function | M1 momentary step band (P1-014: hold the key while turning) | **M2 latched NOTE EDIT (chosen)** | M3 step cursor mode (a STEP mode: keys move a cursor, knobs edit it) | M4 NOTE EDIT with keys as pitches |
| --- | --- | --- | --- | --- |
| a. one step's pitch | hold + K1: 3/1/3/2 = 9 FAIL | hold, K1: 3/2/3/2 = 10 | mode, key, K1 (G3): 3/1/3/2 = 9 FAIL | hold, press the pitch key: 3/2/3/3 = 11 |
| b. ratchet / slide / chance | hold + OCT+ + K (G3): 3/1/3/2 = 9 FAIL | hold, K4 / ARP / LFO: 10 | mode, key, K (G3): 9 FAIL | hold, K / button: 10 |
| c. edit the next step | release, hold next + turn: 9 FAIL | hold next key (G1): 10 | tap next key: 3/3/3/2 = 11 | HOME, hold next (G2): 10 |
| d. perform a step live (twist it while it loops) | hold + turn: 3/1/3/2 = 9 FAIL | turn (page open): 11 | turn (mode on): 11 | turn: 11 |
| e. step ON / OFF from PERF | tap: 12 | tap: 12 | outside the mode 12 | tap: 12 |
| f. ON / OFF of another step while editing | release, tap: 12 | tap: 12 | keys move the cursor; a button: 10 | keys are pitches; leave first: 10 |
| g. hear a pitch before keeping it | hold + K1: 9 FAIL | hold, K1: 10 | 9 FAIL | the key sounds: 11 |
| **Total (7) / fails** | **69 / 5 fail** | **75 / 0** | **71 / 3 fail** | **75 / 0** |
| A whole 16-note line by hand | about 48 gestures | about 48 (or REC: 18) | about 49 | about 33 |
| Hands | one hand pinned to the key | both free | both free | both free |
| Key meaning | one (steps) | **one (steps) on every step page** | two (steps / cursor) | two (steps / pitches) |

**Choice: M2.** M2 and M4 tie on points; M1 and M3 fail the bar. M2 wins the tie on four counts: (1) it is the
owner's model word for word (hold a step, a NOTE EDIT page, the knobs change pitch, octave, gate, velocity, ratchet,
slide, chance); (2) the white keys keep **one** meaning on every page that shows steps (tap = on/off, hold = edit),
which is the learnability rule a first-time user leans on most; (3) moving to the next step is one gesture, which is
what editing 64 steps is made of; (4) M4's one advantage, choosing a pitch by key, is already in the instrument as REC
(18 gestures for 16 notes, with force velocity) and REC opens at the edited step from NOTE EDIT. **Fun:** M2 is the
only model where the user can park on a step and play it (twist its ratchet, gate and pitch while the loop runs) with
both hands free, which is the "per-step performance" the owner asked for in R3.

### 8.5 The count under both L rules

| Rule | Pass | Fail | Failing functions |
| --- | --- | --- | --- |
| Restated rule (8.1; hold named on its starting page = L2) | 66 of 69 | 3 | U15 (E3), U62 (E2), U68 (E1) |
| P1-014 letter (any role found by a hold or combo = L1) | **43 of 69** | 26 | the three above, plus 23 setup functions that fall from 10 to 9 with L1: U16 open NOTE EDIT, U17 step pitch, U18 step octave, U19 gate, U20 velocity, U21 ratchet, U22 slide, U23 accent, U24 chance, U25 on/off in NOTE EDIT, U26 dice one step, U27 edit the next step, U29 clear a step, U30 clear a field, U31 clear the page, U32 re-copy the page before, U45 scale, U46 key, U47 global octave, U59 reset a morph, U60 reset a row, U61 reset the algorithm or a layer, U66 hear a pitch first |

Not affected by the choice: U58 and U63 (holds printed on the reference card: `SAVE hold: undo the last load`,
`HOME hold: Menu`), U49-U51 (momentary black-key moves, L2 as black keys named on the legend under both rules) and
U69 (a turn and a tap, no hold: L2 = 10 under both). U18 (step octave) **is** affected: the printed OCT word fits the
job, but the role is reached only through the step hold, exactly like U17, so it is L2 (10) under the restated rule
and L1 (9) under the P1-014 letter (revision 2; revision 1 wrongly listed it as unaffected at L3). The 23 functions
are all set-up edits that live on a page opened by a hold; no performance function depends on the restatement.

## 9. Data model and budget

### 9.1 Structs (replace ADR-0003 section 3's pin fields; the ADR is amended to match)

```c
#define SQ_NSTEP     64          /* steps per channel = 4 pages of 16 */
#define SQ_PAGE      16
#define SE_AUTO      0           /* every field: 0 = follow the generated layer */
/* One step's edit layer. 4 B. */
typedef struct {
    uint8_t note;    /* bit7 = PITCH set; bits0..6 = semitones above 36+key, 0..96, UNSNAPPED (snapped at play time) */
    uint8_t vel;     /* 0 AUTO, 1..127 */
    uint8_t gr;      /* bits0..3 GATE: 0 AUTO, 1..10 = 10..100 %, 11 = TIE; bits4..6 RATCH: 0 AUTO, 1..4; bit7 0 */
    uint8_t fl;      /* bits0..1 ON: 0 AUTO 1 ON 2 OFF; bits2..3 SLIDE: 0 AUTO 1 ON 2 OFF; bit4 PROB: 0 100 % 1 PROB_LOW;
                        bits5..7 OCT: 0 AUTO / 0, 1..5 = -2..+2 (value - 3), 6..7 unused */
} StepEdit;
_Static_assert(sizeof(StepEdit) == 4, "StepEdit");

/* One channel as saved. VISION rule 3: MIDI channel, algorithm, LENGTH (plus mute and the per-page sources). 344 B */
typedef struct {
    uint8_t  midi_ch;              /* 0..15, 16 = OFF */
    uint8_t  algo;                 /* 0..3 */
    uint8_t  muted;                /* 0/1 */
    uint8_t  len;                  /* 1..64 */
    uint8_t  pg_src;               /* bits 2k..2k+1 = src(k) in 0..k: the generator block page k reads (positions
                                      16*src..16*src+15; 1.4); src(0) = 0; src(k) = k is OWN; a new page takes src(k-1);
                                      the CHAN K3 labels (Pm = first page playing that block) are derived, not stored */
    uint8_t  pg_used;              /* bit k = page k holds content (shown or hidden); extend restores, else copies */
    uint8_t  pad[2];
    AlgoState st[SQ_NALGO];        /* 4 x 20 B, unchanged */
    StepEdit ed[SQ_NSTEP];         /* 64 x 4 B */
} ChanSave;
_Static_assert(sizeof(ChanSave) == 344, "ChanSave");

/* One undo record per channel (decision 46). 92 B */
/* The last change wins: a NOTE EDIT visit snapshots its step (bit3); tapping ANOTHER step's key inside NOTE EDIT
   (ON/OFF) replaces the record with that step, so EDIT tap then undoes that tap, not the visit (toast UNDO: STEP 7). */
typedef struct {
    uint8_t  what;                 /* bit0 seeds, bit1 one page of edits, bit2 morphs, bit3 one step, bit4 len/pg bits */
    uint8_t  algo, seed_mask, idx; /* idx = page (bit1) or step (bit3) */
    uint16_t seed[SQ_NSEED];       /* 12 B, the OTHER values (swap = undo, swap again = redo) */
    uint8_t  morph[SQ_NMORPH];     /* 8 B */
    uint8_t  len, pg_src, pg_used, pad;
    StepEdit ed[SQ_PAGE];          /* 64 B: one page, or ed[0] for one step */
} UndoRec;
_Static_assert(sizeof(UndoRec) == 92, "UndoRec");

/* Preset = 12 + 4 x 344 = 1388 B (version 2); P2-012 adds HarmonySave (8 B, HARMONY.md 7): 1396 B (version 3). */
_Static_assert(sizeof(SeqPreset) == 1396, "SeqPreset"); /* with HarmonySave */
/* ISR-only runtime per channel, 24 B. ADR-0003's uint32 t is removed and the new fields go FIRST, in this order
   (revision 2; compiled: grid@0 pass@4 p@6 tie@7 nxt@8 ... open@21, size 24). Replacing t in place instead
   (p, pass, tie before grid) pads to 28 B and fails the assert. */
typedef struct {
    uint32_t grid;                 /* tick of the nominal (un-swung) start of step p */
    uint16_t pass;                 /* loops since PLAY / RESTRT (chance draw, 9.3) */
    uint8_t  p;                    /* index of the NEXT step to start, 0..len-1 */
    uint8_t  tie;                  /* 1 = a TIE is holding the sounding note (9.2, 10) */
    PulseOut nxt;                  /* 6 B, unchanged from here on (ADR-0003 section 3) */
    uint8_t  nxt_ok;
    uint8_t  snd[2];
    uint8_t  snd_chan[2];
    uint8_t  slide_in, act, open;
} ChanRT;
_Static_assert(sizeof(ChanRT) == 24, "ChanRT");
```

### 9.2 Resolving a step (ISR, `sq_eval`, replaces the pin overlay of ADR-0003 9)

```
k = p / 16; g = 16 * ((pg_src >> 2*k) & 3) + p % 16;     // generator position (1.4); src(0) is always 0
step(cache, g, &o);                                       // generated layer; note/vel/gate ALWAYS valid (EUCLID: contour at g)
e = ed[p];
hit  = ON(e) == AUTO ? (o.flags & PO_HIT) : ON(e) == ON;
note = PITCH set ? sq_note(g, e.note & 0x7F) : o.note;   note += 12 * OCT(e);   (clamped 0..127)
vel  = e.vel ? e.vel : o.vel;
gate = GATE(e) ? table[GATE(e)] : o.gate;  TIE -> hold until the next sounding step, searched with the same wrap as n
       // (after the last step, from step 1): a TIE crosses the loop end like a slide; it ends at the latest when the
       // tied step itself comes round again (at most one loop); cut only by STOP or a LENGTH change while it holds
rep  = RATCH(e) ? RATCH(e) : o.rep;
n    = (p + 1 >= len) ? 0 : p + 1;                       // THE next step: after the last step, step 1
slideout(p) = (SLIDE(e) == AUTO ? (generated PO_SLIDE_IN of step n, read at g(n)) : SLIDE(e) == ON)
              && hit(n)                                  // resolved hit of n (edits, its chance draw with pass+1 at the wrap); NOT mute: mute and SOLO act at the MIDI gate in sq_emit
              // allowed across the loop end; cancelled only by STOP (every note off) or a LENGTH change inside
              // step p (the off returns to p's own gate end, or is sent at once if that has passed)
if PROB(e) && (sp1_hash(ch, p, pass) & 0xFF) >= PROB_LOW_256: hit = 0
```

Pure integer work, about 30 operations per step per channel on top of `step()`; well inside the ADR-0003 budget
(+5 % of the 725 us block). Next position: `p = (p + 1 >= len) ? 0 : p + 1`; `pass++` at the wrap; PLAY and RESTRT
set `p = 0, pass = 0`.

### 9.3 Determinism

Every value above is a function of (seeds, morphs, globals, edits, p, pass). The chance draw uses SP1 on
`ch * 0x9E37 ^ p * 0x45D9 ^ pass * 0x2C1B`, so the same pattern started from PLAY plays the same chance outcomes, and
they repeat after 65536 passes. The only non-deterministic moments stay the dice presses (ADR-0003 3.1) and the new
per-step dice (`FX` / `SCL` in NOTE EDIT), whose results are stored as edits.

### 9.4 RAM (.bss), re-added against ADR-0003 section 11 and SYSTEM_MAP section 9

| Item | B | Note |
| --- | ---: | --- |
| ADR-0003 section 11 total new .bss (P1-013 version, `ChanSave` 4 x 102, `UndoRec` 4 x 34) | 5329 | base |
| `ChanSave` 102 -> 344 B, x4 | +968 | pins (18 B) out; len, page bits, 64 edits in |
| `UndoRec` 34 -> 92 B, x4 | +232 | one page of edits, morphs (reset), len / page bits |
| REC state (armed, cursor, ACC, SLIDE, octave) 4 + sounding-note table 27 | +31 | from P1-014, kept |
| NOTE_EDIT and page UI: edited step, open flag, `view_page[4]`, FOLLOW, ADD prompt, open page id, HELP control | +10 | inside the 3072 B UI allowance in practice; counted anyway |
| `sq_perf` (SOLO, ROLL), saved FILL morph, FILL / ROLL force levels | +4 | |
| Pending preset slot (1 B) + settle deadline (2 B) | +3 | revision 1, U69; the pending load reuses the pool staging copy |
| Page sources `pg_src` (replaces `pg_flow`, same byte) | 0 | revision 1 |
| Force sensing (20 x 8 B key state, 20 x 2 B floors, 20 x 2 B H3 neighbour offsets, MASTER cache) | +242 | section 6.4 (revision 1 adds the H3 40 B) |
| P1-014 reserve for step records (768 + 768) and scenes | 0 | dropped: replaced by the rows above |
| HARMONY state: `HarmonySave` x2 16 + index 1 + `HarmRT` 14 + `HarmUndo` 9 + `undo_last` 1 + typing cursor 1 | +42 | revision 3 (P2-012), HARMONY.md 7; `ChanRT` stays 24 B |
| **Total new .bss** | **6861** | (6819 before revision 3; first draft 6776; P1-014 had 6962) |

| Check | Headroom before | New .bss | Headroom after | Above the 4096 B floor |
| --- | ---: | ---: | ---: | ---: |
| After the E3 removals | 46496 | 6861 | **39635 B** | 35539 B |
| Built before the removals | 17984 | 6861 | **11123 B** (above the 8192 B E9 trigger by 2931 B) | 7027 B |

The 4 KB floor holds in both orders, and so does the owner's 8 KB trigger (decision Mem): the full design fits even
if it is built before any removal.

### 9.5 Pool and flash

| Item | B |
| --- | ---: |
| Preset slots P01..P16, 16 x 1388 | 22208 |
| Staging copy for a queued load + "undo last load" copy, 2 x 1388 | 2776 |
| HarmonySave in each of the 18 preset-sized copies, 18 x 8 | 144 (revision 3: slot size 1396 B) |
| Scenes | 0 (removed) |
| Pending slot (revision 1) | 0 (a stopped pending load is staged in the same staging copy) |
| **Total pool** | **25128** (24984 + 144, revision 3) |

Pool free **today** is 25764 B: it would fit with **780 B** to spare (**636 B** after HarmonySave, revision 3), which is not a margin. **Prerequisite (P2-005
must order it): the SYSTEM_MAP section 6 removal of the fx buses (R6, 145042 B pool) lands before the preset storage
task (E7).** After the removals 217975 - 25128 = 192847 B free. If E7 must come first, `SQ_NSLOT` = 8 (13880 B pool,
11884 B spare) until R6 lands; the slot count is one constant. A packed preset encoding was considered and rejected:
flash slots are sized for the worst case anyway, and folding OCT into PITCH (3-byte edits, 1132 B presets) would make
an octave edit stop following NOTES dice. Flash: about 6 KB new (force 2.5 KB, help and page strings 3.5 KB), against
more than 140 KB freed by SYSTEM_MAP section 9.

## 10. Musical output review

What the user hears, checked against ADR-0003 section 7 (192 PPQN; jitter at most one 0.73 ms control block).

| Operation | What is heard | Timing and note-off guarantee |
| --- | --- | --- |
| Tap a step ON / OFF | the step plays / stops from its next pass; stopped: it sounds once at once | the edit is read at the step start; a sounding note is never cut (its off is queued) |
| Edit a step in NOTE EDIT | every K1 detent sounds the new pitch at once (audition: its own on/off pair, 120 ms, through the queue with the same-pitch safety rule of 7.3 step 6); the loop plays it from the next pass | the audition note carries its channel byte, so a MIDI-channel change cannot strand it |
| Add a page | nothing changes (the copy plays the same notes) | page boundaries are ordinary step boundaries |
| Dice with edits present | the edited steps stay put; the algorithm moves around them: "your hand-made notes, a new line between them" | next step (AF_PAIRING hit / note one step later, decision on question 9) |
| Ratchet (generated or edited) | 2-4 triggers inside the step, SP5 spacing | inside `[S, S1 - 1]`; an odd (swung) step has a shorter cell, so repeats never cross into the next step; spacing >= 3 ticks or `rep` drops |
| Slide (edited ON on step p) | the TB-3 glides from p into the next step if it sounds (TB-303 rule); if it does not, p plays its own gate | off of p at `S1 + D`, `D = max(1, L/8)` ticks (3 at 1/32, 6 at 1/16): the only legal overlap |
| Slide on the last step | glides into step 1 across the loop end, every pass (the acid "pickup into the downbeat") | the next step at the wrap is known (`n = 0`), so the pairing is resolved as for any neighbour. Cut only on STOP (every note off, then CC123) and by a LENGTH change inside that step (the off falls back to the step's own gate end). ACID's generated table never flags position 0 (`ps[0] = 0`, sting 6.3), so a generated glide across the wrap appears only through OFFSET / DIR or an edit |
| Slide from page 1 into page 2 | glides like any neighbour pair | allowed: page boundaries are ordinary step boundaries |
| TIE | the note holds through following silent steps until the next sounding step, across the loop end too (a tie on the last step holds into step 1, the same rule as a slide) | off emitted at the next sounding step's start (offs before ons); at most one loop (the tied step coming round again ends it). Cut only by STOP (every note off, then CC123) and by a LENGTH change while it holds (the off is sent at the next step start). Revision 1 closed a tie at the wrap while letting a slide cross it; there is no musical reason for the difference (both are the 303 "hold into the next note"), so both now cross |
| Swing across pages | identical on every page: parity is by pattern position, pages are 16 (even) | with an odd LENGTH the wrap joins two even steps (both on the grid); stated, by design |
| LENGTH shrunk while playing | the loop closes at the new end from the next step | a sounding note ends by its queued off; an open TIE is closed at the next step start (a LENGTH change cuts a tie, as it cuts a slide) |
| Channels of different lengths | polymeter (CH1 32 against CH2 12) | each channel wraps on its own; the preset bar stays 768 ticks |
| Chance 50 % | the step plays on some passes, the same ones every time from PLAY | draw at step start; a step that does not play sends nothing |
| STOP | silence | every sounding note's off, then CC123 on each channel in use (ADR-0003 8.4) |
| MUTE / SOLO / FILL / ROLL | as before; FILL / ROLL depth follows pressure | ROLL repeats obey the ratchet spacing rule |

## 11. Confusion audit

### 11.1 States in which a first-time user could be lost, and the cue that rescues them

| # | Confusing state | Rescuing cue |
| --- | --- | --- |
| 1 | PLAY pressed, nothing heard (muted, MIDI OFF, K1 = 0, the synth on another channel) | CH1 defaults to MIDI 1 (channels match their numbers), shown `M1` in the header; first-run step 1 says `D#4 K2: MIDI CHANNEL`; channel cell `MUTE n` / `MIDI OFF`, K1 cell `KEYS` (the state is still called KEYS ONLY); strip `CH1 MUTED: F#3` or `MIDI OFF: D#4 K2`. The device cannot see the synth's channel, so the header always shows the MIDI channel it sends on |
| 2 | Held a step and the knobs changed meaning | NOTE_EDIT banner `STEP 21`, every cell relabelled, strip `STEP 21: KNOBS EDIT  HOME=done` |
| 3 | Let go of the step key and the page stayed | the banner stays, HOME is labelled `HOME` (not HELP); strip says `HOME=done` |
| 4 | Tapped a key and a step vanished | it was ON: the cell is hollow with a slash (forced OFF); tap again restores; EDIT takes it back |
| 5 | "My pattern changed when I diced, but some notes did not" | edited steps carry a corner tick; HELP on SCL says `edited steps stay` |
| 6 | Pressed PG+ and a prompt appeared | ghost page cell and a strip that says what PG+ will do (`ADD PAGE 2 = COPY OF 1?`, `SHOW PAGE 3 (HIDDEN)?`, `SHOW ALL OF PAGE 2 (L32)?`); any other control cancels |
| 7 | Added a page and nothing changed | toast `PAGE 2 = COPY OF PAGE 1: CHANGE IT`; the view is on page 2 |
| 8 | The view jumped to another page | FOLLOW is on: `FOLLOW` in the lane header; PG- / PG+ hold the view (FOLLOW off) |
| 9 | The view does not move while playing | `FOLLOW OFF` in the lane header and `>P3` marker; strip `D#4 K4 = FOLLOW` |
| 10 | Keys past the end do nothing | dim cells; toast `PAST THE END: CHAN K1 = LENGTH` |
| 11 | Shrunk the length and steps are gone | hollow page-map cells (`hidden`); toast `HIDDEN, NOT LOST: turn K1 back` |
| 12 | Hard taps make accents I did not want | `ACC` shows in the cell before release; lighter tap; MENU `FORCE OFF` |
| 13 | Turned a ROW 1 knob and a ROW 2 value moved | gutter digit and filled row; OCT+ cell inverted; strip `ROW 2: OCT- = back to row 1` |
| 14 | In some page and unsure how to leave | every page has a banner and `HOME=back` in the strip |
| 15 | Held HOME and the strip changed | `HOME: touch=HELP  let go=MENU` from 0.25 s; letting go early is a plain HOME; after a touch, letting go returns with nothing changed |
| 16 | Pressed EDIT expecting "edit" | toast `UNDO: <what>`; HELP on EDIT; a second tap redoes |
| 17 | Held EDIT and the screen filled with RST | RESET banner; strip `RESET: TOUCH WHAT TO RESET  EDIT=no`; after one reset it closes by itself |
| 18 | Turned PRESETS while playing and nothing happened | header `P04?` (settling, 1.5 s after the last detent), then `P04>` and the bar countdown; strip `P04 AT BAR  SAVE = save here`, then after the load `SAVE hold=undo` |
| 19 | Pressed REC and keys play notes | REC banner and LED; strip `REC: PLAY KEYS TO WRITE  REC=done` |
| 20 | In SCALE, pressing keys changes the key | banner `SCALE`, strip `SCALE: PRESS A KEY = KEY  HOME=back` |
| 21 | A slide does nothing | the next step is silent (OFF or chance): the `~` glyph is drawn dim; HELP on ARP in NOTE EDIT says `glides into the next step if it plays` |
| 23 | Turned STAGE `TOTAL` past 16 and little changed, or part of a long loop never plays | cell `TOTAL` over `32 (16)` / `48 (32)`, toast naming the first unheard span `17-32 NOT HEARD: D#4 K3 = OWN`; every unheard shown position is counted, also a gap between heard ones and a partial last page (`LENGTH 20`, `TOTAL 32`: `21-32 NOT HEARD`) (1.4) |
| 24 | Turned PRESETS and the header shows `P05?` | strip `P05: SAVE = save here` and the settle countdown (playing: then the bar countdown); let go = it loads (stopped after 1.5 s; playing on the first bar 1.5 s or more later); SAVE in that time = save here; turn back = cancel |
| 25 | PG+ on LENGTH 20 did not add page 3 | strip `SHOW ALL OF PAGE 2 (L32)? PG+ = yes`: a partial page is completed first (1.3) |
| 22 | The dice button does nothing (absent layer) | dim `SHAPE -`, toast `NO SHAPE IN EUCLID` (ALGORITHM_SET 4.4) |
| 26 | Turned PRESETS to a slot that was never saved (P2-011, F-15; see owner question 79) | recommended behaviour: nothing is loaded and nothing in the pattern changes; after the settle the header goes back to the slot it showed and the toast says `P09 EMPTY: NOTHING LOADED`; SAVE tapped while `P09?` is pending still saves into it (U69), which is how an empty slot is filled |
| 27 | Power-cycled the device and wondered what it shows (P2-011, F-15) | the PERF main screen, stopped. After a clean shutdown the autosave is restored (ADR-0003 12 step 6): same slot in the header (`Pnn`), pattern, channels, tempo and scale as at power-off; after a crash, or the first power-on, the defaults of ADR-0003 12 (`P01`, CH1 ACID MIDI 1, ...). No toast; nothing plays until PLAY |
| 28 | Gave a channel the MIDI channel another channel uses, and the other one went quiet (P2-011, F-14) | impossible to clash: the other channel is set to OFF, its cell reads `MIDI OFF`, the toast names it (`CH2 IS MIDI OFF: CH1 TAKES 2`), EDIT takes both back (4.4) |

### 11.2 Cue strip priority (first match wins; 34 characters nominal, 240 px binding; table data)

Toasts (SCREEN_SPEC 2.7) are not rows of this table: a toast is drawn in the same strip for 2 s and outranks rows 2 to 15 (never row 1). Rows 10a, 10b and 5a were added or moved by P2-011 (F-01, F-09); the stale row 13a `EXT CLOCK: WAITING FOR USB` (a state P2-010 removed) is deleted.

| Priority | State | Text |
| --- | --- | --- |
| 1 | HOME held | `HOME: touch=HELP  let go=MENU`, then the touched control's line (`HITS: re-roll where notes fall`) |
| 2 | RESET | `RESET: TOUCH WHAT TO RESET  EDIT=no` |
| 3 | ADD_PAGE prompt | `ADD PAGE n = COPY OF n-1? PG+ = yes` / `SHOW PAGE n (HIDDEN)? PG+ = yes` / `SHOW ALL OF PAGE n (Lnn)? PG+ = yes` |
| 4 | REC | `REC: PLAY KEYS TO WRITE  REC=done` |
| 5 | NOTE_EDIT | `STEP n: KNOBS EDIT  HOME=done` |
| 5a | first-run step 1 or 2 while on CHAN, SCALE or SET (P2-011, F-09: it outranks the page rows 6 to 8) | step 1 on CHAN: `K2: MIDI CHANNEL  HOME=done`; step 2: `PRESS PLAY` (on CHAN, SCALE, SET) |
| 6 | CHAN | `CHn: K1 LENGTH  OCT=+-PAGE  D#4=back` |
| 7 | SCALE | `SCALE: PRESS A KEY = KEY  HOME=back` |
| 8 | SET | `SET: GLO hold=SCALE  HOME=back` |
| 8a | SET, tempo source EXT (added, 2.8) | `EXT: K1 OFF, HOST SETS TEMPO` |
| 9 | SOLO / FILL / ROLL held | `ROLL 3x: let go to stop` (same shape) |
| 10 | a preset slot pending | `P05: SAVE = save here` (stopped, with the 1.5 s countdown) / `P05 AT BAR  SAVE = save here` (playing: from the last detent until the load lands on the first bar starting >= 1.5 s later) |
| 10a | the clock source is EXT and PLAY was pressed, the host has not started (`clk_armed`, WAIT; added by P2-010, moved here by P2-011) | `WAIT FOR HOST START  PLAY=cancel` |
| 10b | the host clock was lost: 3 s after TIMING_SPEC R6 / R9 | `CLOCK LOST: STOPPED` (R6, while playing) / `CLOCK LOST: PLAY TO START` (R9, while armed) |
| 11 | just after a dice / reset / clear | `EDIT takes it back` (2 s) |
| 12 | first-run step | the step's text (11.3) |
| 13 | stopped (rows 10a and 10b are above this row, so a WAIT or a lost clock is never hidden by it) | `PLAY: start   K1-K4: morph` |
| 14 | playing, ROW 1 | `TAP STEP=ON/OFF  HOLD=EDIT NOTE` (alternates every 4 s with `FX=dice  EDIT=undo  PG+=page`, and with `HARD TAP = ACCENT` when FORCE is on) |
| 15 | playing, ROW 2 | `ROW 2: OCT- = back to row 1` |

### 11.3 First run (once per device; dismissed by HOME; one bit in `ui_prefs2`)

| Step | Strip | Advances when |
| --- | --- | --- |
| 1 | `D#4 K2: MIDI CHANNEL` (on the CHAN page itself, row 5a: `K2: MIDI CHANNEL  HOME=done`) | K2 turned on the CHAN page (the MIDI channel changed), or PLAY (the synth already listens on 1) |
| 2 | `PRESS PLAY` (row 5a: shown on CHAN, SCALE and SET too) | PLAY |
| 3 | `TURN K1 TO MORPH IT` | any knob |
| 4 | `PRESS FX: NEW PATTERN  EDIT=undo` | any dice button |
| 5 | `TAP A WHITE KEY: STEP ON/OFF` | a white key tap |
| 6 | `HOLD A WHITE KEY: EDIT THAT NOTE` | NOTE_EDIT opens (then `HOME=done` when it closes) |
| 7 | `PG+ TWICE: ADD PAGE 2 (A COPY)` | a page added, or any other control 3 times |

### 11.4 The first minute (walkthrough, no manual)

0 s power-on: PERF, strip `D#4 K2: MIDI CHANNEL`. 4 s D#4, K2 to 2 (the TB-3's channel), HOME. 6 s PLAY: an acid line plays on CH1 and the TB-3. 10 s K1 turned: denser; strip moves on.
12 s FX: a new pattern; EDIT once: the old one is back. 20 s taps key 3: a step appears (the cell fills; while
stopped it would sound at once). 26 s holds key 5: the screen says `NOTE EDIT STEP 5`, the knobs say PITCH GATE VELO
RATCH; turns K1: each detent sounds; presses ARP: `SLIDE ON`, hears the glide on the next pass; HOME: back.
40 s PG+ twice: page 2, a copy, nothing changes in the sound; taps two keys: the second bar answers the first.
55 s holds HOME and touches SEQ: the strip explains HITS; lets go: back on PERF. At no point was a manual or a menu
needed, and the TB-3 played from the first PLAY because the first-run strip led the MIDI channel step in two gestures.

## 12. Flex points (ADR-0004)

### 12.1 A table edit (no code)

| Table | What can change |
| --- | --- |
| button_modes.tsv / control_map.tsv | which button toggles which step field in NOTE_EDIT; which page a hold opens; swap CHAN and a mute key; PG- / PG+ on other black keys |
| knob_map.tsv | the NOTE_EDIT knob order (for example PROB on a knob and RATCH on LFO); CHAN / SET / SCALE cell order; SCALE K4 gets a setting |
| `AlgoDesc.knob_morph[8]` | the 3B row assignment per algorithm (4.2) |
| key_map.tsv | force gestures on / off per key; which black keys are moves |
| Constants | `ACC_VEL` 120, `ACC_THR` 100, `PROB_LOW` 50, the 0.4 / 0.7 s holds, force bin edges and the FORCE menu offsets, FILL / ROLL pressure bins, audition length 120 ms, `SQ_NSLOT`, `SQ_LOAD_AT` (BAR / STEP), `SQ_LOAD_SETTLE` 1.5 s, the 0.25 s HOME strip delay, FOLLOW default, the default MIDI channel per channel (`SQ_DEFAULTS`: 2, 3, 4, 5), DRIVE / pull-up sensing mode |
| Cue-strip, first-run and HELP strings | every text and priority |
| Tempo-source strings, the clock timeout (500 ms), the BPM readout rate (2.8, SCREEN_SPEC 6, 8) | text and constants |

### 12.2 Code or an ADR amendment

| Item | Why |
| --- | --- |
| `StepEdit` fields and sizes, 64 steps, 4 pages, `ChanSave` / `UndoRec` / `SeqPreset` sizes | stored preset layout (ADR-0003) |
| Page sources (2 bits per page, a new page inherits the source of the page before) and the loop reset at LENGTH | `sq_eval` and the algorithm purity contract |
| "Tap acts, hold opens a latched page" and the one-latched-page rule | the UI state machine |
| Force sensing in the scan ISR and the ADC schedule | hardware layer (`fm1_input.h`, `master.c`) |
| SLIDE meaning "glide into the next step", across the loop end | core slide pairing (ADR-0003 7.3) |

## 13. Locked

When the owner approves this document, the following are locked for the dev tasks (no choices left to them):

1. The two-layer model: generated layer + per-step edit layer, fields and AUTO semantics (1.1); dice / morph /
   algorithm / scale never touch a set field (1.2); clearing gestures (1.2).
2. Pages: up to 4 x 16 per channel, LENGTH 1..64 per channel, a new page is an exact copy of the page before (edits
   and source), the three PG+ prompts (copy, show hidden, complete a partial page) and `64 STEPS = MAX`, page sources
   (K3 offers only the blocks earlier pages play, labelled by the first page that plays each, plus `OWN`) and
   `g(p) = 16 x src(k) + p % 16`, the heard-span cue counting every unheard shown position, the loop reset (1.3, 1.4); FOLLOW (1.5); extend / shrink / hide /
   commit (1.6).
3. Nine pages and three overlays with their entry, exit, screen content and cue (section 2, page_states.tsv); HOME
   as the one escape (tap = home; on PERF it only cancels a prompt); HOME hold + touch = HELP, release after 0.7 s
   untouched = MENU (owner decisions 57 / 61); "tap acts, hold opens a page"; every hold named on its starting page
   (R-8); one latched page at a time.
4. Every button x mode x page (button_modes.tsv), every knob x page (knob_map.tsv, including the 3B row
   re-lock), every key x page with force and fallback (key_map.tsv), and their union control_map.tsv.
5. NOTE_EDIT: K1 PITCH, K2 GATE, K3 VELO, K4 RATCH, OCT- / OCT+ octave, FX dice step, SCL dice pitch, SEQ on/off,
   ENV accent, ARP slide, LFO chance 100/50, EDIT tap undo of the last step change (the visit, or a later tap on
   another key: the last change wins), hold another key = move.
6. CHAN (D#4): LENGTH, MIDI (unique across channels, a taken one sets the other channel to OFF, 4.4; defaults CH1..CH4 = 1..4; first-run step 1 prompts for it), SOURCE, FOLLOW, OCT = to the page boundary below / above.
   Presets: PRESETS turn = pending slot `Pnn?` (stopped: loads after 1.5 s; playing: on the first bar starting at
   least 1.5 s after the last detent); SAVE tap while
   pending = save there; SAVE hold = undo the last load (state and slot). SET (GLO tap): BPM, SWING, GATE, DIV. HARMONY (GLO
   hold; revision 3, HARMONY.md): SCALE, KEY, VOICE, MOVE, DRONE (OCT- / OCT+), the progression (keys F3-E4, PROG, BARS, NEW) and the channel ROLE; OCTAVE is OCT- / OCT+ on SET. PG- / PG+ on C#5 / D#5; ADD_PAGE by PG+ twice.
7. No scenes; no tap tempo; tempo on SET; the freed controls as listed in 4.3.
8. Force sensing: FS1-FS4 with their fallbacks, DRIVE sensing mode (pull-up fallback), H1-H3 LED handling, the ADC
   schedule, costs, and the HT-FORCE gate (section 6); `FORCE` default OFF until the gate passes.
9. Data: `StepEdit` 4 B, `ChanSave` 344 B, `UndoRec` 92 B, `SeqPreset` 1388 B (1396 B with HarmonySave, revision 3), `ChanRT` 24 B in the field order of
   9.1 (`grid`, `pass`, `p`, `tie`, then ADR-0003's fields); `pg_src` 2 bits per page; RAM 6861 B new .bss (6819 + 42 HARMONY); pool 25128 B (24984 + 144 HARMONY) with the R6-before-E7 prerequisite (section 9).
10. Musical rules of section 10 (SLIDE = glide into the next step, across the loop end, cut only by STOP and a
    LENGTH change; TIE on the same terms; swing parity by position; audition notes; note-off guarantees).
11. Scores of section 8 under the restated L rule (8.1, subject to owner question 72), with the three listed
    exceptions E1, E2, E3.
12. (Added after the lock by FB-001, owner decision 2026-11-10; additive.) The tempo-source and clock-out state of 2.8: INT / EXT n / EXT -- / WAIT on the header and the SET page with a blinking clock dot (P2-010 rulings: source AUTO, own tempo never overwritten, PLAY on an external source waits for the host start); clock out is automatic when the FM-1 is the master and playing (24 per quarter plus Start / SPP / Continue / Stop over USB) and silent when following; 500 ms timeout returns to INT.

## 14. Owner questions (65-73; also in OWNER_QUESTIONS.md)

65. NOTE EDIT chance has two levels (100 / 50 %) on LFO so it stays one press. Accept, or move PROB to a knob with
    four levels (it would push RATCH onto ARP / LFO presses, and a 4-step press cycle fails the 2-gesture rule)?
66. A new page is an exact copy of **the page before it** (A, A', A''): its edits and its source (the page whose
    generator block it reads; `OWN` lets the algorithm continue). Confirm.
67. A step's SLIDE means "this note glides into the next step" (TB-303 rule; after the last step, into step 1 across
    the loop end; no glide if that step is silent). Confirm on the TB-3 (with question 8).
68. Force sensing ships behind the HT-FORCE gate with `FORCE OFF` until it passes, in Velo's DRIVE mode (+~2.2 ms on
    a REC note); the v1 set is hard-tap accent (scored 9 and kept as optional extra E3), pressure ROLL / FILL and REC
    velocity. Accept this set?
69. Scale, key and octave live on a SCALE page opened by **holding GLO** (SET stays on the tap). The printed card says
    "hold SCL: a key sets the root", but a hold on SCL would delay the NOTES dice to the release, and the GLO hold
    already passes the bar. Accept?
70. SNAKE's long polymeter cycles are cut at the channel length (at most 64 steps), so the lane always shows what
    plays. Accept?
71. Exception E1: a preset turned while playing lands on the first bar that starts at least 1.5 s after the last
    detent (decision 34 plus the save-window settle; O1 by the letter of the rubric). Keep (recommended by the
    reviewer and here: a quantised launch with a visible countdown is the musical norm), or load at the first step
    after the settle (`SQ_LOAD_AT = STEP`, scores 11)?
72. The L rule is restated (8.1): a role reached through a hold scores L2 when the hold is named permanently on the
    page where it starts (the new `h:` sub-labels on PERF) and opens a labelled page; otherwise L1 as in P1-014.
    Accept? Under the P1-014 letter 23 setup functions fall to 9 and 43 of 69 pass (8.5); under the restated rule 66
    of 69 pass with three listed exceptions.
73. Default MIDI channels: CH1..CH4 send on MIDI 1..4 (owner ruling 2026-11-10: the 2-5 default was REJECTED), and
    step 1 of the first run is `D#4 K2: MIDI CHANNEL`, so the TB-3 (factory channel 2) is two visible gestures away. ANSWERED.

77. (P2-011) MIDI channels are unique across the four channels; a taken one turns the other channel's MIDI OFF (4.4). ANSWERED.
78. (P2-011) Setup functions may take two gestures, performance functions one. ANSWERED.

## 15. Clarifications from the scenario walk (P2-011; nothing here changes a locked decision)

The 17 scenarios of `docs/design/scenarios/` found places where this document was silent or two readings were possible.
Each reading below is the one the existing text already implies; owner questions 77 (unique MIDI channels) and 78 (gesture budget) are decided, 79 (a never-saved slot) is open (OWNER_QUESTIONS.md).

| Id | Topic | Statement |
| --- | --- | --- |
| F-10 | NOTE_EDIT ENV, the second press | The step's `VELO` edit equals `ACC_VEL` 120 after the first press. The second press clears it: back to AUTO when the generated layer does not make this step an accent, back to the explicit non-accent velocity 80 when the generator would make it an accent (AUTO would give the accent straight back, and the press must be seen to do something). U23 reads "120 / AUTO-or-80" in that sense |
| F-11 | FILL readout | `FILL` density = K1 + (max - K1) x p, with p = 0.5 at the lightest accepted pressure (FS3 "light = halfway") rising linearly to p = 1.0 at firm pressure, clamped. The readout is the resulting density as a percentage of the algorithm's maximum for K1: `FILL 80%` = 0.8 x max. With force sensing off (the fallback) p = 1.0, so the readout is `FILL 100%` |
| F-12 | NOTE_EDIT first detent from AUTO | The first detent in either direction moves the field **from the generated value rounded to the knob's grid**, one grid step in the turning direction, and the field becomes a set value (`*`). PITCH: one scale degree; GATE: the 10 % grid (AUTO, 10 .. 100 %, TIE), so from a generated 50 % the first clockwise detent gives 60 %; TIE is the top of the range and takes (top - start) detents (the knob's fast mode steps 4); RATCH: 1x..4x; VELO: 1 step (fast 8). The AUTO word of a first-detent field is replaced by the number at once. |
| F-13 | Adding a page and FOLLOW | A page added by PG+ moves the view to the new page by hand, which sets FOLLOW off exactly as PG- / PG+ do (1.3). The lane header shows `FOLLOW OFF` and the toast is `PAGE 2 = COPY OF PAGE 1: CHANGE IT` (the page toast outranks `VIEW P2 . FOLLOW OFF` for the same press) |
| F-16 | Redo of a load | SAVE hold twice: the first is `LOAD UNDONE: P03`, the second `LOAD REDONE: P01` (the slot loaded again; `LOAD UNDONE: Pnn` names the slot returned to) |
| F-19 | RESET, OCT- / OCT+ on CHAN, SET, SCALE | inert, with the toast `NOTHING TO RESET`, like every other inert control of RESET. MENU: the keys stay inert without a toast because the menu has its own screen and hints |
| F-17 | Stale ACID text | ALGORITHM_SET 5.2: ACID K2 is `SLIDE` (this document 4.2); `PEDAL` is ROW 2 S2. The ACID dice sub-labels are those of ALGORITHM_SET 4.4 (`NOTES`, `ACCNT`, `OCTAVE`, `GT+SLD`, `HITS`); SCREEN_SPEC 7 and the mock-up of 2.1 are corrected to it |
| F-04 | Strings | header channel without the `CH` prefix (`1 ACID M12`), muted cell `MUTE n`, K1 cell `KEYS`, `48 (32)`: SCREEN_SPEC 2.1, 2.2, 5. The mock-ups of this document print the 40-column sketch `CH1 ACID M1`; SCREEN_SPEC 2.1 is the pixel authority |
| F-14 | Duplicate MIDI channels | decided by the owner (OWNER_QUESTIONS 77): unique across the channels, the displaced channel goes OFF, section 4.4 |
| Q1 | The gesture budget | decided by the owner (OWNER_QUESTIONS 78, option a): setup functions may take 2 gestures, performance functions stay 1; R-5 is the meaning of VISION rule 1 and is written into VISION.md |
