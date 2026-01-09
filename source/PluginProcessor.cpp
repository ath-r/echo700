#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_core/juce_core.h"
#include <cmath>

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    treeState(*this, nullptr, "PARAMETER", createParameterLayout()),
    pluginInstanceSettings("pluginInstanceSettings")
{
    treeState.addParameterListener("prefilter_cutoff", this);
    
    for (auto* param : getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
        {
            auto paramID = paramWithID->getParameterID();
            treeState.addParameterListener(paramID, this);
        }
    }
}

PluginProcessor::~PluginProcessor()
{
    for (auto* param : getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
        {
            auto paramID = paramWithID->getParameterID();
            treeState.removeParameterListener(paramID, this);
        }
    }
}

void PluginProcessor::parameterChanged(const juce::String& parameterId, float newValue)
{
    if (parameterId == "prefilter_cutoff")
    {
        echo700left.setPrefilterCutoff(newValue);
        echo700right.setPrefilterCutoff(newValue);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> 
                    (
                    juce::ParameterID {"prefilter_cutoff", 0},
                    "Pre LP",
                    juce::NormalisableRange<float>(1000, 14e3, 100.0, 0.5f),
                    14e3,
                    "Cutoff",
                    juce::AudioProcessorParameter::Category::genericParameter,
                    [](float value, int){ return juce::String(value) + juce::String(" Hz"); }
                    )
                );

    return {params.begin(), params.end()};
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    echo700left.setContext(Ath::Dsp::Context(sampleRate));
    echo700right.setContext(Ath::Dsp::Context(sampleRate));

    auto num = treeState.state.getNumChildren();
    for (auto param : getParameters())
    {
        param->sendValueChangedMessageToListeners(param->getValue());
    }
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto* channelDataLeft = buffer.getWritePointer(0);
    echo700left.processBlock(channelDataLeft, buffer.getNumSamples());

    auto* channelDataRight = buffer.getWritePointer(1);
    echo700right.processBlock(channelDataRight, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();

    auto xml = std::make_unique<juce::XmlElement>("pluginInstanceTree");
    auto xmlSettings = std::make_unique<juce::XmlElement>(pluginInstanceSettings);

    std::unique_ptr<juce::XmlElement> pluginValueTree (state.createXml());

    xml->addChildElement(xmlSettings.release());
    xml->addChildElement(pluginValueTree.release());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("pluginInstanceTree"))
        {
            auto state = xmlState->getChildByName(treeState.state.getType());
            if (state != nullptr) treeState.replaceState (juce::ValueTree::fromXml (*state));

            auto settings = xmlState->getChildByName("pluginInstanceSettings");
            if (settings != nullptr) pluginInstanceSettings = *settings;
        }        
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
