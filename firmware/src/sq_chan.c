/* SPDX-License-Identifier: GPL-3.0-only */
/* sq_chan.c: the channel state API (P4-005; ADR-0003 sections 3.1, 6, 10 and 12, as amended by section 22).
 *
 * Everything the UI may call, all MAIN LOOP, none from the ISR: sq_morph_turn, sq_dice_layer, sq_dice_all, sq_undo, sq_set_midi
 * (unique rule, SEQUENCER_UX 4.4), sq_set_algo, sq_set_mute, sq_rebuild (cache flip + epoch guard), sq_global_set (KEY SCALE OCTAVE GATE DIV
 * SWING, P4-024), the power-on defaults
 * (sq_defaults / sq_boot, ADR-0003 12) and sq_preset_save / sq_preset_load (RAM only; flash is P7).
 *
 * Included by the unity build right AFTER sq_core.c (it uses sq_ch, sq_gdb, sq_cache, sq_calgo, sq_cidx, sq_epoch, sq_stage_ch,
 * sq_load_pending, sq_apply_load, sq_entropy_ctr, ALGOS). In felucca.c since the cut-over (P4-012). Freestanding: memset / memcpy.
 *
 * What the API never touches: ChanSave.ed[] (SEQUENCER_UX 1.2: a dice, a morph or an algorithm change rewrites the generated layer
 * only), ChanRT, the event queue, sq_emit. Only a preset load replaces the edits.
 *
 * Not modelled here (other tasks): ModSave's device and slots (the MOD tasks; their 13 B are reserved zero), the CRC (storage
 * layer, P7), HARMONY's use of HarmonySave (stored and validated only) and the MOD slots. The ADR reset family (sq_reset_*) is below, with the page operations (P4-018).
 * The edit-layer writers and the page operations (P4-006) are below, after the algorithm section. */

#ifndef SQ_IRQ_OFF
#define SQ_IRQ_OFF()  fm1_irq_off()      /* a multi-byte swap the ISR must not see half of (the swap pattern of the removed T-1 UI) */
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
static uint8_t     sq_load_landed;       /* P4-044: set by sq_chan_sync when a load landed (either path); ui_sq_glue.c sq_frame consumes it, so its cleanup runs once whichever path landed it */
static uint8_t     sq_used = 0x01u;      /* P4-044: channels in use, bit n = channel n. Main-loop owned core state (the UI reads it as uisq_used). Saved as bit 0 of ChanSave.pad[1] by sq_preset_save, taken back by sq_chan_sync after a load; power-on = CH1 only */
static uint16_t    sq_smod[SQ_NCH][SQ_NALGO];   /* s_mod, one per channel and algorithm; saved and loaded as ModSave (P4-023) */

/* ModSave (MODULATION 7): 25 B per channel, a SIBLING array of SeqPreset (ADR-0003 22: 1556 + 100 = a 1656 B slot), so sizeof(SeqPreset)
 * is untouched. dev and slot[] (the MOD device and its four slots) are the MOD tasks'; here they are reserved and load as 0. */
typedef struct {
    uint8_t  dev;
    uint8_t  slot[4][3];           /* param, level, depth */
    uint8_t  smod[2 * SQ_NALGO];   /* s_mod of each algorithm, little endian bytes (keeps the struct 25 B, no padding) */
} ModSave;
_Static_assert(sizeof(ModSave) == 25, "ModSave");
static HarmonySave sq_harm;
static uint16_t    sq_bpm = SQ_BPM_DEFAULT;
static uint16_t    sq_dice_n;            /* counts dice: keeps two dice at the same sample count different */
static uint8_t     sq_seen_epoch;        /* sq_chan_sync: the epoch the undo records were last cleared for */
static HarmonySave sq_stage_harm;        /* a queued load's HARMONY and BPM, applied by sq_chan_sync when the ISR lands the load */
static uint16_t    sq_stage_bpm;
static uint16_t    sq_stage_smod[SQ_NCH][SQ_NALGO];   /* a queued load's s_mod; the staged caches are built from it, sq_chan_sync takes it */
static uint8_t     sq_stage_valid;

static void sq_chan_sync(void);

/* The per-frame hook (FB-011 a, FB-012): the main loop calls SQ_FRAME_SYNC() ONCE per frame, before ui_input() and so before any sq_* write.
 * It is one compare when idle. When the ISR has landed a preset it clears every UndoRec, takes the staged HARMONY / BPM / s_mod and
 * rebuilds the caches. P4-011 / P4-012 wire it in (P4-024 does not touch the main loop). sq_undo and every dice entry also call it first,
 * so a record can never be swapped onto a preset that landed inside the same frame. */
#define SQ_FRAME_SYNC() sq_chan_sync()

/* NOTE_EDIT session (FB-015): a write to the step being edited, after sq_note_edit_begin, shares ONE undo record. 0 none, 1 opened and no
 * write yet (sq_ne_snap holds the step as it was), 2 the record has been taken. Any other undoable action ends it. */
static uint8_t  sq_ne_state, sq_ne_ch, sq_ne_p;
static StepEdit sq_ne_snap;

/* ------------------------------------------------------------------ rebuild (ADR-0003 6) */

/* Build channel c's algorithm into cache buffer `buf` for global g. The seventh slot is s_mod. */
static void sq_build_buf(uint8_t c, uint8_t buf, const SeqGlobal *g, const ChanSave *cs, const uint16_t *smod)
{   const AlgoDesc *a = &ALGOS[cs->algo];
    uint16_t sd[SQ_NSEED + 1];
    for (uint8_t i = 0; i < SQ_NSEED; i++) sd[i] = cs->st[cs->algo].seed[i];
    sd[SQ_SEED_MOD] = smod[cs->algo];
    if (a->build) a->build(cs->st[cs->algo].morph, sd, g, sq_cache[c][buf]);   /* FB-014: a row not yet written (build NULL) leaves an inert cache: the core plays nothing from it */
    else memset(sq_cache[c][buf], 0, sizeof sq_cache[c][buf]);
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
        sq_build_buf(ch, buf, &sq_gdb[sq_gidx], &sq_ch[ch], sq_smod[ch]);
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
{   sq_chan_sync();                       /* a load that landed this frame has already cleared the old records: never write under a stale epoch */
    sq_ne_state = 0;
    memset(&sq_undo_rec[ch], 0, sizeof sq_undo_rec[ch]);
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
    if (ch >= SQ_NCH) return;
    sq_chan_sync();                                                   /* FB-012: first, so the record below is for the state that is live now */
    if (layer >= SQ_NLAYER || sq_ch[ch].algo >= SQ_NALGO) return;
    memset(&u, 0, sizeof u);
    if (!sq_dice_apply(ch, layer, &u)) return;                        /* an absent layer is inert: no change, no record */
    u.what = 1; u.algo = sq_ch[ch].algo;
    sq_undo_rec[ch] = u; sq_um_valid = 0; sq_ne_state = 0;
    (void)sq_rebuild(ch); }

static void sq_dice_all(uint8_t ch)                                   /* layers 0..4, ONE record for the union */
{   UndoRec u; uint8_t any = 0;
    if (ch >= SQ_NCH) return;
    sq_chan_sync();
    if (sq_ch[ch].algo >= SQ_NALGO) return;
    memset(&u, 0, sizeof u);
    for (uint8_t l = 0; l < SQ_NLAYER; l++) any |= sq_dice_apply(ch, l, &u);
    if (!any) return;
    u.what = 1; u.algo = sq_ch[ch].algo;
    sq_undo_rec[ch] = u; sq_um_valid = 0; sq_ne_state = 0;
    (void)sq_rebuild(ch); }

static void sq_swap8(uint8_t *a, uint8_t *b) { uint8_t t = *a; *a = *b; *b = t; }
static void sq_swap16(uint16_t *a, uint16_t *b) { uint16_t t = *a; *a = *b; *b = t; }

/* Swap the channel's state with the record (undo; the same call again = redo). If the latest action was a MIDI reassignment
 * (UndoMidi) that is what is swapped, as ONE record for both channels. Returns 1 if anything was swapped. */
static uint8_t sq_undo(uint8_t ch)
{   UndoRec *u; ChanSave *cs;
    sq_chan_sync();                                                   /* FB-012: a stale record must not land on a preset that arrived this frame */
    sq_ne_state = 0;
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
    sq_chan_sync();
    memset(&sq_undo_rec[ch], 0, sizeof sq_undo_rec[ch]); sq_ne_state = 0;
    sq_ch[ch].algo = algo;
    (void)sq_rebuild(ch); }

/* ------------------------------------------------------------------ global settings (ADR-0003 5, 6; SEQUENCER_UX SET and SCALE pages) */

#define SQ_G_KEY    0                /* sq_global_set fields */
#define SQ_G_SCALE  1
#define SQ_G_OCTAVE 2
#define SQ_G_GATE   3
#define SQ_G_DIV    4
#define SQ_G_SWING  5
#define SQ_G_NFIELD 6

/* KEY 0..11, SCALE 0..15, OCTAVE -2..+2, GATE 10..100 %, DIV 0..5, SWING 50..75 %. A value outside the range is clamped to its end (a knob
 * that runs past the end stays there). The change is built off to the side: the other SeqGlobal copy gets the new value and its derived snap[]
 * (sq_gbuild), then ALL FOUR channels' caches are built from it into their inactive buffers, and only then do sq_gidx and the four sq_cidx
 * flip, together inside one interrupt-off window. The ISR therefore sees either the old global with the old caches or the new global with
 * the new caches, never a mix and never a half-built cache. If a preset load lands while building (sq_epoch moved) the work starts again from the live
 * state. While a queued load is pending the inactive buffers hold the STAGED global and caches, so the call is refused (returns 0, nothing
 * changed): the load replaces every global anyway. Returns 1 when the setting changed and is live. */
static uint8_t sq_global_set(uint8_t field, int16_t v)
{   if (field >= SQ_G_NFIELD) return 0;
    for (uint8_t tries = 0; tries < 4; tries++) {
        SeqGlobal *ng;
        uint8_t ep, gi, ok;
        sq_chan_sync();
        if (sq_load_pending) return 0;
        ep = sq_epoch; gi = sq_gidx;
        ng = &sq_gdb[gi ^ 1u];
        *ng = sq_gdb[gi];
        switch (field) {
        case SQ_G_KEY:    ng->s.key    = (uint8_t)(v < 0 ? 0 : v > 11 ? 11 : v); break;
        case SQ_G_SCALE:  ng->s.scale  = (uint8_t)(v < 0 ? 0 : v > 15 ? 15 : v); break;
        case SQ_G_OCTAVE: ng->s.octave = (int8_t)(v < -2 ? -2 : v > 2 ? 2 : v); break;
        case SQ_G_GATE:   ng->s.gate   = (uint8_t)(v < 10 ? 10 : v > 100 ? 100 : v); break;
        case SQ_G_DIV:    ng->s.div    = (uint8_t)(v < 0 ? 0 : v > 5 ? 5 : v); break;
        default:          ng->s.swing  = (uint8_t)(v < 50 ? 50 : v > 75 ? 75 : v); break;
        }
        if (!memcmp(&ng->s, &sq_gdb[gi].s, sizeof ng->s)) return 0;   /* nothing to change */
        sq_gbuild(ng);
        for (uint8_t c = 0; c < SQ_NCH; c++) sq_build_buf(c, (uint8_t)(sq_cidx[c] ^ 1u), ng, &sq_ch[c], sq_smod[c]);
        SQ_IRQ_OFF();
        ok = (uint8_t)(ep == sq_epoch && !sq_load_pending);
        if (ok) { for (uint8_t c = 0; c < SQ_NCH; c++) sq_cidx[c] ^= 1u; sq_gidx = (uint8_t)(gi ^ 1u); }
        SQ_IRQ_ON();
        if (ok) return 1;
    }
    return 0; }

/* ------------------------------------------------------------------ edit layer and pages (SEQUENCER_UX 1.1-1.6, 9.1; P4-006)
 * Main loop. Edits never touch a cache: sq_eval (sq_core.c) reads ed[] at play time, so no rebuild is needed. Every writer refuses a
 * step or page the viewed length does not show (1.6: a hidden step is kept, not edited). One undo record per channel (sq_undo_page). */

#define SQ_F_ON     0                /* sq_step_set fields, SEQUENCER_UX 1.1 */
#define SQ_F_PITCH  1
#define SQ_F_OCT    2
#define SQ_F_GATE   3
#define SQ_F_VEL    4
#define SQ_F_RATCH  5
#define SQ_F_SLIDE  6
#define SQ_F_PROB   7
#define SQ_F_NFIELD 8
#define SQ_AUTO     (-32768)         /* the value that makes a field follow the generated layer (not -1: OCT -1 is a real value; BUG-010) */
#define SQ_GATE_TIE 101              /* GATE TIE (the other GATE values are 10..100 in steps of 10) */

static uint8_t sq_follow = 1;        /* CHAN K4 FOLLOW (1.5): UI state, one per device, not in presets; the view itself is the UI's */

static uint8_t sq_page_shown(uint8_t ch, uint8_t k) { return (uint8_t)(k < SQ_NSTEP / SQ_PAGE && (uint16_t)k * SQ_PAGE < sq_ch[ch].len); }

static uint8_t sq_gen_hit(uint8_t ch, uint8_t p)     /* the generated hit of step p, no edit and no chance draw: what a tap compares with */
{   const uint8_t ci = sq_cidx[ch];
    const AlgoDesc *a = &ALGOS[sq_calgo[ch][ci]];
    PulseOut o;
    if (!a->step) return 0;
    memset(&o, 0, sizeof o);
    a->step(sq_cache[ch][ci], sq_gen_pos(&sq_ch[ch], p), (a->flags & AF_CHORD) ? &sq_hc_inert : NULL, &o);
    return (uint8_t)((o.flags & PO_HIT) != 0u); }

/* Copy page k-1 over page k (edits and source) and mark k as holding content: the whole page, even where LENGTH shows part of it. */
static void sq_page_copy_before(ChanSave *cs, uint8_t k)
{   uint8_t s = (uint8_t)((cs->pg_src >> (2u * (k - 1u))) & 3u);
    memcpy(&cs->ed[k * SQ_PAGE], &cs->ed[(k - 1u) * SQ_PAGE], SQ_PAGE * sizeof(StepEdit));
    cs->pg_src = (uint8_t)((cs->pg_src & ~(3u << (2u * k))) | ((uint8_t)s << (2u * k)));
    cs->pg_used |= (uint8_t)(1u << k); }

/* One undo record for a page operation (what = 2 one page of edits, 16 len / page bits). Taken BEFORE the change; sq_undo swaps it. */
static void sq_undo_page(uint8_t ch, uint8_t k)
{   UndoRec *u;
    sq_undo_new(ch);
    u = &sq_undo_rec[ch];
    u->what = (uint8_t)(2u | 16u); u->idx = k;
    u->len = sq_ch[ch].len; u->pg_src = sq_ch[ch].pg_src; u->pg_used = sq_ch[ch].pg_used;
    memcpy(u->ed, &sq_ch[ch].ed[(uint16_t)k * SQ_PAGE], sizeof u->ed); }

/* Step-level undo (FB-015, SEQUENCER_UX 1.2 and 9.1: "one EDIT tap away from coming back"). A UndoRec with what = 8: idx = the step, ed[0] =
 * the step as it was; sq_undo swaps it, so the same call again redoes. Called BEFORE a changing write. Inside a session opened by
 * sq_note_edit_begin on this step, the first write takes the record (from the snapshot made at begin) and the later writes share it; any
 * other write, tap or clear is a record of its own (the last change wins, 9.1). */
static void sq_edit_record(uint8_t ch, uint8_t p)
{   UndoRec *u;
    StepEdit was;
    uint8_t join;
    sq_chan_sync();                                                   /* (a load that landed ends the session: decide after it) */
    join = (uint8_t)(sq_ne_state && sq_ne_ch == ch && sq_ne_p == p);
    if (join && sq_ne_state == 2) return;                             /* the session's record is already in place */
    was = join ? sq_ne_snap : sq_ch[ch].ed[p];
    sq_undo_new(ch);                                                  /* ends any session */
    u = &sq_undo_rec[ch];
    u->what = 8; u->idx = p; u->ed[0] = was;
    if (join) sq_ne_state = 2; }

/* One field of one step (1.1 / 1.2: NOTE EDIT, or RESET + that field's control). v = SQ_AUTO clears the field. Returns 1 when written.
 * ON 0 OFF / 1 ON; PITCH 0..96 semitones above 36 + KEY (unsnapped, SP6 snaps at play); OCT -2..+2 (0 is stored as AUTO's "no shift");
 * GATE 10..100 in steps of 10, or SQ_GATE_TIE; VEL 1..127; RATCH 1..4 (1x..4x); SLIDE 0 OFF / 1 ON; PROB 100 or 50. A set step
 * marks its page as holding content (pg_used). */
static uint8_t sq_step_set(uint8_t ch, uint8_t p, uint8_t field, int16_t v)
{   StepEdit n, *e;
    uint8_t oc;
    if (ch >= SQ_NCH || field >= SQ_F_NFIELD || p >= SQ_NSTEP || p >= sq_ch[ch].len) return 0;
    e = &sq_ch[ch].ed[p];
    if (v != SQ_AUTO) {                                              /* validate before any write */
        switch (field) {
        case SQ_F_ON:    if (v < 0 || v > 1) return 0; break;
        case SQ_F_PITCH: if (v < 0 || v > 96) return 0; break;
        case SQ_F_OCT:   if (v < -2 || v > 2) return 0; break;
        case SQ_F_GATE:  if (v != SQ_GATE_TIE && (v < 10 || v > 100 || v % 10 != 0)) return 0; break;
        case SQ_F_VEL:   if (v < 1 || v > 127) return 0; break;
        case SQ_F_RATCH: if (v < 1 || v > 4) return 0; break;
        case SQ_F_SLIDE: if (v < 0 || v > 1) return 0; break;
        default:         if (v != 100 && v != 50) return 0; break;   /* PROB */
        }
    }
    n = *e;
    switch (field) {
    case SQ_F_ON:    n.fl = (uint8_t)((n.fl & ~3u) | (v == SQ_AUTO ? 0u : v == 1 ? 1u : 2u)); break;
    case SQ_F_PITCH: n.note = (uint8_t)(v == SQ_AUTO ? 0u : 0x80u | (uint8_t)v); break;
    case SQ_F_OCT:   oc = (uint8_t)(v == SQ_AUTO ? 0u : (uint8_t)(v + 3)); n.fl = (uint8_t)((n.fl & 0x1Fu) | (uint8_t)(oc << 5)); break;
    case SQ_F_GATE:  n.gr = (uint8_t)((n.gr & 0xF0u) | (v == SQ_AUTO ? 0u : v == SQ_GATE_TIE ? 11u : (uint8_t)(v / 10))); break;
    case SQ_F_VEL:   n.vel = (uint8_t)(v == SQ_AUTO ? 0u : v); break;
    case SQ_F_RATCH: n.gr = (uint8_t)((n.gr & 0x8Fu) | (uint8_t)((v == SQ_AUTO ? 0 : v) << 4)); break;
    case SQ_F_SLIDE: n.fl = (uint8_t)((n.fl & ~0x0Cu) | (v == SQ_AUTO ? 0u : v == 1 ? 4u : 8u)); break;
    default:         n.fl = (uint8_t)((n.fl & ~0x10u) | (v == 50 ? 0x10u : 0u)); break;
    }
    if (n.note != e->note || n.vel != e->vel || n.gr != e->gr || n.fl != e->fl) {   /* a write that changes nothing takes no undo record */
        sq_edit_record(ch, p);
        *e = n; }
    sq_ch[ch].pg_used |= (uint8_t)(1u << (p / SQ_PAGE));
    return 1; }

/* A white-key tap (1.1 ON): writes the opposite of what the step plays now (its ON field if set, else the generator). If that equals the
 * generator the field is AUTO, so two taps never leave a stale edit. Returns 1 when the step exists. */
static uint8_t sq_step_tap(uint8_t ch, uint8_t p)
{   StepEdit *e;
    uint8_t gen, cur, want;
    if (ch >= SQ_NCH || p >= SQ_NSTEP || p >= sq_ch[ch].len) return 0;
    e = &sq_ch[ch].ed[p];
    gen = sq_gen_hit(ch, p);
    cur = (uint8_t)((e->fl & 3u) == 0u ? gen : (uint8_t)((e->fl & 3u) == 1u));
    want = (uint8_t)!cur;
    sq_edit_record(ch, p);                                            /* a tap always changes the ON field */
    e->fl = (uint8_t)((e->fl & ~3u) | (want == gen ? 0u : want ? 1u : 2u));
    sq_ch[ch].pg_used |= (uint8_t)(1u << (p / SQ_PAGE));
    return 1; }

/* RESET + key (1.2): every field of one step back to AUTO. */
static uint8_t sq_step_clear(uint8_t ch, uint8_t p)
{   const StepEdit *e;
    if (ch >= SQ_NCH || p >= SQ_NSTEP || p >= sq_ch[ch].len) return 0;
    e = &sq_ch[ch].ed[p];
    if (e->note | e->vel | e->gr | e->fl) sq_edit_record(ch, p);      /* an AUTO step is already clear: no record, the older one stays */
    memset(&sq_ch[ch].ed[p], 0, sizeof(StepEdit));
    return 1; }

/* Opens a NOTE_EDIT session on step p (SEQUENCER_UX U16): remembers the step; the first sq_step_set / sq_step_clear that changes it takes the
 * undo record (one for the whole session), so a visit alone does not replace an older record. A step the LENGTH hides is refused. */
static uint8_t sq_note_edit_begin(uint8_t ch, uint8_t p)
{   if (ch >= SQ_NCH || p >= SQ_NSTEP || p >= sq_ch[ch].len) return 0;
    sq_chan_sync();
    sq_ne_ch = ch; sq_ne_p = p; sq_ne_snap = sq_ch[ch].ed[p]; sq_ne_state = 1;
    return 1; }

/* The UI side of the session (P4-014): sq_note_edit_end closes it when NOTE_EDIT is left, so a later PERF tap on the same step takes a record of its
 * own (an open session would swallow it, and EDIT would undo past it); sq_note_edit_on says whether the session is still on (ch, p), because any
 * other undoable action, an EDIT undo or a landed preset ends it silently and the page must then open it again before its next write. */
static void sq_note_edit_end(void) { sq_ne_state = 0; }
static uint8_t sq_note_edit_on(uint8_t ch, uint8_t p) { return (uint8_t)(sq_ne_state && sq_ne_ch == ch && sq_ne_p == p); }
/* sq_undo_step_rec: the latest undo record of the channel is a STEP record (what = 8) and no MIDI-channel record has replaced it: EDIT on NOTE_EDIT may
 * swap it. A dice, a page add or a landed preset has replaced or cleared it (what != 8), and then EDIT there must do nothing. */
static uint8_t sq_undo_step_rec(uint8_t ch) { return (uint8_t)(ch < SQ_NCH && !sq_um_valid && sq_undo_rec[ch].what == 8u && sq_undo_rec[ch].algo < SQ_NALGO); }

/* RESET + REC (1.2): the viewed page's edits to AUTO. Its content flag goes too (1.6: a cleared page is copied again when extended),
 * except on page 1, which always holds content. Undoable (one page). */
static uint8_t sq_page_clear(uint8_t ch, uint8_t k)
{   if (ch >= SQ_NCH || !sq_page_shown(ch, k)) return 0;
    sq_undo_page(ch, k);
    memset(&sq_ch[ch].ed[(uint16_t)k * SQ_PAGE], 0, SQ_PAGE * sizeof(StepEdit));
    if (k) sq_ch[ch].pg_used &= (uint8_t)~(1u << k);
    return 1; }

/* RESET + D#4 (1.2): page k <- page k-1, edits and source (1.4). Page 1 has no page before. Undoable. */
static uint8_t sq_page_recopy(uint8_t ch, uint8_t k)
{   if (ch >= SQ_NCH || k < 1u || !sq_page_shown(ch, k)) return 0;
    sq_undo_page(ch, k);
    sq_page_copy_before(&sq_ch[ch], k);
    return 1; }

/* The factory seed k of algorithm d on channel c (the power-on value sq_default_chan writes; seed_k 0xFF = the literal 0). */
static uint16_t sq_factory_seed(uint8_t c, const AlgoDesc *d, uint8_t k)
{   return (k < d->n_seed && d->seed_k[k] != 0xFFu) ? (uint16_t)((0x3A5Du * (uint32_t)(c * d->n_seed + d->seed_k[k])) & 0xFFFFu) : 0u; }

/* The RESET family (ADR-0003 6, SEQUENCER_UX 1.2, U59..U61; P4-018): morphs back to MorphDesc.def, seeds back to the factory value. Each is ONE undo record
 * (what bit0 seeds, bit2 morphs; sq_undo swaps it, again = redo) taken only when something changes: a reset of what is already default writes no record
 * (the older one stays) and returns 0. Edits (StepEdit) are never touched. `knob` is the slot 0..7 (row * 4 + K), as sq_morph_turn. */
static uint8_t sq_reset_apply(uint8_t ch, uint8_t first, uint8_t n, uint8_t seeds, uint8_t layer_only)   /* morph slots first..first+n-1; seeds: all (layer_only 0) or the layer's (1 + layer) */
{   const uint8_t al = sq_ch[ch].algo;
    const AlgoDesc *a = &ALGOS[al];
    AlgoState *st = &sq_ch[ch].st[al];
    UndoRec u;
    uint8_t changed = 0;
    memset(&u, 0, sizeof u);
    memcpy(u.morph, st->morph, SQ_NMORPH);
    for (uint8_t k = first; k < first + n && k < SQ_NMORPH; k++) {
        const uint8_t id = a->knob_morph[k] & (SQ_NMORPH - 1u);
        if (st->morph[id] != a->morph[id].def) { st->morph[id] = a->morph[id].def; changed |= 4u; } }
    if (seeds) {
        for (uint8_t id = 0; id <= SQ_SEED_MOD; id++) {
            uint16_t *p, want;
            if (layer_only) {
                const LayerDesc *l = &a->layer[layer_only - 1u];
                uint8_t in = 0;
                for (uint8_t k = 0; k < l->n && k < 2u; k++) in |= (uint8_t)(l->seed[k] == id);
                if (!in) continue; }
            else if (id < SQ_NSEED && id >= a->n_seed) continue;                       /* a seed this algorithm does not use stays as it is */
            p = sq_seed_ptr(ch, id);
            want = id == SQ_SEED_MOD ? SQ_SMOD_DEFAULT(ch) : sq_factory_seed(ch, a, id);
            if (*p == want) continue;
            if (id == SQ_SEED_MOD) { u.smod = *p; u.seed_mask |= SQ_UNDO_SMOD; } else { u.seed[id] = *p; u.seed_mask |= (uint8_t)(1u << id); }
            *p = want; changed |= 1u; } }
    if (!changed) return 0;
    u.what = changed; u.algo = al;
    sq_undo_rec[ch] = u; sq_um_valid = 0; sq_ne_state = 0;
    (void)sq_rebuild(ch);
    return 1; }
static uint8_t sq_reset_morph(uint8_t ch, uint8_t knob)               /* one morph to its default */
{   if (ch >= SQ_NCH || knob >= SQ_NMORPH || sq_ch[ch].algo >= SQ_NALGO) return 0;
    sq_chan_sync();
    return sq_reset_apply(ch, knob, 1, 0, 0); }
static uint8_t sq_reset_row(uint8_t ch, uint8_t row)                  /* the four morphs of ROW 1 / ROW 2 */
{   if (ch >= SQ_NCH || row > 1u || sq_ch[ch].algo >= SQ_NALGO) return 0;
    sq_chan_sync();
    return sq_reset_apply(ch, (uint8_t)(row * 4u), 4, 0, 0); }
static uint8_t sq_reset_algo(uint8_t ch)                              /* all 8 morphs and every seed (s_mod too) */
{   if (ch >= SQ_NCH || sq_ch[ch].algo >= SQ_NALGO) return 0;
    sq_chan_sync();
    return sq_reset_apply(ch, 0, SQ_NMORPH, 1, 0); }
static uint8_t sq_reset_layer(uint8_t ch, uint8_t layer)              /* that layer's seeds; an absent layer is inert */
{   if (ch >= SQ_NCH || layer >= SQ_NLAYER || sq_ch[ch].algo >= SQ_NALGO || !ALGOS[sq_ch[ch].algo].layer[layer].n) return 0;
    sq_chan_sync();
    return sq_reset_apply(ch, 0, 0, 1, (uint8_t)(layer + 1u)); }

/* 1.4 K3 SOURCE: page k reads generator block src (0..k; src = k is OWN). Shown pages only. Returns 1 when written. */
static uint8_t sq_page_src_set(uint8_t ch, uint8_t k, uint8_t src)
{   if (ch >= SQ_NCH || !sq_page_shown(ch, k) || src > k) return 0;
    sq_ch[ch].pg_src = (uint8_t)((sq_ch[ch].pg_src & ~(3u << (2u * k))) | (uint8_t)(src << (2u * k)));
    return 1; }

/* 1.6 LENGTH, 1..64. Extend: each page that the new length brings into view and that holds no content is a copy of the page before
 * (whole page, edits and source); a page that was hidden by a shrink comes back as it was; a page already in view is untouched. Shrink:
 * LENGTH only, the steps beyond keep their edits, hidden (pg_used stays). The length is written last, after any copy. Not undoable (1.6). */
static uint8_t sq_len_set(uint8_t ch, uint8_t n)
{   ChanSave *cs;
    uint8_t old, k;
    if (ch >= SQ_NCH) return 0;
    cs = &sq_ch[ch];
    if (n < 1u) n = 1u;
    if (n > SQ_NSTEP) n = SQ_NSTEP;
    old = cs->len;
    if (n == old) return 0;
    for (k = 1u; k < SQ_NSTEP / SQ_PAGE && (uint16_t)k * SQ_PAGE < n; k++)
        if ((uint16_t)k * SQ_PAGE >= old && !((cs->pg_used >> k) & 1u)) sq_page_copy_before(cs, k);
    cs->len = n;
    return 1; }

/* 1.3 PG+ on the last page: complete a partial page (LENGTH to its page boundary, its stored steps come with it), else add the next
 * page: a copy of the page before, or the hidden page as stored. At 64 it does nothing (the UI toasts 64 STEPS = MAX). Undoable. */
static uint8_t sq_page_add(uint8_t ch)
{   uint8_t len, k;
    if (ch >= SQ_NCH) return 0;
    len = sq_ch[ch].len;
    if (len >= SQ_NSTEP) return 0;
    k = (uint8_t)(len / SQ_PAGE);
    sq_undo_page(ch, k);
    return sq_len_set(ch, (uint8_t)((k + 1u) * SQ_PAGE)); }

/* CHAN K4 FOLLOW (1.5): the flag only. */
static void sq_follow_set(uint8_t on) { sq_follow = on ? 1u : 0u; }

/* ------------------------------------------------------------------ channels in use (P4-044) */

/* A channel can be added only when its algorithm row exists (ALGOS[].build, a table read: ADR-0007). */
static uint8_t sq_chan_addable(uint8_t ch)
{   return (uint8_t)(ch < SQ_NCH && !(sq_used >> ch & 1u) && sq_ch[ch].algo < SQ_NALGO && ALGOS[sq_ch[ch].algo].build != NULL); }

/* A mask from a loaded preset: a channel whose algorithm row does not exist is dropped, and CH1 is used when nothing else is, so the core is never empty (CH1 itself may be removed while another channel is in use). */
static uint8_t sq_used_clean(uint8_t mask, const ChanSave *chs)
{   uint8_t r = 0;
    for (uint8_t c = 0; c < SQ_NCH; c++)
        if ((mask >> c & 1u) && chs[c].algo < SQ_NALGO && ALGOS[chs[c].algo].build != NULL) r |= (uint8_t)(1u << c);
    return r ? r : 1u; }

/* ------------------------------------------------------------------ defaults (ADR-0003 12) */

static void sq_default_chan(uint8_t c, ChanSave *cs)
{   static const uint8_t algo_of[SQ_NCH] = { SQ_ALGO_ACID, SQ_ALGO_EUCLID, SQ_ALGO_SNAKE, SQ_ALGO_STAGE };
    memset(cs, 0, sizeof *cs);
    cs->midi_ch = c ? SQ_MIDI_OFF : 0;     /* CH1 = MIDI 1 (decisions 73, 77). A channel out of use holds no MIDI channel (BUG-011, P4-044): CH2..CH4 start OFF and get one when added */
    cs->algo = algo_of[c];
    cs->muted = c ? 1u : 0u;               /* CH1 unmuted; CH2..CH4 muted */
    cs->len = 16;
    cs->pg_src = 0;
    cs->pg_used = 1;                       /* page 1 holds content */
    cs->pad[1] = c ? 0u : 1u;              /* P4-044: bit 0 = channel in use; power-on is CH1 only */
    for (uint8_t a = 0; a < SQ_NALGO; a++) {                          /* every algorithm slot, so a later switch starts from its own defaults */
        const AlgoDesc *d = &ALGOS[a];
        for (uint8_t i = 0; i < SQ_NMORPH; i++) cs->st[a].morph[i] = d->morph[i].def;
        for (uint8_t k = 0; k < SQ_NSEED; k++) cs->st[a].seed[k] = sq_factory_seed(c, d, k);
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

/* ADR-0003 12 step 4 minus audio: fill the state from the defaults, build all four caches into buffer 0, zero the runtime. Sets sq_ready last
 * (the cut-over, P4-012: seq.c events_block runs sq_block from then on). Call before the audio ISR starts. */
static void sq_boot(void)
{   sq_default_global(&sq_gdb[0]); sq_gdb[1] = sq_gdb[0];
    sq_used = 0x01u;
    sq_gidx = 0; sq_load_pending = 0; sq_um_valid = 0; sq_stage_valid = 0; sq_ne_state = 0;
    memset(sq_undo_rec, 0, sizeof sq_undo_rec);
    sq_default_harm(&sq_harm); sq_bpm = SQ_BPM_DEFAULT;
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        sq_default_chan(c, &sq_ch[c]);
        for (uint8_t a = 0; a < SQ_NALGO; a++) sq_smod[c][a] = SQ_SMOD_DEFAULT(c);
        sq_build_buf(c, 0, &sq_gdb[0], &sq_ch[c], sq_smod[c]);
        sq_cidx[c] = 0;
    }
    memset(sq_rt, 0, sizeof sq_rt);
    for (int i = 0; i < SQ_EVQ_N; i++) sq_evq[i].kind = 0;
    sq_seen_epoch = sq_epoch;
    sq_ready = 1; }                                                  /* last: the ISR plays only a state that is complete (ADR-0003 12) */

/* ------------------------------------------------------------------ presets (ADR-0003 10; RAM only) */

/* ms: the sibling ModSave array (SQ_NCH entries) or NULL to skip it. */
static int sq_preset_save(SeqPreset *out, ModSave *ms)
{   if (ms) for (uint8_t c = 0; c < SQ_NCH; c++) { memset(&ms[c], 0, sizeof ms[c]); for (uint8_t a = 0; a < SQ_NALGO; a++) { ms[c].smod[2 * a] = (uint8_t)sq_smod[c][a]; ms[c].smod[2 * a + 1] = (uint8_t)(sq_smod[c][a] >> 8); } }
      memset(out, 0, sizeof *out);
    out->version = SQ_PRESET_VERSION; out->flags = 0; out->bpm = sq_bpm;
    out->g = sq_gdb[sq_gidx].s;
    for (uint8_t c = 0; c < SQ_NCH; c++) { out->ch[c] = sq_ch[c]; out->ch[c].pad[1] = (uint8_t)(sq_used >> c & 1u); }
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
 * (applied at once if stopped). ms (SQ_NCH entries) carries s_mod; NULL keeps the live s_mod. The caches are built into the inactive buffers first; sq_load_pending is set last. */
static int sq_preset_load(const SeqPreset *in, const ModSave *ms, uint8_t when)
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
    for (uint8_t c = 0; c < SQ_NCH; c++)
        for (uint8_t a = 0; a < SQ_NALGO; a++) sq_stage_smod[c][a] = ms ? (uint16_t)(ms[c].smod[2 * a] | ms[c].smod[2 * a + 1] << 8) : sq_smod[c][a];
    for (uint8_t c = 0; c < SQ_NCH; c++) {
        ChanSave *s = &sq_stage_ch[c];
        *s = in->ch[c];
        s->muted = s->muted ? 1u : 0u;
        s->len = sq_clamp8(s->len, 1, SQ_NSTEP);
        s->pg_used &= 0x0Fu;
        s->pad[1] &= 1u;                                             /* P4-044: bit 0 = in use, the rest reserved */
        if (s->midi_ch > SQ_MIDI_OFF) s->midi_ch = SQ_MIDI_OFF;
        if (s->midi_ch < SQ_MIDI_OFF)
            for (uint8_t o = 0; o < c; o++) if (sq_stage_ch[o].midi_ch == s->midi_ch) { s->midi_ch = SQ_MIDI_OFF; break; }
        for (uint8_t a = 0; a < SQ_NALGO; a++)
            for (uint8_t i = 0; i < SQ_NMORPH; i++)
                s->st[a].morph[i] = sq_clamp8(s->st[a].morph[i], ALGOS[a].morph[i].min, ALGOS[a].morph[i].max);
        for (uint8_t i = 0; i < SQ_NSTEP; i++) sq_clean_edit(&s->ed[i]);
        sq_build_buf(c, (uint8_t)(sq_cidx[c] ^ 1u), sg, s, sq_stage_smod[c]);
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
    sq_um_valid = 0; sq_ne_state = 0;
    if (sq_stage_valid) { uint8_t m = 0; for (uint8_t c = 0; c < SQ_NCH; c++) m |= (uint8_t)((sq_ch[c].pad[1] & 1u) << c); sq_used = sq_used_clean(m, sq_ch);   /* P4-044: the landed load's in-use bits */
        for (uint8_t c = 0; c < SQ_NCH; c++) if (!(sq_used >> c & 1u)) { sq_ch[c].muted = 1u; sq_ch[c].pad[0] &= 0x7Fu; sq_ch[c].midi_ch = SQ_MIDI_OFF; }   /* F2: a channel out of use is silent and holds no MIDI channel, whatever the file said */
        sq_load_landed = 1;
        sq_harm = sq_stage_harm; sq_bpm = sq_stage_bpm; memcpy(sq_smod, sq_stage_smod, sizeof sq_smod); sq_stage_valid = 0; }
    for (uint8_t c = 0; c < SQ_NCH; c++) (void)sq_rebuild(c);        /* a dice or undo while the load was pending changed s_mod; the staged smod replaced it above */
}
