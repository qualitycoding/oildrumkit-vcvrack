# Profile record (Rule 0)

| Item | Value | Justification |
|---|---|---|
| Active profiles | `software` | The deliverable is a C++ VCV Rack plugin wrapping an existing engine. No new mathematics, no numerical research result, no publication (intake item 1, A-001). |
| `software.deploys` | `false` | Local build only; no VCV Library submission, no GitHub Release, no running service (A-008). |
| `math.exploration` | n/a | `math` inactive. |

## Not-applicable sections (produce no artifacts, impose no checks)

- Rule 7 (evidence classes), Rule 8 (result-agnostic planning): `[math, computational]` / `[math, publication]`.
- Rule 10 gate `G-001` `[math, computational]`. Gate `G-002` is `[All]` but not triggered: no external or irreversible action is in scope (see `plan/GATES.md`).
- Phase 0.3.3, 0.3.4, 0.3.5 intake blocks; Phase 1 R2b (novelty search), `research/NOVELTY.md`.
- Phase 2A (all of `math/`), 2B.2 provenance contract, 2B numerical V&V categories, 2B.4 unknown-outcome tests, 2C (`figures/SPEC.md`), 2D (`manuscript/`).
- `plan/OPERATIONS.md` (`software.deploys = false`); deployment tests; operational decision rules for deploys.
- Phase 4 lenses tagged only `[math]`, `[computational]`, `[publication]`.

Note: `computational` was considered and rejected. The engine's numerical behaviour is pre-existing and out of scope (A-004);
the port's numerical obligations (bit-level equivalence to a reference schedule, fidelity to host-block rendering) are
software tests (T-010, T-012), not research results.
