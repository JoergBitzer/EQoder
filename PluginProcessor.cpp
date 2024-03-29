#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EQoderAudioProcessor::EQoderAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{

    // It is important to have the paramter set in the m_paramVector
    // e.g. (a better solution is to ue this function in the components)
/*        m_paramVector.push_back(std::make_unique<AudioParameterFloat>(paramHpCutoff.ID,
        paramHpCutoff.name,
        NormalisableRange<float>(paramHpCutoff.minValue, paramHpCutoff.maxValue),
        paramHpCutoff.defaultValue,
        paramHpCutoff.unitName,
        AudioProcessorParameter::genericParameter,
        [](float value, int MaxLen) { value = int(exp(value) * 10) * 0.1;  return (String(value, MaxLen) + " Hz"); },
        [](const String& text) {return text.getFloatValue(); }));
//*/
    // this is just a placeholder (necessary for compiling/testing the template)
    m_eqoderparamter.addParameter(m_paramVector);
    m_envparameter.addParameter(m_paramVector,0);

    m_parameterVTS = std::make_unique<AudioProcessorValueTreeState>(*this, nullptr, Identifier("EquoderVTS"),
        AudioProcessorValueTreeState::ParameterLayout(m_paramVector.begin(), m_paramVector.end()));

	m_presets.setAudioValueTreeState(m_parameterVTS.get());
#ifdef FACTORY_PRESETS    
    m_presets.DeployFactoryPresets();
#endif
    // if needed add categories
    // m_presets.addCategory(StringArray("Unknown", "Soft", "Medium", "Hard", "Experimental"));
    // m_presets.addCategory(JadeSynthCategories);
	m_presets.loadfromFileAllUserPresets();    

    m_Eqoder.prepareParameter(m_parameterVTS);

}

EQoderAudioProcessor::~EQoderAudioProcessor()
{

}

//==============================================================================
const juce::String EQoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EQoderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EQoderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EQoderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EQoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EQoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EQoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EQoderAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String EQoderAudioProcessor::getProgramName (int index)
{
    if (index == 0)
        return "Init";
    //juce::ignoreUnused (index);
    return {};
}

void EQoderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void EQoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (samplesPerBlock);
    m_fs = sampleRate;
    m_Eqoder.prepareToPlay(sampleRate,samplesPerBlock);
    m_meter.prepareToPlay(sampleRate,samplesPerBlock);
}

void EQoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool EQoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EQoderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
#if WITH_MIDIKEYBOARD  
	m_keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
#else
    juce::ignoreUnused (midiMessages);
#endif
    juce::ScopedNoDenormals noDenormals;
    
    // Here the plugin
    //m_protect.enter();
    m_Eqoder.processBlock(buffer,midiMessages);
    m_meter.analyseData(buffer);
#if WITH_MIDIKEYBOARD     
    midiMessages.clear();
#endif
    //m_protect.exit();
}

//==============================================================================
bool EQoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EQoderAudioProcessor::createEditor()
{
    return new EQoderAudioProcessorEditor (*this);
}

//==============================================================================
void EQoderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	auto state = m_parameterVTS->copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);

}

void EQoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
 	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(m_parameterVTS->state.getType()))
        {

            String presetname(xmlState->getStringAttribute("presetname"));
            m_presets.setCurrentPresetName(presetname);
			m_parameterVTS->replaceState(ValueTree::fromXml(*xmlState));
        }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EQoderAudioProcessor();
}
