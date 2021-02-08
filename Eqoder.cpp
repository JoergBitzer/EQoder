
#include "Eqoder.h"

Eqoder::Eqoder()
{
	for (int kk = 0; kk < g_NrOfFilterUnits; kk++)
	{
	    m_filterunits[kk].setFundamentalFrequency(2000.0);
    	m_filterunits[kk].setNrOfFilters(8);
    	m_filterunits[kk].setBWSpread(0.1);
    	m_filterunits[kk].setQ(30.0);
    	// Uebergang der Harmonischen
    	m_filterunits[kk].setMaxGainf0(20.f);
    	m_filterunits[kk].setMaxGainfend(5.f);
    	m_filterunits[kk].setGainForm(1.0);

    	// ermöglicht virtuellen Pitch, wenn < 1.0 (alle nicht 2er Potenzen sind inharmonisch)
    	m_filterunits[kk].setFreqSpread(0.0);
  
	}

}
Eqoder::~Eqoder()
{

}

void Eqoder::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_fs = sampleRate;
    m_maxSamples = samplesPerBlock;

	for (int kk = 0; kk < g_NrOfFilterUnits; kk++)
	{
	    m_filterunits[kk].reset();
	}
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
				double freqNote = msg.getMidiNoteInHertz(msg.getNoteNumber());

				// debug
				m_filterunits[0].setFundamentalFrequency(freqNote);
			 }

		 }
	}   
            
    // Audio processing
    m_data.resize(data.getNumSamples());
    // auto totalNumInputChannels  = 
    auto totalNrChannels = data.getNumChannels();;

	//for (long unsigned int channel = 0; channel < totalNrChannels; ++channel)
    int channel = 0;
	{
        auto* channelData = data.getWritePointer (channel);

        // ..do something to the data...
        for (auto idx = 0u; idx < data.getNumSamples(); idx++)
        {
            m_data[idx] = channelData[idx];
        }
        
		m_protect.enter();
		m_filterunits[0].processData(m_data);
		m_protect.exit();
        
		
		for (auto idx = 0u; idx < data.getNumSamples(); idx++)
        {
            channelData[idx] = m_data[idx];
        }
	}

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
:m_vts(vts), somethingChanged(nullptr)
{
/*	m_delayLabel.setText("Delay", NotificationType::dontSendNotification);
	addAndMakeVisible(m_delayLabel);
	m_delaySlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
	m_delaySlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxAbove, true, 80, 20);
	m_delayAttachment = std::make_unique<SliderAttachment>(m_vts, paramChorusDelay.ID, m_delaySlider);
	addAndMakeVisible(m_delaySlider);
	m_delaySlider.onValueChange = [this]() {if (somethingChanged != nullptr)  somethingChanged(); };
*/

}

void EqoderParameterComponent::paint(Graphics& g)
{
	g.fillAll((getLookAndFeel().findColour(ResizableWindow::backgroundColourId)).brighter(0.2));

}
void EqoderParameterComponent::resized()
{
	int w = getWidth();
	int h = getHeight();

    // make resizable
	//m_delayLabel.setBounds(GUI_MIN_DISTANCE + ROTARYSIZE, GUI_MIN_DISTANCE, LABEL_WIDTH, ELEMNT_HEIGHT);
	//m_delaySlider.setBounds(GUI_MIN_DISTANCE + ROTARYSIZE, GUI_MIN_DISTANCE, w - 2 * GUI_MIN_DISTANCE-2*ROTARYSIZE,2*ELEMNT_HEIGHT);

}
