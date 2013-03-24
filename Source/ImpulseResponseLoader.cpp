/*
  ==============================================================================

    ImpulseResponseLoader.cpp
    Created: 24 Mar 2013 12:45:13pm
    Author:  nik

  ==============================================================================
*/

#include "ImpulseResponseLoader.h"

bool ImpulseResponseLoader::loadFile(const File& file, std::vector<AudioSampleBuffer*>& bufferFreq, int bufferSize) const
{
    AudioFormatManager formatManager;
    AudioFormatReader *reader = formatManager.createReaderFor(file);
    if (reader == nullptr) {
        return false;
    }

    // Reset bufferFreq, free all previous samples
    // TODO: We must stop processing while this is happening or else there is a high risk of bad stuff happening
    for (int i = 0; i < bufferFreq.size(); ++i) {
        delete bufferFreq.at(i);
    }
    bufferFreq.clear();

    int currentSample = 0;
    AudioSampleBuffer *bufferTimeDomain = new AudioSampleBuffer(2, bufferSize);
    while (currentSample <= reader->lengthInSamples) {
        bufferTimeDomain->clear();
        reader->read(bufferTimeDomain, 0, bufferSize, currentSample, true, true);
        AudioSampleBuffer *bufferFreqDomain = new AudioSampleBuffer(2, bufferSize);
        for (int i = 0; i < 2; ++i) {
            fftWrapper->doFFT(bufferTimeDomain->getSampleData(i), bufferFreqDomain->getSampleData(i));
        }
        bufferFreq.push_back(bufferFreqDomain);
        currentSample += bufferSize;
    }

    delete bufferTimeDomain;
    return true;
}
