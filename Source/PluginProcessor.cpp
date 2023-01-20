/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SprinklerAudioProcessor::SprinklerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else 
    :
#endif
    m_tree(*this, nullptr, juce::Identifier{"Sprinkler"}, generateLayout())
{
        bindListeners();
}

SprinklerAudioProcessor::~SprinklerAudioProcessor()
{
}

//==============================================================================
const juce::String SprinklerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SprinklerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SprinklerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SprinklerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SprinklerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SprinklerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SprinklerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SprinklerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SprinklerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SprinklerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SprinklerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_sprinkler.prepareToPlay(samplesPerBlock, sampleRate);
}

void SprinklerAudioProcessor::releaseResources()
{
    m_sprinkler.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SprinklerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SprinklerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    m_sprinkler.getNextAudioBlock(buffer);

}

//==============================================================================
bool SprinklerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SprinklerAudioProcessor::createEditor()
{
    return new SprinklerAudioProcessorEditor (*this, m_tree);
}

//==============================================================================
void SprinklerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SprinklerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SprinklerAudioProcessor::parameterChanged(const juce::String& id, float value)
{
    if (id == "RoomSize") {
        m_sprinkler.setRoomSize(value);
    }
    else if (id == "ReverbAmount") {
        m_sprinkler.setReverbAmount(value);
    }
    else if (id == "SprinkleTime") {
        m_sprinkler.setSprinkleTime(value);
    } 
    else if(id == "SprinkleFeedback") { 
        m_sprinkler.setSprinkleFeedback(value);
    }
    else if(id == "DampeningFrequency") { 
        m_sprinkler.setDampeningFrequency(value);
    }
    else if(id == "DampeningGain") { 
        m_sprinkler.setDampeningGain(value);
    }
}

void SprinklerAudioProcessor::bindListeners()
{
    m_tree.addParameterListener("RoomSize", this);
    m_tree.addParameterListener("ReverbAmount", this);
    m_tree.addParameterListener("SprinkleTime", this);
    m_tree.addParameterListener("SprinkleFeedback", this);
    m_tree.addParameterListener("DampeningFrequency", this);
    m_tree.addParameterListener("DampeningGain", this);

}

APVTS::ParameterLayout SprinklerAudioProcessor::generateLayout()
{
    APVTS::ParameterLayout layout;
    using FloatParam = juce::AudioParameterFloat;
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("RoomSize", 1), "Room Size", juce::NormalisableRange<float>(1, 100, 0.01), 7));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("ReverbAmount", 1), "Reverb Amount", juce::NormalisableRange<float>(0, 1, 0.01), 0.5));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("SprinkleTime", 1), "Sprinkle Time", juce::NormalisableRange<float>(0.1, 60, 0.01f), 10.0f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("SprinkleFeedback", 1), "Sprinkle Feedback", juce::NormalisableRange<float>(0.0f, 0.5f), 0.0f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("DampeningFrequency", 1), "Dampening Frequency", juce::NormalisableRange<float>(1000, 15000, 0.01f, 0.5f), 1000.0f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("DampeningGain", 1), "Dampening Gain", juce::NormalisableRange<float>(-12, 0, 0.01f), -1.5f));
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SprinklerAudioProcessor();
}
