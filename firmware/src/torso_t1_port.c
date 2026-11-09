/* SPDX-License-Identifier: GPL-3.0-only
 * Torso T-1 on the FM-1: what the T-1 sources take from a hosted libc, for the freestanding firmware build
 * (felucca.c defines T1_DEVICE; the host tests build the same sources against the system libc instead).
 * snprintf: only the conversions torso_t1_ui.c uses: %s %u %d %+d %0NX (N <= 8). Always NUL-terminates. */
#include <stdarg.h>

static void t1p_put(char *d, unsigned size, unsigned *n, char c)
{
    if (*n + 1u < size)
        d[*n] = c;
    (*n)++;
}

int snprintf(char *d, unsigned size, const char *f, ...)
{
    va_list ap;
    unsigned n = 0;
    va_start(ap, f);
    while (*f) {
        char c = *f++, tmp[12];
        unsigned width = 0, plus = 0, zero = 0, len = 0, base = 10, upper = 0;
        uint32_t u;
        if (c != '%') {
            t1p_put(d, size, &n, c);
            continue;
        }
        if (*f == '+') { plus = 1; f++; }
        if (*f == '0') { zero = 1; f++; }
        while (*f >= '0' && *f <= '9')
            width = width * 10u + (unsigned)(*f++ - '0');
        c = *f ? *f++ : 0;
        if (c == 's') {
            const char *s = va_arg(ap, const char *);
            while (s && *s)
                t1p_put(d, size, &n, *s++);
            continue;
        }
        if (c == 'c') {
            t1p_put(d, size, &n, (char)va_arg(ap, int));
            continue;
        }
        if (c == 'd') {
            int32_t v = va_arg(ap, int32_t);
            if (v < 0) { t1p_put(d, size, &n, '-'); u = (uint32_t)(-v); }
            else { if (plus) t1p_put(d, size, &n, '+'); u = (uint32_t)v; }
        } else if (c == 'u' || c == 'X' || c == 'x') {
            u = va_arg(ap, uint32_t);
            if (c != 'u') { base = 16; upper = c == 'X'; }
        } else {                                  /* %% or an unsupported conversion: print it as is */
            t1p_put(d, size, &n, c ? c : '%');
            continue;
        }
        do {
            uint32_t r = u % base;
            tmp[len++] = (char)(r < 10u ? '0' + r : (upper ? 'A' : 'a') + r - 10u);
            u /= base;
        } while (u && len < sizeof tmp);
        while (width > len && width-- > 0)
            t1p_put(d, size, &n, zero ? '0' : ' ');
        while (len)
            t1p_put(d, size, &n, tmp[--len]);
    }
    if (size)
        d[n < size ? n : size - 1u] = 0;
    va_end(ap);
    return (int)n;
}
