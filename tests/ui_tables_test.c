// SPDX-License-Identifier: GPL-3.0-only
/* P4-010: build/gen/ui_tables.h (tools/gen_ui_tables.py) against the design TSVs. run_tests.sh passes the TSV line
 * counts (header excluded) as -DT_CONTROL_MAP=... etc. The header must also compile freestanding-clean. */
#include <stdio.h>
#include <string.h>
#include "ui_tables.h"
static int fails;
#define EXPECT(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)
#define ROWS(n, tsv) EXPECT((n) == (tsv), #n " is %d, the TSV has %d rows", (int)(n), (int)(tsv))
int main(void)
{
    ROWS(UI_N_PAGE_STATES, T_PAGE_STATES);
    ROWS(UI_N_CONTROL_MAP, T_CONTROL_MAP);
    ROWS(UI_N_BUTTON_MODES, T_BUTTON_MODES);
    ROWS(UI_N_KNOB_MAP, T_KNOB_MAP);
    ROWS(UI_N_KEY_MAP, T_KEY_MAP);
    ROWS(UI_N_HELP_TEXT, T_HELP_TEXT);
    EXPECT(sizeof ui_page_rows / sizeof ui_page_rows[0] == UI_N_PAGE_STATES, "page array size");
    EXPECT(UI_PG_COUNT == UI_N_PAGE_STATES, "one page id per page_states row");
    EXPECT(UI_C_COUNT == 49, "49 control ids (14 buttons, 7 encoders, MASTER, 27 keys), got %d", (int)UI_C_COUNT);
    /* spot checks: enum <-> name <-> row agree */
    EXPECT(!strcmp(ui_page_names[UI_PG_PERF], "PERF") && ui_page_rows[0].id == UI_PG_PERF, "PERF is the first page");
    EXPECT(!strcmp(ui_ctl_names[UI_C_KEY_B01_FS3], "KEY_B01_F#3"), "sharp keys keep their TSV name");
    EXPECT(!strcmp(ui_ctl_names[UI_C_B_FX], "B_FX"), "B_FX name");
    EXPECT(!strcmp(ui_help_rows[0].line1, "ALL: re-roll every layer") && ui_help_rows[0].ctl == UI_C_B_FX, "help row 0");
    EXPECT(ui_button_rows[0].page == UI_PG_PERF && ui_button_rows[0].ctl == UI_C_B_FX
           && ui_button_rows[0].mode == UI_MODE_TAP && !strcmp(ui_button_rows[0].label, "ALL"), "button row 0");
    for (int i = 0; i < UI_N_KNOB_MAP; i++)
        EXPECT(ui_knob_rows[i].knob < UI_K_COUNT && ui_knob_rows[i].ctx < UI_CTX_COUNT && ui_knob_rows[i].page < UI_PG_COUNT
               && ui_knob_rows[i].label[0], "knob row %d", i);
    for (int i = 0; i < UI_N_HELP_TEXT; i++)
        EXPECT(ui_help_rows[i].ctl < UI_C_COUNT && ui_help_rows[i].page < UI_PG_COUNT && ui_help_rows[i].line1[0]
               && strlen(ui_help_rows[i].line1) <= 34 && strlen(ui_help_rows[i].line2) <= 34, "help row %d", i);
    printf(fails ? "ui_tables_test: %d FAILED\n" : "ui_tables_test: OK (%d failures)\n", fails);
    return fails != 0;
}
