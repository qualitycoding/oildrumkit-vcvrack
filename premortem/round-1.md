# Pre-mortem round 1 ("It is six months later and the plan failed")

Lenses: implementer failure, environment, security, performance, scope/requirements, human gate, test validity.
Performed by the planning agent (D-021). Severity rubric per protocol Phase 4.

| Risk | Failure story | Severity before | Mitigation | After |
|---|---|---|---|---|
| R-001 | The user finds 2.7 ms latency unacceptable in tight patches. | Medium | Documented in README (S-005); asked explicitly at G-101 (T1); rescope path defined. | Medium (accepted, gated) |
| R-002 | User runs Rack at 352.8 kHz+ and the kit is saturated/harsh (engine behaviour, C-014). | Medium | README note (S-005); out of scope per A-004. | Low |
| R-003 | Standard 10 V triggers make every hit full velocity; users expect dynamics. | Medium | README "Velocity" section; G-101 L2. | Low |
| R-004 | Windows (MinGW) build fails; agent cannot test it (C-022 single-source). | High | G-101 W1 with literal commands; `windows-build-fix:` rescope loop. | Medium |
| R-005 | macOS users cannot use it. | Low | Out of scope (A-007). | Low |
| R-006 | Official SDK download path of `setup_ubuntu.sh` untested; zip layout differs. | Medium | Layout corroborated by two sources (C-002); fallback shim verified; DR-01. | Low |
| R-007 | Big patches overload CPU: dense roll costs ~1/3 of a core (engine). | Medium | Engine out of scope; T-040 guarantees ≤ 15 % wrapper overhead; README. | Medium (accepted) |
| R-008 | 15 simultaneous triggers spike ~150 µs in one frame. | Low | Rack's audio block buffers absorb it (C-023). | Low |
| R-009 | Different compiler on the implementer machine breaks the 1e-5 V bound. | Medium | Environment pins g++ 13.3.0 on Ubuntu 24.04; same flags in tests and plugin. | Low |
| R-010 | Implementer mis-orders events or samples controls on the wrong frame. | High | Normative D-010 + executable `tests/reference.hpp` + T-010 scenario with firing-frame-only CV values and block-boundary hits. | Low |
| R-011 | The GitHub token pasted into the planning chat is abused. | Critical | Plan never stores or reuses it (A-016); T-032 blocks committed secrets; human told to revoke it now. | Low once revoked (human action) |
| R-012 | Upstream engine changes alter behaviour. | Low | Submodule pin + T-033. | Low |
| R-013 | Component Library graphics licence (CC BY-NC) violated. | Medium | Free plugin; README credit (S-005, CR-3). | Low |
| R-014 | Slug `OilDrumKit` collides if later submitted to the Library. | Low | No submission in scope (A-008). | Low |
| R-015 | Frozen perf test T-040 flaky on a busy machine. | Medium | Measured 1.024 vs bound 1.15; median of 5 interleaved runs; S-004 allows 2 reruns; DR-07/DR-08. | Low |
| R-016 | A frozen test is invalid and blocks a correct implementation. | High | Oracle implementations pass every test (C-026); stub fails every behaviour test cleanly. | Low |
| R-017 | Setup script fails without sudo or with PEP 668 pip. | Medium | Script detects root/sudo and `--break-system-packages`; DR-03 → BLOCKED. | Low |
| R-018 | Supply-chain tampering of SDK/font/pip wheel. | Medium | SDK hash TOFU + DR-02; font hash DR-12; pip `--require-hashes`; engine pin. Shim apt headers are unpinned (compile-only). | Low |
| R-019 | Planning reviews lacked independent fresh-context agents (D-021). | Medium | Oracle-based test validation, explicit adversarial rounds, disclosure; human may re-run 3.6/Phase 4 in an environment with subagents. | Medium (disclosed) |
| R-020 | At 11.025–24 kHz the kit sounds poor (modes near Nyquist). | Low | Finite output guaranteed (T-030); README sample-rate note. | Low |

Round-1 counts after mitigation: Critical 0, High 0 (R-011 contingent on the human revoking the token).
New Critical/High found this round: R-004, R-010, R-011, R-016 (all mitigated) → another round required.
