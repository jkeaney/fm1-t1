import sys, collections; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
variant("mock")
out = []; P = out.append
P("## 2b. non-text marks per scene (drawn ops by caller; hidden-by-overpaint ops not removed; art = bitmap icon draws)")
P("| scene | art icons (button / header / key / toast / other) | key-role glyph marks on black keys (squares / 2x2 dots / letters / arrows+rst) | row-marker squares | channel chips (rect+frame) | knob tiles | music bars (rect/frame, h>=4) | rest dashes | hatch regions | lit white-key fronts |")
P("|---|---|---|---|---|---|---|---|---|---|")
for sn in SCENES:
    r = capture(S()[sn]); ops = r["ops"]; t = r["texts"]
    arts = [o for o in ops if o["k"] == "art"]
    btn = sum(1 for o in arts if o["fn"] == "buttons7"); hdr = sum(1 for o in arts if o["fn"] == "enc_row"); key = sum(1 for o in arts if o["fn"] == "footer_keybed")
    toast = sum(1 for o in arts if o["fn"] == "toast_top"); oth = len(arts) - btn - hdr - key - toast
    fk = [o for o in ops if o["fn"] == "footer_keybed"]
    sq = sum(1 for o in fk if o["k"] in ("rect", "frame") and o["w"] == 6 and o["h"] == 6)
    dots = sum(1 for o in fk if o["k"] == "rect" and o["w"] == 2 and o["h"] == 2)
    letters = sum(1 for x in t if x["fn"] == "footer_keybed" and x["s"] in "SFR" and len(x["s"]) == 1)
    arrows = sum(1 for o in fk if o["k"] == "art" and o["w"] == 5)
    rm = sum(1 for o in ops if o["fn"] == "row_marker")
    chips = sum(1 for o in ops if o["fn"] == "enc_row" and o["k"] in ("rect", "frame") and o["w"] == 13)
    tiles = sum(1 for o in ops if o["fn"] in ("knobs_fill", "knobs_num") and o["k"] == "rect" and o["w"] == 58 and o["h"] == 52)
    bars = sum(1 for o in ops if o["fn"] == "music_slim" and o["k"] in ("rect", "frame") and 90 <= o["y"] < 124 and o["h"] >= 4 and o["w"] < 15)
    dash = sum(1 for o in ops if o["fn"] == "music_slim" and o["k"] == "rect" and o["h"] == 1 and o["w"] == 3)
    hat = sum(1 for o in ops if o["k"] == "hatch")
    lit = sum(1 for o in fk if o["k"] == "rect" and o["w"] == 14 and o["h"] == 12)
    P("| %s | %d (%d/%d/%d/%d/%d) | %d / %d / %d / %d | %d | %d | %d | %d | %d | %d | %d |" % (sn, len(arts), btn, hdr, key, toast, oth, sq, dots, letters, arrows, rm, chips, tiles, bars, dash, hat, lit))
P("")
P("button icon draws: 12 per scene (6 dice-frame icons, undo, settings, home, save, play-or-stop, rec); header glyphs: transport icon + algorithm glyph; black-key glyphs in non-REC scenes: up to 4 mute squares + 1 CHN 4-dot + 3 letters + 3 5x7 icons = 11 keys with a mark; in REC scene: 0 black-key glyphs plus 1 rec icon in the caption line")
# tile margins
r = capture(S()["playing"])
P("")
P("## 2c. text margins inside the 58 px knob tiles (scene playing; tile k spans x=60k+1 .. 60k+58)")
for x in r["texts"]:
    if x["fn"] == "knobs_fill" and x["ink"]:
        k = (x["x"] - 1) // 60 if x["x"] >= 1 else 0
        a, b, c, d = x["ink"]
        # tile index from centre
        k = min(3, (a + c) // 2 // 60); t0 = 60 * k + 1; t1 = t0 + 58
        P("- %r %s ink x %d..%d in tile %d (%d..%d): left margin %d, right margin %d" % (x["s"], x["f"], a, c, k + 1, t0, t1, a - t0, t1 - c))
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s1b_out.md", "w").write("\n".join(out))
print("\n".join(out))
