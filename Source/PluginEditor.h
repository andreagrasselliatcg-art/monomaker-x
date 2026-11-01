#pragma once

#include "PluginProcessor.h"
#include "MonoMakerLookAndFeel.h"

class MonoMakerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MonoMakerAudioProcessorEditor (MonoMakerAudioProcessor&);
    ~MonoMakerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MonoMakerAudioProcessor& processorRef;
    MonoMakerLookAndFeel lnf;

    juce::Slider widthKnob;
    juce::Slider outputKnob;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> widthAttachment;
    std::unique_ptr<Attachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoMakerAudioProcessorEditor)
};
