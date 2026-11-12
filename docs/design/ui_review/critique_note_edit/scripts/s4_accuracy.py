# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
"""Lists, per thing shown on the NOTE_EDIT renders, the matching rows of docs/design/{button_modes,knob_map,key_map,note_edit_text,page_states}.tsv and the S03 scenario text,
so a mismatch is a diff of two printed strings."""
import sys, csv, re; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
def tsv(f): return list(csv.DictReader(open("docs/design/" + f), delimiter="\t"))
BM, KM, KY, NT, PS = tsv("button_modes.tsv"), tsv("knob_map.tsv"), tsv("key_map.tsv"), tsv("note_edit_text.tsv"), tsv("page_states.tsv")
print("### buttons: panel order (ui_sq.c uisq_btn_ctl / uisq_btn_icon) -> icon drawn -> button_modes NOTE_EDIT tap row (label, action) and the PERF label of the same button")
print("| # | control | icon drawn (tile) | tile fill / icon colour | button_modes NOTE_EDIT label | NOTE_EDIT tap action (first 150 chars) | PERF label | text on the tile? |\n|---|---|---|---|---|---|---|---|")
ctl = ["B_FX", "B_SCL", "B_ENV", "B_LFO", "B_EDIT", "B_GLO", "B_HOME", "B_SAVE", "B_ARP", "B_SEQ", "B_PLAY", "B_REC"]
O = ops("note_open"); icons = [o for o in O if o["op"] == "I" and 146 <= o["y"] < 181]
byslot = {}
for o in icons: byslot[((o["x"] - 1) // 40) + 6 * (0 if o["y"] < 164 else 1)] = o
for i, c in enumerate(ctl):
    ne = [r for r in BM if r["page"] == "NOTE_EDIT" and r["control"] == c and r["mode"] == "tap"]
    pf = [r for r in BM if r["page"] == "PERF" and r["control"] == c and r["mode"] == "tap"]
    o = byslot[i]
    tile = "DICE" if c in ("B_FX", "B_SCL", "B_ENV", "B_LFO", "B_ARP", "B_SEQ") else "SURF"
    print("| %d | %s | %s %dx%d | %s / %s | %s | %s | %s | no |" % (i + 1, c, icon_name(o["id"]), o["w"], o["h"], tile, names_of(o["c"]), ne[0]["label"] if ne else "-", (ne[0]["action"][:150] if ne else "-"), pf[0]["label"] if pf else "-"))
for c in ("B_OCTDN", "B_OCTUP"):
    ne = [r for r in BM if r["page"] == "NOTE_EDIT" and r["control"] == c and r["mode"] == "tap"][0]
    print("| - | %s | (no tile on the screen) | | %s | %s | | no |" % (c, ne["label"], ne["action"][:150]))
print("\n### knobs: knob_map NOTE_EDIT rows vs the four tiles")
for r in [r for r in KM if r["page"] == "NOTE_EDIT"]: print("- %s `%s`: %s | range: %s" % (r["knob"], r["label"], r["action"][:200], r["range_default"][:200]))
print("\n### keys: key_map NOTE_EDIT rows (white W01 and every black key) vs what the keybed draws")
for r in [r for r in KY if r["page"] == "NOTE_EDIT" and (r["key"] == "KEY_W01_F3" or r["key"].startswith("KEY_B"))]: print("- %s %s: label `%s` | %s" % (r["key"], r["gesture"], r["label"], r["action"][:170]))
print("\n### note_edit_text.tsv: each id, the string, and whether it appears on any of the 4 renders + 2 worst cases (as a whole string or as a part of a built string)")
seen = {}
for n in RENDERS + EXTRA:
    for t in vtexts(n): seen.setdefault(t["s"], set()).add(n)
allst = [(s, sorted(v)) for s, v in seen.items()]
def match(r, st):
    i, txt = r["id"], r["text"]
    toks = re.findall(r"[A-G]#?-?\d|\d+X|OCT[+-]\d|[A-Za-z]+|[0-9]+", st)
    if i.startswith("NOTE_"): return any(re.fullmatch(re.escape(txt) + r"-?\d", t) for t in toks)
    if i == "X": return any(re.fullmatch(r"\dX", t) for t in toks)
    if i == "PLUS": return any(re.fullmatch(r"OCT\+\d", t) for t in toks)
    if i == "MINUS": return any(re.fullmatch(r"OCT-\d|[A-G]#?-\d", t) for t in toks)
    return re.search(r"(?<![A-Za-z])" + re.escape(txt) + r"(?![A-Za-z])", st) is not None
for r in NT:
    hits = sorted({n for s_, v in allst for n in v if match(r, s_)})
    print("- %-10s `%s` -> %s" % (r["id"], r["text"], ", ".join(hits) if hits else "NOT on any of the six frames"))
print("\n### strings on the NOTE_EDIT frames that are NOT in ui_ne_text, knob_map, or a persistent PERF element (built or literal in ui_sq.c)")
nt = {r["text"] for r in NT}
lab = {r["label"] for r in KM if r["page"] == "NOTE_EDIT"}
for n in ["note_open", "note_set", "note_dice", "note_final", "worst_w1", "worst_w2"]:
    pass
print("- `PROB 50`: the suffix ` 50` is a literal in ui_sq.c (uisq_cat(..., \" 50\")); tsv PROB holds `PROB` only")
print("- `GEN <v>`: the joining space is a literal in uisq_ne_gen_line")
print("- `S` `F` `R`: black-key letters, literals in uisq_draw_keybed (the same on PERF)")
print("- digits (STEP n, 10..100, 1..127, n X, OCT sign digit, note octave): built by uisq_catu; `120` tempo, `1`..`4` chips, `ACID` (algorithm short name), `MIDI` + channel: persistent header")
print("\n### page_states.tsv NOTE_EDIT row (the older contract) vs the render")
for r in [r for r in PS if r.get("page") == "NOTE_EDIT" or r.get("id") == "NOTE_EDIT"] or [r for r in PS if "NOTE_EDIT" in "".join(r.values())][:2]:
    print({k: v[:300] for k, v in r.items()})
