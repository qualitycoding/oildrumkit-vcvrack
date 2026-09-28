#include "seg.hpp"
#include <cstdio>
#include <chrono>
#include <cmath>
using namespace oildrum;
// same algorithm, but inlined in this TU (different inlining context)
static void renderSegInline(DrumEngine& e, const std::vector<Ev>& ev, int N, int B, std::vector<float>& L, std::vector<float>& R) {
  L.assign(N,0); R.assign(N,0); size_t k=0; int pos=0;
  while (pos<N) { while (k<ev.size() && ev[k].t==pos) { e.trigger(ev[k].inst, ev[k].vel); ++k; }
    int next = (k<ev.size()) ? ev[k].t : N; int blk = (pos/B+1)*B; int end = std::min(std::min(next,N),blk);
    e.process(&L[pos],&R[pos],end-pos); pos=end; }
}
static double maxdiff(const std::vector<float>&a,const std::vector<float>&b){double m=0;for(size_t i=0;i<a.size();++i)m=std::max(m,(double)std::fabs(a[i]-b[i]));return m;}
int main(){
  int sr=48000;
  // (a) isolated hits: one instrument at a time, 1.5 s apart (voices decay? count active overlap irrelevant: different instruments never overlap)
  for (int inst=0; inst<kNumInstruments; ++inst){
    std::vector<Ev> ev={{100,inst,0.8f}}; int N=sr*2; std::vector<float> L1,R1,L2,R2;
    DrumEngine a(sr),b(sr); a.seed(7); b.seed(7); renderSeg(a,ev,N,1,L1,R1); renderSeg(b,ev,N,512,L2,R2);
    printf("isolated inst=%2d B1-vs-B512 maxdiff=%.3g\n",inst,std::max(maxdiff(L1,L2),maxdiff(R1,R2)));
  }
  // (b) cross-TU same segmentation, overlapping
  std::vector<Ev> ev; for(int t=100,k=0;t<sr*3;t+=sr*90/1000,++k) ev.push_back({t,k%kNumInstruments,0.4f+0.06f*(k%10)});
  for (int B: {1,32,512}) { std::vector<float> L1,R1,L2,R2; DrumEngine a(sr),b(sr); a.seed(3); b.seed(3);
    renderSeg(a,ev,sr*3,B,L1,R1); renderSegInline(b,ev,sr*3,B,L2,R2);
    printf("crossTU B=%d maxdiff=%.3g\n",B,std::max(maxdiff(L1,L2),maxdiff(R1,R2))); }
  // (c) CPU dense by B
  std::vector<Ev> d; for(int t=100;t<sr*10;t+=sr*50/1000) for(int i=0;i<kNumInstruments;++i) d.push_back({t,i,0.3f+0.05f*(i%10)});
  std::vector<Ev> m; for(int t=100,k=0;t<sr*10;t+=sr*90/1000,++k) m.push_back({t,k%kNumInstruments,0.5f});
  for (int B: {1,8,16,32,64,128}) { std::vector<float> L,R; DrumEngine e(sr); e.seed(1); auto t0=std::chrono::steady_clock::now(); renderSeg(e,d,sr*10,B,L,R);
    double s=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();
    DrumEngine f(sr); f.seed(1); t0=std::chrono::steady_clock::now(); renderSeg(f,m,sr*10,B,L,R);
    double s2=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();
    printf("B=%3d dense=%.2fx moderate=%.2fx realtime\n",B,10/s,10/s2); }
}
