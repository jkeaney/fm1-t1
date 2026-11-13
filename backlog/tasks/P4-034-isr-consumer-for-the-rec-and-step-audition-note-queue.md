---
id: P4-034
title: ISR consumer for the REC and step-audition note queue
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-017]
gate: none
---
## Goal
REC keys and a stopped step-on tap sound on the unit: drain the 16-slot note ring that ui_sq.c fills, from `events_block`, so every key press plays (and ends) on the selected channel.

## Read first
- backlog/feedback/FB-019 (the full observation and what the consumer must do)
- firmware/src/ui_sq.c REC section (`uisq_aud_push`, `ui_sq_aud_next`); firmware/src/sq_core.c `sq_live_on` / `sq_live_off`; firmware/src/seq.c `events_block`
- backlog/tasks/P4-032 ("Stopped: a step turned ON by a tap sounds once" uses this road)

## Deliverable
- `events_block` pops every queued event each block and calls `sq_live_on(note, vel)` / `sq_live_off(note)`; a forward declaration of `ui_sq_aud_next` in seq.c (or move the ring before seq.c as `transport_req` is)
- Host test: a queued on then off yields NOTE_ON then NOTE_OFF on the selected channel; SELECT between on and off still ends the right channel; a full ring drops an on but never an off; leaving REC with a key down ends the note
- Host counterpart in tests/host_sq_hooks.h if a new hook is needed

## Acceptance criteria
- [x] Host test counts NOTE_ON and NOTE_OFF through events_block for the cases above
- [x] No note stays on after REC is left or the channel changes (no hung note)
- [x] Interrupt path adds no loop without a bound; state the worst-case pops per block in the Log
- [x] `sh tools/check.sh` ends with `CHECK PASS`; paste the `RAM .data+.bss N of 73728` line, headroom at least 8192 B

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, internal plumbing for P4-017 and P4-032 (the producer-visible behaviour is judged in their use cases and at HW-1).

## Out of scope
- The stopped step-tap request itself and toasts (P4-032); hard-tap ACCENT; any screen change

## Log
- 2026-11-13: created from FB-019 (owner ruling: before HW-1). P4-032 now depends on this.

### 2026-11-13 executor (sonnet 5.5)
- Ring moved into new firmware/src/sq_aud.h (guarded; seq.c includes it before events_block, ui_sq.c includes the same file), so every host test that links seq.c has it. `events_block` (seq.c, before `seq_audition`/`sq_block`) pops and calls `sq_live_on` / `sq_live_off`; with `sq_ready` off the events are popped and dropped (no hung note: an off with no on is a no-op).
- Worst-case pops per block: 15 (loop bound `UISQ_AUD_N - 1`; the ring holds 15). Each pop is O(1) (sq_emit / sq_sched_one); no unbounded loop on the ISR path.
- Tests (tests/sq_link_test.c, real events_block): on+off -> NOTE_ON/NOTE_OFF on CH1; SELECT between on and off -> both on the original channel (6); full ring drops an on (15 usable), one block pops all 15; 15 offs fit and all play; sq_live_c empty; sq_ready off drops without playing; 4000-step pseudo-random on/off/SELECT/block run, flushed: held 0, offs == ons. Leave-REC-with-key-down is the UI's owed-off (already in ui_sq_rec_test) plus the same queue road.
- Hazards checked: ring bound (15-pop loop, full-ring test); removed/unused slot (off without on is a no-op, sq_live_c); stale state after SELECT (live test, random test); early return skipping cleanup (sq_ready-off path still pops); page-change flags (UI-side owed offs unchanged, ui_sq_rec_test).
- `sh tools/check.sh`: CHECK PASS. `RAM .data+.bss 29576 B of 73728` (headroom 44152 B >= 8192); pool 78520 B of 368640; image 196756 B. This build includes the BUG-008 build.py comma fix and builds.
- Docs sync: grepped FB-019, ui_sq_aud_next, "nothing in the ISR" in docs/design; fixed docs/design/features/P4-017-rec-page.md and docs/design/scenarios/S07-set-scale-chan-rec.md.

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none -> done). No findings.
- `sh tools/check.sh`: CHECK PASS on a clean tree; stamp tree hash = HEAD tree. `RAM .data+.bss 29576 B of 73728` (headroom 44152 B >= 8192).
- SPSC ring: w written only by the UI after the slot, r only by the ISR after the read; all three variables volatile; wrap by `% 16`; full = nx == r (15 usable), empty = r == w. Single core, one-byte indices: no torn index.
- Bounded: loop runs at most 15 pops per block, each O(1); no early return in events_block skips it (it returns void, none before the loop). sq_ready off pops and drops (an off without an on is a no-op via sq_live_c). sq_all_off clears sq_live_c, so STOP/PANIC cannot leave a stale "held" entry that mutes the next on.
- Off follows on: sq_live_c remembers the channel (SELECT test, 4000-step random test, offs == ons); mute after an on is gated by the slot, not the live flag; the UI owes the off after REC exit (ui_sq_rec_test). Tests drive the real events_block and would read 0 NOTE_ON without the loop.
- Include order: sq_aud.h is guarded, included in seq.c after the stdint types and before events_block, and again (no-op) in ui_sq.c; host and device builds pass.
- Docs sync: P4-017 use case (status, step 2, checklist, section 7-9), S07 step 20 updated; FB-019 mentions left in P4-017 task Log are history. P4-032 depends on P4-034.
- BUG-008 (missing comma after "FELUCCA_ICONS" in tools/build.py): fix at 6c93458 is correct, the build is clean; BUG-008 already `done`, reviewed here.
