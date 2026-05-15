#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// CONSTRUCTOR
SimpleFilterAudioProcessorEditor::SimpleFilterAudioProcessorEditor (SimpleFilterAudioProcessor& p)
    : AudioProcessorEditor (&p),audioProcessor (p),
      freqSliderAttachment(audioProcessor.getState(), "freqHz", frequencySlider),
      bypassButtonAttachment(audioProcessor.getState(), "bypass", bypassButton),
      webComponent (WebBrowserComponent::Options{}
        .withBackend (WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options (WebBrowserComponent::Options::WinWebView2{}
        .withUserDataFolder (File::getSpecialLocation (File::SpecialLocationType::tempDirectory)))
        .withResourceProvider ([this](const auto& url) {
            return WebBrowserComponent::Resource{ /* 로컬 파일 처리 로직 */ };
        })
        .withOptionsFrom (freqRelay)
        .withOptionsFrom (resonanceRelay)
        .withOptionsFrom (bypassRelay)
        .withNativeIntegrationEnabled() // Necessary
    )
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible (webComponent);
#if JUCE_DEBUG
    // Debug mode: Load from local development server for hot-reloading
    webComponent.goToURL ("http://localhost:5173");
#else
    // Release mode: Load from bundled resources
    webComponent.goToURL (WebBrowserComponent::getResourceProviderRoot());
#endif
    
    // This is where our plugin’s editor size is set.
    setSize (480, 320);
    
    /*
    frequencySlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    frequencySlider.setTextBoxStyle(Slider::TextBoxBelow, true, 100, 50);
    // frequencySlider.setRange(0.0f, 1.0f, 0.01f);
    addAndMakeVisible(frequencySlider);
    
    bypassButton.setButtonText("Bypass");
    bypassButton.setToggleState(false, NotificationType::dontSendNotification);
    bypassButton.setClickingTogglesState(true);
    bypassButton.setColour(TextButton::ColourIds::buttonOnColourId, Colours::green);
    bypassButton.setColour(TextButton::ColourIds::buttonColourId, Colours::red);
    bypassButton.onClick = [this]()
    {
        // change the state of the button when it's clicked
        const bool isBypassed = bypassButton.getToggleState();
        bypassButton.setButtonText(isBypassed ? "Bypassed" : "Active");
    };
    addAndMakeVisible(bypassButton);
    
    frequencyLabel.setColour (Label::ColourIds::outlineColourId, Colours::white);
    addAndMakeVisible(frequencyLabel);
    */
}

// DECONSTRUCTOR
SimpleFilterAudioProcessorEditor::~SimpleFilterAudioProcessorEditor()
{
}

//==============================================================================
void SimpleFilterAudioProcessorEditor::paint (Graphics& g)
{
}

void SimpleFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    webComponent.setBounds (0, 0, getWidth(), getHeight());
    
    // sets the position and size of the slider with arguments (x, y, width, height)
    // frequencySlider.setBounds (getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 200);
    // frequencyLabel.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 20);
    bypassButton.setBounds(getWidth() - 120, 30, 100, 30);
}

void SimpleFilterAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
}
