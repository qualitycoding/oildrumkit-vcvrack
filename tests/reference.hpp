// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Independent reference for the D-010 processing contract. It drives oildrum::DrumEngine directly and
// re-implements the Schmitt trigger and the block/segment schedule; it re-uses only the odk:: pure mapping
// functions, which are verified separately by T-001..T-005 (rationale: plan/DECISIONS.md D-010, note R).
#pragma once
#include "KitCore.hpp"
#include "DrumEngine.h"
#include <cmath>
#include <cstdint>
#include <vector>

namespace ref
{
struct Schmitt                                   // Rack dsp::TSchmittTrigger<float> semantics (C-006)
{
    enum S { LOW, HIGH, UNINIT } s = UNINIT;
    bool process (float in)
    {
        if (! std::isfinite (in)) in = 0.0f;     // D-010 rule 2: non-finite input treated as 0 V
        if (s == LOW && in >= odk::kTrigHighV) { s = HIGH; return true; }
        if (s == HIGH && in <= odk::kTrigLowV) s = LOW;
        else if (s == UNINIT && in >= odk::kTrigHighV) s = HIGH;
        else if (s == UNINIT && in <= odk::kTrigLowV) s = LOW;
        return false;
    }
};

struct Event { int offset, inst; float vel, damping, strike, hz; int hammer; };

// Renders the expected KitCore output for the given per-frame inputs and controls.
inline std::vector<odk::FrameOut> render (float sampleRate, uint32_t seed,
                                          const std::vector<odk::FrameIn>& in,
                                          const std::vector<odk::Controls>& ctl)
{
    const int N = (int) in.size(), B = odk::kBlock;
    oildrum::DrumEngine e (sampleRate);
    e.seed (seed);
    Schmitt sch[odk::kNumInstruments];
    std::vector<odk::FrameOut> out ((size_t) N);
    std::vector<float> L (B), R (B), prevL (B, 0.0f), prevR (B, 0.0f);
    std::vector<Event> ev;
    for (int n = 0; n < N; ++n)
    {
        const int j = n % B;
        const float ol = prevL[(size_t) j], orr = prevR[(size_t) j];
        out[(size_t) n].left  = std::isfinite (ol)  ? ol  : 0.0f;
        out[(size_t) n].right = std::isfinite (orr) ? orr : 0.0f;
        const odk::Controls& c = ctl[(size_t) n];
        const odk::FrameIn& f = in[(size_t) n];
        for (int i = 0; i < odk::kNumInstruments; ++i)
            if (sch[i].process (f.trig[i]))
            {
                float v = f.trig[i]; if (! std::isfinite (v)) v = 0.0f;
                const int h = c.hammer < 0 ? 0 : (c.hammer > 2 ? 2 : c.hammer);
                ev.push_back ({ j, i, odk::velocityFromVolts (v),
                                odk::dampingFromUnit (odk::unitWithCv (c.damping, f.dampingCv)),
                                odk::unitWithCv (c.strike, f.strikeCv),
                                odk::pitchHz (i, c.tune, f.voct), h });
            }
        if (j == B - 1)
        {
            e.setRoomMix (odk::roomMixFromUnit (odk::unitWithCv (c.room, f.roomCv)));
            e.setLimiter (c.limiter);
            std::fill (L.begin(), L.end(), 0.0f); std::fill (R.begin(), R.end(), 0.0f);
            int pos = 0; size_t k = 0;
            while (k < ev.size())
            {
                const int off = ev[k].offset;
                if (off > pos) { e.process (&L[(size_t) pos], &R[(size_t) pos], off - pos); pos = off; }
                while (k < ev.size() && ev[k].offset == off)
                {
                    const Event& x = ev[k++];
                    e.setDamping (x.damping); e.setStrikePos (x.strike);
                    e.setHammer ((oildrum::HammerType) x.hammer); e.setPitchHz (x.inst, x.hz);
                    e.trigger (x.inst, x.vel);
                }
            }
            if (pos < B) e.process (&L[(size_t) pos], &R[(size_t) pos], B - pos);
            for (int i = 0; i < B; ++i) { prevL[(size_t) i] = L[(size_t) i] * odk::kOutputVoltsPerUnit; prevR[(size_t) i] = R[(size_t) i] * odk::kOutputVoltsPerUnit; }
            ev.clear();
        }
    }
    return out;
}

// Engine-direct render in fixed blocks of B samples (split at events), like a VST host block loop.
struct Hit { int frame, inst; float vel; };
inline void renderBlocks (float sampleRate, uint32_t seed, const std::vector<Hit>& hits, int N, int B,
                          std::vector<float>& L, std::vector<float>& R)
{
    oildrum::DrumEngine e (sampleRate); e.seed (seed);
    L.assign ((size_t) N, 0.0f); R.assign ((size_t) N, 0.0f);
    size_t k = 0; int pos = 0;
    while (pos < N)
    {
        while (k < hits.size() && hits[k].frame == pos) { e.trigger (hits[k].inst, hits[k].vel); ++k; }
        const int next = k < hits.size() ? hits[k].frame : N;
        const int end = std::min (std::min (next, N), (pos / B + 1) * B);
        e.process (&L[(size_t) pos], &R[(size_t) pos], end - pos); pos = end;
    }
}

struct Lcg { uint32_t s; explicit Lcg (uint32_t x) : s (x) {} uint32_t next() { s = s * 1664525u + 1013904223u; return s >> 8; }
             float uni() { return (float) next() / 16777216.0f; } };
} // namespace ref
