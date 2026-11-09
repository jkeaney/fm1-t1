#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Log every non-clock MIDI message from the FM-1 for a hardware test session.

Listens on the FM-1's USB-MIDI input port (or --port), writes each message with a timestamp to --out, and ends
with a SUMMARY line: note-on count per MIDI channel, note histogram, velocity histogram and CC123 count.
MIDI clock and active sensing are skipped to keep the log readable.

  .venv/bin/python tools/fm1_midi_listen.py                     # 120 s, FM-1 port, log in /tmp
  .venv/bin/python tools/fm1_midi_listen.py --seconds 30 --out /tmp/session.log
  .venv/bin/python tools/fm1_midi_listen.py --list              # list input ports

Exit codes: 0 ok, 2 no FM-1 MIDI port found (or it is ambiguous/unreadable).
Needs: pip install mido python-rtmidi (both in .venv/).
"""
import argparse
import collections
import sys
import time

FM1_HINTS = ("jiant fm", "fm-1", "felucca")
EXCLUDE = ("update",)  # the bootloader enumerates as "JIANT Update"; it sends no music


def find_port(names, wanted=None):
    """Return the FM-1 input port name, or None. `wanted` is a substring overriding the defaults."""
    hints = (wanted.lower(),) if wanted else FM1_HINTS
    for n in names:
        low = n.lower()
        if any(h in low for h in hints) and not any(x in low for x in EXCLUDE):
            return n
    return None


def summarise(ons, vels, chans, cc123, first_at):
    notes = " ".join(f"{k}:{v}" for k, v in sorted(ons.items())) or "-"
    vel = " ".join(f"{k}:{v}" for k, v in sorted(vels.items())) or "-"
    ch = " ".join(f"ch{k}:{v}" for k, v in sorted(chans.items())) or "-"
    return (f"SUMMARY note_ons={sum(chans.values())} per_channel=[{ch}] cc123={cc123} "
            f"notes=[{notes}] velocities=[{vel}] first_note_at={first_at}")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", help="input port name (part of it); default: the FM-1 (JIANT FM / FM-1 / Felucca)")
    ap.add_argument("--seconds", type=float, default=120.0, help="how long to listen, default 120")
    ap.add_argument("--out", default="/tmp/fm1_midi_listen.log", help="log file path")
    ap.add_argument("--list", action="store_true", help="list MIDI input ports and exit")
    a = ap.parse_args(argv)

    try:
        import mido
    except ImportError:
        print("mido is not installed: run with .venv/bin/python (pip install mido python-rtmidi)", file=sys.stderr)
        return 2

    if a.list:
        print("inputs:", mido.get_input_names())
        return 0

    port = find_port(mido.get_input_names(), a.port)
    if port is None:
        print(f"no FM-1 MIDI port found (inputs now: {mido.get_input_names()}). "
              "Plug in the FM-1 over USB-C, or pass --port.")
        return 2

    ons = collections.Counter()    # note -> count
    vels = collections.Counter()   # velocity -> count
    chans = collections.Counter()  # MIDI channel (1-16) -> count
    cc123 = 0
    first_at = None
    t0 = time.monotonic()
    try:
        with mido.open_input(port) as inp, open(a.out, "w") as log:
            log.write(f"# port={port!r} seconds={a.seconds} started={time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            print(f"listening on {port!r} for {a.seconds:g} s -> {a.out}  (Ctrl-C to stop early)", flush=True)
            while time.monotonic() - t0 < a.seconds:
                for m in inp.iter_pending():
                    t = time.monotonic() - t0
                    if m.type in ("clock", "active_sensing"):
                        continue
                    log.write(f"{t:8.3f} {m}\n")
                    log.flush()
                    if m.type == "note_on" and m.velocity > 0:
                        first_at = first_at if first_at is not None else round(t, 3)
                        ons[m.note] += 1
                        vels[m.velocity] += 1
                        chans[m.channel + 1] += 1
                    elif m.type == "control_change" and m.control == 123:
                        cc123 += 1
                time.sleep(0.002)
            log.write(summarise(ons, vels, chans, cc123, first_at) + "\n")
    except KeyboardInterrupt:
        pass
    except OSError as e:
        print(f"could not open {port!r}: {e}", file=sys.stderr)
        return 2
    # Summary also to the console, so the session result is visible without opening the log.
    print(summarise(ons, vels, chans, cc123, first_at))
    return 0


if __name__ == "__main__":
    sys.exit(main())
