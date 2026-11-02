# Oil Drum Kit for VCV Rack 2

A physically modelled kit of struck steel oil drums (15 instruments) as one VCV Rack 2 module
(Galactic HQ, Oil Drum Kit, 20 HP). The sound engine is `DrumEngine.h` from
[qualitycoding/oildrumkit](https://github.com/qualitycoding/oildrumkit), included unchanged as the `engine/`
submodule.

> **Where the code is.** The implementation is on the **`main`** branch. This branch
> (`gen-20260928T150855Z-oildrumkit-vcv-rack-plugin`) holds the plan, research, pre-mortem and review
> records, and it is the repository's default branch, so GitHub shows this file first.

## Status

- Implemented: the module (`src/`), a Rack-independent core (`src/core/KitCore.*`), the panel, a demo
  render and the frozen test suite (`tests/`). The last recorded run on the original port
  (2026-09-30) ended with `ALL FROZEN TESTS PASSED`. Two changes were made after that run (a polyphony cap
  and a velocity-scale parameter, below), and I have not seen a test run that covers them.
- **Tested against a stand-in Rack SDK.** The official Rack SDK could not be reached when the port was built,
  so the tests ran against a shim SDK (`tools/make_shim_sdk.sh`). The module has not been confirmed to load in
  a real copy of Rack, and the Windows build and listening check (gate G-101) were handed to a human.
- Not done: no release or library submission has been made.

## Build (from `main`)

Linux (Ubuntu 24.04):

```bash
git clone --recurse-submodules https://github.com/qualitycoding/oildrumkit-vcvrack
cd oildrumkit-vcvrack
git checkout main
bash env/setup_ubuntu.sh "$HOME/rack-sdk-2.6.6"   # prints: export RACK_DIR=...
export RACK_DIR="$HOME/rack-sdk-2.6.6"
make -j"$(nproc)" && make install
```

Windows uses an MSYS2 MINGW64 shell with the Rack SDK 2.6.6; the commands are in `main`'s README.
Tests: `RACK_DIR=<sdk> bash tests/run_all.sh`.

## Controls

| Control | Type | Mapping |
|---|---|---|
| Damping | knob 0..1 (default 0.5) + CV | decay-rate scale `0.25 * 16^x`, 1.0 at centre; CV adds `cv/10` |
| Strike | knob 0..1 (default 0.5) + CV | strike position, 0.5 neutral; CV adds `cv/10` |
| Room | knob 0..1 (default 0.24) + CV | room mix `0.5 * x`; CV adds `cv/10` |
| Tune | knob -12..+12 semitones + 1 V/oct input | scales every instrument's fundamental |
| Hammer | 3-way switch | Metal / Wood / Rubber |
| Limiter | 2-way switch | soft output limiter on or off |
| Velocity Scale | parameter 0..100 % (default 100 %), **no widget on the panel** | scales trigger inputs above 8 V (see below) |
| Bass drum to Rimshot | 15 trigger inputs | Bass, Snare 1-2, Low tom 1-2, Mid tom 1-2, High tom 1-2, Splash, Ride, Closed hi-hat, Open hi-hat, Cowbell, Rimshot |
| Left / Right | outputs | stereo mix, about +/-5 V; if Right is unpatched, Left carries the mono sum |

Triggers use a Schmitt trigger (fires at 1 V or more, re-arms at 0.1 V or less). Damping, strike, hammer, tuning
and CV values are read on the frame a hit fires. The closed hi-hat chokes the open hi-hat.

## Behaviour to know about

- **Velocity** is the trigger voltage on the firing frame: 10 V is full velocity, 5 V is half. Inputs above
  8 V are treated as plain gates and are multiplied by the Velocity Scale parameter. That parameter is defined in
  the module but the panel has no knob for it (`addParam` is never called for it), so it stays at 100 % and a
  standard 10 V gate still plays at full velocity. To get dynamics, put a VCA or attenuator in front of the
  input, or use triggers of 8 V or less.
- **Polyphony cap.** Each hit is charged an estimated number of resonator modes, and a hit that would take the
  running total past 3000 is dropped. The total decays by about 1/75 per block.
- **Latency.** The engine renders in fixed 128-frame blocks, so the output is delayed by 128 frames: 2.90 ms at
  44.1 kHz, 2.67 ms at 48 kHz, 1.33 ms at 96 kHz.
- **Sample rates.** Matches the VST version up to 192 kHz. At 352.8 kHz and above the engine drives its own limiter and
  the kit sounds saturated.

## Licences

Apache-2.0 (see `LICENSE` on `main`). The engine comes from qualitycoding/oildrumkit (Apache-2.0). Panel labels are
outlined from DejaVu Sans. The knob, jack and switch graphics come from the VCV Rack Component Library, (c) VCV,
licensed CC BY-NC 4.0, and are used non-commercially with credit.
