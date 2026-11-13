/* SPDX-License-Identifier: GPL-3.0-only */
/* The note audition queue (P4-034, FB-019): the main loop (ui_sq.c, the REC page and the stopped step tap) pushes note-on / note-off events, the audio ISR
 * (events_block, seq.c) pops them into sq_live_on / sq_live_off. Single producer, single consumer, no lock: w is written only by the UI, r only by the
 * ISR, and a slot is written before w moves past it. 16 slots, 15 usable. It lives here, ahead of seq.c, so every host test that links seq.c has it
 * whether or not it also builds the screen (ui_sq.c includes this file too; the guard makes the second include a no-op). The UI owes every note-on an
 * off (uisq_rec_flush retries until the ring takes it): a full ring drops an on, never an off. */
#ifndef SQ_AUD_H
#define SQ_AUD_H
#define UISQ_AUD_N      16u
static volatile uint16_t uisq_aud_q[UISQ_AUD_N];                       /* bit 14 = on, bits 7..13 velocity, bits 0..6 note */
static volatile uint8_t  uisq_aud_w, uisq_aud_r;
static uint8_t uisq_aud_push(uint8_t on, uint8_t note, uint8_t vel)
{   const uint8_t w = uisq_aud_w, nx = (uint8_t)((w + 1u) % UISQ_AUD_N);
    if (nx == uisq_aud_r) return 0;
    uisq_aud_q[w] = (uint16_t)((on ? 0x4000u : 0u) | ((uint16_t)(vel & 127u) << 7) | (note & 127u));
    uisq_aud_w = nx;
    return 1; }
/* the consumer's end (the ISR; the host tests): the next queued note event, 0 when empty */
static uint8_t ui_sq_aud_next(uint8_t *on, uint8_t *note, uint8_t *vel)
{   const uint8_t r = uisq_aud_r;
    uint16_t e;
    if (r == uisq_aud_w) return 0;
    e = uisq_aud_q[r];
    *on = (uint8_t)((e >> 14) & 1u); *vel = (uint8_t)((e >> 7) & 127u); *note = (uint8_t)(e & 127u);
    uisq_aud_r = (uint8_t)((r + 1u) % UISQ_AUD_N);
    return 1; }
#endif
