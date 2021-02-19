#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
#if WITH_MIDIKEYBOARD   
EQoderAudioProcessorEditor::EQoderAudioProcessorEditor (EQoderAudioProcessor& p)
    : AudioProcessorEditor (&p), m_processorRef (p), m_presetGUI(p.m_presets),
    	m_keyboard(m_processorRef.m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard),
        m_eqparamcomponent(*m_processorRef.m_parameterVTS),m_envelopecomponent(*m_processorRef.m_parameterVTS,0,"Env1")
#else
EQoderAudioProcessorEditor::EQoderAudioProcessorEditor (EQoderAudioProcessor& p)
    : AudioProcessorEditor (&p), m_processorRef (p), m_presetGUI(p.m_presets)
#endif
{
    setSize (g_minGuiSize_x, g_minGuiSize_x*g_guiratio);
    setResizeLimits (g_minGuiSize_x,g_minGuiSize_x*g_guiratio , g_maxGuiSize_x, g_maxGuiSize_x*g_guiratio);
    getConstrainer()->setFixedAspectRatio(1./g_guiratio);

	addAndMakeVisible(m_presetGUI);
#if WITH_MIDIKEYBOARD      
	addAndMakeVisible(m_keyboard);
#endif
    
    addAndMakeVisible(m_eqparamcomponent);
    m_eqparamcomponent.somethingChanged = [this]() {m_presetGUI.setSomethingChanged(); };

    addAndMakeVisible(m_envelopecomponent);
    m_envelopecomponent.somethingChanged = [this]() {m_presetGUI.setSomethingChanged(); };

}

EQoderAudioProcessorEditor::~EQoderAudioProcessorEditor()
{
}

//==============================================================================
void EQoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));


}
const int g_minPresetHandlerHeight(30);
const float g_midikeyboardratio(0.13);
void EQoderAudioProcessorEditor::resized()
{
    int height = getHeight();
    // necessary to change fontsize of comboboxes and PopUpmenus
    // 0.5 is a good compromisecould be slightly higher or lower
    m_jadeLAF.setFontSize(0.5*height*g_minPresetHandlerHeight/g_minGuiSize_y);
    // top presethandler
    m_presetGUI.setBounds(0, 0, getWidth(), height*g_minPresetHandlerHeight/g_minGuiSize_y);
    // bottom a small midkeyboard
#if WITH_MIDIKEYBOARD    
    m_keyboard.setBounds(0, height*(1-g_midikeyboardratio), getWidth(), height*g_midikeyboardratio);
#endif
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	// int newWidth = getWidth();
	// int newHeight = getHeight();

	int width = getWidth();

	float scaleFactor = float(width)/g_minGuiSize_x;
    m_eqparamcomponent.setScaleFactor(scaleFactor);
    m_eqparamcomponent.setBounds(scaleFactor*EQODER_MIN_XPOS,scaleFactor*EQODER_MIN_YPOS, scaleFactor*EQODER_MIN_WIDTH, scaleFactor*EQODER_MIN_HEIGHT);
    
    m_envelopecomponent.setScaleFactor(scaleFactor);
    m_envelopecomponent.setBounds(scaleFactor*ENVELOPE1_MIN_XPOS,scaleFactor*ENVELOPE1_MIN_YPOS,scaleFactor*ENVELOPE1_MIN_WIDTH,scaleFactor*ENVELOPE1_MIN_HEIGHT);
 }
