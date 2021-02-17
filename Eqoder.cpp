
#include "Eqoder.h"

Eqoder::Eqoder()
{
	m_unitCounter = 0;
	//m_pfilterunit = std::make_unique<EQoderFilterUnit[]> (g_NrOfFilterUnits);
	for (int kk = 0; kk < g_NrOfFilterUnits; ++kk)
	{
		m_pointerPool.add(std::unique_ptr<EQoderFilterUnit>(new EQoderFilterUnit()));
		// auto oneunitpointer = std::make_unique<EQoderFilterUnit>(); 
		// m_pfilterunit.push_back(oneunitpointer);
		// m_pointerPool.add(m_pfilterunit[kk]);
		//m_pfilterunit[kk] = std::unique_ptr<EQoderFilterUnit>(new EQoderFilterUnit());
	}
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
					m_midifilterunitmap[midiNoteNr]->setFundamentalFrequency(freqNote,Velocity);
				}
				else
				{
					m_unitCounter++;
					if (m_unitCounter < g_NrOfFilterUnits) // Resources are available
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

    auto totalNrChannels = data.getNumChannels();;
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
            channelData[idx] = m_data[channel][idx];
        }
	}

}
void Eqoder::updateParameter()
{
	float curValue = *m_eqoderparamter.m_nrOfFilter;
	if (m_eqoderparamter.m_nrOfFilterOld != *m_eqoderparamter.m_nrOfFilter)
	{
		m_eqoderparamter.m_nrOfFilterOld = *m_eqoderparamter.m_nrOfFilter;
		for (auto onefilterunit : m_midifilterunitmap )
		{
			onefilterunit.second->setNrOfFilters(int(m_eqoderparamter.m_nrOfFilterOld));
		}
	}

}

void Eqoder::setParameterForNewFilterUnit(int key)
{
	m_midifilterunitmap[key]->setSamplerate(m_fs);
	m_midifilterunitmap[key]->setNrOfFilters(m_eqoderparamter.m_nrOfFilterOld);
//	m_midifilterunitmap[key]->setNrOfFilters(8);
	m_midifilterunitmap[key]->setBWSpread(0.1);
	m_midifilterunitmap[key]->setQ(30.0);
	// Uebergang der Harmonischen
	m_midifilterunitmap[key]->setMaxGainf0(20.f);
	m_midifilterunitmap[key]->setMaxGainfend(5.f);
	m_midifilterunitmap[key]->setGainForm(1.0);

	// ermöglicht virtuellen Pitch, wenn < 1.0 (alle nicht 2er Potenzen sind inharmonisch)
	m_midifilterunitmap[key]->setFreqSpread(0.0);
}
void Eqoder::prepareParameter(std::unique_ptr<AudioProcessorValueTreeState>& vts)
{
	// m_vts = vts;
    m_eqoderparamter.m_nrOfFilter = vts->getRawParameterValue(paramEqoderNrOfFilters.ID);
	m_eqoderparamter.m_nrOfFilterOld = 1;
}




int EqoderParameter::addParameter(std::vector < std::unique_ptr<RangedAudioParameter>>& paramVector)
{
    	paramVector.push_back(std::make_unique<AudioParameterFloat>(paramEqoderNrOfFilters.ID,
		paramEqoderNrOfFilters.name,
		NormalisableRange<float>(paramEqoderNrOfFilters.minValue, paramEqoderNrOfFilters.maxValue),
		paramEqoderNrOfFilters.defaultValue,
		paramEqoderNrOfFilters.unitName,
		AudioProcessorParameter::genericParameter,
		[](float value, int MaxLen) { return (String(1.0*int(value), MaxLen)); },
		[](const String& text) {return text.getFloatValue(); }));

}

EqoderParameterComponent::EqoderParameterComponent(AudioProcessorValueTreeState& vts)
:m_vts(vts),somethingChanged(nullptr)
{
	m_NrOfFiltersLabel.setText("NrOfFilters", NotificationType::dontSendNotification);
	m_NrOfFiltersLabel.setJustificationType(Justification::centred);
	m_NrOfFiltersLabel.attachToComponent (&m_NrOfFiltersSlider, false);
	addAndMakeVisible(m_NrOfFiltersLabel);

	m_NrOfFiltersSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	// m_NrOfFiltersSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true, 60, 20);
	m_NrOfFiltersAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(m_vts, paramEqoderNrOfFilters.ID, m_NrOfFiltersSlider);
	addAndMakeVisible(m_NrOfFiltersSlider);
	m_NrOfFiltersSlider.onValueChange = [this]() {if (somethingChanged != nullptr) somethingChanged(); };

}

void EqoderParameterComponent::paint(Graphics& g)
{
	g.fillAll((getLookAndFeel().findColour(ResizableWindow::backgroundColourId)).brighter(0.2));

}
void EqoderParameterComponent::resized()
{
	int Height = getHeight();
	int Width = getWidth();

	float scaleFactor = float(Width)/EQODER_MIN_WIDTH;

	auto r = getLocalBounds();

	// reduce for a small border
	int newBorderWidth = jmax(GLOBAL_MIN_DISTANCE,int(scaleFactor*GLOBAL_MIN_DISTANCE));
	r.reduce(newBorderWidth, newBorderWidth);
	auto s = r;
	auto t = r;

	// m_NrOfFiltersLabel.setFont(Font(scaleFactor*GLOBAL_MIN_LABEL_FONTSIZE));
	int newRotaryWidth = jmax(GLOBAL_MIN_ROTARY_WIDTH,int(scaleFactor*GLOBAL_MIN_ROTARY_WIDTH));
	m_NrOfFiltersSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, true,scaleFactor* GLOBAL_MIN_ROTARY_TB_WIDTH, scaleFactor*GLOBAL_MIN_ROTARY_TB_HEIGHT);
	
	int newLabelHeight = jmax(GLOBAL_MIN_LABEL_HEIGHT,int(scaleFactor*GLOBAL_MIN_LABEL_HEIGHT));

//	s = r.removeFromTop(newLabelHeight);
	
//	int newLabelWidth = jmax(GLOBAL_MIN_LABEL_WIDTH,int(scaleFactor*GLOBAL_MIN_LABEL_WIDTH));
//	m_NrOfFiltersLabel.setBounds(s.removeFromLeft(newLabelWidth));
	

	s = r;
	t = s.removeFromBottom(newRotaryWidth + newLabelHeight);
	m_NrOfFiltersSlider.setBounds(t.removeFromLeft(newRotaryWidth));
		
	
}
