/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class Test_vstAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Slider::Listener, private juce::Timer
{
public:
    Test_vstAudioProcessorEditor (Test_vstAudioProcessor&);
    ~Test_vstAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override
    {
        auto flg = audioProcessor.getFFTReady();
        if (flg)
        {
            drawNextFrameOfSpectrum();
            audioProcessor.setFFTReady(false);
            repaint();
        }
    }

    void drawNextFrameOfSpectrum()
    {
        window.multiplyWithWindowingTable(audioProcessor.getFFTDataPtr(), audioProcessor.fftSize);
        forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.getFFTDataPtr());

        auto mindB = -100.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, audioProcessor.fftSize / 2, (int)(skewedProportionX * (float)audioProcessor.fftSize * 0.5));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, 
                                                 juce::Decibels::gainToDecibels(audioProcessor.getFFTDataSample(fftDataIndex))
                                                 - juce::Decibels::gainToDecibels((float)audioProcessor.fftSize)), 
                                                 mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;
        }
    }

    void drawFrame(juce::Graphics& g) 
    {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto scopeHeight = height / 2 - 100;
            auto scopeWidth = width;

            g.drawLine({ (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, scopeWidth),
                                juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)scopeHeight, 0.0f),
                         (float)juce::jmap(i, 0, scopeSize - 1, 0, scopeWidth),
                                juce::jmap(scopeData[i], 0.0f, 1.0f, (float)scopeHeight, 0.0f) });
        }
    }

    static constexpr auto scopeSize = 512;

private:
    void sliderValueChanged(juce::Slider* slider) override;

    Test_vstAudioProcessor& audioProcessor;

    juce::Slider lowFrequency;
    juce::Slider highFrequency;

    // Window size
    int height = 500;
    int width = 600;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    float scopeData[scopeSize];


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Test_vstAudioProcessorEditor)
};
