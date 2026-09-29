# Research round 2 — synthesis, depth, adversarial

- R3 synthesis: two causes of segmentation dependence separated (shared PRNG order vs per-call mode pruning);
  question added: which block size reproduces host behaviour? (Q3.1).
- R4 depth: spikes `seg_main.cpp` (cross-TU determinism C-016; CPU vs block size), `srs.cpp` (all rates, C-014),
  `prune.cpp` (block size vs tail energy/duration, C-013), `misc.cpp` (C-010..C-012). Rack source read for event
  threading (C-007), FTZ/DAZ (C-017), sample-rate menu (C-025), API signatures (C-024), widget sizes (C-020).
- R5 adversarial (same agent, explicitly trying to break each claim; D-021): found (a) the Voltage Standards page has an
  older 2 V revision (contradiction, resolved); (b) Rack also runs at 11.025–24 kHz, not just ≥ 44.1 kHz → T-030 extended;
  (c) Rack sets FTZ/DAZ, so tests must too → harness; (d) `onReset` overrides must call `Module::onReset`;
  (e) Component Library graphics are CC BY-NC (credit required) → README requirement.
- Manifest, Panel, Licensing and Building manual pages fetched (C-004, C-018, C-019, C-022).
