#pragma once

#include <vector>
#include <string>

#include "JuceHeader.h"
#include "EqoderFilterUnit.h"
#include "PointerPool.h"
#include "EqoderGUISettings.h" // necessary for GUI Elements
#include "Envelope.h"

const int g_NrOfFilterUnits(8);
const int g_maxChannels(8);

class EqoderParameter
{
public:
	int addParameter(std::vector < std::unique_ptr<RangedAudioParameter>>& paramVector);

public:
    std::atomic<float>* m_nrOfFilter;
    float m_nrOfFilterOld;

    std::atomic<float>* m_GainF0;
    float m_GainF0Old;

    std::atomic<float>* m_GainForm;
    float m_GainFormOld;

    std::atomic<float>* m_GainFend;
    float m_GainFendOld;

    std::atomic<float>* m_Q;
    float m_QOld;

    std::atomic<float>* m_FreqSpread;
    float m_FreqSpreadOld;

    std::atomic<float>* m_BWSpread;
    float m_BWSpreadOld;


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
    EnvelopeParameter m_envparameter;
    bool hasparameterChanged(float valNew, float &valOld)
    {
        if (valOld != valNew)
        {
            valOld = valNew;
            return true;
        }
        else
        {
            return false;
        }
    };

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

const struct
{
	const std::string ID = "GainF0";
	std::string name = "Gain at f0";
	std::string unitName = "dB";
	float minValue = 0.f;
	float maxValue = 20.f;
	float defaultValue = 6.f;
}paramEqoderGainF0;

const struct
{
	const std::string ID = "GainFend";
	std::string name = "Gain at fend";
	std::string unitName = "dB";
	float minValue = 0.f;
	float maxValue = 20.f;
	float defaultValue = 6.f;
}paramEqoderGainFend;

const struct
{
	const std::string ID = "GainForm";
	std::string name = "Form of Gains";
	std::string unitName = "";
	float minValue = 0.f;
	float maxValue = 20.f;
	float defaultValue = 1.f;
}paramEqoderGainForm;

const struct
{
	const std::string ID = "Q";
	std::string name = "Q";
	std::string unitName = "";
	float minValue = log(0.25f);
	float maxValue = log(40.f);
	float defaultValue = log(20.f);
}paramEqoderQ;

const struct
{
	const std::string ID = "FreqSpread";
	std::string name = "FreqSpread";
	std::string unitName = "";
	float minValue = -2.f;
	float maxValue = 2.f;
	float defaultValue = 0.f;
}paramEqoderFreqSpread;
const struct
{
	const std::string ID = "BWSpread";
	std::string name = "BWSpread";
	std::string unitName = "";
	float minValue = log(0.25f);
	float maxValue = log(4.f);
	float defaultValue = log(1.f);
}paramEqoderBWSpread;





class EqoderParameterComponent : public Component
{
public:
	EqoderParameterComponent(AudioProcessorValueTreeState& );

	void paint(Graphics& g) override;
	void resized() override;
    std::function<void()> somethingChanged;

private:
    AudioProcessorValueTreeState& m_vts; 

    Label m_NrOfFiltersLabel;
    Slider m_NrOfFiltersSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_NrOfFiltersAttachment;


};
