# SPDX-License-Identifier: GPL-3.0-only
"""make_sheets.py: renders before_v1 (current) and proposal_v1 and writes SCENE_before_after.png (current left, proposal right, both 2x).
   .venv/bin/python docs/design/ui_review/proposals/v1/make_sheets.py"""
import os, sys, tempfile
from PIL import Image, ImageDraw
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
import mock                                   # noqa: E402
sys.modules["mock"] = mock
tmp = tempfile.mkdtemp()
for f in ("before_v1.py",):
    mod = mock.load_proposal(os.path.join(HERE, f))
    mock.render_files(mod, tmp, "before_v1")
for sc in ("idle", "playing", "row2", "snake_mute", "long_names", "undo_toast"):
    a = Image.open(os.path.join(tmp, "before_v1_%s.png" % sc)).convert("RGB").resize((480, 480), Image.NEAREST)
    b = Image.open(os.path.join(HERE, "proposal_v1_%s.png" % sc)).convert("RGB").resize((480, 480), Image.NEAREST)
    sheet = Image.new("RGB", (480 * 2 + 12, 480 + 18), (60, 60, 60))
    sheet.paste(a, (0, 18))
    sheet.paste(b, (492, 18))
    d = ImageDraw.Draw(sheet)
    d.text((4, 3), "CURRENT  (%s)" % sc, fill=(255, 255, 255))
    d.text((496, 3), "PROPOSAL v1  (%s)" % sc, fill=(255, 255, 255))
    sheet.save(os.path.join(HERE, "%s_before_after.png" % sc))
    print(sc, "ok")
