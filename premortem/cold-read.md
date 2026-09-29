# Phase 3.6 cold read (performed by the planning agent in a separate pass; no fresh-context agent available, D-021)

Method: read `HANDOFF.md` → `plan/PLAN.md` → `plan/DECISIONS.md` → `plan/GATES.md` → `plan/ENVIRONMENT.md` as if new,
and for each step asked "could I execute this without guessing?".

| # | Finding | Severity | Fix |
|---|---|---|---|
| CR-1 | S-009 said "commit hash after committing the log-free files" — ambiguous about which commit to report. | Medium | Reworded: `git rev-parse HEAD` before `GATE-G-101.md` is committed. |
| CR-2 | G-101 had no path for a Windows compile error other than a full re-plan. | High | Added the `windows-build-fix:` rescope branch (non-frozen, behaviour-neutral fixes only; re-run suite; re-enter gate). |
| CR-3 | README licence section did not require the CC BY-NC credit that the Component Library licence demands (C-018). | Medium | S-005 now requires the credit text. |
| CR-4 | T-021's first draft could not pass for any correct implementation (zeros fed before the UNINITIALIZED check). | High | Found by the oracle run; corrected before freeze (research/rounds/round-3.md). |
| CR-5 | T-030 originally covered 48–768 kHz only; Rack also offers 11.025–24 kHz (C-025). | Medium | Extended before freeze. |
| CR-6 | Step dependencies: S-005/S-006 depend only on S-001, S-007 on S-003+S-005+S-006 — consistent with inputs. | — | none |
| CR-7 | Every step has Tier, Profile, Depends on, Inputs, Actions, Outputs, Evidence, Done when, Checkpoint, On failure, Gate, Relevant IDs. | — | none |
After fixes, a second pass found no step requiring a guess.
