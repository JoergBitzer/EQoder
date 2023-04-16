#pragma once
#include <vector>
#include "DecoupledPeakEQ.h"
#include "ValueMapper.h"
#include "Envelope.h"
 const int m_maxnroffilters(20);
 const int m_maxnrofchannels(8);
 
class EQoderFilterUnit
{
public:
    EQoderFilterUnit();
    EQoderFilterUnit(double fs);
    ~EQoderFilterUnit();

    void setSamplerate(double fs);
    int processData(std::vector<std::vector<double>>& data);
    int processData(std::vector<std::vector<double>>& indata, std::vector<std::vector<double>>& outdata);
    void reset();

    
    void setNrOfFilters(int nroffilters);
    void setMaxGainf0(double maxGain_dB){m_maxGainf0 = maxGain_dB;setGains();};
    void setMaxGainfend(double maxGain_dB){m_maxGainfend = maxGain_dB;setGains();};
    void setGainForm(double form){m_gainform = form; setGains();};
    void setFundamentalFrequency(double freq, double Velocity = 1.0);
    void NoteOff(){m_env.NoteOff();};

    // parameter
    void setBWSpread(double bwspread);// 0..1
    void setQ(double Q);
    void setFreqSpread(double freqspread); // -4...4
    int setNrOfChannels(int nrofchannels)
    {
        if (nrofchannels <= m_maxnrofchannels) 
        {    
            m_nrofchannels = nrofchannels;
            return 0;
        }else
        {
            return -1;
        }
    };

    float getEnvelopeStatus (Envelope::envelopePhases& envphase)
    {
        envphase = m_env.getEnvelopeStatus();
        return m_envdata.back()*m_velocity;
    }

    // Envelope
    void setDelayTime(double del_ms){m_env.setDelayTime(del_ms);};
	void setAttackRate(double att_ms){m_env.setAttackRate(att_ms);};
	void setHoldTime(double hold_ms){m_env.setHoldTime(hold_ms);};
	void setDecayRate(double dec_ms){m_env.setDecayRate(dec_ms);};
	void setSustainLevel(double level){m_env.setSustainLevel(level);};
	void setReleaseRate(double rel_ms){m_env.setReleaseRate(rel_ms);};
	void setInvertOnOff(bool onoff){m_env.setInvertOnOff(onoff);};

private:
    int m_nrofchannels;
    unsigned int m_nroffilters;

    double m_fs;
    double m_Q;
    double m_f0;
    double m_bwspread;
    double m_maxGainf0;
    double m_maxGainfend;
    double m_gainform;
    double m_freqspread;
    double m_velocity;
    std::vector<std::vector<DecoupledPeakEQ>> m_filters;
    std::vector<double> m_Bandwidths;
    std::vector<double> m_Gains;

    void checknroffilters();
    void setFilters();
    void setGains();

    ValueMapper m_valmap;

    // Dynamics
    Envelope m_env;
    std::vector<double> m_envdata;

    // Modulation
    // 1 Envelope + 1-2 LFOs  + Midi (Pitch Bend / ModWheel)
};
