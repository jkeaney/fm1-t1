#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Fidelity gate: renders current_perf.py and diffs each scene against the shipping render tests/ui_sq_test.c wrote to
build/ui_sq/<scene>.png (run ../../../../tests ui_sq_test first if they are stale). Prints the differing-pixel count per image;
exit 0 only when all are 0.   usage: check_fidelity.py [REF_DIR|--regen] [OUT_DIR]
--regen compiles and runs tests/ui_sq_test.c into a temp dir first (as its header says), so the reference is the CURRENT firmware
sources and build/gen fonts; build/ui_sq/*.png can be older than build/gen/ui_fonts.h (then they differ by glyph anti-aliasing only)."""
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mock                                                                   # noqa: E402

ref = sys.argv[1] if len(sys.argv) > 1 else os.path.join(mock.ROOT, "build", "ui_sq")
out = sys.argv[2] if len(sys.argv) > 2 else tempfile.mkdtemp(prefix="mock_fid_")
if ref == "--regen":
    import subprocess
    ref = tempfile.mkdtemp(prefix="mock_ref_")
    exe = os.path.join(ref, "ui_sq_test")
    subprocess.check_call(["cc", "-std=gnu11", "-O1", "-w", "-Ibuild/gen", "-Ifirmware/src", "-o", exe, "tests/ui_sq_test.c", "-lm"], cwd=mock.ROOT)
    subprocess.check_call([exe, ref], cwd=mock.ROOT, stdout=subprocess.DEVNULL)
mod = mock.load_proposal(os.path.join(HERE, "current_perf.py"))
paths = mock.render_files(mod, out, "current_perf")
bad = 0
for scene in mod.STATE["scenes"]:
    n = mock.diff_png(os.path.join(out, "current_perf_%s.png" % scene), os.path.join(ref, scene + ".png"),
                      os.path.join(out, "diff_%s.png" % scene))
    bad += 1 if n is None else n
print("FIDELITY PASS" if not bad else "FIDELITY FAIL: %d differing pixels" % bad)
sys.exit(1 if bad else 0)
