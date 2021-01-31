#include "DecoupledPeakEQ.h"

DecoupledPeakEQ::DecoupledPeakEQ()
:m_fs(44100.0),m_freq(1000.0),m_gain(1.0),m_Q(1.0)
{

    reset();
}
DecoupledPeakEQ::DecoupledPeakEQ(double fs)
:m_fs(fs),m_freq(1000.0),m_gain(1.0),m_Q(1.0)
{

    reset();
}
DecoupledPeakEQ::DecoupledPeakEQ(double fs, double freq, double gain, double Q)
:m_fs(fs),m_freq(freq),m_gain(gain),m_Q(Q)
{

    reset();
}
DecoupledPeakEQ::~DecoupledPeakEQ()
{

}
int DecoupledPeakEQ::processData(std::vector<double>& data)
{
    for (auto kk = 0u; kk < data.size(); ++kk)
    {
        // allpass first
        // k2 first
        double in = data[kk];
        double upper2 = m_k2*(in - m_state2);
        // k1
        double out2 = in + upper2;
        double upper1 = m_k2*(out2-m_state1);
        m_state1 = upper1 + out2;
        m_state2 = upper1 + m_state1;
        double out = m_state2 + upper2;

        // gain
        data[kk] = 0.5*((in + out) + m_gain*(in - out));
    }

    return 0;
}
// DecoupledPeakEQ::processData(AudioBlock audio); // alternative with Juce AudioBlock
void DecoupledPeakEQ::reset()
{
    m_state1 = m_state2 = 0.0;

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

void DecoupledPeakEQ::setQ(double newQ)
{
    m_Q = newQ;

}
void DecoupledPeakEQ::setFreqency(double newFreq)
{
    m_freq = newFreq;

}
void DecoupledPeakEQ::setSamplerate(double fs)
{
    m_fs = fs;
}
void DecoupledPeakEQ::computeCoeffs()
{
    m_k1 = -cos(2.0*M_PI*m_freq/m_fs);
    double B = M_PI*m_freq/m_Q;
    m_k2 = (1-tan(B))/(1+tan(B));
}
