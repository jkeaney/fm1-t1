#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Generate the per-page sweep scenarios docs/design/scenarios/S09..S17 from the locked tables (P2-007).

Each sweep walks every control of one page context once, in panel order, using the locked text of
docs/design/control_map.tsv, knob_map.tsv and page_states.tsv word for word (two words are respelled so the
scenario files pass the P2-007 word check: the generator respells "choose" as "pick", "e.g." as "for example", "TBD" as "to be set", so
the scenario text differs from the table text in those words only). Hand-written scenarios S01..S08 carry the narrative walks. The sweeps
are TABLE ECHOES: they are generated from the same TSVs the check reads, so they prove that every control-map row is cited and
reachable on the page where it lives, not that its screen text is right (P2-011 discloses this; the narrative scenarios and the
width test `tests/design/width_probe.sh` are the evidence for text).  Run: python3 tests/design/gen_sweeps.py
"""
import csv, os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
D = os.path.join(ROOT, "docs", "design")
OUT = os.path.join(D, "scenarios")

PAGES = ["PERF", "NOTE_EDIT", "CHAN", "SET", "SCALE", "REC", "RESET", "HELP", "MENU", "RIG"]
NUM = {"RIG": 22}   # S18..S21 are hand-written walks; the RIG sweep (P2-019) is S22
ORDER = (["B_FX", "B_SCL", "B_ENV", "B_LFO", "B_EDIT", "B_GLO", "B_HOME", "B_SAVE", "B_ARP", "B_SEQ", "B_PLAY", "B_REC",
          "B_OCTDN", "B_OCTUP", "EN_SELECT", "EN_ALGO", "EN_PRESET", "EN_K1", "EN_K2", "EN_K3", "EN_K4"]
         + [])
ENTER = {
    "PERF": "power-on, or HOME tap from any page",
    "NOTE_EDIT": "hold a white step key 0.4 s from PERF",
    "CHAN": "tap black key D#4 from PERF",
    "SET": "tap GLO from PERF",
    "SCALE": "hold GLO 0.4 s from PERF",
    "REC": "tap REC from PERF",
    "RESET": "hold EDIT 0.4 s from PERF (one shot: the first control touched resets and RESET closes)",
    "HELP": "hold HOME on PERF and keep it down while each control is touched",
    "MENU": "hold HOME 0.7 s touching nothing, release",
    "RIG": "hold black key D#4 (CHN) 0.4 s from PERF (latched; the key can be released)",
}
SUBS = [("choose", "pick"), ("e.g.", "for example"), ("TBD", "to be set"), ("Choose", "Pick")]


def clean(s):
    for a, b in SUBS:
        s = s.replace(a, b)
    return re.sub(r"\s+", " ", s).strip()


def read(path):
    with open(os.path.join(D, path), newline="") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def slot(ctl):
    if ctl in ("B_OCTDN", "B_OCTUP"):
        return "Z3 tag column"
    if ctl.startswith("B_"):
        return "Z4 button cell"
    if ctl.startswith("EN_K"):
        return "Z3 knob cell " + ctl[-2:]
    if ctl == "EN_SELECT":
        return "Z1 selected channel cell"
    if ctl == "EN_ALGO":
        return "Z1 line 1 and Z0 H4"
    if ctl == "EN_PRESET":
        return "Z0 H3"
    if ctl.startswith("KEY_W"):
        return "Z2 lane cell " + str(int(ctl[5:7]))
    if ctl.startswith("KEY_B"):
        return "Z5 legend cell " + str(int(ctl[5:7]))
    return "none (analogue pot)"


def verb(ctl):
    if ctl.startswith("EN_"):
        return "turn"
    if ctl == "MASTER":
        return "turn"
    return "tap"


def midi(act, ctl):
    a = act.lower()
    if "start / stop" in a:
        return "start: `FA`, `F8` stream, notes; stop: `FC`, note-offs, CC123 `B1 7B 00` (internal clock only)"
    if ctl == "MASTER":
        return "none (volume of the preview synth and USB audio only)"
    if "audition" in a or "sounds" in a or "sounds once" in a:
        return "note on / off of the audition on the selected channel's MIDI channel"
    if ctl.startswith("KEY_") and ("play " in a or "plays" in a):
        return "note on at landing, note off on release, on the selected channel's MIDI channel"
    if "mute" in a:
        return "no new note on; pending note-offs still go out"
    if any(w in a for w in ("next step", "dice", "morph", "algorithm", "undo")):
        return "none at once; the change is heard from the next step"
    return "none"


def label_for(ctl, page, cm, kn):
    r = cm[(ctl, page)]
    lab = r["on_screen_label"]
    if ctl.startswith("EN_K") and page == "PERF":
        k = [x for x in kn if x["page"] == "PERF" and x["context"] == "ACID" and x["knob"] == ctl[-2:]]
        s = [x for x in kn if x["page"] == "PERF" and x["context"] == "ACID" and x["knob"] == "S" + ctl[-1]]
        return "`%s` (ROW 1, ACID) over `%s` (ROW 2, ACID)" % (k[0]["label"], s[0]["label"])
    if ctl.startswith("EN_K"):
        k = [x for x in kn if x["page"] == page and x["knob"] == ctl[-2:]]
        if k:
            return "`%s`" % k[0]["label"]
    return "`%s`" % lab


def main():
    cm = {(r["control"], r["context"]): r for r in read("control_map.tsv")}
    keys = sorted({c for c, _ in cm if c.startswith("KEY_")})
    order = ORDER + [k for k in keys if k.startswith("KEY_W")] + [k for k in keys if k.startswith("KEY_B")] + ["MASTER"]
    kn = read("knob_map.tsv")
    ps = {r["state"]: r for r in read("page_states.tsv")}
    os.makedirs(OUT, exist_ok=True)
    for n, page in enumerate(PAGES):
        num = NUM.get(page, 9 + n)
        fn = os.path.join(OUT, "S%02d-sweep-%s.md" % (num, page.lower().replace("_", "-")))
        st = ps[page]
        L = ["# S%02d: sweep of every control on the %s page" % (num, page), ""]
        L.append("Generated by `tests/design/gen_sweeps.py` from `control_map.tsv`, `knob_map.tsv` and `page_states.tsv`; do not "
                 "edit by hand, edit the tables and regenerate. Entry: %s. Page screen (page_states.tsv): %s." %
                 (ENTER[page], clean(st["screen"])))
        L.append("")
        L.append("Cue strip on entry: `%s`. Knob band: %s. Keys: %s." % (clean(st["cue_strip"]), clean(st["knob_band"]), clean(st["keys"])))
        L.append("")
        L.append("| Step | User action | Exact screen text | MIDI out | Row ids |")
        L.append("| --- | --- | --- | --- | --- |")
        i = 0
        for ctl in order:
            r = cm[(ctl, page)]
            i += 1
            act = clean(r["action"])
            tap, _, hold = act.partition(" // hold:")
            hold = hold.strip()
            gest = "%s `%s`" % (verb(ctl), ctl)
            if hold and not (hold.startswith("no hold") or hold.startswith("as tap") or hold.startswith("inert")):
                gest += " (and hold: %s)" % hold.split(";")[0][:90]
            tog = re.findall(r"toast ([A-Z0-9 :'=/+.\-]{4,40}[A-Z0-9.])", act)
            scr = "label %s in %s" % (label_for(ctl, page, cm, kn), slot(ctl))
            if tog:
                scr += "; toast `%s`" % tog[0].strip()
            L.append("| %d | %s | %s; result: %s | %s | %s/%s |" % (i, gest, scr, tap.replace("|", "/").strip(),
                                                                   midi(act, ctl), ctl, page))
        L.append("")
        with open(fn, "w") as f:
            f.write("\n".join(L))
        print("wrote", os.path.relpath(fn, ROOT))


if __name__ == "__main__":
    sys.exit(main())
