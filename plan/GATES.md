# Human gates

| Gate | Status | Justification |
|---|---|---|
| G-001 | N/A | Tagged `[math, computational]`; neither profile is active (`plan/PROFILE.md`). |
| G-002 | Not triggered | No external or irreversible action is in scope: no production deployment, public release, package publication, archive deposit or submission (A-008). Pushing branches to the owner's own repository and creating `main` are reversible git operations. If the human later wants a VCV Library submission or a GitHub Release, that is a new task requiring a new plan with G-002. |
| G-101 | Required | Windows build, load test and listening test; latency acceptance. Machine checks cannot judge sound or build on Windows (A-007, A-011, R-001, R-003, R-004). |

## G-101 — Windows build, Rack load and listening

- **Trigger step:** S-009 (after the full frozen suite passes on Linux).
- **Evidence bundle** (the implementer writes `GATE-G-101.md` at the repository root of `impl/vcv-port`, commits and
  pushes it, then halts):
  1. Output of `RACK_DIR=<sdk> bash tests/run_all.sh` (full log, ends with `ALL FROZEN TESTS PASSED`).
  2. The commit hash of `impl/vcv-port` and the GitHub URL of `renders/demo_48k.wav` (D-020) for listening outside Rack.
  3. The Linux package path `dist/OilDrumKit-2.0.0-lin-x64.vcvplugin` and whether the SDK used was official or shim.
  4. **Windows build instructions for the human (literal):**
     1. Install MSYS2 from https://www.msys2.org/ and open **MSYS2 MINGW64** (not the default MSYS shell).
     2. `pacman -Syu` (restart the shell if asked), then
        `pacman -Syu git wget make tar unzip zip mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-cmake autoconf automake libtool mingw-w64-x86_64-jq python zstd mingw-w64-x86_64-pkgconf`
     3. `wget https://vcvrack.com/downloads/Rack-SDK-2.6.6-win-x64.zip && unzip -q Rack-SDK-2.6.6-win-x64.zip && export RACK_DIR="$PWD/Rack-SDK"`
     4. `git clone --recurse-submodules -b impl/vcv-port https://github.com/qualitycoding/oildrumkit-vcvrack && cd oildrumkit-vcvrack`
     5. `make -j4 && make install` (installs `OilDrumKit-2.0.0-win-x64.vcvplugin` into the Rack2 user folder), then
        restart VCV Rack 2 and add **Galactic HQ → Oil Drum Kit**.
  5. **Checklist for the human** (answer each item yes/no with a note):
     - W1 The Windows build and `make install` succeeded; the module appears in the browser and its panel renders
       (labels legible, no missing jacks/knobs).
     - W2 Patch a clock/sequencer (e.g. VCV SEQ-3 gate outputs) into BD, SN1, CH; audio is heard; no crackles at the
       default engine sample rate; the CPU meter reading is acceptable to you.
     - L1 Per instrument: metallic ring and decay shape sound like the VST at the same settings (use `renders/demo_48k.wav`
       or the VST side by side).
     - L2 Velocity: a trigger scaled to ~3 V through a VCA sounds softer and duller than 10 V.
     - L3 Hi-hat: a closed hi-hat trigger chokes a ringing open hi-hat.
     - L4 Controls: Damping, Strike, Room, Tune, Hammer and Limiter each audibly change the sound; the CV inputs do too.
     - L5 Mono: with only LEFT patched, both channels' content is heard.
     - T1 Latency: the fixed 128-frame delay (2.7 ms at 48 kHz, D-003) is acceptable against the rest of your patch.
- **Questions to the human:** (1) Do W1–W2 pass? (2) Do L1–L5 pass? (3) Is the latency in T1 acceptable?
- **Allowed responses:** `proceed` | `proceed-with-rescope: <text>` | `stop`.
- **Branches:**
  - `proceed` → run S-010.
  - `proceed-with-rescope: <text>` → do not change code. Write `BLOCKED.md` quoting the text, commit, push, halt. Any
    rescope that changes behaviour (e.g. a shorter block, engine fixes, velocity handling) touches frozen tests and
    requires re-running the planning protocol from Phase 0 (Test Challenge Rule, Phase 2E.4).
  - `stop` → write `STOPPED.md` with the reason, commit, push, halt.
