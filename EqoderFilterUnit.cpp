
#include "EqoderFilterUnit.h"

EQoderFilterUnit::EQoderFilterUnit()
:m_nroffilters(5),m_fs(44100.0),m_Q(20.0),m_f0(2000.0),m_bwspread(1.0),
m_maxGainf0(10.0),m_maxGainfend(1.0), m_gainform(1.0)
{
    m_filters.resize(m_maxnroffilters);
    m_Bandwidths.resize(m_maxnroffilters);
    m_Gains.resize(m_maxnroffilters);

    setFundamentalFrequency(m_f0);
    setGains();
    setFilters();
}
EQoderFilterUnit::EQoderFilterUnit(double fs)
:m_nroffilters(5),m_fs(fs),m_Q(4.0),m_f0(1000.0),m_bwspread(0.0),
m_maxGainf0(10.0),m_maxGainfend(1.0), m_gainform(1.0)
{
    m_filters.resize(m_maxnroffilters);
    m_Bandwidths.resize(m_maxnroffilters);
    m_Gains.resize(m_maxnroffilters);
    setFundamentalFrequency(m_f0);
    setGains();
    setFilters();
}
EQoderFilterUnit::~EQoderFilterUnit()
{

}

void EQoderFilterUnit::setSamplerate(double fs)
{
    m_fs = fs;
    checknroffilters();
    for (auto onefilter : m_filters)
    {
        onefilter.setSamplerate(m_fs);
    }
    setFilters();

}
int EQoderFilterUnit::processData(std::vector<double>& data)
{
    for (auto kk = 0u; kk < m_nroffilters; ++kk)
    {
        m_filters[kk].processData(data);
    }

    return 0;
}
void EQoderFilterUnit::setNrOfFilters(int nroffilters)
{
    m_nroffilters = nroffilters;
    checknroffilters();

}

void EQoderFilterUnit::setFundamentalFrequency(double freq)
{
    m_f0 = freq;
    checknroffilters();

    for (auto kk = 0u; kk < m_Bandwidths.size(); ++kk)
    {
        m_Bandwidths[kk] = m_f0*pow(double(kk+1),m_bwspread)/m_Q;
    }

    setFilters();
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
void EQoderFilterUnit::checknroffilters()
{
    int maxtoNyquist = int(0.5*m_fs/m_f0)-1; //-1 for security

    if (m_nroffilters>maxtoNyquist)
        m_nroffilters = maxtoNyquist;

    if (m_nroffilters>m_maxnroffilters)
        m_nroffilters = m_maxnroffilters;

}
void EQoderFilterUnit::setFilters()
{
    for (auto kk = 0u; kk < m_filters.size(); ++kk)
    {
        m_filters[kk].setFreqency(m_f0*(kk+1));
        m_filters[kk].setBandwidth(m_Bandwidths[kk]);
    }
}
 void EQoderFilterUnit::setGains()
 {   
     m_valmap.sety1(m_maxGainf0);
     m_valmap.sety2(m_maxGainfend);
     m_valmap.setForm(m_gainform);
     m_valmap.setx1(m_f0);
     m_valmap.setx2(m_f0*m_nroffilters);


  
    for (auto kk = 0u; kk<m_Gains.size();++kk)
    {
        double gain_dB = m_valmap.getValue(m_f0*(kk+1));
        double gain = pow(10.0,gain_dB/20.0);
        m_Gains[kk] = gain;
    // Todo : just for debugging. final this is for the envelope
        m_filters[kk].setGain(gain);
    }

 }