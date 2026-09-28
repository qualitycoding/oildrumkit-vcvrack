// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Deterministic input-scenario generator shared by the integration tests.
#pragma once
#include "KitCore.hpp"
#include "reference.hpp"
#include <vector>

namespace gen
{
struct Scenario { std::vector<odk::FrameIn> in; std::vector<odk::Controls> ctl; int firingPulses = 0; };

// Mixed scenario: overlapping and simultaneous hits, pulses of several heights and widths (some below
// threshold), hits on block boundaries, CV values that differ on the firing frame from their neighbours,
// and control changes (hammer, limiter, room, tune, damping, strike) every ~0.1 s.
inline Scenario mixed (float sr, float seconds, uint32_t seed)
{
    ref::Lcg r (seed);
    const int N = (int) (sr * seconds), B = odk::kBlock;
    Scenario s; s.in.assign ((size_t) N, odk::FrameIn{}); s.ctl.assign ((size_t) N, odk::Controls{});
    odk::FrameIn base;
    odk::Controls c;
    const float heights[] = { 0.5f, 1.0f, 2.5f, 5.0f, 10.0f, 7.3f };
    const int ms = (int) (sr / 1000.0f);
    std::vector<int> starts;
    for (int t = 64; t < N - 6 * ms; t += (int) ((400 + (int) (r.next() % 3600)) * sr / 48000.0f)) starts.push_back (t);
    for (int k = 3; k * B + 1 < N - 6 * ms; k += 37) { starts.push_back (k * B - 1); starts.push_back (k * B); }
    int ctlEvery = (int) (0.1f * sr), hammer = 0;
    for (int n = 0; n < N; ++n)
    {
        if (n % ctlEvery == 0)
        {
            c.damping = r.uni(); c.strike = r.uni(); c.room = r.uni(); c.tune = -12.0f + 24.0f * r.uni();
            c.hammer = hammer++ % 3; c.limiter = ((n / ctlEvery) % 2) == 0;
        }
        if (n % 97 == 0) { base.dampingCv = -5.0f + 10.0f * r.uni(); base.strikeCv = -5.0f + 10.0f * r.uni(); base.voct = -2.0f + 4.0f * r.uni(); }
        base.roomCv = 3.0f * std::sin (0.001f * (float) n);
        s.in[(size_t) n] = base; s.ctl[(size_t) n] = c;
    }
    for (int t : starts)
    {
        const int count = 1 + (int) (r.next() % 3);
        for (int q = 0; q < count; ++q)
        {
            const int inst = (int) (r.next() % odk::kNumInstruments);
            const float h = heights[r.next() % 6];
            const int widthSel = (int) (r.next() % 3), w = widthSel == 0 ? 1 : (widthSel == 1 ? ms : 5 * ms);
            bool clear = true;                       // keep >= 2 frames of 0 V before each pulse so it can fire
            for (int k = std::max (0, t - 2); k < std::min (N, t + w); ++k) if (s.in[(size_t) k].trig[inst] != 0.0f) clear = false;
            if (! clear) continue;
            for (int k = t; k < std::min (N, t + w); ++k) s.in[(size_t) k].trig[inst] = h;
            if (h >= odk::kTrigHighV) ++s.firingPulses;
        }
        s.in[(size_t) t].dampingCv = -5.0f + 10.0f * r.uni();   // firing-frame-only CV values
        s.in[(size_t) t].strikeCv  = -5.0f + 10.0f * r.uni();
        s.in[(size_t) t].voct      = -2.0f + 4.0f * r.uni();
    }
    return s;
}

inline std::vector<odk::FrameOut> runCore (odk::KitCore& k, const Scenario& s)
{
    std::vector<odk::FrameOut> out (s.in.size());
    for (size_t n = 0; n < s.in.size(); ++n) { k.setControls (s.ctl[n]); out[n] = k.processFrame (s.in[n]); }
    return out;
}
} // namespace gen
