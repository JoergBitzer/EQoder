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
        return m_envdata.back();
    }

private:
    int m_nrofchannels;
    int m_nroffilters;

    double m_fs;
    double m_Q;
    double m_f0;
    double m_maxGainf0;
    double m_maxGainfend;
    double m_gainform;
    double m_bwspread;
    double m_freqspread;
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

};
