/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SprinklerAudioProcessorEditor::SprinklerAudioProcessorEditor (SprinklerAudioProcessor& p, APVTS& tree)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);
    instantiateSlider(m_roomSizeSlider, tree, "RoomSize");
    instantiateSlider(m_reverbAmountContainer, tree, "ReverbAmount");
    instantiateSlider(m_sprinkleTimeContainer, tree, "SprinkleTime");
}

SprinklerAudioProcessorEditor::~SprinklerAudioProcessorEditor()
{
}

//==============================================================================
void SprinklerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SprinklerAudioProcessorEditor::resized()
{
    m_roomSizeSlider.slider.setBounds(0, 0, getWidth() / 4, getWidth() / 4);
    m_reverbAmountContainer.slider.setBounds(getWidth() / 4, 0, getWidth() / 4, getWidth() / 4);
    m_sprinkleTimeContainer.slider.setBounds(getWidth() / 2, 0, getWidth() / 4, getWidth() / 4);
}

void SprinklerAudioProcessorEditor::instantiateSlider(SliderContainer& container, APVTS& tree, const juce::String& paramName)
{
    auto param = tree.getParameter(paramName);
    container.label.setText(param->getName(100), juce::dontSendNotification);
    container.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    container.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 30, 20);
    container.label.attachToComponent(&container.slider, false);
    addAndMakeVisible(&container.slider);
    container.attachment.reset(new juce::SliderParameterAttachment(*param, container.slider, nullptr));
}
