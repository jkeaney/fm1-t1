#include <stdio.h>
#include <string.h>
#include "torso_t1.h"
#include "torso_t1_algo.h"
#include "torso_t1_pipeline.h"
#include "torso_t1_state.h"
#include "torso_t1_ui.h"
static uint32_t cur_tick; static int fon,foff;
void synth_note_on(uint8_t v,uint8_t n,uint8_t vel){(void)v;(void)n;(void)vel;fon++;}
void synth_note_off(uint8_t v){(void)v;foff++;}
static int mon,moff; static uint32_t first_on[8]; static int n_on_t[8];
static void cb(uint32_t pkt){int s=(pkt>>8)&0xF0; if(s==0x90)mon++; if(s==0x80)moff++;}
static void fresh(void){ t1_system_init(); t1_pipeline_init(); t1_state_init(); t1_ui_init(); fon=foff=mon=moff=0; }
static void run(uint32_t a,uint32_t b){ for(cur_tick=a;cur_tick<b;cur_tick++){ t1_pipeline_tick(cur_tick); t1_midi_worker_flush_usb(cb); t1_midi_worker_flush_uart(NULL);} }
int main(void){
  puts("## A stuck notes after overflow: stress, then silence, drain 5000 ticks");
  fresh(); for(int t=0;t<NUM_TRACKS;t++){ CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[t].cycles[0]; cp->pulses=16; cp->repeats=16; cp->chord_size=4; cp->sustain=100; cp->time=127; }
  run(0,48*16*2);
  for(int t=0;t<NUM_TRACKS;t++){ t1_sequencer_ram.active_pattern.tracks[t].cycles[0].pulses=0; } t1_pipeline_invalidate_masks();
  run(48*16*2, 48*16*2+5000);
  printf("dropped=%u | MIDI note-on=%d note-off=%d (unmatched=%d) | FM on=%d off=%d (unmatched=%d)\n",t1_micro_queue.dropped_events,mon,moff,mon-moff,fon,foff,fon-foff);
  puts("## B moderate load: all 6 tracks, repeats=4 chord=2, 16 pulses: any drops?");
  fresh(); for(int t=0;t<NUM_TRACKS;t++){ CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[t].cycles[0]; cp->pulses=16; cp->repeats=4; cp->chord_size=2; } run(0,48*16*2);
  printf("dropped=%u\n",t1_micro_queue.dropped_events);
  puts("## C cycle select by key (t1_ui_select_cycle) vs encoder: does the rhythm change?");
  fresh(); { Track*tk=&t1_sequencer_ram.active_pattern.tracks[0]; tk->cycles[0].pulses=4; tk->cycles[1].pulses=12; run(0,48*16); int a=fon; fon=0;
    t1_ui_select_track(0); t1_ui_select_cycle(1); run(48*16,48*16*2); int b=fon; fon=0;
    t1_pipeline_invalidate_masks(); run(48*16*2,48*16*3); int c=fon;
    printf("cycle0 (4 pulses) bar: %d note-ons | after select_cycle(1) (12 pulses): %d | after explicit mask invalidate: %d\n",a,b,c); }
  puts("## D random-target knob: one detent at a time");
  fresh(); { CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; t1_ui_set_page(T1_PAGE_MOD_CC); for(int i=0;i<6;i++){ t1_ui_on_encoder_turn(0,+1); printf("detent %d -> random_targets=0x%02X\n",i+1,cp->random_targets);} t1_ui_on_encoder_turn(0,-1); printf("one detent back -> 0x%02X\n",cp->random_targets); for(int n=2;n<=8;n++){ cp->random_targets=0; t1_ui_on_encoder_turn(0,n); printf("%d detents in one frame -> 0x%02X\n",n,cp->random_targets);} }
  puts("## E step-mod knob (RHYTHM, shift, enc 3)");
  fresh(); { CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[0].cycles[0]; t1_ui_set_shift(true); t1_ui_set_page(T1_PAGE_RHYTHM); for(int i=0;i<3;i++){ t1_ui_on_encoder_turn(3,+1); printf("detent %d -> active_mask_lo=0x%08X\n",i+1,cp->active_mask_lo);} }
  puts("## F muted track phase");
  fresh(); { for(int t=0;t<2;t++){ CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[t].cycles[0]; cp->pulses=16; }
    run(0,48*3); t1_sequencer_ram.active_pattern.tracks[0].muted=true; run(48*3,48*3+30); t1_sequencer_ram.active_pattern.tracks[0].muted=false;
    printf("after un-mute: track0 step_index=%u tick_accum=%u | track1 step_index=%u tick_accum=%u (tick now %u)\n",t1_track_runtimes[0].step_index,t1_track_runtimes[0].tick_accum,t1_track_runtimes[1].step_index,t1_track_runtimes[1].tick_accum,48*3+30); }
  puts("## G zeroed p-lock arena");
  { CycleParams cp; memset(&cp,0,sizeof cp); printf("set(step3,param57) on zeroed arena -> %d (0 = refused: arena looks full)\n",t1_plock_set(&cp,3,57,64)); }
  puts("## H LFSR determinism across loops: probability 50%, 16 pulses, loop_period 1: do consecutive bars match?");
  fresh(); { CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[1].cycles[0]; cp->pulses=16; cp->probability=50; t1_lfsr_init(&t1_track_runtimes[1].lfsr,0x2345,0);
    int bar[4][16]; memset(bar,0,sizeof bar); extern uint32_t cur_tick; (void)cur_tick;
    for(uint32_t t=0;t<48*16*4;t++){ int before=mon; t1_pipeline_tick(t); t1_midi_worker_flush_usb(cb); if(mon>before) bar[t/(48*16)][(t%(48*16))/48]=1; }
    for(int b=0;b<4;b++){ printf("bar %d: ",b+1); for(int s=0;s<16;s++) putchar(bar[b][s]?'x':'.'); puts(""); } }
  puts("## I same with random_rate=3 (loop 4 cycles)");
  fresh(); { CycleParams*cp=&t1_sequencer_ram.active_pattern.tracks[1].cycles[0]; cp->pulses=16; cp->probability=50; t1_lfsr_init(&t1_track_runtimes[1].lfsr,0x2345,3);
    int bar[9][16]; memset(bar,0,sizeof bar);
    for(uint32_t t=0;t<48*16*9;t++){ int before=mon; t1_pipeline_tick(t); t1_midi_worker_flush_usb(cb); if(mon>before) bar[t/(48*16)][(t%(48*16))/48]=1; }
    for(int b=0;b<9;b++){ printf("bar %d: ",b+1); for(int s=0;s<16;s++) putchar(bar[b][s]?'x':'.'); puts(""); } }
  return 0; }
