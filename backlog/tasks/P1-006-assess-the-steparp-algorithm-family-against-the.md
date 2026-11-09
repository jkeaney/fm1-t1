---
id: P1-006
title: Assess the steparp algorithm family against the vision
phase: P1
type: spike
status: done
priority: 2
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Decide whether and how the Step Arp step-sequenced arpeggiator becomes an algorithm choice for a channel, with layers to dice and morphs for the knobs (VISION.md rules 1, 4, 5).

## Read first
- docs/steparp-algorithms/ (all files; mind the [V]/[I]/⚠ tags)
- docs/project/VISION.md
- docs/design/algorithms/_TEMPLATE.md

## Deliverable
- docs/design/algorithms/steparp.md, following _TEMPLATE.md section by section (all 8 sections, no section left empty)

## Acceptance criteria
- [x] All 8 template sections are present and filled
- [x] Section 5 has a 0–3 score and a reason on each of the 5 rows
- [x] Section 6 names at most 4 knob morphs and at least 1 dice layer, with on-screen labels of at most 6 characters
- [x] Section 8 is exactly one of INCLUDE in v1 / LATER / REJECT
- [x] Any design point that relies on a ⚠ claim is listed in section 7

## Verification
```sh
for h in '## 1.' '## 2.' '## 3.' '## 4.' '## 5.' '## 6.' '## 7.' '## 8.'; do grep -q "$h" docs/design/algorithms/steparp.md || echo missing "$h"; done   # prints nothing
grep -E 'INCLUDE in v1|LATER|REJECT' docs/design/algorithms/steparp.md
```

## Out of scope
- Writing code.
- Assessing other families.

## Log

### 2026-11-09 executor (sonnet)
Wrote docs/design/algorithms/steparp.md (all 8 sections). Verification:
```
for h in '## 1.' ... '## 8.'; do grep -q ... || echo missing; done   -> (no output)
grep -E 'INCLUDE in v1|LATER|REJECT' steparp.md  -> line 15 (preview) and line 344 (section 8): LATER
```
Worked example numbers (default chords, orders, firing and ratcheted steps for channels 0..5) were computed with a throwaway Python model of the procedure in /tmp (not in the repo). It showed the SNAKE multiplier 0x3A5D gives the 2-tone "fifth" chord on 4 of 6 default channels, so ARP uses 0x9E37 (documented in 6.1).

Verdict: LATER. Overlap for P1-010 dedupe: ARP's HITS = SNAKE GATES / STAGE RESTS (rank-order density), VEL = same 50..127 lane as SNAKE and STAGE, RATCH = STAGE's ratchet mode (firing steps only). Only CHORD, ORDER and OCT are new (a chord-tone pool walked in an order). If included, share those primitives. Section 7: no design point depends on an unresolved warning-tagged claim; each is redefined [D].

Owner questions:
1. P1-010: ARP as LATER is recommended; if v1 needs a harmonic algorithm, promote it as a fourth algorithm on the shared rank-order/ratchet primitives. Or offer CHORD/ORDER as a pitch option for other algorithms instead?
2. Is a 2..4-note chord pool enough for a TB-3 bass, or does it want a root-on-downbeat bias?
3. Chord snap on sparse scales turns major into minor in minor pentatonic. Acceptable? (Depends on global scale snap rule, existing questions 2 and 4.)
4. Chord snap direction (at or below, as inferred in the source) vs nearest: preference?

### 2026-11-09 review (sonnet)
Verdict: PASS (gate none). Re-ran both Verification commands (no missing headers; single verdict LATER in section 8, mention in preview only). Checked: 8 sections, section 5 has 5 rows scored 0-3 with reasons, 4 morphs (OCT, DENS, RATCH, CHANCE; all labels <= 6 chars), 5 dice layers, section 7 lists every claim tagged unconfirmed with a [D] treatment, no TBD/choose/decide, spec complete enough to promote despite LATER. Re-derived the worked example with a throwaway Python model (xorshift, shuffle, snap, pool, ratchet): channels 0 and 4 match exactly (note lists, firing masks, ratcheted steps and hit counts); channel 4 playback and the 4-loop drift also check out. Overlap claims verified against snake.md (GATES, VEL) and ml185.md (RESTS, VEL, RATCH): correct. LATER rationale is consistent with VISION (TB-3 bass workflow; chord pool repetitive as bass; v1 already has ACID, SNAKE, STAGE). Musical sense OK (chord tones snapped into the scale, sparse-scale caveat disclosed). Owner questions 13-16 appended to OWNER_QUESTIONS.md (executor's four, numbered and deduped against 2 and 4). Minor nit, left: dice layer RATCH and knob RATCH share a label, harmless since they are different control roles.
