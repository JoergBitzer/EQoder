
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
	m_Parallel = false;
	m_midifilterunitmap.clear();
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
	m_InData.clear();
	m_SumData.clear();
	m_midifilterunitmap.clear();
	m_limiter.prepareToPlay(sampleRate,m_nrOfChannels);
}
void Eqoder::processBlock (juce::AudioBuffer<float>& data, juce::MidiBuffer& mididata)
{
    // first analyse Mididata
	int midicount = 0;
    for (const MidiMessageMetadata metadata : mididata)
    {
		 if (metadata.numBytes == 3)
		 {
		
			 MidiMessage msg= metadata.getMessage();
			 if(msg.isNoteOn())
			 {
				midicount++;
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
									
					if (m_unitCounter < m_maxunits) // Resources are available
					{

						
						m_midifilterunitmap[midiNoteNr] = m_pointerPool.acquire();
						setParameterForNewFilterUnit(midiNoteNr);
						m_midifilterunitmap[midiNoteNr]->setFundamentalFrequency(freqNote,Velocity);
					}
					else // Steeling
					{
						m_unitCounter--;
						if (m_midifilterunitmap.contains(m_softestNote))
							m_midifilterunitmap.erase(m_softestNote);
						else // erase lowest note
						{
							int lowestkey = 127;
							for (const auto& [key, _] : m_midifilterunitmap) 
							{
								if (key < lowestkey)
									lowestkey = key;

								DBG(String(key));
								DBG(midicount);
    							//itemKeys.push_back(key);
								if (key != m_softestNote)
									DBG("Stop");
							}
							m_midifilterunitmap.erase(lowestkey);
						}
						DBG(String(m_softestNote) + "-");
						

						if (!m_pointerPool.empty())
						{
							m_midifilterunitmap[midiNoteNr] = m_pointerPool.acquire();
							setParameterForNewFilterUnit(midiNoteNr);
							m_midifilterunitmap[midiNoteNr]->setFundamentalFrequency(freqNote,Velocity);
						}
						
					}
					m_unitCounter++;
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
    // m_data.resize(data.getNumSamples());

    size_t totalNrChannels = static_cast<size_t>(data.getNumChannels());
    m_data.resize(totalNrChannels);
	m_InData.resize(totalNrChannels);
	m_SumData.resize(totalNrChannels);

	for (long unsigned int channel = 0; channel < totalNrChannels; ++channel)
	{
		m_data[channel].resize(static_cast<size_t>(data.getNumSamples()));
		m_InData[channel].resize(static_cast<size_t>(data.getNumSamples()));
		m_SumData[channel].resize(static_cast<size_t>(data.getNumSamples()));
		std::fill(m_SumData[channel].begin(), m_SumData[channel].end(), 0.);
        
		auto* channelData = data.getWritePointer (channel);

        // ..do something to the data...
        for (auto idx = 0; idx < data.getNumSamples(); idx++)
        {
            m_data[channel][idx] = m_OutGain*channelData[idx];
            m_InData[channel][idx] = m_OutGain*channelData[idx];
        }
	}
	m_protect.enter();
	
	float minLevel = 1.f;
	float minLevelRelease = 1.f;
	int notestostop[g_NrOfFilterUnits];
	int notestopcounter(0);
	int softnote = -1;
	int softnoterelease = -1;
	for (auto onefilterunit : m_midifilterunitmap )
	{
		if (m_Parallel)
		{
			onefilterunit.second->processData(m_InData,m_data);
			for (auto cc = 0; cc < totalNrChannels ; ++cc)
			{
				std::transform(m_SumData[cc].begin(), m_SumData[cc].end(), m_data[cc].begin(), m_SumData[cc].begin(), std::plus<double>());
			}
		}
		else
		{
			onefilterunit.second->processData(m_data);
		}
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
			softnote = onefilterunit.first;
		}
		if (Level< minLevelRelease & phase != Envelope::envelopePhases::Attack)
		{
			minLevelRelease = Level;
			softnoterelease = onefilterunit.first;
		}
		if (minLevelRelease != 1.f)
		{
			m_softestNote = softnoterelease;
		}
		else
		{
			m_softestNote = softnote;
		}

	}
	if (m_Parallel)		
	{
		for (auto cc = 0; cc < totalNrChannels ; ++cc)
		{
			std::copy(m_SumData[cc].begin(), m_SumData[cc].end(), m_data[cc].begin());
		}

	}

	for (auto kk = 0 ; kk < notestopcounter; ++kk)
	{
		for (const auto& [key, _] : m_midifilterunitmap) 
		{
			DBG(String(key) + "key");
			//itemKeys.push_back(key);
		}
		DBG(String(notestostop[kk]) + "notetostop");
		m_midifilterunitmap.erase(notestostop[kk]);
		m_unitCounter--;
	}
	
	m_limiter.processSamples(m_data);
	
	m_protect.exit();
        
	for (long unsigned int channel = 0; channel < totalNrChannels; ++channel)
	{
        auto* channelData = data.getWritePointer (channel);
		
		for (auto idx = 0; idx < data.getNumSamples(); idx++)
        {
            channelData[idx] = m_data[channel][idx];
			if (!std::isfinite(channelData[idx] ))
			{
				switch(std::fpclassify(channelData[idx])) 
				{
        			case FP_INFINITE:  DBG( "Inf");break;
        			case FP_NAN:       DBG( "NaN");break;
        			case FP_NORMAL:    DBG( "normal");break;
        			case FP_SUBNORMAL: DBG( "subnormal");break;
        			case FP_ZERO:      DBG( "zero");break;
        			default:           DBG( "unknown");break;
    			}
			}	
        }
	}
	if (m_midifilterunitmap.size() != m_unitCounter)
	{
		DBG("Somethings wrong");
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

	// seriel / parallel switch
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
		[](float value, int MaxLen) { return (String(0.1*int(10.0*value), MaxLen) + " " + paramEqoderGainF0.unitName); },
		[](const String& text) {return text.getFloatValue(); }));

    	paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderGainFend.ID,
		paramEqoderGainFend.name,
		NormalisableRange<float>(paramEqoderGainFend.minValue, paramEqoderGainFend.maxValue),
		paramEqoderGainFend.defaultValue,
		paramEqoderGainFend.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(0.1*int(10.0*value), MaxLen) + " " + paramEqoderGainF0.unitName); },
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
			[](float value, int MaxLen) { return (String(int((value) * 4 + 0.5) * 0.25, MaxLen) + " " + paramEqoderGainF0.unitName); },
			[](const String& text) {return text.getFloatValue(); }));
		
		paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderSwitchParallel.ID,
			paramEqoderSwitchParallel.name,
			NormalisableRange<float>(paramEqoderSwitchParallel.minValue, paramEqoderSwitchParallel.maxValue),
			paramEqoderSwitchParallel.defaultValue,
			paramEqoderSwitchParallel.unitName,
			AudioProcessorParameter::genericParameter,
			[](float value, int MaxLen) { return (String(value, MaxLen) ); },
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
	m_NrOfFiltersSlider.onValueChange = [this]() {m_NrOfFilters = m_NrOfFiltersSlider.getValue(); repaint(); if (somethingChanged != nullptr) somethingChanged(); };

	m_GainF0Label.setText("GainF0", NotificationType::dontSendNotification);
	m_GainF0Label.setJustificationType(Justification::centred);
	m_GainF0Label.attachToComponent (&m_GainF0Slider, false);
	addAndMakeVisible(m_GainF0Label);

	m_GainF0Slider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_GainF0Slider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_GainF0Attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderGainF0.ID, m_GainF0Slider);
	addAndMakeVisible(m_GainF0Slider);
	m_GainF0Slider.onValueChange = [this]() {m_gainStart = m_GainF0Slider.getValue(); repaint(); if (somethingChanged != nullptr) somethingChanged(); };

	m_GainFendLabel.setText("GainFend", NotificationType::dontSendNotification);
	m_GainFendLabel.setJustificationType(Justification::centred);
	m_GainFendLabel.attachToComponent (&m_GainFendSlider, false);
	addAndMakeVisible(m_GainFendLabel);

	m_GainFendSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_GainFendSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_GainFendAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderGainFend.ID, m_GainFendSlider);
	addAndMakeVisible(m_GainFendSlider);
	m_GainFendSlider.onValueChange = [this]() {m_gainEnd = m_GainFendSlider.getValue(); repaint(); if (somethingChanged != nullptr) somethingChanged(); };

	m_GainFormLabel.setText("GainForm", NotificationType::dontSendNotification);
	m_GainFormLabel.setJustificationType(Justification::centred);
	m_GainFormLabel.attachToComponent (&m_GainFormSlider, false);
	addAndMakeVisible(m_GainFormLabel);

	m_GainFormSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_GainFormSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_GainFormAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderGainForm.ID, m_GainFormSlider);
	addAndMakeVisible(m_GainFormSlider);
	m_GainFormSlider.onValueChange = [this]() {m_Form = m_GainFormSlider.getValue(); repaint(); if(somethingChanged != nullptr) somethingChanged(); };

	m_QLabel.setText("Q", NotificationType::dontSendNotification);
	m_QLabel.setJustificationType(Justification::centred);
	m_QLabel.attachToComponent (&m_QSlider, false);
	addAndMakeVisible(m_QLabel);

	m_QSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_QSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_QAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderQ.ID, m_QSlider);
	addAndMakeVisible(m_QSlider);
	m_QSlider.onValueChange = [this]() {m_Q = m_QSlider.getValue(); repaint(); if (somethingChanged != nullptr) somethingChanged(); };

	m_FreqSpreadLabel.setText("FreqSpread", NotificationType::dontSendNotification);
	m_FreqSpreadLabel.setJustificationType(Justification::centred);
	m_FreqSpreadLabel.attachToComponent (&m_FreqSpreadSlider, false);
	addAndMakeVisible(m_FreqSpreadLabel);

	m_FreqSpreadSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_FreqSpreadSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_FreqSpreadAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderFreqSpread.ID, m_FreqSpreadSlider);
	addAndMakeVisible(m_FreqSpreadSlider);
	m_FreqSpreadSlider.onValueChange = [this]() {m_FreqSpread = m_FreqSpreadSlider.getValue(); repaint(); if (somethingChanged != nullptr) somethingChanged(); };

	m_BWSpreadLabel.setText("BWSpread", NotificationType::dontSendNotification);
	m_BWSpreadLabel.setJustificationType(Justification::centred);
	m_BWSpreadLabel.attachToComponent (&m_BWSpreadSlider, false);
	addAndMakeVisible(m_BWSpreadLabel);

	m_BWSpreadSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_BWSpreadSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_BWSpreadAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderBWSpread.ID, m_BWSpreadSlider);
	addAndMakeVisible(m_BWSpreadSlider);
	m_BWSpreadSlider.onValueChange = [this]() {m_BWSpread = m_BWSpreadSlider.getValue(); repaint(); if(somethingChanged != nullptr) somethingChanged(); };

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
	float scaleFactor = m_scaleFactor;
	g.fillAll((getLookAndFeel().findColour(ResizableWindow::backgroundColourId)).darker(0.2));
	g.setColour(JadeTeal);
	g.fillRect ((EQDISP_XPOS)*scaleFactor, (EQDISP_YPOS)*scaleFactor, (EQDISP_WIDTH)*scaleFactor, (EQDISP_HEIGHT)*scaleFactor);
	g.setColour(juce::Colours::black);
	float scaleStart = (EQDISP_XPOS+10);
	float scaleWidth = (EQDISP_XPOS + EQDISP_WIDTH - 10);
	float yscaleStart = (EQDISP_YPOS + EQDISP_HEIGHT-10);
	float yscaleMax = (EQDISP_YPOS+10);
	float yscaleHeight = -yscaleMax + yscaleStart;
	juce::Line <float> up(scaleStart*scaleFactor, yscaleStart*scaleFactor,
							 scaleStart*scaleFactor,yscaleMax*scaleFactor);
	g.drawArrow(up,3*scaleFactor,10*scaleFactor,20*scaleFactor);

	juce::Line <float> right((EQDISP_XPOS+2)*scaleFactor, yscaleStart*scaleFactor,
							 scaleWidth*scaleFactor,yscaleStart*scaleFactor);
	g.drawArrow(right,3*scaleFactor,10*scaleFactor,20*scaleFactor);

	// draw filter
	g.setColour(JadeRed);
	
	float oneoctave = scaleWidth/40; 
	float oneQ = scaleWidth/150; 
	for (auto kk = 0; kk < int(m_NrOfFilters); kk++)
	{
		// normalised xval
		float normxval = float(kk)/(m_NrOfFilters-1);
		if (m_NrOfFilters == 1) // for = 1 and kk = 0 normval is NAN (avoid this here)
			normxval = 0;
		float deformed_normx = powf(normxval,m_Form);
		float gain = (m_gainEnd-m_gainStart)*deformed_normx + m_gainStart;
		float diffX = oneQ*4*pow(double(kk+1),m_BWSpread)/exp(m_Q);
		float startX = (scaleStart + kk*oneoctave*pow(2.0,m_FreqSpread))*scaleFactor;
		float startY = ((yscaleMax+10) + (paramEqoderGainF0.maxValue - gain)/paramEqoderGainF0.maxValue * (yscaleHeight-10))*scaleFactor;
		float endX = (scaleStart + kk*oneoctave*pow(2.0,m_FreqSpread) - diffX );
		float endY = (yscaleStart)*scaleFactor;
		if (endX < EQDISP_XPOS)
			endX = EQDISP_XPOS;
		if (endX > EQDISP_XPOS + EQDISP_WIDTH)
			endX = EQDISP_XPOS + EQDISP_WIDTH;
		g.drawLine(startX, startY, endX*scaleFactor, endY,3*scaleFactor);
		endX = (scaleStart + kk*oneoctave*pow(2.0,m_FreqSpread) + diffX );
		if (endX < EQDISP_XPOS)
			endX = EQDISP_XPOS;
		if (endX > EQDISP_XPOS + EQDISP_WIDTH)
			endX = EQDISP_XPOS + EQDISP_WIDTH;
		g.drawLine(startX, startY, endX*scaleFactor, endY,3*scaleFactor);
	}
	

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

	t = s.removeFromTop(scaleFactor*(GLOBAL_MIN_LABEL_HEIGHT/2+GLOBAL_MIN_ROTARY_WIDTH));
	// labels are not considered as part of the rotary ==> additional offset necessary
	t.setY(t.getY()+GLOBAL_MIN_LABEL_HEIGHT*scaleFactor);
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

	m_OutGainSlider.setBounds(t.removeFromRight(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_OutGainSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	t.removeFromLeft(scaleFactor*GLOBAL_MIN_DISTANCE);	

}
