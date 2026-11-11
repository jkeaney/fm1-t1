/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* A project as firmware before 1.2 wrote it (the host tests' old images; included after src/project.c):
 *   FUN9 (1.1: np 99, 3648 bytes), FUN8 (1.0.x: np 91, 3584), FUN7 (np as given, 3388, no patches)
 * byte for byte as their proj_pack: np parameters mapped by count (the first np - 8 are today's ids 0.., the last 8
 * P_E0..P_E7), 9-byte steps without a nudge (bit 7 of the note bytes 0), the song chain (36 bytes) and the motion block (260 bytes), both zeros
 * since P3-007 (the song chain and the motion are gone; loads skip them), the
 * patches at name - 512 (not FUN7), the name, the FNV-1a hash. -> its size, 0 when q does not fit the old format.
 */
#define OLD_FUN9 0x46554E39u
#define OLD_FUN8 0x46554E38u
#define OLD_FUN7 0x46554E37u
static uint32_t old_pack(uint8_t *b, const project_t *q, uint32_t magic, uint32_t np)
{
    uint32_t st = magic == OLD_FUN9 ? 3648u : magic == OLD_FUN8 ? 3584u : 3388u, pos = 68u, t, i, sum;
    uint32_t name = st - 16u;
    if (np < 8u || np > P_COUNT)
        return 0;
    memset(b, 0, st);
    memcpy(b, &magic, 4);
    memcpy(b + 4, &st, 4);
    memcpy(b + 8, q->g, sizeof q->g);
    b[62] = q->sel; b[63] = q->parts; b[64] = q->phys; b[66] = (uint8_t)np;
    for (t = 0; t < NTRK; t++) {
        for (i = 0; i < np; i++) {
            uint32_t id = i < np - 8u ? i : P_E0 + i - (np - 8u);
            b[pos++] = (uint8_t)(q->t[t].p[id] + 64);
        }
        b[pos++] = q->t[t].engine;
        b[pos++] = q->t[t].preset;
        for (i = 0; i < NSTEP; i++) {
            const step_t *s = &q->t[t].step[i];
            uint32_t r = step_ratchet(s) - 1u, j;
            for (j = 0; j < 4u; j++)
                b[pos++] = s->note[j] & 127u;
            b[pos++] = (uint8_t)(s->n | s->time << 3 | (s->flags & 3u) << 5);
            b[pos++] = (uint8_t)((s->vel & 127u) | (r & 1u) << 7);
            b[pos++] = s->hit;
            b[pos++] = s->acc & s->hit;
            b[pos++] = (uint8_t)(s->probability | (r >> 1) << 7);
        }
    }
    pos += 36u + 260u;                               /* the song chain (36 bytes) and the motion block: removed in P3-007, zeros */
    if (magic != OLD_FUN7) {
        if (pos > name - NTRK * FM6_PACKED)
            return 0;
        memcpy(b + name - NTRK * FM6_PACKED, q->fm6, NTRK * FM6_PACKED);
    } else if (pos > name) {
        return 0;
    }
    for (i = 0; i < PROJ_NAME_LEN && q->name[i]; i++)
        b[name + i] = (uint8_t)q->name[i];
    sum = proj_hash(b, st - 4u);
    memcpy(b + st - 4u, &sum, 4);
    return st;
}
