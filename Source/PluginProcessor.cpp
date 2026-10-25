#include "PluginProcessor.h"
#include "PluginEditor.h"

Retro80Processor::Retro80Processor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Retro80", createLayout (retro::factoryPresets())),
      bank (retro::factoryPresets())
{
    presetParam = parameters.getRawParameterValue ("preset");
    brightnessParam = parameters.getRawParameterValue ("brightness");
    resonanceParam = parameters.getRawParameterValue ("resonance");
    attackParam = parameters.getRawParameterValue ("attack");
    releaseParam = parameters.getRawParameterValue ("release");
    chorusParam = parameters.getRawParameterValue ("chorus");
    driftParam = parameters.getRawParameterValue ("drift");
    volumeParam = parameters.getRawParameterValue ("volume");
}

juce::AudioProcessorValueTreeState::ParameterLayout Retro80Processor::createLayout (const std::vector<retro::Patch>& bank)
{
    using namespace juce;
    StringArray names;
    for (auto& p : bank)
        names.add (p.name);

    AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "preset", 1 }, "Preset", names, 0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "brightness", 1 }, "Brillance",
                                                       NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "resonance", 1 }, "Resonance",
                                                       NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "attack", 1 }, "Attaque (x)",
                                                       NormalisableRange<float> (0.1f, 10.0f, 0.0f, 0.3f), 1.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "release", 1 }, "Release (x)",
                                                       NormalisableRange<float> (0.1f, 10.0f, 0.0f, 0.3f), 1.0f));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "chorus", 1 }, "Chorus",
                                                        StringArray { "Preset", "Off", "I", "II", "I+II" }, 0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "drift", 1 }, "Derive analogique (x)",
                                                       NormalisableRange<float> (0.0f, 3.0f), 1.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "volume", 1 }, "Volume (dB)",
                                                       NormalisableRange<float> (-24.0f, 6.0f), 0.0f));
    return layout;
}

void Retro80Processor::prepareToPlay (double sampleRate, int)
{
    synth.prepare (sampleRate);
    loadedPreset = -1;
    keyboardState.reset();
    syncParameters();
}

bool Retro80Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void Retro80Processor::syncParameters()
{
    const int index = juce::jlimit (0, (int) bank.size() - 1, (int) presetParam->load());
    if (index != loadedPreset)
    {
        synth.setPatch (bank[(size_t) index]);
        loadedPreset = index;
    }

    retro::Macros m;
    m.brightness = brightnessParam->load();
    m.resonance = resonanceParam->load();
    m.attackScale = attackParam->load();
    m.releaseScale = releaseParam->load();
    m.chorus = (int) chorusParam->load() - 1; // 0 = « Preset » -> -1
    m.driftScale = driftParam->load();
    m.volume = juce::Decibels::decibelsToGain (volumeParam->load());
    synth.setMacros (m);
}

void Retro80Processor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        synth.noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        synth.noteOff (m.getNoteNumber());
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        synth.allNotesOff (m.isAllSoundOff());
    else if (m.isPitchWheel())
        synth.setPitchBend ((float) (m.getPitchWheelValue() - 8192) / 8192.0f * 2.0f);
}

void Retro80Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
    syncParameters();

    // Les deux canaux sont toujours rendus ; en mono on ne garde que la gauche.
    float* left = buffer.getWritePointer (0);
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    juce::HeapBlock<float> scratch;
    if (right == nullptr)
    {
        scratch.allocate ((size_t) numSamples, false);
        right = scratch.get();
    }

    // Rendu découpé aux positions des événements MIDI (précision à l'échantillon).
    int pos = 0;
    for (const auto meta : midi)
    {
        const int eventPos = juce::jlimit (0, numSamples, meta.samplePosition);
        if (eventPos > pos)
        {
            synth.render (left + pos, right + pos, eventPos - pos);
            pos = eventPos;
        }
        handleMidi (meta.getMessage());
    }
    if (pos < numSamples)
        synth.render (left + pos, right + pos, numSamples - pos);

    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

int Retro80Processor::getCurrentProgram()
{
    return (int) presetParam->load();
}

void Retro80Processor::setCurrentProgram (int index)
{
    if (auto* p = parameters.getParameter ("preset"))
        p->setValueNotifyingHost (p->convertTo0to1 ((float) index));
}

const juce::String Retro80Processor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, (int) bank.size()) ? juce::String (bank[(size_t) index].name) : juce::String();
}

void Retro80Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void Retro80Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* Retro80Processor::createEditor()
{
    return new Retro80Editor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Retro80Processor();
}
