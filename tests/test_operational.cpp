// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Operational behaviour: sample-rate change, reset, silence (D-010 rules 1, 5, 6; C-010, C-011).
#include "harness.hpp"
#include "gen.hpp"
#include <cmath>
#include <limits>
using namespace odk;

static void ring (KitCore& k, int inst, int frames)
{
    FrameIn f; k.processFrame (f);
    for (int n = 0; n < frames; ++n) { f.trig[inst] = n < 48 ? 10.0f : 0.0f; k.processFrame (f); }
}
static void expectSilence (KitCore& k, int frames, const char* what)
{
    FrameIn f;
    for (int n = 0; n < frames; ++n) { const FrameOut o = k.processFrame (f); CHECK_MSG (o.left == 0.0f && o.right == 0.0f, "%s: non-zero at frame %d", what, n); }
}

TEST ("T-020", "setSampleRate implies reset; invalid rates are ignored (D-010 rule 5)")
{
    KitCore k; k.seed (3); k.setSampleRate (48000.0f);
    ring (k, 10, 9600);
    CHECK (k.hitCount (10) == 1);
    k.setSampleRate (96000.0f);
    CHECK (k.hitCount (10) == 0 && k.lastVelocity (10) == 0.0f);
    expectSilence (k, 96000, "after 48k->96k");
    ring (k, 0, 4800);
    CHECK (k.hitCount (0) == 1);

    const gen::Scenario s = gen::mixed (48000.0f, 0.5f, 5);
    KitCore a, b; a.seed (8); b.seed (8); a.setSampleRate (48000.0f); b.setSampleRate (48000.0f);
    a.setSampleRate (std::numeric_limits<float>::quiet_NaN()); a.setSampleRate (0.0f); a.setSampleRate (-48000.0f);
    a.setSampleRate (std::numeric_limits<float>::infinity());
    const auto oa = gen::runCore (a, s), ob = gen::runCore (b, s);
    for (size_t n = 0; n < oa.size(); ++n) CHECK_MSG (oa[n].left == ob[n].left && oa[n].right == ob[n].right, "invalid rate changed state (frame %zu)", n);
}

TEST ("T-021", "reset silences, zeroes counters and re-arms triggers as UNINITIALIZED (D-010 rule 6)")
{
    KitCore k; k.seed (4); k.setSampleRate (48000.0f);
    ring (k, 12, 4800);
    k.reset();
    CHECK (k.hitCount (12) == 0 && k.lastVelocity (12) == 0.0f);
    expectSilence (k, 48000, "after reset");
    FrameIn f; k.processFrame (f);                  // input 2 is LOW here (zeros were fed above)
    f.trig[2] = 10.0f; k.processFrame (f);
    CHECK (k.hitCount (2) == 1);
    k.reset();                                      // reset while the input is held high
    for (int n = 0; n < 100; ++n) k.processFrame (f);
    CHECK (k.hitCount (2) == 0);                    // UNINITIALIZED -> HIGH without a trigger
    f.trig[2] = 0.0f; k.processFrame (f); f.trig[2] = 10.0f; k.processFrame (f);
    CHECK (k.hitCount (2) == 1);
}

TEST ("T-022", "no triggers -> output is exactly 0 V regardless of controls and CV (C-012)")
{
    KitCore k; k.seed (5); k.setSampleRate (48000.0f);
    FrameIn f; Controls c;
    for (int n = 0; n < 480000; ++n)
    {
        if (n % 4800 == 0) { c.room = (float) ((n / 4800) % 5) / 4.0f; c.limiter = (n / 4800) % 2; c.hammer = (n / 4800) % 3; c.tune = (float) ((n / 4800) % 25 - 12); }
        f.roomCv = 5.0f * std::sin (0.01f * (float) n); f.dampingCv = f.roomCv; f.strikeCv = -f.roomCv; f.voct = 0.2f * f.roomCv;
        k.setControls (c);
        const FrameOut o = k.processFrame (f);
        CHECK_MSG (o.left == 0.0f && o.right == 0.0f, "non-zero at frame %d", n);
    }
}
TEST_MAIN
