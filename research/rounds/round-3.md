# Research round 3 — test validity and saturation

- R6: oracle spikes (`kitcore_oracle/`, `panel_oracle/`, `wrapper_oracle/`) proved every frozen test satisfiable (C-026)
  and found one invalid draft test (T-021 fed zeros before its UNINITIALIZED check) — corrected before freezing.
  Guard checks given positive controls (a fake token and an `fopen` call were detected).
- `env/setup_ubuntu.sh` executed end to end (shim path) and its SDK built the oracle plugin.
- Corroboration: C-018 by Rack LICENSE.md. C-022 (Windows packages) remains single-source Tier 1 → R-004, gated by G-101.
- Not saturated: this round added a load-bearing claim (C-026). Round 4 required.
