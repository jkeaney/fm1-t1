#!/usr/bin/env python3
"""analyze.py: turns the raw dumps of measure.sh (texts_all.tsv, btn_layout.tsv, palettes.tsv) into elements.tsv and the markdown tables
that EVIDENCE.md quotes. Facts only. Run from the repo root after measure.sh:  python3 docs/design/ui_review/analyze.py [rawdir]
Role names (T_TEXT ...) are assigned from the code of firmware/src/ui_sq.c (the draw calls) and checked against the colours the real draw
recorded under palette 0 (GREY); a mismatch aborts."""
import csv, sys, os, collections, re

RAW = sys.argv[1] if len(sys.argv) > 1 else "build/ui_review"
OUT = os.path.join("docs", "design", "ui_review")
ELL = "\x85"

def rd(name):
    with open(os.path.join(RAW, name), encoding="latin-1", newline="") as f:
        return list(csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE))

pal = {r["name"]: r for r in rd("palettes.tsv")}
P0 = pal["GREY"]
def pv(role, p=P0): return int(p["T_" + role], 16)
def rgb(c):
    return (((c >> 11) & 31) * 255 // 31, ((c >> 5) & 63) * 255 // 63, (c & 31) * 255 // 31)
def lum(c):
    def f(v):
        v /= 255.0
        return v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4
    r, g, b = rgb(c); return 0.2126 * f(r) + 0.7152 * f(g) + 0.0722 * f(b)
def cr(a, b):
    la, lb = lum(a), lum(b)
    if la < lb: la, lb = lb, la
    return (la + 0.05) / (lb + 0.05)

LEG_X = [0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218]
texts = rd("texts_all.tsv"); btn = rd("btn_layout.tsv")
for r in texts:
    for k in ("blit_y", "x0", "y0", "x1", "y1", "pen_y", "maxw", "cut", "full_px", "drawn_px", "ink_w"): r[k] = int(r[k])
def disp(s): return s.replace(ELL, "…")

by_scen = collections.defaultdict(list)
for r in texts: by_scen[r["scenario"]].append(r)
btn_by = collections.defaultdict(list)
for b in btn: btn_by[b["scenario"]].append(b)

def page_of(sc):
    m = re.match(r"page_(\w+?)_algo", sc)
    return m.group(1) if m else "PERF"
TWO_ROW = {"PERF", "FIRST_RUN", "ADD_PAGE", "MOVE_HELD", "ALGO_PICK"}   # uisq_pages[].two_rows

def classify(sc, r):
    """-> element id for a TEXT record, from its pen position (the line top: ink rows differ per glyph) and the blit it went into"""
    pg = page_of(sc); by = r["blit_y"]; x0, py = r["x0"], r["pen_y"]
    if by == 0:
        if py < 20:
            if x0 < 12: return "H1"
            if x0 < 68: return "H2"
            if x0 < 96: return "H3"
            if x0 < 172: return "H4"
            if x0 < 214: return "H5.%d" % ((x0 - 174) // 10 + 1)
            return "H6"
        if py < 48:
            c = min(3, x0 // 60)
            if pg in ("NOTE_EDIT", "REC"): return "Z1.BANNER"
            return "CH%d.L%d" % (c + 1, 1 if py < 34 else 2)
        if py < 62: return "LANE.PLAYPAGE" if r["drawn"].startswith(">") else ("LANE.HDR" if pg != "SCALE" else "Z2.HARMONY")
        return "LANE.GLYPH"
    if by == 108:
        if py < 52:
            two = pg in TWO_ROW
            if x0 < 36:
                h = 0 if py < 26 else 1
                return "TAG.%s.L%d" % ("OCTDN" if h == 0 else "OCTUP", 1 if (py - 26 * h) < 12 else 2)
            k = (x0 - 36) // 51 + 1
            if two:
                return "KNOB.R%d.K%d.%s" % (py // 26 + 1, k, "LABEL" if py % 26 < 12 else "VALUE")
            if py >= 28: return "STATUS.%d" % (1 if py < 40 else 2)
            return "KNOB.R1.K%d.%s" % (k, "LABEL" if py < 10 else "VALUE")
        row = (py - 52) // 26
        bl = [b for b in btn_by[sc] if int(b["row"]) == row]
        cx = (x0 + r["x1"]) // 2
        cell = 0
        for b in bl:
            if int(b["x"]) <= cx: cell = int(b["cell"])
        return "BTN.R%d.C%d.%s" % (row + 1, cell + 1, "TAP" if (py - 52) % 26 < 12 else "SUB")
    if py < 9:
        i = 0
        for j, lx in enumerate(LEG_X):
            if lx <= x0: i = j
        return "KEY.B%02d" % (i + 1)
    return "TOAST" if sc.endswith("toast") else "CUE"

def role_of(eid, sc, r):
    """T_* roles of (fg, bg) from the draw code; verified against the recorded colours under palette 0"""
    fg, bg = r["fg565"], r["bg565"]
    f, b = int(fg, 16), int(bg, 16)
    cands_f = [k for k in ("TEXT", "MID", "DIM", "ACCENT", "INK", "THEME", "REC", "BG") if pv(k) == f]
    cands_b = [k for k in ("BG", "SEL", "TINT", "RAISE", "LANE") if pv(k) == b]
    # disambiguate equal palette-0 values by the zone: RAISE/LANE share 39C7 (legend vs lane); ACCENT/REC share FFDF (REC page only)
    if eid.startswith("KEY."): cands_b = ["RAISE"]
    elif eid.startswith("LANE.GLYPH"): cands_b = ["LANE"]
    elif b == pv("BG"): cands_b = ["BG"]
    if "ACCENT" in cands_f and "REC" in cands_f:
        cands_f = ["REC"] if (page_of(sc) == "REC" and eid in ("H1", "Z1.BANNER")) else ["ACCENT"]
    if "BG" in cands_f and "INK" in cands_f: cands_f = ["INK"]
    if "TINT" in cands_b and b == pv("SURF"): pass
    assert cands_f and cands_b, (sc, eid, fg, bg, cands_f, cands_b)
    return cands_f[0], cands_b[0]

# ---------------------------------------------------------------- the PERF-page scenarios (layout page PERF; overlays included in the survey separately)
PERF_SC = [s for s in by_scen if s.startswith("PERF_") or re.match(r"page_PERF_algo\d(_row2)?$", s)]
algo_names = ["ACID", "EUCL", "SNAK", "STAG", "ARP", "PATT"]

# element -> {text -> (px, slot, cut_to, scenarios)}
elem = collections.OrderedDict()
def note(eid, r, sc):
    d = elem.setdefault(eid, {"variants": collections.OrderedDict(), "roles": collections.OrderedDict(), "box": [], "maxw": 0})
    full = r["full"]
    v = d["variants"].setdefault(full, {"px": r["full_px"], "drawn": r["drawn"], "cut": r["cut"], "scen": set(), "ink_w": r["ink_w"], "over": set()})
    v["scen"].add(sc)
    lim = r["maxw"]
    if not lim and eid.startswith("BTN"):
        m = re.match(r"BTN\.R(\d)\.C(\d)", eid)
        lim = [int(b["w"]) for b in btn_by[sc] if int(b["row"]) == int(m[1]) - 1 and int(b["cell"]) == int(m[2]) - 1][0]
    if lim and r["full_px"] > lim: v["over"].add(lim)
    if lim: d["maxw"] = lim
    fr, br = role_of(eid, sc, r)
    d["roles"].setdefault((fr, br), set()).add(sc)
    d["box"].append((r["x0"], r["y0"], r["x1"], r["y1"], r["blit_y"]))
for sc in PERF_SC:
    for r in by_scen[sc]:
        if r["kind"] != "T": continue
        eid = classify(sc, r)
        note(eid, r, sc)

# ---------------------------------------------------------------- slot boxes (screen px) from SCREEN_SPEC 2 and the draw code
def slot(eid):
    m = re.match(r"([A-Z]+)(\d*)\.(.*)", eid)
    if eid == "H1": return (2, 4, 10, 12)
    if eid == "H2": return (23, 4, 45, 12)
    if eid == "H3": return (68, 4, 28, 12)
    if eid == "H4": return (96, 4, 76, 12)
    if eid.startswith("H5."): k = int(eid[3:]) - 1; return (174 + 10 * k, 4, 9, 11)
    if eid == "H6": return (214, 4, 24, 12)
    m = re.match(r"CH(\d)\.L(\d)", eid)
    if m: c, l = int(m[1]) - 1, int(m[2]); return (60 * c + 2, 20 + (2 if l == 1 else 14), 56, 12)
    if eid == "LANE.HDR": return (2, 49, 234, 12)
    if eid == "LANE.PLAYPAGE": return (200, 49, 36, 12)
    if eid == "LANE.GLYPH": return (0, 95, 14, 13)
    m = re.match(r"TAG\.(OCTDN|OCTUP)\.L(\d)", eid)
    if m: h = 0 if m[1] == "OCTDN" else 1; return (0 if m[2] == "1" else 2, 108 + 26 * h + (1 if m[2] == "1" else 13), 36 if m[2] == "1" else 33, 12)
    m = re.match(r"KNOB\.R(\d)\.K(\d)\.(LABEL|VALUE)", eid)
    if m: r_, k = int(m[1]) - 1, int(m[2]) - 1; return (36 + 51 * k + 2, 108 + 26 * r_ + (1 if m[3] == "LABEL" else 13), 49, 12)
    m = re.match(r"BTN\.R(\d)\.C(\d)\.(TAP|SUB)", eid)
    if m:
        row, c = int(m[1]) - 1, int(m[2]) - 1
        b = [b for b in btn_by["PERF_default"] if int(b["row"]) == row and int(b["cell"]) == c][0]
        return (int(b["x"]), 160 + 26 * row + (1 if m[3] == "TAP" else 13), int(b["w"]), 12)
    m = re.match(r"KEY\.B(\d+)", eid)
    if m: i = int(m[1]) - 1; return (LEG_X[i], 212, 20, 12)
    if eid == "CUE": return (0, 224, 240, 12)
    if eid == "TOAST": return (4, 224, 236, 12)
    return None

# ---------------------------------------------------------------- spec text per element group (quotes with file + section)
SPEC = {
 "H1": ("transport: `>` playing, `.` stopped, `R` REC armed (T_REC)", "SCREEN_SPEC 2.1 H1", "B_PLAY,B_REC", "stopped '.', playing '>', REC armed 'R' (T_REC)"),
 "H2": ("clock dot (8x8 at x=12,y=6) and tempo source (section 6): dot + text from x=23", "SCREEN_SPEC 2.1 H2; 6.2", "none on PERF (tempo: EN_K1 on SET)", "INT n / EXT -- / EXT n / WAIT (6.2); ui_sq.c draws INT n only (placeholder)"),
 "H3": ("preset: `P01`, `P05?` pending, `P05>` loading", "SCREEN_SPEC 2.1 H3", "EN_PRESET,B_SAVE", "Pnn / Pnn? / Pnn>; ui_sq.c draws Pnn and Pnn?"),
 "H4": ("`1 ACID M1` channel number, algorithm (4 letters), MIDI channel `M1`..`M16` or `MOFF`", "SCREEN_SPEC 2.1 H4", "EN_SELECT,EN_ALGO,EN_K2 (CHAN)", "any channel x algorithm x MIDI 1..16/OFF"),
 "H5": ("page map, drawn as graphics: viewed = outlined (T_TEXT), playing = filled (T_SEL fill, T_INK digit), exists = plain T_DIM outline, hidden-with-content = dotted T_DIM outline, never existed = 2x2 dot, partial last page = dashed bottom edge", "SCREEN_SPEC 2.1 H5", "KEY_B09_C#5 (PG-),KEY_B10_D#5 (PG+),EN_K1 (CHAN LENGTH)", "viewed, playing, exists, hidden, never existed, partial"),
 "H6": ("`L32` pattern length", "SCREEN_SPEC 2.1 H6", "EN_K1 (CHAN LENGTH)", "L1..L64"),
 "CH": ("4 channel cells (60x28 each): line 1 `1 ACID`, line 2 `MIDI 2`/`MIDI OFF`; muted draws the whole cell dim and line 2 reads `MUTE 2`; selected cell inverted (T_SEL fill, T_INK text); a 4x4 activity dot flashes on each note out", "SCREEN_SPEC 2.2 and 3 (Z1 row)", "EN_SELECT,EN_ALGO,KEY_B01_F#3..KEY_B04_C#4 (M1..M4),EN_K2 (CHAN)", "normal, selected, muted, MIDI OFF (T_ACCENT), sounding (dot)"),
 "LANE.HDR": ("lane header line: `PG 1/2  FOLLOW  hold=EDIT        >P2` (viewed page / pages, FOLLOW, the white-key hold, the playhead page)", "SCREEN_SPEC 2.3; SEQUENCER_UX 2.1", "KEY_B09_C#5,KEY_B10_D#5,EN_K4 (CHAN FOLLOW)", "viewed page, FOLLOW on/off"),
 "LANE.PLAYPAGE": ("`>P3` appears at the lane's right edge when the playhead is on another page", "SEQUENCER_UX 1.5", "B_PLAY", "playing only"),
 "LANE.GLYPH": ("16 step cells, bottom 14 px one glyph row: `'` edited, `!` accent, `~` slide, `x2` ratchet; cell n is the label of white key n+1", "SCREEN_SPEC 2.3", "KEY_W01_F3..KEY_W16_G5 (tap on/off, hold = NOTE EDIT)", "on/off, velocity, accent, slide, ratchet, edited, past LENGTH (dim), playhead"),
 "TAG": ("tag column = the OCT- (upper half) and OCT+ (lower half) button labels of the page; PERF: `OCT-` over `h:peek`, `OCT+` over `h:peek`", "SCREEN_SPEC 2.4 BT", "B_OCTDN,B_OCTUP", "live row filled (T_SEL); labels from button_modes.tsv (ROW 1 / ROW 2)"),
 "KNOB": ("one knob cell: line 1 the label (AF_S, T_MID), line 2 the value (AF_S, T_THEME, T_ACCENT while the knob is hot); ROW 2 (S1..S4) drawn dim; the live row is filled (T_SEL)", "SCREEN_SPEC 2.4 BK1..BK4 / BS1..BS4", "EN_K1..EN_K4", "live row, dim row, hot (600 ms after a turn)"),
 "STATUS": ("lower 24 px hold two status lines (AF_S, 12 px each) at y=136 and y=148 spanning x=36..240; PERF has none", "SCREEN_SPEC 2.4, 7.1", "(read-only)", "single-row pages only"),
 "BTN": ("2 rows of 6 buttons, tap label y+1, sub-label y+13; sub-label = dice layer of the algorithm (ALGORITHM_SET 4.4) or the hold role with a hold mark; absent layer = dim with a dash; tap label that differs from the printed word is T_ACCENT", "SCREEN_SPEC 2.5", "row 1: B_FX B_SCL B_ENV B_LFO B_EDIT B_GLO; row 2: B_HOME B_SAVE B_ARP B_SEQ B_PLAY B_REC", "normal, dim (layer absent), accent (RST/BACK/CANCEL)"),
 "KEY": ("Z5: 11 cells of 20 px at pitch 21 in the physical 3-2-3-2-1 grouping; each cell shows the page's label for that black key (max 6 characters); inverted while the key is down; muted-channel cells dim", "SCREEN_SPEC 2.6", "KEY_B01_F#3..KEY_B11_F#5", "normal, muted (T_DIM), key-down inverted (spec; not drawn by ui_sq.c)"),
 "CUE": ("Z6: one line, 34 characters nominal (binding limit 240 px), priority table SEQUENCER_UX 11.2", "SCREEN_SPEC 2.6; SEQUENCER_UX 11.2", "(read-only)", "page cue; PERF stopped 'PLAY: start   K1-K4: morph'; PERF playing 'TAP STEP=ON/OFF  HOLD=EDIT NOTE'; ALGO_PICK list for 2 s"),
 "TOAST": ("a toast is drawn in the Z6 rectangle, fill T_TINT, ink T_ACCENT, text left-aligned at x=4, 2000 ms; replaces the cue strip", "SCREEN_SPEC 2.7", "(any control that answers with a toast)", "transient"),
 "Z1.BANNER": ("a banner replaces the four cells (NOTE_EDIT, REC)", "SCREEN_SPEC 3 (per-page table)", "EN_SELECT", "NOTE_EDIT / REC only"),
}
def spec_for(eid):
    if eid.startswith("CH"): return SPEC["CH"]
    if eid.startswith("H5"): return SPEC["H5"]
    if eid.startswith("LANE.GLYPH"): return SPEC["LANE.GLYPH"]
    if eid.startswith("TAG"): return SPEC["TAG"]
    if eid.startswith("KNOB"): return SPEC["KNOB"]
    if eid.startswith("STATUS"): return SPEC["STATUS"]
    if eid.startswith("BTN"): return SPEC["BTN"]
    if eid.startswith("KEY"): return SPEC["KEY"]
    return SPEC.get(eid, ("", "", "", ""))

# ctl detail per instance
BTN_CTL = [["B_FX","B_SCL","B_ENV","B_LFO","B_EDIT","B_GLO"],["B_HOME","B_SAVE","B_ARP","B_SEQ","B_PLAY","B_REC"]]
KEY_CTL = ["KEY_B01_F#3","KEY_B02_G#3","KEY_B03_A#3","KEY_B04_C#4","KEY_B05_D#4","KEY_B06_F#4","KEY_B07_G#4","KEY_B08_A#4","KEY_B09_C#5","KEY_B10_D#5","KEY_B11_F#5"]
def ctl_for(eid):
    m = re.match(r"BTN\.R(\d)\.C(\d)", eid)
    if m: return BTN_CTL[int(m[1]) - 1][int(m[2]) - 1]
    m = re.match(r"KEY\.B(\d+)", eid)
    if m: return KEY_CTL[int(m[1]) - 1]
    m = re.match(r"KNOB\.R(\d)\.K(\d)", eid)
    if m: return "EN_K%s (%s)" % (m[2], "ROW 1" if m[1] == "1" else "ROW 2 = S%s" % m[2])
    if eid == "LANE.GLYPH": return "KEY_W01_F3..KEY_W16_G5"
    m = re.match(r"CH(\d)", eid)
    if m: return "EN_SELECT,EN_ALGO,KEY_B0%s (M%s)" % (m[1], m[1])
    m = re.match(r"TAG\.(OCTDN|OCTUP)", eid)
    if m: return "B_" + m[1]
    return spec_for(eid)[2]

def sortkey(e):
    zone_order = ["H", "CH", "Z1", "LANE", "Z2", "TAG", "KNOB", "STATUS", "BTN", "KEY", "CUE", "TOAST"]
    for i, z in enumerate(zone_order):
        if e.startswith(z): break
    return (i, [int(t) if t.isdigit() else t for t in re.split(r"(\d+)", e)])

def zone_of(eid):
    if eid[0] == "H": return "Z0"
    if eid.startswith("CH") or eid.startswith("Z1"): return "Z1"
    if eid.startswith("LANE") or eid.startswith("Z2"): return "Z2"
    if eid.startswith(("TAG", "KNOB", "STATUS")): return "Z3"
    if eid.startswith("BTN"): return "Z4"
    if eid.startswith("KEY"): return "Z5"
    return "Z6"

def container(eid):
    """the screen rows the element's own box spans (zone / cell / half), for the 'ink outside' check"""
    if eid[0] == "H": return (0, 19)
    if eid.startswith("CH"): return (20, 47)
    if eid == "LANE.HDR" or eid == "LANE.PLAYPAGE": return (48, 61)
    if eid == "LANE.GLYPH": return (95, 107)
    m = re.match(r"TAG\.(OCTDN|OCTUP)", eid)
    if m: h = 0 if m[1] == "OCTDN" else 1; return (108 + 26 * h, 133 + 26 * h)
    m = re.match(r"KNOB\.R(\d)", eid)
    if m: r_ = int(m[1]) - 1; return (108 + 26 * r_, 133 + 26 * r_)
    m = re.match(r"BTN\.R(\d)", eid)
    if m: r_ = int(m[1]) - 1; return (160 + 26 * r_, 185 + 26 * r_)
    if eid.startswith("KEY"): return (212, 223)
    if eid in ("CUE", "TOAST"): return (224, 235)
    return None

rows = []
for eid in sorted(elem, key=sortkey):
    d = elem[eid]; s = slot(eid)
    var = d["variants"]
    deflt = None
    for full, v in var.items():
        if "PERF_default" in v["scen"]: deflt = (full, v)
    if deflt is None: deflt = next(iter(var.items()))
    full0, v0 = deflt
    lim = (s[2] if (s and eid.startswith("BTN")) else d["maxw"]) or (s[2] if s else 0)
    over = ["%s=%dpx>%s" % (disp(f), v["px"], "/".join(map(str, sorted(v["over"])))) for f, v in var.items() if v["over"]]
    trunc = ["%s -> %s" % (disp(f), disp(v["drawn"])) for f, v in var.items() if v["cut"]]
    roles = "; ".join("%s/%s [%s]" % ("T_" + fr, "T_" + br, ",".join(sorted(sc)[:2]) + ("..." if len(sc) > 2 else "")) for (fr, br), sc in d["roles"].items())
    ys = [(b[1] + b[4], b[3] + b[4]) for b in d["box"]]; ymin = min(y[0] for y in ys); ymax = max(y[1] for y in ys)
    inkh = ymax - ymin
    sp = spec_for(eid)
    outside = ""
    cy = container(eid)
    if cy and (ymin < cy[0] or ymax - 1 > cy[1]): outside = "ink rows %d..%d vs container rows %d..%d" % (ymin, ymax - 1, cy[0], cy[1])
    rows.append(dict(id=eid, zone=zone_of(eid), x=s[0] if s else "", y=s[1] if s else "", w=s[2] if s else "", h=s[3] if s else "", font="AF_S",
        default_text=disp(full0), default_px=v0["px"], slot_limit_px=lim, fits_default="yes" if not v0["over"] else "NO",
        n_variants=len(var), variants=" | ".join("%s(%d)" % (disp(f), v["px"]) for f, v in var.items()),
        over_limit=" | ".join(over), truncated=" | ".join(trunc), ink_rows_outside_slot=outside,
        colours=roles, spec_function=sp[0], spec_ref=sp[1], controls=ctl_for(eid), states=sp[3]))
for r in rows: r["kind"] = "text"
def G(id_, zone, x, y, w, h, colours, fn, ref, ctl, states):
    return dict(id=id_, zone=zone, x=x, y=y, w=w, h=h, font="", default_text="", default_px="", slot_limit_px="", fits_default="", n_variants="", variants="",
                over_limit="", truncated="", ink_rows_outside_slot="", colours=colours, spec_function=fn, spec_ref=ref, controls=ctl, states=states, kind="graphic")
GR = [
 G("H2.DOT", "Z0", 12, 6, 8, 8, "FIXED amber 0xFD80 (dark bg) / 0xCB80 (light bg) - not a theme role / T_BG", "8x8 clock dot, amber solid = FM-1's own clock (INT)", "SCREEN_SPEC 2.1 H2, 6.3", "none", "INT amber solid drawn; EXT cyan-with-hole and WAIT ring in spec, not drawn"),
 G("H5.CELL", "Z0", "174+10k", 4, 9, 11, "T_TEXT/T_BG outline (viewed or playing); T_DIM/T_BG outline (exists); T_SEL fill + T_INK digit (playing); T_DIM dots (hidden / never existed)", SPEC["H5"][0], SPEC["H5"][1], "KEY_B09_C#5,KEY_B10_D#5,EN_K1 (CHAN)", SPEC["H5"][3]),
 G("CH.SELECT_FILL", "Z1", "60c+1", 20, 58, 28, "T_SEL / T_BG (the selected cell's fill); text on it T_INK", SPEC["CH"][0], SPEC["CH"][1], "EN_SELECT", "selected"),
 G("CH.ACTIVITY_DOT", "Z1", "60c+54", 24, 4, 4, "T_ACCENT/T_BG (unselected) ; T_INK/T_SEL (selected)", "4x4 activity dot flashes on each note out", "SCREEN_SPEC 2.2", "(read-only)", "a note is sounding"),
 G("LANE.CELL_BG", "Z2", "15n", 62, 14, 46, "T_LANE / T_BG (step within LENGTH: T_LANE; past LENGTH: T_BG, drawn as empty)", "16 step cells, 14 px ink + 1 px gap; cells past LENGTH are dim", "SCREEN_SPEC 2.3", "KEY_W01_F3..KEY_W16_G5", "live, past LENGTH"),
 G("LANE.BAR", "Z2", "15n+1", "62+32-h", 12, "2..32", "T_THEME/T_LANE (step on); T_ACCENT/T_LANE (velocity >= 100); T_GRID/T_LANE (step off, 12x2 bar)", "top 32 px a velocity bar (lit = a step plays, height = velocity)", "SCREEN_SPEC 2.3", "KEY_W01_F3..KEY_W16_G5", "on, accent, off"),
 G("LANE.EDIT_FRAME", "Z2", "15n", 62, 14, 46, "T_ACCENT/T_BG frame (NOTE_EDIT page only)", "edited cell boxed", "SCREEN_SPEC 3 (NOTE_EDIT row)", "KEY_W01_F3..KEY_W16_G5 (hold)", "NOTE_EDIT only"),
 G("LANE.PLAYHEAD", "Z2", "15n", 106, 14, 2, "T_ACCENT/T_BG underline", "the playhead is a T_ACCENT underline of the cell", "SCREEN_SPEC 2.3", "B_PLAY", "playing"),
 G("TAG.LIVE_FILL", "Z3", 0, "108+26h", 36, 26, "T_SEL/T_BG (the live-row half of the tag column)", "live row marker", "SCREEN_SPEC 2.4", "B_OCTDN,B_OCTUP", "ROW 1 live / ROW 2 live"),
 G("KNOB.LIVE_FILL", "Z3", "36+51k", "108+26r", 51, 26, "T_SEL/T_BG (PERF: the live row is filled; ROW 2 not live is unfilled)", "the live row is filled (T_SEL)", "SCREEN_SPEC 2.4", "B_OCTDN,B_OCTUP", "ROW 1 live / ROW 2 live"),
 G("BTN.SEPARATOR", "Z4", "cell x - 1", "163/189", 1, 20, "T_LINE/T_BG (1 px divider inside the padding)", "cell separator is a 1 px T_LINE column", "SCREEN_SPEC 2.5", "(none)", "always"),
 G("BTN.HOLD_MARK", "Z4", "cell-relative", "sub line", 5, 7, "T_MID/T_BG (filled down-triangle, 6 px with gap)", "hold sub-label carries a 5x7 hold mark in place of 'h:'", "SCREEN_SPEC 2.5", "hold role of the button", "hold role present"),
 G("KEY.CELL", "Z5", "LEG_X[i]", 212, 20, 12, "T_RAISE/T_BG (the cell's well); label T_TEXT or T_DIM (muted) on it", "11 cells of 20 px at pitch 21", "SCREEN_SPEC 2.6", "KEY_B01_F#3..KEY_B11_F#5", "normal, muted"),
 G("TOAST.FILL", "Z6", 0, 224, 240, 12, "T_TINT/T_BG (replaces the cue strip while a toast shows)", "toast fill T_TINT, ink T_ACCENT", "SCREEN_SPEC 2.7", "(any control that answers)", "transient 2000 ms"),
]
rows.extend(GR)
cols = ['id','kind'] + [c for c in rows[0].keys() if c not in ('id','kind')]
with open(os.path.join(OUT, "elements.tsv"), "w", encoding="utf-8", newline="") as f:
    w = csv.writer(f, delimiter="\t", lineterminator="\n"); w.writerow(cols)
    for r in rows: w.writerow([str(r[c]).replace("\t", " ").replace("\n", " ") for c in cols])
print("elements.tsv: %d rows" % len(rows))
import shutil
for fn in ("palettes.tsv", "fonts.tsv", "truncations.tsv"): pass

# ---------------------------------------------------------------- truncation survey over EVERY page x algorithm scenario (+ PERF extras)
trunc = collections.OrderedDict()
for sc, rs in by_scen.items():
    for r in rs:
        if r["kind"] == "T" and r["cut"]:
            eid = classify(sc, r)
            key = (page_of(sc) if sc.startswith("page_") else "PERF", eid, disp(r["full"]), disp(r["drawn"]), r["full_px"], r["maxw"])
            trunc.setdefault(key, set()).add(sc)
with open(os.path.join(RAW, "truncations.tsv"), "w", encoding="utf-8") as f:
    f.write("page\telement\tfull_string\tdrawn_as\tfull_px\tslot_px\tscenarios\n")
    for k, v in trunc.items(): f.write("\t".join(map(str, k)) + "\t%d\n" % len(v))
print("truncations: %d distinct (page,element,string)" % len(trunc))

for fn in ("palettes.tsv", "fonts.tsv", "truncations.tsv"):
    shutil.copy(os.path.join(RAW, fn), os.path.join(OUT, fn))

# ---------------------------------------------------------------- every (fg, bg) role pair of every TEXT drawn in every scenario (all pages)
pa = collections.OrderedDict()
for sc, rs in by_scen.items():
    for r in rs:
        if r["kind"] != "T": continue
        eid = classify(sc, r); fr, br = role_of(eid, sc, r)
        d = pa.setdefault((fr, br), {"n": 0, "el": set(), "pg": set()})
        d["n"] += 1; d["el"].add(re.sub(r"\d+", "n", eid) if eid.startswith(("CH", "KEY", "BTN", "KNOB")) else eid); d["pg"].add(page_of(sc))
with open(os.path.join(OUT, "text_pairs.tsv"), "w", encoding="utf-8") as f:
    f.write("fg\tbg\trecords\telements\tpages\n")
    for (fr, br), d in sorted(pa.items()):
        f.write("%s\t%s\t%d\t%s\t%s\n" % (fr, br, d["n"], ",".join(sorted(d["el"])), ",".join(sorted(d["pg"]))))
print("text_pairs.tsv: %d pairs" % len(pa))
