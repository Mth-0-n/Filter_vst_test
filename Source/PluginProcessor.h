/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class Test_vstAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Test_vstAudioProcessor();
    ~Test_vstAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void updateFilter(); // Filter parameter update

    // audioBuffer data keep
    void pushNextSampleIntoFifo(float sample) noexcept
    {
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady) 
            {
                std::fill(fftData.begin(), fftData.end(), 0.0f);
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        fifo[fifoIndex++] = sample;
    }

    bool getFFTReady()
    {
        return nextFFTBlockReady;
    }
    void setFFTReady(bool b) 
    {
        nextFFTBlockReady = b;
    }
    float* getFFTDataPtr() 
    {
        return fftData.data();
    }
    float getFFTDataSample(int sample)
    {
        return fftData[sample];
    }

    double lowPassFreq;
    double highPassFreq;

    static constexpr auto fftOrder = 11;
    static constexpr auto fftSize = 1 << fftOrder;


private:
    float lastSampleRate;

    juce::dsp::ProcessorDuplicator < juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float> > lowPassFilter;
    juce::dsp::ProcessorDuplicator < juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float> > highPassFilter;

    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;


    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Test_vstAudioProcessor)
};
