---
id: P1-002
title: Inventory every firmware subsystem with its size, RAM and dependencies
phase: P1
type: task
status: review
priority: 1
executor: sonnet
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
- [x] Every `#include "*.c"` in felucca.c appears in the table
- [x] Flash and RAM totals in the table are within 5% of the build's reported image size and RAM
- [x] The size command is written in the doc so it can be re-run

## Verification
```sh
grep -o '#include "[a-z0-9_]*\.c"' firmware/src/felucca.c | sed 's/.*"\(.*\)"/\1/' | while read f; do grep -q "$f" docs/design/SYSTEM_INVENTORY.md || echo missing $f; done   # prints nothing
```

## Out of scope
- Deciding what to remove (that's P2-002).

## Log

### 2026-11-09 executor (sonnet)
- `./build.sh` (clean tree, c44670e): `ok    image 523032 B; RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640`.
- `objdump -h` / `objdump -t` on build/felucca.elf via the Docker toolchain (commands in SYSTEM_INVENTORY.md "Re-running").
  Sections: .text 519672, .ram_text 2924, .data 436, .bss 55296, .pool 342876, .noinit 15564.
- Table totals equal the sections exactly: flash 523032 (= build image), RAM 55732 (build says 55744: 12 B .data/.bss alignment, explained in the doc), pool 342876.
- Verification command (every `#include "*.c"` of felucca.c is in the doc): printed nothing ("verify-ok").
- Scripts added under docs/design/inventory/ (attrib.py, cov.py, deps.py, gen_table.py, all GPL header); they write only to git-ignored build/inventory/. No firmware edited; no tests run (host suite not required for a doc task); nothing flashed or pushed.
- Reviewer notes: (1) sizes are attributed by symbol-to-defining-file regex, inlined code lands in the caller (ui_t1.c shows 88 B, true cost unconfirmed; midi_uart.c 0 B code). (2) The dependency columns are a heuristic (documented caveat 5). (3) "Runs in" is inferred from file headers and size_fns.py, not a call graph. (4) eng_digital.c is not built (FELUCCA_FM4=0) so it has no size row.
