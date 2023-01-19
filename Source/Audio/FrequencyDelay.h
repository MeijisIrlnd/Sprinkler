/*
  ==============================================================================

    FrequencyDelay.h
    Created: 16 Jan 2023 10:04:04pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <queue>
namespace Sprinkler
{
    class FrequencyDelay
    {
    public: 
        FrequencyDelay();
        ~FrequencyDelay();
        FrequencyDelay(const FrequencyDelay& other) = delete;
        FrequencyDelay(FrequencyDelay&& other) noexcept = delete;
        FrequencyDelay& operator=(const FrequencyDelay& other) = delete;
        FrequencyDelay& operator=(FrequencyDelay&& other) = delete;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        float processSample(const int bufferSize, float x);
        void releaseResources();

        void setHighestBinDelayTime(float delayTimeSeconds);
        void setFilterCF(float newCF);
        void setFilterGain(float newGainDB);
    private: 
        void interpolateCoeffs();
        void stftCallback(float* data, size_t size);
        double m_sampleRate;
        int m_currentlyProcessingChannel{ 0 };
        std::function<void(float*, size_t)> m_stftCallback = [this](float* data, size_t size) { stftCallback(data, size); };
        std::array<SDSP::Fourier::STFT<256, 8>, 2> m_stfts;
        struct STFTPair {
            std::array<float, 2> items;
            float operator[](size_t idx) { return items[idx]; }

            STFTPair& operator+(STFTPair& other) {
                items[0] += other[0];
                items[1] += other[1];
                return *this;
            }

            STFTPair& operator*(float multiplier) {
                items[0] *= multiplier;
                items[1] *= multiplier;
                return *this;
            }
        };
        // 256 fucking delay lines LOL 
        std::vector<SDSP::CircularBuffer<STFTPair> > m_delayLines;
        STFTPair m_prev;
        float m_feedback{ 0.0f };
        std::vector<float> m_accumulator, m_data;
        bool m_hasPerformedFirstTransform{ false };
        int m_samplesUntilUpdate{ 0 };
        int m_writePos{ 0 };
        SDSP::SmoothedFilterCoefficients<1> m_coeffs;
        SDSP::BiquadCascade<1> m_filters;

        struct FilterUpdateParams { 
            const int updateRate{100};
            int samplesUntilUpdate{0};
        } m_filterUpdateParams;

        float m_filterCF{500.0f}, m_filterGain{-6.0f};
    };
}