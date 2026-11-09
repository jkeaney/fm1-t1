/* LFSR draw correlation: consecutive t1_lfsr_modulate calls (pitch then velocity) on one track's LFSR */
#include <stdio.h>
#include <math.h>
#include "torso_t1.h"
#include "torso_t1_algo.h"
int main(void){
  T1LFSRState l; t1_lfsr_init(&l,0x1234,0);
  puts("## consecutive raw draws: s2 vs s1 (first 6)");
  { T1LFSRState q; t1_lfsr_init(&q,0x1234,0); for(int i=0;i<6;i++){ uint16_t a=t1_lfsr_next(&q), b=t1_lfsr_next(&q); printf("s1=0x%04X s2=0x%04X  s1>>1=0x%04X  (s2 == s1>>1 ^ taps? %d)\n",a,b,a>>1,(b==(uint16_t)((a>>1)^((a&1)?0xB400:0))) ); } }
  const int N=65535/2; double sp=0,sv=0,spp=0,svv=0,spv=0; int near=0;
  for(int i=0;i<N;i++){
    int p=t1_lfsr_modulate(&l,T1_RND_TARGET_PITCH,T1_RND_TARGET_PITCH,64,127,0,127)-64;      /* depth 127 */
    int v=t1_lfsr_modulate(&l,T1_RND_TARGET_VELOCITY,T1_RND_TARGET_VELOCITY,64,127,1,127)-64;
    sp+=p; sv+=v; spp+=p*p; svv+=v*v; spv+=p*v;
    /* velocity draw is the next LFSR state: rnd2 mod 128 == (rnd1 mod 256)>>1 */
  }
  double mp=sp/N, mv=sv/N, cov=spv/N-mp*mv, vp=spp/N-mp*mp, vv=svv/N-mv*mv;
  printf("## pitch/velocity modulation, depth 127, %d pairs: correlation = %.3f\n",N,cov/sqrt(vp*vv));
  /* exact relation check */
  T1LFSRState q; t1_lfsr_init(&q,0x1234,0); int exact=0,tot=20000;
  for(int i=0;i<tot;i++){ uint16_t r1=t1_lfsr_next(&q), r2=t1_lfsr_next(&q); if((r2%128u)==((r1%256u)>>1)) exact++; }
  printf("rnd2 mod 128 == (rnd1 mod 256)>>1 in %d of %d consecutive pairs\n",exact,tot);
  /* probability roll (mod 100) then pitch: conditional spread */
  T1LFSRState w; t1_lfsr_init(&w,0x1234,0); int cnt[2][2]={{0,0},{0,0}}; 
  for(int i=0;i<N;i++){ uint16_t a=t1_lfsr_next(&w)%100u; uint16_t b=t1_lfsr_next(&w)%100u; cnt[a<50][b<50]++; }
  printf("P(second<50 | first<50) = %.3f ; P(second<50 | first>=50) = %.3f (independent would be 0.5 both)\n",(double)cnt[1][1]/(cnt[1][0]+cnt[1][1]),(double)cnt[0][1]/(cnt[0][0]+cnt[0][1]));
  return 0; }
