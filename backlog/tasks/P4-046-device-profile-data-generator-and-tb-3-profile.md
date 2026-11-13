---
id: P4-046
title: Device profile data, generator with checks, and the TB-3 profile (a new device is data only)
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-045]
gate: none
---
## Goal
Land the device-profile data layer decided in P4-045 / ADR-0008: a table format, a generator that compiles it into flash-resident C tables with checks, the complete TB-3 profile, and host tests, with zero per-device code. A second, dummy device in the tests proves that adding a device is data only. No engine, no screen: this task only makes the data readable by code.

Owner ruling 2026-11-13: "the next device we add will easily be added in via data and not code."

## Read first
- docs/design/hardware/RIG_MOD_V1.md and docs/project/decisions/ADR-0008-rig-mod-v1.md (P4-045; the format, columns and C shapes are decided there; follow them, do not re-decide)
- docs/design/MODULATION.md sections 1 (the TB-3 parameter table, pick order), 6 (default slots, scatter pair), 7 (budget)
- docs/design/hardware/TB3_PROFILE.md and tools/hwprof/tb3.tsv (the source data, provenance fields); tb3-midi-guide.csv is a reference only, never an input
- tools/gen_ui_tables.py (pattern to copy: deterministic output, `--check`, `--selftest`, width check with the real fonts) and tests/design/width_probe.sh, tests/design/widths.tsv
- tests/run_tests.sh (how generated headers and host tests are wired), tests/host_sq_hooks.h
- firmware/src/felucca.c include order (the generated header is included before sq_*.c), BUILDING.md

## Deliverable
- Profile source files under `docs/design/hardware/profiles/` (per ADR-0008): `tb3.tsv` with all 11 verified CCs, long and short names, kind, zones, home, default depth, pick order, link (SCTYP with SCDEP), park / neutral value on stop, and the provenance block (src, how, cred, level 2, date). Moved from `tools/hwprof/tb3.tsv` (git mv; update references in docs). The data content is exactly MODULATION.md section 1 as corrected by P4-045; unverified items (SCTYP zones 8, homes RESO 40 and EFFEC 0) are kept as the documented placeholders and marked in the file.
- `tools/gen_hwprof.py` -> `build/gen/hwprof.h` (read-only `HW_PROFS[]`, `HW_NPROF`, `HwProf`, `HwParam`, provenance table, `NONE` and `GENERIC` entries as data rows or the first two list entries), deterministic output. `--check` and `--selftest` like gen_ui_tables. It rejects, naming file, line and value: a duplicate id or CC within a profile, a CC > 127, a short name over 5 letters or over 38 px in AF_S (real fonts; skipped with a note when fonts are absent, required under `--check`), a long name over 16, a home or default depth outside the range, a link to a missing parameter, a select with fewer than 2 zones, a missing provenance field, an empty pick order, more parameters than the C table allows, and a device list line with no file. `--selftest` proves each of these fails on a deliberately broken row.
- Hook the generator into `build.sh` the way gen_ui_tables is, and into `tools/check.sh` (`--check` and `--selftest`). (These two are the only edits to existing tools files; nothing under firmware/ except the include of the generated header.)
- A tiny read API used by later tasks, in a new `firmware/src/sq_hw.c` (SPDX header, included after the generated table, before `ui_sq.c`): `hw_nprof()`, `hw_prof(id)`, `hw_param(prof, i)`, `hw_default_slots(prof, out[4])`, `hw_next_param(prof, from, dir, skip_mask)` for the pick walk (skips parameters already on another slot, wraps, `-` = none), `hw_home(param)`, `hw_scale(param, lane, level, depth)` (the clamp formula of MODULATION 3.1 so the engine and screen use ONE function). Pure functions over const data. No branch on a device id, a device name or a CC number anywhere.
- Host tests `tests/hw_profile_test.c` (wired in `run_tests.sh`): the TB-3 table equals a golden dump (`tests/golden_hwprof_tb3.txt`: id, CC, kind, home, depth, pick order); the pick walk (wrap, skip, none); `hw_scale` clamps and truncates toward zero per MODULATION 3.1; default slots are CUT 74, RESO 71, ENV-X 12, ENV-Y 13; SCTYP links SCDEP. **Second-device proof**: `tests/data/hwprof_dummy.tsv` (a made-up 3-parameter synth with a select) is compiled by the same generator into a test-only header (`gen_hwprof.py --extra`), and the same tests walk it with no code change; a test asserts that `grep -n "TB-3\|TB3\|tb3\|74\|0x4A" firmware/src/sq_hw.c` finds no device-specific logic. The task Log records the exact three-step "add a device" recipe that the dummy device followed.
- Docs: `docs/design/hardware/README.md` gets the "add a device in data" recipe (the file, the list line, `gen_hwprof.py --check`).

## Acceptance criteria
- [ ] `python3 tools/gen_hwprof.py --check` and `--selftest` pass; `--selftest` fails each deliberately broken row
- [ ] The TB-3 golden dump matches; the dummy device passes the same test code with no firmware source change
- [ ] `grep` shows no device name, CC number or profile id literal in `sq_hw.c` or any other firmware file
- [ ] Flash growth stated in the Log (profile table bytes) and RAM growth is 0 B `.bss` (const data only)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
python3 tools/gen_hwprof.py --check && python3 tools/gen_hwprof.py --selftest
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, data layer with no user-facing control. The screen names that come from this data (ROW 3 labels, RIG cells) are gated in P4-048 and P4-049. The short names were measured in P4-045 and are re-measured here by the generator.

## Out of scope
- The MOD engine, any CC output, `ModSave` changes (P4-047)
- Any screen change (P4-048, P4-049)
- A second real device (only the test dummy)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created. Owner ruling 2026-11-13: "the next device we add will easily be added in via data and not code."
