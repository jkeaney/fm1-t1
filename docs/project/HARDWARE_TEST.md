# Hardware test routine

Every hardware session follows this page, so results are repeatable and owner feedback reaches the backlog. The
device rules are in [CLAUDE.md](../../CLAUDE.md) (Flashing, restoring and testing). Never flash without asking the
owner first.

## Setup

Three wirings, depending on what is connected. **USB-C on the FM-1 is a USB device only and the 3.5 mm TRS jack is MIDI
IN only, so the FM-1 can never drive the TB-3 by itself: something with a USB host port sits in between.**

```
Test bridge (Mac, no DIN box needed):

  FM-1 --USB-C--> Mac --tools/fm1_bridge.py--> TB-3 (USB-MIDI)

Ableton route (the HW-1 default; also the HW-T route):

  FM-1 --USB-C--> Mac (Ableton Live) --USB--> TB-3 (USB-MIDI)
  Ableton: a MIDI track, Monitor In, MIDI From = the FM-1 port ("JIANT FM"), MIDI To = TB-3, channel 2.
  In Preferences > Link/Tempo/MIDI, the FM-1 input has Track on, Sync OFF and Remote OFF (the FM-1 runs on its own
  internal clock at HW-1; following Ableton is a known gap, below).

USB-MIDI host box (standalone, no computer):

  FM-1 --USB-C--> host box --DIN MIDI OUT--> TB-3 MIDI IN
```

- The TB-3 receives on **channel 2** by default. The FM-1 sends CH1 on MIDI 1 at power-on, so HW-1 sets it to 2.
- Without Ableton, `tools/fm1_bridge.py` is the same route: it forwards notes and CC from the FM-1 to the first "TB-3" port.
  Use one of the two, never both at once.
- The 3.5 mm TRS jack is **MIDI IN only**. It cannot drive a TB-3.
- The bridge drops MIDI clock and transport unless started with `--clock`, so the TB-3's own sequencer does not start.

## Tools

```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"   # identity only, read-only
.venv/bin/python tools/fm1_bridge.py                          # FM-1 -> Mac -> TB-3 (blocks; Ctrl-C to stop)
.venv/bin/python tools/fm1_bridge.py --list                   # list MIDI ports
.venv/bin/python tools/fm1_midi_listen.py --seconds 120 --out /tmp/session.log
```

- `fm1_bridge.py` forwards notes and CC to the TB-3. Run it in one terminal.
- `fm1_midi_listen.py` only listens. It logs every non-clock message with a timestamp and ends with a `SUMMARY`
  line: note-ons per MIDI channel, note and velocity histograms, and the CC123 count. Use it alone to check what the
  FM-1 sends, or to record a session. Do not run it at the same time as the bridge on the same port.
- If the FM-1 is not found, the listener prints `no FM-1 MIDI port found` and exits 2. Plug the FM-1 in, or pass
  `--port`.

## Flashing and restoring

**Always ask the owner before flashing.** Nothing here is run by an agent. In update mode the device enumerates as
"JIANT Update", so every command passes `--port "JIANT"`.

```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"                          # before and after: identity, read-only
.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes        # flash the build
.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"  # back to stock; then power-cycle
```

- If an install stops at "update loader did not appear", rerun with `--port "JIANT"`; the device waits in update mode.
- A build that crashes at boot cannot enter update mode: recovery then needs FM-1-transporter (JieLi mask-ROM USB boot).
- On stock V15 `--info` needs `--port "FM-1"`. Felucca builds report `FM-1_9xx`.
- Record the git SHA of the flashed build in every feedback item (`git rev-parse --short HEAD`).

## Sessions

Each item is a checkbox with the expected sound or screen text. Screen text is quoted as drawn (SCREEN_SPEC_V2,
`overlay_text.tsv`, `page_states.tsv`). A failed item becomes feedback (below); do not fix it in the session. Session
IDs are the ones the hardware tasks use: HW-1 (P4-021), HW-T (P4-022), HW-2 (P5-009), HW-3 (P6-008), HT-FORCE, HW-4
(P7-004), Release (P8-004).

### HW-1 core: first flash of the new sequencer, up to PLAY on the TB-3 (P4-021)

Source: SCREEN_SPEC_V2, SEQUENCER_UX 7 W1 and 11.3/11.4, scenarios S01 and S07, the key/knob/button tables. Setup:
Ableton route (or bridge) to the TB-3 on channel 2. Start the listener for the whole session:
`.venv/bin/python tools/fm1_midi_listen.py --seconds 600 --out /tmp/hw1.log` (stop the bridge first if it runs; Ableton
and the listener cannot share the port either: if Ableton holds it, read the TB-3 instead and skip the `SUMMARY` checks).

**A. Flash and boot**
- [ ] The owner has said "go" for flashing. `fm1_install.py --info --port "JIANT"` ran before the install and shows the old identity.
- [ ] Install `build/felucca.fwsc` as in Flashing and restoring. The FM-1 reboots and `--info` reports `FM-1_9xx`.
- [ ] Power-cycle. A splash (about 0.4 s) then the PERF screen: header `INT 120`, chip `1` selected, `ACID`, `MIDI 1`; four knob tiles `DENS 12`, `SLIDE 4`, `JUMP 5`, `ACCNT 3`; 12 button tiles; the keybed footer. No crash, no reboot loop.
- [ ] First-run strip: the footer is inverted `CHN KEY, K2 = MIDI   CH1` and the D#4 (CHN) black key is lit. The amber/INT tempo shows `120`.
- [ ] Nothing sounds on its own while stopped. The speaker is silent (PREVIEW is off at power-on).

**B. CH1 to MIDI 2 (two gestures)**
- [ ] Tap black key D#4 (CHN). Page CHAN: slot `CHN CH1 SETUP`, tiles `LENGTH 16`, `MIDI 1`, `SOURCE OWN`, `FOLLOW ON`, footer `K2 = MIDI` / `CHN = BACK`.
- [ ] Turn K2 one detent clockwise: tile `MIDI 2`, header `MIDI 2`. The first-run strip changes to `PRESS PLAY`. After about 0.6 s CH2 turns to MIDI OFF with the toast `CH2 IS MIDI OFF: CH1 TAKES 2` (the TB-3 default clashes with CH2's default; this is the unique-MIDI rule).
- [ ] Tap HOME: back to PERF, strip `PRESS PLAY`, header `ACID` `MIDI 2`.

**C. PLAY, ACID on the TB-3, STOP**
- [ ] Press PLAY. Header shows the play triangle; the playhead column and a white pointer run over the 16 steps; the PLAY LED is green; the strip moves to `TURN K1 TO MORPH IT`.
- [ ] The TB-3 plays a bassline on channel 2 (listener `SUMMARY`: note-ons on ch2 only; the bridge prints `ch2 note`). About 12 hits per 16 steps at DENS 12, some with slides, some accented.
- [ ] Turn K1 (DENS) two detents clockwise: tile `DENS 14`, more steps lit on the keys and in the music row, 14 hits per bar from the next step. K2 (SLIDE) up: more slides (the TB-3 glides). The tile shows the new value in the same frame.
- [ ] Press SCL (NOTES dice): toast `[1] NOTES  EDIT=UNDO`, same rhythm, new pitches from the next step. Press EDIT: toast `[1] UNDO  EDIT=REDO`, the old pitches return.
- [ ] Press PLAY again to stop. The notes stop at once and nothing hangs on the TB-3. The listener shows an `ch2` note-off for any sounding note and `cc123` count rises by at least 1 (`B1 7B 00`).
- [ ] Start and stop five times: no stuck note, no double start.

**D. Step entry on the white keys**
- [ ] Stopped. Tap a lit white key (a step that is ON): the step turns off (key front dark, bar becomes a floor dash). Tap an unlit key: it turns on and **sounds once** on the TB-3 for about 120 ms (P4-032). Tapping a step off is silent.
- [ ] Playing: tapped steps change what the TB-3 plays from the next pass; a tap while playing does not sound extra.
- [ ] Set LENGTH to 8 (D#4, K1 down to 8, HOME). Tap white key 12: toast `PAST THE END  LEN`, nothing written, no sound. Set LENGTH back to 16.

**E. NOTE_EDIT**
- [ ] Hold a white key for 0.4 s: page NOTE_EDIT, header chip `STEP n`, tiles `PITCH` `GATE` `VELO` `RATCH` all `AUTO`, footer `HOLD KEY = MOVE` / `HOME = DONE`.
- [ ] Turn K1 one detent: the tile leaves AUTO and shows a note name; the step plays the new note on the next pass (a detent should also sound it; see the audition note in Known gaps if silent). Turn K1 back to its low end: `AUTO` again.
- [ ] K3 up to `ACC` (velocity 100 or more): the TB-3 accents that step. K2 to `TIE`: the note holds into the next step.
- [ ] Hold another white key: the editor moves to that step. HOME tap: back to PERF with the edit kept.

**F. REC keys sound (P4-034)**
- [ ] Tap REC: red disc in the header, REC button red, tiles `CURSOR 1` `ACC OFF` `SLIDE OFF` `OCT 0` in white, a white frame round step 1, footer `PLAY KEYS TO WRITE` / `REC = DONE`.
- [ ] Press and hold a white key (C4): the TB-3 sounds the note on channel 2 **at the press** and stops at the release (listener: `ch2 ON 60 90`, then `ch2 OFF 60`). Step 1 fills and the cursor moves to step 2.
- [ ] Press a black key (F#3): it also sounds and writes (it is a note here, not MUTE1). OCT- / OCT+ move the REC octave and clamp at -2 / +2.
- [ ] K2 ACC ON, press a key: velocity 120 (listener) and the TB-3 accents it. EDIT removes the last written note and the cursor steps back. REC (or HOME) leaves REC; no note hangs.
- [ ] Playing in REC: keys write to the step under the playhead, and K1 only frames its tile.

**G. ALGO encoder, mutes, channels**
- [ ] Turn the ALGO encoder one detent: slot `ALGO <NAME> AT STEP`, the cue strip lists the algorithms. With only ACID built the pattern stays ACID (see Known gaps). Turning with an M key held does nothing.
- [ ] Tap black F#3 (M1): CH1 mutes, its notes stop, its chip shows muted; tap again: notes return from the next step.
- [ ] Turn SELECT one detent clockwise past the last channel: a channel is added, toast `[2] NEW  ALGO=PICK`, footer cue `CHN KEY, K2 = MIDI   CH2`. Hold G#3 (M2): at 0.4 s the slot reads `PREVIEW CH2`, at 1.5 s `REMOVE CH2` with a filling bar, then toast `[2] GONE  EDIT=UNDO`. EDIT restores it.
- [ ] Hold M2 and release before 1.5 s: PREVIEW toggles with a toast naming the state (`PREVIEW ON` / `PREVIEW OFF`); with it on, the speaker plays that channel.

**H. SET, RESET, HELP**
- [ ] Tap GLO: page SET, slot `GLO ALL CHANNELS`, tiles `BPM 120` `SWING 50%` `GATE 50%` `DIV 1/16`, status `OUT MASTER` (playing) or `OUT ON PLAY` (stopped), `OCT 0`, footer `KEYS = STEPS` / `GLO = BACK`.
- [ ] Turn K1 up eight detents: `BPM 128`, header `128`, the TB-3 line speeds up. K2 SWING up: odd steps late. OCT+ then OCT-: slot `OCT+ OCTAVE +1`, notes an octave up, then back. GLO tap returns to PERF. (Holding GLO does nothing yet: Known gaps.)
- [ ] Hold EDIT 0.4 s (on PERF): the hint `RESET` fills from 0.25 s; at 0.4 s the knob labels invert, buttons and white keys get outlines, D#4 lights, slot `RESET` + `TOUCH ONE`, footer `TOUCH WHAT TO RESET` / `EDIT = NO`. Turn K1: it goes back to its default, toast `[1] RESET  EDIT=UNDO`, and RESET closes. EDIT undoes it. Open RESET again and press EDIT: it cancels with no change.
- [ ] Hold HOME and turn K1: a `HELP` box covers the tiles and music row with two lines for K1; the knob does not change; slot `HELP` + `LET GO = BACK`. Release HOME: back, nothing changed, no menu. Hold HOME and press PLAY: PLAY still plays.
- [ ] Hold HOME alone for 0.7 s: the Felucca MENU opens. HOME tap leaves it.
- [ ] A HOME tap on PERF (nothing touched): toast `HOME  HOLD=HELP`, the first-run strip ends.

**I. First run steps 2 to 7 (only on a fresh strip; power-cycle plus the old state if needed)**
- [ ] The footer strip walks `PRESS PLAY`, `TURN K1 TO MORPH IT`, `PRESS FX: NEW PATTERN`, `TAP A WHITE KEY: ON/OFF`, `HOLD A WHITE KEY: EDIT`, `PG+ TWICE: ADD PAGE 2`, each advancing on the event it names, with `HOME = SKIP` on the right.
- [ ] PG+ (D#5) on page 1 at LENGTH 16: toast `PAGE 2?  PG+=ADD`; PG+ again adds page 2 (LENGTH 32, `PAGE 2/2`). Any other control cancels the prompt.

**Sign-off.** Record the firmware SHA, the route used, and file an `FB-nnn` for each failed item. `Approved: <owner> <date> <SHA>` in the P4-021 log.

### HW-T timing gate G1 to G7 (P4-022)

Runs on the timing test image (`build/felucca_timing_test.fwsc`, P4-019), not the normal build; reflash the normal build
afterwards. Equipment: FM-1 on USB to the Mac, Ableton Live 12, `tools/fm1_timing.py`, then the TB-3. The pass criteria
are in [TIMING_SPEC.md](../design/TIMING_SPEC.md) 6.4 (G1 to G7); the sync settings go in the log.

```sh
.venv/bin/python tools/fm1_timing.py --loopback --seconds 30                       # host noise floor first
.venv/bin/python tools/fm1_timing.py --minutes 5 --bpm 120 --out /tmp/fm1_timing   # G1: repeat at 60 90 120 180 240
.venv/bin/python tools/fm1_timing.py --sweep                                       # all default tempos in one go
.venv/bin/python tools/fm1_timing.py --follow --host-port "IAC" --bpm 120          # G3: FM-1 follows Ableton (needs P4-031)
.venv/bin/python tools/fm1_timing.py --from-log /tmp/fm1_timing_120.log            # re-analyse a saved log
```

- [ ] G1 FM-1 sends, no DAW sync: all of T2, T3, T5, T6, T7, T8, T12 pass at 60, 90, 120, 180, 240 BPM; `tm_rel` peak-to-peak at most 0.25 ms.
- [ ] G2 Ableton records the FM-1 clock: notes within 1.5 ms of the grid, Ableton tempo `120.0 +-0.05`.
- [ ] G3 FM-1 follows Ableton: header `EXT <bpm>`, T9 zero drift in 10 minutes, T10, T11, T13 (only once external clock P4-031 is built).
- [ ] G4 hand-over: cold start, start and stop Ableton, pull the cable while playing; no stuck note on the TB-3.
- [ ] G5 FM-1 to TB-3: 10 minutes on channel 2 at 120 and 180: no stuck or missed notes, groove tight against a metronome.
- [ ] G6 CPU: `felucca_dbg.max_us` rises under 5 %; `txr_drops` and `sq_drops` are zero.
- [ ] G7 LEDs: PLAY green mid level and beat flash as TIMING_SPEC 5.4.
- [ ] Decision recorded: nested TIMER5 release kept, or the fallback flag set.

### HW-2 algorithms and HARMONY on the TB-3 (P5-009)

Source: `knob_map.tsv`, ALGORITHM_SET. For each of ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS turn the ALGO encoder to it
(slot `ALGO <NAME> AT STEP`; it lands at the next step) with CH1 on MIDI 2, then judge by ear and tick pass or fail:
- [ ] ACID: K1 DENS 12, K2 SLIDE, K3 JUMP, K4 ACCNT; steps, notes, slides and accents are audible on the TB-3; OCT- / OCT+ switch to ROW 2 (OFFSET PEDAL PUNCH DIR).
- [ ] EUCLID: K1 HITS 5 gives an even 5-in-16 rhythm; K2 OFFSET rotates it; K3 RATCH, K4 ACCNT audible.
- [ ] SNAKE: K1 DENS, K2 LEN, K3 RANGE, K4 PATH change the line; ROW 2 DIR FWD / BWD / PEND.
- [ ] STAGE: K1 DENS, K2 TOTAL, K3 DIR, K4 RATCH; ROW 2 OFFSET RDIV DYN RANGE.
- [ ] ARP: K2 SHAPE (LINE SKIP THUMB OCTV), K3 SPAN, K4 RATE; the toast names the figure length.
- [ ] PATTERNS: ROW 2 S1 PICK walks the library; K1 DENS, K2 VARY, K3 SPAN, K4 ACCNT.
- [ ] Each algorithm: dice FX (ALL) and each layer button re-rolls only its layer; `EDIT` takes it back; the same settings replay the same pattern after a power cycle.
- [ ] HARMONY (hold GLO 0.4 s): default changes nothing audible. Type 1-4-5-1 with F3, B3, C4, F3 (keys F3 to E4 = chords 1 to 7): the TB-3 line follows the chords with ROLE ROOT and again with ROLE FULL. GLO tap leaves.
- [ ] Verdict per algorithm written in the log; `FB-nnn` for each problem; `Approved:` line by the owner.

### HW-3 the whole workflow without the menu (P6-008)

Use a stopwatch from power-on to a performed sequence (VISION reference workflow). The menu is not opened.
- [ ] Power on, set CH1 to MIDI 2, choose an algorithm, enter or dial in steps, press PLAY, perform. Time taken: ____ s.
- [ ] Every knob shows its role on screen at every step (tile labels, button icons, footer line, held-key hints).
- [ ] Morph: turn each of K1 to K4 on both rows (OCT- / OCT+); the sound changes from the next step and turning back restores it.
- [ ] Dice: FX (ALL), SCL (NOTES), ENV (ACCNT), LFO (SHAPE), ARP (ARTIC), SEQ (HITS) each re-roll only their layer (the hatch in the music row shows which steps changed); a layer the algorithm lacks is dim and inert.
- [ ] UNDO: EDIT takes back each roll, a second EDIT redoes it.
- [ ] Mute (M1 to M4 tap), SOLO (hold F#4: only the selected channel sounds, release restores), FILL (hold G#4: denser while held), ROLL (hold A#4: steps repeat 2x while held), RST (F#5: all channels to step 1). Each judged by ear.
- [ ] Two to four channels on different MIDI channels play together, SELECT moves between them.
- [ ] The menu is only reached by holding HOME 0.7 s and was never needed.

### HT-FORCE key-force gate (before FORCE defaults to NORMAL)

Not part of the first sessions. Runs on a debug readout build; criteria G1 to G7 and H0 are in
[SEQUENCER_UX.md](../design/SEQUENCER_UX.md) 6.6 (G1 accent bins, G2 LED disturbance, G3 scan integrity, G4 CPU, G5
latency, G6 pressure, G7 REC levels; H0 TB-3 accent is already verified: 100 and above accents).
- [ ] G1 20 soft + 20 hard taps on each of W01..W12: at least 90 % per key, 95 % overall.
- [ ] G2 a lit neighbour moves the reading by under 10 % of the soft-hard gap.
- [ ] G3 10 minutes of play: no false OCT hold, no stuck key.
- [ ] G4 scan ISR load +3 % at most; audio `max_us` within +5 %.
- [ ] G5 REC note-on within 8 ms of key-down, note-off within 6 ms of release.
- [ ] G6 ROLL 2x / 3x / 4x reached on purpose at least 80 % of tries.
- [ ] G7 16 notes (4 soft, 8 medium, 4 hard) on each of 6 force keys: at least 85 % in the intended level.
- [ ] The owner signs the gate in the task log. On failure of G1 or G2 `FORCE` stays OFF.

### HW-4 storage: save, power-cycle, load (P7-004)

- [ ] Build three different patterns. Tap SAVE to save each into its slot (toast `SAVED Pnn`).
- [ ] Change globals on SET (BPM, SWING, GATE, DIV, OCT). Power-cycle twice. The BPM and globals are still set and the three presets load with the same notes.
- [ ] Load a preset with PRESETS: it lands at the next bar (`P02 AT BAR`; stopped: `P02 IN 1.5S`). The pending slot shows `P02?` until it lands.
- [ ] Select an empty slot (for example P09): the screen reads `P09 EMPTY: NOTHING LOADED` and the pattern is unchanged.
- [ ] `UNDO LOAD` (hold SAVE 0.7 s) brings the previous pattern back.
- [ ] Flash the same build again over the top: the presets survive an update.
- [ ] A pending first run does not return after a power-cycle (the byte persists).

### Release acceptance (P8-004)

Flash `release/v1/felucca.fwsc`. All of HW-1 to HW-4 items pass again on the release image, plus:
- [ ] `fm1_install.py --info --port "JIANT"` reports the release version, and the checksum matches the changelog.
- [ ] The target workflow (VISION reference workflow) is performed on the TB-3 without opening the menu.
- [ ] Soak: 30 minutes at 120 BPM with four channels: no stuck note, no dropped note, no reboot, no screen corruption.
- [ ] Unplug USB while playing: the FM-1 stays up; replug: it recovers (no hung notes on the TB-3 after reconnect).
- [ ] Restore to stock with `restore/FM-1-official-V15.fwsc` and reflash the release: works.
- [ ] `Approved: <owner> <date> <SHA>`; v1 is done and the owner decides on push and tag.

## Known gaps at HW-1 (do not report as bugs)

These are designed, filed and not built yet. A failure that matches one is not feedback.
- **External clock follow (P4-031):** the FM-1 runs on its internal clock only. Ableton Sync input must stay off. A host clock may still disturb the BPM on the device (`midi_clock.c` overwrites the tempo); the header `EXT` and `WAIT` states exist but the source is not wired.
- **K3 SOURCE on CHAN is inert (P4-033):** a new page copies the page before it.
- **SOL, FIL, ROL (F#4, G#4, A#4) are inert beyond naming themselves in the slot (P6-003)**, and so are RST (F#5) and the key-force versions of them.
- **PRESETS encoder (P7-002):** turning it does nothing; SAVE and load are not built, so HW-4 cannot run at HW-1.
- **Hard-tap ACCENT** (key force, P6-006): no accent from key pressure. Use the ENV button, K3 VELO or REC `ACC`.
- **Hold GLO = HARMONY (P5-006):** holding GLO does not open HARMONY; GLO acts on the press and opens SET.
- **Only the ACID algorithm is filled:** EUCLID, SNAKE, STAGE, ARP and PATTERNS are empty rows, so the ALGO encoder stays on ACID (P5).
- **Live MIDI IN notes share slots** with the sequencer: a note played in from a keyboard can hit the same slot as a sequencer note.
- **`check_control_map.py` reports 78 failures** (RIG rows, the ALGO hold clause). That is the recorded baseline, unrelated to the unit.
- Also known: the NOTE_EDIT knob audition (a K1 detent sounding the note) is debt of P4-014; the preview-sound turn (M key held plus ALGO) is not wired; RIG (hold D#4) is not built.

## Things to judge by eye on the unit

For the owner, at HW-1, at the normal 30 to 40 cm viewing distance. Each is a judgement; file `FB-nnn` with a verdict.
- [ ] 9 px text (AF_S) reads at 30 mm for the touched items: the HELP tag and page name in the HELP box, `EDIT = NO` in the RESET footer, `HOME = SKIP` in the first-run strip. Expected: legible without leaning in.
- [ ] The terse hints `PAST THE END  LEN` and `ONE PAGE  K1=LESS` are understood on first sight. The P4-032 reviewer suggested `K1=LEN` and `PG+=ADD` instead. Verdict: keep or change.
- [ ] Keys past LENGTH are dimmed (key_map says "dim" and SCREEN_SPEC_V2 4.4 says hatched `T_LINE`). With LENGTH 8, keys 9 to 16 look different from keys 1 to 8. Not verified in code.
- [ ] EDIT tap acts on the release: a quick tap undoes with the toast `UNDO` right after the finger lifts; a 0.4 s hold arms RESET instead. It does not feel late or ambiguous.
- [ ] HOME hold opens the MENU at 0.7 s: the hint `HELP / MENU` fills and the menu appears at the bar's end, not on release. 0.7 s feels right, not too slow or too easy to trigger.
- [ ] The first-run strip appears once: power-cycle after finishing or skipping it. The second boot shows the normal footer `KEYS = STEPS` with no strip.
- [ ] Known cosmetic gaps from the screen lock (SCREEN_SPEC_V2 9): 3 px strips of the other channels, the playhead column on black, the knob fill read as "selected", the six dice as a group, the missing MIDI cue once set, step on/off drawn twice.
- [ ] First timing observation, not a measurement: with four channels playing at 120 BPM for 5 minutes, listen on the TB-3 for jitter (uneven 16ths), flams or dropped notes, and a late first note after PLAY. Note any. The real measurement is P4-019/P4-022.

## Running a session

1. Connect the setup above and power the FM-1 on.
2. Start the listener in one terminal (`--seconds` long enough for the session), and the bridge in another if the
   TB-3 is connected to the Mac.
3. Work through the session checklist (Sessions above) and tick items in a copy of this file for the session, or
   note the results in the log.
4. Read the `SUMMARY` line in the listener output, and keep the `--out` log with the session notes.

## Filing feedback

Any problem seen on hardware becomes a backlog item, not a note in chat.

1. Copy `backlog/_templates/feedback.md` to `backlog/feedback/FB-nnn-<short-name>.md` (next free number).
2. Fill in the front matter: `session` (date, firmware git SHA, setup, for example `FM-1 -> bridge -> TB-3`).
3. Put what you did, what you saw or heard, and what you expected under **Observation**. Attach the
   `--out` log path or a recording path.
4. Run `python3 tools/backlog.py check`. The next `/next-task` run triages it into a task or bug, or drops it with
   a reason.

Bugs found on hardware close only after the owner's `hardware-test` sign-off (see [PROCESS.md](PROCESS.md)).
