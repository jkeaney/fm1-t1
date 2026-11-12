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
Label width: every label must fit the 51 px knob/button cell (SCREEN_SPEC 2.4, the "knob cell value / label" rows of
widths.tsv), measured with the real text_w (tests/design/width_probe.sh, needs build/gen/ui_fonts.h from ./build.sh);
without fonts the pixel step is skipped with a note (--check insists on it).

Usage: gen_ui_tables.py [OUT.h]   (default build/gen/ui_tables.h)    gen_ui_tables.py --check
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


def width_check(labels, strict):
    if not os.path.exists(FONTS):
        if strict:
            bad("no build/gen/ui_fonts.h: run ./build.sh once, the label widths are measured on its fonts")
        else:
            sys.stderr.write("gen_ui_tables: note: no build/gen/ui_fonts.h yet, label widths not measured\n")
        return
    lines = width_lines(["S\t%d\t%s\tlabel" % (LABEL_PX, s) for s in sorted(labels)])
    p = subprocess.run(["sh", PROBE, "-"], input="\n".join(lines) + "\n", text=True, capture_output=True)
    if p.returncode != 0:
        for l in p.stdout.splitlines():
            if " OVER " in l:
                bad("label/string over its width: " + l.strip())
        if p.returncode != 1 or not any(" OVER " in l for l in p.stdout.splitlines()):
            bad("width probe failed (%d): %s" % (p.returncode, (p.stderr or p.stdout).strip()))


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
    labels = set()
    for rows in (bm, km, ky):
        for r in rows:
            if not r["label"].strip():
                bad("%s: empty label" % r["_at"])
            labels.add(r["label"])
    return dict(ps=ps, cm=cm, bm=bm, km=km, ky=ky, ht=ht, pages=pages, ctls=ctls, knobs=knobs, ctxs=ctxs,
                labels=labels)


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

    table("ui_page_rows", "ui_page_row_t", [("", "uint8_t id"), ("", "const char *kind")],
          [["UI_PG_" + ident(r["state"]), cstr(r["kind"])] for r in t["ps"]], "UI_N_PAGE_STATES")
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
    args = [a for a in argv if not a.startswith("--")]
    out = args[0] if args else os.path.join(ROOT, "build", "gen", "ui_tables.h")
    t = build()
    if not err:
        text = emit(t)
        width_check(t["labels"], check)
    if err:
        sys.stderr.write("gen_ui_tables: %d error(s)\n" % len(err))
        for e in err[:60]:
            sys.stderr.write("  " + e + "\n")
        return 1
    if check:
        print("gen_ui_tables: OK (%d pages, %d controls, %d knobs; rows: control %d, button %d, knob %d, key %d, help %d)"
              % (len(t["pages"]), len(t["ctls"]), len(t["knobs"]), len(t["cm"]), len(t["bm"]), len(t["km"]),
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
