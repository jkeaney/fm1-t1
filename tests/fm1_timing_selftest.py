#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Self-test of tools/fm1_timing.py: synthetic logs through the analysis functions, with the verdicts checked.

Needs no FM-1 and no MIDI port. Cases: a perfect grid, a grid with small and with large jitter, a missing clock
pulse, a tempo 400 ppm fast, a note-on over an open note, a note open at Stop, follow mode with a steady offset
and with a missing step, and a log round trip.
  .venv/bin/python tests/fm1_timing_selftest.py      -> TIMING SELFTEST PASS (exit 0) or FAIL (exit 1)
"""
import os
import random
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "tools"))
import fm1_timing as T  # noqa: E402

failures = []


def check(name, cond, detail=""):
    if not cond:
        failures.append(f"{name} {detail}".rstrip())


def grid(bpm=120, quarters=16, latency_ms=2.0, jitter_ms=0.0, seed=1, ppm=0.0, drop_pulse=None,
         drop_note=None, stop=True):
    """Clock from Start, one note on every 1/16 (gate 50 %), the device's tempo `ppm` fast. The notes lag
    the clock pulses by latency_ms: T4 is the note offset from its own clock tick (see the tool's docstring)."""
    rng = random.Random(seed)
    p16 = 60e9 / (bpm * 4)
    pc = p16 / 6
    scale = 1 - ppm * 1e-6
    lat = latency_ms * 1e6
    jit = jitter_ms * 1e6
    ev = [(0, 0xFA, 0, 0)]
    for i in range(quarters * 24):
        if i != drop_pulse:
            ev.append((round(i * pc * scale + rng.uniform(-jit, jit)), 0xF8, 0, 0))
    last = 0
    for k in range(quarters * 4):
        if k == drop_note:
            continue
        t_on = lat + k * p16 * scale + rng.uniform(-jit, jit)
        t_off = t_on + 0.5 * p16 * scale + rng.uniform(-jit, jit)
        ev.append((round(t_on), 0x90, 60, 100))
        ev.append((round(t_off), 0x80, 60, 0))
        last = max(last, t_off)
    if stop:
        ev.append((round(last + 10e6), 0xFC, 0, 0))
    return ev


def rows_by_id(events, bpm=120, host=None):
    return {r.id: r for r in T.analyse(events, bpm, host)}


def expect(name, rows, rid, verdict, value=None, tol=0.01):
    r = rows[rid]
    check(name, r.verdict == verdict, f"got {r.verdict} value={r.value}")
    if value is not None:
        check(name + " value", r.value is not None and abs(r.value - value) <= tol,
              f"got {r.value} want {value}")


# 1. perfect grid: everything passes, latency is the 2 ms put in
rows = rows_by_id(grid())
for rid in ("T2", "T3", "T5", "T6", "T7", "T8", "T12a", "T12b"):
    expect("perfect " + rid, rows, rid, "PASS")
expect("perfect T4 note lag behind its clock tick", rows, "T4", "PASS", value=2.0, tol=0.01)
expect("perfect T1 not in host log", rows, "T1", "N/A")

# 2. small jitter (+-0.3 ms uniform): still inside the targets
rows = rows_by_id(grid(jitter_ms=0.3))
expect("jitter 0.3 T2", rows, "T2", "PASS")
expect("jitter 0.3 T3", rows, "T3", "PASS")
expect("jitter 0.3 T7", rows, "T7", "PASS")

# 3. large jitter (+-2 ms uniform): note jitter fails the hard limit
rows = rows_by_id(grid(jitter_ms=2.0))
expect("jitter 2.0 T2", rows, "T2", "FAIL")
expect("jitter 2.0 T3", rows, "T3", "FAIL")

# 4. missing clock pulse: the quarter with 23 pulses fails T6, exactly one quarter
rows = rows_by_id(grid(drop_pulse=37))
expect("missing pulse T6", rows, "T6", "FAIL", value=1)

# 5. tempo 400 ppm fast: T5 fails (signed value is negative, verdict on the magnitude)
rows = rows_by_id(grid(ppm=400.0))
expect("400 ppm T5", rows, "T5", "FAIL", value=-400.0, tol=1.0)
expect("400 ppm T8", rows, "T8", "FAIL", value=-400.0, tol=1.0)

# 6. a note-on while the same note is still open (T12a)
ev = grid()
ev.append((50_000_000, 0x90, 60, 90))
ev.append((50_000_500, 0x90, 60, 90))
rows = rows_by_id(ev)
expect("double on T12a", rows, "T12a", "FAIL", value=2)

# 7. a note left open at Stop (T12b): drop the last note-off
ev = grid()
last_off = max(e for e in ev if e[1] == 0x80)
ev.remove(last_off)
rows = rows_by_id(ev)
expect("stuck T12b", rows, "T12b", "FAIL", value=1)

# 8. no Stop in the log: T12b cannot be shown, so it is not a pass
rows = rows_by_id(grid(stop=False))
expect("no stop T12b", rows, "T12b", "FAIL")

# 9. follow mode: FM-1 notes 1.5 ms after each host step, no drift -> T9 0, T10 0 (p-p of a constant offset)
p16 = 60e9 / (120 * 4)
host = [(0, 0xFA, 0, 0)] + [(round(i * p16 / 6), 0xF8, 0, 0) for i in range(24 * 16)]
fm = [(round(k * p16 + 1.5e6), 0x90, 60, 100) for k in range(64)]
fm += [(round(k * p16 + 1.5e6 + p16 / 2), 0x80, 60, 0) for k in range(64)]
fm.append((round(63 * p16 + 20e6), 0xFC, 0, 0))
rows = rows_by_id(fm, host=host)
expect("follow T9 steady", rows, "T9", "PASS", value=0)
expect("follow T10 steady", rows, "T10", "PASS", value=0.0, tol=0.01)
expect("follow T2 n/a", rows, "T2", "N/A")

# 10. follow mode with one step missing from the FM-1 notes: drift of one step -> MARGIN (hard limit 1)
fm2 = [e for e in fm if not (e[1] == 0x90 and e[0] == round(20 * p16 + 1.5e6))]
rows = rows_by_id(fm2, host=host)
expect("follow T9 one missing step", rows, "T9", "MARGIN", value=1)

# 11. log round trip
ev = grid(quarters=2)
fd, path = tempfile.mkstemp(suffix=".log")
os.close(fd)
try:
    T.write_log(path, ev, "selftest")
    back = T.load_log(path)
    check("log round trip", back == sorted(ev, key=lambda e: e[0]) or back == ev, f"{len(back)} vs {len(ev)}")
finally:
    os.remove(path)

if failures:
    print("TIMING SELFTEST FAIL:")
    for f in failures:
        print("  " + f)
    sys.exit(1)
print("TIMING SELFTEST PASS")
