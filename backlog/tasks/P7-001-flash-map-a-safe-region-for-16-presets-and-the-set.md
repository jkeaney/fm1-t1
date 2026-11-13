---
id: P7-001
title: Flash map: a safe region for 16 presets and the settings (ADR-0005)
phase: P7
type: task
status: done
priority: 2
executor: sonnet
depends_on: [P3-012]
gate: human-review
---
## Goal
Choose and document flash offsets for 16 preset slots (SeqPreset v4, 1556 B, plus its 100 B ModSave sibling = a 1656 B slot payload before the storage-layer header and CRC; ADR-0003 section 22 row 1) and settings that cannot collide with the app image, the update loader, OTA or the existing storage A/B sectors.

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
- [x] Every region boundary cites the file and line it came from
- [x] The preset region does not overlap the image's maximum size or the loader/OTA sectors (a table proves it)
- [x] Design locked: owner line in the log

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

### 2026-11-13 executor (sonnet)
Docs only (no build, no firmware/ or tools/ edits, as instructed).
- Wrote docs/project/decisions/ADR-0005-preset-flash-map.md: full map with file:line per region, the proof table, wear and power-loss,
  upgrade behaviour, and an 11-point host test plan for P7-002. Updated docs/project/decisions/README.md (row 0005).
- Decision: presets = 16 A/B sector pairs at 0xA0000..0xBFFFF (ids 10..25); settings stay at 0xFC000/0xFD000; autosave unchanged.
  Inside the existing `FL_STORE_OK` window (firmware/hal/fm1_flash.h:53) and outside both loader windows (firmware/loader/loader.c:21).
- Facts found: the image hard stop is 0x93000 (tools/fm1pkg_make.py:31, ldr_core.c:32); app slot 0x8DFBC (app.ld:15); the loader may also
  write 0x93000..0x97000 so the region starts well clear. Trap for P7-002: storage.c:72 `obj >= OBJ_UPRESET0` would capture the new ids,
  so the preset branch must come first (test 9).
- Not verified (marked [I] in the ADR): image start at flash 0x4000 giving 0x91FBC; NOR endurance of the part (datasheet not checked).
- Docs-sync grep terms: "reserved: flash map", "ADR-0005", "flash map" in docs/project, docs/design: only the decisions README row hit (fixed).
- Hazards checked: array bounds (slot 0..15, test 1, 11); removed/unused slot guards (stale sectors, test 7); stale state after preset
  landing (version reject leaves live state, test 6); early return skipping cleanup (power cut at every step, test 4); page-change flags n/a.
  Pseudo-random sequence test is test 11 in the plan (to be built in P7-002).
```
python3 tools/backlog.py check
```

Output: `166 items, 0 errors`.

Awaiting owner: check the one choice in plain language. Do you accept putting the 16 presets in the old, unused sample-slot area of flash
(addresses 0xA0000 to 0xBFFFF), keeping settings where they already are, and treating a preset saved by an older firmware version as
"EMPTY, nothing loaded"? Reply "Design locked: ADR-0005 <date>" in this log to close the third criterion.
- 2026-11-13: stays in review (owner). It blocks nothing until P7-002.

- 2026-11-13 Approved: owner (open-items 2A) - Design locked: ADR-0005 (preset flash map: 16 slots in 0xA0000-0xBFFFF, settings and autosave unchanged, old-version preset = EMPTY).
