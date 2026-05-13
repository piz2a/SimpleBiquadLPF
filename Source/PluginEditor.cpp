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
    // addAndMakeVisible (webComponent);
    webComponent.goToURL ("http://localhost:5173");
    
    // This is where our plugin’s editor size is set.
    setSize (500, 700);
    
    // these define the parameters of our slider object
    midiVolume.setSliderStyle (juce::Slider::LinearBarVertical);
    midiVolume.setRange (0.0, 127.0, 1.0);
    midiVolume.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled (true, false, this);
    midiVolume.setTextValueSuffix (" Volume");
    midiVolume.setValue (1.0);
    // this function adds the slider to the editor
    addAndMakeVisible (midiVolume);
    // add the listener to the slider
    midiVolume.addListener (this);
    
    frequencySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    // frequencySlider.setRange(0.0f, 1.0f, 0.01f);
    addAndMakeVisible(frequencySlider);
    
    playButton.setButtonText("Playing");
    playButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    playButton.setClickingTogglesState(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    playButton.onClick = [this]()
    {
        // change the state of the button when it's clicked
        const bool isPlaying = playButton.getToggleState();
        playButton.setButtonText(isPlaying ? "Playing" : "Bypassed");
    };
    addAndMakeVisible(playButton);
    
    frequencyLabel.setColour (juce::Label::ColourIds::outlineColourId, juce::Colours::white);
    addAndMakeVisible(frequencyLabel);
}

// DECONSTRUCTOR
SimpleFilterAudioProcessorEditor::~SimpleFilterAudioProcessorEditor()
{
}

//==============================================================================
void SimpleFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);  // (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Midi Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void SimpleFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    webComponent.setBounds (0, 0, getWidth(), getHeight() / 3);
    
    // sets the position and size of the slider with arguments (x, y, width, height)
    midiVolume.setBounds (40, 30, 20, getHeight() - 60);
    frequencySlider.setBounds (getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 200);
    frequencyLabel.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 20);
    playButton.setBounds(getWidth() - 120, 30, 100, 30);
}

void SimpleFilterAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    audioProcessor.noteOnVel = midiVolume.getValue();
}
