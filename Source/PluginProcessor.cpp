#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleLinearFilterAudioProcessor::SimpleLinearFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), state(*this, nullptr, "parameters", createParameters())
#endif
{
}

SimpleLinearFilterAudioProcessor::~SimpleLinearFilterAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleLinearFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleLinearFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleLinearFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleLinearFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleLinearFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleLinearFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleLinearFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleLinearFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleLinearFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleLinearFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleLinearFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    sineWaves.resize(getTotalNumOutputChannels());
    for (auto& wave : sineWaves) {
        wave.prepare(sampleRate);
    }

    frequencyParam = state.getRawParameterValue("freqHz");
    playParam = state.getRawParameterValue("play");
}

void SimpleLinearFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleLinearFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleLinearFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float freq = frequencyParam->load();
    const bool shouldBePlaying = static_cast<bool>(playParam->load());
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* output = buffer.getWritePointer(channel);
        sineWaves[channel].setFrequency(freq);
        sineWaves[channel].setAmplitude(shouldBePlaying ? 0.4f : 0.0f);
        sineWaves[channel].process(output, buffer.getNumSamples());
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//
//        // ..do something to the data...
//    }
}

//==============================================================================
bool SimpleLinearFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleLinearFilterAudioProcessor::createEditor()
{
    return new SimpleLinearFilterAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleLinearFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleLinearFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleLinearFilterAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleLinearFilterAudioProcessor::createParameters()
{
    return {
        std::make_unique<juce::AudioParameterFloat> (  // why use make_unique? because the createParameters function needs to return a ParameterLayout object, which is a vector of unique pointers to RangedAudioParameter objects. By using make_unique, we can create a new AudioParameterFloat object and automatically wrap it in a unique pointer, which is then added to the ParameterLayout vector.
            juce::ParameterID { "freqHz" },
            "Frequency",
            20.0f,
            20000.0f,
            220.0f
        ),
        std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { "play" },
            "Play",
            true
        )
    };
}