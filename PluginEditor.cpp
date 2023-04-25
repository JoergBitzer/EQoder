#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
#if WITH_MIDIKEYBOARD
EQoderAudioProcessorEditor::EQoderAudioProcessorEditor(EQoderAudioProcessor &p)
    : AudioProcessorEditor(&p), m_processorRef(p), m_presetGUI(p.m_presets),
      m_keyboard(m_processorRef.m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard),
      m_eqparamcomponent(*m_processorRef.m_parameterVTS), m_envelopecomponent(*m_processorRef.m_parameterVTS, 0, "Env1"),
      m_leveldisplay(m_processorRef.m_meter), m_aboutboxvisible(false)
#else
EQoderAudioProcessorEditor::EQoderAudioProcessorEditor(EQoderAudioProcessor &p)
    : AudioProcessorEditor(&p), m_processorRef(p), m_presetGUI(p.m_presets)
#endif
{
    setSize(g_minGuiSize_x, g_minGuiSize_x * g_guiratio);
    setResizeLimits(g_minGuiSize_x, g_minGuiSize_x * g_guiratio, g_maxGuiSize_x, g_maxGuiSize_x * g_guiratio);
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(1. / g_guiratio);

    addAndMakeVisible(m_presetGUI);
#if WITH_MIDIKEYBOARD
    addAndMakeVisible(m_keyboard);
#endif

    addAndMakeVisible(m_eqparamcomponent);
    m_eqparamcomponent.somethingChanged = [this]()
    { m_presetGUI.setSomethingChanged(); };

    addAndMakeVisible(m_envelopecomponent);
    m_envelopecomponent.somethingChanged = [this]()
    { m_presetGUI.setSomethingChanged(); };

    addAndMakeVisible(m_leveldisplay);

    m_TitleImage = ImageFileFormat::loadFrom(BinaryData::Title_png, BinaryData::Title_pngSize);
    // m_JadeLogo = ImageFileFormat::loadFrom(BinaryData::LogoJadeHochschule_jpg, BinaryData::LogoJadeHochschule_jpgSize);
    m_JadeLogo = ImageFileFormat::loadFrom(BinaryData::LogoJadeHochschuleTrans_png, BinaryData::LogoJadeHochschuleTrans_pngSize);
    m_AboutBox = ImageFileFormat::loadFrom(BinaryData::AboutBox_png, BinaryData::AboutBox_pngSize);
    m_about.setImage(m_AboutBox);
    addAndMakeVisible(m_about);
    m_about.setVisible(false);
}

EQoderAudioProcessorEditor::~EQoderAudioProcessorEditor()
{
}

//==============================================================================
void EQoderAudioProcessorEditor::paint(juce::Graphics &g)
{

    int width = getWidth();
    int height = getHeight();
    float scaleFactor = float(width) / g_minGuiSize_x;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(JadeGray.brighter(0.4));

    g.drawImage(m_TitleImage, scaleFactor * 10, scaleFactor * (ENVELOPE1_MIN_YPOS),
                scaleFactor * (m_TitleImage.getWidth() - 20), scaleFactor * (m_TitleImage.getHeight() - 10), 0, 0, m_TitleImage.getWidth(), m_TitleImage.getHeight());

    float LogoScaler = 0.5;
    // float newLogo_x = LogoSize*m_JadeLogo.getWidth()/m_JadeLogo.getHeight();
    g.drawImage(m_JadeLogo, scaleFactor * (460), scaleFactor * (ENVELOPE1_MIN_YPOS),
                LogoScaler * scaleFactor * m_JadeLogo.getWidth(), LogoScaler * scaleFactor * m_JadeLogo.getHeight(), 0, 0, m_JadeLogo.getWidth(), m_JadeLogo.getHeight());
    //*/

}
const int g_minPresetHandlerHeight(30);
const float g_midikeyboardHeight(60);
void EQoderAudioProcessorEditor::resized()
{
    int height = getHeight();
    int width = getWidth();
    float scaleFactor = float(width) / g_minGuiSize_x;
    // necessary to change fontsize of comboboxes and PopUpmenus
    // 0.5 is a good compromisecould be slightly higher or lower
    m_jadeLAF.setFontSize(0.5 * height * g_minPresetHandlerHeight / g_minGuiSize_y);
    // top presethandler
    m_presetGUI.setBounds(0, 0, getWidth(), scaleFactor * g_minPresetHandlerHeight);
    // bottom a small midkeyboard
#if WITH_MIDIKEYBOARD
    m_keyboard.setBounds(0, height - scaleFactor * g_midikeyboardHeight, width, scaleFactor * g_midikeyboardHeight);
#endif
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // int newWidth = getWidth();
    // int newHeight = getHeight();

    m_eqparamcomponent.setScaleFactor(scaleFactor);
    m_eqparamcomponent.setBounds(scaleFactor * (EQODER_MIN_XPOS), scaleFactor * (EQODER_MIN_YPOS), scaleFactor * (EQODER_MIN_WIDTH), scaleFactor * (EQODER_MIN_HEIGHT));

    m_envelopecomponent.setScaleFactor(scaleFactor);
    m_envelopecomponent.setBounds(scaleFactor * (ENVELOPE1_MIN_XPOS), scaleFactor * (ENVELOPE1_MIN_YPOS), scaleFactor * (ENVELOPE1_MIN_WIDTH), scaleFactor * (ENVELOPE1_MIN_HEIGHT));

    m_leveldisplay.setBounds(scaleFactor * (DISPLAY_MIN_XPOS), scaleFactor * (DISPLAY_MIN_YPOS), scaleFactor * (DISPLAY_MIN_WIDTH), scaleFactor * (DISPLAY_MIN_HEIGHT));

    m_about.setScaleFactor(scaleFactor);
    m_about.setBounds(0,0,width, height);
}
void EQoderAudioProcessorEditor::mouseDown(const MouseEvent &event)
{
    int x = event.getMouseDownX();
    int y = event.getMouseDownY();

    int w = getWidth();
    int h = getHeight();
    float scaleFactor = float(w) / g_minGuiSize_x;
    float LogoScaler = 0.5;
    if (m_aboutboxvisible == false)
    {
        // Is the Logo cicked
        if (x>(scaleFactor*(460)) & y > scaleFactor*(ENVELOPE1_MIN_YPOS) &
        x < (scaleFactor*(460) + LogoScaler*scaleFactor*m_JadeLogo.getWidth() )
        & y < (scaleFactor*(ENVELOPE1_MIN_YPOS)+LogoScaler*scaleFactor*m_JadeLogo.getHeight()))
        {
            m_about.setVisible(true);
            m_about.setIsVisible(true);
            m_aboutboxvisible = true;
        }
    }
    else
    {
        m_about.setIsVisible(false);
        m_aboutboxvisible = false;
    }

    //*/
    repaint();
}