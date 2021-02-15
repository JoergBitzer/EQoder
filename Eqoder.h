#pragma once

#include <vector>
#include <string>

#include "JuceHeader.h"
#include "EqoderFilterUnit.h"
#include "PointerPool.h"

const int g_NrOfFilterUnits(8);
const int g_maxChannels(8);

class EqoderParameter
{
public:
	int addParameter(std::vector < std::unique_ptr<RangedAudioParameter>>& paramVector);

public:
    std::atomic<float>* m_nrOfFilter;
    float m_nrOfFilterOld;
};

class Eqoder
{
public:
    Eqoder();
    ~Eqoder();

    void prepareToPlay (double sampleRate, int samplesPerBlock, int nrofchannels = 2);
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&);

    
    void prepareParameter(std::unique_ptr<AudioProcessorValueTreeState>&  vts);
 
private:
    int m_nrOfChannels;
    CriticalSection m_protect;
    std::vector<std::vector<double>> m_data;	
    double m_fs;
    int m_maxSamples;
    
    // filter pool
    int m_unitCounter;

   	std::map <int, std::shared_ptr<EQoderFilterUnit>> m_midifilterunitmap;
    PointerPool<EQoderFilterUnit> m_pointerPool;
    int m_softestNote;
    void setParameterForNewFilterUnit(int key);

    // parameter handling
    void updateParameter();
    EqoderParameter m_eqoderparamter;

};

const struct
{
	const std::string ID = "NrOfFilters";
	std::string name = "Number of Filters";
	std::string unitName = "";
	float minValue = 1.f;
	float maxValue = 20.f;
	float defaultValue = 7.f;
}paramEqoderNrOfFilters;





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
