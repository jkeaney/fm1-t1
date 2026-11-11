/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_types.h: the sequencer core's data model. P2-008 reference copy.
 *
 * Source of truth: ADR-0003 section 3 as amended by SEQUENCER_UX 9.1 (StepEdit, ChanSave 344, UndoRec 92, ChanRT),
 * HARMONY 2 (HarmonySave), docs/design/algorithms/arp/SPEC.md 6.10 (AF_CHORD, the chord argument of step(),
 * AlgoDesc.sub_note) and docs/design/MODULATION.md 3 and 7 (the mod gen: s_mod, ModIdx, AlgoDesc.mod, UndoRec +2 B).
 * P2-017 FINAL for the six v1 algorithms (ACID EUCLID SNAKE STAGE ARP PATTERNS): ChanSave holds six AlgoState slots,
 * SeqPreset is 1556 B, PulseOut has PO_TIE. The firmware file is a copy of this one (P4-001 adds `static` where a function is
 * defined, never a field). Freestanding: stdint, stddef and stdbool only. Every size is asserted: a struct size change is
 * a contract change (the preset layout is stored) and needs an ADR-0003 amendment (P2-029 writes the ones listed in
 * docs/design/algorithms/LOCK.md section 5). */
#ifndef SQ_TYPES_H
#define SQ_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SQ_NCH        4          /* channels (owner decision 31) */
#define SQ_NALGO      6          /* 0 ACID, 1 EUCLID, 2 SNAKE, 3 STAGE, 4 ARP, 5 PATTERNS (decision 82) */
#define SQ_NMORPH     8          /* slots 0..3 = K1..K4 (ROW 1), 4..7 = S1..S4 (ROW 2) */
#define SQ_NSEED      6          /* seeds stored per algorithm in AlgoState (ACID has 6; ARP and PATTERNS 5, so six is final) */
#define SQ_SEED_MOD   6          /* seed id of s_mod. NOT in AlgoState: ModSave holds it, one per algorithm per channel (MODULATION 7),
                                    and the core hands build() / the dice harness a 7th slot. Every algorithm's SHAPE layer re-rolls it */
#define SQ_SMOD_DEFAULT(ch) ((uint16_t)((0x3A5Du * (uint32_t)((ch) * 7u + 7u)) & 0xFFFFu))   /* a fresh channel's s_mod: never 0 */
#define SQ_NLAYER     5          /* 0 NOTES, 1 HITS, 2 ACCNT, 3 ARTIC, 4 SHAPE (ALGORITHM_SET 4.1; fixed) */
#define SQ_CACHE_MAX  64         /* bytes of an algorithm's private cache */
#define SQ_PPQN       192
#define SQ_MIDI_OFF   16         /* ChanSave.midi_ch value for OFF */
#define SQ_NSTEP      64         /* steps per channel = 4 pages of 16 (SEQUENCER_UX 9.1) */
#define SQ_PAGE       16
#define SE_AUTO       0          /* every StepEdit field: 0 = follow the generated layer */
#define SQ_BAR_TICKS  768        /* the fixed bar grid (decision 45) */

/* algorithm ids = index in ALGOS[] */
#define SQ_ALGO_ACID      0
#define SQ_ALGO_EUCLID    1
#define SQ_ALGO_SNAKE     2
#define SQ_ALGO_STAGE     3
#define SQ_ALGO_ARP       4
#define SQ_ALGO_PATTERNS  5

/* One step as an algorithm reports it. 6 bytes. note, vel and gate are valid for EVERY call, also for a rest. */
typedef struct {
    uint8_t note;    /* final MIDI note 0..127: scale, key, octave already applied (sq_note) */
    uint8_t vel;     /* 1..127 */
    uint8_t gate;    /* 1..100: the algorithm's own gate as % of one step (before global GATE) */
    uint8_t flags;   /* PO_* below */
    uint8_t hold;    /* 1..8 pulses; valid only with PO_HOLD */
    uint8_t pad;     /* 0 */
} PulseOut;
#define PO_HIT        0x01   /* this step sounds */
#define PO_SLIDE_IN   0x02   /* stored glide flag "step reached by a glide from the step played before" (core decides if effective) */
#define PO_HOLD       0x04   /* long note: ignores global GATE, lasts `hold` pulses (STAGE) */
#define PO_PACE_DEC   0x08   /* ratchet spacing decelerates (else accelerates; two-way, decision 115) */
#define PO_REP_SHIFT  4      /* bits 4..5: repeats-1 (0..3 -> 1..4 triggers in this step) */
#define PO_REP_MASK   0x30
#define PO_PIN        0x40   /* reserved (the pin overlay is replaced by StepEdit, SEQUENCER_UX 9.2) */
#define PO_TIE        0x80   /* PATTERNS: the gate holds into the next sounding step (P2-017; the stored T flag of a library pattern) */
_Static_assert(sizeof(PulseOut) == 6, "PulseOut");

typedef struct { char label[7]; uint8_t min, max, def, step, fast; } MorphDesc;  /* label <= 6 chars + NUL; step per slow detent, fast per detent under 40 ms; 12 B */
typedef struct { uint8_t n; uint8_t seed[2]; } LayerDesc;                        /* n = 0: layer absent (dimmed); 3 B */
_Static_assert(sizeof(MorphDesc) == 12, "MorphDesc");
_Static_assert(sizeof(LayerDesc) == 3, "LayerDesc");

/* Global performance settings. 6 B saved + 12 B derived = 18 B */
typedef struct { uint8_t key; int8_t octave; uint8_t scale; uint8_t gate; uint8_t div; uint8_t swing; } GlobalSave;
typedef struct { GlobalSave s; uint8_t snap[12]; } SeqGlobal;
/* key 0..11 (C=0); octave -2..+2; scale 0..15 (SQ_SCALE_MASKS order); gate 10..100 (%); div 0..5 = 1/32, 1/16T, 1/16,
 * 1/8T, 1/8, 1/4; swing 50..75 (%). snap[r] = scale tone for interval r above the key (SP6), derived by sq_gbuild, never
 * saved. snap[r] can be 12 (the next octave's key note is the nearest tone). */
_Static_assert(sizeof(GlobalSave) == 6 && sizeof(SeqGlobal) == 18, "SeqGlobal");

/* What one algorithm remembers for one channel (S7). 8 morphs + 6 seeds = 20 B */
typedef struct { uint8_t morph[SQ_NMORPH]; uint16_t seed[SQ_NSEED]; } AlgoState;
_Static_assert(sizeof(AlgoState) == 20, "AlgoState");

/* One step's edit layer (SEQUENCER_UX 9.1). 4 B. */
typedef struct {
    uint8_t note;    /* bit7 = PITCH set; bits0..6 = semitones above 36+key, 0..96, UNSNAPPED */
    uint8_t vel;     /* 0 AUTO, 1..127 */
    uint8_t gr;      /* bits0..3 GATE: 0 AUTO, 1..10 = 10..100 %, 11 = TIE; bits4..6 RATCH: 0 AUTO, 1..4; bit7 0 */
    uint8_t fl;      /* bits0..1 ON; bits2..3 SLIDE; bit4 PROB; bits5..7 OCT (value - 3) */
} StepEdit;
_Static_assert(sizeof(StepEdit) == 4, "StepEdit");

/* One channel as saved. 384 B (was 344 with four algorithm slots) */
typedef struct {
    uint8_t  midi_ch;              /* 0..15, 16 = OFF */
    uint8_t  algo;                 /* 0..SQ_NALGO-1 */
    uint8_t  muted;
    uint8_t  len;                  /* 1..64 */
    uint8_t  pg_src;               /* bits 2k..2k+1 = the generator block page k reads (SEQUENCER_UX 1.4) */
    uint8_t  pg_used;              /* bit k = page k holds content */
    uint8_t  pad[2];               /* pad[0] carries the P2-024 preview byte (not modelled here; PREVIEW.md) */
    AlgoState st[SQ_NALGO];        /* 6 x 20 B, indexed by algorithm id (P2-017: ARP and PATTERNS each add 20 B per channel; SEQUENCER_UX 9.1
                                      had 4 slots) */
    StepEdit ed[SQ_NSTEP];         /* 64 x 4 B */
} ChanSave;
_Static_assert(sizeof(ChanSave) == 384, "ChanSave");

/* HARMONY (HARMONY.md 2.1). 8 B, saved per preset. */
typedef struct {
    uint8_t len;     /* bits0..2 bars-1 (0..7); bits3..5 nchords-1 (0..7). Rule: nchords <= bars */
    uint8_t deg[3];  /* 8 slots x 3 bits: degree-1 of slot i = ((deg[0] | deg[1]<<8 | deg[2]<<16) >> (3*i)) & 7 */
    uint8_t vm;      /* bits0..3 VOICE 0..6 (default 2); bits4..7 MOVE 0..8 (default 8) */
    uint8_t drone;   /* 0 = off; 1..14 = the candidate walked to */
    uint8_t roles;   /* 2 bits per channel (bit 2c): 0 AUTO, 1 OFF, 2 ROOT, 3 FULL */
    uint8_t rsv;
} HarmonySave;
_Static_assert(sizeof(HarmonySave) == 8, "HarmonySave");
#define HR_AUTO 0
#define HR_OFF  1
#define HR_ROOT 2
#define HR_FULL 3

/* The chord and voicing query result (HARMONY.md 9). 14 B as compiled (HARMONY.md says 16 B; the fields it lists add up to 14) */
#define HARM_MAXV 6
typedef struct {
    uint8_t  active;          /* 0 = HARMONY inert: callers behave as before */
    uint8_t  slot, n;         /* chord index 0..7, number of voiced notes in vt[] (1..6) */
    uint8_t  deg;             /* degree 1..7 */
    int8_t   root;            /* chord root, semitones above the key */
    int8_t   drone;           /* drone pitch, semitones above the key, or -1 */
    int8_t   vt[HARM_MAXV];   /* voiced notes ascending, semitones above the key (VOICE + MOVE + drone) */
    uint16_t pcs;             /* 12-bit pitch-class set of the chord tones and the drone, bit k = key + k */
} HarmChord;
_Static_assert(sizeof(HarmChord) == 14, "HarmChord");

/* Owner ruling 2026-11-10: assigning a taken MIDI channel sets the other channel to OFF; one undo record for both. */
typedef struct { uint8_t ch, old_midi, other, other_old; } UndoMidi;
_Static_assert(sizeof(UndoMidi) == 4, "UndoMidi");

/* One undo / redo record per channel (decision 46). 94 B (92 + s_mod, MODULATION 7) */
typedef struct {
    uint8_t  what;                 /* bit0 seeds, bit1 one page of edits, bit2 morphs, bit3 one step, bit4 len / pg bits */
    uint8_t  algo, seed_mask, idx; /* idx = page (bit1) or step (bit3) */
    uint16_t seed[SQ_NSEED];       /* the OTHER values (swap = undo, swap again = redo) */
    uint8_t  morph[SQ_NMORPH];
    uint16_t smod;                 /* the OTHER s_mod (MODULATION 7: the SHAPE layer re-rolls it, so undo restores it) */
    uint8_t  len, pg_src, pg_used, pad;
    StepEdit ed[SQ_PAGE];          /* one page, or ed[0] for one step */
} UndoRec;
_Static_assert(sizeof(UndoRec) == 94, "UndoRec");

/* The saved device state = a preset = the autosave. 1556 B (six algorithm slots per channel; HARMONY.md 2.1). ModSave (MODULATION 7,
 * 25 B per channel) is a sibling array and not counted here. */
typedef struct {
    uint8_t  version;              /* 4: six algorithm slots (3 had four; the bump is the storage layer's, P7-005) */
    uint8_t  flags;                /* 0 */
    uint16_t bpm;                  /* 40..240 */
    GlobalSave g;
    ChanSave ch[SQ_NCH];
    HarmonySave harm;
    uint16_t crc;                  /* CRC-16 over the bytes before it (storage layer) */
} SeqPreset;
_Static_assert(sizeof(SeqPreset) == 1556, "SeqPreset");
_Static_assert(sizeof(((SeqPreset *)0)->ch) == 1536, "SeqPreset.ch");

/* ISR-only runtime per channel. 24 B. The new fields go FIRST (SEQUENCER_UX 9.1 revision 2): grid@0 pass@4 p@6 tie@7
 * nxt@8 ... open@21. */
typedef struct {
    uint32_t grid;                 /* tick of the nominal (un-swung) start of step p */
    uint16_t pass;                 /* loops since PLAY / RESTRT */
    uint8_t  p;                    /* index of the NEXT step to start, 0..len-1 */
    uint8_t  tie;                  /* 1 = a TIE is holding the sounding note */
    PulseOut nxt;                  /* pairing snapshot of the next step (AF_PAIRING only) */
    uint8_t  nxt_ok;
    uint8_t  snd[2];               /* note+1 currently sounding (2 only during a slide overlap), 0 = free */
    uint8_t  snd_chan[2];
    uint8_t  slide_in;
    uint8_t  act;
    uint8_t  open;
} ChanRT;
_Static_assert(sizeof(ChanRT) == 24, "ChanRT");
_Static_assert(offsetof(ChanRT, p) == 6 && offsetof(ChanRT, nxt) == 8 && offsetof(ChanRT, open) == 21, "ChanRT layout");

/* A scheduled MIDI event. 8 B. */
typedef struct {
    uint32_t tick;
    uint8_t  chan;                 /* bits 0..3 MIDI channel; bit 4 = MIDI OFF; bits 5..6 sequencer channel */
    uint8_t  kind;                 /* 0 empty, 1 NOTE_OFF, 2 NOTE_ON */
    uint8_t  note;
    uint8_t  vel;
} SeqEvent;
_Static_assert(sizeof(SeqEvent) == 8, "SeqEvent");
#define SQ_EVQ_N 96                /* 768 B */

/* The mod gen's read-out (MODULATION 3.1, 4). The algorithm says WHICH index each lane is read at; the shared primitives SP18..SP20
 * (sq_prims.h) turn it into a value per slot: slots 1 and 3 read lane A, slots 2 and 4 lane B. 8 B, valid for every step (also a rest). */
typedef struct {
    uint8_t  a;        /* lane A: the cell index (an index the algorithm already has: gate position, hit number, stage, figure position, step) */
    uint8_t  vel;      /* the step's velocity: lane A leans on loudness (SP20) */
    uint8_t  b_cell;   /* lane B when b_lfo == 0: a cell index (SNAKE mod snake B) */
    uint8_t  b_lfo;    /* 1 = lane B is the LFO of (t, period) */
    uint16_t t;        /* LFO position, in the algorithm's own steps */
    uint16_t period;   /* LFO period in steps; 0 = the channel LENGTH (the core knows it) */
} ModIdx;
_Static_assert(sizeof(ModIdx) == 8, "ModIdx");

/* The algorithm vtable (ADR-0003 section 4 + ARP/SPEC 6.10 + ADR-0003 section 20 + MODULATION 3.1). */
#define AF_PAIRING 0x01            /* step needs the next-step snapshot (slide) */
#define AF_CHORD   0x02            /* step() reads the chord (ARP); the core then skips harm_apply (AM-3) */

typedef struct AlgoDesc {
    char       name[8];                    /* "ACID" (<= 6 chars) */
    uint8_t    id;                         /* index in ALGOS[] */
    uint8_t    n_seed;                     /* seeds this algorithm uses (<= SQ_NSEED) */
    uint8_t    flags;                      /* AF_* */
    uint8_t    cache_bytes;                /* sizeof its cache, <= SQ_CACHE_MAX */
    uint8_t    knob_morph[SQ_NMORPH];      /* knob slot (K1..K4, S1..S4) -> morph id. THE knob remap table */
    uint8_t    seed_k[SQ_NSEED];           /* default-seed k per seed (0xFF = literal 0) */
    MorphDesc  morph[SQ_NMORPH];           /* label / min / max / def / steps, indexed by MORPH ID (build()'s m[]) */
    LayerDesc  layer[SQ_NLAYER];           /* shared layer -> seed ids (n = 0 absent). THE dice remap table */
    char       sub[SQ_NLAYER][8];          /* dice sub-labels; "-" for an absent layer */
    void     (*build)(const uint8_t m[SQ_NMORPH], const uint16_t seed[SQ_NSEED], const SeqGlobal *g, void *cache);
    uint8_t  (*loop_len)(const void *cache);                          /* positions of the visible loop, 2..64 */
    void     (*step)(const void *cache, uint32_t g, const HarmChord *hc, PulseOut *out);  /* hc is NULL unless AF_CHORD */
    uint8_t  (*hi)(const uint8_t m[SQ_NMORPH]);                       /* highest semitone above the base the algorithm can emit (ADR-0003 section 20) */
    uint8_t  (*sub_note)(const void *cache, uint32_t g, uint8_t k, const HarmChord *hc);   /* ARP repeat pitch; NULL for the others */
    void     (*mod)(const void *cache, uint32_t g, const HarmChord *hc, const PulseOut *o, ModIdx *out);   /* the mod lanes' indices; pure like step() */
} AlgoDesc;

#endif /* SQ_TYPES_H */
