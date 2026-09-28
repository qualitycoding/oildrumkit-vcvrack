// KitCore.cpp -- STUB committed by the planning agent (Phase 2B.1).
// Every entry point raises "not implemented" so the frozen suite fails cleanly (red phase).
// Replace with the implementation specified in plan/DECISIONS.md D-005 / D-010 (plan step S-002, S-003).
#include "KitCore.hpp"
#include <stdexcept>
#include <string>

namespace odk
{
[[noreturn]] static void notImplemented (const char* what) { throw std::logic_error (std::string ("not implemented: ") + what); }

float    velocityFromVolts (float)             { notImplemented ("velocityFromVolts"); }
float    unitWithCv (float, float)             { notImplemented ("unitWithCv"); }
float    dampingFromUnit (float)               { notImplemented ("dampingFromUnit"); }
float    roomMixFromUnit (float)               { notImplemented ("roomMixFromUnit"); }
float    pitchHz (int, float, float)           { notImplemented ("pitchHz"); }
FrameOut monoFold (FrameOut, bool)             { notImplemented ("monoFold"); }

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
