# Oil Drum Kit for VCV Rack 2

A physically modelled kit of struck steel oil drums (15 instruments) as a single VCV Rack 2 module (Galactic HQ →
Oil Drum Kit, 20 HP). The sound engine is `DrumEngine.h` from
[qualitycoding/oildrumkit](https://github.com/qualitycoding/oildrumkit), used unchanged as the `engine/` submodule.

## Build

Linux (Ubuntu 24.04):

```bash
git clone --recurse-submodules https://github.com/qualitycoding/oildrumkit-vcvrack
cd oildrumkit-vcvrack
bash env/setup_ubuntu.sh "$HOME/rack-sdk-2.6.6"   # prints: export RACK_DIR=...
export RACK_DIR="$HOME/rack-sdk-2.6.6"
make -j"$(nproc)" && make install
```

Windows (MSYS2 MINGW64 shell):

```bash
pacman -Syu git wget make tar unzip zip mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-cmake autoconf automake libtool mingw-w64-x86_64-jq python zstd mingw-w64-x86_64-pkgconf
wget https://vcvrack.com/downloads/Rack-SDK-2.6.6-win-x64.zip && unzip -q Rack-SDK-2.6.6-win-x64.zip && export RACK_DIR="$PWD/Rack-SDK"
git clone --recurse-submodules https://github.com/qualitycoding/oildrumkit-vcvrack && cd oildrumkit-vcvrack
make -j4 && make install
```

Tests: `RACK_DIR=<sdk> bash tests/run_all.sh`.

## Controls

| Control | Type | Mapping |
|---|---|---|
| Damping | knob 0..1 (default 0.5) + Damping CV | decay-rate scale `0.25 · 16^x`, 1.0 at centre; CV adds `cv/10` |
| Strike | knob 0..1 (default 0.5) + Strike position CV | strike position, 0.5 neutral; CV adds `cv/10` |
| Room | knob 0..1 (default 0.24) + Room CV | room mix `0.5 · x`; CV adds `cv/10` |
| Tune | knob −12..+12 semitones + Tune 1 V/oct | scales every instrument's fundamental: `2^(semitones/12 + volts)` |
| Hammer | 3-way switch | Metal / Wood / Rubber |
| Limiter | 2-way switch | soft output limiter on/off |
| Bass drum … Rimshot | 15 trigger inputs | engine order: Bass, Snare 1, Snare 2, Low tom 1–2, Mid tom 1–2, High tom 1–2, Splash, Ride, Closed hi-hat, Open hi-hat, Cowbell, Rimshot |
| Left / Right | outputs | stereo mix, ±5 V scale; if Right is unpatched, Left carries the mono sum |

Trigger inputs use a Schmitt trigger (fires at ≥ 1 V, re-arms at ≤ 0.1 V). Damping, strike, hammer, tuning and CV
values are sampled on the frame a hit fires. The closed hi-hat chokes the open hi-hat.

## Latency

The engine renders in fixed 128-frame blocks, which matches the sound of a DAW host. The output is therefore delayed
by exactly 128 frames: 2.90 ms at 44.1 kHz, 2.67 ms at 48 kHz, 1.33 ms at 96 kHz and 0.67 ms at 192 kHz.

## Velocity

Velocity is the trigger voltage on the firing frame: 10 V is full velocity, 5 V is half. Standard 10 V gates
therefore always play at full velocity; put a VCA or attenuator in front of an input for dynamics.

## Sample rates

Behaviour is identical to the VST up to 192 kHz. At 352.8 kHz and above the engine drives its own limiter, so the kit
sounds saturated. Rates below 44.1 kHz are checked for numerical safety only, not for sound.

## Licences

Apache-2.0 (see `LICENSE`). The engine comes from qualitycoding/oildrumkit (Apache-2.0). Panel labels are converted
to outlines from DejaVu Sans (see `tools/fonts/DejaVuSans-LICENSE.txt`). The knob, jack and switch graphics are
the VCV Rack Component Library, © VCV, licensed CC BY-NC 4.0, and are used non-commercially with credit.
