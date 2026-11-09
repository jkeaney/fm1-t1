/* Throwaway probe: exercises the real T1 sources and prints facts for the docs. Not part of the repo. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "torso_t1.h"
#include "torso_t1_algo.h"
#include "torso_t1_pipeline.h"
#include "torso_t1_state.h"
#include "torso_t1_ui.h"

/* ---- capture hooks: override the weak FM stubs ---- */
typedef struct { uint32_t tick; int on; int v; int note; int vel; } FmEv;
static FmEv fm[4096]; static int nfm; static uint32_t cur_tick;
void synth_note_on(uint8_t v, uint8_t n, uint8_t vel){ if(nfm<4096) fm[nfm++]=(FmEv){cur_tick,1,v,n,vel}; }
void synth_note_off(uint8_t v){ if(nfm<4096) fm[nfm++]=(FmEv){cur_tick,0,v,0,0}; }
typedef struct { uint32_t tick; int type; int ch; int a; int b; } MidiEv;
static MidiEv mev[8192]; static int nmev;
static void usb_cb(uint32_t pkt){ int s=(pkt>>8)&0xFF; if(nmev<8192) mev[nmev++]=(MidiEv){cur_tick,s&0xF0,s&0x0F,(pkt>>16)&0xFF,(pkt>>24)&0xFF}; }

static void bits(uint64_t m,int n){ for(int i=0;i<n;i++) putchar((m>>i)&1?'x':'.'); }
static void fresh(void){ t1_system_init(); t1_pipeline_init(); t1_state_init(); t1_ui_init(); nfm=0; nmev=0; }

static void run(uint32_t ticks){ for(cur_tick=0;cur_tick<ticks;cur_tick++){ t1_pipeline_tick(cur_tick); t1_midi_worker_flush_usb(usb_cb); t1_midi_worker_flush_uart(NULL);} }

int main(void){
  puts("## 1 EUCLIDEAN  (x = pulse)  steps,pulses -> pattern | reference Bresenham equals up to rotation?");
  int pairs[][2]={{8,3},{8,5},{16,4},{16,5},{16,7},{16,9},{12,5},{13,5},{64,23},{7,3},{9,4},{10,3}};
  for(unsigned k=0;k<sizeof pairs/sizeof*pairs;k++){
    int s=pairs[k][0],p=pairs[k][1]; uint64_t m=t1_euclidean_bjorklund(s,p);
    /* reference: bit i set if floor((i+1)*p/s) != floor(i*p/s) */
    uint64_t ref=0; for(int i=0;i<s;i++) if(((i+1)*p)/s != (i*p)/s) ref|=1ULL<<i;
    int eq=0; for(int r=0;r<s&&!eq;r++){ if(t1_euclidean_rotate(m,s,r)==ref) eq=1; }
    int pc=__builtin_popcountll(m);
    printf("E(%d,%d) ",p,s); bits(m,s); printf("  pop=%d  rot-equal-ref=%d\n",pc,eq);
  }
  puts("## 2 SCALE MASKS: name -> pitch classes set in code");
  for(int i=0;i<T1_NUM_SCALES;i++){ printf("%2d %-11s 0x%03X:",i,T1_SCALE_NAMES[i],T1_SCALE_MASKS[i]); for(int b=0;b<12;b++) if((T1_SCALE_MASKS[i]>>b)&1) printf(" %d",b); puts(""); }
  puts("## 3 QUANTIZE root=0: raw 60..71 -> out, per scale 1(Major) 2(Minor) 5(MajPent)");
  int sc[]={1,2,5,10}; for(int q=0;q<4;q++){ printf("scale %d:",sc[q]); for(int n=60;n<72;n++) printf(" %d",t1_harmony_quantize(n,0,sc[q])); puts(""); }
  printf("root=9(A) scale 10(min pent) raw 57..69:"); for(int n=57;n<70;n++) printf(" %d",t1_harmony_quantize(n,9,10)); puts("");
  puts("## 4 PHRASE offsets, total_steps=16, range=1 (max 7 semitones)");
  const char*pn[]={"FLAT","SINE","TRI","SAWU","SAWD","ARCH","INVA","RAND"};
  for(int sh=0;sh<8;sh++){ printf("%-5s",pn[sh]); for(int i=0;i<16;i++) printf(" %3d",t1_harmony_eval_phrase(sh,1,i,16)); puts(""); }
  printf("TRI range=12 (device default) max:"); int mx=0; for(int i=0;i<16;i++){int d=t1_harmony_eval_phrase(2,12,i,16); if(d>mx)mx=d;} printf(" %d semitones\n",mx);
  puts("## 5 STYLE index order, pool=4 (steps 0..11)");
  const char*sn[]={"UP","DOWN","UPDN","DNUP","CNVRG","DIVRG","RAND","ORDER"};
  for(int st=0;st<8;st++){ printf("%-5s",sn[st]); for(int i=0;i<12;i++) printf(" %d",t1_harmony_eval_style(st,i,4)); puts(""); }
  puts("## 5b STYLE pool=7 steps 0..13 (what the pipeline uses)");
  for(int st=0;st<8;st++){ printf("%-5s",sn[st]); for(int i=0;i<14;i++) printf(" %d",t1_harmony_eval_style(st,i,7)); puts(""); }
  puts("## 6 PACE: tick offsets for 4 repeats, 192-tick step, time=64");
  int paces[]={-64,-32,0,32,63};
  for(int q=0;q<5;q++){ CycleParams cp; memset(&cp,0,sizeof cp); cp.repeats=4; cp.pace=paces[q]; cp.time=64; cp.velocity=100; T1RatchetPattern r; t1_pace_eval_ratchet(&cp,192,&r); printf("pace %3d:",paces[q]); for(int k=0;k<4;k++) printf(" %3u(g%u)",r.tick_offsets[k],r.gate_ticks[k]); puts(""); }
  { CycleParams cp; memset(&cp,0,sizeof cp); cp.repeats=8; cp.pace=0; cp.time=127; cp.velocity=100; cp.offset=-40; T1RatchetPattern r; t1_pace_eval_ratchet(&cp,48,&r); printf("8 repeats in 48t offset=-40 velocities:"); for(int k=0;k<8;k++) printf(" %u",r.velocities[k]); printf(" | offsets:"); for(int k=0;k<8;k++) printf(" %u",r.tick_offsets[k]); puts(""); }
  puts("## 7 LFSR: period and modulate range");
  { T1LFSRState l; t1_lfsr_init(&l,0x1234,0); uint16_t start=l.state; uint32_t n=0; do{ t1_lfsr_next(&l); n++; }while(l.state!=start && n<70000); printf("period from 0x1234: %u\n",n);
    int lo=999,hi=-999; for(int i=0;i<100000;i++){ int32_t v=t1_lfsr_modulate(&l,T1_RND_TARGET_PITCH,T1_RND_TARGET_PITCH,60,127,0,127); int d=v-60; if(d<lo)lo=d; if(d>hi)hi=d; } printf("modulate depth=127 delta range: %d..%d\n",lo,hi);
    lo=999;hi=-999; for(int i=0;i<100000;i++){ int32_t v=t1_lfsr_modulate(&l,T1_RND_TARGET_PITCH,T1_RND_TARGET_PITCH,60,24,0,127); int d=v-60; if(d<lo)lo=d; if(d>hi)hi=d; } printf("modulate depth=24 delta range: %d..%d\n",lo,hi);
    for(int r=0;r<9;r++){ T1LFSRState q; t1_lfsr_init(&q,1,r); printf("random_rate %d -> loop_period %u\n",r,q.loop_period); } }
  puts("## 8 SWING step lengths at 1/16 (48t)");
  { fresh(); for(int sw=50;sw<=80;sw+=10){ t1_sequencer_ram.active_pattern.swing=sw; CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; cp->pulses=16; cp->steps=16;
      t1_pipeline_reset_transport(); t1_pipeline_invalidate_masks(); nfm=0; run(48*4+1);
      printf("swing %d note-on ticks:",sw); for(int i=0;i<nfm;i++) if(fm[i].on) printf(" %u",fm[i].tick); puts(""); } }
  puts("## 9 DEFAULT-INITIALISED track 0, 4 pulses of 16, style 0, phrase 0, scale 0: note sequence over 3 bars");
  { fresh(); CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; cp->pulses=4; cp->scale_id=0; run(48*16*3);
    printf("notes:"); for(int i=0;i<nfm;i++) if(fm[i].on) printf(" %d",fm[i].note); puts(""); }
  puts("## 9b same, style=1(DOWN)"); { fresh(); CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; cp->pulses=4; cp->style=1; run(48*16*3); printf("notes:"); for(int i=0;i<nfm;i++) if(fm[i].on) printf(" %d",fm[i].note); puts(""); }
  puts("## 9c repeats=3, style=2: repeat pitches"); { fresh(); CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; cp->pulses=2; cp->repeats=3; cp->style=2; run(48*16*2); printf("notes:"); for(int i=0;i<nfm;i++) if(fm[i].on) printf(" %d",fm[i].note); puts(""); }
  puts("## 10 DEVICE DEFAULT TB-3 bass (t1_default_pattern replica), 8 bars, track 1 -> MIDI ch index 1");
  { fresh(); CycleParams*b=&t1_sequencer_ram.active_pattern.tracks[1].cycles[0];
    b->steps=16;b->pulses=7;b->division=4;b->pitch_offset=-27;b->root_note=9;b->scale_id=10;b->phrase=2;b->range=12;b->time=127;b->sustain=90;b->velocity=80;b->accent=40;b->probability=95;b->random_targets=T1_RND_TARGET_PITCH|T1_RND_TARGET_ACCENT;b->random_depth=24;b->random_rate=3;
    t1_sequencer_ram.active_pattern.tracks[1].midi_channel=1;
    for(int t=0;t<NUM_TRACKS;t++){ const Track*k=&t1_sequencer_ram.active_pattern.tracks[t]; t1_lfsr_init(&t1_track_runtimes[t].lfsr,(uint16_t)(0x1234u+t*0x1111u),k->cycles[0].random_rate);}
    run(48*16*8); int lo=999,hi=0,cnt=0; int hist[128]={0}; for(int i=0;i<nmev;i++) if(mev[i].type==0x90){ cnt++; if(mev[i].a<lo)lo=mev[i].a; if(mev[i].a>hi)hi=mev[i].a; hist[mev[i].a]++; }
    printf("note-ons=%d  lowest=%d highest=%d  first bar:",cnt,lo,hi); int shown=0; for(int i=0;i<nmev&&shown<10;i++) if(mev[i].type==0x90){ printf(" %d@%u(v%d)",mev[i].a,mev[i].tick,mev[i].b); shown++; } puts("");
    int offs=0; for(int i=0;i<nmev;i++) if(mev[i].type==0x80) offs++; printf("note-offs=%d\n",offs);
    int distinct=0; for(int n=0;n<128;n++) if(hist[n]) distinct++; printf("distinct pitches=%d\n",distinct); }
  puts("## 11 GLOBAL page-4 bug: turn each encoder with GLOBAL active on MOD/CC page; do STEPS change?");
  { for(int e=0;e<4;e++){ for(int sh=0;sh<2;sh++){ fresh(); t1_ui_set_page(T1_PAGE_MOD_CC); t1_ui_set_global(true); t1_ui_set_shift(sh); int before=t1_sequencer_ram.active_pattern.tracks[2].cycles[0].steps; t1_ui_on_encoder_turn(e,+3); int after=t1_sequencer_ram.active_pattern.tracks[2].cycles[0].steps; printf("enc %d shift %d: steps %d -> %d  random_depth=%d delay=%d\n",e,sh,before,after,t1_sequencer_ram.active_pattern.tracks[2].cycles[0].random_depth,t1_sequencer_ram.active_pattern.tracks[2].cycles[0].delay_ticks);} } }
  puts("## 12 RHYTHM page GLOBAL enc3 (cycle knob) -> ?"); { fresh(); t1_ui_set_page(T1_PAGE_RHYTHM); t1_ui_set_global(true); int b=t1_sequencer_ram.active_pattern.tracks[0].cycles[0].steps; t1_ui_on_encoder_turn(3,+4); printf("steps %d -> %d, active_cycle=%d\n",b,t1_sequencer_ram.active_pattern.tracks[0].cycles[0].steps,t1_sequencer_ram.active_pattern.tracks[0].active_cycle); }
  puts("## 13 MASTER LENGTH OVERRIDE has effect?"); { fresh(); CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; cp->pulses=16; t1_sequencer_ram.active_pattern.master_length_override=4; run(48*16); int n=0; for(int i=0;i<nfm;i++) if(fm[i].on) n++; printf("note-ons in one 16-step bar with override=4: %d (16 expected if ignored)\n",n); }
  puts("## 14 QUEUE OVERFLOW: all 6 tracks, repeats=16, chord=4, 1/16, 16 pulses");
  { fresh(); for(int t=0;t<NUM_TRACKS;t++){ CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[t].cycles[0]; cp->pulses=16; cp->repeats=16; cp->chord_size=4; cp->sustain=100; cp->time=127; }
    run(48*16*2); int on=0,off=0; for(int i=0;i<nmev;i++){ if(mev[i].type==0x90)on++; if(mev[i].type==0x80)off++; } int fon=0,foff=0; for(int i=0;i<nfm;i++){ if(fm[i].on)fon++; else foff++; }
    printf("dropped_events=%u  midi on=%d off=%d  fm on=%d off=%d  uart_dropped=%u usb_dropped=%u\n",t1_micro_queue.dropped_events,on,off,fon,foff,t1_midi_uart_queue.dropped_bytes,t1_midi_usb_queue.dropped_bytes); }
  puts("## 15 P-LOCK arena: system_init sets 0xFF; fresh memset(0) cycle behaves?");
  { CycleParams cp; memset(&cp,0,sizeof cp); int16_t v; printf("zeroed arena: lock(step0,param0) found=%d\n",t1_plock_get(&cp,0,0,&v)); t1_plock_clear_all(&cp); printf("after clear_all found=%d\n",t1_plock_get(&cp,0,0,&v)); int ok=0; for(int i=0;i<12;i++) ok+=t1_plock_set(&cp,i,57,i); printf("set 12 locks: accepted=%d (arena=11)\n",ok); }
  puts("## 16 FLASH: bank offsets and sector sharing");
  { for(int b=0;b<6;b++) printf("bank %d off=0x%05X end=0x%05X sector64=%u\n",b,t1_flash_get_bank_offset(b),t1_flash_get_bank_offset(b)+T1_FLASH_BANK_ALLOCATION-1,t1_flash_get_bank_offset(b)/T1_FLASH_SECTOR_SIZE); printf("verify=%d  sizeof(Pattern)*6=%zu\n",t1_flash_verify_boundaries(),sizeof(Pattern)*6); }
  puts("## 17 TEMP revert/commit");
  { fresh(); CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; cp->pulses=3; t1_temp_enter(); cp->pulses=9; t1_temp_revert(); printf("after revert pulses=%d (3 expected)\n",cp->pulses); t1_temp_enter(); cp->pulses=9; t1_temp_commit(); t1_temp_enter(); cp->pulses=1; t1_temp_revert(); printf("after commit+revert pulses=%d (9 expected)\n",cp->pulses); }
  return 0;
}
