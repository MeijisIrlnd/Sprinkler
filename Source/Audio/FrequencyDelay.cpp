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
        SDSP::RBJ::highShelf(m_coeffs.target(0), sampleRate, 500.0, -3.0, 0.5);
        std::memcpy(m_coeffs.current(0), m_coeffs.target(0), sizeof(double) * 6);
        m_filters.setCoefficients(m_coeffs.current(0));

        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 1 };
        for (auto i = 0; i < m_delayLines.size(); i++) {
            m_delayLines[i].prepare(samplesPerBlockExpected, sampleRate / 256);
            m_delayLines[i].setInterpolationRate(500);
            double currentMax;
            double scaledDelayTime{ 0 };
            if (i <= 29) {
                scaledDelayTime = juce::jmap<double>(i, 0, 29, 0, 60);
            }
            else {
                scaledDelayTime = 0;
            }
            currentMax = scaledDelayTime;
            m_delayLines[i].setMaxDelaySeconds(currentMax + 0.1);
            scaledDelayTime = static_cast<int>(scaledDelayTime * sampleRate / 256);
            m_delayLines[i].setDelay(scaledDelayTime);
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
        if(m_filterUpdateParams.samplesUntilUpdate == 0) { 
            SDSP::RBJ::lowShelf(m_coeffs.target(0), m_sampleRate, m_filterCF, m_filterGain, 0.5f);
            m_filterUpdateParams.samplesUntilUpdate = m_filterUpdateParams.samplesUntilUpdate;
        }
        interpolateCoeffs();
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
        float out = m_filters.processSample(m_data[m_writePos]);
        ++m_writePos;
        --m_samplesUntilUpdate;
        --m_filterUpdateParams.samplesUntilUpdate;
        return out;
    }

    void FrequencyDelay::releaseResources()
    {
    }

    void FrequencyDelay::interpolateCoeffs() 
    { 
        m_coeffs.interpolate();
        m_filters.setCoefficients(m_coeffs.current(0));
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

    void FrequencyDelay::setFilterCF(float newCf) { 

    }

    void FrequencyDelay::setFilterGain(float newGainDB) { 

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