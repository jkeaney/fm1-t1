/* SPDX-License-Identifier: GPL-3.0-only */
/* The firmware copy of the shared primitives (firmware/src/sq_prims.c, static) against the prototype: prints the same dump as
 * `tests/design/prims_test --dump` (tests/sq_prims_dump.h); run_tests.sh compares it byte for byte with tests/sq_prims_golden.txt. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sq_prims.c"
#include "sq_prims_dump.h"

int main(void) { sq_prims_dump(); return 0; }
