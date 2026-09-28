// SPIKE (research/spikes/kitcore_oracle) -- throwaway oracle implementation of plan/DECISIONS.md D-005/D-010,
// written by the planning agent only to prove that the frozen suite is satisfiable (R6, "test the tests").
// It is NOT the deliverable; the implementer writes src/core/KitCore.cpp in S-002/S-003 and may consult this.
#include "KitCore.hpp"
#include "DrumEngine.h"
#include <cmath>
#include <algorithm>

namespace odk
{
static_assert (kNumInstruments == oildrum::kNumInstruments, "instrument count");
static_assert (kBlock == oildrum::kChunk, "block == engine chunk (D-003)");
static float fin (float x) { return std::isfinite (x) ? x : 0.0f; }

float velocityFromVolts (float v) { return std::clamp (fin (v) / 10.0f, 0.0f, 1.0f); }
float unitWithCv (float k, float cv) { return std::clamp (fin (k) + fin (cv) / 10.0f, 0.0f, 1.0f); }
float dampingFromUnit (float x) { return 0.25f * std::pow (16.0f, std::clamp (fin (x), 0.0f, 1.0f)); }
float roomMixFromUnit (float x) { return 0.5f * std::clamp (fin (x), 0.0f, 1.0f); }
float pitchHz (int inst, float tune, float voct)
{
    if (inst < 0 || inst >= kNumInstruments) return 0.0f;
    const float e = std::clamp (fin (tune), -12.0f, 12.0f) / 12.0f + std::clamp (fin (voct), -10.0f, 10.0f);
    return oildrum::kVoicing[inst].defaultHz * std::exp2 (e);
}
FrameOut monoFold (FrameOut o, bool rc) { if (! rc) o.left = 0.5f * (o.left + o.right); return o; }

struct KitCore::Impl
{
    enum S : uint8_t { LOW, HIGH, UNINIT };
    struct Ev { int off, inst; float vel, damp, strike, hz; int hammer; };
    oildrum::DrumEngine eng { 44100.0 };
    float sr = 44100.0f;
    Controls ctl;
    S sch[kNumInstruments];
    Ev ev[kBlock * kNumInstruments]; int nev = 0;
    float outL[kBlock], outR[kBlock], L[kBlock], R[kBlock];
    int j = 0;
    uint64_t hits[kNumInstruments]; float lastVel[kNumInstruments];
    void clear()
    {
        eng.setSampleRate (sr); eng.allNotesOff();
        std::fill (sch, sch + kNumInstruments, UNINIT); nev = 0; j = 0;
        std::fill (outL, outL + kBlock, 0.0f); std::fill (outR, outR + kBlock, 0.0f);
        std::fill (hits, hits + kNumInstruments, 0); std::fill (lastVel, lastVel + kNumInstruments, 0.0f);
    }
    bool schmitt (int i, float v)
    {
        v = fin (v);
        if (sch[i] == LOW && v >= kTrigHighV) { sch[i] = HIGH; return true; }
        if (sch[i] == HIGH && v <= kTrigLowV) sch[i] = LOW;
        else if (sch[i] == UNINIT && v >= kTrigHighV) sch[i] = HIGH;
        else if (sch[i] == UNINIT && v <= kTrigLowV) sch[i] = LOW;
        return false;
    }
};

KitCore::KitCore() : impl (new Impl) { impl->eng.seed (1); impl->clear(); }
KitCore::~KitCore() { delete impl; }
void KitCore::setSampleRate (float s) { if (! (std::isfinite (s) && s > 0.0f)) return; impl->sr = s; impl->clear(); }
void KitCore::seed (uint32_t s) { impl->eng.seed (s); }
void KitCore::setControls (const Controls& c) { impl->ctl = c; }
void KitCore::reset() { impl->clear(); }
uint64_t KitCore::hitCount (int i) const { return (i >= 0 && i < kNumInstruments) ? impl->hits[i] : 0; }
float KitCore::lastVelocity (int i) const { return (i >= 0 && i < kNumInstruments) ? impl->lastVel[i] : 0.0f; }

FrameOut KitCore::processFrame (const FrameIn& in)
{
    Impl& m = *impl; const Controls& c = m.ctl;
    FrameOut o; o.left = fin (m.outL[m.j]); o.right = fin (m.outR[m.j]);
    for (int i = 0; i < kNumInstruments; ++i)
        if (m.schmitt (i, in.trig[i]))
        {
            const float v = velocityFromVolts (in.trig[i]);
            m.ev[m.nev++] = { m.j, i, v, dampingFromUnit (unitWithCv (c.damping, in.dampingCv)), unitWithCv (c.strike, in.strikeCv),
                              pitchHz (i, c.tune, in.voct), std::clamp (c.hammer, 0, 2) };
            ++m.hits[i]; m.lastVel[i] = v;
        }
    if (m.j == kBlock - 1)
    {
        m.eng.setRoomMix (roomMixFromUnit (unitWithCv (c.room, in.roomCv))); m.eng.setLimiter (c.limiter);
        std::fill (m.L, m.L + kBlock, 0.0f); std::fill (m.R, m.R + kBlock, 0.0f);
        int pos = 0, k = 0;
        while (k < m.nev)
        {
            const int off = m.ev[k].off;
            if (off > pos) { m.eng.process (m.L + pos, m.R + pos, off - pos); pos = off; }
            for (; k < m.nev && m.ev[k].off == off; ++k)
            {
                const Impl::Ev& x = m.ev[k];
                m.eng.setDamping (x.damp); m.eng.setStrikePos (x.strike); m.eng.setHammer ((oildrum::HammerType) x.hammer);
                m.eng.setPitchHz (x.inst, x.hz); m.eng.trigger (x.inst, x.vel);
            }
        }
        if (pos < kBlock) m.eng.process (m.L + pos, m.R + pos, kBlock - pos);
        for (int i = 0; i < kBlock; ++i) { m.outL[i] = m.L[i] * kOutputVoltsPerUnit; m.outR[i] = m.R[i] * kOutputVoltsPerUnit; }
        m.nev = 0; m.j = 0;
    }
    else ++m.j;
    return o;
}
} // namespace odk
