# Handoff: current state of the project (read this first after a context reset)

Updated 2026-11-13 (project review and clean-up before a fresh session). If this disagrees with a task file, the task
file and `python3 tools/backlog.py list` win. Decisions: `docs/project/OWNER_QUESTIONS.md`. Older handoffs:
`docs/project/sessions/HANDOFF-*-archive.md`. **Update this file at the close of every `/next-task` cycle.**

## Where we are
The proof of concept is done. HW-1 (first flash of the new sequencer core) was signed off by the owner on 2026-11-13:
the unit boots to the new screen and plays the TB-3 on channel 2 through the bridge. Branch `t1-hardware-bringup`:
**pushed to origin up to e9f5671; later commits are local only.** Ask before any push or flash.

Goal and rules: `docs/project/VISION.md` and CLAUDE.md. In short: a UX-first sequencer for the FM-1; channels send MIDI
out, each channel has an algorithm; knobs morph, buttons dice layers; no menu diving.

## What is built (all in firmware/src, one unity build in felucca.c)
- Core, pure and host-tested: `sq_types.h` (data model, locked sizes), `sq_prims.c` (shared primitives SP1.. incl. the
  xorshift32 RNG), `sq_algo_acid.c` + `sq_algos.c` (the `ALGOS[]` table: only ACID is filled; other rows are inert),
  `sq_core.c` (scheduler, event queue, `sq_block` in the audio ISR), `sq_chan.c` (channel state API, morph, dice, undo,
  in-use mask, RAM presets), `clk_ext.c` (clock source model; not yet fed into transport, P4-031).
- Screen: `ui_sq.c` (SCREEN_SPEC_V2: header, knob tiles, music row, button tiles, keybed footer, message slot) and
  `ui_sq_glue.c` (controls and the per-frame sync). Pages built: PERF, NOTE_EDIT, CHAN, SET, REC, plus the overlays
  RESET, HELP and FIRST_RUN, ADD PAGE and ALGO_PICK. Screen code names no algorithm: algorithms describe their own UI
  (ADR-0007). Screen design is LOCKED (SCREEN_SPEC_V2, ADR-0006/0007, renders in docs/design/ui_review/proposals/v12/).
- Power-on: CH1 in use (ACID, MIDI 1); CH2..CH4 are not in use, muted, MIDI OFF. A channel is added by turning SELECT
  past the last one, but only for an algorithm row that is built, so nothing can be added until EUCLID (P5-002).
- Not built yet: the other five algorithms, HARMONY, MOD/RIG, ROW 3, LED mapping (LEDs still show legacy state),
  presets/settings in flash, external clock follow, performance keys (SOL FIL ROL RST), ALGO on NOTE_EDIT/SET.
- RAM `.data+.bss 29640 B of 73728` (about 44 KB headroom; build floor 4 KB; E9 trigger below 8 KB). ISR CPU cost is
  still unmeasured (P4-019/P4-022).

## Owner rulings of 2026-11-13 (all logged in OWNER_QUESTIONS.md or the task logs)
- HW-1 signed off (P4-021 `Approved:` line).
- LEDs: a hard blink is allowed ONLY for the playhead and the beat; everything else is solid, dim or breathing. The LED
  experiment (P4-037) runs after the builds it needs.
- MOD output is IN v1 (per-algorithm `s_mod` lanes sent as CC to the channel's device).
- FULL RIG is IN v1, driven by device profiles that are DATA (a new device is a table, not code). The TB-3 is the first
  fully supported device.
- The tie cap of 17 steps (P4-042, ISR bound) is accepted.
- ADR-0005 (preset flash map: 16 A/B slots at 0xA0000..0xBFFFF, settings at 0xFC000) is locked.
- Hint wording: PAST THE END hint becomes `PG+=ADD`, ONE PAGE hint becomes `K1=LEN` (done inside P4-040).
- Push in blocks, and ask each time. Flash only when he says go.

## Agreed execution order (the `priority:` field encodes it; table in ROADMAP.md)
P4-040 ACID finish (owner approves marks) -> P4-037 HW-LED session (human; the orchestrator builds a throwaway image
and asks before flashing) -> P4-038 LED design -> P4-039 LED-1 build -> P4-043 unwired controls -> P4-045 RIG/MOD
design (docs, ADR-0008; may run while LED builds) -> P4-046 profile data + TB-3 profile -> P4-047 MOD engine (ISR) ->
P4-048 ROW 3 knobs -> P4-049 RIG page -> P4-050 HW-RIG -> P4-031 external clock (+ BUG-009 + P4-025, one batch) ->
P4-019 -> P4-022 HW-T -> P4-033 -> P5-002+003+004 -> P5-005 -> P5-006 -> P5-007+008 -> P5-009 HW-2 -> P6-001..008 ->
P7-002..004 -> P8-001..004 -> E9 (v2, owner decides).
Dependencies added today: P4-019 waits for P4-047, P6-001 for P4-048, P6-008 and P8-004 for P4-050, P7-002 for P4-047.
Known conflict P4-045 must resolve: the control tables still carry the P2-019/021 RIG design (RIG page on CHN hold)
while MODULATION.md (P2-022, the built engine) says ROW 3 is the MOD page and there is no RIG page. The owner wants both
ROW 3 live knobs and a RIG setup page.

## Process rules (docs/project/PROCESS.md and ORCHESTRATOR.md 8b)
- One agent builds or runs `tools/check.sh` at a time (shared `build/`). Read-only agents may run alongside.
- Executor then a Sonnet reviewer. Review depth by task type: UI, ISR and algorithm tasks get a full reviewer;
  docs-only and one-line tasks are `review: self`. One reviewer pass = ONE complete findings list (hazards and docs
  sync); a second round only verifies fixes. Screen and ISR tasks run on Sonnet; Haiku only for exact-copy tasks.
- Every executor finishes with a docs-sync pass (grep the old wording; `docs/design/forbidden_terms.txt` is enforced by
  check.sh). Batch same-recipe tasks: P5-002+003+004, P5-007+008.
- Never flash or push without asking. Human gates (`human-review`, `hardware-test`) close only on the owner's own
  `Approved:` line in his words (AskUserQuestion answers count when logged verbatim).
- The owner answers in a word. When he asks for open items, use the `open-items` skill (numbered, plain context,
  lettered options, a recommendation).
- Stray-file trap: `>>` into a glob path creates a literal `*` file, and `git rm 'a-*.md'` expands the glob in git: use
  `:(literal)` pathspecs. Resolve the real path first. (One such file was found and removed on 2026-11-13.)

## Known debts and unverified facts
- Firmware: external clock transport/tempo not via clk_ext, and midi_clock.c overwrites BPM under a host clock (P4-031);
  K3 SOURCE on CHAN inert (P4-033); SOL/FIL/ROL/RST keys (P6-003); PRESETS encoder (P7-002); hard-tap ACCENT (P6-006);
  hold GLO = HARMONY (P5-006); live MIDI IN notes share slots with sequence notes; preview-sound pick unwired (P4-043);
  hand-made preset where an out-of-use channel holds an in-use channel's MIDI number loads the in-use one as OFF (P7 note).
- Tools: `--selftest` of gen_ui_tables not in check.sh; `check_control_map.py` has a 78-failure baseline (RIG rows).
- Docs: `SYSTEM_INVENTORY.md`, `CONTROL_INVENTORY.md`, `CLEAN_HOUSE_REPORT.md`, `TIMING_SPEC.md` still cite the removed T-1
  files (ui_t1.c, t1_glue.c, torso_t1.c/pipeline/state/ui) as the pre-cutover baseline; they are snapshots, left on purpose.
- TB-3 facts verified by ear (OWNER_QUESTIONS 104-106): velocity >= 100 accents; next note-on before the previous
  note-off slides; all 11 CCs work (74 cutoff, 71 resonance, 12/13 env mod, 16 accent, 17 effect, 104 tuning, 68/69
  scatter, 11 pad Y). UNVERIFIED: anything else in tools/hwprof/tb3.tsv and the TB-3 profile (P4-045 lists verified vs
  unverified); the 17-step tie behaviour on the TB-3; behaviour under an external clock; interrupt timing.
- Known failures that are not code regressions (CLAUDE.md): the "web pages" test step (space in the path) and the
  intermittent wall-clock benchmarks.

## Lab state
- The unit is flashed with build `felucca.fwsc` sha1 `8a929084...` (about the 6d5f396 tree, `FM-1_900`). Later commits
  (P4-032..044) are NOT on the unit yet; the next flash is the HW-LED throwaway image.
- Stock V15 shows as `FM-1_015` on port "FM-1"; Felucca builds as `FM-1_9xx` on "JIANT FM", loader "JIANT Update":
  install with `--port "JIANT"`. Restore image: `restore/FM-1-official-V15.fwsc` (gitignored).
- TB-3 receives on MIDI channel 2. USB-C is device-only and the TRS jack is MIDI IN only, so the route is
  FM-1 -> Mac -> TB-3: `.venv/bin/python tools/fm1_bridge.py` (blocks; Ctrl-C stops; `--list` shows ports; drops clock
  unless `--clock`). Use the bridge or Ableton, never both. Listener: `tools/fm1_midi_listen.py --seconds 120 --out /tmp/x.log`.
  Checklist: docs/project/HARDWARE_TEST.md (updated 2026-11-13 for power-on CH1 only, MIDI OFF defaults).

## First three actions for the next session
1. Read CLAUDE.md and this file, then run `python3 tools/backlog.py next` and `python3 tools/backlog.py list`. Expect
   P4-040 first. (`sh tools/check.sh` only when a task needs it; the last run ended CHECK PASS.)
2. Run `/next-task` for P4-040 (executor Sonnet, then a Sonnet reviewer; screen + ISR-adjacent, full review). Its gate is
   `human-review`: show the owner the new marks (renders) and wait for his `Approved:` line.
3. While P4-040 awaits him, ask the owner whether to start P4-045 (docs only, can run in parallel) and whether to push
   the local commits (a block since e9f5671). Then P4-037 needs his "go" before the throwaway LED image is flashed.
