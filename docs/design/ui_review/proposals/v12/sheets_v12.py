# SPDX-License-Identifier: GPL-3.0-only
"""sheets_v12.py: the full state set of the accepted design (captions are annotation, not screen text)."""
from PIL import Image, ImageDraw, ImageFont
FB = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 22)
FR = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 17)


def sheet(out, items, cols=4, k=2):
    cw = 240 * k + 40
    rows = (len(items) + cols - 1) // cols
    S = Image.new("RGB", (24 + cols * cw, rows * (240 * k + 100) + 20), (236, 234, 228))
    d = ImageDraw.Draw(S)
    for n, (f, c, sub) in enumerate(items):
        x = 24 + (n % cols) * cw
        y = 20 + (n // cols) * (240 * k + 100)
        d.text((x, y), c, font=FB, fill=(20, 20, 20))
        d.text((x, y + 28), sub, font=FR, fill=(90, 90, 90))
        S.paste(Image.open("out/v12_%s.png" % f).resize((240 * k, 240 * k), Image.NEAREST), (x, y + 58))
    S.save("out/" + out)


sheet("S_v12_states_1.png", [
    ("first_run", "First run", "CH1 is new: the MIDI cue + CHN key lit"),
    ("chan_page", "CHN key: channel setup", "LENGTH MIDI SOURCE FOLLOW; D#4 = BACK"),
    ("chan_midi_clash", "CH2 takes MIDI 2", "CH1 turns OFF: toast names it"),
    ("hold_mute_remove", "Long press M4", "release early = PREVIEW; hold on = REMOVE"),
    ("removed", "CH4 removed", "EDIT undoes it"),
    ("undo", "EDIT tap", "UNDO; EDIT again = REDO"),
    ("ext_sync", "External clock", "EXT over the tempo"),
    ("ext_wait", "Waiting for the host", "EXT / WAIT"),
])
sheet("S_v12_states_2.png", [
    ("ext_lost", "Clock lost", "EXT / -- in red"),
    ("follow_off", "FOLLOW off", "viewing page 1, PLAY 2: no playhead here"),
    ("add_page", "PG+ on the last page", "PAGE 2? PG+=ADD"),
    ("past_length", "LENGTH 12", "steps 13-16 hatched on lane and keys"),
    ("held_solo", "Holding SOLO", ""),
    ("held_fill", "Holding FILL", "amount from key force"),
    ("held_roll", "Holding ROLL", ""),
    ("playing", "Main screen (reference)", "pointer playhead, ROW label"),
])
print("ok")
