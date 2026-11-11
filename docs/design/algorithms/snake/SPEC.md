# SNAKE: algorithm spec for the prototype and the firmware (P2-016)

Status: reference prototype, design only. Source of the behaviour: [../snake.md](../snake.md) 6 (the reasoning, the 14 traversal tables and the informative check); this file is the **implementation contract** that
`proto.c` follows and `golden.txt` records. Where the two differ, section 7 says so. The shared types and primitives are in [../common/](../common/); sections cite ADR-0003 as `A3`, ALGORITHM_SET as `AS`
and the sibling ACID spec as [../acid/SPEC.md](../acid/SPEC.md).

SNAKE reads three lanes of a 4x4 grid (which steps sound, which pitch, which velocity) through one of 14 traversal paths. Each lane has its own length, so the pattern is a polymeter
that drifts against the bar. `proto.c` is the firmware file: P4 copies it, adds `static` to its three functions and the cache type, and wires the `ALGOS[2]` row. Nothing else changes (A3 14).

## 1. Parameters (morph id order; `build()`'s `m[]`)

| Id | Label | Range | Default | Step / fast | Knob slot (`knob_morph`) | Meaning |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | `DENS` | 0..16 | 10 | 1 / 1 | K1 | cells that trigger: those with rank < DENS (0 = KEYS ONLY: no generated hit) |
| 1 | `LEN` | 1..16 | 7 | 1 / 1 | K2 | length of the pitch lane (polymeter against the 16-step gate lane) |
| 2 | `RANGE` | 0..48 | 12 | 1 / 4 | K3 | pitch span in semitones above the root (a bass-sized default: one octave) |
| 3 | `PATH` | 1..14 | 7 | 1 / 1 | K4 | which of the 14 traversals orders the cells (re-orders pitch, rhythm and velocity at once) |
| 4 | `OFFSET` | 0..15 | 0 | 1 / 1 | S1 | where the lanes start: all three read `u = t + OFFSET` |
| 5 | `GLEN` | 1..16 | 16 | 1 / 1 | S2 | length of the gate lane (a shorter lane repeats and drifts) |
| 6 | `DYN` | 0..10 | 10 | 1 / 1 | S3 | velocity depth: 10 = the lane as drawn, 0 = flat 90 (below the TB-3 accent threshold) |
| 7 | `DIR` | 0..2 | 0 | 1 / 1 | S4 | pitch lane direction: FWD, BWD, PEND |

`knob_morph = {0,1,2,3,4,5,6,7}` (ids are in knob order: ROW 1 = K1 DENS, K2 LEN, K3 RANGE, K4 PATH; ROW 2 = S1 OFFSET, S2 GLEN, S3 DYN, S4 DIR). **Morph id 2 is the pitch width (RANGE)** that `harm_pull` reads.
Every ROW 2 default is neutral. Every morph is a threshold or an index on stored draws, so it is reversible: turning back restores the exact pattern.

## 2. Layers, seeds and dice

Four seeds (`n_seed` 4), one stream each (`sq_rng_init(seed, salt)`; the salts are constants of `proto.c` and define the golden file). Default seed k of channel ch: `(0x3A5D * (ch * 4 + seed_k)) & 0xFFFF`.

| Seed id | Name | `seed_k` | Salt | Draws |
| --- | --- | --- | --- | --- |
| 0 | NOTES | 1 | `0x4E2B9A71` | `rnd(256)` for each of the 16 cells |
| 1 | GATES | 2 | `0x1F6D83C5` | shuffle of the 16 cells (rank order) |
| 2 | VEL | 3 | `0x7B30E5A9` | `50 + rnd(78)` for each of the 16 cells |
| 3 | POLY | 4 | `0x29C4F18D` | `vlen = 3 + rnd(10)` |

| Layer | Button sub-label | Seeds re-rolled |
| --- | --- | --- |
| NOTES | `NOTES` | 0 |
| HITS | `GATES` | 1 |
| ACCNT | `VELO` | 2 |
| ARTIC | `-` | **absent** (`LayerDesc.n = 0`): pressing it changes nothing, the screen shows `ARTIC -` (AS 4.4) |
| SHAPE | `DRIFT` | 3 (how long the velocity lane is, so how it drifts against the rhythm) and `s_mod` |

### 2.1 The mod lanes and `s_mod` (P2-017, MODULATION 3 and 4)

The mod gen adds **no morph and no button**. Its one input is `s_mod` (`SQ_SEED_MOD`, seed id 6): it is stored in `ModSave` (not in `AlgoState`), handed to the harness as a seventh seed, and **re-rolled by the SHAPE layer together with the seed the layer already had** (DRIFT + `s_mod`). `build()` and `step()` never read it, so dicing it cannot move a note (checked by the purity test (g) and visible in the golden: the `dice SHAPE` cases keep the notes of the base case).
`AlgoDesc.mod(cache, g, hc, out_pulse, ModIdx*)` reports **which index each lane is read at** (a pure function like `step()`); the shared SP18..SP20 turn it into the value of each of the 4 slots (`sq_mod_lane`). Lane A (slots 1, 3) is **mod snake A**: the cell of the PITCH lane (`order[dir_map(u, len, dir)]`, the same traversal cell as the note, so the filter move is glued to the pitch). Lane B (slots 2, 4) is **mod snake B**: a cell read like the VELOCITY lane (`order[u mod vlen]`, so it loops with the POLY length and drifts against the pitch lane); MODULATION 4 also has it glide between neighbouring cells, which the prototype leaves to the core's CC emitter (it reports the cell). The golden shows the four slot values after `hold` on every row, for `s_mod` = the channel's default (`SQ_SMOD_DEFAULT`) and a channel LENGTH of 16.

**Decided (owner 2026-11-12, option A):** the SHAPE sub-label stays the primary name (`DRIFT`); the dice toast adds `+ MOD` when the mod seed moves too (an active device slot). MODULATION.md 3.4's `OCT+MOD` style labels are not used.

## 3. Build (A3 4; pure; reads only `m`, `seed`, `g->s.key`, `g->s.octave`, `g->snap[]`)

1. **PATH.** `order[SHAPE[PATH - 1][k] - 1] = k` (`SHAPE` is the 14 x 16 table of snake.md 6.3, a permutation of 1..16 per row). `order[s]` = the cell that speaks at traversal position `s`.
2. **NOTES.** `note[k] = sq_note(g, sq_span(raw[k], RANGE))` for each cell: SP7 maps the raw value to `0..RANGE` semitones and the late quantise (SP6) snaps it in the one `sq_note` call.
3. **GATES.** `rank` from `sq_rank_build(first_fixed = 0)`. The gate lane is `g[j] = (rank[order[j]] < DENS)` for `j = 0 .. GLEN - 1`; if `DENS >= 1` and none is set, `g[0] = 1` (a short lane never goes silent). DENS 0 forces nothing.
4. **VEL.** `vel[k] = sq_dyn_vel(50 + rnd(78), DYN)` per cell (SP17): DYN is baked in here.
5. **POLY.** `vlen = 3 + rnd(10)` (3..12).
6. `len = LEN`, `gl = GLEN`, `off = OFFSET`, `dir = DIR`.

Cache (55 B, `SnakeCache`): `order[16] note[16] vel[16] gb[2] len vlen gl off dir`. The gate bits are two bytes so the struct has no padding. `cache_bytes = 55`.

## 4. Step (ISR; pure function of `(cache, g)`; `hc` is ignored and `AF_CHORD` is clear)

`u = g + off`. `hit = gb bit (u mod gl)`. The pitch lane reads cell `order[sq_dir_map(u, len, dir)]` (FWD / BWD / PEND, SP16); the velocity lane reads cell `order[u mod vlen]`; the gate lane reads forward.
`note, vel` are valid for every `g`, hit or not (a step turned ON by hand plays its lane pitch). `gate = 50`, `flags = PO_HIT` on a hit, `hold = 0`. `loop_len = 16`. `g` is the generator position of SEQUENCER_UX 1.4:
the lane lengths run on `g`, so the longest cycle of the polymeter is at most 64 steps, and a page that reads an earlier page repeats it while an OWN page lets the lanes drift.

## 5. The AlgoDesc table (the one `proto.c` reads)

`run_protos.sh` extracts the block between the two marker comments from this file and from `proto.c` and requires them to be identical:
a remap (a knob, a dice button, a range, a default, a label) is an edit to this block and the same edit in the firmware copy, nothing else.

```c
/* ALGODESC-BEGIN (run_protos.sh diffs this block against SPEC.md section 5) */
const AlgoDesc ALGO_SNAKE = {
    .name = "SNAKE", .id = SQ_ALGO_SNAKE, .n_seed = 4, .flags = 0, .cache_bytes = 55,
    .knob_morph = { 0, 1, 2, 3, 4, 5, 6, 7 },
    .seed_k = { 1, 2, 3, 4 },
    .morph = {
        { "DENS",   0, 16, 10, 1, 1 },
        { "LEN",    1, 16,  7, 1, 1 },
        { "RANGE",  0, 48, 12, 1, 4 },
        { "PATH",   1, 14,  7, 1, 1 },
        { "OFFSET", 0, 15,  0, 1, 1 },
        { "GLEN",   1, 16, 16, 1, 1 },
        { "DYN",    0, 10, 10, 1, 1 },
        { "DIR",    0,  2,  0, 1, 1 },
    },
    .layer = { {1, {0}}, {1, {1}}, {1, {2}}, {0}, {2, {3, SQ_SEED_MOD}} },
    .sub = { "NOTES", "GATES", "VELO", "-", "DRIFT" },
    .build = snake_build, .loop_len = snake_loop_len, .step = snake_step, .hi = snake_hi, .sub_note = 0, .mod = snake_mod,
};
/* ALGODESC-END */
```

## 6. How SNAKE meets the rest of the design

- **Global settings** (A3 5): KEY, OCTAVE and SCALE enter only through `g` (`sq_note`). GATE, DIV and SWING are the core's; `build()` never reads them (the purity test varies them and requires an identical cache).
- **Steps entered on the keys** (SEQUENCER_UX 9.2): the core overlays `StepEdit` on `step()`'s output, keyed to the step (the gate position).
- **HARMONY** (HARMONY.md 4.3, A3 20): `pull = 2 + RANGE * 6 / 48` (`harm_pull`, default 3); strong steps are the lane notes with velocity >= 100; the first hit of each chord is the anchor; the fold range is `lo = 0 .. hi(m)`
  (`harm_lo`, `snake_hi`). PATH changes keep each lane note's pitch (the pull acts per lane note). The default HARMONY state gives the plain output (H-INERT, checked in `golden.txt`).
- **`hi(m) = max(RANGE + 1, 12)`**: the raw value maps to at most RANGE, and a snap up to the next scale tone adds at most 1 (no scale of the 16 has a gap over 3 semitones).

**Test approximation (P2-016 review, P2-017):** the chord run of `tests/design/proto_main.c` (`harm_run`) is not the core. It takes a step as *strong* when its velocity is >= 100 or it is a hold; the other strong steps of HARMONY.md 4.3 are the core's to add (it knows the cycle and the stage starts) and are not cached. The golden `HARM` sections therefore show the pull with that approximation.

## 7. Deviations and open points found while writing the prototype (for the reviewer)

1. **`hi` is `max(RANGE + 1, 12)`, not `max(RANGE, 12)`** (ADR-0003 section 20): the late quantise can snap the top value up by a semitone, and the host test checks every note against `hi`. ADR-0003 is not edited here (P2-029 owns the amendment).
2. **Late quantise replaces snake.md's `idx = (pos * N) / 12` table lookup**: the span is `sq_span(raw, RANGE)` and the note is the nearest scale tone (SP6/SP7). The notes of snake.md's informative check therefore differ; its **hit pattern and velocities for the first 16 steps are reproduced exactly**
   (rest, hit pattern `- X X X X - - X - - X X X X - X`, velocities 95 82 89 122 / 82 / 60 95 82 89 / 60), which is the lane logic, and the notes are the golden file's.
3. **DYN is baked into the per-cell velocity at build**, so the cache needs no `dyn` byte; the cache stays at ADR-0003's 55 B only because the gate bits are stored as two bytes (a `uint16_t` would pad the struct to 56).
4. **`loop_len = 16`** (the gate lane is the bar and stays 16 for the playhead and the key-entry positions, whatever GLEN is).
5. **Gate is a constant 50 %** (`gate_pct`, snake.md 6.3); the core multiplies by the global GATE.
6. **Morph ids equal knob order**, so `knob_morph` is the identity; `knob_map.tsv` (3B re-lock) keeps SNAKE unchanged.

## 8. Golden file and tests

`golden.txt` (regenerate with `GOLDEN_UPDATE=1 sh tests/design/run_protos.sh`) holds, each as `t note vel gate flags hold lane1 lane2 lane3 lane4` for t = 0..127: 16 base cases (channel 0, 1 x MINPEN, CHROM x key 0, 5 x octave 0, -1 at the default morphs and seeds);
every morph at min, mid and max (24 cases: all four ROW 1 and all four ROW 2 knobs); each present dice layer with two diced seeds (8 cases), and the absent layer (ARTIC) as two `absent (no change)` lines, the driver checking that
no seed moved; three combinations (all minimum, all maximum, all layers diced); then the HARMONY runs: the default state and ROLE OFF with 1-4-5-1 (both equal to the plain output, H-INERT) and 1-4-5-1 in C major for ROLE AUTO, ROOT and FULL.
`tests/design/proto_main.c` also checks, for 40 random settings, the purity contract (A3 14), the table rules and the music theory: every note is in the scale and in `-12 .. hi(m)`, `vel` 1..127, `gate` 1..100.
