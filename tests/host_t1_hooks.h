/* SPDX-License-Identifier: GPL-3.0-only
 * Host hooks for the Torso T-1 (P3-001, ADR-0003 16.1): the host tests link the REAL torso_t1*.c and t1_glue.c, with
 * seq.c / ui*.c compiled as on the device (no t1_mode stub). Included by tests/hostsim.c right after seq.c.
 *
 * What the T-1 needs from the device, and where the host gets it:
 *   midi_out_event   usb.c's own ring (midi_out_q): a test sets usb.config = 1 and reads mo_r..mo_w (the capture)
 *   beat_samples     fx.c (song.g[G_BPM])
 *   synth_note_on/off  t1_glue.c (T1_HOST_GLUE keeps torso_t1.c's weak stubs out)
 *   memset/memcpy/snprintf  the hosted libc (the sources are built without T1_DEVICE)
 * ui_t1.c (the screen and controls) is included by the tests that build the UI (tests/ui_test.c). */
#ifndef HOST_T1_HOOKS_H
#define HOST_T1_HOOKS_H
#define T1_HOST_GLUE 1
#include "../firmware/src/torso_t1.c"
#include "../firmware/src/torso_t1_algo.c"
#include "../firmware/src/torso_t1_pipeline.c"
#include "../firmware/src/torso_t1_state.c"
#include "../firmware/src/torso_t1_ui.c"
#include "../firmware/src/t1_glue.c"
#endif
