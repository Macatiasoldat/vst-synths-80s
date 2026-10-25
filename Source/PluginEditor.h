#pragma once

#include "PluginProcessor.h"

// Interface de prototype : réglages générés automatiquement + clavier à l'écran.
// Une vraie interface (une « face » par machine) viendra plus tard.
class Retro80Editor : public juce::AudioProcessorEditor
{
public:
    explicit Retro80Editor (Retro80Processor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::GenericAudioProcessorEditor controls;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Retro80Editor)
};
