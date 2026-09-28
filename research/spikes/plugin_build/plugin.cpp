#include <rack.hpp>
#include <optional>
using namespace rack;
Plugin* pluginInstance;
static_assert(__cplusplus >= 201703L, "C++17 required");
struct M : Module { std::optional<int> o; M(){ config(1,1,1,0); configParam(0,0.f,1.f,0.5f,"x"); }
  void onSampleRateChange(const SampleRateChangeEvent& e) override {}
  void process(const ProcessArgs& a) override { outputs[0].setVoltage(inputs[0].getVoltage()); } };
struct MW : ModuleWidget { MW(M* m){ setModule(m); box.size = Vec(RACK_GRID_WIDTH*4, RACK_GRID_HEIGHT);} };
Model* modelM = createModel<M, MW>("M");
void init(Plugin* p) { pluginInstance = p; p->addModel(modelM); }
