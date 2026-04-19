#include "SineWaveChannel.h"
#include <iostream>
#include <cassert>
#include <cmath>

static constexpr float PI = 3.14159265358979323846f;

void SineWaveChannel::prepare(const double sampleRate)
{
    currentSampleRate = static_cast<float>(sampleRate);
    phase = 0.0f;
    smoothedFreq.reset(sampleRate, 0.05f);
    smoothedFreq.setCurrentAndTargetValue(getFrequency());
}

void SineWaveChannel::process(float* buffer, const int numSamples)
{
    assert(amplitude >= 0.0f && amplitude <= 1.0f);
    for (int sample = 0; sample < numSamples; ++sample) {
        const float frequency = smoothedFreq.getNextValue();
        const float phaseInc = (2.0f * PI * frequency) / currentSampleRate;
        buffer[sample] = amplitude * std::sinf(phase);
        phase += phaseInc;
    }
    std::cout << "Current Phase: " << phase << std::endl;  // Debugging line to check the value of phase
    std::cout << "Time Interval: " << (phase / (2.0f * PI)) * currentSampleRate << std::endl;  // Debugging line to check the total time interval covered by the processed samples
}
