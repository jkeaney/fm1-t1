<!-- SPDX-License-Identifier: GPL-3.0-only -->
# LOCK: the six v1 algorithms, ready for the owner's design lock (P2-017, ADR-0004)

Status: **owner answers recorded 2026-11-12 (section 6); the final `Approved:` line is the orchestrator's**. Everything below is generated from the prototypes by `sh tests/design/run_protos.sh` (`PROTOS PASS`, six algorithms).
After the lock every algorithm dev task (P4-012..P4-017) is a copy-and-wire job: `proto.c` is the firmware file, `golden.txt` its expected output.

## 1. The six algorithms (what the owner is approving)

| # | Name (cell) | Idea in one line | K1 K2 K3 K4 | S1 S2 S3 S4 | Dice sub-labels (SCL SEQ ENV ARP LFO) | Seeds | Cache |
| ---: | --- | --- | --- | --- | --- | ---: | ---: |
| 0 | `ACID` | 16-step 303 line: notes, hits, octave jumps, gates and slides | DENS SLIDE JUMP ACCNT | OFFSET PEDAL PUNCH DIR | NOTES HITS ACCNT GT+SLD OCTAVE | 6 | 54 B |
| 1 | `EUCL` | Bjorklund hits, a contour melody that follows the hit, seeded accents and ratchets | HITS OFFSET RATCH ACCNT | LEN SPAN PUNCH CURVE | NOTES ROTATE ACCNT RATCH **MOD** | 4 | 38 B |
| 2 | `SNAK` | three lanes (gates, pitch, velocity) walked over a 4x4 grid by one of 14 paths; polymeter | DENS LEN RANGE PATH | OFFSET GLEN DYN DIR | NOTES GATES VELO - DRIFT | 4 | 55 B |
| 3 | `STAG` | eight stages of 1..8 pulses: rest, once, hold, or re-fire | DENS TOTAL DIR RATCH | OFFSET RDIV DYN RANGE | NOTES RESTS VELO RT+HLD LONG | 5 | 43 B |
| 4 | `ARP` | arpeggio of the HARMONY chord of the bar: 6 directions x 6 shapes x 6 rates | DENS SHAPE SPAN RATE | OFFSET LEGATO DYN DIR | VARY RESTS VELO LENGTH PHASE | 5 | 48 B |
| 5 | `PATT` | 120 library patterns (6 styles x 20), thinned / varied / pulled to the chord by the same morphs | DENS VARY SPAN ACCNT | PICK SLIDE DYN CTONE | VARY ORDER ACCNT SLIDES UP/DN | 5 | 22 B |

Every algorithm also carries `s_mod` (the SHAPE button re-rolls it together with the seed it already had) and a mod-lane hook, so the 4 device slots of MODULATION.md have a lane A and a lane B in all six. Mod lane A / B per algorithm: ACID gate position / 16-step LFO; EUCL hit ordinal / LEN LFO;
SNAK pitch cell / velocity-lane cell; STAG stage number / TOTAL LFO; ARP figure position / LENGTH LFO; PATT pattern step / pattern-length LFO (each SPEC 2.1 or 6.12a / 4.9).

## 2. Cost against PROGRAM_PLAN section 5

| Resource | Plan (section 5) | Measured or counted here | Verdict |
| --- | --- | --- | --- |
| RAM `.bss`: algorithm caches | 2 x 64 B x 4 channels already counted (ADR-0003 11) | every cache is <= 64 B (above); ARP 48, PATT 22 | no change |
| RAM `.bss`: `ChanSave` live | 4 x 344 B counted | 4 x 384 B (six `AlgoState` of 20 B) = **+160 B** | inside the 6861 B + ARP / PATTERNS allowance |
| RAM `.bss`: `UndoRec` | 4 x 92 B counted | 4 x 94 B (the other `s_mod`) = **+8 B** | negligible |
| RAM `.bss`: `ModRT` (MODULATION 7) | ~250 B | unchanged, not part of this task | counted in P2-022 |
| RAM headroom after P3 | ~46496 B; floor 4096 B; trigger 8192 B | 46496 - 6861 - 600 - 168 - 250 = **about 38.6 KB** | far above the 8 KB trigger |
| Pool: `SeqPreset` | 18 x 1396 B = 25128 B | `SeqPreset` **1556 B** (six slots) + `ModSave` 100 B = 1656 B; 18 x 1656 = **29808 B** (+4680 B) | pool free about 217975 - 29808 = 188 KB; fits 16 slots. The MODULATION 7 note about `SQ_NSLOT = 8` until R6 stands |
| Pool / RAM: PATTERNS data | 0 | **0 B** (the library is `static const`) | none |
| Flash: PATTERNS data | "pattern data" | 3992 B steps + 480 B heads = **4472 B** (+72 B names in the P5-008 name table) | ok |
| Flash: code | "~6 KB" for v1 | x86-64 `-Os` proxy, text only: ARP 2.5 KB, PATTERNS 3.7 KB (after removing its 4.5 KB table), mod primitives < 1 KB; the pi32v2 code density is unmeasured here, so read these as order of magnitude | fits the ~235 KB of image room |
| CPU (ISR) | TIMING_SPEC G6 | ARP: under 150 operations a step (RAND at most one 24-draw shuffle); PATT: about 12 000 operations a step at 32 steps, once per 16th step; mod hook: one hash and a table read per slot | PATT is the one to measure on the device (HW-ARP / H-MOD sheets) |

No resource goes over its floor. The one figure the owner may want to see changed is the PATTERNS CPU (a stateless step that re-derives its 32 steps); a cache of the sounding set would cut it to a table read at the price of 8 to 12 B of cache, a P4 optimisation that changes no golden.

## 3. The picker with six names (`width_probe.sh`, real `text_w`, AF_S)

```
 H4 (76 px)  1 ARP M16 = 59 px     1 PATT M16 = 64 px     1 PATT MOFF = 71 px  (worst of the six)
 CHB cell    [1 ARP  ]  32 px of 56            [1 PATT ]  37 px of 56
 CUE (240)   ACID EUCL SNAK STAG [ARP] PATT   185 px     (the current name in brackets; the other five cues are in widths.tsv)
 slots (40)  [ ACID ][ EUCL ][ SNAK ][ STAG ][ ARP  ][[PATT]]   26 29 30 29 22 27 px of 40
 sub-label   MOD (EUCLID under LFO) 27 of 42 px
```
`sh tests/design/width_probe.sh` reports 297 strings, 0 over.

## 4. What the prototypes found (each is written into the SPEC of its algorithm)

| # | Finding | Where |
| --- | --- | --- |
| 1 | `s_mod` lives in `ModSave`, so it is the seventh seed id (`SQ_SEED_MOD`) of the dice harness; every SHAPE layer binds it exactly once (table test) | sq_types.h, each SPEC 2.1 |
| 2 | The mod hook is `AlgoDesc.mod(cache, g, hc, step, ModIdx*)`: the algorithm says which index each lane is read at, SP18..SP20 give the values. MODULATION.md wrote `PulseOut.mod_a/mod_b`; that would move `ChanRT` offsets, so it is **not adopted** here (a core that wants the two bytes computes them from `ModIdx`) | sq_types.h, sq_prims.h |
| 3 | `sp18_cell` takes the whole index (ARP figures reach 137), not `i & 15` | sq_prims.h |
| 4 | EUCLID's cache is 38 B (the hit ordinals for lane A) | euclid/SPEC.md |
| 5 | ARP: `step()` has no `SeqGlobal`, so the cache keeps `base`; `hi` is 60; SPEC 6.8 "any shape" was wrong for OCTV | arp/SPEC.md 6.13 |
| 6 | PATT: name `PATT`; `PO_TIE` added; a 22 B cache; the C and the Python prototype agree on 126 cases | patterns/SPEC.md 4.9 |
| 7 | ARP THUMB with a drone played the drone on every step (the walking tones skipped pool index 0). **Fixed by owner O-5**: they skip the home note's pool index; at VOICE 0 + drone THUMB now plays `7 0 7 0` | arp/SPEC.md 6.13 item 7, arp golden `same, THUMB` |

## 5. Contract changes for ADR-0003 (P2-029 writes them; this task does not edit the ADR)

`sq_types.h` is final for six algorithms. Compared with ADR-0003 section 3 as amended by SEQUENCER_UX 9.1 and HARMONY 2:
`ChanSave` 344 to **384 B** (`st[SQ_NALGO]`), `SeqPreset` 1396 to **1556 B**, `UndoRec` 92 to **94 B**, `PO_TIE` 0x80 in `PulseOut.flags`, the new `ModIdx` (8 B) and `AlgoDesc.mod`, `SQ_SEED_MOD` and `SQ_SMOD_DEFAULT`;
AM-1 to AM-4 of arp/SPEC.md 6.10 (`hc` argument, `sub_note`, `AF_CHORD`, six algorithms); `hi(m)` of section 20: EUCLID `SPAN+3`, SNAKE and STAGE `RANGE+1` (P2-016), ARP and PATT **60**; the preset version becomes 4.

## 6. Owner decisions (2026-11-12) -- decided

Answers received from the owner through the coordinator; the lock line `Approved: owner <date>` is written by the orchestrator after verifying.

1. **The six, and the held ROW 2 form (score 10):** accepted as written.
2. **ARP:** DIR names `ASPL` / `CHORD`, default DENS 12, figure restarts with the loop, chord chip always on: accepted. **O-5 decided YES**: THUMB's walking tones skip the home note's pool index (a drone pedal and a walking root work together). Done in `arp/proto.c`, `arp/SPEC.md`, `arp_ref.py`; only the ARP THUMB lines (non-UP directions and the drone case) of the ARP golden changed, every other golden is byte-identical.
3. **PATTERNS:** categories and 20 each (O2), idiom-faithful not note-exact (O1), no bake into steps (O3): settled.
4. **SHAPE sub-labels: option A decided.** Keep the primary names (`OCTAVE DRIFT LONG PHASE UP/DN`, `MOD` for EUCLID); the toast adds `+ MOD` when the mod seed moves too. Written into every SPEC and MODULATION.md 3.4.
5. **Contract changes of section 5** (preset 1556 B, `PO_TIE`, the `AlgoDesc.mod` hook instead of `PulseOut.mod_a/b`): confirmed.
6. **Cost table:** confirmed; the PATTERNS CPU is measured on the device later, not optimised now.
7. **Gate A of the use case and its stale-row list:** stale rows stay for P2-029 / P4-031.

## 7. Evidence

`sh tests/design/run_protos.sh` -> `PROTOS PASS`: header hygiene; primitives; HARMONY C == Python (3566 table lines); per algorithm: table equals SPEC, matrix, 40 random purity cases (incl. the mod lanes), table rules, two byte-identical runs, golden equal;
ACID `acid_ref.py` (53 cases), EUCLID `euclid_ref.py` (53), **ARP `arp_ref.py` (1728 grid cases, independent Python, includes the O-5 THUMB rule)**, **PATTERNS `patterns_ref.py` (126 cases against `patterns_proto.py`)**; the PATTERNS library sweep (287424 step checks) and `gen_patterns.py --check`.
