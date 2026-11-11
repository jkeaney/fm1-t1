---
id: P7-001
title: Flash map: a safe region for 16 presets and the settings (ADR-0005)
phase: P7
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P3-012]
gate: human-review
---
## Goal
Choose and document flash offsets for 16 preset slots (SeqPreset v3, 1396 B) and settings that cannot collide with the app image, the update loader, OTA or the existing storage A/B sectors.

## Read first
- firmware/src/storage.c, storage_hw.c, ota.c, ota_hw.c, loader/ (sector use)
- firmware linker script and build.py (image placement, size limit)
- docs/design/SEQUENCER_UX.md §9.5
- docs/project/decisions/ADR-0003-sequencer-core.md §10
- docs/project/OWNER_QUESTIONS.md decisions 79, 80

## Deliverable
- docs/project/decisions/ADR-0005-preset-flash-map.md: the full flash map (start/end of every region from the linker and loader), the chosen preset and settings sectors, wear and power-loss behaviour (A/B or journal), and how an image upgrade leaves them intact
- A host test plan for P7-002 (power loss mid-write, version mismatch, empty slot)

## Acceptance criteria
- [ ] Every region boundary cites the file and line it came from
- [ ] The preset region does not overlap the image's maximum size or the loader/OTA sectors (a table proves it)
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
