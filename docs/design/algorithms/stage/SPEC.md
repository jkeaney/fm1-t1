# STAGE: algorithm spec for the prototype and the firmware (P2-016)

Status: reference prototype, design only. Source of the behaviour: [../ml185.md](../ml185.md) 6 (the reasoning, the fire rule and the informative check); this file is the **implementation contract** that `proto.c` follows and
`golden.txt` records. Where the two differ, section 7 says so. The shared types and primitives are in [../common/](../common/); sections cite ADR-0003 as `A3`, ALGORITHM_SET as `AS` and the sibling ACID spec as
[../acid/SPEC.md](../acid/SPEC.md).

STAGE is the stage sequencer: eight stages, each 1..8 pulses long, in a loop of `TOTAL` pulses (8..64). Each stage rests, fires once, holds one note for the rest of the stage, or re-fires every `div` pulses. It is the most complex of the four:
a variable loop, a cumulative-sum lookup, long holds and the only algorithm whose loop is not a divisor of the bar. `proto.c` is the firmware file: P4 copies it, adds `static` to its three functions and the cache type, and wires the
`ALGOS[3]` row. Nothing else changes (A3 14).

## 1. Parameters (morph id order; `build()`'s `m[]`)

| Id | Label | Range | Default | Step / fast | Knob slot (`knob_morph`) | Meaning |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | `DENS` | 0..8 | 6 | 1 / 1 | K1 | stages that play: those with rank < DENS (stage 0 ranks first; 0 = KEYS ONLY) |
| 1 | `TOTAL` | 8..64 | 16 | 1 / 4 | K2 | pulses in the loop; one pulse per detent gives exactly one stage one more pulse |
| 2 | `RANGE` | 0..36 | 12 | 1 / 4 | S4 | pitch span in semitones above the root |
| 3 | `RATCH` | 0..8 | 2 | 1 / 1 | K4 | how many stages that can really repeat are made to re-fire |
| 4 | `OFFSET` | 0..63 | 0 | 1 / 4 | S1 | where the loop starts: the read position is `t + OFFSET` |
| 5 | `RDIV` | 1..4 | 4 | 1 / 1 | S2 | cap on a stage's re-fire divider (4 = as drawn, 1 = every pulse) |
| 6 | `DYN` | 0..10 | 10 | 1 / 1 | S3 | velocity depth: 10 = as drawn, 0 = flat 90 |
| 7 | `DIR` | 0..2 | 0 | 1 / 1 | K3 | loop direction: FWD, BWD, PEND |

`knob_morph = {0,1,7,3,4,5,6,2}` (the 3B re-lock of `knob_map.tsv`): ROW 1 = K1 DENS, K2 TOTAL, K3 DIR, K4 RATCH; ROW 2 = S1 OFFSET, S2 RDIV, S3 DYN, S4 RANGE. **Morph id 2 is the pitch width (RANGE)** that `harm_pull` reads.
Every ROW 2 default is neutral. Every morph is a threshold or an index on stored draws, so it is reversible: turning back restores the exact pattern. A rest is `PO_HIT` clear; a pulse in the middle of a stage is also a rest.

## 2. Layers, seeds and dice

Five seeds (`n_seed` 5), one stream each (`sq_rng_init(seed, salt)`; the salts are constants of `proto.c` and define the golden file). Default seed k of channel ch: `(0x3A5D * (ch * 5 + seed_k)) & 0xFFFF`.

| Seed id | Name | `seed_k` | Salt | Draws |
| --- | --- | --- | --- | --- |
| 0 | NOTES | 1 | `0x6C1E94B3` | `rnd(256)` for each of the 8 stages |
| 1 | LENS | 2 | `0x13A7D5F9` | shuffle of 56 slots (each stage seven times) |
| 2 | RESTS | 3 | `0x5E82C0A7` | shuffle of stages 1..7 (stage 0 ranks first) |
| 3 | MODE | 4 | `0x34F9B16D` | shuffle of 8 stages, then 8 dividers `1 + rnd(4)`, then 8 long flags `rnd(100) < 25` |
| 4 | VEL | 5 | `0x7A05E3C1` | `50 + rnd(78)` for each of the 8 stages |

| Layer | Button sub-label | Seeds re-rolled |
| --- | --- | --- |
| NOTES | `NOTES` | 0 |
| HITS | `RESTS` | 2 |
| ACCNT | `VELO` | 4 |
| ARTIC | `RT+HLD` | 3 (which stages re-fire and which hold) |
| SHAPE | `LONG` | 1 (which stages are long) and `s_mod` |

All five layers are present.

### 2.1 The mod lanes and `s_mod` (P2-017, MODULATION 3 and 4)

The mod gen adds **no morph and no button**. Its one input is `s_mod` (`SQ_SEED_MOD`, seed id 6): it is stored in `ModSave` (not in `AlgoState`), handed to the harness as a seventh seed, and **re-rolled by the SHAPE layer together with the seed the layer already had** (LONG + `s_mod`). `build()` and `step()` never read it, so dicing it cannot move a note (checked by the purity test (g) and visible in the golden: the `dice SHAPE` cases keep the notes of the base case).
`AlgoDesc.mod(cache, g, hc, out_pulse, ModIdx*)` reports **which index each lane is read at** (a pure function like `step()`); the shared SP18..SP20 turn it into the value of each of the 4 slots (`sq_mod_lane`). Lane A (slots 1, 3) is the cell of the **stage number** (one value per stage, held for the stage's whole duration: a long stage is one CC, not one per pulse). Lane B (slots 2, 4) is an LFO of period TOTAL pulses read at the loop pulse `p` (one sweep per loop). The golden shows the four slot values after `hold` on every row, for `s_mod` = the channel's default (`SQ_SMOD_DEFAULT`) and a channel LENGTH of 16.

## 3. Build (A3 4; pure; reads only `m`, `seed`, `g->s.key`, `g->s.octave`, `g->snap[]`)

1. **NOTES.** `note[k] = sq_note(g, sq_span(raw[k], RANGE))` per stage: SP7 span, then the late quantise (SP6).
2. **LENS and TOTAL.** `slots` = each stage seven times, shuffled. `L[k] = 1 + (count of k in slots[0 .. TOTAL - 9])`, so `sum(L) = TOTAL` exactly and every `L[k]` is 1..8; raising TOTAL by one lengthens exactly one stage by one pulse.
   `C[k] = L[0] + .. + L[k]`, `C[7] = TOTAL`.
3. **RESTS and DENS.** `rank` from `sq_rank_build(n = 8, first_fixed = 1)`; `play[k] = rank[k] < DENS` (DENS >= 1 always plays stage 0).
4. **MODE, RDIV and RATCH.** The order `ord_m`, the dividers `div[k]` and the long flags are drawn (7 + 8 + 8 draws, always). `div_eff[k] = min(div[k], RDIV)`. Each stage starts as mode 0 if it does not play, I- (hold) if it is long, else I (one note).
   Walking `ord_m`, the first RATCH stages with `play[k]` and `L[k] > div_eff[k]` (stages that really repeat) become mode II (re-fire every `div_eff` pulses).
5. **VEL.** `vel[k] = sq_dyn_vel(50 + rnd(78), DYN)` per stage (SP17): DYN is baked in here.
6. `tot = TOTAL`, `off = OFFSET`, `dir = DIR`.

Cache (43 B, `StageCache`): `note[8] vel[8] mode[8] div[8] C[8] tot off dir`. `cache_bytes = 43`.

## 4. Step (ISR; pure function of `(cache, g)`; `hc` is ignored and `AF_CHORD` is clear)

`u = g + off`; `p = sq_dir_map(u, tot, dir)` (FWD / BWD / PEND over the `tot` pulses, SP16); `k` = the first stage with `p < C[k]` (at most 7 compares); `pos = p - C[k-1]` (0 for stage 0).
Fire rule: mode 0 never; mode I and I- when `pos == 0`; mode II when `pos mod div_eff == 0`. `note, vel` are the stage's and are valid for every `g`. `gate = 50`.
A firing mode I- stage returns `PO_HOLD` with `hold` = the number of pulses (1..8) that the traversal stays inside the stage going forward from this pulse: all the stage's pulses with FWD, one with BWD, and the exact forward run with PEND.
The core holds the note that long (ignoring the global GATE) and ends it one control block before the next note-on. The pulses that follow return no hit. `loop_len = tot` (8..64). `g` is the generator position of SEQUENCER_UX 1.4:
a `TOTAL` above 16 is heard past step 16 only on OWN pages; a page that reads an earlier page repeats it.

## 5. The AlgoDesc table (the one `proto.c` reads)

`run_protos.sh` extracts the block between the two marker comments from this file and from `proto.c` and requires them to be identical:
a remap (a knob, a dice button, a range, a default, a label) is an edit to this block and the same edit in the firmware copy, nothing else.

```c
/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_STAGE = {
    .name = "STAGE", .id = SQ_ALGO_STAGE, .n_seed = 5, .flags = 0, .cache_bytes = 43,
    .knob_morph = { 0, 1, 7, 3, 4, 5, 6, 2 },
    .seed_k = { 1, 2, 3, 4, 5 },
    .morph = {
        { "DENS",   0,  8,  6, 1, 1 },
        { "TOTAL",  8, 64, 16, 1, 4 },
        { "RANGE",  0, 36, 12, 1, 4 },
        { "RATCH",  0,  8,  2, 1, 1 },
        { "OFFSET", 0, 63,  0, 1, 4 },
        { "RDIV",   1,  4,  4, 1, 1 },
        { "DYN",    0, 10, 10, 1, 1 },
        { "DIR",    0,  2,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {2}}, {1, {4}}, {1, {3}}, {2, {1, SQ_SEED_MOD}} },
    .sub = { "NOTES", "RESTS", "VELO", "RT+HLD", "LONG" },
    .build = stage_build, .loop_len = stage_loop_len, .step = stage_step, .hi = stage_hi, .sub_note = 0, .mod = stage_mod,
};
/* ALGODESC-END */
```

## 6. How STAGE meets the rest of the design

- **Global settings** (A3 5): KEY, OCTAVE and SCALE enter only through `g` (`sq_note`). GATE, DIV and SWING are the core's (a pulse is one DIV step); `build()` never reads them (the purity test varies them and requires an identical cache).
- **Steps entered on the keys** (SEQUENCER_UX 9.2): the core overlays `StepEdit` on `step()`'s output, keyed to the pulse position; a step turned ON by hand plays its stage's pitch.
- **HARMONY** (HARMONY.md 4.3, A3 20): `pull = 2 + RANGE * 6 / 36` (`harm_pull`, default 4); strong steps are the first pulse of a stage and the long holds; the first hit of each chord is the anchor; the fold range is `lo = 0 .. hi(m)`
  (`harm_lo`, `stage_hi`). A re-fired stage shares one pulled pitch. The default HARMONY state gives the plain output (H-INERT, checked in `golden.txt`). The prototype's chord run marks strong steps from `vel >= 100` and holds; the first pulse of a stage is the core's to add.
- **`hi(m) = max(RANGE + 1, 12)`**: the raw value maps to at most RANGE, and a snap up to the next scale tone adds at most 1 (no scale of the 16 has a gap over 3 semitones).

**Test approximation (P2-016 review, P2-017):** the chord run of `tests/design/proto_main.c` (`harm_run`) is not the core. It takes a step as *strong* when its velocity is >= 100 or it is a hold; the first pulse of each stage; the other strong steps of HARMONY.md 4.3 are the core's to add (it knows the cycle and the stage starts) and are not cached. The golden `HARM` sections therefore show the pull with that approximation.

## 7. Deviations and open points found while writing the prototype (for the reviewer)

1. **`hi` is `max(RANGE + 1, 12)`, not `max(RANGE, 12)`** (ADR-0003 section 20): the late quantise can snap the top value up by a semitone, and the host test checks every note against `hi`. ADR-0003 is not edited here (P2-029 owns the amendment).
2. **Late quantise replaces ml185.md's `idx = (pos * N) / 12` table lookup** (SP6/SP7), so its informative notes differ. The rest of its informative check is reproduced: the sounding steps `X X - - X X - X - X - - - X X -`, the velocities (98 98 89 95 95 63 78 82), which is the lane logic (a mode I- stage of one pulse sits at step 13); the notes are the golden file's.
3. **The I- hold is a pulse count in `PulseOut.hold`, computed by `step()`** from the traversal (ml185.md 6.3 says "L pulses minus one control block"): the minus-one-block is the core's (A3 7.3). With BWD the traversal reaches a stage at its last pulse, so the hold is 1; with PEND it is the exact forward run.
   With `L = 1` a mode I- stage returns `PO_HOLD` with `hold = 1`: one full pulse, ignoring the global GATE (ml185.md called it "almost the same as I").
4. **`div_eff` is stored, not `div`**: RDIV is baked into the cache by `build()`, so `step()` needs no `rdiv`; the cache is ADR-0003's 43 B.
5. **`knob_morph` is `{0,1,7,3,4,5,6,2}`** (the 3B re-lock of `knob_map.tsv`: DIR on K3, RANGE on S4); ALGORITHM_SET 5.2 and FULL_CONTROL 3.4 still show the earlier layout. Only the table changes: the morph ids are the same.
6. **Gate is a constant 50 %** of a pulse for modes I and II (ml185.md `gate_pct`); the core multiplies by the global GATE. A hold ignores it.

## 8. Golden file and tests

`golden.txt` (regenerate with `GOLDEN_UPDATE=1 sh tests/design/run_protos.sh`) holds, each as `t note vel gate flags hold lane1 lane2 lane3 lane4` for t = 0..127: 16 base cases (channel 0, 1 x MINPEN, CHROM x key 0, 5 x octave 0, -1 at the default morphs and seeds);
every morph at min, mid and max (24 cases: all four ROW 1 and all four ROW 2 knobs, including TOTAL 8 and 64); each of the five dice layers with two diced seeds (10 cases); three combinations (all minimum, all maximum, all layers diced);
then the HARMONY runs: the default state and ROLE OFF with 1-4-5-1 (both equal to the plain output, H-INERT) and 1-4-5-1 in C major for ROLE AUTO, ROOT and FULL.
`tests/design/proto_main.c` also checks, for 40 random settings, the purity contract (A3 14), the table rules and the music theory: every note is in the scale and in `-12 .. hi(m)`, `vel` 1..127, `gate` 1..100, and a `PO_HOLD` carries `hold` 1..8.
