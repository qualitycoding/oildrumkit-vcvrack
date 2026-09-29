# Assumptions (A-###)

The Phase 0 intake batch was sent once. The human replied only with the target repository and a credential; every other
item adopted its proposed default (Global Rule 2, Phase 0.3.6 step 3).

| ID | Assumption / resolution | Source | Consequence |
|---|---|---|---|
| A-001 | Profiles: `software` only; `software.deploys = false`. | intake item 1 (default) | See `plan/PROFILE.md`. |
| A-002 | One module, 20 HP, 15 mono trigger inputs in engine order, velocity from trigger voltage, stereo mix out; no per-instrument outputs. | item 2 (default) | Panel size fixed by D-002/D-013. |
| A-003 | Controls: Damping, Strike, Room knobs each with a CV input; Hammer 3-way switch; Limiter switch; global Tune knob (±12 semitones) + 1 V/oct input scaling every fundamental. | item 3 (default) | D-005, D-011. |
| A-004 | Port the engine unchanged (no voicing or engine fixes). The engine is consumed read-only from `qualitycoding/oildrumkit@35fbfea`. | item 4 (default) | Known engine issues (C-013 pruning, C-014 high-rate saturation) are documented, not fixed. The intake's "bit-identical to a headless render" criterion is realised as T-010 (bit-level vs the D-010 reference schedule) and T-012 (fidelity vs 512-sample host blocks), because research showed output depends on call segmentation (C-013). |
| A-005 | Block adapter: intake default "per-sample, switch to 32-sample block if CPU target missed". | item 5 (default) | Superseded by D-003 (128-sample block) per its own decision rule and the fidelity finding C-013. |
| A-006 | CPU: "dense roll ≥ 4× real time". | item 6 (default) | Unachievable with the unmodified engine (engine alone ≈ 3.0×, C-015); amended by D-012. |
| A-007 | Rack 2.6.6 (latest tag, C-001). Windows x64 primary target, built and loaded by the human at G-101; Linux x64 for the agent's build and tests; macOS out of scope. | item 7 (default) | Windows build is not agent-verifiable (R-004). |
| A-008 | Distribution: local build only. No VCV Library submission, no GitHub Release, no binary publication. | item 8 (default) | No `G-002`. |
| A-009 | Repository: `github.com/qualitycoding/oildrumkit-vcvrack` (empty at planning start). Engine consumed as git submodule `engine/`. | human answer (overrides item 9 default) | D-001. |
| A-010 | Brand "Galactic HQ"; plugin and module slug `OilDrumKit` (not brand-prefixed; acceptable because no Library submission). | item 10 (default) | R-014 (Low). |
| A-011 | A human listening gate is required before completion. | item 11 (default) | G-101. |
| A-012 | Threat model: local desktop plugin. Untrusted inputs are cable voltages and parameter values (patch files may come from third parties). The plugin performs no network, file or process I/O of its own and handles no secrets or personal data. Data sensitivity: none. Retention: none. | planning resolution | Security tests T-030, T-031, T-032. |
| A-013 | No CI workflow (not requested; a fine-grained PAT may also lack the Workflows permission). Tests run locally via `tests/run_all.sh`. | planning resolution | D-016. |
| A-014 | Target user: the repository owner, running VCV Rack 2 (Free or Pro) on Windows x64. | planning resolution | — |
| A-015 | Versioning: semver `2.MINOR.REVISION` (major = Rack major, C-004), starting at `2.0.0`. Maintenance after delivery: owner only. Release channel: none. | planning resolution | — |
| A-016 | Implementer credential: a GitHub token with Contents read/write on `qualitycoding/oildrumkit-vcvrack`, supplied through the environment (`GH_TOKEN`) or a git credential helper, never written to a tracked file. **The token pasted into the planning conversation must be revoked by the human and must not be reused** (R-011). | planning resolution | Checked by T-032 for tracked files only. |
| A-017 | Implementer machine: Ubuntu 24.04 x86-64 (x86-64-v2 or newer, since Rack builds with `-march=nehalem`), network access to github.com, archive.ubuntu.com, pypi.org, and optionally vcvrack.com. | planning resolution | `plan/ENVIRONMENT.md`. |
| A-018 | Rack Component Library widgets (RoundBlackKnob, PJ301MPort, CKSS, CKSSThree, ScrewSilver) may be used: their graphics are licensed for non-commercial use, and this plugin is free (C-018). | planning resolution | R-013. |
| A-019 | Planning ran with one model tier (Opus) and no subagent spawning; see `.checkpoints/state.json` `tier_substitutions` and D-021. | environment diagnostic | R-019. |
| A-020 | AI assistance disclosure: not applicable (no publication profile). | planning resolution | — |
