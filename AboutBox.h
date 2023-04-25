#pragma once
//#include "JuceHeader.h"
#include <juce_gui_basics/juce_gui_basics.h>

class AboutBoxComponent : public juce::Component
{
public:
    void setImage(juce::Image &aboutimage){m_AboutBox = aboutimage;};
	void paint(Graphics& g) 
    {
        if (m_isvisible)
        {
            int width = getWidth();
            int height = getHeight();

            g.fillAll(Colour::fromFloatRGBA(0.352941176470588, 0.372549019607843, 0.337254901960784, 0.5));
            g.drawImage(m_AboutBox, width / 2 - m_AboutBox.getWidth() / 2, height / 2 - m_AboutBox.getHeight() / 2,
                    m_AboutBox.getWidth(), m_AboutBox.getHeight(), 0, 0, m_AboutBox.getWidth(), m_AboutBox.getHeight());
        }    
    };
	void resized()
    {};
    void setScaleFactor(float newscale){m_scaleFactor = newscale;};
    void setIsVisible(bool newstatus){
        m_isvisible = newstatus;
        };
    void mouseDown(const MouseEvent &event)
    {
        m_isvisible = false;
        setVisible(false);
    };
private:
    bool m_isvisible = false;
    float m_scaleFactor = 1.f;
    juce::Image m_AboutBox;
};