#include "seg.hpp"
#include <cstdio>
#include <cmath>
using namespace oildrum;
// tail energy (after 50 ms) and duration-to--60dB relative to B=512, per block size
int main(){
  for (int sr : {44100,48000,96000,192000}) for (int B : {1,16,32,64,128}) {
    double worstDb=0, worstDurPct=0; int worstInst=-1;
    for (int inst=0; inst<kNumInstruments; ++inst) {
      double e[2]={0,0}, dur[2]={0,0};
      for (int s=0;s<5;++s) for (float vel : {0.3f,1.0f}) for (int pass=0; pass<2; ++pass) {
        int b = pass? 512 : B; std::vector<Ev> ev={{100,inst,vel}}; int N=sr*3; std::vector<float> L,R;
        DrumEngine a(sr); a.seed(100+s); a.setRoomMix(0); renderSeg(a,ev,N,b,L,R);
        double pk=0; for(int i=0;i<N;++i) pk=std::max(pk,(double)std::fabs(L[i]));
        int last=0; for(int i=0;i<N;++i) if(std::fabs(L[i])>pk*1e-3) last=i;
        dur[pass]+=last; for(int i=100+sr/20;i<N;++i) e[pass]+=L[i]*(double)L[i]+R[i]*(double)R[i];
      }
      double db=10*std::log10(e[0]/e[1]); double dp=100*(dur[0]/dur[1]-1);
      if (std::fabs(db)>std::fabs(worstDb)) {worstDb=db; worstInst=inst;} if (std::fabs(dp)>std::fabs(worstDurPct)) worstDurPct=dp;
    }
    printf("sr=%6d B=%3d vs512: worst tail-energy %+.3f dB (inst %d), worst -60dB-duration %+.2f%%\n",sr,B,worstDb,worstInst,worstDurPct);
  }
}
