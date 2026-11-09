# Hardware test routine

Every hardware session follows this page, so results are repeatable and owner feedback reaches the backlog. The
device rules are in [CLAUDE.md](../../CLAUDE.md) (Flashing, restoring and testing). Never flash without asking the
owner first.

## Setup

Two wirings, depending on what is connected.

```
Test bridge (Mac, no DIN box needed):

  FM-1 --USB-C--> Mac --tools/fm1_bridge.py--> TB-3 (USB-MIDI)

USB-MIDI host box (standalone, no computer):

  FM-1 --USB-C--> host box --DIN MIDI OUT--> TB-3 MIDI IN
```

- The TB-3 receives on **channel 2** by default.
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

## Smoke checklist

Tick each item while running the session. Note any failure in the log, then file feedback (below).

- [ ] Boot: the FM-1 powers up to the main screen with no crash or reboot.
- [ ] Identity: `fm1_install.py --info` reports the expected firmware (`FM-1_9xx` for Felucca builds).
- [ ] MENU → SYSTEM → T1 SEQ is ON, and the T-1 screen shows on the main page.
- [ ] PLAY: the transport starts and the screen shows that it is playing.
- [ ] Notes: the TB-3 plays notes on **channel 2** (listener: `ch2` note-ons, or bridge prints `ch2 note`).
- [ ] STOP: the transport stops and the FM-1 sends CC123 (listener `SUMMARY cc123` count rises by at least 1).
- [ ] Knobs: turning K1–K4 changes the on-screen card values.
- [ ] SEQ pages: the SEQ button changes pages, and each page shows its own labels.
- [ ] Keys: pressing a track key selects that track, and the screen shows the selection.
- [ ] Mute: muting a track stops its notes, and unmuting brings them back.

## Running a session

1. Connect the setup above and power the FM-1 on.
2. Start the listener in one terminal (`--seconds` long enough for the session), and the bridge in another if the
   TB-3 is connected to the Mac.
3. Work through the smoke checklist and tick items in this file's copy for the session, or note the results in
   the log.
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
