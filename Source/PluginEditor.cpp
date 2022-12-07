/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
InterstellarAudioProcessorEditor::InterstellarAudioProcessorEditor (InterstellarAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 400);
    
    drySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    drySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    drySlider.addListener(this);
    addAndMakeVisible(&drySlider);
       
    dryValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "dry", drySlider);
    
    drySlider.textFromValueFunction = nullptr;
    drySlider.setTextValueSuffix(" dB");
    drySlider.setNumDecimalPlacesToDisplay(1);
       
    dryLabel.attachToComponent(&drySlider, false);
    dryLabel.setText("Dry", juce::dontSendNotification);
    dryLabel.setJustificationType(juce::Justification::centred);
    

    
    
    wetSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    wetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    wetSlider.addListener(this);
    addAndMakeVisible(&wetSlider);
       
    wetValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "wet", wetSlider);
    
    wetSlider.textFromValueFunction = nullptr;
    wetSlider.setTextValueSuffix(" dB");
    wetSlider.setNumDecimalPlacesToDisplay(1);
       
    wetLabel.attachToComponent(&wetSlider, false);
    wetLabel.setText("Wet", juce::dontSendNotification);
    wetLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    delaySlider_ms.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delaySlider_ms.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    delaySlider_ms.addListener(this);
    addAndMakeVisible(&delaySlider_ms);
    
    delaySlider_notes.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delaySlider_notes.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    delaySlider_notes.addListener(this);
    delaySlider_notes.setTextValueSuffix(" note");
    delaySlider_notes.setRange(0, MAX_DELAY_NOTE);
    addChildComponent(&delaySlider_notes);
       
    delayValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "delay", delaySlider_ms);
    
    delaySlider_ms.textFromValueFunction = nullptr;
    delaySlider_ms.setTextValueSuffix(" ms");
    delaySlider_ms.setNumDecimalPlacesToDisplay(0);
       
    delayLabel.attachToComponent(&delaySlider_ms, false);
    delayLabel.setText("Delay", juce::dontSendNotification);
    delayLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    sliderModeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    sliderModeButton.setClickingTogglesState (true);
    
    addAndMakeVisible(&sliderModeButton);
        
    sliderModeButtonValue = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.treeState, "slidermode", sliderModeButton);
    sliderModeButton.onClick = [this](){ swapSliderMode(); };
    
    
    
//    clearButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    clearButton.setClickingTogglesState (true);
    
    addAndMakeVisible(&clearButton);
        
    clearButtonValue = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.treeState, "clear", clearButton);
    
    
    
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    feedbackSlider.addListener(this);
    feedbackSlider.setTextValueSuffix(" %");
    addAndMakeVisible(&feedbackSlider);
       
    feedbackValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "feedback", feedbackSlider);
       
    feedbackLabel.attachToComponent(&feedbackSlider, false);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setVisible(true);
    feedbackLabel.setBounds(feedbackSlider.getX()-16, feedbackSlider.getY()+11, 70, 50);
    
    
    
    
    lpfSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lpfSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    lpfSlider.addListener(this);
    
    addAndMakeVisible(&lpfSlider);
       
    lpfValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "lpf", lpfSlider);
    
    lpfSlider.textFromValueFunction = nullptr;
    lpfSlider.setTextValueSuffix(" Hz");
    lpfSlider.setNumDecimalPlacesToDisplay(0);
    
    lpfLabel.attachToComponent(&lpfSlider, false);
    lpfLabel.setText("High Cut", juce::dontSendNotification);
    lpfLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    
    hpfSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    hpfSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    hpfSlider.addListener(this);
    addAndMakeVisible(&hpfSlider);
       
    hpfValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "hpf", hpfSlider);
    
    hpfSlider.textFromValueFunction = nullptr;
    hpfSlider.setTextValueSuffix(" Hz");
    hpfSlider.setNumDecimalPlacesToDisplay(0);
       
    hpfLabel.attachToComponent(&hpfSlider, false);
    hpfLabel.setText("Low Cut", juce::dontSendNotification);
    hpfLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    
    warpSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    warpSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    warpSlider.addListener(this);
    addAndMakeVisible(&warpSlider);
       
    warpValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "warp", warpSlider);
    
    warpSlider.textFromValueFunction = nullptr;
    warpSlider.setTextValueSuffix(" %");
    warpSlider.setNumDecimalPlacesToDisplay(1);
       
    warpLabel.attachToComponent(&warpSlider, false);
    warpLabel.setText("Warp", juce::dontSendNotification);
    warpLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    
    
    densitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    densitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    densitySlider.addListener(this);
    addAndMakeVisible(&densitySlider);
       
    densityValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "density", densitySlider);
    
    densitySlider.textFromValueFunction = nullptr;
    densitySlider.setNumDecimalPlacesToDisplay(1);
    densitySlider.setTextValueSuffix(" %");
       
    densityLabel.attachToComponent(&densitySlider, false);
    densityLabel.setText("Density", juce::dontSendNotification);
    densityLabel.setBounds(densitySlider.getX()+densitySlider.getWidth()*.70, densitySlider.getY()+11, 70, 50);
    
    
    
    
    
    diffusionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    diffusionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    diffusionSlider.addListener(this);
    addAndMakeVisible(&diffusionSlider);
       
    diffusionValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "diffusion", diffusionSlider);
    
    diffusionSlider.textFromValueFunction = nullptr;
    diffusionSlider.setNumDecimalPlacesToDisplay(1);
    diffusionSlider.setTextValueSuffix(" %");
       
    diffusionLabel.attachToComponent(&diffusionSlider, false);
    diffusionLabel.setText("Diffusion", juce::dontSendNotification);
    diffusionLabel.setBounds(diffusionSlider.getX()-16, diffusionSlider.getY()+11, 70, 50);
    
    
    
    
    
    modFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    modFreqSlider.addListener(this);
    addAndMakeVisible(&modFreqSlider);
       
    modFreqValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "modFreq", modFreqSlider);
    
    modFreqSlider.textFromValueFunction = nullptr;
    modFreqSlider.setTextValueSuffix(" Hz");
    modFreqSlider.setNumDecimalPlacesToDisplay(2);
       
    modFreqLabel.attachToComponent(&modFreqSlider, false);
    modFreqLabel.setText("Mod Rate", juce::dontSendNotification);
    modFreqLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    
    modDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 90, 25);
    modDepthSlider.addListener(this);
    addAndMakeVisible(&modDepthSlider);
       
    modDepthValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "modDepth", modDepthSlider);
       
    modDepthSlider.textFromValueFunction = nullptr;
    modDepthSlider.setNumDecimalPlacesToDisplay(1);
    modDepthSlider.setTextValueSuffix(" %");
    
    modDepthLabel.attachToComponent(&modDepthSlider, false);
    modDepthLabel.setText("Mod Depth", juce::dontSendNotification);
    modDepthLabel.setJustificationType(juce::Justification::centred);
    
    
    
    
    
    
    addAndMakeVisible (algorithmDropdown);
    algorithmDropdown.addItem ("Default",  1);
    algorithmDropdown.addItem ("Reverse",   2);
    algorithmDropdown.addItem ("Sped-up", 3);
    algorithmDropdown.addItem ("Slowed", 4);
    
    algorithmValue = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.treeState, ("algorithm"), algorithmDropdown);
    
    
    
    
    
    
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::gold);
    getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::grey);
    getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
}

InterstellarAudioProcessorEditor::~InterstellarAudioProcessorEditor()
{
}

//==============================================================================
void InterstellarAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto backgroundImage = juce::ImageCache::getFromMemory (BinaryData::InterstellarBackground_jpg, BinaryData::InterstellarBackground_jpgSize);
    g.drawImage(backgroundImage, 0, 0, getWidth(), getHeight(), 0, 0, 454, 237);
    
    auto orangeHue = juce::Colours::orange.getHue();
    auto goldHue = juce::Colours::gold.getHue();
    
    g.setColour(juce::Colour::fromHSV (orangeHue, 0.65f, 0.9f, 1.0f));
    g.drawLine(delayLabel.getX()+delayLabel.getWidth(), 29, delayLabel.getX()+delayLabel.getWidth(), getHeight()-24, 3);
    
    g.setColour(juce::Colour::fromHSV (goldHue, 1.f, 1.0f, 1.0f));
    g.drawLine(modFreqLabel.getX()+modFreqLabel.getWidth(), 29, modFreqLabel.getX()+modFreqLabel.getWidth(), getHeight()-24, 3);
    
//    g.setColour(juce::Colour::fromHSV (goldHue, 1.0f, 1.0f, 1.0f));
    g.drawLine((delayLabel.getX()+delayLabel.getWidth())*3, 29, (delayLabel.getX()+delayLabel.getWidth())*3, getHeight()-24, 3);
    
    g.setColour(juce::Colour::fromHSV (orangeHue, 0.65f, 0.9f, 1.0f));
    g.drawLine(hpfLabel.getX()+delayLabel.getWidth(), 29, hpfLabel.getX()+hpfLabel.getWidth(), getHeight()-24, 3);
    
}

void InterstellarAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //    Dividing the GUI window into a grid of 10x10
        auto x = getWidth()/20;
        auto y = getHeight()/20;

    imageComponent.setBounds(0, 0, getWidth(), getHeight());
    
    sliderModeButton.setBounds(0.25*x,     9.45*y,     1.2*x,   1.3*y);
    algorithmDropdown.setBounds(1.6*x,     9.45*y,     2.2*x,   1.3*y);
      delaySlider_ms.setBounds(0,        3*y,     4*x,     6*y);
   delaySlider_notes.setBounds(delaySlider_ms.getBounds());
       modFreqSlider.setBounds(4*x,      3*y,     4*x,     6*y);
      feedbackSlider.setBounds(8.5*x,      2*y,     4*x,     5*y);
        densitySlider.setBounds(7.5*x,     7.5*y,     4*x,     5*y);
     diffusionSlider.setBounds(8.5*x,      13*y,     4*x,     5*y);
           lpfSlider.setBounds(12*x,     3*y,     4*x,     6*y);
           drySlider.setBounds(16*x,     3*y,     4*x,     6*y);
          warpSlider.setBounds(0,       12*y,     4*x,     6*y);
      modDepthSlider.setBounds(4*x,     12*y,     4*x,     6*y);
           hpfSlider.setBounds(12*x,    12*y,     4*x,     6*y);
        wetSlider.setBounds(16*x,    12*y,     4*x,     6*y);
         clearButton.setBounds(17*x,   9.45*y,     2*x,     1.3*y);
    
}

void InterstellarAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider==&delaySlider_notes && sliderModeButton.getToggleState() == true){
        //lock slider to integer values
        delaySlider_notes.setValue(round(delaySlider_notes.getValue()));
        delaySlider_ms.setValue(audioProcessor.utilities.getMSFromNote(delaySlider_notes.getValue()));
        if (delaySlider_ms.getValue() == 0)
            sliderModeButton.setButtonText("");
    }
}

void InterstellarAudioProcessorEditor::swapSliderMode()
{
    if (sliderModeButton.getToggleState() == true)
    {
        delaySlider_notes.textFromValueFunction = [](int note){
            if (note==0) return "0";
            if (note==1) return "1/32";
            if (note==2) return "1/16T";
            if (note==3) return "1/32D";
            if (note==4) return "1/16";
            if (note==5) return "1/8T";
            if (note==6) return "1/16D";
            if (note==7) return "1/8";
            if (note==8) return "1/4T";
            if (note==9) return "1/8D";
            if (note==10) return "1/4";
            if (note==11) return "1/2T";
            if (note==12) return "1/4D";
            if (note==13) return "1/2";
            if (note==14) return "1/1T";
            if (note==15) return "1/2D";
            if (note==16) return "1/1";
            else return "?";
        };
        audioProcessor.utilities.mapNotesToMS(audioProcessor.bpm, MAX_DELAY_NOTE, MAX_DELAY_MS);
        delaySlider_notes.setValue(audioProcessor.utilities.getNoteFromMS(delaySlider_ms.getValue()));
        sliderModeButton.setButtonText("note");
        
        delayLabel.attachToComponent(&delaySlider_notes, false);
        
        delaySlider_notes.setVisible(true);
        delaySlider_ms.setVisible(false);
    }
    else
    {
        sliderModeButton.setButtonText("ms");
        
        delayLabel.attachToComponent(&delaySlider_ms, false);
        
        delaySlider_notes.setVisible(false);
        delaySlider_ms.setVisible(true);
    }
}


