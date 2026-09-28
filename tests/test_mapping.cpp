// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Unit tests for the pure mapping functions (plan/DECISIONS.md D-005). Tolerance: 1e-6 relative, i.e.
// ~8 float32 ulps, allowing either v/10 or v*0.1 style evaluation (C-008 unsafe-math reassociation).
#include "harness.hpp"
#include "KitCore.hpp"
#include "Voicing.h"
#include <cmath>
#include <limits>
using namespace odk;
static const float NaN = std::numeric_limits<float>::quiet_NaN(), Inf = std::numeric_limits<float>::infinity();
static bool near (float a, float b, float rel = 1e-6f) { return std::fabs (a - b) <= rel * std::max (1.0f, std::fabs (b)); }

TEST ("T-001", "velocityFromVolts maps 0..10 V to 0..1, clamps, non-finite -> 0 (D-005, C-005)")
{
    CHECK (near (velocityFromVolts (10.0f), 1.0f));
    CHECK (near (velocityFromVolts (5.0f), 0.5f));
    CHECK (near (velocityFromVolts (1.0f), 0.1f));
    CHECK (near (velocityFromVolts (3.7f), 0.37f));
    CHECK (velocityFromVolts (0.0f) == 0.0f);
    CHECK (velocityFromVolts (12.0f) == 1.0f);
    CHECK (velocityFromVolts (-3.0f) == 0.0f);
    CHECK (velocityFromVolts (NaN) == 0.0f);
    CHECK (velocityFromVolts (Inf) == 0.0f);
    CHECK (velocityFromVolts (-Inf) == 0.0f);
}

TEST ("T-002", "dampingFromUnit is 0.25*16^x on [0,1], clamped, non-finite -> 0.25 (D-005)")
{
    CHECK (near (dampingFromUnit (0.0f), 0.25f));
    CHECK (near (dampingFromUnit (0.5f), 1.0f));
    CHECK (near (dampingFromUnit (1.0f), 4.0f));
    CHECK (near (dampingFromUnit (0.25f), 0.5f));
    CHECK (near (dampingFromUnit (-1.0f), 0.25f));
    CHECK (near (dampingFromUnit (2.0f), 4.0f));
    CHECK (near (dampingFromUnit (NaN), 0.25f));
    float prev = 0.0f;
    for (int i = 0; i <= 100; ++i) { const float d = dampingFromUnit ((float) i / 100.0f); CHECK_MSG (d > prev, "not increasing at %d", i); prev = d; }
}

TEST ("T-003", "unitWithCv adds cv/10 to the knob and clamps to [0,1]; non-finite args -> 0 (D-005)")
{
    CHECK (near (unitWithCv (0.5f, 0.0f), 0.5f));
    CHECK (near (unitWithCv (0.5f, 5.0f), 1.0f));
    CHECK (near (unitWithCv (0.5f, -5.0f), 0.0f));
    CHECK (near (unitWithCv (0.2f, 1.0f), 0.3f));
    CHECK (unitWithCv (0.9f, 10.0f) == 1.0f);
    CHECK (unitWithCv (0.1f, -10.0f) == 0.0f);
    CHECK (near (unitWithCv (0.7f, NaN), 0.7f));
    CHECK (near (unitWithCv (0.7f, Inf), 0.7f));
    CHECK (near (unitWithCv (NaN, 2.0f), 0.2f));
}

TEST ("T-004", "roomMixFromUnit is 0.5*x on [0,1], clamped (D-005; engine clamps room to 0..0.5)")
{
    CHECK (roomMixFromUnit (0.0f) == 0.0f);
    CHECK (near (roomMixFromUnit (1.0f), 0.5f));
    CHECK (near (roomMixFromUnit (0.24f), 0.12f));
    CHECK (near (roomMixFromUnit (3.0f), 0.5f));
    CHECK (roomMixFromUnit (-1.0f) == 0.0f);
    CHECK (roomMixFromUnit (NaN) == 0.0f);
}

TEST ("T-005", "pitchHz follows 1 V/oct and semitone tune from each instrument's default (D-005, C-005)")
{
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const float f0 = oildrum::kVoicing[i].defaultHz;
        CHECK_MSG (near (pitchHz (i, 0.0f, 0.0f), f0), "inst %d default", i);
        CHECK_MSG (near (pitchHz (i, 12.0f, 0.0f), 2.0f * f0), "inst %d +12 st", i);
        CHECK_MSG (near (pitchHz (i, 0.0f, 1.0f), 2.0f * f0), "inst %d +1 V", i);
        CHECK_MSG (near (pitchHz (i, 12.0f, -1.0f), f0), "inst %d +12 st -1 V", i);
        CHECK_MSG (near (pitchHz (i, -12.0f, 0.0f), 0.5f * f0), "inst %d -12 st", i);
        CHECK_MSG (near (pitchHz (i, 7.0f, 0.0f), f0 * std::pow (2.0f, 7.0f / 12.0f), 2e-6f), "inst %d +7 st", i);
        CHECK_MSG (near (pitchHz (i, 24.0f, 0.0f), 2.0f * f0), "inst %d tune clamp", i);
        CHECK_MSG (near (pitchHz (i, 0.0f, 20.0f), 1024.0f * f0), "inst %d voct clamp", i);
        CHECK_MSG (near (pitchHz (i, NaN, NaN), f0), "inst %d NaN", i);
    }
    CHECK (pitchHz (-1, 0.0f, 0.0f) == 0.0f);
    CHECK (pitchHz (kNumInstruments, 0.0f, 0.0f) == 0.0f);
}

TEST ("T-014", "monoFold sums to LEFT only when RIGHT is unconnected (D-011)")
{
    FrameOut o; o.left = 1.0f; o.right = -3.0f;
    FrameOut a = monoFold (o, true);
    CHECK (a.left == 1.0f && a.right == -3.0f);
    FrameOut b = monoFold (o, false);
    CHECK (near (b.left, -1.0f) && b.right == -3.0f);
}
TEST_MAIN
