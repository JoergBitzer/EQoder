#pragma once

#include "EqoderGUISettings.h"
#include "PluginProcessor.h"
#include "JadeLookAndFeel.h"
#include "PresetHandler.h"
#include "Eqoder.h"
#include "Envelope.h"
#include "SimpleMeter.h"


//==============================================================================
class EQoderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    
    explicit EQoderAudioProcessorEditor (EQoderAudioProcessor&);
    ~EQoderAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JadeLookAndFeel m_jadeLAF;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EQoderAudioProcessor& m_processorRef;
    PresetComponent m_presetGUI;
#if WITH_MIDIKEYBOARD    
    MidiKeyboardComponent m_keyboard;
#endif
    // plugin specific components
    EqoderParameterComponent m_eqparamcomponent;
    EnvelopeParameterComponent m_envelopecomponent;
    SimpleMeterComponent m_leveldisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQoderAudioProcessorEditor)
};
