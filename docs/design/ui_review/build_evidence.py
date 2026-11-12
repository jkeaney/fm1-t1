#!/usr/bin/env python3
"""build_evidence.py: composes EVIDENCE.md from the measured dumps (build/ui_review, written by measure.sh) and the files of this directory.
Facts and measurements only. Run from the repo root:  sh docs/design/ui_review/measure.sh && python3 docs/design/ui_review/analyze.py && python3 docs/design/ui_review/build_evidence.py"""
import csv, collections, re, subprocess, os, sys
RAW = "build/ui_review"; D = "docs/design/ui_review"
ELL = "\x85"
def rd(path, enc="utf-8"): return list(csv.DictReader(open(path, encoding=enc, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
els = rd(D + "/elements.tsv"); texts = rd(RAW + "/texts_all.tsv", "latin-1"); fonts = open(D + "/fonts.tsv", encoding="utf-8").read().splitlines()
trunc = rd(D + "/truncations.tsv")
out = []
def P(s=""): out.append(s)
def disp(s): return s.replace(ELL, "…")
def cell(s): return str(s).replace("|", "\\|")
def strip_sc(c): return re.sub(r"\s*\[[^\]]*\]", "", c)

# ---------------------------------------------------------------- 0
P("# UI review: EVIDENCE (B)")
P()
P("Facts and measurements only: no opinions, no recommendations. Inputs: `firmware/src/ui_sq.c` (P4-011), the generated tables `build/gen/ui_tables.h` and fonts `build/gen/ui_fonts.h`,")
P("`docs/design/SCREEN_SPEC.md`, `SEQUENCER_UX.md`, `page_states.tsv`, `button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, `help_text.tsv`, `control_map.tsv`, `tests/design/widths.tsv`.")
P()
P("## 0. How this was measured")
P()
P("- `measure.sh` compiles the real `gfx.c` (real fonts, real `text_w`, real `cv_text_fit`), the real `ui_sq.c`, `sq_core.c`, `sq_chan.c` and the generated tables on the host, with the test harness of `tests/ui_sq_test.c` (its prefix is reused verbatim), and hooks every text and rectangle that `ui_sq.c` draws (`GFX_HOOK_TEXT`, `GFX_HOOK_INK`, `GFX_HOOK_PEN`, wrappers on `cv_text_fit`, `cv_rect`, `cv_frame`). No firmware or TSV file is edited.")
P("- Scenarios dumped (raw: `build/ui_review/texts_all.tsv`, about 15 000 draw records): PERF in 11 states (default power-on, ROW 2 live, knob hot, test render with length 20, playing, playing with CH2 selected, viewing page 2, toast, algorithm-pick strip, mutes, mute + MIDI OFF), every page x every algorithm id 0..5 on the selected channel (13 pages; MENU is the Felucca menu and draws nothing), and all four channels on every algorithm with MIDI 16.")
P("- Palette: the screenshots in `build/ui_sq/` and all numbers here use palette 0 **GREY** (`panel.c` sets `settings.palette = UI_GREY_INDEX` as the default). Contrast is also computed for all 10 palettes of `ui_palettes.h`.")
P("- Algorithm data: the shipped `ALGOS[]` (`sq_algos.c`) has only row 0 (ACID) filled; rows 1..5 are zero, for which `ui_sq.c` draws no dice sub-label and the raw knob index as the morph slot. The test harness (and the PNGs) use a stand-in `ALGOS` table with names, morph ranges and sub-labels for all six. `measure.sh` replaces the stand-in ACID `knob_morph` (identity) with the shipped `{0,5,2,3,4,1,6,7}` so ACID knob labels and values agree; algorithms 1..5 are the stand-in's.")
P("- Colour roles (`T_TEXT` ...) are assigned from the draw calls in `ui_sq.c` and verified against the RGB565 values the real draw recorded; the sRGB hex uses `r*255/31, g*255/63, b*255/31` as the test's PNG writer does. A `-` or `...` in a state column means the state does not change that element.")
P("- Pixel coordinates are screen pixels (0..239). Text positions are the pen (line top) the code passes; ink rows are measured from the real glyph boxes.")
P("- Files: `EVIDENCE.md` (this), `elements.tsv` (86 rows, one per drawn element), `contrast.py`, `fonts.tsv`, `palettes.tsv`, `text_pairs.tsv`, `truncations.tsv`, `measure.sh` + `measure_body.c` + `mh_decl.h` (instrumentation), `analyze.py`, `build_evidence.py`.")
P()

# ---------------------------------------------------------------- 1 palette & contrast
P("## 1. Palette and contrast")
P()
cp = subprocess.run([sys.executable, D + "/contrast.py"], capture_output=True, text=True, check=True).stdout
P(cp.replace("### ", "#### "))
P()
P("Notes on the contrast numbers:")
P("- In the MONO palette `T_DIM`, `T_RAISE`, `T_LINE` and `T_LANE` are all the same grey (0x528A), so the ratio of T_DIM on T_RAISE reads 1.00; `ramp()` in `gfx.c` substitutes `T_BG` ink when fg == bg (\"MONO: DIM on a RAISE well: black, recessed\"), so the drawn ink is not the 1.00 pair.")
P("- On GREY, `T_BG`, `T_INK` are the same value (0x0841), `T_SURF`/`T_TINT` (0x2104), `T_LINE`/`T_GRID` (0x2945), `T_RAISE`/`T_LANE` (0x39C7) and `T_ACCENT`/`T_REC`/(0xFFDF) coincide. Text drawn on a `T_RAISE` legend well (Z5) and on a `T_LANE` step cell (Z2) is on the same 0x39C7.")
P("- Antialiased text: the glyph ramp runs from the bg to the fg over 16 levels, so the ratios above are the maximum (full-coverage) ratio of each pair; thin strokes of AF_S (stems 1 px wide) never reach full coverage on every pixel.")
P("- Contrast of `T_ACCENT` vs `T_SEL` and `T_TEXT` vs `T_SEL` is not drawn anywhere on the PERF screen (text on a `T_SEL` fill is always `T_INK`).")
P()

# ---------------------------------------------------------------- 2 fonts
P("## 2. Fonts")
P()
P("`ui_sq.c` draws every string with **AF_S** (grep: no `AF_M`, no `AF_L` in the file). AF_M and AF_L are listed because SCREEN_SPEC 1 and the spec table name them (\"AF_M is used only for one-word banners\"); `ui_sq.c` does not use them. Data: `build/gen/ui_fonts.h` (generated by `tools/gen_aa_font.py`, Inter Tight, 4-bit alpha, 4 horizontal phases for S and M, 2 for L); measured by `dump_font()` in `measure_body.c` from the real glyph tables; `fonts.tsv` is the full dump.")
P()
def fl(font, key):
    for l in fonts:
        if l.startswith(key + "\t" + font + "\t"): return l.split("\t")
    return None
def samp(font, ch):
    for l in fonts:
        t = l.split("\t")
        if t[0] == "SAMPLE" and t[1] == font and t[2] == ch: return dict(kv.split("=") for kv in t[3:] if "=" in kv)
    return None
P("| Property | AF_S | AF_M | AF_L |\n| --- | --- | --- | --- |")
def fv(font, k):
    t = fl(font, "FONT"); return [x.split("=")[1] for x in t if x.startswith(k + "=")][0]
P("| Nominal size (spec / header comment) | 12 px (\"12 px line\") | 15 px | 28 px |")
P("| Line box height `h` (struct field) | %s px | %s px | %s px |" % (fv("S", "line_h"), fv("M", "line_h"), fv("L", "line_h")))
P("| Baseline (`asc`) | row %s | row %s | row %s |" % (fv("S", "baseline"), fv("M", "baseline"), fv("L", "baseline")))
def ink(font, ch, key):
    s = samp(font, ch); return s[key] if s else "-"
def inkspan(font, ch):
    s = samp(font, ch); return "%s px (rows %s..%s)" % (s["ink_h"], s["ink_top"], s["ink_bot"]) if s else "absent"
P("| Capital ink height (H) | %s | %s | %s |" % (inkspan("S", "H"), inkspan("M", "H"), inkspan("L", "H")))
P("| Lowercase x-height (x) | %s | %s | absent (lowercase folded to capitals) |" % (inkspan("S", "x"), inkspan("M", "x")))
P("| Ascender ink (d / l) | %s / %s | %s / %s | absent |" % (inkspan("S", "d"), inkspan("S", "l"), inkspan("M", "d"), inkspan("M", "l")))
P("| Descender ink (g / p / y) | %s / %s / %s | %s / %s / %s | absent |" % (inkspan("S", "g"), inkspan("S", "p"), inkspan("S", "y"), inkspan("M", "g"), inkspan("M", "p"), inkspan("M", "y")))
P("| Digit ink height (0 / 1) | %s / %s | %s / %s | %s / %s |" % (inkspan("S", "0"), inkspan("S", "1"), inkspan("M", "0"), inkspan("M", "1"), inkspan("L", "0"), inkspan("L", "1")))
P("| Small marks: `.` / `'` / `~` / `-` ink height | %s / %s / %s / %s px | %s / %s / %s / %s px | `.` %s px |" % (ink("S", ".", "ink_h"), ink("S", "'", "ink_h"), ink("S", "~", "ink_h"), ink("S", "-", "ink_h"), ink("M", ".", "ink_h"), ink("M", "'", "ink_h"), ink("M", "~", "ink_h"), ink("M", "-", "ink_h"), ink("L", ".", "ink_h")))
def mean(font, rng):
    for l in fonts:
        t = l.split("\t")
        if t[0] == "MEAN_ADV" and t[1] == font and t[2] == rng: return t[3]
P("| Mean advance: A-Z / a-z / 0-9 | %s / %s / %s px | %s / %s / %s px | %s / - / %s px |" % (mean("S", "A-Z"), mean("S", "a-z"), mean("S", "0-9"), mean("M", "A-Z"), mean("M", "a-z"), mean("M", "0-9"), mean("L", "A-Z"), mean("L", "0-9")))
def g(font): return fl(font, "GLYPHS")
P("| Widest advance (ASCII 33..126), mean advance of all non-space glyphs | %s, %s | %s, %s | n/a (digits 18) |" % (g("S")[3].split("=")[1], g("S")[4].split("=")[1] + " px", g("M")[3].split("=")[1], g("M")[4].split("=")[1] + " px"))
P("| Typical glyph widths (advance, px) | `0` 7, `W` 11, `i` 2 | `0` 9, `W` 14, `i` 3 | `0` 18, `W` 27, `i` 7 |")
P("| Real `text_w` samples | `ACCENT` 46, `HELLO WORLD` 79 | `ACCENT` 58, `HELLO WORLD` 100 | `ACCENT` 111 (no lowercase), `HELLO WORLD` 190 |")
P("| Lowercase exists | yes (a-z, 26 glyphs) | yes | **no**: `fold()` maps a-z to A-Z |")
P("| Characters present | ASCII 32..126 (95 glyphs) + extras 169 176 183 196 214 220 228 246 252 and 133 (the ellipsis `…`, the glyph `text_fit` appends) | same set | space (32) + extras: `# + - . /`, `0-9`, `A`..`Y` (no `Z`, no `,` `:` `;` `'` `!` `~` `<` `>` `=` `(` `)` `%` `[` `]` `*`) |")
P("| Kerning pairs | 919 | 992 | 123 |")
P("| Cap band (macro) | `AF_S_CAP_Y 3`, `AF_S_CAP_H 9` | 4, 11 | 7, 21 |")
P()
P("Sizes in use on the PERF screen (not font faces): clock dot 8x8, hold mark 5x7, activity dot 4x4, page-map cells 9x11, lane step cell 14x46 (bar 12 px wide, 2..32 px high, glyph row 14 px), separator 1x20. The smallest text ink in use is AF_S: capitals/digits 9..10 px high, lowercase x-height 7 px, the punctuation glyphs `.` (2 px), `'` (4 px), `~` (3 px), `-` (2 px).")
P()

# ---------------------------------------------------------------- 3 screen map
P("## 3. Screen map of the PERF page")
P()
P("### 3.1 Zones (SCREEN_SPEC 2; constants `UISQ_Z*` in `ui_sq.c`)")
P()
P("| Zone | Name | x,y,w,h | Blit | Font | Holds |\n| --- | --- | --- | --- | --- | --- |")
for z, n, g_, b, h in [("Z0", "HEAD", "0,0,240,20", "A (0..108)", "transport, clock dot + tempo, preset, channel/algorithm/MIDI, page map, length"),
                       ("Z1", "CHANNELS", "0,20,240,28", "A", "4 cells 60x28"), ("Z2", "LANE", "0,48,240,60", "A", "header line 14 px + 16 cells 15x46 pitch"),
                       ("Z3", "BAND", "0,108,240,52", "B (108..212)", "tag column 36 px + 4 knob cells 51 px; PERF: 2 rows of 26 px"),
                       ("Z4", "BUTTONS", "0,160,240,52", "B", "2 rows x 6 buttons, 26 px each, tap line + sub line"),
                       ("Z5", "LEGEND", "0,212,240,12", "C (212..240)", "11 black-key cells 20x12 at pitch 21/22/23"),
                       ("Z6", "CUE / TOAST", "0,224,240,12", "C", "cue strip or toast"), ("Z7", "MARGIN", "0,236,240,4", "C", "blank (T_BG)")]:
    P("| %s | %s | %s | %s | %s | %s |" % (z, n, g_, b, "AF_S" if z != "Z7" else "none", h))
P()
P("Blit sizes: A 240x108 = 25 920 px, B 240x104 = 24 960 px, C 240x28 = 6 720 px (limit 240x124). Redraw: a blit is redrawn when the signature of its inputs changes or `ui.force`. Timing constants: toast 2000 ms (`UISQ_TOAST_MS`), ALGO_PICK strip 2000 ms (`UISQ_PICK_MS`), hot knob 600 ms (`UISQ_HOT_MS`).")
P()

P("### 3.2 Per-element map (text elements; every string is `AF_S`)")
P()
P("Columns: slot x,y,w,h = the box SCREEN_SPEC 2 / the draw code assigns (screen px); default string = what PERF draws at power-on (`PERF_default`); px = `text_w`; limit = the width the code passes to `cv_text_fit` (or the cell width for button taps); variants = distinct strings seen over all PERF scenarios and all six algorithm ids (`elements.tsv` has them with widths); colours = fg/bg roles with the state that selects them.")
P()
groups = [("Z0 HEAD", lambda e: e["id"].startswith(("H1", "H2", "H3", "H4", "H5", "H6")) and e["kind"] == "text"),
          ("Z1 CHANNELS", lambda e: e["id"].startswith("CH") and e["kind"] == "text"),
          ("Z2 LANE", lambda e: e["id"].startswith("LANE") and e["kind"] == "text"),
          ("Z3 BAND", lambda e: e["id"].startswith(("TAG", "KNOB", "STATUS")) and e["kind"] == "text"),
          ("Z4 BUTTONS", lambda e: e["id"].startswith("BTN") and e["kind"] == "text"),
          ("Z5 LEGEND", lambda e: e["id"].startswith("KEY") and e["kind"] == "text"),
          ("Z6 CUE / TOAST", lambda e: e["id"] in ("CUE", "TOAST"))]
STATE = {  # per element class: the colour rule in ui_sq.c by state: normal / selected / muted / playing / latched(live) / dim
 "H1": "normal `.`/`>` T_TEXT on T_BG; REC page `R` T_REC on T_BG",
 "H2": "T_TEXT on T_BG (one state; clock dot beside it is fixed amber)",
 "H3": "T_TEXT on T_BG (one state)", "H4": "T_TEXT on T_BG (one state; follows the selected channel)", "H6": "T_TEXT on T_BG (one state)",
 "H5": "digit: viewed/exists T_DIM on T_BG; playing T_INK on T_SEL fill",
 "CH.L1": "normal T_TEXT/T_BG; selected T_INK/T_SEL; muted T_DIM/T_BG",
 "CH.L2": "normal T_MID/T_BG; selected T_INK/T_SEL; muted T_DIM/T_BG; MIDI OFF, unmuted, unselected T_ACCENT/T_BG (no 3 s timer in code)",
 "LANE.HDR": "T_MID on T_BG (one state)", "LANE.PLAYPAGE": "playing only: T_ACCENT on T_BG", "LANE.GLYPH": "T_TEXT on T_LANE (one state)",
 "TAG": "live half (two-row pages, row selected): T_INK on T_SEL; otherwise T_TEXT on T_BG; sub-line T_MID",
 "KNOB.LABEL": "live row (two-row page): T_INK on T_SEL; not live: T_DIM on T_BG; single-row pages: T_MID on T_BG",
 "KNOB.VALUE": "live row (two-row page): T_INK on T_SEL always; not live: T_DIM on T_BG, T_ACCENT on T_BG for 600 ms after that column's knob is turned; single-row pages: T_THEME (T_ACCENT when hot) on T_BG",
 "STATUS": "T_MID on T_BG (single-row pages only)",
 "BTN.TAP": "normal T_TEXT on T_BG; dice layer absent: T_DIM; tap label differing from the printed word (RST.., BACK, CANCEL): T_ACCENT",
 "BTN.SUB": "T_MID on T_BG; layer absent (`-`): T_DIM",
 "KEY": "T_TEXT on T_RAISE; muted channel's cell (M1..M4): T_DIM on T_RAISE",
 "CUE": "T_TEXT on T_BG", "TOAST": "T_ACCENT on T_TINT fill"}
def state_for(eid):
    for k in ("H5", "H1", "H2", "H3", "H4", "H6"):
        if eid.startswith(k) and (k != "H1" or eid == "H1"): return STATE[k]
    if eid.startswith("CH"): return STATE["CH.L1" if eid.endswith("L1") else "CH.L2"]
    if eid.startswith("TAG"): return STATE["TAG"]
    if eid.startswith("KNOB"): return STATE["KNOB.LABEL" if eid.endswith("LABEL") else "KNOB.VALUE"]
    if eid.startswith("STATUS"): return STATE["STATUS"]
    if eid.startswith("BTN"): return STATE["BTN.TAP" if eid.endswith("TAP") else "BTN.SUB"]
    if eid.startswith("KEY"): return STATE["KEY"]
    return STATE.get(eid, "")
for title, f in groups:
    P("**%s**" % title); P()
    P("| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |\n| --- | --- | --- | --- | --- | --- |")
    for e in els:
        if not f(e): continue
        lim = e["slot_limit_px"] or "-"
        P("| %s | %s,%s,%s,%s | `%s` | %s / %s | %s | %s |" % (e["id"], e["x"], e["y"], e["w"], e["h"], cell(e["default_text"]), e["default_px"], lim, e["n_variants"], cell(strip_sc(e["colours"]))))
    P()

P("### 3.3 Colour by state (the rule in the draw code; verified against the recorded draw colours)")
P()
P("| Element | Rule |\n| --- | --- |")
seen = set()
for k, v in STATE.items():
    P("| %s | %s |" % (k, v))
P()
P("Non-text elements (rows `kind=graphic` in `elements.tsv`, with geometry and colours): clock dot 8x8 at (12,6) fixed amber 0xFD80 (dark) / 0xCB80 (light), not a theme role; page-map cells 9x11 at x=174+10k (outline T_TEXT when viewed or playing, T_DIM otherwise, T_SEL fill when playing); channel select fill 58x28 T_SEL; activity dot 4x4 at (60c+54, 24); lane cell 14x46 at x=15n (T_LANE live, T_BG past LENGTH); lane bar 12 px wide, height 2 + vel*30/127 (T_THEME, T_ACCENT when velocity >= 100), off-step bar 12x2 T_GRID; edited frame (NOTE_EDIT) and playhead underline 14x2 at y=106 T_ACCENT; live-row fill T_SEL; separator 1x20 T_LINE; hold mark 5x7 T_MID; legend well 20x12 T_RAISE; toast fill 240x12 T_TINT.")
P()

P("### 3.4 Strings cut with an ellipsis (`text_fit` flag 1) in the current render")
P()
P("Over every scenario (PERF family and every page x algorithm): **%d** distinct (page, element, string) cuts, **all in the Z5 black-key legend** (limit 18 px: a 20 px cell less 1 px each side). No label in Z0..Z4 or Z6 is cut in any scenario. The ellipsis glyph (AF_S glyph 133) replaces all but the first letter.\n" % len(trunc))
P("PERF page:\n")
P("| Element | Full string | Drawn as | Full px | Limit px |\n| --- | --- | --- | --- | --- |")
for t in trunc:
    if t["page"] == "PERF": P("| %s (key %s) | `%s` | `%s` | %s | %s |" % (t["element"], ["", "F#3", "G#3", "A#3", "C#4", "D#4", "F#4", "G#4", "A#4", "C#5", "D#5", "F#5"][int(t["element"][5:])], t["full_string"], t["drawn_as"], t["full_px"], t["slot_px"]))
P()
P("Other pages (same cells; full list in `truncations.tsv`):\n")
oth = collections.OrderedDict()
for t in trunc:
    if t["page"] != "PERF": oth.setdefault((t["element"], t["full_string"], t["drawn_as"], t["full_px"]), []).append(t["page"])
P("| Element | Full string | Drawn as | Full px | Pages |\n| --- | --- | --- | --- | --- |")
for (e, f, d, px), pg in oth.items():
    if (e, f) in [("KEY.B05", "CHN"), ("KEY.B06", "SOL"), ("KEY.B08", "ROL"), ("KEY.B09", "PG-"), ("KEY.B10", "PG+"), ("KEY.B11", "RST")] and len(pg) >= 6: continue
    P("| %s | `%s` | `%s` | %s | %s |" % (e, f, d, px, ", ".join(pg)))
P()
P("(The six cuts that repeat on every page that carries the PERF legend - CHN, SOL, ROL, PG-, PG+, RST on KEY.B05/06/08/09/10/11 - are omitted from this second table; they are the PERF table above.)")
P()
P("Legend labels that are NOT cut but have 1 px slack: `M1`..`M4` (17 px of 18), `FIL` (15 of 18). The four mute keys M1..M4 draw distinct strings.")
P()

P("### 3.5 Tightest fits (slack = limit - text_w, PERF family, all algorithm ids)")
P()
sl = {}
for r in texts:
    if not (r["scenario"].startswith("PERF_") or r["scenario"].startswith("page_PERF_")): continue
    if r["kind"] != "T": continue
    lim = int(r["maxw"]) or 0
    if not lim: continue
    k = (r["full"], lim)
    sl[k] = lim - int(r["full_px"])
rows = sorted(sl.items(), key=lambda kv: (kv[1], kv[0][0]))
P("| String | Limit px | Slack px |\n| --- | --- | --- |")
shown = 0
for (s_, lim), v in rows:
    if v < 0: continue
    if shown >= 14: break
    P("| `%s` | %d | %d |" % (disp(s_), lim, v)); shown += 1
P()
P("Z4 button cells are sized per page and algorithm from the widest label plus padding 3 (1 when the row is tight), then widened to 240; PERF row 1 cell widths for ACID: 23 / 41 / 42 / 47 / 44 / 43 (sum 240), row 2: 53 / 51 / 46 / 28 / 34 / 28. Row-2 label ink on PERF ACID: HLP/MN 44 (+ hold mark 6), UNDO L 42 (+6), GT+SLD 43, HITS 25, PLAY 28, REC 22.")
P()

P("### 3.6 Ink that leaves its own box by 1 px (measured from glyph rows)")
P()
P("| Element | Ink rows vs own box rows |\n| --- | --- |")
for e in els:
    if e["ink_rows_outside_slot"]: P("| %s | %s |" % (e["id"], e["ink_rows_outside_slot"]))
P()
P("These come from glyphs that reach above or below the cap band (`/`, round digits and the ellipsis: AF_S rows 2..13 for `/`, 3..12 for round glyphs); the box is the zone, cell or half the element is drawn in. The legend cell is 12 px high (rows 212..223) and Z6 text starts at row 224 (pen 221, cap top row 3).")
P()

# ---------------------------------------------------------------- 4 function / purpose / controls
P("## 4. Function, purpose, controls and states of each element")
P()
P("Wording in quotation marks is taken from the section named (several clauses of one section are joined and abridged; the words are the spec's). Control ids are `control_map.tsv` ids (`EN_*` encoders, `B_*` buttons, `KEY_W*` white keys, `KEY_B*` black keys).")
P()
P("| Element (ids in `elements.tsv`) | Function / purpose as the spec states it | Source | Driven by | States it can show |\n| --- | --- | --- | --- | --- |")
seen = set()
for e in els:
    key = re.sub(r"\.\d+|\d+", "", e["id"]) if not e["id"].startswith(("H", "LANE.GLYPH")) else (e["id"][:2] if e["id"].startswith("H") else "LANE.GLYPH")
    if e["id"].startswith(("KNOB", "BTN", "KEY", "TAG", "CH")): key = e["id"].split(".")[0] if not e["id"].startswith("KNOB") else "KNOB"
    if e["id"].startswith(("CH",)): key = "CH"
    if key in seen: continue
    seen.add(key)
    names = {"CH": "CH1..CH4 .L1/.L2, CH.SELECT_FILL, CH.ACTIVITY_DOT", "KNOB": "KNOB.R1/R2.K1..K4 .LABEL/.VALUE, KNOB.LIVE_FILL", "BTN": "BTN.R1/R2.C1..C6 .TAP/.SUB, BTN.SEPARATOR, BTN.HOLD_MARK", "KEY": "KEY.B01..B11, KEY.CELL", "TAG": "TAG.OCTDN/OCTUP .L1/.L2, TAG.LIVE_FILL", "H5": "H5.1..H5.4, H5.CELL"}
    tagctl = "B_OCTDN (upper half), B_OCTUP (lower half)"
    nm = names.get(key, e["id"])
    ctl = e["controls"] if not e["id"].startswith(("KNOB", "BTN", "KEY", "LANE.GLYPH", "CH")) else {"KNOB": "EN_K1..EN_K4 (ROW 1 = K1..K4, ROW 2 = S1..S4 via B_OCTDN / B_OCTUP)", "BTN": "B_FX B_SCL B_ENV B_LFO B_EDIT B_GLO / B_HOME B_SAVE B_ARP B_SEQ B_PLAY B_REC", "KEY": "KEY_B01_F#3 .. KEY_B11_F#5", "LANE.GLYPH": "KEY_W01_F3 .. KEY_W16_G5", "CH": "EN_SELECT, EN_ALGO, KEY_B01..KEY_B04 (M1..M4), EN_K2 on CHAN"}[key]
    if key == "TAG": ctl = tagctl
    P("| %s | \"%s\" | %s | %s | %s |" % (nm, cell(e["spec_function"]), e["spec_ref"], ctl, cell(e["states"])))
for gid in ("H2.DOT", "LANE.CELL_BG", "LANE.BAR", "LANE.PLAYHEAD", "LANE.EDIT_FRAME", "BTN.HOLD_MARK"):
    pass
P()
P("Additional statements the spec makes about elements (verbatim):")
P()
P("- Whole screen, VISION rule 1 as SCREEN_SPEC opens: \"at every moment the screen shows what the device is **doing** and the current role of **every** knob and button.\" (SCREEN_SPEC, intro)")
P("- SEQUENCER_UX R-4: \"Every knob is labelled on every page; a deliberately inert knob is drawn `-`.\" (SEQUENCER_UX 0, framework rules)")
P("- SEQUENCER_UX R-8: \"Every hold is named on the page where it starts: the `h:` sub-label on the PERF button picture ..., `hold=EDIT` in the lane header for the white keys, the black-key legend for SOLO / FILL / ROLL.\" (SEQUENCER_UX 0)")
P("- SCREEN_SPEC 2.4 on BT: \"tag column = the OCT- (upper half, y=108..134) and OCT+ (lower half, y=134..160) button labels of the page; see 7. PERF: `OCT-` over `h:peek`, `OCT+` over `h:peek`\"; `button_modes.tsv` (PERF, B_OCTDN / B_OCTUP) and `control_map.tsv` give the PERF labels `ROW 1` / `ROW 2`; the render draws `ROW` over `1` and `ROW` over `2` (two lines, `uisq_wrap`).")
P("- SCREEN_SPEC 2.2 (muted): \"A **muted** channel draws the whole cell dim and line 2 reads `MUTE 2`, `MUTE 12` (49) or `MUTE -` (MIDI OFF) ... Three of the four channels are muted at power-on, so this is the default screen.\" The render at power-on (`sq_boot`): CH1 unmuted MIDI 1, CH2..CH4 muted, MIDI 2..4.")
P("- SCREEN_SPEC 6.2 clock dot: \"amber solid = the FM-1's own clock (internal, and it is sending clock out); cyan with a 2 x 2 centre hole = a host clock drives the tempo; cyan ring (4 x 4 hole) blinking at 2 Hz = armed, waiting for the host to start\" and \"never colour alone\".")
P()

# ---------------------------------------------------------------- 5 other pages
P("## 5. The other pages: what the spec says, and what `ui_sq.c` draws")
P()
P("`ui_sq.c` gives every page an entry in `uisq_pages[]`; every `input` function is `uisq_pg_stub` (does nothing). Drawing is one generic path: the zones are drawn from the page's tables (knob, button, key labels, cue from `page_states.tsv`) and the page-specific code is limited to what is listed in the last column. Knob values are drawn only on PERF (`lp == UI_PG_PERF`); on every other page a knob cell is its label only.")
P()
P("| Page | page_states.tsv `screen` / SCREEN_SPEC 3 row (abridged) | `ui_sq.c` draws | Real content or stub |\n| --- | --- | --- | --- |")
pages = [
 ("PERF", "header + channel band + 16-cell lane + 8-cell knob band (live row filled) + button picture + black-key legend + cue strip; cue `PLAY: start   K1-K4: morph`", "all zones with real state (channels, lane via `sq_eval`, morph values, playhead, page map, toast, algo-pick strip); H2 INT only; H3 `Pnn` only", "real content (input stub)"),
 ("NOTE_EDIT", "header + lane (edited step outlined) + band PITCH GATE VELO RATCH with AUTO/set state + 6 buttons + OCT cells + legend + cue `STEP n: KNOBS EDIT  HOME=done`; Z1 banner `CH1 NOTE EDIT STEP 21`", "Z1 banner `CH1 NOTE EDIT STEP 21` (T_ACCENT on T_TINT), edited-step frame (T_ACCENT) in the lane, knob labels PITCH GATE VELO RATCH without values, status line 2 `* = set by you (EDIT: undo)`, button/legend/cue from tables; status line 1 (`AUTO = generated: ...`) is empty", "partial: banner, frame, labels, one status line; no AUTO/value cells"),
 ("CHAN", "header + lane + band LENGTH MIDI SOURCE FOLLOW + page map (4 cells: used / viewed / playing); tag `-PAGE`/`+PAGE`; status `PAGES  1 OWN  2 P1 ...` and `D#4 = BACK`", "channel cells and PERF lane unchanged, knob labels LENGTH MIDI SOURCE FOLLOW (no values), tag `-PAGE`/`+PAGE`, status line 2 `D#4 = BACK`; the four large page-map cells and status line 1 are not drawn", "stub-level: labels and one status line"),
 ("SET", "header + lane + band BPM SWING GATE DIV + 2 status lines (TEMPO INT/EXT, OUT MASTER/FOLLOW, host-clock texts); tag `OCTAVE -/+`", "knob labels BPM SWING GATE DIV (no values), tag `OCT-`/`OCT+`, both status lines empty strings, GLO reads `BACK`, cue `SET: GLO hold=HARM  HOME=back`", "stub-level: labels only; tempo-source status lines not drawn"),
 ("SCALE", "HARMONY screen: Z2 progression strip (degree numerals AF_L, chord names) + legend `F3 G3 ... = 1..7`; band SCALE KEY VOICE MOVE; status DRONE / ROLE; buttons PROG BARS ROLE NEW", "Z2 draws only the word `HARMONY` (T_TEXT) at x=2; knob labels SCALE KEY VOICE MOVE (no values); buttons PROG, BARS, ROLE, NEW from tables; legend keys all `-`; status lines empty", "stub (Z2 placeholder word)"),
 ("REC", "banner REC + lane with write cursor + band CURSOR ACC SLIDE OCT + `KEYS = NOTES (all 27)`; cue `REC: PLAY KEYS TO WRITE  REC=done`", "Z1 banner `CH1 REC writing to page 1` (T_REC on T_RAISE), H1 `R` in T_REC, knob labels CURSOR ACC SLIDE OCT (no values), status line 2 `KEYS = NOTES (all 27)`, legend `NOTE` x11 (cut to `N...`), REC button reads `DONE`; no write cursor `^`", "partial: banner, labels, status line"),
 ("RESET", "the page beneath with every resettable cell tagged `RST` + banner `RESET`; K1..K4 = `RST`; tag `RST R1`/`RST R2`; buttons `RST AL`, `RST NT` ...", "no banner, no `RST` tags on cells; knob labels `RST` x4 (no values), tag `RST`/`R1`, `RST`/`R2` wrapped on two lines, buttons `RST AL RST NT RST AC RST SH CANCEL -` etc. from tables, legend `-` except `COPY P`", "stub-level: labels from tables"),
 ("HELP", "the page beneath dimmed + a 2-line help box over y=108..212 naming the touched control; cue `HELP: <control>  let go = back`", "no dimming, no help box; every knob and most button labels read `?`, HOME cell `BACK` over `HELP`, PLAY cell `PLAY`, from tables; cue `HELP: let go = back` (override); legend cells from table", "stub"),
 ("MENU", "Felucca menu drawn by `ui_menu.c`", "`ui_sq_draw` returns 0 (draws nothing)", "not this file's"),
 ("FIRST_RUN", "overlay of PERF: cue strip shows the guided step; step 1 = `D#4 K2: MIDI CHANNEL`, then `PRESS PLAY` ... (SEQUENCER_UX 11.3)", "PERF layout; cue string fixed `D#4 K2: MIDI CHANNEL`; no step sequencing", "stub (one static cue)"),
 ("ADD_PAGE", "prompt overlay: a ghost page cell + cue text `ADD PAGE n = COPY OF n-1? PG+ = yes` (or SHOW PAGE n / SHOW ALL OF PAGE n)", "PERF layout; cue `ADD PAGE 2 = COPY OF 1? PG+ = yes` (the generated example text); no ghost page cell", "stub"),
 ("MOVE_HELD", "overlay: held black-key cell inverted; FILL / ROLL cell shows the live amount; cue `ROLL 3x: let go to stop`", "PERF layout; cue `ROLL 3x: let go to stop`; no inverted cell", "stub"),
 ("ALGO_PICK", "overlay: the cue strip lists all six algorithms with the current one in brackets, `ACID EUCL SNAK STAG [ARP] PATT` (185 px of 240), 2 s after the last detent", "PERF layout; the strip text is built from `uisq_short[]` when `uisq_pick_until` is non-zero (`[ACID] EUCL SNAK STAG ARP PATT` for CH1; 185 px). When `uisq_page` is set to ALGO_PICK without a detent the cue is the empty string", "real for the strip (via `ui_sq_algo_pick_touch`); the page entry itself is a stub"),
 ("RIG", "RIG panel in the lane area: banner `RIG CHn`, four wires, one line each; band PARAM LAYER FORM MOVE; tag `RIDE -/+`; cue `RIG: TAP 1-4 = WHO DRIVES  HOLD = PICK`", "PERF lane, knob labels PARAM LAYER FORM MOVE (no values), tag `RIDE-`/`RIDE+`, cue from table; no banner, no wire lines", "stub"),
]
for p, a, b, c in pages: P("| %s | %s | %s | %s |" % (p, cell(a), cell(b), c))
P()
P("Per-page text drawn by the generic path (page_<NAME>_algo0 scenario, `texts_all.tsv`): the Z4 buttons of NOTE_EDIT show `ALL NOTE ACCENT PROB UNDO SET / HOME SAVE SLIDE ON/OFF PLAY REC` (the page's own tap labels); SET `GLO` cell reads `BACK`; HELP draws `?` for 10 of the 12 button taps; HOME reads `BACK` over `HELP`, PLAY reads `PLAY`.")
P()

# ---------------------------------------------------------------- 6 counts
P("## 6. Counts (for the working-memory heuristic)")
P()
def zone_of_t(sc, r):
    by = int(r["blit_y"]); py = int(r["pen_y"])
    if by == 0: return "Z0" if py < 20 else "Z1" if py < 48 else "Z2"
    if by == 108: return "Z3" if py < 52 else "Z4"
    return "Z5" if py < 9 else "Z6"
def count(sc):
    T = [r for r in texts if r["scenario"] == sc and r["kind"] == "T"]
    z = collections.Counter(zone_of_t(sc, r) for r in T)
    dist = collections.Counter(zone_of_t(sc, r) for r in {(zone_of_t(sc, r), r["drawn"]): r for r in T}.values())
    fg = set(r["fg565"] for r in T); bg = set(r["bg565"] for r in T)
    return T, z, dist, fg, bg
T, z, dist, fg, bg = count("PERF_default")
TP, zp, distp, fgp, bgp = count("PERF_playing")
P("Scenario `PERF_default` (power-on, stopped, ROW 1 live, ACID selected; CH1 unmuted, CH2..CH4 muted) and `PERF_playing` (transport running, CH1 sounding).")
P()
P("| Zone | Text draws, default | distinct strings, default | text draws, playing | Non-text items drawn (default) | Visible interactive labels |\n| --- | --- | --- | --- | --- | --- |")
nontext = {"Z0": "clock dot, 4 page-map cells (frames/dots)", "Z1": "select fill, activity dot (when sounding)", "Z2": "16 cell backgrounds, up to 16 bars, playhead (playing)", "Z3": "live-row fills: 5 (tag half + 4 knob cells)", "Z4": "10 separators (5 per row), 4 hold marks", "Z5": "11 wells", "Z6": "none (toast: 1 fill)"}
inter = {"Z0": "H3 preset (EN_PRESET), H4 (EN_SELECT/EN_ALGO)", "Z1": "4 channel cells", "Z2": "16 step cells (white keys), header", "Z3": "8 knob cells + 2 tag labels", "Z4": "12 buttons", "Z5": "11 black keys", "Z6": "0"}
for zz in ("Z0", "Z1", "Z2", "Z3", "Z4", "Z5", "Z6"):
    P("| %s | %d | %d | %d | %s | %s |" % (zz, z[zz], dist[zz], zp[zz], nontext[zz], inter[zz]))
P("| **Total** | **%d** | **%d** | **%d** | | |" % (len(T), sum(dist.values()), len(TP)))
P()
P("- Distinct text items on the PERF screen: **%d** text draws (%d distinct strings) in the default state; **%d** while playing (adds `>`, `>P1` lane-header marker and lane glyphs of the playing pattern)." % (len(T), sum(dist.values()), len(TP)))
allperf = [r for r in texts if (r["scenario"].startswith("PERF_") or r["scenario"].startswith("page_PERF_")) and r["kind"] == "T"]
fgall = sorted(set(r["fg565"] for r in allperf)); bgall = sorted(set(r["bg565"] for r in allperf))
P("- Distinct text colours (fg RGB565): %d in the default state (%s), %d over all PERF states (%s). Distinct backgrounds under text: %d default, %d over all states." % (len(fg), ", ".join(sorted(fg)), len(fgall), ", ".join(fgall), len(bg), len(bgall)))
allc = set()
for r in texts:
    if r["scenario"] == "PERF_default": allc.add(r["fg565"]); (allc.add(r["bg565"]) if r["kind"] == "T" else None)
P("- Distinct colours of every pixel-drawing call (text fg/bg, rect, frame) in `PERF_default`: **%d** (%s); one of them is the fixed amber clock dot, the rest are theme roles that on GREY collapse to the values listed in section 1." % (len(allc), ", ".join(sorted(allc))))
P("- Distinct theme roles drawn on PERF: T_BG, T_TEXT, T_MID, T_DIM, T_INK, T_SEL, T_RAISE, T_LANE, T_LINE, T_GRID, T_THEME, T_ACCENT (12), plus T_TINT for the toast, plus T_REC on the REC page; plus the fixed amber dot.")
P("- Distinct font faces / sizes used on PERF: **1** (AF_S, 15 px line box, 9 px caps). Text weights: 1 (400).")
P("- Items per zone as a reader meets them: Z0 6 text + dot + 4-cell map (header slots H1..H6); Z1 4 cells x 2 lines; Z2 1 header line + 16 cells; Z3 8 knob cells (label + value) + 2 tag labels; Z4 12 buttons (12 taps + 9 sub-labels at ACID); Z5 11 legend labels; Z6 1 line.")
P("- Controls with an on-screen role label at once: 4 channel cells + 16 step cells + 2 tag + 8 knob + 12 buttons + 11 black keys = **53** labelled controls on one screen (the control map has 49 controls: 16 white keys, 11 black keys, 14 buttons, 7 encoders incl. SELECT/ALGO/PRESET, MASTER).")
ys = sorted(set((int(r['blit_y']) + int(r['pen_y'])) for r in T))
P("- Distinct text line tops (screen y) on PERF default: %d (%s)." % (len(ys), ", ".join(str(v) for v in ys)))
P()

# ---------------------------------------------------------------- 7 spec vs render
P("## 7. Observed differences between the spec text and the current render (facts)")
P()
P("| # | Spec says | Render does |\n| --- | --- | --- |")
diffs = [
 ("SCREEN_SPEC 2.4: PERF tag column `OCT-` over `h:peek`, `OCT+` over `h:peek`", "`button_modes.tsv` / `control_map.tsv` label B_OCTDN / B_OCTUP `ROW 1` / `ROW 2`; the tag draws `ROW` over `1` and `ROW` over `2` (two lines each, 27 px); no `h:peek` sub-label"),
 ("SCREEN_SPEC 5: DENS / HITS at 0 shows `KEYS`; SEQUENCER_UX 2.1 mock: `DIR FWD`", "knob values are the raw morph integer (`DIR 0`, `DENS 0`); `uisq_draw_z3` does no value formatting"),
 ("SCREEN_SPEC 2.1 H2: INT n / EXT -- / EXT n / WAIT; dot colours amber / cyan / cyan ring", "`INT <bpm>` and the amber solid dot only (`uisq_h2_text`, `uisq_dot`), marked a placeholder in the code"),
 ("SCREEN_SPEC 2.3: lane header `PG 1/2  FOLLOW  hold=EDIT        >P2`", "FOLLOW off draws the word `hold`: `PG 1/2  hold  hold=EDIT`; `>Pn` is right-aligned at x=236 only while playing"),
 ("SEQUENCER_UX 11.2 row 14 alternates every 4 s; row 15 `ROW 2: OCT- = back to row 1` while ROW 2 is live and playing", "`uisq_cue_text` draws row 14's first string `TAP STEP=ON/OFF  HOLD=EDIT NOTE` (203 px) whenever PERF is playing; no alternation, no row 15"),
 ("SCREEN_SPEC 2.2: MIDI OFF drawn T_ACCENT for 3 s after displacement, then T_TEXT", "line 2 `MIDI OFF` is T_ACCENT for as long as the channel is unmuted, unselected and MIDI OFF (no timer)"),
 ("SCREEN_SPEC 2.6: legend cell inverted while the key is down", "no key-down state is read; legend cells are T_RAISE wells with T_TEXT / T_DIM labels"),
 ("SCREEN_SPEC 2.6 / F3: labels over 20 px are cut with an ellipsis, \"the full word is in HELP\"", "6 PERF legend labels (CHN, SOL, ROL, PG-, PG+, RST) draw as one letter plus the ellipsis; HELP page is a stub (`?`)"),
 ("SCREEN_SPEC 2.4: the live row is filled (T_SEL); \"line 2 the value (T_THEME, T_ACCENT while the knob is hot)\"", "the live row's value is always T_INK on T_SEL; the hot colour T_ACCENT is applied to the value of the **other** (non-live) row's cell in that column (`hot` is indexed by column only)"),
 ("SCREEN_SPEC 7.1 / 3: NOTE_EDIT, CHAN, SET, SCALE, REC, RESET, HELP bodies", "see section 5: values absent on all non-PERF pages; HELP box, RESET tags, CHAN page-map cells, SCALE strip, RIG wires, REC cursor not drawn"),
 ("SCREEN_SPEC 2.5: cell widths data per page, `h:` replaced by a hold mark", "hold mark drawn as the 5x7 down-triangle, widths per page and algorithm computed from `text_w` (test asserts sum = 240 on every page x algorithm)"),
]
for i, (a, b) in enumerate(diffs, 1): P("| %d | %s | %s |" % (i, cell(a), cell(b)))
P()
P("## 8. Reproducing")
P()
P("```sh\nsh docs/design/ui_review/measure.sh build/ui_review     # compiles the instrumented harness, writes build/ui_review/*.tsv\npython3 docs/design/ui_review/analyze.py                  # elements.tsv, truncations.tsv, text_pairs.tsv, copies palettes/fonts here\npython3 docs/design/ui_review/contrast.py                 # the contrast tables of section 1\npython3 docs/design/ui_review/build_evidence.py           # regenerates this file\n```")
open(D + "/EVIDENCE.md", "w", encoding="utf-8").write("\n".join(out) + "\n")
print("EVIDENCE.md written, %d lines" % len(out))
