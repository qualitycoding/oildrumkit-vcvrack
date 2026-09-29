# Traceability

## Success criteria → evidence → steps

| ID | Success criterion (measurable) | Evidence | Steps |
|---|---|---|---|
| SC-01 | The plugin builds for Linux x64 against Rack SDK 2.6.6 and packages as `OilDrumKit-2.0.0-lin-x64.vcvplugin` containing plugin.so, plugin.json, res/OilDrumKit.svg, LICENSE | T-052, T-053 | S-001, S-005, S-007 |
| SC-02 | The plugin builds on Windows x64 (MSYS2) and loads in VCV Rack 2 with a correctly rendered panel | G-101 W1 | S-009 |
| SC-03 | Each of the 15 trigger inputs fires its instrument per Rack's trigger standard (0.1 V/1.0 V Schmitt, rising edge, UNINITIALIZED start) | T-006, T-010 | S-003 |
| SC-04 | Velocity = trigger voltage / 10 V on the firing frame | T-001, T-007, T-010, G-101 L2 | S-002, S-003 |
| SC-05 | Damping, Strike, Room, Tune (±12 st + 1 V/oct), Hammer and Limiter act per D-005 | T-002..T-005, T-010, G-101 L4 | S-002, S-003, S-007 |
| SC-06 | Output equals the D-010 reference render within 1e-5 V at 44.1/48/96/192 kHz, including defaults | T-010, T-023 | S-003 |
| SC-07 | Single hits match the engine in 512-sample host blocks within ±0.05 dB tail energy and ±1 % −60 dB duration | T-012, G-101 L1 | S-003, S-004 |
| SC-08 | Latency is exactly 128 frames and is documented | T-011, S-005 README check, G-101 T1 | S-003, S-005 |
| SC-09 | Stereo output at ±5 V scale, mono fold on LEFT when RIGHT is unpatched | T-014, T-010, G-101 L5 | S-002, S-007 |
| SC-10 | Sample-rate change and reset silence the module and re-arm triggers; silence is exactly 0 V | T-020, T-021, T-022 | S-003 |
| SC-11 | Hostile inputs never produce non-finite output; limiter bounds output to 4.72 V; trigger storms are safe; all Rack engine rates 11.025–768 kHz | T-030 | S-003 |
| SC-12 | Wrapper overhead ≤ 15 % over the engine; ≥ 10× real time on a moderate pattern at 48 kHz | T-040, T-041 | S-004 |
| SC-13 | Panel: 20 HP, 128.5 mm, no text/CSS, layout constraints, generated artefacts current | T-051, T-055, T-056 | S-006 |
| SC-14 | Manifest, licence and module registration correct | T-050 | S-005, S-007 |
| SC-15 | Security: no file/network/process APIs, no committed credentials, engine pinned and unmodified | T-031, T-032, T-033 | S-001..S-009 |
| SC-16 | Frozen suite unmodified | T-034 | all |
| SC-17 | The human accepts the sound and latency | G-101 L1–L5, T1 | S-009, S-010 |

## Tests → requirement

| Test | File | Requirement(s) | Claims / decisions |
|---|---|---|---|
| T-001 | tests/test_mapping.cpp | SC-04 | C-005, D-005 |
| T-002 | tests/test_mapping.cpp | SC-05 | D-005 |
| T-003 | tests/test_mapping.cpp | SC-05 | C-005, D-005 |
| T-004 | tests/test_mapping.cpp | SC-05 | D-005 |
| T-005 | tests/test_mapping.cpp | SC-05 | C-005, D-005 |
| T-006 | tests/test_trigger.cpp | SC-03 | C-005, C-006, D-004 |
| T-007 | tests/test_trigger.cpp | SC-04 | D-005, D-010 |
| T-010 | tests/test_reference.cpp | SC-03..SC-06, SC-09 | C-013, C-016, D-010 |
| T-011 | tests/test_reference.cpp | SC-08 | C-012, D-003 |
| T-012 | tests/test_fidelity.cpp | SC-07 | C-013, D-003 |
| T-014 | tests/test_mapping.cpp | SC-09 | D-011 |
| T-020 | tests/test_operational.cpp | SC-10 | C-010, D-010 |
| T-021 | tests/test_operational.cpp | SC-10 | C-006, C-011, D-010 |
| T-022 | tests/test_operational.cpp | SC-10 | C-012 |
| T-023 | tests/test_reference.cpp | SC-06 | D-010 |
| T-030 | tests/test_robustness.cpp | SC-11, SC-15 | C-005, C-014, C-025, A-012 |
| T-031 | tests/test_guards.sh | SC-15 | A-012 |
| T-032 | tests/test_guards.sh | SC-15 | A-016 |
| T-033 | tests/test_guards.sh | SC-15 | C-021, D-001 |
| T-034 | tests/test_guards.sh | SC-16 | Phase 2E |
| T-040 | tests/test_perf.cpp | SC-12 | C-015, D-012 |
| T-041 | tests/test_perf.cpp | SC-12 | C-015, D-012 |
| T-050 | tests/test_manifest.py | SC-14 | C-004, C-018, D-014 |
| T-051 | tests/test_panel.py | SC-13 | C-019, D-013 |
| T-052 | tests/test_build.sh | SC-01 | C-003, C-008, D-008 |
| T-053 | tests/test_build.sh | SC-01 | C-003, D-014 |
| T-055 | tests/test_panel.py | SC-13 | C-020, D-013 |
| T-056 | tests/test_panel.py | SC-13 | D-013 |

Test categories: unit T-001..T-007, T-014; integration T-010..T-012, T-023, T-050..T-056; operational T-020..T-022
(start-up defaults T-023, configuration errors T-020 invalid rates, resource exhaustion T-030 storm, graceful reset
T-021); security T-030..T-033 (dependency scan: no package-manager dependencies; supply chain is covered by pins —
engine T-033, SDK hash D-018/DR-02, font hash DR-12, pip `--require-hashes`); performance T-040, T-041. Deployment
tests: N/A (`software.deploys = false`).
