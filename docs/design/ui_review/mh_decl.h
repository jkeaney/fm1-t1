/* mh_decl.h: forward declarations for the measurement hooks (docs/design/ui_review/measure_body.c). Read-only instrumentation: it changes no firmware. */
#include <stdint.h>
void mh_text(int x0, int y0, int x1, int y1, const char *s, unsigned flags);
void mh_ink(unsigned fg, unsigned bg);
void mh_blit(unsigned x, unsigned y);
void mh_pen(int y, const void *f);
