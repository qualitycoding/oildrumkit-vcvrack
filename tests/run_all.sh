#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
# Runs the whole frozen suite. Exit 0 only if every T-### passes. Usage: RACK_DIR=<sdk> tests/run_all.sh
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
rc=0
make -s -C tests run || rc=1
python3 tests/test_manifest.py || rc=1
python3 tests/test_panel.py || rc=1
bash tests/test_build.sh || rc=1
bash tests/test_guards.sh || rc=1
[ $rc -eq 0 ] && echo "ALL FROZEN TESTS PASSED" || echo "FROZEN SUITE: FAILURES PRESENT"
exit $rc
