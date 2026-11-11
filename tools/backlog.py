#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Backlog tool: the single source of "what's next" (docs/project/PROCESS.md). Python stdlib only.

  backlog.py next [--executor haiku]     the next ready task: status todo, every depends_on done; by priority, then id
  backlog.py list [--phase P1] [--all]   status table (done and dropped hidden unless --all)
  backlog.py show ID                     print a task file
  backlog.py set ID STATUS               change status (todo ready* in_progress review done blocked dropped)
  backlog.py check                       Definition of Ready, unknown deps, cycles, duplicate ids; exit 1 on errors
  backlog.py index                       regenerate backlog/BACKLOG.md
  (* ready is computed, never stored)
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BACKLOG = ROOT / "backlog"
DIRS = ("tasks", "bugs", "feedback")
STATUSES = ("todo", "in_progress", "review", "done", "blocked", "dropped")
TYPES = ("task", "spike", "bug", "feedback", "epic")
EXECUTORS = ("haiku", "sonnet", "opus", "human")
GATES = ("none", "human-review", "hardware-test")
REQUIRED = ("id", "title", "phase", "type", "status", "priority", "executor", "depends_on", "gate")
SECTIONS = ("## Goal", "## Read first", "## Deliverable", "## Acceptance criteria", "## Verification")


def parse(path):
    text = path.read_text()
    m = re.match(r"---\n(.*?)\n---\n", text, re.S)
    if not m:
        return None
    meta = {}
    for line in m.group(1).splitlines():
        line = line.split("#", 1)[0].rstrip() if not line.lstrip().startswith("title") else line.rstrip()
        if ":" not in line:
            continue
        k, v = line.split(":", 1)
        v = v.strip()
        if v.startswith("[") and v.endswith("]"):
            v = [x.strip() for x in v[1:-1].split(",") if x.strip()]
        meta[k.strip()] = v
    meta["_path"] = path
    meta["_body"] = text[m.end():]
    return meta


def load():
    items = []
    for d in DIRS:
        for p in sorted((BACKLOG / d).glob("*.md")):
            t = parse(p)
            if t:
                items.append(t)
    return items


def prio(t):
    try:
        return int(t.get("priority", 9))
    except ValueError:
        return 9


def is_ready(t, by_id):
    return t.get("status") == "todo" and t.get("type") != "epic" and all(
        by_id.get(d, {}).get("status") == "done" for d in t.get("depends_on", []))


def shown_status(t, by_id):
    return "ready" if is_ready(t, by_id) else t.get("status", "?")


def sort_key(t):
    return (prio(t), t["id"])


def check(items):
    errors, by_id = [], {}
    for t in items:
        tid = t.get("id", "?")
        if tid in by_id:
            errors.append(f"{tid}: duplicate id ({t['_path'].name}, {by_id[tid]['_path'].name})")
        by_id[tid] = t
        for k in REQUIRED:
            if k not in t:
                errors.append(f"{tid}: missing front matter '{k}'")
        if not t["_path"].name.startswith(tid + "-") and t["_path"].stem != tid:
            errors.append(f"{tid}: file name {t['_path'].name} should start with '{tid}-'")
        for k, allowed in (("status", STATUSES), ("type", TYPES), ("executor", EXECUTORS), ("gate", GATES)):
            if k in t and t[k] not in allowed:
                errors.append(f"{tid}: {k} '{t[k]}' not one of {allowed}")
        if not isinstance(t.get("depends_on", []), list):
            errors.append(f"{tid}: depends_on must be a [list]")
        if t.get("type") != "epic":
            for s in SECTIONS:
                if s not in t["_body"]:
                    errors.append(f"{tid}: missing section '{s}'")
            if t.get("status") not in ("done", "dropped") and "## UX gate" not in t["_body"]:
                errors.append(f"{tid}: open task needs a '## UX gate' section (docs/project/UX_GATE.md; 'n/a, <reason>' is allowed)")
            if "- [" not in t["_body"].split("## Acceptance criteria", 1)[-1]:
                errors.append(f"{tid}: acceptance criteria need '- [ ]' checklist items")
    for t in items:
        for d in t.get("depends_on", []):
            if d not in by_id:
                errors.append(f"{t['id']}: depends on unknown '{d}'")
    # cycles (DFS)
    state = {}

    def visit(n, stack):
        if state.get(n) == 1:
            errors.append("dependency cycle: " + " -> ".join(stack + [n]))
            return
        if state.get(n) == 2 or n not in by_id:
            return
        state[n] = 1
        for d in by_id[n].get("depends_on", []):
            visit(d, stack + [n])
        state[n] = 2

    for n in by_id:
        visit(n, [])
    return errors


def table(items, by_id):
    rows = ["| ID | Status | Pri | Executor | Gate | Depends on | Title |", "| --- | --- | --- | --- | --- | --- | --- |"]
    for t in items:
        rel = t["_path"].relative_to(BACKLOG)
        rows.append(f"| [{t['id']}]({rel}) | {shown_status(t, by_id)} | {t.get('priority')} | {t.get('executor')} | "
                    f"{t.get('gate')} | {', '.join(t.get('depends_on', [])) or '–'} | {t.get('title')} |")
    return "\n".join(rows)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    n = sub.add_parser("next")
    n.add_argument("--executor")
    ls = sub.add_parser("list")
    ls.add_argument("--phase")
    ls.add_argument("--all", action="store_true")
    sh = sub.add_parser("show")
    sh.add_argument("id")
    st = sub.add_parser("set")
    st.add_argument("id")
    st.add_argument("status", choices=STATUSES)
    sub.add_parser("check")
    sub.add_parser("index")
    a = ap.parse_args(argv)
    items = load()
    by_id = {t["id"]: t for t in items}

    if a.cmd == "check":
        errs = check(items)
        for e in errs:
            print("ERROR", e)
        print(f"{len(items)} items, {len(errs)} errors")
        return 1 if errs else 0
    if a.cmd == "next":
        ready = sorted((t for t in items if is_ready(t, by_id) and (not a.executor or t.get("executor") == a.executor)),
                       key=sort_key)
        if not ready:
            waiting = [t["id"] for t in items if t.get("status") in ("review", "blocked")]
            print("no ready task" + (f" (waiting on review/blocked: {', '.join(waiting)})" if waiting else ""))
            return 1
        t = ready[0]
        print(f"{t['id']}  [{t.get('executor')}, gate {t.get('gate')}]  {t.get('title')}\n  {t['_path'].relative_to(ROOT)}")
        return 0
    if a.cmd == "list":
        sel = [t for t in items if (not a.phase or t.get("phase") == a.phase)
               and (a.all or t.get("status") not in ("done", "dropped"))]
        print(table(sorted(sel, key=lambda t: (t.get("phase", ""), t["id"])), by_id))
        return 0
    if a.cmd == "show":
        t = by_id.get(a.id)
        if not t:
            print(f"no item {a.id}")
            return 1
        print(t["_path"].read_text())
        return 0
    if a.cmd == "set":
        t = by_id.get(a.id)
        if not t:
            print(f"no item {a.id}")
            return 1
        p = t["_path"]
        text = re.sub(r"^status:.*$", f"status: {a.status}", p.read_text(), count=1, flags=re.M)
        p.write_text(text)
        print(f"{a.id}: {t.get('status')} -> {a.status}")
        return 0
    if a.cmd == "index":
        out = ["# Backlog", "", "Generated by `python3 tools/backlog.py index`: do not edit by hand. Process: "
               "[docs/project/PROCESS.md](../docs/project/PROCESS.md). Roadmap: [docs/project/ROADMAP.md](../docs/project/ROADMAP.md).", ""]
        for ph in sorted({t.get("phase", "") for t in items}):
            out += [f"## {ph}", "", table(sorted((t for t in items if t.get("phase") == ph), key=lambda t: t["id"]), by_id), ""]
        (BACKLOG / "BACKLOG.md").write_text("\n".join(out))
        print(f"wrote backlog/BACKLOG.md ({len(items)} items)")
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
