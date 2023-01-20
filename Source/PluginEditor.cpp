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
    setSize (300, 400);
    setLookAndFeel(&m_lf);
    instantiateSlider(m_roomSizeSlider, tree, "RoomSize");
    instantiateSlider(m_reverbAmountContainer, tree, "ReverbAmount");
    instantiateSlider(m_sprinkleTimeContainer, tree, "SprinkleTime");
    instantiateSlider(m_sprinkleFeedbackContainer, tree, "SprinkleFeedback");
    instantiateSlider(m_dampeningFreqContainer, tree, "DampeningFrequency");
    instantiateSlider(m_dampeningGainContainer, tree, "DampeningGain");
}

SprinklerAudioProcessorEditor::~SprinklerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SprinklerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    std::stringstream text;
    text << "副作用は次のとおりです。\n";
    text << "吐き気!!\n";
    text << "リンパ節の腫れ!!\n";
    text << "(頻脈)!!\n";
    text << "小さな肺？\n";
    text << "私は盲目です！\n";
    text << "水の問題！\n";
    text << "胸痛！\n";
    text << "夜の悲しみ！\n";
    text << "関節炎！\n";
    text << "空気の質が息苦しくなる！";
    g.setColour(juce::Colour(0x237f7f7f));
    g.setFont(juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::NotoSansJPRegular_otf, BinaryData::NotoSansJPRegular_otfSize)).withHeight(getHeight() / 16));
    g.drawFittedText(text.str(), getBounds(), juce::Justification::centred, 13);
}

void SprinklerAudioProcessorEditor::resized()
{
    m_roomSizeSlider.slider.setBounds(0, 0, getWidth() / 4, getWidth() / 4);
    m_reverbAmountContainer.slider.setBounds(getWidth() / 4, 0, getWidth() / 4, getWidth() / 4);
    m_sprinkleTimeContainer.slider.setBounds(getWidth() / 2, 0, getWidth() / 4, getWidth() / 4);
    m_sprinkleFeedbackContainer.slider.setBounds(0, getWidth() / 4, getWidth() / 4, getWidth() / 4);
    m_dampeningFreqContainer.slider.setBounds(getWidth() / 4, getWidth() / 4, getWidth() / 4, getWidth() / 4); 
    m_dampeningGainContainer.slider.setBounds(getWidth() / 2, getWidth() / 4, getWidth() / 4, getWidth() / 4);

}

void SprinklerAudioProcessorEditor::instantiateSlider(SliderContainer& container, APVTS& tree, const juce::String& paramName)
{
    auto param = tree.getParameter(paramName);
    container.label.setText(param->getName(100), juce::dontSendNotification);
    container.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    container.slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(&container.slider);
    container.attachment.reset(new juce::SliderParameterAttachment(*param, container.slider, nullptr));
}
