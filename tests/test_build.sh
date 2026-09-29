#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
# T-052 plugin build (Linux x64) and T-053 distributable package (C-001, C-003, C-008; D-008, D-014).
# Requires RACK_DIR (official Rack SDK 2.6.6 lin-x64, or tools/make_shim_sdk.sh fallback), jq, zstd, nm.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
pass=0; total=2
fail() { echo "FAIL $1: $2"; }
if [ -z "${RACK_DIR:-}" ] || [ ! -f "${RACK_DIR}/plugin.mk" ]; then
  fail "T-052 plugin build" "RACK_DIR unset or has no plugin.mk"; fail "T-053 dist package" "RACK_DIR unset"
  echo "SUMMARY 0/$total passed"; exit 1
fi
if [ ! -f "$ROOT/Makefile" ]; then
  fail "T-052 plugin build" "Makefile missing"; fail "T-053 dist package" "Makefile missing"; echo "SUMMARY 0/$total passed"; exit 1
fi
cd "$ROOT"
LOG="$(mktemp)"; trap 'rm -f "$LOG"' EXIT
make -s clean >/dev/null 2>&1 || true
if make RACK_DIR="$RACK_DIR" >"$LOG" 2>&1; then
  why=""
  grep -q -- '-std=c++17' "$LOG" || why="$why; compile commands lack -std=c++17 (see D-008)"
  [ -f plugin.so ] || why="$why; plugin.so not produced"
  if [ -f plugin.so ]; then
    nm -D --defined-only plugin.so | grep -Eq ' T init$' || why="$why; plugin.so does not export init"
    nm -DC --undefined-only plugin.so | grep -q 'rack::' || why="$why; plugin.so does not reference the Rack API"
  fi
  grep -q 'engine/Source/DrumEngine.h' <(find build -name '*.d' -exec cat {} + 2>/dev/null) || why="$why; engine header not used from engine/ submodule"
  if [ -z "$why" ]; then echo "PASS T-052 plugin build"; pass=$((pass+1)); else fail "T-052 plugin build" "${why#; }"; fi
else
  fail "T-052 plugin build" "make failed: $(grep -m3 -E 'error' "$LOG" | tr '\n' ' ')"
fi
VERSION="$(jq -r .version plugin.json 2>/dev/null || echo '?')"
PKG="dist/OilDrumKit-${VERSION}-lin-x64.vcvplugin"
if make RACK_DIR="$RACK_DIR" dist >"$LOG" 2>&1 && [ -f "$PKG" ]; then
  LIST="$(zstd -dc "$PKG" | tar -t 2>/dev/null)"
  why=""
  for f in OilDrumKit/plugin.so OilDrumKit/plugin.json OilDrumKit/res/OilDrumKit.svg OilDrumKit/LICENSE; do
    echo "$LIST" | grep -qx "$f" || why="$why; $f missing from package"
  done
  if [ -z "$why" ]; then echo "PASS T-053 dist package"; pass=$((pass+1)); else fail "T-053 dist package" "${why#; }"; fi
else
  fail "T-053 dist package" "make dist failed or $PKG missing"
fi
echo "SUMMARY $pass/$total passed"; [ "$pass" -eq "$total" ]
