/**
 * @file DecoupledPeakEQ.h
 * @author J. Bitzer  @ Jade Hochschule
 * @brief a decoupled 2nd order EQ filter based on a Regalia Mitra structure
 * @version 0.0.1
 * @date 2021-01-31
 * 
 * @copyright Copyright (c) 2021, MIT License 
 * 
 */
/* this class is based on 
Regalia, Phillip A., Sanjit K. Mitra, and P. P. Vaidyanathan. "The digital all-pass filter: A versatile signal processing building block." Proceedings of the IEEE 76.1 (1988): 19-37.
*/

/* Todo:
JUCE Audioblock implementation
*/
#pragma once

#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <vector>
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

class DecoupledPeakEQ
{
public:
    DecoupledPeakEQ();
    DecoupledPeakEQ(double fs);
    DecoupledPeakEQ(double fs, double freq, double gain, double Q);
    ~DecoupledPeakEQ();
    int processData(std::vector<double>& data);
    // processData(AudioBlock audio); // alternative with Juce AudioBlock
    void reset();
    void setGain(double newGain);
    void setGaindb(double newGain);
    void setQ(double newQ);
    void setFreqency(double newFreq);
    void setSamplerate(double fs);

private:
    double m_fs; // samplerate
    double m_freq;
    double m_gain;
    double m_Q;

    double m_state1,m_state2;
    double m_k1,m_k2;
    void computeCoeffs();

};
