<!-- SPDX-License-Identifier: GPL-3.0-only -->
# UX gate: how a feature is judged before and after it is built

Owner directive 2026-11-12. Every task that adds or changes something a producer can touch (a button, knob, key,
screen, page, algorithm, morph, dice layer, MIDI behaviour) passes two gates: **Gate A before the build** and
**Gate B after it**. Both use the existing ease score (UX_REVIEW.md section 3: V visual cue, I input effort,
O output feedback, L learnable without a manual; /12). Purely internal work (tooling, refactors, tests) writes
`UX gate: n/a, <reason>` in its task and skips both gates.

## The artifact: one feature use case per feature

`docs/design/features/<TASK-ID>-<slug>.md`, from `docs/design/features/_TEMPLATE.md`. It is the single record that
both gates read and write. Gate A fills sections 1-6; Gate B fills 7-9. The task cites it in **Read first**.

## Gate A: before the build (design sign-off, part of Definition of Ready)

The author (a Sonnet planner for design tasks) answers, in the use case:

1. **Who and when.** The producer's situation in one line (for example: "bass line is running, wants a busier bar
   without stopping").
2. **The steps.** The exact gesture list a producer performs, from the main screen, with the control id from
   `control_map.tsv` and the screen text the producer sees after each step. If it needs more than two gestures, or
   leaves the main screen for a page, say why that is acceptable (setup functions only).
3. **Classify each function** as Performance (live) or Setup, as UX_REVIEW does.
4. **Predicted score** V/I/O/L per function, with the reason for each number. Pass rule: performance >= 11/12 with
   no sub-score below 2; setup >= 9/12 with no 0. A predicted fail goes back to design, not to build.
5. **Sync check (the five surfaces).** Name the row in each, adding or changing the row as part of the same task:

   | Surface | Source of truth | What must agree |
   | --- | --- | --- |
   | Buttons and knobs | `control_map.tsv`, `button_modes.tsv`, `knob_map.tsv`, `key_map.tsv` | the control does this, in this context, and nothing else conflicts |
   | UI / screen | `SCREEN_SPEC.md`, `page_states.tsv` | label text, position, width (`widths.tsv`), cue strip, what is latched |
   | UX / help | `help_text.tsv`, `UX_REVIEW.md` | the hold-HOME help line and the printed-word fit |
   | Workflow | `docs/design/scenarios/` | a scenario step (or a new scenario) walks the use case; `check_scenarios.py` still shows no gap |
   | Live performance | section 6 below | passes the live checklist |

6. **Live checklist** (can I drive and produce music live with it?). Every item is yes, no with a reason, or n/a:
   - reachable from the main screen, no menu, no page change to start it (performance functions);
   - undoable in one press (EDIT) or self-reverting on release;
   - lands on a musical boundary: next step or next bar, never mid-note; no hung or cut notes (CC123 on stop);
   - the screen shows the new value within one frame and the sound changes by the next step;
   - works with one hand while the other is on the keys or a knob;
   - does not change another channel, or says so on screen;
   - survives stop/start, MIDI-clock source change and a preset load;
   - CPU and RAM cost recorded against the budget (RAM headroom >= 8 KB).

Gate A sign-off: the Sonnet reviewer for ordinary features; the owner (`human-review`) when a button, knob or
page assignment changes, or any prediction is below its pass rule with a reason to proceed. Write
`Design locked: <name> <date>` in the use case.

## Gate B: after the build (validation, part of Definition of Done)

The reviewer (and the owner on `hardware-test`) fills sections 7-9:

7. **Walk it.** Perform the section 2 steps on the host UI test or a real FM-1; record per step what the screen
   showed and what MIDI left (listener log, `tools/fm1_midi_listen.py`). Differences from the prediction are listed.
8. **Actual score** V/I/O/L from what was observed, against the predicted one. Any function that now fails its pass
   rule fails the task. A score that dropped by 2 or more is a finding even if it still passes.
9. **Sync re-check** of the five surfaces: run `python3 tests/design/check_scenarios.py`, `check_help.py` and the
   host UI tests; confirm the TSVs, SCREEN_SPEC and help text match the firmware. Then the live checklist is
   re-answered from observation, not prediction. A feature that is not on the owner's real FM-1 is not validated
   (CLAUDE.md: hardware check before done), so the task stays `review` until the owner's "Approved:" line.

Outcomes: **Pass** (task done), **Pass with debt** (a FB- item is filed for each gap and linked), **Fail** (back to
todo with the failing function and sub-score named).

## Where this plugs in

- `PROCESS.md`: Definition of Ready requires Gate A done for UX-facing tasks; Definition of Done requires Gate B.
- `backlog/_templates/task.md`: has a `## UX gate` section pointing at the use case.
- `fm1-task-reviewer`: checks the gate as step 3.
- Not yet automated: `tools/backlog.py check` does not require the section. Add that (required when
  `ux: true` in front matter) once the first two features have used the gate and the owner is happy with the form.
