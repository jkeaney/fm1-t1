# Handoff: current state of the project (read this first after a context reset)

Last updated 2026-11-12 by the Sonnet orchestrator. If anything here disagrees with a task file, the task file and
`python3 tools/backlog.py list` win. Decisions: `docs/project/OWNER_QUESTIONS.md`. The previous long handoff (all the
2026-11-10/11 update blocks) is archived in `docs/project/sessions/HANDOFF-2026-11-11-archive.md`.

## What we are building
A UX-first **sequencer firmware** for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules in
`docs/project/VISION.md`. The device boots straight into the sequencer UI. Branch `t1-hardware-bringup`, remote
`origin` = jkeaney/fm1-t1, **ahead of origin by ~35 commits: nothing has been pushed since f68d3df. Ask before any push.**

## State (2026-11-12)
- **Done:** P0, P1, P2-001..015, 018..028, P2-008 (prototype harness + ACID), P3-001..014, BUG-001..005.
- **P3-013 HW-0 passed on the real FM-1 + TB-3** (2026-11-12): the cleaned build (SHA-1 fb0a4da8..., `FM-1_900`, port
  "JIANT FM") boots to the old T-1 screen, plays the TB-3 on MIDI channel 2 through `tools/fm1_bridge.py`, STOP sends
  CC123 on channels 1-5, the HOME-hold menu opens. The new UX (morph, dice, labels) is NOT in firmware yet: it arrives in
  P4-P6. Not exercised: knobs/mute, preview synth on the speaker, Ableton Sync check (decision 83).
- **The FM-1 is flashed with that build.** Ableton is open only to monitor TB-3 audio; the Python bridge does the
  FM-1 -> TB-3 routing (start it, stop it afterwards).
- Final cleaned build line: `image 168876 B; RAM .data+.bss 23060 B of 73728 (50668 free); pool 92500 B of 368640`.

## New process (2026-11-12): the UX gate
`docs/project/UX_GATE.md`. Every task a producer can touch has a feature use case in `docs/design/features/`
(`_TEMPLATE.md`): Gate A before the build (steps a producer takes, predicted ease score, five-surface sync, live-
performance checklist), Gate B after (walk it, actual score, re-check). Every open task has a `## UX gate` section and
`tools/backlog.py check` enforces it; internal work writes `n/a, <reason>`. The reviewer agent runs Gate B.

## Next
1. **P2-016** EUCLID/SNAKE/STAGE prototypes (Sonnet; first producer-facing design task: fill its use case), then
   **P2-017** ARP/PATTERNS (owner human-review locks all six prototypes).
2. **P2-029** (new): amend ADR-0003 / HARMONY.md for the P2-008 findings (ACID range -12..+36, HarmChord 14 B,
   drone_pitch). P4-001 and P4-002 now wait for it.
3. **P3-015** remove the installer page's SysEx backup flow (Sonnet, ready). P4-003 and P4-010 are also ready.
4. Not yet scheduled: ADR-0003 amendment for the four ARP core additions; P4 task updates for MODULATION.md and
   PREVIEW.md effects. By-ear tests later: H-MOD1..H-MOD4.

## Known issues / caveats
- Web emulator edits unverifiable (no Emscripten); `eng_slice.c` slc_man_load/save never set; `engine_t.mem` unused.
- CLEAN_HOUSE_REPORT's BUG-001/002 figure should read -192 B, not -208 B (minor).
- **Stray-file trap:** writing to a shell glob like `backlog/tasks/P3-013-*.md` with `>>` creates a literal file named
  with the `*`, and `git rm`/`git checkout` treat `*` as a wildcard. Resolve the real path first. Two such strays from
  earlier sessions were merged into P2-002 and P2-012 on 2026-11-12.

## Lessons
Never run two `tools/check.sh` at once (shared build/). A Haiku task blocked on missing callers is a spec gap: route it
to a Sonnet planner. Removal patches can go stale when an earlier task changes comments (use the `--3way` fallback).
Hardware facts are tested live and logged as decisions. Sonnet orchestrator runs the loop: docs/project/ORCHESTRATOR.md.

## Owner decisions to remember (full list: OWNER_QUESTIONS.md)
4 channels; 4 v1 algorithms ACID/EUCLID/SNAKE/STAGE (ARP, RHYTHM, MELODY deferred, specs complete); each algorithm has up to
8 morphs (K1-K4 plus a row switch on OCT-/OCT+); five shared dice layers plus ALL; B2 button layout (printed-word fit);
one global scale/key and global GATE/DIV/SWING/tempo on the SET page; no scenes, no tap tempo; keybed force sensing
(Velo/CTL-1: velocity + poly aftertouch, F3-C5 only, LEDs disturb readings) is IN v1 with fallbacks and a hardware-test
gate; step-centric editing (hold a step -> NOTE EDIT page; first 16 duplicated into later pages up to 64 steps); screen
labels + hold-HOME help (no overlay strip); ADR-0003 answers 43-47 settled (swing doubled, GATE 50% neutral, preset load
on a 768-tick bar, one undo record per channel, no note cut on algorithm/MIDI-channel change); tempo source AUTO (internal default, a heard MIDI clock takes over, clock out automatic when master, K1 inert in EXT, PLAY on EXT waits, blinking colour-coded clock dots); timing test is a separate firmware build; MIDI channels unique across the 4 channels (taking a used one sets the other to OFF); setup functions may take 2 gestures; CH1-CH4 default MIDI 1-4 with a first-run 'D#4 K2: MIDI CHANNEL' step; memory trigger: if RAM
headroom < 8 KB or the ram stage fails, pull E9 (cleanup) forward. 2026-11-10: 79 empty preset slot loads nothing; 80 16 slots, fx-bus removal first; 81/90 PATTERNS = research-led famous-song + genre patterns, exact notes under style names; 82 six v1 algorithms, E5 split; 84 keybed needs use cases (P2-015).

## Budget facts
Build line (measured 2026-11-11 after the P3 removals, P3-012): `image 168876-168892 B (16 B varies between builds);
RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640`. RAM headroom 50668 B (floor 4 KB, E9 trigger 8 KB); pool 276140 B free. Before the removals (P3-001 log):
`image 523224 B; RAM 55744 B; pool 342876 B`. The per-step figures and the predicted-versus-measured table are in
docs/design/CLEAN_HOUSE_REPORT.md. The pool is no longer the tight resource; preset storage (P7-002 replaces the FUN10 record)
should be sized against the 276 KB free pool, not the old 780 B spare.

## Hardware / lab state (2026-11-10)
- FM-1 is flashed with the fixed bassline build (`FM-1_900`, port "JIANT FM" for Felucca builds; stock V15 shows as `FM-1_015`
  on port "FM-1"). Factory restore: `restore/FM-1-official-V15.fwsc` (gitignored); after a restore power-cycle the device.
  Install: `tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes` (the loader appears as "JIANT Update").
- The T-1 sends MIDI on channels 2-6 (track 1 is the internal synth); the TB-3 default receive channel is 2. Ableton routing
  that works: External Instrument track, MIDI From JIANT FM ch 2, MIDI To TB-3 ch 1 (set TB-3 receive channel to match);
  stop the TB-3's own sequencer; Ableton Sync for the TB-3 port is Off.
- The old Python bridge `tools/fm1_bridge.py` is stopped (Ableton replaces it).

## Open owner questions
Decisions are in OWNER_QUESTIONS.md (rows 65-78 decided; row 79 open). The owner asked for the
sequencer UX to be locked at a high bar: easy, no deep learning, every state a page, every button has modes, every
screen shows what the knobs control.
## SCREEN DESIGN IS OPEN: read docs/design/ui_review/HANDOFF/00_START_HERE.md (2026-11-12)
The 30 mm, 240x240 screen has been through four mock-up rounds (v1, v3, v3.1, v3.2); the owner judged v3.2 "not good
enough" without saying why, and decision 120 (freeze) is withdrawn. A handoff package for a Claude design session to
own it is in `docs/design/ui_review/HANDOFF/` (start with `00_START_HERE.md`; paste-in prompt in `SESSION_PROMPT.md`).
**Hazard:** `python3 tools/backlog.py next` returns P4-012, which would wire the OLD dense layout into the new core:
do not run it before the owner decides (block it, or split the cut-over: HANDOFF `07_ACCEPTANCE.md` section D).

