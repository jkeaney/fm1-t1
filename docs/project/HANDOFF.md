# Handoff: current state of the project (read this first after a context reset)

Last updated 2026-11-10 by the orchestrator. If anything here disagrees with a task file, the task file and
`python3 tools/backlog.py list` win. Decisions live in `docs/project/OWNER_QUESTIONS.md` (the decision tables at the end).

## What we are building
A UX-first **sequencer firmware** for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules in
`docs/project/VISION.md` (rule 3 amended: a channel has a MIDI channel, an algorithm and a pattern length). The
device boots straight into the new sequencer UI and works immediately. The groovebox is out of the v1 UI (decision 32b).
Branch: `t1-hardware-bringup`, remote `origin` = jkeaney/fm1-t1. Everything through P1-017's first pass is pushed.

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

## Where we are
Done: P0-001..004 (commit, green tests, `tools/check.sh`, hardware test tool), P1-001..015 (inventories, 7 algorithm
assessments, ALGORITHM_SET, UX_REVIEW, FULL_CONTROL, PERFORMANCE_DESIGN (B2), KEYBED reference), P2-001 (CONTROL_MAP),
P2-002 (SYSTEM_MAP), P2-004 (ADR-0003 sequencer core), P2-009. P1-016 dropped (superseded by P1-017).

**In flight: P1-017 revision** (Opus executor): sequencer functionality + UX lock (`docs/design/SEQUENCER_UX.md`, TSV tables
page_states/button_modes/knob_map/key_map, `tools/check_sequencer_ux.py`). The first Opus review said "close, not lockable" with
six fixes (exact page copy via per-page source, honest L scoring with hold labels on PERF, save-to-slot via pending slot,
slides across the loop end, HOME stays tap=home/hold+touch=HELP/release 0.7 s=MENU per decisions 57/61, small items
incl. CLAUDE.md rule 3). After the revision: independent Opus re-review, then the owner's gate.

Next, in order: P1-017 owner approval -> P2-003 (screen spec) -> P2-006 (triage T-1 issues F1-F19; ready now) ->
P2-007 (scenarios) -> P2-008 (algorithm prototypes + golden files) -> P2-005 (break P3-P6 into Haiku tasks; must
include: host tests link real T-1/sequencer sources first, hardware-test gates with a TB-3 checklist for steps/notes/
slides/accents/morph/dice, RAM headroom reported on every dev review) -> E3..E8 -> E9 (v2 cleanup).
P0-005 (listen test on the TB-3) is the owner's, deferred until the design work is done.

## Owner decisions to remember (full list: OWNER_QUESTIONS.md)
4 channels; 4 v1 algorithms ACID/EUCLID/SNAKE/STAGE (ARP, RHYTHM, MELODY deferred, specs complete); each algorithm has up to
8 morphs (K1-K4 plus a row switch on OCT-/OCT+); five shared dice layers plus ALL; B2 button layout (printed-word fit);
one global scale/key and global GATE/DIV/SWING/tempo on the SET page; no scenes, no tap tempo; keybed force sensing
(Velo/CTL-1: velocity + poly aftertouch, F3-C5 only, LEDs disturb readings) is IN v1 with fallbacks and a hardware-test
gate; step-centric editing (hold a step -> NOTE EDIT page; first 16 duplicated into later pages up to 64 steps); screen
labels + hold-HOME help (no overlay strip); ADR-0003 answers 43-47 settled (swing doubled, GATE 50% neutral, preset load
on a 768-tick bar, one undo record per channel, no note cut on algorithm/MIDI-channel change); memory trigger: if RAM
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
See the tables in OWNER_QUESTIONS.md (rows 65-71 from P1-017, plus anything the revision adds). The owner asked for the
sequencer UX to be locked at a high bar: easy, no deep learning, every state a page, every button has modes, every
screen shows what the knobs control.
