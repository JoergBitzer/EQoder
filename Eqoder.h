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
    std::atomic<float>* m_nrOfFilterUnits;
    float m_nrOfFilterUnitsOld;

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

    std::atomic<float>* m_OutGain;
    float m_OutGainOld;
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
    int m_maxunits;

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
    double m_OutGain;

};

const struct
{
	const std::string ID = "NrOfFilterUnits";
	std::string name = "Number of FilterUnits";
	std::string unitName = "";
	float minValue = 1.f;
	float maxValue = 8.f;
	float defaultValue = 4.f;
}paramEqoderNrOfFilterUnits;
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
	float minValue = -2.1f;
	float maxValue = 2.1f;
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

const struct
{
	const std::string ID = "OutGain";
	std::string name = "OutGain";
	std::string unitName = "dB";
	float minValue = -90.f;
	float maxValue = 10.f;
	float defaultValue = 0.f;
}paramEqoderOutGain;





class EqoderParameterComponent : public Component
{
public:
	EqoderParameterComponent(AudioProcessorValueTreeState& );

	void paint(Graphics& g) override;
	void resized() override;
    std::function<void()> somethingChanged;
    void setScaleFactor(float newscale){m_scaleFactor = newscale;};

private:
    AudioProcessorValueTreeState& m_vts; 

    Label m_NrOfFilterUnitsLabel;
    Slider m_NrOfFilterUnitsSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_NrOfFilterUnitsAttachment;

    Label m_NrOfFiltersLabel;
    Slider m_NrOfFiltersSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_NrOfFiltersAttachment;

    Label m_GainF0Label;
    Slider m_GainF0Slider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_GainF0Attachment;

    Label m_GainFendLabel;
    Slider m_GainFendSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_GainFendAttachment;

    Label m_GainFormLabel;
    Slider m_GainFormSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_GainFormAttachment;

    Label m_QLabel;
    Slider m_QSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_QAttachment;

    Label m_FreqSpreadLabel;
    Slider m_FreqSpreadSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_FreqSpreadAttachment;

    Label m_BWSpreadLabel;
    Slider m_BWSpreadSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_BWSpreadAttachment;

    Label m_OutGainLabel;
    Slider m_OutGainSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> m_OutGainAttachment;

    float m_scaleFactor;

};
