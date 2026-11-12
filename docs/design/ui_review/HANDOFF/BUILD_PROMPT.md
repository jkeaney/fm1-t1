<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Build prompt: paste into a new Claude Code session opened in the repo root

```
You are picking up the FM-1 sequencer firmware after the screen design was LOCKED. Repo:
/Users/jameskeaney/Dev/apps/FM-1 - T1 - Bassline, branch t1-hardware-bringup. The design commit is 8864557
("Screen design v2: locked"). Nothing is pushed. Work through the backlog as CLAUDE.md says; never flash or push
without asking the owner.

READ FIRST, in order
1. CLAUDE.md, docs/project/HANDOFF.md (last section), docs/project/PROCESS.md, docs/project/UX_GATE.md.
2. docs/design/SCREEN_SPEC_V2.md: the locked spec. This is the source of truth for the main screen.
3. docs/project/decisions/ADR-0006-screen-architecture-v2.md and ADR-0007-algorithm-ui-descriptor.md.
4. docs/design/features/P4-026-screen-v2-main-screen.md (use case, Gate A done, "Design locked: James Keaney 2026-11-12").
5. docs/project/OWNER_QUESTIONS.md rows 121-126 (they supersede 116-118; 119 stands).
6. The reference implementation: docs/design/ui_review/proposals/v12/ (v12.py draws every state with the REAL firmware
   fonts and palettes through the mock tool; algos_v12.py = the per-algorithm registry; icons_v12.py = the 1-bpp icons;
   out/v12_<scene>.png = the golden renders; out/S_v12_states_1.png, S_v12_states_2.png = contact sheets). The mock
   tool and its commands: docs/design/ui_review/mock/README.md (render: python3 ../../mock/mock.py render v12.py --out out).
   v12.py imports ../v5/el.py (painter) and ../v7/icons_v7.py (base icons): keep those folders.

THE NEW DESIGN IN ONE PAGE (details in SCREEN_SPEC_V2)
- One main screen, six bands in panel order (y px): header 1-21 | knob tiles 24-76 | status line 77-91 | music row
  92-141 | 2x6 button picture 146-180 | keybed footer 186-236. Horizontal grid = 16 columns of 15 px = the 16 white keys.
  Firmware strips 0-119 / 120-239; no text crosses y 120.
- Header: transport icon; tempo (INT = AF_M BPM; external = AF_S "EXT" over BPM / WAIT / red "--" when lost); one
  chip per channel IN USE (selected = filled + white bar; muted = struck grey; MIDI OFF = red cap; "+" = SELECT past the
  last channel adds one); the selected channel's algorithm short name in its colour; "MIDI n" (inverted red "MIDI OFF").
- Message slot = the header from x104 to 239: toasts ([channel chip] + result + how to undo), touched-control names
  (SELECT/ALGO/PRESETS/OCT/held SOLO FILL ROLL, boxed printed name + what it does now), hold hints (held button icon +
  its hold action + a bar that fills to the fire time), the CHAN page title. Transport, tempo and chips stay visible.
  Every string is in MESSAGES and must fit (check_messages()).
- Knob tiles K1-K4: the tile is the meter (fill + full-colour level edges); value lists (<= 10 names) get position dots
  instead; ONE font per knob for its whole range; turning = the tile frames itself + white ticks at the old level.
  REC band CURSOR ACC SLIDE OCT; CHAN band LENGTH MIDI SOURCE FOLLOW; ROW 3 = the RIG device's wire-set names.
- Status line: preset (inverted "P02?" when pending) | "ROW" + numbered boxes | "PAGE v/n" (FOLLOW off: boxed + "PLAY p").
- Music row: the SELECTED channel full width (bar height = pitch, accent = white cap, slide bridge, hatched = just
  diced, hollow = muted / red hollow = MIDI OFF); the other channels as 3 px hit strips in a FIXED row per channel number;
  playhead = grey column + white pointer; past LENGTH hatched; REC cursor frame.
- Buttons: icons only. A die frame = re-roll and its inside names the layer (FX ALL, SCL NOTES, ENV ACCNT, LFO SHAPE,
  ARP ARTIC, SEQ HITS); EDIT undo arrow, GLO cog, HOME house, SAVE floppy, PLAY, REC ring/red disc. A layer the
  algorithm lacks is drawn dim. Pressed = inverted.
- Keybed footer: white keys = the selected channel's steps (front lit when on, white bar under the playing step); black
  keys carry their role (mute blocks in channel colours / X when muted, CHN = MIDI glyph or BACK on CHAN, S F R, page
  arrows, restart); footer line names the mode ("KEYS = STEPS / HOLD = EDIT", REC "PLAY KEYS TO WRITE / REC = DONE",
  CHAN "K2 = MIDI / CHN = BACK"). One standing cue, inverted: "CHN KEY, K2 = MIDI  CHn" for a new channel (incl. CH1 at
  first run) or MIDI OFF, with the CHN key lit.
- Colour: HI-CON palette + new tokens T_CH1..4 (Okabe-Ito #E69F00 #56B4E9 #009E73 #CC79A7), T_CHF1..4, T_PH, T_DICE,
  T_OFF. Every state also changes SHAPE (colour never alone).
- Channels: added by SELECT past the last; removed by a long press on the channel's mute key (release < 1.5 s = PREVIEW
  on release; 1.5 s = remove, undoable). The 1.5 s timing is a design proposal the owner must sign in P4-030.
- Extensible: the screen code names NO algorithm. Algorithms describe their UI (ADR-0007): short name, lane type
  (pitched / single), value names, unit, zero word, optional value_text hook, absent layers. A build-time check proves
  every label/value/message fits.

WHAT NEEDS TO BE DONE (backlog; `python3 tools/backlog.py next` returns P4-026 first)
- P4-026 framework: bands, strip plan, palette tokens on every palette + mono fallback, HI-CON default, icon table
  generated from icons_v12.py + cv_icon blit, cost line. Replaces ui_sq.c's old Z0..Z7 zones (reuse its page enum,
  toast queue, table-driven labels).
- P4-027 AlgoDesc/MorphDesc UI fields (ADR-0007) with asserted sizes; ACID row filled; gen_ui_tables.py runs the width
  check over every algorithm row and the MESSAGES table; a dummy-algorithm host test renders with no screen-code change.
- P4-028 header, knob tiles, status line, music row. P4-029 button picture, keybed footer, message slot (priority: hold
  hint > touched control > toast > page title; hold bars timed from the TSV hold times).
- P4-030 (human-review) table changes: knob_map SELECT adds a channel; key_map M1-M4 hold timing; page_states
  PERF/CHAN/REC/FIRST_RUN screen + cue columns; help_text + widths rows; all checkers pass; owner signs the timing.
- Then P4-012 (cut-over, re-scoped to wire THIS screen; depends on P4-026..P4-030), then P4-013..P4-018 (re-pointed to
  SCREEN_SPEC_V2), then P4-020/P4-021 HW-1 on the real unit, where the accepted gaps (SCREEN_SPEC_V2 section 9: 3 px
  strips, faint playhead, knob fill reading as "selected", weak dice group) are judged at real size.
- Acceptance for every screen task: host renders match the v12 golden PNGs pixel for pixel (or each difference listed
  and approved); no per-algorithm branch in screen code; `sh tools/check.sh` ends CHECK PASS; RAM headroom >= 8 KB.

CLEAN UP (one task or one commit each; ask if unsure)
- docs/design/SCREEN_SPEC.md: add a banner at the top "superseded for PERF/CHAN/REC/overlays by SCREEN_SPEC_V2.md
  (ADR-0006)"; keep it for the pages not yet redrawn (NOTE_EDIT, SET, SCALE, RESET, HELP, MENU, RIG).
- backlog/feedback/FB-016 (legend labels vs the old SCREEN_SPEC): obsolete under v2 (no legend); set it to dropped with
  a Log line citing ADR-0006, unless the owner wants it kept.
- docs/design/ui_review/HANDOFF/00..09: historical; leave as is (00 has the LOCKED banner). Do not delete the proposals
  v1-v11 or the critiques: they are the design record. If repo size matters, ask the owner before removing *_grid.png.
- Make sure __pycache__/ stays ignored (it is untracked today); never commit it.
- docs/design/ui_review/mock/README.md: note the palette-parser fix (HI-CON was skipped, MONO mis-indexed) and that
  HI-CON is the v2 base palette.
- Owner decision 118's HOME-hold MAP is superseded (ADR-0006): check button_modes.tsv B_HOME hold still reads
  "touch = HELP, let go = MENU" and that nothing in the backlog still builds a MAP.

COMMITS
- Commit locally with explicit paths (never `git add -A`), one task per commit, with the trailers:
  Co-Authored-By: Claude <model> <noreply@anthropic.com>
  Claude-Session: <your session URL>
- If git cannot find an identity, use the repo's author (`git log -1 --format='%an <%ae>'`), per commit.
- Commit this prompt file too (docs/design/ui_review/HANDOFF/BUILD_PROMPT.md) with the first clean-up commit.
- Ask the owner before any push or flash.
```
