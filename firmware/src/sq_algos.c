/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_algos.c: the ALGOS[] table (ADR-0003 section 4): one AlgoDesc per algorithm, indexed by algorithm id (SQ_ALGO_*). Included by the unity
 * build after the algorithm files (sq_algo_acid.c, ...) and before sq_core.c, which reads it. Const, so the ISR and the main loop share it.
 *
 * P5-001 creates the table with SQ_NALGO entries and fills row 0 (ACID). Each later algorithm task (P5-002 EUCLID, P5-003 SNAKE, P5-004 STAGE,
 * P5-007 ARP, P5-008 PATTERNS) includes its file above and adds its own designated row, so a row that does not exist yet stays zero (build and step
 * NULL) and the unity build still compiles. Nothing selects an algorithm id that has no row: sq_core.c is not in the build until the cut-over
 * (P4-012), and the table test of tests/sq_algo_acid_test.c fails when a row it is told exists is missing. Row id must equal its index. */
static const AlgoDesc ALGOS[SQ_NALGO] = {
    [SQ_ALGO_ACID] = ALGO_ACID_ROW,
};
