
#include "EqoderFilterUnit.h"

EQoderFilterUnit::EQoderFilterUnit()
:m_nroffilters(5),m_fs(44100.0),m_Q(20.0),m_f0(2000.0),m_bwspread(1.0),
m_maxGainf0(10.0),m_maxGainfend(1.0), m_gainform(1.0), m_freqspread(0.0), m_nrofchannels(2)
{
    m_filters.resize(m_maxnrofchannels);
    for (auto kk = 0u; kk < m_filters.size(); ++kk)
        m_filters[kk].resize(m_maxnroffilters);

    m_Bandwidths.resize(m_maxnroffilters);
    m_Gains.resize(m_maxnroffilters);

    setFundamentalFrequency(m_f0);
    setGains();
    setFilters();

    // dynmaics
    m_env.setSamplerate(m_fs);
    m_env.setDelayTime(0.0);
    m_env.setAttackRate(200.0);
    m_env.setDecayRate(500.0);
    m_env.setSustainLevel(1.0);
    m_env.setHoldTime(0.0);
    m_env.setReleaseRate(5000.0);
    
    reset();
    // kust for debugging
    // m_env.NoteOn();

}
EQoderFilterUnit::EQoderFilterUnit(double fs)
:m_nroffilters(5),m_fs(fs),m_Q(4.0),m_f0(1000.0),m_bwspread(0.0),
m_maxGainf0(10.0),m_maxGainfend(10.0), m_gainform(1.0), m_freqspread(0.0), m_nrofchannels(2)
{
    m_filters.resize(m_maxnrofchannels);
    for (auto kk = 0u; kk < m_filters.size(); ++kk)
        m_filters[kk].resize(m_maxnroffilters);


    m_Bandwidths.resize(m_maxnroffilters);
    m_Gains.resize(m_maxnroffilters);
    setFundamentalFrequency(m_f0);
    setGains();
    setFilters();
    // dynmaics
    m_env.setSamplerate(m_fs);
    m_env.setDelayTime(0.0);
    m_env.setAttackRate(6000.0);
    m_env.setDecayRate(500.0);
    m_env.setSustainLevel(0.0);
    m_env.setHoldTime(0.0);
    m_env.setReleaseRate(500.0);
    // just for debugging
    reset();

}
EQoderFilterUnit::~EQoderFilterUnit()
{

}
void EQoderFilterUnit::reset()
{
    for (auto onefiltervec : m_filters)
    {
        for (auto onefilter : onefiltervec)
            onefilter.reset();
    }
    
}
void EQoderFilterUnit::setSamplerate(double fs)
{
    m_fs = fs;
    checknroffilters();
    for (auto onefiltervec : m_filters)
    {
        for (auto onefilter : onefiltervec)
            onefilter.setSamplerate(m_fs);
    }
    setFilters();
    m_env.setSamplerate(m_fs);
}
int EQoderFilterUnit::processData(std::vector<std::vector<double>>& data)
{
    // one envelope for all channels
    m_envdata.resize(data[0].size());
    m_env.getData(m_envdata);

    for (auto cc = 0u; cc < data.size(); ++cc)
    {
        for (auto kk = 0u; kk < m_nroffilters; ++kk)
        {
            m_filters[cc][kk].processDataWithEnvelope(data[cc],m_envdata);
        }
    }
    return 0;
}
void EQoderFilterUnit::setNrOfFilters(int nroffilters)
{
    m_nroffilters = nroffilters;
    checknroffilters();

}

void EQoderFilterUnit::setFundamentalFrequency(double freq, double Velocity)
{
    m_f0 = freq;
    checknroffilters();

    for (auto kk = 0u; kk < m_Bandwidths.size(); ++kk)
    {
        m_Bandwidths[kk] = m_f0*pow(double(kk+1),m_bwspread)/m_Q;
    }

    setFilters();
    m_velocity = Velocity;
    m_env.setMaxLevel(m_velocity);
    m_env.NoteOn();
}

void EQoderFilterUnit::setBWSpread(double bwspread)
{
    if (bwspread >= -2.0 && bwspread <= 2.0)
        m_bwspread = bwspread;

    for (auto kk = 0u; kk < m_Bandwidths.size(); ++kk)
    {
        m_Bandwidths[kk] = m_f0*pow(double(kk+1),m_bwspread)/m_Q;
    }
    setFilters();
}
void EQoderFilterUnit::setQ(double Q)
{
    m_Q = Q;
    setBWSpread(m_bwspread);
}
void EQoderFilterUnit::setFreqSpread(double freqspread)
{
    m_freqspread = freqspread;
    checknroffilters();
    setFilters();
}


void EQoderFilterUnit::checknroffilters()
{
    int maxtoNyquist = int(0.5*m_fs/(m_f0*pow(2.0,m_freqspread))); //-1 for security

    if (m_nroffilters>maxtoNyquist)
        m_nroffilters = maxtoNyquist;

    if (m_nroffilters>m_maxnroffilters)
        m_nroffilters = m_maxnroffilters;

}
void EQoderFilterUnit::setFilters()
{
    for (auto cc = 0u; cc < m_filters.size(); ++cc)
    {
        for (auto kk = 0u; kk < m_filters[cc].size(); ++kk)
        {
            // m_filters[kk].setFreqency(m_f0*pow(2.0,m_freqspread)*(kk+1));
            m_filters[cc][kk].setFreqency(m_f0 + m_f0*pow(2.0,m_freqspread)*(kk));
            m_filters[cc][kk].setBandwidth(m_Bandwidths[kk]);
        }
    }
}
 void EQoderFilterUnit::setGains()
 {   
     m_valmap.sety1(m_maxGainf0);
     m_valmap.sety2(m_maxGainfend);
     m_valmap.setForm(m_gainform);
     m_valmap.setx1(m_f0);
     m_valmap.setx2(m_f0*m_nroffilters+0.1); // + 0.1 to prevent division 0
 
    for (auto cc = 0u; cc < m_filters.size(); ++cc)
    {
        for (auto kk = 0u; kk < m_filters[cc].size(); ++kk)
        {
            double gain_dB = m_valmap.getValue(m_f0*(kk+1));
            double gain = pow(10.0,gain_dB/20.0);
            m_Gains[kk] = gain;
            m_filters[cc][kk].setGain(gain);
        }
    }

 }