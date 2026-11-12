# SPDX-License-Identifier: GPL-3.0-only
"""icons_v8.py: v7's set with every stroke inside the dice at least 2 px (critique v7 P1: 1 px detail does not read at
35 cm), REC as a ring (idle) / disc (armed), a MIDI (DIN) glyph, a stop square, and the 'has a hold' corner notch."""
import os, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "v7"))
from icons_v7 import ICONS as V7, die

ICONS = dict(V7)
ICONS["dice_accnt"] = die("""
##.......
####.....
..####...
....####.
......###
....####.
..####...
####.....
##.......""")
ICONS["dice_shape"] = die("""
.........
.##......
####.....
#..##....
#...##..#
.....##.#
......###
.......#.
.........""")
ICONS["dice_artic"] = die("""
.........
#########
#########
.........
.........
...###...
...###...
...###...
.........""")
ICONS["dice_hits"] = die("""
.........
##.##.##.
##.##.##.
.........
.........
.........
#########
#########
.........""")
ICONS["rec_ring"] = """
....#####....
..#########..
.###.....###.
.##.......##.
##.........##
##.........##
##.........##
.##.......##.
.###.....###.
..#########..
....#####...."""
ICONS["midi"] = """
..#####..
.#.....#.
#.#...#.#
#.......#
#.#...#.#
#...#...#
.#.....#.
..#####.."""
ICONS["midi_s"] = """
.#####.
#.#.#.#
#.....#
#..#..#
.#####."""
ICONS["stop_s"] = """
#########
#########
#########
#########
#########
#########
#########
#########
#########"""
ICONS["play_s"] = """
##.......
####.....
######...
########.
#########
########.
######...
####.....
##......."""
ICONS["x7"] = """
##...##
###.###
.#####.
..###..
.#####.
###.###
##...##"""

# printed, icon, tap role, hold role (None = the same as tap), dice
BUTTONS = (("FX", "dice_all", "ALL", None, True), ("SCL", "dice_notes", "NOTES", None, True),
           ("ENV", "dice_accnt", "ACCNT", None, True), ("LFO", "dice_shape", "SHAPE", None, True),
           ("EDIT", "undo", "UNDO", "RESET", False), ("GLO", "settings", "SET", "HARM", False),
           ("HOME", "home", "HOME", "HLP/MN", False), ("SAVE", "save", "SAVE", "UNDO L", False),
           ("ARP", "dice_artic", "ARTIC", None, True), ("SEQ", "dice_hits", "HITS", None, True),
           ("PLAY", "play", "PLAY", None, False), ("REC", "rec_ring", "REC", None, False))

ICONS["din"] = """
...#####...
..#.....#..
.#..#.#..#.
#.........#
#.#.....#.#
#....#....#
.#.......#.
..#.....#..
...##.##..."""


# v11: ARTIC = note lengths (one long gate, three short ones); HITS = drum-notation x note heads
ICONS["dice_artic"] = die("""
.........
#########
#########
.........
.........
##.##.##.
##.##.##.
.........
.........""")
ICONS["dice_hits"] = die("""
.........
#.#...#.#
.#.....#.
#.#...#.#
.........
...#.#...
....#....
...#.#...
.........""")

from el import IC as _IC
ICONS["spkx"] = _IC["spkx"]   # the muted speaker (el.py), for the remove-channel hold
for _n in ("left", "right", "rst"):   # P4-029: the black-key glyphs PG- PG+ RST / BACK (el.py), so the firmware table has them too
    ICONS[_n] = _IC[_n]
