#include "MonoMakerLookAndFeel.h"

MonoMakerLookAndFeel::MonoMakerLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,    Palette::textMain);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

juce::Colour MonoMakerLookAndFeel::ledColourAt (float p)
{
    // Sweep the hue backwards (amber -> red -> magenta -> violet -> blue -> cyan)
    // so every LED stays fully saturated instead of going muddy in the middle.
    p = juce::jlimit (0.0f, 1.0f, p);
    const float hMono   = Palette::monoColour.getHue();
    const float hStereo = Palette::stereoColour.getHue() - 1.0f;
    float h = hMono + (hStereo - hMono) * p;
    if (h < 0.0f) h += 1.0f;
    return juce::Colour::fromHSV (h, 0.88f, 1.0f, 1.0f);
}

void MonoMakerLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                             float pos, float start, float end, juce::Slider& s)
{
    auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat();

    if (s.getProperties()["knobStyle"].toString() == "main")
        drawMainKnob (g, bounds, pos, start, end);
    else
        drawSmallKnob (g, bounds, pos, start, end);
}

void MonoMakerLookAndFeel::drawKnobBody (juce::Graphics& g, juce::Point<float> c, float r)
{
    // Drop shadow
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (c.x - r * 1.04f, c.y - r * 1.0f, r * 2.08f, r * 2.14f);

    // Outer bezel
    juce::ColourGradient bezel (juce::Colour (0xff3a3a40), c.x, c.y - r,
                                juce::Colour (0xff0a0a0c), c.x, c.y + r, false);
    g.setGradientFill (bezel);
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);

    // Knob cap (matte black / dark grey)
    const float capR = r * 0.88f;
    juce::ColourGradient cap (juce::Colour (0xff2b2b30), c.x - capR * 0.4f, c.y - capR * 0.6f,
                              juce::Colour (0xff111114), c.x + capR * 0.5f, c.y + capR, true);
    g.setGradientFill (cap);
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.0f, capR * 2.0f);

    // Subtle concentric machining rings
    g.setColour (juce::Colours::white.withAlpha (0.025f));
    for (float k = 0.3f; k < 0.86f; k += 0.08f)
        g.drawEllipse (c.x - capR * k, c.y - capR * k, capR * 2.0f * k, capR * 2.0f * k, 1.0f);

    // Rim highlight
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawEllipse (c.x - capR, c.y - capR, capR * 2.0f, capR * 2.0f, 1.2f);
}

void MonoMakerLookAndFeel::drawMainKnob (juce::Graphics& g, juce::Rectangle<float> bounds,
                                         float pos, float start, float end)
{
    const auto  c      = bounds.getCentre();
    const float outerR = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // ---- Square LED ring ---------------------------------------------------
    constexpr int numLeds = 41;
    const float ledRadius = outerR * 0.90f;
    const float ledSize   = outerR * 0.075f;
    const float litUpTo   = pos * (float) (numLeds - 1);

    for (int i = 0; i < numLeds; ++i)
    {
        const float p     = (float) i / (float) (numLeds - 1);
        const float angle = start + p * (end - start);
        const bool  lit   = (float) i <= litUpTo + 0.001f;

        const auto  ledCentre = c.getPointOnCircumference (ledRadius, angle);
        const auto  t = juce::AffineTransform::rotation (angle, ledCentre.x, ledCentre.y);

        auto square = juce::Rectangle<float> (ledSize, ledSize).withCentre (ledCentre);

        if (lit)
        {
            const auto col = ledColourAt (p);

            // Glow
            for (int k = 3; k >= 1; --k)
            {
                juce::Path glow;
                glow.addRoundedRectangle (square.expanded (ledSize * 0.28f * (float) k), ledSize * 0.3f);
                g.setColour (col.withAlpha (0.07f));
                g.fillPath (glow, t);
            }

            juce::Path led;
            led.addRoundedRectangle (square, ledSize * 0.12f);
            g.setColour (col);
            g.fillPath (led, t);

            juce::Path hot;
            hot.addRoundedRectangle (square.reduced (ledSize * 0.28f), ledSize * 0.08f);
            g.setColour (col.brighter (0.8f).withAlpha (0.7f));
            g.fillPath (hot, t);
        }
        else
        {
            juce::Path led;
            led.addRoundedRectangle (square, ledSize * 0.12f);
            g.setColour (Palette::ledOff);
            g.fillPath (led, t);
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.strokePath (led, juce::PathStrokeType (1.0f), t);
        }
    }

    // ---- Knob body ---------------------------------------------------------
    const float knobR = outerR * 0.70f;
    drawKnobBody (g, c, knobR);

    // ---- Pointer (lit, same colour as current LED) -------------------------
    const float angle = start + pos * (end - start);
    const auto  col   = ledColourAt (pos);

    juce::Path pointer;
    const float pw = knobR * 0.07f;
    pointer.addRoundedRectangle (-pw * 0.5f, -knobR * 0.80f, pw, knobR * 0.32f, pw * 0.5f);
    const auto pt = juce::AffineTransform::rotation (angle).translated (c.x, c.y);

    g.setColour (col.withAlpha (0.25f));
    g.strokePath (pointer, juce::PathStrokeType (pw * 1.2f), pt);
    g.setColour (col);
    g.fillPath (pointer, pt);
}

void MonoMakerLookAndFeel::drawSmallKnob (juce::Graphics& g, juce::Rectangle<float> bounds,
                                          float pos, float start, float end)
{
    const auto  c      = bounds.getCentre();
    const float outerR = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // Thin arc track
    const float arcR = outerR * 0.92f;
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, start, end, true);
    g.setColour (Palette::ledOff);
    g.strokePath (track, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc from 0 dB (centre of range is not 0 dB, so compute it)
    const float zeroPos   = 24.0f / 36.0f;  // 0 dB within -24..+12
    const float zeroAngle = start + zeroPos * (end - start);
    const float valAngle  = start + pos * (end - start);
    juce::Path val;
    val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f,
                       juce::jmin (zeroAngle, valAngle), juce::jmax (zeroAngle, valAngle), true);
    g.setColour (juce::Colour (0xffc8c8d0));
    g.strokePath (val, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob body
    const float knobR = outerR * 0.74f;
    drawKnobBody (g, c, knobR);

    // White pointer
    juce::Path pointer;
    const float pw = knobR * 0.10f;
    pointer.addRoundedRectangle (-pw * 0.5f, -knobR * 0.80f, pw, knobR * 0.38f, pw * 0.5f);
    g.setColour (juce::Colour (0xffe0e0e6));
    g.fillPath (pointer, juce::AffineTransform::rotation (valAngle).translated (c.x, c.y));
}
