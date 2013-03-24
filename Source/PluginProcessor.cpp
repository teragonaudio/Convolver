/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ConvolverAudioProcessor::ConvolverAudioProcessor()
{
}

ConvolverAudioProcessor::~ConvolverAudioProcessor()
{
}

//==============================================================================
const String ConvolverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int ConvolverAudioProcessor::getNumParameters()
{
    return 0;
}

float ConvolverAudioProcessor::getParameter (int index)
{
    return 0.0f;
}

void ConvolverAudioProcessor::setParameter (int index, float newValue)
{
}

const String ConvolverAudioProcessor::getParameterName (int index)
{
    return String::empty;
}

const String ConvolverAudioProcessor::getParameterText (int index)
{
    return String::empty;
}

const String ConvolverAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ConvolverAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ConvolverAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ConvolverAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ConvolverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ConvolverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ConvolverAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double ConvolverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ConvolverAudioProcessor::getNumPrograms()
{
    return 0;
}

int ConvolverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ConvolverAudioProcessor::setCurrentProgram (int index)
{
}

const String ConvolverAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void ConvolverAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ConvolverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
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
bool ConvolverAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ConvolverAudioProcessor::createEditor()
{
    return new ConvolverAudioProcessorEditor (this);
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
