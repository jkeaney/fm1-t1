---
id: P2-002
title: Decide keep / adapt / remove for every subsystem (system map)
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P1-002, P2-001]
gate: human-review
---
## Goal
Define the 'clean house' target: only UI, system, algorithms and setup that drive the vision remain (ADR-0001).

## Read first
- docs/design/SYSTEM_INVENTORY.md
- docs/design/CONTROL_MAP.md
- docs/design/ALGORITHM_SET.md
- docs/project/decisions/ADR-0001-fork-felucca-and-keep-t1.md

## Deliverable
- docs/design/SYSTEM_MAP.md: each inventory row marked KEEP / ADAPT (what changes) / REMOVE, with a reason; the removal order (leaves first, by the dependency column); the expected flash and RAM freed; the host test suites that go or change with each removal; which engine stays as the preview synth and why

## Acceptance criteria
- [x] Every SYSTEM_INVENTORY row has a decision
- [x] The platform-core list is all KEEP; the update loader and installer path are KEEP (ADAPT only strips groovebox code; loader/ota/installer untouched)
- [x] The removal order never removes something that a KEEP item still depends on (16 edges verified, section 5)
- [ ] The owner approved it (log line)
- [x] SYSTEM_MAP has a 'v2' column (redefined by the coordinator update: v2 = clean-up of what v1 could not safely remove): each subsystem is KEEP, ADAPT, REMOVE (in v1) or v2-REMOVE (kept in v1 as the legacy groovebox mode per owner decision 32, removed in v2 epic E9)

## Verification
```sh
grep -cE 'KEEP|ADAPT|REMOVE' docs/design/SYSTEM_MAP.md
```

## Out of scope
- Removing code (that's P3).

## Log

### 2026-11-10 executor (sonnet)
- Mid-task coordinator update (supersedes owner decision 32): groovebox fully out of the UI in v1, no MODE row. Redid the map: groovebox-only subsystems are REMOVE in v1; v2 column = residue of ADAPT rows. Added section 3 "Boot and first-use". Recorded as a superseding note in OWNER_QUESTIONS.md.
- Created docs/design/SYSTEM_MAP.md. Verification: `grep -cE 'KEEP|ADAPT|REMOVE' docs/design/SYSTEM_MAP.md` -> 81.
- v1 REMOVE frees (firm, inventory sizes): flash 234574 B (image 523032 -> 288458), RAM 28512 B (headroom 17984 -> 46496), pool 192211 B. Upper-bound extras listed in section 9. v2 firm figure from the inventory is only mod matrix 1858 B / 52 B RAM; the rest is measurable after v1.
- Preview synth: ANALOG (index 0 and eng_idx fallback, 5746 B, no eng_mem).
- Hidden edges found (inventory said otherwise): seq->song chain, ui->T-1 screen, midi_control->chord, settings stored in favorites array, voice/seq->DRUM; false edges: voice->fx, slicer->fx.
- Menu rows 37 -> 28 (DISPLAY 8, CONTROL 4, AUDIO 6, MIDI 4, SYSTEM 6); ui_test asserts "8 8 6 8 7" become "8 4 6 4 6". CONTROL_MAP/UX_REVIEW still mention MODE SEQ/GROOVE and need a one-line update (not done: out of scope).
- Not run: build, full tests, flashing (design-only). Sizes are cited from the inventory, not re-measured.
- Owner questions 37-42 added to docs/project/OWNER_QUESTIONS.md: editor removal, console/USB audio, CHORD ENTRY row, preview patch/TUNE, MIDI learn, host tests linking the T-1.
- Owner approval: NOT given. Awaiting human-review.
