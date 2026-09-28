# Research spikes (R6) — throwaway verification code

All spikes compiled with Rack's flags: `g++ -std=c++17 -O3 -funsafe-math-optimizations -fno-omit-frame-pointer -march=nehalem -I<engine>/Source`
against engine commit `35fbfead61a11a8c7f5609a95831a29f2cb2bf78`, g++ 13.3.0, Ubuntu 24.04, 1 vCPU sandbox.

| Spike | Claim(s) | Result |
|---|---|---|
| `c11.cpp` | C-008 | engine fails under `-std=c++11` (`std::clamp`), compiles under `-std=c++17` |
| `equiv.cpp` → `equiv.out` | C-013, C-015 | per-sample vs block rendering differ sample-wise (shared PRNG draw order); per-sample costs ~2x |
| `seg_main.cpp` + `seg_a.cpp` → `seg.out` | C-016, C-015 | same segmentation is bit-identical across translation units; CPU by block size |
| `srs.cpp` → `srs.out` | C-014 | all Rack sample rates 44.1k–768k finite; engine saturates to limiter ceiling at ≥352.8 kHz |
| `prune.cpp` → `prune.out` | C-013 | block size < 128 truncates tails (mode pruning once per `process()`); 128 matches 512-block host |
| `misc.cpp` → `misc.out` | C-011, C-012 | re-prepare ≡ fresh engine; reset gives exact zeros; onset on trigger sample; silence exact zeros |
| `plugin_build/` | C-001, C-003, C-008 | minimal plugin builds, links, `make dist` works against Rack v2.6.6 headers (`tools/make_shim_sdk.sh`) with `EXTRA_CXXFLAGS += -std=c++17` |
| `textpath.py` | C-019 | fontTools 4.60.1 converts labels to SVG paths from Rack's DejaVuSans.ttf |
| `kitcore_oracle/` | test validity | oracle implementation of D-005/D-010: all C++ frozen tests PASS (see `oracle_run.txt`); stub: all FAIL cleanly |
