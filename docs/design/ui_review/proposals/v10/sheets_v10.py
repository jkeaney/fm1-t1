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
        S.paste(Image.open("out/v10_%s.png" % f).resize((240 * k, 240 * k), Image.NEAREST), (x, y + 58))
    S.save("out/" + out)


sheet("S_v10_main.png", [
    ("playing", "4 channels playing", "music row 1.5x taller; 3 px strips; knob level edges"),
    ("dice", "Pressing SCL", ""),
    ("turn_select", "Turning SELECT", "named only while touched"),
    ("turn_algo", "Turning ALGO", "lands at the next step"),
    ("turn_presets", "Turning PRESETS", "P02 pending, loads at the next bar"),
    ("oct_row2", "OCT+ : row 2", "row marker + top bar"),
    ("row3_rig", "OCT+ : row 3 (RIG)", "only when the channel has a RIG wire"),
    ("hold_glo", "Holding GLO", "HARMONY before it fires"),
    ("hold_save", "Holding SAVE", "UNDO LOAD before it fires"),
    ("midi_off_after", "CH2 MIDI OFF", ""),
    ("rec_playing", "REC while playing", "dot with a play mark; REC = DONE"),
    ("pending_p02", "P02 pending, playhead on step 2", "pill clear of the playhead"),
])
sheet("S_v10_algos.png", [
    ("sheet_row1_a", "ROW 1: ACID EUCLID SNAKE STAGE", "same code; value lists get dots"),
    ("sheet_row2_a", "ROW 2: ACID EUCLID SNAKE STAGE", ""),
    ("sheet_row1_b", "ROW 1: ARP PATTERNS DEMO", "DEMO = never seen by the screen code"),
    ("sheet_row2_b", "ROW 2: ARP PATTERNS DEMO", "PICK reads ACD 1 (category + n)"),
    ("demo_algo", "DEMO selected", "its missing accent layer greys the ENV die"),
    ("stage_heard", "STAGE TOTAL 48(32)", "the algorithm's own value text"),
], cols=3)
print("ok")
