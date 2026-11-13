/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_core.c: the sequencer core's scheduler (P4-004; ADR-0003 sections 6, 7, 8 and 14, as amended by section 22).
 *
 * What is here: the 96-slot event queue, sq_emit (the ONLY exit to midi_out_event_at / trk_note_on / trk_note_off), the
 * pairing rule (an ON and its OFF are queued together or not at all), the two-pass dispatch (all OFFs due, then all ONs),
 * sq_step_start (swing, gate, ratchet, slide, tie, hold), sq_tick, sq_all_off, sq_on_start / sq_on_stop, the PLAY
 * accumulator (sq_block) and the ISR half of a queued preset load (sq_apply_bar_boundary). The edit layer's resolver is here too
 * (P4-006: sq_eval resolves StepEdit over the generated layer, sq_slide_out and sq_tie_end place the slide and the TIE). NOT here: the
 * channel state API and the edit writers (P4-005 / P4-006, sq_chan.c), the stamped release ring and exact tick position (P4-007: done
 * below, M1/M2/M4/M7/M9/M10 of TIMING_SPEC), the clock source and its PLL (P4-009), HARMONY (an AF_CHORD algorithm gets an inert chord),
 * live keys.
 *
 * Timing (P4-007, TIMING_SPEC section 3): sq_block finds the sample inside the block where each 192-PPQN tick lands (M1); every
 * packet sq_emit sends goes out as midi_out_event_at(pkt, due) with due = half reference + (block offset + sample + LEAD) * 544
 * TIMER4 ticks (M2), so usb.c releases it at that time and not in the audio interrupt's burst; clock out (0xF8 every 8 ticks, 0xFA
 * at tick 0, 0xFC on stop) goes through the same stamps (M7); the tempo counts in 1/16 samples at the real 44117.65 Hz (M10).
 * sq_half_begin(t0) is the one hook the audio ISR owes this file once per half buffer (P4-012 wires it into fm1_alnk0_irq).
 * Hooks the including file may define first (the host tests do): SQ_EXT_BEAT() = samples per beat of an external clock or 0,
 * SQ_BPM() = the user's tempo. Defaults read fx.c's midi_beat_samples and song.g[].
 *
 * Included by the unity build AFTER the algorithm table (`static const AlgoDesc ALGOS[SQ_NALGO]`, sq_algos.c) and after
 * usb.c / voice.c / fx.c, which supply midi_out_event_at, midi_out_free, MO_RESERVE, trk[], trk_note_on / trk_note_off and
 * the beat length (SQ_EXT_BEAT / SQ_BPM). It is in felucca.c since the cut-over (P4-012: seq.c events_block calls sq_block, audio.c sq_half_begin); tests/sq_core_test.c includes it
 * against mock hooks and a mock ALGOS table. The ISR owns everything below except the double-buffered caches and the
 * staged preset, which the main loop writes (ADR-0003 6). Freestanding: memset / memcpy only. */

#define SQ_GAP        2u                 /* ticks a HOLD note ends before its last pulse boundary (ADR-0003 7.3) */
#ifdef MO_RESERVE
#define SQ_RESERVE    MO_RESERVE         /* usb.c: free slots below which NOTE_ONs are dropped (ADR-0003 8.3) */
#else
#define SQ_RESERVE    8u
#endif
#define SQ_REQ_START   1u                /* sq_req bits, consumed and cleared by the ISR (ADR-0003 6) */
#define SQ_REQ_STOP    2u
#define SQ_REQ_PANIC   4u
#define SQ_REQ_RESTART 8u

#ifndef SEQ_FS_Q4
#define SEQ_FS_Q4     705882u            /* TIMING_SPEC M10: the real sample rate 24 MHz / 544 = 44117.647 Hz, in 1/16 samples per second (flex row) */
#endif
#define SQ_QSTEP      (SQ_PPQN * 16u)    /* sq_acc units per sample: PPQN in the 1/16-sample unit of sq_beat_q4 */
#ifndef SQ_DAC_TICKS
#define SQ_DAC_TICKS  544u               /* TIMER4 ticks per I2S frame (audio.c DAC_TICKS) */
#endif
#ifndef SQ_HALF_FRAMES
#define SQ_HALF_FRAMES 128u              /* the half buffer, in samples (core.h HALF_FRAMES) */
#endif
#ifndef MO_LEAD_SAMPLES
#define MO_LEAD_SAMPLES 128u             /* usb.c: LEAD, M2 */
#endif
#ifndef SQ_IS_MASTER
#define SQ_IS_MASTER() (sq_master)       /* P4-009: the cut-over defines it as clkx_master() (clk_ext.c): clock out is silent in every EXT state (R12) */
#endif
#ifndef SQ_EXT_BEAT
#define SQ_EXT_BEAT() (song.g[G_CLOCK] ? midi_beat_samples : 0u)
#endif
#ifndef SQ_BPM
#define SQ_BPM()      ((uint32_t)song.g[G_BPM])
#endif
#define SQ_NUDGE_MIN  (-512)             /* ticks a half's reference may be pulled back: less than the 544 of one sample, so stamps never go backwards */
#define SQ_RESYNC     24000              /* ticks (1 ms): a larger error of the reference is a restart of the filter, not a drift */

static const uint8_t SQ_DIV_TICKS[6] = { 24, 32, 48, 64, 96, 192 };      /* ticks per step for DIV 0..5 (ADR-0003 5) */
static const HarmChord sq_hc_inert;                                       /* active = 0: HARMONY is inert (wired by its own task) */

static ChanSave  sq_ch[SQ_NCH];           /* main-loop owned; the ISR reads midi_ch, muted, len and pad[0] (the preview byte) */
static SeqGlobal sq_gdb[2];               /* double buffer; the ISR reads sq_gdb[sq_gidx] once per tick */
static volatile uint8_t sq_gidx;
static uint32_t  sq_cache[SQ_NCH][2][SQ_CACHE_MAX / 4];                   /* 4-byte aligned: algorithms cast it to their cache struct */
static uint8_t   sq_calgo[SQ_NCH][2];     /* the algorithm id each buffer was built for: the ISR never reads ChanSave.algo */
static volatile uint8_t sq_cidx[SQ_NCH];
static ChanRT    sq_rt[SQ_NCH];
static SeqEvent  sq_evq[SQ_EVQ_N];
static ChanSave  sq_stage_ch[SQ_NCH];     /* a queued preset (the main loop stages it and builds the inactive caches, then sets pending last) */
static volatile uint8_t sq_load_pending;
static uint8_t   sq_epoch;                /* bumped when a load is applied: sq_rebuild's race guard (P4-005) */
static volatile uint8_t sq_req;
static uint32_t  sq_drops;                /* pairs / events lost to a full queue, NOTE_ONs lost to the reserve rule */
static uint32_t  sq_now;                  /* next tick sq_block will run (ticks since PLAY) */
static uint32_t  sq_cur;                  /* the tick of the call in progress: what a host log stamps its lines with */
static uint32_t  sq_acc;                  /* PPQN * 16 * samples against sq_beat_q4() (the T-1 accumulator of t1_glue.c, in 1/16 samples: M10) */
static uint32_t  sq_entropy_ctr;          /* free-running, also while stopped: the dice seed source (ADR-0003 3.1) */
static uint8_t   sq_playing;
static volatile uint8_t sq_sel;           /* the selected channel: the UI writes (ui_sq_select), the live notes read it once per call */
static uint8_t   sq_live_c[128];          /* live notes (below): 0 = not sounding, else 1 + the channel it went to */
static volatile uint8_t sq_ready;         /* set last by sq_boot (ADR-0003 12); seq.c events_block runs sq_block only when set (seq.c holds the first declaration) */
static uint8_t   sq_off_stage;            /* 1 while sq_all_off still has note-offs or CC123 to send (the ring was short) */
static uint16_t  sq_cc_pend;              /* MIDI channels still owed a CC123 */
static uint32_t  sq_tref;                 /* TIMER4 time at which the current half buffer starts rendering (M2), filtered by sq_half_begin */
static uint8_t   sq_tref_ok;
static uint32_t  sq_boff;                 /* samples of the current half already rendered: the block's offset inside it */
static uint32_t  sq_tick_s;               /* the sample inside the block where the tick in progress lands (M1) */
static uint32_t  sq_due;                  /* the stamp every packet sq_emit sends right now carries */
static uint8_t   sq_master = 1;           /* 1 = this unit is the tempo master: clock out is sent (INT source; P4-009 clears it in the EXT states) */

#define SQ_PRV_ON(c)  ((sq_ch[c].pad[0] & 0x80u) != 0u)                   /* PREVIEW.md: prv = on << 7 | snd, one byte in ChanSave */

/* ------------------------------------------------------------------ stamps (TIMING_SPEC M2) */

/* Once per half buffer, from the audio ISR, with t0 = fm1_ticks() read at its start. Raw t0 jitters with interrupt entry, so the
 * reference runs at the nominal half length and takes a sixteenth of the error each half; an error over 1 ms (a restart, a stall)
 * resynchronises outright. The pull-back is limited to under one sample so the stamps of one half never precede the previous one's. */
static void sq_half_begin(uint32_t t0)
{   if (!sq_tref_ok) { sq_tref = t0; sq_tref_ok = 1; }
    else {
        int32_t e;
        sq_tref += SQ_HALF_FRAMES * SQ_DAC_TICKS;
        e = (int32_t)(t0 - sq_tref);
        if (e > SQ_RESYNC || e < -SQ_RESYNC) sq_tref = t0;
        else { e /= 16; if (e < SQ_NUDGE_MIN) e = SQ_NUDGE_MIN; sq_tref += (uint32_t)e; }
    }
    sq_boff = 0; }

static void sq_stamp_now(void)                                        /* a stop, a panic, anything not on a tick: due at once, no LEAD (M8) */
{   sq_due = sq_tref + sq_boff * SQ_DAC_TICKS; }

static void sq_stamp_tick(uint32_t s)                                 /* sample s of this block, played one half buffer later */
{   sq_due = sq_tref + (sq_boff + s + MO_LEAD_SAMPLES) * SQ_DAC_TICKS; }

static void sq_out(uint32_t pkt) { midi_out_event_at(pkt, sq_due); }

/* Clock out (M7): one byte, CIN 0xF, through the same stamp as a note. Only while this unit is the master (P4-009 owns the source). */
static void sq_rt_byte(uint8_t b) { if (SQ_IS_MASTER()) sq_out(0x0Fu | (uint32_t)b << 8); }

/* ------------------------------------------------------------------ the one exit */

static void sq_emit_cc123(uint8_t m)
{   sq_out(0x0Bu | (0xB0u | (uint32_t)(m & 15u)) << 8 | 123u << 16); }

static void sq_midi_off(uint8_t chan_byte, uint8_t note)
{   sq_out(0x08u | (0x80u | (uint32_t)(chan_byte & 15u)) << 8 | (uint32_t)(note & 0x7Fu) << 16); }

static void sq_snd_drop(ChanRT *r, uint8_t i)             /* free a sounding slot; slot 0 stays the older one */
{   r->snd[i] = 0; r->snd_chan[i] = 0;
    if (i == 0 && r->snd[1]) { r->snd[0] = r->snd[1]; r->snd_chan[0] = r->snd_chan[1]; r->snd[1] = 0; r->snd_chan[1] = 0; } }

/* ADR-0003 8.1. chan: bits 0..3 MIDI channel, bit 4 MIDI OFF, bits 5..6 sequencer channel. Returns 0 only for a NOTE_OFF that
 * the ring could not take (it stays queued and is retried); everything else is final.
 * MIDI gate (NOTE_ON): the channel is not OFF, not muted (read now: decision 111) and the USB ring has SQ_RESERVE slots free
 * (ADR-0003 8.3, P4-003). Preview gate: the channel's prv_on, independent of the MIDI gate. The slot remembers whether MIDI
 * went out (bit 4 of snd_chan set = no) and whether the ON reached the preview voice (bit 7 set = yes, BUG-007: the flag may move before the OFF), so an OFF is sent only when its ON was, and an OFF whose ON was never sent is discarded. */
static uint8_t sq_emit(uint8_t chan, uint8_t kind, uint8_t note, uint8_t vel)
{   uint8_t c = (uint8_t)((chan >> 5) & 3u);
    ChanRT *r = &sq_rt[c];
    note &= 0x7Fu;
    if (kind == 2) {
        uint8_t midi = (uint8_t)(!(chan & 0x10u) && !sq_ch[c].muted);
        uint8_t prv = (uint8_t)SQ_PRV_ON(c);
        uint8_t i;
        if (midi && midi_out_free() < SQ_RESERVE) { sq_drops++; midi = 0; }
        if (!midi && !prv) return 1;
        if (vel < 1) vel = 1;
        if (vel > 127) vel = 127;
        i = r->snd[0] == (uint8_t)(note + 1u) ? 0u : r->snd[1] == (uint8_t)(note + 1u) ? 1u : 2u;
        if (i == 2u && r->snd[0] && r->snd[1]) i = 0u;     /* both slots busy: the older note ends first */
        if (i < 2u) {                                      /* that note is still sounding (a HOLD, a repeat's pitch): off before on, whoever scheduled it */
            if (!(r->snd_chan[i] & 0x10u) && midi_out_free() == 0u) { sq_drops++; return 1; }   /* cannot end it: lose the new ON, never hang a note */
            if (!(r->snd_chan[i] & 0x10u)) sq_midi_off(r->snd_chan[i], (uint8_t)(r->snd[i] - 1u));
            if (r->snd_chan[i] & 0x80u) trk_note_off(&trk[c], (uint32_t)(r->snd[i] - 1u));
            sq_snd_drop(r, i);
        }
        i = r->snd[0] ? 1u : 0u;
        r->snd[i] = (uint8_t)(note + 1u);
        r->snd_chan[i] = (uint8_t)((chan & 0x6Fu) | (midi ? 0u : 0x10u) | (prv ? 0x80u : 0u));
        if (midi) sq_out(0x09u | (0x90u | (uint32_t)(chan & 15u)) << 8 | (uint32_t)note << 16 | (uint32_t)vel << 24);
        if (prv) trk_note_on(&trk[c], note, vel);
        r->act++;
        return 1;
    } else {
        uint8_t i = 0;
        if (r->snd[0] != (uint8_t)(note + 1u)) { if (r->snd[1] == (uint8_t)(note + 1u)) i = 1; else return 1; }   /* its ON never went out: discard */
        if (!(r->snd_chan[i] & 0x10u)) {
            if (midi_out_free() == 0u) return 0;
            sq_midi_off(r->snd_chan[i], note);
        }
        if (r->snd_chan[i] & 0x80u) trk_note_off(&trk[c], note);
        sq_snd_drop(r, i);
        return 1;
    } }

/* ------------------------------------------------------------------ the event queue (ADR-0003 8.2) */

static int sq_evq_free(void)
{   for (int i = 0; i < SQ_EVQ_N; i++) if (!sq_evq[i].kind) return i;
    return -1; }

static void sq_evq_put(int i, uint32_t tick, uint8_t chan, uint8_t kind, uint8_t note, uint8_t vel)
{   sq_evq[i].tick = tick; sq_evq[i].chan = chan; sq_evq[i].kind = kind; sq_evq[i].note = note; sq_evq[i].vel = vel; }

/* An ON and its OFF go in together or not at all: a full queue loses a note, never leaves one hanging. */
static uint8_t sq_sched_pair(uint8_t chan, uint32_t on_t, uint32_t off_t, uint8_t note, uint8_t vel)
{   int a = sq_evq_free(), b = -1;
    if (a >= 0) { sq_evq[a].kind = 3; b = sq_evq_free(); sq_evq[a].kind = 0; }   /* (3 = reserved while looking for the second slot) */
    if (a < 0 || b < 0) { sq_drops++; return 0; }
    sq_evq_put(a, on_t, chan, 2, note, vel);
    sq_evq_put(b, off_t, chan, 1, note, 0);
    return 1; }

/* One event alone: the slide / tie continuation OFF, or a lone ON that starts a tie chain. An OFF never fails: with no free slot
 * it evicts the earliest ON still waiting in the queue (that note is lost, its OFF is then discarded by sq_emit). */
static uint8_t sq_sched_one(uint8_t chan, uint32_t tick, uint8_t kind, uint8_t note, uint8_t vel)
{   int a = sq_evq_free();
    if (a < 0 && kind == 1) {
        for (int i = 0; i < SQ_EVQ_N; i++)
            if (sq_evq[i].kind == 2 && (a < 0 || sq_evq[i].tick < sq_evq[a].tick)) a = i;
        if (a >= 0) sq_drops++;
    }
    if (a < 0) { sq_drops++; return 0; }
    sq_evq_put(a, tick, chan, kind, note, vel);
    return 1; }

static void sq_evq_purge_off(uint8_t c, uint8_t note)     /* the OFFs still queued for an instance of this note that has just been ended */
{   for (int i = 0; i < SQ_EVQ_N; i++)
        if (sq_evq[i].kind == 1 && ((sq_evq[i].chan >> 5) & 3u) == c && sq_evq[i].note == note) sq_evq[i].kind = 0; }

/* BUG-006. A ratchet repeat of this pitch will end whatever instance of it still sounds when the repeat's ON falls due (sq_emit's
 * same-pitch guard), so an OFF queued earlier for that pitch and due AFTER the ON (a HOLD's) is stale and would cut the repeat short.
 * Purged when the repeat is scheduled, before its own OFF is queued; a purge at emit time could not tell the repeat's own OFF from it. */
static void sq_evq_purge_off_after(uint8_t c, uint8_t note, uint32_t on_t)
{   for (int i = 0; i < SQ_EVQ_N; i++)
        if (sq_evq[i].kind == 1 && ((sq_evq[i].chan >> 5) & 3u) == c && sq_evq[i].note == note && sq_evq[i].tick > on_t) sq_evq[i].kind = 0; }

/* Emit what is due: ALL NOTE_OFFs first, then all NOTE_ONs, each pass by tick then slot (so the order is a function of the queue,
 * not of which slot a pair happened to land in). Overdue events (after a stall) fire at once. An OFF the ring cannot take stays
 * queued and stops the whole dispatch (the ONs wait behind it: an ON must never overtake the OFF of the same note). */
static void sq_dispatch(uint32_t now)
{   for (uint8_t kind = 1; kind <= 2; kind++)
        for (;;) {
            int b = -1;
            SeqEvent e;
            for (int i = 0; i < SQ_EVQ_N; i++)
                if (sq_evq[i].kind == kind && sq_evq[i].tick <= now && (b < 0 || sq_evq[i].tick < sq_evq[b].tick)) b = i;
            if (b < 0) break;
            e = sq_evq[b];
            if (kind == 1) { if (!sq_emit(e.chan, 1, e.note, 0)) return; sq_evq[b].kind = 0; }
            else { sq_evq[b].kind = 0; (void)sq_emit(e.chan, 2, e.note, e.vel); } } }

/* ------------------------------------------------------------------ stepping */

/* SEQUENCER_UX 1.4 / 9.2: the generator position of step p. Page k = p / 16 reads block src(k) = (pg_src >> 2k) & 3 (0..k; k = OWN). */
static uint32_t sq_gen_pos(const ChanSave *cs, uint8_t p)
{   uint8_t k = (uint8_t)(p / SQ_PAGE), s = (uint8_t)((cs->pg_src >> (2u * k)) & 3u);
    if (s > k) s = k;                                                 /* a stored source beyond the page is OWN */
    return (uint32_t)SQ_PAGE * s + (uint32_t)(p % SQ_PAGE); }

/* SEQUENCER_UX 9.3: the chance draw, SP1 seeded from ch * 0x9E37 ^ p * 0x45D9 ^ pass * 0x2C1B (the constants are the spec's). */
static uint32_t sp1_hash(uint8_t c, uint8_t p, uint16_t pass)
{   SqRng r;
    sq_rng_init(&r, (uint16_t)((uint32_t)c * 0x9E37u ^ (uint32_t)p * 0x45D9u ^ (uint32_t)pass * 0x2C1Bu), 0u);
    return sq_rng_next(&r); }

#ifndef PROB_LOW
#define PROB_LOW 50               /* ADR-0003 12.2 flex constant: the chance, in %, of a PROB step (PROB_LOW = 50 -> the threshold below is 128) */
#endif
#define SQ_PROB_LOW_256 (((uint32_t)(PROB_LOW) * 256u + 50u) / 100u)   /* a PROB step is silent when its draw (0..255) is this or more (9.2 names the test) */

/* SEQUENCER_UX 9.2: the resolved step p of channel c at `pass`. The generated layer at g(p) gives note, vel, gate, repeats and the
 * generated flags; the edit at ed[p] overrides each field it sets (AUTO fields keep the generated value). Output flags: PO_HIT resolved
 * (ON / OFF edit, then PROB), PO_TIE set on a hit whose GATE is TIE (or whose GATE is AUTO and the generator says TIE), PO_REP from RATCH,
 * PO_HOLD / PO_PACE_DEC / PO_SLIDE_IN as generated. The SLIDE of step p is not here: sq_slide_out() decides it. hc is NULL unless AF_CHORD. */
static void sq_eval(uint8_t c, uint8_t p, uint16_t pass, const AlgoDesc *a, const void *k, const SeqGlobal *g, PulseOut *o)
{   const StepEdit *e = &sq_ch[c].ed[p];
    const uint8_t on = (uint8_t)(e->fl & 3u), gt = (uint8_t)(e->gr & 15u), rt = (uint8_t)((e->gr >> 4) & 7u);
    const uint8_t oc = (uint8_t)(e->fl >> 5);
    uint8_t tie, hit;
    a->step(k, sq_gen_pos(&sq_ch[c], p), (a->flags & AF_CHORD) ? &sq_hc_inert : NULL, o);
    o->note &= 0x7Fu;
    if (e->note & 0x80u) o->note = sq_note(g, (int16_t)(e->note & 0x7Fu));       /* PITCH set: unsnapped semitones, snapped now (SP6) */
    if (oc >= 1u && oc <= 5u) {                                                  /* OCT: value - 3 octaves; 0 = AUTO / no shift */
        int16_t nt = (int16_t)o->note + 12 * ((int16_t)oc - 3);
        o->note = (uint8_t)(nt < 0 ? 0 : nt > 127 ? 127 : nt); }
    if (e->vel) o->vel = e->vel;
    if (gt >= 1u && gt <= 10u) o->gate = (uint8_t)(gt * 10u);                    /* GATE 1..10 = 10..100 %; 11 = TIE */
    if (rt >= 1u) {                                                              /* RATCH 1x..4x: repeats - 1 in the PO_REP field */
        uint8_t r4 = rt > 4u ? 4u : rt;
        o->flags = (uint8_t)((o->flags & ~PO_REP_MASK) | ((uint8_t)(r4 - 1u) << PO_REP_SHIFT)); }
    tie = (uint8_t)(gt == 11u || (gt == 0u && (o->flags & PO_TIE) != 0u));
    hit = on == 0u ? (uint8_t)((o->flags & PO_HIT) != 0u) : (uint8_t)(on == 1u);   /* ON / OFF edit overrides the generator's hit */
    if (((e->fl >> 4) & 1u) && (sp1_hash(c, p, pass) & 0xFFu) >= SQ_PROB_LOW_256) hit = 0;   /* PROB 50 % */
    o->flags = (uint8_t)((o->flags & ~(PO_HIT | PO_TIE)) | (hit ? PO_HIT : 0u) | ((hit && tie) ? PO_TIE : 0u));
    if (o->vel < 1) o->vel = 1;
    if (o->vel > 127) o->vel = 127;
    if (o->gate < 1) o->gate = 1;
    if (o->gate > 100) o->gate = 100; }

/* SEQUENCER_UX 9.2 slideout(p): SLIDE ON / OFF from the edit of step p; AUTO follows the generated PO_SLIDE_IN of the next step, and only
 * an algorithm that pairs (AF_PAIRING) has one. The caller still requires the next step to sound. */
static uint8_t sq_slide_out(uint8_t c, uint8_t p, uint8_t pairing, uint8_t gen_next_slide_in)
{   const uint8_t sl = (uint8_t)((sq_ch[c].ed[p].fl >> 2) & 3u);
    if (sl == 1u) return 1u;
    if (sl == 2u) return 0u;
    return (uint8_t)(pairing && gen_next_slide_in); }

/* P4-042: the longest tie search, in steps, per tick (the ISR bound). One probe = one sq_eval = one step() call, so a tick costs at most
 * SQ_TIE_SCAN + 2 evals per channel (this one, THE next, the search) instead of up to 63 + 2. A pattern of 17 steps or fewer never
 * reaches the bound, so its behaviour is exactly the full search's (the goldens, the fuzz). */
#define SQ_TIE_SCAN 16

/* SEQUENCER_UX 9.2, TIE: the tick at which a tied note ends = the start of the next sounding step, searched forward with the same wrap as n
 * (free-running step count tf + d, so the pass moves at the wrap). At most one loop: when no other step sounds before the tied step comes
 * round again, the note ends at that start. P4-042: and at most SQ_TIE_SCAN steps ahead, so a tie over more silent steps than that ends
 * there (the note is held SQ_TIE_SCAN + 1 steps). o = this step's swing offset, G / L as in sq_step_start. */
static uint32_t sq_tie_end(uint8_t c, uint8_t p, uint16_t pass, uint8_t len, const AlgoDesc *a, const void *k, const SeqGlobal *g,
                           uint32_t G, uint32_t L, uint32_t o)
{   const uint32_t tf = (uint32_t)pass * len + p;
    const uint32_t lim = len < SQ_TIE_SCAN + 1u ? len : SQ_TIE_SCAN + 1u;   /* the fallback distance; probes d = 1 .. lim - 1 */
    uint32_t d, hd = lim;
    for (d = 1; d < lim; d++) {
        PulseOut q;
        const uint32_t t = tf + d;
        sq_eval(c, (uint8_t)(t % len), (uint16_t)(t / len), a, k, g, &q);
        if (q.flags & PO_HIT) { hd = d; break; } }
    return G + hd * L + (((tf + hd) & 1u) ? o : 0u); }

static uint8_t sq_chan_byte(uint8_t c)
{   uint8_t m = sq_ch[c].midi_ch;
    return (uint8_t)((c << 5) | (m < SQ_MIDI_OFF ? m : 0x10u)); }

static int sq_snd_find(const ChanRT *r, uint8_t note)
{   if (r->snd[0] == (uint8_t)(note + 1u)) return 0;
    if (r->snd[1] == (uint8_t)(note + 1u)) return 1;
    return -1; }

/* End the note in slot i at once (a tie that did not happen, a repeated pitch). If the ring cannot take the OFF it is queued for
 * `now` and retried. */
static void sq_off_now(uint8_t c, uint8_t i, uint32_t now)
{   uint8_t note = (uint8_t)(sq_rt[c].snd[i] - 1u);
    if (!sq_emit((uint8_t)(c << 5), 1, note, 0)) (void)sq_sched_one((uint8_t)(sq_rt[c].snd_chan[i] & 0x7Fu), now, 1, note, 0); }

/* ADR-0003 7.3, step by step (the numbers are that section's). Channel c starts the step whose nominal start is rt.grid. */
static void sq_step_start(uint8_t c, uint32_t now, uint8_t L)
{   ChanRT *r = &sq_rt[c];
    const SeqGlobal *g = &sq_gdb[sq_gidx];
    const uint8_t ci = sq_cidx[c];                                    /* ONE read: every step() of this tick comes from this buffer */
    const AlgoDesc *a = &ALGOS[sq_calgo[c][ci]];
    const void *k = sq_cache[c][ci];
    const uint8_t len = sq_ch[c].len < 1u ? 1u : sq_ch[c].len > SQ_NSTEP ? SQ_NSTEP : sq_ch[c].len;
    const uint8_t p = r->p >= len ? 0u : r->p;
    const uint8_t n = (uint8_t)(p + 1u >= len ? 0u : p + 1u);        /* THE next step (SEQUENCER_UX 9.2): after the last step, step 0 */
    const uint32_t G = r->grid;
    const uint8_t sw = g->s.swing < 50u ? 50u : g->s.swing > 75u ? 75u : g->s.swing;
    const uint32_t o = ((uint32_t)L * (uint32_t)(sw - 50u) * 2u + 50u) / 100u;      /* odd steps start later (ADR-0003 5) */
    const uint32_t tf = (uint32_t)r->pass * len + p;                  /* the free-running step count: swing parity survives an odd length */
    const uint32_t S = G + ((tf & 1u) ? o : 0u);                      /* 1. actual start of this step */
    const uint32_t S1 = G + L + (((tf + 1u) & 1u) ? o : 0u);          /*    actual start of the next */
    const uint32_t D = L / 8u < 1u ? 1u : L / 8u;                     /* slide overlap */
    const uint8_t chan = sq_chan_byte(c);
    const uint8_t pairing = (uint8_t)(a->flags & AF_PAIRING);
    PulseOut out, out1;
    uint8_t slideout = 0, tie = 0, tiehold, hold, gate_eff, ok = 1, nr = 0, nsl;
    int li;
    uint32_t off_t = 0, hold_t = 0;
    SqRep rr[3];

    if (r->tie && r->tie != len) {                                    /* 0. LENGTH changed while a TIE held (9.2: cut by a LENGTH change): end it now. The step start */
        li = r->snd[1] ? 1 : 0;                                       /*    after the change is at or past the end of the tied step's own gate, so "sent at once" applies */
        if (r->snd[li]) { sq_evq_purge_off(c, (uint8_t)(r->snd[li] - 1u)); sq_off_now(c, (uint8_t)li, now); }
        r->tie = 0; }

    if (!a->step) { r->open = 0; r->slide_in = 0; r->tie = 0; goto advance; }   /* FB-014: an ALGOS[] row nobody has written yet (build / step NULL) is an inert, silent channel: its clock runs, nothing is evaluated or sent */

    sq_eval(c, p, r->pass, a, k, g, &out);                            /* 2. */
    if (pairing && r->nxt_ok && r->nxt.pad == (uint8_t)(a->id + 1u)) {   /*    pairing fields come from the snapshot taken one step ago */
        out.flags = (uint8_t)((out.flags & ~(PO_HIT | PO_SLIDE_IN)) | (r->nxt.flags & (PO_HIT | PO_SLIDE_IN)));
        out.note = r->nxt.note;
    }
    sq_eval(c, n, (uint16_t)(r->pass + (n == 0u ? 1u : 0u)), a, k, g, &out1);   /* 3. the wrap belongs to the next pass (9.2, 9.3) */
    if (pairing) { r->nxt = out1; r->nxt.pad = (uint8_t)(a->id + 1u); r->nxt_ok = 1; }

    if (!(out.flags & PO_HIT)) {                                      /* 4. a rest (mute is NOT tested here: it closes the MIDI gate in sq_emit) */
        li = r->snd[1] ? 1 : 0;
        if (r->open && r->snd[li]) sq_off_now(c, (uint8_t)li, now);
        r->open = 0; r->slide_in = 0;
        goto advance;
    }

    slideout = (uint8_t)(sq_slide_out(c, p, pairing, (uint8_t)((out1.flags & PO_SLIDE_IN) != 0u)) && (out1.flags & PO_HIT));   /* 5. a slide crosses the loop end */
    tiehold = (uint8_t)((out.flags & PO_TIE) != 0u);
    if (tiehold && slideout && ((sq_ch[c].ed[p].fl >> 2) & 3u) == 1u) tiehold = 0;   /* 9.2 (P4-024): a SET SLIDE whose next step sounds wins over TIE (the glide's S1 + D) */
    else if (tiehold) slideout = 0;                                   /*    otherwise a TIE holds this step and does not also glide; TIE always cancels RATCH (below) */
    r->tie = tiehold ? len : 0;                                       /*    the LENGTH this hold is scheduled under: step 0 compares it at the next step start */
    li = r->snd[1] ? 1 : 0;                                           /*    the newest sounding note */
    tie = (uint8_t)(r->slide_in && r->snd[li] && out.note == (uint8_t)(r->snd[li] - 1u));
    if (r->open && !tie && r->snd[li]) sq_off_now(c, (uint8_t)li, now);   /* the announced tie did not happen */
    r->open = 0;

    {   uint32_t ge = (uint32_t)out.gate * ((uint32_t)g->s.gate * 2u) / 100u;                /* GATE 50 % = x1.00 (decisions 43, 44) */
        gate_eff = (uint8_t)(ge > 100u ? 100u : ge < 1u ? 1u : ge); }
    hold = (uint8_t)((out.flags & PO_HOLD) ? (out.hold < 1u ? 1u : out.hold > 8u ? 8u : out.hold) : 0u);
    nsl = (uint8_t)(slideout && out1.note == out.note);               /* the next step ties onto this note */
    if (tiehold) hold_t = sq_tie_end(c, p, r->pass, len, a, k, g, G, L, o);   /* 9.2 TIE: to the next sounding step (wrap included) */

    if (!slideout && !hold && !tie && !tiehold && (out.flags & PO_REP_MASK)) {   /* 7. ratchet: SP5 repeats inside [S, S1 - 1] (a TIE holds: no repeats) */
        uint32_t w = S1 - S;
        nr = sq_ratchet((uint8_t)(w > 255u ? 255u : w), (uint8_t)(((out.flags & PO_REP_MASK) >> PO_REP_SHIFT) + 1u),
                        (out.flags & PO_PACE_DEC) ? 1 : -1, out.vel, 0, gate_eff, rr);
    }

    if (slideout) { if (!nsl) off_t = S1 + D; }                       /* the off is the next step's start + D; a tie into the same note has none yet */
    else if (tiehold) off_t = hold_t;                                 /* the next sounding step's start (sq_tie_end) */
    else if (hold) off_t = G + (uint32_t)hold * L - SQ_GAP;           /* ignores GATE */
    else if (nr) {                                                    /* the first trigger ends inside its own cell */
        uint32_t sp = rr[0].off, fl = sp * gate_eff / 100u;
        if (fl > sp - 1u) fl = sp - 1u;
        if (fl < 1u) fl = 1u;
        off_t = S + fl;
    } else {
        uint32_t ln = (uint32_t)L * gate_eff / 100u;
        if (ln < 1u) ln = 1u;
        off_t = S + ln;
        if (off_t > S1 - 1u) off_t = S1 - 1u;                         /* strictly before the next start: a non-slide pair is always off-before-on */
    }
    if (!nsl && off_t <= S) off_t = S + 1u;

    if (tie) {                                                        /* 6. the sounding note continues: schedule nothing for the ON */
        if (nsl) r->open = 1; else (void)sq_sched_one(chan, off_t, 1, out.note, 0);
    } else {
        int same = sq_snd_find(r, out.note);
        if (same >= 0) { sq_off_now(c, (uint8_t)same, now); sq_evq_purge_off(c, out.note); }   /* same-pitch safety */
        if (nsl) { ok = sq_sched_one(chan, S, 2, out.note, out.vel); if (ok) r->open = 1; }
        else ok = sq_sched_pair(chan, S, off_t, out.note, out.vel);
        if (ok && nr) {
            const HarmChord *hcp = (a->flags & AF_CHORD) ? &sq_hc_inert : NULL;
            for (uint8_t j = 0; j < nr; j++) {
                uint8_t note = out.note;
                if (a->sub_note) note = (uint8_t)(a->sub_note(k, sq_gen_pos(&sq_ch[c], p), (uint8_t)(j + 1u), hcp) & 0x7Fu);
                sq_evq_purge_off_after(c, note, S + rr[j].off);
                (void)sq_sched_pair(chan, S + rr[j].off, S + rr[j].off + rr[j].len, note, rr[j].vel);
            }
        }
    }
    r->slide_in = slideout;

advance:                                                              /* 8. */
    r->p = n;
    if (n == 0) r->pass++;
    r->grid = G + L; }

/* ------------------------------------------------------------------ transport */

static void sq_rt_run(uint32_t at)                                    /* every channel to step 0, its next step starting at tick `at` */
{   for (uint8_t c = 0; c < SQ_NCH; c++) {
        sq_rt[c].grid = at; sq_rt[c].p = 0; sq_rt[c].pass = 0; sq_rt[c].tie = 0;
        sq_rt[c].nxt_ok = 0; sq_rt[c].slide_in = 0; sq_rt[c].open = 0; } }

/* STOP, PANIC and the start of PLAY (ADR-0003 8.4): a NOTE_OFF for every sounding note on the channel byte it was sent with, the
 * preview parts released, then CC123 on each MIDI channel in use (the configured ones and those of any sounding note), the queue
 * cleared, the runtime reset. Returns 1 when clean; 0 when the ring was short: it resumes where it stopped on the next call
 * (sq_block makes it) and the clock does not run until it is done. Offs first, then CC123, as t1_all_off did. */
static uint8_t sq_all_off(void)
{   if (!sq_off_stage) {
        sq_off_stage = 1; sq_cc_pend = 0;
        for (uint8_t c = 0; c < SQ_NCH; c++) {
            if (sq_ch[c].midi_ch < SQ_MIDI_OFF) sq_cc_pend |= (uint16_t)(1u << sq_ch[c].midi_ch);
            for (uint8_t i = 0; i < 2; i++)
                if (sq_rt[c].snd[i] && !(sq_rt[c].snd_chan[i] & 0x10u)) sq_cc_pend |= (uint16_t)(1u << (sq_rt[c].snd_chan[i] & 15u));
        }
    }
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        ChanRT *r = &sq_rt[c];
        while (r->snd[0] || r->snd[1]) {
            uint8_t i = r->snd[0] ? 0u : 1u, note = (uint8_t)(r->snd[i] - 1u);
            if (!(r->snd_chan[i] & 0x10u)) { if (midi_out_free() == 0u) return 0; sq_midi_off(r->snd_chan[i], note); }
            if (r->snd_chan[i] & 0x80u) trk_note_off(&trk[c], note);
            sq_snd_drop(r, i);
        }
    }
    for (uint8_t m = 0; m < 16; m++)
        if ((sq_cc_pend >> m) & 1u) { if (midi_out_free() == 0u) return 0; sq_emit_cc123(m); sq_cc_pend = (uint16_t)(sq_cc_pend & ~(1u << m)); }
    for (int i = 0; i < SQ_EVQ_N; i++) sq_evq[i].kind = 0;
    memset(sq_rt, 0, sizeof sq_rt);
    memset(sq_live_c, 0, sizeof sq_live_c);                        
    sq_off_stage = 0;
    return 1; }

/* Samples per beat in 1/16 samples (M10): the external clock's measured beat, else the user's tempo at the real sample rate. */
static uint32_t sq_beat_q4(void)
{   uint32_t e = (uint32_t)SQ_EXT_BEAT(), b;
    if (e) return e << 4;
    b = (uint32_t)SQ_BPM();
    return b ? (SEQ_FS_Q4 * 60u + b / 2u) / b : 0u; }

static void sq_on_start(void)                                         /* PLAY / external Start: tick 0 runs in the first block (t1_on_start did the same) */
{   sq_playing = 1;
    sq_stamp_now();
    sq_now = 0; sq_cur = 0;
    if (sq_all_off()) sq_rt_run(0);                                   /* (a short ring: sq_block finishes it, then runs sq_rt_run(sq_now)) */
    sq_acc = sq_beat_q4(); }

static void sq_on_stop(void)
{   sq_stamp_now();
    if (sq_playing) sq_rt_byte(0xFC);             /* due at once, ahead of the note-offs of sq_all_off */
    sq_playing = 0;
    (void)sq_all_off(); }

/* Live notes (ADR-0003 6, 16.9): a key or a MIDI IN note plays on the SELECTED channel, out its MIDI channel and into its preview part,
 * through the same exit as the sequence (sq_emit, so STOP / PANIC end it and the unique-MIDI and mute gates apply). ISR side: keyboard_block
 * and midi_event call these from events_block. sq_sel is written by the UI (ui_sq_select) and read once per call; sq_live_c remembers which
 * channel each note went to, so a note-off follows its note-on after the selection moved. The note is sent as played (no scale snap yet:
 * what a key MEANS on a page is P4-013's). */
static void sq_live_on(uint8_t note, uint8_t vel)
{   const uint8_t c = (uint8_t)(sq_sel & 3u);
    note &= 0x7Fu;
    if (sq_live_c[note]) return;                                      /* a repeated note-on of a held note: one note */
    sq_stamp_now();
    sq_live_c[note] = (uint8_t)(c + 1u);
    (void)sq_emit(sq_chan_byte(c), 2, note, vel ? vel : 1u); }

static void sq_live_off(uint8_t note)
{   uint8_t c;
    note &= 0x7Fu;
    if (!sq_live_c[note]) return;
    c = (uint8_t)(sq_live_c[note] - 1u);
    sq_live_c[note] = 0;
    sq_stamp_now();
    if (!sq_emit((uint8_t)(c << 5), 1, note, 0))                      /* the ring was full: retry at the next tick (sq_dispatch), never a hung note */
        (void)sq_sched_one((uint8_t)(c << 5), sq_now, 1, note, 0); }

/* The ISR half of a queued preset load (ADR-0003 10): at a 768-tick bar start the staged ChanSaves go live, the caches the main loop
 * built into the inactive buffers and the staged SeqGlobal become the active ones. rt.p / grid are untouched: the music does not jump. */
static void sq_apply_load(void)
{   for (uint8_t c = 0; c < SQ_NCH; c++) { memcpy(&sq_ch[c], &sq_stage_ch[c], sizeof sq_ch[c]); sq_cidx[c] ^= 1u; }
    sq_gidx ^= 1u;
    sq_epoch++;
    sq_load_pending = 0; }

static void sq_apply_bar_boundary(uint32_t now)
{   if (sq_load_pending && (now % SQ_BAR_TICKS) == 0u) sq_apply_load(); }

/* Song Position then Continue (M7): SPP = tick / 48 sixteenths, 14 bits LSB first. P4-009 calls it when a continue is wired. */
static void sq_clock_continue(uint32_t tick)
{   uint32_t spp = tick / 48u;
    if (!SQ_IS_MASTER()) return;
    sq_out(0x03u | 0xF2u << 8 | (spp & 0x7Fu) << 16 | ((spp >> 7) & 0x7Fu) << 24);
    sq_rt_byte(0xFB); }

/* One 192-PPQN tick (ADR-0003 7.2): every channel whose step is due starts it (muted or not), then the due events go out.
 * Order on the wire at one tick (M4): clock, then Start, then the OFFs, then the ONs (sq_dispatch), CCs are never on a tick. */
static void sq_tick(uint32_t now)
{   const SeqGlobal *g;
    uint8_t L;
    sq_cur = now;
    sq_stamp_tick(sq_tick_s);
    if (now == 0u) sq_rt_byte(0xFA);                                  /* a start from step 0, then the first pulse at the same stamp */
    if ((now & 7u) == 0u) sq_rt_byte(0xF8);                           /* 24 per quarter: every 8th tick of the 192 grid */
    sq_apply_bar_boundary(now);
    g = &sq_gdb[sq_gidx];
    L = SQ_DIV_TICKS[g->s.div > 5u ? 5u : g->s.div];
    for (uint8_t c = 0; c < SQ_NCH; c++)
        if (now >= sq_rt[c].grid) sq_step_start(c, now, L);
    sq_dispatch(now); }

static void sq_consume_requests(void)
{   uint8_t q = sq_req;
    if (!q) return;
    sq_req = 0;                                                       /* (the ISR is the only consumer; a request set after this read waits for the next block) */
    if (q & SQ_REQ_STOP) sq_on_stop();
    if (q & SQ_REQ_START) sq_on_start();
    if (q & SQ_REQ_PANIC) { if (sq_all_off() && sq_playing) sq_rt_run(sq_now); }
    if (q & SQ_REQ_RESTART)                                           /* F#5 RESTRT: every channel to step 0 at its next step */
        for (uint8_t c = 0; c < SQ_NCH; c++) { sq_rt[c].p = 0; sq_rt[c].pass = 0; sq_rt[c].nxt_ok = 0; } }

/* Where the tick lands inside the block (M1). need = (k+1) beats minus the accumulator before the block, in sq_acc units; one sample
 * is SQ_QSTEP of them. The tick belongs to sample ceil(need / QSTEP) - 1 (0 when it was already due): exact to one sample. */
static uint32_t sq_tick_pos(int32_t need, uint32_t n)
{   uint32_t s;
    if (need <= 0) return 0u;
    s = ((uint32_t)need + SQ_QSTEP - 1u) / SQ_QSTEP - 1u;
    return s >= n ? n - 1u : s; }

static void sq_block_run(uint32_t n)
{   uint32_t bsq, acc0;
    int32_t need;
    sq_entropy_ctr += n;
    sq_stamp_now();
    sq_consume_requests();
    if (sq_off_stage) {                                               /* a stop / panic the ring was too short for finishes first */
        if (!sq_all_off()) return;
        if (sq_playing) sq_rt_run(sq_now);
    }
    bsq = sq_beat_q4();
    if (!sq_playing || !bsq) return;
    acc0 = sq_acc;
    sq_acc += n * SQ_QSTEP;
    need = (int32_t)bsq - (int32_t)acc0;
    while (sq_acc >= bsq) {
        sq_acc -= bsq;
        sq_tick_s = sq_tick_pos(need, n);
        sq_tick(sq_now); sq_now++;
        need += (int32_t)bsq;
    }
    sq_tick_s = 0; }

/* Once per control block, n samples (events_block's seq_n, which already follows the internal BPM or an external clock). */
static void sq_block(uint32_t n)
{   sq_block_run(n);
    sq_boff += n;                                                     /* (sq_half_begin zeroes it each half; the wrap only matters if nobody calls it) */
    if (sq_boff >= SQ_HALF_FRAMES) sq_boff -= SQ_HALF_FRAMES; }
