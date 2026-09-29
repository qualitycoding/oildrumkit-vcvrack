// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Security / input-validation tests per the threat model (plan/ASSUMPTIONS.md A-012): the only untrusted
// inputs are patch voltages and parameter values. Outputs must stay finite (Rack Voltage Standards,
// "NaNs and Infinity", C-005) and, with the limiter on, within 5 V x 0.944 (engine soft ceiling, C-014),
// at Rack engine rates from the lowest to the highest offered (C-025: 11.025 kHz .. 768 kHz).
#include "harness.hpp"
#include "reference.hpp"
#include <cmath>
#include <limits>
using namespace odk;

static float nasty (ref::Lcg& r)
{
    static const float v[] = { std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
                               -std::numeric_limits<float>::infinity(), 1e30f, -1e30f, 1e-40f, 0.0f, 10.0f, 0.0f, 10.0f, 1e6f, -1e6f };
    return v[r.next() % 12];
}

static void run (float sr, bool limiter, uint32_t seed)
{
    ref::Lcg r (seed);
    KitCore k; k.seed (seed); k.setSampleRate (sr);
    const int N = (int) (0.5f * sr);
    FrameIn f; Controls c;
    const float bound = kOutputVoltsPerUnit * 0.944f * (1.0f + 1e-5f);
    for (int n = 0; n < N; ++n)
    {
        if (n % 64 == 0)
        {
            c.damping = nasty (r); c.strike = nasty (r); c.room = nasty (r); c.tune = nasty (r);
            c.hammer = (int) (r.next() % 11) - 4; c.limiter = limiter;
            k.setControls (c);
        }
        for (int i = 0; i < kNumInstruments; ++i) f.trig[i] = (r.next() % 7 == 0) ? nasty (r) : ((n / 3) % 2 ? 10.0f : 0.0f);
        f.dampingCv = nasty (r); f.strikeCv = nasty (r); f.roomCv = nasty (r); f.voct = nasty (r);
        const FrameOut o = k.processFrame (f);
        CHECK_MSG (std::isfinite (o.left) && std::isfinite (o.right), "sr=%g non-finite output at frame %d", (double) sr, n);
        if (limiter) CHECK_MSG (std::fabs (o.left) <= bound && std::fabs (o.right) <= bound, "sr=%g |out| %.4f > %.4f at frame %d", (double) sr, (double) std::max (std::fabs (o.left), std::fabs (o.right)), (double) bound, n);
    }
}

TEST ("T-030", "hostile inputs (NaN, inf, 1e30 V, denormals, out-of-range controls, trigger storm) are safe")
{
    for (float sr : { 11025.0f, 22050.0f, 48000.0f, 384000.0f, 768000.0f }) { run (sr, true, 21); run (sr, false, 22); }
    KitCore k; k.seed (2); k.setSampleRate (48000.0f);          // storm: every input toggles every frame
    FrameIn f;
    for (int n = 0; n < 48000; ++n)
    {
        for (int i = 0; i < kNumInstruments; ++i) f.trig[i] = (n % 2) ? 10.0f : 0.0f;
        const FrameOut o = k.processFrame (f);
        CHECK (std::isfinite (o.left) && std::isfinite (o.right));
    }
    for (int i = 0; i < kNumInstruments; ++i) CHECK_MSG (k.hitCount (i) == 24000, "inst %d hits %llu", i, (unsigned long long) k.hitCount (i));
}
TEST_MAIN
