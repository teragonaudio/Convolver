/*
  ==============================================================================

    FFTWrapper.h
    Created: 24 Mar 2013 1:49:01pm
    Author:  nik

  ==============================================================================
*/

#ifndef __FFTWRAPPER_H_2DA48785__
#define __FFTWRAPPER_H_2DA48785__

#define USE_FFT_FIXED_SIZE 0
#if USE_FFT_FIXED_SIZE
#include "FFTRealFixLen.h"
#else
#include "FFTReal.h"
#endif

class FFTWrapper {
public:
    FFTWrapper(int bufferSize);
    virtual ~FFTWrapper();

    void doFFT(const float* inputData, float* outputData) const;
    void doIFFT(const float* inputData, float* outputData) const;

private:
#if USE_FFT_FIXED_SIZE
    // TODO: Allocate several fixed-length processors, pick the best at runtime
#else
    ffft::FFTReal<float> *fftProcessor;
#endif
};

#endif  // __FFTWRAPPER_H_2DA48785__
