#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""FM-1 timing measurement (docs/design/TIMING_SPEC.md section 6.2). Mac side, no firmware change.

Captures the FM-1's MIDI output with python-rtmidi in a callback that takes the driver's own delta times (not a
polling loop with time.monotonic()). Every message is logged as `time_ns status d1 d2` (decimal, one per line, the
time in ns since the port opened, on a clock shared by the host and FM-1 logs of one run), including clock (0xF8),
Start (0xFA), Continue (0xFB), Stop (0xFC) and Song Position (0xF2). Then the log is analysed against the
thresholds of TIMING_SPEC section 2 (T1..T12) and the verdicts are printed, one line each.

  .venv/bin/python tools/fm1_timing.py --loopback                  # host noise floor on the IAC bus first
  .venv/bin/python tools/fm1_timing.py --minutes 5 --bpm 120       # FM-1 sends (set its tempo, press PLAY)
  .venv/bin/python tools/fm1_timing.py --sweep                     # 60 90 120 180 240, prompt before each
  .venv/bin/python tools/fm1_timing.py --follow --host-port IAC    # FM-1 follows a host; T9 and T10
  .venv/bin/python tools/fm1_timing.py --from-log /tmp/x.log --bpm 120   # re-analyse a saved log
  .venv/bin/python tools/fm1_timing.py --list                      # list MIDI input ports

Verdicts: PASS at or under the target, MARGIN between target and hard limit, FAIL over the hard limit. N/A rows
(T1: a device counter, TIMING_SPEC 6.1, not in the host log; T9/T10 in send mode; T2..T8 in follow mode) are not
counted. Exit 0 only if every counted row is PASS; 1 otherwise; 2 when the port or the setup is missing.

Send-mode note-on and note-off jitter are measured against a straight-line fit of the grid (the constant latency
and the tempo error are removed), using the grid index counted from the gaps, so a tempo error of a few hundred ppm
never mis-indexes a note. The test image plays one note on every 1/16 (TIMING_SPEC 6.1); missed notes and pulses
show up as holes in the index, not as shifted grids.

T4 (constant latency) is the note-on offset from its own clock tick, with the first 0xF8 after Start as the grid
origin. The FM-1's port alone cannot show the absolute grid tick, so an offset shared by notes and clock pulses
reads as 0; the Mac-side reference of TIMING_SPEC 6.4 (G2) is needed for the absolute figure. Reviewer: check this.
"""
import argparse
import bisect
import collections
import math
import os
import statistics
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fm1_midi_listen import find_port  # port finding only (TIMING_SPEC 6.2: this tool does not extend it)

NS = 1_000_000_000
CLOCK, START, CONTINUE, STOP, SPP = 0xF8, 0xFA, 0xFB, 0xFC, 0xF2
SWEEP_DEFAULT = "60,90,120,180,240"
DEVICE_ONLY = "device counter (TIMING_SPEC 6.1), not in the host log"
ORDER = ["T1", "T2", "T3", "T4", "T5", "T6", "T7", "T8", "T9", "T10", "T12a", "T12b"]

# id -> (label, unit, target (PASS at or under), hard limit (MARGIN up to it, FAIL over it)); TIMING_SPEC section 2.
LIMITS = {
    "T1": ("release lateness in the FM-1, p-p", "ms", 0.25, 0.40),
    "T2": ("note on/off jitter at host, p-p", "ms", 1.5, 2.5),
    "T3": ("note on/off jitter at host, RMS", "ms", 0.4, 0.7),
    "T4": ("constant latency, grid tick to host", "ms", 6.0, 10.0),
    "T5": ("internal tempo error", "ppm", 100.0, 200.0),
    "T6": ("clock quarters without exactly 24 pulses", "quarters", 0, 0),
    "T7": ("clock pulse spacing jitter, p-p", "ms", 1.5, 2.5),
    "T8": ("clock mean spacing error", "ppm", 100.0, 200.0),
    "T9": ("follow drift against the host", "steps", 0, 1),
    "T10": ("follow note jitter from host step, p-p", "ms", 3.0, 4.5),
    "T12a": ("note-on while the same note is still open", "count", 0, 0),
    "T12b": ("stuck notes 0.5 s after Stop", "count", 0, 0),
}

Row = collections.namedtuple("Row", "id label value unit verdict note")


# ---------------------------------------------------------------- log format

def write_log(path, events, header):
    with open(path, "w") as f:
        f.write(f"# {header}\n")
        for t, st, d1, d2 in events:
            f.write(f"{t} {st} {d1} {d2}\n")


def load_log(path):
    events = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            t, st, d1, d2 = (int(x, 0) for x in line.split()[:4])
            events.append((t, st, d1, d2))
    return events


# ---------------------------------------------------------------- analysis

def verdict(value, target, hard):
    if value is None:
        return "FAIL"
    v = abs(value)
    if v <= target + 1e-9:
        return "PASS"
    if v <= hard + 1e-9:
        return "MARGIN"
    return "FAIL"


def make_row(rid, value, na_reason=None):
    label, unit, target, hard = LIMITS[rid]
    if na_reason:
        return Row(rid, label, None, unit, "N/A", na_reason)
    note = "no data in log" if value is None else ""
    return Row(rid, label, value, unit, verdict(value, target, hard), note)


def fit_line(xs, ys):
    """Least-squares y = a + b*x. Returns (a, b, residuals), or None when x has no spread."""
    n = len(xs)
    if n < 2:
        return None
    mx = sum(xs) / n
    my = sum(ys) / n
    sxx = sum((x - mx) ** 2 for x in xs)
    if sxx == 0:
        return None
    b = sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sxx
    a = my - b * mx
    return a, b, [y - (a + b * x) for x, y in zip(xs, ys)]


def p2p(v):
    return max(v) - min(v)


def rms(v):
    return math.sqrt(sum(x * x for x in v) / len(v))


def worst(v, n=5):
    return sorted(v, key=abs, reverse=True)[:n]


def cumulative_index(times, period):
    """Grid index of each time (times relative to the grid origin). Each index is the previous index plus the
    rounded gap, so a tempo error never accumulates into a wrong index; holes are missed notes or pulses."""
    if not times:
        return []
    out = [round(times[0] / period)]
    for prev, t in zip(times, times[1:]):
        out.append(out[-1] + round((t - prev) / period))
    return out


def clock_origin(events):
    """(t0, pulses from t0): t0 is the first 0xF8 after the first Start (0xFA), else the first 0xF8."""
    pulses = [t for t, st, _, _ in events if st == CLOCK]
    if not pulses:
        return None, []
    t0 = pulses[0]
    starts = [t for t, st, _, _ in events if st == START]
    if starts:
        after = [p for p in pulses if p >= min(starts)]
        if after:
            t0 = after[0]
    return t0, [p for p in pulses if p >= t0]


def note_events(events):
    """(t, 'on'|'off', channel 1-16, note) for channel voice messages; a note-on with velocity 0 is an off."""
    out = []
    for t, st, d1, d2 in events:
        if st >= 0xF0:
            continue
        ch = (st & 0x0F) + 1
        kind = st & 0xF0
        if kind == 0x90 and d2 > 0:
            out.append((t, "on", ch, d1))
        elif kind == 0x80 or (kind == 0x90 and d2 == 0):
            out.append((t, "off", ch, d1))
    return out


def pair_notes(notes):
    """ons: on times in order. offs: (off time, position in ons of the on it closes). doubles: note-ons that
    arrived while the same channel and note was still open (TIMING_SPEC T12: no note-on before its note-off)."""
    open_, ons, offs, doubles = {}, [], [], 0
    for t, kind, ch, n in notes:
        key = (ch, n)
        if kind == "on":
            if key in open_:
                doubles += 1
            open_[key] = len(ons)
            ons.append(t)
        elif key in open_:
            offs.append((t, open_.pop(key)))
    return ons, offs, doubles


def stuck_after_stops(notes, stops, grace_ns=500_000_000):
    """Notes still open 0.5 s after each Stop, summed over the Stops."""
    if not stops:
        return None
    checks = sorted(s + grace_ns for s in stops)
    open_, stuck, ci = set(), 0, 0
    for t, kind, ch, n in notes:
        while ci < len(checks) and checks[ci] <= t:
            stuck += len(open_)
            ci += 1
        if kind == "on":
            open_.add((ch, n))
        else:
            open_.discard((ch, n))
    while ci < len(checks):
        stuck += len(open_)
        ci += 1
    return stuck


def send_values(events, bpm):
    """T2..T8 for a device that sends (FM-1 clock out and notes). Times in ms or ppm as LIMITS says."""
    vals = {}
    t0, pulses = clock_origin(events)
    if t0 is None:
        return vals
    p16 = 60 * NS / (bpm * 4)
    pc = p16 / 6
    ons, offs, _ = pair_notes(note_events(events))
    on_rel = [t - t0 for t in ons]
    on_k = cumulative_index(on_rel, p16)
    fon = fit_line(on_k, on_rel)
    if fon is not None:
        a, b, res_on = fon
        vals["T4"] = abs(a) / 1e6
        vals["T5"] = (b - p16) / p16 * 1e6
        res_all = [res_on]
        if offs:
            off_k = [on_k[i] for _, i in offs]
            off_rel = [t - t0 for t, _ in offs]
            foff = fit_line(off_k, off_rel)
            if foff is not None:
                res_all.append(foff[2])
        vals["T2"] = max(p2p(r) for r in res_all) / 1e6
        vals["T3"] = max(rms(r) for r in res_all) / 1e6
    pk_rel = [p - t0 for p in pulses]
    pk = cumulative_index(pk_rel, pc)
    fc = fit_line(pk, pk_rel)
    if fc is not None:
        _, bc, _ = fc
        gaps = [pk_rel[i] - pk_rel[i - 1] - bc for i in range(1, len(pk)) if pk[i] - pk[i - 1] == 1]
        if gaps:
            vals["T7"] = p2p(gaps) / 1e6
        vals["T8"] = (bc - pc) / pc * 1e6
    if pk:
        counts = collections.Counter(i // 24 for i in pk)
        nq = (pk[-1] + 1) // 24
        if nq > 0:
            vals["T6"] = sum(1 for q in range(nq) if counts[q] != 24)
    return vals


def follow_values(notes, host_events):
    """T9 and T10: the FM-1 notes against the host's clock (both logs on the one clock of the run). Assumes every
    step of the host's 1/16 grid has a note (the test image), so drift is steps of the host minus notes."""
    vals = {}
    h0, hpulses = clock_origin(host_events)
    if h0 is None:
        return vals
    rel = [p - h0 for p in hpulses]
    if len(rel) < 2:
        return vals
    period = statistics.median(rel[i] - rel[i - 1] for i in range(1, len(rel)))
    hk = cumulative_index(rel, period)
    steps = [(k // 6, t) for k, t in zip(hk, rel) if k % 6 == 0]
    st_t = [t for _, t in steps]
    st_s = [s for s, _ in steps]
    on_abs = [t for t, kind, _, _ in notes if kind == "on"]
    if len(on_abs) < 2 or not st_t:
        return vals
    resid, nearest_s = [], []
    for tn in on_abs:
        i = bisect.bisect_left(st_t, tn - h0)
        cands = [j for j in (i - 1, i) if 0 <= j < len(st_t)]
        j = min(cands, key=lambda c: abs(st_t[c] - (tn - h0)))
        resid.append((tn - h0) - st_t[j])
        nearest_s.append(st_s[j])
    vals["T10"] = p2p(resid) / 1e6
    vals["T9"] = (nearest_s[-1] - nearest_s[0]) - (len(on_abs) - 1)
    return vals


def analyse(events, bpm, host_events=None):
    """Rows of TIMING_SPEC section 2 for one run. host_events given means follow mode."""
    events = sorted(events, key=lambda e: e[0])
    notes = note_events(events)
    stops = [t for t, st, _, _ in events if st == STOP]
    vals, na = {}, {"T1": DEVICE_ONLY}
    if host_events is None:
        vals.update(send_values(events, bpm))
        na["T9"] = na["T10"] = "follow mode only (--follow)"
    else:
        for rid in ("T2", "T3", "T4", "T5", "T6", "T7", "T8"):
            na[rid] = "send mode only"
        vals.update(follow_values(notes, sorted(host_events, key=lambda e: e[0])))
    _, _, doubles = pair_notes(notes)
    vals["T12a"] = doubles
    vals["T12b"] = stuck_after_stops(notes, stops)
    return [make_row(rid, vals.get(rid), na.get(rid)) for rid in ORDER]


def format_row(r):
    if r.verdict == "N/A":
        return f"{r.id:<5}{r.label:<42} {'-':>10}  N/A ({r.note})"
    lim = LIMITS[r.id]
    if r.value is None:
        val = "missing"
    elif r.unit in ("ms", "ppm"):
        val = f"{r.value:+.3f}" if r.unit == "ppm" else f"{r.value:.3f}"
    else:
        val = f"{r.value:g}"
    tail = f"  [{r.note}]" if r.note else ""
    return (f"{r.id:<5}{r.label:<42} {val:>10} {r.unit:<8} target {lim[2]:g} hard {lim[3]:g}  "
            f"{r.verdict}{tail}")


def print_rows(rows, title):
    print(title)
    for r in rows:
        print("  " + format_row(r))
    bad = [r.id for r in rows if r.verdict in ("FAIL", "MARGIN")]
    print("TIMING VERDICT " + ("PASS" if not bad else "FAIL: " + ", ".join(bad)))
    return not bad


def print_detail(events, bpm, host_events=None):
    """The worst five residuals of the note-on and note-off fits, for the log (not a verdict)."""
    t0, _ = clock_origin(events)
    if t0 is None:
        return
    p16 = 60 * NS / (bpm * 4)
    ons, offs, _ = pair_notes(note_events(sorted(events, key=lambda e: e[0])))
    on_k = cumulative_index([t - t0 for t in ons], p16)
    fon = fit_line(on_k, [t - t0 for t in ons])
    if fon:
        print("  worst five note-on residuals, ms: " + " ".join(f"{x / 1e6:+.3f}" for x in worst(fon[2])))


# ---------------------------------------------------------------- MIDI I/O

def open_input(rtmidi, hint, events, origin):
    """Open the first input port matching hint with a callback that appends (t_ns, status, d1, d2)."""
    probe = rtmidi.MidiIn()
    names = probe.get_ports()
    probe.delete()
    name = find_port(names, hint)
    if name is None:
        return None
    mi = rtmidi.MidiIn()
    mi.ignore_types(sysex=False, timing=False, active_sense=False)  # the default filters out clock: keep it
    acc = [0.0]

    def cb(event, _data=None):
        msg, delta = event
        acc[0] += delta
        st = msg[0]
        d1 = msg[1] if len(msg) > 1 else 0
        d2 = msg[2] if len(msg) > 2 else 0
        events.append((origin + round(acc[0] * NS), st, d1, d2))

    mi.set_callback(cb)
    mi.open_port(names.index(name))
    return mi, name


def capture(minutes, hint, host_hint=None):
    """Record the FM-1 port (and the host port in follow mode) for the given minutes. None if a port is missing."""
    import rtmidi
    fm_ev, host_ev = [], []
    origin = time.perf_counter_ns()
    fm = open_input(rtmidi, hint, fm_ev, origin)
    if fm is None:
        print(f"no FM-1 MIDI port found (hint {hint!r}); plug in the FM-1 over USB-C or pass --port.")
        return None
    host = None
    if host_hint:
        host = open_input(rtmidi, host_hint, host_ev, origin)
        if host is None:
            fm[0].cancel_callback()
            fm[0].close_port()
            print(f"no host MIDI port found (hint {host_hint!r}); --host-port must name the DAW's output.")
            return None
    print(f"recording {fm[1]!r}" + (f" and host {host[1]!r}" if host else "") +
          f" for {minutes:g} min (Ctrl-C to stop early)", flush=True)
    end = time.monotonic() + minutes * 60
    try:
        while time.monotonic() < end:
            time.sleep(0.25)
    except KeyboardInterrupt:
        print("stopped early: analysing what was captured")
    for mi in [fm] + ([host] if host else []):
        mi[0].cancel_callback()
        mi[0].close_port()
    return fm_ev, host_ev


def loopback(seconds, bpm):
    """Send a clock train on the IAC bus and receive it through the same callback path: the host noise floor."""
    import rtmidi
    probe = rtmidi.MidiOut()
    names = probe.get_ports()
    probe.delete()
    name = find_port(names, "iac")
    if name is None:
        print("no IAC bus found (Audio MIDI Setup > MIDI Studio > IAC Driver, online).")
        return 2
    out = rtmidi.MidiOut()
    out.open_port(names.index(name))
    got = []
    origin = time.perf_counter_ns()
    inp = open_input(rtmidi, "iac", got, origin)
    if inp is None:
        print("IAC bus found for output but not for input.")
        return 2
    pc = 60 * NS / (bpm * 24)
    n = int(seconds * bpm * 24 / 60)
    start = time.perf_counter_ns() + 200_000_000
    for i in range(n):
        target = start + round(i * pc)
        while time.perf_counter_ns() < target - 300_000:
            time.sleep(0.0002)
        while time.perf_counter_ns() < target:
            pass
        out.send_message([CLOCK])
    time.sleep(0.5)
    inp[0].cancel_callback()
    inp[0].close_port()
    out.close_port()
    pulses = [t for t, st, _, _ in got if st == CLOCK]
    rel = [t - pulses[0] for t in pulses] if pulses else []
    fit = fit_line(cumulative_index(rel, pc), rel) if len(rel) > 1 else None
    if fit is None:
        print(f"LOOPBACK no pulses received (sent {n})")
        return 2
    res = fit[2]
    print(f"LOOPBACK host floor at {bpm:g} BPM over {len(pulses)} of {n} pulses: "
          f"p-p {p2p(res) / 1e6:.3f} ms, RMS {rms(res) / 1e6:.3f} ms (measured host noise; "
          f"verdicts above are not reduced by it)")
    return 0 if len(pulses) == n else 1


# ---------------------------------------------------------------- main

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", help="FM-1 input port name (part of it); default: JIANT FM / FM-1 / Felucca")
    ap.add_argument("--host-port", help="follow mode: the host's output that carries its clock and notes")
    ap.add_argument("--follow", action="store_true", help="the FM-1 follows the host: adds T9 and T10")
    ap.add_argument("--minutes", type=float, default=5.0, help="length of each run, default 5 (TIMING_SPEC 6.2)")
    ap.add_argument("--bpm", type=float, default=120.0, help="the tempo the FM-1 is set to, default 120")
    ap.add_argument("--sweep", nargs="?", const=SWEEP_DEFAULT,
                    help="run each tempo in turn (default 60,90,120,180,240), prompting before each run")
    ap.add_argument("--loopback", action="store_true", help="host noise floor on the IAC bus, then exit")
    ap.add_argument("--seconds", type=float, default=30.0, help="loopback length, default 30")
    ap.add_argument("--out", default="/tmp/fm1_timing", help="log path prefix: <out>_<bpm>.log, default /tmp/fm1_timing")
    ap.add_argument("--from-log", help="analyse a saved FM-1 log instead of capturing")
    ap.add_argument("--host-log", help="with --from-log and --follow: the host's log")
    ap.add_argument("--list", action="store_true", help="list MIDI input ports and exit")
    a = ap.parse_args(argv)

    if a.list:
        import rtmidi
        p = rtmidi.MidiIn()
        print("inputs:", p.get_ports())
        p.delete()
        return 0
    if a.loopback:
        return loopback(a.seconds, a.bpm)
    if a.from_log:
        events = load_log(a.from_log)
        host = load_log(a.host_log) if (a.follow and a.host_log) else None
        if a.follow and host is None:
            print("--follow with --from-log needs --host-log")
            return 2
        ok = print_rows(analyse(events, a.bpm, host), f"RUN from {a.from_log} bpm={a.bpm:g}"
                        + (f" follow host={a.host_log}" if host is not None else " send"))
        return 0 if ok else 1
    if a.follow and not a.host_port:
        print("--follow needs --host-port (the host's output, e.g. an IAC bus fed by the DAW)")
        return 2

    bpms = [float(x) for x in a.sweep.split(",")] if a.sweep else [a.bpm]
    all_ok = True
    for bpm in bpms:
        if a.sweep:
            input(f"Set the FM-1 to {bpm:g} BPM, press PLAY, then press Enter to record {a.minutes:g} min: ")
        got = capture(a.minutes, a.port, a.host_port if a.follow else None)
        if got is None:
            return 2
        fm_ev, host_ev = got
        tag = f"{a.out}_{bpm:g}"
        write_log(tag + ".log", fm_ev, f"fm1 port={a.port or 'auto'} bpm={bpm:g} minutes={a.minutes:g}")
        host_arg = None
        if a.follow:
            write_log(tag + ".host.log", host_ev, f"host port={a.host_port} bpm={bpm:g}")
            host_arg = host_ev
        ok = print_rows(analyse(fm_ev, bpm, host_arg),
                        f"RUN bpm={bpm:g} minutes={a.minutes:g} "
                        + ("follow" if a.follow else "send") + f" log={tag}.log")
        print_detail(fm_ev, bpm)
        all_ok = all_ok and ok
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
