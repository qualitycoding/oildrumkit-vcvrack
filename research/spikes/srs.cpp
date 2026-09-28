#include "seg.hpp"
#include <cstdio>
#include <cmath>
using namespace oildrum;
int main(){
  for (int sr : {44100,48000,88200,96000,176400,192000,352800,384000,705600,768000}) {
    double worstIso=0; bool finite=true; double peak=0;
    for (int inst=0; inst<kNumInstruments; ++inst) for (float vel : {0.1f, 0.5f, 1.0f}) {
      std::vector<Ev> ev={{100,inst,vel}}; int N=sr/2; std::vector<float> L1,R1,L2,R2;
      DrumEngine a(sr),b(sr); a.seed(11); b.seed(11); renderSeg(a,ev,N,32,L1,R1); renderSeg(b,ev,N,512,L2,R2);
      for(int i=0;i<N;++i){ if(!std::isfinite(L1[i])||!std::isfinite(R1[i])) finite=false; worstIso=std::max(worstIso,(double)std::max(std::fabs(L1[i]-L2[i]),std::fabs(R1[i]-R2[i]))); peak=std::max(peak,(double)std::fabs(L1[i])); }
    }
    printf("sr=%6d finite=%d peak=%.3f isolated B32-vs-B512 maxdiff=%.3g\n",sr,finite,peak,worstIso);
  }
}
