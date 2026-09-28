// KitCore.hpp -- Rack-independent core of the Oil Drum Kit VCV Rack module.
//
// Normative behaviour: plan/DECISIONS.md, D-005 (mappings), D-010 (processing contract).
// This header must NOT include rack.hpp or the engine headers (D-009: pimpl isolation).
// It is a public interface: changing it requires halting and writing BLOCKED.md (plan/DECISIONS.md, default rule).
#pragma once
#include <cstdint>

namespace odk
{

constexpr int   kNumInstruments     = 15;    // == oildrum::kNumInstruments (static_assert in KitCore.cpp)
constexpr int   kBlock              = 128;   // D-003: render block == oildrum::kChunk; also the latency in frames
constexpr float kOutputVoltsPerUnit = 5.0f;  // D-006: engine full scale 1.0 -> 5 V (Rack audio standard)
constexpr float kTrigLowV           = 0.1f;  // D-004: Schmitt low threshold (Rack Voltage Standards)
constexpr float kTrigHighV          = 1.0f;  // D-004: Schmitt high threshold

enum Hammer : int { HammerMetal = 0, HammerWood = 1, HammerRubber = 2 };

struct Controls                      // panel positions (not voltages)
{
    float damping = 0.5f;            // 0..1   -> dampingFromUnit()
    float strike  = 0.5f;            // 0..1   -> engine strike position
    float room    = 0.24f;           // 0..1   -> roomMixFromUnit()
    float tune    = 0.0f;            // semitones, -12..+12
    int   hammer  = HammerWood;      // clamped to 0..2
    bool  limiter = true;
};

struct FrameIn                       // one frame of input voltages (0 V when a port is unconnected)
{
    float trig[kNumInstruments] = {};
    float dampingCv = 0.0f, strikeCv = 0.0f, roomCv = 0.0f, voct = 0.0f;
};

struct FrameOut { float left = 0.0f, right = 0.0f; };   // volts

// ---- Pure mapping functions (D-005). Any non-finite argument is treated as 0.
float    velocityFromVolts (float volts);                       // clamp(volts / 10, 0, 1)
float    unitWithCv        (float knob, float cvVolts);         // clamp(knob + cvVolts / 10, 0, 1)
float    dampingFromUnit   (float x);                           // 0.25 * 16^clamp(x, 0, 1)
float    roomMixFromUnit   (float x);                           // 0.5 * clamp(x, 0, 1)
float    pitchHz           (int inst, float tuneSemis, float voctVolts);
                                                                // defaultHz(inst) * 2^(clamp(tune,-12,12)/12 + clamp(voct,-10,10)); 0 if inst out of range
FrameOut monoFold          (FrameOut in, bool rightConnected);  // !rightConnected: left = 0.5*(L+R), right unchanged

// ---- Processing core (D-010)
class KitCore
{
public:
    KitCore();                                  // sample rate 44100, seed 1, default Controls, state as after reset()
    ~KitCore();
    KitCore (const KitCore&) = delete;
    KitCore& operator= (const KitCore&) = delete;

    void     setSampleRate (float sampleRate);  // NOT real-time safe (allocates). Ignored unless finite and > 0. Implies reset().
    void     seed (uint32_t s);                 // seeds the engine PRNG (0 is mapped to 1 by the engine)
    void     setControls (const Controls& c);   // real-time safe
    FrameOut processFrame (const FrameIn& in);  // real-time safe; output is delayed by exactly kBlock frames
    void     reset();                           // NOT real-time safe. Silence, clear buffers, zero counters, re-arm triggers.

    uint64_t hitCount (int inst) const;         // number of triggers fired on inst since reset (0 if inst out of range)
    float    lastVelocity (int inst) const;     // velocity of the most recent trigger on inst (0 if none / out of range)

private:
    struct Impl;
    Impl* impl = nullptr;
};

} // namespace odk
