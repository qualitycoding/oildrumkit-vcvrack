// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Integration tests of the D-010 processing contract against the independent reference (tests/reference.hpp).
// Tolerance 1e-5 V: covers float reassociation between inlining contexts under -funsafe-math-optimizations
// (observed 0 in spike research/spikes/seg_main.cpp), while any segmentation, ordering, timing or parameter
// error produces differences >= 1.6e-4 V (smallest chunking-induced difference observed, spike seg.out).
#include "harness.hpp"
#include "gen.hpp"
#include <cmath>
using namespace odk;

static void compare (const std::vector<FrameOut>& a, const std::vector<FrameOut>& b, const char* what)
{
    CHECK (a.size() == b.size());
    double maxd = 0.0, energy = 0.0; size_t at = 0;
    for (size_t n = 0; n < a.size(); ++n)
    {
        const double d = std::max (std::fabs ((double) a[n].left - b[n].left), std::fabs ((double) a[n].right - b[n].right));
        if (! (d <= maxd)) { maxd = d; at = n; }
        energy += (double) b[n].left * b[n].left;
    }
    CHECK_MSG (maxd <= 1e-5, "%s: max |core - reference| = %.3g V at frame %zu", what, maxd, at);
    CHECK_MSG (energy > 1.0, "%s: reference output is (nearly) silent; scenario invalid", what);
}

TEST ("T-010", "KitCore output equals the D-010 reference render at 44.1/48/96/192 kHz (tol 1e-5 V)")
{
    uint32_t seed = 1234;
    for (float sr : { 44100.0f, 48000.0f, 96000.0f, 192000.0f })
    {
        const gen::Scenario s = gen::mixed (sr, 1.2f, seed);
        KitCore k; k.seed (seed); k.setSampleRate (sr);
        const auto got = gen::runCore (k, s);
        const auto exp = ref::render (sr, seed, s.in, s.ctl);
        char what[64]; std::snprintf (what, sizeof what, "sr=%g", (double) sr);
        compare (got, exp, what);
        uint64_t hits = 0; for (int i = 0; i < kNumInstruments; ++i) hits += k.hitCount (i);
        CHECK_MSG (hits == (uint64_t) s.firingPulses, "%s: hits %llu, expected %d", what, (unsigned long long) hits, s.firingPulses);
        seed += 17;
    }
}

TEST ("T-011", "latency is exactly kBlock frames and a hit sounds on its first output frame (D-003, C-012)")
{
    for (int inst = 0; inst < kNumInstruments; ++inst)
    {
        KitCore k; k.seed (1); k.setSampleRate (48000.0f);
        FrameIn f; const int t = 1000;
        for (int n = 0; n < t + kBlock + 4; ++n)
        {
            f.trig[inst] = (n >= t && n < t + 48) ? 10.0f : 0.0f;
            const FrameOut o = k.processFrame (f);
            if (n < t + kBlock) CHECK_MSG (o.left == 0.0f && o.right == 0.0f, "inst %d: output before latency at frame %d", inst, n);
            if (n == t + kBlock) CHECK_MSG (o.left != 0.0f || o.right != 0.0f, "inst %d: silent on onset frame", inst);
        }
    }
}

TEST ("T-023", "default construction: 44.1 kHz, seed 1, default Controls (D-010 rule 1)")
{
    const gen::Scenario s = gen::mixed (44100.0f, 0.6f, 77);
    std::vector<Controls> defaults (s.in.size());
    KitCore k;
    std::vector<FrameOut> got (s.in.size());
    for (size_t n = 0; n < s.in.size(); ++n) got[n] = k.processFrame (s.in[n]);
    compare (got, ref::render (44100.0f, 1, s.in, defaults), "defaults");
}
TEST_MAIN
