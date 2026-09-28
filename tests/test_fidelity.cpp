// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// T-012: single hits through KitCore must match the engine rendered VST-style in 512-sample host blocks.
// Metrics (per instrument, averaged over 3 seeds x 2 velocities): tail energy after 50 ms, and the time
// until the left channel last exceeds -60 dB re its peak. Bounds: |energy| <= 0.05 dB (10x below the
// ~0.5 dB level JND; 6x above the 0.008 dB worst case observed for kBlock=128 in spike prune.out) and
// |duration| <= 1 % (3.5x above the 0.28 % observed). Smaller blocks (e.g. 32) fail this by design:
// the engine prunes modes once per process() call, which truncates tails (spike prune.out, C-013).
#include "harness.hpp"
#include "reference.hpp"
#include <cmath>
using namespace odk;

struct Metrics { double energy = 0.0, dur = 0.0; };
static void measure (const std::vector<float>& L, const std::vector<float>& R, int start, int sr, Metrics& m)
{
    double pk = 0.0; for (float x : L) pk = std::max (pk, (double) std::fabs (x));
    int last = 0; for (size_t i = 0; i < L.size(); ++i) if (std::fabs (L[i]) > pk * 1e-3) last = (int) i;
    m.dur += last;
    for (size_t i = (size_t) (start + sr / 20); i < L.size(); ++i) m.energy += (double) L[i] * L[i] + (double) R[i] * R[i];
}

TEST ("T-012", "single-hit fidelity vs 512-sample host blocks: tail energy +-0.05 dB, -60 dB duration +-1 %")
{
    for (int sr : { 44100, 48000, 96000, 192000 })
        for (int inst = 0; inst < kNumInstruments; ++inst)
        {
            Metrics core, vst;
            const int N = sr * 3, t = 100;
            for (uint32_t seed = 100; seed < 103; ++seed)
                for (float vel : { 0.3f, 1.0f })
                {
                    std::vector<float> L, R;
                    ref::renderBlocks ((float) sr, seed, { { t, inst, vel } }, N, 512, L, R);
                    measure (L, R, t, sr, vst);
                    KitCore k; k.seed (seed); k.setSampleRate ((float) sr);
                    std::vector<float> cl ((size_t) N), cr ((size_t) N);
                    FrameIn f;
                    for (int n = 0; n < N + kBlock; ++n)
                    {
                        f.trig[inst] = (n >= t && n < t + sr / 1000) ? vel * 10.0f : 0.0f;
                        const FrameOut o = k.processFrame (f);
                        if (n >= kBlock) { cl[(size_t) (n - kBlock)] = o.left / kOutputVoltsPerUnit; cr[(size_t) (n - kBlock)] = o.right / kOutputVoltsPerUnit; }
                    }
                    measure (cl, cr, t, sr, core);
                }
            const double db = 10.0 * std::log10 (core.energy / vst.energy), pct = 100.0 * (core.dur / vst.dur - 1.0);
            CHECK_MSG (std::fabs (db) <= 0.05, "sr=%d inst=%d tail energy %+.3f dB", sr, inst, db);
            CHECK_MSG (std::fabs (pct) <= 1.0, "sr=%d inst=%d -60 dB duration %+.2f %%", sr, inst, pct);
        }
}
TEST_MAIN
