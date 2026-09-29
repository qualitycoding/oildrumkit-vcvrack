# Research round 1 — decompose, breadth, first spikes

- R1: built `research/QUESTIONS.md` (Q1–Q5).
- R2 breadth: read Rack v2.6.6 build system and headers; fetched the Voltage Standards page; read the engine.
- R6 spikes: `c11.cpp` (C-008), `equiv.cpp` (C-013 per-sample vs block, C-015 cost, C-023), shim SDK +
  `plugin_build/` (C-003).
- New load-bearing findings: the engine needs C++17 but Rack defaults to C++11; engine output is not invariant to
  process() segmentation; per-sample processing costs ~2x; the official SDK host is blocked in the sandbox.
- Downgrades: intake default "bit-identical to a headless render" is not achievable as stated (C-013) → A-004.
