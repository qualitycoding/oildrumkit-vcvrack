// KitCore.cpp -- Rack-independent core of the Oil Drum Kit module (plan/DECISIONS.md D-003..D-011).
#include "KitCore.hpp"
#include "DrumEngine.h"
#include <algorithm>
#include <cmath>

namespace odk
{
static_assert (kNumInstruments == oildrum::kNumInstruments, "instrument count must match the engine");
static_assert (kBlock == oildrum::kChunk, "render block must equal the engine chunk (D-003)");

static float fin (float x) { return std::isfinite (x) ? x : 0.0f; }

// ---- Pure mapping functions (D-005) -----------------------------------------------------------
float velocityFromVolts (float v) { return std::clamp (fin (v) / 10.0f, 0.0f, 1.0f); }
float unitWithCv (float knob, float cv) { return std::clamp (fin (knob) + fin (cv) / 10.0f, 0.0f, 1.0f); }
float dampingFromUnit (float x) { return 0.25f * std::pow (16.0f, std::clamp (fin (x), 0.0f, 1.0f)); }
float roomMixFromUnit (float x) { return 0.5f * std::clamp (fin (x), 0.0f, 1.0f); }

float pitchHz (int inst, float tuneSemis, float voctVolts)
{
    if (inst < 0 || inst >= kNumInstruments) return 0.0f;
    const float octaves = std::clamp (fin (tuneSemis), -12.0f, 12.0f) / 12.0f + std::clamp (fin (voctVolts), -10.0f, 10.0f);
    return oildrum::kVoicing[inst].defaultHz * std::exp2 (octaves);
}

FrameOut monoFold (FrameOut in, bool rightConnected)
{
    if (! rightConnected) in.left = 0.5f * (in.left + in.right);
    return in;
}

// ---- Processing core (D-010) ------------------------------------------------------------------
struct KitCore::Impl
{
    enum SchmittState : uint8_t { Low, High, Uninitialized };

    struct Event { int offset, inst; float vel, damping, strike, hz; int hammer; };

    oildrum::DrumEngine engine { 44100.0 };
    float sampleRate = 44100.0f;
    Controls controls;
    SchmittState schmitt[kNumInstruments];
    Event events[kBlock * kNumInstruments];
    int numEvents = 0;
    float outL[kBlock], outR[kBlock];        // last rendered block, volts
    float bufL[kBlock], bufR[kBlock];        // engine scratch
    int frame = 0;                           // position within the block (n mod kBlock)
    uint64_t hits[kNumInstruments];
    float lastVel[kNumInstruments];

    void clear()
    {
        engine.setSampleRate ((double) sampleRate);
        engine.allNotesOff();
        std::fill (schmitt, schmitt + kNumInstruments, Uninitialized);
        numEvents = 0; frame = 0;
        std::fill (outL, outL + kBlock, 0.0f);  std::fill (outR, outR + kBlock, 0.0f);
        std::fill (hits, hits + kNumInstruments, (uint64_t) 0);
        std::fill (lastVel, lastVel + kNumInstruments, 0.0f);
    }

    bool fires (int i, float volts)                 // Rack dsp::SchmittTrigger<float> semantics (D-004)
    {
        volts = fin (volts);
        SchmittState& s = schmitt[i];
        if (s == Low && volts >= kTrigHighV) { s = High; return true; }
        if (s == High && volts <= kTrigLowV) s = Low;
        else if (s == Uninitialized && volts >= kTrigHighV) s = High;
        else if (s == Uninitialized && volts <= kTrigLowV) s = Low;
        return false;
    }

    void renderBlock (const FrameIn& in)
    {
        const Controls& c = controls;
        engine.setRoomMix (roomMixFromUnit (unitWithCv (c.room, in.roomCv)));
        engine.setLimiter (c.limiter);
        std::fill (bufL, bufL + kBlock, 0.0f);
        std::fill (bufR, bufR + kBlock, 0.0f);
        int pos = 0, k = 0;
        while (k < numEvents)
        {
            const int offset = events[k].offset;
            if (offset > pos) { engine.process (bufL + pos, bufR + pos, offset - pos); pos = offset; }
            for (; k < numEvents && events[k].offset == offset; ++k)
            {
                const Event& e = events[k];
                engine.setDamping (e.damping);
                engine.setStrikePos (e.strike);
                engine.setHammer ((oildrum::HammerType) e.hammer);
                engine.setPitchHz (e.inst, e.hz);
                engine.trigger (e.inst, e.vel);
            }
        }
        if (pos < kBlock) engine.process (bufL + pos, bufR + pos, kBlock - pos);
        for (int i = 0; i < kBlock; ++i)
        {
            outL[i] = bufL[i] * kOutputVoltsPerUnit;
            outR[i] = bufR[i] * kOutputVoltsPerUnit;
        }
        numEvents = 0;
    }
};

KitCore::KitCore() : impl (new Impl) { impl->engine.seed (1); impl->clear(); }
KitCore::~KitCore() { delete impl; }

void KitCore::setSampleRate (float sr)
{
    if (! (std::isfinite (sr) && sr > 0.0f)) return;
    impl->sampleRate = sr;
    impl->clear();
}

void KitCore::seed (uint32_t s) { impl->engine.seed (s); }
void KitCore::setControls (const Controls& c) { impl->controls = c; }
void KitCore::reset() { impl->clear(); }
uint64_t KitCore::hitCount (int inst) const { return (inst >= 0 && inst < kNumInstruments) ? impl->hits[inst] : 0; }
float KitCore::lastVelocity (int inst) const { return (inst >= 0 && inst < kNumInstruments) ? impl->lastVel[inst] : 0.0f; }

FrameOut KitCore::processFrame (const FrameIn& in)
{
    Impl& m = *impl;
    const Controls& c = m.controls;

    FrameOut out;
    out.left = fin (m.outL[m.frame]);
    out.right = fin (m.outR[m.frame]);

    for (int i = 0; i < kNumInstruments; ++i)
        if (m.fires (i, in.trig[i]))
        {
            const float vel = velocityFromVolts (in.trig[i]);
            m.events[m.numEvents++] = { m.frame, i, vel,
                                        dampingFromUnit (unitWithCv (c.damping, in.dampingCv)),
                                        unitWithCv (c.strike, in.strikeCv),
                                        pitchHz (i, c.tune, in.voct),
                                        std::clamp (c.hammer, 0, 2) };
            ++m.hits[i];
            m.lastVel[i] = vel;
        }

    if (m.frame == kBlock - 1) { m.renderBlock (in); m.frame = 0; }
    else ++m.frame;
    return out;
}

} // namespace odk
