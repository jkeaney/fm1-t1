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
        S.paste(Image.open("out/v9_%s.png" % f).resize((240 * k, 240 * k), Image.NEAREST), (x, y + 58))
    S.save("out/" + out)


sheet("S_v9_main.png", [
    ("playing", "4 channels playing", "CH1 selected = full width; CH2-4 = hit strips (CH3 muted)"),
    ("dice", "Pressing SCL", "toast names the channel; diced steps hatched"),
    ("turning_k2", "Turning K2 (SLIDE 4 > 6)", "the tile answers; white ticks = old value"),
    ("hold_edit", "Holding EDIT", "the hold is named before it fires; bar = time to fire"),
    ("midi_off", "CH2 MIDI OFF", "red chip, red cap on the chip, hollow red bars, toast"),
    ("channel_added", "SELECT past CH1 = new CH2", "toast says what to do next"),
    ("midi_off_after", "CH2 MIDI OFF, after the toast", "MIDI OFF stays loud in the header"),
    ("rec_armed", "REC armed", "REC knobs (CURSOR ACC SLIDE OCT), plain keys"),
    ("sel_muted", "Selected channel muted", "CH3: hollow grey bars, struck chip + white bar"),
    ("pending_p02", "Preset P02 pending", "inverted P02? on the status line"),
])
sheet("S_v9_algos.png", [
    ("sheet_row1_a", "ROW 1: ACID EUCLID SNAKE STAGE", "same code; value lists get dots"),
    ("sheet_row2_a", "ROW 2: ACID EUCLID SNAKE STAGE", ""),
    ("sheet_row1_b", "ROW 1: ARP PATTERNS DEMO", "DEMO = never seen by the screen code"),
    ("sheet_row2_b", "ROW 2: ARP PATTERNS DEMO", "PICK reads ACD 1 (category + n)"),
    ("demo_algo", "DEMO selected", "its missing accent layer greys the ENV die"),
    ("stage_heard", "STAGE TOTAL 48(32)", "the algorithm's own value text"),
], cols=3)
print("ok")
