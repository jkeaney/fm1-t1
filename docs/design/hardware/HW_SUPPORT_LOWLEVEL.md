# HW_SUPPORT_LOWLEVEL: the hardware support model, phase 2 (low-level design)

> **SUPERSEDED (P2-022, decision 107): the wire / RIG / ROW 3 design below is replaced by the modulation layer in [MODULATION.md](../MODULATION.md). Kept as history; do not build from it. What survives is listed in MODULATION.md 5.3.**

Task P2-019 (owner decision 96; phase 1 approved 2026-11-11 as decisions 97-100). Status: **for owner review** (gate `human-review`).
Design only: no firmware. The table rows of section 5 are real and checked (`check_scenarios.py`, `check_help.py`,
`tools/check_sequencer_ux.py` pass). The TB-3 specifics (wire list, lane mapping, accent and slide behaviour) are phase 3, task P2-020;
every device name, wire name and number in sections 1-8 is a **placeholder that illustrates the format** (P2-020 filled in the TB-3 and P2-021 extended the format: section 9; the mock-ups of 4.2 and 4.3 now show the real TB-3 names).

Tags: **[V]** verified in a document or on hardware, **[I]** inferred, **(warning)** unconfirmed, a hypothesis for an H-test (section 6.3).

Inputs read: CLAUDE.md, PROCESS.md, HW_SUPPORT_HIGHLEVEL.md (HL), hardware/README.md, OWNER_QUESTIONS 96-100, P2-018 Log (owner conditions),
SEQUENCER_UX 2, 4.2, 9, 11, SCREEN_SPEC 2, TIMING_SPEC 3 (M2 to M5), ADR-0003 3.1, 7, 8, ALGORITHM_SET 4, the six algorithm SPECs, the six design TSVs,
the checkers `tests/design/*.py` and `tools/check_sequencer_ux.py`, `width_probe.sh`, `tb3-midi-guide.csv` (reference only).

## 0. What this document keeps, and the refinements it makes to HL

Kept unchanged (locked by decisions 96-100): per-channel RIG with up to 4 WIRES; states DEV / HAND / SEQ; algorithms stay oblivious (goldens untouched);
Option B (ROW 3 on PERF, RIG page by holding CHN); optional device, inert when none (decision 98); built-in profile table plus GENERIC raw CC, our own data,
midi.guide cited as a reference only (99); STOP parks SEQ wires at LEVEL, 4 wires, undo = seeds only, no per-step lock, a chosen device starts its wires in HAND,
SEQ-wire cap 8, no REC of ROW 3 (100); the three-rows condition (97).

Refinements (found while making HL buildable; none changes a locked decision; the ones that touch a rule the owner may care about are owner questions in section 7):

| # | HL said | This document | Why |
| --- | --- | --- | --- |
| RF1 | `Rig` lives inside `ChanSave` (344 -> 372 B) | `RigSave rig[4]` is a **sibling array** of `HarmonySave` in `SeqPreset` v4; `ChanSave` stays 344 B | the `sizeof(ChanSave) == 344` and `UndoRec == 92` asserts, the per-channel page-copy code and every P4-001..P4-006 test stay untouched; the preset total is the same 1508 B |
| RF2 | .bss 184 B | **274 B** (section 1.6) | the runtime table has to remember the CC, MIDI channel and neutral of what it last sent, or a rewire or a preset swap cannot park the old parameter |
| RF3 | `~` appended to dice sub-labels and morph cells (`ACCNT~`) | a 3x3 px **ride dot** in the cell corner (graphics, 0 px of text width) | measured: with `~` the PERF button row 2 needs 247 px of ink against 240 (`GT+SLD~` 50, `ROTATE~` 49) |
| RF4 | profile header: acc / slide fields | also the four **suggested wires with their layer, form and MOVE defaults** (HwProf 192 B; **superseded by RF9, P2-021: per-parameter lanes and named wire sets, HwProf 316 B**) | "a device starts its wires in HAND" (decision 100) needs the starting configuration somewhere; it is the profile |
| RF5 | RST R3 "as one undo record" | RST R3 is **not undoable** (it is a LEVEL/MOVE/state edit) | decision 100 (undo = seeds only); the RESET page already cancels with EDIT before anything is reset |
| RF6 | lane CCs all stamped before their note | 2 CCs ahead, the rest after the note, 1 ms apart (3.5) | a DIN link needs 0.96 ms per 3-byte message [V: MIDI spec]; 8 CCs ahead of a note would delay it by up to 7 ms |
| RF7 | wire names "illustrative" | names **at most 5 letters and 38 px** (AF_S), enforced by the profile generator | the compact ROW 3 cell is chip + name in 49 px (4.1) |
| RF8 | muting is silent for the rig | muting a channel **parks** its SEQ wires (like STOP); unmuting resumes at the next step | otherwise a muted bass leaves the filter wherever the last lane value was |

## 1. Data structures

All structs are packed by construction: every member is `uint8_t` or `char`, so alignment is 1 and there is no padding. Multi-byte runtime fields are `uint16_t` placed on even
offsets. Byte orders do not matter (RAM only; the preset is stored in the device's own endianness like `SeqPreset` today).

### 1.1 Saved: `Wire`, `Rig`, `RigSave` (new file `sq_rig.h`, included straight after `sq_types.h`)

```c
#define SQ_NWIRE     4          /* wires per channel = the four knobs of ROW 3 (decision 100) */
#define RIG_SEQ_CAP  8          /* SEQ wires on the whole device (decision 100) */
enum { WS_DEV = 0, WS_HAND = 1, WS_SEQ = 2 };                 /* cfg bits 0..1; 3 is invalid and loads as DEV */
enum { WF_RAND = 0, WF_WALK = 1, WF_RAMP = 2, WF_ACC = 3 };   /* cfg bits 2..3 */
enum { WL_NONE = 0, WL_NOTES = 1, WL_HITS = 2, WL_ACCNT = 3, WL_ARTIC = 4, WL_SHAPE = 5 };  /* cfg bits 4..6; 6, 7 load as NONE */
typedef struct {
    uint8_t cc;      /* @0  0 = empty wire, else 1..127: the control change number */
    uint8_t cfg;     /* @1  bits0..1 STATE, bits2..3 FORM, bits4..6 LAYER, bit7 reserved (0) */
    uint8_t level;   /* @2  0..127 resting value; the knob of ROW 3 */
    uint8_t move;    /* @3  0..127 lane depth */
    uint8_t ride;    /* @4  0 none, 1..8 = morph slot K1..K4 = 1..4, S1..S4 = 5..8 that scales MOVE */
    uint8_t rsv;     /* @5  0; reserved for the per-step lock flag (decision 100: not in v1) */
} Wire;                                                         /* 6 B */
typedef struct {
    uint8_t prof;    /* @0  0 = no device (the rig is inert), 1 = GENERIC, 2.. = built-in profiles (never renumbered) */
    uint8_t rsv0;    /* @1  0 */
    Wire    w[SQ_NWIRE];  /* @2, @8, @14, @20 */
    uint8_t rsv1[2]; /* @26 0; reserved */
} Rig;                                                          /* 28 B */
typedef struct { Rig r[SQ_NCH]; } RigSave;                      /* 112 B; SQ_NCH = 4 */
```

Rules: a loader zeroes `rsv*`, maps invalid `cfg` fields as noted, clamps `level`/`move` to 127, treats an unknown `prof` id (a preset saved by newer firmware) as GENERIC (names `CCnn`),
and downgrades SEQ wires beyond the cap of 8 (counted in channel then wire order) to HAND with the toast `SEQ WIRES CAPPED AT 8`. An empty wire (`cc == 0`) is always DEV.
`ride` binds to the morph **slot**, not the name, so it follows an algorithm change (HL 3.1). A wire with `cc != 0` and `prof == 0` is kept but inert (the rig is off).

### 1.2 Runtime: `RigRT` (ISR-owned, `.bss`)

```c
typedef struct {          /* one per wire slot, index c * 4 + i; written only by the ISR (rig_*), read by the UI for the sent-value bar */
    uint8_t  cc;          /* @0 CC number last sent on this slot, 0 = none ever */
    uint8_t  mch;         /* @1 MIDI channel (0..15) it was sent on, 0xFF = unknown */
    uint8_t  last;        /* @2 last value sent, 0xFF = unknown (resend whatever the value) */
    uint8_t  home;        /* @3 the neutral of the parameter that cc names (profile `home`), kept so a rewire can park it */
    uint8_t  flags;       /* @4 bit0 ACTIVE (SEQ drives it, a park is owed), bit1 DIRTY (send at the next chance), bit2 HANDPEND (the knob moved), bit3 PARKING */
    uint8_t  rsv;         /* @5 */
    uint16_t tick;        /* @6 low 16 bits of fm1_ms at the last send (the HAND rate limit) */
} RigRT;                                                        /* 8 B; 16 slots = 128 B */
```

The ISR is the only writer, so there is no read-modify-write race on `flags`. The main loop never touches `RigRT` except to read `last` (one byte) for the sent-value bar.

### 1.3 UI to ISR: the request ring (`.bss`, 18 B)

```c
typedef struct { uint8_t q[16]; volatile uint8_t head, tail; } RigQ;   /* 8 entries x {op:3 | c:2 | i:2 | spare:1, arg:8}; single producer (main loop), single consumer (ISR) */
enum { RQ_LEVEL = 1,   /* a ROW 3 knob moved: arg unused; the ISR reads wire.level itself, so a lost duplicate is harmless */
       RQ_SYNC  = 2,   /* the Rig of channel c changed (state, param, device, preset): reconcile RigRT with it (3.4) */
       RQ_PLAYNOW = 3  /* send LEVEL of (c, i) now although the sequencer is stopped (HAND or SEQ, user turned it while stopped) */ };
```
A full queue drops the new entry and the UI re-pushes on the next detent (the value is read at service time, so nothing is lost for `RQ_LEVEL`). `RQ_SYNC` is never dropped: the UI
retries from the main loop until it is accepted (at most 8 outstanding, one per channel per change).

### 1.4 UI state (`.bss`, 8 B, inside the 3072 B UI allowance of SEQUENCER_UX 9.4)

`rig_sel` (selected wire 0..3, 2 bits) + `rig_open` (1 bit) + `knob_row` (0..2, 2 bits) + `row_flash` (frames left, 4 bits) + `rig_cue` (first-use bits, 3) + `rig_nseq` (SEQ wires on the device, 4 bits) +
`rig_chip[4]` (per-channel chip cache: 0 none, 1 H, 2 ~) + `rig_toast_id`. Two **persistent** bits go into `ui_prefs2` (they survive power-off, like the first-run bit): `PREF2_RIG_INTRO`
(the one-time "4 KNOBS ON ROW 3" toast was shown) and `PREF2_RIG_ROW3SEEN` (ROW 3 was visited once; the `OCT+ TWICE` cue stops). P4-030 checks that two bits are free.

### 1.5 Profiles (flash, const): see section 2 (`HwParam` 14 B, `HwSet` 10 B, `HwProf` 316 B, `HwProv` 76 B; P2-021 v2, section 9).

### 1.6 Where it lives, and the numbers

| Item | Where | Bytes | Note |
| --- | --- | ---: | --- |
| `RigSave` (the live copy of the 4 rigs) | .bss | 112 | beside the live `SeqPreset` image |
| `RigRT rig_rt[16]` | .bss | 128 | 4 channels x 4 wires |
| `RigQ`, stats (`sent`, `dedup`, `drop` as `uint16_t`), `rig_epoch`, `rig_nseq` | .bss | 18 + 6 + 2 = 26 | stats are the H-RIG10 counters |
| UI state | .bss | 8 | 1.4 |
| **Total new .bss** | | **274** | HL said 184 (RF2) |
| Undo | .bss | 0 | `UndoRec` stays 92 B: seeds are the undo unit (decision 100) |
| `SeqPreset` v4 = 1396 + 112 | pool | 1508 B x 18 copies = 27144 B | was 25128 B: **+2016 B** |
| Profile table (8 profiles x 316) + provenance (8 x 76) | flash | 2528 + 608 = 3136 (P2-021; was 2144) | plus about 1.7 KB of code and 2 KB of strings: about 7 KB in all |
| CPU | ISR | about 40 integer operations per SEQ wire per step; `rig_block` about 70 operations per 725 us block | 8 wires = 320 operations per step instant, inside the +5 % of ADR-0003 |

Checks against the budgets: **.bss** headroom is 17984 B before the removals; SEQUENCER_UX 9.4 leaves 11123 B after its own 6861 B, so the rig leaves **10849 B**: above the owner's
8192 B trigger by 2657 B and above the 4096 B floor by 6753 B; after the E3 removals 46496 - 6861 - 274 = **39361 B**. **Pool**: free today 25764 B; SEQUENCER_UX needs 25128 B; with the rig
27144 B, which **does not fit before the fx-bus removal R6** (it is 1380 B short); the escape is unchanged (`SQ_NSLOT` = 8: 8 x 1508 + 2 x 1508 = 15080 B, 10684 B spare); after R6, 217975 - 27144 =
**190831 B** free. **Flash**: about 6 KB of more than 140 KB freed.

### 1.7 Snapshots, presets, undo

| Mechanism | Rig interaction |
| --- | --- |
| Preset slot (flash, P7-001/002) | stores `RigSave` after `HarmonySave`; `SQ_PRESET_BYTES` 1396 -> 1508 and the preset **version 3 -> 4**; P7-001 sizes the flash map from `sizeof(SeqPreset)`, not from a literal |
| Loading a v3 preset | `rig` is zero-filled: `prof = 0` everywhere, so the rig is inert and the plain sequencer is unchanged |
| Staging copy (a queued load) and the undo-load copy (SAVE hold) | both are `SeqPreset` copies, so they carry the rig; applying either goes through `rig_bar_swap` (3.4) |
| Per-channel undo (`UndoRec`, EDIT tap) | **seeds only** (decision 100): dice, morph, reset and edits are undone; wire state, LEVEL, MOVE, LAYER, FORM, PARAM, RIDE and the device are not. Because the lanes read the seeds, undoing a dice re-rolls the lanes back with the notes |
| RESET page (hold EDIT) | RESET + dice button resets that layer's seeds, so any lane riding the layer resets with it (same seeds). RST R3 (RESET + OCT+ on ROW 3) resets LEVEL to the neutral, MOVE to 0, state to HAND, form/layer to the profile suggestion; not undoable (RF5) |
| Algorithm change | the rig is kept; nothing is sent by the change itself (3.4) |
| TEMP/T-1 snapshot, old pattern tracks | not involved |

### 1.8 Static asserts to write (all in `sq_rig.h` unless noted; P4-023)

```c
_Static_assert(sizeof(Wire) == 6 && offsetof(Wire, cc) == 0 && offsetof(Wire, cfg) == 1 && offsetof(Wire, level) == 2 && offsetof(Wire, move) == 3 && offsetof(Wire, ride) == 4, "Wire");
_Static_assert(sizeof(Rig) == 28 && offsetof(Rig, w) == 2 && offsetof(Rig, rsv1) == 26, "Rig");
_Static_assert(sizeof(RigSave) == 4 * 28, "RigSave");
_Static_assert(sizeof(RigRT) == 8 && offsetof(RigRT, tick) == 6, "RigRT");
_Static_assert(sizeof(HwParam) == 14 && offsetof(HwParam, kind) == 10 && offsetof(HwParam, link) == 11 && offsetof(HwParam, dcfg) == 12 && offsetof(HwParam, dmove) == 13
               && sizeof(HwSet) == 10 && sizeof(HwProf) == 316 && offsetof(HwProf, nset) == 15 && offsetof(HwProf, prm) == 32 && offsetof(HwProf, set) == 256 && sizeof(HwProv) == 76, "Hw* v2");
_Static_assert(sizeof(ChanSave) == 344 && sizeof(UndoRec) == 92, "rig must not change these");        /* in sq_types.h, already there */
_Static_assert(sizeof(SeqPreset) == 1396 + sizeof(RigSave), "SeqPreset v4");                            /* 1508 */
_Static_assert(SQ_NWIRE * SQ_NCH <= 16 && RIG_SEQ_CAP <= 16 && (1 << 7) > 127, "rig limits");
```
Host tests (P4-025/P4-027) add: every enum fits its bit field; a random `Rig` round-trips through save, load and the loader's clamping with every invalid `cfg`; `sizeof(RigQ) == 18`.

## 2. The profile format

### 2.1 Records (flash, `const`, generated; no pointers, so there are no relocations)

```c
typedef struct {            /* 14 B (v2, P2-021; v1 was 10 B: cc lo hi home nm[6]) */
    uint8_t cc, lo, hi, home;   /* @0  lo..hi the legal range sent; home the neutral (a SELECT: the CC at the centre of its home zone) */
    char    nm[6];          /* @4  at most 5 letters + NUL */
    uint8_t kind;           /* @10 bits0..1 KIND (0 CONT, 1 SEL; 2, 3 load as CONT); bits4..7 = zones - 1 for SEL (2..16 zones), 0 for CONT */
    uint8_t link;           /* @11 bits0..4 index of the partner in prm[] (0x1F = none); bit5 GATED (a depth that matters only when its type is not in zone 1 = OFF);
                                   bit6 RESEND (re-send the depth 1 ms after each type change); bit7 DUCK (send the depth as 0 first, the type, then the depth again);
                                   bits 5..7 are 0 until H-RIG20 says otherwise */
    uint8_t dcfg;           /* @12 the lane this parameter gets when a wire takes it: cfg bits 2..6 (FORM, LAYER); for SEL the FORM field is DICE 0 / LOOP 1 (2.6) */
    uint8_t dmove;          /* @13 the MOVE it gets (a SEL: the spread, 0..127) */
} HwParam;
typedef struct { char nm[6]; uint8_t p[4]; } HwSet;     /* 10 B: a named WIRE SET (4.9): the four parameters (indexes into prm[], 0xFF = empty) that wires 1..4 take */
typedef struct {            /* 316 B (v2; v1 was 192 B) */
    char    name[9];        /* @0  device name, at most 8 characters + NUL: the toast, the RIG banner, the picker */
    uint8_t id;             /* @9  stable forever; 0 NONE and 1 GENERIC are reserved */
    uint8_t n;              /* @10 parameters in use, 0..16 (GENERIC: 0) */
    uint8_t acc_mode;       /* @11 0 VEL (default = today's behaviour), 1 CC, 2 NONE */
    uint8_t acc_arg;        /* @12 VEL: the accent velocity threshold (0 = the global ACC_VEL); CC: the CC number */
    uint8_t slide_mode;     /* @13 0 OVERLAP (default = today's behaviour), 1 CC, 2 NONE */
    uint8_t slide_arg;      /* @14 CC mode: the CC number */
    uint8_t nset;           /* @15 wire sets in use, 0..6; set 0 is what a freshly picked device starts with (v1: sug[4]) */
    uint8_t prov;           /* @16 index into the provenance table */
    uint8_t flags;          /* @17 bit0 GENERIC (raw CC: the name is `CCnn`, range 0..127, home 64) */
    uint8_t rsv[14];        /* @18 */
    HwParam prm[16];        /* @32 ... @256 */
    HwSet   set[6];         /* @256 ... @316 (v1: the four suggested wires' layer, form and MOVE lived in sug_cfg[]/sug_move[]; now per parameter, in HwParam) */
} HwProf;
typedef struct {            /* 76 B, per profile: who says so (decision 99) */
    char    src[24];        /* where the facts come from, for example "vendor MIDI chart" or "own test" */
    char    how[24];        /* how it was checked, for example "by ear, H-RIG1" or "chart only" */
    char    cred[24];       /* the contributor credited, for example "J. Keaney" or "community chart" */
    uint8_t level;          /* 0 listed in one source, 1 cross-checked in two independent sources, 2 verified on hardware by a named person */
    uint8_t yy, mm;         /* year since 2026 and month of the last check */
    uint8_t rsv;
} HwProv;
```
The zero value of `acc_mode`, `slide_mode`, `acc_arg` is **today's behaviour** (accent by velocity at the global threshold, slide by overlapping notes), so a profile that says nothing changes nothing and
the no-device channel needs no special case. NONE (2) means the device cannot be asked: the accent and slide glyphs dim on that channel so the screen is honest (HL 5, "slide semantics").

`prof` ids: 0 NONE (no record), 1 GENERIC (record with `n = 0`, flag GENERIC), 2 = the TB-3 (P2-020), then others. An id is **never renumbered or reused**, because it is saved in presets.

### 2.2 GENERIC (raw CC)

A wire on GENERIC takes any CC 1..127 with K1 PARAM (the value shows as `CC74`); names are `CCnn` (at most 5 letters); range 0..127; neutral 64; no suggested wires (all four start empty). It is the answer for every
device that has no profile and the path for a user who knows the CC chart of their gear. Accent and slide are as today.

### 2.3 Provenance, ownership and licence (decision 99)

**Hardware belongs to its owners** (the statement for the future user guide, P8-002; keep it short and keep it in the guide's chapter on devices):

> *Hardware belongs to the people who own it, not to the company that made it. The device profiles built into this sequencer describe how to talk to your gear with plain MIDI. They are
> our own notes, written from the manufacturers' published MIDI charts, from tests on real units, and from facts that owners and communities have shared and checked; sites such as midi.guide
> are cited as references and are not copied. Every profile records where its facts come from, how they were checked and who contributed them, and you can read that in MENU > INFO. If your device
> is not listed, GENERIC lets you type any control change number, and your corrections are welcome: a profile is a small text file.*

Rules the generator (2.4) enforces so that the statement stays true:

| Rule | Check |
| --- | --- |
| Every profile has all three provenance strings, a level and a date | `tools/gen_hwprof.py` fails on an empty field |
| The data is ours | a profile file may not name `tb3-midi-guide.csv` or any `docs/design/hardware/*.csv` as input; the generator never opens them; `src` may mention midi.guide only as "cross-checked", never as the sole source (a profile whose only source is a database is level 0 and fails the build for the suggested wires) |
| Every wired parameter has two sources or a hardware check | `level >= 1` for any param named in a `set` line |
| The CSV stays reference | `docs/design/hardware/README.md` keeps the CC BY-SA 4.0 attribution (already there); the CSV is never read by `gen_ui_tables.py` or `gen_hwprof.py` |
| Names fit the screen | at most 5 letters, uppercase, digits and `-`/`+` only, and at most 38 px in AF_S, measured through `width_probe.sh` (the compact cell is chip + space + name in 49 px; `H ` is 11 px) |

Where the user sees it: MENU > INFO gets one row per profile in use, `HW DATA: TB-3`, whose value is `L2 JK 2611` (level, credit initials, year-month); ABOUT lists the credits (P8). The RIG page
does not show provenance (screen space is for the rig).

### 2.4 Source files and generator

`tools/hwprof/<name>.tsv` (one file per profile, hand-written, reviewed once) with three blocks (v2, P2-021): a header (`id name acc_mode acc_arg slide_mode slide_arg prov.src prov.how prov.cred prov.level prov.date`),
one line per parameter (`cc lo hi home name kind link layer form move`; `kind` is `C` or `S<n>`; `link` is `-` or the partner's name with optional `:G` `:R`; `layer form move` is the lane the parameter gets when a wire takes it)
and up to six `set NAME p1 p2 p3 p4` lines (parameter names). `tools/hwprof/tb3.tsv` is the worked example. `tools/gen_hwprof.py` writes `build/gen/hwprof_tab.h` (the const arrays) and fails on: a duplicate id, a duplicate CC or name inside a
profile, more than 16 params or 6 sets, a name over budget, `home` outside `lo..hi`, a SEL with fewer than 2 or more than 16 zones, a `link` that is not mutual or names an unknown parameter, a SEL form other than DICE / LOOP,
a CONT form other than RAND / WALK / RAMP / ACC, a set naming an unknown parameter, a set name over 5 letters or 38 px, a missing provenance field, a wired parameter at level 0. The tool lives in P4-024; P2-020 supplies `tb3.tsv`, P2-021 extends it.

### 2.5 Lookup (pure, host-testable)

```c
const HwProf  *hw_prof(uint8_t id);                           /* 0 and unknown ids -> NULL (the caller treats unknown as GENERIC) */
const HwParam *hw_param(uint8_t id, uint8_t cc);               /* NULL for GENERIC or an unknown cc */
void           hw_range(uint8_t id, uint8_t cc, uint8_t *lo, uint8_t *hi, uint8_t *home);   /* GENERIC/unknown: 0, 127, 64 */
uint8_t        hw_next_cc(uint8_t id, uint8_t cc, int8_t dir);   /* K1 PARAM: the next listed cc (wrapping), or cc +/- 1 on GENERIC */
const HwSet   *hw_set(uint8_t id, uint8_t i);                     /* P2-021: wire set i of the profile, NULL past nset */
uint8_t        hw_zones(const HwParam *p);                        /* P2-021: 1 for CONT, n for SEL */
uint8_t        hw_zone_cc(const HwParam *p, uint8_t z);           /* P2-021: SEL zone z (1..n) -> the CC at its centre: lo + (2*z - 1) * (hi - lo + 1) / (2*n) */
uint8_t        hw_cc_zone(const HwParam *p, uint8_t v);           /* P2-021: a CC value -> its zone: (v - lo) * n / (hi - lo + 1) + 1 */
```

## 3. Engine integration

### 3.1 The lane: a pure function of the dice seeds, the step and the morphs

Algorithms are not touched: no algorithm code, `PulseOut`, `step()` or golden changes. The rig evaluates **beside** `sq_eval` from the same data the dice buttons write: the layer seeds in `ChanSave.st[algo].seed[]`,
read through the layer table that the dice code already uses.

```c
/* sq_rig.c: pure functions, no hardware, no hosted includes; tests/rig_test.c links them with a stub */
uint16_t rig_mix(uint16_t seed, uint8_t ch, uint8_t wi, uint8_t pos, uint8_t salt);
    /* x = seed<<16 | (ch<<6|wi<<4|(salt&15))<<8 | pos; x ^= x>>16; x *= 0x7FEB352D; x ^= x>>15; x *= 0x846CA68B; x ^= x>>16; return (uint16_t)x.
       It does NOT call the SP1 generator, so it consumes none of the algorithm's random state and cannot change a golden. */
int16_t  rig_lane(uint8_t form, uint16_t seed, uint8_t ch, uint8_t wi, uint8_t p, uint8_t g, uint8_t len, uint8_t accent);   /* -127..+127 */
uint8_t  rig_value(uint8_t level, uint8_t move, uint8_t ride_norm /*255 when ride == 0*/, int16_t lane, uint8_t lo, uint8_t hi);
uint8_t  rig_layer_seed(uint8_t ch, uint8_t layer, uint16_t *seed);   /* 1 = present: the first seed the dice of that layer re-rolls (ACID ARTIC: the GATE seed); 0 = the algorithm lacks the layer */
```

| Form | `lane(p)` (integers; `s` = seed of the riding layer, or the constant `RIG_SEED0` = 0xA55A when LAYER is NONE or the algorithm lacks the layer: the lane freezes, honestly) |
| --- | --- |
| RAND | `((rig_mix(s,ch,wi,g,1) & 255) * 254 + 127) / 255 - 127` |
| WALK | `a = rig_mix(s,ch,wi,g>>2,2) & 255; b = rig_mix(s,ch,wi,(g>>2)+1,2) & 255; v = a + (b - a) * (g & 3) / 4;` then as RAND on `v`; value noise with a lattice every 4 generator steps: smooth, no state (a jump at the loop end is accepted) |
| RAMP | `r = rig_mix(s,ch,wi,0,3); rot = (r >> 8) % len; q = (p + rot) % len; x = len > 1 ? q * 254 / (len - 1) : 127; lane = (r & 1) ? 127 - x : x - 127`; the seed picks direction and start, so a dice of the layer moves the sweep |
| ACC | `accent ? +127 : -127`; `accent` = the step's resolved velocity at or above the accent threshold (the profile's `acc_arg`, or the global `ACC_VEL`) and it hits; follows edits and the ACCNT dice for free |

`g` is the generator position of `sq_eval` (SEQUENCER_UX 9.2), so a page that copies a page copies its lane exactly (SEQUENCER_UX 1.4); `p` is the step index and is used only by RAMP (a sweep over the loop).
`value = clamp(level + (lane * depth) / 127, lo, hi)` with `depth = ride ? (move * ride_norm + 127) / 255 : move`, `ride_norm` the morph at slot `ride` normalised to 0..255 by the algorithm's own range
(`sq_morph_norm(ch, slot)`, a P4-006 accessor), `lo/hi` from `hw_range`. Division truncates toward zero; the host test fixes the exact table (`tests/rig_golden.txt`, separate from `tests/golden.txt`).

**Why goldens cannot change** (the tests that prove it, P4-025): T-RIG1 the event stream of the sequencer core with `prof = 0`, and with a rig whose wires are all DEV, is byte-identical to the current golden; T-RIG2 with SEQ wires
the NOTE events are identical to T-RIG1 and only CC events are added; T-RIG3 `rig_lane` is a pure function (same inputs, same output, any call order); T-RIG4 dicing a layer changes exactly the lanes that ride it.

### 3.2 Where the ISR calls it (placement in the unity build)

Include order (after `sq_types.h`; `felucca.c` gains five lines):

`sq_types.h` -> **`sq_rig.h`** (types, enums, asserts) -> **`sq_hwprof.c`** (includes the generated `hwprof_tab.h`; `hw_*` lookups) -> `sq_prims.c` -> `sq_algo_*.c` -> `sq_algos.c` -> **`sq_rig.c`** (lanes, send path, park/reconcile) -> `sq_core.c` (scheduler, the only file that
touches `midi_out_event`; it forward-declares nothing from the rig: `sq_rig.c` comes first and calls `sq_emit_cc`, which `sq_core.c` defines, so `sq_rig.c` carries one forward declaration).
UI: **`ui_rig.c`** (ROW 3 handling and the RIG page) after `ui_sq.c` and the PERF/CHAN page files, before `ui_input.c`'s dispatch tail (the exact slot follows P4-011). Host tests: `#ifndef T1_DEVICE` guards, as ADR-0003 12: the hooks
become no-op stubs in `seq.c` and `ui.c`.

```c
/* the single exit for control changes; defined in sq_core.c (next to sq_emit), called only by sq_rig.c.
   prio: 0 lane (needs >= 24 free ring slots), 1 HAND (>= 12), 2 park / PLAY (>= 8, like NOTE_ON). Returns 1 if queued, 0 if refused (no host, ring short): the caller keeps the wire DIRTY. */
uint8_t sq_emit_cc(uint8_t mch, uint8_t cc, uint8_t val, uint32_t due, uint8_t prio);
/* the step hook: sq_core.c calls it from sq_step_start(c, ...) right after sq_eval, one compare when the rig is off */
void rig_step(uint8_t c, uint8_t p, uint8_t g, uint8_t accent, uint32_t note_due);
void rig_block(void);                         /* once per sq_block, playing or not: drain RigQ, HAND sends, park / DIRTY retries */
void rig_play(void);                          /* sq_on_start, after sq_all_off: mark every SEQ wire DIRTY so step 1 sends even an unchanged value */
void rig_stop(uint8_t why);                   /* sq_on_stop / PANIC / usb-config edge, after the note-offs and CC123: park ACTIVE wires; why = RS_STOP, RS_PANIC */
void rig_bar_swap(const RigSave *next);       /* sq_apply_bar_boundary, before the new image goes live (3.4) */
void rig_midi_ch(uint8_t c, uint8_t oldm);    /* after a MIDI-channel change of channel c (including the displacement rule) */
void rig_mute(uint8_t c, uint8_t muted);      /* on a mute edge */
void rig_usb_edge(uint8_t up);                /* usb.config edge: 0 -> 1 or 1 -> 0 */
```
Main-loop API (`sq_core.c`, the channel state API of P4-005; every setter is byte-atomic or wrapped in `fm1_irq_off()/fm1_irq_on()` and ends by pushing `RQ_SYNC`):
```c
enum { RF_CC, RF_STATE, RF_FORM, RF_LAYER, RF_LEVEL, RF_MOVE, RF_RIDE };
void    sq_rig_set_prof(uint8_t c, uint8_t id);                        /* pick a device: clears the rig, fills the four suggestions in HAND at their neutral, sends nothing */
uint8_t sq_wire_set(uint8_t c, uint8_t i, uint8_t field, uint8_t val);  /* 0 = refused (cap of 8, empty wire, range); RF_LEVEL also pushes RQ_LEVEL (or RQ_PLAYNOW while stopped) */
uint8_t sq_wire_get(uint8_t c, uint8_t i, uint8_t field);
uint8_t sq_rig_nseq(void);                                             /* SEQ wires on the device */
uint8_t sq_rig_row3_ok(uint8_t c);                                     /* prof != 0 and some wire is not DEV: ROW 3 exists */
void    sq_rig_reset(uint8_t c);                                       /* RST R3 */
uint8_t sq_rig_sent(uint8_t c, uint8_t i);                             /* RigRT.last, 0xFF unknown: the sent-value bar */
```

### 3.3 The send path: dedupe, rate, ring

```
rig_send(slot, v, prio, due):
    if (mch == 16 (MIDI OFF) or channel muted and prio == lane) return;            /* nothing to say; wire stays as it is */
    if (v == rt.last && !(rt.flags & DIRTY)) { stats.dedup++; return; }              /* unchanged values are never resent */
    if (!sq_emit_cc(mch, cc, v, due, prio)) { rt.flags |= DIRTY; stats.drop++; return; }   /* retried at the next step / block */
    rt.last = v; rt.cc = cc; rt.mch = mch; rt.tick = (uint16_t)fm1_ms; rt.flags &= ~DIRTY; stats.sent++;
```
`last` is updated only when the packet was queued, so a refused send is retried and never believed sent. The packet is `0x0B | (0xB0 | mch) << 8 | cc << 16 | v << 24` through `midi_out_event_at`.

| Source | When | Rate |
| --- | --- | --- |
| SEQ lane | at each step start, only if the value changed (or the wire is DIRTY) | at most one CC per wire per step |
| HAND | `rig_block` after `RQ_LEVEL`; last value wins | at most one per wire per **10 ms** (`tick` compare); no send at PLAY, at preset load or at power-up |
| Park / PLAY first value | STOP, PANIC, state change to HAND, mute edge, wire removed | once per wire; retried each block until sent |

**CC budget.** Worst case, all at once, with the SEQ cap of 8: step rate = 4 channels at DIV 1/32 and 300 BPM = 40 steps/s per channel, but the cap bounds the total to 8 wires: 8 x 40 = **320 CC/s** only if every wire changes every step (form RAND);
WALK, RAMP and ACC change far less, and dedupe removes repeats. At 120 BPM and 1/16 (the target workflow) it is 8 x 8 = 64 CC/s. A DIN link carries about 1040 three-byte messages/s [V: 31.25 kbit/s, 10 bits per byte], so 320 CC/s is 31 %
of it, and notes (4 channels x two messages x 40 = 320/s worst case) add another 31 %: 62 % plus clock. USB direct to the device has no such limit. HAND: 4 wires x 100/s = 400/s at the very worst, but one person turns one or two knobs.
The 128-entry ring (TIMING_SPEC M5) holds 75 events at 240 BPM in the worst case; 8 CCs on each of those 3 ticks add 24, so 99 of 128; the reserve thresholds 24 / 12 / 8 (lane / HAND / park and NOTE_ON) make CCs the first thing dropped and the park/PLAY CCs the last, never NOTE_OFF, CC123 or clock.

### 3.4 Transport, preset, algorithm, channel and device edges

| Event | Behaviour (all through `RigRT`; "park" = send the wire's LEVEL if the slot still holds the same CC, else the stored `home`) |
| --- | --- |
| **PLAY** (after the count-in, `sq_all_off` first) | `rig_play`: every SEQ wire is marked DIRTY, so step 1 sends LEVEL + lane even when the value equals the last; HAND and DEV send nothing |
| **STOP** (button, external Stop) | after the note-offs and CC123: `rig_stop(RS_STOP)` parks every ACTIVE wire at LEVEL (decision F/100); HAND and DEV untouched; a refused park stays DIRTY and is retried by `rig_block` |
| **PANIC** (`panic_req`) | notes off, CC123, then the same park; **no CC 121 (reset all controllers) and no CC 120**, which would also reset the player's HAND values |
| **Preset load** (lands on a bar, SEQUENCER_UX 4.1) | `rig_bar_swap(next)`: an ACTIVE wire that is still a SEQ wire with the same CC and MIDI channel in `next` keeps going (no park, the new LEVEL and lane apply from the first step of the bar); every other ACTIVE wire is parked at its stored `home`; new SEQ wires are DIRTY; HAND and DEV send nothing. Undo-load uses the same path |
| **Algorithm change** | nothing is sent. The lane of the next step uses the new algorithm's seeds; a wire riding a layer the new algorithm lacks freezes (`RIG_SEED0`), the dice button is dim as ALGORITHM_SET 4.4 says. `ride` follows the morph slot |
| **MIDI-channel change** (CHAN K2, incl. the displacement of another channel to OFF) | `rig_midi_ch`: ACTIVE wires are parked on the **old** MIDI channel (LEVEL), then `mch`/`last` become unknown; the new channel gets the first value at the next step; to OFF: parked, then silent |
| **Mute / unmute** | mute: park (RF8); unmute: SEQ wires DIRTY, resume at the next step; HAND turns on a muted channel still send (the player is steering the sound) |
| **Wire edits (UI)** | `RQ_SYNC` reconciles: SEQ -> HAND parks at LEVEL (the device now rests where the knob says); SEQ -> DEV **sends nothing** (DEV means the FM-1 never sends; the user knows it was handed back); SEQ -> a different PARAM or an emptied wire parks the old CC at its `home`; DEV/HAND -> SEQ marks DIRTY (first value at the next step, or at PLAY) |
| **Device change / NONE** | the old rig is replaced (28 B under `fm1_irq_off`); `RQ_SYNC`: ACTIVE wires with no counterpart in the new rig park at their `home`; the new wires are HAND and send nothing |
| **Cap** | the UI refuses a ninth SEQ wire (toast `8 SEQ WIRES MAX`); the loader downgrades extras; the ISR also counts and treats wires beyond the eighth, in channel then wire order, as HAND, so a bug cannot flood the link |
| **USB disconnect** (`usb.config` 1 -> 0) | `rig_usb_edge(0)`: every `last` becomes unknown; ACTIVE flags stay (a park is owed); nothing can be sent and nothing is queued |
| **USB (re)connect** (0 -> 1) | `sq_usb_edge` already panics (notes off); the park follows (ACTIVE wires rest at LEVEL); while playing, SEQ wires resend at the next step (unknown `last`); HAND does not resend |
| **External clock lost / tempo change** | lanes follow steps; nothing special |
| **Count-in** | no CC until the count-in ends (rig_play runs after it) |

### 3.5 Timing: where the CC sits relative to its note

The CC for step n has the note's stamp as reference: `due_note`. The first **two** CCs of a tick (wire order: channel, wire) are stamped `due_note - 1 ms` and `due_note - 2 ms`; the third and later are stamped after the note,
`due_note + 1 ms`, `+ 2 ms` ... (RF6): a 3-byte message takes 0.96 ms on a DIN link, and the LEAD of 2.90 ms (TIMING_SPEC M2) leaves room for exactly two before the note without moving it. `RIG_PRE_N = 2` and `RIG_GAP_US = 1000` are in the flex
table (a direct-USB target could use 0 and 200). **Requirement on the ring (P4-003/P4-007):** `midi_out_event_at` inserts by a bounded backward scan (at most 16 slots; equal stamps keep insertion order) instead of assuming non-decreasing stamps, because pre- and post-stamped CCs interleave with notes of other channels.

### 3.6 Failure first (what this section protects)

| Failure | Protection |
| --- | --- |
| Device ignores the CC (wrong channel, mode) | MIDI is one way [V]: the screen shows the sent-value bar (what the FM-1 sent), never what the device did; first send after power-on raises the toast `RIG SENDS ONLY: CHECK THE DEVICE`; H-RIG1, H-RIG9 |
| Notes late because of CCs | pre-stamp limited to two; thresholds 24 / 12 / 8; H-RIG6/H-RIG11 against the timing gate |
| Value stuck after STOP, mute, preset, rewire | the park table above; H-RIG7, H-RIG12, H-RIG15 |
| The FM-1 fights the player's hand | HAND sends only on a turn; PLAY, load and power-up send nothing for HAND/DEV; H-RIG8 |
| A lane value out of the device's legal range | `rig_value` clamps to the profile's `lo..hi` |
| A flood from a bug | cap of 8, one CC per wire per step, 10 ms HAND limit, `stats.drop` visible in the timing-test image |

## 4. The UX in detail

All mock-ups are 40 columns of about 6 px, like SEQUENCER_UX 2; the pixel geometry is in the tables and is the binding part. Fonts AF_S except where noted; colours are the theme names of SCREEN_SPEC 1. Every on-screen string was measured with the
real `text_w` (`tests/design/width_probe.sh`) and the strings are now rows of `tests/design/widths.tsv` (the P2-019 block, 74 rows, 0 over).

### 4.1 The owner condition, as a design: three rows, always visible, always labelled

The condition (decision 97): *the screen must make it obvious that there are three knob rows, show the change when the row changes, and always show which row and which page the user is on.* The design has four devices that work together, none of
which depends on colour alone:

1. **The rail.** The band's tag column (36 px) becomes a *rail* of three stacked cells whenever the channel has a rig with a wire that is not DEV. Each cell is the row's own label; the **live** row's cell is inverted, 26 px high, and reads `ROW` over `n/3`
   (`ROW`, `1/3`, `2/3`, `3/3`: so the number three is on the screen at all times); the other two are 13 px cells with just their digit, dim. The ROW 3 cell and its row are drawn in `T_ACCENT` (the "outside the FM-1" colour); rows 1 and 2 in `T_SEL`.
2. **The shape.** The live row is always expanded to 26 px (label over value, as today); the other two rows shrink to 13 px lines of labels. The geometry itself tells the row:

| Live row | ROW 1 y,h | ROW 2 y,h | ROW 3 y,h | Rail cells (y,h) |
| --- | --- | --- | --- | --- |
| 1 | 108, 26 | 134, 13 | 147, 13 | 108,26 live / 134,13 / 147,13 |
| 2 | 108, 13 | 121, 26 | 147, 13 | 108,13 / 121,26 live / 147,13 |
| 3 | 108, 13 | 121, 13 | 134, 26 | 108,13 / 121,13 / 134,26 live |

   The band still tiles Z3 (108..160) exactly. Without a rig (or with every wire DEV) the band is the locked two-row band of SCREEN_SPEC 2.4 and the rail is the two-label tag column: **nothing changes for a plain sequencer**.
3. **The change.** On every row change: (a) the new live row's cells flash `T_ACCENT` for 9 frames (150 ms), then settle to `T_SEL`; (b) a 2 s toast names the row: `ROW 1 OF 3: PLAY MORPHS`, `ROW 2 OF 3: DIAL-IN MORPHS`, `ROW 3 OF 3: TB-3 KNOBS`; (c) the rail and geometry as above;
   (d) the cue strip then holds the standing row cue (4.6). This works with the redraw rule "redraw when a signature changes": the signature gains `knob_row`, `row_flash` and `rig_chip[]`.
4. **The page.** Where am I (always one line of the screen answers it):

| Screen | Row | Page |
| --- | --- | --- |
| PERF, no rig | rail `1` / `2` (as locked) | lane header `PG 1/2  FOLLOW  hold=EDIT` |
| PERF, rig | rail `ROW n/3` + geometry + toast | lane header `PG 1/2  FOLLOW  hold=EDIT  h:CHN=RIG  >P2` (the CHN hold named where it starts, R-8; 238 of 240 px) |
| RIG page | no rows (single-row page: tag column `RIDE-` / `RIDE+`) | banner `RIG CH1  TB-3` in `T_ACCENT` on `T_TINT` plus `WIRE n/4` at the right; the lane is replaced, so the PERF lane header is gone |
| a channel without a rig, ROW 3 latched | falls back to ROW 1 with the toast `ROW 3 NEEDS A WIRE: HOLD CHN` | as PERF |

   The ride dot (3x3 px, `T_ACCENT`, top-right corner) marks every dice cell whose layer is ridden by a wire (RF3) and every morph cell that a wire's RIDE binds; it never changes a cell's width.

**ROW 3 cells.** Live (26 px, 51 wide): line 1 the wire's name (the device's own word, at most 5 letters; AF_S, `T_MID`), line 2 `64 ~` (LEVEL then a chip glyph: `~` SEQ, `H` HAND, `-` DEV; `T_THEME`, `T_ACCENT` while the knob is hot), and the bottom 2 px
a **sent-value bar** (`rt.last` x 49 / 127, `T_INK` on `T_SEL`) that moves with the lane: the screen shows what the FM-1 *sent*. Compact (13 px): chip, space, name (`~ CUT`, `H RESO`, `- ENV`): at most 49 px (`H DECAY` is the longest allowed).
An empty wire shows `-`. A DEV wire's knob does nothing and raises the toast `CUT IS DEV: HOLD CHN TO TAKE IT` (185 px).

### 4.2 PERF screens with a rig

```
ROW 1 live (rig on CH1; ROW 3 is the device)            ROW 3 live (HAND turn in progress)
+----------------------------------------+              +----------------------------------------+
|> 120 P01  1 ACID M2  [1]           L16 |              |> 120 P01  1 ACID M2  [1]           L16 |
|[1 ACID ] 2 EUCL m  3 SNAK m  4 STAG m  |              |[1 ACID ] 2 EUCL m  3 SNAK m  4 STAG m  |
| PG 1/1  FOLLOW  hold=EDIT  h:CHN=RIG   |              | PG 1/1  FOLLOW  hold=EDIT  h:CHN=RIG   |
| #  .  #' #  .  #! .  #~ #  .  #  .  #  |              | #  .  #' #  .  #! .  #~ #  .  #  .  #  |
|ROW |DENS 12 |SLIDE 4 |JUMP 5 |ACCNT 3 |              |  1 |DENS    |SLIDE   |JUMP    |ACCNT   |
|1/3 |        |        |        |        |              |  2 |OFFSET  |PEDAL   |PUNCH   |DIR     |
|  2 |OFFSET  |PEDAL   |PUNCH   |DIR     |              |ROW |CUT     |RESO    |ENV-X   |ENV-Y   |
|  3 |~ CUT   |H RESO  |H ENV-X |H ENV-Y |              |3/3 | 74 ~   | 43 H   | 64 H   | 64 H   |
| ALL  NOTES ACCNT SHAPE UNDO  SET       |              | ALL  NOTES ACCNT SHAPE UNDO  SET       |
|  ... buttons as PERF ...               |              |  ... buttons as PERF ...               |
| M1 M2 M3|M4 CHN|SOL FIL ROL|PG- PG+|RST|              | M1 M2 M3|M4 CHN|SOL FIL ROL|PG- PG+|RST|
| OCT+ TWICE = ROW 3: TB-3 KNOBS         |              | ROW 3 OF 3: ~=SEQ  H=HAND  -=DEV       |
+----------------------------------------+              +----------------------------------------+
```
Channel cell 1 reads `1 ACID` over `MIDI 2 ~`. The ride dot is a graphic and is not drawn in these text mock-ups: with a CUT wire riding ACCNT, the dice cell `ACCNT` (ENV) carries it. The live row is filled; the rail is dim except the live cell.

ROW 2 live: rail `1` / `ROW 2/3` / `3` (the third in accent), ROW 2 expanded, ROW 1 and ROW 3 compact; cue `ROW 2 OF 3: OCT- = ROW 1  OCT+ = ROW 3`. Row-change frame: ROW 3 just entered = the same as the right-hand mock-up, with the row's cells `T_ACCENT`-filled for 150 ms and the toast `ROW 3 OF 3: TB-3 KNOBS` in the strip.

States of the ROW 3 band:

| State | What the band shows | Cue / toast |
| --- | --- | --- |
| no device, or all wires DEV | the locked two-row band; rail = tag column | none |
| device with a non-DEV wire, ROW 3 never visited | three rows; rail with `3` in the accent colour | cue 13a `OCT+ TWICE = ROW 3: TB-3 KNOBS` until the first visit (`PREF2_RIG_ROW3SEEN`) |
| ROW 3 live, playing | live cells with the moving sent-value bar | cue 15a |
| ROW 3 live, channel MIDI OFF | cells and bars drawn dim; chips unchanged | cue `RIG: MIDI OFF, NOTHING IS SENT` (HAND turns do not send either) |
| ROW 3 live, channel muted | as normal; HAND turns send, SEQ is parked | none |
| a knob on a DEV wire turned | value does not change | toast `CUT IS DEV: HOLD CHN TO TAKE IT` |
| ROW 3 latched, user selects a channel with no rig | falls back to ROW 1 | toast `ROW 3 NEEDS A WIRE: HOLD CHN` |
| OCT+ on ROW 3 | stays | toast `LAST ROW: OCT- = ROW 1` |
| OCT+ on ROW 2 with a rig that is all DEV | stays on ROW 2 | toast `ROW 3 NEEDS A WIRE: HOLD CHN` |

**OCT- / OCT+.** `OCT-` tap always latches ROW 1 (the way home, unchanged from the lock); `OCT+` tap steps 1 -> 2 -> 3 and is inert on the last row; holding OCT- / OCT+ and turning peeks ROW 1 / ROW 2 as locked; ROW 3 has no peek. Going from ROW 3 to ROW 2 is OCT- then OCT+ (owner question A).

### 4.3 The RIG page (hold CHN 0.4 s from PERF, NOTE_EDIT, CHAN or SET; latched)

Geometry (zones of SCREEN_SPEC 2; Z2 and Z3 change, the rest is PERF's):

| Zone | y,h | Content on the RIG page |
| --- | --- | --- |
| Z0 HEAD, Z1 CHANNELS | 0..48 | unchanged; the cell of the selected channel is the rig being edited; line 2 of any cell with a rig carries a chip: `MIDI 2 ~` (some SEQ), `MIDI 2 H` (HAND only); a muted cell drops the chip |
| Z2 (replaces the lane) | 48,14 banner; 62..108 four wire rows of 12, 12, 11, 11 px | banner: `RIG CH1  TB-3 BITE` (device and set; `TB-3 CUSTOM` once the four wires no longer match a set) left in `T_ACCENT` on `T_TINT`, `WIRE 1/4` right |
| Z3 BAND (single-row page) | 108,52 | tag column `RIDE-` / `RIDE+`; K1..K4 cells `PARAM`, `LAYER`, `FORM`, `MOVE` with the selected wire's values; two status lines (204 px) |
| Z4 buttons, Z5 legend, Z6 cue | as PERF | dice buttons keep their roles and labels (you hear the lanes change while setting up); the legend shows `BACK` on D#4 |

Wire row columns (AF_S, x of the left edge, widths binding): key digit `1` at x=2 (7); name at x=14 (at most 40); LEVEL right-aligned to x=79 (21); who drives it at x=84 (`DEV` 22, `SEQ~` ~29, `HAND` 32; 34 column); the dice layer at x=120 (`ACCNT` 39; `-` for none; 42 column);
the lane form at x=164 (`RAND` `WALK` `RAMP` `ACC`; 34 column); MOVE as `M48` at x=200 (`M127` 31). The selected wire is inverted (`T_SEL`/`T_INK`); DEV rows dim; SEQ rows have the state word in `T_ACCENT`.

```
+----------------------------------------+   TB-3 BITE chosen, wire 1 selected (P2-020 names, P2-021 banner)
|> 120 P01  1 ACID M2  [1]           L16 |
|[1 ACID ] 2 EUCL m  3 SNAK m  4 STAG m  |   cell 1 line 2: MIDI 2 ~
| RIG CH1  TB-3 BITE             WIRE 1/4|   banner (accent on tint)
|>1 CUT   64 SEQ~ ACCNT ACC   M48        |   selected row inverted
| 2 RESO  43 HAND ARTIC WALK  M24        |
| 3 ENV-X 64 HAND SHAPE RAMP  M32        |
| 4 ENV-Y 64 HAND -     RAND  M0         |
|RIDE-|PARAM CUT|LAYER ACCNT|FORM ACC|MOVE 48|
|RIDE+|CC74  LEVEL 64  MIDI 2                |
|     |RIDE -  OCT- / OCT+                   |
| ALL  NOTES ACCNT SHAPE UNDO  SET       |
|  ... buttons as PERF ...               |
| M1 M2 M3|M4 BACK|SOL FIL ROL|PG- PG+|RST|
| RIG: TAP 1-4 = WHO DRIVES  HOLD = PICK |
+----------------------------------------+
```

| RIG state | Screen | Cue |
| --- | --- | --- |
| **NONE** (no device; every channel at power-on and every old preset) | banner `RIG CH1  NO DEVICE`; line `TURN ALGO TO PICK A DEVICE`; the device list `[NONE] GENERIC TB-3` (current in brackets, horizontal like the algorithm picker); band cells `PARAM -` `LAYER -` `FORM -` `MOVE -` dim | row 4a `RIG: TURN ALGO TO PICK THE DEVICE` |
| **GENERIC** | banner `RIG CH1  GENERIC`; four empty wires `1 -`; the key tap on an empty wire raises `PICK A PARAM FIRST: K1`; K1 shows `CC74` style raw numbers | 4b |
| **device** | as the mock-up | 4b |
| **MIDI OFF on the channel** | the wires are shown as they are; status line 1 reads `MIDI OFF: THE RIG IS SILENT`; the banner is drawn dim | row 4c `RIG: MIDI OFF, NOTHING IS SENT` |
| **cap reached** | the SEQ tap that would be the ninth is refused | toast `8 SEQ WIRES MAX` |
| **ALGO turned** | device list in the cue for 2 s (current in brackets), the wire list refills (4.4) | `NONE GENERIC [TB-3]` |
| **HOME held** | HELP (4.7) | row 1 |

K1 PARAM walks the device's list (`hw_next_cc`); changing a parameter on a SEQ wire parks the old CC at its neutral first and restarts LEVEL at the new neutral. K2 LAYER: `-`, NOTES, HITS, ACCNT, ARTIC, SHAPE; layers the algorithm lacks are shown dim.
K3 FORM: RAND WALK RAMP ACC. K4 MOVE 0..127 (step 1, fast 8). OCT-/OCT+ = RIDE -/+ over the eight morph slots; status line 2 names the morph (`RIDE PUNCH`). LEVEL is not on this page: it is ROW 3 (so the knob that does the live work lives where it is performed); the status line shows it.

### 4.4 Setting up, and choosing who is in control (at most 2 gestures)

| Task | Gestures | How |
| --- | ---: | --- |
| Pick the device | 2 | hold CHN (RIG opens), turn ALGO |
| Make a wire SEQ / take it back to DEV | 2 | hold CHN, tap its key 1-4 (the key cycles DEV -> HAND -> SEQ -> DEV and selects the wire) |
| Pick the wire's parameter / layer / form / MOVE | 2 | hold CHN, tap its key to select it (or hold the key to select without changing the state), turn K1 / K2 / K3 / K4 |
| Turn a device knob while playing | 1 once ROW 3 is latched | turn K1..K4 on ROW 3; entering ROW 3 is `OCT+` twice (a latched mode, like ROW 2) |
| See who is in control | 0 | the chips on ROW 3 and the state words on the RIG page, the `~`/`H` on the channel cell |

The three states, in the words on the screen: `DEV` "the device knob, the FM-1 never sends it" (toast `DEV: THE DEVICE KNOB, NOT SENT`), `HAND` "your FM-1 knob is a remote for the device's knob; sent when you turn it" (`HAND: YOUR KNOB, SENT WHEN TURNED`),
`SEQ` "the sequencer plays this knob; you steer it with LEVEL, MOVE and the dice" (`SEQ: MOVES WITH THE MUSIC`). **Defaults (decision 100):** every wire starts DEV; choosing a device fills its four suggested wires in HAND at their neutral and sends nothing;
nothing becomes SEQ without a tap; a preset carries all of it; the plain sequencer with no device is exactly as locked. SEQ -> HAND parks the device at LEVEL; SEQ -> DEV sends nothing.

### 4.5 First use: no manual (VISION rule 1)

| Moment | Guidance |
| --- | --- |
| The CHN hold exists | named in the PERF lane header `h:CHN=RIG` from the first boot (R-8), and in the HELP text of the key |
| First device ever picked (`PREF2_RIG_INTRO`) | toast `TB-3: 4 KNOBS ON ROW 3`, then cue 13a `OCT+ TWICE = ROW 3: TB-3 KNOBS` on PERF until ROW 3 is visited once |
| First visit to ROW 3 | the transition cue (4.1) and the standing cue `ROW 3 OF 3: ~=SEQ  H=HAND  -=DEV` |
| First wire put on SEQ | toast `SEQ: MOVES WITH THE MUSIC` (every state change has its toast) |
| First CC sent after power-up | toast `RIG SENDS ONLY: CHECK THE DEVICE` (one way, [V]) |
| Dice that moved a wire | toast `ACCNT + CUT` (P6-002), so the player learns that the dice reach the device |
| Nothing happens on the device | H-RIG1 and H-RIG9 tell the team what to put in the guide: the device must listen on the channel and accept CC |

### 4.6 Cue strip and toast rows added (SEQUENCER_UX 11.2 / SCREEN_SPEC 2.7; all measured)

| Priority | State | Text |
| --- | --- | --- |
| 4a | RIG page, no device | `RIG: TURN ALGO TO PICK THE DEVICE` |
| 4b | RIG page, a device | `RIG: TAP 1-4 = WHO DRIVES  HOLD = PICK` |
| 4c | RIG page, MIDI OFF | `RIG: MIDI OFF, NOTHING IS SENT` |
| 13a | PERF stopped, a rig, ROW 3 never visited | `OCT+ TWICE = ROW 3: TB-3 KNOBS` |
| 15 (extended) | playing, ROW 2, a rig | `ROW 2 OF 3: OCT- = ROW 1  OCT+ = ROW 3` (without a rig: the locked `ROW 2: OCT- = back to row 1`) |
| 15a | ROW 3 | `ROW 3 OF 3: ~=SEQ  H=HAND  -=DEV` |
| toasts | row change | `ROW 1 OF 3: PLAY MORPHS`, `ROW 2 OF 3: DIAL-IN MORPHS`, `ROW 3 OF 3: TB-3 KNOBS` (the device name replaces TB-3, at most 8 characters; GENERIC fits) |
| toasts | rig | `ROW 3 NEEDS A WIRE: HOLD CHN`, `LAST ROW: OCT- = ROW 1`, `8 SEQ WIRES MAX`, `SEQ WIRES CAPPED AT 8`, `PICK A PARAM FIRST: K1`, `CUT IS DEV: HOLD CHN TO TAKE IT`, the three state toasts, `RIG SENDS ONLY: CHECK THE DEVICE`, `TB-3: 4 KNOBS ON ROW 3` |

### 4.7 Help (HOME held)

`help_text.tsv` gained a RIG page (49 rows, generated by `tests/design/gen_help.py`, two lines of at most 34 characters, 224 px) and the PERF rows for OCT-, OCT+, K1..K4 and CHN were amended: `OCT+: next knob row (2, then 3)` /
`hold and turn: peek at ROW 2`; `OCT-: back to knob ROW 1`; `K1: {r1} (ROW 1)` / `OCT+: {r2}; twice: device`; `CHN: tap = settings, hold = RIG` / `LENGTH MIDI | the device knobs`. The RIG rows include `K1: PARAM this wire drives`, `K2: LAYER, the dice that re-rolls`,
`K3: FORM of the moving value`, `K4: MOVE, how far SEQ goes` / `0 = still; HAND ignores it`, `Key F3: wire 1, who drives` / `DEV > HAND > SEQ; hold = select`, `ALGO: pick the device`.

### 4.8 L-scores (SEQUENCER_UX 8.1 rubric; the designer's scores, not machine-checked)

| Function | Class | Gestures | V | I | O | L | Total | Verdict |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| R1 pick the device | S | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R2 who is in control of a wire | S | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R3 pick a parameter, layer, form, MOVE | S | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R4 move a device knob live (ROW 3 latched) | P | 1 | 3 | 2 | 3 | 3 | 11 | pass |
| R4b reach ROW 3 from ROW 1 (OCT+ twice) and turn | S | 2 (+1 turn) | 3 | 2 | 3 | 2 | 10 | pass as a latch like ROW 2 (owner question A) |
| R5 dice the lanes | P | 1 | 3 | 3 | 3 | 2 | 11 | pass |
| R6 see who is in control | P | 0 | 3 | 3 | 3 | 2 | 11 | pass |
| R7 know which row and page I am on | P | 0 | 3 | 3 | 3 | 3 | 12 | pass (rail, geometry, banner, toast) |

## 5. Table rows and scenarios (added and checked in this task)

The RIG page is the tenth page, so the full matrices grew: `page_states.tsv` +1 row (RIG, with MOVE_HELD's overlay list extended); `control_map.tsv` +49 rows (one per control, context `RIG`); `button_modes.tsv` +28 (14 buttons x tap/hold);
`knob_map.tsv` +11 (RIG K1..K4, SELECT, ALGO, PRESET; PERF `R1`..`R4` = the LEVEL knobs of ROW 3, labels `WIRE1`..`WIRE4`); `key_map.tsv` +32 (27 keys on RIG, the four wire keys with a hold row, and the PERF hold row of D#4); `help_text.tsv` +49 (generated).
Existing PERF rows amended in place (the locked text is kept; ROW 3 is appended): `B_OCTUP`, `B_OCTDN`, `EN_K1..K4`, `KEY_B05_D#4` (hold = RIG).

| Table row (control / context) | Role on the RIG page (full text in the TSV) |
| --- | --- |
| `KEY_B05_D#4 / PERF` hold | opens the RIG page, latched (tap is still CHAN) |
| `KEY_B05_D#4 / RIG` | BACK to PERF |
| `KEY_W01..W04 / RIG` | tap = cycle DEV > HAND > SEQ and select; hold 0.4 s = select only |
| `KEY_W05..W16 / RIG` | inert |
| `EN_K1..K4 / RIG` | PARAM, LAYER, FORM, MOVE |
| `EN_ALGO / RIG` | the device (NONE, GENERIC, TB-3) |
| `B_OCTDN`, `B_OCTUP / RIG` | RIDE - / + |
| dice, EDIT, SAVE, PLAY, REC, GLO, HOME, PRESET, mute / SOLO / FILL / ROLL / RST keys | as PERF (GLO, REC, HOME leave the page) |
| `B_OCTUP`, `B_OCTDN`, `EN_K1..K4 / PERF` | ROW 3 appended (3-row stepping, the LEVEL of wire n) |

Scenarios: **S21** (`scenarios/S21-rig-device-knobs.md`, 14 steps, hand-written: pick a device, SEQ one wire, hand-turn another, dice, undo, stop parks, take a wire back) and **S22** (`S22-sweep-rig.md`, generated by `gen_sweeps.py`: every control on the RIG page once). `check_scenarios.py`
now lists RIG in its matrix and treats S22 as a sweep; `check_help.py` and `gen_help.py` know the RIG page; `gen_sweeps.py` writes S22. The S21 device and numbers are placeholders until P2-020. Results:
`check_scenarios.py` 22 files, 490 control-map rows, 0 gaps; `check_help.py` OK 392 help rows, 2695 strings; `tools/check_sequencer_ux.py` OK (10 pages, 490 control rows).

## 6. Implementation plan

### 6.1 New tasks (proposals only; ids final when created; none exists yet)

| Id | Task | Executor | Depends on | Where it hooks |
| --- | --- | --- | --- | --- |
| P4-023 | `sq_rig.h`: Wire, Rig, RigSave, RigRT, RigQ, HwProf/Param/Prov, enums, all static asserts; preset v4 and the v3 loader | haiku | P4-001 | `sq_types.h` stays untouched; `SeqPreset` gains `RigSave` after `HarmonySave` |
| P4-024 | `tools/gen_hwprof.py` + `tools/hwprof/` format, GENERIC profile, the checks of 2.3 (provenance, names via the width probe, no CSV input), `sq_hwprof.c` lookups, host test | sonnet | P4-023, P4-010 | writes `build/gen/hwprof_tab.h`; TB-3 data arrives with P2-020 |
| P4-025 | `sq_rig.c` pure part: `rig_mix`, `rig_lane`, `rig_value`, `rig_layer_seed`; `tests/rig_golden.txt`; T-RIG1..T-RIG4 against the unchanged core goldens | sonnet | P4-006, P4-023 | reads seeds through the dice accessor of P4-006 |
| P4-026 | `rig_step/rig_block/rig_play/rig_stop/rig_bar_swap/rig_midi_ch/rig_mute/rig_usb_edge`, `sq_emit_cc`, the 24 / 12 / 8 thresholds, event-level tests with a host ring mock (dedupe, park, retry, cap, ordering) | sonnet | P4-004, P4-025 | call sites in `sq_step_start`, `sq_on_start/stop`, `sq_apply_bar_boundary`, `sq_usb_edge`, the mute and CHAN-K2 setters |
| P4-027 | main-loop API (`sq_rig_set_prof`, `sq_wire_set`, `RigQ`, `sq_rig_reset`), cap, loader clamping, preset round trip, undo non-interaction test | sonnet | P4-005, P4-026 | `sq_core.c` channel API |
| P4-028 | ring: sorted insert (16-slot backward scan), CC thresholds in `midi_out_free()` use, drop counters | sonnet | P4-003, P4-007 | `usb.c`, `midi_out_event_at` |
| P4-029 | UI framework: band with 2 or 3 rows from table data (row geometry, rail, flash, ride dot, signature fields), Z1 chip | sonnet | P4-011 | `ui_sq.c` zones; must land before P6-001 so ROW 1/2 are drawn by the same code |
| P4-030 | `ui_rig.c`: the RIG page, the CHN hold route, wire keys, K1..K4, device list; first-use bits in `ui_prefs2`; `gen_ui_tables.py` reads the RIG rows | sonnet | P4-015, P4-027, P4-029 | `ui_input.c` dispatch; the page tables of P4-010 |
| P4-031 | cue rows 4a-4c, 13a, 15, 15a, the toasts, HELP rows; `RST R3` | haiku | P4-018, P4-030 | strings from the TSVs |
| P6-009 | ROW 3 live: the OCT+ stepping, LEVEL knobs, HAND/SEQ behaviour while stopped or playing, DEV toast, MIDI OFF/mute behaviour | sonnet | P6-001, P4-030 | the knob router of P6-001 |
| P6-010 | dice integration: ride dot on dice cells, toast `ACCNT + CUT`, RESET + dice resets the lane seeds | haiku | P6-002, P4-027 | no new seed |
| P5-010 | HW-2b: the H-RIG sheet on the real TB-3 (H-RIG1..H-RIG5, H-RIG11) with the P2-020 profile | sonnet (owner runs the hardware) | P5-009, P4-031, P2-020 | the human gate |
| P7-005 | preset flash map for 1508-byte slots; the pending-load park rule | haiku | P7-001 | `SQ_PRESET_BYTES` |
| P8-005 | user guide chapter "Your gear" + the "Hardware belongs to its owners" text; MENU > INFO `HW DATA` rows | haiku | P4-024 | docs and one menu row |

### 6.2 Hooks in existing tasks (reserve now, so nothing is reworked)

| Existing task | What to take into account (a note for the task owner; this task edits none of them) |
| --- | --- |
| P4-001 | nothing: `sq_rig.h` is separate; the asserts on `ChanSave` and `UndoRec` stay |
| P4-003 | `midi_out_free()` returns the free count so `sq_emit_cc` can apply 24 / 12 / 8; the out ring counts CC drops separately |
| P4-004 | `sq_step_start` calls `rig_step` after `sq_eval`; `sq_on_start/stop`, `sq_all_off` call `rig_play/rig_stop`; one compare when `prof == 0` |
| P4-005 | the channel API owns `RigSave`; the preset v4 layout; displacement and mute setters call `rig_midi_ch/rig_mute` |
| P4-006 | expose `rig_layer_seed` data (the dice's layer table) and `sq_morph_norm(ch, slot)`; no copy of the seed table |
| P4-007 | stamped release ring accepts bounded out-of-order insertion (3.5) |
| P4-010 / P4-011 | the table compiler reads the RIG rows and `knob_row`; the framework draws a 2- or 3-row band and a rail; widths from `widths.tsv` |
| P4-013 / P4-015 | the PERF lane header gets `h:CHN=RIG`; the legend `CHN` hold route |
| P4-018 | cue rows and toasts of 4.6; `RST R3` |
| P6-001 / P6-002 | the knob router and the dice toast know ROW 3 and the wire names |
| P7-001 / P7-002 | slot size from `sizeof(SeqPreset)` (1508 B) |

### 6.3 Hardware tests (add to HARDWARE_TEST.md via P4-020; H-RIG1..H-RIG10 are HL 10)

| # | Check | Pass |
| --- | --- | --- |
| H-RIG1 | a CC from the FM-1 via the host box reaches the TB-3 on its channel and the named knob moves | each CC of the P2-020 profile moves audibly; the others are struck |
| H-RIG2 | the sent-value bar and the number on screen equal what the device shows | no drift |
| H-RIG3 | accent: velocity threshold versus CC (decision 95, H0) | profile `acc_*` filled |
| H-RIG4 | slide by overlapping notes; CC 102/103 do not affect incoming notes | profile `slide_*` filled |
| H-RIG5 | zipper noise: HAND detent 1 and 4 at 10 ms | smooth enough, else widen the limit |
| H-RIG6 | 8 SEQ wires on 4 channels at 1/32 against `tools/fm1_timing.py` through the host box and DIN | note jitter inside the timing gates G1..G7 |
| H-RIG7 | STOP parks at LEVEL | no wire rests on a lane value |
| H-RIG8 | the player turns the device knob, then PLAY: HAND does not overwrite it, SEQ does | as stated |
| H-RIG9 | the device's own receive channel and global settings | note in the profile |
| H-RIG10 | the ring never starves notes under the worst CC load | note drops 0 |
| H-RIG11 | the 2-before / rest-after CC stamping (3.5): note-on timing with 8 SEQ wires through DIN versus all-before | notes inside G1..G7; pick `RIG_PRE_N` |
| H-RIG12 | a preset load removes a SEQ wire: it parks at `home`; a kept wire does not park | as stated |
| H-RIG13 | unplug and replug USB while playing: SEQ resends, HAND does not, no stuck note | as stated |
| H-RIG14 | change the channel's MIDI channel while a wire runs: old channel parked, new channel gets values | as stated |
| H-RIG15 | mute and unmute a channel with a SEQ wire | parked on mute, resumes on unmute |
| H-RIG16 | readability (human, a person who has not seen the design): within 2 s of a row change they say which row is live and that there are three | 5 of 5 |
| H-RIG17 | first use without the guide: pick the device and make the cutoff move | under 60 s, 5 of 5 |
| H-RIG18 | save a rig in a preset, power-cycle, load; load an old v3 preset | rig restored; v3 loads with no rig |
| H-RIG19 | 30 minutes of 8 SEQ wires with dice and morph | `drop` counter 0 and no hang |

## 7. Risks and owner questions

### 7.1 Risks

| Risk | Effect | Mitigation |
| --- | --- | --- |
| DIN link saturation (3.3) | late notes at high step rates with RAND forms | cap 8, dedupe, two-before pre-stamp, H-RIG6/11; the cap can drop to 4 as a constant |
| Pool before R6 (1.6) | the preset set does not fit | R6 first, or `SQ_NSLOT` = 8 (one constant) |
| Compact rows lose the dim ROW 2 values when a rig exists | the player cannot read ROW 2 values while on ROW 1 or 3 | owner question C |
| The device ignores CC (MIDI one way) | the player thinks the rig is broken | sent-value bar, the one-way toast, H-RIG9 |
| 38 px name budget | profile names are terse (`CUT`, `ENV`) | enforced in the generator; names are the device's own lingo |
| Two `ui_prefs2` bits may not be free | the bits are needed | P4-031 checks first; fall back to one bit |
| Unknown profile id from newer firmware | a preset loads with an unnamed rig | falls back to GENERIC, `CCnn` names, CCs kept |
| Lane noise on `WALK`/`RAND` during a held note | zipper noise on a sustained note | forms are the user's choice; MOVE 0 silences; H-RIG5 |

### 7.2 Owner questions (lettered options and a recommendation; none changes a locked decision)

| # | Question | Options | Recommendation |
| --- | --- | --- | --- |
| A | Reaching ROW 3 from ROW 1 costs two OCT+ taps (OCT- stays "home = ROW 1" as locked) | A) keep: OCT+ twice; B) when a rig exists OCT- from ROW 1 wraps to ROW 3; C) hold OCT+ 0.4 s without turning jumps to ROW 3 | **A** (nothing the owner locked changes; the standing cue teaches it); revisit after H-RIG17 |
| B | Is MOVE a performance control? Today it is on the RIG page only (K4); ROW 3 knobs are LEVEL | A) RIG page only, plus RIDE so a morph knob can deepen it; B) a second ROW 3 layer on OCT- hold | **A** |
| C | With a rig, the non-live knob rows shrink to a 13 px line of labels (values only on the live row). Accept? | A) accept (rail + geometry make the row unmistakable); B) three 17 px rows, label + value in 8 px type; C) keep ROW 2 at 26 px and let ROW 3 replace ROW 1's space | **A** |
| D | Leaving SEQ for DEV sends nothing (the device keeps the last lane value) | A) nothing (DEV means the FM-1 never sends); B) park at LEVEL first | **A** |
| E | Profile parameter names at most 5 letters and 38 px | A) accept; B) allow 6 letters and drop the chip from compact cells | **A** |
| F | The ride dot replaces `~` after dice sub-labels and morph names (RF3) | A) dot; B) widen the button cells and drop the hold marks | **A** |
| G | Muting a channel parks its SEQ wires (RF8) | A) yes; B) leave wires where they are | **A** |
| H | Wire keys: tap cycles, hold selects (2 gestures for every setup) | A) as designed; B) tap selects, a second tap cycles (3 gestures for wires 2-4) | **A** |

## 8. Checks run for this document

- `python3 tests/design/check_scenarios.py`: 22 files, 490 control-map rows, 0 with no scenario. `python3 tests/design/check_help.py`: OK, 392 help rows, 2695 strings measured. `python3 tools/check_sequencer_ux.py`: OK, 10 pages, 490 control rows, 79 scored functions.
- `sh tests/design/width_probe.sh`: the 74 strings of the P2-019 block of `tests/design/widths.tsv` (rail, ROW 3 cells, banners, wire columns, status lines, cues and toasts) are within their limits; the file as a whole: 236 strings, 0 over.
  Measured facts that shaped the design: `ROW 3` alone is 36 px of a 36 px rail (hence `ROW` over `3/3`); `H ENVMD` is 52 px against 49 (hence names of at most 38 px); `ACCNT~` costs 7 px (hence the ride dot); `CHN` is 25 px in a 20 px legend cell
  (hence the hold is named in the lane header); the cue `ROW 3 OF 3 GENERIC: ...` is 246 px (hence no device name in the cue).
- `python3 tools/backlog.py check`: see the P2-019 Log.
- No firmware, no `tools/check.sh`, no build (the build directory is in use by another task).

## 9. P2-021: profile v2 (parameter kinds, linked parameters, wire sets) and the alignment of 11 parameters onto 4 wires

Task P2-021 (owner decision 106). Status: **for owner review** (gate `human-review`). Design and data only: no firmware. Nothing in sections 1-8 that the owner locked (decisions 96-101) is changed; every change below is a refinement marked RF9..RF13 or an owner question (9.9).
Section 2.1 (records), 2.4 (source file), 2.5 (lookups), 1.6 (flash) and 1.8 (asserts) above were edited in place to the v2 layout; this section says why and adds what they refer to. The TB-3 data is `tools/hwprof/tb3.tsv`; the TB-3 facts and the family maps are in [TB3_PROFILE.md](TB3_PROFILE.md).

**Why.** Decision 106: all 11 TB-3 CCs move their parameter by ear. The P2-020 profile had 7 entries and 4 suggested wires. Eleven parameters have to share four wires (decision 100), one of them (scatter type, CC 68) is a **mode select** and not a knob, and it is **linked** to another (scatter depth, CC 69). The owner's emphasis: *the key is aligning them into the sequencer so that we can control the ones we need and want.*
(Count note: 7 of the 11 were already in the P2-020 table from the vendor chart; they are upgraded from [I] to [V-ear]. The **new** entries are 4: CC 1, 11, 68, 69.)

### 9.1 Refinements

| # | v1 said | v2 says | Why |
| --- | --- | --- | --- |
| RF9 | the profile names four suggested wires, each with its layer, form and MOVE (`sug[4]`, `sug_cfg[4]`, `sug_move[4]`, HwProf 192 B) | the **lane lives on the parameter** (`HwParam.dcfg`, `dmove`: the lane it gets when any wire takes it) and the profile carries up to six named **wire sets** (`HwSet`: a 5-letter name and four parameter indexes); a freshly picked device starts with set 0 | with 11 parameters the old shape cannot say what each one does when it is swapped onto a wire; a set is then only four indexes |
| RF10 | `HwParam` is `cc lo hi home nm` (10 B) | `kind` (CONT or SEL with 2..16 zones), `link` (partner, GATED, RESEND, DUCK), `dcfg`, `dmove` (14 B) | scatter type is a select, scatter depth belongs to it |
| RF11 | FORM RAND / WALK / RAMP / ACC for every wire | for a **SEL** parameter the FORM field reads DICE (0) or BAR (1); 2 and 3 load as DICE (the loader rule of 1.1 already maps invalid fields) | a select must not sweep through modes (9.2); the `Wire` struct is unchanged |
| RF12 | `RigRT.rsv` is spare | `RigRT.rsv` holds the time of the last HAND turn in 16 ms units (`turn16`) for the SEL settle | `sizeof(RigRT)` stays 8, so the assert of 1.8 stays |
| RF13 | `sq_rig_set_prof(c, id)` fills the four suggestions | `sq_rig_set_prof(c, id, set)`; a new hook `rig_bar(bar)` latches every SEL wire at each 768-tick bar line (9.3) | sets; SEL changes only on a boundary |

Budget of v2: `HwParam` +4 B x 16 x 8 profiles = 512 B, `HwSet` 60 B x 8 = 480 B, minus the 12 B x 8 of `sug*` = **+992 B flash** (3136 B in all; 1.6). .bss: **0 B** (RF12 reuses a byte; the `sets` are const). CPU: a SEL wire is evaluated once per bar, not per step: cheaper than any CONT wire. The static asserts are in 1.8 (edited).
`HwParam.nm`, set names: at most 5 letters, 38 px (AF_S) through `width_probe.sh`: PAD-Y 33, PAD-Z 34, SCTYP 37, SCDEP 37, BITE 24, WET 25, PADS 29, PUSH 31 (rows of `tests/design/widths.tsv`).

### 9.2 Parameter kinds: CONT and SEL

| | CONT (continuous) | SEL (mode select, n equal zones over `lo..hi`) |
| --- | --- | --- |
| Example | CUT, RESO, SCDEP | SCTYP (CC 68) |
| What a knob turn does (HAND) | LEVEL moves one step per detent (fast 8), a CC is sent at once, at most one per 10 ms | LEVEL moves **one zone per detent**, no acceleration; the **cell shows the target zone at once** (`#3`), the CC is sent **150 ms after the last detent** (`SEL_SETTLE_MS`, [I] to be tuned in H-RIG20), so spinning from `#1` to `#6` sends one CC, the CC of `#6`, and no mode in between is ever selected |
| What is sent | any 0..127 | **the CC at the centre of the zone**: `hw_zone_cc(p, z) = lo + (2z - 1) * (hi - lo + 1) / (2n)` (n = 8: 8, 24 ... 120), never an edge value, so an error in the device's zone edges costs nothing until it is larger than half a zone |
| LEVEL stores | the CC | the centre CC of the base zone (the `Wire` byte is unchanged; `hw_cc_zone` converts back) |
| FORM | RAND WALK RAMP ACC | **DICE** (the zone is a pure function of the layer seed: it changes only when the layer is diced) or **BAR** (the zone also depends on the bar number mod 4: up to four zones over a four-bar cycle) |
| MOVE | lane depth | **spread**: how many zones above the base zone the lane may visit, `spread = (MOVE * (n - 1) + 63) / 127` (MOVE 40 on 8 zones = 2; MOVE 0 = never leaves the base zone; this keeps "0 = the lane is silent") |
| When SEQ changes it | any step (at most one CC per step) | **only on a bar line** (768 ticks): `rig_bar` recomputes the zone from the seed and sends if it changed; a dice press between bar lines is announced at once (toast `HITS + SCTYP`) and lands on the next bar line (the cell shows `#3>5` until then) |
| Dedupe, park, undo, preset | as 3.3, 3.4, 1.7 | the same code paths: the value is a CC, so dedupe, park (`home` = the centre of the home zone), SEQ cap 8 and "undo = seeds" all hold |

The lane (all integers, a pure function, so a golden can fix it):
```c
/* SEL lane, called by rig_bar(bar) for every SEQ wire whose parameter is SEL */
n = hw_zones(p);  base = hw_cc_zone(p, level);                       /* 1..n */
spread = (move * (n - 1) + 63) / 127;                                /* 0..n-1 */
h = rig_mix(seed, ch, wi, form == BAR ? (bar & 3) : 0, 4);          /* salt 4: the CONT forms use salts 1..3 */
zone = (base - 1 + h % (spread + 1)) % n + 1;                       /* wraps past n */
value = hw_zone_cc(p, zone);                                        /* then the send path of 3.3, prio 0, due = the bar line */
```
`seed` is the seed of the riding layer or `RIG_SEED0` (LAYER NONE or absent: the zone freezes at the base zone shifted by one constant, honestly). It touches no algorithm state, so goldens are unchanged (T-RIG1/T-RIG2 hold).

Why no chaos: (1) a select is never swept; (2) it changes at most once per bar; (3) MOVE bounds the choice to a window of zones, so the owner dials "a little variety" or "any type"; (4) at MOVE 0 it never moves; (5) the device sees one CC, not a burst.

### 9.3 Linked parameters (scatter type 68 and scatter depth 69)

What the owner established [V-ear, decision 106]: both CCs work; 68 is a mode select, 69 the depth of that effect; they are linked. What is **not** known (each one is a `link` bit or a TSV field, so the answer is data and not code; H-RIG20, 9.8):

| # | Unknown | If yes | Where it goes |
| --- | --- | --- | --- |
| U1 | how many types the TB-3 has and where the zone edges of CC 68 are (equal splits assumed; 8 zones is a placeholder) | the TSV `S<n>` and, if the edges are not equal, a one-line follow-up (an edge table, 17 B) | `kind` |
| U2 | depth 0 means "no effect" | `home` of SCDEP = 0 stays | `home` |
| U3 | zone 1 (or CC 0..x) of the type means OFF, and then the depth is ignored | `:G`: the depth cell is drawn dim while the type sits in zone 1 (UI only; nothing is withheld) | `link` bit5 |
| U4 | a type change with the depth up makes a click, a hang or a restart | `:D` (duck): the engine sends depth 0, the type, then the depth again, 1 ms apart | `link` bit7 |
| U5 | the device needs the depth re-sent after a type change | `:R` (resend): the depth is sent again 1 ms after the type | `link` bit6 |
| U6 | the effect is tempo-synced and needs MIDI clock | a note in the profile and the H-RIG sheet (the FM-1 sends clock when it is master) | text |
| U7 | the type change lands at once or on the device's next beat | decides whether "next bar line" can be pre-stamped earlier | `RIG_PRE_N` |

Rules that are true whatever the answers are (the **linked pair**):
1. A linked pair is two ordinary wires. The profile sets put them side by side (wires 3 and 4 of WET), so they sit next to each other on ROW 3 and on the RIG page. One wire cannot carry two CCs (the `Wire` struct is locked, decision 101), and a "compound" single control would hide which of the two knobs does what.
2. **The pair never half-exists silently.** If K1 puts a SEL parameter on a wire and its partner is not on any wire of the channel, the toast reads `SCTYP NEEDS SCDEP ON A WIRE` (177 px); it is a hint, nothing is added behind the player's back (four wires, decision 100). Putting the depth parameter alone raises no toast (a depth with no type is the type the device already has).
3. The type goes **before** the depth in the same tick (3.5 stamps: the type at `due_note - 1 ms`, a resent or ducked depth at `+1 ms`, so the depth always follows the type).
4. On ROW 3 the pair is identified by the names (`SCTYP`, `SCDEP`), by the adjacency and by the RIG status line `RIDE -  PAIRS WITH SCDEP` (140 px). No new glyph (the compact cell has no room, 4.1).
5. HAND works as for the others: turn the type knob (zone steps, 150 ms settle), turn the depth knob (continuous).
6. K1 on the RIG page **walks past parameters that already sit on another wire of the channel** (a duplicate would send two lanes to one CC); taking a parameter off a wire (K1 moves on) whose partner is wired raises the same toast `SCTYP NEEDS SCDEP ON A WIRE`.

### 9.4 Aligning 11 parameters onto 4 wires: the options

The problem in the owner's words: *control the ones we need and want.* What the player wants to say: "give me the filter set", "give me the scatter pair", "swap just this one knob for that one". Three designs, scored with the rubric of SEQUENCER_UX 8.1 (the designer's scores, not machine-checked; a setup function needs 10 with no sub-score under 2 and at most 2 gestures).

#### Option 1: WIRE SETS on ALGO (device and set in one list)

The profile names up to six sets (`HwSet`). On the RIG page the ALGO list becomes `NONE GENERIC TB-3 BITE WET PADS PUSH`: a device is its first set; the list is device + set. Turning ALGO moves a *pending* choice; it applies 0.6 s after the last detent (`SET_SETTLE_MS`, the same pattern as the preset load, decision 71) so spinning through the list does not rewrite and re-park the wires on the way. Applying is `sq_rig_set_prof(c, id, set)`: the four wires take the set's parameters, each with its own lane (`dcfg`, `dmove`), all in HAND, nothing is sent; old SEQ wires are parked as for a device change (3.4).
```
RIG page, ALGO just turned to WET                  after 0.6 s (applied)
+----------------------------------------+         +----------------------------------------+
|> 120 P01  1 ACID M2  [1]           L16 |         |> 120 P01  1 ACID M2  [1]           L16 |
|[1 ACID ] 2 EUCL m  3 SNAK m  4 STAG m  |         |[1 ACID ] 2 EUCL m  3 SNAK m  4 STAG m  |
| RIG CH1  TB-3 BITE             WIRE 1/4|         | RIG CH1  TB-3 WET              WIRE 1/4|
|>1 CUT   64 HAND ACCNT ACC   M48        |         |>1 CUT   64 HAND ACCNT ACC   M48        |
| 2 RESO  40 HAND ARTIC WALK  M24        |         | 2 EFFEC  0 HAND -     RAND  M0         |
| 3 ENV-X 64 HAND SHAPE RAMP  M32        |         | 3 SCTYP #1 HAND HITS  DICE  M40        |
| 4 ENV-Y 64 HAND -     RAND  M0         |         | 4 SCDEP  0 HAND ACCNT ACC   M40        |
|RIDE-|PARAM CUT|LAYER ACCNT|FORM ACC|MOVE 48|     |RIDE-|PARAM CUT|LAYER ACCNT|FORM ACC|MOVE 48|
|RIDE+|CC74  LEVEL 64  MIDI 2                |     |RIDE+|SET WET: CUT EFFEC SCTYP SCDEP         |
|     |RIDE -  OCT- / OCT+                   |     |     |SCTYP + SCDEP WORK AS A PAIR          |
| ALL  NOTES ACCNT SHAPE UNDO  SET       |         | ALL  NOTES ACCNT SHAPE UNDO  SET       |
|  ... buttons as PERF ...               |         |  ... buttons as PERF ...               |
| M1 M2 M3|M4 BACK|SOL FIL ROL|PG- PG+|RST|         | M1 M2 M3|M4 BACK|SOL FIL ROL|PG- PG+|RST|
|NONE GENERIC TB-3 BITE [WET] PADS PUSH  |         | TB-3 WET: CUT EFFEC SCTYP SCDEP        |
+----------------------------------------+         +----------------------------------------+
```
(the right side's status lines show for 2 s after the apply, then the normal `CC74 ...` lines.) The banner reads `TB-3 CUSTOM` as soon as the four wires differ from every set (a comparison of four CC numbers).
Gestures from PERF: **2** (hold CHN, turn ALGO). Scores: V 3 (banner, list, wire rows), I 2, O 3, L 2 (the CHN hold is named in the lane header, R-8) = **10, pass**. Strength: the whole character of the sound in two gestures; a set is a *musical* name, not a CC list. Weakness: it replaces all four wires (guarded by the settle; the previous rig is gone, since undo is seeds only, decision 100; a preset carries it).

#### Option 2: SWAP one wire from ROW 3 (a latch on the last row)

On ROW 3, `OCT+` (inert today: toast `LAST ROW`) toggles SWAP for 4 s: the cells show the parameter names inverted, K1..K4 pick the parameter of their wire from the profile list, OCT+ or 4 s without a turn leaves it.
```
ROW 3, SWAP latched (OCT+ tap)
|SWAP|[CUT]   |[RESO]  |[SCTYP] |[ENV-Y] |      the rail cell reads SWAP; the names are inverted
|3/3 | 74 ~   | 43 H   | #3 H   | 64 H   |
| SWAP: TURN A KNOB TO PICK ITS KNOB     |
```
Gestures: **2** (OCT+, turn) once on ROW 3; from ROW 1 it is two OCT+ more. Scores: V 3, I 2, O 3, **L 1** (the role of OCT+ on the last row is found only by trying it; nothing names it on the page before the press, and the cue strip never lifts L) = **9, fails the setup bar**; to pass it needs a permanent name on ROW 3, and there is no room (the rail cell is `ROW 3/3`). It also **changes a locked row** (`OCT+ on ROW 3: stays` in 4.2). A latched sub-mode inside a performance row is the mode trap the owner warned against (VISION rule 1). Not recommended.

#### Option 3: PALETTE and FOLLOW on the RIG page (K1 picks from all eleven)

Two changes to the page that already exists. (a) **FOLLOW:** the RIG page opens with the wire selected that belongs to the ROW 3 knob the player touched last (the selected-wire index is today "kept"; now it is also set by a ROW 3 turn), so "I am turning the knob that is wrong, I want another" is hold CHN, turn K1: nothing to select. (b) **PALETTE:** for 2 s after every K1 turn the two status lines show **all eleven names**, the wired ones in the accent colour and the cursor inverted, so the player sees what exists and what is on a wire.
```
RIG page, wire 3 selected (FOLLOW), K1 turned to SCTYP (palette shown for 2 s)
+----------------------------------------+
|> 120 P01  1 ACID M2  [1]           L16 |
|[1 ACID ] 2 EUCL m  3 SNAK m  4 STAG m  |
| RIG CH1  TB-3 CUSTOM           WIRE 3/4|
| 1 CUT   74 SEQ~ ACCNT ACC   M48        |
| 2 RESO  43 HAND ARTIC WALK  M24        |
|>3 SCTYP #3 HAND HITS  DICE  M40        |
| 4 ENV-Y 64 HAND -     RAND  M0         |
|RIDE-|PARAM SCTYP|LAYER HITS|FORM DICE|MOVE 40|
|RIDE+|CUT RESO ENV-X ENV-Y PAD-Y PAD-Z      |      * the 4 wired names in T_ACCENT, the cursor SCTYP inverted
|     |ACC EFFEC TUNE [SCTYP] SCDEP          |      * 201 px and 174 px of 204
| ALL  NOTES ACCNT SHAPE UNDO  SET       |
|  ... buttons as PERF ...               |
| M1 M2 M3|M4 BACK|SOL FIL ROL|PG- PG+|RST|
| SCTYP NEEDS SCDEP ON A WIRE            |      toast: the partner is on no wire
+----------------------------------------+
```
Gestures: **2** when FOLLOW matches (hold CHN, turn K1); **3** for a wire that was not the last touched (hold CHN, tap or hold its key to select, turn K1). Scores at 2 gestures: V 3, I 2, O 3, L 2 = **10, pass**; at 3 gestures the same total but over the gesture cap (P2-019 R3 counted "select + turn" as one; counted strictly it is 3; owner question G). Strength: precise, shows the whole menu, no locked row changes beyond the select rule; weakness: it is a page visit, not a performance gesture.

#### Comparison and recommendation

| Option | What it gives | Gestures | V I O L | Total | Verdict |
| --- | --- | ---: | --- | ---: | --- |
| 1 SETS on ALGO | the sound character, 4 wires at once | 2 | 3 2 3 2 | 10 | pass |
| 2 SWAP on ROW 3 | one wire, no page change | 2 (+2 to reach ROW 3) | 3 2 3 1 | 9 | **fails**, changes a locked row |
| 3 PALETTE + FOLLOW | one wire, with the whole menu in view | 2 (3 if not the last-touched wire) | 3 2 3 2 | 10 | pass at 2 |

**Recommendation: Option 1 + Option 3.** *Hold CHN and turn ALGO to pick a named wire set (TB-3: BITE, WET, PADS, PUSH) in two gestures, then correct any single wire with K1 against a visible eleven-name palette on the RIG page, which opens on the knob you touched last.* Sets answer "which four do I want" (11 parameters become 4 musical choices and every parameter is in at least one set; TUNE is in none, by design: an unmusical live move, still one K1 turn away); the palette answers "just this one is wrong". Option 2 stays a later possibility (owner question I).

#### Scores of the new and changed functions

| Function | Class | Gestures | V | I | O | L | Total | Verdict |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| R8 pick a wire set (hold CHN, turn ALGO) | S | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R9 swap the wire I just turned (FOLLOW, K1) | S | 2 | 3 | 2 | 3 | 2 | 10 | pass |
| R10 swap another wire (tap its key, K1) | S | 3 | 3 | 2 | 3 | 2 | 10 | over the cap (owner question G); R3 of 4.8 counted it as 2 |
| R11 see all 11 knobs and which are wired (palette) | S | 0 (rides R9) | 3 | 3 | 3 | 2 | 11 | pass |
| R12 pick a scatter type live (ROW 3 latched, turn Kn) | P | 1 | 3 | 2 | 3 | 3 | 11 | pass (as R4) |
| R13 dice a layer that drives a select (HITS) | P | 1 | 3 | 3 | 3 | 2 | 11 | pass (as R5) |

### 9.5 Which layer and morph drive what, per algorithm family (summary; full tables in TB3_PROFILE.md section 13)

Rule: **the dice layer is the cause, the lane is the effect, and a select is driven by the layer that is present in every algorithm.** ACCNT, HITS and NOTES exist in all six algorithms; ARTIC is absent in SNAKE (ALGORITHM_SET 4.2; EUCLID SHAPE is the live `MOD` layer since P2-022). Therefore the lanes that must always work use ACCNT (CUT, PAD-Z, SCDEP) and HITS (SCTYP); the lanes that may freeze use ARTIC (RESO) or SHAPE (ENV-X, PAD-Y), and then the dice button is dim and the lane is honest (3.1). **Set application resolves an absent layer once** to its stand-in (ARTIC -> SHAPE) and writes the *real* layer into the wire, so nothing is hidden at run time and no locked rule changes; a later algorithm change does not re-resolve (owner question B).
SCTYP is driven by **HITS** ("where" the music falls decides "how" it scatters; one press re-rolls the rhythm and the scatter flavour together), spread 2 zones at MOVE 40, bar-latched. SCDEP rides **ACCNT** (accent steps get the deeper scatter, the same idea as CUT). RIDE (morph scaling of MOVE) defaults to none everywhere (S21's numbers hold); TB3_PROFILE 13.3 lists the suggested RIDE per family for the player.

### 9.6 Screens: what is added (all strings measured; rows `P2-021` of `tests/design/widths.tsv`)

| Where | Text |
| --- | --- |
| RIG banner | `RIG CH1  TB-3 BITE` (device and set), `RIG CH1  TB-3 CUSTOM` |
| Cue while ALGO moves (2 s) | `NONE GENERIC TB-3 BITE WET PADS PUSH` with the current in brackets (239 of 240 px; a second device will need a windowed list: the current one centred) |
| Toast on apply | `TB-3 WET: CUT EFFEC SCTYP SCDEP` (200 px), `TB-3 BITE: 4 KNOBS ON ROW 3` for the first device ever |
| ROW 3 cell of a SEL wire | name `SCTYP`, value `#3 H` (zone, then the chip); pending `#3>5 ~` on the value line while a dice waits for the bar line; compact `~ SCTYP` is 47 of 49 px |
| RIG status lines (SEL wire) | `CC68  #4 OF 8  SPREAD 2 ZONES` (177 px) and `RIDE -  PAIRS WITH SCDEP` (140 px) |
| Toasts | `SCTYP NEEDS SCDEP ON A WIRE`, the dice toast `HITS + SCTYP` |
| RIG page palette | status lines 1 and 2 for 2 s after a K1 turn (201 px and 174 px) |

### 9.7 Table rows and scenarios

Edited in place (no new control, so the 490-row matrices keep their shape): `knob_map.tsv` RIG ALGO (device and set), K1 (palette, SEL zones), K3 (the SEL forms), K4 (spread), PERF R1..R4 (a SEL steps one zone per detent); `control_map.tsv` EN_ALGO, EN_K1, EN_K3, EN_K4 on RIG; `page_states.tsv` RIG screen text; `gen_help.py` and the regenerated `help_text.tsv` for the same rows; `widths.tsv` +P2-021 block. Scenario S21 re-walked (set names), new **S23** (`scenarios/S23-rig-sets-scatter.md`): pick the WET set, hand-turn a select, play, dice HITS, undo, palette swap. S22 is regenerated.

### 9.8 H-RIG20: scatter linkage by ear (about 15 minutes; the owner runs it)

Rig of H-RIG1 (TB-3 on channel 2, a held or repeating note, host box in the path). Use any tool that sends single CCs (the same `rtmidi` one-liners as 2026-11-11).
1. **Zones (U1).** Depth CC 69 at 100. Send CC 68 from 0 to 127 in steps of 1 every 400 ms, repeating one note. Write down the CC values where the sound changes: those are the zone edges; count the sounds: that is `n`.
2. **Depth (U2).** Type at a middle zone. CC 69 = 0, 32, 64, 96, 127: does 0 mean no effect? Is the change smooth or stepped?
3. **Off (U3).** Type CC 68 = 0 and CC 69 = 127: any effect? Then CC 68 = a mid zone with CC 69 = 0: any effect? Write which of the two silences it.
4. **Change under load (U4).** Depth 100. Switch the type between two far zones, three times each. Click, hang, restart, silence?
5. **Resend (U5).** Depth 100, switch the type, then send CC 69 = 100 again: does anything change?
6. **Clock (U6, U7).** Repeat step 4 with the FM-1 as clock master, stopped, playing: does the change wait for a beat? Does the effect need clock?
7. **Both ends.** CC 68 = 0 and 127, CC 69 = 0 and 127, four corners: anything stuck after the test? Set both back to 0.
Write into `tb3.tsv`: the zone count and edges (`S<n>`, an edge table if not equal), `:G` (U3), `:D` (U4), `:R` (U5); each is a one-token change. Until then the TSV carries `S8` equal zones, no flags: the placeholder that cannot do harm (centre values, one CC per bar).

### 9.9 Owner questions (none changes a locked decision)

| # | Question | Options | Recommendation |
| --- | --- | --- | --- |
| A | Wire sets: how does the player pick them? | A) in the ALGO list with the device (`TB-3 BITE`, `TB-3 WET` ...), applied 0.6 s after the last detent; B) a separate control on the RIG page (it has no free encoder, so a knob would lose a job); C) no sets: K1 only (3 gestures per wire) | **A** |
| B | A set whose wire rides a layer the algorithm lacks (RESO on SNAKE) | A) the set application writes the stand-in layer (ARTIC -> SHAPE) once; B) it writes the layer as defined and the lane freezes (as 3.1); C) A at application **and** re-resolved on every algorithm change | **A** (honest, no run-time magic; the RIG page shows the real layer) |
| C | The RIG page opens on the wire of the ROW 3 knob touched last (FOLLOW) | A) yes; B) keep the old "kept index" | **A** |
| D | SEL behaviour: DICE and BAR forms, changes only on a bar line, MOVE = spread, HAND settle 150 ms | A) as 9.2; B) change on the step; C) change on the pattern loop start | **A** (a bar line is the one boundary every channel and length shares; step-wise changes are the chaos the owner asked to avoid) |
| E | Scatter zone count and edges are unknown (8 equal zones is a placeholder) | A) run H-RIG20 (15 minutes) before P4-024 and fix the TSV; B) ship the placeholder and tune in P5-010 | **A** |
| F | The TB-3 sets: BITE (CUT RESO ENV-X ENV-Y), WET (CUT EFFEC SCTYP SCDEP), PADS (PAD-Y PAD-Z CUT RESO), PUSH (CUT ACC EFFEC RESO); TUNE in none | A) accept; B) other members or names | **A** |
| G | Swapping a wire that was not the last touched costs 3 strict gestures (hold CHN, select key, K1); P2-019 counted it as 2 | A) accept as one listed exception (a rare correction, sets cover the common case); B) hold a wire key and turn K1 (a modifier, I1: worse) | **A** |
| H | Provenance level of the TB-3 profile: all 11 CCs heard by a named person is level 2 of the HwProv scale | A) set level 2 now (`L2 JK 2611`); B) wait for H-RIG2 (the resting values) | **A** |
| I | Option 2 (SWAP latch on ROW 3, scores 9) | A) not now; B) reopen OCT+ on the last row after H-RIG17 | **A** |

### 9.10 Checks run for this section

`python3 tests/design/check_scenarios.py`, `python3 tests/design/check_help.py`, `python3 tools/check_sequencer_ux.py`, `sh tests/design/width_probe.sh` and `python3 tools/backlog.py check`: see the P2-021 Log. No firmware, no `tools/check.sh`, no build.
