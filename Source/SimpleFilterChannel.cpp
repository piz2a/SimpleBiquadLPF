#include "SimpleFilterChannel.h"

void SimpleFilterChannel::setCoefficients()
{
    // Biquad LPF: hard coded coefficients calculation
    const float w0 = 2.0f * juce::MathConstants<float>::pi * cutoffFrequency / currentSampleRate;
    const float alpha = std::sin(w0) / (2.0f * q);
    const float a_0 = 1.0f + alpha;
    b_1 = (1.0f - std::cos(w0)) / a_0;
    b_2 = b_1 / 2.0f;
    a_1 = -2.0f * std::cos(w0) / a_0;
    a_2 = (1.0f - alpha) / a_0;
}

void SimpleFilterChannel::prepare (double sampleRate)
{
    currentSampleRate = static_cast<float>(sampleRate);
    previousOutput1 = 0.0f;
    previousOutput2 = 0.0f;
    previousInput1 = 0.0f;
    previousInput2 = 0.0f;
}

void SimpleFilterChannel::process (float* channelData, const int numSamples)
{
    float prevIn1 = previousInput1;
    float prevIn2 = previousInput2;
    float prevOut1 = previousOutput1;
    float prevOut2 = previousOutput2;

    for (int sample = 0; sample < numSamples; ++sample) {
        float input_0 = channelData[sample];
        // Biquadratic low-pass filter
        float output = b_2 * input_0 + b_1 * prevIn1 + b_2 * prevIn2 - a_1 * prevOut1 - a_2 * prevOut2;

        // 안정성 검사: NaN이나 Inf가 나오면 모든 상태를 0으로 리셋
        if (std::isnan(output) || std::isinf(output)) {
            previousOutput1 = previousOutput2 = previousInput1 = previousInput2 = 0.0f;
            output = 0.0f;
        }

        channelData[sample] = output;
        prevIn2 = prevIn1;
        prevIn1 = input_0;
        prevOut2 = prevOut1;
        prevOut1 = output;
    }

    previousOutput1 = prevOut1;
    previousOutput2 = prevOut2;
    previousInput1 = prevIn1;
    previousInput2 = prevIn2;
}