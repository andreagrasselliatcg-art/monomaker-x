#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Palette
{
    const juce::Colour bgTop      { 0xff1c1c20 };
    const juce::Colour bgBottom   { 0xff0b0b0d };
    const juce::Colour panelLine  { 0xff2a2a30 };
    const juce::Colour textMain   { 0xffe8e8ec };
    const juce::Colour textDim    { 0xff7a7a84 };
    const juce::Colour ledOff     { 0xff222228 };
    const juce::Colour monoColour   { 0xffff7a1a };   // amber  = mono
    const juce::Colour stereoColour { 0xff19e3ff };   // cyan   = stereo
}

/** Custom look: black / dark-grey knobs, square-LED ring on the main knob. */
class MonoMakerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MonoMakerLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    static juce::Colour ledColourAt (float proportion);

private:
    void drawMainKnob  (juce::Graphics&, juce::Rectangle<float>, float pos, float start, float end);
    void drawSmallKnob (juce::Graphics&, juce::Rectangle<float>, float pos, float start, float end);
    static void drawKnobBody (juce::Graphics&, juce::Point<float> centre, float radius);
};
