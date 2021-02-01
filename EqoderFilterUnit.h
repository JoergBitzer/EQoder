#pragma once
#include <vector>
#include "DecoupledPeakEQ.h"
#include "ValueMapper.h"

class EQoderFilterUnit
{
public:
    EQoderFilterUnit();
    EQoderFilterUnit(double fs);
    ~EQoderFilterUnit();

    void setSamplerate(double fs);
    int processData(std::vector<double>& data);
    void setNrOfFilters(int nroffilters);
    void setMaxGainf0(double maxGain_dB){m_maxGainf0 = maxGain_dB;setGains();};
    void setMaxGainfend(double maxGain_dB){m_maxGainfend = maxGain_dB;setGains();};
    void setGainForm(double form){m_gainform = form; setGains();};
    void setFundamentalFrequency(double freq);

    // parameter
    void setBWSpread(double bwspread);// 0..1
    void setQ(double Q);


private:
    const int m_maxnroffilters = 20;
    double m_fs;
    double m_Q;
    double m_f0;
    double m_maxGainf0;
    double m_maxGainfend;
    double m_gainform;
    int m_nroffilters;
    double m_bwspread;
    std::vector<DecoupledPeakEQ> m_filters;
    std::vector<double> m_Bandwidths;
    std::vector<double> m_Gains;

    void checknroffilters();
    void setFilters();
    void setGains();

    ValueMapper m_valmap;
};
