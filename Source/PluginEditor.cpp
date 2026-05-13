#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// CONSTRUCTOR
SimpleFilterAudioProcessorEditor::SimpleFilterAudioProcessorEditor (SimpleFilterAudioProcessor& p)
    : AudioProcessorEditor (&p),audioProcessor (p),
      freqSliderAttachment(audioProcessor.getState(), "freqHz", frequencySlider),
      playButtonAttachment(audioProcessor.getState(), "play", playButton),
      webComponent (juce::WebBrowserComponent::Options{}
        .withResourceProvider ([this](const auto& url) {
            return juce::WebBrowserComponent::Resource{ /* 로컬 파일 처리 로직 */ };
        })
        // 'updateParameter'라는 이름의 함수를 브라우저에 노출합니다.
        .withNativeFunction ("updateParameter", [this] (const juce::var& args, auto completion) {
            if (args.size() >= 2) {
                auto paramID = args[0].toString();
                auto newValue = (float) args[1];

                // APVTS에서 해당 파라미터를 찾아 값을 업데이트합니다.
                if (auto* param = audioProcessor.getState().getParameter(paramID)) {
                    // 오디오 스레드에 안전하게 값을 알립니다.
                    param->setValueNotifyingHost(newValue);
                }
            }
            // 성공 여부를 JS에 다시 전달할 수도 있습니다.
            completion (juce::var (true));
        }))
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible (webComponent);
    webComponent.goToURL ("http://localhost:5173");
    
    // This is where our plugin’s editor size is set.
    setSize (480, 320);
    
    /*
    frequencySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    // frequencySlider.setRange(0.0f, 1.0f, 0.01f);
    addAndMakeVisible(frequencySlider);
    
    playButton.setButtonText("Active");
    playButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    playButton.setClickingTogglesState(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    playButton.onClick = [this]()
    {
        // change the state of the button when it's clicked
        const bool isPlaying = playButton.getToggleState();
        playButton.setButtonText(isPlaying ? "Active" : "Bypassed");
    };
    addAndMakeVisible(playButton);
    
    frequencyLabel.setColour (juce::Label::ColourIds::outlineColourId, juce::Colours::white);
    addAndMakeVisible(frequencyLabel);
    */
}

// DECONSTRUCTOR
SimpleFilterAudioProcessorEditor::~SimpleFilterAudioProcessorEditor()
{
}

//==============================================================================
void SimpleFilterAudioProcessorEditor::paint (juce::Graphics& g)
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
    playButton.setBounds(getWidth() - 120, 30, 100, 30);
}

void SimpleFilterAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
}
