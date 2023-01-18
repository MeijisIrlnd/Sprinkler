/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SprinklerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SprinklerAudioProcessorEditor (SprinklerAudioProcessor&, APVTS& tree);
    ~SprinklerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SprinklerAudioProcessor& audioProcessor;

    struct SliderContainer {
        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<juce::SliderParameterAttachment> attachment{ nullptr };
    };
    void instantiateSlider(SliderContainer& container, APVTS& tree, const juce::String& paramName);
    SliderContainer m_roomSizeSlider, m_reverbAmountContainer, m_sprinkleTimeContainer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SprinklerAudioProcessorEditor)
};
