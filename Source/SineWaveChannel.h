#pragma once

#ifndef SINEWAVE_CHANNEL_H
#define SINEWAVE_CHANNEL_H

#include <juce_audio_basics/juce_audio_basics.h>

class SineWaveChannel
{
public:
    void prepare (double sampleRate);
    void process (float* buffer, int numSamples);
    [[nodiscard]] float getAmplitude() const { return amplitude; }
    [[nodiscard]] float getFrequency() { return smoothedFreq.getNextValue(); }
    void setAmplitude(const float newAmplitude) { amplitude = newAmplitude; }
    void setFrequency(const float newFrequency) { smoothedFreq.setTargetValue(newFrequency); }

private:
    float amplitude = 0.2f;
    float currentSampleRate = 0.0f;
    float phase = 0.0f;
    float currentTime;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedFreq;
};

#endif  // !SINEWAVE_CHANNEL_H
