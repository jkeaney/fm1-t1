/* measure_body.c: appended to the test harness prefix of tests/ui_sq_test.c by measure.sh. It instruments gfx.c (text, ink colours, rects) and
 * dumps fonts, palettes, every text drawn per scenario, and the Z4 cell layout, as TSV. Instrumentation only: no firmware or TSV file is edited. */
#define MH_MAXREC 4000
typedef struct { int kind, x0, y0, x1, y1, pen_y, maxw, flags, blit; unsigned fg, bg, rw, rh, rc; char s[64], orig[64]; } MRec;
static MRec mrec[MH_MAXREC]; static int nmrec, mpend0;
static const char *mh_orig; static int mh_maxw; static unsigned mh_fg, mh_bg; static int mh_peny; static const void *mh_pf;
void mh_ink(unsigned fg, unsigned bg) { mh_fg = fg; mh_bg = bg; }
void mh_pen(int y, const void *f) { mh_peny = y; mh_pf = f; }
void mh_text(int x0, int y0, int x1, int y1, const char *s, unsigned flags)
{   MRec *r; if (nmrec >= MH_MAXREC) return; r = &mrec[nmrec++]; memset(r, 0, sizeof *r);
    r->kind = 0; r->x0 = x0; r->y0 = y0; r->x1 = x1; r->y1 = y1; r->pen_y = mh_peny; r->flags = (int)flags; r->fg = mh_fg; r->bg = mh_bg; r->maxw = mh_orig ? mh_maxw : 0;
    strncpy(r->s, s, 63); if (mh_orig) strncpy(r->orig, mh_orig, 63); r->blit = -1; }
void mh_blit(unsigned x, unsigned y) { for (int i = mpend0; i < nmrec; i++) mrec[i].blit = (int)y; mpend0 = nmrec; }
static int32_t mh_fit(int32_t x, int32_t y, const aafont_t *f, const char *s, uint16_t fg, uint16_t bg, int32_t maxw)
{   int32_t r; mh_orig = s; mh_maxw = maxw; r = cv_text_fit(x, y, f, s, fg, bg, maxw); mh_orig = 0; return r; }
static void mh_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c)
{   if (nmrec < MH_MAXREC) { MRec *r = &mrec[nmrec++]; memset(r, 0, sizeof *r); r->kind = 1; r->x0 = x; r->y0 = y; r->rw = (unsigned)w; r->rh = (unsigned)h; r->rc = c; r->blit = -1; }
    cv_rect(x, y, w, h, c); }
static void mh_frame(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c)
{   if (nmrec < MH_MAXREC) { MRec *r = &mrec[nmrec++]; memset(r, 0, sizeof *r); r->kind = 2; r->x0 = x; r->y0 = y; r->rw = (unsigned)w; r->rh = (unsigned)h; r->rc = c; r->blit = -1; }
    cv_frame(x, y, w, h, c); }
#define cv_text_fit mh_fit
#define cv_rect mh_rect
#define cv_frame mh_frame
#include "../firmware/src/ui_sq.c"
#undef cv_rect
#undef cv_frame

static void frame(void) { fm1_ms += 17; ui_sq_input(); ui_sq_draw(); }
static FILE *ft, *fl, *ff, *fp, *fs;
static uint32_t ux16(uint16_t c) { return c; }
static void dump_scen(const char *name, uint8_t page)
{   nmrec = 0; mpend0 = 0; uisq_page = page; ui.force = 1; frame(); ui.force = 0;
    for (int i = 0; i < nmrec; i++) { MRec *r = &mrec[i];
        if (r->kind == 0) {
            const char *full = r->orig[0] ? r->orig : r->s;
            fprintf(ft, "%s\t%d\tT\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%04X\t%04X\t%s\t%s\t%d\t%d\n", name, r->blit, r->x0, r->y0, r->x1, r->y1, r->pen_y, r->maxw, r->flags & 1,
                    (int)(text_w(&AF_S, full)), r->fg, r->bg, r->s, full, (int)text_w(&AF_S, r->s), r->x1 - r->x0);
        } else
            fprintf(ft, "%s\t%d\t%c\t%d\t%d\t%u\t%u\t0\t0\t0\t0\t%04X\t0000\t-\t-\t0\t0\n", name, r->blit, r->kind == 1 ? 'R' : 'F', r->x0, r->y0, r->rw, r->rh, r->rc); }
    /* the Z4 layout of this scenario */
    { uint8_t lp = uisq_layout_page(); for (uint8_t row = 0; row < 2; row++) { int16_t w[6]; int x = 0; uisq_btn_widths(lp, row, uisq_sel, w);
        for (uint8_t i = 0; i < 6; i++) { UisqBtn b; uisq_btn_get(&b, lp, (uint8_t)(row * 6 + i), uisq_sel);
            fprintf(fl, "%s\t%d\t%d\t%d\t%d\t%s\t%s\t%d\t%d\t%d\t%d\n", name, row, i, x, w[i], b.tap, b.sub, b.sub_mark, b.dim, b.accent, (int)uisq_btn_need(&b)); x += w[i]; } } } }

static void dump_font(const char *nm, const aafont_t *f)
{   int minh = 99, maxh = 0, widest = 0, wch = 0; long sumw = 0; int nw = 0; char present[300]; int np = 0;
    int ascii = 0;
    for (int c = f->first; c <= f->last; c++) { int k = glyph_at(f, (uint32_t)c); if (k < 0) continue; present[np++] = (char)c; ascii++;
        { const aag_t *g = &f->g[(uint32_t)k << f->psh]; if (g->adv / 16.0 > widest) { widest = (g->adv + 8) >> 4; wch = c; } if (c > 32) { sumw += g->adv; nw++; } } }
    (void)minh; (void)maxh;
    fprintf(ff, "FONT\t%s\tline_h=%d\tbaseline=%d\tfirst=%d\tlast=%d\textras=%d\tphases=%d\tcap_ink_rows=AF_%s_CAP\n", nm, f->h, f->asc, f->first, f->last, f->nex, 1 << f->psh, nm);
    fprintf(ff, "EXTRAS\t%s\t", nm); for (int i = 0; i < f->nex; i++) fprintf(ff, "%d ", f->ex[i]); fprintf(ff, "\n");
    fprintf(ff, "GLYPHS\t%s\tascii_present=%d\twidest_adv_px=%d(%c)\tmean_adv_px(non-space)=%.2f\n", nm, ascii, widest, wch >= 32 ? wch : '?', nw ? sumw / 16.0 / nw : 0.0);
    /* ink rows per sample glyph: top row (by) and bottom row (by+bh-1) relative to the line top, and ink width bw */
    { const char *samp = "HEXxdlgpyq0159.,:;'!~<>=+-/()%[]#*"; for (const char *p = samp; *p; p++) { int k = glyph_at(f, (uint32_t)*p); const aag_t *g; if (k < 0) { fprintf(ff, "SAMPLE\t%s\t%c\tabsent\n", nm, *p); continue; }
        g = &f->g[(uint32_t)k << f->psh];
        fprintf(ff, "SAMPLE\t%s\t%c\tink_top=%d\tink_bot=%d\tink_h=%d\tink_w=%d\tadv=%.2f\n", nm, *p, g->by, g->by + g->bh - 1, g->bh, g->bw, g->adv / 16.0); } }
    { double sw = 0; int n = 0; for (int c = 'A'; c <= 'Z'; c++) { int k = glyph_at(f, (uint32_t)c); if (k >= 0) { sw += f->g[(uint32_t)k << f->psh].adv / 16.0; n++; } } fprintf(ff, "MEAN_ADV\t%s\tA-Z\t%.2f\t(%d glyphs)\n", nm, n ? sw / n : 0, n);
      sw = 0; n = 0; for (int c = 'a'; c <= 'z'; c++) { int k = glyph_at(f, (uint32_t)c); if (k >= 0) { sw += f->g[(uint32_t)k << f->psh].adv / 16.0; n++; } } fprintf(ff, "MEAN_ADV\t%s\ta-z\t%.2f\t(%d glyphs)\n", nm, n ? sw / n : 0, n);
      sw = 0; n = 0; for (int c = '0'; c <= '9'; c++) { int k = glyph_at(f, (uint32_t)c); if (k >= 0) { sw += f->g[(uint32_t)k << f->psh].adv / 16.0; n++; } } fprintf(ff, "MEAN_ADV\t%s\t0-9\t%.2f\t(%d glyphs)\n", nm, n ? sw / n : 0, n); }
    fprintf(ff, "PRESENT\t%s\t", nm); for (int i = 0; i < np; i++) fputc(present[i], ff); fprintf(ff, "\n");
    fprintf(ff, "TEXT_W\t%s\tHELLO WORLD=%d\tACCENT=%d\t0=%d\tW=%d\ti=%d\n", nm, (int)text_w(f, "HELLO WORLD"), (int)text_w(f, "ACCENT"), (int)text_w(f, "0"), (int)text_w(f, "W"), (int)text_w(f, "i")); }

int main(int argc, char **argv)
{   const char *od = argc > 1 ? argv[1] : "build/ui_review"; char p[300];
    snprintf(p, sizeof p, "mkdir -p '%s'", od); if (system(p)) return 1;
    snprintf(p, sizeof p, "%s/texts_all.tsv", od); ft = fopen(p, "w");
    snprintf(p, sizeof p, "%s/btn_layout.tsv", od); fl = fopen(p, "w");
    snprintf(p, sizeof p, "%s/fonts.tsv", od); ff = fopen(p, "w");
    snprintf(p, sizeof p, "%s/palettes.tsv", od); fp = fopen(p, "w");
    snprintf(p, sizeof p, "%s/state.tsv", od); fs = fopen(p, "w");
    fprintf(ft, "scenario\tblit_y\tkind\tx0\ty0\tx1\ty1\tpen_y\tmaxw\tcut\tfull_px\tfg565\tbg565\tdrawn\tfull\tdrawn_px\tink_w\n");
    fprintf(fl, "scenario\trow\tcell\tx\tw\ttap\tsub\tmark\tdim\taccent\tneed_px\n");
    fprintf(fp, "idx\tname\tlight\tmono\tT_BG\tT_SURF\tT_TEXT\tT_THEME\tT_ACCENT\tT_MID\tT_DIM\tT_LINE\tT_SEL\tT_TINT\tT_INK\tT_REC\tT_RAISE\tT_KEY\tT_LANE\tT_GRID\n");
    for (uint32_t i = 0; i < NPALETTES; i++) { palette_set(i);
        fprintf(fp, "%u\t%s\t%d\t%d\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\t%04X\n", i, UI_PALETTES[i].name, ux.light, ux.mono,
                ux.bg, ux.surf, ux.text, ux.theme, ux.accent, ux.mid, ux.dim, ux.line, ux.sel, ux.tint, ux.ink, ux.rec, ux.raise, ux.key, ux.lane, ux.grid); }
    dump_font("S", &AF_S); dump_font("M", &AF_M); dump_font("L", &AF_L);
    fprintf(ff, "CAPS\tS\tcap_y=%d\tcap_h=%d\nCAPS\tM\tcap_y=%d\tcap_h=%d\nCAPS\tL\tcap_y=%d\tcap_h=%d\n", AF_S_CAP_Y, AF_S_CAP_H, AF_M_CAP_Y, AF_M_CAP_H, AF_L_CAP_Y, AF_L_CAP_H);
    palette_set(0); sq_boot(); fm1_ms = 1000;
    fprintf(fs, "default state after sq_boot (palette 0 GREY)\n");
    for (int c = 0; c < SQ_NCH; c++) fprintf(fs, "ch%d algo=%d midi_ch=%d muted=%d len=%d pg_used=%d\n", c + 1, sq_ch[c].algo, sq_ch[c].midi_ch, sq_ch[c].muted, sq_ch[c].len, sq_ch[c].pg_used);
    fprintf(fs, "bpm=%d follow=%d playing=%d\n", sq_bpm, sq_follow, sq_playing);
    /* S01: default power-on PERF */
    uisq_sel = 0; dump_scen("PERF_default", UI_PG_PERF);
    uisq_row = 1; dump_scen("PERF_default_row2", UI_PG_PERF); uisq_row = 0;
    ui_sq_knob_touch(1); dump_scen("PERF_knob_hot", UI_PG_PERF); fm1_ms += 1000; frame();
    /* the render of the test (CH2 unmuted, CH3 MIDI OFF, length 20, edited steps) */
    sq_ch[0].len = 20; sq_ch[0].pg_used = 3; sq_ch[1].muted = 0; sq_ch[2].midi_ch = SQ_MIDI_OFF;
    sq_ch[0].ed[2].vel = 90; sq_ch[0].ed[4].fl = 1; sq_ch[0].ed[7].fl = (uint8_t)(1 | (1 << 2)); sq_ch[0].ed[9].gr = 0x20; sq_rebuild(0);
    dump_scen("PERF_testrender", UI_PG_PERF);
    sq_playing = 1; sq_rt[0].p = 6; sq_rt[0].snd[0] = 1; dump_scen("PERF_playing", UI_PG_PERF);
    uisq_sel = 1; dump_scen("PERF_playing_sel2", UI_PG_PERF); uisq_sel = 0; sq_playing = 0; sq_rt[0].snd[0] = 0;
    uisq_vpage = 1; dump_scen("PERF_viewpage2", UI_PG_PERF); uisq_vpage = 0;
    ui_sq_toast("SAVED P05"); dump_scen("PERF_toast", UI_PG_PERF); uisq_toast_until = 0;
    ui_sq_algo_pick_touch(); dump_scen("PERF_algopick_cue", UI_PG_PERF); uisq_pick_until = 0;
    sq_ch[1].muted = 1; sq_ch[2].midi_ch = 1; dump_scen("PERF_mutes", UI_PG_PERF); sq_ch[1].muted = 0;
    sq_ch[1].muted = 1; sq_ch[1].midi_ch = SQ_MIDI_OFF; dump_scen("PERF_mute_midioff", UI_PG_PERF); sq_ch[1].muted = 0; sq_ch[1].midi_ch = 1;
    /* every page x every algorithm on the selected channel: the truncation survey */
    for (uint8_t pg = 0; pg < UI_PG_COUNT; pg++) { if (pg == UI_PG_MENU) continue;
        for (uint8_t a = 0; a < SQ_NALGO; a++) { char nm[64]; sq_set_algo(0, a); uisq_row = 0; uisq_edit_step = 20;
            snprintf(nm, sizeof nm, "page_%s_algo%d", ui_page_names[pg], a); dump_scen(nm, pg);
            if (pg == UI_PG_PERF) { uisq_row = 1; snprintf(nm, sizeof nm, "page_%s_algo%d_row2", ui_page_names[pg], a); dump_scen(nm, pg); uisq_row = 0; } } }
    sq_set_algo(0, 0);
    /* all four channels on every algorithm (Z1 cell widths) */
    for (uint8_t a = 0; a < SQ_NALGO; a++) { char nm[64]; for (int c = 0; c < SQ_NCH; c++) { sq_set_algo((uint8_t)c, a); sq_ch[c].midi_ch = 15; } snprintf(nm, sizeof nm, "PERF_allch_algo%d_midi16", a); dump_scen(nm, UI_PG_PERF); }
    fclose(ft); fclose(fl); fclose(ff); fclose(fp); fclose(fs);
    return 0; }
