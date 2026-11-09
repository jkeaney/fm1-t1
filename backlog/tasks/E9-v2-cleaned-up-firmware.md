---
id: E9
title: v2 cleaned-up firmware: remove the legacy groovebox and unused subsystems
phase: P9
type: epic
status: todo
priority: 3
executor: haiku
depends_on: [E8]
gate: human-review
---
## Goal
After v1 ships (the sequencer is the default, the Felucca groovebox kept as a legacy menu mode, owner decision 32), produce a v2 that is the sequencer only: remove the groovebox and every subsystem the v1 SYSTEM_MAP marks 'v2 REMOVE', then use the freed flash and RAM. See docs/project/ROADMAP.md.

## Scope notes (for the planning task that breaks this down)
- Source of truth: the 'v2' column in docs/design/SYSTEM_MAP.md (P2-002), per subsystem: keep / remove in v1 / remove in v2.
- Candidates from the UX review: the groovebox mode (MENU > SYSTEM > MODE GROOVE), ARP and SEQ tools, SCL chord/voicing, GLO song/solo, FX performance effects, ENV/LFO sound editing, and anything they alone use.
- After removal: re-run tools/check.sh after every removal step, report flash and RAM freed, update ui_test.c row counts, update CLAUDE.md architecture, re-baseline golden files.
- Re-open the free-slot decisions: 6 channels (owner question 31), the momentary-morph gesture (36), unused menu rows.

## Breakdown
Broken into tasks by a planning task at the end of P8 (rolling wave). Until then, this is a placeholder.

## Log
