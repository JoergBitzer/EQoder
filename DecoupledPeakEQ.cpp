#include <cassert>
#include <cmath>
#include <JuceHeader.h>
#include "DecoupledPeakEQ.h"

DecoupledPeakEQ::DecoupledPeakEQ()
:m_fs(44100.0),m_freq(1000.0),m_gain(4.0),m_Q(1.0),m_BW(100.0)
{
    computeCoeffs();
    reset();
}
DecoupledPeakEQ::DecoupledPeakEQ(double fs)
:m_fs(fs),m_freq(1000.0),m_gain(1.0),m_Q(1.0),m_BW(100.0)
{
    computeCoeffs();
    reset();
}
DecoupledPeakEQ::DecoupledPeakEQ(double fs, double freq, double gain, double Q)
:m_fs(fs),m_freq(freq),m_gain(gain),m_Q(Q),m_BW(100.0)
{
    computeCoeffs();
    reset();
}
DecoupledPeakEQ::~DecoupledPeakEQ()
{

}
int DecoupledPeakEQ::processData(std::vector<double>& data)
{
    for (auto kk = 0u; kk < data.size(); ++kk)
    //for(auto in : data)
    {
        // allpass first
        // k2 first
        double in = data[kk];
        double upper2 = m_k2*(in - m_state2);
        // k1
        double out2 = in + upper2;
        double upper1 = m_k1*(out2 - m_state1);
        double out = m_state2 + upper2;
        m_state2 = upper1 + m_state1;
        m_state1 = upper1 + out2;

        // gain
        data[kk] = 0.5*(in + out) + 0.5*m_gain*(in - out);

    }

    return 0;
}
int  DecoupledPeakEQ::processDataWithEnvelope(std::vector<double>& data,const std::vector<double>& envdata)
{
    processDataWithEnvelope(data, data, envdata);
    return 0;   
}
int  DecoupledPeakEQ::processDataWithEnvelope(std::vector<double>& indata,std::vector<double>& outdata,const std::vector<double>& envdata)
{
     for (auto kk = 0u; kk < indata.size(); ++kk)
    //for(auto in : data)
    {
        // allpass first
        // k2 first
        double in = indata[kk];
        double upper2 = m_k2*(in - m_state2);
        // k1
        double out2 = in + upper2;
        double upper1 = m_k1*(out2 - m_state1);
        double out = m_state2 + upper2;
        m_state2 = upper1 + m_state1;
        m_state1 = upper1 + out2;
        
        if (m_state2>m_maxVal)
            m_state2 = m_maxVal;
        
        if (m_state2 < -m_maxVal)
            m_state2 = -m_maxVal;

        // gain
        outdata[kk] = 0.5*(in + out) + 0.5*(((m_gain-1.0)*envdata[kk])+1.0) *(in - out);

        if (!std::isfinite(outdata[kk] ))
        {
            switch(std::fpclassify(outdata[kk])) 
            {
                case FP_INFINITE:  DBG( "Inf");break;
                case FP_NAN:       DBG( "NaN");break;
                case FP_NORMAL:    DBG( "normal");break;
                case FP_SUBNORMAL: DBG( "subnormal");break;
                case FP_ZERO:      DBG( "zero");break;
                default:           DBG( "unknown");break;
            }
        }	


    
        if (fabs(m_state1)>1e7)
            reset();

        if (fabs(m_state2)>1e7)
            reset();
    }
    return 0;   
}



// DecoupledPeakEQ::processData(AudioBlock audio); // alternative with Juce AudioBlock
void DecoupledPeakEQ::reset()
{
    m_state1 = 0.0;
    m_state2 = 0.0;

}
void DecoupledPeakEQ::setGain(double newGain)
{
    m_gain = newGain;
}
void DecoupledPeakEQ::setGaindb(double newGain_dB)
{
    double gain = pow(10.0,newGain_dB*0.05);
    setGain(gain);
}
/* cannot be quaranteed for changing frequency
void DecoupledPeakEQ::setQ(double newQ)
{
    m_Q = newQ;
    setBWviaQ();
    computeCoeffs();
}
//*/
void DecoupledPeakEQ::setBandwidth(double BW_Hz)
{
    m_BW = BW_Hz;
    computek2Bandwidth();
}
void DecoupledPeakEQ::setFreqency(double newFreq)
{
    m_freq = newFreq;
    computeCoeffs();
}
void DecoupledPeakEQ::setSamplerate(double fs)
{
    m_fs = fs;
    computeCoeffs();
    reset();
}
void DecoupledPeakEQ::computeCoeffs()
{
    computek1Freq();
    computek2Bandwidth();
}
void DecoupledPeakEQ::computek1Freq()
{
    m_k1 = -cos(2.0*M_PI*m_freq/m_fs);
    //assert(fabs(m_k1) < 1.0);
    if (fabs(m_k1) >= 1.0)
        m_k1 = 0.0;
}
void DecoupledPeakEQ::setBWviaQ()
{
    m_BW = m_freq/m_Q;
    computek2Bandwidth();
}
void DecoupledPeakEQ::computek2Bandwidth()
{
    double B = (2.0*M_PI*m_BW)/m_fs;
    m_k2 = (1-tan(B))/(1+tan(B)); 
    //assert(fabs(m_k2) < 1.0);
    if (fabs(m_k2) >= 1.0)
        m_k2 = 0.0;   
}









