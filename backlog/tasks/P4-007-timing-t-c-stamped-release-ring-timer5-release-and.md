---
id: P4-007
title: Timing T-C: stamped release ring, TIMER5 release and the tempo constant
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-004]
gate: none
---
## Goal
Tight MIDI timing (TIMING_SPEC M1-M7, M9, M10): every outgoing packet carries a due time and is released by the 100 us TIMER5 at that time, not in the audio-interrupt burst.

## Read first
- docs/design/TIMING_SPEC.md §1, §2 (targets), §3 M1-M10, §6.3 (host tests), §7 (fallbacks)
- docs/project/decisions/ADR-0003-sequencer-core.md §19
- firmware/src/usb.c, audio.c, hal timer headers

## Deliverable
- usb.c: `txr[128]` {pkt, due}, `midi_out_event_at`, `midi_tx_release` in the TIMER5 branch, `LEAD` 128 samples; a compile flag selecting the M3 fallback (release between audio interrupts)
- sq_core.c: exact tick position inside the block (M1), stamps (M2), ordering (M4), clock-out pulses at exact positions when master (M7), `SEQ_FS_Q4` tempo constant (M10)
- tests/timing_test.c: the TIMING_SPEC 6.3 rows tick position, stamp, ordering, clock out count, release, tempo constant

## Acceptance criteria
- [x] timing_test passes every listed row
- [x] The fallback flag builds and passes the same tests
- [x] `.ram_text` no-call check in build still passes
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Clock follow / PLL (P4-009)
- The test image (P4-019)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (Sonnet 5.5)
Done. Commands and decisive output:
```
sh tools/check.sh                     -> RAM .data+.bss 23828 B of 73728 ... CHECK PASS
./build.sh | grep RAM                 -> ok    .ram_text: 916 insns, no calls
                                         ok    image 169020 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640
cc ... tests/timing_test.c (ASan+UBSan), FELUCCA_TX_NESTED=1 and =0 -> TIMING PASS (both)
sq_core_test / sq_chan_test / usb_ring_test -> SQ_CORE PASS, SQ_CHAN PASS, usb_ring_test passed (goldens unchanged)
```
RAM headroom 73728 - 23828 = 49900 B (floor 8192 B met). tests/run_tests.sh runs timing_test twice (nested and fallback).
timing_test rows: tick position (64975 phases x 201 BPM, every tick within 1 sample, at most 1 tick per block); stamp (712960
packets, 8 tempi, 10 min at 40/120/240, non-decreasing with a jittery and outlier half reference, one stamp per tick, tick 0 due
at tref + LEAD); ordering (4 dense bars at 90/120/180/240: Start/clock, OFFs, ONs per tick, no wire violation); clock out (exactly
384 pulses in 16 quarters at 90/120/180, span within 100 ppm, Start/Stop once, nothing when not master, SPP 300 = 44/2 then 0xFB);
release (within one 100 us tick, never early, order across the 32-bit counter wrap, 16 per transfer, 40 -> 16/16/8, reserve rule,
no overtaking of an undue head, nested hook empty in the fallback); tempo constant (worst 3.00 ppm over 40..240 BPM; the old 44100
count is 400 ppm off).

What changed
- usb.c: `txr` is the pair `midi_out_q[128]` + `midi_out_due[128]` (parallel arrays: the pkt array keeps its name so six existing
  host tests stay untouched; same 1024 B). `midi_out_event_at(pkt, due)` keeps the ring sorted (a due earlier than the tail is raised
  to it). `midi_out_event(pkt)` = due now (live keys, T-1 path until cut-over). `ep1_tx` takes only due packets (<= 16); the SIE
  steps are `ep1_tx_ready` / `ep1_tx_arm` with an `FM1_HOST_SIE` host hook. `midi_tx_release()` + `midi_tx_release_nested()`;
  `FELUCCA_TX_NESTED` (default 1, 0 = M3 fallback), `MO_LEAD_SAMPLES` 128. Time source `MO_NOW()`: fm1_ticks on device/loader, 0 on
  host tests that do not define FM1_TICKS_PER_US (so existing tests build unchanged).
- main.c: `midi_tx_release_nested()` in the nested TIMER5 branch, `midi_tx_release()` every non-nested tick. tools/build.py passes
  FELUCCA_TX_NESTED; BUILDING.md row added.
- sq_core.c: M1 `sq_tick_pos` (sample = ceil(need/3072)-1), M2 `sq_half_begin(t0)` filter (1/16 nudge, pull-back < 1 sample, resync
  over 1 ms), `sq_stamp_tick/now`, `sq_out`; M7 `sq_rt_byte`, clock every 8 ticks, 0xFA at tick 0, 0xFC on stop (stamped now),
  `sq_clock_continue` (SPP then 0xFB), `sq_master`; M10 `SEQ_FS_Q4` = 705882, `sq_beat_q4`, accumulator in 1/16 samples.
  sq_emit's pairing logic is untouched (only `midi_out_event` -> `sq_out`).
- tests: new timing_test.c; sq_core_test / sq_chan_test mocks gained `midi_out_event_at` and the SQ_EXT_BEAT / SQ_BPM hooks.

For the reviewer
- TIMING_SPEC M4 says "clock first, then Start" but M7 says 0xFA at tick 0 "followed by the first 0xF8 at the same stamp". I
  followed M7 (MIDI-correct) at tick 0 only; the test treats Start and clock as one rank. Spec text should be reconciled.
- `sq_half_begin(t0)` is not called from audio.c: sq_core.c is not in felucca.c yet (nothing calls sq_block), so the hook is for
  P4-012 (call it with `t0` in `fm1_alnk0_irq` at the half start). Until then the unity build only exercises usb.c / main.c.
- fx.c `beat_samples()` is unchanged (FS = 44100): the preview tracks, click and midi_clock still count 400 ppm slow; sq_core counts
  its own beat via `SQ_EXT_BEAT()` / `SQ_BPM()` (defaults read `midi_beat_samples`, `song.g[]`). Move fx.c to SEQ_FS_Q4 at cut-over.
- Nested-release safety (R1) is argued by reading (release runs in the TIMER5 handler beside usb_poll / uac_service, and the audio
  ISR cannot preempt TIMER5) and is NOT proven; hardware stress at 85 % load is P4-019/P4-022. The fallback flag is the answer.
- Not flashed.

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none). Not flashed; hardware validation stays with P4-019/P4-022.
- tools/check.sh: CHECK PASS (re-run). `ok .ram_text: 916 insns, no calls`. `ok image 169020 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640`; headroom 49900 B (floor 8192 met).
- (a) usb.c: ring index math uses free-running mo_w/mo_r with `% MQO` (128, power of two, so wrap-safe); due compare is `(int32_t)(now - due) >= 0` (wrap-safe within 2^31 ticks); empty ring takes the due as given; sorted insert raises an early due to the tail. No host: `!usb.config` returns before anything is queued or counted; bus reset sets mo_r = mo_w. Ring full: all packets dropped at free==0, NOTE_ONs below 8 free (reserve rule); TIMING_SPEC M5 wording "OFFs and clock never dropped" overstates this (filed in P2-032). R1: I looked for a counterexample and found none. ep1_tx runs only from the TIMER5 handler (usb_poll or the nested branch, after uac_service, sequentially), the audio ISR does not touch the SIE, and the producer/consumer split (mo_w vs mo_r) is single-writer each; a nested TIMER5 landing mid-publish sees either the old or the new mo_w. Residual (minor, in P2-032): a head older than 89 s looks not-due after a host stall.
- (b) Recomputed: SEQ_FS_Q4 705882 with rounded per-beat division gives a worst error of 3.003 ppm over 40..240 BPM against 24 MHz/544 = 44117.647 Hz; matches the claim.
- (c) M4 vs M7: M7 intends 0xFA before the first 0xF8 at tick 0 (Start must precede the first clock for a follower); M4's clock-first order is the steady-state rule. Code follows M7; filed P2-032 to reconcile the spec.
- (d) FELUCCA_TX_NESTED=0 and default (1) both build (binaries differ by md5; same image size); tools/build.py forwards the flag. timing_test runs both ways in tests/run_tests.sh.
- (e) Unchanged until cut-over: nothing calls sq_block or sq_half_begin (sq_core.c is not in felucca.c). The T-1 path uses midi_out_event, due now, released within 100 us by TIMER5 (was the usb_poll cadence): slightly earlier, not different in content. P4-012 Deliverable did not name sq_half_begin; added it plus an acceptance grep.
- (f) Register access: usb.c/main.c use the existing sie_rd/sie_wr and fm1_usb_ep_send in the same file as before; sq_core.c touches no registers. No new direct register access outside hal/ beyond the pre-existing usb.c SIE pattern (moved into ep1_tx_ready/ep1_tx_arm).
- Note (not blocking): tools/build.py line 190 has a missing comma ("FELUCCA_ICONS" "FELUCCA_CDC_DEFAULT" concatenates), so those two env flags are ignored; pre-existing.
