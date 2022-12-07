/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
InterstellarAudioProcessor::InterstellarAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
treeState (*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

InterstellarAudioProcessor::~InterstellarAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout InterstellarAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    auto normRangeLPF = juce::NormalisableRange<float>(200,20000);
    normRangeLPF.setSkewForCentre(1000);
    
    auto normRangeHPF = juce::NormalisableRange<float>(10,2000);
    normRangeHPF.setSkewForCentre(250);
    
    auto normRangeMod = juce::NormalisableRange<float>(0,10);
    normRangeMod.setSkewForCentre(2);
    
    auto normRangeDelay = juce::NormalisableRange<float>(0,MAX_DELAY_MS);
    normRangeDelay.setSkewForCentre(500);
    
    auto normRangeVolume = juce::NormalisableRange<float>(-120,0);
    normRangeVolume.setSkewForCentre(-18);
    
    juce::StringArray algorithmList = {"Default","Reverse","Sped-up","Slowed"};
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>("dry", "Dry",  normRangeVolume, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("wet", "Wet",  normRangeVolume, -14));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("delay", "Delay",  normRangeDelay, 500));
    params.push_back (std::make_unique<juce::AudioParameterInt>("feedback", "Feedback",  0, 100, 50));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("lpf", "Lpf",  normRangeLPF, 10000));
    params.push_back (std::make_unique<juce::AudioParameterBool>("slidermode", "Slidermode",  false));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("hpf", "Hpf",  normRangeHPF, 150));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("warp", "Warp",  0, 100, 100));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("density", "Density",  0, 100, 100));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("diffusion", "Diffusion",  0, 100, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("modFreq", "ModFreq",  normRangeMod, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("modDepth", "ModDepth",  0, 100, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool>("clear", "Clear",  false));
    params.push_back (std::make_unique<juce::AudioParameterChoice>("algorithm", "Algorithm", algorithmList, 0));
    

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String InterstellarAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool InterstellarAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool InterstellarAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool InterstellarAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double InterstellarAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int InterstellarAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int InterstellarAudioProcessor::getCurrentProgram()
{
    return 0;
}

void InterstellarAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String InterstellarAudioProcessor::getProgramName (int index)
{
    return {};
}

void InterstellarAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void InterstellarAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;
    
    effect.setMaxDelay(sampleRate, MAX_DELAY_MS);
    
    utilities.mapNotesToMS(bpm,MAX_DELAY_NOTE, MAX_DELAY_MS);
}

void InterstellarAudioProcessor::reset()
{
    
    float delayValue = treeState.getRawParameterValue ("delay")->load();
    float warp = treeState.getRawParameterValue ("warp")->load()/100;
    float modFreq = treeState.getRawParameterValue ("modFreq")->load();
    effect.reset(delayValue, warp, modFreq);
}

void InterstellarAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool InterstellarAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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


void InterstellarAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    playHead = this->getPlayHead();
    playHead->getCurrentPosition (currentPositionInfo);

    bool delaySliderInNoteMode = treeState.getRawParameterValue ("slidermode")->load();
    if (bpm != currentPositionInfo.bpm && delaySliderInNoteMode)
    {
        bpm = currentPositionInfo.bpm;
        utilities.mapNotesToMS(bpm, MAX_DELAY_NOTE, MAX_DELAY_MS);
    }
    

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        
        if (clearValue != treeState.getRawParameterValue ("clear")->load())
        {
            clearValue = !clearValue;
            effect.clearBuffer();
        }
        
        effect.setParameters(treeState.getRawParameterValue ("delay")->load(),
                             treeState.getRawParameterValue ("feedback")->load()/100,
                             treeState.getRawParameterValue ("warp")->load()/100,
                             treeState.getRawParameterValue ("density")->load()/100,
                             treeState.getRawParameterValue ("diffusion")->load()/100,
                             treeState.getRawParameterValue ("lpf")->load(),
                             treeState.getRawParameterValue ("hpf")->load(),
                             treeState.getRawParameterValue ("modFreq")->load(),
                             treeState.getRawParameterValue ("modDepth")->load()/100,
                             treeState.getRawParameterValue ("wet")->load(),
                             treeState.getRawParameterValue ("dry")->load(),
                             treeState.getRawParameterValue ("algorithm")->load());
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            
            channelData[sample] = effect.processAudioSample(channelData[sample], channel);
        }
    }
}

//==============================================================================
bool InterstellarAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* InterstellarAudioProcessor::createEditor()
{
    return new InterstellarAudioProcessorEditor (*this);
}

//==============================================================================
void InterstellarAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void InterstellarAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
         
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (treeState.state.getType()))
            treeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InterstellarAudioProcessor();
}
