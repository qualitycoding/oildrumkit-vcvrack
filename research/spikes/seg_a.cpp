#include "seg.hpp"
void renderSeg(oildrum::DrumEngine& e, const std::vector<Ev>& ev, int N, int B, std::vector<float>& L, std::vector<float>& R) {
  L.assign(N,0); R.assign(N,0); size_t k=0; int pos=0;
  while (pos<N) { while (k<ev.size() && ev[k].t==pos) { e.trigger(ev[k].inst, ev[k].vel); ++k; }
    int next = (k<ev.size()) ? ev[k].t : N; int blk = (pos/B+1)*B; int end = std::min(std::min(next,N),blk);
    e.process(&L[pos],&R[pos],end-pos); pos=end; }
}
