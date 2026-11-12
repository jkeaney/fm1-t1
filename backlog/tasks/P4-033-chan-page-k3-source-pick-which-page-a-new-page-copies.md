---
id: P4-033
title: CHAN page: K3 SOURCE chooses which page a new page copies
phase: P4
type: task
status: todo
priority: 3
executor: sonnet
depends_on: [P4-015]
gate: none
---
## Goal
K3 SOURCE on the CHAN page is drawn (`OWN`) but inert. Let the producer choose P1..P3 as the source of the next added page (sq_page_src_set exists). Not needed for HW-1 (a new page copies the previous page).

## Read first
- backlog/feedback/FB-018 item 2; SCREEN_SPEC_V2 CHAN band; knob_map.tsv CHAN K3; firmware sq_page_src_set.

## Deliverable
- K3 turns SOURCE through OWN, P1..P(n-1); value names from the table; host test; render matches v12 chan scenes.

## Acceptance criteria
- [ ] A new page copies the chosen page. [ ] check.sh CHECK PASS, RAM headroom >= 8 KB.

## Verification
```sh
sh tools/check.sh
```

## UX gate
Use case docs/design/features/P4-015-chan-page-midi-channel-algorithm-length.md covers the page; add a step for SOURCE at Gate A before building.

## Out of scope
- Flashing.

## Log
- 2026-11-13: filed from FB-018.
