/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
//#include "myLookAndFeel.h"

//==============================================================================
/**
*/
class InterstellarAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    InterstellarAudioProcessorEditor (InterstellarAudioProcessor&);
    ~InterstellarAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider* sliderGain) override;
    
    void swapSliderMode();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    InterstellarAudioProcessor& audioProcessor;
    
    juce::Slider drySlider;
    juce::Label dryLabel;
    
    juce::Slider wetSlider;
    juce::Label wetLabel;
    
    juce::Slider delaySlider_ms;
    juce::Label delayLabel;
    
    juce::Slider delaySlider_notes;
    
    juce::Slider feedbackSlider;
    juce::Label feedbackLabel;
    
    juce::Slider lpfSlider;
    juce::Label lpfLabel;
    
    juce::TextButton sliderModeButton {"ms"};
    
    juce::Slider hpfSlider;
    juce::Label hpfLabel;
    
    juce::Slider warpSlider;
    juce::Label warpLabel;
    
    juce::Slider densitySlider;
    juce::Label densityLabel;
    
    juce::Slider diffusionSlider;
    juce::Label diffusionLabel;
    
    juce::Slider modFreqSlider;
    juce::Label modFreqLabel;
    
    juce::Slider modDepthSlider;
    juce::Label modDepthLabel;
    
    juce::TextButton clearButton {"Clear"};
    
    juce::ComboBox algorithmDropdown;
    
    juce::ImageComponent imageComponent;
    
//    juce::String juce::Slider::getTextFromValue(double value)
//    {
//        return String (value, getNumDecimalPlacesToDisplay());
//    }
    
//    myLookAndFeelV1 myLookAndFeelV1;
//    myLookAndFeelV2 myLookAndFeelV2;
//    myLookAndFeelV3 myLookAndFeelV3;
    
    
public:
    
    juce::Label bpmLabel;
    
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> dryValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> wetValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> delayValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> feedbackValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> lpfValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> hpfValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> warpValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> densityValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> modFreqValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> modDepthValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> diffusionValue;
    
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> sliderModeButtonValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> clearButtonValue;
    
    std::unique_ptr <juce::AudioProcessorValueTreeState::ComboBoxAttachment> algorithmValue;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterstellarAudioProcessorEditor)
};
