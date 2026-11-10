# Orchestrator runbook (for a Sonnet session driving the backlog)

You are the orchestrator. You never implement. You dispatch one executor subagent per task, then one independent
reviewer, commit, and bring the owner only what needs them. The map is [PROGRAM_PLAN.md](PROGRAM_PLAN.md); the live
state is `python3 tools/backlog.py list`; the process rules are [PROCESS.md](PROCESS.md).

## 1. Session start (every time, including after a context reset)

1. Read docs/project/HANDOFF.md, this file, then run `python3 tools/backlog.py check` and `python3 tools/backlog.py list`.
2. `git status` and `git log --oneline -5`. Uncommitted changes from a crashed agent: read them, then either finish
   the task's commit (if the Log shows it passed) or `git restore` them and re-dispatch the task. Never leave them.
3. List what is at `review` behind a human gate. If anything is, open with the owner sheet (section 6).

## 2. The loop (one task at a time, `/next-task` does steps 2-5)

1. `python3 tools/backlog.py next` picks the task.
2. **Dispatch the executor**: Agent `fm1-task-executor`, `model` = the task's `executor` field, prompt exactly:
   "Execute backlog task <ID> (<path>). Follow your instructions." Nothing else: the task file is the brief.
3. **Dispatch the reviewer** when the executor reports `review`: Agent `fm1-task-reviewer`, model `sonnet`, prompt:
   "Review backlog task <ID> (<path>). Follow your instructions. Report RAM headroom from the build line."
4. **On PASS**: dev tasks with `gate: none` are set `done` by the reviewer; commit if it didn't. Gated tasks stay at
   `review` for the owner.
5. **On FAIL**: send the findings back to the *same* executor agent with SendMessage (it keeps its context, so this is
   cheaper than a new agent). Then re-review with SendMessage to the same reviewer.
6. Report to the owner in at most 6 lines only when something needs them, or every 5 completed tasks.

## 3. Parallelism (keep it low: one worktree)

- **Dev tasks (P3+) run one at a time.** The firmware is one unity build in one worktree; two agents editing it
  break each other's builds.
- **Allowed in parallel:** design tasks with disjoint deliverables (P2-013, P2-014, P2-015 together), and
  P4-008 (a Python tool) alongside any one dev task.
- Parallel agents must `git add` only their own files (never `git add -A`). Tell them so in the dispatch if two
  are running.
- Use idle time behind an owner gate on the next independent task in the same phase. Don't sit waiting.

## 4. Model routing and escalation

| Situation | Model |
| --- | --- |
| Task `executor: haiku` | Haiku |
| Task `executor: sonnet` (design, scheduler, timing, ISR, cut-over, keybed scan, large UI refactor) | Sonnet |
| Reviewer, every task | Sonnet |
| Haiku executor reports `blocked` for a reason that is not a spec gap, **or** fails review twice | Re-dispatch on **Sonnet**. Log "Escalated to sonnet: <reason>" and leave the `executor` field as it is |
| Executor reports a **spec gap** (the design doesn't say) | Don't escalate. File BUG/FB against the design and route it to a Sonnet design fix; the owner approves if it changes a locked decision |
| Opus | Only when the owner asks |

## 5. Gates that stop the loop

- **Human review** (`gate: human-review`): stop at `review`, put it on the owner sheet. When the owner says approve,
  append `- Approved: owner <date> <notes>` to the Log, `set <ID> done`, commit.
- **Hardware test** (`executor: human` or `gate: hardware-test`): build first (`./build.sh`), then ask the owner for
  the OK to flash, giving the exact commands from the task. Never flash without that OK. Results the owner reports
  become Log lines. Problems become `backlog/feedback/FB-nnn` files, triaged at the next `/next-task`.
- **Push**: ask each time. Never push a branch with a failing `tools/check.sh`.
- **Memory trigger**: if any build line shows RAM headroom under 8192 B, stop feature work. Set E9 to priority 0
  and tell the owner (E9 says what to do).

## 6. The owner sheet (use the `/open-items` format)

Numbered items, blockers first. Each has plain context, lettered options and one recommendation, and the sheet ends
"Reply like: 1A, 2B". Batch everything that's waiting into one sheet: approvals, flashes, pushes, open design questions.
Combine hardware sessions where the plan allows: HW-1 with HW-T, and HW-3 with HT-FORCE.

## 7. Bugs and feedback

- Triage every `type: feedback` at `todo` before picking work (PROCESS.md "Bugs and feedback loop").
- A High-severity bug in a shipped feature gets priority 0.
- A bug found in a `done` dev task becomes a new BUG file. Don't reopen the task.

## 8. Cost discipline

- Dispatch prompts are one line: the task file is the brief. Don't paste docs into prompts.
- Don't read executor transcripts. Read the task Log and the reviewer's report.
- Re-use an agent with SendMessage for fix rounds.
- No watchdog cron unless the owner asks. Agents notify when they finish.
- Keep HANDOFF.md current at the end of each session (where we are, what's at review, what's next) in under 30 lines of change.

## 9. Definition of finished

P8-004 has the owner's `Approved:` line. Then update HANDOFF/ROADMAP to "v1 released" and ask the owner about E9.
