/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_chan.c: the channel state API (P4-005; ADR-0003 sections 3.1, 6, 10 and 12, as amended by section 22).
 *
 * Everything the UI may call, all MAIN LOOP, none from the ISR: sq_morph_turn, sq_dice_layer, sq_dice_all, sq_undo, sq_set_midi
 * (unique rule, SEQUENCER_UX 4.4), sq_set_algo, sq_set_mute, sq_rebuild (cache flip + epoch guard), the power-on defaults
 * (sq_defaults / sq_boot, ADR-0003 12) and sq_preset_save / sq_preset_load (RAM only; flash is P7).
 *
 * Included by the unity build right AFTER sq_core.c (it uses sq_ch, sq_gdb, sq_cache, sq_calgo, sq_cidx, sq_epoch, sq_stage_ch,
 * sq_load_pending, sq_apply_load, sq_entropy_ctr, ALGOS). Not in felucca.c yet (cut-over is P4-012). Freestanding: memset / memcpy.
 *
 * What the API never touches: ChanSave.ed[] (SEQUENCER_UX 1.2: a dice, a morph or an algorithm change rewrites the generated layer
 * only), ChanRT, the event queue, sq_emit. Only a preset load replaces the edits.
 *
 * Not modelled here (other tasks): ModSave beyond the one s_mod per channel and algorithm the dice needs (P4-023), the CRC (storage
 * layer, P7), HARMONY's use of HarmonySave (stored and validated only), the reset family and the edit-layer writers (P4-006 / P6). */

#ifndef SQ_IRQ_OFF
#define SQ_IRQ_OFF()  fm1_irq_off()      /* a multi-byte swap the ISR must not see half of (the t1u_temp pattern, ui_t1.c) */
#define SQ_IRQ_ON()   fm1_irq_on()
#endif

#define SQ_BPM_DEFAULT 120u
#define SQ_UNDO_SEEDS  0x3Fu             /* UndoRec.seed_mask bits 0..5 = seed ids, bit 6 = s_mod (SQ_SEED_MOD) */
#define SQ_UNDO_SMOD   0x40u
#define SQ_PRESET_VERSION 4u
#define SQ_HARM_DEFAULT_LEN 0x03u        /* bars 4, nchords 1 (HARMONY 2.1: "1---") */
#define SQ_HARM_DEFAULT_VM  0x82u        /* VOICE 2, MOVE 8 */

static UndoRec     sq_undo_rec[SQ_NCH];  /* one record per channel (decision 46); what == 0 = empty */
static UndoMidi    sq_um;                /* the MIDI-reassignment record (SEQUENCER_UX 4.4); latest while sq_um_valid */
static uint8_t     sq_um_valid;
static uint16_t    sq_smod[SQ_NCH][SQ_NALGO];   /* s_mod: ModSave holds it (MODULATION 7); the dice and undo need it before P4-023 */
static HarmonySave sq_harm;
static uint16_t    sq_bpm = SQ_BPM_DEFAULT;
static uint16_t    sq_dice_n;            /* counts dice: keeps two dice at the same sample count different */
static uint8_t     sq_seen_epoch;        /* sq_chan_sync: the epoch the undo records were last cleared for */
static HarmonySave sq_stage_harm;        /* a queued load's HARMONY and BPM, applied by sq_chan_sync when the ISR lands the load */
static uint16_t    sq_stage_bpm;
static uint8_t     sq_stage_valid;

static void sq_chan_sync(void);

/* ------------------------------------------------------------------ rebuild (ADR-0003 6) */

/* Build channel c's algorithm into cache buffer `buf` for global g. The seventh slot is s_mod. */
static void sq_build_buf(uint8_t c, uint8_t buf, const SeqGlobal *g, const ChanSave *cs)
{   const AlgoDesc *a = &ALGOS[cs->algo];
    uint16_t sd[SQ_NSEED + 1];
    for (uint8_t i = 0; i < SQ_NSEED; i++) sd[i] = cs->st[cs->algo].seed[i];
    sd[SQ_SEED_MOD] = sq_smod[c][cs->algo];
    a->build(cs->st[cs->algo].morph, sd, g, sq_cache[c][buf]);
    sq_calgo[c][buf] = cs->algo; }

/* Main loop. Builds into the inactive buffer, then flips (one byte store). If the ISR applied a preset meanwhile (sq_epoch moved) the
 * state it built from is gone: build again. While a preset load is pending the inactive buffers hold the STAGED caches, so nothing is
 * built (returns 0): the state edit stays in sq_ch, and the load replaces it anyway. Returns 1 when the new cache is live. */
static uint8_t sq_rebuild(uint8_t ch)
{   if (ch >= SQ_NCH) return 0;
    for (uint8_t tries = 0; tries < 4; tries++) {
        uint8_t ep, buf, ok;
        if (sq_load_pending) return 0;
        ep = sq_epoch;
        buf = (uint8_t)(sq_cidx[ch] ^ 1u);
        sq_build_buf(ch, buf, &sq_gdb[sq_gidx], &sq_ch[ch]);
        SQ_IRQ_OFF();
        ok = (uint8_t)(ep == sq_epoch && !sq_load_pending);
        if (ok) sq_cidx[ch] = buf;
        SQ_IRQ_ON();
        if (ok) return 1;
    }
    return 0; }

/* ------------------------------------------------------------------ morph, dice, undo */

static uint16_t sq_entropy(void)          /* ADR-0003 3.1: a free-running sample counter, mixed so two dice at one count differ */
{   return (uint16_t)(sq_entropy_ctr ^ (sq_entropy_ctr >> 16) ^ sq_mix((uint32_t)++sq_dice_n * 0x9E3779B1u)); }

static uint16_t *sq_seed_ptr(uint8_t ch, uint8_t id)
{   return id == SQ_SEED_MOD ? &sq_smod[ch][sq_ch[ch].algo] : &sq_ch[ch].st[sq_ch[ch].algo].seed[id]; }

/* K1..K4 / S1..S4 (slot 0..7) turn one morph by `detents` x MorphDesc.step, clamped to min / max. Not undoable (turn back). */
static void sq_morph_turn(uint8_t ch, uint8_t knob, int8_t detents)
{   const AlgoDesc *a; const MorphDesc *md; uint8_t id; int32_t v;
    if (ch >= SQ_NCH || knob >= SQ_NMORPH || sq_ch[ch].algo >= SQ_NALGO) return;
    a = &ALGOS[sq_ch[ch].algo];
    id = a->knob_morph[knob] & (SQ_NMORPH - 1u);
    md = &a->morph[id];
    v = (int32_t)sq_ch[ch].st[sq_ch[ch].algo].morph[id] + (int32_t)detents * md->step;
    if (v < md->min) v = md->min;
    if (v > md->max) v = md->max;
    if ((uint8_t)v == sq_ch[ch].st[sq_ch[ch].algo].morph[id]) return;
    sq_ch[ch].st[sq_ch[ch].algo].morph[id] = (uint8_t)v;
    (void)sq_rebuild(ch); }

static void sq_undo_new(uint8_t ch)       /* a new undoable action: empty record for the algorithm in use, and the MIDI record is no longer the latest */
{   memset(&sq_undo_rec[ch], 0, sizeof sq_undo_rec[ch]);
    sq_undo_rec[ch].algo = sq_ch[ch].algo;
    sq_um_valid = 0; }

/* Re-roll seed `id` of layer slot `second` (0 first, 1 second). Records the old value once per call sequence. */
static void sq_dice_seed(uint8_t ch, uint8_t id, uint8_t second, UndoRec *u)
{   uint16_t *p, old, nv;
    if (id > SQ_SEED_MOD) return;
    p = sq_seed_ptr(ch, id);
    old = *p;
    if (!(u->seed_mask & (id == SQ_SEED_MOD ? SQ_UNDO_SMOD : (1u << id)))) {
        if (id == SQ_SEED_MOD) u->smod = old; else u->seed[id] = old;
        u->seed_mask |= (uint8_t)(id == SQ_SEED_MOD ? SQ_UNDO_SMOD : (1u << id)); }
    nv = sq_entropy();
    if (second) nv ^= 0x5A5Au;
    if (nv == old) nv = (uint16_t)(nv + 0x9E37u);
    if (id < SQ_NSEED && ALGOS[sq_ch[ch].algo].seed_k[id] == 0xFFu && nv == 0u) nv = 1u;   /* EUCLID ROT: 0 = no rotation, a dice never lands on it */
    *p = nv; }

static uint8_t sq_dice_apply(uint8_t ch, uint8_t layer, UndoRec *u)   /* returns 1 if the layer holds at least one seed */
{   const LayerDesc *l = &ALGOS[sq_ch[ch].algo].layer[layer];
    if (!l->n) return 0;
    for (uint8_t k = 0; k < l->n && k < 2u; k++) sq_dice_seed(ch, l->seed[k], k, u);
    return 1; }

static void sq_dice_layer(uint8_t ch, uint8_t layer)
{   UndoRec u;
    if (ch >= SQ_NCH || layer >= SQ_NLAYER || sq_ch[ch].algo >= SQ_NALGO) return;
    memset(&u, 0, sizeof u);
    if (!sq_dice_apply(ch, layer, &u)) return;                        /* an absent layer is inert: no change, no record */
    u.what = 1; u.algo = sq_ch[ch].algo;
    sq_undo_rec[ch] = u; sq_um_valid = 0;
    (void)sq_rebuild(ch); }

static void sq_dice_all(uint8_t ch)                                   /* layers 0..4, ONE record for the union */
{   UndoRec u; uint8_t any = 0;
    if (ch >= SQ_NCH || sq_ch[ch].algo >= SQ_NALGO) return;
    memset(&u, 0, sizeof u);
    for (uint8_t l = 0; l < SQ_NLAYER; l++) any |= sq_dice_apply(ch, l, &u);
    if (!any) return;
    u.what = 1; u.algo = sq_ch[ch].algo;
    sq_undo_rec[ch] = u; sq_um_valid = 0;
    (void)sq_rebuild(ch); }

static void sq_swap8(uint8_t *a, uint8_t *b) { uint8_t t = *a; *a = *b; *b = t; }
static void sq_swap16(uint16_t *a, uint16_t *b) { uint16_t t = *a; *a = *b; *b = t; }

/* Swap the channel's state with the record (undo; the same call again = redo). If the latest action was a MIDI reassignment
 * (UndoMidi) that is what is swapped, as ONE record for both channels. Returns 1 if anything was swapped. */
static uint8_t sq_undo(uint8_t ch)
{   UndoRec *u; ChanSave *cs;
    if (sq_um_valid) {
        uint8_t m = sq_ch[sq_um.ch].midi_ch;
        sq_ch[sq_um.ch].midi_ch = sq_um.old_midi; sq_um.old_midi = m;
        if (sq_um.other < SQ_NCH) {
            m = sq_ch[sq_um.other].midi_ch;
            sq_ch[sq_um.other].midi_ch = sq_um.other_old; sq_um.other_old = m;
        }
        return 1; }
    if (ch >= SQ_NCH) return 0;
    u = &sq_undo_rec[ch]; cs = &sq_ch[ch];
    if (!u->what || u->algo >= SQ_NALGO) return 0;
    if (u->what & 1u) {
        for (uint8_t i = 0; i < SQ_NSEED; i++)
            if (u->seed_mask & (1u << i)) sq_swap16(&u->seed[i], &cs->st[u->algo].seed[i]);
        if (u->seed_mask & SQ_UNDO_SMOD) sq_swap16(&u->smod, &sq_smod[ch][u->algo]);
    }
    if (u->what & 4u)
        for (uint8_t i = 0; i < SQ_NMORPH; i++) sq_swap8(&u->morph[i], &cs->st[u->algo].morph[i]);
    if (u->what & 2u) {
        uint8_t base = (uint8_t)((u->idx & 3u) * SQ_PAGE);
        for (uint8_t i = 0; i < SQ_PAGE; i++) { StepEdit t = cs->ed[base + i]; cs->ed[base + i] = u->ed[i]; u->ed[i] = t; }
    }
    if (u->what & 8u) {
        uint8_t s = (uint8_t)(u->idx & (SQ_NSTEP - 1u));
        StepEdit t = cs->ed[s]; cs->ed[s] = u->ed[0]; u->ed[0] = t;
    }
    if (u->what & 16u) { sq_swap8(&u->len, &cs->len); sq_swap8(&u->pg_src, &cs->pg_src); sq_swap8(&u->pg_used, &cs->pg_used); }
    (void)sq_rebuild(ch);
    return 1; }

/* ------------------------------------------------------------------ channel settings */

/* 0..15 or SQ_MIDI_OFF. If another channel o holds midi_ch (not OFF) o becomes OFF in the same call and ONE UndoMidi is written; o is
 * returned (-1 = none displaced, also for a rejected value). Sounding notes end on the byte they were sent with (decision 47). */
static int8_t sq_set_midi(uint8_t ch, uint8_t midi_ch)
{   int8_t other = -1;
    if (ch >= SQ_NCH || midi_ch > SQ_MIDI_OFF || sq_ch[ch].midi_ch == midi_ch) return -1;
    if (midi_ch < SQ_MIDI_OFF)
        for (uint8_t o = 0; o < SQ_NCH; o++)
            if (o != ch && sq_ch[o].midi_ch == midi_ch) other = (int8_t)o;
    sq_um.ch = ch; sq_um.old_midi = sq_ch[ch].midi_ch;
    sq_um.other = other < 0 ? 0xFFu : (uint8_t)other; sq_um.other_old = other < 0 ? 0xFFu : sq_ch[other].midi_ch;
    sq_um_valid = 1;
    if (other >= 0) sq_ch[other].midi_ch = SQ_MIDI_OFF;               /* the displaced channel first: never two on one MIDI channel */
    sq_ch[ch].midi_ch = midi_ch;
    return other; }

static void sq_set_mute(uint8_t ch, uint8_t on)
{   if (ch < SQ_NCH) sq_ch[ch].muted = on ? 1u : 0u; }

/* Keeps st[] (S7), pins / edits, pg bits; clears this channel's UndoRec; the new algorithm lands at the next step (the ISR takes the
 * algorithm from sq_calgo of the flipped buffer). */
static void sq_set_algo(uint8_t ch, uint8_t algo)
{   if (ch >= SQ_NCH || algo >= SQ_NALGO || sq_ch[ch].algo == algo) return;
    memset(&sq_undo_rec[ch], 0, sizeof sq_undo_rec[ch]);
    sq_ch[ch].algo = algo;
    (void)sq_rebuild(ch); }

/* ------------------------------------------------------------------ defaults (ADR-0003 12) */

static void sq_default_chan(uint8_t c, ChanSave *cs)
{   static const uint8_t algo_of[SQ_NCH] = { SQ_ALGO_ACID, SQ_ALGO_EUCLID, SQ_ALGO_SNAKE, SQ_ALGO_STAGE };
    memset(cs, 0, sizeof *cs);
    cs->midi_ch = c;                       /* CH1..CH4 = MIDI 1..4 (decisions 73, 77): the 0-based value is the channel index */
    cs->algo = algo_of[c];
    cs->muted = c ? 1u : 0u;               /* CH1 unmuted; CH2..CH4 muted */
    cs->len = 16;
    cs->pg_src = 0;
    cs->pg_used = 1;                       /* page 1 holds content */
    for (uint8_t a = 0; a < SQ_NALGO; a++) {                          /* every algorithm slot, so a later switch starts from its own defaults */
        const AlgoDesc *d = &ALGOS[a];
        for (uint8_t i = 0; i < SQ_NMORPH; i++) cs->st[a].morph[i] = d->morph[i].def;
        for (uint8_t k = 0; k < SQ_NSEED; k++)
            cs->st[a].seed[k] = (k < d->n_seed && d->seed_k[k] != 0xFFu)
                ? (uint16_t)((0x3A5Du * (uint32_t)(c * d->n_seed + d->seed_k[k])) & 0xFFFFu) : 0u;
    } }

static void sq_default_global(SeqGlobal *g)
{   memset(g, 0, sizeof *g);
    g->s.key = 0; g->s.octave = 0; g->s.scale = 10;                   /* C, MINPEN */
    g->s.gate = 50; g->s.div = 2; g->s.swing = 50;
    sq_gbuild(g); }

static void sq_default_harm(HarmonySave *h)
{   memset(h, 0, sizeof *h); h->len = SQ_HARM_DEFAULT_LEN; h->vm = SQ_HARM_DEFAULT_VM; }

static void sq_defaults(SeqPreset *p)                                 /* the power-on SeqPreset (crc 0: the storage layer's) */
{   memset(p, 0, sizeof *p);
    p->version = SQ_PRESET_VERSION; p->bpm = SQ_BPM_DEFAULT;
    { SeqGlobal g; sq_default_global(&g); p->g = g.s; }
    for (uint8_t c = 0; c < SQ_NCH; c++) sq_default_chan(c, &p->ch[c]);
    sq_default_harm(&p->harm); }

/* ADR-0003 12 step 4 minus audio: fill the state from the defaults, build all four caches into buffer 0, zero the runtime. The caller
 * sets sq_ready (cut-over, P4-012). Call before the ISR runs sq_block. */
static void sq_boot(void)
{   sq_default_global(&sq_gdb[0]); sq_gdb[1] = sq_gdb[0];
    sq_gidx = 0; sq_load_pending = 0; sq_um_valid = 0; sq_stage_valid = 0;
    memset(sq_undo_rec, 0, sizeof sq_undo_rec);
    sq_default_harm(&sq_harm); sq_bpm = SQ_BPM_DEFAULT;
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        sq_default_chan(c, &sq_ch[c]);
        for (uint8_t a = 0; a < SQ_NALGO; a++) sq_smod[c][a] = SQ_SMOD_DEFAULT(c);
        sq_build_buf(c, 0, &sq_gdb[0], &sq_ch[c]);
        sq_cidx[c] = 0;
    }
    memset(sq_rt, 0, sizeof sq_rt);
    for (int i = 0; i < SQ_EVQ_N; i++) sq_evq[i].kind = 0;
    sq_seen_epoch = sq_epoch; }

/* ------------------------------------------------------------------ presets (ADR-0003 10; RAM only) */

static int sq_preset_save(SeqPreset *out)
{   memset(out, 0, sizeof *out);
    out->version = SQ_PRESET_VERSION; out->flags = 0; out->bpm = sq_bpm;
    out->g = sq_gdb[sq_gidx].s;
    for (uint8_t c = 0; c < SQ_NCH; c++) out->ch[c] = sq_ch[c];
    out->harm = sq_harm;
    out->crc = 0;                                                    /* the storage layer computes it (P7) */
    return 0; }

static uint8_t sq_clamp8(uint8_t v, uint8_t lo, uint8_t hi) { return v < lo ? lo : v > hi ? hi : v; }

static void sq_clean_edit(StepEdit *e)                               /* SEQUENCER_UX 9.1: keep every field inside its encoding */
{   uint8_t n = e->note & 0x7Fu, gate = e->gr & 15u, rat = (uint8_t)((e->gr >> 4) & 7u), on = e->fl & 3u, sl = (uint8_t)((e->fl >> 2) & 3u), oc = (uint8_t)(e->fl >> 5);
    if (n > 96u) n = 96u;
    e->note = (uint8_t)((e->note & 0x80u) | n);
    if (e->vel > 127u) e->vel = 127u;
    if (gate > 11u) gate = 0;
    if (rat > 4u) rat = 4u;
    if (on > 2u) on = 0;
    if (sl > 2u) sl = 0;
    if (oc > 5u) oc = 0;
    e->gr = (uint8_t)(gate | (rat << 4));
    e->fl = (uint8_t)(on | (sl << 2) | (e->fl & 0x10u) | (oc << 5)); }

/* Validate and clamp `in` into the staging copies (ADR-0003 10). Rejects (returns -1, nothing touched) a wrong version or an algorithm
 * id out of range: those cannot be clamped into something meaningful. Everything else is clamped; channels sharing a non-OFF MIDI channel
 * keep it on the lowest-numbered one only. when 0: applied at once (under the ISR guard); when 1: queued to the next 768-tick bar
 * (applied at once if stopped). The caches are built into the inactive buffers first; sq_load_pending is set last. */
static int sq_preset_load(const SeqPreset *in, uint8_t when)
{   SeqGlobal *sg;
    if (in->version != SQ_PRESET_VERSION) return -1;
    for (uint8_t c = 0; c < SQ_NCH; c++) if (in->ch[c].algo >= SQ_NALGO) return -1;
    SQ_IRQ_OFF(); sq_load_pending = 0; SQ_IRQ_ON();                 /* a still-pending older load is replaced */
    sg = &sq_gdb[sq_gidx ^ 1u];
    memset(sg, 0, sizeof *sg);
    sg->s = in->g;
    sg->s.key = sq_clamp8(sg->s.key, 0, 11);
    sg->s.octave = (int8_t)(sg->s.octave < -2 ? -2 : sg->s.octave > 2 ? 2 : sg->s.octave);
    sg->s.scale = sq_clamp8(sg->s.scale, 0, 15);
    sg->s.gate = sq_clamp8(sg->s.gate, 10, 100);
    sg->s.div = sq_clamp8(sg->s.div, 0, 5);
    sg->s.swing = sq_clamp8(sg->s.swing, 50, 75);
    sq_gbuild(sg);
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        ChanSave *s = &sq_stage_ch[c];
        *s = in->ch[c];
        s->muted = s->muted ? 1u : 0u;
        s->len = sq_clamp8(s->len, 1, SQ_NSTEP);
        s->pg_used &= 0x0Fu;
        s->pad[1] = 0;
        if (s->midi_ch > SQ_MIDI_OFF) s->midi_ch = SQ_MIDI_OFF;
        if (s->midi_ch < SQ_MIDI_OFF)
            for (uint8_t o = 0; o < c; o++) if (sq_stage_ch[o].midi_ch == s->midi_ch) { s->midi_ch = SQ_MIDI_OFF; break; }
        for (uint8_t a = 0; a < SQ_NALGO; a++)
            for (uint8_t i = 0; i < SQ_NMORPH; i++)
                s->st[a].morph[i] = sq_clamp8(s->st[a].morph[i], ALGOS[a].morph[i].min, ALGOS[a].morph[i].max);
        for (uint8_t i = 0; i < SQ_NSTEP; i++) sq_clean_edit(&s->ed[i]);
        sq_build_buf(c, (uint8_t)(sq_cidx[c] ^ 1u), sg, s);
    }
    sq_stage_bpm = in->bpm < 40u ? 40u : in->bpm > 240u ? 240u : in->bpm;
    sq_stage_harm = in->harm;
    {   HarmonySave *h = &sq_stage_harm;
        uint8_t bars = (uint8_t)((h->len & 7u) + 1u), nch = (uint8_t)(((h->len >> 3) & 7u) + 1u);
        uint32_t deg = (uint32_t)h->deg[0] | (uint32_t)h->deg[1] << 8 | (uint32_t)h->deg[2] << 16, nd = 0;
        if (nch > bars) nch = bars;
        h->len = (uint8_t)((bars - 1u) | ((nch - 1u) << 3));
        for (uint8_t i = 0; i < 8; i++) { uint32_t d = (deg >> (3 * i)) & 7u; if (d > 6u) d = 6u; nd |= d << (3 * i); }
        h->deg[0] = (uint8_t)nd; h->deg[1] = (uint8_t)(nd >> 8); h->deg[2] = (uint8_t)(nd >> 16);
        { uint8_t v = sq_clamp8((uint8_t)(h->vm & 15u), 0, 6), m = sq_clamp8((uint8_t)(h->vm >> 4), 0, 8); h->vm = (uint8_t)(v | (m << 4)); }
        h->drone = sq_clamp8(h->drone, 0, 14);
        h->rsv = 0;
    }
    sq_stage_valid = 1;
    if (when == 0u || !sq_playing) {
        SQ_IRQ_OFF(); sq_apply_load(); SQ_IRQ_ON();                  /* stopped: at once (sq_apply_load flips the caches, bumps the epoch) */
        sq_chan_sync();                                              /* undo records cleared, HARMONY and BPM taken */
    } else sq_load_pending = 1;                                      /* last store: the ISR lands it on the next 768-tick bar */
    return 0; }

/* Main loop, once per frame (one compare when idle): when the ISR (or the immediate path) has landed a load, clear every UndoRec
 * (ADR-0003 10), take the staged HARMONY / BPM and rebuild the four caches from the live state (a few hundred integer ops each). */
static void sq_chan_sync(void)
{   if (sq_epoch == sq_seen_epoch) return;
    sq_seen_epoch = sq_epoch;
    memset(sq_undo_rec, 0, sizeof sq_undo_rec);
    sq_um_valid = 0;
    if (sq_stage_valid) { sq_harm = sq_stage_harm; sq_bpm = sq_stage_bpm; sq_stage_valid = 0; }
    for (uint8_t c = 0; c < SQ_NCH; c++) (void)sq_rebuild(c);        /* s_mod is not in the preset: a dice or undo while the load was pending changed it, the staged cache never saw it */
}
