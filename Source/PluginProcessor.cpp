#include "PluginProcessor.h"
#include "PluginEditor.h"

MonoMakerAudioProcessor::MonoMakerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MonoMakerX", createParameterLayout())
{
    widthParam  = apvts.getRawParameterValue (ParamIDs::width);
    outputParam = apvts.getRawParameterValue (ParamIDs::output);
}

juce::AudioProcessorValueTreeState::ParameterLayout MonoMakerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Stereo width: 0 % = total mono, 100 % = full (original) stereo.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::width, 1 },
        "Stereo Width",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("%")
            .withStringFromValueFunction ([] (float v, int)
            {
                if (v <= 0.05f)  return juce::String ("Mono");
                if (v >= 99.95f) return juce::String ("Stereo");
                return juce::String (v, 1) + " %";
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::output, 1 },
        "Output",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int)
            {
                return (v > 0.0f ? "+" : "") + juce::String (v, 1) + " dB";
            })));

    return { params.begin(), params.end() };
}

bool MonoMakerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

void MonoMakerAudioProcessor::prepareToPlay (double sampleRate, int)
{
    // Short ramps keep automation click-free while still feeling instant.
    widthSmoothed.reset (sampleRate, 0.03);
    gainSmoothed.reset  (sampleRate, 0.03);

    widthSmoothed.setCurrentAndTargetValue (widthParam->load() * 0.01f);
    gainSmoothed.setCurrentAndTargetValue  (juce::Decibels::decibelsToGain (outputParam->load()));
}

void MonoMakerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2)
        return;

    widthSmoothed.setTargetValue (widthParam->load() * 0.01f);
    gainSmoothed.setTargetValue  (juce::Decibels::decibelsToGain (outputParam->load()));

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);
    const int numSamples = buffer.getNumSamples();

    // Mid/Side width: the Mid stays untouched, the Side is scaled by width.
    // width 0 -> L = R = (L+R)/2 (total mono), width 1 -> original stereo image.
    for (int i = 0; i < numSamples; ++i)
    {
        const float w = widthSmoothed.getNextValue();
        const float g = gainSmoothed.getNextValue();

        const float mid  = 0.5f * (left[i] + right[i]);
        const float side = 0.5f * (left[i] - right[i]) * w;

        left[i]  = (mid + side) * g;
        right[i] = (mid - side) * g;
    }
}

juce::AudioProcessorEditor* MonoMakerAudioProcessor::createEditor()
{
    return new MonoMakerAudioProcessorEditor (*this);
}

void MonoMakerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MonoMakerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MonoMakerAudioProcessor();
}
