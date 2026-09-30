# G-101 evidence bundle (Windows build, Rack load, listening, latency)

Branch: `impl/vcv-port`. Implementation of `plan/PLAN.md` steps S-001..S-009 is complete.

## 1. Full frozen suite (Linux x64)

`RACK_DIR=/root/rack-sdk-2.6.6 bash tests/run_all.sh` — full log (ends with `ALL FROZEN TESTS PASSED`):

```
== test_mapping
PASS T-001 velocityFromVolts maps 0..10 V to 0..1, clamps, non-finite -> 0 (D-005, C-005)
PASS T-002 dampingFromUnit is 0.25*16^x on [0,1], clamped, non-finite -> 0.25 (D-005)
PASS T-003 unitWithCv adds cv/10 to the knob and clamps to [0,1]; non-finite args -> 0 (D-005)
PASS T-004 roomMixFromUnit is 0.5*x on [0,1], clamped (D-005; engine clamps room to 0..0.5)
PASS T-005 pitchHz follows 1 V/oct and semitone tune from each instrument's default (D-005, C-005)
PASS T-014 monoFold sums to LEFT only when RIGHT is unconnected (D-011)
SUMMARY 6/6 passed
== test_trigger
PASS T-006 Schmitt trigger: 0.1 V / 1.0 V thresholds, rising edge only, UNINITIALIZED start (C-006)
PASS T-007 velocity is taken from the trigger voltage on the firing frame (D-005, D-010)
SUMMARY 2/2 passed
== test_reference
PASS T-010 KitCore output equals the D-010 reference render at 44.1/48/96/192 kHz (tol 1e-5 V)
PASS T-011 latency is exactly kBlock frames and a hit sounds on its first output frame (D-003, C-012)
PASS T-023 default construction: 44.1 kHz, seed 1, default Controls (D-010 rule 1)
SUMMARY 3/3 passed
== test_fidelity
PASS T-012 single-hit fidelity vs 512-sample host blocks: tail energy +-0.05 dB, -60 dB duration +-1 %
SUMMARY 1/1 passed
== test_operational
PASS T-020 setSampleRate implies reset; invalid rates are ignored (D-010 rule 5)
PASS T-021 reset silences, zeroes counters and re-arms triggers as UNINITIALIZED (D-010 rule 6)
PASS T-022 no triggers -> output is exactly 0 V regardless of controls and CV (C-012)
SUMMARY 3/3 passed
== test_robustness
PASS T-030 hostile inputs (NaN, inf, 1e30 V, denormals, out-of-range controls, trigger storm) are safe
SUMMARY 1/1 passed
== test_perf
  T-040: core 2.832 s, engine 2.869 s, ratio 0.987
PASS T-040 dense roll (all 15 every 50 ms, 10 s): KitCore <= 1.15 x engine-only time
  T-041: 38.7x real time
PASS T-041 moderate pattern (one hit per 90 ms, 10 s at 48 kHz): KitCore >= 10x real time
SUMMARY 2/2 passed
PASS T-050 plugin manifest
SUMMARY 1/1 passed
PASS T-051 panel SVG meets Rack panel rules
PASS T-055 layout constraints (ids, widgets, margins, 2 mm clearance)
PASS T-056 generated panel and Layout.hpp are up to date
SUMMARY 3/3 passed
PASS T-052 plugin build
PASS T-053 dist package
SUMMARY 2/2 passed
PASS T-031 no file/network/process APIs in src/
PASS T-032 no credentials in tracked files
PASS T-033 engine submodule pinned and unmodified
PASS T-034 frozen manifest intact
SUMMARY 4/4 passed
ALL FROZEN TESTS PASSED
```

## 2. Commit under test and listening render

- Commit under test: `e411eb6c17fd2473f45d745d95be955b9b1e5792`
- Demo render (48 kHz, 16-bit stereo, 67.5 s; every instrument at 3/6/10 V, then an 8-bar beat):
  https://github.com/qualitycoding/oildrumkit-vcvrack/blob/impl/vcv-port/renders/demo_48k.wav
  (use "Download raw file" to listen outside GitHub)

## 3. Linux package

`dist/OilDrumKit-2.0.0-lin-x64.vcvplugin` (built by `make dist`; not committed). SDK used: **shim** (Rack v2.6.6
headers at `061ccf63`; the official SDK host was unreachable from the build sandbox; DR-01 logged in `DEVIATIONS.md`).
Measured: T-040 wrapper/engine ratio 0.987 (limit 1.15); T-041 38.7x real time (limit 10x).

4. **Windows build instructions for the human (literal):**
     1. Install MSYS2 from https://www.msys2.org/ and open **MSYS2 MINGW64** (not the default MSYS shell).
     2. `pacman -Syu` (restart the shell if asked), then
        `pacman -Syu git wget make tar unzip zip mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-cmake autoconf automake libtool mingw-w64-x86_64-jq python zstd mingw-w64-x86_64-pkgconf`
     3. `wget https://vcvrack.com/downloads/Rack-SDK-2.6.6-win-x64.zip && unzip -q Rack-SDK-2.6.6-win-x64.zip && export RACK_DIR="$PWD/Rack-SDK"`
     4. `git clone --recurse-submodules -b impl/vcv-port https://github.com/qualitycoding/oildrumkit-vcvrack && cd oildrumkit-vcvrack`
     5. `make -j4 && make install` (installs `OilDrumKit-2.0.0-win-x64.vcvplugin` into the Rack2 user folder), then
        restart VCV Rack 2 and add **Galactic HQ → Oil Drum Kit**.
  5. **Checklist for the human** (answer each item yes/no with a note):
     - W1 The Windows build and `make install` succeeded; the module appears in the browser and its panel renders
       (labels legible, no missing jacks/knobs).
     - W2 Patch a clock/sequencer (e.g. VCV SEQ-3 gate outputs) into BD, SN1, CH; audio is heard; no crackles at the
       default engine sample rate; the CPU meter reading is acceptable to you.
     - L1 Per instrument: metallic ring and decay shape sound like the VST at the same settings (use `renders/demo_48k.wav`
       or the VST side by side).
     - L2 Velocity: a trigger scaled to ~3 V through a VCA sounds softer and duller than 10 V.
     - L3 Hi-hat: a closed hi-hat trigger chokes a ringing open hi-hat.
     - L4 Controls: Damping, Strike, Room, Tune, Hammer and Limiter each audibly change the sound; the CV inputs do too.
     - L5 Mono: with only LEFT patched, both channels' content is heard.
     - T1 Latency: the fixed 128-frame delay (2.7 ms at 48 kHz, D-003) is acceptable against the rest of your patch.

## Questions for the human

- **Questions to the human:** (1) Do W1–W2 pass? (2) Do L1–L5 pass? (3) Is the latency in T1 acceptable?

Allowed responses: `proceed` | `proceed-with-rescope: <text>` | `stop`
(`proceed-with-rescope: windows-build-fix: <compiler/linker output>` lets the implementer fix the Windows build in
non-frozen files and re-enter this gate; see `plan/GATES.md`).

## Human response (2026-09-30)

`proceed`

Recorded as given; the implementer has no means to verify that W1-W2, L1-L5 and T1 were carried out on Windows/in Rack.
