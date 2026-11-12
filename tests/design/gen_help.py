#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-011 (F-06): generate docs/design/help_text.tsv, the HELP text of every control on every page.

HOME held + touching a control shows the control's help (U62, SCREEN_SPEC 4): two lines of at most 34 characters in the
224 px help box. One row per (control, page the control is explained on); the pages are the seven that HOME can be
held on (PERF NOTE_EDIT CHAN SET SCALE REC RESET). The PERF knob rows hold the placeholders {r1} / {r2}: the ROW 1 /
ROW 2 label of the selected channel's algorithm, taken from knob_map.tsv at run time.
Run: python3 tests/design/gen_help.py  (rewrites the table; check_help.py verifies it).
"""
import csv, os

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
D = os.path.join(ROOT, "docs", "design")
PAGES = ["PERF", "NOTE_EDIT", "CHAN", "SET", "SCALE", "REC", "RESET", "RIG"]
H = {}  # (control, page) -> (line1, line2)


def put(ctl, pages, l1, l2):
    if isinstance(pages, str):
        pages = pages.split()
    for p in pages:
        H[(ctl, p)] = (l1, l2)


ALL = " ".join(PAGES)
DICE = "PERF CHAN SET SCALE RIG"

# ---- the six dice buttons and EDIT / GLO -------------------------------------------------------------------------
put("B_FX", DICE + " REC", "ALL: re-roll every layer", "FX; edited steps are kept")
put("B_FX", "NOTE_EDIT", "FX: dice THIS step", "new pitch, accent, slide, gate")
put("B_FX", "RESET", "RST AL: algorithm to factory", "8 morphs and seeds; edits stay")
put("B_SCL", DICE + " REC", "NOTES: re-roll the pitches", "SCL; edited pitches stay")
put("B_SCL", "NOTE_EDIT", "SCL: dice this step's pitch", "a new scale note; EDIT = undo")
put("B_SCL", "RESET", "RST NT: notes seeds to factory", "edits stay; EDIT = undo")
put("B_ENV", DICE + " REC", "ACCNT: re-roll which are loud", "ENV; VELO edits stay")
put("B_ENV", "NOTE_EDIT", "ENV: accent on this step", "120 or back; EDIT = undo")
put("B_ENV", "RESET", "RST AC: accent seeds to factory", "edits stay; EDIT = undo")
put("B_LFO", DICE + " REC", "SHAPE: re-roll the register", "LFO; every algo has it")
put("B_LFO", "NOTE_EDIT", "LFO: chance 100% / 50%", "same draw every pass")
put("B_LFO", "RESET", "RST SH: shape seeds to factory", "edits stay; EDIT = undo")
put("B_EDIT", DICE, "EDIT: tap = undo / redo", "hold 0.4 s: RESET what you touch")
put("B_EDIT", "NOTE_EDIT", "EDIT: undo this step's change", "hold 0.4 s: property to AUTO")
put("B_EDIT", "REC", "EDIT: undo the last note", "again = redo")
put("B_EDIT", "RESET", "EDIT: cancel RESET", "nothing is reset")
put("B_GLO", "PERF NOTE_EDIT CHAN RIG", "GLO: tap = SET, hold = SCALE", "tempo, swing | scale and key")
put("B_GLO", "SET", "GLO: back to PERF", "hold 0.4 s: switch to SCALE")
put("B_GLO", "SCALE", "GLO: back to PERF", "you are on SCALE")
put("B_GLO", "REC", "GLO: not used while recording", "tap REC to finish")
put("B_GLO", "RESET", "GLO: nothing to reset", "EDIT cancels RESET")
# ---- the second row ---------------------------------------------------------------------------------------------
put("B_HOME", "PERF", "HOME: already home", "hold: HELP; let go untouched: MENU")
put("B_HOME", "NOTE_EDIT CHAN SET SCALE REC RIG", "HOME: back to PERF", "hold: HELP; let go untouched: MENU")
put("B_HOME", "RESET", "HOME: cancel RESET, to PERF", "hold: HELP; let go untouched: MENU")
put("B_SAVE", "PERF NOTE_EDIT CHAN SET SCALE REC RIG", "SAVE: save to the header slot", "hold 0.7 s: undo the last load")
put("B_SAVE", "RESET", "SAVE: nothing to reset", "EDIT cancels RESET")
put("B_ARP", DICE + " REC", "ARTIC: re-roll gate and slide", "ARP; edits stay")
put("B_ARP", "NOTE_EDIT", "ARP: SLIDE on this step", "glides into the next step")
put("B_ARP", "RESET", "RST AR: artic seeds to factory", "edits stay; EDIT = undo")
put("B_SEQ", DICE + " REC", "HITS: re-roll where notes fall", "SEQ; ON/OFF edits stay")
put("B_SEQ", "NOTE_EDIT", "SEQ: this step ON / OFF", "same as tapping its key")
put("B_SEQ", "RESET", "RST HT: hits seeds to factory", "edits stay; EDIT = undo")
put("B_PLAY", ALL, "PLAY: start / stop everything", "stop silences held notes")
put("B_REC", DICE, "REC: play keys, write notes", "tap REC again to finish")
put("B_REC", "NOTE_EDIT", "REC: write from this step", "the cursor starts on it")
put("B_REC", "REC", "REC: done, back to PERF", "")
put("B_REC", "RESET", "RST PG: clear the page's edits", "the page is pure generator")
put("B_OCTDN", "PERF", "OCT-: back to knob ROW 1", "hold and turn: peek at ROW 1")
put("B_OCTUP", "PERF", "OCT+: next knob row (2, then 3)", "hold and turn: peek at ROW 2")
put("B_OCTDN", "NOTE_EDIT", "OCT-: this step an octave down", "-2 to +2, auditioned")
put("B_OCTUP", "NOTE_EDIT", "OCT+: this step an octave up", "-2 to +2, auditioned")
put("B_OCTDN", "CHAN", "-PAGE: LENGTH down a page", "hidden steps keep their edits")
put("B_OCTUP", "CHAN", "+PAGE: LENGTH up a page", "a new page is a copy")
put("B_OCTDN", "SET", "OCT-: not used on SET", "one row of four knobs")
put("B_OCTUP", "SET", "OCT+: not used on SET", "one row of four knobs")
put("B_OCTDN", "SCALE", "OCT-: output octave down", "-2 to +2, all channels")
put("B_OCTUP", "SCALE", "OCT+: output octave up", "-2 to +2, all channels")
put("B_OCTDN", "REC", "OCT-: keyboard octave down", "same as K4 OCT")
put("B_OCTUP", "REC", "OCT+: keyboard octave up", "same as K4 OCT")
put("B_OCTDN", "RESET", "RST R1: reset the ROW 1 morphs", "NOTE EDIT: step octave to AUTO")
put("B_OCTUP", "RESET", "RST R2: reset the ROW 2 morphs", "NOTE EDIT: step octave to AUTO")
# ---- encoders -----------------------------------------------------------------------------------------------------
put("EN_SELECT", "PERF CHAN SET SCALE RESET RIG", "SELECT: pick CH1 to CH4", "the channel the knobs act on")
put("EN_SELECT", "NOTE_EDIT", "SELECT: pick a channel", "this closes NOTE EDIT")
put("EN_SELECT", "REC", "SELECT: pick a channel", "the cursor returns to step 1")
put("EN_ALGO", "PERF NOTE_EDIT CHAN SET SCALE REC", "ALGO: this channel's algorithm", "ACID EUCL SNAK STAG ARP PATT")
put("EN_ALGO", "RESET", "ALGO: not used in RESET", "EDIT cancels RESET")
put("EN_PRESET", "PERF NOTE_EDIT CHAN SET SCALE RIG", "PRESETS: pick a slot P01-P16", "loads in 1.5 s; SAVE = save here")
put("EN_PRESET", "REC", "PRESETS: not used in REC", "tap REC to finish")
put("EN_PRESET", "RESET", "PRESETS: not used in RESET", "EDIT cancels RESET")
# ---- the four knobs -----------------------------------------------------------------------------------------------
for n in "1234":
    k = "EN_K" + n
    put(k, "PERF", "K%s: %s (ROW 1)" % (n, "{r1}"), "OCT+: {r2}; twice: device")
    put(k, "RESET", "K%s: back to its default" % n, "turn one detent to reset it")
put("EN_K1", "NOTE_EDIT", "K1: PITCH of this step", "one scale degree per detent")
put("EN_K2", "NOTE_EDIT", "K2: GATE of this step", "10-100 %, the top value is TIE")
put("EN_K3", "NOTE_EDIT", "K3: VELO of this step", "100 or more is an accent")
put("EN_K4", "NOTE_EDIT", "K4: RATCH of this step", "repeats inside the step, 1x-4x")
put("EN_K1", "CHAN", "K1: LENGTH of this channel", "1-64 steps; shrinking only hides")
put("EN_K2", "CHAN", "K2: MIDI channel it sends on", "1-16, or OFF")
put("EN_K3", "CHAN", "K3: SOURCE of the viewed page", "which notes this page plays")
put("EN_K4", "CHAN", "K4: FOLLOW the playhead", "the view turns with the page")
put("EN_K1", "SET", "K1: BPM, your own tempo", "inert while a host clock runs")
put("EN_K2", "SET", "K2: SWING, odd steps late", "50 is straight")
put("EN_K3", "SET", "K3: GATE, the note length", "50 % is neutral")
put("EN_K4", "SET", "K4: DIV, the step length", "applies to every channel")
put("EN_K1", "SCALE", "K1: SCALE for every channel", "all pitches snap to it")
put("EN_K2", "SCALE", "K2: KEY, the root note", "or just press a key")
put("EN_K3", "SCALE", "K3: OCTAVE of the output", "also OCT- / OCT+")
put("EN_K4", "SCALE", "K4: not used on SCALE", "nothing global is left")
put("EN_K1", "REC", "K1: CURSOR, the write position", "stopped: the step sounds")
put("EN_K2", "REC", "K2: ACC on written notes", "harder play is louder")
put("EN_K3", "REC", "K3: SLIDE on written notes", "ON glides into the next step")
put("EN_K4", "REC", "K4: OCT of the keyboard", "also OCT- / OCT+")
put("MASTER", ALL, "MASTER: preview and USB volume", "no effect on MIDI out")
# ---- keys ---------------------------------------------------------------------------------------------------------
WHITE = "F3 G3 A3 B3 C4 D4 E4 F4 G4 A4 B4 C5 D5 E5 F5 G5".split()
BLACK = "F#3 G#3 A#3 C#4 D#4 F#4 G#4 A#4 C#5 D#5 F#5".split()
for i, nm in enumerate(WHITE, 1):
    c = "KEY_W%02d_%s" % (i, nm)
    put(c, "PERF CHAN SET", "Step %d: tap = ON / OFF" % i, "hold 0.4 s: edit that note")
    put(c, "NOTE_EDIT", "Step %d: tap = ON / OFF" % i, "hold: move NOTE EDIT here")
    put(c, "SCALE", "Key %s: sets the KEY" % nm, "the root of the scale")
    put(c, "REC", "Key %s: plays and writes it" % nm, "at the cursor or nearest step")
    put(c, "RESET", "Step %d: CLR its edits" % i, "back to the generator")
MOVE = {
    5: ("CHN: tap = settings, hold = RIG", "LENGTH MIDI | the device knobs"),
    6: ("SOL: hold = only this channel", "release restores the mutes"),
    7: ("FIL: hold = a denser pattern", "press harder = denser"),
    8: ("ROL: hold = repeat every step", "press harder = 2x 3x 4x"),
    9: ("PG-: view the page before", "turns FOLLOW off"),
    10: ("PG+: next page, or add one", "on the last page PG+ twice adds"),
    11: ("RST: all channels to step 1", "at the next step"),
}
for i, nm in enumerate(BLACK, 1):
    c = "KEY_B%02d_%s" % (i, nm)
    if i <= 4:
        put(c, "PERF NOTE_EDIT CHAN SET RIG", "M%d: mute / unmute CH%d" % (i, i), "its phase keeps running")
    else:
        put(c, "PERF NOTE_EDIT SET RIG", MOVE[i][0], MOVE[i][1])
        put(c, "CHAN", *(MOVE[i] if i != 5 else ("CHN: back to PERF", "tap again to leave CHAN")))
    put(c, "SCALE", "Key %s: sets the KEY" % nm, "the root of the scale")
    put(c, "REC", "Key %s: plays and writes it" % nm, "at the cursor or nearest step")
    if i == 5:
        put(c, "RESET", "COPY P: re-copy the page before", "edits and source; EDIT = undo")
    else:
        put(c, "RESET", "Key %s: nothing to reset" % nm, "EDIT cancels RESET")

# ---- P2-012 HARMONY: the SCALE table page is the HARMONY screen; these override the SCALE / SET rows above -------------
put("B_GLO", "PERF NOTE_EDIT CHAN", "GLO: tap = SET, hold = HARM", "tempo, swing | scale, key, chords")
put("B_GLO", "SET", "GLO: back to PERF", "hold 0.4 s: switch to HARM")
put("B_GLO", "SCALE", "GLO: back to PERF", "you are on HARMONY")
put("B_SCL", "SCALE", "PROG: re-roll the chord loop", "SCL; EDIT = undo")
put("B_LFO", "SCALE", "BARS: length of the chord loop", "1-8 bars; LFO")
put("B_ARP", "SCALE", "ROLE: this channel's part", "OFF AUTO ROOT FULL; ARP")
put("B_SEQ", "SCALE", "NEW: start a new progression", "then type chords 1-7 on keys")
put("B_OCTDN", "SCALE", "OCT-: pedal tone one step down", "OFF at the bottom")
put("B_OCTUP", "SCALE", "OCT+: pedal tone one step up", "clash marks show on screen")
put("B_OCTDN", "SET", "OCT-: output octave down", "-2 to +2, all channels")
put("B_OCTUP", "SET", "OCT+: output octave up", "-2 to +2, all channels")
put("EN_K2", "SCALE", "K2: KEY, the root note", "the chords move with it")
put("EN_K3", "SCALE", "K3: VOICE, the chord size", "0 root, 2 triad, 4 ninth, 6 wide")
put("EN_K4", "SCALE", "K4: MOVE, voice-leading", "8 = root position, 0 = smoothest")
for _i, _nm in enumerate(WHITE, 1):
    _c = "KEY_W%02d_%s" % (_i, _nm)
    if _i <= 7:
        put(_c, "SCALE", "Key %s: chord %d" % (_nm, _i), "types it into the loop; sounds it")
    else:
        put(_c, "SCALE", "Key %s: not used here" % _nm, "the key is K2")
for _i, _nm in enumerate(BLACK, 1):
    put("KEY_B%02d_%s" % (_i, _nm), "SCALE", "Key %s: not used here" % _nm, "the key is K2")
put("B_EDIT", "PERF CHAN SET SCALE", "EDIT: tap = undo / redo", "also a HARM change; hold = RESET")

# ---- hardware support model (P2-019): the RIG page; these override the rows above for the RIG page -----------------------
put("B_EDIT", "RIG", "EDIT: undo the last dice", "seeds only; knobs by hand")
put("B_OCTDN", "RIG", "OCT-: RIDE one morph down", "the wire's MOVE follows that morph")
put("B_OCTUP", "RIG", "OCT+: RIDE one morph up", "0 = none; the wire's MOVE is fixed")
put("EN_ALGO", "RIG", "ALGO: pick device and wire set", "TB-3 BITE/WET/PADS/PUSH; in 0.6 s")
put("EN_K1", "RIG", "K1: PARAM this wire drives", "a name; the palette lists all")
put("EN_K2", "RIG", "K2: LAYER, the dice that re-rolls", "NOTES HITS ACCNT ARTIC SHAPE, none")
put("EN_K3", "RIG", "K3: FORM of the moving value", "RAND WALK RAMP ACC | DICE BAR")
put("EN_K4", "RIG", "K4: MOVE, how far SEQ goes", "0 = still; a select: its spread")
for _i, _nm in enumerate(WHITE, 1):
    _c = "KEY_W%02d_%s" % (_i, _nm)
    if _i <= 4:
        put(_c, "RIG", "Key %s: wire %d, who drives" % (_nm, _i), "DEV > HAND > SEQ; hold = select")
    else:
        put(_c, "RIG", "Key %s: not used here" % _nm, "keys 1-4 are the wires")
put("KEY_B05_D#4", "RIG", "CHN: back to PERF", "tap again to leave the RIG page")
put("KEY_B09_C#5", "RIG", "PG-: not used on the RIG page", "there is no lane to page")
put("KEY_B10_D#5", "RIG", "PG+: not used on the RIG page", "there is no lane to page")

if __name__ == "__main__":
    cm = list(csv.DictReader(open(os.path.join(D, "control_map.tsv")), delimiter="\t"))
    ctls = []
    for r in cm:
        if r["control"] not in ctls:
            ctls.append(r["control"])
    miss = [(c, p) for c in ctls for p in PAGES if (c, p) not in H]
    if miss:
        raise SystemExit("missing: %r" % miss[:10])
    extra = [k for k in H if k[0] not in ctls]
    if extra:
        raise SystemExit("unknown control: %r" % extra[:5])
    with open(os.path.join(D, "help_text.tsv"), "w", newline="") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["control", "context", "line1", "line2"])
        for c in ctls:
            for p in PAGES:
                w.writerow([c, p] + list(H[(c, p)]))
    print("help_text.tsv: %d rows" % (len(ctls) * len(PAGES)))
