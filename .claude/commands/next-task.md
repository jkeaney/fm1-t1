---
description: Run one backlog cycle (triage, pick, execute with a subagent, review with a subagent, update the backlog)
argument-hint: "[task id to force] [--executor haiku]"
---
You are the ORCHESTRATOR for one cycle of docs/project/PROCESS.md. You never implement tasks yourself. The runbook is docs/project/ORCHESTRATOR.md: parallelism, escalation of Haiku to Sonnet (blocked, or two failed reviews), gates and cost rules.

1. **State.** Run `python3 tools/backlog.py check` (fix any front-matter errors before continuing) and
   `python3 tools/backlog.py list`. Then:
   - If any item is at `review` with a `human-review` or `hardware-test` gate and has no "Approved:" line, tell the
     owner exactly what to check and ask whether they approve.
   - If they approve, append "Approved: <owner> <date>" to the task's Log, `set <ID> done` and commit.
2. **Triage.** If any `type: feedback` item is `todo`, triage it before anything else:
   - Create BUG-/P*- items from backlog/_templates/ (or drop the feedback with a reason).
   - Set the feedback to `done` and commit.
   - High-severity bugs get priority 0.
3. **Pick.** Use $ARGUMENTS if it names a task id. Otherwise run `python3 tools/backlog.py next` (pass
   `--executor` through if given).
   - `executor: human`: tell the owner what to do (the task's Deliverable and Verification) and stop.
   - Nothing ready: report what is blocked or awaiting the owner, and stop.
   - A task that would flash a device, push or delete user data: ask the owner first.
4. **Execute.** Dispatch the `fm1-task-executor` agent with the task id and path.
   - Set the Agent `model` to the task's `executor` field (haiku/sonnet/opus).
   - The prompt is just: "Execute backlog task <ID> (<path>). Follow your instructions."
5. **Review.** When the executor reports `review`, dispatch `fm1-task-reviewer` (model sonnet) with the same id. On a FAIL, send the findings to the same executor with SendMessage and re-review; after a second FAIL on a Haiku task, re-dispatch on Sonnet (ORCHESTRATOR.md 4). If the executor
   reported `blocked`, show the owner the reason and stop.
6. **Close.** Run `python3 tools/backlog.py list` and report to the owner in at most 8 lines: what was done, the
   verdict, what's next (`backlog.py next`), and anything awaiting them. Update docs/project/HANDOFF.md (State, Path, open owner items) and commit it. One task per run.
