/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


#include "Utilities.h"
#include "EffectMaster.h"

#define MAX_DELAY_MS 2000
#define MAX_DELAY_NOTE 16

//==============================================================================
/**
*/
class InterstellarAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    InterstellarAudioProcessor();
    ~InterstellarAudioProcessor() override;

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
    
    juce::AudioProcessorValueTreeState treeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    EffectMaster effect;
    
    float lastSampleRate = 44100;
    juce::AudioPlayHead* playHead;
    juce::AudioPlayHead::CurrentPositionInfo currentPositionInfo;
    
    void reset() override;
    
    float dryData;
    float wetData;
    
    float mixValue;
    
    bool clearValue;
    
    double bpm = 120;
    
    Utilities utilities;
    
    bool clear = false;
    

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterstellarAudioProcessor)
};
