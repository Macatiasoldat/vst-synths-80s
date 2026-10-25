#pragma once

#include "dsp/Presets.h"
#include "dsp/SynthEngine.h"
#include <juce_audio_utils/juce_audio_utils.h>

class Retro80Processor : public juce::AudioProcessor
{
public:
    Retro80Processor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Retro80"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    // Les presets d'usine sont exposés comme « programmes » au DAW.
    int getNumPrograms() override { return (int) bank.size(); }
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    juce::MidiKeyboardState keyboardState; // clavier à l'écran

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout (const std::vector<retro::Patch>& bank);
    void handleMidi (const juce::MidiMessage& m);
    void syncParameters();

    std::vector<retro::Patch> bank;
    retro::SynthEngine synth;
    int loadedPreset = -1;

    std::atomic<float>* presetParam = nullptr;
    std::atomic<float>* brightnessParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* chorusParam = nullptr;
    std::atomic<float>* driftParam = nullptr;
    std::atomic<float>* volumeParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Retro80Processor)
};
