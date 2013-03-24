/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINPROCESSOR_H_C821754D__
#define __PLUGINPROCESSOR_H_C821754D__

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>

#include "FFTWrapper.h"
#include "ImpulseResponseLoader.h"
#include "PluginParameters.h"

using namespace teragon;

//==============================================================================
class ConvolverAudioProcessor : public AudioProcessor, public ImpulseResponseListener
{
public:
    //==============================================================================
    ConvolverAudioProcessor();
    ~ConvolverAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    const String getName() const { return JucePlugin_Name; }

    int getNumParameters() { return parameters.size(); }
    float getParameter (int index) { return parameters[index]->getScaledValue(); }
    void setParameter (int index, float newValue) { parameters[index]->setScaledValue(newValue); }
    const String getParameterName (int index) { return parameters[index]->getName().c_str(); }
    const String getParameterText (int index) { return parameters[index]->getDisplayText().c_str(); }

    const String getInputChannelName (int channelIndex) const { return String(channelIndex + 1); }
    const String getOutputChannelName (int channelIndex) const { return String(channelIndex + 1); }
    bool isInputChannelStereoPair (int index) const { return true; }
    bool isOutputChannelStereoPair (int index) const { return true; }

    bool acceptsMidi() const { return false; }
    bool producesMidi() const { return false; }
    bool silenceInProducesSilenceOut() const { return false; }
    // TODO: Need an actual value here
    double getTailLengthSeconds() const { return 0.0f; }

    //==============================================================================
    int getNumPrograms() { return 0; }
    int getCurrentProgram() { return 0; }
    void setCurrentProgram (int index) {}
    const String getProgramName (int index) { return String::empty; }
    void changeProgramName (int index, const String& newName) {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    bool hasEditor() const { return true; }
    AudioProcessorEditor* createEditor();
    void onImpulseResponseSelected(const File& file);

private:
    PluginParameterSet parameters;

    FFTWrapper *fftWrapper;

    int convolutionBufferSize;
    std::vector<AudioSampleBuffer*> impulseResponseBuffersFreq;
    AudioSampleBuffer inputSignalBufferFreq;
    std::vector<AudioSampleBuffer*> convolutionResultBuffersFreq;
    std::vector<AudioSampleBuffer*> convolutionResultBuffersTimeDomain;
    AudioSampleBuffer convolutionResult;
    AudioSampleBuffer convolutionResultTail;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvolverAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_C821754D__
