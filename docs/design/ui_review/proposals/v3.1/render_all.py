# SPDX-License-Identifier: GPL-3.0-only
"""render_all.py: renders every scene of proposal_v3_1 (NAME.png, NAME_4x.png, NAME_grid.png via mock.render_files) into this directory and writes the
before/after sheets SCENE_v3_vs_v3_1.png (left v3-glance, right v3.1, both 2x). The v3 side is rendered from ../v3-glance/proposal_v3.py on the same palette.
   .venv/bin/python docs/design/ui_review/proposals/v3.1/render_all.py"""
import importlib.util
import os
import sys
import tempfile
from PIL import Image, ImageDraw
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
sys.path.insert(0, HERE)
import mock                                   # noqa: E402
sys.modules["mock"] = mock
import proposal_v3_1 as P                     # noqa: E402

V3DIR = os.path.join(HERE, "..", "v3-glance")
sys.path.insert(0, V3DIR)
spec = importlib.util.spec_from_file_location("proposal_v3", os.path.join(V3DIR, "proposal_v3.py"))
V3 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(V3)

# v3.1 scene -> (v3 scene, palette for the v3 render)
PAIRS = {"glance_idle": "glance_idle", "glance_playing": "glance_playing", "row2": "row2", "mutes": "mutes", "stress": "stress", "stress_row2": "stress_row2",
         "toast_algo": "toast_algo", "toast_preset_pending": "toast_preset_pending", "toast_dice_notes": "toast_dice_notes", "toast_undo": "toast_undo",
         "map": "map", "map_menu_countdown": "map", "map_touch": "map", "snake_path": "mutes", "glance_playing_mono": "glance_playing_mono",
         "toast_algo_header": "toast_algo", "toast_algo_lane": "toast_algo", "toast_algo_full": "toast_algo", "toast_preset_lane": "toast_preset_pending",
         "toast_preset_header": "toast_preset_pending", "toast_dice_header": "toast_dice_notes", "toast_dice_lane": "toast_dice_notes", "stress_selL": "stress"}
for pn in ("paper", "amber", "ice", "green", "violet", "rose", "night", "mono"):
    PAIRS["stress_pal_" + pn] = "stress"


def main():
    P.mock.render_files(P, HERE, "proposal_v3_1")
    tmp = tempfile.mkdtemp()
    cache = {}
    for sc, v3sc in PAIRS.items():
        pal = P.STATE["scenes"][sc].get("palette", "GREY")
        key = (v3sc, pal)
        if key not in cache:
            st = dict(V3.STATE["scenes"][v3sc])
            screen, _ = mock.render_state(V3, st, pal)
            cache[key] = screen.to_image().convert("RGB")
        a = cache[key].resize((480, 480), Image.NEAREST)
        b = Image.open(os.path.join(HERE, "proposal_v3_1_%s.png" % sc)).convert("RGB").resize((480, 480), Image.NEAREST)
        sheet = Image.new("RGB", (480 * 2 + 12, 480 + 18), (60, 60, 60))
        sheet.paste(a, (0, 18))
        sheet.paste(b, (492, 18))
        d = ImageDraw.Draw(sheet)
        d.text((4, 3), "v3-glance  (%s, %s)" % (v3sc, pal), fill=(255, 255, 255))
        d.text((496, 3), "v3.1  (%s, %s)" % (sc, pal), fill=(255, 255, 255))
        sheet.save(os.path.join(HERE, "%s_v3_vs_v3_1.png" % sc))
        print(sc, "sheet ok")


if __name__ == "__main__":
    main()
