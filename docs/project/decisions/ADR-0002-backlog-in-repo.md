# ADR-0002: Backlog as markdown files in the repo, run by subagents

- **Status:** accepted (2026-11-09)
- **Context:** work is to be done one task at a time by subagents, Haiku by default. They need a deterministic,
  offline, versioned task source with no tooling dependencies.
- **Decision:** one markdown file per task in `backlog/` with front matter, validated and queried by
  `tools/backlog.py` (Python stdlib only). `/next-task` (.claude/commands) runs one cycle using the
  `fm1-task-executor` and `fm1-task-reviewer` agents (.claude/agents). The process is in docs/project/PROCESS.md.
- **Consequences:** status changes are commits, which gives history for free. GitHub Issues can mirror the backlog
  later if wanted, but the files stay the source of truth.
