#pragma once 
#include <JuceHeader.h>
namespace Sprinkler 
{ 
    class LF : public juce::LookAndFeel_V4
    { 
        public: 
        LF();
        ~LF() override;
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    private: 
        juce::Image m_rThumb;
    };
}