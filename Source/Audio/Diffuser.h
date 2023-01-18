/*
  ==============================================================================

    Diffuser.h
    Created: 17 Jan 2023 1:25:17am
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
namespace Sprinkler
{
    template<int NCHANNELS>
    class Diffuser
    {
    public: 
        template<int N>
        struct Args
        {
            std::array<int, N> times;
            std::array<bool, N> polarities;
        };

        Diffuser(const Args<NCHANNELS>& args) : m_diffuserDelayLineTimes(args.times), m_polarities(args.polarities), m_delayLines(NCHANNELS) { }
        ~Diffuser() { }
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
        {
            juce::dsp::ProcessSpec spec{ sampleRate, samplesPerBlockExpected, 1 };
            for (auto i = 0; i < m_delayLines.size(); i++) {
                m_delayLines[i].prepare(spec);
                m_delayLines[i].setMaximumDelayInSamples(static_cast<int>((m_diffuserDelayLineTimes[i] / 1000.0f) * sampleRate) + 1);
                m_delayLines[i].setDelay((m_diffuserDelayLineTimes[i] / 1000.0f) * sampleRate);
            }
        }

        void diffuse(std::array<float, NCHANNELS>& channels)
        {
            for (auto channel = 0; channel < NCHANNELS; channel++)
            {
                m_delayLines[channel].pushSample(0, channels[channel]);
                auto delayed = m_delayLines[channel].popSample(0);
                // Polarity 
                delayed *= m_polarities[channel] ? 1 : -1;
                channels[channel] = delayed;
            }
            // Hadamard 
            SDSP::Hadamard<float, 8>::inPlace(channels.data());
        }


    private: 
        std::vector<juce::dsp::DelayLine<float> > m_delayLines;
        const std::array<int, 8> m_diffuserDelayLineTimes;
        const std::array<bool, 8> m_polarities;
    };
}