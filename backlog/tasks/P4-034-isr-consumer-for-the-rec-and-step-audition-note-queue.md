---
id: P4-034
title: ISR consumer for the REC and step-audition note queue
phase: P4
type: task
status: in_progress
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
- [ ] Host test counts NOTE_ON and NOTE_OFF through events_block for the cases above
- [ ] No note stays on after REC is left or the channel changes (no hung note)
- [ ] Interrupt path adds no loop without a bound; state the worst-case pops per block in the Log
- [ ] `sh tools/check.sh` ends with `CHECK PASS`; paste the `RAM .data+.bss N of 73728` line, headroom at least 8192 B

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
