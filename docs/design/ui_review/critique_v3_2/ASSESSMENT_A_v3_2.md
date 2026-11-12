# Assessment A: design review of UI proposal v3.2 (glance)

Reviewer role: design director, independent (NOTES.md, the earlier critiques and Evidence B were not read).
Inputs: PRODUCT.md, the impeccable critique method (Assessment A scope), every scene listed in the brief at 1x and 4x, the four
contact sheets, `docs/controls.jpg`, `docs/design/button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, and `proposal_v3_2.py`
where the drawing rule mattered (lane encoding, cue table, map timing).

Viewing-distance yardstick (brief): AF_S caps 9 px = 7.6 arcmin, AF_M caps 11 px = 9.6', AF_L 21 px = 18'. Floor for a glance
read about 10'; below 5' invisible at arm's length. A 4 px rule = 0.5 mm = about 3' tall: fine for *detecting* a coloured line,
useless for *reading* anything.

---

## AI-slop / generic-hardware verdict

**Not AI slop. Mildly generic.** Nobody would say "a model made this": there is no glow, no gradient, no rounded-card phone
chrome, one accent hue that means one thing ("now": playhead, play state, playing page, sounded channel), and a real
module grid (15 px horizontal, 4 px vertical) that the source measures. It reads as a competent Torso/Elektron derivative.

Where it goes generic: it is **tile soup**. Four rows of identical flat grey rectangles with 2 px gaps (channel tiles, lane
columns, knob tiles, ROW cells), all on the same 4-column, 60 px pitch. Identity comes only from the orange. An Elektron or
Torso user would trust it at first sight and then pause at, in order: the bars (they look like pitch, they are velocity),
`1→2`, the cyan foot under some steps, the ROW strip, `P01` (pattern or preset?), `SAVE: KEEP HERE`, `UNDO L`, `ALGO OF CH 1`,
`GLO SET`, and every black-key chip (`M2`, `SOL`, `CHN`, `RST`), whose names aren't printed on the panel. That is about
ten pauses. The bar for this category is close to zero.

---

## Overall impression

The glance screen is the best so far. **It does not regress to "too much, too small"**: no AF_S on the glance screen,
the four knob values in AF_L (18') dominate, and the playhead is a full-height orange column that survives greyscale and
blur. Toasts are big, name the printed button first, and say what changed ("CH1 NEW PITCHES", "NO ARTIC IN SNAKE").
The cue line that follows the hand works as an idea.

What doesn't work: **the biggest graphic on the screen tells the producer the wrong thing.** Lane bar height is velocity
(`bar_h(vel)` in the source), not pitch. For the target workflow, an ACID bassline into a TB-3 where accent rather than velocity
carries the dynamics, the bars will be read as a pitch contour. They don't move when you dice NOTES, and soft notes shrink to
4 px stubs that look like rests (`lane_vel_low`). Second, the **channel setup step of the acceptance workflow has no screen**:
CHN opens a CHAN page where K1..K4 = LENGTH, MIDI, SOURCE, FOLLOW, but the proposal keeps drawing DENS, SLIDE, JUMP, ACCNT on the
knob cards while the cue says "K2 = MIDI CHANNEL". Third, the **risky moments still have holes**: the preset-pending toast
labels an overwrite "KEEP HERE", and a selected channel that is muted, MIDI OFF and in REC gives no warning.

**Single biggest opportunity:** make the lane show what the producer is shaping: pitch, with velocity and accent as secondary marks.
Then the dice NOTES / SHAPE / ACCNT / ARTIC buttons would each produce a visible change on the main screen, and the toast
would confirm the change instead of being the only evidence.

---

## Design Health Score

| # | Heuristic | Score | Key issue |
|---|-----------|-------|-----------|
| 1 | Visibility of system status | 3 | Playhead, play state, pending preset with settle bar, mute and solo are all clear. Gaps: CLOCK LOST appears only in the transient cue line (the header still shows `▶ 120`), the CHAN page has no visible state, and dicing NOTES produces no lane change. |
| 2 | Match system / real world | 2 | Velocity bars read as pitch. Jargon with no inline gloss: `1→2`, DENS, ARTIC, HITS, PEDAL, PUNCH, GLEN, SOL/FIL/ROL/RST/CHN, `UNDO L`, `SET`, `HARM`, `OF CH 1`. Producers will learn most of it, but a first-time user can't decode `1→2`. |
| 3 | User control and freedom | 3 | Undo/redo toggle on EDIT with a large toast, SAVE-hold undoes a preset load, HOME cancels pending prompts, SOL/FIL/ROL are momentary. Undo is one level (toggle), and nothing says so. |
| 4 | Consistency and standards | 2 | Cyan means ACCENT (cap) and EDITED (foot). The cue chip claims to be "the printed name", but seven of its values name unprinted black keys. The three knob rows use three card styles. The map puts the 6x2 button panel into a 4x3 grid. The pending toast says `SAVE: KEEP HERE` where the spec says `SAVE = save here`. |
| 5 | Error prevention | 2 | `SAVE: KEEP HERE` overwrites the pending slot with the current state (button_modes B_SAVE tap). A muted, MIDI OFF channel accepts REC input with no warning. A MIDI channel steal sets another channel to OFF, and the toast doesn't name it (the spec says it must). The HOME release model is unresolved: per the spec, release after 0.7 s means MENU. |
| 6 | Recognition rather than recall | 3 | Knob roles, dice roles (idle cue) and the last control are always named. Black-key roles and the lane legend are recall only: they live on MAP B. |
| 7 | Flexibility and efficiency | 3 | OCT-hold peeks a row, force on FIL/ROL, touch-for-help while HOME is held, PG-/PG+ with FOLLOW. Missing: a channel-switch confirmation and any overview of all four patterns. |
| 8 | Aesthetic and minimalist design | 3 | Clear hierarchy (AF_L values, orange playhead). Reference data competes at equal AF_M weight: four MIDI routings, four algorithm names, the ROW strip, a static idle cue. |
| 9 | Error recovery | 3 | Good copy: "PAST THE END: CHN K1 = LENGTH", "NO ARTIC IN SNAKE / NOTHING CHANGED", "CLOCK LOST: OWN TEMPO 120". The PAST THE END fix points at a page the proposal never draws. |
| 10 | Help and documentation | 3 | HOLD-HOME map plus touch-to-explain is the right architecture. Map A's printed names are AF_S (7.6') in MID grey, the smallest and dimmest text on the page, and the map layout doesn't mirror the panel. |
| **Total** | | **27/40** | **Acceptable** (top of the 20–27 band). The foundation is solid; the remaining issues are semantic, not cosmetic. |

---

## Cognitive load assessment

| # | Check | Result | Evidence |
|---|-------|--------|----------|
| 1 | Single focus | PASS | The lane and the knob values own 132 of 240 px. Toasts take over one band at a time. |
| 2 | Chunking (≤4 per group) | PASS | 4 channel tiles, 4 knobs, 16 steps shaded in groups of 4, 3–4 header items. |
| 3 | Grouping | **FAIL** | All three zones share one 4-column, 60 px grid, so proximity and alignment group **columns**: the CH1 tile, lane steps 1–4 and the K1 card stack up as if related. They aren't. The lane belongs to the selected channel only, and nothing links the lit tile to the lane (see `mutes`: CH3 selected, but steps 1–4 still sit under CH1). |
| 4 | Visual hierarchy | PASS | AF_L values and the orange column win. The selected tile is inverted. |
| 5 | One thing at a time | PASS | One toast or one cue at a time. Toasts don't stack. |
| 6 | Minimal choices (≤4) | **FAIL** | Dice decision: 6 buttons (the cue lists all 6). Black keys: 11 roles. Map A: ~30 control tiles. Much of this is intrinsic, but the dice six are presented flat with no ordering. |
| 7 | Working memory | **FAIL** | Black-key roles (M1–M4, CHN, SOL, FIL, ROL, PG±, RST) and the lane legend (cyan foot = edited, digit = ratchet, tie = slide, bar = velocity, which isn't in the legend at all) must be carried over from MAP B. The MIDI steal changes another channel off screen. |
| 8 | Progressive disclosure | PASS | Map on hold, toasts on use, row 2/3 on OCT+. |

**3 failures: moderate cognitive load. Address soon.**

Item counts per decision point (counted on the renders):

| Decision point | Visible items | Verdict |
|---|---|---|
| Glance, playing, at rest (`glance_playing`) | **24 text items**: header 3 (`120`, `P01`, `1`), tiles 8, ratchet digits 2, knob labels 4 + values 4, ROW cells 2, cue 1 (8 tokens). Plus ~45 graphic marks: 16 bars, 4 accent caps, 3 ties, 2 edited feet, 2 activity bars, 4 meters, transport, page underline, playhead | Text count is OK. Marks are dense but structured |
| Glance, stress (`stress`) | **~31 text items** (`EXT 126`, `P16`, 4 page numerals, 8 tile texts, `M`, 4 ratchet digits, 4 labels + 4 values, 2 ROW cells, REC chip + cue) plus SHAPE dot plot and RATE pips | At the limit. Nothing below AF_M |
| Which layer to dice | 6 | Over: needs an order or grouping (rhythm vs pitch vs feel) |
| Which knob (per row) | 4 (x3 rows = 12) | OK |
| Mute | 4 keys, 4 tiles | OK |
| Preset pending | 3 (wait, SAVE, HOME) | OK, but one is mislabelled |
| Map A | ~55 text items in 26 tiles | Overloaded, by design a reference page |
| Map B | ~35 text items | Heavy |

---

## Emotional journey (TB-3 acceptance run)

1. **Power on / first run** (`state_first_run`): a populated pattern, `▢ 120`, a boxed `→1` on CH1, and the cue `CHN K2 = MIDI CHANNEL`.
   Good intent: the screen tells you the one thing to do. **Valley:** "CHN" is not printed anywhere on the panel. It is
   the 5th black key (D#4), and only MAP B says so. The producer has to hold HOME, page with OCT+, find CHN, let go, find
   the key. Meanwhile the K2 card reads `SLIDE 6`, which contradicts the cue.
2. **Set MIDI 2** (`toast_midi_channel`): big `MIDI 2 / CH1 SENDS ON MIDI CHANNEL 2`. Reassuring. **Hidden cost:** per
   knob_map CHAN K2, CH2 (which defaulted to MIDI 2) has just been set to OFF. The toast doesn't say so, and the producer finds out
   later when CH2 is silent.
3. **Choose algorithm** (`toast_algo`): `EUCLID / ALGORITHM CH1` plus a 6-pip position strip, and the cards change at once.
   One of the best moments.
4. **Dial in and dice** (`toast_dice_*`): **peak.** A large printed-name chip and the layer, with a plain sentence of what changed and
   `EDIT: UNDO` in the corner. **Flat spot:** for NOTES and SHAPE the lane doesn't visibly change, because pitch isn't drawn, so the
   peak rests entirely on a 1.2 s toast.
5. **PLAY**: orange triangle, orange page box, orange playhead, orange activity bars. Unmistakable.
6. **Perform: morph** – the card value changes in place in AF_L. Excellent. **Dice again**: same peak. **Mute** (`mutes`): the tile
   goes dark with an outline and an `M` chip, the activity bar disappears, and the cue says `CH2 MUTED TAP AGAIN = ON`. Four signals,
   good. **Switch channel**: the tile highlight moves and the cards relabel, but no toast or cue confirms "CH3 SNAKE"
   (no scene shows one).
7. **Undo** (`toast_undo`): `EDIT UNDONE / CH1 NOTES / EDIT AGAIN = REDO`. Strong reassurance. But nobody is told it is one level deep.
8. **Preset change** (`toast_preset_pending`, `toast_preset_loading`): `P04? ` in the header with a settle bar, a big `P04 LOADS AT BAR`
   with a countdown, then `P04 ▶ LOADING / SAVE HOLD = UNDO LAST LOAD`. This is the most anxious moment of a set and it is mostly
   well handled, **except** that the pending toast offers `SAVE: KEEP HERE`, which a performer reads as "stay where I am". It actually
   **overwrites P04** with the current state (B_SAVE tap while pending). The deepest valley in the run, and it is
   destructive.
9. **Stop / end**: cue `PLAY STOP: ALL NOTES OFF`. A calm, trustworthy ending.

Peak-end: the peak (dice toast) and the end (stop) are both good, so the run will be remembered fondly. The valleys are at
setup (CHN), at the MIDI steal and at preset change, which are exactly the moments the brief says need reassurance.

---

## What's working

1. **The knob band is the right hero.** Four AF_L values (18') with a 6 px meter, enum values drawn as pictures (DIR ramp, SNAKE
   PATH, ARP SHAPE dots, RATE pips), and a row identity carried by the whole card style (row 2 inverted label band, row 3
   boxed). At arm's length you can read every value, and you can tell which row is live without reading a word.
2. **The playhead and the colour discipline.** One accent hue means "now" everywhere: transport, playing page, sounded channel,
   playhead column. The full-height orange column with the step drawn in ink inside it survives deuteranopia, protanopia,
   tritanopia (it turns pink but stays the brightest object), greyscale and blur (`lane_accent_test.png`). Shape carries state
   alongside colour: accent is a triangle, mute is an outline plus `M`, solo is hatching, past-length is hatching, a hold role is a box.
3. **Toasts that lead with the printed button and say the consequence.** `SCL NOTES / CH1 NEW PITCHES`, `ARP ARTIC / NO ARTIC IN
   SNAKE` (dimmed, "NOTHING CHANGED"), `EDIT UNDONE / CH1 NOTES / EDIT AGAIN = REDO`, `ADD PAGE 3 / PG+ AGAIN: COPY OF 2 HOME: NO`.
   Each one teaches the button and confirms the action. This is how a panel-only instrument should document itself.

---

## Priority issues

### [P1] The lane draws velocity, not pitch: the main graphic misleads and dicing NOTES/SHAPE is invisible
- **What:** bar height = `bar_h(vel)` (velocity 1..127 → 4..36 px). There is no pitch anywhere on the glance screen. In `lane_vel_low`
  soft notes are 4 px stubs, indistinguishable from the 2 px rest baseline at 50 cm (0.5 mm, about 3'). MAP B's legend doesn't say
  what bar height means.
- **Why it matters:** every producer coming from a 303/TB-3, Push, OP-1 or T-1 reads a bar lane under a melodic channel as a pitch
  contour. ACID's dynamics are carried by accent, so velocity bars mostly carry no information. When the producer dices NOTES
  ("new pitches") or SHAPE ("new octave jumps"), the pattern view doesn't change. The toast is the only proof, and it lasts 1.2 s.
  This breaks product rule 1 ("the screen always shows what the device is doing") for the core performance gesture.
- **Fix:** draw pitch as the bar's vertical **position**: a 13x4 note block on a per-pattern pitch range that auto-fits the
  lowest to highest note in the 56 px band, with an octave line. Carry velocity as block brightness or a 2-level thickness, and
  keep the accent triangle. Rests stay empty columns. Then dice NOTES/SHAPE/HITS/ACCNT/ARTIC each change a different visible
  mark, and the toast confirms the change instead of replacing it. Add "HEIGHT = PITCH" to the MAP B legend.
- **Suggested command:** `/impeccable shape` (then `/impeccable clarify` for the legend).

### [P1] Channel setup (CHN page) has no screen, and the knob cards lie while it is open
- **What:** key_map: the CHN key (black key 5) tap opens the CHAN page, with K1 LENGTH, K2 MIDI, K3 SOURCE, K4 FOLLOW. In
  `state_first_run` the cue reads `CHN K2 = MIDI CHANNEL` while the K2 card reads `SLIDE 6`. `state_midi_channel` and
  `state_past_length` ("CHN K1 = LENGTH") show the same DENS/SLIDE/JUMP/ACCNT cards. No scene renders the CHAN page. The MIDI toast
  also omits the channel the steal set to OFF (knob_map CHAN K2: "the toast names it").
- **Why it matters:** this is step 1 of the acceptance workflow (select channel 1, set MIDI 2 for the TB-3). PRODUCT.md principle 2:
  "a role that changes shows its new role while it is changed". It is violated exactly here. Silently turning CH2 OFF
  means the producer later "loses" a channel mid-session.
- **Fix:** while CHAN is latched, relabel the knob band `LENGTH 16 / MIDI 2 / SOURCE OWN / FOLLOW ON` with a distinct card style
  (like rows 2/3) and a cue `CHN CHANNEL SETUP  CHN AGAIN = BACK`. The MIDI toast takes the second line when a steal happens:
  `CH1 → MIDI 2 · CH2 NOW OFF`. Draw the scene.
- **Suggested command:** `/impeccable harden`.

### [P1] Risky moments mislabelled or unflagged: `SAVE: KEEP HERE`, silent selected channel, transient CLOCK LOST
- **What:**
  (a) `toast_preset_pending` says `SAVE: KEEP HERE`. Per button_modes B_SAVE tap, while a slot is pending SAVE **saves the current
  state into the pending slot** (P04) and cancels the load. "Keep here" reads as "stay on P01, cancel", so P04 is destroyed by the
  button the performer believes is safe. HOME is the real cancel.
  (b) `state_sel_muted_off`: the selected channel is muted **and** MIDI OFF **and** in REC. The cue says `REC KEYS WRITE NOTES`.
  The only signs that nothing will sound are a 9.6' `M` and `4--`.
  (c) `state_ext_lost`: the header keeps `▶ 120` with no EXT tag or warning tint. The only signal is the cue line, which the next
  control touch overwrites.
- **Why it matters:** these are the moments the brief calls high-stakes (preset, mute, stop, clock). A performer who loses a preset or
  plays into a dead channel on stage stops trusting the instrument. That is support-ticket territory, so at least P1.
- **Fix:** (a) `SAVE: OVERWRITE P04   HOME: CANCEL`, or better, make SAVE during pending do nothing destructive and remove it
  from the toast. (b) When the selected channel can't sound, the cue takes a persistent warning that outranks REC:
  `CH4 SILENT: MUTED + MIDI OFF`, and the lane shows its hollow-bar mute style (it already does). (c) Show a sticky header state:
  `INT 120` with the transport shape outlined, until the clock returns or the user acknowledges.
- **Suggested command:** `/impeccable harden` (then `/impeccable clarify` for the copy).

### [P2] One 4-column grid makes channels, step quarters and knobs read as columns of the same thing
- **What:** channel tiles, lane groups of 4 steps and knob cards all sit on the identical 60 px pitch, with the same tile
  shape and gap. The lit channel tile isn't connected to the lane. In `mutes` (CH3 selected) steps 1–4 and DENS still sit
  directly under the CH1 tile.
- **Why it matters:** Gestalt alignment wins over intent. A first-timer, or a producer glancing mid-set, reads "DENS 12 belongs to
  ACID/CH1, SLIDE 6 to EUCLID/CH2", and the lane as "four steps per channel". Every glance has to suppress that wrong reading.
- **Fix:** separate the channel row from the lane and knobs. Either make the channel row visually lighter (no tiles: four inline
  `1 ACID · 2 EUCLID ...` labels with mute shapes), or connect the selected tile to the lane with a continuous selected-colour
  edge or tab, so the lane hangs off its owner. Consider dropping the MIDI routing from unselected tiles (it is setup information).
- **Suggested command:** `/impeccable layout`.

### [P2] The MAP doesn't mirror the panel, hides the printed names, and its MENU timing contradicts the spec
- **What:** the physical panel (`docs/controls.jpg`) has the 12 buttons in **two rows of six** (FX SCL ENV LFO EDIT GLO / HOME SAVE
  ARP SEQ PLAY REC), the encoders as a **2x2** block and K1–K4 in a line. Map A puts the buttons in **three rows of four**, so EDIT and GLO
  appear under FX and SCL and HOME sits third in row 2, and it lays the encoders out 1x4. Printed names (the only link to the hardware)
  are AF_S, 7.6', in MID grey, the smallest and dimmest text on the page. Timing: the source has `MENU_HOLD_MS, MENU_COUNT_MS = 1500, 1000`
  (hold, countdown, MENU while held). button_modes B_HOME says **release** after 0.7 s with nothing touched opens MENU. Under the spec,
  every leisurely look at the map ends in the MENU.
- **Why it matters:** a map is a spatial artefact, and a reshuffled one forces a name lookup instead of a glance down at the
  same place. The timing conflict decides whether the map is safe to use at all.
- **Fix:** lay Map A out in the panel's geometry: a 6x2 button block, the encoders 2x2 on the left, OCT under them, K1–K4 across the top
  right. Printed names in AF_M, roles AF_M, holds in boxes. Resolve the HOME model with the owner and update button_modes or the
  proposal so they agree (the brief's "hold longer for MENU" plus a countdown is the safer one).
- **Suggested command:** `/impeccable layout` (then `/impeccable document` to sync button_modes.tsv).

---

## Persona red flags

**Alex (power user, knows Elektron/Torso):**
- Bursts of dice raise the 40 px toast slab over and over (y 196–236). It hides the ROW strip and the bottom of the knob meters for
  the whole burst, and gives no option to shorten it.
- Undo is a one-level toggle (`EDIT AGAIN = REDO`), and the screen never says so. Alex will try two undos and get a redo.
- There is no overview of the other three channels' patterns. The activity bar shows only "sounded".
- `GLO = SET` and `GLO hold = HARM` hide BPM, swing, scale and key, which a T-1 user expects one press away. Acceptable, since MENU is for
  set-and-forget, but `SET` says nothing about what it sets.

**Sam (low vision, poor viewing angle, colour-blind):**
- Every label on the glance screen except the values is AF_M at 9.6', just under the 10' floor, including the selected channel's
  **algorithm name**, which the brief names as essential.
- Cyan does two jobs (accent cap, edited foot). In greyscale the edited foot is a slightly lighter 5 px strip, effectively invisible
  (`lane_accent_test`, greyscale rows).
- At an off-axis angle the dark-on-dark distinctions collapse first: inactive ROW cell text, unselected vs muted tile fill, the two
  alternating lane shades. Mute survives through the outline and `M`, but the `M` chip is 13 px.
- Low-velocity notes (4 px) disappear entirely.
- Positive: the playhead, accent shape, mute outline, solo hatching and past-length hatching never depend on hue.

**Jordan (first-timer):**
- `1→2` (channel → MIDI channel) and `4--` (MIDI off) are explained only on MAP B page 2.
- The cue chip `M2`, `SOL`, `CHN`, `RST`, `PG-` names keys with **no printed label**. The stock panel silkscreen visible in
  `docs/controls.jpg` shows different faint legends above the black keys, so the cue may name a key that is printed as something else.
  Needs owner verification on the unit.
- `FX SCL ENV LFO ARP SEQ = DICE` tells Jordan which buttons dice but not what "dice" means, and the printed words (FX, SCL, ENV) now mean
  something else.
- `HOLD HOME = MAP` on the idle cue is the right onboarding hook, but it is replaced by the dice line as soon as PLAY is pressed.

**Dana (studio producer, hands busy on knobs and keys, glancing for under a second):**
- She turns K2, glances: `SLIDE 6`, AF_L, perfect.
- She dices NOTES, glances at the lane: nothing changed (velocity bars). She glances lower: the toast may already be gone.
- She taps the 2nd black key to mute CH2, glances: the dark outlined tile with `M`, plus the cue. Good.
- She switches channel with SELECT: the tiles change and the cards change, but there is no word confirming *which* channel is now live
  except the lit tile, whose algorithm name is the same AF_M size as the three unlit ones.
- She sets MIDI 2 for the TB-3 and doesn't notice CH2 went OFF.

---

## Minor observations

- **Mock bugs that would mislead an owner review:** `lane_muted` shows CH1 muted (the `M` is on the selected tile) while the cue reads
  `M2 CH2 MUTED`, and CH2 is visibly sounding. `cue_mute_held` renders the *tap* cue (`TAP AGAIN = ON`), not the defined
  `mute_hold` cue `HOLD = PREVIEW CH2`. `toast_undo`/`toast_redo` show an identical lane before and after. `state_first_run` shows a
  fully edited lane (cyan feet), which is not a first-run state.
- `TAP AGAIN = ON` is ambiguous. `TAP AGAIN = UNMUTE` fits (6 characters).
- `UNDO L` on Map A is a truncation. The cue and toast spell it `UNDO PRESET LOAD` / `UNDO LAST LOAD`. Pick one wording. `LOAD-` or `UNLOAD`
  fit 6 characters.
- `ALGO OF CH 1` is awkward copy. `CH1 ALGO` or just `ALGO` is better.
- The ghost playhead (`lane_offpage`, `state_follow_off`) is an orange outline on a page that is **not** playing. In the "now" hue it
  reads as "this step is playing". Draw it in MID grey, or as a tick on the page box instead.
- In REC the transport triangle is replaced by a red ring (stopped) or a red dot (playing). Play vs stop then depends on fill vs
  outline of a 13 px glyph. Keep the triangle or square and add the red dot beside it.
- The ROW strip uses a full 16 px band to say what the card style already says, and its right half is empty. Merge it into the knob
  band (a 3-pip row index in the label band), and give the 16 px to a larger selected-channel name.
- The cyan EDITED foot is the smallest mark on the lane, yet uses the second strongest hue. Use MID grey and keep cyan only for accent.
- Map B header `TOUCH = HELP < OCT- = BUTTONS` runs to the right edge. Two messages in one line.
- The PRESETS map tile says `P01-16`, but the pending flow (Pnn?, settle, loads at bar) is the important behaviour and isn't hinted.
- Black keys M1–M4 have a hold role (PREVIEW, key_map PRV1–PRV4) that MAP B doesn't show (no boxed sub-label).
- The SNAKE PATH picture and the `7` beside it: the digit is AF_M next to a 31 px picture, so the number is the weaker element.
  Fine, but the meter is missing on that card while the others have one.

**Control-documentation check against the TSVs (9 checked):**

| Screen label | Source | Verdict |
|---|---|---|
| FX ALL / SCL NOTES / ENV ACCNT / LFO SHAPE / ARP ARTIC / SEQ HITS | button_modes PERF B_FX..B_SEQ | Correct |
| EDIT UNDO, box RESET | B_EDIT tap/hold | Correct |
| GLO SET, box HARM | B_GLO tap/hold | Correct (but uninformative) |
| SAVE SAVE, box UNDO L | B_SAVE tap/hold | Correct, truncated |
| Pending toast `SAVE: KEEP HERE` | B_SAVE tap: "while a slot is pending, save to the PENDING slot and cancel its load" | **Wrong / dangerous wording** |
| HOME CANCEL, box MENU, MENU countdown while held | B_HOME hold: "release after 0.7 s with nothing touched = MENU" | **Contradicts the spec** (proposal: 1.5 s hold + 1 s countdown) |
| OCT- ROW 1, OCT+ ROW+ | B_OCTDN / B_OCTUP | Correct |
| ACID row 2 OFFSET PEDAL PUNCH DIR, ARP row 2 OFFSET LEGATO DYN DIR (CHORD), SNAKE K1–K4 DENS LEN RANGE PATH | knob_map PERF | Correct, and the meters match the ranges (SLIDE 6/8, JUMP 5/20, PUNCH 40/47) |
| Black keys M1 M2 M3 M4 CHN, SOL FIL ROL (boxed = hold), PG- PG+ RST, grouping 3-2-3-2-1 | key_map PERF KEY_B01..B11 | Correct; M1–M4 hold PREVIEW missing |
| `CHN K2 = MIDI CHANNEL`, `CHN K1 = LENGTH` | knob_map CHAN K1/K2 | Correct mapping, but the cards on screen contradict it (P1 above) |

---

## Verdict on density

**No regression to "too much information, too small"**, with one exception.

- **Glance:** 24 text items when playing, about 31 in the stress case. **No AF_S anywhere.** The smallest text is AF_M (9.6', at the
  floor), and the information the brief cares most about is the largest: values at 18', the playhead full height. The owner's two
  earlier rejections had many sub-floor labels. This one has none. What remains is *equal-weight reference data*: four MIDI
  routings and four algorithm names at the same AF_M as the knob labels, the ROW strip, and a static idle cue. That makes about 12
  items the brief didn't ask for on the glance. Cutting the unselected routings and the ROW strip would bring it near 18.
- **MAP A/B: yes, this is where the owner's complaint will recur.** About 55 and 35 text items, with the hardware-identifying names at AF_S
  (7.6'). It is a deliberate reference page, so density is more forgivable, but it is the screen that looks most like the
  rejected versions.

---

## Questions to consider

1. What if the lane showed **pitch** and the dice buttons each lit up the mark they change for one bar? Would the toasts then
   need to be 40 px tall?
2. Does an unselected channel need its MIDI routing on the glance screen at all, or is `1→2` setup information that belongs on the CHAN
   page and a small mark only when it is OFF?
3. Should SAVE do anything while a preset is pending? What is the cost of making the pending state accept only HOME (cancel) or
   time (load)?
4. If the map mirrored the panel 1:1, would it need the printed names at all, or would position alone carry them?
5. What does the screen say when the producer is about to play into silence? Is there one "you won't hear this" state that
   outranks every cue?
6. Is the four-column alignment a feature (one knob per channel, T-1 style) that the firmware might grow into? If not, why keep
   the grid that implies it?

---

## Element verdicts

| Zone | Element | Size / distance | Verdict | Note |
|---|---|---|---|---|
| Header | Transport shape (▶ / ■ / ● / ○) | 13x14 px (1.6 mm, ~10') | KEEP, fix REC | The REC glyph replaces play/stop. Add the dot beside it |
| Header | Tempo `120` / `EXT 126` | AF_M 9.6' | KEEP | The CLOCK LOST state needs a sticky header mark |
| Header | Preset `P01`, pending `P04?` + settle bar | AF_M 9.6' | KEEP | Pending state is good. `P` could be misread as pattern |
| Header | Page boxes `1 2 3 4`, orange = playing, underline = viewed | AF_M in 13x14 | KEEP | Clear. The underline is 3 px, detectable only |
| Channels | Tile, selected = inverted | 58x38 | KEEP | Strong selection. Its algorithm name should be larger than the others |
| Channels | `1→2` routing | AF_M 9.6' | CHANGE | Jargon. Drop it on unselected tiles. Show `OFF` explicitly instead of `--` |
| Channels | Algorithm name | AF_M 9.6' | KEEP / ENLARGE selected | At the floor. The brief names it as essential |
| Channels | `M` chip + outline + dark fill | 13 px chip | KEEP | Redundant encoding, good |
| Channels | Activity bar (orange, 4 px) | ~3' tall | KEEP | Detection only. It's fine as a "sounded" flicker |
| Channels | Solo hatching on the others | full tile | KEEP | Distinct from mute. Good |
| Lane | Bar height = velocity | 4..36 px | **REPLACE** | Must be pitch (P1). Soft notes vanish |
| Lane | Accent triangle cap | 13x8 | KEEP | Shape-coded, survives greyscale |
| Lane | Slide tie | 15x5 | KEEP | Readable, crosses the gap correctly |
| Lane | Ratchet digit | AF_M 9.6' | KEEP | Blurs out at distance but is secondary |
| Lane | Edited foot (cyan, 5 px) | ~4' | CHANGE | Hue collision with accent. Invisible in greyscale. Use MID |
| Lane | Playhead column | 13x70, orange | KEEP | The best element on the screen |
| Lane | Ghost playhead (other page) | 2 px orange frame | CHANGE | Reads as "playing here". Use grey |
| Lane | Past-length hatching | full columns | KEEP | Clear and colour-free |
| Lane | Muted lane (hollow bars, grey caps) | | KEEP | Good. Should also trigger the silent warning when selected |
| Knobs | Label (row 1 plain, row 2 inverted band, row 3 boxed) | AF_M 9.6' | KEEP | Row identity without reading |
| Knobs | Value | AF_L 18' | KEEP | Hero, correct |
| Knobs | Meter | 48x6 (~5') | KEEP | Range sense at a glance, matches the TSV ranges |
| Knobs | Enum pictures (DIR ramp, PATH, SHAPE dots, RATE pips) | ~31 px | KEEP | Distinctive, memorable |
| Knobs | `KEYS` (DENS 0) | AF_M | KEEP | |
| Strip | ROW 1 / ROW 2 / ROW 3 cells | AF_M, 16 px band | CUT / MERGE | Redundant with the card style. Half empty |
| Cue | Idle stopped `HOLD HOME = MAP` | AF_M | KEEP | Good onboarding hook |
| Cue | Idle playing `FX SCL ENV LFO ARP SEQ = DICE` | AF_M | KEEP | OK. Consider pairing each printed name with its layer on the map, not here |
| Cue | Chip + role (`SCL NOTES EDIT = UNDO`, `OCT+ OFFSET PEDAL PUNCH DIR`) | AF_M | KEEP | The follows-the-hand idea works |
| Cue | Black-key chips (`M2`, `SOL`, `CHN`, `RST`, `PG-`) | AF_M | CHANGE | Unprinted names presented as printed. Prefix with a key glyph |
| Cue | `TAP AGAIN = ON` | AF_M | CHANGE | `= UNMUTE` |
| Cue | `CLOCK LOST: OWN TEMPO 120` | AF_M | CHANGE | Must be sticky, not transient |
| Cue | `REC KEYS WRITE NOTES` on a silent channel | AF_M | CHANGE | Needs a silent-channel override |
| Toast | Dice (chip + layer AF_L, sentence AF_M, `EDIT: UNDO`) | 40 px slab | KEEP | Excellent. Shorten for bursts |
| Toast | Dice, layer absent (`NO ARTIC IN SNAKE`, dimmed) | | KEEP | Model "nothing changed" copy |
| Toast | Undo / redo | | KEEP | Add a one-level hint |
| Toast | Preset pending `P04 LOADS AT BAR` + bar, `SAVE: KEEP HERE HOME: CANCEL` | | **FIX COPY** | P1: the overwrite is mislabelled |
| Toast | Preset loading `P04 ▶ LOADING / SAVE HOLD = UNDO LAST LOAD` | | KEEP | Reassuring escape |
| Toast | Algorithm (`EUCLID`, 6 pips) | | KEEP | Position pips are good |
| Toast | MIDI channel (`MIDI 2 / CH1 SENDS ON MIDI CHANNEL 2`) | | CHANGE | Must name the channel set to OFF |
| State | EXT waiting (`EXT --`, `WAIT`) | | KEEP | Clear |
| State | REC stopped / playing | | CHANGE | Transport glyph ambiguity |
| State | Follow off (`PG- VIEW 2 OF 4 FOLLOW OFF`) | | KEEP | Grey the ghost |
| State | Add page (`ADD PAGE 3 / PG+ AGAIN: COPY OF 2 HOME: NO`) | | KEEP | Strong |
| State | Past length (`PAST THE END / CHN K1 = LENGTH / 20 STEPS`) | | KEEP | Points to a page not yet drawn |
| State | First run (`1 [→1]`, `CHN K2 = MIDI CHANNEL`) | | CHANGE | The K2 card contradicts the cue. CHN is unprinted |
| State | Reset (`RESET` bar + `EDIT RESET: TOUCH A CONTROL`) | | KEEP | Clear latched mode |
| State | SOL / FIL (`▲DENS 16`) / ROL | | KEEP | FIL's raised label is a nice touch |
| State | Selected muted + MIDI OFF + REC | | **FIX** | P1: no silent warning |
| State | CHAN page | absent | **ADD** | P1: missing screen in the acceptance workflow |
| Map A | Layout (4x3 buttons, 1x4 encoders) | | CHANGE | Mirror the panel 6x2 / 2x2 |
| Map A | Printed names | AF_S 7.6', MID | CHANGE | Below the floor. Raise to AF_M |
| Map A | Roles + boxed hold roles | AF_M | KEEP | The one hold encoding is clean |
| Map A | Live-row highlight (OCT- tile lit) | | KEEP | Good contextual state |
| Map A | Touch = help line (`ALL: re-roll every layer`) | AF_M mixed case | KEEP | The only mixed-case line. Make the case consistent |
| Map A | MENU countdown bar | 187x11 | DECIDE | Contradicts the button_modes release model |
| Map B | Black-key labels with leader lines, 3-2-3-2-1 | AF_M | KEEP | Accurate to key_map. Add the M1–M4 hold PREVIEW |
| Map B | White keys 1/5/9/13 + `TAP = STEP ON/OFF`, `HOLD = EDIT THAT NOTE` | AF_M | KEEP | |
| Map B | Legend (accent, slide, ratchet, edited, muted, MIDI out/off, sounded) | AF_M | CHANGE | Missing "bar height = ?". Cyan listed twice |
| Map B | Header `TOUCH = HELP < OCT- = BUTTONS` | AF_M, full width | CHANGE | Crowded. Split or drop "TOUCH = HELP" on page B |
