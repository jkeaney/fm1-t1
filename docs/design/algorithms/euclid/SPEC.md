# EUCLID: algorithm spec for the prototype and the firmware (P2-016)

Status: reference prototype, design only. Source of the behaviour: [../t1.md](../t1.md) 6 (the reasoning, the worked example and the
flex points); this file is the **implementation contract** that `proto.c` follows and `golden.txt` records. Where the two differ,
section 7 says so. The shared types and primitives are in [../common/](../common/); sections cite ADR-0003 as `A3`, ALGORITHM_SET
as `AS` and the sibling ACID spec as [../acid/SPEC.md](../acid/SPEC.md).

EUCLID spreads `HITS` pulses as evenly as possible over a cycle of `LEN` steps (Bjorklund), lays a contour melody over the hits in hit
order, and adds a seeded accent count and seeded ratchets. It is the rhythm-first alternative to ACID's random-order hits. `proto.c` is
the firmware file: P4 copies it, adds `static` to its three functions and the cache type, and wires the `ALGOS[1]` row. Nothing else
changes (A3 14). Its only borrowed code is `eu_bjorklund` and `eu_rotate` (the T-1's `t1_euclidean_bjorklund` and `_rotate` with the
same output for `LEN <= 16`, SP11) and `eu_phrase` (`t1_harmony_eval_phrase`, shapes 1..6, **with the BUG-001 fix**: ARCH and INV now
have a shape, T1_ISSUES F3).

## 1. Parameters (morph id order; `build()`'s `m[]`)

| Id | Label | Range | Default | Step / fast | Knob slot (`knob_morph`) | Meaning |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | `HITS` | 0..16 | 5 | 1 / 1 | K1 | pulses spread over the cycle; `min(HITS, LEN)`; 0 = KEYS ONLY (no generated hit) |
| 1 | `LEN` | 2..16 | 16 | 1 / 1 | S1 | the generator's own cycle in steps (polymeter against the channel length) |
| 2 | `SPAN` | 0..24 | 12 | 1 / 1 | S2 | contour excursion in semitones above the root (0 = every note is the root) |
| 3 | `ACCNT` | 0..8 | 2 | 1 / 1 | K4 | how many hit ordinals are accented |
| 4 | `OFFSET` | 0..15 | 0 | 1 / 1 | K2 | rotation of the whole figure, added to the dice rotation |
| 5 | `RATCH` | 0..8 | 4 | 1 / 1 | K3 | ratchet odds (4 = 15 % double and 5 % triple, 0 = none, 8 = double) |
| 6 | `PUNCH` | 0..47 | 40 | 1 / 4 | S3 | accent velocity = 80 + PUNCH (default 120) |
| 7 | `CURVE` | 0..6 | 0 | 1 / 1 | S4 | contour: 0 AUTO (seeded) 1 SINE 2 TRI 3 SAW+ 4 SAW- 5 ARCH 6 INV |

`knob_morph = {0,4,5,3,1,2,6,7}` (the 3B re-lock of `knob_map.tsv`): ROW 1 = K1 HITS, K2 OFFSET, K3 RATCH, K4 ACCNT; ROW 2 = S1 LEN,
S2 SPAN, S3 PUNCH, S4 CURVE. **Morph id 2 is the pitch width (SPAN)**, because `harm_pull` reads `m[2]` for every algorithm. Every ROW 2
default is neutral. Every morph is a threshold or an index on stored draws, so it is reversible: turning back restores the exact
pattern. One exception worth knowing (FULL_CONTROL 3.5): a hit's ratchet count can fall from 3 to 2 as RATCH rises past a threshold,
still a pure function of RATCH.

## 2. Layers, seeds and dice

Four seeds (`n_seed` 4), one stream each (`sq_rng_init(seed, salt)`; the salts are constants of `proto.c` and define the golden file):

| Seed id | Name | `seed_k` | Salt | Draws (always the same count) |
| --- | --- | --- | --- | --- |
| 0 | ROT | `0xFF` (literal 0) | `0x1E3779B9` | `r16 = rnd(65536)` only when the seed is not 0 |
| 1 | NOTES | 1 | `0x2545F491` | `d = 1 + rnd(6)`, then 16 jitter draws `rnd(5)` |
| 2 | ACCNT | 2 | `0x1B873593` | shuffle of the 16 ordinals |
| 3 | RATCH | 3 | `0x3C6EF372` | per ordinal 0..15: `u = rnd(100)`, `d = rnd(3)` |

The default ROT seed is 0 (no rotation); a dice never lands on 0 (A3 3.1). The other defaults are `(0x3A5D * (ch * 4 + seed_k)) & 0xFFFF`.

| Layer | Button sub-label | Seeds re-rolled |
| --- | --- | --- |
| NOTES | `NOTES` | 1 |
| HITS | `ROTATE` | 0 (which step the figure starts on; the rhythm itself is fixed by HITS and LEN) |
| ACCNT | `ACCNT` | 2 |
| ARTIC | `RATCH` | 3 |
| SHAPE | `MOD` | `s_mod` only (P2-022 / P2-017: EUCLID's SHAPE layer is no longer absent; the button re-rolls the mod lanes and nothing else) |

### 2.1 The mod lanes and `s_mod` (P2-017, MODULATION 3 and 4)

The mod gen adds **no morph and no button**. Its one input is `s_mod` (`SQ_SEED_MOD`, seed id 6): it is stored in `ModSave` (not in `AlgoState`), handed to the harness as a seventh seed, and **re-rolled by the SHAPE layer together with the seed the layer already had** (`s_mod` alone). `build()` and `step()` never read it, so dicing it cannot move a note (checked by the purity test (g) and visible in the golden: the `dice SHAPE` cases keep the notes of the base case).
`AlgoDesc.mod(cache, g, hc, out_pulse, ModIdx*)` reports **which index each lane is read at** (a pure function like `step()`); the shared SP18..SP20 turn it into the value of each of the 4 slots (`sq_mod_lane`). Lane A (slots 1, 3) is the cell of the **hit ordinal** (the k-th hit gets the k-th cell, so the value stays with the note when OFFSET rotates; a rest reads the ordinal of the hit before it, like its contour pitch; 4 bits per step in the cache, `ho[8]`). Lane B (slots 2, 4) is an LFO of period LEN read at the step position. The golden shows the four slot values after `hold` on every row, for `s_mod` = the channel's default (`SQ_SMOD_DEFAULT`) and a channel LENGTH of 16.

## 3. Build (A3 4; pure; reads only `m`, `seed`, `g->s.key`, `g->s.octave`, `g->snap[]`)

1. **Hits.** `n = LEN`, `p = min(HITS, n)`, `m0 = eu_bjorklund(n, p)`, `H = popcount(m0)`.
2. **ROT.** `r16 = rnd(65536)` if `seed[0] != 0`, else 0. `R = ((r16 mod n) + OFFSET) mod n`. `m1 = eu_rotate(m0, n, R)`: bit `k` moves to `k - R`, so rotated step `i`
   comes from unrotated step `j = (i + R) mod n`. `hit = m1`.
3. **Ordinal.** For step `i` (hit or not), `cnt = popcount(m0 & ((2 << j) - 1))` and `h = max(0, cnt - 1)`: a hit's own rank among the hits, and for a rest the rank of the nearest hit at or before it.
   The melody follows the ordinal of the **unrotated** figure, so ROT and OFFSET move the figure with its notes, and LEN never changes which note a hit plays.
4. **NOTES.** `shape = CURVE` if CURVE > 0, else `d`; `jit[h] = rnd(5) - 2` for `h = 0..15`. For each step: `c = eu_phrase(shape, SPAN, h, H)` (0 when SPAN is 0 or `H <= 1`),
   `s = max(0, c + (SPAN > 0 ? jit[h] : 0))`, `note = sq_note(g, s)`: the line never goes below the root, and scale, key, octave and the nearest-tone rule (SP6) happen in that one call (late quantise).
5. **ACCNT.** `ord` = a seeded order of 0..15; `sq_accent_pick(ord, 16, (1 << H) - 1, ACCNT)` selects the first ACCNT ordinals that exist (< H). A hit is accented if its ordinal is selected.
   `vel = min(127, 80 + PUNCH)` accented, else 80 (also for every rest).
6. **RATCH.** `P2 = 15 * RATCH / 4`, `P3 = 20 * RATCH / 4`; per ordinal `reps = 2 if u < P2, 3 if u < P3, else 1`; `decelerating = (d == 2)`. A hit stores `(reps - 1) | decel << 2`; a rest stores 0.
7. `len = n`, `hv = min(127, 80 + PUNCH)`.

Cache (38 B, `EuclidCache`): `note[16] hit(u16) acc(u16) rp[8] ho[8] len hv`. `cache_bytes = 38` (was 30 before the mod lane A's ordinals).

## 4. Step (ISR; pure function of `(cache, g)`; `hc` is ignored and `AF_CHORD` is clear)

`i = g mod len`. `note, vel` are read from lane `i` and are valid for every `g` (a rest carries the contour value of the hit before it, so a step the producer turns ON by hand plays the line, SEQUENCER_UX 9.2).
`gate = 50` for every step (the core multiplies by the global GATE). `flags = PO_HIT | (reps - 1) << PO_REP_SHIFT | PO_PACE_DEC if decelerating` on a hit, 0 on a rest. `hold = 0`.
The core schedules the extra triggers of a ratchet with `sq_ratchet` (SP5): the pace direction is `-1` (accelerating) or `+1` (`PO_PACE_DEC`). `loop_len = len`. `g` is the generator position of SEQUENCER_UX 1.4:
a page reading the same block repeats the cycle, an OWN page continues it, so `LEN 12` drifts across a 32-step pattern.

## 5. The AlgoDesc table (the one `proto.c` reads)

`run_protos.sh` extracts the block between the two marker comments from this file and from `proto.c` and requires them to be identical:
a remap (a knob, a dice button, a range, a default, a label) is an edit to this block and the same edit in the firmware copy, nothing else.

```c
/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_EUCLID = {
    .name = "EUCLID", .id = SQ_ALGO_EUCLID, .n_seed = 4, .flags = 0, .cache_bytes = 38,
    .knob_morph = { 0, 4, 5, 3, 1, 2, 6, 7 },
    .seed_k = { 0xFF, 1, 2, 3 },
    .morph = {
        { "HITS",   0, 16,  5, 1, 1 },
        { "LEN",    2, 16, 16, 1, 1 },
        { "SPAN",   0, 24, 12, 1, 1 },
        { "ACCNT",  0,  8,  2, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "RATCH",  0,  8,  4, 1, 1 },
        { "PUNCH",  0, 47, 40, 1, 4 },
        { "CURVE",  0,  6,  0, 1, 1 },
    },
    .layer = { {1, {1}}, {1, {0}}, {1, {2}}, {1, {3}}, {1, {SQ_SEED_MOD}} },
    .sub = { "NOTES", "ROTATE", "ACCNT", "RATCH", "MOD" },
    .build = euclid_build, .loop_len = euclid_loop_len, .step = euclid_step, .hi = euclid_hi, .sub_note = 0, .mod = euclid_mod,
};
/* ALGODESC-END */
```

## 6. How EUCLID meets the rest of the design

- **Global settings** (A3 5): KEY, OCTAVE and SCALE enter only through `g` (`sq_note`). GATE, DIV and SWING are the core's; `build()` never reads them (the purity test varies them and requires an identical cache).
- **Steps entered on the keys** (SEQUENCER_UX 9.2): the core overlays `StepEdit` on `step()`'s output; an edited RATCH replaces `reps`, an edited VELO replaces the 80.
- **HARMONY** (HARMONY.md 4.3, A3 20): `pull = 2 + SPAN * 6 / 24` (`harm_pull`, default 5); strong steps are the accented hits and hit ordinal 0 of each cycle; the first hit of each chord is the anchor;
  the fold range is `lo = 0 .. hi(m)` (`harm_lo`, `euclid_hi`). The contour is built first and then pulled; a ratchet shares the pulled pitch. The default HARMONY state gives the plain output (H-INERT, checked in `golden.txt`).
  The prototype's chord run marks strong steps from `vel >= 100` only; ordinal 0 is the core's to add (it knows the cycle start), and it is not cached.
- **`hi(m) = max(SPAN + 3, 12)`**: the contour peaks at SPAN, the jitter adds 2, and a snap up to the next scale tone adds at most 1 (no scale of the 16 has a gap over 3 semitones).

**Test approximation (P2-016 review, P2-017):** the chord run of `tests/design/proto_main.c` (`harm_run`) is not the core. It takes a step as *strong* when its velocity is >= 100 or it is a hold; ordinal 0 of each cycle; the other strong steps of HARMONY.md 4.3 are the core's to add (it knows the cycle and the stage starts) and are not cached. The golden `HARM` sections therefore show the pull with that approximation.

## 7. Deviations and open points found while writing the prototype (for the reviewer)

1. **`hi` is `max(SPAN + 3, 12)`, not `max(SPAN, 12)`** as ADR-0003 section 20 says: with jitter and the snap up, a note can sit up to 3 semitones above SPAN, and the host test checks every note against `hi`. Bound too low would let the chord stage
   treat a legal note as out of range. ADR-0003 is not edited here (P2-029 owns the amendment); this is the number the firmware needs.
2. **Late quantise replaces `t1_harmony_quantize`** ("snap at or below"): the note is the nearest scale tone, ties to the lower pitch (SP6). The worked example of t1.md 6.3 (`36 46 43 36 36`) therefore no longer applies; the golden file supersedes it.
3. **The "even" pace is not stored.** PulseOut carries one bit (`PO_PACE_DEC`), so of t1.md's three pace outcomes (`d = rnd(3)`: accelerating, even, decelerating) the draw count is kept, `d == 2` decelerates and the other two accelerate.
   The ratchet remains audibly two-way; a three-way pace would need a second flag bit (`PO_PIN` is reserved).
4. **Non-hit steps carry a note** (the nearest hit at or before them), as SEQUENCER_UX 9.2 (P1-017) needs; t1.md 6.3 only defined notes for hits.
5. **Gate is a constant 50 %** (t1.md used the T-1's `time` field mapped from GATE). The core's global GATE scales it.
6. **`LEN` is the ROW 2 slot S1 and `OFFSET` the ROW 1 slot K2** (the 3B re-lock of `knob_map.tsv`); ALGORITHM_SET 5.2 and FULL_CONTROL 3.2 still show the earlier layout. Only `knob_morph` changes: the morph ids are the same.
7. **`rnd(65536)` for ROT** follows t1.md 6.3 literally: `r16` is the first draw after the SP1 warm-up.
8. **Bjorklund over 16 steps only**: the groups array is capped at 16 and the patterns are 32-bit, as t1.md 6.8 allows; the output for `LEN <= 16` equals the original (checked independently, section 8).

## 8. Golden file and tests

`golden.txt` (regenerate with `GOLDEN_UPDATE=1 sh tests/design/run_protos.sh`) holds, each as `t note vel gate flags hold lane1 lane2 lane3 lane4` for t = 0..127: 16 base cases (channel 0, 1 x MINPEN, CHROM x key 0, 5 x octave 0, -1 at the default morphs and seeds);
every morph at min, mid and max (24 cases: all four ROW 1 and all four ROW 2 knobs); each present dice layer with two diced seeds (8 cases), and the SHAPE layer (`MOD`, `s_mod` only) with two diced seeds (EUCLID has no absent layer now); three combinations (all minimum, all maximum, all layers diced); then the HARMONY runs: the default state and ROLE OFF with 1-4-5-1 (both equal to the plain output, H-INERT) and 1-4-5-1 in C major for ROLE AUTO, ROOT and FULL.
`tests/design/euclid_ref.py` is an independent check of the rhythm written from section 3 (not from `proto.c`): every case must be periodic in LEN, have `min(HITS, LEN)` hits, form a maximally even set (a rotation of `floor(i * n / k)`),
and E(5,16) must be steps 0 3 6 9 12. `tests/design/proto_main.c` also checks, for 40 random settings, the purity contract (A3 14), the table rules and the music theory: every note is in the scale and in `-12 .. hi(m)`, `vel` 1..127, `gate` 1..100.
