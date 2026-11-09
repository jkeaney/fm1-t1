# 3. Snake shapes (the core algorithm)

**Input:** step number `s` (1 to 16) from the lane clock, and Snake Shape `n` (1 to 14).
**Output:** which of the 16 grid cells plays on this step.

## 3.1 The idea **[V]**

The 16 stored values sit in a 4×4 grid, cells numbered 1 to 16 in reading order (left to right, top to bottom).
A **shape** is a permutation table `shape[k]` giving, for each cell `k`, the step number at which that cell plays.
When step `s` arrives, the cell with `shape[k] == s` fires.

```
function cell_for_step(shape, s):          # s in 1..16
    for k in 1..16:
        if shape[k] == s: return k         # exactly one match, since shape is a permutation
```
Equivalent and faster: precompute the inverse `order[s] = k` once per shape change.

The device builds this as a 16-way `sel` whose match values are loaded from the chosen table, so changing
shape just reloads 16 match values. Nothing else is touched.

## 3.2 The 14 shapes **[V]**

Each grid shows the step number at which each cell plays (top-left is cell 1). All 14 tables were checked to
be valid permutations of 1..16.

```
 1  raster rows       2  rows, bottom-up     3  raster columns      4  columns, right-to-left
 1  2  3  4          13 14 15 16            1  5  9 13             4  8 12 16
 5  6  7  8           9 10 11 12            2  6 10 14             3  7 11 15
 9 10 11 12           5  6  7  8            3  7 11 15             2  6 10 14
13 14 15 16           1  2  3  4            4  8 12 16             1  5  9 13

 5  column serpentine   6  column serpentine (mirrored)   7  row serpentine    8  row serpentine (mirrored)
 1  8  9 16            4  5 12 13                         1  2  3  4          4  3  2  1
 2  7 10 15            3  6 11 14                         8  7  6  5          5  6  7  8
 3  6 11 14            2  7 10 15                         9 10 11 12         12 11 10  9
 4  5 12 13            1  8  9 16                        16 15 14 13         13 14 15 16

 9  diagonal zig-zag     10  diagonal zig-zag (mirrored)
 1  2  6  7             10 11 15 16
 3  5  8 13              4  9 12 14
 4  9 12 14              3  5  8 13
10 11 15 16              1  2  6  7

11  spiral in (cw)     12  spiral in (ccw)    13  spiral (from bottom-left)   14  spiral (from bottom-right)
 1  2  3  4             4  3  2  1            10  9  8  7                    7  8  9 10
12 13 14  5             5 14 13 12            11 16 15  6                    6 15 16 11
11 16 15  6             6 15 16 11            12 13 14  5                    5 14 13 12
10  9  8  7             7  8  9 10             1  2  3  4                    4  3  2  1
```

(Names are descriptive labels for the documentation, not the device's own.)

## 3.3 The raw tables (cell 1 to 16 → step number) **[V]**

| Shape | `shape[1..16]` |
| --- | --- |
| 1 | 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 |
| 2 | 13 14 15 16 9 10 11 12 5 6 7 8 1 2 3 4 |
| 3 | 1 5 9 13 2 6 10 14 3 7 11 15 4 8 12 16 |
| 4 | 4 8 12 16 3 7 11 15 2 6 10 14 1 5 9 13 |
| 5 | 1 8 9 16 2 7 10 15 3 6 11 14 4 5 12 13 |
| 6 | 4 5 12 13 3 6 11 14 2 7 10 15 1 8 9 16 |
| 7 | 1 2 3 4 8 7 6 5 9 10 11 12 16 15 14 13 |
| 8 | 4 3 2 1 5 6 7 8 12 11 10 9 13 14 15 16 |
| 9 | 1 2 6 7 3 5 8 13 4 9 12 14 10 11 15 16 |
| 10 | 10 11 15 16 4 9 12 14 3 5 8 13 1 2 6 7 |
| 11 | 1 2 3 4 12 13 14 5 11 16 15 6 10 9 8 7 |
| 12 | 4 3 2 1 5 14 13 12 6 15 16 11 7 8 9 10 |
| 13 | 10 9 8 7 11 16 15 6 12 13 14 5 1 2 3 4 |
| 14 | 7 8 9 10 6 15 16 11 5 14 13 12 4 3 2 1 |

## 3.4 Properties (use these as tests)

- Every shape is a permutation of 1..16.
- **Contiguous paths** (each step is a grid neighbour of the previous): shapes 5, 6, 7, 8 and 11, 12, 13, 14. These are the true "snakes".
- **Jumping orders** (consecutive steps not always adjacent): shapes 1, 2, 3, 4, 9, 10.
- Shape 1 is the identity: cell `k` plays at step `k`.
- Shapes come in pairs related by a flip: 1/2, 3/4, 5/6, 7/8, 9/10 (reversed grids), and the spirals are mirror images:
  12 is 11 flipped left-right, 13 is 11 flipped top-bottom, and 14 is 12 flipped top-bottom.

## 3.5 How the shape interacts with Length and Shift

- The clock produces a step number in `1..16` after Shift is added (see [06](06-playback-timing.md)). With Length `L` and Shift `d`,
  only steps `d+1 .. d+L` are ever produced, so only cells with `shape[k]` in that range play.
- Consequence: shortening the lane under a **serpentine or spiral** plays a *connected region* of the grid
  (the first `L` cells along the path), while under a **raster** it plays whole rows/columns.
- The shape does not move the stored values; it only re-maps which cell speaks at which step.

## 3.6 Rebuild notes

```
uint8_t order[17];                         // order[s] = cell k, built from SHAPE[n]
void set_shape(int n) { for k in 1..16: order[ SHAPE[n][k] ] = k; }
int  cell_for(int s)  { return order[s]; } // O(1) per tick
```
Store the 14 tables as `const uint8_t SHAPE[14][16]` (224 bytes). Generate the inverse at shape-change time.
