#!/usr/bin/env python3
"""Test bridge: FM-1 USB-MIDI -> computer -> another USB-MIDI device (e.g. a Roland TB-3).

Stands in for a USB-MIDI host box while testing the T1 sequencer. Forwards notes, CC, pitch bend and
aftertouch; drops clock / start / stop / continue / SysEx by default, so the target's own sequencer is
never started by the FM-1.

  python3 tools/fm1_bridge.py                       # FM-1 ("JIANT FM" / "FM-1" / "Felucca") -> first "TB-3" port
  python3 tools/fm1_bridge.py --to "TB-3" --clock   # also pass MIDI clock and transport
  python3 tools/fm1_bridge.py --list                # list ports

Needs: pip install mido python-rtmidi
"""
import argparse
import sys
import time

import mido

FROM_HINTS = ("jiant fm", "fm-1", "felucca")
REALTIME = {"clock", "start", "stop", "continue", "songpos", "song_select"}


def find(names, hints, exclude=("update",)):
    for n in names:
        low = n.lower()
        if any(h in low for h in hints) and not any(x in low for x in exclude):
            return n
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--from", dest="src", help="input port (part of its name); default: the FM-1")
    ap.add_argument("--to", default="TB-3", help="output port (part of its name), default TB-3")
    ap.add_argument("--clock", action="store_true", help="also forward clock and start/stop")
    ap.add_argument("--quiet", action="store_true", help="do not print each note")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()
    if a.list:
        print("inputs: ", mido.get_input_names())
        print("outputs:", mido.get_output_names())
        return 0

    src = dst = None
    shown = False
    while not (src and dst):                       # wait for both devices (plug them in any order)
        src = find(mido.get_input_names(), (a.src.lower(),) if a.src else FROM_HINTS)
        dst = find(mido.get_output_names(), (a.to.lower(),))
        if not (src and dst):
            if not shown:
                print(f"waiting for {'the FM-1' if not src else ''}{' and ' if not src and not dst else ''}"
                      f"{a.to if not dst else ''} ...  (outputs now: {mido.get_output_names()})", flush=True)
                shown = True
            time.sleep(1)
    print(f"bridging {src!r} -> {dst!r}  (Ctrl-C to stop)", flush=True)
    sent = 0
    with mido.open_input(src) as inp, mido.open_output(dst) as out:
        try:
            for msg in inp:
                if msg.type in ("active_sensing", "sysex") or (msg.type in REALTIME and not a.clock):
                    continue
                out.send(msg)
                sent += 1
                if not a.quiet and msg.type == "note_on" and msg.velocity:
                    print(f"ch{msg.channel + 1} note {msg.note:3d} vel {msg.velocity:3d}", flush=True)
        except KeyboardInterrupt:
            pass
        finally:
            for ch in range(16):                   # nothing left hanging on the target
                out.send(mido.Message("control_change", channel=ch, control=123, value=0))
    print(f"stopped: {sent} messages forwarded")
    return 0


if __name__ == "__main__":
    sys.exit(main())
