// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Trigger-input semantics through KitCore (D-004, D-010 rule 2; C-006 Rack SchmittTrigger semantics).
#include "harness.hpp"
#include "KitCore.hpp"
#include <cmath>
#include <limits>
#include <vector>
using namespace odk;

static uint64_t hitsFor (const std::vector<float>& volts, int inst = 3)
{
    KitCore k; k.seed (9); k.setSampleRate (48000.0f);
    FrameIn f;
    for (float v : volts) { f.trig[inst] = v; k.processFrame (f); }
    for (int i = 0; i < kNumInstruments; ++i) if (i != inst) CHECK_MSG (k.hitCount (i) == 0, "cross-talk to inst %d", i);
    return k.hitCount (inst);
}

TEST ("T-006", "Schmitt trigger: 0.1 V / 1.0 V thresholds, rising edge only, UNINITIALIZED start (C-006)")
{
    const float NaN = std::numeric_limits<float>::quiet_NaN();
    CHECK (hitsFor ({ 10.0f }) == 0);                              // high from power-up: no trigger
    CHECK (hitsFor ({ 10.0f, 0.0f, 10.0f }) == 1);
    CHECK (hitsFor ({ 0.0f, 10.0f }) == 1);
    std::vector<float> held (1000, 10.0f); held.insert (held.begin(), 0.0f);
    CHECK (hitsFor (held) == 1);                                   // held gate fires once
    CHECK (hitsFor ({ 0.0f, 10.0f, 0.5f, 10.0f }) == 1);           // 0.5 V does not re-arm
    CHECK (hitsFor ({ 0.0f, 10.0f, 0.1f, 10.0f }) == 2);           // <= 0.1 V re-arms
    CHECK (hitsFor ({ 0.0f, 0.999f }) == 0);
    CHECK (hitsFor ({ 0.0f, 1.0f }) == 1);
    CHECK (hitsFor ({ 0.0f, 10.0f, NaN, 10.0f }) == 2);            // non-finite treated as 0 V
    CHECK (hitsFor ({ 0.0f, std::numeric_limits<float>::infinity() }) == 0);
    for (int i = 0; i < kNumInstruments; ++i) CHECK_MSG (hitsFor ({ 0.0f, 10.0f, 0.0f, 5.0f }, i) == 2, "inst %d", i);
}

TEST ("T-007", "velocity is taken from the trigger voltage on the firing frame (D-005, D-010)")
{
    KitCore k; k.seed (9); k.setSampleRate (48000.0f);
    FrameIn f; k.processFrame (f);
    f.trig[0] = 3.7f; k.processFrame (f);
    f.trig[0] = 9.0f; k.processFrame (f);                        // still high: must not re-fire or change velocity
    CHECK (k.hitCount (0) == 1);
    CHECK (std::fabs (k.lastVelocity (0) - 0.37f) < 1e-6f);
    f.trig[0] = 0.0f; k.processFrame (f); f.trig[0] = 12.0f; k.processFrame (f);
    CHECK (k.hitCount (0) == 2 && k.lastVelocity (0) == 1.0f);
    f.trig[5] = 1.0f; k.processFrame (f);
    CHECK (std::fabs (k.lastVelocity (5) - 0.1f) < 1e-6f);
    CHECK (k.lastVelocity (7) == 0.0f && k.hitCount (-1) == 0 && k.lastVelocity (99) == 0.0f);
}
TEST_MAIN
