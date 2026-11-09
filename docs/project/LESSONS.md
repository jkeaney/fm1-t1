# Lessons learned

Orchestrator-maintained. Add a line whenever a cycle stalls, bounces or surprises us. Read before dispatching.

## Process
- **Time-box every dispatch.** A subagent that has not reported in ~15 min gets a status check (SendMessage); no reply in 5 more min, stop it and re-dispatch narrower. Never sit silent on a wait.
- **Full host suite takes ~20 min.** Executors run it once. Reviewers must NOT rerun it unless code changed; for docs-only changes, spot-check. Run slow things with a log file and a hard timeout.
- **Reviewers waiting on a Monitor can stall** (P0-002: watched a run that had already finished). Tell reviewers to check for live processes/log tail, not wait blindly.
- **Trivial doc-only findings: reviewer fixes them directly** and commits, rather than failing the task (P0-002 cost a full extra round trip for a one-line edit).
- **Acceptance criteria must be checkable.** P0-001's "ahead of" check could never pass without an upstream; reword criteria that depend on state that does not exist.
- **Amending a commit changes its hash**: log the hash after the final amend, or log the subject line instead.
- **Executors must not touch local git config** (.git/info/exclude) without saying so in the log (P0-001 did; it was disclosed, fine).
- **Model routing:** owner asked for Haiku only for dispatches. Tasks whose executor field says sonnet/opus (P1+, design) conflict with that; ask the owner before dispatching them on a bigger model, or run them on Haiku with narrowed scope.

## Technical
- Spaced repo path breaks `import.meta.url.pathname`; use `fileURLToPath`.
- Wall-clock benchmarks are gated behind `T1_BENCH=1`.
- **Owner decision 2026-11-09: ALL work runs on Haiku**, including P1/P2 design and E3–E8 epics (executor fields changed to haiku). Compensation: briefs must be narrow and explicit; design tasks keep their human-review gates as the quality check; split any task that is too big for Haiku before dispatching it; the reviewer verifies against the vision, not just the checklist.
- **Owner decision 2026-11-09 (supersedes the Haiku-only line above): planning and design tasks (P1, P2) run on Sonnet; development tasks (P0, P3+) run on Haiku.** Orchestrator dispatches by the task's executor field.
- **Design must be flexible (ADR-0004 addendum):** control map, screens and algorithm layer/morph/dice mappings are data tables so producer-driven workflow changes after testing are table edits, not code changes.
- **Owner decisions 2026-11-09 recorded in docs/project/OWNER_QUESTIONS.md** (flash skipped, slide in v1, scale/GATE/DIV/SWING global). Design tasks must read it.
- **Memory trigger (owner 2026-11-10):** if RAM/flash runs short, re-prioritise groovebox clean-up (E9). Reviewers: report RAM headroom from the build line on every dev task; under 8 KB headroom means file a BUG and tell the orchestrator.
