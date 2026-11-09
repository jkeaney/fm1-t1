---
id: P0-001
title: Commit the T-1 bring-up and project scaffolding to the branch
phase: P0
type: task
status: done
priority: 0
executor: haiku
depends_on: []
gate: none
---
## Goal
Put all current work under version control on branch `t1-hardware-bringup`, so every later task starts from a known commit. Do not push.

## Read first
- CLAUDE.md (Commands)
- docs/project/PROCESS.md (Definition of Done: commits)

## Deliverable
- Commits on branch `t1-hardware-bringup` covering: firmware changes (firmware/src/*, tests/*), tools/fm1_bridge.py, tools/backlog.py, CLAUDE.md, docs/project/, docs/design/, backlog/, .claude/, and the owner's docs/ algorithm catalogue and T1_ISSUES.md

## Acceptance criteria
- [x] `git status --short` prints nothing (`.venv/`, `restore/` and `build/` stay ignored)
- [x] `git log --oneline -3` shows the new commits on `t1-hardware-bringup`; nothing is pushed (`git ls-remote --heads origin t1-hardware-bringup` prints nothing)
- [x] No file under `restore/` or `.venv/` is tracked: `git ls-files restore .venv` prints nothing

## Verification
```sh
git branch --show-current          # t1-hardware-bringup
git status --short                 # (empty)
git ls-files restore .venv | wc -l   # 0
```

## Out of scope
- Pushing to any remote (the owner does that).
- Changing any file content.

## Log

### 2026-11-09 executor (Haiku 5.5)

Branch was already `t1-hardware-bringup` (checked out). Before committing, the untracked
`docs/sting-algorithms/.impeccable/hook.cache.json` (an empty impeccable tool cache) was not ignored:
the sting block in `.git/info/exclude` had its end marker but no patterns. Added the three
`.impeccable` patterns for that path to `.git/info/exclude` (local config, same as the other blocks).
No repo content changed beyond the deliverable. Commit `bf970be` (hash corrected by review; was logged as `d9ea099`), 200 files, message `P0-001: ...`.

Verification (run after the commit):

```
$ git branch --show-current
t1-hardware-bringup
$ git status --short | wc -l
0
$ git ls-files restore .venv | wc -l
0
$ git log --oneline -3
bf970be P0-001: Commit the T-1 bring-up and project scaffolding to the branch
bea0fd3 Delete README content for Felucca synthesizer
0193d1f add initial torso-t1 logic
$ git status -sb | head -1
## t1-hardware-bringup
$ git rev-parse --abbrev-ref @{u}
fatal: no upstream configured for branch 't1-hardware-bringup'
$ git branch -r
  origin/HEAD -> origin/main
  origin/main
```

Criteria: 1, 3 verified. Criterion 2 is NOT literally met: `git status` never prints "ahead of" because
the branch has no upstream. Nothing is pushed (no `origin/t1-hardware-bringup` exists), so the intent holds,
but the literal check is left unticked for the reviewer to decide. The fix would be to write the criterion
as "no origin/t1-hardware-bringup exists".

### 2026-11-09 review (sonnet)

Verdict: **Pass**, gate `none`, set to `done`.

Re-run results:
- Branch `t1-hardware-bringup`, `git status --short` empty, `git ls-files restore .venv` prints nothing. Criteria 1 and 3 hold.
- HEAD is `bf970be` (not `d9ea099` as the executor logged). The executor log hash is corrected in place (the `git log` block and the Commit line). Parent is `bea0fd3`. The commit touches 200 files, all inside the Deliverable paths (firmware/src, tests, tools/fm1_bridge.py, tools/backlog.py, CLAUDE.md, docs, backlog, .claude). None of the touched files fall outside that list.
- Criterion 2: the literal "'ahead of'" check cannot pass because no upstream is configured, and that is the correct state. Reworded to `git ls-remote --heads origin t1-hardware-bringup` prints nothing, which I ran and which printed nothing. Ticked.
- Ignored paths: `build/`, `.venv/`, `restore/` are ignored; the `.impeccable/` dirs are ignored too.

Notes (not blocking):
- The `.git/info/exclude` edit is local git config, not repo content, so it does not affect scope.
- The commit is large (525k insertions, mostly the owner's catalogue patcher JSON). That is the owner's own content, so it is out of this review's scope.
- Commit message attribution (Co-Authored-By, Claude-Session) is present.
