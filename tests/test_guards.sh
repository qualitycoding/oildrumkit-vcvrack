#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
# Guard checks (plan/DECISIONS.md D-015: invariants that already hold at freeze time, so they are green
# from the start rather than red): T-031 no file/network/process APIs in src/ (threat model A-012),
# T-032 no credentials in tracked files, T-033 engine submodule pinned and unmodified (C-021),
# T-034 frozen manifest intact.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
pass=0; total=4
r() { if [ -z "$3" ]; then echo "PASS $1 $2"; pass=$((pass+1)); else echo "FAIL $1 $2: $3"; fi; }
hits="$(grep -rnE '\b(fopen|freopen|ifstream|ofstream|fstream|popen|system\s*\(|exec[lv]p?\s*\(|socket\s*\(|curl_|network::|std::filesystem)' src 2>/dev/null | head -5)"
r "T-031" "no file/network/process APIs in src/" "$hits"
pat='(github_pat_[A-Za-z0-9_]{30,}|gh[pousr]_[A-Za-z0-9]{30,}|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY-----|xox[baprs]-[A-Za-z0-9-]{10,})'
sec="$(git grep -nE "$pat" -- . ':!engine' 2>/dev/null | cut -c1-80 | head -5)"
r "T-032" "no credentials in tracked files" "$sec"
PIN=35fbfead61a11a8c7f5609a95831a29f2cb2bf78
st="$(git submodule status engine 2>/dev/null)"; why=""
case "$st" in " $PIN engine"*) ;; *) why="engine submodule not at $PIN (status: '$st')";; esac
[ -z "$(git -C engine status --porcelain 2>/dev/null)" ] || why="$why engine working tree modified"
r "T-033" "engine submodule pinned and unmodified" "$why"
out="$(sha256sum --quiet -c tests/FROZEN_MANIFEST.sha256 2>&1)"
r "T-034" "frozen manifest intact" "$out"
echo "SUMMARY $pass/$total passed"; [ "$pass" -eq "$total" ]
