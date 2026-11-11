#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
"""PPM -> PNG for the UI renders of tests/ui_render.c (Pillow). Under OUTDIR (default build/ui_new):
  <PALETTE>/<screen>.png   240 x 240, true size
  sheet_<PALETTE>.png      every screen of one palette, 1x, with its name
  (MENU > STYLE: LINE_<PALETTE>/ and sheet_LINE_<PALETTE>.png for GREY MONO NIGHT PAPER)
The screens (P3-006): the T-1 screen, the menu, ABOUT, INFO, CALIBRATION, UPDATE MODE, the splash, the dialog, the count-in.
With SLOTDIR (ui_render.py OUTDIR SLOTDIR): SLOTDIR/*.ppm -> *.png if there are any.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

out = Path(sys.argv[1] if len(sys.argv) > 1 else "build/ui_new")
shots = {}
for f in sorted((out / "ppm").glob("*.ppm")):
    pal, name = f.stem.split("_", 1)
    styled = pal in ("LINE",)   # STYLE renders: <STYLE>_<PALETTE>_<screen>
    if styled:
        p2, name = name.split("_", 1)
        pal = f"{pal}_{p2}"
    (out / pal).mkdir(parents=True, exist_ok=True)
    img = Image.open(f).convert("RGB")
    img.save(out / pal / f"{name}.png")
    shots.setdefault(pal, []).append((name, img))
    if styled:
        continue
for pal, items in shots.items():
    cols, cw, ch = 8, 248, 262
    rows = (len(items) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * cw + 8, rows * ch + 8), (90, 90, 90))
    d = ImageDraw.Draw(sheet)
    for i, (name, img) in enumerate(items):
        x, y = 8 + (i % cols) * cw, 8 + (i // cols) * ch
        sheet.paste(img, (x, y))
        d.text((x, y + 242), name, fill=(235, 235, 235))
    sheet.save(out / f"sheet_{pal}.png")
print(f"ui_render: PNGs in {out}/<palette>/ and {out}/sheet_<palette>.png")
if len(sys.argv) > 2:
    for f in sorted(Path(sys.argv[2]).glob("*.ppm")):
        Image.open(f).convert("RGB").save(f.with_suffix(".png"))
    print(f"ui_render: rolling-digit filmstrips in {sys.argv[2]}/*.png")
