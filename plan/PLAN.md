<!-- STATUS HEADER (Phase 5) -->

# Implementation plan — Oil Drum Kit as a VCV Rack 2 plugin

Read `HANDOFF.md` first. Every step runs on branch `impl/vcv-port` (D-017). After each step: commit with message
`S-### <title>`, update `.checkpoints/impl-state.json` (schema below), and `git push origin impl/vcv-port`
(on rejection: DR-10). Every step is idempotent: re-running it overwrites its outputs.

`impl-state.json` schema: `{"step_completed": ["S-001", ...], "sdk": {"kind": "official|shim", "sha256": "..."},
"last_suite": "<summary line>", "updated_at": "<ISO-8601 UTC>"}`. Resume = run the first step not in `step_completed`.

Commands assume the repository root as working directory and `RACK_DIR` exported (S-001).

### S-001 Environment and branch
- Tier: Sonnet
- Profile: software
- Depends on: none
- Inputs: `plan/ENVIRONMENT.md`, `env/setup_ubuntu.sh`, `env/requirements.lock`, `tools/make_shim_sdk.sh`, a GitHub token in `GH_TOKEN` or a credential helper (A-016)
- Actions:
  1. Run the "Setup commands (literal)" block in `plan/ENVIRONMENT.md`.
  2. If `env/rack-sdk-2.6.6-lin-x64.sha256` was created, `git add` it.
  3. Create `DEVIATIONS.md` containing the line `# Deviations log` (skip if it exists). If the setup printed the DR-01 message, append `- DR-01: official Rack SDK unreachable; shim SDK used (<date>)`.
  4. Write `.checkpoints/impl-state.json` with `step_completed: ["S-001"]` and the SDK kind and hash (hash of the zip, or `"shim"`).
- Outputs: `DEVIATIONS.md`, `.checkpoints/impl-state.json`, optionally `env/rack-sdk-2.6.6-lin-x64.sha256`
- Evidence produced: none
- Done when: `sha256sum -c tests/FROZEN_MANIFEST.sha256` prints only `OK` lines; `test -f "$RACK_DIR/plugin.mk"`; `make -s -C tests all` exits 0; `bash tests/test_guards.sh` prints `SUMMARY 4/4 passed`.
- Checkpoint: `step_completed += S-001`, `sdk`.
- On failure: DR-01, DR-02, DR-03, DR-11. Re-running is safe (script is idempotent).
- Gate: none
- Relevant decisions/claims: D-017, D-018, C-001, C-002, C-003

### S-002 Mapping functions
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: `src/core/KitCore.hpp`, `src/core/KitCore.cpp` (stub), `plan/DECISIONS.md` D-005, D-009; optional reference `research/spikes/kitcore_oracle/KitCore.cpp`
- Actions:
  1. In `src/core/KitCore.cpp`, `#include "DrumEngine.h"` and `<cmath>`, `<algorithm>`; add `static_assert(kNumInstruments == oildrum::kNumInstruments)` and `static_assert(kBlock == oildrum::kChunk)`.
  2. Replace the stub bodies of `velocityFromVolts`, `unitWithCv`, `dampingFromUnit`, `roomMixFromUnit`, `pitchHz`, `monoFold` with the D-005 formulas (non-finite argument → 0; `monoFold` per D-011). Leave the `KitCore` member stubs untouched.
  3. `make -s -C tests all && tests/build/test_mapping`.
- Outputs: `src/core/KitCore.cpp`
- Evidence produced: T-001, T-002, T-003, T-004, T-005, T-014 pass
- Done when: `tests/build/test_mapping` prints `SUMMARY 6/6 passed`.
- Checkpoint: `step_completed += S-002`.
- On failure: fix against D-005; never edit tests (DR-13).
- Gate: none
- Relevant decisions/claims: D-005, D-009, C-005, C-008

### S-003 KitCore processing core
- Tier: Opus
- Profile: software
- Depends on: S-002
- Inputs: `plan/DECISIONS.md` D-003, D-004, D-006, D-010; `tests/reference.hpp` (read-only, the executable form of D-010)
- Actions:
  1. Implement `KitCore::Impl` holding one `oildrum::DrumEngine`, the sample rate, current `Controls`, 15 Schmitt states, a fixed event array of `kBlock * kNumInstruments`, two output and two render buffers of `kBlock` floats, the frame index, hit counters and last velocities. No heap allocation after construction except inside `setSampleRate()`/`reset()` (engine room buffers).
  2. Implement the constructor, `setSampleRate`, `seed`, `setControls`, `processFrame`, `reset`, `hitCount`, `lastVelocity` exactly per D-010 rules 1–7.
  3. `make -s -C tests all` and run `tests/build/test_trigger`, `tests/build/test_reference`, `tests/build/test_operational`, `tests/build/test_robustness`.
- Outputs: `src/core/KitCore.cpp`
- Evidence produced: T-006, T-007, T-010, T-011, T-020, T-021, T-022, T-023, T-030 pass
- Done when: the four binaries print `SUMMARY 2/2`, `SUMMARY 3/3`, `SUMMARY 3/3`, `SUMMARY 1/1 passed` respectively.
- Checkpoint: `step_completed += S-003`.
- On failure: DR-05; DR-13 for suspected test errors.
- Gate: none
- Relevant decisions/claims: D-003, D-004, D-010, C-006, C-007, C-010, C-011, C-012, C-013, C-016

### S-004 Fidelity and performance
- Tier: Sonnet
- Profile: software
- Depends on: S-003
- Inputs: `src/core/KitCore.cpp`
- Actions:
  1. `make -s -C tests all`; run `tests/build/test_fidelity` (≈ 5–15 min on one core) and `tests/build/test_perf` (≈ 1–2 min). Run the performance test on an otherwise idle machine.
  2. Record the printed T-040 ratio and T-041 factor in `impl-state.json` `last_suite`.
- Outputs: `.checkpoints/impl-state.json`
- Evidence produced: T-012, T-040, T-041 pass
- Done when: `tests/build/test_fidelity` prints `SUMMARY 1/1 passed` and `tests/build/test_perf` prints `SUMMARY 2/2 passed`.
- Checkpoint: `step_completed += S-004`, ratio and factor.
- On failure: DR-06 (T-012), DR-07 (T-040), DR-08 (T-041). A single T-040 failure may be re-run twice before applying DR-07 (timer noise).
- Gate: none
- Relevant decisions/claims: D-003, D-012, C-013, C-015

### S-005 Manifest, licence, readme, build files
- Tier: Haiku
- Profile: software
- Depends on: S-001
- Inputs: `research/spikes/wrapper_oracle/plugin.json`, `research/spikes/wrapper_oracle/Makefile`, `engine/LICENSE`
- Actions:
  1. `cp research/spikes/wrapper_oracle/plugin.json plugin.json && cp research/spikes/wrapper_oracle/Makefile Makefile && cp engine/LICENSE LICENSE`
  2. Write `README.md` with these sections (headings exactly): `## Build` (Linux and Windows commands from `plan/ENVIRONMENT.md` and `plan/GATES.md`), `## Controls` (a table of every param/input/output from D-011 with D-005 mappings), `## Latency` (state 128 frames and the four millisecond values from D-003), `## Velocity` (trigger voltage on the firing frame, 10 V = full; use a VCA for dynamics), `## Sample rates` (behaviour identical to the VST up to 192 kHz; at 352.8 kHz and above the engine drives its limiter, C-014), `## Licences` (Apache-2.0; engine from qualitycoding/oildrumkit; panel labels from DejaVu Sans, see `tools/fonts/DejaVuSans-LICENSE.txt`; VCV Component Library graphics © VCV, licensed CC BY-NC 4.0, used non-commercially).
- Outputs: `plugin.json`, `Makefile`, `LICENSE`, `README.md`
- Evidence produced: none (T-050 completes in S-007)
- Done when: `jq -e '.slug=="OilDrumKit"' plugin.json` exits 0; `for h in "## Build" "## Controls" "## Latency" "## Velocity" "## Sample rates" "## Licences"; do grep -qF "$h" README.md || echo MISSING $h; done` prints nothing; `grep -q "Apache License" LICENSE`.
- Checkpoint: `step_completed += S-005`.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-014, C-004, C-018

### S-006 Panel generation
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: `research/spikes/panel_oracle/layout.json`, `research/spikes/panel_oracle/gen_panel.py`, D-013
- Actions:
  1. `mkdir -p tools/fonts && cp research/spikes/panel_oracle/layout.json tools/layout.json && cp research/spikes/panel_oracle/gen_panel.py tools/gen_panel.py`
  2. `curl -fsSL -o tools/fonts/DejaVuSans.ttf https://raw.githubusercontent.com/VCVRack/Rack/v2.6.6/res/fonts/DejaVuSans.ttf && curl -fsSL -o tools/fonts/DejaVuSans-LICENSE.txt https://raw.githubusercontent.com/VCVRack/Rack/v2.6.6/res/fonts/DejaVuSans-LICENSE.txt`
  3. `echo "7da195a74c55bef988d0d48f9508bd5d849425c1770dba5d7bfc6ce9ed848954  tools/fonts/DejaVuSans.ttf" | sha256sum -c` (on mismatch: DR-12).
  4. Edit the first comment line of `tools/gen_panel.py` to `# Panel generator (plan/DECISIONS.md D-013): tools/layout.json -> res/OilDrumKit.svg + src/Layout.hpp.`
  5. `python3 tools/gen_panel.py` then `python3 tests/test_panel.py`.
- Outputs: `tools/layout.json`, `tools/gen_panel.py`, `tools/fonts/DejaVuSans.ttf`, `tools/fonts/DejaVuSans-LICENSE.txt`, `res/OilDrumKit.svg`, `src/Layout.hpp`
- Evidence produced: T-051, T-055, T-056 pass
- Done when: `python3 tests/test_panel.py` prints `SUMMARY 3/3 passed`.
- Checkpoint: `step_completed += S-006`.
- On failure: DR-12; if layout changes are needed, edit only `tools/layout.json` and regenerate.
- Gate: none
- Relevant decisions/claims: D-013, C-019, C-020

### S-007 Rack wrapper
- Tier: Opus
- Profile: software
- Depends on: S-003, S-005, S-006
- Inputs: D-007, D-011; `research/spikes/wrapper_oracle/{plugin.hpp,plugin.cpp,OilDrumKit.cpp}`; `src/Layout.hpp`
- Actions:
  1. Create `src/plugin.hpp`, `src/plugin.cpp`, `src/OilDrumKit.cpp` implementing D-011 exactly (the oracle files implement it and may be copied, removing the "SPIKE" comment).
  2. `make clean && make -j"$(nproc)"`, then `python3 tests/test_manifest.py` and `bash tests/test_build.sh`.
- Outputs: `src/plugin.hpp`, `src/plugin.cpp`, `src/OilDrumKit.cpp`
- Evidence produced: T-050, T-052, T-053 pass
- Done when: `python3 tests/test_manifest.py` prints `SUMMARY 1/1 passed` and `bash tests/test_build.sh` prints `SUMMARY 2/2 passed`.
- Checkpoint: `step_completed += S-007`.
- On failure: DR-04, DR-14; a missing Rack API symbol → check C-024 locators in the Rack headers under `$RACK_DIR/include`.
- Gate: none
- Relevant decisions/claims: D-007, D-008, D-011, D-014, C-003, C-007, C-024

### S-008 Demo render for the listening gate
- Tier: Sonnet
- Profile: software
- Depends on: S-003
- Inputs: D-020, `src/core/KitCore.hpp`
- Actions:
  1. Write `tools/render_demo.cpp` implementing D-020 (drive `odk::KitCore` frame by frame at 48 kHz; write a canonical 44-byte-header PCM WAV; discard the first `kBlock` output frames so audio is aligned).
  2. `mkdir -p build renders && g++ -std=c++17 -O3 -funsafe-math-optimizations -march=nehalem -Isrc/core -Iengine/Source tools/render_demo.cpp src/core/KitCore.cpp -o build/render_demo && ./build/render_demo renders/demo_48k.wav`
- Outputs: `tools/render_demo.cpp`, `renders/demo_48k.wav` (committed)
- Evidence produced: none (gate evidence)
- Done when: `python3 -c "import wave,array;w=wave.open('renders/demo_48k.wav');assert (w.getnchannels(),w.getsampwidth(),w.getframerate())==(2,2,48000);n=w.getnframes();assert n>=48000*20;a=array.array('h',w.readframes(n));assert max(abs(x) for x in a)>1000;print('ok')"` prints `ok`.
- Checkpoint: `step_completed += S-008`.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-020

### S-009 Full suite and G-101 evidence
- Tier: Sonnet
- Profile: software
- Depends on: S-004, S-007, S-008
- Inputs: all of the above
- Actions:
  1. `RACK_DIR="$RACK_DIR" bash tests/run_all.sh 2>&1 | tee build/suite.log`
  2. Write `GATE-G-101.md` containing every item of the G-101 evidence bundle in `plan/GATES.md` (paste `build/suite.log` in a code block; include the output of `git rev-parse HEAD` taken before `GATE-G-101.md` is committed (the commit containing the code under test), and `https://github.com/qualitycoding/oildrumkit-vcvrack/blob/impl/vcv-port/renders/demo_48k.wav`; copy the Windows instructions and checklist verbatim).
  3. Commit, push, **halt** and wait for the human's response.
- Outputs: `GATE-G-101.md`
- Evidence produced: all T-### recorded in one log
- Done when: `build/suite.log` ends with `ALL FROZEN TESTS PASSED`; `GATE-G-101.md` is pushed.
- Checkpoint: `step_completed += S-009`, `last_suite` = final line of the log.
- On failure: resolve with the DR for the failing test; never proceed to the gate with failures.
- Gate: G-101
- Relevant decisions/claims: D-003, D-021, C-022

### S-010 Publish to main (only after G-101 = proceed)
- Tier: Haiku
- Profile: software
- Depends on: S-009 and G-101 response `proceed`
- Inputs: the human's G-101 response
- Actions:
  1. Append the response and date to `GATE-G-101.md`; commit.
  2. `git push origin impl/vcv-port:main`
  3. `git tag -a v2.0.0 -m "Oil Drum Kit 2.0.0 for VCV Rack 2"` and `git push origin v2.0.0`
- Outputs: remote `main`, tag `v2.0.0`
- Evidence produced: none
- Done when: `git ls-remote origin refs/heads/main` shows the hash of `git rev-parse HEAD`, and `git ls-remote --tags origin v2.0.0` is non-empty.
- Checkpoint: `step_completed += S-010`.
- On failure: DR-10. Never create a GitHub Release or upload binaries (A-008).
- Gate: none
- Relevant decisions/claims: D-017, A-008
