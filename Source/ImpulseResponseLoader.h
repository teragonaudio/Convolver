/*
  ==============================================================================

    ImpulseResponseLoader.h
    Created: 24 Mar 2013 12:45:13pm
    Author:  nik

  ==============================================================================
*/

#ifndef __IMPULSERESPONSELOADER_H_8B889137__
#define __IMPULSERESPONSELOADER_H_8B889137__

#include "../JuceLibraryCode/JuceHeader.h"
#include "FFTWrapper.h"
#include <vector>

class ImpulseResponseListener {
public:
    ImpulseResponseListener() {}
    virtual ~ImpulseResponseListener() {}

    virtual void onImpulseResponseSelected(const File& file) = 0;
};

class ImpulseResponseLoader {
public:
    ImpulseResponseLoader(const FFTWrapper* fft) : fftWrapper(fft) {}
    virtual ~ImpulseResponseLoader() {}

    bool loadFile(const File& file, std::vector<AudioSampleBuffer*>& bufferFreq, int bufferSize) const;

private:
    const FFTWrapper* fftWrapper;
};

#endif  // __IMPULSERESPONSELOADER_H_8B889137__
