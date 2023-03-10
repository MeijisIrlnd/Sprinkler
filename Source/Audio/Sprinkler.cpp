/*
  ==============================================================================

    Sprinkler.cpp
    Created: 16 Jan 2023 9:53:27pm
    Author:  Syl

  ==============================================================================
*/

#include "Sprinkler.h"

namespace Sprinkler
{
    SprinklerProcessor::SprinklerProcessor() : 
    m_diffusers
    {
        Diffuser<8>::Args<8>{
            //std::array<int, 8>{10, 15, 116, 2, 0, 30, 9, 12},
            std::array<int, 8>{0, 2, 10, 23, 15, 30, 32, 40},
            std::array<bool, 8>{false, true, true, false, false, false, true, true}
        },
        Diffuser<8>::Args<8>{
            std::array<int, 8>{0, 9, 30, 53, 57, 62, 72, 87},
                //1 1 1 0 1 1 1 0
            std::array<bool, 8>{true, true, true, false, true, true, true, false}
        },
        Diffuser<8>::Args<8>{
            std::array<int, 8>{0, 10, 18, 50, 90, 105, 132, 170},
                //1 1 0 1 1 1 0 0
            std::array<bool, 8>{true, true, false, true, true, true, false, false}
        },
        Diffuser<8>::Args<8>{
            std::array<int, 8>{67, 91, 79, 95, 205, 121, 196, 290},
                //0 0 1 1 0 1 0 0
            std::array<bool, 8>{false, false, true, true, false, true, false, false}
        }
    }
    {
    }
    SprinklerProcessor::~SprinklerProcessor()
    {
    }

    void SprinklerProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        m_sampleRate = sampleRate;
        SDSP::RBJ::highShelf(m_coeffs.target(0), sampleRate, m_dampeningCF, m_dampeningGain, 1.0f);
        std::memcpy(m_coeffs.current(0), m_coeffs.target(0), sizeof(float) * 6);

        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 1 };
        int i = 0;
        for(auto i = 0; i < 8; i++) { 
            m_highShelves[i].setCoefficients(m_coeffs.target(0));
            m_delayLines[i].prepare(samplesPerBlockExpected, sampleRate);
            m_delayLines[i].setMaxDelaySeconds(1.0f);
            m_delayLines[i].setDelay(static_cast<int>(m_delayTimes[static_cast<size_t>(i)] * m_sampleRate));
        }

        for (auto& d : m_diffusers) {
            d.prepareToPlay(samplesPerBlockExpected, sampleRate);
        }
        m_smoothedFeedback.reset(sampleRate, 0.1);
        m_smoothedReverbAmount.reset(sampleRate, 0.1);
        m_smoothedFeedback.setCurrentAndTargetValue(m_feedback);
        m_smoothedReverbAmount.setCurrentAndTargetValue(m_reverbAmount);
        for (auto& d : m_frequencyDelays) {
            d.prepareToPlay(samplesPerBlockExpected, sampleRate);
        }
        m_hasBeenPrepared = true;
    }

    void SprinklerProcessor::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
    {
        auto* read = buffer.getArrayOfReadPointers();
        auto* write = buffer.getArrayOfWritePointers();
        const auto channelsPerChannel = m_samples.size() / buffer.getNumChannels();
        for (auto sample = 0; sample < buffer.getNumSamples(); sample++) {
            if(m_samplesUntilUpdate == 0) { 
                SDSP::RBJ::highShelf(m_coeffs.target(0), m_sampleRate, m_dampeningCF, m_dampeningGain, 1.0f);
                m_samplesUntilUpdate = m_updateRate;
            }
            interpolateCoeffs();
            for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
                float in = read[channel][sample];
                juce::FloatVectorOperations::fill(m_samples.data() + (channelsPerChannel * channel), in, channelsPerChannel);
            }

            juce::FloatVectorOperations::copy(m_earlyReflections.data(), m_samples.data(), 8);
            for (auto i = 0; i < 3; i++) {
                m_diffusers[i].diffuse(m_samples);
                juce::FloatVectorOperations::copy(m_temp.data(), m_samples.data(), 8);
                juce::FloatVectorOperations::multiply(m_temp.data(), 1 / static_cast<float>(m_diffusers.size()), 8);
                juce::FloatVectorOperations::add(m_earlyReflections.data(), m_temp.data(), 8);
            }

            SDSP::Hadamard<float, 8>::inPlace(m_earlyReflections.data());
            auto reverbAmount = m_smoothedReverbAmount.getNextValue();
            juce::FloatVectorOperations::multiply(m_earlyReflections.data(), 1 - reverbAmount, 8);
            juce::FloatVectorOperations::multiply(m_samples.data(), reverbAmount, 8);
            juce::FloatVectorOperations::add(m_samples.data(), m_earlyReflections.data(), 8);

            float feedback = m_smoothedFeedback.getNextValue();
            for(auto channel = 0; channel < 8; channel++) { 
                // read from delay line, and insert current sample..
                m_prev[channel] = m_frequencyDelays[channel].processSample(buffer.getNumSamples(), m_highShelves[channel].processSample(m_prev[channel]));
                m_samples[channel] = m_delayLines[channel].getNextSample(m_samples[channel] + (m_prev[channel] * feedback));
                m_prev[channel] = m_samples[channel];
            }
            SDSP::Householder<float, 8>::inPlace(m_prev.data());
            juce::FloatVectorOperations::add(m_samples.data(), m_earlyReflections.data(), 8);

            for (auto channel = 0; channel < 2; channel++) {
                write[channel][sample] = m_samples[channel];
            }
            --m_samplesUntilUpdate;
        }

    }

    void SprinklerProcessor::releaseResources()
    {
        for (auto& freqDelay : m_frequencyDelays) {
            freqDelay.releaseResources();
        }
    }

    void SprinklerProcessor::setRoomSize(float sizeInMetres)
    {
        // Speed of sound = 343m/s, so 1 metre -> (1 / 343)
        float seconds = sizeInMetres / m_speedOfSound;
        for (auto i = 0; i < m_delayTimes.size(); i++) {
            m_delayTimes[i] = seconds * std::powf(2, i / static_cast<float>(m_delayLines.size()));
        }
        // delay time = seconds * 2^i/N
        if (m_hasBeenPrepared) {
            for (auto i = 0; i < m_delayLines.size(); i++) {
                m_delayLines[i].setDelay(m_delayTimes[i] * m_sampleRate);
            }
        }
    }

    void SprinklerProcessor::setFeedback(float feedback)
    {
        m_feedback = feedback;
        if (m_hasBeenPrepared) {
            m_smoothedFeedback.setTargetValue(feedback);
        }
    }

    void SprinklerProcessor::setReverbAmount(float reverbAmount)
    {
        m_reverbAmount = reverbAmount;
        if (m_hasBeenPrepared) {
            m_smoothedReverbAmount.setTargetValue(reverbAmount);
        }
    }

    void SprinklerProcessor::interpolateCoeffs() 
    { 
        m_coeffs.interpolate();
        for(auto& f : m_highShelves) { 
            f.setCoefficients(m_coeffs.current(0));
        }
    }

}