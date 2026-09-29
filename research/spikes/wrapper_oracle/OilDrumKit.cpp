// SPIKE oracle of the D-011 Rack wrapper: proves T-052/T-053 are satisfiable with the shim SDK.
#include "plugin.hpp"
#include "KitCore.hpp"
#include "Layout.hpp"

static const char* const kNames[odk::kNumInstruments] = { "Bass drum", "Snare 1", "Snare 2", "Low tom 1", "Low tom 2",
    "Mid tom 1", "Mid tom 2", "High tom 1", "High tom 2", "Splash", "Ride", "Closed hi-hat", "Open hi-hat", "Cowbell", "Rimshot" };

struct OilDrumKit : Module
{
    enum ParamId { DAMP_PARAM, STRIKE_PARAM, ROOM_PARAM, TUNE_PARAM, HAMMER_PARAM, LIMITER_PARAM, PARAMS_LEN };
    enum InputId { ENUMS (TRIG_INPUT, odk::kNumInstruments), DAMP_CV_INPUT, STRIKE_CV_INPUT, ROOM_CV_INPUT, VOCT_INPUT, INPUTS_LEN };
    enum OutputId { LEFT_OUTPUT, RIGHT_OUTPUT, OUTPUTS_LEN };
    odk::KitCore core;
    OilDrumKit()
    {
        config (PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, 0);
        configParam (DAMP_PARAM, 0.f, 1.f, 0.5f, "Damping");
        configParam (STRIKE_PARAM, 0.f, 1.f, 0.5f, "Strike position");
        configParam (ROOM_PARAM, 0.f, 1.f, 0.24f, "Room", "%", 0.f, 100.f);
        configParam (TUNE_PARAM, -12.f, 12.f, 0.f, "Tune", " semitones");
        configSwitch (HAMMER_PARAM, 0.f, 2.f, 1.f, "Hammer", { "Metal", "Wood", "Rubber" });
        configSwitch (LIMITER_PARAM, 0.f, 1.f, 1.f, "Limiter", { "Off", "On" });
        for (int i = 0; i < odk::kNumInstruments; ++i) configInput (TRIG_INPUT + i, std::string (kNames[i]) + " trigger");
        configInput (DAMP_CV_INPUT, "Damping CV"); configInput (STRIKE_CV_INPUT, "Strike position CV");
        configInput (ROOM_CV_INPUT, "Room CV"); configInput (VOCT_INPUT, "Tune 1 V/oct");
        configOutput (LEFT_OUTPUT, "Left (mono sum if Right unpatched)"); configOutput (RIGHT_OUTPUT, "Right");
        core.seed (random::u32());
    }
    void onSampleRateChange (const SampleRateChangeEvent& e) override { core.setSampleRate (e.sampleRate); }
    void onReset (const ResetEvent& e) override { Module::onReset (e); core.reset(); }
    void process (const ProcessArgs& args) override
    {
        odk::Controls c;
        c.damping = params[DAMP_PARAM].getValue(); c.strike = params[STRIKE_PARAM].getValue();
        c.room = params[ROOM_PARAM].getValue(); c.tune = params[TUNE_PARAM].getValue();
        c.hammer = (int) std::round (params[HAMMER_PARAM].getValue()); c.limiter = params[LIMITER_PARAM].getValue() > 0.5f;
        core.setControls (c);
        odk::FrameIn in;
        for (int i = 0; i < odk::kNumInstruments; ++i) in.trig[i] = inputs[TRIG_INPUT + i].getVoltage();
        in.dampingCv = inputs[DAMP_CV_INPUT].getVoltage(); in.strikeCv = inputs[STRIKE_CV_INPUT].getVoltage();
        in.roomCv = inputs[ROOM_CV_INPUT].getVoltage(); in.voct = inputs[VOCT_INPUT].getVoltage();
        const odk::FrameOut o = odk::monoFold (core.processFrame (in), outputs[RIGHT_OUTPUT].isConnected());
        outputs[LEFT_OUTPUT].setVoltage (o.left); outputs[RIGHT_OUTPUT].setVoltage (o.right);
    }
};

struct OilDrumKitWidget : ModuleWidget
{
    OilDrumKitWidget (OilDrumKit* m)
    {
        using namespace odk::layout;
        setModule (m);
        setPanel (createPanel (asset::plugin (pluginInstance, "res/OilDrumKit.svg")));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        auto p = [] (Pos q) { return mm2px (Vec (q.x, q.y)); };
        addParam (createParamCentered<RoundBlackKnob> (p (DAMP), m, OilDrumKit::DAMP_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (p (STRIKE), m, OilDrumKit::STRIKE_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (p (ROOM), m, OilDrumKit::ROOM_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (p (TUNE), m, OilDrumKit::TUNE_PARAM));
        addParam (createParamCentered<CKSSThree> (p (HAMMER), m, OilDrumKit::HAMMER_PARAM));
        addParam (createParamCentered<CKSS> (p (LIMITER), m, OilDrumKit::LIMITER_PARAM));
        const Pos trig[odk::kNumInstruments] = { TRIG_0, TRIG_1, TRIG_2, TRIG_3, TRIG_4, TRIG_5, TRIG_6, TRIG_7, TRIG_8, TRIG_9, TRIG_10, TRIG_11, TRIG_12, TRIG_13, TRIG_14 };
        for (int i = 0; i < odk::kNumInstruments; ++i) addInput (createInputCentered<PJ301MPort> (p (trig[i]), m, OilDrumKit::TRIG_INPUT + i));
        addInput (createInputCentered<PJ301MPort> (p (DAMP_CV), m, OilDrumKit::DAMP_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (p (STRIKE_CV), m, OilDrumKit::STRIKE_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (p (ROOM_CV), m, OilDrumKit::ROOM_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (p (VOCT), m, OilDrumKit::VOCT_INPUT));
        addOutput (createOutputCentered<PJ301MPort> (p (LEFT), m, OilDrumKit::LEFT_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (p (RIGHT), m, OilDrumKit::RIGHT_OUTPUT));
    }
};

Model* modelOilDrumKit = createModel<OilDrumKit, OilDrumKitWidget> ("OilDrumKit");
