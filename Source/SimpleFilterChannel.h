#pragma once

#ifndef SIMPLEFILTERCHANNEL_H
#define SIMPLEFILTERCHANNEL_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class SimpleFilterChannel  // Sample Filter for each channel
{
public:
    void prepare (double sampleRate);
    void process (float* channelData, const int numSamples);
    void setCoefficients();
    void setCutoffFrequency(float newCutoff) { cutoffFrequency = newCutoff; setCoefficients(); }
    void setQ(float newQ) { q = newQ; setCoefficients(); }
    [[nodiscard]] float getCutoffFrequency() const { return cutoffFrequency; } 
    [[nodiscard]] float getQ() const { return q; }

private:
    float cutoffFrequency = 1000.0f;
    float q = 0.707f;  // Q factor
    float currentSampleRate = 0.0f;
    float a_1, a_2, b_1, b_2;  // Filter coefficients (b_0 = b_2)
    float previousInput1 = 0.0f, previousInput2 = 0.0f;   // second-order filter state
    float previousOutput1 = 0.0f, previousOutput2 = 0.0f;   // second-order filter state
};

#endif  // !SIMPLEFILTERCHANNEL_H