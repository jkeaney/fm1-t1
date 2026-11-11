---
id: P8-003
title: Release package: version, changelog, images, checksums, install and restore
phase: P8
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P8-002]
gate: none
---
## Goal
Everything needed to install v1 and get back to stock, in one folder.

## Read first
- CLAUDE.md (installer, ports, restore)
- BUILDING.md
- build.sh output names
- docs/project/HARDWARE_TEST.md

## Deliverable
- Firmware version string bumped to the v1 value (FM-1_9xx scheme, next free number) in the one place it is defined
- CHANGELOG.md v1 section
- release/v1/: felucca.fwsc, ota.bin, SHA256SUMS, INSTALL.md (install, update mode 'JIANT Update', `--port "JIANT"`, restore to stock V15, power-cycle note), the user guide
- HARDWARE_TEST.md 'release acceptance' checklist finalised

## Acceptance criteria
- [ ] `shasum -a 256 -c release/v1/SHA256SUMS` passes
- [ ] `tools/fm1_install.py --help` matches INSTALL.md commands
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
(cd release/v1 && shasum -a 256 -c SHA256SUMS)
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Pushing or tagging (owner does that)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
