#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

using namespace juce;

//==============================================================================
/**
*/
class SimpleFilterAudioProcessorEditor  : public AudioProcessorEditor, private Slider::Listener  // [2]
{
public:
    SimpleFilterAudioProcessorEditor (SimpleFilterAudioProcessor&);
    ~SimpleFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider* slider) override;  // [3]

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleFilterAudioProcessor& audioProcessor;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFilterAudioProcessorEditor)
};
