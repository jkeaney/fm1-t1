# SPDX-License-Identifier: GPL-3.0-only
"""make_sheets_v3.py: renders before_v3 (current shipping screen) and proposal_v3 and writes SCENE_before_after.png (current left, proposal right, 2x).
   .venv/bin/python docs/design/ui_review/proposals/v3-glance/make_sheets_v3.py"""
import os
import sys
import tempfile
from PIL import Image, ImageDraw
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
sys.path.insert(0, HERE)
import mock                                   # noqa: E402
sys.modules["mock"] = mock
import before_v3                              # noqa: E402
import proposal_v3 as P                       # noqa: E402

tmp = tempfile.mkdtemp()
mock.render_files(before_v3, tmp, "before_v3")
for sc in P.STATE["scenes"]:
    a = Image.open(os.path.join(tmp, "before_v3_%s.png" % sc)).convert("RGB").resize((480, 480), Image.NEAREST)
    b = Image.open(os.path.join(HERE, "proposal_v3_%s.png" % sc)).convert("RGB").resize((480, 480), Image.NEAREST)
    sheet = Image.new("RGB", (480 * 2 + 12, 480 + 18), (60, 60, 60))
    sheet.paste(a, (0, 18))
    sheet.paste(b, (492, 18))
    d = ImageDraw.Draw(sheet)
    d.text((4, 3), "CURRENT  (%s)" % sc, fill=(255, 255, 255))
    d.text((496, 3), "PROPOSAL v3-glance  (%s)" % sc, fill=(255, 255, 255))
    sheet.save(os.path.join(HERE, "%s_before_after.png" % sc))
    print(sc, "ok")
