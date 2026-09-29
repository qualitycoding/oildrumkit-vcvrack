# Research round 4 — saturation check

Re-examined every load-bearing claim below `verified` and every claim whose test runs under conditions that differ from
its spike:
- C-013 / C-016 were measured without FTZ/DAZ; the frozen tests run with FTZ/DAZ (C-017) and the oracle still passed
  T-010 (bitwise-level tolerance) and T-012 → no change.
- Engine submodule URL is public and anonymous-clonable; `main` is still at 35fbfea (pin unaffected by future pushes).
- C-002 (official SDK URL for 2.6.6) remains corroborated but unexercised in the sandbox → already R-006.
- C-022 remains single-source → already R-004 (gated).
Result: no new load-bearing claims, no confidence downgrades, no unresolved contradictions → **saturated**. Research
ends after 4 rounds.
