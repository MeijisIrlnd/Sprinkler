/*
  ==============================================================================

    FrequencyDelay.cpp
    Created: 16 Jan 2023 10:04:04pm
    Author:  Syl

  ==============================================================================
*/

#include "FrequencyDelay.h"

namespace Sprinkler
{
    FrequencyDelay::FrequencyDelay() : m_delayLines(256)
    {
    }

    FrequencyDelay::~FrequencyDelay()
    {
    }

    void FrequencyDelay::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        m_sampleRate = sampleRate;
        juce::dsp::ProcessSpec spec{ sampleRate, samplesPerBlockExpected, 2 };
        for (auto i = 0; i < 2; i++) {
            for (auto j = 0; j < m_delayLines.size(); j++) {
                m_delayLines[j][i].prepare(samplesPerBlockExpected, sampleRate / 256);
                m_delayLines[j][i].setInterpolationRate(500);
                double currentMax;
                double scaledDelayTime{ 0 };
                if (j <= 29) {
                   scaledDelayTime = juce::jmap<double>(j, 0, 29, 0, 60);
                }
                else {
                    scaledDelayTime = 0;
                }
                currentMax = scaledDelayTime;
                m_delayLines[j][i].setMaxDelaySeconds(currentMax + 0.1);
                scaledDelayTime = static_cast<int>(scaledDelayTime * sampleRate / 256);
                m_delayLines[j][i].setDelay(scaledDelayTime);
            }
        }
    }

    float FrequencyDelay::processSample(const int bufferSize, float x)
    {
        if (m_data.size() != bufferSize) { 
            m_data.resize(bufferSize); 
            juce::FloatVectorOperations::fill(m_data.data(), 0.0f, bufferSize); 
        }
        if (m_accumulator.size() != bufferSize) { 
            m_accumulator.resize(bufferSize); 
            juce::FloatVectorOperations::fill(m_accumulator.data(), 0.0f, bufferSize);
        }
        if (!m_hasPerformedFirstTransform ) { 
            m_samplesUntilUpdate = bufferSize;
            m_hasPerformedFirstTransform = true;
        }
        else {
            if (m_samplesUntilUpdate == 0) {
                // process... 
                m_stfts[0].process(m_accumulator.data(), m_data.data(), bufferSize, m_stftCallback);
                m_samplesUntilUpdate = bufferSize;
                m_writePos = 0;

            }
        }
        m_accumulator[m_writePos] = x;
        float out = m_data[m_writePos];
        ++m_writePos;
        --m_samplesUntilUpdate;
        return out;
    }

    void FrequencyDelay::getNextAudioBlock(juce::AudioBuffer<float>& buffer)
    {
        auto* read = buffer.getArrayOfReadPointers();
        auto* write = buffer.getArrayOfWritePointers();
        for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
            m_currentlyProcessingChannel = channel;
            m_stfts[channel].process(read[channel], write[channel], buffer.getNumSamples(), m_stftCallback);
        }
    }

    void FrequencyDelay::releaseResources()
    {
    }

    void FrequencyDelay::setHighestBinDelayTime(float delayTimeSeconds)
    {
        for (auto i = 0; i < 2; i++) {
            for (auto j = 0; j < m_delayLines.size(); j++) {
                float currentScaled = 0;
                if (j <= 29) {
                    currentScaled = juce::jmap<double>(j, 0, 29, 0, delayTimeSeconds);
                }

                m_delayLines[j][i].setDelay(static_cast<int>(currentScaled * (m_sampleRate / 256.0f)));
            }
        }
    }

    void FrequencyDelay::stftCallback(float* data, size_t size)
    {
        for (auto i = 0; i < size - 1; i += 2) {
            STFTPair incoming{ {data[i], data[i + 1]} };
            // How the fuck do we know the channel? 
            
            STFTPair read = m_delayLines[i / 2][m_currentlyProcessingChannel].getNextSample(incoming + m_prev[m_currentlyProcessingChannel] * m_feedback);
            data[i] = read[0];
            data[i + 1] = read[1];
            m_prev[m_currentlyProcessingChannel] = read;
        }
    }
}