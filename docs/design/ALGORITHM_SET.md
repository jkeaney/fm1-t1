# Algorithm set for v1, the shared layer vocabulary and the knob convention

> Change note (P1-013): the knob convention gains a second row. K1..K4 (ROW 1) are unchanged; ROW 2 is S1 START (`OFFSET`), S2 ARTIC, S3 DYNAMICS, S4 READ, giving 8 morphs per algorithm (section 5.2b, 5.3). Shared primitives SP16 (`dir_map`) and SP17 (`dyn_vel`) are added to section 6. The five-layer dice vocabulary (section 4) is unchanged. Details: [FULL_CONTROL.md](FULL_CONTROL.md).

> **Change note (P1-012, 2026-11-10).** Amended to match [UX_REVIEW.md](UX_REVIEW.md) and
> [control_map.tsv](control_map.tsv) (owner decisions S1-S7). Changes: (1) the 7 dice inputs are bound to buttons:
> SCL tap NOTES, SEQ HITS, ENV ACCNT, ARP ARTIC, LFO SHAPE, FX dice-all, EDIT tap undo/redo (4.3). (2) Every dice
> button shows a second-line sub-label with the algorithm's own layer name (4.4). (3) K1 ranges start at 0
> (`KEYS ONLY`: only pins play); ACID K2 `TYPE` is relabelled `PEDAL` (5.2). (4) S1: ACID ARTIC dices GATE and SLIDE in
> one press, no separate slide gesture in v1 (N1 resolved). (5) S4: SP6 snap also applies to pins and live keys. (6)
> S6 confirmed: MINPEN, key C, SNAKE RANGE 12. (7) S7: per-algorithm morphs and seeds are kept per channel when the
> algorithm changes (N2 resolved). (8) Global OCTAVE (-2..+2) joins KEY and SCALE in SP9. Approval of P1-012 re-approves
> this document.

Task P1-010, amended by P1-012. Status: **FOR OWNER REVIEW together with P1-012. Not approved.** Every owner-question answer in section 8 is a
**recommendation awaiting the owner**. Nothing here is locked until the owner writes "Approved: <name> <date>" in the
P1-010 log (this file) and later "Design locked" per ADR-0004.

Inputs read: LESSONS.md, VISION.md, ADR-0004 (and its addendum), OWNER_QUESTIONS.md (all 30 questions and the
decisions table), the seven assessments in `docs/design/algorithms/`, CONTROL_INVENTORY.md. Per ADR-0004 addendum,
everything below is a data table so that a post-hardware workflow change is a table edit (section 10 lists the flex
points). This task is design only: no code, no edits to the seven assessments (section 5.4 lists the edits they need).

Naming: algorithm names on screen are `ACID`, `EUCLID`, `SNAKE`, `STAGE` (max 6 characters, VISION rule 1 labels).
Source assessments call them sting, t1, snake, ml185.

## 1. Summary of decisions (one table)

| # | Decision | Section |
| --- | --- | --- |
| 1 | v1 ships **4 algorithms**: ACID, EUCLID, SNAKE, STAGE. Deferred: ARP, RHYTHM, MELODY. | 2, 3 |
| 2 | **5 shared layers** (dice buttons): NOTES, HITS, ACCNT, ARTIC, SHAPE. Every v1 algorithm maps onto them; unmapped layer = dimmed, inert. | 4 |
| 3 | **Knob convention:** K1 = DENSITY, K2 = FORM, K3 = PITCH, K4 = SPICE. Four of the algorithms need small reorder edits (not EUCLID). K1 = 0 means `KEYS ONLY` (P1-012). | 5 |
| 4 | Implementation order: shared primitives, then ACID, EUCLID, SNAKE, STAGE. | 7 |
| 5 | 15 shared primitives are specified once; no algorithm re-specifies them. | 6 |
| 6 | SLIDE keeps its own seed and its own undo, and is dice-grouped with GATE under `ARTIC` (one ARP press). Resolved as S1 in UX_REVIEW.md 5.3 by score (one press 9; a slide-only hold 7 fails). | 4.3, 9 |
| 7 | Physical binding of the 7 dice inputs and every other control: UX_REVIEW.md section 1 and control_map.tsv. | 4.3 |

## 2. The v1 list

Scores are the 0 to 3 "fit against the vision" totals from the assessments (5 criteria, max 15).

| Order | Algorithm (screen) | Source | Assessment verdict | Fit total / 15 | Why it ships in v1 | State per channel |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | `ACID` | sting + P1-011 slide | INCLUDE | 14 | The direct match to the reference workflow (TB-3 acid line): a defaulted 16-step line plays on selection, has the richest layer set (all 5 shared layers, 6 seeds) and so exercises the whole dice model. Accent, gate and slide mirror the TB-3's own model. | ~258 B |
| 2 | `EUCLID` | t1 | INCLUDE | 14 | The only rhythm-first algorithm: hits placed by even spacing (Bjorklund), so no knob position sounds random. Code (`t1_euclidean_bjorklund`, `_rotate`, `t1_harmony_quantize`, ratchet maths) already runs on the device, so it is cheap. It is also the answer to owner question 21. | ~129 B |
| 3 | `SNAKE` | snake | INCLUDE | 14 | The only structural algorithm: 14 traversal shapes re-order the same material with one knob, and independent pitch, velocity and rhythm lane lengths give polymeter. Maps 4 morphs and 4 dice layers one to one. | ~122 B |
| 4 | `STAGE` | ml185 | INCLUDE | 13 | The only algorithm whose rhythm is the durations of note slots: loop length is exact (8 to 64 pulses), ratchets and long holds are built in, and it is stateless (every change lands within one pulse). | ~193 B |

Notes:
- All four INCLUDE verdicts are in v1, so criterion 1 of the task holds. No INCLUDE is deferred.
- 4 is inside the 3 to 6 limit. Reasons not to go to 5 or 6 (ARP, MELODY, RHYTHM): section 3.
- Total channel state if every channel stored all four algorithms is about 700 B; storing only the active algorithm
  is about 260 B. Either way it goes in `.pool` (360 KB, initialised explicitly), so the 4 KB `.bss` headroom rule is
  untouched. How many channels and whether state is per algorithm is for the engine ADR (P2-004); see section 9.

## 3. Deferred and rejected

| Item | Verdict | Reason | Revisit trigger |
| --- | --- | --- | --- |
| `ARP` (steparp) | LATER | Defining input (held keys) does not exist; rhythm half (HITS, VEL, RATCH) duplicates SNAKE/STAGE; new content is only CHORD, ORDER, OCT. A 2 to 4 note pool is repetitive as a bass. Stateless, ~170 B, so promotion is cheap later. | Hardware test says the four algorithms are too melodically flat for chord work. Starting code is `t1_harmony_eval_chord/_voicing/_style` (t1 6.8). |
| `RHYTHM` (rhythmic-steps) | LATER | Pitchless drum-voice use only (scores 1 on the TB-3 workflow); mechanisms are re-skins of rank-order density, ratchet, velocity lane, lane length; has no Euclidean placement (EUCLID covers that). | Owner makes a drum-voice channel a goal (question 20). |
| `MELODY` (melodic-steps) | LATER | Lane-per-parameter polymeter already in SNAKE/STAGE/ACID; velocity, gate, octave, rest lanes are re-skins of shared primitives. Its two genuinely new things (contour walk, per-note lane clock) are deferred, see below. | Hardware test says uniform-random or EUCLID-contour pitches are not bass-like enough (question 5, 22). |
| Contour **walk** (LEAP/SPAN) as a NOTES pitch source | LATER (post-v1) | Needs a morph slot on SNAKE/STAGE and K2/K3 are taken; v1 already has contour **shapes** in EUCLID (existing code). Adds a second contour primitive for little gain before hardware feedback. | Question 5 fails on the TB-3. Declared as primitive SP10 mode `WALK`, off in v1. |
| Per-note lane clock `hc` | LATER | Changes lane indexing from `t` (all four v1 algorithms) to hit count; would invalidate golden files. | MELODY promoted (question 23). |
| NUDGE (reversible small mutate) | LATER | Would need a sixth/seventh dice gesture; the 5 layers + dice-all + undo already need 7 inputs from P2-001. | Hardware feedback says full re-roll is too coarse (question 19). |
| Groove-template HITS order (RHYTHM) | LATER | A second order source for the shared rank-order primitive; changes golden files for ACID/SNAKE/STAGE. | After hardware test, as a table flag on SP2 (`order_source`). |
| CC / modulation lane (SNAKE Custom lanes) | LATER | Rule 3: a channel has MIDI channel + algorithm only. Needs the engine to carry CC events. | Post-v1 (question 6). |
| Source features dropped everywhere | REJECT | Free-running random (jitter, CakeWalk, Classic, LFSR modulation), direction modes, per-lane speed/swing/note length, chord size/voicing/arp style on EUCLID, 6 tracks x 6 cycles, p-locks, user scales. They break rule 1 (menu) or rule 4 (same seed = same pattern). Per-assessment drop lists stand. | n/a |
| Steps beyond 16 on the white keys | LATER | Keys reach 16 pulses (STAGE, EUCLID, ACID). Long loops (STAGE 17 to 64) come from the generator only. | Hardware test (questions 10, 18, 26). |

## 4. Shared layer vocabulary (the dice buttons)

Rule: **the dice button means the same thing in every algorithm.** The screen always shows these five names on the
dice buttons, whatever the algorithm. Each algorithm's own internal layer names (ACID `OCT`, STAGE `LENS`, ...) appear
only in specs and in the diced-layer toast, never as the button label. At most 5 layers: exactly 5.

### 4.1 Definitions

| Layer (button label) | One-line meaning (shown in help) | Rule for "does it belong here" |
| --- | --- | --- |
| `NOTES` | Which pitches play (the melody). | Re-rolls pitch content only. Never changes which steps sound, loudness or length. |
| `HITS` | Which steps sound (where the rhythm falls). | Re-rolls the choice of sounding steps / rests (or, for a fixed-spacing rhythm, its phase). Never changes pitch. |
| `ACCNT` | How loud each note is (the dynamics). | Re-rolls velocity/accent only. |
| `ARTIC` | How each note is played in time: length, glide, repeats, holds. | Re-rolls per-note articulation. Never changes which steps sound or pitch. |
| `SHAPE` | How the loop is laid out: register jumps, slot lengths, lane drift. | Re-rolls structure that is none of the above four. |

### 4.2 Mapping matrix (button -> internal seed(s) of each algorithm)

"-" = the algorithm has no such layer (behaviour in 4.4). Seed ids are those in each assessment's 6.1 table.

| Shared layer | ACID | EUCLID | SNAKE | STAGE |
| --- | --- | --- | --- | --- |
| `NOTES` | `s_notes` (NOTES) | `s_notes` (NOTES) | `s_notes` (NOTES) | `s_notes` (NOTES) |
| `HITS` | `s_hits` (HITS) | `s_rot` (ROT, the phase of the evenly spaced hits) | `s_gates` (GATES) | `s_rests` (RESTS) |
| `ACCNT` | `s_acc` (ACCNT) | `s_acc` (ACCNT) | `s_vel` (VEL) | `s_vel` (VEL) |
| `ARTIC` | `s_gate` (GATE) **and** `s_slide` (SLIDE), one press re-rolls both | `s_rat` (RATCH) | - | `s_mode` (MODE: which stages ratchet or hold) |
| `SHAPE` | `s_oct` (OCT, register jumps) | - | `s_poly` (POLY, velocity-lane length) | `s_lens` (LENS, which stages are long) |
| Seeds in the algorithm | 6 | 4 | 4 | 5 |
| Layers present of 5 | 5 | 4 | 4 | 5 |

Coverage check: every assessed layer maps to exactly one shared layer (ACID 6 of 6, EUCLID 4 of 4, SNAKE 4 of 4,
STAGE 5 of 5, no orphans). MELODY, ARP and RHYTHM layers also map (deferred check): MELODY NOTES/RESTS/OCT/GATE/ACCNT ->
NOTES/HITS/SHAPE/ARTIC/ACCNT; ARP CHORD/ORDER -> NOTES, HITS -> HITS, VEL -> ACCNT, RATCH -> ARTIC; RHYTHM HITS/CHANCE ->
HITS, ALT -> NOTES, VEL -> ACCNT, ROLL -> ARTIC. So adding the deferred three needs no sixth layer.

### 4.3 Dice actions

| Action | Effect | Per algorithm |
| --- | --- | --- |
| Dice layer L | Replace the seed(s) bound to L with the low 16 bits of the audio sample counter at the press (EUCLID ROT: forced to 1 if 0). Store the old seed as the "previous" seed. | A layer bound to 2 seeds (ACID `ARTIC`) re-rolls both with two different counter reads (second read XOR 0x5A5A) and stores both previous seeds. |
| Dice all | Dice every layer present for the active algorithm in the order NOTES, HITS, ACCNT, ARTIC, SHAPE. Skips absent layers. | ACID 6 seeds, EUCLID 4, SNAKE 4, STAGE 5. |
| Undo dice | Recall each seed's previous seed once (one level). A second undo does nothing (previous == current). | Per seed, so undoing ACID `ARTIC` restores GATE and SLIDE together. Undo of a dice-all restores all layers. |
| Physical buttons | NOTES = SCL tap (acts on release; SCL hold = scale layer), HITS = SEQ, ACCNT = ENV, ARTIC = ARP, SHAPE = LFO, dice-all = FX, undo = EDIT tap (acts on release; a second tap redoes: undo swaps each seed with its previous seed). Dice buttons without a hold act on press. Source: UX_REVIEW.md 1.1, 5.2. | all |

Owner decision 2 ("ACID gets slide as a sixth dice layer") is met as a sixth **seed** on five **buttons** (S1, resolved
in P1-012). SLIDE has its own seed, its own previous seed and its own probability table, exactly as P1-011 specifies;
only the button is shared with GATE. If the owner wants SLIDE alone on its own gesture, the table-only fix is a
second binding row (for example `ARTIC` long press = SLIDE only), which costs no algorithm change (section 10).

### 4.4 When an algorithm has no such layer

| Case | Behaviour (normative) |
| --- | --- |
| Algorithm lacks layer L (EUCLID `SHAPE`; SNAKE `ARTIC`) | The button label is drawn dimmed with a dash after it (`SHAPE -`). Pressing it changes nothing: no seed changes, no undo entry, no sound change. A 0.5 s toast reads `NO SHAPE IN EUCLID`. LED unchanged. Dice-all and undo skip it. |
| Sub-label (P1-012, S2) | Under every dice button's shared name the screen draws the algorithm's own layer name, max 6 characters: ACID `NOTES HITS ACCNT GT+SLD OCTAVE`; EUCLID `NOTES ROTATE ACCNT RATCH -`; SNAKE `NOTES GATES VELO - DRIFT`; STAGE `NOTES RESTS VELO RT+HLD LONG`. A sub-label equal to the main label is drawn once. |
| Why not hide or remap it | Rule 1: a control keeps its position and meaning; a missing layer is honest and learnable. Never remap a button to a different layer per algorithm. |
| Switching algorithm while a layer is dimmed | The label un-dims when the new algorithm has the layer. The layer table is the only thing read (`layer_present[algo][layer]`). |

## 5. Knob convention (K1 to K4)

### 5.1 The convention

| Knob | Role name (never shown, used in specs) | Always means | Musical test (a producer should be able to say this) |
| --- | --- | --- | --- |
| K1 | DENSITY | How many notes play: fewer to more. | "Turn K1 up for busier, down for sparser." |
| K2 | FORM | The length or character of the loop: how long it is, or how much of it repeats versus varies. | "K2 changes how the pattern is built." |
| K3 | PITCH | How wide the pitch movement is: range, span, register jumps. | "K3 up for bigger melodic reach." |
| K4 | SPICE | The expressive extra on top of the pattern (accent, ratchet, reading path). | "K4 for energy." |

Each algorithm keeps its own on-screen label (max 6 chars) and range; the convention fixes the **slot**. Knobs stay
reversible and monotonic as each assessment specifies. The screen draws knob slots left to right K1..K4 for ROW 1 and S1..S4 for ROW 2 (both rows always visible, FULL_CONTROL.md 2).

| Knob (ROW 2) | Role name | Always means | Musical test |
| --- | --- | --- | --- |
| S1 | START | Where the loop begins (`OFFSET`, a rotation of what is read). | "Slide the whole pattern along the bar." |
| S2 | ARTIC | How much articulation: slide, ratchet, gate-lane length, ratchet speed. | "More glide / more repeats." |
| S3 | DYNAMICS | How loud and how different: accent depth, velocity spread. | "Flatter or punchier." |
| S4 | READ | The order the content is read in: direction, or contour curve. | "Play it backwards / another shape." |

Every ROW 2 default is the neutral value, so the output equals the 4-morph design until a ROW 2 knob is turned.

### 5.2 Mapping and final parameter table (after the edits in 5.4)

| Algorithm | K1 DENSITY | K2 FORM | K3 PITCH | K4 SPICE |
| --- | --- | --- | --- | --- |
| `ACID` | `DENS` 0..16, default 12 | `PEDAL` (was `TYPE`) 0..12, default 6 | `JUMP` 0..20, default 5 | `ACCNT` 0..8, default 3 |
| `EUCLID` | `HITS` 0..16, default 5 | `LEN` 2..16, default 16 | `SPAN` 0..24, default 12 | `ACCNT` 0..8, default 2 |
| `SNAKE` | `DENS` 0..16, default 10 | `LEN` 1..16, default 7 | `RANGE` 0..48, default **12** (was 24) | `PATH` 1..14, default 7 |
| `STAGE` | `DENS` 0..8, default 6 | `TOTAL` 8..64, default 16 | `RANGE` 0..36, default 12 | `RATCH` 0..8, default 2 |

Fit argument per cell (where it is not obvious):

| Cell | Why it fits |
| --- | --- |
| ACID K2 `PEDAL` (was `TYPE`) as FORM | ACID is fixed at 16 steps, so FORM cannot be a length. PEDAL sets how much of the line is the repeated root versus the raw motif, which is the loop's character (0 = raw motif, 12 = hammering acid). |
| ACID K3 `JUMP` as PITCH | Octave-jump probability is the only pitch-width control ACID has (register reach). |
| SNAKE K2 `LEN` as FORM | Pitch-lane loop length against the fixed 16-step rhythm: how the loop is built (polymeter). |
| SNAKE K4 `PATH` as SPICE | The most dramatic stepped change per detent (re-orders melody and rhythm at once), the "twist" control. |
| STAGE K2 `TOTAL` as FORM | Exact loop length 8 to 64 pulses, one pulse per detent. |
| STAGE K4 `RATCH` as SPICE | Repeats on top of the pattern. |
| EUCLID K2 `LEN` as FORM | Loop length and polymeter. K2 stays `LEN`, not `ROTATE` (question 28). |
| EUCLID K1 `HITS` as DENSITY | Bjorklund pulse count; every value is a groove, but it is still "how many". |

### 5.2b ROW 2 parameters (P1-013)

| Algorithm | S1 START | S2 ARTIC | S3 DYNAMICS | S4 READ |
| --- | --- | --- | --- | --- |
| `ACID` | `OFFSET` 0..15, 0 | `SLIDE` 0..8, 4 | `PUNCH` 0..47, 40 | `DIR` 0..2, 0 |
| `EUCLID` | `OFFSET` 0..15, 0 | `RATCH` 0..8, 4 | `PUNCH` 0..47, 40 | `CURVE` 0..6, 0 |
| `SNAKE` | `OFFSET` 0..15, 0 | `GLEN` 1..16, 16 | `DYN` 0..10, 10 | `DIR` 0..2, 0 |
| `STAGE` | `OFFSET` 0..63, 0 | `RDIV` 1..4, 4 | `DYN` 0..10, 10 | `DIR` 0..2, 0 |

Formulas, restored source parameters and the list of parameters that stay cut: FULL_CONTROL.md section 3.

### 5.3 Knob step size

| Knob | Rule |
| --- | --- |
| STAGE `TOTAL` (57 values) | Step of 4 pulses when turned fast (detents under 40 ms apart), 1 when turned slowly (P1-013 rule, FULL_CONTROL.md 5.1). Every value 8..64 stays reachable. |
| K1 at 0 (all four) | `KEYS ONLY`: no generated hits, pins still play (rank-order `n = 0` plays nothing; `t1_euclidean_bjorklund(steps, 0)` returns 0). Range edit only. |
| All other morphs | One detent = one step of 1 when turned slowly. Ranges over 32 values (STAGE `TOTAL` and `RANGE`, SNAKE `RANGE`, `PUNCH`, STAGE `OFFSET`) change by 4 per detent when detents arrive under 40 ms apart (FULL_CONTROL.md 5.1); the menu's KNOB ACCEL no longer affects morphs. |

### 5.4 Edits the assessments need (table of edits, not applied here)

Applied by the P2-008 prototype task or a doc-edit task after the owner approves. Section numbers are in each file.

| File | Section | Edit |
| --- | --- | --- |
| sting.md | 6.1 + section 6 bullets | Knob slots: `den` becomes K1, `typ` becomes K2 (swap); `jmp` K3 and `acc` K4 unchanged. Label order in text: K1 `DENS`, K2 `TYPE`, K3 `JUMP`, K4 `ACCNT`. |
| sting.md | 3, 6, 6.6 | Add a column "Shared layer": NOTES->`NOTES`, HITS->`HITS`, OCT->`SHAPE`, GATE and SLIDE->`ARTIC` (one press dices both seeds), ACCNT->`ACCNT`. |
| sting.md | 6.1, 6.5 | Pins: store `pin_note[16]` (semitone offset from base, 0..96) instead of `pin_pitch[16]` pitch class 0..11, to match SNAKE/STAGE/EUCLID (primitive SP8). The pin plays as its pitch class in octave 0 by default; octave comes from the key, as in the others. |
| t1.md | 3, 6 | Add the shared-layer column: ROT->`HITS`, NOTES->`NOTES`, ACCENT->`ACCNT`, RATCH->`ARTIC`; `SHAPE` absent. Knobs unchanged. |
| t1.md | 6.8, 7.7 | Change the shared snap from "down only" to nearest with ties to the lower pitch class (primitive SP6); `t1_harmony_quantize` is changed once for all algorithms. |
| snake.md | 6.1, section 6 | Knob slots: `den` K1 (was K3), `len` K2 (unchanged), `rng` K3 (was K4), `shp` K4 (was K1). |
| snake.md | 6.1 | `rng` default 24 becomes **12** (question 5: a bass-sized default; one octave). |
| snake.md | 3, 6 | Shared-layer column: NOTES->`NOTES`, GATES->`HITS`, VEL->`ACCNT`, POLY->`SHAPE`, `ARTIC` absent. |
| ml185.md | 6.1, section 6 | Knob slots: `den` K1 (was K2), `tot` K2 (was K1), `rng` K3 (was K4), `rat` K4 (was K3). |
| ml185.md | 3, 6 | Shared-layer column: NOTES->`NOTES`, RESTS->`HITS`, VEL->`ACCNT`, MODE->`ARTIC`, LENS->`SHAPE`. |
| all four | 6.1 | Default seeds: keep each file's `0x3A5D * (ch*N + k)` formula (it defines golden files); the shared dice code reads the seed from the table, never assumes N. |
| steparp.md, rhythmic-steps.md, melodic-steps.md | 3 | No edit now (deferred). The mapping in 4.2 is the record that they fit the same 5 layers. |

## 6. Shared primitives (deduplicated list for the dev tasks)

One implementation each, in a shared module included before the algorithms (unity build order). "Used by" is the v1
set; (L) = deferred algorithm. Golden tests per primitive are written once (P2-008).

| Id | Primitive | Definition (normative source) | Used by | Duplicates folded in |
| --- | --- | --- | --- | --- |
| SP1 | Seeded RNG | xorshift32 `next()`, `rnd(n) = (next()>>8) % n`, Fisher-Yates `shuffle`, `init(seed16, salt)` (sting 6.2). Each layer builds its own stream from its own seed + salt; salts per algorithm as in each 6.2. | all | Replaces the T-1 LFSR (F16, F19). |
| SP2 | Rank-order density | Shuffle an order of N cells with a seed; cell plays iff `rank < n`; option `first_fixed` (cell 0 ranks first: ACID, STAGE; SNAKE has none). N = 16 steps, or 8 stages. Order source flag `order_source` = `shuffle` (v1), `groove` (LATER). | ACID HITS+DENS, SNAKE GATES+DENS, STAGE RESTS+DENS, (L: ARP, MELODY, RHYTHM) | The "rank-order density" duplicated in 6 of 7 assessments. |
| SP3 | Seeded accent count | "The first `a` cells in an accent-seed order get velocity 120, the rest 80". Reuses SP2 over the **hit** cells (ACID: hit steps; EUCLID: hit ordinals). | ACID ACCNT, EUCLID ACCNT | Fold ACID and EUCLID accents into one function with a `cell_set` argument. |
| SP4 | Velocity lane | `vel[L]` random 50..127 from a seed, read as `vel[t % lane_len]`; `lane_len` fixed (STAGE: 8 stages) or diced (SNAKE POLY 3..12). | SNAKE VEL+POLY, STAGE VEL, (L: ARP, RHYTHM, MELODY) | SP3 vs SP4 is the one deliberate split: two-level accent (count knob) vs continuous lane (dice only). |
| SP5 | Retrigger-in-step | Given a base step, `count` (2..4), spacing in ticks and velocity slope, emit repeat events. Selection "first `n` eligible firing cells in a seeded order" is SP2 again. EUCLID adds the pace curve (accelerating or decelerating, `t1_pace_curve_q15`); STAGE re-fires every `div` pulses inside a stage. | EUCLID RATCH, STAGE RATCH/MODE, (L: ARP, RHYTHM) | One emitter, two parameter sets. |
| SP6 | Scale quantise | `quantise(pitch_class_or_semitone)`: **nearest scale tone by circular pitch-class distance, ties to the lower**, applied at output to generated notes, pins and live keys (S4, P1-012); scale and root from the single global control; `pitch_orig` is kept so a scale change re-snaps reversibly. Table `T1_SCALE_MASKS` (F1 fixed, F2 residue: delete stale `T1_SCALE_NAMES`). | all | Replaces `t1_harmony_quantize` "snap down" (changes behaviour; question 16). |
| SP7 | Span mapping | `p = r * rng / 255` for a raw 0..255 value then SP6 (late quantise), so scale size does not change the span. | SNAKE RANGE, STAGE RANGE | One function, two ranges (0..48, 0..36). |
| SP8 | Pins | `pin_mask` u16 + `pin_note[16]` (offset 0..96 semitones from base 36), set from the white keys. A pin overrides the generated note and always sounds; it does not move the generator's state. A pin on a step at or past the loop length is silent until the length rises. | all | ACID `pin_pitch` is unified to this (5.4). |
| SP9 | Note mapper | `midi = clamp(36 + root + 12*octave + semitone, 0, 127)` (`octave` = global OCTAVE -2..+2, P1-012; it is the global transpose). Base note 36 for every algorithm. `root`, `transpose`, `gate`, `div`, `swing` are the **global** settings (owner decisions 3 and 4); an algorithm owns none. | all | Base 36 was repeated in 5 assessments. |
| SP10 | Contour (pitch shapes) | Mode `SHAPES` (v1): the 6 phrase shapes SINE, TRI, SAW U, SAW D, ARCH, INV A + seeded jitter +-2, anchored on the root, via `t1_harmony_eval_phrase` after F3 is fixed. Mode `WALK` (LATER): LEAP/SPAN reflected walk starting on the root. | EUCLID (SHAPES), (L: MELODY WALK) | The two contour ideas of EUCLID and MELODY are one primitive with a mode flag. |
| SP11 | Bjorklund + rotate | `t1_euclidean_bjorklund`, `t1_euclidean_rotate` reused unchanged (cap the `groups[]` array at 16 entries; do not change output). Rotation `R = r16 mod LEN`. | EUCLID | n/a (single user; kept shared so a later placement option reuses it). |
| SP12 | Step clock | `t` = step index derived from the clock, `t mod len`; muting keeps phase (T1 F10 fixed). Lane index is per step `t`, not per sounding note (question 23). Changes commit at the next step boundary; STAGE commits at the next pulse. | all | Repeated in 5 assessments. |
| SP13 | Seed store | Per layer seed + previous seed (u16 each) and `dice(layer)`, `dice_all()`, `undo()` driven by the layer table of section 4.2 (`layer_seeds[algo][layer] -> list of seed ids`). Default seed = `(0x3A5D * (ch*N + k)) & 0xFFFF`, with `N` and `k` from the algorithm table. | all | The dice/undo code was copied into every assessment. |
| SP14 | Output scheduler | Emits note-on, note-off at `gate_pct` of the step (global GATE), overlap for slides (ACID, P1-011 6.3a), long hold (STAGE `I-`, ignores global GATE per question 11), and panics (note-off + CC123 on stop). Note-off pairing is per channel; at most one sounding note per channel except slide overlap. | all | Start/stop/CC123 handling from `t1_glue.c` is the model. |
| SP15 | Parameter table reader | Reads morph name, range, default, knob slot, and `layer_seeds` from data tables (section 10); the algorithm functions take plain scalars (`den`, `len`, ...), never the table. | all | Makes remaps a table edit (ADR-0004 addendum). |
| SP16 | Direction map (P1-013) | `dir_map(u, n, d)`: `d = 0` FWD `u mod n`; `d = 1` BWD `n - 1 - (u mod n)`; `d = 2` PEND (ping-pong without repeating the ends): `n = 1` gives 0, else `q = u mod (2n - 2)`, result `q` if `q < n` else `2n - 2 - q`. | ACID (n 16), SNAKE pitch lane (n = LEN), STAGE (n = TOTAL) | Direction in sting, snake, ml185 sources (random modes stay cut). |
| SP17 | Velocity depth (P1-013) | `dyn_vel(v, dyn) = clamp(100 - ((100 - v) * dyn) / 10, 1, 127)`, division truncating toward zero; `dyn = 10` returns `v`, `dyn = 0` returns 100. | SNAKE, STAGE | Velocity lane range in the sources. |

Primitives deliberately not shared: ACID's motif tile + TYPE replacement (ACID only), SNAKE's 14 traversal shapes
(SNAKE only), STAGE's cumulative-sum stage lookup (STAGE only), ACID's slide probability table (ACID only), ARP's
hashed chance roll (deferred).


## 7. Implementation order

Order is for the dev tasks that P2-005 creates after design lock; P2-008 prototypes (golden files) follow the same
order. Each step ends with its golden test passing before the next starts.

| Step | Work | Depends on | Why this position |
| --- | --- | --- | --- |
| 0 | Shared primitives SP1, SP2, SP3, SP4, SP6, SP7, SP8, SP9, SP12, SP13, SP14, SP15 with golden tests. | ADR-0003, locked global controls (P2-001, P2-004) | Everything else is built on them; settles the scale (F1, F2) and quantise direction once. |
| 1 | `ACID` | step 0 | Reference workflow, uses all 5 layers and 6 seeds, so it validates the layer table, undo, the slide overlap and the 5-button model. Fixes the display and scenario shape for the rest. |
| 2 | `EUCLID` (+ SP5, SP10 SHAPES, SP11) | step 1; F3 fixed first | Smallest delta: existing verified code, rhythm-first alternative to ACID's random-order hits, first test of an absent layer (`SHAPE -`). |
| 3 | `SNAKE` (+ 14 traversal tables) | step 1 | Adds table data and lane lengths, no new core mechanism; second absent-layer case (`ARTIC -`). |
| 4 | `STAGE` | steps 1 to 3 | Most complex: variable loop length 8 to 64, cumulative-sum lookup, long holds, TOTAL knob step sizes, SP5 as pulse re-fire, within-one-pulse commit. Built last on a proven core. |
| 5 (not v1) | `ARP`, `RHYTHM`, `MELODY`, WALK, NUDGE | step 4 + hardware feedback | Section 3. |

## 8. The 30 owner questions: recommendations

**These are recommendations awaiting the owner. None is a decision.** "Resolved" means the OWNER_QUESTIONS decisions
table already answers it. "Hardware" means it cannot be answered without the device.

| # | Short form | Recommendation | Reasoning | Status |
| --- | --- | --- | --- | --- |
| 1 | ACID without slide acceptable? | Moot. Slide is in v1 (owner decision 2). | P1-011 adds `s_slide`; this doc keeps its own seed, grouped under `ARTIC`. | Resolved by decision 2 |
| 2 | Scale snap vs ACID's own | ACID owns no snap: it uses the single global scale and the shared quantiser SP6; `pitch_orig` is kept so changes re-snap reversibly. | Owner decision 3; one function, one behaviour. | Recommend yes |
| 3 | GATE/DIV/SWING global? | Global. | Owner decision 4. | Resolved by decision 4 |
| 4 | Default scale, is "off" allowed | Default **minor pentatonic**, root C; chromatic stays selectable as "off". | Random pitches (SNAKE, STAGE, ACID) never clash on pentatonic, which gets a usable bass in seconds (rule 5). Sparse-scale flattening matters only for chords (deferred). | Recommend, then hardware |
| 5 | Uniform random pitch OK as a bass? | Yes for v1 **with** two mitigations: SNAKE `RANGE` default 12 (was 24, 5.4) and the pentatonic default (4). Contour walk stays LATER (3). | STAGE already defaults to 12; ACID has TYPE root-hammering; EUCLID has contour. Bass-likeness is a hardware-test judgement. | Recommend, then hardware |
| 6 | Custom lanes back as a CC lane? | Post-v1 only. Not in the v1 list. | Rule 3, and the engine has no CC event path yet. | Recommend LATER |
| 7 | SLIDE has no amount control OK? | Yes for v1: dice `ARTIC` until slide count is acceptable (fixed ~16% of steps). | All 4 knobs taken by the convention (5.2). A second binding row can give SLIDE its own gesture (section 10). | Recommend yes |
| 8 | Does overlapping-note slide glide on the TB-3? | Check at the first hardware test. Fallback CC65 (portamento on) is a post-v1 CC path. | Cannot be known on the host. | Hardware |
| 9 | Dice/knob change takes effect next step | Yes, accept (commit at the next step boundary, up to ~125 ms at 120 BPM, 250 ms at 60 BPM); STAGE commits per pulse. | Consistent with SP12 and with rule 4 (no phase loss). Hardware test watches it. | Recommend yes, then hardware |
| 10 | STAGE pins reach pulses 0..15 only | Yes, accept for v1. | Long loops come from the generator; a page/shift on the keys would be a menu (rule 1). Revisit with P2-003 only if hardware shows a need. | Recommend yes |
| 11 | STAGE long holds ignore GATE | Yes, accept; keep a table constant to flip it. | A hold that follows GATE would no longer be a "long" note. | Recommend yes |
| 12 | STAGE defaults good on a TB-3? | Keep (TOTAL 16, DENS 6, RATCH 2, RANGE 12, base C2); tune at hardware test. Note: with the convention swap (5.2) K1 is DENS=6. | Table values, a cheap edit. | Hardware |
| 13 | Promote ARP to a fourth algorithm? | No. ARP stays LATER; CHORD/ORDER are not offered as a pitch option to other algorithms in v1. | EUCLID is the better fourth algorithm (question 30); ARP duplicates the rhythm half. | Recommend no |
| 14 | 2..4-note chord pool enough? | Moot while ARP is LATER. | n/a | Deferred |
| 15 | Chord snap on sparse scales turns major to minor | Moot while ARP is LATER; revisit when ARP is promoted. | n/a | Deferred |
| 16 | Chord-tone snap direction | **Nearest, ties to lower**, for the shared quantiser (SP6) so it behaves the same everywhere. | "Down only" flattens low SPAN values on sparse scales (t1 risk 7). | Recommend |
| 17 | RHYTHM fixed base note 36 / alt +12 | Moot while RHYTHM is LATER; base 36 + alt +12 stays the proposal. | n/a | Deferred |
| 18 | Pin at or past LEN silent | Yes, accept (same shape as 10). The key shows dimmed while its step is past the loop length. | Keeps pins without a menu. | Recommend yes |
| 19 | NUDGE in the global vocabulary | No for v1. | v1 already needs 7 dice inputs (5 layers + all + undo); adding NUDGE adds modifier gestures. Revisit after hardware. | Recommend LATER |
| 20 | Drum voice a v1 goal? Lift groove-template order? | No drum-voice goal; RHYTHM stays LATER. Groove-template order stays a LATER `order_source` flag on SP2. | v1 is judged by the TB-3 line (VISION). A new order source changes golden files. | Recommend no |
| 21 | Rework Euclidean into v1? Groove-template as second density source? | Yes to EUCLID in v1 (algorithm 2 of the order). No to the second density source in v1. | Only placement-by-spacing algorithm; code exists and runs. | Recommend yes / no |
| 22 | Contour walk as a NOTES option for SNAKE/STAGE? | Yes, post-v1, as primitive SP10 mode `WALK`; not in v1. | No free morph slot on SNAKE/STAGE (K2, K3 taken by the convention); revisit after the hardware answer to question 5. | Recommend LATER |
| 23 | Note lanes advance per note or per step | **Per step (`t`)**, shared primitive SP12. | Consistent across all four algorithms, simpler, keeps golden files. Per-note clock waits for MELODY. | Recommend per step |
| 24 | Typed step consumes a contour note? | No (pins never move generator state, SP8). In v1 the only contour user is EUCLID, where pins do not consume a hit ordinal. | Typed notes are additions; the tune stays in order. | Recommend no |
| 25 | SNAKE + LEAP instead of MELODY? | No for v1. | Conditional on question 22 being yes now; recommendation for 22 is LATER. | Recommend no |
| 26 | EUCLID max 16 steps? | Yes, 16 for v1 (LEN 2..16); 32/64 later with a page/shift design. | Matches the 16 white keys and the 16-bit mask; same reasoning as 10. | Recommend yes |
| 27 | RATCH as a fixed-odds dice with no knob? | Yes for v1: dice `ARTIC` until a ratchet pattern is liked. | K4 slot is ACCNT by the convention (5.2); STAGE keeps a RATCH knob because its ratchet is its identity. | Recommend yes |
| 28 | ROTATE as a knob replacing LEN? | No. Keep K2 = `LEN`; rotation is the `HITS` dice. | Convention: K2 = FORM; LEN gives the polymeter that no other algorithm has on its hit pattern. | Recommend no |
| 29 | Root-anchored contour or centred? | Keep root-anchored (never below root, base 36, ceiling 73); global transpose shifts it. Check on the TB-3. | A bass should not go below its root; one table constant to change. | Hardware |
| 30 | EUCLID the fourth algorithm, or only a placement source? | EUCLID as its own algorithm. | Its identity is placement + ratchet + accent as one coherent set; making it a source inside another algorithm would blur the 5-layer mapping. | Recommend yes |

New questions raised by this task (append to OWNER_QUESTIONS.md after review):

| # | Question | Recommendation |
| --- | --- | --- |
| N1 | SLIDE shares the `ARTIC` button with GATE. | **Resolved (S1, P1-012):** one press dices both; no separate gesture in v1. |
| N2 | Algorithm switch: keep or reset morphs and seeds? | **Resolved (owner S7):** kept per algorithm per channel; switching back restores them. Pins are kept. Storage is all four algorithms per channel (~700 B per channel, `.pool`). |
| N3 | Which buttons carry the 7 dice inputs? | **Resolved (P1-012):** section 4.3 and UX_REVIEW.md 1.1. |

## 9. Conflicts and hand-offs

| Item | Detail | Goes to |
| --- | --- | --- |
| Owner decision 2 vs 5-layer limit | See 4.3 and N1. | Owner |
| Quantiser behaviour change | `t1_harmony_quantize` snap-down becomes nearest-ties-lower; EUCLID goldens are generated from the new behaviour. Reviewer check: the existing `tests/torso_phase2_test.c` snap assertions (lines 212-224: A# in A P-MIN -> A, C# and D# in C major -> down) still hold under the new rule (all are ties or nearer-below). It does change today's T-1 pipeline output (`torso_t1_pipeline.c` 262, 323) whenever the nearer scale tone is above, e.g. B in C P-MIN now goes up to C. | P2-008 (prototype), dev task for SP6 |
| F3 (ARCH and INV A broken) | Must be fixed before EUCLID's SPAN is correct (t1 7.3). | EUCLID dev task, first step |
| Seven dice inputs | Bound in P1-012 (4.3); P2-001 copies control_map.tsv into the firmware table. | P2-001 |
| Absent-layer display | Dimmed label rule in 4.4. | P2-003 (screen spec) |
| Per-algorithm state storage | Active-only (~260 B) vs all four (~700 B) per channel, in `.pool`. | P2-004 |
| Slide behaviour on TB-3 | Question 8. | First hardware test |
| `T1_SCALE_NAMES` stale table | Delete; use the UI's names (t1 risk 4). | Dev task for SP6 |
| Assessment edits | Table 5.4 is not applied by this task. | P2-008 / doc-edit task after approval |

## 10. Flex points (ADR-0004 addendum)

What can be changed by editing a table row, with no algorithm code change:

| Table | Row/field | Change possible by table edit |
| --- | --- | --- |
| Layer vocabulary (4.1) | Names, one-line help | Rename a layer label (max 6 chars) or its help text. |
| `layer_seeds[algo][layer]` (4.2) | Seed list per cell | Move a seed to another layer (for example EUCLID `HITS` and `SHAPE` swap), split ACID `ARTIC` into two rows (SLIDE alone on a long press), bind two seeds to one layer, add a dim/absent cell. |
| `layer_present[algo][layer]` (4.4) | 0 or 1 | Mark a layer absent or present. |
| Knob table (5.2) | Slot, label, range, default | Remap any K1..K4 to another morph of that algorithm; change a label, range or default (for example SNAKE `RANGE` default 12 to 24). |
| Knob step table (5.3) | Step per detent, accel | Change TOTAL's coarse step. |
| Constants (SP9, 6) | Base note 36, velocities 120/80, `gate_pct`, ratchet odds, slide odds | Retune. |
| Default scale (question 4) | Scale id, root | Change default from minor pentatonic to anything. |
| Dice actions (4.3) | Order of dice-all, second-seed XOR constant | Reorder, retune. |
| `control_map.tsv` (P1-012) | control x context -> action, label | Any physical binding, including a slide-only gesture, moving UNDO, re-placing channel keys. |
| Dice sub-labels (4.4) | text per algorithm and layer | Rename. |

What cannot be changed without code:

| Item | Why |
| --- | --- |
| The xorshift generator, the salts, the shuffle method | They define the golden files (SP1). |
| The count of shared layers (5) and the layer meanings in 4.1 | Defines the dice contract that every algorithm and the control map rely on; a sixth layer needs a new ADR, a new button and a new screen row. |
| Order-based monotonic and reversible morph principle | Core guarantee (VISION: undoable by turning back). |
| 16-step grids (ACID, EUCLID, SNAKE), 8 stages and 64 pulses (STAGE) | Cache sizes and pin masks. |
| Per-step lane indexing (`t`) | Would change golden files (question 23). |
| Adding a new generator (contour WALK, groove order, per-note clock) | New algorithm code (they are declared as LATER primitive modes). |

Change path (ADR-0004): edit the table, re-run scenarios and prototypes, owner re-locks, then a small Haiku task
updates the table. No change in this document requires touching algorithm code except the items in the second table.

## 11. Acceptance check

| Criterion | Where met |
| --- | --- |
| Every INCLUDE recommendation is in the v1 list or deferred with a reason | Section 2 (all four INCLUDE verdicts in v1); LATER verdicts in section 3 |
| Layer vocabulary at most 5, every v1 algorithm maps onto it | Section 4.1 (5 layers), 4.2 (ACID 5, EUCLID 4, SNAKE 4, STAGE 5; no orphan seeds) |
| Knob convention with mapping and concrete fixes | Section 5.1, 5.2, 5.4 |
| Implementation order and deferred list | Sections 7 and 3 |
| Shared primitives deduplicated | Section 6 |
| Owner questions answered by recommendation | Section 8 (all 30 plus N1 to N3) |
| Owner approval line | **Not present. Awaits the owner.** |
