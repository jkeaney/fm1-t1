# SPDX-License-Identifier: GPL-3.0-only
"""algos_v9.py: the screen's ONLY source of algorithm-specific content. The screen code never names an algorithm.

Each algorithm is one UiAlgo record, built here from docs/design/knob_map.tsv (PERF rows; labels, ranges, defaults,
value names), mirroring the firmware's AlgoDesc (firmware/src/sq_types.h: name[8], morph[] label/min/max/def,
knob_morph[] slots K1..K4 = ROW 1, S1..S4 = ROW 2, layer[]/sub[] for the five shared dice layers). Adding an
algorithm = adding its AlgoDesc row + its knob_map.tsv rows (+ an entry in UI_EXTRA only if it needs one of the
optional hints below). Every element of the screen then adapts:
  header        short name (<= 6 chars; UI_EXTRA 'short' when the name is longer than the header slot)
  knob tiles    per row: label, value text (number / value name / format), meter fraction = (v-min)/(max-min)
  music row     'lane': 'pitched' (bar height = pitch) or 'single' (one pitch: a hit marker); default pitched
  dice          the five shared layers are fixed (SQ_NLAYER), so the six dice icons never change; an algorithm
                only supplies its sub-labels (AlgoDesc.sub) for the toast, '-' = layer absent (icon drawn dimmed)
  keybed        nothing algorithm-specific
check() validates every record against the screen's limits (label <= 6 chars and fits the tile; every value text fits
the tile in AF_L, else AF_M; short name fits the header), the way tools/gen_ui_tables.py checks widths."""
import os, re

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", "..", "..", ".."))

# optional per-algorithm hints that the TSVs do not carry (everything else is parsed)
PATT_TAGS = ("ACD", "HSE", "FNK", "TEC", "DUB", "MEL")          # docs/design/algorithms/patterns/SPEC.md section 1


def patt_pick(v, st):
    return "%s %d" % (PATT_TAGS[(v - 1) // 20], (v - 1) % 20 + 1)


def stage_total(v, st):
    heard = (st or {}).get("heard")
    return "%d" % v if heard is None or heard >= v else "%d(%d)" % (v, heard)


UI_EXTRA = {
    "PATTERNS": {"short": "PATT", "value_fn": {"PICK": patt_pick}},
    "EUCLID": {"lane": "single"},
    "STAGE": {"value_fn": {"TOTAL": stage_total}},
}
SLOT_ROW = {"K": 0, "S": 1}


def parse_value(spec):
    """'den 0..16 def 12; 0 = KEYS ONLY (the cell value reads KEYS)' -> dict(min, max, def, names, fmt, zero)"""
    d = {"min": 0, "max": 0, "def": 0, "names": None, "fmt": None, "zero": None}
    head = spec.split(";")[0]
    m = re.search(r"(-?\d+)\.\.(-?\d+)", head)
    toks = head.split()
    if m:
        d["min"], d["max"] = int(m.group(1)), int(m.group(2))
        md = re.search(r"\bdef (\w+)", head)
        if md and md.group(1).lstrip("-").isdigit():
            d["def"] = int(md.group(1))
        else:
            d["def"] = d["min"]
        after = head[m.end():]
        after = re.sub(r"\bdef \S+", "", after)
        after = re.sub(r"\(.*?\)|\bfast\b.*", "", after)
        names = [t for t in after.split() if re.fullmatch(r"[A-Z0-9/+\-]{1,6}", t) and not t.isdigit()]
        if len(names) == d["max"] - d["min"] + 1:
            d["names"] = names
    else:                                             # 'dir FWD BWD PEND' / 'crv AUTO SINE ...' / '... def UP'
        md = re.search(r"\bdef (\w+)", head)
        body = re.sub(r"\bdef \S+", "", head).split()
        names = [t for t in body[1:] if re.fullmatch(r"[A-Z0-9/+\-]{1,6}", t)]
        if md and md.group(1) in names:
            names = [n for n in names]
        d["names"], d["min"], d["max"] = names, 0, len(names) - 1
        d["def"] = names.index(md.group(1)) if md and md.group(1) in names else 0
    fm = re.search(r"cell reads (\d+) (\w+)\.\.", spec)
    if fm:
        d["fmt"] = "%d " + fm.group(2)
    zm = re.search(r"0 = KEYS ONLY", spec)
    if zm:
        d["zero"] = "KEYS"
    if "CAT nn" in spec:
        d["fmt"] = "CAT %d"
    return d


def load():
    path = os.path.join(REPO, "docs", "design", "knob_map.tsv")
    algos = {}
    with open(path, encoding="utf-8") as f:
        next(f)
        for line in f:
            c = line.rstrip("\n").split("\t")
            if len(c) < 6 or c[0] != "PERF" or c[1] == "*" or not re.fullmatch(r"[KS][1-4]", c[2]):
                continue
            a = algos.setdefault(c[1], {"name": c[1], "rows": [[None] * 4, [None] * 4]})
            a["rows"][SLOT_ROW[c[2][0]]][int(c[2][1]) - 1] = dict(label=c[3], **parse_value(c[5]))
    for n, a in algos.items():
        a.update({"short": n, "lane": "pitched", "absent": ()})
        ex = UI_EXTRA.get(n, {})
        for row in a["rows"]:
            for k in row:
                if k and k["label"] in ex.get("value_fn", {}):
                    k["fn"] = ex["value_fn"][k["label"]]
        a.update({kk: vv for kk, vv in ex.items() if kk != "value_fn"})
    return algos


def value_text(k, v=None, st=None):
    """the text a knob tile shows for value v (default when None); an algorithm callback wins"""
    v = k["def"] if v is None else v
    if k.get("fn"):
        return k["fn"](v, st)
    if v == 0 and k["zero"]:
        return k["zero"]
    if k["names"]:
        return k["names"][v - k["min"]]
    if k["fmt"]:
        return k["fmt"] % v
    return str(v)


def widest_value(k, text_w, f, zero=True):
    vals = [value_text(k, v) for v in range(k["min"], k["max"] + 1) if zero or not (v == 0 and k["zero"])]
    if k.get("fn") and k["label"] == "TOTAL":
        vals.append(k["fn"](k["max"], {"heard": k["max"] - 1}))
    return max(vals, key=lambda s: text_w(f, s))


def value_font(k, text_w, AF_M, AF_L, tile=54):
    """ONE font per knob for its whole range, so the value never jumps size while turning"""
    w = widest_value(k, text_w, AF_L, zero=False)      # the zero word (KEYS) is a mode change: it may drop to AF_M
    ok_l = text_w(AF_L, w) <= tile and all(c.isdigit() or c.isupper() or c == " " for c in w)
    return AF_L if ok_l else AF_M


MAX_DOTS = 10                                    # a list longer than this gets a meter: its dots would be < 3 px


def is_list(k):
    """a value list (FWD/BWD/PEND): position dots instead of a meter"""
    return bool(k["names"]) and len(k["names"]) <= MAX_DOTS


def frac(k, v=None):
    v = k["def"] if v is None else v
    return 0 if k["max"] == k["min"] else 100 * (v - k["min"]) // (k["max"] - k["min"])


ALGOS = load()
ORDER = ["ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS"]


def register(name, rows, **hints):
    """how a new algorithm joins the screen (here: the demo in v8.py); in firmware: its AlgoDesc row + TSV rows"""
    ALGOS[name] = {"name": name, "rows": rows, "short": hints.get("short", name), "lane": hints.get("lane", "pitched"),
                   "absent": hints.get("absent", ())}
    if name not in ORDER:
        ORDER.append(name)


def check(text_w, AF_M, AF_L, tile=54, header=60):
    """every limit the screen relies on; returns a list of problems (empty = every algorithm fits every element)"""
    bad = []
    for n in ORDER:
        a = ALGOS[n]
        if text_w(AF_M, a["short"]) > header or len(a["short"]) > 6:
            bad.append("%s: short name %r does not fit the header" % (n, a["short"]))
        for r, row in enumerate(a["rows"]):
            for i, k in enumerate(row):
                if k is None:
                    bad.append("%s row %d K%d: missing" % (n, r + 1, i + 1))
                    continue
                if len(k["label"]) > 6 or text_w(AF_M, k["label"]) > tile + 2:
                    bad.append("%s %s: label too wide" % (n, k["label"]))
                widest = widest_value(k, text_w, AF_M)
                if text_w(AF_M, widest) > tile + 2:
                    bad.append("%s %s: value %r too wide even in AF_M" % (n, k["label"], widest))
    return bad
