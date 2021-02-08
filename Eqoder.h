#pragma once

#include <vector>
#include <string>

#include "JuceHeader.h"
#include "EqoderFilterUnit.h"

const int g_NrOfFilterUnits(8);

class Eqoder
{
public:
    Eqoder();
    ~Eqoder();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&);


private:
    CriticalSection m_protect;
    std::vector<double> m_data;	
    double m_fs;
    int m_maxSamples;
    EQoderFilterUnit m_filterunits[g_NrOfFilterUnits];

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
