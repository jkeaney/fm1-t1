# ADR-0004: Design lock before development

Status: accepted (owner directive, 2026-11-09)

## Context
This product is UX and algorithm driven, and every task runs on Haiku. Haiku must not guess. Design gaps found
during development become rework on hardware.

## Decision
No development task (P3 onward) may become ready until the design it implements is **locked**. A design is locked when:

1. **Fully specified, no choices left.** Control map = a table: every control × every context → exact action and
   on-screen label. Screen spec = exact text, positions, states. Algorithm = parameters with ranges and defaults,
   the step-by-step procedure, and how each knob (morph) and button (dice) acts on each layer.
2. **Tested as a prototype.** Each v1 algorithm has a seeded, deterministic host reference implementation
   (docs/design/algorithms/<name>/) whose output is checked in as golden files. Dev tasks reproduce the golden output.
3. **Validated by scenario.** Scripted walkthroughs (docs/design/scenarios/) cover the target workflow (TB-3: select
   channel 1 → set MIDI channel → choose algorithm → enter or dial a sequence → perform) with the exact expected
   screen text and MIDI at each step. Every control-map row is exercised by at least one scenario.
4. **Signed off.** The owner writes "Design locked: <name> <date>" in the log of the design task.

Every dev task then **cites** its locked spec section, golden file and/or scenario step in Read first, and its
acceptance criteria are those artefacts. A dev task with an open question ("TBD", "choose", "decide") is not ready;
the executor stops and files a BUG/FB instead of guessing.

## Consequences
- Extra design-validation tasks (P2-007, P2-008) before P2-005 planning.
- P2-005 must refuse to create a dev task that lacks a spec citation.
- Design changes after lock go through a new ADR or an amended spec plus owner re-lock, never a dev task.
