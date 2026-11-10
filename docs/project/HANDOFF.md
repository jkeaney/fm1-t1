# Handoff: current state of the project (read this first after a context reset)

Last updated 2026-11-10 by the orchestrator (project paused). If anything here disagrees with a task file, the task file and
`python3 tools/backlog.py list` win. Decisions live in `docs/project/OWNER_QUESTIONS.md` (the decision tables at the end).

## What we are building
A UX-first **sequencer firmware** for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules in
`docs/project/VISION.md` (rule 3 amended: a channel has a MIDI channel, an algorithm and a pattern length). The
device boots straight into the new sequencer UI and works immediately. The groovebox is out of the v1 UI (decision 32b).
Branch: `t1-hardware-bringup`, remote `origin` = jkeaney/fm1-t1. Pushed through `6bdf675`; see PAUSED below.

## How work runs (do not skip)
- I (the main session) am the **orchestrator**: I never implement. One task at a time via the backlog
  (`python3 tools/backlog.py next|list|show|set|check|index`), executor subagent `fm1-task-executor`, then an
  independent `fm1-task-reviewer`, then commit. Process: `docs/project/PROCESS.md`; lessons: `docs/project/LESSONS.md`.
- **Model routing (owner):** planning/design tasks run on **Sonnet**; development tasks on **Haiku**. Opus only when the
  owner asks (used for P1-012 UX unification and the P1-017 sequencer UX lock + its independent review).
- **Design lock (ADR-0004):** no dev task is ready until the design it implements is locked (fully specified, prototyped
  with golden outputs, validated by scenario, owner-approved). Designs are data tables so workflow changes are table edits.
- Human gates (`human-review`, `hardware-test`) stop at `review` until the owner writes/approves "Approved:". The
  reviewer never writes it. I present decisions with the `/open-items` format (numbered, lettered options, one
  recommendation, "Reply like: 1A, 2B").
- **Always ask before flashing or pushing** (the owner has approved pushes of this branch in each round; ask again each time).
- Watchdog: a session-only cron job (every 11 min) checks the backlog and nudges silent agents. It dies with the session:
  recreate it with CronCreate if needed (prompt text: check `tools/backlog.py list` and `git log`, nudge agents silent > 15 min,
  dispatch the next ready task, stop at human gates/flash/push, delete itself after 3 idle checks).
- Gotchas: macOS BSD sed (use python for edits); quote shell patterns containing arrows or `>` (they create stray files);
  `pgrep -f` in a wait loop matches its own shell; no foreground `sleep`; downloaded code is read-only, never executed.

## PAUSED (2026-11-10): work is stopped on purpose, resume from "How to resume"
The owner paused the programme. No agent is running, the watchdog cron is deleted, nothing is half-edited. Commits through
`96e0715` are local except the last push (`6bdf675`); push needs the owner's OK (agreed: push after P2-011 approval).

## Update 2026-11-10 (resumed)
P2-007 and P2-011 are approved and done, and the branch is pushed (166ebcd). P2-012 HARMONY is dispatched (Sonnet).
Owner decisions 79-84 are recorded: 16 preset slots with the fx-bus removal first in E3; the research picks the PATTERNS categories
(famous-song and genre patterns, copyright option at review); six v1 algorithms with E5 split; Ableton Sync check plus P0-005
on 2026-11-11; new P2-015 keybed use cases. Order: P2-012 -> P2-013, P2-014 and P2-015 in parallel -> P2-008 -> P2-005.

## Where we are
Done: P0-001..004, P1-001..015, P1-017 (sequencer UX locked, owner-approved), P2-001, P2-002, P2-003 (screen spec,
approved), P2-004, P2-006 (T-1 issues triaged: BUG-001/002 tracked, 14 dropped), P2-009, P2-010 (timing + clock design,
approved). P1-016 dropped.

**At `review`, waiting for the owner's sign-off (approve together):**
- P2-007 Scenario walkthroughs: independently reviewed, PASS (01252dd).
- P2-011 Apply the P2-007 findings (19 defects fixed or decided): committed 96e0715, executor checks all green, but it has
  NOT had its independent review yet (dispatch `fm1-task-reviewer`, Sonnet; scope: the 19 FINDINGS statuses, the executor's
  extra rules listed in its Log: MIDI commit after 0.6 s still, H4 without `CH`, page-add sets FOLLOW off, row renames 10a/10b/5a).
Owner then writes the Approved line on both (or says approve and the orchestrator writes it as before).

**New design work added while paused (owner request 2026-11-10), not started:**
- P2-012 HARMONY: global key, scale and chord progression (1-3-5-3 style, default `1----`, dice on the progression, every channel
  quantised to the chord, morphs favour chord voicing). Depends on P2-011.
- P2-013 ARP algorithm over the chord progression (shape, direction, speed performable). Depends on P2-012.
- P2-014 PATTERNS library (top 20 each: ACID, BASS, MELODY, others) quantised to scale/chords. Depends on P2-012.
P2-008 and P2-005 now depend on all three, so the order is P2-011 review/approval -> P2-012 -> P2-013 + P2-014 -> P2-008 -> P2-005.

**Open owner question 79** (OWNER_QUESTIONS): turning PRESETS to a never-saved slot loads nothing and toasts
`P09 EMPTY: NOTHING LOADED` (recommended; owner may overrule).

## How to resume
1. Read this file, `python3 tools/backlog.py list`, `git log --oneline -5`.
2. Dispatch the independent review of P2-011 (Sonnet reviewer). If it passes, ask the owner to approve P2-007 + P2-011 and
   push (open-items format), then set both `done` with an "Approved: owner <date>" line.
3. Recreate the watchdog cron (see "How work runs") only if wanted.
4. Next tasks in order: **P2-008** algorithm reference prototypes with golden outputs (Sonnet, human-review; ready now) ->
   **P2-005** break P3-P6 into Haiku dev tasks (Sonnet, human-review). P2-005 MUST include: a first dev task linking real
   T-1/sequencer sources into host tests; RAM headroom reported on every dev review; hardware-test gates with a TB-3
   checklist (steps/notes/slides/accents/morph/dice); the timing test image and measurement tool from TIMING_SPEC (separate
   build, jitter thresholds, Ableton + TB-3 sessions); clock out + AUTO tempo source + blinking clock dots (FB-001, close
   FB-001 when its E4 dev task exists); BUG-001/BUG-002 (phase P3) scheduled; the fx-bus removal before preset storage.
   Then E3..E8, then E9 (v2 cleanup; pull forward if RAM headroom < 8 KB).
5. Owner-only: P0-005 (TB-3 listening test; deferred until the first build task is ready); the Ableton Sync-on check
   for the JIANT FM port (the likely cause of the loose recorded timing; unconfirmed until measured).

## Owner decisions to remember (full list: OWNER_QUESTIONS.md)
4 channels; 4 v1 algorithms ACID/EUCLID/SNAKE/STAGE (ARP, RHYTHM, MELODY deferred, specs complete); each algorithm has up to
8 morphs (K1-K4 plus a row switch on OCT-/OCT+); five shared dice layers plus ALL; B2 button layout (printed-word fit);
one global scale/key and global GATE/DIV/SWING/tempo on the SET page; no scenes, no tap tempo; keybed force sensing
(Velo/CTL-1: velocity + poly aftertouch, F3-C5 only, LEDs disturb readings) is IN v1 with fallbacks and a hardware-test
gate; step-centric editing (hold a step -> NOTE EDIT page; first 16 duplicated into later pages up to 64 steps); screen
labels + hold-HOME help (no overlay strip); ADR-0003 answers 43-47 settled (swing doubled, GATE 50% neutral, preset load
on a 768-tick bar, one undo record per channel, no note cut on algorithm/MIDI-channel change); tempo source AUTO (internal default, a heard MIDI clock takes over, clock out automatic when master, K1 inert in EXT, PLAY on EXT waits, blinking colour-coded clock dots); timing test is a separate firmware build; MIDI channels unique across the 4 channels (taking a used one sets the other to OFF); setup functions may take 2 gestures; CH1-CH4 default MIDI 1-4 with a first-run 'D#4 K2: MIDI CHANNEL' step; memory trigger: if RAM
headroom < 8 KB or the ram stage fails, pull E9 (cleanup) forward.

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
