# Handoff: current state of the project (read this first after a context reset)

Last updated 2026-11-10 by the orchestrator (Opus planning session). If anything here disagrees with a task file, the task file and
`python3 tools/backlog.py list` win. Decisions live in `docs/project/OWNER_QUESTIONS.md` (the decision tables at the end).

## What we are building
A UX-first **sequencer firmware** for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules in
`docs/project/VISION.md` (rule 3 amended: a channel has a MIDI channel, an algorithm and a pattern length). The
device boots straight into the new sequencer UI and works immediately. The groovebox is out of the v1 UI (decision 32b).
Branch: `t1-hardware-bringup`, remote `origin` = jkeaney/fm1-t1. Pushed through `6bdf675`; see PAUSED below.

## START HERE (2026-11-10): a Sonnet orchestrator runs the plan
- **Runbook:** docs/project/ORCHESTRATOR.md (loop, parallelism, Haiku-to-Sonnet escalation, gates, cost rules). **Map:**
  docs/project/PROGRAM_PLAN.md (every phase P2-P8, the critical path, budgets, risks). Every task to the release flash
  (P8-004) exists in backlog/ with Definition-of-Ready content; `python3 tools/backlog.py next` is always the next step.
- Model routing: the orchestrator session runs on Sonnet; executors follow each task's `executor` field (mostly Haiku);
  reviewers are always Sonnet. Opus only when the owner asks.
- Gotchas: macOS BSD sed (use python for edits); quote shell patterns containing arrows or `>`; no foreground `sleep`;
  downloaded code is read-only; parallel agents `git add` only their own files.

## Where we are (2026-11-10, end of the Opus planning session)
- Done: P0-001..004, P1 (all), P2-001..004, P2-006, P2-007, P2-009, P2-010, P2-011. E3-E8 broken down (closed). FB-001 closed.
- P2-012 HARMONY approved and done (85A 86A 87B 88A 89A 91A; the HARMONY dial is named `DRONE`). P2-005 program plan approved and done.
- **Ready now:** P2-013 ARP, P2-014 PATTERNS, P2-015 keybed (Sonnet, run in parallel); P2-008 prototype harness + ACID (Sonnet);
  P3-001 host tests link the T-1 (Sonnet, the first dev task: run it alone); P4-008 timing tool (Haiku, may run alongside).
  Suggested first wave: P2-013 + P2-014 + P2-015 in parallel, then P3-001 + P4-008, then P2-008.
- Owner, 2026-11-11: Ableton Sync check on the JIANT FM port plus P0-005 TB-3 listening test on the current build.
- Last push: 6f91357 (2026-11-10). Ask before every push.

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
Build line (today): RAM 55744 of 73728 (17984 B free before the 4 KB floor); pool 342876 of 368640. After the planned
removals RAM headroom ~46 KB. The pool is the tight resource (preset slots: 16 x 1388 B = 24984 B vs 780 B spare today; the
fx-bus removal must precede preset storage or v1 ships 8 slots).

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
