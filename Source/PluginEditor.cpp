#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <WebViewFiles.h>

namespace
{
    std::vector<std::byte> streamToVector(juce::InputStream &stream)
    {
        using namespace juce;
        const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
        std::vector<std::byte> result(sizeInBytes);
        stream.setPosition(0);
        [[maybe_unused]] const auto bytesRead =
            stream.read(result.data(), result.size());
        jassert(bytesRead == static_cast<ssize_t>(sizeInBytes));
        return result;
    }

    static const char *getMimeForExtension(const juce::String &extension)
    {
        static const std::unordered_map<juce::String, const char *> mimeMap = {
            {{"htm"}, "text/html"},
            {{"html"}, "text/html"},
            {{"txt"}, "text/plain"},
            {{"jpg"}, "image/jpeg"},
            {{"jpeg"}, "image/jpeg"},
            {{"svg"}, "image/svg+xml"},
            {{"ico"}, "image/vnd.microsoft.icon"},
            {{"json"}, "application/json"},
            {{"png"}, "image/png"},
            {{"css"}, "text/css"},
            {{"map"}, "application/json"},
            {{"js"}, "text/javascript"},
            {{"woff2"}, "font/woff2"}};

        if (const auto it = mimeMap.find(extension.toLowerCase());
            it != mimeMap.end())
            return it->second;

        jassertfalse;
        return "";
    }

    juce::Identifier getExampleEventId()
    {
        static const juce::Identifier id{"exampleEvent"};
        return id;
    }

#ifndef ZIPPED_FILES_PREFIX
#error \
    "You must provide the prefix of zipped web UI files' paths, e.g., 'public/', in the ZIPPED_FILES_PREFIX compile definition"
#endif

    /**
     * @brief Get a web UI file as bytes
     *
     * @param filepath path of the form "index.html", "js/index.js", etc.
     * @return std::vector<std::byte> with bytes of a read file or an empty vector
     * if the file is not contained in webview_files.zip
     */
    std::vector<std::byte> getWebViewFileAsBytes(const juce::String &filepath)
    {
        juce::MemoryInputStream zipStream{webview_files::webview_files_zip,
                                          webview_files::webview_files_zipSize,
                                          false};
        juce::ZipFile zipFile{zipStream};

        // print all entries in the zip file for debugging
        std::cout << "Zip file contains the following entries:" << std::endl;
        for (int i = 0; i < zipFile.getNumEntries(); ++i)
        {
            if (const auto *entry = zipFile.getEntry(i))
            {
                std::cout << " - " << entry->filename << std::endl;
            }
        }

        if (auto *zipEntry = zipFile.getEntry(ZIPPED_FILES_PREFIX + filepath))
        {
            const std::unique_ptr<juce::InputStream> entryStream{
                zipFile.createStreamForEntry(*zipEntry)};

            if (entryStream == nullptr)
            {
                jassertfalse;
                return {};
            }

            return streamToVector(*entryStream);
        }

        return {};
    }

    constexpr auto LOCAL_DEV_SERVER_ADDRESS = "http://127.0.0.1:5173";
} // namespace

//==============================================================================
// CONSTRUCTOR
SimpleFilterAudioProcessorEditor::SimpleFilterAudioProcessorEditor(SimpleFilterAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      freqSliderAttachment(audioProcessor.getState(), "freqHz", frequencySlider),
      bypassButtonAttachment(audioProcessor.getState(), "bypass", bypassButton),
      webComponent(WebBrowserComponent::Options{}
                       .withBackend(WebBrowserComponent::Options::Backend::webview2)
                       .withWinWebView2Options(WebBrowserComponent::Options::WinWebView2{}
                                                   .withUserDataFolder(File::getSpecialLocation(File::SpecialLocationType::tempDirectory)))
                       .withResourceProvider([this](const auto &url)
                                             { return getResource(url); })
                       // .withInitialisationData("vendor", JUCE_COMPANY_NAME),
                       .withOptionsFrom(freqRelay)
                       .withOptionsFrom(resonanceRelay)
                       .withOptionsFrom(bypassRelay)
                       .withNativeIntegrationEnabled() // Necessary
      )
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(webComponent);
#if JUCE_DEBUG
    // Debug mode: Load from local development server for hot-reloading
    webComponent.goToURL(LOCAL_DEV_SERVER_ADDRESS);
#else
    // Release mode: Load from bundled resources
    webComponent.goToURL(WebBrowserComponent::getResourceProviderRoot());
#endif

    // This is where our plugin’s editor size is set.
    setSize(480, 320);

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
void SimpleFilterAudioProcessorEditor::paint(Graphics &g)
{
}

void SimpleFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    webComponent.setBounds(0, 0, getWidth(), getHeight());

    // sets the position and size of the slider with arguments (x, y, width, height)
    // frequencySlider.setBounds (getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 200);
    // frequencyLabel.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 20);
    bypassButton.setBounds(getWidth() - 120, 30, 100, 30);
}

void SimpleFilterAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
}

auto SimpleFilterAudioProcessorEditor::getResource(const juce::String &url) const
    -> std::optional<juce::WebBrowserComponent::Resource>
{
    std::cout << "ResourceProvider called with " << url << std::endl;

    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    if (resourceToRetrieve == "outputLevel.json")
    {
        juce::DynamicObject::Ptr levelData{new juce::DynamicObject{}};
        // levelData->setProperty("left", audioProcessor.outputLevelLeft.load());
        const auto jsonString = juce::JSON::toString(levelData.get());
        juce::MemoryInputStream stream{jsonString.getCharPointer(),
                                       jsonString.getNumBytesAsUTF8(), false};
        return juce::WebBrowserComponent::Resource{
            streamToVector(stream), juce::String{"application/json"}};
    }

    const auto resource = getWebViewFileAsBytes(resourceToRetrieve);
    if (!resource.empty())
    {
        const auto extension =
            resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return juce::WebBrowserComponent::Resource{std::move(resource), getMimeForExtension(extension)};
    } else {
        std::cout << "Resource " << resourceToRetrieve << " not found in zip" << std::endl;
    }

    return std::nullopt;
}