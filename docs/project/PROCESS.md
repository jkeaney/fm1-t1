# Process: how work moves through this project

This is a **stage-gated, rolling-wave** process built for **one task at a time, run by subagents**:

- The backlog is plain files in `backlog/`, so any agent can read, update and commit it.
- `tools/backlog.py` is the single source of "what's next".
- Phases run in order. Inside a phase, any task whose dependencies are done may run.
- Only the current and next phase are broken into tasks. Later phases stay as epics until a planning task breaks
  them down with what the earlier phases learned.

## Roles

| Role | Who | Does |
| --- | --- | --- |
| **Owner** | The human (product owner) | Sets the vision, approves `human-review` gates, runs `hardware-test` gates on a real FM-1, files feedback |
| **Orchestrator** | The main Claude session, via `/next-task` | Picks the next ready task, dispatches the executor, then the reviewer, updates status, never implements |
| **Executor** | Subagent `fm1-task-executor` (model per the task's `executor` field; Haiku by default) | Does exactly one task, runs its verification, writes its log, sets status `review` |
| **Reviewer** | Subagent `fm1-task-reviewer` (Sonnet) | Checks the deliverable against the acceptance criteria and the vision, then sets `done` or sends it back to `todo` with findings |

## Task lifecycle

```
todo ──(deps done)──▶ ready* ──▶ in_progress ──▶ review ──▶ done
  ▲                                                │
  └──────────── changes requested ◀────────────────┘
blocked: waiting on something outside the backlog (owner input, hardware, a missing tool); say what in the log
dropped: no longer needed; say why in the log
```

\* "ready" is computed by `tools/backlog.py`, not stored: status `todo` and every `depends_on` task `done`.

**Gates.** A task with `gate: human-review` or `gate: hardware-test` stops at `review` until the owner signs off in
its log ("Approved: <name> <date>"). The reviewer subagent can't pass a human gate.

## Definition of Ready (checked by `tools/backlog.py check`)

- Front matter is complete: `id title phase type status priority executor depends_on gate`.
- It has **Goal**, **Read first**, **Deliverable**, **Acceptance criteria** and **Verification** sections.
- UX-facing tasks have Gate A done in the use case (predicted score passes, sync rows named, live checklist answered).
- Every acceptance criterion can be checked by a command or a file inspection, never "works well".
- **Haiku-executed tasks name exact files, exact commands and the expected output.** A task that needs judgement
  across many files, or design taste, is `executor: sonnet` or `opus`, or is split.

## UX gate (UX_GATE.md)

Any task a producer can touch has a feature use case (`docs/design/features/`). Gate A (steps a producer takes,
predicted ease score, button/UI/UX/workflow sync, live-performance checklist) is part of Definition of Ready.
Gate B (walk it, actual score, sync re-check) is part of Definition of Done. Internal work writes `UX gate: n/a`.

## Design lock (ADR-0004)

No development task is ready until the design it implements is **locked**: fully specified (no choices left),
prototyped with golden outputs, validated by scripted scenarios, and signed off by the owner ("Design locked:" in
the log). Dev tasks cite the spec, golden file or scenario step they implement. An open question in a dev task means
stop and file a BUG/FB; never guess. Design tasks run on Sonnet and dev tasks on Haiku; the owner's `human-review` gate
is the quality check on design.

## Definition of Done

- For UX-facing tasks, Gate B is filled in the use case: actual ease score meets the pass rule and the five surfaces agree.
- Every acceptance criterion is met, and the verification commands pass, with their output pasted into the task log.
- `tools/check.sh` prints `CHECK PASS` (build, RAM headroom, host tests, backlog check; logs in build/check/).
- RAM headroom of at least 4 KB, reported by the build.
- Docs touched by the change are updated: CLAUDE.md for architecture and commands, T1_ISSUES.md for fixed issues.
- One commit per task: `<id>: <title>`. Firmware never goes onto a device without the owner's OK.

## Work types

| `type` | Use for | ID scheme |
| --- | --- | --- |
| `task` | Planned work in a phase | `P<phase>-<nnn>` |
| `spike` | Time-boxed investigation that ends in a doc or a decision | `P<phase>-<nnn>` |
| `bug` | A defect with a reproduction | `BUG-<nnn>` |
| `feedback` | Owner or hardware-session observation, triaged into tasks or bugs | `FB-<nnn>` |
| `epic` | A future phase's placeholder, broken down by a planning task | `E<phase>` |

## Bugs and feedback loop

1. **Intake.**
   - Anyone files `backlog/bugs/BUG-nnn-*.md` from `backlog/_templates/bug.md`: steps, expected, actual,
     firmware build (git SHA) and severity.
   - Hardware sessions file `backlog/feedback/FB-nnn-*.md` from `backlog/_templates/feedback.md`.
2. **Triage** happens at the start of every `/next-task` run, when there is untriaged work:
   - Untriaged feedback (`status: todo`, `type: feedback`) is converted into tasks or bugs, or dropped with a reason.
   - Bugs get a phase and priority.
   - A **High**-severity bug in a shipped feature jumps the queue (priority 0).
3. **Fix** like any task: write a failing test first where host-testable, then fix, verify and log.
4. **Hardware verification.** A bug found on hardware closes only after a `hardware-test` by the owner.

## Decisions

Lasting decisions go in `docs/project/decisions/ADR-nnnn-*.md`: context, decision, consequences. Write one when a
choice constrains later tasks, such as the data model, the algorithm interface, what gets removed or a control
assignment. Tasks reference ADRs in **Read first**.

## Running the loop

```sh
python3 tools/backlog.py next              # the next ready task (lowest priority number first, then lowest id)
python3 tools/backlog.py next --executor haiku
python3 tools/backlog.py list [--phase P1] # status table
python3 tools/backlog.py show P1-001
python3 tools/backlog.py set P1-001 in_progress
python3 tools/backlog.py check             # Definition of Ready, dependency graph, cycles
python3 tools/backlog.py index             # regenerate backlog/BACKLOG.md
```

In Claude Code: **`/next-task`** runs one full cycle (triage → pick → execute → review → commit). Run it again for
the next task.

## Model routing

Owner directive (2026-11-09): **planning and design run on Sonnet; development runs on Haiku.**

| Work | Executor |
| --- | --- |
| Planning, design, analysis, assessments, inventories, breakdown (P1, P2) | `sonnet`, plus a `human-review` gate where it locks a design |
| Development against a locked design: exact edits, tests, tooling (P0, P3+) | `haiku` |
| Flashing, listening, approving | `human` |
