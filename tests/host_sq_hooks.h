/* SPDX-License-Identifier: GPL-3.0-only
 * Host hooks for the sequencer core (P4-012, ADR-0003 16.1): the host tests link the REAL sq_*.c next to the real seq.c / ui*.c, as the firmware
 * does (felucca.c). Included by tests/hostsim.c right after seq.c.
 *
 * What the core needs from the device, and where the host gets it:
 *   midi_out_event_at / midi_out_free   usb.c's own ring (midi_out_q): a test sets usb.config = 1 and reads mo_r..mo_w (the capture)
 *   beat_samples / song.g[G_BPM]        fx.c
 *   trk_note_on / trk_note_off          voice.c (the preview parts)
 *   clkx_*                              clk_ext.c (hostsim.c includes it before seq.c: events_block feeds it)
 *   memset / memcpy                     the hosted libc
 *   SQ_IRQ_OFF / SQ_IRQ_ON              no interrupts on the host: nothing to mask
 * ui_sq.c / ui_sq_glue.c (the screen and its controls) are included by the tests that build the UI (tests/ui_test.c). */
#ifndef HOST_SQ_HOOKS_H
#define HOST_SQ_HOOKS_H
#define SQ_IRQ_OFF() ((void)0)
#define SQ_IRQ_ON()  ((void)0)
#include "../firmware/src/torso_t1_algo.c"
#include "../firmware/src/sq_types.h"
#include "../firmware/src/sq_prims.h"
#include "../firmware/src/sq_prims.c"
#include "../firmware/src/sq_algo_acid.c"
#include "../firmware/src/sq_algos.c"
#define SQ_IS_MASTER() clkx_master()
#include "../firmware/src/sq_core.c"
#include "../firmware/src/sq_chan.c"
#endif
