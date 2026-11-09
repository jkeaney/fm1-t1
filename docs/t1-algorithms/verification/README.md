# Verification harnesses for the T-1 review

Two throwaway programs that compile the real T-1 sources on the host and print the behaviours documented in `../`. They do **not** modify the firmware and are not part of the project's test suite.

| File | Purpose |
| --- | --- |
| `probe.c` | Euclid patterns, scale masks, quantiser, phrase and style tables, pace table, LFSR, swing, default-pattern pitch range, GLOBAL routing, master length override, queue overflow, p-locks, flash layout, TEMP |
| `probe2.c` | Stuck notes after overflow, cycle selection by key, random-target and step-mod knobs, muted-track phase, zeroed lock arena, LFSR loop determinism (rates 0 and 3) |
| `probe3.c` | Correlation between consecutive LFSR draws (finding F19) |
| `probe.out.txt`, `probe2.out.txt`, `probe3.out.txt` | Captured output against the **current working tree** (2026-11-09, after the scale-mask and range fixes) |
| `probe.before-fixes.out.txt` | The first run of `probe.c`, before the scale masks and `range` were corrected; shows F1, F2 and F4 as originally found |

Build (from the repo root; see `../10-tests-and-open-questions.md` §10.5):

```
cc -O1 -Ifirmware/src -o /tmp/t1_probe docs/t1-algorithms/verification/probe.c firmware/src/torso_t1.c firmware/src/torso_t1_algo.c firmware/src/torso_t1_pipeline.c firmware/src/torso_t1_state.c firmware/src/torso_t1_ui.c -lm
```

The output is deterministic (no timing is measured). If the firmware is fixed, expect lines in these outputs to change: that is the point.
