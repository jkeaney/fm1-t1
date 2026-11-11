---
id: BUG-003
title: beat_samples divides by a zero song tempo in the DRUM stress run with a SONG playing
phase: P2
type: bug
status: done
priority: 1
executor: sonnet
depends_on: []
gate: none
severity: high   # high: wrong notes, crash, a core feature broken · medium: a control misbehaves · low: cosmetic or latent
found_in: 4fb0b65 (HEAD before P3-014; reproduced on a clean copy of it)
---
## Goal
Fix: beat_samples() (firmware/src/fx.c, `FS * 60 / song.g[G_BPM]`) must never divide by zero, so that the DRUM grid stress run with a SONG playing (`tests/drumfuzz_test.c` seed 101, `song` mode) passes under UBSan.

## Read first
- firmware/src/fx.c (beat_samples, div_samples)
- tests/drumfuzz_test.c (song mode; BPM is set to 60..240 at line 246, so the zero comes from elsewhere: song chain / song load path)
- tests/run_tests.sh line 327

## Deliverable
- Find which path sets song.g[G_BPM] to 0 during the song run and fix that (or guard beat_samples if the zero is a legal state). Add a host check that failed before the fix.

## Steps to reproduce
1. Build the host tests (`./build.sh`), then `clang -DPREEMPT -g -fwrapv -fsanitize=address,undefined -fno-sanitize=shift -fno-sanitize-recover=all -w -Ibuild/gen -Ifirmware/src -o /tmp/drumfuzz tests/drumfuzz_test.c -lm`.
2. `/tmp/drumfuzz 101 3 1500 song`.

## Expected
Run completes with no UBSan report (`ALL HOST TESTS PASSED` in tests/run_tests.sh).

## Actual
`tests/../firmware/src/fx.c:224:90: runtime error: division by zero` (fx.c:115 after P3-014), abort. Fails the same way on the pre-P3-014 HEAD, so it predates P3-014 and was not caused by it. Found while running `tools/check.sh` for P3-014.

## Acceptance criteria
- [x] The new test fails on the old code and passes on the new.
- [x] `tools/check.sh` passes.

## Verification
```sh
sh tools/check.sh
```

## Log
- 2026-11-11 executor (haiku): filed while executing P3-014 (blocked). See P3-014 log.

### 2026-11-11 executor (sonnet)
Root cause: not a firmware path. G_BPM is never 0 at run time (all writers clamp to 40..240; project load clamps via GP). The zero was the
host harness: drumfuzz `setup()` calls `ui_power_on()` (tests/ui_test.c, the mirror of main.c felucca_init) while the -DPREEMPT CPU-time
signal emulates the audio ISR. ui_power_on does `memset(&song, 0, ...)` and only later restores `song.g[i] = GP[i].def`, so a signal in that
window rendered a block with G_BPM 0 (stack: beat_samples <- events_block <- mix_block <- host_isr <- on_vt <- ui_power_on). On the device
felucca_init runs before audio_init, so this cannot happen. Hence intermittent and unrelated to P3-014. No firmware change; beat_samples
left unguarded on purpose (0 is not a legal state).
Fix (tests only): tests/drumfuzz_test.c holds the emulated ISR off for the whole of setup's power-on (`fz_boot`, the signal only counts).
Deterministic regression: tests/ui_test.c ui_power_on has an `#ifdef FZ_BOOT_HOOK` call right after the song memset; drumfuzz defines it to
deliver one signal there.
- `/tmp/drumfuzz 101 1 100 song unguarded` (old behaviour) -> `fx.c:115:90: runtime error: division by zero` every time (was ~intermittent).
- `/tmp/drumfuzz 101 3 1500 song` (guarded) -> seeds 101..103 ok, no UBSan report.
- `sh tools/check.sh` -> CHECK PASS (RAM .data+.bss 55248 of 73728).
Note: drumfuzz seed 102 can still HANG (5 s alarm, "ui max" ~1.5 s) on a loaded host under ASan; it is a timing fault seen before my change too.

### 2026-11-11 review (sonnet)
Verdict: PASS. Root cause is sound: main.c runs felucca_init (l.175) before audio_init (l.179), so the half-initialised song window exists only in the host harness (ui_power_on memset then defaults, with the -DPREEMPT signal acting as ISR). Fix is tests-only (drumfuzz fz_boot guard plus FZ_BOOT_HOOK deterministic hook in ui_test.c); the "unguarded" mode reproduces the old failure, guarded passes. tools/check.sh: CHECK PASS, RAM .data+.bss 55248 of 73728 (headroom 18480 B). Note: drumfuzz seed 102 may hang on a loaded host (pre-existing timing fault, not this bug).
