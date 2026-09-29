# Sources (accessed 2026-09-28)

| Tier | Source | Version / locator | Verification note |
|---|---|---|---|
| 1 | VCVRack/Rack source, https://github.com/VCVRack/Rack | tag v2.6.6 = 061ccf63; plugin.mk, compile.mk, include/dsp/digital.hpp, include/engine/Module.hpp, include/engine/Port.hpp, include/helpers.hpp, include/window/Svg.hpp, include/random.hpp, include/app/common.hpp, src/engine/Engine.cpp, src/system.cpp, src/app/MenuBar.cpp, res/ComponentLibrary, res/fonts, LICENSE.md, dep.mk | Cloned and read locally; line locators in claims.json |
| 1 | qualitycoding/oildrumkit, Source/DrumEngine.h, Voicing.h, docs/RETROSPECTIVE.md | 35fbfead61a1 | Cloned; engine compiled in all spikes |
| 1 | VCV Rack Manual — Voltage Standards, https://vcvrack.com/manual/VoltageStandards | fetched 2026-09-28 | Current thresholds 0.1 V / 1 V |
| 1 | VCV Rack Manual — Plugin Manifest, https://vcvrack.com/manual/Manifest | fetched | slug/version/tags |
| 1 | VCV Rack Manual — Module Panel Guide, https://vcvrack.com/manual/Panel | fetched | mm units, text→paths |
| 1 | VCV Rack Manual — Plugin Licensing, https://vcvrack.com/manual/PluginLicensing | fetched | exception, Component Library |
| 1 | VCV Rack Manual — Building, https://vcvrack.com/manual/Building | fetched | SDK URLs, MSYS2 packages |
| 2 | VCV manual history, https://git.kx.studio/VCVRack/manual/commit/25a6793287c7c4bb80271239378144ef83b2132b | older Voltage Standards revision | Contradiction (2 V high threshold) resolved in favour of the current page |
| 4 | https://community.vcvrack.com/t/can-the-release-of-an-adsr-influence-a-clock-bug/17118?page=2 | forum quote | Corroborates trigger levels |
| 4 | https://github.com/gbraad-dotfiles/applications/blob/main/vcvrack-sdk.md; https://github.com/apfelaudio/verilog-vcvrack (workflow main.yml @2dd2f4a) | SDK URL pattern, Rack-SDK/ top dir | Corroborate C-002 |
| 4 | https://github.com/shlabs-audio/stochast/releases | release built with SDK 2.6.6 | Corroborates C-001 currency |
| 1 | fontTools 4.60.1 (PyPI wheel sha256 8177ec96…) | — | Used in spike textpath.py |
| 1 | marton78/pffft @04cebbc5 | include/pffft/pffft.h | Header used by the shim SDK only |
