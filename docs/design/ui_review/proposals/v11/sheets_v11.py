# SPDX-License-Identifier: GPL-3.0-only
"""sheets_v9.py: contact sheets for the owner (captions are annotation, not screen text)."""
from PIL import Image, ImageDraw, ImageFont
FB = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 22)
FR = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 17)


def sheet(out, items, cols=3, k=2):
    cw = 240 * k + 40
    rows = (len(items) + cols - 1) // cols
    S = Image.new("RGB", (24 + cols * cw, rows * (240 * k + 100) + 20), (236, 234, 228))
    d = ImageDraw.Draw(S)
    for n, (f, c, sub) in enumerate(items):
        x = 24 + (n % cols) * cw
        y = 20 + (n // cols) * (240 * k + 100)
        d.text((x, y), c, font=FB, fill=(20, 20, 20))
        d.text((x, y + 28), sub, font=FR, fill=(90, 90, 90))
        S.paste(Image.open("out/v11_%s.png" % f).resize((240 * k, 240 * k), Image.NEAREST), (x, y + 58))
    S.save("out/" + out)


sheet("S_v11_main.png", [
    ("playing", "4 channels playing", "fixed strip row per channel; numbered knob rows; PAGE 1/2"),
    ("dice", "Pressing SCL", "messages on the right; play/BPM/chips stay"),
    ("turn_algo", "Turning ALGO", "knobs stay ACID until SNAKE lands"),
    ("hold_save", "Holding SAVE", ""),
    ("hold_home", "Holding HOME", "falls back to the small font"),
    ("row3_rig", "OCT+ : row 3", "the device profile's wire names (TB-3 BITE)"),
    ("added_after", "New CH2", "the standing MIDI cue + CHN key lit"),
    ("midi_off_after", "CH2 MIDI OFF", "same cue"),
    ("rec_playing", "REC while playing", "write cursor on step 4"),
])
sheet("S_v11_algos.png", [
    ("sheet_row1_a", "ROW 1: ACID EUCLID SNAKE STAGE", "same code; value lists get dots"),
    ("sheet_row2_a", "ROW 2: ACID EUCLID SNAKE STAGE", ""),
    ("sheet_row1_b", "ROW 1: ARP PATTERNS DEMO", "DEMO = never seen by the screen code"),
    ("sheet_row2_b", "ROW 2: ARP PATTERNS DEMO", "PICK reads ACD 1 (category + n)"),
    ("demo_algo", "DEMO selected", "its missing accent layer greys the ENV die"),
    ("stage_heard", "STAGE TOTAL 48(32)", "the algorithm's own value text"),
], cols=3)
print("ok")
