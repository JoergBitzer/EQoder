#pragma once

#include <vector>
#include <string>

#include "JuceHeader.h"
#include "EqoderFilterUnit.h"
#include "PointerPool.h"

const int g_NrOfFilterUnits(8);
const int g_maxChannels(8);
class Eqoder
{
public:
    Eqoder();
    ~Eqoder();

    void prepareToPlay (double sampleRate, int samplesPerBlock, int nrofchannels = 2);
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&);


private:
    int m_nrOfChannels;
    CriticalSection m_protect;
    std::vector<std::vector<double>> m_data;	
    double m_fs;
    int m_maxSamples;
    
    // filter pool
    int m_unitCounter;
    // std::vector<std::unique_ptr<EQoderFilterUnit>> m_pfilterunit;
   	std::map <int, std::shared_ptr<EQoderFilterUnit>> m_midifilterunitmap;
    PointerPool<EQoderFilterUnit> m_pointerPool;
    int m_softestNote;
    void setParameterForNewFilterUnit(int key);

};

const struct
{
	const std::string ID = "NrOfFilters";
	std::string name = "Number of Harmonics";
	std::string unitName = "";
	float minValue = 1.f;
	float maxValue = 20.f;
	float defaultValue = 7.f;
}paramEqoderNrOfFilters;




class EqoderParameter
{
public:
	int addParameter(std::vector < std::unique_ptr<RangedAudioParameter>>& paramVector);

};

class EqoderParameterComponent : public Component
{
public:
	EqoderParameterComponent(AudioProcessorValueTreeState& );

	void paint(Graphics& g) override;
	void resized() override;
    std::function<void()> somethingChanged;

private:
    AudioProcessorValueTreeState& m_vts; 
};
