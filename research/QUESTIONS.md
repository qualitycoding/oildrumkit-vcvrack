# Question tree (profile branch: software / Engineering)

Status: A = answered (claim), P = pruned (informs nothing), R = residual risk.

- Q1 Toolchain and SDK
  - Q1.1 Which Rack version to target? → A C-001 (informs D-018, A-007)
  - Q1.2 Where is the SDK and how is it laid out? → A C-002 (S-001)
  - Q1.3 What does a plugin need to build/link? Can the agent build without the official SDK? → A C-003 (D-018, T-052)
  - Q1.4 C++ standard and compiler flags imposed by Rack; does the engine compile under them? → A C-008 (D-008)
  - Q1.5 Windows toolchain? → A C-022, single-source → R-004 (G-101)
- Q2 Rack module semantics
  - Q2.1 Trigger thresholds and states → A C-005, C-006 (D-004, T-006)
  - Q2.2 Voltage levels, V/oct, NaN handling, polyphony rules → A C-005 (D-005..D-007, T-030)
  - Q2.3 When/where are sample-rate and reset events delivered; is allocation allowed there? → A C-007 (D-010)
  - Q2.4 Threading and FPU mode → A C-009, C-017 (harness FTZ)
  - Q2.5 Which sample rates must be supported? → A C-025 (T-030)
  - Q2.6 API signatures used by the wrapper → A C-024 (D-011)
- Q3 Engine behaviour under a per-sample host
  - Q3.1 Is output independent of process() segmentation? → A C-013 (no) (D-003, T-012)
  - Q3.2 Cost of per-sample vs block processing → A C-015 (D-003, D-012)
  - Q3.3 Determinism across translation units → A C-016 (T-010 tolerance)
  - Q3.4 Reset/re-prepare semantics, onset timing, silence → A C-010, C-011, C-012 (D-010, T-011, T-020..T-022)
  - Q3.5 Stability across all rates → A C-014 (T-030, R-002)
  - Q3.6 Trigger-time CPU spikes → A C-023 (R-008)
- Q4 Packaging and presentation
  - Q4.1 Manifest schema and tags → A C-004 (D-014, T-050)
  - Q4.2 Panel rules, fonts, widget sizes → A C-019, C-020 (D-013, T-051, T-055)
  - Q4.3 Licensing of a non-GPL plugin and Component Library graphics → A C-018 (D-014, A-018)
- Q5 Test validity
  - Q5.1 Are the frozen tests satisfiable by a correct implementation? → A C-026 (oracle spikes)
- Pruned: P1 VST3/JUCE wrapper internals (the VCV port does not use them); P2 VCV Library submission process (A-008);
  P3 macOS notarisation (A-007); P4 engine voicing improvements (A-004).
