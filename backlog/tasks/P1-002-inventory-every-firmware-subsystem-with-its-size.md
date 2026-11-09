---
id: P1-002
title: Inventory every firmware subsystem with its size, RAM and dependencies
phase: P1
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P0-001]
gate: none
---
## Goal
The ground truth for the keep/adapt/remove system map (P2-002): what each part of the firmware is, what it costs and what depends on it.

## Read first
- firmware/src/felucca.c (include order = subsystem list)
- BUILDING.md (build flags)
- tools/size_fns.py, tools/build.py (size reporting)
- CLAUDE.md (Architecture)

## Deliverable
- docs/design/SYSTEM_INVENTORY.md: one row per included .c file (or tight group) with Subsystem | Files | Purpose (1 line) | Flash bytes | RAM/pool bytes | Runs in (ISR/main/boot) | Depends on | Depended on by | Host tests covering it
- Sizes come from the real build: use the linker map or `nm --size-sort` on build/felucca.elf (or whatever build.py writes) via the Docker toolchain, and record the command used
- A 'platform core' list: what any firmware on this device needs (boot, input, LCD, USB, storage, loader)

## Acceptance criteria
- [ ] Every `#include "*.c"` in felucca.c appears in the table
- [ ] Flash and RAM totals in the table are within 5% of the build's reported image size and RAM
- [ ] The size command is written in the doc so it can be re-run

## Verification
```sh
grep -o '#include "[a-z0-9_]*\.c"' firmware/src/felucca.c | sed 's/.*"\(.*\)"/\1/' | while read f; do grep -q "$f" docs/design/SYSTEM_INVENTORY.md || echo missing $f; done   # prints nothing
```

## Out of scope
- Deciding what to remove (that's P2-002).

## Log
