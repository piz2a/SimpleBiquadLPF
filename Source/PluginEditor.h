#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "Square.h"

//==============================================================================
/**
*/
class SimpleFilterAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Slider::Listener  // [2]
{
public:
    SimpleFilterAudioProcessorEditor (SimpleFilterAudioProcessor&);
    ~SimpleFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* slider) override;  // [3]

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleFilterAudioProcessor& audioProcessor;
    juce::WebBrowserComponent webComponent;

    Square square;
    juce::Slider midiVolume;  // [1]
    juce::Slider frequencySlider;
    juce::TextButton playButton;
    juce::Label frequencyLabel {"FrequencyLabel", "Frequency"};
    juce::AudioProcessorValueTreeState::SliderAttachment freqSliderAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment playButtonAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFilterAudioProcessorEditor)
};
