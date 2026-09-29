// KitCore.cpp -- Rack-independent core of the Oil Drum Kit module (plan/DECISIONS.md D-005, D-010).
#include "KitCore.hpp"
#include "DrumEngine.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <string>

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


[[noreturn]] static void notImplemented (const char* what) { throw std::logic_error (std::string ("not implemented: ") + what); }
struct KitCore::Impl {};
KitCore::KitCore()                             { notImplemented ("KitCore::KitCore"); }
KitCore::~KitCore()                            { delete impl; }
void     KitCore::setSampleRate (float)        { notImplemented ("KitCore::setSampleRate"); }
void     KitCore::seed (uint32_t)              { notImplemented ("KitCore::seed"); }
void     KitCore::setControls (const Controls&){ notImplemented ("KitCore::setControls"); }
FrameOut KitCore::processFrame (const FrameIn&){ notImplemented ("KitCore::processFrame"); }
void     KitCore::reset()                      { notImplemented ("KitCore::reset"); }
uint64_t KitCore::hitCount (int) const         { notImplemented ("KitCore::hitCount"); }
float    KitCore::lastVelocity (int) const     { notImplemented ("KitCore::lastVelocity"); }
} // namespace odk
