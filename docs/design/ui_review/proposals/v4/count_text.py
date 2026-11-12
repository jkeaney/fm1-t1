# SPDX-License-Identifier: GPL-3.0-only
"""count_text.py: text items per v4 scene, by script (the canvas records every text draw)."""
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "mock"))
import mock
import proposal_v4 as P
mock.palette_set(P.PALETTE)
for name, sc in P.STATE["scenes"].items():
    cv = mock.Canvas()
    cv.texts = []
    seen = []
    orig = mock.Canvas.text_flags
    def tf(self, x, y, f, s, fg, bg, flags=0, _o=orig):
        seen.append((f.name, s)); return _o(self, x, y, f, s, fg, bg, flags)
    mock.Canvas.text_flags = tf
    P.draw(cv, sc)
    mock.Canvas.text_flags = orig
    by = {}
    for f, s in seen: by.setdefault(f, []).append(s)
    print("%-22s %2d items  " % (name, len(seen)) + "  ".join("%s:%d" % (k, len(v)) for k, v in sorted(by.items())), "|", " ".join(s for _, s in seen))
