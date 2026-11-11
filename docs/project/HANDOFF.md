# Handoff: current state of the project (read this first after a context reset)

Last updated 2026-11-10 by the orchestrator (Opus planning session). If anything here disagrees with a task file, the task file and
`python3 tools/backlog.py list` win. Decisions live in `docs/project/OWNER_QUESTIONS.md` (the decision tables at the end).

## What we are building
A UX-first **sequencer firmware** for the M-VAVE FM-1 (fork of Felucca 1.6 via tensor-programming/fm1-t1). Rules in
`docs/project/VISION.md` (rule 3 amended: a channel has a MIDI channel, an algorithm and a pattern length). The
device boots straight into the new sequencer UI and works immediately. The groovebox is out of the v1 UI (decision 32b).
Branch: `t1-hardware-bringup`, remote `origin` = jkeaney/fm1-t1. Pushed through `6bdf675`; see PAUSED below.

## UPDATE 2026-11-11 (late): newest state; read this first, then the section below it

Done since the section below was written (all committed; branch pushed to origin at c8adb2b, later commits local):
- Design, all owner-approved: P2-021 (closed), **P2-022 modulation layer** (docs/design/MODULATION.md, decision 108; supersedes the RIG/ROW 3 wire design), **P2-023..P2-028 preview / CUE** (docs/design/PREVIEW.md, decisions 109-113): per-channel PREVIEW gate independent of the MIDI gate (mute = MIDI gate only), presets ACID LEAD BASS PAD STRINGS PIANO (PIANO = ANALOG e-piano, so no sample/FM6 engine kept), all channels preview OFF by default, MASTER pot = preview level, hold mute key 0.4 s = preview on/off, hold + ALGO = sound, one shared mix on the master output (speaker, rear headphone jack; headphones auto-mute the speaker [V on owner's unit]). TB-3 TUNING (CC104) = pitch offset on the synth. VISION rule 3 reworded.
- Code removal chain: P3-001, P3-002 (slicer), BUG-003 (test-harness race, not firmware), P3-014 (fx buses; pool 342876 -> 192808), P3-003 (DIGITAL engine) all done and reviewed. Build line now RAM .data+.bss 55248 of 73728 (18480 free), pool 192808.

In flight / next:
- **P3-004 (web editor removal) is blocked** on a spec gap; Sonnet planner ae32209c6c22164dc is rewriting it (FB-005), rehearsing in a scratch copy, and statically auditing P3-005..P3-012 for the same gaps. If it is gone: re-dispatch a Sonnet planner on backlog/tasks/P3-004 + feedback FB-005, then the Haiku executor on P3-004.
- Then P3-005, P3-006 (sonnet), P3-007, P3-008..P3-012 (P3-008..P3-010 are now clear: ANALOG only), BUG-001/BUG-002, then P3-013 (HW-0, owner flashes).
- P2-008 (prototype harness, Sonnet) is ready but waits so test runs never overlap; then P2-016/P2-017 (must carry the mod-lane hooks and the MIDI/PREVIEW gates).
- Not yet scheduled: ADR-0003 amendment for the four ARP core additions; P4 tasks need updating for MODULATION.md and PREVIEW.md effects (listed in those docs).
- Owner items still open: Ableton Sync check on JIANT FM (decision 83) goes in the HW-0 session; H-MOD1..H-MOD4 by-ear tests later. Ask before any push or flash.

Lessons: run only ONE check.sh / run_tests at a time (concurrent runs stall); spec gaps go to a Sonnet planner, never to the owner; after a design change, grep for stale claims in ADR-0003/SEQUENCER_UX (reviewers caught several).

## UPDATE 2026-11-11 (end of the Sonnet orchestrator's first session; read this before the older text below)
- **Done this session:** P0-005 (TB-3 listening test, owner signed off), P2-013 ARP, P2-014 PATTERNS, P2-015 keybed (UC5 parked to v2), P3-001, P4-008, and the whole hardware-support design: P2-018 (high level, Option B), P2-019 (low level), P2-020 (TB-3 profile). Decisions 92-106 are in OWNER_QUESTIONS.md.
- **Measured on the owner's TB-3 (ch2, USB, via tools/fm1_bridge.py or direct rtmidi):** velocity >= 100 accents (flat velocity is now 90, decision 104); slide = overlap, accent on a slide destination is inaudible (105); all 11 CCs work incl. 68 scatter type (a mode SELECT, linked to 69 depth) (106). FM-1 runs `FM-1_900`, port "JIANT FM", no reflash was needed.
- **In flight when this was written:** (a) planner agent `af7b6636e49f19387` fixing P3-002/P3-014 scope (FB-002 done; FB-003 open; it runs scratch host tests; uncommitted: backlog/feedback/FB-003-*.md and P3-002 log). If it is gone, re-dispatch a Sonnet planner on FB-003 with a compile-driven audit (see FB-003 and its FB-002 log). (b) designer `a025a0b5f0ad4886f` writing P2-021 (TB-3 profile v2: select kinds, linked params, which-of-11 alignment UX); then review it and bring the owner a sheet.
- **Next:** P3-002 (slicer) -> P3-014 (fx buses) -> P3-003.. (removal chain, one task at a time), P2-008 prototype harness + ACID, P2-016/017 prototypes (ARP/PATTERNS and the hardware lanes need hooks, see P2-019 section 6), P4 timing tools. Ask the owner before every push (nothing pushed this session; branch is ahead of origin).
- **Lessons:** never let two `tools/check.sh` run at once (shared build/); a Haiku task blocked on missing callers is a spec gap, route to a Sonnet planner not an escalation; hardware facts are tested live with rtmidi + `say` announcements and logged as decisions.

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
