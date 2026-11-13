# Prompt for a fresh session (paste this)

You are continuing the FM-1 sequencer firmware project (repo: this folder, branch t1-hardware-bringup). No prior context
is assumed. Read, in this order:
1. CLAUDE.md (product rules, hardware facts, commands, architecture).
2. docs/project/HANDOFF.md (state, owner rulings, execution order, process rules, lab state).
3. `python3 tools/backlog.py next` and `python3 tools/backlog.py list` (the order is the `priority:` field).
Then read the next task file and its "Read first" list. Docs index: docs/project/ORCHESTRATOR.md, PROCESS.md, ROADMAP.md.

Standing rules:
- All work goes through the backlog; one agent builds or runs `sh tools/check.sh` at a time (expect CHECK PASS).
- Executor agent then a Sonnet reviewer (`/next-task`); one complete findings list per review; docs-sync pass every task.
- Screens and ISR work run on Sonnet. Do not change firmware outside a task.
- NEVER flash the unit or push to origin without asking the owner first; push in blocks and ask each time.
- Human gates need the owner's own `Approved:` line. He answers in a word; use AskUserQuestion with plain context, and
  the open-items skill when he asks what you need from him.
- Hardware is the judge: host tests do not prove the screen or the timing.
- Commit with explicit paths and end messages with the Co-Authored-By and Claude-Session trailers you are given.

Start by running `python3 tools/backlog.py next`, then list the open items for the owner (what waits on him) and begin
the first ready task with `/next-task`.
