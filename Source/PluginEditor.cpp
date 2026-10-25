#include "PluginEditor.h"

Retro80Editor::Retro80Editor (Retro80Processor& p)
    : AudioProcessorEditor (p),
      controls (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible (controls);
    addAndMakeVisible (keyboard);
    keyboard.setAvailableRange (24, 96);
    setSize (620, 420);
}

void Retro80Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1b1424));
    g.setColour (juce::Colour (0xffff4fa3));
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("RETRO 80", getLocalBounds().removeFromTop (36), juce::Justification::centred);
}

void Retro80Editor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (36);
    keyboard.setBounds (area.removeFromBottom (90));
    controls.setBounds (area.reduced (6));
}
