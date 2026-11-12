/* SPDX-License-Identifier: GPL-3.0-only */
/* Host test of the USB-MIDI out ring in src/usb.c (ADR-0003 8.3, P4-003): midi_out_event (the audio ISR's producer)
 * against the ring's own indices (mo_w, mo_r: what ep1_tx would send, read here as the capture).
 *   size       the out ring holds MQO = 128 packets (the in ring stays MQ = 64); midi_out_free() counts them.
 *   no host    usb.config = 0: nothing is queued and nothing is counted.
 *   reserve    with fewer than MO_RESERVE = 8 free slots a NOTE_ON (status 0x9n, velocity != 0) is dropped and
 *              counted; with exactly 8 free it still queues. NOTE_OFF, velocity-0 NOTE_ON, CC123 and clock (0xF8)
 *              queue while any slot is free. When the ring is full everything is refused and counted.
 *   order      the accepted packets come out in the order they went in, across the wrap of the indices.
 * The SIE is never touched; usb_poll and ep1_tx are not called (the capture reads the ring directly). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define RING_PUBLISH() __asm__ volatile("" ::: "memory")
#define HALF_FRAMES 128
#define FELUCCA_OTA 0
#define FELUCCA_CDC 0
#define FELUCCA_UAC 0
#define FELUCCA_USB_LAYOUT 0
#define FELUCCA_CDC_DEFAULT 0
#define FELUCCA_UAC_48K 0
#define FELUCCA_UAC_AS_FIRST 0
static void fm1_delay_ms(uint32_t ms) { (void)ms; }
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"   /* SIE register macros (never touched here) */
#include "../firmware/src/usb.c"

static int fails;
static void check(const char *what, int ok)
{
    printf("%-72s %s\n", what, ok ? "ok" : "FAIL");
    if (!ok)
        fails++;
}

/* USB-MIDI event packets as seq.c / t1_glue.c build them: CIN | status << 8 | d1 << 16 | d2 << 24 */
static uint32_t n_on(uint32_t n, uint32_t v) { return 0x09u | (0x90u | 0u) << 8 | n << 16 | v << 24; }
static uint32_t n_off(uint32_t n) { return 0x08u | 0x80u << 8 | n << 16; }
static uint32_t clk(void) { return 0x0Fu | 0xF8u << 8; }
static uint32_t cc123(void) { return 0x0Bu | 0xB0u << 8 | 123u << 16; }

/* the capture: what the ring holds from mo_r to mo_w, in order */
static uint32_t got[1024];
static uint32_t ngot;
static void drain(void)
{
    while (mo_r != mo_w) {
        got[ngot++] = midi_out_q[mo_r % MQO];
        mo_r++;
    }
}
static void reset_ring(void)
{
    mo_r = mo_w = 0;
    ngot = 0;
    midi_out_drops = 0;
}

int main(void)
{
    uint32_t exp[256], nexp = 0, i, k, d0, f0;
    int ok;

    /* size and the no-host case */
    reset_ring();
    usb.config = 1;
    check("empty out ring: midi_out_free() = MQO (128), the in ring is still 64", midi_out_free() == 128u && MQ == 64u);
    usb.config = 0;
    midi_out_event(n_on(60, 100));
    midi_out_event(n_off(60));
    check("no host (usb.config = 0): nothing queued, nothing counted",
          mo_w == 0 && midi_out_free() == 128u && midi_out_drops == 0);
    usb.config = 1;

    /* fill with 120 NOTE_OFFs: 8 slots left, the reserve */
    reset_ring();
    for (i = 0; i < 120u; i++) {
        midi_out_event(n_off(i % 128u));
        exp[nexp++] = n_off(i % 128u);
    }
    check("120 NOTE_OFFs queue; 8 slots left (the reserve)", midi_out_free() == 8u && midi_out_drops == 0);

    /* exactly 8 free: a NOTE_ON still queues */
    midi_out_event(n_on(60, 100));
    exp[nexp++] = n_on(60, 100);
    check("NOTE_ON with exactly 8 free queues (the reserve is 'fewer than 8')", midi_out_free() == 7u && midi_out_drops == 0);

    /* 7 free: NOTE_ONs drop; NOTE_OFF, velocity-0 NOTE_ON, CC123 and clock still queue */
    d0 = midi_out_drops;
    midi_out_event(n_on(61, 100));
    midi_out_event(n_on(62, 90));
    check("NOTE_ON with 7 free is dropped and counted (2 drops)", midi_out_drops == d0 + 2u && midi_out_free() == 7u);
    midi_out_event(n_off(60));
    exp[nexp++] = n_off(60);
    midi_out_event(n_on(63, 0));
    exp[nexp++] = n_on(63, 0);
    midi_out_event(cc123());
    exp[nexp++] = cc123();
    midi_out_event(clk());
    exp[nexp++] = clk();
    check("with 7 free: NOTE_OFF, velocity-0 NOTE_ON, CC123, clock queue", midi_out_free() == 3u && midi_out_drops == d0 + 2u);

    /* the last 3 slots: offs and clock keep going until full */
    midi_out_event(n_off(61));
    exp[nexp++] = n_off(61);
    midi_out_event(clk());
    exp[nexp++] = clk();
    midi_out_event(n_off(62));
    exp[nexp++] = n_off(62);
    check("ring full (128): the last slots taken by offs and clock", midi_out_free() == 0u);
    d0 = midi_out_drops;
    midi_out_event(n_off(5));
    midi_out_event(clk());
    midi_out_event(n_on(64, 100));
    check("full ring refuses everything, counted (3 drops)", midi_out_drops == d0 + 3u && midi_out_free() == 0u);

    /* order and contents of the capture */
    drain();
    ok = ngot == nexp;
    for (k = 0; ok && k < nexp; k++)
        ok = got[k] == exp[k];
    check("the capture equals the accepted packets, in order (128 entries)", ok && ngot == 128u);
    check("the ring is empty after the drain", midi_out_free() == 128u);

    /* a flood of NOTE_ONs: 121 queue, the rest of the flood is dropped; the 7 reserved slots take the offs */
    reset_ring();
    for (i = 0; i < 300u; i++)
        midi_out_event(n_on(i % 128u, 100));
    check("300 NOTE_ONs: 121 queue (free 128 .. 8), 179 dropped, 7 slots left",
          midi_out_free() == 7u && midi_out_drops == 179u);
    for (i = 0; i < 7u; i++)
        midi_out_event(clk());
    check("7 clocks after the flood fill the ring and are kept", midi_out_free() == 0u && midi_out_drops == 179u);
    midi_out_event(n_on(1, 100));
    check("a NOTE_ON into the full ring is dropped (one more)", midi_out_drops == 180u);
    drain();
    ok = ngot == 128u;
    for (k = 0; ok && k < 121u; k++)
        ok = (got[k] & 0xF0u << 8) == (0x90u << 8) && ((got[k] >> 24) != 0u);
    for (; ok && k < 128u; k++)
        ok = got[k] == clk();
    check("flood: the 121 NOTE_ONs come first, then the 7 clocks (no ON after a clock)", ok);

    /* wrap: 1000 rounds of mixed packets, drained each round, indices pass 128 many times */
    reset_ring();
    d0 = midi_out_drops;
    f0 = 0;
    ok = 1;
    for (i = 0; i < 1000u; i++) {
        uint32_t a = n_on(i % 16u, 1u + i % 127u), b = n_off(i % 128u), c = clk();
        midi_out_event(a);
        midi_out_event(b);
        midi_out_event(c);
        ngot = 0;
        drain();
        ok &= ngot == 3u && got[0] == a && got[1] == b && got[2] == c;
        f0++;
    }
    check("1000 rounds of ON / OFF / clock across the wrap (mo_w > 128): in order, no drops",
          ok && mo_w > 128u && midi_out_drops == d0 && f0 == 1000u);
    printf("  (indices after the wrap test: mo_w=%u mo_r=%u)\n", (unsigned)mo_w, (unsigned)mo_r);

    printf("%s (%d failures)\n", fails ? "usb_ring_test FAILED" : "usb_ring_test passed", fails);
    return fails ? 1 : 0;
}
