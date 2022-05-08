/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Test_vstAudioProcessorEditor::Test_vstAudioProcessorEditor (Test_vstAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), forwardFFT(p.fftOrder), window(p.fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    setSize (width, height);

    lowFrequency.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    lowFrequency.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 25);
    lowFrequency.setRange (25.0, 22000.0, 0.01);  // 25~22000Hz
    lowFrequency.setValue (20000);            // default 20000Hz
    lowFrequency.setSkewFactorFromMidPoint(2048.0); // 
    lowFrequency.addListener(this);
    addAndMakeVisible(&lowFrequency);

    highFrequency.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    highFrequency.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 25);
    highFrequency.setRange (0.0, 18000.0, 0.01);  // 0~18000Hz
    highFrequency.setValue (25);            // default 25Hz
    lowFrequency.setSkewFactorFromMidPoint(2048.0);
    highFrequency.addListener(this);
    addAndMakeVisible(&highFrequency);

    setOpaque(true);
    startTimerHz(30);
}

Test_vstAudioProcessorEditor::~Test_vstAudioProcessorEditor()
{
}

//==============================================================================
void Test_vstAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);

    g.drawText("LowPassFilter", width/12, height / 3, 150, 150, juce::Justification::centred, true);
    g.drawText("HighPassFilter", width * 2 / 3, height / 3, 150, 150, juce::Justification::centred, true);

    g.setOpacity(1.0f);
    drawFrame(g);
}

void Test_vstAudioProcessorEditor::resized()
{
    lowFrequency.setBounds(width / 12, height / 2, 150, 150);
    highFrequency.setBounds(width * 2 / 3, height / 2, 150, 150);
}

void Test_vstAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &lowFrequency) {
        audioProcessor.lowPassFreq = lowFrequency.getValue();
    }
    if (slider == &highFrequency) {
        // 100Hz未満で音を出し続けたままsliderを操作するとfilterが発散する
        // 専用の処理を加える必要あり
        audioProcessor.highPassFreq = highFrequency.getValue(); 
    }
}

