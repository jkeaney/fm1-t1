<!-- SPDX-License-Identifier: GPL-3.0-only -->
# 06 Process: how to work on this design

## 1. How the owner wants to be worked with

- **Short and visual.** Send images with `SendUserFile` (load it via `ToolSearch select:SendUserFile`); ask for a
  one-word verdict ("good enough" / "not"). The owner said exactly that: "Show me the mocks here. I'll give a simple
  it's good enough or not."
- **No jargon dumps.** The owner is a producer, not a designer or an embedded engineer. Say what the screen does for a
  player, not what the generator does.
- **Ask with `AskUserQuestion` when you need a decision**, but **show pictures of every option first**. (A text-only
  multiple choice produced the decision that may now be wrong.)
- **Never flash the device or push to git without asking.** Commit locally freely, with the trailers in section 6.
- The owner reads what you say in chat but not the files; summarise results in plain language with the numbers that
  matter (score, counts), not file lists.

## 2. The repo's own process (read `CLAUDE.md`, `docs/project/PROCESS.md`, `docs/project/UX_GATE.md`)

- All firmware work goes through the **backlog** (`backlog/`; `python3 tools/backlog.py next|list|show|set|check|index`).
  **Do not patch firmware outside a task.** You design; the build is tasks.
- **UX gate (UX_GATE.md):** every task a producer can touch has a **feature use case** in `docs/design/features/`
  (`_TEMPLATE.md`): Gate A before the build (the steps a producer takes, predicted ease score V/I/O/L, five-surface sync,
  live checklist), Gate B after. Ease-score pass rule: performance functions >= 11/12 with no sub-score below 2; setup
  functions >= 9/12 with no 0. `tools/backlog.py check` fails an open task without a `## UX gate` section.
- **Design lock (ADR-0004):** no development task is ready until the design is locked: fully specified, prototyped with
  golden outputs, validated by scripted scenarios, **signed off by the owner** ("Design locked: <name> <date>").
- Model routing: planning/design on Sonnet, development on Haiku, human gates for flashing/listening/approving.
- Lessons that cost time: never run two `tools/check.sh` at once (shared `build/`); **never write to or `git rm` a path
  containing a literal `*`** (it creates stray files / matches wildcards; resolve the real path first); a Haiku task blocked
  on missing callers is a spec gap, route it to a Sonnet planner.

## 3. The mock-up tool (`docs/design/ui_review/mock/`; read its `README.md`)

Pixel-exact: the **real** bitmap fonts and palettes parsed from `build/gen/`. Run `./build.sh` once so `build/gen/`
exists. Use the repo venv `.venv/bin/python`. From the repo root, with `M=docs/design/ui_review/mock`:

```sh
.venv/bin/python $M/mock.py render  <proposal.py> --out DIR     # NAME.png (1x), NAME_4x.png, NAME_grid.png (4x + pixel grid + zone outlines)
.venv/bin/python $M/mock.py diff    A.png B.png --out diff.png  # count, clusters, highlighted diff
.venv/bin/python $M/mock.py measure "TEXT" --font S --cell 36   # advance, ink box, cap height, does it fit?
.venv/bin/python $M/mock.py contrast TEXT BG --palette GREY     # WCAG ratio (palette tokens, 0xRGB565 or #rrggbb)
.venv/bin/python $M/mock.py palette GREY                        # a palette's tokens as RGB565/hex
.venv/bin/python $M/check_fidelity.py --regen                   # the gate: the tool still reproduces the shipping render (0 px diff)
```

A proposal is a Python file exposing `draw(canvas, state)` and a `STATE` dict (README "Defining a proposal"). It warns
when text is clipped by the blit strip or ellipsised, and when a canvas exceeds 240 x 124. The shipping renders and
fixtures are reproduced by `mock/current_perf.py` (0 differing pixels). **Always render with the tool, never draw by
hand**, or the fonts will not be real.

## 4. The critique method (the Impeccable skill; version 3.8.0 is installed in the plugin cache)

The owner asked for the **Impeccable** skill to review the design. It is installed at
`/Users/jameskeaney/.claude/plugins/cache/impeccable/impeccable/3.8.0/skills/impeccable/` (a newer 4.8.2 exists; `npx
impeccable update` cannot update a plugin-cache install: update through the plugin manager). `PRODUCT.md` (repo root) is
the project context it requires (register: product). Method, from `reference/critique.md`:

1. **Resolve the target** to a path (e.g. `docs/design/ui_review/proposals/v3.2/proposal_v3_2.py`) and compute a slug:
   `node <skill>/scripts/critique-storage.mjs slug "<path>"`.
2. **Two independent assessments, run by separate agents that must not see each other's output:**
   - **A: design review** (a fresh design director: AI-slop verdict, hierarchy, cognitive load checklist, emotional
     journey, the 10 Nielsen heuristics scored 0-4, personas Alex/Sam/Jordan plus the project persona "Dana", the
     viewing-distance test at the real 30 mm). **A must not read the builder's NOTES.md or any prior assessment.**
   - **B: deterministic evidence** (re-render and diff, text inventory with arcmin, contrast, grid, an edge-case sweep,
     accuracy against the TSVs, a real attempt at the detector). **B reports facts only**, no opinions.
   - The Impeccable detector (`detect.mjs`) only scans **web markup**; on this repo it scans zero files and says `[]`:
     report that as a **non-scan**, not a clean result.
3. **Synthesize** one critique (health score table, anti-patterns verdict, overall impression, what works, priority issues
   P0-P3 with suggested commands, persona red flags, minor, questions), **persist a snapshot** with
   `IMPECCABLE_CRITIQUE_META='{...}' node <skill>/scripts/critique-storage.mjs write <slug> <bodyfile>` and read the
   trend with `... trend <slug> 5`. Snapshots live in `.impeccable/critique/` (committed). Note the slug depends on the
   path string, so **use the same path every time** or the trend will not link.
4. **Ask the owner targeted questions** (priority, intent, scope) and only then plan the fixes.
Prior runs: `critique_v3/` (24/40) and `critique_v3_2/` (27/40), with their scripts so you can re-run them.

## 5. How the previous session delegated (and how to do it better)

- Builders and reviewers were **subagents** (`Agent` tool): `general-purpose` Sonnet for mock builders and evidence,
  **Opus** for the independent design review, each told which paths they may write and not to run `tools/check.sh` or
  `./build.sh` (the shared build), and to commit only their own folder with explicit paths.
- **Brief with numbers, not adjectives.** The first mock-up agents re-created the dense small-text screen until the brief
  stated the arcmin figures and a text-count cap.
- **Make builders list "least sure about" and "owner questions"**: they did, and it surfaced real problems.
- **Do not let a builder grade its own work.** Always follow a build with an independent critique; always verify two or
  three claims yourself (the SAVE-hold label error was caught only by checking `button_modes.tsv`).
- **Look at the renders yourself** (the `Read` tool shows images; upscale 3-4x with Pillow NEAREST to see pixels).
- **Smaller loops.** A 70-scene proposal takes 20-30 minutes to build and 20+ to critique. Prefer a **vertical slice**
  (the main screen in 3-4 states) for the owner to react to before building the states sheet and maps.

## 6. Commits

Trailers on every commit made from a Claude session (the current session's session id will differ; use yours):
```
Co-Authored-By: Claude <model> <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_<your id>
```
Commit docs with explicit paths. Do not `git add -A` blindly (the working tree may contain another agent's files).
