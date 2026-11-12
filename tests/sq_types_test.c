/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_types_test.c (P4-001): the locked data model compiles as C11 with -Wall -Werror, and its sizes are the
 * locked ones (ADR-0003 section 22, LOCK.md section 5). Build and run:
 *   cc -std=c11 -Wall -Werror -Ifirmware/src -o /tmp/sq_types_test tests/sq_types_test.c && /tmp/sq_types_test
 * Exit 0 when every size matches. The header itself asserts the same sizes at compile time; this prints them. */
#include <stdio.h>
#include "sq_types.h"

static int fails;

static void check(const char *name, size_t got, size_t want)
{
    printf("%s %u\n", name, (unsigned)got);
    if (got != want) {
        printf("  FAIL: %s is %u, locked value is %u\n", name, (unsigned)got, (unsigned)want);
        fails++;
    }
}

int main(void)
{
    check("ChanSave", sizeof(ChanSave), 384);
    check("ChanRT", sizeof(ChanRT), 24);
    check("UndoRec", sizeof(UndoRec), 94);
    check("SeqPreset", sizeof(SeqPreset), 1556);
    check("HarmonySave", sizeof(HarmonySave), 8);
    printf("%s\n", fails ? "SQ_TYPES FAIL" : "SQ_TYPES PASS");
    return fails ? 1 : 0;
}
