# ACID: algorithm spec for the prototype and the firmware (P2-008)

Status: reference prototype, design only. Source of the behaviour: [../sting.md](../sting.md) 6 (read it for the reasoning and
the worked slide example); this file is the **implementation contract** that `proto.c` follows and `golden.txt` records. Where the
two differ, this file says so in section 7. The shared types and primitives are in [../common/](../common/) (`sq_types.h`,
`sq_prims.h`); sections below cite ADR-0003 as `A3` and ALGORITHM_SET as `AS`.

ACID builds a 16-step monophonic acid line from six seeded layers. `proto.c` is the firmware file: P4-001 copies it, adds
`static` to its three functions and the cache type, and wires the `ALGOS[0]` row. Nothing else changes (A3 14).

## 1. Parameters (morph id order; `build()`'s `m[]`)

| Id | Label | Range | Default | Step / fast | Knob slot (`knob_morph`) | Meaning |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | `DENS` | 0..16 | 12 | 1 / 1 | K1 | steps that play: those with rank < DENS (0 = KEYS ONLY: no generated hit) |
| 1 | `PEDAL` | 0..12 | 6 | 1 / 1 | S2 | steps overwritten with the root (sting's TYPE) |
| 2 | `JUMP` | 0..20 | 5 | 1 / 1 | K3 | octave-jump odds |
| 3 | `ACCNT` | 0..8 | 3 | 1 / 1 | K4 | how many hit steps are accented |
| 4 | `OFFSET` | 0..15 | 0 | 1 / 1 | S1 | where the loop starts (rotation) |
| 5 | `SLIDE` | 0..8 | 4 | 1 / 1 | K2 | glide odds (4 = the table, 0 = none, 8 = double) |
| 6 | `PUNCH` | 0..47 | 40 | 1 / 4 | S3 | accent velocity = 80 + PUNCH (default 120) |
| 7 | `DIR` | 0..2 | 0 | 1 / 1 | S4 | read direction: FWD, BWD, PEND |

`knob_morph = {0,5,2,3,4,1,6,7}` (ADR-0003 4.1, 3B re-lock): ROW 1 = K1 DENS, K2 SLIDE, K3 JUMP, K4 ACCNT; ROW 2 = S1 OFFSET, S2 PEDAL,
S3 PUNCH, S4 DIR. Every ROW 2 default is neutral. Every morph is a threshold or an index on stored draws, so it is monotonic and
reversible: turning back restores the exact pattern.

## 2. Layers, seeds and dice

Six seeds (`n_seed` 6), one stream each (`sq_rng_init(seed, salt)`; salts are constants of `proto.c`, they define the golden file):

| Seed id | Name | `seed_k` | Salt | Draws (always the same count, so no layer's stream depends on another's data) |
| --- | --- | --- | --- | --- |
| 0 | NOTES | 1 | `0x5A17C3E1` | `n = 3 + rnd(3)`; shuffle 12 pitch classes, motif = first n; tile over 16, shuffle 16; shuffle the 16-step replacement order |
| 1 | HITS | 2 | `0x2C9E6B07` | shuffle of steps 1..15 (step 0 ranks first) |
| 2 | OCT | 3 | `0x71D4A5F3` | per step i = 0..15: `u1 = rnd(100)`, `u2 = rnd(100)`, `dir = rnd(3) - 1` |
| 3 | GATE | 4 | `0x3B8F1D29` | `rnd(2)` for step 0, then per step 1..15 two Bernoulli draws |
| 4 | ACCNT | 5 | `0x68E2B94D` | shuffle of 16 steps |
| 5 | SLIDE | 6 | `0x4F6A9C15` | `rnd(100)` for steps 0..15 |

Default seed k of channel ch: `(0x3A5D * (ch * 6 + seed_k)) & 0xFFFF` (A3 3.1). Layers (the five dice buttons) and their sub-labels:

| Layer | Button sub-label | Seeds re-rolled |
| --- | --- | --- |
| NOTES | `NOTES` | 0 |
| HITS | `HITS` | 1 |
| ACCNT | `ACCNT` | 4 |
| ARTIC | `GT+SLD` | 3 and 5 (two reads of the entropy, the second `^ 0x5A5A`) |
| SHAPE | `OCTAVE` | 2 and `s_mod` (the second read of the entropy, `^ 0x5A5A`) |

### 2.1 The mod lanes and `s_mod` (P2-017, MODULATION 3 and 4)

The mod gen adds **no morph and no button**. Its one input is `s_mod` (`SQ_SEED_MOD`, seed id 6): it is stored in `ModSave` (not in `AlgoState`), handed to the harness as a seventh seed, and **re-rolled by the SHAPE layer together with the seed the layer already had** (OCTAVE + `s_mod`). `build()` and `step()` never read it, so dicing it cannot move a note (checked by the purity test (g) and visible in the golden: the `dice SHAPE` cases keep the notes of the base case).
`AlgoDesc.mod(cache, g, hc, out_pulse, ModIdx*)` reports **which index each lane is read at** (a pure function like `step()`); the shared SP18..SP20 turn it into the value of each of the 4 slots (`sq_mod_lane`). Lane A (slots 1, 3) is the cell of the gate position `i` (the index that the accent and slide flags use), leaning on the step's velocity so that accents open the filter. Lane B (slots 2, 4) is an LFO of period 16 read at the same `i`, so OFFSET and DIR move the filter with the notes. The golden shows the four slot values after `hold` on every row, for `s_mod` = the channel's default (`SQ_SMOD_DEFAULT`) and a channel LENGTH of 16.

**Decided (owner 2026-11-12, option A):** the SHAPE sub-label stays the primary name (`OCTAVE`); the dice toast adds `+ MOD` when the mod seed moves too (an active device slot). MODULATION.md 3.4's `OCT+MOD` style labels are not used.

## 3. Build (A3 4; pure; reads only `m`, `seed`, `g->s.key`, `g->s.octave`, `g->snap[]`)

1. **NOTES**: as the table above, then `pitch[i] = tile[i]`, and `pitch[ord_type[r]] = 0` for `r < PEDAL`.
2. **HITS**: `rank` from `sq_rank_build(first_fixed = 1)`; `hit[i] = rank[i] < DENS`.
3. **OCT**: `p_eff = min(100, BASE_P[i] * JUMP / 5)` with `BASE_P = {10,11,12,13,15,16,17,18,20,21,22,23,25,26,27,28}`;
   `n = (u1 < p_eff) + (u2 < p_eff)`; `octave = max(-1, dir * n)`, so **-1..+2** (sting 6.3 clamps the low side at -1). The note is
   `sq_note(g, pitch[i] + 12 * octave)`: scale, key, octave and the nearest-tone rule (SP6, late quantise) all happen in that one call.
4. **GATE**: step 0 is 100 (shown as 79 % of a step) or 70 (55 %) by one coin; steps 1..15 add two Bernoulli draws with
   `PA = {-,8,11,14,17,20,23,26,29,32,35,38,41,44,47,50}` and `PB = {-,11,13,14,16,18,20,22,24,26,28,31,33,35,38,40}`;
   the sum 0, 1, 2 maps to a gate of 24, 55, 100 % of a step.
5. **ACCNT**: a seeded order of 16 steps; the first ACCNT **hit** steps in that order are accented (`sq_accent_pick`). `vel = min(127, 80 + PUNCH)` accented, else 80.
6. **SLIDE**: `slide[i] = rnd(100) < min(100, PS[i] * SLIDE / 4)` with `PS = {0,20,10,25,8,20,10,25,8,20,10,25,8,20,10,30}`. `PS[0] = 0`: a glide never enters index 0.
7. `off = OFFSET`, `dir = DIR`.

Cache (54 B, `AcidCache`): `note[16] vel[16] gate[16] hit(u16) slide(u16) off dir`. `cache_bytes = 54`.

## 4. Step (ISR; pure function of `(cache, g)`; `hc` is ignored and `AF_CHORD` is clear)

`i = sq_dir_map(g + off, 16, dir)` (FWD `u mod 16`, BWD `15 - u mod 16`, PEND ping-pong without repeating the ends over a period of 30).
`note, vel, gate` are read from lane `i` and are valid for every `g`, hit or not. `flags = PO_HIT if hit[i]`, plus `PO_SLIDE_IN if slide[i]`.
`PO_SLIDE_IN` is the **stored** flag: the core makes it effective only when the step and the step played before it both sound
(A3 7.3 step 5, SEQUENCER_UX 9.2), so raising DENS can bring a stored glide back. `loop_len = 16`. `g` is the generator position of
SEQUENCER_UX 1.4 (0..63): ACID's lanes repeat every 16, so pages that read the same block play the same line, while PEND (period 30) differs by page.

## 5. The AlgoDesc table (the one `proto.c` reads)

`run_protos.sh` extracts the block between the two marker comments from this file and from `proto.c` and requires them to be identical:
a remap (a knob, a dice button, a range, a default, a label) is an edit to this block and the same edit in the firmware copy, nothing else.

```c
/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_ACID = {
    .name = "ACID", .id = SQ_ALGO_ACID, .n_seed = 6, .flags = AF_PAIRING, .cache_bytes = 54,
    .knob_morph = { 0, 5, 2, 3, 4, 1, 6, 7 },
    .seed_k = { 1, 2, 3, 4, 5, 6 },
    .morph = {
        { "DENS",   0, 16, 12, 1, 1 },
        { "PEDAL",  0, 12,  6, 1, 1 },
        { "JUMP",   0, 20,  5, 1, 1 },
        { "ACCNT",  0,  8,  3, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "SLIDE",  0,  8,  4, 1, 1 },
        { "PUNCH",  0, 47, 40, 1, 4 },
        { "DIR",    0,  2,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {4}}, {2, {3, 5}}, {2, {2, SQ_SEED_MOD}} },
    .sub = { "NOTES", "HITS", "ACCNT", "GT+SLD", "OCTAVE" },
    .build = acid_build, .loop_len = acid_loop_len, .step = acid_step, .hi = acid_hi, .sub_note = 0, .mod = acid_mod,
};
/* ALGODESC-END */
```

## 6. How ACID meets the rest of the design

- **Global settings** (A3 5): KEY, OCTAVE and SCALE enter only through `g` (`sq_note`). GATE, DIV and SWING are the core's; `build()` never reads them (the purity test varies them, and the scale index, and requires an identical cache).
- **Steps entered on the keys** (SEQUENCER_UX 9.2): the core overlays `StepEdit` on `step()`'s output; ACID supplies `vel` and `gate` for rests too.
- **HARMONY** (HARMONY.md 4, A3 20): `step()` returns the scale-quantised note; the chord stage runs after it in the core. For ACID
  `pull = 3 + JUMP * 3 / 20` (`harm_pull`), strong steps are accented steps (velocity >= 100) and octave-jump notes (semitone >= 12), the first hit of each chord
  is the anchor, and the fold range is `lo = -12 .. hi(m)` (`harm_lo`, `acid_hi`). The default HARMONY state gives the plain output (H-INERT, checked in `golden.txt`).
- **Hi:** `hi(m) = 36` when JUMP > 0 (pitch class 11, or 12 when the scale snaps up, plus octave +2), else 12. The lowest note is 12 semitones under the base (octave -1).

**Test approximation (P2-016 review, P2-017):** the chord run of `tests/design/proto_main.c` (`harm_run`) is not the core. It takes a step as *strong* when its velocity is >= 100 or it is a hold or an octave-jump note (semitone >= 12); the other strong steps of HARMONY.md 4.3 are the core's to add (it knows the cycle and the stage starts) and are not cached. The golden `HARM` sections therefore show the pull with that approximation.

## 7. Deviations and open points found while writing the prototype (for the reviewer)

1. **ACID's range is -12..+36 semitones, not 0..24.** ADR-0003 section 20 gives `hi` as "12 + 12 if JUMP > 0" and HARMONY 4.2 folds into 0..hi. sting.md 6.3 lets the octave
   lane reach +2 (`dir * n` with `n = 2`) and -1. The prototype follows sting.md (it is the assessed behaviour, and -1 is the bass register of an acid line), so `acid_hi` returns 36
   and the chord stage needs a floor: `harm_apply` takes `lo` (ACID -12, others 0) and floors with floor division. The HARMONY.md golden tables use `lo = 0, hi = 24`
   and are unchanged (compared bit for bit). If the owner prefers octave +1 as the ceiling, it is the `oct` clamp in `acid_build` plus `acid_hi` = 24, and the golden is regenerated.
2. **HarmChord is 14 B**, not 16 B as HARMONY.md says (the fields it lists add up to 14); `sq_types.h` asserts 14.
3. **`drone_pitch` in harmony_proto.py folded on 7 degrees** (fixed in P2-029: it now uses `n' = min(7, scale size)` and clamps at `2n'` as HARMONY.md 3.3 and `harm.c` do; the compared dump covers p 1..14 on 7-note scales and 1..11 on the pentatonic).
4. **The progression dice** (HARMONY 6.1) seed their generator **without** SP1's four-draw warm-up, because the golden table of 6.1 comes from `harmony_proto.py`, which does not discard.
5. **Snap tie rule at the octave wrap:** nearest scale tone by the distance between the pitches, ties to the lower pitch, so `snap[r]` can be 12 (MAJPEN: r = 11 snaps up to the next key note).
   sting.md's "numerically lower pitch class" reading would send a tie across the wrap to pitch class 0; the two readings differ only for a tie that straddles the wrap.
6. **Pull and strong are in `harm.c`** (`harm_pull`, table from HARMONY 4.3), not in `AlgoDesc`: ADR-0003 20 added only `hi`. The "octave-jump" test uses the semitone (>= 12).
7. **SP5 `sq_ratchet`** is implemented generically (even / accelerating / decelerating, spacing >= 3 ticks, `len < spacing`); EUCLID's own pace curve is P2-016's. ACID does not ratchet.

## 8. Golden file and tests

`golden.txt` (regenerate with `GOLDEN_UPDATE=1 sh tests/design/run_protos.sh`) holds, each as `t note vel gate flags hold lane1 lane2 lane3 lane4` for t = 0..127:
16 base cases (channel 0, 1 x MINPEN, CHROM x key 0, 5 x octave 0, -1 at the default morphs and seeds); every morph at min, mid and max (24 cases: all four ROW 1 and all four
ROW 2 knobs); each of the five dice layers with two diced seeds (10 cases); three combinations (all minimum, all maximum, all layers diced); then the HARMONY runs: the default state
and ROLE OFF with 1-4-5-1 (both equal to the plain output, H-INERT) and 1-4-5-1 in C major for ROLE AUTO, ROOT and FULL.
`tests/design/acid_ref.py` is an independent Python implementation written from sections 1 to 4; it must reproduce every plain case. The driver `tests/design/proto_main.c`
also checks, for 40 random settings, the purity contract (A3 14: builds are equal whatever the poison; gate / div / swing / scale do not reach the cache; `step` ignores the unused cache tail, the order
of calls and `hc`), the table rules, and the music theory: every note is in the scale and in `-12 .. hi(m)`, `vel` 1..127, `gate` 1..100.
