# SPDX-License-Identifier: GPL-3.0-only
"""v12_export.py (P4-028): the scenes of the v12 screen mock as C data, for tests/ui_sq_v12_test.c.

   python3 tests/v12_export.py build/gen/v12_scenes.h

The mock (docs/design/ui_review/proposals/v12/v12.py) is the reference implementation of SCREEN_SPEC_V2. This script runs its own scene
definitions through the same derivations the mock's draw functions make (defaults, the knob row of the mode, the playhead, the used mask ...)
and writes, per scene, the UisqView fields the firmware element code draws from, so the host test can draw the same scene with ui_sq.c and compare
pixels. It also writes the algorithm rows of the mock (knob_map.tsv, by knob slot) as MorphDesc tables: ACID is the real firmware row in the test,
the others are these. Nothing here is firmware."""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
V12 = os.path.join(HERE, "..", "docs", "design", "ui_review", "proposals", "v12")
sys.path[:0] = [os.path.join(V12, "..", "..", "mock"), V12]
import mock                                  # noqa: E402
mock.palette_set("HI-CON")
import v12                                   # noqa: E402
import algos_v12 as A                        # noqa: E402

ALGO_ORDER = ["ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS", "DEMO"]   # index 6 = DEMO (the test swaps it into slot 5)
BAND = {"row": 0, "rec": 1, "chan": 2}   # UISQ_BAND_ROW / REC / CHAN of ui_sq.c
CLK = {"INT": 0, "EXT": 1, "WAIT": 2, "LOST": 3}


def cstr(s):
    return "0" if s is None else '"%s"' % s


def emit_algos(out):
    out.append("/* the mock's algorithm rows by knob slot (K1..K4 = 0..3, S1..S4 = 4..7); ACID is the firmware's own row in the test */")
    for ai, name in enumerate(ALGO_ORDER):
        a = A.ALGOS[name]
        for s in range(8):
            d = a["rows"][s // 4][s % 4]
            if d["names"]:
                out.append("static const char *const V12_N_%d_%d[%d] = { %s };" % (ai, s, len(d["names"]), ", ".join('"%s"' % n for n in d["names"])))
    out.append("static const MorphDesc V12_MORPH[7][8] = {")
    for ai, name in enumerate(ALGO_ORDER):
        a = A.ALGOS[name]
        out.append("  [%d] = {   /* %s */" % (ai, name))
        for s in range(8):
            d = a["rows"][s // 4][s % 4]
            names = "V12_N_%d_%d" % (ai, s) if d["names"] else "0"
            unit = None
            if d["fmt"] and d["fmt"].startswith("%d "):
                unit = d["fmt"][3:]
            out.append('    { "%s", %d, %d, %d, 1, 1, %s, %s, %s },' % (d["label"], d["min"], d["max"], d["def"], names, cstr(unit), cstr(d["zero"])))
        out.append("  },")
    out.append("};")
    out.append("static const char *const V12_SHORT[7] = { %s };" % ", ".join('"%s"' % A.ALGOS[n]["short"] for n in ALGO_ORDER))
    out.append("static const uint8_t V12_SINGLE[7] = { %s };" % ", ".join("1" if A.ALGOS[n]["lane"] == "single" else "0" for n in ALGO_ORDER))
    pick = [d["label"] for d in A.ALGOS["PATTERNS"]["rows"][1]].index("PICK") + 4
    total = [d["label"] for d in A.ALGOS["STAGE"]["rows"][0]].index("TOTAL")
    out.append("#define V12_PICK_SLOT  %d   /* PATTERNS: the slot whose text is tag + number (value_text hook in the test) */" % pick)
    out.append("#define V12_TOTAL_SLOT %d   /* STAGE: TOTAL shows `48(32)` when the pattern has played fewer */" % total)
    out.append("static const char *const V12_PATT_TAGS[6] = { %s };" % ", ".join('"%s"' % t for t in A.PATT_TAGS))
    rig = v12.DEVICES["TB-3 BITE"]
    out.append("static const MorphDesc V12_RIG[4] = { %s };" % ", ".join('{ "%s", 0, 127, %d, 1, 1 }' % (n, v) for n, v in rig))


def scene_fields(name, S):
    sel = S["sel"]
    chans = S["chans"]
    rec = bool(S.get("rec"))
    chan_page = S.get("page") == "CHAN"
    band = "rec" if rec else "chan" if chan_page else "row"
    row = S.get("row", 0)
    rigged = bool(S.get("rig"))
    krow = v12.knob_row(S)
    vals = S.get("vals", {})
    val = []
    for k in range(4):
        d = krow[k]
        v = vals.get(k, d["def"])
        if band == "rec" and d["min"] < 0:
            v -= d["min"]                      # the firmware stores OCT -2..+2 as 0..4
        val.append(v)
    tr = S.get("turning")
    playing = S.get("playing", True)
    ph = S.get("playhead", 5) if playing and not S.get("follow_off") else 255
    used = sum(1 << i for i in S["used"])
    msg = bool(S.get("toast") or S.get("hold") or chan_page)
    f = dict(name=name, msg=int(msg), playing=int(playing), rec=int(rec), clock=CLK[S.get("clock", "INT")], bpm=S.get("bpm", 120),
             used=used, sel=sel, band=BAND[band], row=row, nrow=3 if rigged else 2, val=val,
             turn=(tr[0] + 1) if tr else 0, turn_old=tr[1] if tr else 0,
             preset=S.get("preset", 1), pend=S.get("pend", 0), pages=S.get("pages", 1), vpage=S.get("vpage", 1), ppage=S.get("ppage", 2),
             follow_off=int(bool(S.get("follow_off"))), ph=ph, len16=S.get("length") if S.get("length") is not None else 16,
             cursor=S.get("cursor", 1), diced=sum(1 << i for i in S.get("diced", ())),
             heard=(S.get("algo_state") or {}).get("heard", 255))
    return f, chans


def emit_scene(out, f, chans):
    algo = [ALGO_ORDER.index(c["algo"]) for c in chans]
    midi = [16 if c["midi"] is None else c["midi"] - 1 for c in chans]
    muted = [int(c["muted"]) for c in chans]
    lanes = []
    for c in chans:
        lanes.append("{ %s }" % ", ".join("{ %d, %d, %d }" % ((s["p"], int(s["acc"]), int(s["sl"])) if s else (-1, 0, 0)) for s in c["lane"]))
    d = dict(f, algo=algo, midi=midi, muted=muted)
    fields = ['.name = "%s"' % f["name"]]
    for k in ("msg", "playing", "rec", "clock", "bpm", "sel", "row", "nrow", "turn", "turn_old", "preset", "pend", "pages", "vpage", "ppage",
              "follow_off", "ph", "len16", "cursor", "heard"):
        fields.append(".%s = %d" % (k, f[k]))
    fields.append(".used = 0x%X" % f["used"])
    fields.append(".diced = 0x%X" % f["diced"])
    fields.append(".band = %d" % f["band"])
    for k in ("algo", "midi", "muted"):
        fields.append(".%s = { %s }" % (k, ", ".join(map(str, d[k]))))
    fields.append(".val = { %s }" % ", ".join(map(str, f["val"])))
    fields.append(".lane = { %s }" % ", ".join(lanes))
    out.append("  { %s }," % ", ".join(fields))


def main(path):
    out = ["/* SPDX-License-Identifier: GPL-3.0-only */", "/* generated by tests/v12_export.py from docs/design/ui_review/proposals/v12/v12.py: do not edit */"]
    emit_algos(out)
    out.append("typedef struct { int8_t note; uint8_t acc, sl; } V12Step;")
    out.append("typedef struct { const char *name; uint8_t msg, playing, rec, clock; uint16_t bpm; uint8_t used, sel; uint8_t algo[4], midi[4], muted[4];")
    out.append("    uint8_t band, row, nrow, val[4], turn, turn_old, preset, pend, pages, vpage, ppage, follow_off, ph, len16, cursor, heard; uint16_t diced; V12Step lane[4][16]; } V12Scene;")
    out.append("static const V12Scene V12_SCENES[] = {")
    n = 0
    sheets = []
    for name, S in v12.STATE["scenes"].items():
        if S.get("sheet"):
            sheets.append((name, S))
            continue
        f, chans = scene_fields(name, S)
        emit_scene(out, f, chans)
        n += 1
    out.append("};")
    out.append("#define V12_NSCENES %d" % n)
    out.append("typedef struct { const char *name; uint8_t row, n, algo[4]; } V12Sheet;")
    out.append("static const V12Sheet V12_SHEETS[] = {")
    for name, S in sheets:
        al = [ALGO_ORDER.index(a) for a in S["algos"]]
        out.append('  { "%s", %d, %d, { %s } },' % (name, S["row"], len(al), ", ".join(map(str, al + [0] * (4 - len(al))))))
    out.append("};")
    out.append("#define V12_NSHEETS %d" % len(sheets))
    with open(path, "w") as fh:
        fh.write("\n".join(out) + "\n")
    print("v12_export: %d scenes, %d sheets -> %s" % (n, len(sheets), path))


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "build/gen/v12_scenes.h")
