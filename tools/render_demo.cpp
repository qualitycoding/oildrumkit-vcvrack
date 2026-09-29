// render_demo.cpp -- writes renders/demo_48k.wav for the G-101 listening test (plan/DECISIONS.md D-020).
// Drives odk::KitCore frame by frame at 48 kHz, seed 1, default controls; 16-bit stereo PCM.
#include "KitCore.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace
{
constexpr int kSr = 48000;
struct Hit { int frame, inst; float volts; };

void put16 (std::FILE* f, uint16_t v) { std::fputc (v & 255, f); std::fputc (v >> 8, f); }
void put32 (std::FILE* f, uint32_t v) { put16 (f, (uint16_t) (v & 0xffff)); put16 (f, (uint16_t) (v >> 16)); }
}

int main (int argc, char** argv)
{
    const char* path = argc > 1 ? argv[1] : "renders/demo_48k.wav";
    std::vector<Hit> hits;
    int t = kSr / 2;
    // Part 1: every instrument, three hits at 3 V, 6 V, 10 V spaced 0.4 s, then 2 s of silence.
    for (int inst = 0; inst < odk::kNumInstruments; ++inst)
    {
        for (float v : { 3.0f, 6.0f, 10.0f }) { hits.push_back ({ t, inst, v }); t += (int) (0.4f * kSr); }
        t += 2 * kSr;
    }
    // Part 2: 8 bars at 120 BPM (0.5 s per beat, 2 s per bar, eighth = 0.25 s).
    const int eighth = kSr / 4;
    for (int bar = 0; bar < 8; ++bar)
    {
        const int b0 = t + bar * 8 * eighth;
        for (int beat : { 0, 2 }) hits.push_back ({ b0 + beat * 2 * eighth, 0, 10.0f });        // bass on 1 and 3
        for (int beat : { 1, 3 }) hits.push_back ({ b0 + beat * 2 * eighth, 1, 10.0f });        // snare 1 on 2 and 4
        for (int e = 0; e < 8; ++e) hits.push_back ({ b0 + e * eighth, e == 7 ? 12 : 11, 8.0f }); // hats, open on the last 8th
        if (bar >= 4) for (int q = 0; q < 4; ++q) hits.push_back ({ b0 + q * 2 * eighth, 10, 7.0f }); // ride on quarters, bars 5-8
    }
    const int end = t + 8 * 8 * eighth + 3 * kSr;
    std::sort (hits.begin(), hits.end(), [] (const Hit& a, const Hit& b) { return a.frame < b.frame; });

    const int pulse = kSr / 1000;            // 1 ms trigger
    odk::KitCore core; core.seed (1); core.setSampleRate ((float) kSr);
    std::vector<int16_t> pcm;
    pcm.reserve ((size_t) end * 2);
    float volts[odk::kNumInstruments] = {}; int until[odk::kNumInstruments] = {};
    size_t h = 0;
    for (int n = 0; n < end + odk::kBlock; ++n)
    {
        odk::FrameIn in;
        while (h < hits.size() && hits[h].frame == n) { volts[hits[h].inst] = hits[h].volts; until[hits[h].inst] = n + pulse; ++h; }
        for (int i = 0; i < odk::kNumInstruments; ++i) in.trig[i] = n < until[i] ? volts[i] : 0.0f;
        const odk::FrameOut o = core.processFrame (in);
        if (n >= odk::kBlock)                // discard the fixed latency so audio lines up with the schedule
        {
            auto conv = [] (float v) { return (int16_t) std::clamp (std::lround (v / 5.0f * 32767.0f), -32768L, 32767L); };
            pcm.push_back (conv (o.left)); pcm.push_back (conv (o.right));
        }
    }
    std::FILE* f = std::fopen (path, "wb");
    if (! f) { std::perror (path); return 1; }
    const uint32_t bytes = (uint32_t) (pcm.size() * 2);
    std::fwrite ("RIFF", 1, 4, f); put32 (f, 36 + bytes); std::fwrite ("WAVEfmt ", 1, 8, f);
    put32 (f, 16); put16 (f, 1); put16 (f, 2); put32 (f, kSr); put32 (f, kSr * 4); put16 (f, 4); put16 (f, 16);
    std::fwrite ("data", 1, 4, f); put32 (f, bytes);
    for (int16_t s : pcm) put16 (f, (uint16_t) s);
    std::fclose (f);
    std::printf ("wrote %s: %.1f s\n", path, (double) pcm.size() / 2.0 / kSr);
    return 0;
}
