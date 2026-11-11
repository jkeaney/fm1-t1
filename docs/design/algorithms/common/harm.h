/* SPDX-License-Identifier: GPL-3.0-only */
/* harm.h: the HARMONY chord stage and the query interface of HARMONY.md 4 and 9. P2-008 reference copy; stdint only. */
#ifndef HARM_H
#define HARM_H

#include "sq_types.h"

#define HQ_PC     0      /* harm_snap: nearest chord tone, all octaves */
#define HQ_VOICED 1      /* nearest exact voiced note */
#define HQ_ROOT   2      /* chord root in the octave of s */

/* Bind the state the queries read (the firmware reads harm_db[harm_gidx] and sq_gdb[sq_gidx]; here the caller owns both). */
void     harm_bind(const SeqGlobal *g, const HarmonySave *h);
void     harm_reset(void);                                /* PLAY: every channel's anchor is due again */

/* HARMONY.md 9 */
uint8_t  harm_active(void);                               /* 0 = inert (1.1) */
void     harm_chord_at_bar(uint16_t bar, HarmChord *out); /* the chord of bar `bar` (taken % bars); builds the voice-leading chain */
void     harm_set_time(uint32_t sq_bar_ticks);            /* the ISR's HarmRT: what harm_chord_now reads */
void     harm_chord_now(HarmChord *out);
uint16_t harm_ticks_to_change(uint32_t sq_bar_ticks);
int16_t  harm_snap(const HarmChord *c, int16_t s, uint8_t mode);
int16_t  harm_tone(const HarmChord *c, uint8_t idx);

/* The chain itself (3.1, 3.2): the voiced notes of progression slot `slot`, pure chord tones (no drone), ascending. Returns n. */
uint8_t  harm_voicing(uint8_t slot, int8_t vt[HARM_MAXV]);
int16_t  harm_drone_pitch(uint8_t p);                     /* 3.3: -1 when off */
char     harm_drone_mark(const int8_t *vt, uint8_t n, int16_t drone);   /* '+' in the chord, 'X' a semitone from a chord tone, '-' otherwise */

/* 4.2 core (pure) and 4.1 entry (reads the bound state, the anchor bits and the roles). */
int16_t  harm_pull_note(const HarmChord *c, int16_t s, uint8_t t, uint8_t strong, uint8_t pull, uint8_t first, int16_t lo, int16_t hi);
/* lo..hi = the semitone range the algorithm plays (HARMONY.md 4.2 rule 5 folds into it; the spec's 0..hi is lo = 0). */
int16_t  harm_apply(uint8_t ch, uint8_t t, uint8_t strong, int16_t s, int16_t lo, int16_t hi, uint8_t pull, uint32_t tick);
int16_t  harm_lo(uint8_t algo);                          /* the lowest semitone algorithm `algo` emits: ACID -12, the others 0 */
/* 4.3: the pull amount (0..8) of algorithm `algo` from its morphs m[] (0 while inert is the caller's test through harm_apply). */
uint8_t  harm_pull(uint8_t algo, const uint8_t m[SQ_NMORPH]);

/* 6.1 the progression dice: nscale = size of the scale; degs[] get degrees 1..7. xorshift seeded WITHOUT the SP1 warm-up
 * (HARMONY.md 6.1 quotes the golden table from harmony_proto.py, which does not discard). */
void     harm_dice(uint16_t seed16, uint8_t nscale, uint8_t *bars, uint8_t degs[8], uint8_t *n);

#endif /* HARM_H */
