# Risk register (final)

| ID | Risk | Severity (residual) | Owner / control |
|---|---|---|---|
| R-001 | 128-frame latency (2.7 ms @ 48 kHz) unacceptable | Medium | Human at G-101 (T1); README |
| R-002 | Engine saturates at ≥ 352.8 kHz | Low | README; out of scope |
| R-003 | Standard triggers → constant full velocity | Low | README; G-101 L2 |
| R-004 | Windows build unverified by agent | Medium | G-101 W1 + windows-build-fix loop |
| R-005 | No macOS build | Low | Out of scope |
| R-006 | Official SDK path untested in sandbox | Low | DR-01 shim fallback |
| R-007 | Engine CPU cost in dense patterns | Medium | Accepted; T-040 bounds wrapper overhead |
| R-008 | Trigger-time CPU spike | Low | — |
| R-009 | Compiler-dependent float differences | Low | Pinned toolchain |
| R-010 | Event ordering/timing mistakes | Low | T-010 + reference.hpp |
| R-011 | Chat-exposed GitHub token | Low **if revoked** (Critical otherwise) | Human revokes now; A-016; T-032 |
| R-012 | Engine drift | Low | T-033 |
| R-013 | Component Library licence | Low | README credit |
| R-014 | Slug collision on future Library submission | Low | Out of scope |
| R-015 | Perf-test flakiness | Low | Median of 5; reruns; DR-07/08 |
| R-016 | Invalid frozen test | Low | Oracle validation (C-026) |
| R-017 | Setup script privileges / PEP 668 | Low | Script handles; DR-03 |
| R-018 | Supply chain | Low | Hash pins; DR-02, DR-12 |
| R-019 | Reviews not independent (D-021) | Medium | Disclosed; optional re-review with subagents |
| R-020 | Poor sound at 11–24 kHz | Low | README |

Open Critical: 0. Open High: 0. Open Medium: R-001, R-004, R-007, R-019.

## v3.9 addendum risk round (2026-11-02)

| Risk | Severity | Mitigation | Status |
|---|---|---|---|
| No independent fresh-context review (D-906) | High | Recorded substitution; independent review scheduled before product execution resumes | Open |
| CI-dependent checks (mutation pricing/floor, matrix jobs) not locally executable (D-905) | Medium | CI is the authoritative venue; estimates labelled; post-conditions gate in CI | Open |
| Lesson/knowledge store push pending G-003 sign-off | Low | Bundle fallback documented; does not block amendment completion | Open |

Pre-mortem rounds repeated to zero Critical risks; the High risk is retained
honestly with owner and mitigation.
