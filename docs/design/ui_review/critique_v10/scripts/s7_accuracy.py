from common import *
import csv, re
DES = os.path.abspath(os.path.join(UI, ".."))
def tsv(n):
    with open(os.path.join(DES, n), encoding="utf-8") as f: return list(csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE))
BM, KM, KEYM, PS = tsv("button_modes.tsv"), tsv("knob_map.tsv"), tsv("key_map.tsv"), tsv("page_states.tsv")
import icons_v10 as IV
out = []
def claim(area, v10, tsvtxt, ok, note=""):
    out.append((area, v10, tsvtxt, "MATCH" if ok is True else ("MISMATCH" if ok is False else str(ok)), note))
# 1 layout
lay_tsv = ["FX", "SCL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ", "PLAY", "REC"]
claim("button grid order (2x6)", " ".join(b[0] for b in IV.BUTTONS), "HANDOFF/02_CONSTRAINTS.md s2: row1 FX SCL ENV LFO EDIT GLO, row2 HOME SAVE ARP SEQ PLAY/STOP REC", [b[0] for b in IV.BUTTONS] == lay_tsv)
# 2 tap roles
cid = {"FX": "B_FX", "SCL": "B_SCL", "ENV": "B_ENV", "LFO": "B_LFO", "EDIT": "B_EDIT", "GLO": "B_GLO", "HOME": "B_HOME", "SAVE": "B_SAVE", "ARP": "B_ARP", "SEQ": "B_SEQ", "PLAY": "B_PLAY", "REC": "B_REC"}
for pn, ic, role, hold, dice in IV.BUTTONS:
    tap = [r for r in BM if r["page"] == "PERF" and r["control"] == cid[pn] and r["mode"] == "tap"][0]
    hd = [r for r in BM if r["page"] == "PERF" and r["control"] == cid[pn] and r["mode"] == "hold"][0]
    claim("PERF %s tap label" % pn, "role %s (icon %s)" % (role, ic), "button_modes PERF %s tap label %s" % (cid[pn], tap["label"]), role == tap["label"])
    if hold:
        claim("PERF %s hold label" % pn, "hold role %s" % hold, "hold label %s" % hd["label"], hold == hd["label"])
    else:
        claim("PERF %s hold" % pn, "no hold role", "hold: %s" % hd["action"][:70], ("no hold role" in hd["action"]) or hd["label"] == tap["label"] or "acted once" in hd["action"])
# dice ones
dice_tsv = {"B_FX": 1, "B_SCL": 1, "B_ENV": 1, "B_LFO": 1, "B_ARP": 1, "B_SEQ": 1}
claim("which buttons are dice", ",".join(b[0] for b in IV.BUTTONS if b[4]), "02_CONSTRAINTS: FX SCL ENV LFO ARP SEQ = six dice", [b[0] for b in IV.BUTTONS if b[4]] == ["FX", "SCL", "ENV", "LFO", "ARP", "SEQ"])
# 3 black keys: x -> note -> TSV label
whites = "F3 G3 A3 B3 C4 D4 E4 F4 G4 A4 B4 C5 D5 E5 F5 G5".split()
import el
for cx, role in el.BLACK:
    i = cx // 15 - 1; w = whites[i]; sharp = w[0] + "#" + w[1:]
    rows = [r for r in KEYM if r["page"] == "PERF" and r["key"].endswith("_" + sharp)]
    lbl = rows[0]["label"] if rows else "?"
    claim("black key at x=%d (%s)" % (cx, sharp), role, "key_map PERF KEY_B..%s label %s (gestures: %s)" % (sharp, lbl, "/".join(r["gesture"] for r in rows)), role == lbl)
# glyph claims on black keys (code reading, rendered)
claim("M1..M4 glyph", "channel-colour 6x7 block if channel used; x7 glyph (MID) if muted; nothing if unused", "key_map: tap = mute/unmute CHn; hold = PREVIEW", "shown: tap-state only", "hold PREVIEW (M1..M4) has no mark; unused-channel key is blank though tap still mutes CHn")
claim("CHN glyph", "midi_s DIN 7x5 icon", "key_map: tap opens CHAN page (LENGTH MIDI SOURCE FOLLOW); hold = RIG", "partial", "icon says MIDI; page also holds LENGTH SOURCE FOLLOW; RIG hold not shown")
# 4 hold hints
def holdrow(c): return [r for r in BM if r["page"] == "PERF" and r["control"] == c and r["mode"] == "hold"][0]
scn = mod.STATE["scenes"]
claim("hold EDIT hint", scn["hold_edit"]["hold"][1], "B_EDIT hold: '0.4 s: open RESET (latched, one shot)'", True, "text 'HOLD EDIT: RESET' = label RESET; fill at 60 % = 0.24 s of 0.4 s")
claim("hold GLO hint", scn["hold_glo"]["hold"][1], "B_GLO hold: '0.4 s: open HARMONY'", True, "label HARM spelled HARMONY; 60 % = 0.24 s of 0.4 s")
claim("hold SAVE hint", scn["hold_save"]["hold"][1], "B_SAVE hold: '0.7 s: undo the last preset load...; while a slot is pending: cancel it instead'", "MATCH (partial)", "pending-slot variant 'cancel it' has no hint text; 80 % = 0.56 s of 0.7 s; no TSV time for hint onset except HOME")
claim("hold HOME hint", scn["hold_home"]["hold"][1], "B_HOME hold: 'from 0.25 s the strip reads HOME: touch=HELP  let go=MENU'", "MATCH (wording)", "drops the 'HOME:' prefix and upper-cases; 100 % bar = 0.7 s, drawn OFF_RED")
claim("HOME hold semantics", "TOUCH=HELP  LET GO=MENU", "HANDOFF/02_CONSTRAINTS s3: owner decision 118 changes HOME hold to 'map while held, keep holding to escalate to MENU' (TSV not updated)", "TSV-consistent, decision-118-inconsistent", "v10 follows the TSV text")
# 5 encoders
claim("SELECT toast", "SELECT | CH2  EUCLID", "knob_map PERF SELECT: 'turn: select channel CH1..CH4 (clamped, no wrap)', label 'CH n'", True, "adds the algorithm name; 'CH2' matches label form")
claim("ALGO toast", "ALGO | SNAKE AT NEXT STEP", "knob_map PERF ALGO: 'lands at the next step; morphs and seeds kept'", True, "wording matches")
S = scene("turn_algo"); R = rec(S); hdr = [t["s"] for t in R["texts"] if t["ycap"] == 5 and t["x"] == 105]; kn = [t["s"] for t in R["texts"] if t["ycap"] == 27]
claim("ALGO scene state", "header %s; knob labels %s (toast says AT NEXT STEP)" % (hdr, kn), "lands at the next step (old algorithm still active until then)", False, "the scene draws the NEW algorithm's header and knobs while the toast says it has not landed")
claim("PRESETS toast (playing)", "PRESETS | P02 AT NEXT BAR (scene playing=True)", "knob_map PERF PRESET: playing: first 768-tick bar >= 1.5 s after last detent", True, "")
claim("PRESETS toast (stopped)", "no scene; text is fixed 'AT NEXT BAR'", "stopped: loads 1.5 s after the last detent", "NOT SHOWN", "no stopped-state wording exists in v10")
claim("pending header", "pill 'P02?' only", "knob_map: header 'Pnn?' then 'Pnn>' once armed to the bar", "partial", "the 'Pnn>' armed state is not drawn")
claim("OCT+ ROW 2", "OCT+ | ROW 2 OF 2", "button_modes B_OCTUP tap label 'ROW 2'; ROW 1->ROW 2", True, "'OF 2' not in TSV (derived from absence of a RIG)")
claim("OCT+ ROW 3", "OCT+ | ROW 3: RIG; markers 3 only when S['rig']", "B_OCTUP tap: ROW 2 -> ROW 3 'only when the channel has a device with a wire that is not DEV'", True, "")
claim("OCT+ on last row", "no scene", "'on ROW 3 inert, toast LAST ROW: OCT- = ROW 1'", "NOT SHOWN", "")
claim("OCT- toast", "no scene", "B_OCTDN tap: latch ROW 1; hold = peek", "NOT SHOWN", "only OCT+ toasts exist")
claim("ROW 3 tile labels", "WIRE1..WIRE4 with 0..127 bars", "knob_map PERF R1-R4: 'the cell shows the device's name for the parameter'", False, "generic WIRE n labels, not device parameter names; value range 0..127 generic")
claim("SELECT past last channel", "docstring: SELECT past the last channel adds one; header '+' chip", "knob_map PERF SELECT: 'clamped, no wrap'; SQ_NCH 4", "CONFLICT", "no TSV row defines 'add a channel' (grep add-channel in the 4 TSVs: none)")
# 6 REC
REC = {r["knob"]: r for r in KM if r["page"] == "REC" and r["knob"] in ("K1", "K2", "K3", "K4")}
for k, kd in zip(("K1", "K2", "K3", "K4"), mod.REC_ROW):
    claim("REC %s" % k, "%s %s..%s %s" % (kd["label"], kd["min"], kd["max"], kd["names"] or ""), "label %s; range %s" % (REC[k]["label"], REC[k]["range_default"]), kd["label"] == REC[k]["label"], "CURSOR fixed 1..16 vs 1..LENGTH wraps" if k == "K1" else "")
cue = [r for r in PS if r["state"] == "REC"][0]["cue_strip"]
claim("REC cue strip", "PLAY KEYS TO WRITE  /  REC = DONE", "page_states REC cue_strip: '%s'" % cue, "MATCH (wording)", "drops 'REC:' prefix; 'REC = DONE' vs 'REC=done'")
claim("REC lane", "lane = normal playhead lane (no write cursor mark)", "page_states REC screen: 'lane with the write cursor'", False, "the write cursor position is only the K1 CURSOR number in a tile")
claim("REC PRESET turn", "no scene", "knob_map REC PRESET: inert, toast 'REC: tap REC to finish'", "NOT SHOWN", "")
claim("REC keys", "plain keys, keybed lane tints removed, black keys plain", "page_states REC: all 27 keys play and write; force level on F3-C5", "MATCH", "")
# toasts
claim("dice toast", "NOTES | EDIT=UNDO (lit SCL)", "button_modes B_SCL tap: dice NOTES; 'EDIT tap undoes'", True, "")
claim("MIDI OFF toast", "MIDI OFF | CHN=SET", "key_map CHN tap: opens the CHAN page (not SET); SET is the GLO tap page (BPM SWING GATE DIV)", False, "the hint names SET where the page that holds MIDI is CHAN")
claim("channel added toast", "NEW | ALGO=PICK", "knob_map ALGO turn = algorithm", "MATCH (derived)", "no TSV row for adding a channel")
claim("MIDI OFF header", "red chip 'MIDI OFF'", "page_states PERF line 2 reads 'MUTE n / MIDI OFF'", True, "text MIDI OFF equal")
claim("header content", "play/stop, BPM, 4 channel chips, algorithm, MIDI n, preset, row markers, page dots", "page_states PERF header: play, clock dot + BPM with source INT/EXT n/EXT --/WAIT, preset, CH, algorithm, MIDI, page map, length", "partial", "absent: clock dot, tempo source, LENGTH; present extras: row markers")
claim("hold-sub-labels on buttons", "none drawn (hold named only while held)", "page_states PERF: button picture line 2 = hold sub-label h:RST h:HARM h:HLP/MENU h:UNDO L / dice sub-label", False, "")
claim("dice absent-layer dimming", "only DEMO has absent=('ACCNT',); algos load() sets absent=() for all 6 real algorithms", "button_modes B_ARP: 'dim if absent'; sq_types.h AlgoDesc.layer[].n=0 absent", "unverifiable from TSVs", "TSVs carry no per-algorithm layer-presence column")
claim("AlgoDesc.name", "PATTERNS is 8 chars; header uses 'PATT'", "sq_types.h AlgoDesc.name[8] comment '<= 6 chars'", "n/a", "PATTERNS (8) does not fit name[8] with NUL; UI_EXTRA short=PATT handles the header only; toast 'PATTERNS AT NEXT STEP' uses the full name")
n_ok = sum(1 for o in out if o[3] == "MATCH"); n_bad = sum(1 for o in out if o[3] == "MISMATCH")
print("# 7 accuracy vs TSVs: %d claims checked; %d MATCH; %d MISMATCH; %d other (partial / not shown / conflict / unverifiable)" % (len(out), n_ok, n_bad, len(out) - n_ok - n_bad))
print("| # | area | v10 | TSV / source | verdict | note |"); print("|---|---|---|---|---|---|")
for i, o in enumerate(out, 1): print("| %d | %s | %s | %s | %s | %s |" % ((i,) + o))
