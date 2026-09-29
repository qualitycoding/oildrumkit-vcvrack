# Deviations log
- DR-01: official Rack SDK unreachable; shim SDK used (2026-09-29)
- S-009: `plan/PLAN.md` says to `tee build/suite.log`, but `tests/test_build.sh` runs `make clean`, which deletes `build/`.
  The suite log was written to `/tmp/suite.log` and pasted into `GATE-G-101.md` instead (reversible, no scope change).
- S-008: the committed demo WAV is 13 MB (67.5 s), larger than the ~7 MB the plan assumed; well under GitHub's limits.
