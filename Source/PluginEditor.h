#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

using namespace juce;

//==============================================================================
/**
*/
class WeatherSoundAudioProcessorEditor  : public AudioProcessorEditor, private Slider::Listener  // [2]
{
public:
    WeatherSoundAudioProcessorEditor (WeatherSoundAudioProcessor&);
    ~WeatherSoundAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider* slider) override;  // [3]
    auto getResource(const juce::String& url) const -> std::optional<juce::WebBrowserComponent::Resource>;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WeatherSoundAudioProcessor& audioProcessor;

    Slider frequencySlider;
    TextButton bypassButton;
    Label frequencyLabel {"FrequencyLabel", "Frequency"};
    AudioProcessorValueTreeState::SliderAttachment freqSliderAttachment;
    AudioProcessorValueTreeState::ButtonAttachment bypassButtonAttachment;
    WebSliderRelay freqRelay { "freqHz" };
    WebSliderParameterAttachment freqAttachment {
        *audioProcessor.getState().getParameter("freqHz"), freqRelay, nullptr
    };
    WebSliderRelay resonanceRelay { "resonance" };
    WebSliderParameterAttachment resonanceAttachment {
        *audioProcessor.getState().getParameter("resonance"), resonanceRelay, nullptr
    };
    WebToggleButtonRelay bypassRelay { "bypass" };
    WebToggleButtonParameterAttachment bypassAttachment {
        *audioProcessor.getState().getParameter("bypass"), bypassRelay, nullptr
    };

    WebBrowserComponent webComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WeatherSoundAudioProcessorEditor)
};
