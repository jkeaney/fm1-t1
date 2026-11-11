/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Renders of the firmware UI (firmware/src/ui*.c on tests/ui_test.c's stubs), the layout lint, the GREY and MONO
 * checks and the host draw cost. P3-006 (R3b): the screens are the T-1 screen, the menu (every tab and row, ABOUT,
 * INFO, CALIBRATION), the header's states, the count-in, the dialog, UPDATE MODE and the splash.
 *   ui_render OUTDIR [SLOTDIR]   (run by tests/run_tests.sh; tests/ui_render.py makes the PNGs; SLOTDIR is unused now)
 * Layout lint, over every screen in every palette: the ink box of every text and icon drawn (gfx.c GFX_HOOK_TEXT) as it
 * lands on the screen. A finding: a box off the screen, cut by its canvas, partly covered by a later strip or fill,
 * overlapping another box, or ellipsised when it is not free text (labels and values must fit; names and messages may
 * be ellipsised), or spilling out of its cell: touching a rounded rectangle of its canvas (gfx.c cv_rrect: a cell, card,
 * row, button, GFX_HOOK_CELL) without lying inside it, though still on the screen, or drawn in the colour it is drawn
 * on (gfx.c GFX_HOOK_INK: a text, icon or keycap label that cannot be seen; MONO's tokens share values, DIM and RAISE
 * its one grey).
 * GREY: every pixel of every screen is RGB565 gray (R = B, G = 2 R). MONO (black and white, blended per channel):
 * every pixel is neutral (R = B, G within 2 of 2 R).
 * MENU > STYLE LINE: every screen again in every palette, the same lint; a text or icon on a divider
 * (ui_draw.c lcd_rule, GFX_HOOK_RULE) is a finding too.
 * Output: OUTDIR/ppm/<PALETTE>_<screen>.ppm for GREY MONO GREEN PAPER NIGHT, OUTDIR/report.txt (findings, ellipsised
 * free text), OUTDIR/text_audit.tsv (GREY, per screen: texts, icons and keycaps with their ink-box px, the ellipsised
 * ones, the texts closer than 2 px to their cell's edge, the words).
 * Alignment (gfx.c GFX_HOOK_ALIGN): every place where a text, icon or keycap is meant to sit centred or on a line shared
 * with its neighbours declares its box; the ink drawn there is measured against it, on every screen in FLAT and LINE and
 * every palette, and align_sweeps (every value each place can show). 0.5 px is the odd pixel left over (it goes left /
 * up); 1 px or more is a finding. OUTDIR/align.txt: per place, how often measured, the worst offsets and where.
 * Exit 1 on a finding, a GREY pixel off gray, a MONO one off neutral, or an alignment 1 px or more off. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

typedef struct { int16_t x0, y0, x1, y1; uint8_t flags; char s[28]; char fnt; } rbox_t;   /* fnt: S M L (its face) */
static rbox_t pend[256], scr[1200];
static int16_t cells[64][4];                    /* GFX_HOOK_CELL: the cells and cards of the canvas being drawn */
static uint32_t ncells, nspill;
static FILE *aud;                               /* OUTDIR/text_audit.tsv: GREY, the text of each screen */
static char tight[400];                         /* .. the texts of this screen closer than 2 px to their cell's edge */
static uint32_t ntight;
static uint32_t npend, nscr, nfind, nfree;
static uint64_t px_visited, n_text, blit_px;
static FILE *rep;
static const char *cur_name = "";
static char free_seen[64][40];
static uint32_t nfree_seen;
/* the alignment check (gfx.c GFX_HOOK_ALIGN): a place declares the box its next item(s) are meant to be centred in,
 * across and / or up and down; the items' ink as drawn (a text: its glyphs' boxes across, its capitals' band up and
 * down, a text of no letter or figure: across only; an icon: the ink of its cell; a keycap: its pill; a cushion: the
 * cushion) against the box: the offset of the ink's centre from the box's, in half pixels. 0.5 is the odd pixel
 * left over (it goes left / up: a +0.5 is counted apart); 1 px or more is a finding. Per place (its tag): how
 * often it was measured, the worst offsets and where, in OUTDIR/align.txt */
typedef struct { int32_t x0, y0, x1, y1, ix0, iy0, ix1, iy1; uint32_t mode, left, nv; const char *tag; char s[24]; } al_t;
typedef struct { const char *tag; uint32_t n, nfail, half_lo, half_hi; int32_t wh2, wv2; char ex_h[112], ex_v[112]; } alstat_t;
static al_t al_stk[8];
static uint32_t al_n, al_fail, al_lost, al_count, al_off;
static int32_t al_cy0, al_cy1;                  /* GFX_HOOK_PEN: the capitals' band of the text being hooked */
static alstat_t al_st[160];
static uint32_t al_nst;
static FILE *al_rep;
static alstat_t *al_stat(const char *tag)
{
    uint32_t i;
    for (i = 0; i < al_nst && strcmp(al_st[i].tag, tag); i++) ;
    if (i == al_nst && al_nst < 160u) { memset(&al_st[i], 0, sizeof al_st[i]); al_st[i].tag = tag; al_nst++; }
    return &al_st[i < 160u ? i : 159u];
}
/* one measurement: offsets in half pixels (h2, v2; 99: not measured) */
static void al_record(const char *tag, int32_t h2, int32_t v2, const char *s, int32_t bx0, int32_t by0, int32_t bx1,
                      int32_t by1, int32_t ix0, int32_t iy0, int32_t ix1, int32_t iy1)
{
    alstat_t *a = al_stat(tag);
    int32_t ah = h2 == 99 ? 0 : h2 < 0 ? -h2 : h2, av = v2 == 99 ? 0 : v2 < 0 ? -v2 : v2;
    a->n++;
    al_count++;
    if (h2 == 1 || v2 == 1) a->half_hi++;
    if (h2 == -1 || v2 == -1) a->half_lo++;
    if (ah > a->wh2 || (!a->ex_h[0] && h2 != 99)) {
        if (ah > a->wh2) a->wh2 = ah;
        snprintf(a->ex_h, sizeof a->ex_h, "%+.1f '%s' %s ink x %d..%d box %d..%d", h2 / 2.0, s, cur_name, ix0, ix1, bx0, bx1);
    }
    if (av > a->wv2 || (!a->ex_v[0] && v2 != 99)) {
        if (av > a->wv2) a->wv2 = av;
        snprintf(a->ex_v, sizeof a->ex_v, "%+.1f '%s' %s ink y %d..%d box %d..%d", v2 / 2.0, s, cur_name, iy0, iy1, by0, by1);
    }
    if (ah >= 2 || av >= 2) {
        a->nfail++;
        al_fail++;
        if (al_rep && a->nfail <= 3u)
            fprintf(al_rep, "OFF  %-28s %-26s '%s' across %+.1f, up/down %+.1f  (ink %d,%d-%d,%d box %d,%d-%d,%d)\n", tag, cur_name,
                    s, h2 == 99 ? 0.0 : h2 / 2.0, v2 == 99 ? 0.0 : v2 / 2.0, ix0, iy0, ix1, iy1, bx0, by0, bx1, by1);
    }
}
static void hk_align(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t mode, const char *tag)
{
    al_t *a;
    if (al_off) return;                         /* (the draw cost: timed without the check)  */
    if (al_n >= 8u) { al_lost++; return; }
    a = &al_stk[al_n++];
    memset(a, 0, sizeof *a);
    a->x0 = x0; a->y0 = y0; a->x1 = x1; a->y1 = y1; a->mode = mode; a->tag = tag;
    a->left = mode >> 8 ? mode >> 8 : 1u;
    a->ix0 = a->iy0 = 1 << 20; a->ix1 = a->iy1 = -(1 << 20);
}
/* an item drawn (cell: a cv_rrect, else ink): its extent across (x0..x1), up and down (y0..y1; vy 0: none), to the
 * place declared last (AL_PASS, its item done: to the one before it too) */
static void al_item_k(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int vy, const char *s, int cell)
{
    al_t *a;
    int32_t h2 = 99, v2 = 99;
    uint32_t pass;
    if (!al_n) return;
    a = &al_stk[al_n - 1u];
    if (!(a->mode & 32u) != !cell) return;          /* (AL_CELLS: its cells only; else ink only) */
    if (x0 < a->ix0) a->ix0 = x0;
    if (x1 > a->ix1) a->ix1 = x1;
    if (vy) {
        if (y0 < a->iy0) a->iy0 = y0;
        if (y1 > a->iy1) a->iy1 = y1;
        a->nv++;
    }
    if (!a->s[0] || strlen(a->s) + strlen(s) + 2u < sizeof a->s)
        snprintf(a->s + strlen(a->s), sizeof a->s - strlen(a->s), "%s%s", a->s[0] ? "+" : "", s);
    if (--a->left) return;
    al_n--;
    pass = a->mode & 16u;
    if (a->mode & 1u) h2 = (a->ix0 + a->ix1) - (a->x0 + a->x1);       /* AL_H: the centres */
    if (a->mode & 8u) h2 = 2 * (a->ix1 - a->x1);                      /* AL_R: the right edges */
    if (a->nv && (a->mode & 2u)) v2 = (a->iy0 + a->iy1) - (a->y0 + a->y1);
    if (a->nv && (a->mode & 4u)) v2 = 2 * (a->iy1 - a->y1);           /* AL_B: the baselines (capitals' bottoms) */
    al_record(a->tag, h2, v2, a->s, a->x0, a->y0, a->x1, a->y1, a->ix0, a->iy0, a->ix1, a->iy1);
    if (pass) al_item_k(x0, y0, x1, y1, vy, s, cell);
}
static void al_item(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int vy, const char *s) { al_item_k(x0, y0, x1, y1, vy, s, 0); }
static int al_alnum(const char *s)
{
    for (; *s; s++)
        if ((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9')) return 1;
    return 0;
}
static void al_text(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const char *s, uint32_t flags)
{
    if (!al_n || ((flags & 4u) && !strcmp(s, "icon"))) return;      /* (an icon: GFX_HOOK_ICON) */
    if (flags & 4u) al_item(x0, y0, x1, y1, 1, s);                    /* a keycap: its pill */
    else al_item(x0, al_cy0, x1, al_cy1, al_alnum(s), s);
}
static void al_flush(void)                      /* a canvas blitted or begun: declared places left without their items */
{
    uint32_t i;
    for (i = 0; i < al_n; i++)
        if (al_rep && al_lost + i < 12u) fprintf(al_rep, "LOST %-28s %s (declared, nothing drawn after it)\n", al_stk[i].tag, cur_name);
    al_lost += al_n;
    al_n = 0;
}
static int32_t icon_cell(uint32_t *size, uint32_t id);
static void hk_icon(int32_t x, int32_t y, uint32_t size, uint32_t id);   /* (after the icons: their cells' nibbles) */
#define GFX_HOOK_ALIGN(x0, y0, x1, y1, mode, tag) hk_align(x0, (y0) + cv_oy, x1, (y1) + cv_oy, mode, tag)
static uint8_t pen_h;                           /* GFX_HOOK_PEN: the line height of the text being hooked (its face) */
#define GFX_HOOK_PEN(y, f) (pen_h = (f)->h, al_n ? (al_cy0 = (y) + cv_oy + (f)->g[glyph((f), 'H')].by, al_cy1 = al_cy0 + (f)->g[glyph((f), 'H')].bh) : 0)
#define GFX_HOOK_ITEM(x0, y0, x1, y1) al_item(x0, (y0) + cv_oy, x1, (y1) + cv_oy, 1, "cushion")
#define GFX_HOOK_ICON(x, y, size, id) hk_icon(x, (y) + cv_oy, size, id)
static char splash_seen[96];                    /* the splash scene: its texts as drawn, '|' between (draw) */
static uint8_t in_splash;
static void hk_text(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const char *s, uint32_t flags)
{
    al_text(x0, y0, x1, y1, s, flags);
    if (in_splash && strlen(splash_seen) + strlen(s) + 2u < sizeof splash_seen)
        snprintf(splash_seen + strlen(splash_seen), sizeof splash_seen - strlen(splash_seen), "%s%s", splash_seen[0] ? "|" : "", s);
    n_text++;
    if (npend >= 256u) return;
    pend[npend] = (rbox_t){(int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1, (uint8_t)flags, {0}};
    strncpy(pend[npend].s, s, 27);
    pend[npend].fnt = (flags & 4u) ? 'K' : pen_h >= 30u ? 'L' : pen_h >= 18u ? 'M' : 'S';
    pen_h = 0;
    npend++;
}
static void hk_cell(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    al_item_k(x0, y0, x1, y1, 1, "cell", 1);
    if (ncells < 64u) {
        cells[ncells][0] = (int16_t)x0; cells[ncells][1] = (int16_t)y0;
        cells[ncells][2] = (int16_t)x1; cells[ncells][3] = (int16_t)y1;
        ncells++;
    }
}
static void hk_blit(uint32_t x, uint32_t y, uint32_t r0, uint32_t w, uint32_t h);
static void hk_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
/* STYLE LINE: the dividers drawn on the screen (ui_draw.c lcd_rule) while no later blit or fill covers them */
static int16_t rules[32][4];
static uint32_t nrules;
static void hk_rule(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    if (nrules < 32u) {
        rules[nrules][0] = (int16_t)x; rules[nrules][1] = (int16_t)y;
        rules[nrules][2] = (int16_t)(x + w); rules[nrules][3] = (int16_t)(y + h);
        nrules++;
    }
}
static void rules_cover(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    uint32_t i, k = 0;
    for (i = 0; i < nrules; i++)
        if (!(rules[i][0] >= x0 && rules[i][2] <= x1 && rules[i][1] >= y0 && rules[i][3] <= y1))
            memcpy(rules[k++], rules[i], sizeof rules[i]);
    nrules = k;
}
#define GFX_HOOK_TEXT(x0, y0, x1, y1, s, flags) hk_text(x0, y0, x1, y1, s, flags)
#define GFX_HOOK_BLIT(x, y, r0) hk_blit(x, y, r0, cv_w, cv_h)
#define GFX_HOOK_BEGIN() (npend = ncells = 0, al_flush())
#define GFX_HOOK_CELL(x0, y0, x1, y1) hk_cell(x0, y0, x1, y1)
#define GFX_HOOK_RULE(x, y, w, h) hk_rule(x, y, w, h)
static uint32_t nink;                           /* GFX_HOOK_INK: inks drawn on their own colour */
static void hk_ink(uint16_t ink, uint16_t bg);
#define GFX_HOOK_INK(ink, bg) hk_ink(ink, bg)
#define GFX_HOOK_PIXELS(n) (px_visited += (n))
#define UI_FILL_HOOK(x, y, w, h) hk_fill(x, y, w, h)
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static void icon_unpack(const uint8_t *d, uint8_t *out, uint32_t n)
{
    uint32_t k = 0, pos = 0;
    while (k < n) {
        uint32_t code = 0, first = 0, idx = 0, len, s, r;
        for (len = 0; len < 15u; len++) {
            code |= (d[pos >> 3] >> (7u - (pos & 7u))) & 1u;
            pos++;
            if (code - first < AI_HC[len]) break;
            idx += AI_HC[len];
            first = (first + AI_HC[len]) << 1;
            code <<= 1;
        }
        s = AI_HC[15u + idx + code - first];
        for (r = 0; r <= s >> 4 && k < n; r++) out[k++] = (uint8_t)(s & 15u);
    }
}
static void hk_icon(int32_t x, int32_t y, uint32_t size, uint32_t id)
{
    int32_t k, x0 = 99, y0 = 99, x1 = -1, y1 = -1;
    uint32_t i;
    const uint8_t *c;
    uint8_t cell[24 * 24];
    if (!al_n || (k = icon_cell(&size, id)) < 0) return;
    c = size == 24u ? AI24_DATA + AI24_OFF[k] : size == 16u ? AI16_DATA + AI16_OFF[k] : AI12_DATA + AI12_OFF[k];
    icon_unpack(c, cell, size * size);
    for (i = 0; i < size * size; i++)
        if (cell[i]) {
            int32_t px = (int32_t)(i % size), py = (int32_t)(i / size);
            if (px < x0) x0 = px;
            if (px + 1 > x1) x1 = px + 1;
            if (py < y0) y0 = py;
            if (py + 1 > y1) y1 = py + 1;
        }
    if (cv_flip) { int32_t t = x0; x0 = (int32_t)size - x1; x1 = (int32_t)size - t; }   /* (mirrored: the metronome) */
    al_item(x + x0, y + y0, x + x1, y + y1, 1, "icon");
}

/* the FM6 chart lint: each part of a chart (FMH_*) drawn alone onto a canvas of SENT, its pixels kept in fmp_mask
 * (bit: the part), then the canvas as drawn restored with the part on it; at FMH_END the parts are checked */
static void finding(const char *what, const rbox_t *b, const rbox_t *o)
{
    static char seen[600][64];
    static uint32_t nseen;
    char key[64];
    uint32_t i;
    nfind++;
    snprintf(key, sizeof key, "%.20s|%.14s|%.24s", cur_name, what, b->s);
    for (i = 0; i < nseen && strcmp(seen[i], key); i++) ;
    if (i < nseen) return;                       /* each finding once per screen */
    if (nseen < 600u) strcpy(seen[nseen++], key);
    else return;
    fprintf(rep, "LINT %-22s %-32s '%s' (%d,%d)-(%d,%d)", cur_name, what, b->s, b->x0, b->y0, b->x1, b->y1);
    if (o) fprintf(rep, " / '%s' (%d,%d)-(%d,%d)", o->s, o->x0, o->y0, o->x1, o->y1);
    fputc('\n', rep);
}
/* a rectangle of the screen is drawn again: boxes inside it are gone, boxes it covers in part are hidden */
static void cover(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    uint32_t i, k = 0;
    for (i = 0; i < nscr; i++) {
        rbox_t *b = &scr[i];
        int in = b->x0 >= x0 && b->x1 <= x1 && b->y0 >= y0 && b->y1 <= y1;
        int touch = b->x0 < x1 && x0 < b->x1 && b->y0 < y1 && y0 < b->y1;
        if (in) continue;
        if (touch && !(b->flags & 48u)) finding("hidden in part by a later draw", b, 0);
        if (touch && (b->flags & 48u)) scr[k++] = *b;   /* split across bands / scrolled: drawn on purpose */
        else scr[k++] = *b;
    }
    nscr = k;
}
static void hk_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    cover((int32_t)x, (int32_t)y, (int32_t)(x + w), (int32_t)(y + h));
    rules_cover((int32_t)x, (int32_t)y, (int32_t)(x + w), (int32_t)(y + h));
}
/* a text across two bands (the menu and the document draw 124 + 85 rows): each band draws its part, the same
 * screen box twice, cut at the band edge; together they are whole (flag 16) */
static int split_pair(rbox_t *b)
{
    uint32_t j;
    if (!(b->flags & 2u)) return 0;
    for (j = 0; j < nscr; j++) {
        rbox_t *o = &scr[j];
        if ((o->flags & 2u) && o->x0 == b->x0 && o->x1 == b->x1 && o->y0 == b->y0 && o->y1 == b->y1 && !strcmp(o->s, b->s)) {
            o->flags = (uint8_t)((o->flags & ~2u) | 16u);
            return 1;
        }
    }
    return 0;
}
/* a text or icon box that touches a cell or card of its canvas (GFX_HOOK_CELL) but spills past its edge; for the
 * audit, a text inside its smallest cell closer than 2 px to an edge */
static void contain(int32_t x, int32_t y)
{
    uint32_t i, c;
    for (i = 0; i < npend; i++) {
        const rbox_t *b = &pend[i];
        int32_t best = 1 << 30, gap = 99;
        for (c = 0; c < ncells && !(b->flags & 4u); c++) {   /* (texts only: an icon's box is its whole cell) */
            const int16_t *r = cells[c];
            int32_t a = (r[2] - r[0]) * (r[3] - r[1]), g;
            if (b->x0 < r[0] || b->x1 > r[2] || b->y0 < r[1] || b->y1 > r[3] || a >= best) continue;
            best = a;
            g = b->x0 - r[0];
            if (r[2] - b->x1 < g) g = r[2] - b->x1;
            if (b->y0 - r[1] < g) g = b->y0 - r[1];
            if (r[3] - b->y1 < g) g = r[3] - b->y1;
            gap = g;
        }
        if (gap < 2 && aud) {
            ntight++;
            if (strlen(tight) + strlen(b->s) + 8u < sizeof tight)
                snprintf(tight + strlen(tight), sizeof tight - strlen(tight), "%s'%s' %dpx", tight[0] ? ", " : "", b->s, (int)gap);
        }
        for (c = 0; c < ncells; c++) {
            const int16_t *r = cells[c];
            int touch = b->x0 < r[2] && r[0] < b->x1 && b->y0 < r[3] && r[1] < b->y1;
            int in = b->x0 >= r[0] && b->x1 <= r[2] && b->y0 >= r[1] && b->y1 <= r[3];
            int around = r[0] >= b->x0 && r[2] <= b->x1 && r[1] >= b->y0 && r[3] <= b->y1;   /* (a mark inside the box) */
            if (touch && !in && !around) {
                rbox_t t = *b, o = {(int16_t)(r[0] + x), (int16_t)(r[1] + y), (int16_t)(r[2] + x), (int16_t)(r[3] + y), 0, "cell"};
                t.x0 += (int16_t)x; t.x1 += (int16_t)x; t.y0 += (int16_t)y; t.y1 += (int16_t)y;
                nspill++;
                finding("spills out of its cell", &t, &o);
                break;
            }
        }
    }
}
static void hk_blit(uint32_t x, uint32_t y, uint32_t r0, uint32_t w, uint32_t h)
{
    uint32_t i, k = 0;
    al_flush();
    contain((int32_t)x, (int32_t)y);
    ncells = 0;
    blit_px += (uint64_t)w * (h > r0 ? h - r0 : 0u);
    for (i = 0; i < npend; i++) {
        rbox_t b = pend[i];
        if (b.y1 <= (int32_t)r0) continue;           /* rows not blitted */
        if (b.y0 < (int32_t)r0) b.flags |= 2u;
        b.x0 += (int16_t)x; b.x1 += (int16_t)x; b.y0 += (int16_t)y; b.y1 += (int16_t)y;
        if (!split_pair(&b)) pend[k++] = b;
    }
    cover((int32_t)x, (int32_t)(y + r0), (int32_t)(x + w), (int32_t)(y + h));
    rules_cover((int32_t)x, (int32_t)(y + r0), (int32_t)(x + w), (int32_t)(y + h));
    for (i = 0; i < k && nscr < 1200u; i++)
        scr[nscr++] = pend[i];
    npend = 0;
}

/* SONG (ui_graph.c graph_song): its texts only in the lanes (their cushions, the slots' letters, + ADD), the ruler's
 * numbers under its line and ONE line at the panel's foot (at most a text left and one right, on one baseline) */
static void lint(void)
{
    uint32_t i, j;
    for (i = 0; i < nscr; i++) {
        const rbox_t *b = &scr[i];
        if (b->x0 < 0 || b->y0 < 0 || b->x1 > 240 || b->y1 > 240) finding("off the screen", b, 0);
        if ((b->flags & 2u) && !(b->flags & 32u)) finding("cut by its canvas", b, 0);
        if ((b->flags & 1u) && !(b->flags & 8u)) finding("ellipsised (not free text)", b, 0);
        if ((b->flags & 9u) == 9u) {
            for (j = 0; j < nfree_seen && strcmp(free_seen[j], b->s); j++) ;
            if (j == nfree_seen && nfree_seen < 64u) {
                snprintf(free_seen[nfree_seen++], 40, "%s", b->s);
                fprintf(rep, "free   %-22s ellipsised '%s'\n", cur_name, b->s);
            }
            nfree++;
        }
        for (j = i + 1u; j < nscr; j++) {
            const rbox_t *o = &scr[j];
            if (b->x0 < o->x1 && o->x0 < b->x1 && b->y0 < o->y1 && o->y0 < b->y1) finding("overlaps", b, o);
        }
        for (j = 0; j < nrules; j++) {               /* a divider is a cell edge: nothing on it */
            const int16_t *r = rules[j];
            rbox_t o = {r[0], r[1], r[2], r[3], 0, "divider"};
            if (b->x0 < r[2] && r[0] < b->x1 && b->y0 < r[3] && r[1] < b->y1) finding("touches a divider", b, &o);
        }
    }
}

/* the audit of one screen (GREY): its texts (count, ink-box px), icons, keycaps, the ellipsised and tight texts,
 * and the words themselves */
static FILE *audf;
static void audit_scene(const char *name)
{
    uint32_t i, nt = 0, ni = 0, nk = 0, ne = 0, at = 0, ai = 0, ak = 0;
    char words[1200] = "";
    if (!audf) return;
    for (i = 0; i < nscr; i++) {
        const rbox_t *b = &scr[i];
        uint32_t a = (uint32_t)((b->x1 - b->x0) * (b->y1 - b->y0));
        if ((b->flags & 4u) && !strcmp(b->s, "icon")) { ni++; ai += a; continue; }
        if (b->flags & 4u) { nk++; ak += a; continue; }
        nt++; at += a;
        if (b->flags & 1u) ne++;
        if (strlen(words) + strlen(b->s) + 4u < sizeof words)
            snprintf(words + strlen(words), sizeof words - strlen(words), "%s%s", words[0] ? " | " : "", b->s);
    }
    fprintf(audf, "%s\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%s\t%s\n", name, nt, at, ni, ai, nk, ak, ne, ntight, words, tight);
}
/* OUTDIR/font_audit.tsv (GREY, FLAT and LARGE): every text of every screen with its face (S M L), its ink box and
 * whether it was ellipsised */
static FILE *fontf;
static void font_scene(const char *mode, const char *name)
{
    uint32_t i;
    if (!fontf) return;
    for (i = 0; i < nscr; i++) {
        const rbox_t *b = &scr[i];
        if (b->flags & 4u) continue;                 /* (icons, keycaps) */
        fprintf(fontf, "%s\t%s\t%c\t%d\t%d\t%d\t%d\t%s%s\n", mode, name, b->fnt ? b->fnt : '?', b->x0, b->y0, b->x1, b->y1,
                b->s, (b->flags & 1u) ? "\t(ellipsised)" : "");
    }
}
/* GREY: every pixel RGB565 gray (R = B, G = 2 R); MONO (black and white, its edges blended per channel): neutral
 * (R = B, G within 2 of 2 R, the 5-6-5 rounding). Other palettes: nothing */
static uint32_t mono_bad;
static void mono_check(void)
{
    uint32_t i, bw = settings.palette == UI_BW_INDEX;
    if (settings.palette != UI_GREY_INDEX && !bw) return;
    for (i = 0; i < 240u * 240u; i++) {
        uint16_t c = swap16(host_screen[i]);
        int32_t d = (int32_t)((c >> 5) & 63u) - (int32_t)(c >> 11) * 2;
        if ((c >> 11) != (c & 31u) || (bw ? d < -2 || d > 2 : d != 0)) {
            if (!mono_bad) fprintf(rep, "%s %s: pixel %u,%u = %04x off %s\n", bw ? "MONO" : "GREY", cur_name, i % 240u, i / 240u, c,
                                   bw ? "neutral" : "gray");
            mono_bad++;
        }
    }
}
static void hk_ink(uint16_t ink, uint16_t bg)
{
    if (ink != bg) return;
    if (nink < 20u) fprintf(rep, "%s: ink %04x drawn on its own colour (%s)\n", cur_name, ink, UI_PALETTES[settings.palette % NPALETTES].name);
    nink++;
}
static void write_ppm(const char *dir, const char *pal, const char *name)
{
    char path[512];
    uint32_t i;
    FILE *f;
    snprintf(path, sizeof path, "%s/ppm/%s_%s.ppm", dir, pal, name);
    f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "cannot write %s\n", path); return; }
    fprintf(f, "P6\n240 240\n255\n");
    for (i = 0; i < 240u * 240u; i++) {
        uint16_t c = swap16(host_screen[i]);
        uint8_t b[3] = {(uint8_t)((c >> 11) * 255u / 31u), (uint8_t)(((c >> 5) & 63u) * 255u / 63u), (uint8_t)((c & 31u) * 255u / 31u)};
        fwrite(b, 1, 3, f);
    }
    fclose(f);
}

/* ------------------------------------------------------------------ the screens --- */
/* a divider as the LINE style drew them (lcd_rule left with the pages' frame): the lint's self-test needs one */
static void lcd_rule(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    lcd_fill(x, y, w, h, T_RULE);
    GFX_HOOK_RULE(x, y, w, h);
}
static void pal(uint32_t p) { settings.palette = p; palette_set(p); }
/* a playing T-1 on the working pattern (its default line), the tempo 124, the bar's 3rd beat */
static void state(void)
{
    ui_power_on();
    t1_glue_init();
    song.playing = 1;
    song.g[G_BPM] = 124;
    clk_pos = 0; clk_step = 8; cin_total = cin_left = 0;   /* the bar's 3rd beat: the header's metronome at rest */
}

enum { S_T1, S_T1_IDLE, S_T1_PAGE2, S_T1_PAGE3, S_T1_PAGE4, S_T1_SHIFT, S_T1_GLOBAL, S_T1_TEMP, S_T1_MUTED, S_T1_TRACK3,
       S_T1_HOT, S_MESSAGE, S_MESSAGE_KEY, S_MESSAGE_NOFILE, S_CONFIRM, S_CONFIRM_PROJ, S_CONFIRM_USER, S_CONFIRM_ERASE,
       S_MENU, S_MENU_SPEAKER, S_ABOUT, S_ABOUT_REC, S_ABOUT_CREDITS, S_ABOUT_END, S_MENU_INFO, S_UBOOT, S_CALIBRATION, S_SPLASH,
       S_BATT0, S_BATT1, S_BATT2, S_BATT3, S_BATT_USB, S_MENU_SLIDE, S_COUNTIN, S_COUNTIN2, S_HEAD_PLAY_REC, S_HEAD_REC_OTHER,
       S_HEAD_BPM_TURN, S_HEAD_DOWNBEAT, S_HEAD_BEAT_2, S_MENU_ROW, S_COUNT = S_MENU_ROW + 28 };
static const char *s_name(int s)
{
    static const char *const N[S_MENU_ROW] = {"t1", "t1_idle", "t1_page2", "t1_page3", "t1_page4", "t1_shift", "t1_global",
        "t1_temp", "t1_muted", "t1_track3", "t1_hot", "message", "message_key", "message_nofile", "confirm", "confirm_project",
        "confirm_user", "confirm_erase", "menu", "menu_speaker", "about", "about_rec", "about_credits", "about_end", "menu_info",
        "uboot", "calibration", "splash", "batt_0", "batt_1", "batt_2", "batt_3", "batt_usb", "menu_slide", "countin",
        "countin_2bars", "head_play_rec", "head_rec_other", "head_bpm_turn", "head_downbeat", "head_beat_2"};
    static char row[24];
    if (s < S_MENU_ROW) return N[s];
    snprintf(row, sizeof row, "menu_row_%02d", s - S_MENU_ROW);
    return row;
}
#define S_NAME_OF(s) s_name((int)(s))
static void t1_page(uint32_t pg) { t1_ui_state.current_page = (uint8_t)pg; }
static void setup(int s)
{
    memset(kb_chn, 0, sizeof kb_chn);
    state();
    t1_ui_state.shift_active = t1_ui_state.global_active = t1_ui_state.temp_active = false;
    t1_ui_state.hot_timeout = 0;
    t1_ui_state.selected_track = 1;
    t1_page(0);
    switch (s) {
    case S_T1: song.octave = 2; song.rec = 1; usb.config = 1; break;
    case S_T1_IDLE: song.playing = 0; song.batt_raw = 570; break;
    case S_T1_PAGE2: t1_page(1); break;
    case S_T1_PAGE3: t1_page(2); break;
    case S_T1_PAGE4: t1_page(3); break;
    case S_T1_SHIFT: t1_ui_state.shift_active = true; break;
    case S_T1_GLOBAL: t1_ui_state.global_active = true; break;
    case S_T1_TEMP: t1_ui_state.temp_active = true; break;
    case S_T1_MUTED: t1_sequencer_ram.active_pattern.tracks[1].muted = 1; break;
    case S_T1_TRACK3: t1_ui_state.selected_track = 3; break;
    case S_T1_HOT: t1_ui_state.hot_encoder = 1; t1_ui_state.hot_timeout = 30; break;
    case S_MESSAGE: ui_say("LOADED ", "07 A VERY LONG PATTERN NAME"); break;
    case S_MESSAGE_KEY: ui_message("[SAVE] HOLD TO UNDO"); break;            /* a message with a keycap */
    case S_MESSAGE_NOFILE: ui_message(MSG_NO_SAMPLE); break;
    case S_CONFIRM: ui.confirm = CF_CLEAR_SEQ; ui.confirm_trk = 2; break;
    case S_CONFIRM_PROJ: ui.confirm = CF_OVR_PROJ; ui.confirm_trk = 0; break;
    case S_CONFIRM_USER: song.playing = 0; ui.confirm = CF_OVR_USER; ui.confirm_trk = 6; break;
    case S_CONFIRM_ERASE: song.playing = 0; ui.confirm = CF_ERASE_USER; ui.confirm_trk = 6; break;
    case S_MENU: ui.menu = 1; ui.menu_sel = 0; song.rec = 1; break;
    case S_MENU_SPEAKER: ui.menu = 1; ui.menu_sel = MI_LOWCUT; settings.lowcut = 2; break;
    case S_ABOUT: ui.menu = 2; ui.menu_scroll = 0; break;
    case S_ABOUT_REC: ui.menu = 2; ui.menu_scroll = 0; song.rec = 1; break;          /* the REC mark beside OCT- BACK */
    case S_ABOUT_CREDITS: ui.menu = 2; ui.menu_scroll = 360; break;
    case S_ABOUT_END: ui.menu = 2; ui.menu_scroll = (uint16_t)menu_scroll_max(); break;
    case S_MENU_INFO:
        ui.menu = 3; ui.menu_sel = MI_INFO; song.cpu_q8 = 87u;   /* (34 %; USB: the host has none, OFF) */
        info.t = fm1_ms - INFO_CPU_MS;
        break;
    case S_UBOOT: ui.uboot = 3; break;
    case S_BATT0: usb.config = 0; song.batt_raw = 500; break;
    case S_BATT1: usb.config = 0; song.batt_raw = 540; break;
    case S_BATT2: usb.config = 0; song.batt_raw = 570; break;
    case S_BATT3: usb.config = 0; song.batt_raw = 600; break;
    case S_BATT_USB: usb.config = 1; usb.suspended = 0; song.batt_raw = 500; break;
    case S_MENU_SLIDE: ui.menu = 1; ui.menu_sel = MI_ACCEL; break;
    case S_COUNTIN: song.playing = 0; song.rec = 1; cin_total = 4; cin_left = 3; break;
    case S_COUNTIN2: song.playing = 0; song.rec = 1; cin_total = 8; cin_left = 8; break;
    case S_HEAD_PLAY_REC: song.rec = 1u << song.sel; break;
    case S_HEAD_REC_OTHER: song.rec = 1u << ((song.sel + 2u) % NTRK); break;
    case S_HEAD_BPM_TURN: song.g[G_BPM] = 128; ui.roll_bpm = 128; ui.bpm_t = 40; break;
    case S_HEAD_DOWNBEAT: clk_step = 1; song.rec = 1u << song.sel; break;
    case S_HEAD_BEAT_2: clk_step = 5; song.rec = 1u << song.sel; break;
    default:
        if (s >= S_MENU_ROW) {                       /* every row of the menu, selected (its tab with it), values moved off 0 */
            ui.menu = 1; ui.menu_sel = (uint8_t)(s - S_MENU_ROW);
            ui_prefs = (s & 1) ? 0xFFu : 0u; settings_hold = (uint8_t)(s % 3); settings.lowcut = (uint8_t)(s % 3);
            settings_leds = (uint8_t)(s % LEDS_COUNT);
        }
        break;
    }
}

static uint8_t t1_scene;                          /* the scene shows the T-1 screen (FB-007: its findings are listed, not failing) */
static uint32_t nt1;
static uint32_t scene_n0;                         /* nfind when the scene began drawing (draw() may find too) */
static void lint_scene(void)
{
    uint32_t n0 = scene_n0;
    lint();
    if (t1_scene) {                               /* (FB-007) */
        nt1 += nfind - n0;
        nfind = n0;
    }
}
static void draw(int s)
{
    scene_n0 = nfind;
    nscr = npend = nrules = 0;
    ntight = 0;
    tight[0] = 0;
    memset(host_screen, 0, sizeof host_screen);
    if (s == S_CALIBRATION) {                     /* the blocking setup screen: its two drawing steps */
        setup_title();
        setup_show("TURN RIGHT", "ALGORITHM");
        return;
    }
    if (s == S_SPLASH) {                          /* the power-on splash (main.c, before the UI): its lines, the version
                                                   * shown FELUCCA_VERSION (run_tests.sh: src/felucca.c's) */
        static const char WANT[] = "Felucca|" FELUCCA_VERSION "|H\xFCgelton Instruments|with community";
        splash_seen[0] = 0;
        in_splash = 1;
        draw_splash();
        in_splash = 0;
        {   /* the square whole on the screen (176 x 176 at 32,32, more than the canvas): its edges and square corners
             * RAISE, BG just outside; its text's ink 16 px in from the left and the top (the bottom: AL_B, the hook) */
            const int32_t q0 = 32, q1 = 32 + 176 - 1;
            uint16_t bg = host_screen[0], ra = host_screen[(uint32_t)(q0 + 1) * 240u + (uint32_t)q0 + 1];
            int32_t i, x, y, ix = 999, iy = 999, bad = ra == bg;
            for (i = q0 - 1; i <= q1 + 1; i++) {
                int32_t in = i >= q0 && i <= q1;
                bad |= host_screen[(uint32_t)(q0 - 1) * 240u + (uint32_t)i] != bg || host_screen[(uint32_t)(q1 + 1) * 240u + (uint32_t)i] != bg
                    || host_screen[(uint32_t)i * 240u + (uint32_t)(q0 - 1)] != bg || host_screen[(uint32_t)i * 240u + (uint32_t)(q1 + 1)] != bg;
                if (in)
                    bad |= host_screen[(uint32_t)q0 * 240u + (uint32_t)i] != ra || host_screen[(uint32_t)q1 * 240u + (uint32_t)i] != ra
                        || host_screen[(uint32_t)i * 240u + (uint32_t)q0] != ra || host_screen[(uint32_t)i * 240u + (uint32_t)q1] != ra;
            }
            for (y = q0; y <= q1; y++)
                for (x = q0; x <= q1; x++)
                    if (host_screen[(uint32_t)y * 240u + (uint32_t)x] != ra) {
                        if (x - q0 < ix) ix = x - q0;
                        if (y - q0 < iy) iy = y - q0;
                    }
            if (bad || ix != 16 || iy != 16) {
                if (rep) fprintf(rep, "FIND %-26s the splash square not whole (edges %s) or its text not 16 px in (left %d, top %d)\n",
                                 cur_name, bad ? "off" : "ok", (int)ix, (int)iy);
                nfind++;
            }
        }
        if (strcmp(splash_seen, WANT)) {
            if (rep) fprintf(rep, "FIND %-26s the splash shows '%s', not '%s'\n", cur_name, splash_seen, WANT);
            nfind++;
        } else {
            static uint8_t said;
            if (!said++) printf("ui_render: the splash shows the version " FELUCCA_VERSION " (FELUCCA_VERSION) and its lines\n");
        }
        return;
    }
    t1_scene = !ui.menu && !ui.confirm && !ui.uboot && !seq_counting() && !cin_total;
    ui.force = 1;
    mt.frame = ui.frame + 7u;                         /* (the MENU's tabs settled, as when it opens) */
    if (s == S_MENU_SLIDE) mt.pos = 0, mt.frame = ui.frame;   /* (one step of the slide from DISPLAY) */
    ui_draw();
}

/* the sweeps of what the menu can show: every row with its values, TUNE, MIDI IN, INFO's USB states and CPU, the tabs
 * sliding, the count-in beats, the dialog; for the alignment check (not lint scenes: their draws pile up) */
static void align_sweeps(void)
{
    uint32_t st, i, k, nf = nfind;
    FILE *r0 = rep, *nul = fopen("/dev/null", "w");
    if (nul) rep = nul;
    for (st = ST_FLAT; st <= ST_LINE; st++) {
        state(); pal(UI_GREY_INDEX);
        ui_style = (uint8_t)st; style_apply();
        cur_name = st ? "sweep LINE: cushions" : "sweep FLAT: cushions";
        for (k = 0; k < NTRK; k++)
            for (i = 0; i < 3u; i++) {
                cv_begin(40, 40, T_SURF);
                cv_icon_on(4, 4, i == 0u ? 12u : i == 1u ? 16u : 24u, trk_icon(k, 1), T_ACCENT, T_SURF);
            }
        cur_name = st ? "sweep LINE: dialogs" : "sweep FLAT: dialogs";
        for (i = CF_CLEAR_SEQ; i <= CF_ERASE_USER; i++)
            for (k = 0; k < 4u; k++) {
                ui.confirm = (uint8_t)i; ui.confirm_trk = (uint8_t)k;
                draw_confirm();
            }
        ui.confirm = 0;
        cur_name = st ? "sweep LINE: menu rows" : "sweep FLAT: menu rows";
        ui.menu = 1;
        for (i = 0; i < MI_COUNT; i++)
            for (k = 0; k < 4u; k++) {
                ui.menu_sel = (uint8_t)i; ui_prefs = k & 1u ? 0xFFu : 0u; settings_hold = (uint8_t)k; settings.lowcut = (uint8_t)(k % 3u);
                ui_rec_prefs = (uint8_t)((k % 3u) | (k % 3u) << 2 | ((k + 1u) % 3u) << 4);   /* (CLICK, LEVEL, COUNT-IN: each value) */
                settings_leds = (uint8_t)(k % LEDS_COUNT); ui.force = 1;   /* (every LEDS name) */
                draw_menu();
            }
        ui_prefs = 0; ui_rec_prefs = 0;
        cur_name = st ? "sweep LINE: menu TUNE / MIDI IN" : "sweep FLAT: menu TUNE / MIDI IN";
        for (k = 0; k < 2u * TUNE_N; k++) {             /* every TUNE (the cents, signed), selected or not, S and M */
            ui.menu_sel = (uint8_t)(k & 2u ? MI_TUNE : MI_LOWCUT); ui_prefs = k & 1u ? PREF_LARGE : 0u;
            song.g[G_TUNE] = (int16_t)((int32_t)(k / 2u) + TUNE_MIN); ui.force = 1;
            draw_menu();
        }
        for (k = 0; k < 4u * NELEM(N_ROUTE); k++) {     /* every MIDI IN, selected or not, S and M */
            ui.menu_sel = MI_MIDIIN; ui_prefs = k & 1u ? PREF_LARGE : 0u;
            song.g[G_ROUTE] = (int16_t)(k / 4u); ui.force = 1;
            draw_menu();
        }
        song.g[G_TUNE] = 0; song.g[G_ROUTE] = 0;
        cur_name = st ? "sweep LINE: INFO" : "sweep FLAT: INFO";   /* (1.2: every USB state, the CPU 0..99 %, S and M) */
        {
            uint8_t up0 = usb.up, cf0 = usb.config;
            uint32_t su0 = usb.setups, sof0 = usb.sof_seen;
            ui.menu = 3;
            for (k = 0; k < 5u * 2u * 10u; k++) {
                uint32_t u = k % 5u;
                usb.up = (uint8_t)(u != 0u); usb.config = (uint8_t)(u == 4u); usb.setups = u >= 3u; usb.sof_seen = u >= 2u;
                song.cpu_q8 = (k / 10u) * 28u; info.t = fm1_ms - INFO_CPU_MS;
                ui_prefs = k / 5u & 1u ? PREF_LARGE : 0u; ui.force = 1;
                draw_menu();
            }
            usb.up = up0; usb.config = cf0; usb.setups = su0; usb.sof_seen = sof0; song.cpu_q8 = 0;
            ui.menu = 1;
        }
        cur_name = st ? "sweep LINE: menu tabs sliding" : "sweep FLAT: menu tabs sliding";
        for (i = 0; i < MI_COUNT; i++)                  /* every tab, from every place of the bar, S and M */
            for (k = 0; k <= (MTAB_COUNT - 1u) * 16u; k++) {
                ui.menu_sel = (uint8_t)i; ui_prefs = k & 1u ? PREF_LARGE : 0u;
                mt.pos = (int16_t)(k * 4u); mt.frame = ui.frame - 1u; ui.force = 1;
                draw_menu();
            }
        ui.menu = 0; ui_prefs = 0; ui_rec_prefs = 0; settings_hold = 0; settings.lowcut = 0; settings_leds = 0;
        cur_name = st ? "sweep LINE: count-in" : "sweep FLAT: count-in";
        for (i = 1; i <= 2u; i++)                       /* 1 and 2 bars, every beat */
            for (k = 4u * i; k >= 1u; k--) {
                cin_total = (uint8_t)(4u * i); cin_left = (uint8_t)k; ui.force = 1;
                draw_countin();
            }
        cin_total = cin_left = 0;
        ui.force = 0;
    }
    ui_style = ST_FLAT; style_apply();
    state();
    nscr = npend = nrules = 0;
    nfind = nf;
    rep = r0;
    if (nul) fclose(nul);
}

/* the keycaps' labels in their pills (tools/gen_aa_keycaps.py), from the data: the label's ink (nibbles 6..15) in the
 * pill (KC_H rows, its width) */
static void al_keycaps(void)
{
    uint32_t id;
    cur_name = "keycap data";
    for (id = 0; id < KC_COUNT; id++) {
        const kc_t *k = &KC[id];
        int32_t x, y, x0 = 999, y0 = 999, x1 = -1, y1 = -1, w = k->w;
        uint32_t i = 0;
        for (y = 0; y < KC_H; y++)
            for (x = 0; x < w; x++, i++)
                if (((KC_DATA[k->off + (i >> 1)] >> ((i & 1u) ? 0 : 4)) & 15u) >= 6u) {
                    if (x < x0) x0 = x;
                    if (y < y0) y0 = y;
                    if (x + 1 > x1) x1 = x + 1;
                    if (y + 1 > y1) y1 = y + 1;
                }
        al_record("keycap label in its pill", (x0 + x1) - w, (y0 + y1) - KC_H, k->label, 0, 0, w, KC_H, x0, y0, x1, y1);
        if (al_rep) fprintf(al_rep, "keycap %-8s w %2d  ink x %2d..%2d y %d..%d  across %+.1f up/down %+.1f\n", k->label, w, x0, x1, y0, y1,
                            ((x0 + x1) - w) / 2.0, ((y0 + y1) - KC_H) / 2.0);
    }
}
static void al_table(void)
{
    uint32_t i;
    if (!al_rep) return;
    fprintf(al_rep, "\n%-30s %8s %5s %5s %5s %6s %6s\n", "place", "measured", "1px+", "-0.5", "+0.5", "across", "up/dn");
    for (i = 0; i < al_nst; i++) {
        const alstat_t *a = &al_st[i];
        if (!strcmp(a->tag, "self-test")) continue;
        fprintf(al_rep, "%-30s %8u %5u %5u %5u %6.1f %6.1f\n", a->tag, a->n, a->nfail, a->half_lo, a->half_hi, a->wh2 / 2.0,
                a->wv2 / 2.0);
        if (a->ex_h[0]) fprintf(al_rep, "    across  worst %s\n", a->ex_h);
        if (a->ex_v[0]) fprintf(al_rep, "    up/down worst %s\n", a->ex_v);
    }
    fprintf(al_rep, "\n%u measurements, %u places, %u off by 1 px or more; %u declared places drawn without their items\n",
            al_count, al_nst, al_fail, al_lost);
}

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "build/ui_new";
    char path[600];
    uint32_t p, s;
    static const char *const SHOW[] = {"GREY", "MONO", "GREEN", "PAPER", "NIGHT"};   /* (NIGHT: #50, to review) */
    snprintf(path, sizeof path, "%s/report.txt", out);
    rep = fopen(path, "w");
    if (!rep) { fprintf(stderr, "cannot write %s\n", path); return 1; }
    {
        char ap[600];
        snprintf(ap, sizeof ap, "%s/align.txt", out);
        al_rep = fopen(ap, "w");
        if (al_rep) fprintf(al_rep, "alignment: the ink of each centred text / icon / keycap against its box (gfx.c GFX_HOOK_ALIGN);"
                            " offsets in px, + right / down\n\n");
    }
    {
        char ap[600];
        snprintf(ap, sizeof ap, "%s/text_audit.tsv", out);
        audf = fopen(ap, "w");
    }
    {
        char ap[600];
        snprintf(ap, sizeof ap, "%s/font_audit.tsv", out);
        fontf = fopen(ap, "w");
        if (fontf) fprintf(fontf, "mode\tscreen\tface\tx0\ty0\tx1\ty1\ttext\n");
    }
    if (audf) fprintf(audf, "screen\ttexts\ttext_px\ticons\ticon_px\tkeycaps\tkeycap_px\tellipsised\ttight\twords\ttight_list\n");
    {   /* the lint itself: an overlap, a value too wide for its column, a text past its canvas, one past its cell */
        state(); pal(0); cur_name = "self-test";
        nscr = npend = 0;
        cv_begin(COL_W, COL_H, T_BG);
        cv_text(0, 0, &AF_S, "AAAA", T_TEXT);
        cv_text(10, 4, &AF_S, "BBBB", T_TEXT);
        cv_text_fit(0, 15, &AF_M, "WWWWWWWW", T_THEME, T_BG, COL_W);
        cv_text(40, 30, &AF_M, "CUT", T_THEME);
        cv_rrect(2, 31, 20, 13, 3, T_SURF, T_BG);          /* a cell narrower than its word */
        cv_text(4, 31, &AF_S, "SPILL", T_TEXT);
        cv_blit(4, Y_LABEL);
        lint();
        nrules = 0;                                         /* a word on a divider (STYLE LINE) */
        lcd_rule(0, 120, 240, 1);
        cv_begin(40, 16, T_BG);
        cv_text(2, 0, &AF_S, "RULE", T_TEXT);
        cv_blit(8, 112);
        lint();
        nrules = 0;
        {   /* the alignment check: a word 2 px right of its box's centre, one centred */
            uint32_t f0 = al_fail;
            cv_begin(60, 20, T_BG);
            GFX_HOOK_ALIGN(0, 0, 60, 20, AL_HV, "self-test");
            cv_text_in(4, 0 + CAP_IN(S, 20), 60, &AF_S, "OFF", T_TEXT, T_BG);
            GFX_HOOK_ALIGN(0, 0, 60, 20, AL_HV, "self-test");
            cv_text_in(0, 0 + CAP_IN(S, 20), 60, &AF_S, "MID", T_TEXT, T_BG);
            if (al_fail - f0 != 1u) { fprintf(stderr, "ui_render: the alignment check missed its self-test (%u)\n", al_fail - f0); return 1; }
            al_fail = f0;
            al_count -= 2u;
            npend = 0;
        }
        if (nfind < 5u || !nspill) { fprintf(stderr, "ui_render: the lint missed its self-test (%u)\n", nfind); return 1; }
        fprintf(rep, "(self-test: %u findings above are expected)\n\n", nfind);
        nfind = 0;
    }
    for (p = 0; p < NPALETTES; p++)
        for (s = 0; s < S_COUNT; s++) {
            char name[64];
            uint32_t k;
            snprintf(name, sizeof name, "%s/%s", UI_PALETTES[p].name, S_NAME_OF(s));
            cur_name = name;
            setup((int)s);
            pal(p);
            if (p == UI_GREY_INDEX) aud = audf;
            draw((int)s);
            lint_scene();
            if (p == UI_GREY_INDEX) { audit_scene(S_NAME_OF(s)); font_scene("FLAT", S_NAME_OF(s)); aud = 0; }
            mono_check();
            for (k = 0; k < sizeof SHOW / sizeof SHOW[0]; k++)
                if (!strcmp(UI_PALETTES[p].name, SHOW[k])) write_ppm(out, SHOW[k], S_NAME_OF(s));
        }
    {   /* STYLE LINE (gfx.c ST_*): every screen in every palette, linted (GREY: gray, MONO: neutral); GREY MONO NIGHT
         * PAPER as OUTDIR/ppm/<STYLE>_<PALETTE>_<screen>.ppm (ui_render.py: sheet_<STYLE>_<PALETTE>.png) */
        static const char *const ST_N[2] = {"FLAT", "LINE"}, *const ST_PAL[] = {"GREY", "MONO", "NIGHT", "PAPER"};
        uint32_t st, k, n0 = nfind, nruled = 0;
        for (st = ST_LINE; st <= ST_LINE; st++)
            for (p = 0; p < NPALETTES; p++) {
                for (k = 0; k < sizeof ST_PAL / sizeof ST_PAL[0] && strcmp(UI_PALETTES[p].name, ST_PAL[k]); k++) ;
                for (s = 0; s < S_COUNT; s++) {
                    char name[64], tag[24];
                    snprintf(tag, sizeof tag, "%s_%s", ST_N[st], UI_PALETTES[p].name);
                    snprintf(name, sizeof name, "%s/%s", tag, S_NAME_OF(s));
                    cur_name = name;
                    setup((int)s);
                    pal(p);
                    ui_style = (uint8_t)st; style_apply();
                    draw((int)s);
                    if (ux.style != st) { fprintf(stderr, "ui_render: %s drawn in style %u\n", name, ux.style); return 1; }
                    nruled += nrules != 0u;
                    lint_scene();
                    mono_check();
                    if (k < sizeof ST_PAL / sizeof ST_PAL[0]) write_ppm(out, tag, S_NAME_OF(s));
                }
            }
        ui_style = ST_FLAT; style_apply();
        fprintf(rep, "STYLE LINE (every palette): %u lint findings; %u screens with dividers (no page frame draws them since R3b)\n",
                nfind - n0, nruled);
    }
    al_off = 1;
    /* draw cost: a full redraw of a screen (all strips), and the T-1 screen frame by frame */
    {
        static const int COST[] = {S_T1, S_T1_PAGE2, S_MENU, S_ABOUT_CREDITS, S_CONFIRM_PROJ};
        enum { N = 200 };
        uint32_t i, k;
        fprintf(rep, "\ndraw cost on the host (cc -O1; only the ratios mean anything on the device):\n");
        fprintf(rep, "  screen            full redraw us   texts+icons   glyph px read\n");
        for (k = 0; k < sizeof COST / sizeof COST[0]; k++) {
            clock_t c0;
            uint64_t px, nt;
            setup(COST[k]); pal(1);
            draw(COST[k]);
            px_visited = n_text = 0;
            c0 = clock();
            for (i = 0; i < N; i++) draw(COST[k]);
            px = px_visited / N; nt = n_text / N;
            fprintf(rep, "  %-16s %15.1f %13llu %15llu\n", S_NAME_OF(COST[k]), (double)(clock() - c0) / CLOCKS_PER_SEC / N * 1e6,
                    (unsigned long long)nt, (unsigned long long)px);
        }
        {
            clock_t c0;
            setup(S_T1); pal(1); draw(S_T1);
            px_visited = n_text = 0;
            c0 = clock();
            for (i = 0; i < N * 4; i++) { ui_draw(); nscr = npend = 0; }
            fprintf(rep, "  T-1 screen, a frame without force (the signature only): %.1f us, %llu texts\n",
                    (double)(clock() - c0) / CLOCKS_PER_SEC / (N * 4) * 1e6, (unsigned long long)(n_text / (N * 4)));
        }
    }
    al_off = 0;
    al_keycaps();
    align_sweeps();
    al_table();
    if (al_rep) fclose(al_rep);
    fprintf(rep, "\nalignment: %u measurements of %u places, %u off by 1 px or more (OUTDIR/align.txt)\n", al_count, al_nst - 1u, al_fail);
    if (audf) fclose(audf);
    if (fontf) fclose(fontf);
    fprintf(rep, "\n%u lint findings on the T-1 screen (FB-007: listed above, not failing)\n", nt1);
    fprintf(rep, "\n%u lint findings, %u inks on their own colour, %u ellipsised free texts, %u GREY / MONO pixels off gray / "
            "neutral\n", nfind, nink, nfree, mono_bad);
    fclose(rep);
    printf("ui_render: %u screens x %u palettes; %u lint findings, %u ellipsised free texts (%u distinct), "
           "%u inks on their own colour, %u GREY / MONO pixels off gray / neutral; report %s\n",
           (unsigned)S_COUNT, (unsigned)NPALETTES, nfind, nfree, nfree_seen, nink, mono_bad, path);
    printf("ui_render: alignment: %u measurements of %u places, %u off by 1 px or more%s\n", al_count, al_nst - 1u, al_fail,
           al_fail ? " (see align.txt)" : "");
    return nfind || nink || mono_bad || al_fail ? 1 : 0;
}
