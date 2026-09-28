#!/usr/bin/env bash
# Fallback Rack SDK for Linux x64 when https://vcvrack.com/downloads is unreachable (decision rule DR-01).
# Assembles Rack v2.6.6 headers + build makefiles from GitHub, third-party headers from GitHub and the Ubuntu
# archive, and a stub libRack.so (plugins only need -lRack to resolve at link time; Rack provides the real
# library at load time, C-003). Verified in the planning sandbox (research/rounds/round-1.md, spike plugin_build).
# Usage: tools/make_shim_sdk.sh <output-dir>      then: export RACK_DIR=<output-dir>
set -euo pipefail
OUT="${1:?usage: make_shim_sdk.sh <output-dir>}"
RACK_TAG=v2.6.6
RACK_COMMIT=061ccf63c1758599396ac1bb10d47345d9d34076
PFFFT_COMMIT=04cebbc5285ed8dbba4cfd96fecb3b0d9ab285ef
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
git clone -q --depth 1 --branch "$RACK_TAG" https://github.com/VCVRack/Rack "$WORK/rack"
test "$(git -C "$WORK/rack" rev-parse HEAD)" = "$RACK_COMMIT" || { echo "Rack commit mismatch" >&2; exit 2; }
for s in nanovg nanosvg oui-blendish simde; do git -C "$WORK/rack" submodule update --init --depth 1 "dep/$s" >/dev/null; done
git clone -q https://github.com/marton78/pffft "$WORK/pffft"
git -C "$WORK/pffft" checkout -q "$PFFFT_COMMIT"
( cd "$WORK" && apt-get download libglew-dev libglfw3-dev >/dev/null && for d in libglew-dev_*.deb libglfw3-dev_*.deb; do dpkg-deb -x "$d" debx; done )
mkdir -p "$OUT/dep/include/simde"
cp -r "$WORK/rack/include" "$WORK/rack/"*.mk "$WORK/rack/helper.py" "$OUT/"
cp -r "$WORK/debx/usr/include/"* "$OUT/dep/include/"
R="$WORK/rack/dep"
cp "$R/nanovg/src/"nanovg*.h "$R/nanosvg/src/nanosvg.h" "$R/oui-blendish/blendish.h" "$OUT/dep/include/"
cp -r "$R/simde/simde/"* "$OUT/dep/include/simde/"
cp "$WORK/pffft/include/pffft/pffft.h" "$OUT/dep/include/"
# jansson.h and speex/speex_resampler.h come from the system (apt: libjansson-dev libspeexdsp-dev)
cp /usr/include/jansson.h /usr/include/jansson_config.h "$OUT/dep/include/"
mkdir -p "$OUT/dep/include/speex" && cp /usr/include/speex/speexdsp_types.h /usr/include/speex/speexdsp_config_types.h /usr/include/speex/speex_resampler.h "$OUT/dep/include/speex/"
echo 'void rack_shim_stub(void){}' > "$WORK/stub.c"
gcc -shared -fPIC "$WORK/stub.c" -o "$OUT/libRack.so"
echo "shim SDK ready at $OUT (Rack $RACK_TAG @ $RACK_COMMIT)"
