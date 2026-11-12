# SPDX-License-Identifier: GPL-3.0-only
"""make_trace.py: writes scripts/ne_trace_test.c = tests/ui_sq_note_test.c + a draw-op logger, then compiles it to /tmp and runs it.
Nothing under build/ or firmware/ is touched. Output goes to critique_note_edit/data/. Run from the repo root: .venv/bin/python <this>."""
import re, os, subprocess, sys
ROOT = os.getcwd()
D = "docs/design/ui_review/critique_note_edit"
src = open("tests/ui_sq_note_test.c").read()
src = src.replace('#include "../firmware/src/', '#include "firmware/src/')

HOOK = r'''
/* ---- evidence hooks (critique_note_edit): every text / rect / frame / icon op is logged to `oplog`, in screen coordinates, with its strip ---- */
static FILE *oplog;
static const void *lt_f; static uint16_t lt_fg, lt_bg; static int32_t lt_x, lt_y;
static void ne_hook_text(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const char *s, uint32_t fl);
#define GFX_HOOK_TEXT(x0, y0, x1, y1, s, flags) ne_hook_text(x0, y0, x1, y1, s, flags)
#define GFX_HOOK_BLIT(x, y, r0) ((void)0)
#define GFX_HOOK_BEGIN() ((void)0)
#define GFX_HOOK_PIXELS(n) ((void)0)
'''
src = src.replace('#include "firmware/src/gfx.c"', HOOK + '#include "firmware/src/gfx.c"\n' + r'''
static const char *fn_(const void *f) { return f == &AF_S ? "S" : f == &AF_M ? "M" : "L"; }
static void ne_hook_text(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const char *s, uint32_t fl)
{   if (oplog) fprintf(oplog, "T\t%d\t%d\t%d\t%s\t%s\t%u\t%u\t%d\t%d\t%d\t%d\t%u\n", (int)-cv_oy, lt_x, lt_y, fn_(lt_f), s, lt_fg, lt_bg, x0, y0 - (int)cv_oy, x1, y1 - (int)cv_oy, fl); }
static int32_t w_text_on(int32_t x, int32_t y, const aafont_t *f, const char *s, uint16_t fg, uint16_t bg)
{   lt_f = f; lt_fg = fg; lt_bg = bg; lt_x = x; lt_y = y; return (cv_text_on)(x, y, f, s, fg, bg); }
static int32_t w_text_fit(int32_t x, int32_t y, const aafont_t *f, const char *s, uint16_t fg, uint16_t bg, int32_t mw)
{   lt_f = f; lt_fg = fg; lt_bg = bg; lt_x = x; lt_y = y; return (cv_text_fit)(x, y, f, s, fg, bg, mw); }
static void w_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c) { if (oplog) fprintf(oplog, "R\t%d\t%d\t%d\t%d\t%d\t%u\n", (int)-cv_oy, x, y, w, h, c); (cv_rect)(x, y, w, h, c); }
static void w_frame(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c) { if (oplog) fprintf(oplog, "F\t%d\t%d\t%d\t%d\t%d\t%u\n", (int)-cv_oy, x, y, w, h, c); (cv_frame)(x, y, w, h, c); }
static void w_icon(int32_t x, int32_t y, uint32_t id, uint16_t fg) { if (oplog && id < SQI_N) fprintf(oplog, "I\t%d\t%d\t%d\t%u\t%u\t%u\t%u\n", (int)-cv_oy, x, y, id, SQI_TAB[id].w, SQI_TAB[id].h, fg); (cv_icon)(x, y, id, fg); }
static void w_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t c) { if (oplog) fprintf(oplog, "L\t%d\t%d\t%d\t%d\t%d\t%u\n", (int)-cv_oy, x0, y0, x1, y1, c); (cv_line)(x0, y0, x1, y1, c); }
#define cv_text_on(...) w_text_on(__VA_ARGS__)
#define cv_text_fit(...) w_text_fit(__VA_ARGS__)
#define cv_rect(...) w_rect(__VA_ARGS__)
#define cv_frame(...) w_frame(__VA_ARGS__)
#define cv_icon(...) w_icon(__VA_ARGS__)
#define cv_line(...) w_line(__VA_ARGS__)
''')

# the shot(): keep the original PNG, then re-trace the same frame (no time advance), then the same state drawn as PERF
new_shot = r'''static const char *outdir = "build/ui_sq";
static void draw_traced(const char *name)
{   char p[300];
    snprintf(p, sizeof p, "%s/%s.ops", outdir, name); oplog = fopen(p, "w");
    ui.force = 1; ui_sq_draw(); ui.force = 0;
    fclose(oplog); oplog = NULL;
    snprintf(p, sizeof p, "%s/%s.png", outdir, name); png_write(p, fb, 240, 0, 0, 240, 240); }
static void shot(const char *name, int x, int y, int w, int h)
{   char nm[100]; const uint8_t pg = uisq_page; (void)x; (void)y; (void)w; (void)h;
    snprintf(nm, sizeof nm, "%s", name); { char p[300]; snprintf(p, sizeof p, "%s/%s_orig.png", outdir, name); png_write(p, fb, 240, 0, 0, 240, 240); }
    draw_traced(name);                                              /* NOTE_EDIT again: same ms, forced */
    uisq_page = UI_PG_PERF; snprintf(nm, sizeof nm, "%s_perf", name); draw_traced(nm);   /* the same state on the PERF page */
    uisq_page = pg; ui.force = 1; ui_sq_draw(); ui.force = 0; }'''
src, n = re.subn(r'static const char \*outdir = "build/ui_sq";\nstatic void shot\(.*?\n\{.*?\}\n', new_shot + "\n", src, count=1, flags=re.S)
assert n == 1

EXTRA = r'''
    /* ---- evidence: worst-case views drawn through the same element code, and the width tables ---- */
    { UisqView w; FILE *tf; char b[24], gl[32];
      sq_playing = 0; uisq_page = UI_PG_NOTE_EDIT; uisq_edit_ch = 0; uisq_edit_step = 8; fresh(); fill(); w = V;
      /* W1: step 64, every field set to its widest text, all four chips */
      w.ne_step = 64; w.edit = 16; w.ne_set = 15;
      uisq_ne_note_name(w.ne_txt[0], 8, 1);  uisq_ne_gate_text(w.ne_txt[1], 8, 10); uisq_ne_vel_text(w.ne_txt[2], 8, 127); uisq_ne_rat_text(w.ne_txt[3], 8, 4);
      uisq_ne_note_name(b, sizeof b, 8); uisq_ne_gen_line(w.ne_gen[0], 10, b); uisq_ne_gate_text(b, sizeof b, 10); uisq_ne_gen_line(w.ne_gen[1], 10, b);
      uisq_ne_vel_text(b, sizeof b, 99); uisq_ne_gen_line(w.ne_gen[2], 10, b); uisq_ne_rat_text(b, sizeof b, 4); uisq_ne_gen_line(w.ne_gen[3], 10, b);
      uisq_cpy(w.ne_chip[0], 10, ui_ne_text[UI_NE_NO_SLIDE]); uisq_cpy(w.ne_chip[1], 10, "PROB 50"); uisq_cpy(w.ne_chip[2], 10, "OCT+2"); uisq_cpy(w.ne_chip[3], 10, ui_ne_text[UI_NE_OFF]);
      { char p[300]; snprintf(p, sizeof p, "%s/worst_w1.ops", outdir); oplog = fopen(p, "w");
        for (uint32_t s = 0; s < UISQ_NSTRIP; s++) { cv_begin(UISQ_W, UISQ_STRIP_H, T_BG); cv_oy = -(int32_t)uisq_strip_y[s]; uisq_draw_elements(&w); cv_oy = 0; cv_blit(0, uisq_strip_y[s]); }
        fclose(oplog); oplog = NULL; snprintf(p, sizeof p, "%s/worst_w1.png", outdir); png_write(p, fb, 240, 0, 0, 240, 240); }
      /* W2: step 1, negative octave pitch SET (A#-1) with a long gen line, TIE gate, ACC velocity, no chips */
      w = V; w.ne_step = 1; w.edit = 1; w.ne_set = 7;
      uisq_ne_note_name(w.ne_txt[0], 8, 10); uisq_ne_gate_text(w.ne_txt[1], 8, 11); uisq_ne_vel_text(w.ne_txt[2], 8, 120); uisq_cpy(w.ne_txt[3], 8, ui_ne_text[UI_NE_AUTO]);
      uisq_ne_note_name(b, sizeof b, 3); uisq_ne_gen_line(w.ne_gen[0], 10, b); uisq_ne_gate_text(b, sizeof b, 11); uisq_ne_gen_line(w.ne_gen[1], 10, b);
      uisq_ne_vel_text(b, sizeof b, 110); uisq_ne_gen_line(w.ne_gen[2], 10, b); uisq_ne_rat_text(b, sizeof b, 3); uisq_ne_gen_line(w.ne_gen[3], 10, b);
      memset(w.ne_chip, 0, sizeof w.ne_chip);
      { char p[300]; snprintf(p, sizeof p, "%s/worst_w2.ops", outdir); oplog = fopen(p, "w");
        for (uint32_t s = 0; s < UISQ_NSTRIP; s++) { cv_begin(UISQ_W, UISQ_STRIP_H, T_BG); cv_oy = -(int32_t)uisq_strip_y[s]; uisq_draw_elements(&w); cv_oy = 0; cv_blit(0, uisq_strip_y[s]); }
        fclose(oplog); oplog = NULL; snprintf(p, sizeof p, "%s/worst_w2.png", outdir); png_write(p, fb, 240, 0, 0, 240, 240); }
      /* the width tables */
      { char pth[300]; snprintf(pth, sizeof pth, "%s/widths.tsv", outdir); tf = fopen(pth, "w"); }
      fprintf(tf, "kind\tvalue\tstring\tgenline\twS\twM\twL\tgenS\n");
      for (int n = 0; n < 128; n++) { uisq_ne_note_name(b, sizeof b, (uint8_t)n); uisq_ne_gen_line(gl, sizeof gl, b);
          fprintf(tf, "pitch\t%d\t%s\t%s\t%d\t%d\t%d\t%d\n", n, b, gl, (int)text_w(&AF_S, b), (int)text_w(&AF_M, b), (int)text_w(&AF_L, b), (int)text_w(&AF_S, gl)); }
      for (int g = 1; g <= 11; g++) { uisq_ne_gate_text(b, sizeof b, (uint8_t)g); uisq_ne_gen_line(gl, sizeof gl, b);
          fprintf(tf, "gate\t%d\t%s\t%s\t%d\t%d\t%d\t%d\n", g, b, gl, (int)text_w(&AF_S, b), (int)text_w(&AF_M, b), (int)text_w(&AF_L, b), (int)text_w(&AF_S, gl)); }
      for (int v = 1; v <= 127; v++) { uisq_ne_vel_text(b, sizeof b, (uint8_t)v); uisq_ne_gen_line(gl, sizeof gl, b);
          fprintf(tf, "velo\t%d\t%s\t%s\t%d\t%d\t%d\t%d\n", v, b, gl, (int)text_w(&AF_S, b), (int)text_w(&AF_M, b), (int)text_w(&AF_L, b), (int)text_w(&AF_S, gl)); }
      for (int r = 1; r <= 4; r++) { uisq_ne_rat_text(b, sizeof b, (uint8_t)r); uisq_ne_gen_line(gl, sizeof gl, b);
          fprintf(tf, "ratch\t%d\t%s\t%s\t%d\t%d\t%d\t%d\n", r, b, gl, (int)text_w(&AF_S, b), (int)text_w(&AF_M, b), (int)text_w(&AF_L, b), (int)text_w(&AF_S, gl)); }
      for (int k = 0; k < 4; k++) { const char *l = uisq_knob_label(UI_PG_NOTE_EDIT, UI_CTX_ANY, (uint8_t)k);
          fprintf(tf, "label\t%d\t%s\t\t%d\t%d\t%d\t0\n", k, l, (int)text_w(&AF_S, l), (int)text_w(&AF_M, l), (int)text_w(&AF_L, l)); }
      for (int i = 0; i < UI_NE_COUNT; i++) { const char *t = ui_ne_text[i];
          fprintf(tf, "ne_text\t%d\t%s\t\t%d\t%d\t%d\t0\n", i, t, (int)text_w(&AF_S, t), (int)text_w(&AF_M, t), (int)text_w(&AF_L, t)); }
      fclose(tf);
      /* tokens */
      { char pth[300]; snprintf(pth, sizeof pth, "%s/tokens.tsv", outdir); tf = fopen(pth, "w"); }
#define TK(n) fprintf(tf, "%s\t%u\n", #n, (unsigned)n)
      TK(T_BG); TK(T_SURF); TK(T_TEXT); TK(T_THEME); TK(T_ACCENT); TK(T_MID); TK(T_DIM); TK(T_LINE); TK(T_SEL); TK(T_TINT); TK(T_INK); TK(T_REC); TK(T_RAISE); TK(T_KEY); TK(T_LANE); TK(T_GRID);
      TK(T_CH1); TK(T_CH2); TK(T_CH3); TK(T_CH4); TK(T_CHF1); TK(T_CHF2); TK(T_CHF3); TK(T_CHF4); TK(T_PH); TK(T_DICE); TK(T_OFF); TK(UISQ_WHITE); TK(UISQ_INK);
      fclose(tf);
      uisq_page = UI_PG_PERF; }
'''
src = src.replace('    printf("ui_sq_note_test: %d failure(s)\\n", fails);', EXTRA + '\n    printf("ui_sq_note_test: %d failure(s)\\n", fails);')
open(D + "/scripts/ne_trace_test.c", "w").write("/* GENERATED by make_trace.py from tests/ui_sq_note_test.c (evidence only; not part of the build) */\n" + src)
exe = "/tmp/ne_trace_test"
r = subprocess.run(["cc", "-std=gnu11", "-O1", "-w", "-Ibuild/gen", "-Ifirmware/src", "-I.", "-o", exe, D + "/scripts/ne_trace_test.c", "-lm"], capture_output=True, text=True)
print(r.stdout, r.stderr[:3000])
if r.returncode: sys.exit(1)
r = subprocess.run([exe, D + "/data"], capture_output=True, text=True); print(r.stdout[-1500:], r.stderr[-500:])
