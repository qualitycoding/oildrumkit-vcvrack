// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Performance (plan/ASSUMPTIONS.md A-006 as amended by D-003/D-012). Hardware-independent overhead bound:
// KitCore time <= 1.15 x the engine alone on the same schedule and block size (median of 5 interleaved
// runs; per-frame wrapper work is O(15) comparisons, so the bound mainly absorbs timer noise), plus an
// absolute sanity floor on a moderate pattern: >= 10x real time at 48 kHz (spike: engine 36-38x).
#include "harness.hpp"
#include "reference.hpp"
#include <algorithm>
#include <chrono>
using namespace odk;

static double secs (std::chrono::steady_clock::time_point a) { return std::chrono::duration<double> (std::chrono::steady_clock::now() - a).count(); }
static double median (std::vector<double> v) { std::sort (v.begin(), v.end()); return v[v.size() / 2]; }

static double timeCore (const std::vector<ref::Hit>& hits, int N)
{
    KitCore k; k.seed (1); k.setSampleRate (48000.0f);
    FrameIn f; size_t h = 0; std::vector<int> off (kNumInstruments, -1);
    float sink = 0.0f;
    const auto t0 = std::chrono::steady_clock::now();
    for (int n = 0; n < N; ++n)
    {
        while (h < hits.size() && hits[h].frame == n) { f.trig[hits[h].inst] = hits[h].vel * 10.0f; off[(size_t) hits[h].inst] = n + 48; ++h; }
        for (int i = 0; i < kNumInstruments; ++i) if (off[(size_t) i] == n) f.trig[i] = 0.0f;
        const FrameOut o = k.processFrame (f); sink += o.left;
    }
    const double s = secs (t0);
    CHECK (sink == sink);
    return s;
}
static double timeEngine (const std::vector<ref::Hit>& hits, int N)
{
    std::vector<float> L, R;
    const auto t0 = std::chrono::steady_clock::now();
    ref::renderBlocks (48000.0f, 1, hits, N, kBlock, L, R);
    return secs (t0);
}

TEST ("T-040", "dense roll (all 15 every 50 ms, 10 s): KitCore <= 1.15 x engine-only time")
{
    std::vector<ref::Hit> hits; const int N = 480000;
    for (int t = 100; t < N - 100; t += 2400) for (int i = 0; i < kNumInstruments; ++i) hits.push_back ({ t, i, 0.3f + 0.05f * (float) (i % 10) });
    std::vector<double> rc, re;
    for (int k = 0; k < 5; ++k) { rc.push_back (timeCore (hits, N)); re.push_back (timeEngine (hits, N)); }
    const double ratio = median (rc) / median (re);
    std::printf ("  T-040: core %.3f s, engine %.3f s, ratio %.3f\n", median (rc), median (re), ratio);
    CHECK_MSG (ratio <= 1.15, "overhead ratio %.3f", ratio);
}

TEST ("T-041", "moderate pattern (one hit per 90 ms, 10 s at 48 kHz): KitCore >= 10x real time")
{
    std::vector<ref::Hit> hits; const int N = 480000;
    for (int t = 100, k = 0; t < N - 100; t += 4320, ++k) hits.push_back ({ t, k % kNumInstruments, 0.5f });
    std::vector<double> rc; for (int k = 0; k < 3; ++k) rc.push_back (timeCore (hits, N));
    const double x = 10.0 / median (rc);
    std::printf ("  T-041: %.1fx real time\n", x);
    CHECK_MSG (x >= 10.0, "only %.1fx real time", x);
}
TEST_MAIN
