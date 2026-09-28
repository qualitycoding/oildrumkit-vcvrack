#include "seg.hpp"
#include <cstdio>
#include <cmath>
using namespace oildrum;
int main(){
  // (1) DrumEngine(44100)->setSampleRate(48000) == DrumEngine(48000)
  std::vector<Ev> ev; for(int t=100,k=0;t<48000*2;t+=4321,++k) ev.push_back({t,k%kNumInstruments,0.7f});
  DrumEngine a(44100); a.setSampleRate(48000); a.seed(5); DrumEngine b(48000); b.seed(5);
  std::vector<float> L1,R1,L2,R2; renderSeg(a,ev,96000,128,L1,R1); renderSeg(b,ev,96000,128,L2,R2);
  double d=0; for(size_t i=0;i<L1.size();++i) d=std::max(d,(double)std::fabs(L1[i]-L2[i])+std::fabs(R1[i]-R2[i])); printf("reprepare maxdiff=%g\n",d);
  // (2) after reset via setSampleRate+allNotesOff while ringing -> exact zeros?
  DrumEngine c(48000); c.seed(2); c.trigger(Ride,1.0f); float l[4800]={0},r[4800]={0}; c.process(l,r,4800);
  c.setSampleRate(48000); c.allNotesOff(); float l2[48000]={0},r2[48000]={0}; c.process(l2,r2,48000); double m=0; for(int i=0;i<48000;++i) m=std::max(m,(double)std::fabs(l2[i])+std::fabs(r2[i])); printf("post-reset max=%g\n",m);
  // (3) first nonzero sample index after trigger at sample 0, per instrument
  for(int i=0;i<kNumInstruments;++i){ DrumEngine e(48000); e.seed(1); e.trigger(i,0.1f); float x[64]={0},y[64]={0}; e.process(x,y,64); int f=-1; for(int k=0;k<64;++k) if(x[k]!=0||y[k]!=0){f=k;break;} printf("%d:%d ",i,f);} printf("\n");
  // (4) silence: no triggers -> exact zero
  DrumEngine s(48000); float z[48000]={0},w[48000]={0}; s.process(z,w,48000); double q=0; for(int i=0;i<48000;++i) q=std::max(q,(double)std::fabs(z[i])); printf("silence max=%g\n",q);
}
