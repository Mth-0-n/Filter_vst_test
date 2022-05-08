/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Test_vstAudioProcessor::Test_vstAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
        #endif
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        #endif
            ), lowPassFilter(juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 22000.0, 0.1)),
               highPassFilter(juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 22000.0, 0.1))

#endif
{
}

Test_vstAudioProcessor::~Test_vstAudioProcessor()
{
}

//==============================================================================
const juce::String Test_vstAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Test_vstAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Test_vstAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Test_vstAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Test_vstAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Test_vstAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Test_vstAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Test_vstAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Test_vstAudioProcessor::getProgramName (int index)
{
    return {};
}

void Test_vstAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Test_vstAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowPassFilter.reset();
    highPassFilter.reset();

    updateFilter();

    lowPassFilter.prepare(spec);
    highPassFilter.prepare(spec);
}

void Test_vstAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Test_vstAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Test_vstAudioProcessor::updateFilter() 
{
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(lastSampleRate, lowPassFreq);
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(lastSampleRate, highPassFreq);

}

void Test_vstAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);

    updateFilter();
    
    lowPassFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    highPassFilter.process(juce::dsp::ProcessContextReplacing<float>(block));

    if (buffer.getNumChannels() > 0)
    {
        auto* channelData = buffer.getReadPointer(0);

        for (auto i = 0; i < buffer.getNumSamples(); ++i)
            pushNextSampleIntoFifo(channelData[i]);
    }
}

//==============================================================================
bool Test_vstAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Test_vstAudioProcessor::createEditor()
{
    return new Test_vstAudioProcessorEditor (*this);
}

//==============================================================================
void Test_vstAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Test_vstAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Test_vstAudioProcessor();
}
