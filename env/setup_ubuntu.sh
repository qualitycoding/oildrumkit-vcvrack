#!/usr/bin/env bash
# Implementer environment setup, Ubuntu 24.04 x86-64 (plan/ENVIRONMENT.md). Idempotent.
# Usage: bash env/setup_ubuntu.sh [SDK_DIR]      (default SDK_DIR=$HOME/rack-sdk-2.6.6)
# Prints the RACK_DIR export line on success.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
SDK_DIR="${1:-$HOME/rack-sdk-2.6.6}"
SUDO=""; [ "$(id -u)" -ne 0 ] && SUDO="sudo"
$SUDO apt-get update -q || echo "warning: apt-get update reported errors (e.g. an unrelated third-party source); continuing"
$SUDO apt-get install -y -q --no-install-recommends g++ make git jq zstd unzip curl ca-certificates python3 python3-pip binutils libjansson-dev libspeexdsp-dev
PIPX=""; python3 -m pip install --help 2>/dev/null | grep -q -- '--break-system-packages' && PIPX="--break-system-packages"
python3 -m pip install -q $PIPX --require-hashes -r env/requirements.lock
git submodule update --init engine
if [ ! -f "$SDK_DIR/plugin.mk" ]; then
  ZIP="$(mktemp -d)/Rack-SDK-2.6.6-lin-x64.zip"
  if curl -fsSL --retry 3 -o "$ZIP" https://vcvrack.com/downloads/Rack-SDK-2.6.6-lin-x64.zip; then
    SUM="$(sha256sum "$ZIP" | cut -d' ' -f1)"; LOCK=env/rack-sdk-2.6.6-lin-x64.sha256
    if [ -f "$LOCK" ]; then [ "$SUM" = "$(cat "$LOCK")" ] || { echo "Rack SDK hash mismatch (DR-02)" >&2; exit 3; }
    else echo "$SUM" > "$LOCK"; echo "recorded Rack SDK sha256 (trust on first use): $SUM"; fi
    T="$(mktemp -d)"; unzip -q "$ZIP" -d "$T"; mkdir -p "$(dirname "$SDK_DIR")"; mv "$T/Rack-SDK" "$SDK_DIR"
  else
    echo "official Rack SDK unreachable -> building shim SDK (decision rule DR-01)"
    $SUDO apt-get install -y -q --no-install-recommends dpkg-dev >/dev/null 2>&1 || true
    bash tools/make_shim_sdk.sh "$SDK_DIR"
  fi
fi
test -f "$SDK_DIR/plugin.mk"
echo "export RACK_DIR=$SDK_DIR"
