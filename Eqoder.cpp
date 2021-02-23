
#include "Eqoder.h"


Eqoder::Eqoder()
:m_maxunits(g_NrOfFilterUnits)
{
	m_unitCounter = 0;

	for (int kk = 0; kk < g_NrOfFilterUnits; ++kk)
	{
		m_pointerPool.add(std::unique_ptr<EQoderFilterUnit>(new EQoderFilterUnit()));
	}
	m_OutGain = 1.0;
}
Eqoder::~Eqoder()
{

}

void Eqoder::prepareToPlay (double sampleRate, int samplesPerBlock,int nrofchannels)
{
    m_fs = sampleRate;
    m_maxSamples = samplesPerBlock;
	m_nrOfChannels = nrofchannels;

	m_data.clear();
}
void Eqoder::processBlock (juce::AudioBuffer<float>& data, juce::MidiBuffer& mididata)
{
    // first analyse Mididata
	
    for (const MidiMessageMetadata metadata : mididata)
    {
		 if (metadata.numBytes == 3)
		 {
			 MidiMessage msg= metadata.getMessage();
			 if(msg.isNoteOn())
			 {
				int midiNoteNr = msg.getNoteNumber();
				float Velocity = 1.0/127.0 * msg.getVelocity();
				double freqNote = msg.getMidiNoteInHertz(midiNoteNr);
				if (m_midifilterunitmap.count(midiNoteNr)) // note already on, re attack
				{
					//int unitnr = m_midifilterunitmap[midiNoteNr];
					setParameterForNewFilterUnit(midiNoteNr);
					m_midifilterunitmap[midiNoteNr]->setFundamentalFrequency(freqNote,Velocity);
				}
				else
				{
					m_unitCounter++;
					if (m_unitCounter < m_maxunits) // Resources are available
					{
						m_midifilterunitmap[midiNoteNr] = m_pointerPool.acquire();
						setParameterForNewFilterUnit(midiNoteNr);
						m_midifilterunitmap[midiNoteNr]->setFundamentalFrequency(freqNote,Velocity);
					}
					else // Steeling
					{
						m_unitCounter--;
						m_midifilterunitmap.erase(m_softestNote);
						if (!m_pointerPool.empty())
						{
							m_midifilterunitmap[midiNoteNr] = m_pointerPool.acquire();
							setParameterForNewFilterUnit(midiNoteNr);
							m_midifilterunitmap[midiNoteNr]->setFundamentalFrequency(freqNote,Velocity);
						}
						
					}
				}
			 }
 			 if(msg.isNoteOff())
			 {
 				int midiNoteNr = msg.getNoteNumber();
				if (m_midifilterunitmap.count(midiNoteNr)) // note already on, re attack
				{
					m_midifilterunitmap[midiNoteNr]->NoteOff();
				}
			 }
		 }
	}   
	// update parameter
	updateParameter();

    // Audio processing
    m_data.resize(data.getNumSamples());

    size_t totalNrChannels = data.getNumChannels();
    m_data.resize(totalNrChannels);
	for (long unsigned int channel = 0; channel < totalNrChannels; ++channel)
	{
		m_data[channel].resize(data.getNumSamples());
        auto* channelData = data.getWritePointer (channel);

        // ..do something to the data...
        for (auto idx = 0u; idx < data.getNumSamples(); idx++)
        {
            m_data[channel][idx] = channelData[idx];
        }
	}
	m_protect.enter();
	
	float minLevel = 1.f;
	int notestostop[g_NrOfFilterUnits];
	int notestopcounter(0);
	for (auto onefilterunit : m_midifilterunitmap )
	{
		onefilterunit.second->processData(m_data);
		Envelope::envelopePhases phase;
		float Level = onefilterunit.second->getEnvelopeStatus(phase);
		if (phase == Envelope::envelopePhases::Off)
		{
			notestostop[notestopcounter] = onefilterunit.first;
			notestopcounter++;
		}
		if (Level< minLevel)
		{
			minLevel = Level;
			m_softestNote = onefilterunit.first;
		}
	}
	for (auto kk = 0 ; kk < notestopcounter; ++kk)
	{
		m_midifilterunitmap.erase(notestostop[kk]);
		m_unitCounter--;
	}
	m_protect.exit();
        
	for (long unsigned int channel = 0; channel < totalNrChannels; ++channel)
	{
        auto* channelData = data.getWritePointer (channel);
		
		for (auto idx = 0u; idx < data.getNumSamples(); idx++)
        {
            channelData[idx] = m_OutGain*m_data[channel][idx];
        }
	}

}

void Eqoder::updateParameter()
{
	if (hasparameterChanged(*m_eqoderparamter.m_nrOfFilterUnits, m_eqoderparamter.m_nrOfFilterUnitsOld))
	{
		m_maxunits = m_eqoderparamter.m_nrOfFilterUnitsOld;
		m_protect.enter();
		m_unitCounter = 0;
		m_midifilterunitmap.clear();

		m_protect.exit();
	}
	if (hasparameterChanged(*m_eqoderparamter.m_nrOfFilter, m_eqoderparamter.m_nrOfFilterOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setNrOfFilters(int(m_eqoderparamter.m_nrOfFilterOld));
		}
	}
	if (hasparameterChanged(*m_eqoderparamter.m_GainF0, m_eqoderparamter.m_GainF0Old))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setMaxGainf0(m_eqoderparamter.m_GainF0Old);
		}
	}
	if (hasparameterChanged(*m_eqoderparamter.m_GainFend, m_eqoderparamter.m_GainFendOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setMaxGainfend(m_eqoderparamter.m_GainFendOld);
		}
	}
	if (hasparameterChanged(*m_eqoderparamter.m_GainForm, m_eqoderparamter.m_GainFormOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setGainForm(m_eqoderparamter.m_GainFormOld);
		}
	}

	if (hasparameterChanged(*m_eqoderparamter.m_Q, m_eqoderparamter.m_QOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setQ(exp(m_eqoderparamter.m_QOld));
		}
	}

	if (hasparameterChanged(*m_eqoderparamter.m_FreqSpread, m_eqoderparamter.m_FreqSpreadOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setFreqSpread(m_eqoderparamter.m_FreqSpreadOld);
		}
	}
	if (hasparameterChanged(*m_eqoderparamter.m_BWSpread, m_eqoderparamter.m_BWSpreadOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setBWSpread(exp(m_eqoderparamter.m_BWSpreadOld));
		}
	}
	if (hasparameterChanged(*m_eqoderparamter.m_OutGain, m_eqoderparamter.m_OutGainOld))
	{
		m_OutGain = pow(10.0,m_eqoderparamter.m_OutGainOld/20.0);

	}



	// Envelope
	if (hasparameterChanged(*m_envparameter.m_attack, m_envparameter.m_attackOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setAttackRate(exp(m_envparameter.m_attackOld));
		}		
	}
	if (hasparameterChanged(*m_envparameter.m_decay, m_envparameter.m_decayOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setDecayRate(exp(m_envparameter.m_decayOld));
		}		
	}
	if (hasparameterChanged(*m_envparameter.m_sustain, m_envparameter.m_sustainOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setSustainLevel((m_envparameter.m_sustainOld));
		}		
	}
	if (hasparameterChanged(*m_envparameter.m_release, m_envparameter.m_releaseOld))
	{
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setReleaseRate(exp(m_envparameter.m_releaseOld));
		}		
	}
}

void Eqoder::setParameterForNewFilterUnit(int key)
{
	m_midifilterunitmap[key]->setSamplerate(m_fs);
	m_midifilterunitmap[key]->setNrOfFilters(m_eqoderparamter.m_nrOfFilterOld);
	m_midifilterunitmap[key]->setMaxGainf0(m_eqoderparamter.m_GainF0Old);
	m_midifilterunitmap[key]->setMaxGainfend(m_eqoderparamter.m_GainFendOld);
	m_midifilterunitmap[key]->setGainForm(m_eqoderparamter.m_GainFormOld);

	m_midifilterunitmap[key]->setBWSpread(exp(m_eqoderparamter.m_BWSpreadOld));
	m_midifilterunitmap[key]->setQ(exp(m_eqoderparamter.m_QOld));
	// Uebergang der Harmonischen
	// ermöglicht virtuellen Pitch, wenn < 1.0 (alle nicht 2er Potenzen sind inharmonisch)
	m_midifilterunitmap[key]->setFreqSpread(m_eqoderparamter.m_FreqSpreadOld);

	// envelope
	m_midifilterunitmap[key]->setAttackRate(exp(m_envparameter.m_attackOld));
	m_midifilterunitmap[key]->setDecayRate(exp(m_envparameter.m_decayOld));
	m_midifilterunitmap[key]->setReleaseRate(exp(m_envparameter.m_releaseOld));
	m_midifilterunitmap[key]->setSustainLevel(m_envparameter.m_sustainOld);
	m_midifilterunitmap[key]->reset();
}
void Eqoder::prepareParameter(std::unique_ptr<AudioProcessorValueTreeState>& vts)
{
	// m_vts = vts;
    m_eqoderparamter.m_nrOfFilterUnits = vts->getRawParameterValue(paramEqoderNrOfFilterUnits.ID);
	m_eqoderparamter.m_nrOfFilterUnitsOld = paramEqoderNrOfFilterUnits.defaultValue;
    m_eqoderparamter.m_nrOfFilter = vts->getRawParameterValue(paramEqoderNrOfFilters.ID);
	m_eqoderparamter.m_nrOfFilterOld = paramEqoderNrOfFilters.defaultValue;
    m_eqoderparamter.m_GainF0 = vts->getRawParameterValue(paramEqoderGainF0.ID);
	m_eqoderparamter.m_GainF0Old = paramEqoderGainF0.defaultValue;
    m_eqoderparamter.m_GainFend = vts->getRawParameterValue(paramEqoderGainFend.ID);
	m_eqoderparamter.m_GainFendOld = paramEqoderGainFend.defaultValue;
    m_eqoderparamter.m_GainForm = vts->getRawParameterValue(paramEqoderGainForm.ID);
	m_eqoderparamter.m_GainFormOld = paramEqoderGainForm.defaultValue;
    m_eqoderparamter.m_Q = vts->getRawParameterValue(paramEqoderQ.ID);
	m_eqoderparamter.m_QOld = paramEqoderQ.defaultValue;
    m_eqoderparamter.m_FreqSpread = vts->getRawParameterValue(paramEqoderFreqSpread.ID);
	m_eqoderparamter.m_FreqSpreadOld = paramEqoderFreqSpread.defaultValue;
    m_eqoderparamter.m_BWSpread = vts->getRawParameterValue(paramEqoderBWSpread.ID);
	m_eqoderparamter.m_BWSpreadOld = paramEqoderBWSpread.defaultValue;
    m_eqoderparamter.m_OutGain = vts->getRawParameterValue(paramEqoderOutGain.ID);
	m_eqoderparamter.m_OutGainOld = paramEqoderOutGain.defaultValue;



	// envelope
	m_envparameter.m_attack = vts->getRawParameterValue(paramEnvAttack.ID[0]);
	m_envparameter.m_attackOld = paramEnvAttack.defaultValue;
	m_envparameter.m_decay = vts->getRawParameterValue(paramEnvDecay.ID[0]);
	m_envparameter.m_decayOld = paramEnvDecay.defaultValue;
	m_envparameter.m_release = vts->getRawParameterValue(paramEnvRelease.ID[0]);
	m_envparameter.m_releaseOld = paramEnvRelease.defaultValue;
	m_envparameter.m_sustain = vts->getRawParameterValue(paramEnvSustain.ID[0]);
	m_envparameter.m_sustainOld = paramEnvSustain.defaultValue;




}




void EqoderParameter::addParameter(std::vector < std::unique_ptr<RangedAudioParameter>>& paramVector)
{
    	paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderNrOfFilterUnits.ID,
		paramEqoderNrOfFilterUnits.name,
		NormalisableRange<float>(paramEqoderNrOfFilterUnits.minValue, paramEqoderNrOfFilterUnits.maxValue),
		paramEqoderNrOfFilterUnits.defaultValue,
		paramEqoderNrOfFilterUnits.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(1.0*int(value), MaxLen)); },
		[](const String& text) {return text.getFloatValue(); }));

    	paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderNrOfFilters.ID,
		paramEqoderNrOfFilters.name,
		NormalisableRange<float>(paramEqoderNrOfFilters.minValue, paramEqoderNrOfFilters.maxValue),
		paramEqoderNrOfFilters.defaultValue,
		paramEqoderNrOfFilters.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(1.0*int(value), MaxLen)); },
		[](const String& text) {return text.getFloatValue(); }));

    	paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderGainF0.ID,
		paramEqoderGainF0.name,
		NormalisableRange<float>(paramEqoderGainF0.minValue, paramEqoderGainF0.maxValue),
		paramEqoderGainF0.defaultValue,
		paramEqoderGainF0.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(0.1*int(10.0*value), MaxLen)); },
		[](const String& text) {return text.getFloatValue(); }));

    	paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderGainFend.ID,
		paramEqoderGainFend.name,
		NormalisableRange<float>(paramEqoderGainFend.minValue, paramEqoderGainFend.maxValue),
		paramEqoderGainFend.defaultValue,
		paramEqoderGainFend.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(0.1*int(10.0*value), MaxLen)); },
		[](const String& text) {return text.getFloatValue(); }));

    	
		paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderGainForm.ID,
		paramEqoderGainForm.name,
		NormalisableRange<float>(paramEqoderGainForm.minValue, paramEqoderGainForm.maxValue),
		paramEqoderGainForm.defaultValue,
		paramEqoderGainForm.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(0.1*int(10.0*value), MaxLen)); },
		[](const String& text) {return text.getFloatValue(); }));

		paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderQ.ID,
			paramEqoderQ.name,
			NormalisableRange<float>(paramEqoderQ.minValue, paramEqoderQ.maxValue),
			paramEqoderQ.defaultValue,
			paramEqoderQ.unitName,
			AudioProcessorParameter::genericParameter,
			[](float value, int MaxLen) { return (String(int(exp(value) * 10 + 0.5) * 0.1, MaxLen) ); },
			[](const String& text) {return text.getFloatValue(); }));

		paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderBWSpread.ID,
			paramEqoderBWSpread.name,
			NormalisableRange<float>(paramEqoderBWSpread.minValue, paramEqoderBWSpread.maxValue),
			paramEqoderBWSpread.defaultValue,
			paramEqoderBWSpread.unitName,
			AudioProcessorParameter::genericParameter,
			[](float value, int MaxLen) { return (String(int(exp(value) * 10 + 0.5) * 0.1, MaxLen) ); },
			[](const String& text) {return text.getFloatValue(); }));

		paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderFreqSpread.ID,
			paramEqoderFreqSpread.name,
			NormalisableRange<float>(paramEqoderFreqSpread.minValue, paramEqoderFreqSpread.maxValue),
			paramEqoderFreqSpread.defaultValue,
			paramEqoderFreqSpread.unitName,
			AudioProcessorParameter::genericParameter,
			[](float value, int MaxLen) { return (String(int((value) * 16 + 0.5) * 0.0625, MaxLen) ); },
			[](const String& text) {return text.getFloatValue(); }));

		paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderOutGain.ID,
			paramEqoderOutGain.name,
			NormalisableRange<float>(paramEqoderOutGain.minValue, paramEqoderOutGain.maxValue),
			paramEqoderOutGain.defaultValue,
			paramEqoderOutGain.unitName,
			AudioProcessorParameter::genericParameter,
			[](float value, int MaxLen) { return (String(int((value) * 4 + 0.5) * 0.25, MaxLen) ); },
			[](const String& text) {return text.getFloatValue(); }));

}

EqoderParameterComponent::EqoderParameterComponent(AudioProcessorValueTreeState& vts)
:somethingChanged(nullptr),m_vts(vts),m_scaleFactor(1.f)
{
	m_NrOfFilterUnitsLabel.setText("NrOfUnits", NotificationType::dontSendNotification);
	m_NrOfFilterUnitsLabel.setJustificationType(Justification::centred);
	m_NrOfFilterUnitsLabel.attachToComponent (&m_NrOfFilterUnitsSlider, false);
	addAndMakeVisible(m_NrOfFilterUnitsLabel);
	m_NrOfFilterUnitsSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_NrOfFiltersSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_NrOfFilterUnitsAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderNrOfFilterUnits.ID, m_NrOfFilterUnitsSlider);
	addAndMakeVisible(m_NrOfFilterUnitsSlider);
	m_NrOfFilterUnitsSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_NrOfFiltersLabel.setText("NrOfFilters", NotificationType::dontSendNotification);
	m_NrOfFiltersLabel.setJustificationType(Justification::centred);
	m_NrOfFiltersLabel.attachToComponent (&m_NrOfFiltersSlider, false);
	addAndMakeVisible(m_NrOfFiltersLabel);

	m_NrOfFiltersSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_NrOfFiltersSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_NrOfFiltersAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderNrOfFilters.ID, m_NrOfFiltersSlider);
	addAndMakeVisible(m_NrOfFiltersSlider);
	m_NrOfFiltersSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_GainF0Label.setText("GainF0", NotificationType::dontSendNotification);
	m_GainF0Label.setJustificationType(Justification::centred);
	m_GainF0Label.attachToComponent (&m_GainF0Slider, false);
	addAndMakeVisible(m_GainF0Label);

	m_GainF0Slider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_GainF0Slider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_GainF0Attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderGainF0.ID, m_GainF0Slider);
	addAndMakeVisible(m_GainF0Slider);
	m_GainF0Slider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_GainFendLabel.setText("GainFend", NotificationType::dontSendNotification);
	m_GainFendLabel.setJustificationType(Justification::centred);
	m_GainFendLabel.attachToComponent (&m_GainFendSlider, false);
	addAndMakeVisible(m_GainFendLabel);

	m_GainFendSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_GainFendSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_GainFendAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderGainFend.ID, m_GainFendSlider);
	addAndMakeVisible(m_GainFendSlider);
	m_GainFendSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_GainFormLabel.setText("GainForm", NotificationType::dontSendNotification);
	m_GainFormLabel.setJustificationType(Justification::centred);
	m_GainFormLabel.attachToComponent (&m_GainFormSlider, false);
	addAndMakeVisible(m_GainFormLabel);

	m_GainFormSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_GainFormSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_GainFormAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderGainForm.ID, m_GainFormSlider);
	addAndMakeVisible(m_GainFormSlider);
	m_GainFormSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_QLabel.setText("Q", NotificationType::dontSendNotification);
	m_QLabel.setJustificationType(Justification::centred);
	m_QLabel.attachToComponent (&m_QSlider, false);
	addAndMakeVisible(m_QLabel);

	m_QSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_QSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_QAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderQ.ID, m_QSlider);
	addAndMakeVisible(m_QSlider);
	m_QSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_FreqSpreadLabel.setText("FreqSpread", NotificationType::dontSendNotification);
	m_FreqSpreadLabel.setJustificationType(Justification::centred);
	m_FreqSpreadLabel.attachToComponent (&m_FreqSpreadSlider, false);
	addAndMakeVisible(m_FreqSpreadLabel);

	m_FreqSpreadSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_FreqSpreadSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_FreqSpreadAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderFreqSpread.ID, m_FreqSpreadSlider);
	addAndMakeVisible(m_FreqSpreadSlider);
	m_FreqSpreadSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_BWSpreadLabel.setText("BWSpread", NotificationType::dontSendNotification);
	m_BWSpreadLabel.setJustificationType(Justification::centred);
	m_BWSpreadLabel.attachToComponent (&m_BWSpreadSlider, false);
	addAndMakeVisible(m_BWSpreadLabel);

	m_BWSpreadSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_BWSpreadSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_BWSpreadAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderBWSpread.ID, m_BWSpreadSlider);
	addAndMakeVisible(m_BWSpreadSlider);
	m_BWSpreadSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

	m_OutGainLabel.setText("OutGain", NotificationType::dontSendNotification);
	m_OutGainLabel.setJustificationType(Justification::centred);
	m_OutGainLabel.attachToComponent (&m_OutGainSlider, false);
	addAndMakeVisible(m_OutGainLabel);

	m_OutGainSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_OutGainSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_OutGainAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderOutGain.ID, m_OutGainSlider);
	addAndMakeVisible(m_OutGainSlider);
	m_OutGainSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };


}

void EqoderParameterComponent::paint(Graphics& g)
{
	g.fillAll((getLookAndFeel().findColour(ResizableWindow::backgroundColourId)).darker(0.2));

}
void EqoderParameterComponent::resized()
{
	float scaleFactor = m_scaleFactor;

	auto r = getLocalBounds();

	// reduce for a small border
	int newBorderWidth = scaleFactor*GLOBAL_MIN_DISTANCE;
	r.reduce(newBorderWidth, newBorderWidth);
	auto s = r;
	auto t = r;

	t = s.removeFromBottom(scaleFactor*(GLOBAL_MIN_LABEL_HEIGHT/2+GLOBAL_MIN_ROTARY_WIDTH));
	m_NrOfFilterUnitsSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_NrOfFilterUnitsSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_NrOfFiltersSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_NrOfFiltersSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_GainF0Slider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_GainF0Slider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_GainFendSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_GainFendSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_GainFormSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_GainFormSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_QSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_QSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_FreqSpreadSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_FreqSpreadSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_BWSpreadSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_BWSpreadSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

	m_OutGainSlider.setBounds(t.removeFromLeft(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_OutGainSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

}
