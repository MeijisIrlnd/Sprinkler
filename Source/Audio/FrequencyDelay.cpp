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
        for (size_t i = 0; i < m_delayLines.size(); i++) {
            m_delayLines[i].prepare(samplesPerBlockExpected, sampleRate / 256);
            m_delayLines[i].setInterpolationRate(500);
            double currentMax;
            double scaledDelayTime{ 0 };
            if (i <= 29) {
                //scaledDelayTime = juce::jmap<double>(i, 0, 29, 0, 60);
                // log_1.1(y) + 29..
                auto dtNorm = SDSP::KMath::log<float>(i == 0 ? 0.001 / 29.0f : i / 29.0f, 1.1f);
                dtNorm += 40;
                dtNorm = dtNorm < 0 ? 0 : dtNorm;
                scaledDelayTime = juce::jmap<float>(dtNorm, 0, 40, 0, 60);
            }
            else {
                scaledDelayTime = 0;
            }
            currentMax = scaledDelayTime;
            m_delayLines[i].setMaxDelaySeconds(currentMax + 0.1);
            scaledDelayTime = static_cast<int>(scaledDelayTime * sampleRate / 256);
            m_delayLines[i].setDelay(scaledDelayTime);
        }
        m_smoothedFeedback.reset(sampleRate, 0.1);
        m_smoothedFeedback.setCurrentAndTargetValue(m_feedback);
        m_hasBeenPrepared = true;
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

        if (!m_hasPerformedFirstTransform) { 
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

    void FrequencyDelay::releaseResources()
    {
    }

    void FrequencyDelay::setHighestBinDelayTime(float delayTimeSeconds)
    {
        for (auto i = 0; i < m_delayLines.size(); i++) {
            float currentScaled = 0;
            if (i <= 29) {
                currentScaled = juce::jmap<double>(i, 0, 29, 0, delayTimeSeconds);
            }

            m_delayLines[i].setDelay(static_cast<int>(currentScaled * (m_sampleRate / 256.0f)));
        }
    }

    void FrequencyDelay::setFeedback(float feedback) { 
        m_feedback = feedback;
        if(m_hasBeenPrepared) { 
            m_smoothedFeedback.setTargetValue(feedback);
        }
    }

    void FrequencyDelay::stftCallback(float* data, size_t size)
    {
        for (auto i = 0; i < size - 1; i += 2) {
            STFTPair incoming{ {data[i], data[i + 1]} };
            STFTPair read = m_delayLines[i / 2].getNextSample(incoming + (m_prev * m_feedback));
            data[i] = read[0];
            data[i + 1] = read[1];
            m_prev = read;
        }
    }
}