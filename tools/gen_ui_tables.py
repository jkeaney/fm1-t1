#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P4-010 (ADR-0004): compile the design TSVs into flash-resident C tables, build/gen/ui_tables.h.

Sources (docs/design/, all hand-edited or produced by their own generator; this tool only reads them):
  page_states.tsv   the page ids (state, kind)                    -> enum ui_page_t, ui_page_rows
  control_map.tsv   every control x page it is used on            -> enum ui_ctl_t,  ui_control_rows
  button_modes.tsv  button label per page and gesture             -> ui_button_rows
  knob_map.tsv      knob label per page and algorithm context     -> enum ui_knob_t, enum ui_ctx_t, ui_knob_rows
  key_map.tsv       key label per page and gesture                -> ui_key_rows
  help_text.tsv     two help lines per control and page (made by tests/design/gen_help.py; checked by
                    tests/design/check_help.py)                   -> ui_help_rows
  tests/design/widths.tsv   pixel limits of UI strings (read by tests/design/width_probe.sh)
Only ids, labels and help text are compiled in; the prose columns (action, principle, screen, ...) stay in the TSVs.

Fails with a message naming the file, line and value when: an id is unknown or does not agree between the tables, a
row is duplicated, a label is over its width, a help line is over 34 characters, or a width in widths.tsv is over.
Label limits (SCREEN_SPEC 5): every knob, button and black-key label is at most 6 characters; 51 px (the knob cell,
SCREEN_SPEC 2.4) is the ceiling for any one label. Button cells are data per page (2.5), so the real button limit is
that each page's two rows of six cells fit 240 px (widest label + hold mark + 1 px); black-key labels over 20 px are
cut by design (section 5), so no px limit applies to them beyond 51. Knob contexts must be * or an SQ_ALGO_ name of
sq_types.h. Widths are measured with the real text_w (tests/design/width_probe.sh, needs build/gen/ui_fonts.h from ./build.sh);
without fonts the pixel step is skipped with a note (--check insists on it).

P4-027 (ADR-0007) adds the algorithm-descriptor check, run by --check and a normal run: the knob_map.tsv PERF rows of every algorithm
(and, where its firmware row exists, the AlgoDesc row of sq_algo_*.c, which must agree with the TSV) are measured on the real fonts the way the
mock's check() does (docs/design/ui_review/proposals/v12/algos_v12.py): short name <= 6 chars and fits the header slot, every one of the 8 knob
slots filled, label <= 6 chars and <= 58 px in AF_M, the widest value fits 54 px in AF_L (else 58 px in AF_M), a value list's names equal its range.
Every SCREEN_SPEC_V2 section 5 message (MESSAGES of proposals/v12/v12.py) must fit the 136 px message slot. `--selftest` proves the check fails on
a deliberately too-wide label, short name and message.

Usage: gen_ui_tables.py [OUT.h]   (default build/gen/ui_tables.h)    gen_ui_tables.py --check    gen_ui_tables.py --selftest
The output is deterministic (TSV order, no timestamps)."""
import csv, os, re, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
D = os.path.join(ROOT, "docs", "design")
WIDTHS = os.path.join(ROOT, "tests", "design", "widths.tsv")
FONTS = os.path.join(ROOT, "build", "gen", "ui_fonts.h")
PROBE = os.path.join(ROOT, "tests", "design", "width_probe.sh")
LABEL_PX = 51
HELP_CHARS = 34
MODES = ["tap", "hold", "hold-both"]
GESTURES = ["tap", "hold", "tap/hold"]
KINDS = ["page", "overlay", "prompt"]
err = []


def bad(msg):
    err.append(msg)


def rd(name):
    path = os.path.join(D, name + ".tsv")
    with open(path, encoding="utf-8", newline="") as f:
        rows = list(csv.reader(f, delimiter="\t", quoting=csv.QUOTE_NONE))
    head, body = rows[0], rows[1:]
    out = []
    for i, r in enumerate(body, 2):
        if len(r) != len(head):
            bad("%s.tsv line %d: %d fields, header has %d" % (name, i, len(r), len(head)))
            continue
        d = dict(zip(head, r))
        d["_at"] = "%s.tsv line %d" % (name, i)
        out.append(d)
    return out


def ident(s):
    s = s.replace("#", "S").replace("*", "ANY")
    return re.sub(r"[^A-Za-z0-9]+", "_", s).strip("_").upper()


def enum(prefix, ids, what):
    names = {}
    for i in ids:
        n = prefix + ident(i)
        if n in names and names[n] != i:
            bad("%s: ids %r and %r both become %s" % (what, names[n], i, n))
        names[n] = i
    return names


def cstr(s):
    o = ['"']
    for b in s.encode("utf-8"):
        c = chr(b)
        if c in '"\\':
            o.append("\\" + c)
        elif 32 <= b < 127:
            o.append(c)
        else:
            o.append("\\%03o" % b)
    o.append('"')
    return "".join(o)


def uniq(rows, keys, what):
    seen = {}
    for r in rows:
        k = tuple(r[x] for x in keys)
        if k in seen:
            bad("%s: duplicate %s (also %s)" % (r["_at"], "/".join(k), seen[k]))
        seen[k] = r["_at"]


def width_lines(extra):
    """widths.tsv rows (font, limit, string) plus the label rows; the probe's input format."""
    out = []
    for l in open(WIDTHS, encoding="utf-8"):
        if l.strip() and not l.startswith("#"):
            out.append(l.rstrip("\n"))
    return out + extra


ROW1 = ["B_FX", "B_SCL", "B_ENV", "B_LFO", "B_EDIT", "B_GLO"]
ROW2 = ["B_HOME", "B_SAVE", "B_ARP", "B_SEQ", "B_PLAY", "B_REC"]
HOLD_MARK = 6         # SCREEN_SPEC 2.5: the hold mark plus its gap, counted in a cell that has a hold sub-label
ROW_PX = 240
LABEL_CHARS = 6       # SCREEN_SPEC 5: knob label, button tap/sub-label and black-key label are at most 6 characters
# The knob contexts: "*" (every algorithm) or an algorithm name; they must equal the SQ_ALGO_* ids of sq_types.h.
ALGO_H = os.path.join(ROOT, "firmware", "src", "sq_types.h")


def algo_names():
    try:
        txt = open(ALGO_H, encoding="utf-8").read()
    except OSError:
        return None
    return set(re.findall(r"^#define SQ_ALGO_([A-Z]+)\s+\d+", txt, re.M))


def row_sums(bm, W):
    """per page and button row: widest label of each cell (+ hold mark when a hold label differs from its tap label)
    plus 1 px padding; the six cells must fit 240 px (SCREEN_SPEC 2.5). Returns [(page, row, sum)] over."""
    cell, tap = {}, {}
    for r in bm:
        if r["mode"] == "tap":
            tap[(r["page"], r["control"])] = r["label"]
    for r in bm:
        k = (r["page"], r["control"])
        w = W.get(r["label"])
        if w is None:
            continue
        if r["mode"] != "tap" and r["label"] != tap.get(k):
            w += HOLD_MARK
        cell[k] = max(cell.get(k, 0), w)
    out = []
    for pg in sorted({r["page"] for r in bm}):
        for name, row in (("row 1", ROW1), ("row 2", ROW2)):
            tot = sum(cell.get((pg, c), 0) + 1 for c in row)
            if tot > ROW_PX:
                out.append((pg, name, tot))
    return out


def width_check(labels, strict, bm=()):
    if not os.path.exists(FONTS):
        if strict:
            bad("no build/gen/ui_fonts.h: run ./build.sh once, the label widths are measured on its fonts")
        else:
            sys.stderr.write("gen_ui_tables: note: no build/gen/ui_fonts.h yet, label widths not measured\n")
        return
    lines = width_lines(["S\t%d\t%s\tlabel" % (LABEL_PX, s) for s in sorted(labels)])
    p = subprocess.run(["sh", PROBE, "-"], input="\n".join(lines) + "\n", text=True, capture_output=True)
    W = {}
    for l in p.stdout.splitlines():
        m = re.match(r"\s*(\d+)\s*/\s*\d+\s+(OK|OVER)\s+`(.*)`\s", l)
        if m:
            W[m.group(3)] = int(m.group(1))
    if p.returncode != 0:
        for l in p.stdout.splitlines():
            if " OVER " in l:
                m = re.search(r"`(.*)`\s+label\s*$", l)
                where = ", ".join(labels[m.group(1)][:3]) if m and m.group(1) in labels else "widths.tsv"
                bad("%s: label over its width: %s" % (where, l.strip()))
        if p.returncode != 1 or not any(" OVER " in l for l in p.stdout.splitlines()):
            bad("width probe failed (%d): %s" % (p.returncode, (p.stderr or p.stdout).strip()))
    else:
        for pg, row, tot in row_sums(bm, W):
            bad("button_modes.tsv: page %s %s needs %d px of cells, the screen has %d (SCREEN_SPEC 2.5)" % (pg, row, tot, ROW_PX))


def build():
    ps, cm, bm = rd("page_states"), rd("control_map"), rd("button_modes")
    km, ky, ht = rd("knob_map"), rd("key_map"), rd("help_text")
    for r in ps:
        if r["kind"] not in KINDS and not r["kind"].startswith("overlay") and not r["kind"].startswith("prompt"):
            bad("%s: unknown kind %r" % (r["_at"], r["kind"]))
    pages = [r["state"] for r in ps]
    uniq(ps, ["state"], "page_states")
    PG = set(pages)
    ctls = []
    for r in cm:
        if r["control"] not in ctls:
            ctls.append(r["control"])
    CT = set(ctls)
    uniq(cm, ["control", "context"], "control_map")
    uniq(bm, ["page", "control", "mode"], "button_modes")
    uniq(ky, ["page", "key", "gesture"], "key_map")
    uniq(ht, ["control", "context"], "help_text")
    uniq(km, ["page", "context", "knob"], "knob_map")
    cmpair = {(r["control"], r["context"]) for r in cm}
    for r in cm:
        if r["context"] not in PG:
            bad("%s: unknown page %r (not in page_states.tsv)" % (r["_at"], r["context"]))
    for r in bm:
        if r["page"] not in PG:
            bad("%s: unknown page %r" % (r["_at"], r["page"]))
        if r["control"] not in CT or not r["control"].startswith("B_"):
            bad("%s: unknown button %r (not a B_ control of control_map.tsv)" % (r["_at"], r["control"]))
        elif (r["control"], r["page"]) not in cmpair:
            bad("%s: (%s, %s) is not a row of control_map.tsv" % (r["_at"], r["control"], r["page"]))
        if r["mode"] not in MODES:
            bad("%s: unknown mode %r" % (r["_at"], r["mode"]))
    for r in ky:
        if r["page"] not in PG:
            bad("%s: unknown page %r" % (r["_at"], r["page"]))
        if r["key"] not in CT or not r["key"].startswith("KEY_"):
            bad("%s: unknown key %r (not a KEY_ control of control_map.tsv)" % (r["_at"], r["key"]))
        elif (r["key"], r["page"]) not in cmpair:
            bad("%s: (%s, %s) is not a row of control_map.tsv" % (r["_at"], r["key"], r["page"]))
        if r["gesture"] not in GESTURES:
            bad("%s: unknown gesture %r" % (r["_at"], r["gesture"]))
    for r in ht:
        if r["control"] not in CT:
            bad("%s: unknown control %r" % (r["_at"], r["control"]))
        if r["context"] not in PG:
            bad("%s: unknown page %r" % (r["_at"], r["context"]))
        elif (r["control"], r["context"]) not in cmpair:
            bad("%s: (%s, %s) is not a row of control_map.tsv" % (r["_at"], r["control"], r["context"]))
        if not r["line1"].strip():
            bad("%s: empty line1" % r["_at"])
        for n in ("line1", "line2"):
            if len(r[n]) > HELP_CHARS:
                bad("%s: %s is %d characters, the help box holds %d: %r" % (r["_at"], n, len(r[n]), HELP_CHARS, r[n]))
    knobs, ctxs = [], []
    for r in km:
        if r["page"] not in PG:
            bad("%s: unknown page %r" % (r["_at"], r["page"]))
        if r["knob"] not in knobs:
            knobs.append(r["knob"])
        if r["context"] not in ctxs:
            ctxs.append(r["context"])
        k = r["knob"]
        if not re.fullmatch(r"[KRS][1-4]|ALGO|PRESET|SELECT", k):
            bad("%s: unknown knob %r" % (r["_at"], k))
        elif k[0] == "K" or k in ("ALGO", "PRESET", "SELECT"):
            if "EN_" + k not in CT:
                bad("%s: knob %s has no control EN_%s in control_map.tsv" % (r["_at"], k, k))
    # label widths: every label of the three tables must fit the knob / button cell
    labels = {}
    for rows in (bm, km, ky):
        for r in rows:
            if not r["label"].strip():
                bad("%s: empty label" % r["_at"])
            elif len(r["label"]) > LABEL_CHARS:
                bad("%s: label %r is %d characters, the limit is %d (SCREEN_SPEC 5)" % (r["_at"], r["label"], len(r["label"]), LABEL_CHARS))
            labels.setdefault(r["label"], []).append(r["_at"])
    names = algo_names()
    ok_ctx = {"*"} | (names or set())
    for r in km:
        if names is not None and r["context"] not in ok_ctx:
            bad("%s: unknown context %r (not * or an SQ_ALGO_ name of sq_types.h: %s)" % (r["_at"], r["context"], " ".join(sorted(names))))
    return dict(ps=ps, cm=cm, bm=bm, km=km, ky=ky, ht=ht, pages=pages, ctls=ctls, knobs=knobs, ctxs=ctxs,
                labels=labels)


# ---- P4-027 (ADR-0007): the algorithm-descriptor check -------------------------------------------------------------------------------------
TILE_LABEL_PX = 58    # SCREEN_SPEC_V2 2: the knob tile and its label band are 58 px wide
VALUE_L_PX = 54       # a value in AF_L must fit 54 px (SCREEN_SPEC_V2 section 3)
HEADER_PX = 60        # the header slot of the algorithm short name (mock check())
SHORT_CHARS = 6
MSG_X, SCREEN_W = 104, 240   # the message slot (SCREEN_SPEC_V2 section 5)
SRC = os.path.join(ROOT, "firmware", "src")
MOCK_V12 = os.path.join(D, "ui_review", "proposals", "v12", "v12.py")
PATT_TAGS = ("ACD", "HSE", "FNK", "TEC", "DUB", "MEL")   # docs/design/algorithms/patterns/SPEC.md section 1
# algorithm-only value text (AlgoDesc.value_text): the strings the hook can produce, to be measured (the P5 hooks must stay within these)
HOOK_TEXTS = {("PATTERNS", "PICK"): lambda mn, mx: ["%s %d" % (PATT_TAGS[(v - 1) // 20], (v - 1) % 20 + 1) for v in range(max(mn, 1), mx + 1)],
              ("STAGE", "TOTAL"): lambda mn, mx: ["%d" % v for v in range(mn, mx + 1)] + ["%d(%d)" % (mx, mx - 1)]}


def parse_value(spec):
    """'den 0..16 def 12; 0 = KEYS ONLY (the cell value reads KEYS)' -> dict(min, max, def, names, fmt, zero) (mock algos_v12.parse_value)"""
    d = {"min": 0, "max": 0, "def": 0, "names": None, "fmt": None, "zero": None}
    head = spec.split(";")[0]
    m = re.search(r"(-?\d+)\.\.(-?\d+)", head)
    if m:
        d["min"], d["max"] = int(m.group(1)), int(m.group(2))
        md = re.search(r"\bdef (\w+)", head)
        d["def"] = int(md.group(1)) if md and md.group(1).lstrip("-").isdigit() else d["min"]
        after = re.sub(r"\bdef \S+", "", head[m.end():])
        after = re.sub(r"\(.*?\)|\bfast\b.*", "", after)
        names = [t for t in after.split() if re.fullmatch(r"[A-Z0-9/+\-]{1,6}", t) and not t.isdigit()]
        if len(names) == d["max"] - d["min"] + 1:
            d["names"] = names
    else:                                             # 'dir FWD BWD PEND' / '... def UP'
        md = re.search(r"\bdef (\w+)", head)
        body = re.sub(r"\bdef \S+", "", head).split()
        names = [t for t in body[1:] if re.fullmatch(r"[A-Z0-9/+\-]{1,6}", t)]
        d["names"], d["min"], d["max"] = names, 0, len(names) - 1
        d["def"] = names.index(md.group(1)) if md and md.group(1) in names else 0
    fm = re.search(r"cell reads (\d+) (\w+)\.\.", spec)
    if fm:
        d["fmt"] = "%d " + fm.group(2)
    if re.search(r"0 = KEYS ONLY", spec):
        d["zero"] = "KEYS"
    if "CAT nn" in spec:
        d["fmt"] = "CAT %d"
    return d


def algo_order():
    try:
        txt = open(ALGO_H, encoding="utf-8").read()
    except OSError:
        return []
    return [n for _, n in sorted((int(v), n) for n, v in re.findall(r"^#define SQ_ALGO_([A-Z]+)\s+(\d+)", txt, re.M))]


def placeholder_shorts(order):
    """ui_sq.c's uisq_short[] (the header names until an algorithm row fills short_name) by algorithm index"""
    try:
        txt = open(os.path.join(SRC, "ui_sq.c"), encoding="utf-8").read()
    except OSError:
        return {}
    m = re.search(r"uisq_short\[SQ_NALGO\]\s*=\s*\{([^}]*)\}", txt)
    sh = re.findall(r'"([^"]*)"', m.group(1)) if m else []
    return {order[i]: sh[i] for i in range(min(len(order), len(sh)))}


def firmware_row(name):
    """the ALGO_<NAME>_ROW macro of firmware/src/sq_algo_*.c as {short, knob_morph, morph[8] of (label, min, max, def, names, unit, zero)}, or None"""
    for fn in sorted(os.listdir(SRC)):
        if not (fn.startswith("sq_algo_") and fn.endswith(".c")):
            continue
        txt = open(os.path.join(SRC, fn), encoding="utf-8").read()
        m = re.search(r"#define ALGO_%s_ROW \{(.*?)/\* ALGODESC-END" % name, txt, re.S)
        if not m:
            continue
        body = m.group(1).replace("\\\n", "\n")
        arrays = {a: re.findall(r'"([^"]*)"', v) for a, v in re.findall(r"const char \*const (\w+)\[\d*\]\s*=\s*\{([^}]*)\}", txt)}
        r = {"file": fn, "short": None, "knob_morph": None, "morph": []}
        sm = re.search(r'\.short_name\s*=\s*"([^"]*)"', body)
        r["short"] = sm.group(1) if sm else None
        km = re.search(r"\.knob_morph\s*=\s*\{([^}]*)\}", body)
        r["knob_morph"] = [int(x) for x in km.group(1).split(",") if x.strip()] if km else None
        mm = re.search(r"\.morph\s*=\s*\{(.*?)\n\s*\},", body, re.S)
        for e in re.findall(r"\{\s*\"(\w+)\"\s*,([^{}]*)\}", mm.group(1) if mm else ""):
            f = [x.strip() for x in e[1].split(",")]
            names = arrays.get(f[5]) if len(f) > 5 and f[5] != "0" else None
            unit = f[6].strip('"') if len(f) > 6 and f[6] != "0" else None
            zero = f[7].strip('"') if len(f) > 7 and f[7] != "0" else None
            r["morph"].append((e[0], int(f[0]), int(f[1]), int(f[2]), names, unit, zero))
        return r
    return None


def collect_algos(km):
    """-> (algos, order, problems): per algorithm {short, rows[2][4], fw}; rows from the knob_map.tsv PERF rows; short from the firmware row, else the placeholder"""
    order = algo_order()
    shorts = placeholder_shorts(order)
    algos, probs = {}, []
    for r in km:
        if r["page"] != "PERF" or r["context"] == "*" or not re.fullmatch(r"[KS][1-4]", r["knob"]):
            continue
        a = algos.setdefault(r["context"], {"rows": [[None] * 4, [None] * 4]})
        a["rows"]["KS".index(r["knob"][0])][int(r["knob"][1]) - 1] = dict(label=r["label"], at=r["_at"], **parse_value(r["range_default"]))
    for n, a in algos.items():
        a["fw"] = firmware_row(n)
        a["short"] = (a["fw"] or {}).get("short") or shorts.get(n, n)
        for row in a["rows"]:
            for k in row:
                if k and (n, k["label"]) in HOOK_TEXTS:
                    k["hook"] = HOOK_TEXTS[(n, k["label"])]
    for n in algos:
        if order and n not in order:
            probs.append("knob_map.tsv: PERF context %r is not an SQ_ALGO_ name" % n)
    return algos, [n for n in order if n in algos] + [n for n in algos if n not in order], probs


def value_strings(k, zero=True):
    """every text the tile can show for knob k"""
    if k.get("hook"):
        return k["hook"](k["min"], k["max"])
    out = []
    for v in range(k["min"], k["max"] + 1):
        if v == 0 and k["zero"]:
            if zero:
                out.append(k["zero"])
        elif k["names"]:
            out.append(k["names"][v - k["min"]])
        elif k["fmt"]:
            out.append(k["fmt"] % v)
        else:
            out.append(str(v))
    return out


def load_messages():
    import ast
    try:
        tree = ast.parse(open(MOCK_V12, encoding="utf-8").read())
    except OSError:
        return None
    for node in tree.body:
        if isinstance(node, ast.Assign) and any(getattr(t, "id", "") == "MESSAGES" for t in node.targets):
            return ast.literal_eval(node.value)
    return None


def probe_measure(font, strings):
    """{string: px} in AF_<font> by the real text_w (tests/design/width_probe.sh)"""
    strings = sorted({x for x in strings if x})
    if not strings:
        return {}
    lines = "\n".join("%s\t9999\t%s\tm" % (font, x) for x in strings) + "\n"
    p = subprocess.run(["sh", PROBE, "-"], input=lines, text=True, capture_output=True)
    W = {}
    for l in p.stdout.splitlines():
        m = re.match(r"\s*(\d+)\s*/\s*\d+\s+(OK|OVER)\s+`(.*)`\s", l)
        if m:
            W[m.group(3)] = int(m.group(1))
    miss = [x for x in strings if x not in W]
    if miss:
        raise RuntimeError("width probe did not measure %r: %s" % (miss[:3], (p.stderr or p.stdout).strip()[:200]))
    return W


def algo_check(algos, order, messages, measure=probe_measure):
    """the mock's check() over every algorithm row, plus the SCREEN_SPEC_V2 section 5 messages; returns the problems"""
    bad = []
    need = {"S": set(), "M": set(), "L": set()}
    for n in order:
        a = algos[n]
        need["M"].add(a["short"])
        for row in a["rows"]:
            for k in row:
                if k:
                    need["M"].add(k["label"])
                    for t in value_strings(k):
                        need["M"].add(t)
                        need["L"].add(t)
    for kind, x, y in messages or []:
        need["S"].update((x, y)); need["M"].update((x, y))
    W = {f: measure(f, need[f]) for f in need}
    for n in order:
        a = algos[n]
        if len(a["short"]) > SHORT_CHARS or W["M"][a["short"]] > HEADER_PX:
            bad.append("%s: short name %r does not fit the header (%d chars, %d px of %d)" % (n, a["short"], len(a["short"]), W["M"][a["short"]], HEADER_PX))
        for r, row in enumerate(a["rows"]):
            for i, k in enumerate(row):
                slot = "%s%d" % ("KS"[r], i + 1)
                if k is None:
                    bad.append("%s %s: missing (every algorithm fills all 8 knob slots)" % (n, slot))
                    continue
                at = k.get("at", n)
                if len(k["label"]) > SHORT_CHARS or W["M"][k["label"]] > TILE_LABEL_PX:
                    bad.append("%s: %s %s label %r too wide (%d chars, %d px of %d in AF_M)" % (at, n, slot, k["label"], len(k["label"]), W["M"][k["label"]], TILE_LABEL_PX))
                if k["names"] and len(k["names"]) != k["max"] - k["min"] + 1:
                    bad.append("%s: %s %s has %d value names for the range %d..%d" % (at, n, k["label"], len(k["names"]), k["min"], k["max"]))
                vals = value_strings(k)
                wl = max([W["L"][t] for t in value_strings(k, zero=False)] or [0])
                wm = max([W["M"][t] for t in vals] or [0])
                if wl > VALUE_L_PX and wm > TILE_LABEL_PX:
                    bad.append("%s: %s %s widest value %r is %d px in AF_L and %d px in AF_M, the tile holds %d" % (at, n, k["label"], max(vals, key=lambda t: W["M"][t]), wl, wm, TILE_LABEL_PX))
        fw = a.get("fw")
        if fw:
            km_ = fw["knob_morph"] or []
            if len(km_) != 8 or len(fw["morph"]) != 8:
                bad.append("%s: %s row has %d knob_morph / %d morph entries, wants 8 / 8" % (n, fw["file"], len(km_), len(fw["morph"])))
                continue
            if not fw["short"]:
                bad.append("%s: %s row has no .short_name" % (n, fw["file"]))
            for s in range(8):
                k = a["rows"][s // 4][s % 4]
                lab, mn, mx, df, names, unit, zero = fw["morph"][km_[s]]
                if k and (lab, mn, mx, df, names, zero) != (k["label"], k["min"], k["max"], k["def"], k["names"], k["zero"]):
                    bad.append("%s: %s %s (slot %d -> morph %d) disagrees with knob_map.tsv %s: row %r, TSV %r" % (
                        n, fw["file"], lab, s, km_[s], k.get("at", ""), (lab, mn, mx, df, names, zero), (k["label"], k["min"], k["max"], k["def"], k["names"], k["zero"])))
    room = SCREEN_W - MSG_X
    for kind, x, y in messages or []:
        if kind == "enc":
            w = W["S"][x] + 6 + 8 + min(W["M"][y], W["S"][y]) if y else W["S"][x] + 14
        elif kind == "hold":
            w = 22 + min(W["M"][x], W["S"][x])
        else:
            w = 3 + 17 + min(W["M"][x], W["S"][x]) + 6 + (W["S"][y] if y else 0) + 3
        if w > room:
            bad.append("SCREEN_SPEC_V2 message (%s, %r, %r) needs %d px, the slot has %d" % (kind, x, y, w, room))
    return bad


def algo_check_run(km, strict):
    if not os.path.exists(FONTS):
        if strict:
            bad("no build/gen/ui_fonts.h: run ./build.sh once, the algorithm widths are measured on its fonts")
        else:
            sys.stderr.write("gen_ui_tables: note: no build/gen/ui_fonts.h yet, algorithm widths not measured\n")
        return 0
    algos, order, probs = collect_algos(km)
    for p in probs:
        bad(p)
    msgs = load_messages()
    if msgs is None:
        bad("MESSAGES not found in %s" % os.path.relpath(MOCK_V12, ROOT))
    for p in algo_check(algos, order, msgs):
        bad(p)
    return len(order)


def selftest():
    """the check passes on the real rows and FAILS on a deliberately too-wide label, short name, value and message"""
    km = rd("knob_map")
    if err or not os.path.exists(FONTS):
        sys.stderr.write("selftest needs build/gen/ui_fonts.h (./build.sh) and a clean knob_map.tsv\n" + "\n".join(err) + "\n")
        return 1
    import copy
    algos, order, probs = collect_algos(km)
    msgs = load_messages()
    clean = probs + algo_check(algos, order, msgs)
    fails = []
    if clean:
        fails.append("the real rows should pass, got: %s" % clean[:3])
    cases = []
    a = copy.deepcopy(algos); a["ACID"]["rows"][0][0]["label"] = "TOOWIDE"; cases.append(("label of 7 chars", a, msgs, "label 'TOOWIDE' too wide"))
    a = copy.deepcopy(algos); a["ACID"]["rows"][1][3]["label"] = "WWWWWW"; cases.append(("label of 6 wide chars", a, msgs, "label 'WWWWWW' too wide"))
    a = copy.deepcopy(algos); a["ACID"]["short"] = "ABCDEFG"; cases.append(("short name of 7 chars", a, msgs, "short name 'ABCDEFG'"))
    a = copy.deepcopy(algos); a["ACID"]["rows"][0][1]["names"] = ["WWWWWW"] * 3; a["ACID"]["rows"][0][1].update(min=0, max=2, zero=None)
    cases.append(("value too wide in both fonts", a, msgs, "widest value"))
    cases.append(("message too long", algos, list(msgs) + [("toast", "WWWWWWWWWWWW", "EDIT=UNDO")], "needs"))
    a = copy.deepcopy(algos); a["ACID"]["rows"][1][3] = None; cases.append(("missing knob slot", a, msgs, "missing"))
    for what, al, ms, want in cases:
        got = algo_check(al, order, ms)
        if not any(want in g for g in got):
            fails.append("a %s was not caught (wanted %r in %s)" % (what, want, got[:2]))
        else:
            print("gen_ui_tables selftest: %s -> caught: %s" % (what, [g for g in got if want in g][0]))
    if fails:
        sys.stderr.write("gen_ui_tables selftest FAILED:\n  " + "\n  ".join(fails) + "\n")
        return 1
    print("gen_ui_tables selftest: OK (%d algorithms, %d messages; 6 deliberate misfits caught)" % (len(order), len(msgs)))
    return 0


def emit(t):
    L = ["/* generated by tools/gen_ui_tables.py from the docs/design TSVs (P4-010, ADR-0004): do not edit.",
         "   Flash-resident const tables only; ids are enum values, rows keep the order of their TSV. */",
         "#pragma once", "#include <stdint.h>", "#include <stddef.h>", "#include <stdbool.h>", ""]
    pg = enum("UI_PG_", t["pages"], "page ids")
    ct = enum("UI_C_", t["ctls"], "control ids")
    kb = enum("UI_K_", t["knobs"], "knob ids")
    cx = enum("UI_CTX_", t["ctxs"], "knob contexts")
    md = {m: "UI_MODE_" + ident(m) for m in MODES}
    gs = {g: "UI_GEST_" + ident(g) for g in GESTURES}
    for pre, d, cnt in (("page", pg, "UI_PG_COUNT"), ("ctl", ct, "UI_C_COUNT"), ("knob", kb, "UI_K_COUNT"),
                        ("ctx", cx, "UI_CTX_COUNT")):
        L.append("typedef enum {")
        L += ["    %s%s" % (n, " = 0," if i == 0 else ",") for i, n in enumerate(d)]
        L += ["    %s" % cnt, "} ui_%s_t;" % pre, ""]
        L.append("static const char *const ui_%s_names[%s] = {" % (pre, cnt))
        L += ["    %s," % cstr(v) for v in d.values()]
        L += ["};", ""]
    L += ["typedef enum { %s } ui_mode_t;" % ", ".join(md.values()),
          "typedef enum { %s } ui_gest_t;" % ", ".join(gs.values()), ""]

    def table(name, ctype, fields, rows, cnt):
        if not rows:
            bad("%s: no rows" % name)
        L.append("#define %s %d" % (cnt, len(rows)))
        L.append("typedef struct { %s } %s;" % (" ".join(f[1] + ";" for f in fields), ctype))
        L.append("static const %s %s[%s] = {" % (ctype, name, cnt))
        for r in rows:
            L.append("    { %s }," % ", ".join(r))
        L.extend(["};", ""])

    table("ui_page_rows", "ui_page_row_t", [("", "uint8_t id"), ("", "const char *kind"), ("", "const char *cue")],
          [["UI_PG_" + ident(r["state"]), cstr(r["kind"]), cstr(r["cue_strip"])] for r in t["ps"]], "UI_N_PAGE_STATES")
    table("ui_control_rows", "ui_control_row_t",
          [("", "uint8_t ctl"), ("", "uint8_t page"), ("", "bool owns"), ("", "const char *label")],
          [["UI_C_" + ident(r["control"]), "UI_PG_" + ident(r["context"]),
            "true" if r["page_or_owns"] == "owns" else "false", cstr(r["on_screen_label"])] for r in t["cm"]],
          "UI_N_CONTROL_MAP")
    table("ui_button_rows", "ui_button_row_t",
          [("", "uint8_t page"), ("", "uint8_t ctl"), ("", "uint8_t mode"), ("", "const char *label")],
          [["UI_PG_" + ident(r["page"]), "UI_C_" + ident(r["control"]), md[r["mode"]], cstr(r["label"])]
           for r in t["bm"]], "UI_N_BUTTON_MODES")
    table("ui_knob_rows", "ui_knob_row_t",
          [("", "uint8_t page"), ("", "uint8_t ctx"), ("", "uint8_t knob"), ("", "const char *label")],
          [["UI_PG_" + ident(r["page"]), "UI_CTX_" + ident(r["context"]), "UI_K_" + ident(r["knob"]),
            cstr(r["label"])] for r in t["km"]], "UI_N_KNOB_MAP")
    table("ui_key_rows", "ui_key_row_t",
          [("", "uint8_t page"), ("", "uint8_t ctl"), ("", "uint8_t gesture"), ("", "const char *label")],
          [["UI_PG_" + ident(r["page"]), "UI_C_" + ident(r["key"]), gs[r["gesture"]], cstr(r["label"])]
           for r in t["ky"]], "UI_N_KEY_MAP")
    table("ui_help_rows", "ui_help_row_t",
          [("", "uint8_t ctl"), ("", "uint8_t page"), ("", "const char *line1"), ("", "const char *line2")],
          [["UI_C_" + ident(r["control"]), "UI_PG_" + ident(r["context"]), cstr(r["line1"]), cstr(r["line2"])]
           for r in t["ht"]], "UI_N_HELP_TEXT")
    return "\n".join(L)


def main(argv):
    check = "--check" in argv
    if "--selftest" in argv:
        return selftest()
    args = [a for a in argv if not a.startswith("--")]
    out = args[0] if args else os.path.join(ROOT, "build", "gen", "ui_tables.h")
    t = build()
    if not err:
        text = emit(t)
        width_check(t["labels"], check, t["bm"])
        t["n_algo"] = algo_check_run(t["km"], check)
    if err:
        sys.stderr.write("gen_ui_tables: %d error(s)\n" % len(err))
        for e in err[:60]:
            sys.stderr.write("  " + e + "\n")
        return 1
    if check:
        print("gen_ui_tables: OK (%d algorithms, widths measured; %d pages, %d controls, %d knobs; rows: control %d, button %d, knob %d, key %d, help %d)"
              % (t["n_algo"], len(t["pages"]), len(t["ctls"]), len(t["knobs"]), len(t["cm"]), len(t["bm"]), len(t["km"]),
                 len(t["ky"]), len(t["ht"])))
        return 0
    os.makedirs(os.path.dirname(out), exist_ok=True)
    old = open(out, encoding="utf-8").read() if os.path.exists(out) else None
    if old != text + "\n":
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write(text + "\n")
    print("gen_ui_tables: wrote %s" % os.path.relpath(out, ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
