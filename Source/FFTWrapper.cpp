/*
  ==============================================================================

    FFTWrapper.cpp
    Created: 24 Mar 2013 1:49:01pm
    Author:  nik

  ==============================================================================
*/

#include "FFTWrapper.h"

FFTWrapper::FFTWrapper(int bufferSize)
{
#if USE_FFT_FIXED_SIZE
// TODO: Find best fit for size
#else
    fftProcessor = new ffft::FFTReal<float>(bufferSize);
#endif
}

FFTWrapper::~FFTWrapper()
{
    if (fftProcessor) {
        delete fftProcessor;
    }
}

void FFTWrapper::doFFT(const float* inputData, float* outputData) const
{
    fftProcessor->do_fft(outputData, inputData);
}

void FFTWrapper::doIFFT(const float* inputData, float* outputData) const
{
    fftProcessor->do_ifft(inputData, outputData);
    fftProcessor->rescale(outputData);
}
