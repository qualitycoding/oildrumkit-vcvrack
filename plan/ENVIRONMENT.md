# Environment (pinned and verified)

Verified in the planning sandbox on 2026-09-28 by running `bash env/setup_ubuntu.sh /tmp/sdk-verify` (exit 0; the
official SDK host was blocked there, so the DR-01 shim path ran) and then building the oracle plugin with it
(T-052/T-053 PASS). The official-SDK branch of the script could not be exercised in the sandbox (R-006).

## Implementer platform (Linux, agent)

| Component | Pinned version | Notes |
|---|---|---|
| OS | Ubuntu 24.04 LTS x86-64 (verified on 24.04.4) | CPU must support `-march=nehalem` (any x86-64-v2) |
| Compiler | g++ 13.3.0 (`13.3.0-6ubuntu2~24.04.1`) | Ubuntu 24.04 default `g++` package |
| GNU Make | 4.3 | |
| git | 2.43.0 | submodules required |
| Python | 3.12.3 | |
| fontTools | 4.60.1, hash-locked in `env/requirements.lock` | panel generator (D-013) |
| jq | 1.7.1-3ubuntu0.24.04.2 | used by Rack `plugin.mk` |
| zstd | 1.5.5+dfsg2-2build1.1 | used by `make dist` and T-053 |
| binutils (`nm`) | 2.42 | T-052 |
| libjansson-dev | 2.14-2build2 | shim SDK only |
| libspeexdsp-dev | 1.2.1-1ubuntu3 | shim SDK only |
| libglew-dev / libglfw3-dev | 2.2.0-4build1 / 3.3.10-1build1 (headers via `apt-get download`) | shim SDK only |
| Rack SDK | 2.6.6 lin-x64, official zip; SHA-256 recorded on first use in `env/rack-sdk-2.6.6-lin-x64.sha256` | D-018 |
| Rack source (shim) | tag v2.6.6 = `061ccf63c1758599396ac1bb10d47345d9d34076` | `tools/make_shim_sdk.sh` |
| pffft header (shim) | `github.com/marton78/pffft@04cebbc5285ed8dbba4cfd96fecb3b0d9ab285ef` | API-compatible header for `dsp/fft.hpp` |
| Engine | `qualitycoding/oildrumkit@35fbfead61a11a8c7f5609a95831a29f2cb2bf78` (submodule `engine/`) | read-only |
| Panel font | DejaVuSans.ttf sha256 `7da195a74c55bef988d0d48f9508bd5d849425c1770dba5d7bfc6ce9ed848954` | from Rack v2.6.6 `res/fonts/` |

Compiler flags are Rack's (`-O3 -funsafe-math-optimizations -fno-omit-frame-pointer -march=nehalem`) plus
`-std=c++17`; the C++ tests use the same flags (`tests/Makefile`) and set Rack's FTZ/DAZ mode (C-017).

## Setup commands (literal)

```bash
git clone https://github.com/qualitycoding/oildrumkit-vcvrack
cd oildrumkit-vcvrack
git checkout gen-20260928T150855Z-oildrumkit-vcv-rack-plugin
git checkout -b impl/vcv-port
bash env/setup_ubuntu.sh "$HOME/rack-sdk-2.6.6"     # prints: export RACK_DIR=...
export RACK_DIR="$HOME/rack-sdk-2.6.6"
sha256sum -c tests/FROZEN_MANIFEST.sha256           # must print OK for every line
make -s -C tests all                                 # C++ tests compile against the stub (all FAIL when run)
```

## Human platform (Windows, G-101 only)

MSYS2 MINGW64 shell with the package list from the Rack Building manual (C-022), Rack SDK 2.6.6 win-x64, VCV Rack 2.6.x
installed. Literal commands are in `plan/GATES.md` (G-101).
