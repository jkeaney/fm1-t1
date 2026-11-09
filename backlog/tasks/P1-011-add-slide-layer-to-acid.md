---
id: P1-011
title: Add a slide dice layer to the ACID design
phase: P1
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P1-003]
gate: none
---
## Goal
Owner decision: ACID gets slide (TB-3 portamento) as a sixth dice layer in v1, with no guessing left for a dev task (ADR-0004).

## Read first
- docs/design/algorithms/sting.md
- docs/project/OWNER_QUESTIONS.md (decisions 2-4: scale and GATE/DIV/SWING are global)
- docs/sting-algorithms/ (how slide works in the source; mind [V]/[I]/⚠)
- docs/project/decisions/ADR-0004-design-lock.md

## Deliverable
- docs/design/algorithms/sting.md updated: a SLIDE dice layer (label at most 6 chars) with its own seed, probability table, undo, procedure and how slide is emitted as MIDI for a TB-3 (overlapping note-on before note-off, legato rules, and any CC); parameters table row; RAM update
- Section 2/6/7 updated; scale wording changed to use the single global scale (no ACID-private scale snap controls)

## Acceptance criteria
- [x] Section 6 lists 6 dice layers including SLIDE; still at most 4 knob morphs; labels at most 6 chars
- [x] The slide MIDI emission rule is fully specified, with a worked 8-step example showing note-on/off order
- [x] No TBD/choose/decide wording; any ⚠ reliance listed in section 7
- [x] Scale and GATE/DIV/SWING are described as global settings

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/sting.md || echo missing "$h"; done   # prints nothing
grep -n 'SLIDE' docs/design/algorithms/sting.md
```

## Out of scope
- Other algorithms; code.

## Log

### 2026-11-09 executor (sonnet)
Design only. Verification: section-heading loop printed nothing; `grep -n SLIDE docs/design/algorithms/sting.md` shows 14 hits (layer table, params row `s_slide`, salt, 6.3 procedure, 6.3a emission, section 7 row 9). `grep -nE 'TBD|choos|decid'` on sting.md prints nothing wrong (no TBD/choose/decide left).
Changes: sting.md sections 1,2,3,4,5,6 (6.1 params, 6.2 salt, 6.3 SLIDE layer, new 6.3a MIDI emission with worked 8-step example, 6.4 RAM 220 -> 254 B, 6.6), 7 (new row 9), 8. Default-seed formula now `ch*6+k`. Scale/GATE/DIV/SWING reworded as global (owner decisions 3, 4).
Design: SLIDE has own seed/salt, fixed probability table `ps`, undo (previous seed), no knob; slide = overlapping note-on before previous note-off by D = max(L/8, 1 block); same-note overlap becomes a tie; no CC; one-step-ahead commit of the next step.
Note: the source (sting) has no slide output (only "Slide" modulation dials), so 6.3a is entirely [D]; TB-3 glide on overlap is an unverified assumption (section 7 row 9).
Owner questions added to docs/project/OWNER_QUESTIONS.md as #7 (no slide-off control), #8 (confirm overlap glides on TB-3, CC65 fallback), #9 (one-step dice latency).

### 2026-11-09 review (sonnet)
Verdict: PASS (gate none -> done). Section loop prints nothing; 6 layers incl SLIDE, 4 morphs, labels <= 6 chars; no TBD/choose/decide; scale/GATE/DIV/SWING global. Re-derived the 8-step event table: 6 note-ons, 6 note-offs, times all correct, tie adds neither, on-before-off on slide pairs, every note has an off before the end. Other algorithms' seeds (snake ch*4) are independent, nothing else references the old formula. TB-3 glide assumption flagged (section 7 row 9, OWNER_QUESTIONS #8).
Fixed directly in sting.md: RAM was understated (overlap needs two sounding notes and two pending offs: 254 -> 258 B; stale "190 B" in section 5, "106 B" -> 114 B, cache "64 bytes" -> 48); stop/panic rule now says every sounding note (up to two during overlap) plus CC123; clarified the seed row in 6.4.
Latency: next-step commit makes a dice/knob change audible up to 2 steps later (1 extra step, ~125 ms at 120 BPM 1/16). Borderline for rule 1 on knobs; the screen still updates at once. Added a mitigation to 6.3a (snapshot only hit/note/slide of i+1, read vel/gate live) in case owner rejects it (OWNER_QUESTIONS #9).
