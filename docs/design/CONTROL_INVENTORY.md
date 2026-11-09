# Control inventory: what every physical control does in the firmware today

Ground truth for the global control map (P2-001). This is a read-only inventory of the firmware **as the code
really behaves**, taken by reading the source on 2026-11-09 (task P1-001). Nothing here proposes a new mapping.

How to read it:

- Citations are `file:line` in `firmware/src/` unless a path is given. Lines are those of the repository at the time
  of writing.
- **[code]** = read directly from the code. **[unconfirmed]** = inferred, or depends on something I could not check
  without hardware. Nothing was run on a device. "Host tests don't render the screen" (CLAUDE.md), so every
  on-screen label here is from the draw code, not from a photo.
- Two sequencers share the panel. **Felucca mode** (default, 4 tracks, a full groovebox) and **T1 mode** (MENU >
  SYSTEM > T1 SEQ ON, 6 tracks, the algorithmic T-1 sequencer). `ui_input()` hands the whole panel to `t1_input()`
  when `t1_mode` is on (ui_input.c:1510-1519), so the two columns below are independent.
- The panel image `docs/controls.jpg` agrees with the printed labels used here (SELECT, MASTER, PRESETS, ALGORITHM,
  KNOB1-4, the 14 buttons, OCT-/OCT+, 27 keys F3-G5). Its "ALGORITHM = Track T1-T4" row confirms that the encoder
  printed ALGORITHM selects the *track* in Felucca mode (ui_input.c:1767-1768), not an algorithm.

## 0. Hardware facts the rest depends on

| Fact | Value | Source |
| --- | --- | --- |
| Buttons (enum `B_*`) | 14: `FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP` | panel.c:9-10 |
| Encoders (enum `EN_*`) | 7: `SELECT ALGO PRESET K1 K2 K3 K4`, relative detents via `panel_enc()` | panel.c:11, 60-66 |
| Printed names | SELECT, ALGORITHM, PRESETS, KNOB 1-4 | panel.c:14-15 |
| MASTER | An analogue pot (ADC), **not** an `EN_*` encoder; sets `song.master_q12`. No firmware gesture. | master.c:8-15 |
| Matrix ids per label | table, overridable by CALIBRATION; default FX..REC = 2..13, OCT- = 0, OCT+ = 1 | panel.c:26-31 |
| Keys | 27, F3-G5; bit 0 = F3; 16 white (place 0-15), 11 black (place 0-10) | seq.c:96-104 |
| Tracks | Felucca `NTRK = 4`; T1 mode `NUM_TRACKS = 6` | core.h:12-13; ui_t1.c:91 |
| Hold thresholds | HOME, SAVE: fixed 700 ms (`btn_hold`). Layer buttons (FX GLO SCL EDIT SEQ REC): MENU > HOLD 0.3/0.4/0.5/0.6 s, default 0.4 s | ui_input.c:1298-1313; panel.c:76-81; ui_layer.c:263-290 |
| Double-tap lock window | 300 ms | ui_layer.c:112 |
| Octave range | -3..+3, both OCT held = 0 | ui_input.c:1708-1718 |

Per-frame order in `ui_input()` (ui_input.c:1472-1825), which decides who "wins" a control: MIDI notes taken ->
**SCREEN OFF wake swallow** (`scr_input`, ui.c:298) -> MOMENTARY check -> **T1 mode hand-over** -> REC/SAVE/HOME
timers -> layers -> menu (returns) -> NAME (returns) -> SAVE hold/tap -> confirm dialog (returns) -> layer tap, HOME
tap -> button edges (PLAY, OCT) -> page buttons on release -> action-page OCT -> keys -> PRESETS, ALGO, SELECT -> KNOB
1-4.

## 1. Buttons and encoders (Felucca mode and T1 mode)

Column key: **Tap** = pressed and let go before the hold threshold. **Hold** = held past the threshold alone.
**Combos** = with another control down. **T1** = in T1 mode. "Acts on press" vs "on release" is stated where it
matters.

### 1.1 The 14 buttons

| Control | Tap (Felucca) | Hold | Combos | In T1 mode | file:line |
| --- | --- | --- | --- | --- | --- |
| `B_FX` | Opens FX family; again = next page: FX, SLICER, INSERT, INSERT 2, DLY, REVERB, CHORUS. Acts on release. | **FX layer** (HOLD kind): white keys = performance effects, black keys 1-4 = track mutes while held, K1-4 = macros FILTER/CRUSH/THROW/SHIMR-or-DEPTH. Double-tap locks the layer open. With MENU > FX LATCH ON the layer latches. | FX held + PRESETS turn = assign an effect to the held white key; FX held + EDIT = that key's default; FX held + PLAY works; SAVE/HOME/SEQ swallowed. LED lit while an effect/macro is on (latch). | Nothing: no handler in `t1_input` [code]. | params.c:455-462 (pages); ui_input.c:1338-1380 (`page_tap`); ui_layer.c:59-60, 631-636, 702-708 (layer); ui_input.c:1559-1564 (FX + PRESETS, FX + EDIT); ui_input.c:411-412 (LED) |
| `B_SCL` | Opens SCL family: SCL (ROOT SCALE QNT TRN), then CHORD (CHRD VOIC). | **SCL layer** (SET): any key = ROOT (that key's note name); K1-4 = ROOT, SCALE, CHORD, VOICING. | OCT- let go in the layer = UNDO ("SCALE PUT BACK"). | Nothing [code]. | params.c:472-473; ui_layer.c:61, 652-654, 702-723, 790-800 |
| `B_ENV` | Opens ENV family: ENV (ATK DEC SUS REL), ENV DEST. | No layer: a long press just opens the page on release. | LFO is not ENV's modifier. | Nothing [code]. | params.c:446-447; ui_input.c:1719-1729 (default page button branch) |
| `B_LFO` | Opens LFO family: LFO, LFO 2, LFO DEST, MOD. **The tap is dropped if any knob turned while LFO was held.** | No layer. **LFO held = MOMENTARY mode** (below). | LFO held + K1-4 = momentary value (snaps back on release); LFO held + OCT+ = keep the changes ("KEPT"). | **LFO held = TEMP** (same as SAVE held): edits are undone on release; OCT+ commits ("KEPT"). | ui_input.c:50-108 (MOMENTARY), 1480-1483; params.c:448-452; ui_t1.c:56-68 |
| `B_EDIT` | Opens EDIT family: EDIT 1, EDIT 2, FILTER (ANALOG), LANES/LANES 2 (DRUM), SLICES (SLICE), OPERATOR/OP ENV/.. (FM6), VOICE, VOICE 2, VOICE 3 (pages shown depend on the track's engine, `page_visible`). **Acts instead of opening a page** on: STEP roll (clears the cursor step, moves cursor +1), DETAIL (resets CHANCE/RATCH/NUDGE/VEL), AUTOMATION (deletes the row's record), SAVE > USER / PROJECT (rename slot). With steps held on STEP: clears their locks. | **EDIT layer** (SET): white keys F3.. = engines (one key each), then INIT (E5), then CLEAN INIT (F5); black keys = the engine's quick actions (DRUM: lane mutes 1-8, others none); K1-4 = ENG, No. (sound), FAV (K4 none). | EDIT + FX layer: default key. OCT- let go in the layer = UNDO the sound loads. | Nothing [code]. | ui_input.c:1338-1380; params.c:474-492; ui_layer.c:62, 655-668, 710-717, 769-780 |
| `B_GLO` | Opens GLO's only page: **SONG** (a song-chain editor; K1 SECTION, K2 TRACK, K3 PAT, K4 REPS; keys A3 B3 C4 D4 set the slot). | **GLO layer** (SET): black keys 1-4 (F#3 G#3 A#3 C#4) = mute T1-T4 latched; white F3 G3 A3 B3 = SOLO T1-T4 while held; C4 = unmute all; D4 = MIDI LEARN on/off; E4 = FILL while held; F4 = TAP tempo (needs internal clock); K1-4 = T1-T4 LEVEL. | **GLO + SELECT = tempo** (even with BPM LOCK); **GLO + PLAY = restart from the top**; GLO + K1-4 = levels. | **GLO held = GLOBAL**: K1-4 move all six tracks (relative). | params.c:493; ui_layer.c:60, 638-651, 706-715, 741-753; ui_input.c:1566 (GLO + SELECT), 1689-1690 (GLO + PLAY); ui_t1.c:69-72 |
| `B_HOME` | **HOME acts on release**: on the HOME screen -> MIXER (K1 LEVEL, K2 PAN, K3 REV, K4 MUTE of the selected track) -> again CLOCK (BPM, SWING, CLK) -> again HOME; from any other page -> HOME. In the menu: closes it. | **0.7 s: opens the MENU** (or closes it if open). Fixed 700 ms, not the HOLD setting. | HOME held while a layer button is held is swallowed. | **Hold = MENU** (same). A tap does nothing [code: `t1_input` has no HOME]. | ui.c:806-818; ui_input.c:1510-1519, 1583-1597; params.c:495-500 |
| `B_SAVE` | **On release**: opens SAVE family, entering at USER (user presets: K1 SLOT, K2 LOAD, K3 ERASE, K4 SAVE); again = PRESETS, PHRASES, PROJECT, TOOLS. | **0.7 s: UNDO the last sound/pattern load** (held again: redo). "STOP TO UNDO" while a song chain plays. | Swallowed while a layer button is held. SAVE hold does not apply in the menu or a dialog. | **SAVE held = TEMP** (as LFO). | ui_input.c:1521, 1617-1622; ui.c:769-790; params.c:494-498; ui_t1.c:56-68 |
| `B_ARP` | Opens ARP family: ARP (MODE RATE OCT GATE), ARP 2 (SWING PROB HOLD ORDER). | No layer. | -- | Nothing [code]. | params.c:499-500 region (ARP rows); ui_input.c:1719-1729 |
| `B_SEQ` | Opens SEQ family: STEP (piano roll, or the 16-step grid on a DRUM track), DETAIL (cursor step's CHANCE RATCH NUDGE VEL), AUTOMATION. | **SEQ layer** (SET, "TOOLS"): white keys = sequence tools (below); black keys 1-8 = DRUM lane; K1-4 = LEN DIV SWING GATE (EUCLID open: HITS STEPS ROT, K4 = LANE on DRUM). Each tool is one undo. | SEQ-layer + OCT+ applies / OCT- cancels an open EUCLID. | **SEQ tap = next T-1 page**: RHYTHM, RATCHET, HARMONY, MOD/CC (acts on press). | params.c:495-497; ui_layer.c:63, 669-680, 702-736; ui_t1.c:74-75 |
| `B_PLAY` | **Acts on press.** Stopped: start; playing / counting in / song chain: stop. On the SONG page: starts the song chain. | -- | GLO held: restart without stopping. In NAME: only stops a transport started elsewhere. In the menu / dialogs: nothing. PLAY works while any layer is held. | Start/stop (acts on press). | ui_input.c:1689-1700; ui_layer.c:741-753; ui_input.c:1609-1612; ui_t1.c:53-54 |
| `B_REC` | **On release**: arm/disarm recording of the *selected* track on any page; arming while stopped also starts the transport, except on the STEP roll (then step-recording: keys write the cursor step; "KEYS WRITE STEPS"). On SONG: "[SEQ] TO RECORD". Nothing in menu/dialog/NAME. LED lit while armed. | **REC layer** (SET): white F3 = CLEAR (selected track's steps and automation, one undo), G3 = CLICK (OFF/REC/ON cycle), A3 = COUNT-IN (OFF/1 BAR/2 BARS), B3 = CLICK LEVEL (LOW/MID/HIGH); K1-3 the same three. | REC + PLAY: PLAY plays, REC let go soon is a tap (arms). | **Nothing** [code: `B_REC` not referenced in ui_t1.c]. | ui_input.c:582-608, 1597-1598; ui_layer.c:64, 312-322, 428-478, 669-671; ui_input.c:415 (LED) |
| `B_OCTDN` | Octave -1 (range -3..+3). **Acts on press** on normal pages. On action pages / dialogs / menu / NAME / SET layers / MIDI LEARN it acts on **release** as "back / cancel / UNDO / clear". | -- | OCT- + OCT+ together = octave reset 0. Both held **5 s** = UPDATE MODE (countdown shown from 2 s; let go to cancel). Both held at power-on = CALIBRATION. | **OCT- = previous T-1 page** (on press). | ui_input.c:1150-1167, 1708-1718, 1730-1738; main.c:189-192, 212-246; ui_t1.c:76-77 |
| `B_OCTUP` | Octave +1. Same release-vs-press rule. Dialog/action page/menu/NAME: "do it / confirm / write". | -- | LFO held + OCT+: keep MOMENTARY changes. SCL/SEQ/EUCLID layers: apply. | **OCT+ held = SHIFT**: K1-4 address the second row of each page. In TEMP: press commits. | ui_input.c:1708-1718, 1730-1733; ui_layer.c:766-790; ui_t1.c:62-73 |

Layer buttons FX/GLO/SCL/EDIT/SEQ/REC share one gesture engine (ui_layer.c:263-290). **Tap** (let go before the HOLD
time with nothing else touched) = the button's page. **Hold alone past HOLD** = the map shows (a peek); let go does
nothing. **A key, knob or other button pressed meanwhile** = a combo: the map opens at once and the tap is cancelled.
**Double-tap** (second press within 300 ms) = locks the layer open with no button held (ui_layer.c:206-229): PLAY, REC,
OCT-/OCT+ stay live; any other button closes it. Only one layer at a time (a second layer button is ignored).
While a layer is open keys, K1-4 and OCT- belong to it, and SAVE/HOME/other page buttons are swallowed
(ui_input.c:1548-1572). The first tap before a layer has ever been opened shows "HOLD [GLO] QUICK" (ui_layer.c:326-332).

**MOMENTARY (LFO held, Felucca)**: K1-4 change the value shown on HOME, the sound/FX pages and MIXER, and each value
turned snaps back when LFO is let go. Not on action pages, lists, STEP, AUTOMATION or SONG. OCT+ keeps. Not recorded as
automation. Up to `MOM_N` values ("MOMENTARY FULL"). (ui_input.c:50-108.)

### 1.2 The 7 encoders

| Control | Normal pages | Menu | Layers | NAME / dialogs | In T1 mode | file:line |
| --- | --- | --- | --- | --- | --- | --- |
| `EN_SELECT` | **Tempo (BPM 40-240)** by default. MENU > SELECT can change it to **PAGE** (steps the family's pages, a detent each), **MODW** (mod wheel CC1 of the selected track, sent on its MIDI channel) or **VEL** (key velocity 1-127). MENU > BPM LOCK ON: the turn does nothing and says "BPM LOCKED" unless GLO held. | Dropped (`enc_drop`) | GLO held: always tempo. Other layers: still tempo (the PAGE/MODW/VEL modes are off while a layer is held). | Dropped | Always BPM (`accel`ed). | ui_input.c:1769-1786 (+ sel_play 114-131); ui_input.c:1566; ui_t1.c:81-82; ui_menu.c:489-494 |
| `EN_ALGO` (printed ALGORITHM) | **Selects the track T1-T4**, one per detent, clamped (no wrap), on every page. NOT an algorithm selector in this firmware. | **Steps the MENU tabs** (DISPLAY CONTROL AUDIO MIDI SYSTEM), wraps | Ignored while a layer is held (OCT- puts back the layer's track only) | Dropped | Dropped (read and discarded) | ui_input.c:1565, 1767-1768; ui_menu.c:514-520, 540-545; ui_t1.c:83 |
| `EN_PRESET` (printed PRESETS) | By page: **HOME, PRESETS (BROWSE) and every sound/FX/ARP/SCL/MIXER/CLOCK page: loads the next/previous sound** for the selected track across all engines (steps kept; SAVE held undoes). On SEQ STEP/AUTOMATION, USER, PHRASES, PROJECT, SONG = same as KNOB 1 (cursor/row/slot). TOOLS, SLICES: nothing. With steps held on STEP: **NUDGE** them (-8..+7 sixteenths). DETAIL: the cursor. | **Moves between rows** of the tab (wraps). In ABOUT: scrolls. | FX layer + a white key held: assigns the next effect to that key. Other layers: dropped. | Dropped | Dropped | ui_input.c:1268-1295, 1766; ui_menu.c:508-513, 530-531; ui_input.c:1559-1564; ui_t1.c:84 |
| `EN_K1`, `EN_K2`, `EN_K3`, `EN_K4` | The page's four columns (see 1.3). Fast turns accelerate (MENU > KNOB ACCEL). Recorded as automation when REC is armed and playing (`motion_capture`); picked by MIDI LEARN. | **Any of K1-4 changes the selected row's value** (all four are equivalent; summed) | Layer-specific (section 3) | NAME: K1 cursor, K2 character, K3 category (USER) or random name (PROJECT), K4 none. Dialog: dropped. | The four cards of the current T-1 page; OCT+ held = second row; GLO held = all tracks | ui_input.c:1788-1824 (`edit_param` 941-1055); ui_menu.c:539-546; ui_name.c:~270-306; ui_t1.c:78-80 |

### 1.3 K1-K4 by page (Felucca)

`edit_param` (ui_input.c:941-1055) selects the behaviour by the page's `scope` / `graph`; the table of pages is
params.c:445-500.

| Page / context | K1 | K2 | K3 | K4 | Source |
| --- | --- | --- | --- | --- | --- |
| HOME (view SCOPE/TRACKS) | the selected track's engine's four knob params (`ENGINES[..]->knob[k]`) | | | | ui.c:1739-1745; ui_input.c:1815-1820 |
| HOME (view LEVELS, MENU > HOME) | LEVEL of T1 | LEVEL of T2 | LEVEL of T3 | LEVEL of T4 | ui.c:1739; ui_input.c:1815-1820 |
| ENV | ATK | DEC | SUS | REL | params.c:446 |
| ENV DEST | ED_FLT | ED_PIT | ED_SHP | ESYNC | params.c:447 |
| LFO | LRATE | LWAVE | LPHASE | LFADE | params.c:448 |
| LFO 2 | LSYNC | LTRIG | LPOL | - | params.c:449 |
| LFO DEST | LD_PIT | LD_FLT | LD_SHP | LD_AMP | params.c:450 |
| MOD | slot 1-4 | M1SRC | M1DST | M1AMT | params.c:451; ui_input.c:1040-1043 |
| FX / SLICER / INSERT | DIST CHOR DLY REV; SLCR SLPAT SLRATE SLDEPTH; ITYPE IA IB IC | | | | params.c:452-455 |
| INSERT 2 / DLY / REVERB / CHORUS | IMIX; DTIME DFDBK DCOLOR DMIX; RTYPE RSIZE RDAMP; CRATE CDEPTH | | | | params.c:456-459 |
| SCL | ROOT | SCALE | QNT | TRN | params.c:460 |
| CHORD | CHRD | VOIC | - | - | params.c:461 |
| EDIT 1 / EDIT 2 / FILTER | engine params E0-E3 / E4-E7 / FTYPE E4 E5 E7 | | | | params.c:462-464 |
| LANES, LANES 2 (DRUM) | lane levels 0-3 / 4-7 | | | | params.c:465-466 |
| SLICES (SLICE) | marker | move marker | (SPLIT/JOIN via OCT+) | | params.c:467; ui_input.c:1004-1009 |
| OPERATOR, OP ENV, OPERATOR 2, OPn ENV, OP LEVEL (FM6) | operator | its bytes | | | params.c:468-473 |
| VOICE / VOICE 2 / VOICE 3 | VOICE GLIDE GLMODE PRIO; ALLOC DETUNE PAN MUTE; SPRD | | | | params.c:474-476 |
| SONG | SECTION | TRACK | PAT (A-D, -) | REPS | ui_input.c:951-972 |
| USER | slot | (action picks: K2 LOAD, K3 ERASE, K4 SAVE; right picks, left drops, OCT+ does it) | | | ui_input.c:1021-1030 |
| PRESETS (BROWSE) | preset | engine | favourite mark | LIST filter (ALL FAV BASS ..) | ui_input.c:986-1002 |
| PHRASES | pattern | LOAD (OCT+) | - | - | ui_input.c:1031-1035 |
| PROJECT | slot | ERASE (OCT+) | LOAD | SAVE | params.c:481 (ids); ui_input.c:1021-1025 |
| TOOLS | CLEAR | INIT / CLEAN INIT | DELETE SECTION | CLEAR SONG | params.c:482; ui_input.c:1010-1020 |
| ARP / ARP 2 | MODE RATE OCT GATE; SWING PROB HOLD ORDER | | | | params.c:483-484 |
| STEP (piano roll) | STEP (cursor) | NOTE (transpose step) | TIME / note LEN | FLAG (- ACC SLD A+S) | ui_input.c:889-939 |
| STEP (DRUM grid) | STEP | LANE | HIT on/off | ACC on/off | ui_input.c:649-661 |
| DETAIL | CHANCE | RATCH | NUDGE | VEL | ui_events.c:504; ui_input.c:840-868 |
| STEP with a step key held | parameter **lock** of the knob's param on that step (the four params of the last sound page) | | | | ui_input.c:711-770 |
| AUTOMATION | list rows, kind, values (ui_events.c ev_knob) | | | | ui_events.c:348 |
| MIXER (HOME tap) | LEVEL | PAN | REV | MUTE | ui_input.c:505-528 |
| CLOCK (HOME tap, tap) | BPM | SWING | CLK (int/ext) | - | params.c:499 |

### 1.4 T1 mode: K1-K4 by page (OCT+ off / OCT+ held "SHIFT")

Four pages, switched by SEQ (next) and OCT- (previous). Source: torso_t1_ui.c:134-470 (turn handlers) and 500-691
(card labels). With GLO held the same turn becomes a relative delta on all six tracks (torso_t1_ui.c:140-182).

| Page | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| RHYTHM | STEPS / (SHIFT) MAX LEN | PULSES / ROTATE | DIVISION / GATE LEN | CYCLE / STEP MOD |
| RATCHET | REPEATS / OFFSET | TIME / PACE | VELOCITY / ACCENT | PROB / GROOVE |
| HARMONY | PITCH / M-TRANS | ROOT / SCALE | VOICING / STYLE | PHRASE / RANGE |
| MOD/CC | RND TGT / CHANNEL (MIDI) | RND DEPTH / RND RATE | CC NUM / SLEW-DEL | CC VALUE / VOICES |

Notes: T1 track 1 plays the FM-1's own synth; tracks 2-6 send MIDI on `CHANNEL` (ui_t1.c:190-195).
A P-LOCK mode exists in `torso_t1_ui.c:171-206` (`edit_step`) but **nothing in ui_t1.c ever sets `edit_step`**
(no `t1_ui_set_edit_step` caller): step-key P-LOCK is not reachable. [code: grep, no caller]
**TORSO_T1_GUIDE.md section 2 vs code**:

| Guide claim | Code | Verdict |
| --- | --- | --- |
| REC arms real-time recording | REC unreferenced in `t1_input` (ui_t1.c:46-102) | **false** |
| Keys 1-16 hold = step P-LOCK | keys 1-6 select track, 7-12 select cycle, black 1-6 mute (ui_t1.c:85-99); `edit_step` never set | **false** |
| Page nav via SEQ / OCT- / OCT+ | SEQ next, OCT- prev; **OCT+ is SHIFT**, not navigation (ui_t1.c:73-77) | **partly** |
| K4 on page 1 = cycle | RHYTHM K4 = CYCLE (torso_t1_ui.c:263-272) | true |
| SAVE or LFO held = TEMP; OCT+ commits | ui_t1.c:56-68 | true |
| GLO held = all tracks | ui_t1.c:69-72 | true |
| Page 4 K4 shift = "LoopSync" | shift K4 = chord VOICES (torso_t1_ui.c:442-456) | **differs** |

## 2. The 27 keys

Key places: 16 white (F3 G3 A3 B3 C4 D4 E4 F4 G4 A4 B4 C5 D5 E5 F5 G5 = place 0-15) and 11 black (F#3 G#3 A#3 C#4 D#4
F#4 G#4 A#4 C#5 D#5 + one more: places 0-10), from `key_black()` / `key_place()` (seq.c:98-104). `song.grid`
(= `keys_mode()`, ui_name.c:62; set ui_input.c:1582 and 1740) decides what a *press* does; the ISR side is
`keyboard_block` (seq.c:775-835). Order of precedence for a key press:

1. A layer's button down (`kb_mask`) or a layer locked (`kb_lock`): the key is the layer's, silent
   (seq.c:798-805).
2. `song.grid` 2 (NAME or T1 mode) -> every key silent (seq.c:792-807). `song.grid` 3 (SONG page) -> A3 B3 C4 D4 silent.
3. `song.grid` 1 (DRUM grid) -> `grid_key_note` (seq.c:110-113): black place 0-7 play lane notes, the rest silent.
4. Otherwise `kb_map(track, key)` (seq.c:181-185): a note, with TRN, OCT, QNT, CHORD, the engine's own key map.

### 2.1 Per `song.grid`

| song.grid | Active when | White keys | Black keys | file:line |
| --- | --- | --- | --- | --- |
| **0 notes** | default | Play the selected track's note (F3 = 53 + OCT*12 + TRN, scale-quantised per QNT). Also MIDI out (`key_on`, kb_vel). With CHORD, play a chord. | Same, chromatic (or silent when QNT WHITE). | seq.c:181-185, 720-760, 810-820 |
| **0 + STEP piano roll, REC armed, stopped** | SEQ > STEP / DETAIL non-DRUM, REC armed, not playing | The key's note is **written into the cursor step** and the cursor advances (MENU > CHORD ENTRY ADD: adds to the step, cursor stays). Notes also sound. | same | ui_input.c:1213-1256, 1816-1827 |
| 0 + STEP, armed and playing | | Record live at the play head (`rec_note`) | | ui_input.c:610-613; seq.c:494 (`rec_note`) |
| 0 + STEP, **not** armed | | Play only; "being on STEP writes nothing" | | ui_input.c:1750-1757 |
| 0 + STEP, step key held | held key = the step being entered; K1-4 = parameter lock | | | ui_input.c:711-731 |
| 0 + STEP, stopped, cursor on a note | key LEDs show the cursor step's notes | | | ui_input.c:268-288 |
| **1 DRUM grid** | SEQ > STEP / DETAIL on a DRUM-engine track (not HOME/menu/dialog) | **16 steps** of page `ui.bank`: toggle the selected lane's hit at that step (ACC held = accent), cursor to that step; a hit on an already-hit step is removed on release unless held for a lock. DETAIL: just picks the step. Steps past LEN do nothing. "STOP TO EDIT" while a song plays. | place 0-7 = lane select and play that lane; place 8 = **ACC** (modifier); place 9 = page down; place 10 = page up. | ui.c:599; ui_input.c:663-706; seq.c:106-113 |
| **2 silent** | **NAME** (ui_name.c) or **T1 mode** | NAME: letter groups `AB CD EF GH IJK LM NO PQ RS TU VW XYZ 123 456 789 0-.` (multi-tap within 0.8 s). T1: place 0-5 = **select track 1-6**, place 6-11 = **select cycle 1-6**, place 12-15 nothing. | NAME: F# cursor left, G# SPACE, A# cursor right, C# DELETE (repeats when held), D# ABC/123 (names, both octaves). T1: place 0-5 = **mute/unmute track 1-6**; the rest nothing. | ui_name.c:62-82, 270-310; ui_t1.c:85-99 |
| **3 SONG** | GLO > SONG page, not in a layer | A3 B3 C4 D4 (key indexes 4 6 7 9; `SONG_KEYS`) set the selected cell's slot A-D and are **silent**; other keys play. On "+ ADD" they add the section first. | play | seq.c:114-122; ui_input.c:547-565; ui.c:609 |

### 2.2 Keys while a layer's button is held (`kb_layer`)

Keys pressed with the layer button down are the layer's: silent, never recorded, no MIDI. `layer_key()` is
ui_layer.c:631-680; labels are drawn by ui_layer.c:982-1260.

| Layer | White keys (place) | Black keys (place) | Source |
| --- | --- | --- | --- |
| FX | each plays an effect while held; default map F3 REPEAT 1/8, G3 1/16, A3 1/32, B3 REVERSE, C4 LPF, D4 HPF, E4 TAPE STOP, F4 FREEZE, G4 OCT UP, A4 OCT DN, B4 FLANGER, C5 PHASER, D5 REV THROW, E5 ECHO THROW, F5 none, G5 none. Re-mappable (PRESETS) and saved. | place 0-3 = mute T1-T4 while held | perform.c:80-86; ui_layer.c:479-482, 631-636, 982-999; seq.c:670-673 |
| GLO | F3 G3 A3 B3 = SOLO T1-T4 (held); C4 = UNMUTE ALL; D4 = MIDI LEARN toggle; E4 = FILL (held); F4 = TAP tempo | place 0-3 = MUTE T1-T4 latched (lit = sounding) | ui_layer.c:638-651, 851-854 |
| SCL | any = ROOT = that key's note name (F3 = F) | any = ROOT | ui_layer.c:652-654 |
| EDIT | place 0..(NENG_SHOWN-1) = engine by PRESETS order; place 13 (E5) = INIT dialog; place 14 (F5) = CLEAN INIT dialog | the engine's quick actions (DRUM: lanes 1-8 MUTE, hold) | ui_layer.c:50-52, 655-668, 556-620 |
| SEQ | place 0 CLEAR, 1 REVERSE, 2 SHIFT <, 3 SHIFT >, 4 RANDOM, 5 COOK, 6 BEAT (DRUM), 7 **EUCLID (F4)**, 8-11 lane CLEAR/REVERSE/FILL/RANDOM (DRUM), 12 DOUBLE (D5), 13 HALF (E5) | place 0-7 = DRUM lane | ui_tools.c:15-28; ui_layer.c:386-426, 672-679 |
| REC | F3 CLEAR, G3 CLICK, A3 COUNT-IN, B3 CLICK LEVEL | none | ui_layer.c:466-478, 669-671 |
| LFO (MOMENTARY) | not a layer: keys play normally | | ui_input.c:63-66 |

### 2.3 Key presses in the menu, dialogs, MIDI learn

In the menu and confirm dialogs `song.grid` is recomputed so keys play again (ui_input.c:1582). In T1 mode the keys
are silent (song.grid 2). While MIDI LEARN is on, keys play normally (not found to be intercepted) [unconfirmed].

## 3. Layer knobs and OCT in one place

| Layer | K1 | K2 | K3 | K4 | OCT- (on release) | OCT+ | Source |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FX | FILTER (LP/HP, -100..100) | CRUSH | THROW | SHIMR (or DEPTH with harmony FX) | (latch) ALL OFF | - | ui_layer.c:704-709, 1172-1190 |
| GLO | T1 LEVEL | T2 LEVEL | T3 LEVEL | T4 LEVEL | "MIX PUT BACK" (mutes, levels, BPM) | - | ui_layer.c:710-714, 800-806 |
| SCL | ROOT | SCALE | CHORD | VOICING | "SCALE PUT BACK" | - | ui_layer.c:722-735 |
| EDIT | ENG | No. (sound) | FAV | - | undo sound loads | - | ui_layer.c:715-721 |
| SEQ | LEN | DIV | SWING | GATE (EUCLID open: HITS STEPS ROT, LANE) | "SEQUENCE PUT BACK"; EUCLID open: cancel | EUCLID open: apply | ui_layer.c:36-38, 730-736, 770-790 |
| REC | CLICK | COUNT-IN | CLICK LEVEL | - | "REC PUT BACK" | - | ui_layer.c:722-726, 800-820 |

## 4. LEDs and what drives them

All driven by `ui_leds()` (ui_input.c:393-466), main loop, every frame. Layers of light: lit (`fm1_led`), dim glow
(`fm1_led_dim`), mid level (`fm1_led_mid`), breathing (`fm1_led_breath`). MENU > LEDS: OFF, DIM LO, DIM HI (default),
INV, INV LO, INV HI (menu_items.c:160-161; panel.c:89-105).

| LED | Lit / blinking when | Source |
| --- | --- | --- |
| Family button (HOME ENV LFO FX SCL EDIT GLO SAVE ARP SEQ; HOME for MIXER/CLOCK) | steady lit while that family's page is shown (`FAM_BTN`) | ui_input.c:38-41, 407-408 |
| Layer button | **breathes** while its map is up | ui_input.c:413-414 |
| ARP | **flashes on the beat** (bar's first beat longer) while an ARP is playing on any part; on ARP's own page the flash is dark | ui_input.c:207-215, 409-410 |
| FX | lit while an effect or macro is latched/held | ui_input.c:411-412 |
| REC | lit while the selected-track record arming is on (`song.rec`), except while its layer is up | ui_input.c:415 |
| PLAY | second **green LED** (matrix col 8) lit while playing, and on each beat of the count-in; PLAY's own LED dark while playing | ui_input.c:29-30, 438-440, 456-457 |
| OCT- / OCT+ | lit when the octave is below / above 0; in dialogs/action pages/NAME/menu: OCT- lit, OCT+ **breathes** while it would act; menu: both breathe on a value row while that way has a value; MOMENTARY: OCT+ breathes | ui_input.c:44-49, 133-150, 416-420 |
| Keys (27) | held keys and the notes the selected track's sequencer/ARP/MIDI IN sound (`play_leds`, `midi_leds`); on STEP stopped the cursor step's notes (`step_leds`); a layer's map (`layer_leds`); NAME's keys; DRUM grid (hits lit, playhead inverted, beats mid-level); SCALE LEDS (menu): the scale's keys glow, root brighter; SONG: the slot's key | ui_input.c:152-372, 421-433, 837-876 |
| Idle animation | after 60 s without input with MENU > ANIM IDLE and nothing active | ui_input.c:333-356 |
| Boot sweep | power-on light sweep (not with ANIM OFF or after a crash) | main.c:127-165 |
| Calibration, UPDATE countdown | UBOOT countdown shown on screen (not an LED) | main.c:212-246 |

**T1 mode LEDs: no T1-specific LED code exists** [code: ui_leds has no `t1_mode` branch]. They keep showing whatever
Felucca page/family was last shown (`cur_fam()`), the OCT state, REC `song.rec`, and held keys. So the SEQ / OCT- /
OCT+ T1 roles have no matching LED feedback. [unconfirmed on hardware]
Comment at ui_input.c:60 says "LFO has no LED"; `FAM_BTN` maps LFO to its own button so it lights on LFO pages;
treat the comment as stale [unconfirmed].

## 5. Hidden gestures (not on the panel labels)

| Gesture | Effect | Source |
| --- | --- | --- |
| OCT- + OCT+ at **power-on** | HARDWARE CALIBRATION (re-teach each label) | main.c:189-192; ui_input.c:1855-1912 |
| OCT- + OCT+ held **5 s** | UPDATE MODE (UBOOT) with a countdown from 2 s | main.c:212-246 |
| Layer button double-tap | locks the layer map open | ui_layer.c:206-229 |
| Layer button hold **before** a first tap | "HOLD [GLO] QUICK" hint once | ui_layer.c:326-332 |
| SELECT with GLO held | tempo, even with BPM LOCK | ui_input.c:1566, 1769 |
| First press/turn after SCREEN OFF | swallowed (wake) | ui.c:298-325 |
| SAVE held 0.7 s | undo/redo the last load | ui_input.c:1617-1620 |
| Both OCT with LFO held | keeps MOMENTARY changes | ui_input.c:1504-1508 |

## 6. Menu-diving list: every MI_* row

Reached by **holding HOME 0.7 s** (ui_input.c:1583-1597). Inside: PRESETS = next row, ALGORITHM = next tab, any of
K1-K4 or OCT-/OCT+ changes the value. `MI_VALUES` rows (CALIBRATION, INFO, ABOUT) open on OCT+ (ui_menu.c:548-565).
Five tabs, at most 8 rows each (menu_items.c:43-45, 46-66). "Class": **M** = musical (a performance or channel function
that should be on the panel, per VISION rules 1 and 3), **S** = set-and-forget setting (stays in the menu), **P** =
already reachable from the panel (duplicate), **I** = information / maintenance action.

| Row | Tab | What it does | Class | Note | file:line |
| --- | --- | --- | --- | --- | --- |
| `MI_COLOR` | DISPLAY | palette | S | | menu_items.c:16, 46-47, 256-258 |
| `MI_STYLE` | DISPLAY | FLAT / LINE cards | S | | menu_items.c:16, 88, 259 |
| `MI_LARGE` | DISPLAY | big knob labels | S | | menu_items.c:95 |
| `MI_ANIM` | DISPLAY | ON / OFF / IDLE animation | S | | menu_items.c:123-127, 263-266 |
| `MI_LEDS` | DISPLAY | OFF DIM LO DIM HI INV INV LO INV HI | S | | menu_items.c:158-161, 274-277 |
| `MI_SCROFF` | DISPLAY | screen off timeout | S | | menu_items.c:121-122, 262 |
| `MI_SCOPE` | DISPLAY | scope OUT / MIX | S | | menu_items.c:103 |
| `MI_HOME` | DISPLAY | HOME view SCOPE / TRACKS / LEVELS (LEVELS turns HOME's K1-4 into per-track LEVEL) | S | changes knob roles; the new role is shown | menu_items.c:128-129, 267-270; ui_input.c:75-77 |
| `MI_HOLD` | CONTROL | layer hold time 0.3-0.6 s | S | | menu_items.c:116; panel.c:76-81 |
| `MI_ACCEL` | CONTROL | knob acceleration | S | | menu_items.c:96; ui_input.c:480-497 |
| `MI_LATCH` | CONTROL | FX LATCH: the FX layer's effects latch | **M** | performance behaviour of a layer; belongs on the FX layer | menu_items.c:97; ui_layer.c:1-13 |
| `MI_BPMLOCK` | CONTROL | SELECT sets tempo only with GLO held | S | performance safety; borderline | menu_items.c:99; ui_input.c:1778-1786 |
| `MI_SCLLED` | CONTROL | keys show the scale | S | display aid | menu_items.c:102; ui_input.c:301-330 |
| `MI_PREVIEW` | CONTROL | STEP PREVIEW: cursor step sounds | S | | menu_items.c:104; ui_input.c:622-637 |
| `MI_CHADD` | CONTROL | CHORD ENTRY HOLD / ADD | S | entry style; borderline | menu_items.c:105; ui_input.c:1213-1256 |
| `MI_SELECT` | CONTROL | what SELECT does: BPM / PAGE / MODW / VEL | **M** | re-defines a main encoder; violates "always shows what each knob does" unless labelled | menu_items.c:163-164, 271-273; ui_input.c:1769-1786 |
| `MI_LOWCUT` | AUDIO | SPEAKER EQ FLAT / LOWCUT / BASS+ | S | | menu_items.c:111-115 |
| `MI_USB` | AUDIO | USB LEVEL MASTER / FIXED | S | | menu_items.c:98 |
| `MI_CLICK` | AUDIO | metronome OFF / REC / ON | **P** | also on the REC layer (G3, K1) | menu_items.c:117-119; ui_layer.c:452-478 |
| `MI_CLKLVL` | AUDIO | click level | **P** | REC layer (B3, K3) | menu_items.c:119; ui_layer.c:452-478 |
| `MI_COUNTIN` | AUDIO | count-in OFF / 1 / 2 bars | **P** | REC layer (A3, K2) | menu_items.c:120; ui_layer.c:452-478 |
| `MI_TUNE` | AUDIO | project tuning -50..+50 cents | S | project value (song.g G_TUNE) | menu_items.c:67-70, 279 |
| `MI_MIDIIN` | MIDI | **MIDI IN block** CH1-4 / SEL / CH5-8 / CH9-12 / CH13-16 / TRACK; the same block sets the **MIDI OUT channel of keys** of tracks 1-4 (`trk_midi_ch`) | **M** | the "set channel's MIDI channel" step of the target workflow is here | menu_items.c:28, 280; seq.c:78-92 |
| `MI_TCH1` | MIDI | T1 channel (1-16, OMNI, OFF), used when MIDI IN = TRACK | **M** | MIDI channel of channel 1: reference workflow step 1 | menu_items.c:30, 59, 281-283; seq.c:86-92 |
| `MI_TCH2` | MIDI | T2 channel | **M** | as above | menu_items.c:59, 281-283 |
| `MI_TCH3` | MIDI | T3 channel | **M** | as above | menu_items.c:59, 281-283 |
| `MI_TCH4` | MIDI | T4 channel | **M** | as above | menu_items.c:59, 281-283 |
| `MI_MFOLLOW` | MIDI | AUTO SELECT: incoming notes select their track | S | | menu_items.c:108; ui_input.c:1427-1450 |
| `MI_SCRWAKE` | MIDI | SCREEN WAKE +MIDI / PANEL | S | | menu_items.c:109; ui.c:298-325 |
| `MI_LCLEAR` | MIDI | LEARN CLEAR: OCT+ clears every learned CC (action, no value) | I | MIDI LEARN itself is on the GLO layer (D4) | ui_menu.c:548-550 |
| `MI_SERIAL` | SYSTEM | USB SERIAL on/off (re-enumerates USB) | S | | menu_items.c:100, 139-148 |
| `MI_RESTORE` | SYSTEM | RESTORE LAST: autosave on/off | S | | menu_items.c:101 |
| `MI_HELP` | SYSTEM | page hints in the footer | S | | menu_items.c:107 |
| `MI_T1SEQ` | SYSTEM | **T1 SEQ ON: switches the whole panel to the T-1 algorithmic sequencer** | **M** | the only way to reach any algorithm; a mode switch behind the menu, in the opposite of "no menu diving" | menu_items.c:106; ui_input.c:1510-1519; ui_t1.c:27-41 |
| `MI_PANEL` | SYSTEM | CALIBRATION (also at power-on) | I | opens with OCT+ | ui_menu.c:551-554 |
| `MI_INFO` | SYSTEM | CPU/RAM information screen | I | opens with OCT+ | ui_menu.c:555-558 |
| `MI_ABOUT` | SYSTEM | about / credits (PRESETS scrolls) | I | opens with OCT+ | ui_menu.c:559-563 |

37 rows (`MI_COUNT`, menu_items.c:18-19). Counted: DISPLAY 8, CONTROL 8, AUDIO 6, MIDI 8, SYSTEM 7.

Other things reachable **only** through the menu or with no panel gesture, found while reading:

- **The MIDI output channel of each Felucca track** (MI_MIDIIN + MI_TCH1-4). On the panel there is none.
- **The T-1 mode switch** (`MI_T1SEQ`). While in T-1 mode a *menu visit* is required to leave it (HOME hold).
- **In T1 mode only**: nothing on the panel shows or sets "algorithm"; the T-1 pages are the algorithm's parameters
  (Euclid, ratchet, harmony). Felucca mode has no algorithm concept at all: the only generative behaviour is SEQ
  TOOLS RANDOM / COOK / EUCLID and ARP.
- Felucca's `EN_ALGO` does **not** select an algorithm despite its printed name (it picks the track).
- Track 1..4 MIDI out channel for Felucca's **own sequencer** is not applicable: it only drives the internal synth;
  MIDI out of notes happens for live key presses and T1 tracks 2-6 (CLAUDE.md, seq.c:728, 746).

## 7. Open questions / unconfirmed

1. T1 LEDs (section 4): no T1 branch in `ui_leds()`; actual behaviour on hardware not observed.
2. `kb_mask` is set by `layer_masks()`, which `ui_input()` skips in T1 mode; if a layer button was held when T1
   mode turned on a stale mask might persist, but T1 mode's `song.grid = 2` silences every key anyway [unconfirmed].
3. MIDI LEARN: whether keys are intercepted while it is on was not traced.
4. Exact page order of the EDIT family depends on `page_visible()` per engine (ui.c:425-450); only the static
   order is listed.
5. ENV, ARP: held for longer than the layer threshold they behave as a tap (no layer) [code: default branch,
   ui_input.c:1719-1729].
6. The `MI_LATCH` and `MI_SELECT` "M" classifications are judgement; they are listed so P2-001 can decide.
