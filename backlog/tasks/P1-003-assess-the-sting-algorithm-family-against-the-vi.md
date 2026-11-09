---
id: P1-003
title: Assess the sting algorithm family against the vision
phase: P1
type: spike
status: done
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Sting acid line generator becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/sting-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/sting.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [x] All 8 template sections are present and filled
- [x] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [x] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [x] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [x] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/sting.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/sting.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Design doc only; no code, no test suite. Wrote docs/design/algorithms/sting.md (all 8 sections).
```
for h in '## 1.' ... '## 8.'; do grep -q "$h" docs/design/algorithms/sting.md || echo missing "$h"; done   -> (no output)
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/sting.md -> only section 8: "**INCLUDE in v1.** ACID is ..."
```
Section 5: 5 rows scored 3/3/3/2/3. Section 6: 4 morphs (TYPE DENS JUMP ACCNT), 5 dice layers (NOTES HITS OCT GATE ACCNT), all labels <= 6 chars; parameter table, normative PRNG and procedure, RAM 220 B/channel, flex points. Section 7 lists 8 source-warning items and how each is neutralised (design defines own rules, marked [D]). Open question for owner: relation of SCL QNT to ACID snap; slide not generated.

### 2026-11-09 review (sonnet)
Verdict: PASS (gate none -> done). Verification commands re-run: no missing sections; section 8 is exactly INCLUDE in v1. Template: 8 sections, 5 scored rows with reasons, 4 morphs, 5 dice layers, all labels <= 6 chars, all source warnings that the design touches are in section 7 (others are dropped features). No TBD/decide wording. Gate tables pa/pb/base_p recomputed and match their formulas. [D] replacements are musically sound (root hammering, circular snap, octave clamp -1, 120/80 velocities).
Trivial fixes made by reviewer in sting.md: range claim "C1 to C6" corrected to MIDI 24..~82; unsigned 32-bit and integer-division wording added for golden-file reproducibility; "seeds never 0" clarified; added fallback defaults for the undefined shared settings (gate scale, DIV, SWING, SCL) so a prototype needs no guessing.
Owner questions: (1) is acid without slide acceptable for v1; (2) how shared SCL QNT relates to ACID snap; (3) channel "only setting is MIDI channel" vs shared GATE/DIV/SWING, needs the channel spec. Prototype and golden files plus owner re-lock still outstanding (ADR-0004).
