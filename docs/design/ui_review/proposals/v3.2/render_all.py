# SPDX-License-Identifier: GPL-3.0-only
"""render_all.py: renders every scene of proposal_v3_2 (NAME.png, NAME_4x.png, NAME_grid.png via mock.render_files, prefix proposal_v3_2_) into this directory,
then the review sheets:  SCENE_v3_1_vs_v3_2.png (v3.1 left, v3.2 right, both 2x; the v3.1 side is v3.1's own draw() on the same state),
states_sheet_1x.png / states_sheet_2x.png (the 15 undefined states), toast_dice_strip.png, cue_series_sheet.png, lane_accent_test.png (colour-blind and mono test).
   .venv/bin/python docs/design/ui_review/proposals/v3.2/render_all.py
"""
import os
import sys
from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
sys.path.insert(0, HERE)
import mock                                   # noqa: E402
sys.modules["mock"] = mock
import proposal_v3_2 as P                     # noqa: E402

P1 = P.P1
PRE = "proposal_v3_2_"


def img(name):
    return Image.open(os.path.join(HERE, PRE + name + ".png")).convert("RGB")


def v31_image(state, palette="GREY"):
    """v3.1's own draw() on a (converted) state: toast kinds v3.1 does not know are dropped, the stress ROW / fixtures are shared."""
    st = dict(state)
    st["toast"] = None
    st.pop("map", None)
    st["_name"] = ""
    st["row"] = min(st.get("row", 0), 1)
    st["palette"] = palette
    st.setdefault("hot", [0, 0, 0, 0])
    screen, _ = mock.render_state(P1, st, palette)
    return screen.to_image().convert("RGB")


def sheet(left, right, ltxt, rtxt, path, k=2, extra=None):
    ims = [left, right] + ([extra] if extra else [])
    ims = [i.resize((i.width * k, i.height * k), Image.NEAREST) for i in ims]
    labels = [ltxt, rtxt] + ([""] if extra else [])
    w = sum(i.width for i in ims) + 12 * (len(ims) - 1)
    sh = Image.new("RGB", (w, max(i.height for i in ims) + 18), (60, 60, 60))
    x = 0
    d = ImageDraw.Draw(sh)
    for i, t in zip(ims, labels):
        sh.paste(i, (x, 18))
        d.text((x + 4, 3), t, fill=(255, 255, 255))
        x += i.width + 12
    sh.save(path)


# v3.2 scene -> v3.1 scene (a name present in P1.STATE) or "same" (v3.1 draws the same state with its own draw)
PAIRS = {
    "glance_idle": "glance_idle", "glance_playing": "glance_playing", "row2": "row2", "mutes": "mutes", "stress": "stress", "stress_row2": "stress_row2",
    "row3": "same", "toast_dice_notes": "toast_dice_notes", "toast_undo": "toast_undo", "toast_preset_pending": "toast_preset_pending", "toast_algo": "toast_algo",
    "toast_redo": "same", "toast_preset_loading": "same", "toast_midi_channel": "same", "toast_dice_artic_snake": "same",
    "stress_pal_paper": "stress_pal_paper", "stress_pal_mono": "stress_pal_mono", "stress_pal_ice": "stress_pal_ice",
    "lane_full_playhead": "same", "lane_full_playhead_first": "same", "lane_vel_low": "same", "lane_ratchet_sweep": "same", "lane_muted": "same", "lane_offpage": "same",
    "map_menu_countdown": "map_menu_countdown", "map_touch": "map_touch",
    "cue_dice_notes": "same", "cue_mute_held": "same", "cue_solo": "same", "cue_fill": "same", "cue_roll": "same", "cue_oct_latch": "row2",
}
STATES = ("ext_sync", "wait", "ext_lost", "rec_stopped", "rec_playing", "follow_off", "add_page", "past_length", "first_run", "reset", "sol", "fil", "rol")


def main():
    mock.render_files(P, HERE, "proposal_v3_2")
    scenes = P.STATE["scenes"]
    for sc, v31 in PAIRS.items():
        st = scenes[sc]
        pal = st.get("palette", "GREY")
        if v31 == "same":
            left = v31_image(st, pal)
            ltxt = "v3.1  (same state drawn by v3.1: it has no such state / toast)"
        else:
            s31 = P1.STATE["scenes"][v31]
            screen, _ = mock.render_state(P1, s31, s31.get("palette", pal))
            left = screen.to_image().convert("RGB")
            ltxt = "v3.1  (%s, %s)" % (v31, s31.get("palette", "GREY"))
        sheet(left, img(sc), ltxt, "v3.2  (%s, %s)" % (sc, pal), os.path.join(HERE, "%s_v3_1_vs_v3_2.png" % sc))
        print(sc, "sheet ok")
    # the map: v3.1 (one AF_S page) | v3.2 A | v3.2 B
    s31 = P1.STATE["scenes"]["map"]
    screen, _ = mock.render_state(P1, s31, "GREY")
    sheet(screen.to_image().convert("RGB"), img("map_A"), "v3.1  (map: one page, AF_S)", "v3.2  (map_A: AF_M roles)", os.path.join(HERE, "map_v3_1_vs_v3_2.png"), extra=img("map_B"))
    # states contact sheets
    ims = [(n, img("state_" + n)) for n in STATES] + [("sel_muted_off", img("state_sel_muted_off")), ("midi_channel", img("state_midi_channel"))]
    for k, name in ((1, "states_sheet_1x.png"), (2, "states_sheet_2x.png")):
        cols = 5 if k == 1 else 3
        rows = (len(ims) + cols - 1) // cols
        cw, ch = 240 * k, 240 * k
        sh = Image.new("RGB", (cols * (cw + 6), rows * (ch + 16)), (60, 60, 60))
        d = ImageDraw.Draw(sh)
        for i, (n, im) in enumerate(ims):
            x, y = (i % cols) * (cw + 6), (i // cols) * (ch + 16)
            sh.paste(im.resize((cw, ch), Image.NEAREST) if k > 1 else im, (x, y + 14))
            d.text((x + 2, y + 1), "state: %s" % n, fill=(255, 255, 255))
        sh.save(os.path.join(HERE, name))
    # six dice toasts, bottom 54 px, stacked
    names = ["toast_dice_" + n for n in ("all", "notes", "accnt", "shape", "artic", "hits")] + ["toast_dice_artic_snake", "toast_undo", "toast_redo", "toast_preset_pending", "toast_preset_loading", "toast_midi_channel"]
    strip = Image.new("RGB", (2 * 480 + 6, 6 * (108 + 14)), (60, 60, 60))
    d = ImageDraw.Draw(strip)
    for i, n in enumerate(names):
        im = img(n).crop((0, 186, 240, 240)).resize((480, 108), Image.NEAREST)
        x, y = (i % 2) * 486, (i // 2) * 122
        strip.paste(im, (x, y + 14))
        d.text((x + 2, y + 1), n, fill=(255, 255, 255))
    strip.save(os.path.join(HERE, "toast_dice_strip.png"))
    # cue series: bottom 40 px of every cue scene
    cues = ["cue_idle_stopped", "cue_idle_playing", "cue_dice_all", "cue_dice_notes", "cue_dice_accnt", "cue_dice_shape", "cue_dice_artic", "cue_dice_hits",
            "cue_mute_held", "cue_solo", "cue_fill", "cue_roll", "cue_undo", "cue_oct_latch", "cue_save_hold", "cue_page"]
    sh = Image.new("RGB", (2 * 480 + 6, 8 * (80 + 14)), (60, 60, 60))
    d = ImageDraw.Draw(sh)
    for i, n in enumerate(cues):
        im = img(n).crop((0, 200, 240, 240)).resize((480, 80), Image.NEAREST)
        x, y = (i % 2) * 486, (i // 2) * 94
        sh.paste(im, (x, y + 14))
        d.text((x + 2, y + 1), n, fill=(255, 255, 255))
    sh.save(os.path.join(HERE, "cue_series_sheet.png"))
    accent_test()


# Machado 2009 colour-vision-deficiency matrices (severity 1.0), applied to linear RGB
CVD = {
    "deuteranopia": ((0.367322, 0.860646, -0.227968), (0.280085, 0.672501, 0.047413), (-0.011820, 0.042940, 0.968881)),
    "protanopia": ((0.152286, 1.052583, -0.204868), (0.114503, 0.786281, 0.099216), (-0.003882, -0.048116, 1.051998)),
    "tritanopia": ((1.255528, -0.076749, -0.178779), (-0.078411, 0.930809, 0.147602), (0.004733, 0.691367, 0.303900)),
}


def _lin(v):
    v /= 255.0
    return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4


def _srgb(v):
    v = max(0.0, min(1.0, v))
    return int(round(255 * (v * 12.92 if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055)))


def cvd(im, kind):
    m = CVD[kind]
    out = Image.new("RGB", im.size)
    px = im.load()
    po = out.load()
    cache = {}
    for y in range(im.height):
        for x in range(im.width):
            c = px[x, y]
            if c not in cache:
                lr, lg, lb = _lin(c[0]), _lin(c[1]), _lin(c[2])
                cache[c] = tuple(_srgb(m[i][0] * lr + m[i][1] * lg + m[i][2] * lb) for i in range(3))
            po[x, y] = cache[c]
    return out


def accent_test():
    """The lane of glance_playing (v3.2) and v3.1's lane, normally, under three colour-vision deficiencies, in greyscale and in greyscale + blur (about 4 px = 5 arcmin
    of blur at 8 px/mm and 50 cm: a viewer without the glasses on), at 3x.  What must survive: the accent cap vs the plain bars."""
    a = img("glance_playing").crop((0, 72, 240, 144))
    b = v31_image(P.STATE["scenes"]["glance_playing"]).crop((0, 72, 240, 144))
    rows = [("v3.1 (cyan whole bar)", b), ("v3.2 (cyan cap 13x8)", a)]
    cols = [("normal", lambda i: i), ("deuteranopia", lambda i: cvd(i, "deuteranopia")), ("protanopia", lambda i: cvd(i, "protanopia")), ("tritanopia", lambda i: cvd(i, "tritanopia")),
            ("greyscale", lambda i: i.convert("L").convert("RGB")), ("grey + blur", lambda i: i.convert("L").filter(ImageFilter.GaussianBlur(1.6)).convert("RGB"))]
    k = 2
    W_, H_ = 240 * k, 72 * k
    # layout: for each row image, 6 variants in a 2 x 3 block; the two blocks stacked
    sh = Image.new("RGB", (2 * (W_ + 6), 6 * (H_ + 16)), (60, 60, 60))
    d = ImageDraw.Draw(sh)
    for r, (rn, im) in enumerate(rows):
        for c, (cn, f) in enumerate(cols):
            x, y = (c % 2) * (W_ + 6), (r * 3 + c // 2) * (H_ + 16)
            sh.paste(f(im).resize((W_, H_), Image.NEAREST), (x, y + 14))
            d.text((x + 2, y + 1), "%s: %s" % (rn, cn), fill=(255, 255, 255))
    sh.save(os.path.join(HERE, "lane_accent_test.png"))


if __name__ == "__main__":
    main()
