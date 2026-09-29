# HANDOFF — Oil Drum Kit as a VCV Rack 2 plugin

## Purpose
Port the existing modal-synthesis oil-drum engine (`qualitycoding/oildrumkit@35fbfea`, `Source/DrumEngine.h`) to a
VCV Rack 2 module, unchanged in sound, as a Rack-independent core (`src/core/KitCore.*`) plus a thin Rack wrapper.

## Active profile
`software` only; `software.deploys = false` (`plan/PROFILE.md`). No release, no publication.

## Reading order
1. This file. 2. `plan/PLAN.md` (steps S-001..S-010). 3. `plan/DECISIONS.md` (D-001..D-021, interfaces, decision rules
DR-01..DR-14 and the default rule). 4. `plan/GATES.md`. 5. `plan/ENVIRONMENT.md`. 6. `src/core/KitCore.hpp`.
7. `tests/reference.hpp` (executable form of D-010). Background only: `plan/ASSUMPTIONS.md`, `plan/TRACEABILITY.md`,
`research/`, `premortem/RISK_REGISTER.md`.

## Environment setup
Follow "Setup commands (literal)" in `plan/ENVIRONMENT.md` (Ubuntu 24.04 x86-64). You need a GitHub token with
Contents read/write on `qualitycoding/oildrumkit-vcvrack` in `GH_TOKEN` or a git credential helper. Never write a token
into a tracked file. Do **not** use the token that appeared in the planning conversation (A-016).

## Running the frozen suite
```bash
export RACK_DIR=<sdk dir from S-001>
bash tests/run_all.sh            # everything; prints ALL FROZEN TESTS PASSED on success
make -s -C tests run             # C++ tests only (T-001..T-041)
tests/build/test_reference T-010 # a single C++ test by ID
```
Before implementation every behaviour test fails with `not implemented` (red). The guard checks T-031..T-034 are green
from the start (D-015).

## Verifying the freeze
```bash
sha256sum -c tests/FROZEN_MANIFEST.sha256    # every line must print OK (this is also T-034)
```
Frozen files carry a `FROZEN — DO NOT MODIFY` header. You may not modify, skip, weaken or mark as expected-failure any
frozen file, and you may not change `src/core/KitCore.hpp` (public interface, also hash-pinned).

## Steps at a glance
| Step | What | Done when |
|---|---|---|
| S-001 | Environment, SDK, branch `impl/vcv-port` | manifest OK, tests compile, guards 4/4 |
| S-002 | Mapping functions | T-001..T-005, T-014 |
| S-003 | KitCore processing core | T-006, T-007, T-010, T-011, T-020..T-023, T-030 |
| S-004 | Fidelity and performance | T-012, T-040, T-041 |
| S-005 | plugin.json, Makefile, LICENSE, README | jq/grep checks |
| S-006 | Panel generator and panel | T-051, T-055, T-056 |
| S-007 | Rack wrapper | T-050, T-052, T-053 |
| S-008 | Demo WAV for listening | WAV check |
| S-009 | Full suite + gate evidence → **halt at G-101** | ALL FROZEN TESTS PASSED |
| S-010 | After `proceed`: push `main`, tag v2.0.0 | remote refs present |

Working references: `research/spikes/kitcore_oracle/`, `panel_oracle/` and `wrapper_oracle/` pass the frozen tests
and may be adopted (D-019).

## Human gates
- **G-101** at the end of S-009: Windows build, Rack load, listening checklist, latency acceptance. Write
  `GATE-G-101.md`, push, and wait. Allowed responses: `proceed`, `proceed-with-rescope: <text>`, `stop`
  (branches in `plan/GATES.md`; a `windows-build-fix:` rescope lets you fix the Windows build in non-frozen files and re-enter the gate). G-001 and G-002 are not applicable (`plan/GATES.md`).

## Halt / deviation protocol
- Every fork has a rule in `plan/DECISIONS.md` (DR-01..DR-14). For anything else apply the default rule:
  > Choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md` with rationale, and
  > continue — **unless** it touches frozen tests, security, data integrity, a public interface, research integrity,
  > or (upward) a statement's evidence class, in which case halt and write `BLOCKED.md`.
- A frozen test that appears wrong → `TEST_CHALLENGE.md` (item ID, evidence, proposed fix) and halt; the planning
  protocol is then re-run from Phase 0.
- Never modify `engine/` (T-033). Never create a GitHub Release, upload binaries or submit to the VCV Library.

## Integrity rule (Rule 9, verbatim)
**Integrity** `[All]`: No step may fabricate, cherry-pick without disclosure, or manually alter data, test results,
benchmarks, or figures. In addition:
* `[computational, publication]` Every reported number is generated from committed results, not transcribed by hand.
* `[publication]` Generative-AI images are never used as data figures. AI assistance is disclosed according to the
  venue's policy, as recorded in `plan/ASSUMPTIONS.md`.

(The two tagged sub-clauses are not applicable to this `software`-only plan; they are reproduced for completeness.)

## Operations
Not applicable (`software.deploys = false`); there is no `plan/OPERATIONS.md`.
