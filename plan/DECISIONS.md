# Decisions, interfaces and decision rules

Claim IDs (`C-###`) refer to `research/claims.json`. Test IDs (`T-###`) refer to frozen files under `tests/`.

## Design decisions (D-###)

**D-001 Repository and engine.** Code lives in `qualitycoding/oildrumkit-vcvrack`. The engine is the git submodule
`engine/` pinned to `35fbfead61a11a8c7f5609a95831a29f2cb2bf78` (`qualitycoding/oildrumkit`, header-only, no JUCE,
C-021). Nothing under `engine/` is ever modified (T-033). (A-004, A-009)

**D-002 Module shape.** One module, slug `OilDrumKit`, 20 HP (101.6 mm), per A-002/A-003. Panel content is fixed by
D-013. Instrument order is the engine's `oildrum::Instrument` enum: 0 Bass, 1 Snare1, 2 Snare2, 3 TomLow1, 4 TomLow2,
5 TomMid1, 6 TomMid2, 7 TomHigh1, 8 TomHigh2, 9 Splash, 10 Ride, 11 HihatClosed, 12 HihatOpen, 13 Cowbell, 14 Rimshot.

**D-003 Block rendering, `kBlock = 128`, latency 128 frames.** KitCore renders the engine in blocks of 128 frames
(equal to `oildrum::kChunk`; a `static_assert` enforces it). Output is delayed by exactly 128 frames
(2.90 ms at 44.1 kHz, 2.67 ms at 48 kHz, 1.33 ms at 96 kHz, 0.67 ms at 192 kHz). Rationale:
(a) the engine prunes modes once per `process()` call, so blocks shorter than 128 truncate decays (32-frame blocks shorten the −60 dB
duration by up to 16 % and lose up to 0.24 dB of tail energy at 96 kHz; the largest energy loss is on Rimshot), while 128 matches a 512-frame host within 0.008 dB / 0.28 %
(C-013); (b) per-sample calls cost ~2× (1.47× vs 3.0× real time dense, C-015). This supersedes A-005. Revisiting
the latency is a question at G-101.

**D-004 Trigger detection.** Rack Schmitt-trigger semantics exactly (C-006): states LOW/HIGH/UNINITIALIZED, start
UNINITIALIZED, fire on LOW→HIGH when input ≥ 1.0 V, re-arm when input ≤ 0.1 V, UNINITIALIZED→HIGH (≥ 1.0 V) or
→LOW (≤ 0.1 V) without firing. Thresholds from the current Voltage Standards page (C-005; an older revision used
2 V; the current page governs). Non-finite input is treated as 0 V.

**D-005 Mappings** (all non-finite arguments are treated as 0):
- `velocityFromVolts(v) = clamp(v/10, 0, 1)` — velocity is sampled on the firing frame.
- `unitWithCv(knob, cv) = clamp(knob + cv/10, 0, 1)` — ±5 V bipolar CV sweeps half the range each way (C-005).
- `dampingFromUnit(x) = 0.25 · 16^clamp(x,0,1)` — exponential over the engine's full 0.25..4 range, 1.0 at centre.
- `roomMixFromUnit(x) = 0.5 · clamp(x,0,1)` — engine room range 0..0.5; knob default 0.24 = engine default 0.12.
- Strike position = `unitWithCv(strike, strikeCv)` passed to `setStrikePos` (engine neutral 0.5).
- `pitchHz(i, tune, voct) = kVoicing[i].defaultHz · 2^(clamp(tune,−12,12)/12 + clamp(voct,−10,10))`, 0 for an
  out-of-range index; the engine then clamps to each recipe's min/max (1 V/oct, C-005).
- Hammer = `clamp(hammer, 0, 2)` → `oildrum::HammerType`.
Evaluation order within these formulas is free (tests use 1e-6 relative tolerance, T-001..T-005).

**D-006 Output level.** Volts = engine units × 5 (engine limiter ceiling 0.944 → ±4.72 V, C-014). A non-finite
output sample is replaced by 0 V (C-005). No additional clipping.

**D-007 Monophonic module.** Trigger and CV ports are read with `Port::getVoltage()` (channel 0), per the Voltage
Standards rule for monophonic modules with CV inputs (C-005). The "Polyphonic" tag is not used.

**D-008 C++ standard.** The engine needs C++17 (`std::clamp`, C-008); Rack's `compile.mk` adds `-std=c++11` first
and appends `EXTRA_CXXFLAGS` afterwards, so the plugin Makefile sets `EXTRA_CXXFLAGS += -std=c++17` (verified,
C-008). Do not put `-std` in `FLAGS` (it is also used for C files).

**D-009 Isolation.** `src/core/KitCore.hpp` includes neither `rack.hpp` nor engine headers. Only
`src/core/KitCore.cpp` includes `DrumEngine.h`; only `src/plugin.*` and `src/OilDrumKit.cpp` include `rack.hpp`.
This keeps the core testable without libRack and avoids name/macro collisions.

**D-010 KitCore processing contract (normative; mirrored by `tests/reference.hpp`).**
1. *Construction:* engine at 44 100 Hz, engine seed 1, default `Controls`, state as after `reset()`.
2. *Inputs:* any non-finite `FrameIn` voltage is treated as 0 V; all `Controls` fields pass through the D-005
   functions (which sanitise them).
3. *`processFrame(in)`*, with frame index `n` counted from the last reset and `j = n mod 128`:
   a. The return value is sample `j` of the previously rendered block (zeros before the first render), with
      non-finite values replaced by 0.
   b. For instruments `i = 0..14` in ascending order: run the Schmitt trigger on `in.trig[i]`. If it fires, append
      an event `{offset j, inst i, vel = velocityFromVolts(in.trig[i]), damping = dampingFromUnit(unitWithCv(c.damping,
      in.dampingCv)), strike = unitWithCv(c.strike, in.strikeCv), hz = pitchHz(i, c.tune, in.voct),
      hammer = clamp(c.hammer,0,2)}`, where `c` is the most recent `setControls` value; increment `hitCount(i)`;
      set `lastVelocity(i)`. The event store is a fixed array of `128 × 15` entries (no allocation).
   c. If `j == 127`, render the block: `setRoomMix(roomMixFromUnit(unitWithCv(c.room, in.roomCv)))`,
      `setLimiter(c.limiter)`; zero two 128-sample buffers; walk the events in stored order: for each distinct
      offset `k` (ascending), first `process()` the pending range `[pos, k)` if non-empty and set `pos = k`, then for
      every event at `k` call `setDamping`, `setStrikePos`, `setHammer`, `setPitchHz(inst, hz)`, `trigger(inst, vel)`;
      finally `process()` `[pos, 128)`. Multiply by 5 and store as the next block's output; clear the events.
4. *Latency:* exactly 128 frames; a hit fired at frame `f` is audible at frame `f + 128` (C-012, T-011).
5. *`setSampleRate(sr)`:* ignored unless `sr` is finite and > 0; otherwise store `sr` and `reset()`. Not real-time
   safe (allocates, C-010); Rack calls it under the engine's exclusive lock (C-007).
6. *`reset()`:* `engine.setSampleRate(sr)`, `engine.allNotesOff()`, Schmitt states to UNINITIALIZED, event store and
   output buffer cleared, frame index 0, hit counts and last velocities 0. The engine seed is *not* changed. After
   reset, output is exactly 0 until a trigger fires (C-011).
7. *`seed(s)`:* forwards to `engine.seed(s)`; does not reset.

Note R (why the reference re-uses the mapping functions): a 1-ulp difference in a trigger frequency drifts a
resonator's phase enough to exceed the 1e-5 V comparison tolerance within seconds, so T-010 checks the orchestration
using the core's own mapping functions, which T-001..T-005 check independently.

**D-011 Rack wrapper contract** (`src/OilDrumKit.cpp`, `src/plugin.hpp`, `src/plugin.cpp`):
- Enums: `ParamId {DAMP_PARAM, STRIKE_PARAM, ROOM_PARAM, TUNE_PARAM, HAMMER_PARAM, LIMITER_PARAM, PARAMS_LEN}`;
  `InputId {ENUMS(TRIG_INPUT, 15), DAMP_CV_INPUT, STRIKE_CV_INPUT, ROOM_CV_INPUT, VOCT_INPUT, INPUTS_LEN}`;
  `OutputId {LEFT_OUTPUT, RIGHT_OUTPUT, OUTPUTS_LEN}`; no lights.
- `config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, 0)`; `configParam(DAMP_PARAM, 0, 1, 0.5, "Damping")`;
  `configParam(STRIKE_PARAM, 0, 1, 0.5, "Strike position")`; `configParam(ROOM_PARAM, 0, 1, 0.24, "Room", "%", 0, 100)`;
  `configParam(TUNE_PARAM, -12, 12, 0, "Tune", " semitones")`;
  `configSwitch(HAMMER_PARAM, 0, 2, 1, "Hammer", {"Metal","Wood","Rubber"})`;
  `configSwitch(LIMITER_PARAM, 0, 1, 1, "Limiter", {"Off","On"})`; `configInput(TRIG_INPUT+i, "<name> trigger")`
  with names Bass drum, Snare 1, Snare 2, Low tom 1, Low tom 2, Mid tom 1, Mid tom 2, High tom 1, High tom 2, Splash,
  Ride, Closed hi-hat, Open hi-hat, Cowbell, Rimshot; `configInput` "Damping CV", "Strike position CV", "Room CV",
  "Tune 1 V/oct"; `configOutput` "Left (mono sum if Right unpatched)", "Right". (API: C-024.)
- Constructor ends with `core.seed(random::u32())`.
- `onSampleRateChange(e)` → `core.setSampleRate(e.sampleRate)`. `onReset(e)` → `Module::onReset(e); core.reset();`
  (C-007). No `onRandomize` override.
- `process()`: build `Controls` from params (`hammer = (int) std::round(value)`, `limiter = value > 0.5f`), call
  `setControls`, build `FrameIn` from `getVoltage()` of every input, `out = monoFold(core.processFrame(in),
  outputs[RIGHT_OUTPUT].isConnected())`, `setVoltage` both outputs.
- Widget: `setPanel(createPanel(asset::plugin(pluginInstance, "res/OilDrumKit.svg")))`, two `ScrewSilver` at
  `(RACK_GRID_WIDTH, 0)` and `(box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)`; every component
  placed with `create*Centered<Widget>(mm2px(Vec(layout::X.x, layout::X.y)), ...)` using the widget types in D-013.
- Registration: `Model* modelOilDrumKit = createModel<OilDrumKit, OilDrumKitWidget>("OilDrumKit");`,
  `init()` adds it. A compiling reference of exactly this contract is `research/spikes/wrapper_oracle/`.

**D-012 Performance target (amends A-006).** T-040: KitCore ≤ 1.15 × the engine alone on the same dense schedule
and block size (hardware-independent overhead bound). T-041: moderate pattern ≥ 10× real time at 48 kHz. The
engine's own cost is out of scope (A-004).

**D-013 Panel.** Single source `tools/layout.json`; generator `tools/gen_panel.py` (CLI: `python3 tools/gen_panel.py
[--out-dir DIR]`, default DIR = repository root) writes `DIR/res/OilDrumKit.svg` and `DIR/src/Layout.hpp`, with
LF line endings and deterministic output. Labels are converted to paths with fontTools 4.60.1 from
`tools/fonts/DejaVuSans.ttf` (sha256 `7da195a74c55bef988d0d48f9508bd5d849425c1770dba5d7bfc6ce9ed848954`, copied from
Rack v2.6.6 `res/fonts/`, license file `tools/fonts/DejaVuSans-LICENSE.txt` from the same place); the generator
verifies the hash. SVG: `width="101.60mm" height="128.5mm" viewBox="0 0 101.60 128.5"`, no text/CSS/images (C-019).
`Layout.hpp` format: `namespace odk { namespace layout {`, `constexpr int kPanelHp = 20;`,
`struct Pos { float x, y; };`, then one line per component `constexpr Pos <ID> = { <x>f, <y>f };` (mm, centre).
The adopted layout is `research/spikes/panel_oracle/layout.json` (verified against T-051/T-055/T-056); widget per id:
knobs `RoundBlackKnob`, HAMMER `CKSSThree`, LIMITER `CKSS`, all ports `PJ301MPort` (sizes C-020).

**D-014 Manifest and packaging.** `plugin.json` exactly as `research/spikes/wrapper_oracle/plugin.json`. Makefile
exactly as `research/spikes/wrapper_oracle/Makefile` (`FLAGS += -Iengine/Source -Isrc/core`,
`EXTRA_CXXFLAGS += -std=c++17`, `SOURCES += $(wildcard src/*.cpp) src/core/KitCore.cpp`,
`DISTRIBUTABLES += res LICENSE README.md`, `include $(RACK_DIR)/plugin.mk`). `LICENSE` = the Apache-2.0 text from
`engine/LICENSE`.

**D-015 Guard checks are green at freeze.** T-031..T-034 assert invariants that already hold on the planning branch
(no forbidden APIs, no secrets, engine pin, manifest). They cannot be red against stubs without being contrived; this
is a recorded deviation from Phase 2B.5, which applies to all behaviour tests (all red, verified).

**D-016 No CI.** (A-013.) `tests/run_all.sh` is the single verification command.

**D-017 Branching.** The implementer creates `impl/vcv-port` from the head of the generation branch, commits after
every step and pushes it. `main` is created from `impl/vcv-port` only after G-101 returns `proceed` (S-010).

**D-018 Rack SDK acquisition.** `env/setup_ubuntu.sh` downloads the official `Rack-SDK-2.6.6-lin-x64.zip` (C-002),
records its SHA-256 in `env/rack-sdk-2.6.6-lin-x64.sha256` on first use and verifies it thereafter; if the host is
unreachable it builds the verified shim SDK (`tools/make_shim_sdk.sh`, pinned Rack commit, C-003).

**D-019 Oracle spikes.** `research/spikes/kitcore_oracle/`, `panel_oracle/` and `wrapper_oracle/` were written only to
prove the frozen tests satisfiable. The implementer may adopt them; the deliverable is whatever passes the frozen suite.

**D-020 Demo render for the listening gate.** `tools/render_demo.cpp` links `src/core/KitCore.cpp` and writes
`renders/demo_48k.wav` (16-bit PCM stereo, 48 000 Hz) from this pattern, seed 1, default controls: for each instrument
0..14, three hits at 3 V, 6 V and 10 V spaced 0.4 s (pulse 1 ms), then 2 s of silence, then 8 bars at 120 BPM of
bass on beats 1 and 3, snare 1 on 2 and 4, closed hi-hat on every 8th (open hi-hat on the last 8th of each bar),
ride on bar 5–8 quarters. Conversion to 16-bit: `clamp(round(v / 5 * 32767), -32768, 32767)`.

**D-021 Protocol deviations in planning (disclosed).** No subagent spawning was available, so the adversarial pass
(R5), the cold reads (3.6) and the pre-mortems (Phase 4) were performed by the planning agent itself in separate,
explicitly adversarial passes rather than by fresh-context agents. This weakens their independence (R-019).

## Public interfaces

- `src/core/KitCore.hpp` (committed; normative together with D-005/D-010). Changing it = halt + `BLOCKED.md`.
- `tools/gen_panel.py` CLI and `src/Layout.hpp` format (D-013). `plugin.json` (D-014). Makefile targets `all`,
  `dist`, `clean`, `install` come from Rack's `plugin.mk`.

## Decision rules (if → then)

- **DR-01** Official SDK download fails (network/403) → `env/setup_ubuntu.sh` builds the shim SDK automatically; log
  "DR-01" in `DEVIATIONS.md`. If the shim build also fails → retry once; then halt with `BLOCKED.md`.
- **DR-02** Official SDK hash differs from `env/rack-sdk-2.6.6-lin-x64.sha256` → do not use it; halt, `BLOCKED.md`
  (possible supply-chain tampering; security).
- **DR-03** `apt-get install` fails for a package → `apt-get update` then retry once; if still failing, halt
  `BLOCKED.md` naming the package.
- **DR-04** Compile error inside `engine/` headers → check that `-std=c++17` is present (D-008) and the include path is
  `engine/Source`; never edit `engine/`. If unresolved → `BLOCKED.md`.
- **DR-05** T-010 fails → compare the implementation with every clause of D-010 (event order, offsets, render on
  `j == 127`, controls sampled on the firing frame, room/limiter on the block's last frame, output before render).
  Never alter the tolerance. After 3 failed fix attempts → `TEST_CHALLENGE.md`.
- **DR-06** T-012 fails while T-010 passes → the contract itself is at fault: write `TEST_CHALLENGE.md` (do not change
  `kBlock`).
- **DR-07** T-040 ratio > 1.15 → profile; optimise only `src/core/KitCore.cpp` (e.g. hoist per-frame branching); do not
  change `kBlock` or the engine. After 3 attempts → `TEST_CHALLENGE.md` with the measurements.
- **DR-08** T-041 < 10× while T-040 passes → hardware-limited: write `TEST_CHALLENGE.md` with CPU model
  (`lscpu`), measured values and halt.
- **DR-09** T-031 or T-032 fails → remove the offending code or secret before any push. If a secret was already pushed
  → halt, `BLOCKED.md`, and tell the human to revoke it (security).
- **DR-10** `git push` rejected → retry 3 times with 10 s, 30 s, 90 s backoff; then `git bundle create
  /tmp/oildrumkit-vcv-impl.bundle --all` and `BLOCKED.md` with its path.
- **DR-11** Engine submodule fetch fails → retry 3 times; then `BLOCKED.md`.
- **DR-12** `tools/fonts/DejaVuSans.ttf` hash mismatch → re-fetch from the pinned URL in S-006; if still mismatched →
  `BLOCKED.md` (supply chain).
- **DR-13** A frozen test looks wrong → never edit it; `TEST_CHALLENGE.md` (item ID, evidence, proposed fix) and halt.
- **DR-14** Compiler warnings in own code → fix them if the fix is local; warnings from `engine/` or Rack headers are
  accepted.
- **Default rule (verbatim):** Choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md`
  with rationale, and continue — **unless** it touches frozen tests, security, data integrity, a public interface,
  research integrity, or (upward) a statement's evidence class, in which case halt and write `BLOCKED.md`.
