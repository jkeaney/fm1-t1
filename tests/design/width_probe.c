// SPDX-License-Identifier: GPL-3.0-only
/* P2-007 / P2-011: measure UI strings with the REAL text_w of firmware/src/gfx.c on the real glyph tables
 * (build/gen/ui_fonts.h). width_probe.sh extracts glyph_at .. text_w from gfx.c into text_w.inc at build time, so the
 * numbers cannot drift from the firmware. Input (stdin), tab separated: FONT(S|M|L) LIMIT_PX STRING [WHERE].
 * Output: width, LIMIT, OK or OVER. Exit 1 if any string is OVER. A STRING may contain \x85 (the ellipsis glyph). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ELLIPSIS '\x85'
typedef struct { uint16_t off, adv; int8_t bx; uint8_t by, bw, bh; } aag_t;
typedef struct {
    uint8_t h, asc, first, last, nex;
    uint16_t nk;
    const aag_t *g;
    const uint8_t *data;
    const uint16_t *kkey;
    const int8_t *kd;
    const uint8_t *ex;
    uint8_t psh;
    const uint8_t *hc;
} aafont_t;
#include "ui_fonts.h"
#include "text_w.inc"
int main(void)
{
    char line[512];
    int bad = 0, n = 0;
    while (fgets(line, sizeof line, stdin)) {
        char *f, *l, *s, *w;
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0] || line[0] == '#') continue;
        f = strtok(line, "\t"); l = strtok(NULL, "\t"); s = strtok(NULL, "\t"); w = strtok(NULL, "\t");
        if (!f || !l || !s) { fprintf(stderr, "bad line\n"); return 2; }
        const aafont_t *ft = f[0] == 'M' ? &AF_M : f[0] == 'L' ? &AF_L : &AF_S;
        int px = text_w(ft, s), lim = atoi(l);
        printf("%4d / %3d  %s  `%s`  %s\n", px, lim, px <= lim ? "OK  " : "OVER", s, w ? w : "");
        n++; if (px > lim) bad++;
    }
    printf("%d strings, %d over\n", n, bad);
    return bad ? 1 : 0;
}
