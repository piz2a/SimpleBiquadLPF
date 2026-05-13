#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleFilterAudioProcessor::SimpleFilterAudioProcessor()
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

SimpleFilterAudioProcessor::~SimpleFilterAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    filters.resize(getTotalNumOutputChannels());
    for (auto& filter : filters) {
        filter.prepare(sampleRate);
        filter.setCutoffFrequency(1000.0f);
        filter.setQ(0.707f);
        filter.setCoefficients();  // mandatory
    }

    frequencyParam = state.getRawParameterValue("freqHz");
    resonanceParam = state.getRawParameterValue("resonance");
    playParam = state.getRawParameterValue("play");
    smoothedFreq.reset(sampleRate, 0.05); // 50ms 동안 부드럽게 변화
    smoothedFreq.setCurrentAndTargetValue(frequencyParam->load());
}

void SimpleFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    // If your algorithm only overwrites some of the output channels, make sure to
    // keep this code to avoid leaving garbage in the remaining output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float freq = frequencyParam->load();
    smoothedFreq.setTargetValue(freq);
    float currentFreq = smoothedFreq.getNextValue();
    smoothedFreq.skip(numSamples - 1);

    const float res = resonanceParam->load();
    const float q = 0.707f * juce::Decibels::decibelsToGain (res);  // Convert dB to linear gain. 0 dB = 0.707
    smoothedQ.setTargetValue(q);
    float currentQ = smoothedQ.getNextValue();
    smoothedQ.skip(numSamples - 1);
    const bool shouldBePlaying = static_cast<bool>(playParam->load());

    /*static float lastFreq = -1.0f;
    if (std::abs(freq - lastFreq) > 0.001f) {
        for (auto& filter : filters) {
            filter.setCutoffFrequency(freq);
            // shouldBePlaying is not used yet
            filter.setCoefficients();
        }
        lastFreq = freq;
    }*/

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);

        filters[channel].setCutoffFrequency(currentFreq);
        filters[channel].setQ(currentQ);
        filters[channel].setCoefficients();

        if (shouldBePlaying) {
            filters[channel].process(channelData, numSamples);
        } else {
            // bypass
        }
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
bool SimpleFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleFilterAudioProcessor::createEditor()
{
    return new SimpleFilterAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleFilterAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleFilterAudioProcessor::createParameters()
{
    return {
        std::make_unique<juce::AudioParameterFloat> (  // why use make_unique? because the createParameters function needs to return a ParameterLayout object, which is a vector of unique pointers to RangedAudioParameter objects. By using make_unique, we can create a new AudioParameterFloat object and automatically wrap it in a unique pointer, which is then added to the ParameterLayout vector.
            juce::ParameterID { "freqHz", 1 },
            "Frequency",
            20.0f,
            20000.0f,
            220.0f
        ),
        std::make_unique<juce::AudioParameterFloat> (  // why use make_unique? because the createParameters function needs to return a ParameterLayout object, which is a vector of unique pointers to RangedAudioParameter objects. By using make_unique, we can create a new AudioParameterFloat object and automatically wrap it in a unique pointer, which is then added to the ParameterLayout vector.
            juce::ParameterID { "resonance", 1 },
            "Resonance",
            0.0f,
            12.0f,
            0.0f
        ),
        std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { "play", 1 },
            "Play",
            true
        )
    };
}