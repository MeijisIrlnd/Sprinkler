/*
  ==============================================================================

    Sprinkler.h
    Created: 16 Jan 2023 9:53:27pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <Audio/Diffuser.h>
#include <Audio/FrequencyDelay.h>
namespace Sprinkler
{
    class SprinklerProcessor
    {
    public: 
        SprinklerProcessor();
        ~SprinklerProcessor();
        SprinklerProcessor(const SprinklerProcessor& other) = delete;
        SprinklerProcessor(SprinklerProcessor&& other) noexcept = delete;
        SprinklerProcessor& operator=(const SprinklerProcessor& other) = delete;
        SprinklerProcessor& operator=(SprinklerProcessor&& other) noexcept = delete;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        void getNextAudioBlock(juce::AudioBuffer<float>& buffer);
        void releaseResources();

        void setRoomSize(float sizeInMetres);
        void setFeedback(float feedback);
        void setReverbAmount(float reverbAmount);
        void setSprinkleTime(float sprinkleTime) {
            for (auto& d : m_frequencyDelays) {
                d.setHighestBinDelayTime(sprinkleTime);
            } 
        }
    private:
        bool m_hasBeenPrepared{ false };
        double m_sampleRate{ 0 };
        std::array<float, 8> m_samples, m_earlyReflections, m_temp, m_prev;
        std::array<Diffuser<8>, 4> m_diffusers;
        std::array<FrequencyDelay, 8> m_frequencyDelays;
        std::array<SDSP::CircularBuffer<float>, 8> m_delayLines;
        const float m_speedOfSound{ 343.0f };
        std::array<float, 8> m_delayTimes{
            (100 / m_speedOfSound) * std::powf(2, 1 / 8.0f),
            (100 / m_speedOfSound) * std::powf(2, 2 / 8.0f),
            (100 / m_speedOfSound) * std::powf(2, 3 / 8.0f),
            (100 / m_speedOfSound) * std::powf(2, 4 / 8.0f),
            (100 / m_speedOfSound) * std::powf(2, 5 / 8.0f),
            (100 / m_speedOfSound) * std::powf(2, 6 / 8.0f),
            (100 / m_speedOfSound) * std::powf(2, 7 / 8.0f),
            (100 / m_speedOfSound) * 2.0f
        };
        float m_feedback{ 0.4f }, m_reverbAmount{ 0.5f };
        juce::SmoothedValue<float> m_smoothedFeedback;
        juce::SmoothedValue<float> m_smoothedReverbAmount;
    };
}