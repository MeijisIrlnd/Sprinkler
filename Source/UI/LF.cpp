#include "LF.h"

namespace Sprinkler 
{ 
    LF::LF() 
    { 
        m_rThumb = juce::ImageCache::getFromMemory(BinaryData::SprinklerSliderDark_png, BinaryData::SprinklerSliderDark_pngSize);
        // #ebc4ce, #bfedef, #5d675b, #beee62, #fadf7f
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFFEBC4CE));
    }

    LF::~LF() 
    { 

    }

    void LF::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
    {
        using namespace juce;
        auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
        auto fill = slider.findColour(Slider::rotarySliderFillColourId);

        auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

        auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        g.setColour(outline);
        g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
        juce::AffineTransform rotation = juce::AffineTransform::rotation(toAngle, width / 2.0f, height / 2.0f);
        m_rThumb = m_rThumb.rescaled(width, height, juce::Graphics::ResamplingQuality::highResamplingQuality);
        g.drawImageTransformed(m_rThumb, rotation);
    }
}