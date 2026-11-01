#include "PluginEditor.h"

namespace
{
    constexpr int editorW = 440;
    constexpr int editorH = 520;
    const juce::Rectangle<int> mainKnobArea   { 70, 105, 300, 300 };
    const juce::Rectangle<int> outputKnobArea { editorW - 108, editorH - 118, 76, 76 };

    constexpr float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
    constexpr float rotaryEnd   = juce::MathConstants<float>::pi * 2.75f;
}

MonoMakerAudioProcessorEditor::MonoMakerAudioProcessorEditor (MonoMakerAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lnf);

    for (auto* s : { &widthKnob, &outputKnob })
    {
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s->setRotaryParameters (rotaryStart, rotaryEnd, true);
        s->onValueChange = [this] { repaint(); };
        addAndMakeVisible (*s);
    }

    widthKnob.getProperties().set ("knobStyle", "main");
    widthKnob.setMouseDragSensitivity (300);
    widthKnob.setDoubleClickReturnValue (true, 100.0);
    widthKnob.setPopupDisplayEnabled (false, false, this);

    outputKnob.getProperties().set ("knobStyle", "small");
    outputKnob.setDoubleClickReturnValue (true, 0.0);

    widthAttachment  = std::make_unique<Attachment> (processorRef.apvts, ParamIDs::width,  widthKnob);
    outputAttachment = std::make_unique<Attachment> (processorRef.apvts, ParamIDs::output, outputKnob);

    setSize (editorW, editorH);
}

MonoMakerAudioProcessorEditor::~MonoMakerAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MonoMakerAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // ---- Background: dark grey to black, with fine brushed lines ----------
    g.setGradientFill (juce::ColourGradient (Palette::bgTop, 0.0f, 0.0f,
                                             Palette::bgBottom, 0.0f, bounds.getHeight(), false));
    g.fillAll();

    g.setColour (juce::Colours::white.withAlpha (0.012f));
    for (int yy = 0; yy < getHeight(); yy += 3)
        g.drawHorizontalLine (yy, 0.0f, bounds.getWidth());

    // Inner frame
    g.setColour (Palette::panelLine);
    g.drawRoundedRectangle (bounds.reduced (8.0f), 10.0f, 1.0f);

    // ---- Header -------------------------------------------------------------
    const float widthNorm = (float) widthKnob.valueToProportionOfLength (widthKnob.getValue());
    const auto  accent    = MonoMakerLookAndFeel::ledColourAt (widthNorm);

    g.setColour (Palette::textMain);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("MONO MAKER", juce::Rectangle<int> (0, 26, editorW, 34), juce::Justification::centred);

    // "- X" badge
    g.setColour (accent);
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    auto badge = juce::Rectangle<float> (editorW * 0.5f - 18.0f, 62.0f, 36.0f, 17.0f);
    g.drawRoundedRectangle (badge, 3.0f, 1.2f);
    g.drawText ("- X", badge, juce::Justification::centred);

    // ---- Main knob labels -------------------------------------------------
    const auto kc = mainKnobArea.toFloat().getCentre();
    const float kr = mainKnobArea.getWidth() * 0.5f;
    const auto startPt = kc.getPointOnCircumference (kr * 1.02f, rotaryStart);
    const auto endPt   = kc.getPointOnCircumference (kr * 1.02f, rotaryEnd);

    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.setColour (Palette::monoColour.withAlpha (widthNorm < 0.5f ? 1.0f : 0.55f));
    g.drawText ("TOTAL MONO", juce::Rectangle<float> (startPt.x - 60.0f, startPt.y + 6.0f, 90.0f, 16.0f),
                juce::Justification::centredRight);
    g.setColour (Palette::stereoColour.withAlpha (widthNorm >= 0.5f ? 1.0f : 0.55f));
    g.drawText ("FULL STEREO", juce::Rectangle<float> (endPt.x - 30.0f, endPt.y + 6.0f, 96.0f, 16.0f),
                juce::Justification::centredLeft);

    // ---- Width readout ------------------------------------------------------
    const double w = widthKnob.getValue();
    juce::String readout = w <= 0.05  ? "MONO"
                         : w >= 99.95 ? "STEREO"
                                      : juce::String (w, 1) + " %";

    auto readoutBox = juce::Rectangle<float> (editorW * 0.5f - 70.0f, 412.0f, 140.0f, 30.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (readoutBox, 5.0f);
    g.setColour (Palette::panelLine);
    g.drawRoundedRectangle (readoutBox, 5.0f, 1.0f);
    g.setColour (accent);
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 17.0f, juce::Font::bold));
    g.drawText (readout, readoutBox, juce::Justification::centred);

    g.setColour (Palette::textDim);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText ("STEREO WIDTH", juce::Rectangle<int> (0, 446, editorW, 14), juce::Justification::centred);

    // ---- Output knob labels -----------------------------------------------
    g.setColour (Palette::textDim);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText ("OUTPUT", outputKnobArea.withY (outputKnobArea.getY() - 18).withHeight (14),
                juce::Justification::centred);

    const double db = outputKnob.getValue();
    g.setColour (Palette::textMain);
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::plain));
    g.drawText ((db > 0.0 ? "+" : "") + juce::String (db, 1) + " dB",
                outputKnobArea.withY (outputKnobArea.getBottom() + 2).withHeight (14).expanded (10, 0),
                juce::Justification::centred);

    // ---- Manufacturer -------------------------------------------------------
    g.setColour (Palette::textDim);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("TUNE TALKS", juce::Rectangle<int> (28, editorH - 44, 160, 18), juce::Justification::centredLeft);
    g.setColour (Palette::textDim.withAlpha (0.6f));
    g.setFont (juce::FontOptions (9.5f));
    g.drawText ("v1.0", juce::Rectangle<int> (28, editorH - 28, 160, 12), juce::Justification::centredLeft);
}

void MonoMakerAudioProcessorEditor::resized()
{
    widthKnob.setBounds (mainKnobArea);
    outputKnob.setBounds (outputKnobArea);
}
