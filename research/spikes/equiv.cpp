// R6 spike: per-sample vs block processing equivalence + CPU cost, Rack compile flags
#include "DrumEngine.h"
#include <cstdio>
#include <chrono>
#include <vector>
using namespace oildrum;
struct Ev { int t; int inst; float vel; };
static std::vector<Ev> schedule(int sr, int seconds, int periodMs, bool all) {
  std::vector<Ev> ev; int per = sr*periodMs/1000; int k=0;
  for (int t = 100; t < sr*seconds; t += per, ++k) {
    if (all) for (int i=0;i<kNumInstruments;++i) ev.push_back({t,i,0.3f+0.05f*(i%10)});
    else ev.push_back({t, k%kNumInstruments, 0.4f+0.06f*(k%10)});
  }
  return ev;
}
static void renderBlock(DrumEngine& e, const std::vector<Ev>& ev, int N, std::vector<float>& L, std::vector<float>& R) {
  L.assign(N,0); R.assign(N,0); size_t k=0; int pos=0;
  while (pos<N) { while (k<ev.size() && ev[k].t==pos) { e.trigger(ev[k].inst, ev[k].vel); ++k; }
    int next = (k<ev.size()) ? ev[k].t : N; int n = std::min(next,N)-pos; e.process(&L[pos],&R[pos],n); pos+=n; }
}
static void renderPerSample(DrumEngine& e, const std::vector<Ev>& ev, int N, std::vector<float>& L, std::vector<float>& R) {
  L.assign(N,0); R.assign(N,0); size_t k=0;
  for (int i=0;i<N;++i) { while (k<ev.size() && ev[k].t==i) { e.trigger(ev[k].inst, ev[k].vel); ++k; }
    float l=0,r=0; e.process(&l,&r,1); L[i]=l; R[i]=r; }
}
int main() {
  for (int sr : {44100, 48000, 96000, 192000}) {
    auto ev = schedule(sr, 3, 90, false); int N = sr*3;
    DrumEngine a(sr), b(sr); a.seed(12345); b.seed(12345);
    std::vector<float> L1,R1,L2,R2; renderBlock(a,ev,N,L1,R1); renderPerSample(b,ev,N,L2,R2);
    int diff=0; double maxd=0, peak=0; for (int i=0;i<N;++i){ double d=std::max(std::fabs(L1[i]-L2[i]),std::fabs(R1[i]-R2[i])); if(d>0)++diff; maxd=std::max(maxd,d); peak=std::max(peak,(double)std::fabs(L1[i])); }
    printf("sr=%d samples_differing=%d maxabsdiff=%.3g peak=%.3f\n", sr, diff, maxd, peak);
  }
  int sr=48000, N=sr*10; auto ev=schedule(sr,10,50,true); DrumEngine e(sr); e.seed(1);
  std::vector<float> L,R; auto t0=std::chrono::steady_clock::now(); renderPerSample(e,ev,N,L,R);
  double s=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();
  printf("dense per-sample: %.2f s for 10 s audio => %.2fx realtime\n", s, 10.0/s);
  DrumEngine f(sr); f.seed(1); t0=std::chrono::steady_clock::now(); renderBlock(f,ev,N,L,R);
  s=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count(); printf("dense block: %.2fx realtime\n", 10.0/s);
  ev=schedule(sr,10,90,false); DrumEngine g(sr); g.seed(1); t0=std::chrono::steady_clock::now(); renderPerSample(g,ev,N,L,R);
  s=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count(); printf("moderate per-sample: %.2fx realtime\n", 10.0/s);
  DrumEngine h(sr); double worst=0; for(int rep=0;rep<50;++rep){ auto a0=std::chrono::steady_clock::now(); for(int i=0;i<kNumInstruments;++i) h.trigger(i,1.0f); double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-a0).count(); worst=std::max(worst,us); float l[64]={0},r[64]={0}; h.process(l,r,64);}
  printf("15 simultaneous triggers worst: %.0f us\n", worst);
}
