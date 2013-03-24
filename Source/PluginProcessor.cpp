/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


static const int kMaxBufferSize = 512;

//==============================================================================
ConvolverAudioProcessor::ConvolverAudioProcessor() : AudioProcessor(), ImpulseResponseListener(),
    parameters(), convolutionBufferSize(kMaxBufferSize),
    impulseResponseBuffersFreq(), inputSignalBufferFreq(2, kMaxBufferSize),
    convolutionResultBuffersFreq(), convolutionResultBuffersTimeDomain(),
    convolutionResult(2, kMaxBufferSize), convolutionResultTail(2, kMaxBufferSize)
{
    fftWrapper = new FFTWrapper(kMaxBufferSize);
}

ConvolverAudioProcessor::~ConvolverAudioProcessor()
{
}

//==============================================================================
void ConvolverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // TODO: Reallocate FFTWrapper
}

void ConvolverAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ConvolverAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);

        // ..do something to the data...
    }

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
AudioProcessorEditor* ConvolverAudioProcessor::createEditor()
{
    ConvolverAudioProcessorEditor *editor = new ConvolverAudioProcessorEditor(this);
    editor->setOnImpulseResponseSelectedListener(this);
    return editor;
}

void ConvolverAudioProcessor::onImpulseResponseSelected(const File& file)
{
    ImpulseResponseLoader loader(fftWrapper);
    // TODO: Check error
    loader.loadFile(file, impulseResponseBuffersFreq, kMaxBufferSize);
}

//==============================================================================
void ConvolverAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ConvolverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ConvolverAudioProcessor();
}
