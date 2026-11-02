# GATE-G-LISTEN — VST/Rack implementation equivalence (blind listening)

**Status:** BLOCKED — reference/candidate render bundles not yet produced
(round-001 not scored).
**Branch:** `impl/vcv-listening` (base `main` @ 61ba36650251b01d4d933598ccceb2dfa5e7f5e8;
engine submodule 35fbfead61a11a8c7f5609a95831a29f2cb2bf78, pinned and
initialized).
**Decisions:** D-022 (listening amendment), D-023 (vendored pair tools).
**Frozen seed:** 20261106 (rounds +1000/+2000).

## Boundary
Post-G-101 validation amendment. Historical G-101 (human `proceed`) is
preserved and not reopened; this blind test happens **after** publication and
is not claimed to have happened before it. No new release, tag or VCV
Library submission is authorized. A pass here supports VST/Rack equivalence
only — physical-drum realism is oildrumkit's separate G-LISTEN.

## Validation re-run (2026-11-03, candidate 61ba36650251)
- `RACK_DIR=<shim> bash tests/run_all.sh` → **ALL FROZEN TESTS PASSED**
  (shim SDK manually assembled from pinned Rack v2.6.6 codeload tarballs).
- T-040 dense-roll ratio 0.616 (≤ 1.15); T-041 25.0x real time (≥ 10x).
- T-033 engine submodule pinned and unmodified.

## Frozen settings
12 matched instrument/event onset pairs, `--window start`, 1.0 s clips,
common RMS −23 dBFS, 10 ms fades, 44.1 kHz PCM-24 WAV; pass = ≤ 8/12
correct AND no unresolved equivalence finding, **in addition to** T-010/T-012
and the full frozen suite; max three rounds. The fixed 128-frame latency
(2.7 ms @ 48 kHz, D-003) is accounted for explicitly before windowing.
Builder/scorer vendored from qualitycoding/skills @
7915c7b7e0cecbee5116e72a4cf5699383df5983 (sha256 3c4f75854e2a…d6824c7 /
f2d9ea5c76a1…e49defa).

## Questions for the human listener
1. Which side is the VST (per pair)?
2. Any instrument differing in attack, ring/decay, velocity/timbre, choke,
   control/CV response, stereo/mono behaviour, latency, crackles, or reset
   behaviour? Preserve the existing Windows/Rack checklist (G-101 W1–W2,
   L1–L5, T1) as host evidence — it is not replaced by the blind score.

## Allowed responses
`proceed` → close supplementary G-LISTEN, resume the amended S-RETRO/close
sequence; do not rerun S-010 or create another tag.
`iterate: <notes>` → correct wrapper scheduling/block/latency/parameter
behaviour or another general divergence; rerun frozen suite/reviews; new
seed; engine submodule unchanged unless separately authorized.
`proceed-with-rescope: <text>` → record narrowed support/equivalence claim;
behaviour-changing rescope follows the existing re-planning rule.
`stop` → halt at G-LISTEN with the close sequence pending; no external
release action.

## Current state
- Round-001 record: `listening/G-LISTEN/round-001/result.json`
  (status `blocked`; P-4/P-5 render bundles missing).
