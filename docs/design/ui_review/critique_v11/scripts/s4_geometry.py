from lib import *
import itertools
v = mod
print("## 4a band y-ranges (pixel scan of 'playing' + code constants)")
sc, cv, rec, arts = render(scene("playing"))
def rowocc(sc, x0=0, x1=240):
    return [sum(1 for x in range(x0, x1) if sc.px[y * 240 + x] != T.BG) for y in range(240)]
occ = rowocc(sc)
runs = []; cur = None
for y, o in enumerate(occ):
    if o and cur is None: cur = y
    if not o and cur is not None: runs.append((cur, y - 1)); cur = None
if cur is not None: runs.append((cur, 239))
print("non-BG row runs (y0..y1):", runs)
print("gaps between runs:", [(runs[i][1] + 1, runs[i + 1][0] - 1, runs[i + 1][0] - runs[i][1] - 1) for i in range(len(runs) - 1)])
print("code: header chips y1..19 (+sel bar 21..22), knob tiles y24..76 (label 24..39, body 41..76), status text cap y80 (boxes y79..91), music y96..141 (playhead cap y93..94, column 95..141), buttons y146..181 (2 rows x17, 1 px gap), keybed y186..218 (+playhead bar 219..220), cue strip text y225 (white panel 222..237)")
print("strip seam y=120 (strips (0,120),(120,240))")
print("\n## 4b texts crossing y=120 (ink bbox y0<120<y1), all scenes")
cross = []
for n in v.STATE["scenes"]:
    if n.startswith("sheet"): continue
    sc, tx, arts = texts_with_bbox(scene(n))
    for r in tx:
        if r["bbox"][1] < 120 < r["bbox"][3]: cross.append((n, r["s"], r["bbox"]))
print(cross or "none")
print("\n## 4c message panel")
print("TB_X =", v.TB_X, "panel x", v.TB_X, "..", 239, "width", 240 - v.TB_X, "; y0..21; chips end x:", 46 + 3 * 14 + 13, "; header algo name starts x105; MIDI field ends ~177")
sc, cv, rec, arts = render(scene("playing"))
hdr = [(r["s"], r["bbox"]) for r in texts_with_bbox(scene("playing"))[1] if r["bbox"][1] < 22]
print("header text ink in 'playing':", hdr)
print("header content overlapped by panel (x>=104): algo name + MIDI field (x105..177) are covered whenever a toast/hold shows; chips (46..99) and BPM stay.")
print("\n## 4d message text extents per scene (ink bbox of texts drawn inside the panel; panel x 104..239)")
for n in ["dice", "hold_edit", "hold_home", "hold_glo", "hold_save", "midi_off", "channel_added", "turn_select", "turn_algo", "turn_presets", "oct_row2", "row3_rig"]:
    st = scene(n); sc, tx, arts = texts_with_bbox(st); vis = visible_flags(st, sc, tx)
    ms = [(r["s"], r["font"], r["bbox"]) for r, vv in zip(tx, vis) if vv and r["bbox"][1] < 22 and r["bbox"][0] >= 104 - 1]
    cutflag = [(r["s"]) for r in tx if r["flags"] & 2]
    print(n, ms, "cut-flags:", cutflag)
print("\n## 4e fixed strip rows (music)")
y0, y1 = 96, 141
strip_h = 3; nrows = 4; sh = nrows * 4; bh_max = (y1 - y0) - sh - 1
print("bh_max =", bh_max, "bars occupy y%d..%d; strip rows start y=%d: CH1 %d..%d, CH2 %d..%d, CH3 %d..%d, CH4 %d..%d; spacing 4 px (3 px strip + 1 px gap); lane bottom y1=%d (bars end y%d)" % (
  y0, y0 + bh_max - 1, y0 + bh_max + 2, *(sum([[y0 + bh_max + 2 + c * 4, y0 + bh_max + 2 + c * 4 + 2] for c in range(4)], [])), y1, y0 + bh_max - 1))
print("bar min height pitched = 6, max =", bh_max - 3, "; single lane bar = 8; accent cap 2 px at by-4 (2 px gap above bar); highest pitched bar top y=", y0 + bh_max - (6 + (bh_max - 9)), "; accent cap on tallest = y", y0 + bh_max - (6 + (bh_max - 9)) - 4, "(playhead cap at y93..94, column starts 95)")
print("\n## 4f things < 2 px apart / touching (computed)")
items = []
# pixel-level: horizontal gaps between neighbours
print("knob tiles: x1..58, 61..118, 121..178, 181..238 (each 58 wide), gap 2 px between tiles, 1 px left, 1 px right margin; label cell y24..39 / value cell y41..76: gap 1 px (y40)")
print("header chips: x46..58, 60..72, 74..86, 88..100 (13 wide, 1 px gaps); sel bar y21..22 directly under chip (chip ends y19: gap 1 px); chips y1..19; panel starts x104 (gap 3 px from chip 4 right edge x100? chip4 ends x100)")
print("status row boxes: 11 wide, 2 px gap (x pitch 13); playhead cap y93..94 vs status text ink bottom y<=92 -> 0..1 px")
# playhead / cursor vs status texts: numerically
st = scene("playing")
res = []
for ph in range(16):
    S2 = dict(st, playhead=ph); S2["pages"] = 2
    sc, tx, arts = texts_with_bbox(S2)
    capx = ph * 15 + 1 - 1; capx1 = capx + 15
    for r in tx:
        if 76 <= r["bbox"][1] < 93:
            x0, y0b, x1, y1b = r["bbox"]
            if x0 < capx1 and x1 > capx:
                res.append((ph + 1, r["s"], r["bbox"], "gap to cap top y93 = %d px" % (93 - y1b)))
print("playhead cap (y93..94) x-overlap with status text:", res)
res = []
for cu in range(1, 17):
    S2 = dict(scene("rec_playing"), cursor=cu)
    sc, tx, arts = texts_with_bbox(S2)
    c = cu - 1; fx0 = c * 15 + 1 - 2; fx1 = fx0 + 17
    for r in tx:
        if 60 <= r["bbox"][1] < 93 or r["bbox"][1] >= 140 and r["bbox"][1] < 190:
            x0, y0b, x1, y1b = r["bbox"]
            if x0 < fx1 and x1 > fx0 and r["bbox"][1] < 93: res.append((cu, r["s"], r["bbox"]))
print("REC cursor frame (y93..141, x pitch 15, 17 wide) overlaps text above (y<93):", res)
print("REC cursor frame y93..141 vs buttons y146: 4 px gap; frame vs knob tiles bottom y76: 16 px; frame (2 px thick, 1 px inner gap) double ring x-2..x+14 vs neighbours: adjacent frame pitch 15 so adjacent cursors' outer ring overlaps column neighbours by 2 px")
print("lane column pitch 15: bar 13 wide, 2 px gap between bars; slide bridge 2 px; strip 13 wide")
print("button tiles: x pitch 40, 38 wide, 2 px gap; rows y146..162 and y164..180, 1 px gap between rows; keybed y186: 4 px below buttons")
print("keybed: white keys x pitch 15, 14 wide (1 px gap); black keys 10 wide, 1 px BG halo; keys y186..218, playhead bar y219..220 (0 gap, directly under)")
print("cue strip text ink: KEYS = STEPS y%s; white panel (setup) y222..237" % [r["bbox"] for r in texts_with_bbox(scene('playing'))[1] if r['s'] == 'KEYS = STEPS'])
print("bottom margin: last text ink bottom (playing):", max(r["bbox"][3] for r in texts_with_bbox(scene("playing"))[1]), "screen 240")
