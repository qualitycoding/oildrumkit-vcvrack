# EXECUTION PLAN: Oil Drum Kit VCV Rack 2 Defect Remediation & Hardening (v3.3 Protocol)
**Target Repository:** `qualitycoding/oildrumkit-vcvrack`  
**Base Engine:** `qualitycoding/oildrumkit@35fbfea`  
**Active Profile:** `software` (`software.deploys = false`)  
**Protocol Version:** `v3.3` (Periodic Checkpointing, Full Execution Context & Zero-Context Resumption)  
**Evidence Target:** Zero data loss, bit-reproducible renders, automated test suite exit 0, and human sign-off at Gate G-101.

---

## 1. Intake, Diagnostics & Known Defects from Retrospective

Based on the retrospective findings in `docs/RETROSPECTIVE.md` (oildrumkit) and the v2.0 implementation log of `oildrumkit-vcvrack`:

### 1.1 The Known Defects Addressed
1. **Missing Global 3000-Mode Voice Cap (Retrospective §9, §11.4, Engine 3.12):**
   - *Defect:* The modal synthesis engine previously enforced only `kMaxInstances = 8` per instrument, but lacked the global 3000-mode voice cap across all 15 instruments. In dense polyphonic rolls, worst-case CPU spikes could exceed real-time budgets and cause buffer underruns in VCV Rack.
   - *Fix:* Enforce active modal budget tracking and priority culling in `src/core/KitCore.cpp` ensuring active mode count $\le 3000$ at all times.

2. **Fixed 128-Frame Latency vs. Per-Sample VCV Rack Paradigm (D-003, R-001):**
   - *Defect:* VCV Rack calls `process(const ProcessArgs& args)` sample-by-sample. To match the block-based mode pruning (C-013), `OilDrumKit.cpp` buffered 128 samples, introducing a 2.7 ms delay (at 48 kHz).
   - *Fix:* Introduce an adaptive sub-block scheduling engine that allows low-latency direct evaluation for critical onset frames while preserving deterministic decay tail energy within $\pm 0.05$ dB.

3. **Trigger Velocity Usability & Voltage Scaling (R-003, T-007):**
   - *Defect:* Standard Eurorack sequencers emit 10 V gates, causing all hits to fire at 100% velocity unless routed through an external attenuator.
   - *Fix:* Add an optional auto-velocity toggle or dedicated Velocity CV inputs per voice group with internal attenuversion.

4. **Sample-Rate Mode Culling & High-Rate Limiter Saturation ($\ge 352.8$ kHz, C-014, R-002):**
   - *Defect:* At extreme host sample rates ($\ge 352.8$ kHz), the modal synthesis engine saturates the 0.944 limiter ceiling. Mode culling at $0.45 \cdot f_s$ and sample-rate invariant filter smoothing were never verified at 96 kHz and 192 kHz.
   - *Fix:* Add dynamic Nyquist clamping and test suite coverage across 44.1 kHz, 48 kHz, 96 kHz, 192 kHz, and 384 kHz.

5. **Missing Per-Instrument Direct Outputs (A-002 Limitation):**
   - *Defect:* The v2.0 module provided only main Left/Right mix jacks, preventing separate mixing, compression, or external spatialization of individual drums (Kick, Snare, Hats, Cymbals).
   - *Fix:* Extend the panel and DSP core with an expandable 8-channel direct out routing block (Kick, Snare 1/2, Toms, Hats, Ride/Crash, Cowbell, Rimshot).

6. **Windows MSVC/MSYS2 Validation Gap (Retrospective §9, R-004):**
   - *Defect:* Windows builds were left as manual instructions for the human at Gate G-101 without automated CI validation.
   - *Fix:* Implement GitHub Actions cross-platform build matrix validating Linux GCC and Windows MinGW-w64 on every push.

---

## 2. Checkpoint State Baseline (Protocol v3.3)

In accordance with Rule 5 and Phase 1.2, this plan operates with the enhanced checkpoint schema committed to `.checkpoints/state.json`:

```json
{
  "run_id": "run-oildrum-vcvrack-remediation",
  "sequence": 1,
  "branch": "fix/vcv-remediation",
  "profiles": ["software"],
  "mode_flags": { "strict_verification": true, "software.deploys": false },
  "phase": "3",
  "subphase": "3.1",
  "active_step_id": "S-001",
  "completed_steps": [],
  "pending_steps": ["S-001", "S-002", "S-003", "S-004", "S-005", "S-006", "S-007"],
  "artifacts": {
    "plan/PLAN.md": "a8274bc3f1208991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e",
    "src/core/KitCore.hpp": "c13a2ef4ae3022672a94144eae7572821b7c3250a8274bc3f1208991b7852b85"
  },
  "execution_context": {
    "current_objective": "Enforce global 3000-mode voice cap in KitCore",
    "call_stack": [
      "Phase 3: Execution Plan Formulation",
      "Step S-001: Implement 3000-mode global polyphony cap"
    ],
    "step_pointer": {
      "phase": "3",
      "subphase": "3.1",
      "item_index": 0,
      "sub_action": "writing_step_specification",
      "retry_count": 0
    },
    "scratchpad": "Analyzing KitCore.cpp active voice count. Mode cap must prune lowest energy modes when sum of active instrument modes > 3000 without clicking.",
    "in_flight_operation": null,
    "tool_cache": {
      "compiler_version": "g++ (Ubuntu 24.04) 13.2.0",
      "rack_sdk": "Rack-SDK-2.6.6 verified"
    }
  },
  "active_variables": {
    "global_mode_cap": 3000,
    "max_instruments": 15,
    "block_size": 128,
    "target_platforms": ["linux-x64", "win-x64"],
    "supported_sample_rates": [44100, 48000, 96000, 192000, 384000],
    "open_questions": [
      { "id": "Q-01", "status": "investigating", "question": "Optimal cross-fade time when global mode cap forces voice stealing" }
    ],
    "resolved_questions": [
      { "id": "Q-00", "status": "answered", "findings": "Schroeder T60 and Schroeder integration verified on unclipped float32" }
    ],
    "candidate_decisions": [
      { "id": "D-101", "selected_option": "Priority-queue modal eviction with 1ms Hann fade-out", "status": "approved" }
    ],
    "uncommitted_drafts": {},
    "variable_store": {
      "audio_buffer_frames": 128,
      "headroom_ceiling_dbfs": -0.5
    }
  },
  "updated_at": "2026-10-10T17:05:00.000Z"
}
```

---

## 3. Actionable Sub-Steps for Implementing Agent

### Step S-001: Global 3000-Mode Voice Polyphony Cap
- **Target Tier:** `sonnet`
- **Objective:** Prevent CPU underruns under dense rolls by enforcing an active modal cap of $\le 3000$ concurrent modes across all 15 instruments in `KitCore`.
- **Preconditions:** Git branch `fix/vcv-remediation` clean, `make -s -C tests run` passing baseline.
- **Exact Commands:**
  ```bash
  git checkout -b fix/vcv-remediation
  # Edit src/core/KitCore.hpp and src/core/KitCore.cpp to add GlobalModeBudgetTracker
  make -s -C tests run
  tests/build/test_perf
  ```
- **Files to Modify:**
  - `src/core/KitCore.hpp`
  - `src/core/KitCore.cpp`
  - `tests/test_perf.cpp`
- **Acceptance Criteria:**
  - [ ] Concurrent mode count never exceeds 3000 under all 15 instruments firing every 10 ms.
  - [ ] Stolen modes ramp to zero over 1 ms (48 samples at 48 kHz) to prevent audio clicks.
  - [ ] `T-040` (dense roll) and new `T-042` (stress roll with 3000 cap assertion) pass.
- **Verification Check:**
  ```bash
  tests/build/test_perf && echo "S-001 VERIFIED"
  ```
- **Fallback Rule:** If mode sorting exceeds 0.2 ms per block, use bucketed energy thresholds instead of `std::sort`.
- **Periodic Checkpoint:** Flush `.checkpoints/state.json` (Seq #2).

---

### Step S-002: Adaptive Sub-Block Scheduling Engine (Latency Reduction)
- **Target Tier:** `sonnet`
- **Objective:** Reduce the fixed 128-sample latency (2.7 ms) by supporting an adaptive sub-block buffer (32, 64, or 128 samples) without altering modal tail fidelity.
- **Preconditions:** S-001 verified.
- **Exact Commands:**
  ```bash
  # Implement adaptive block dispatch in src/core/KitCore.cpp and src/OilDrumKit.cpp
  make -s -C tests test_fidelity
  tests/build/test_reference T-011
  ```
- **Files to Modify:**
  - `src/core/KitCore.cpp`
  - `src/OilDrumKit.cpp`
  - `tests/test_reference.cpp`
  - `tests/test_fidelity.cpp`
- **Acceptance Criteria:**
  - [ ] Block size selectable via compile-time or runtime config (`kBlock = 32, 64, 128`).
  - [ ] At `kBlock = 32`, latency reduced to 0.67 ms at 48 kHz.
  - [ ] Tail energy within $\pm 0.05$ dB of 512-sample host reference.
- **Verification Check:**
  ```bash
  tests/build/test_reference && tests/build/test_fidelity
  ```
- **Fallback Rule:** If sub-block PRNG draw order drifts, isolate PRNG seeds per instrument voice.
- **Periodic Checkpoint:** Flush `.checkpoints/state.json` (Seq #3).

---

### Step S-003: Velocity CV Normalization & Dynamic Attenuation
- **Target Tier:** `haiku`
- **Objective:** Prevent standard 10 V Eurorack gates from always triggering at maximum velocity by implementing configurable velocity sensitivity.
- **Preconditions:** S-002 verified.
- **Exact Commands:**
  ```bash
  # Update src/OilDrumKit.cpp with Velocity Mode switch / attenuverter
  make -s -C tests test_mapping test_trigger
  ```
- **Files to Modify:**
  - `src/OilDrumKit.cpp`
  - `src/Layout.hpp`
  - `tests/test_mapping.cpp`
  - `tests/test_trigger.cpp`
- **Acceptance Criteria:**
  - [ ] If input is a 10 V gate pulse (square), velocity follows knob attenuator setting.
  - [ ] If input is a dynamic trigger, velocity is captured from edge voltage as before.
  - [ ] Tests `T-001` through `T-007` remain 100% green.
- **Verification Check:**
  ```bash
  tests/build/test_mapping && tests/build/test_trigger
  ```
- **Fallback Rule:** Default switch state must preserve legacy 1:1 voltage mapping for backward compatibility.
- **Periodic Checkpoint:** Flush `.checkpoints/state.json` (Seq #4).

---

### Step S-004: Sample-Rate Nyquist Mode Culling & High-Rate Limiter Softening
- **Target Tier:** `sonnet`
- **Objective:** Fix high-rate saturation ($\ge 352.8$ kHz) and ensure clean frequency response at 96 kHz, 192 kHz, and 384 kHz.
- **Preconditions:** S-003 verified.
- **Exact Commands:**
  ```bash
  # Implement 0.45 * fs mode culling and soften limiter knee in src/core/KitCore.cpp
  tests/build/test_operational
  tests/build/test_robustness
  ```
- **Files to Modify:**
  - `src/core/KitCore.cpp`
  - `tests/test_operational.cpp`
  - `tests/test_robustness.cpp`
- **Acceptance Criteria:**
  - [ ] Modes with frequency $> 0.45 \cdot f_s$ culled at initialization across all sample rates.
  - [ ] Peak output at 384 kHz strictly under 0.944 without hard clipping.
  - [ ] Zero NaNs, Infs, or denormals across all sample rates.
- **Verification Check:**
  ```bash
  tests/build/test_operational && tests/build/test_robustness
  ```
- **Fallback Rule:** If culling reduces cymbal brightness at 44.1 kHz, compensate with energy-density matched noise residual.
- **Periodic Checkpoint:** Flush `.checkpoints/state.json` (Seq #5).

---

### Step S-005: Expandable Direct Instrument Voice Outputs
- **Target Tier:** `haiku`
- **Objective:** Add direct output jacks for individual drum groups (Kick, Snare, Toms, Hats, Metals) to allow external mixing.
- **Preconditions:** S-004 verified.
- **Exact Commands:**
  ```bash
  # Update panel definition and Layout.hpp
  python3 tools/gen_panel.py
  make -s -C tests test_panel
  ```
- **Files to Modify:**
  - `tools/layout.json`
  - `tools/gen_panel.py`
  - `src/Layout.hpp`
  - `src/OilDrumKit.cpp`
  - `tests/test_panel.py`
- **Acceptance Criteria:**
  - [ ] When direct output is patched, that voice is optionally removed from main stereo mix (normalled break).
  - [ ] Panel SVG satisfies all VCV Rack 2 design guidelines and 2mm clearance rules.
  - [ ] `T-051`, `T-055`, and `T-056` pass.
- **Verification Check:**
  ```bash
  python3 tests/test_panel.py
  ```
- **Fallback Rule:** Maintain main Left/Right mix identical when no direct jacks are plugged.
- **Periodic Checkpoint:** Flush `.checkpoints/state.json` (Seq #6).

---

### Step S-006: Automated Cross-Platform CI (Linux & Windows MinGW)
- **Target Tier:** `script`
- **Objective:** Automate the build and test suite in GitHub Actions, eliminating the unverified Windows build risk (R-004).
- **Preconditions:** S-005 verified.
- **Exact Commands:**
  ```bash
  mkdir -p .github/workflows
  # Create .github/workflows/ci.yml with ubuntu-latest and windows-latest
  git add .github/workflows/ci.yml
  ```
- **Files to Modify:**
  - `.github/workflows/ci.yml`
- **Acceptance Criteria:**
  - [ ] CI runs `tests/run_all.sh` on Linux x86_64.
  - [ ] CI compiles with MinGW-w64 on Windows and runs `test_reference` successfully.
- **Verification Check:**
  ```bash
  bash tests/run_all.sh
  ```
- **Fallback Rule:** Use cached official Rack SDK in CI runner.
- **Periodic Checkpoint:** Flush `.checkpoints/state.json` (Seq #7).

---

### Step S-007: Full Frozen Verification Suite & Human Gate G-101
- **Target Tier:** `sonnet`
- **Objective:** Execute the entire verification suite, produce new 48 kHz / 96 kHz listening renders, update `GATE-G-101.md`, and await human sign-off.
- **Preconditions:** S-001 through S-006 verified.
- **Exact Commands:**
  ```bash
  bash tests/run_all.sh 2>&1 | tee /tmp/suite_final.log
  ./build/render_demo renders/demo_remediated_48k.wav
  # Update GATE-G-101.md with test logs and audio link
  ```
- **Files to Modify:**
  - `GATE-G-101.md`
  - `renders/demo_remediated_48k.wav`
- **Acceptance Criteria:**
  - [ ] `tests/run_all.sh` outputs `ALL FROZEN TESTS PASSED`.
  - [ ] Audio render contains zero clicks, stable decay, and balanced frequency spectrum.
  - [ ] Human reviews and signs off on gate `GATE-G-101.md`.
- **Verification Check:**
  ```bash
  grep "ALL FROZEN TESTS PASSED" /tmp/suite_final.log
  ```
- **Fallback Rule:** If human requests tuning at G-101, enter scoped tuning loop without modifying frozen test definitions.
- **Final Checkpoint:** State marked completed.

---

## 4. Pre-Mortem Risk Register & Circuit Breakers

| Risk ID | Failure Mode | Severity | Mitigation & Verification Check |
|---|---|---|---|
| **R-101** | Voice stealing click when 3000-mode cap evicts active voices | High | Enforce 1 ms cosine fade-out window before zeroing mode state (`S-001`). Verified by `test_robustness`. |
| **R-102** | Sub-block processing breaks PRNG voice decorrelation | Medium | Dedicated per-voice PRNG state instance instead of shared engine seed (`S-002`). |
| **R-103** | High-rate culling dulls cymbal shimmer at 44.1 kHz | Medium | Guard condition: culling only activates when mode frequency $> 0.45 \cdot f_s$ AND $f_s > 48000$ (`S-004`). |
| **R-104** | Direct output breaking changes panel dimensions | Low | Maintain strict 20 HP footprint using compact vertical dual-jacks (`S-005`). |
| **R-105** | Agent session crash or external reset during long build | High | **Protocol v3.3 Zero-Context Resumption:** Checkpoint saved after every step. Fresh agent inspects `.checkpoints/state.json`, verifies artifact hashes, rehydrates state, and resumes without rework. |
