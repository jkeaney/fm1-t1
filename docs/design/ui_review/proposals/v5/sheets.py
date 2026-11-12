# SPDX-License-Identifier: GPL-3.0-only
"""sheets.py: contact sheets per section from out/v5_*.png (captions are annotation, not screen text)."""
import os
from PIL import Image, ImageDraw, ImageFont
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")
FB = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 22)
FR = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 17)
FT = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 30)
BG, FG, SUB = (236, 234, 228), (20, 20, 20), (90, 90, 90)


def im(name, h=240, k=2):
    i = Image.open(os.path.join(OUT, "v5_%s.png" % name)).crop((0, 0, 240, h))
    return i.resize((240 * k, h * k), Image.NEAREST)


def sheet(fname, title, rows, cols=2, k=2):
    """rows: [(name, crop_h, caption, sub)]"""
    cw = 240 * k + 40
    cells = []
    for (n, h, cap, sub) in rows:
        cells.append((im(n, h, k), cap, sub))
    hs = []
    for r in range(0, len(cells), cols):
        hs.append(max(c[0].height for c in cells[r:r + cols]) + 90)
    W = cols * cw + 40
    H = 80 + sum(hs) + 20
    S = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(S)
    d.text((24, 22), title, font=FT, fill=FG)
    y = 80
    for r in range(0, len(cells), cols):
        for j, (img, cap, sub) in enumerate(cells[r:r + cols]):
            x = 24 + j * cw
            d.text((x, y), cap, font=FB, fill=FG)
            d.text((x, y + 28), sub, font=FR, fill=SUB)
            S.paste(img, (x, y + 58))
        y += hs[r // cols]
    S.save(os.path.join(OUT, fname))
    return fname


sheet("S3_music.png", "3a  MUSIC: all channels in one place (2x)", [
    ("music_lanes", 72, "M-A  Lanes", "one thin lane per channel; height = pitch"),
    ("music_shades", 56, "M-B  Shade stack (your idea)", "1 row; cell per channel; shade = pitch (12)"),
    ("music_shades2", 56, "M-B with 2 channels", "rows grow when fewer channels play"),
    ("music_roll", 68, "M-C  Condensed piano roll", "12 pitch rows; colours show the interplay"),
])
sheet("S3_knobs.png", "3b  KNOBS K1-K4 (2x)", [
    ("knobs_num", 60, "K-A  Name + big value + meter", "v4"),
    ("knobs_arc", 60, "K-B  Dial", "arc = where in the range; looks like a knob"),
    ("knobs_tworow", 72, "K-C  Both rows", "live row big, the OCT row small underneath"),
    ("knobs_fill", 60, "K-D  Tile is the meter", "fill height = value"),
    ("knobs_turning", 64, "While turning K2", "old > new, the full range; ~1 s"),
], cols=2)
sheet("S3_encoders.png", "3c  LEFT ENCODERS: SELECT / PRESETS / ALGO (2x)", [
    ("enc_block", 60, "E-A  Block shaped like the panel", "MASTER  SELECT / PRESETS  ALGO"),
    ("enc_row", 28, "E-B  One header row", "play+BPM | channels (+ = add) | algo | preset"),
    ("enc_row_pending", 28, "E-B  preset pending", "P02? boxed until it loads"),
], cols=2)
sheet("S3_buttons.png", "3d  THE 12 BUTTONS, shaped like the panel (2x)", [
    ("buttons_icon", 50, "B-A  Icons only", "tinted = dice buttons"),
    ("buttons_printed", 50, "B-B  Icon + printed name", "FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC"),
    ("buttons_role", 62, "B-C  Icon + what it does", "ALL NOTES ACCNT SHAPE UNDO SET ..."),
    ("buttons_lit", 50, "Pressed: SCL lights", "the press is shown where the button is"),
], cols=2)
sheet("S3_keys_feedback.png", "3e  KEYBED + 3f FEEDBACK (2x)", [
    ("keys", 32, "Keybed ruler", "16 steps = 16 white keys; black-key roles as glyphs"),
    ("toast", 28, "F-B  Quick toast (top)", "what happened + what to do next"),
], cols=2)
sheet("S4_layouts.png", "4  FITTED LAYOUTS (2x)", [
    ("A_3ch", 240, "A  3 channels playing", "header / both knob rows / shade stack / buttons / keys"),
    ("A_diced", 240, "A  pressing SCL", "toast + SCL lit + diced steps hatched"),
    ("A_turning", 240, "A  turning K2", ""),
    ("A_1ch", 240, "A  1 channel", ""),
    ("B_3ch", 240, "B  panel mirror + piano roll", "encoders left, knobs right, as on the unit"),
    ("B_pending", 240, "B  preset P02 pending", ""),
], cols=2)
print("ok")
